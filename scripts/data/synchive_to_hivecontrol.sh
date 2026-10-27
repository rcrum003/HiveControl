#!/bin/bash
# Uploads this hive's local history (allhivedata) to HiveControl, in batches.
#
# Safe to run again: the cloud ignores readings it already has, and progress is
# kept in data/sync.state, so an interrupted run picks up where it stopped.
#
# Usage: synchive_to_hivecontrol.sh [--since YYYY-MM-DD] [--restart]
#   --since    only send readings recorded on or after this date
#   --restart  forget saved progress and start again from the oldest reading

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/cloud.inc

LOCALDATABASE=/home/HiveControl/data/hive-data.db
RUNDIR=/home/HiveControl/data
STATEFILE=$RUNDIR/sync.state
LOCKDIR=$RUNDIR/sync.lock
BATCH_SIZE=${SYNC_BATCH_SIZE:-500}   # ~350 KB a request; the cloud accepts up to 1 MB
PAUSE=${SYNC_PAUSE:-1}               # seconds between requests; the cloud allows about 1 a second

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

# Local column -> cloud field, the same fields hc_senddata sends live. Columns a
# device's database doesn't have yet (older versions) are skipped.
COLUMN_MAP="
date:hive_observation_time_local
date_utc:hive_observation_time_utc
hivetempf:hive_temp_f
hivetempc:hive_temp_c
hivehum:hive_humidity
hiveweight:hive_weight_lbs
in_count:hive_flight_in
out_count:hive_flight_out
weather_stationid:wx_station_id
observationdatetime:wx_observation_time_rfc822
weather_tempf:wx_temp_f
weather_humidity:wx_relative_humidity
weather_dewf:wx_dewpoint_f
weather_tempc:wx_temp_c
wind_mph:wx_wind_mph
wind_dir:wx_wind_dir
wind_degrees:wx_wind_degrees
wind_gust_mph:wx_wind_gust_mph
pressure_mb:wx_pressure_mb
pressure_in:wx_pressure_in
weather_dewc:wx_dewpoint_c
solarradiation:wx_solar_radiation
precip_1hr_in:wx_precip_1hr_in
precip_1hr_metric:wx_precip_1hr_metric
precip_today_in:wx_precip_today_in
precip_today_metric:wx_precip_today_metric
air_datetime:air_date
air_temp:air_temp
air_humidity:air_humidity
air_pm1:air_pm1
air_pm2_5:air_pm2_5
air_pm10:air_pm10
air_pm2_5_aqi:air_pm2_5_aqi
air_pm10_aqi:air_pm10_aqi
air_pm2_5_raw:air_pm2_5_raw
air_pm10_raw:air_pm10_raw
air_pm1_raw:air_pm1_raw
air_pressure:air_pressure
air_source:air_source
"

US=$'\x1f'   # field separator: never appears in readings, unlike commas
RS=$'\x1e'   # record separator

fail() {
	echo "SYNC STOPPED: $1"
	loglocal "$DATE" HIVECONTROL ERROR "History sync stopped: $1"
	exit 1
}

# select_list prints "col AS field, ..." for the mapped columns this database has.
select_list() {
	local have pair col list=""
	have=$(sqlite3 "$LOCALDATABASE" "PRAGMA table_info(allhivedata);" | cut -d'|' -f2 | tr 'A-Z' 'a-z')
	for pair in $COLUMN_MAP; do
		col=${pair%%:*}
		if echo "$have" | grep -qx "$col"; then
			list="$list, $col AS ${pair#*:}"
		fi
	done
	echo "${list#, }"
}

# to_json turns sqlite3's header + rows into a JSON array of records for this hive.
to_json() {
	/usr/bin/jq -R -s -c --arg id "$HIVEID" --arg name "$HIVENAME" '
		split("\u001e") | map(select(length > 0) | split("\u001f"))
		| .[0] as $cols | .[1:]
		| map([$cols, .] | transpose | map({key: .[0], value: .[1]}) | from_entries
			| del(.sync_rowid) + {hive_id: $id, hive_name: $name})'
}

# send_batch posts syncout.json; sets HTTP_CODE and BODY.
send_batch() {
	local response
	response=$(/usr/bin/curl --silent --show-error --retry 5 --max-time 120 -w "\n%{http_code}" \
		"$SYNCAPI_URL" --data-binary @"$RUNDIR/syncout.json" \
		-H 'Content-Type: application/json' -H "X-Requested-With: XMLHttpRequest" \
		-H "Authorization: Bearer $HIVEAPI")
	HTTP_CODE=${response##*$'\n'}
	BODY=${response%$'\n'*}
	[ "$BODY" = "$HTTP_CODE" ] && BODY=""
}

json_field() {
	echo "$BODY" | /usr/bin/jq -r ".$1 // empty" 2>/dev/null
}

#===========
# Options and checks
#===========
SINCE=""
while [ $# -gt 0 ]; do
	case "$1" in
		--since) shift; SINCE="$1"; shift ;;
		--restart) rm -f "$STATEFILE"; shift ;;
		*) echo "Usage: $0 [--since YYYY-MM-DD] [--restart]"; exit 2 ;;
	esac
done

if [[ -n "$SINCE" && ! "$SINCE" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]; then
	echo "--since needs a date like 2026-05-01"
	exit 2
fi
if [ -z "$HIVEAPI" ]; then
	echo "--- Unable to share with hivecontrol.org because HIVEAPI is not set"
	exit 1
fi
if [ -z "$HIVENAME" ]; then
	echo "--- Unable to share with hivecontrol.org because HIVENAME is not set"
	exit 1
fi

if ! mkdir "$LOCKDIR" 2>/dev/null; then
	echo "A history sync is already running (remove $LOCKDIR if it isn't)"
	exit 1
fi
trap 'rm -rf "$LOCKDIR" "$RUNDIR/syncout.txt" "$RUNDIR/syncout.json"' EXIT

COLUMNS=$(select_list)
case "$COLUMNS" in
	*hive_observation_time_local*) ;;
	*) fail "allhivedata has no date column" ;;
esac

LAST_ROWID=$(cat "$STATEFILE" 2>/dev/null)
[[ "$LAST_ROWID" =~ ^[0-9]+$ ]] || LAST_ROWID=0

# filter is the WHERE clause for readings not yet sent.
filter() {
	local where="rowid > $LAST_ROWID"
	[ -n "$SINCE" ] && where="$where AND date >= '$SINCE'"
	echo "$where"
}

TOTAL=$(sqlite3 "$LOCALDATABASE" "SELECT count(*) FROM allhivedata WHERE $(filter);")
if [ "${TOTAL:-0}" -eq 0 ]; then
	echo "Nothing new to send"
	exit 0
fi
echo "SYNCING $TOTAL readings, this may take a while"

#===========
# Send each batch, oldest first
#===========
SENT=0
REJECTED=0
while :; do
	BATCH=$(sqlite3 "$LOCALDATABASE" "SELECT count(*), max(rowid) FROM (SELECT rowid FROM allhivedata WHERE $(filter) ORDER BY rowid LIMIT $BATCH_SIZE);")
	COUNT=${BATCH%%|*}
	BATCH_LAST=${BATCH#*|}
	[ "${COUNT:-0}" -eq 0 ] && break

	sqlite3 -batch -header -separator "$US" -newline "$RS" "$LOCALDATABASE" \
		"SELECT rowid AS sync_rowid, $COLUMNS FROM allhivedata WHERE $(filter) ORDER BY rowid LIMIT $BATCH_SIZE;" \
		> "$RUNDIR/syncout.txt" || fail "could not read the local database"
	to_json < "$RUNDIR/syncout.txt" > "$RUNDIR/syncout.json" || fail "could not convert readings to JSON"

	send_batch
	ERROR=$(json_field error)
	ACTION=$(json_field action)
	case "$HTTP_CODE:$ERROR:$ACTION" in
		200::)
			;;
		200:INVALID_DATA:)
			BAD=$(json_field rejected)
			REJECTED=$((REJECTED + ${BAD:-0}))
			;;
		200::UPDATE_HIVEID,*)
			# The cloud knows this hive by another id: use it and resend the batch.
			NEW_ID=${ACTION#UPDATE_HIVEID,}
			[ "$NEW_ID" = "$HIVEID" ] && fail "the cloud keeps asking for hive id $NEW_ID"
			echo "Cloud hive id is $NEW_ID (was $HIVEID), resending"
			HIVEID=$NEW_ID
			continue
			;;
		200:NOT_REGISTERED:)
			fail "hive $HIVENAME isn't registered at HiveControl yet; let a regular upload register it, then run this again"
			;;
		401:*)
			fail "the API key was refused; check Settings > Basic Config"
			;;
		000:*)
			fail "could not reach $SYNCAPI_URL"
			;;
		*)
			fail "HTTP $HTTP_CODE: $BODY"
			;;
	esac

	SENT=$((SENT + COUNT))
	LAST_ROWID=$BATCH_LAST
	echo "$LAST_ROWID" > "$STATEFILE"
	echo "Sent $SENT of $TOTAL"
	sleep "$PAUSE"
done

echo "$DATE synced $SENT readings, $REJECTED rejected" >> "$RUNDIR/sync.log"
loglocal "$DATE" HIVECONTROL INFO "History sync sent $SENT readings ($REJECTED rejected by the cloud)"
echo "DONE: sent $SENT readings, $REJECTED rejected"
