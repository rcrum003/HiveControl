#!/bin/bash
# Weather data collection dispatcher with cascading fallback and health tracking
# Calls configured weather providers in order: primary → fallback 1 → fallback 2
# Validates observation freshness and logs provider health metrics

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

PATH=/usr/local/sbin:/usr/local/bin:/bin:/sbin:/usr/sbin:/usr/bin:/home/HiveControl/scripts/weather:/home/HiveControl/scripts/system

DATE=$(TZ=":$TIMEZONE" date '+%F %T')
WEATHER_STATIONID="$WXSTATION"
JSON_PATH="/home/HiveControl/scripts/weather/JSON.sh"
DATASOURCE="$HOMEDIR/data/hive-data.db"
MAX_STALE="${WX_MAX_STALE_MINUTES:-120}"

# Initialize all weather variables to null
init_wx_vars() {
	A_TEMP="null"; A_TEMP_C="null"; B_HUMIDITY="null"
	pressure_mb="null"; A_PRES_IN="null"; UV="null"
	solarradiation="null"; precip_1hr_in="null"; precip_1hr_metric="null"
	wind_degrees="null"; wind_gust_kph="null"; wind_gust_mph="null"
	wind_kph="null"; wind_mph="null"; A_WIND_MPH="null"
	A_TIME=""; A_HUMIDITY=""; A_WIND_DIR=""
	A_PRES_TREND=""; A_DEW=""; weather_dewc=""
	OBSERVATIONDATETIME=""; precip_today_string=""
	precip_today_in=""; precip_today_metric=""; lux="0"
}

# Call the appropriate weather provider script
call_wx_provider() {
	local level="$1"
	case "$level" in
		hive)             GETNOW=$($HOMEDIR/scripts/weather/wxunderground/getwxxml.sh) ;;
		localws)
			case $local_wx_type in
				"WS1400ip") GETNOW=$($HOMEDIR/scripts/weather/ws1400/getWS1400.sh) ;;
				"ourweather") GETNOW=$($HOMEDIR/scripts/weather/ourweather/getOurweather.sh) ;;
			esac
			;;
		localsensors)     GETNOW=$($HOMEDIR/scripts/weather/localsensors/localsensors.sh); WEATHER_STATIONID="LOCALSENSOR" ;;
		ambientwx)        GETNOW=$($HOMEDIR/scripts/weather/ambient/getAmbientAPI.sh) ;;
		wf_tempest_local) GETNOW=$($HOMEDIR/scripts/weather/weatherflow/getWeatherFlowLocal.sh) ;;
		openmeteo)        GETNOW=$($HOMEDIR/scripts/weather/openmeteo/getopenmeteo.sh) ;;
		openweathermap)   GETNOW=$($HOMEDIR/scripts/weather/openweathermap/getopenweathermap.sh) ;;
		nws)              GETNOW=$($HOMEDIR/scripts/weather/nws/getnws.sh) ;;
		weatherapi)       GETNOW=$($HOMEDIR/scripts/weather/weatherapi/getweatherapi.sh) ;;
		visualcrossing)   GETNOW=$($HOMEDIR/scripts/weather/visualcrossing/getvisualcrossing.sh) ;;
		pirateweather)    GETNOW=$($HOMEDIR/scripts/weather/pirateweather/getpirateweather.sh) ;;
		*)                GETNOW="" ;;
	esac
}

# Parse JSON output from any provider into standard variables
parse_wx_json() {
	if [[ -z "$GETNOW" ]]; then
		return 1
	fi

	A_TEMP=$(/bin/echo $GETNOW | $JSON_PATH -b | grep temp_f | awk -F'"' '{print $6}')
	A_TEMP_C=$(/bin/echo $GETNOW | $JSON_PATH -b | grep temp_c | awk -F'"' '{print $6}')
	A_WIND_MPH=$(/bin/echo $GETNOW | $JSON_PATH -b | grep wind_mph | awk -F'"' '{print $6}')
	OBSERVATIONDATETIME=$(/bin/echo $GETNOW | $JSON_PATH -b | grep observation_time | awk -F'"' '{print $6}')
	wind_degrees=$(/bin/echo $GETNOW | $JSON_PATH -b | grep wind_degrees | awk -F'"' '{print $6}')
	wind_gust_mph=$(/bin/echo $GETNOW | $JSON_PATH -b | grep wind_gust_mph | awk -F'"' '{print $6}')
	wind_kph=$(/bin/echo $GETNOW | $JSON_PATH -b | grep wind_kph | awk -F'"' '{print $6}')
	wind_gust_kph=$(/bin/echo $GETNOW | $JSON_PATH -b | grep wind_gust_kph | awk -F'"' '{print $6}')
	weather_dewc=$(/bin/echo $GETNOW | $JSON_PATH -b | grep dewpoint_c | awk -F'"' '{print $6}')

	# Fallback for unquoted temp values (legacy providers)
	if [[ -z "$A_TEMP" ]]; then
		A_TEMP=$(/bin/echo $GETNOW | $JSON_PATH -b | grep temp_f | awk '{print $2}')
	fi
	if [[ -z "$A_TEMP_C" ]]; then
		A_TEMP_C=$(/bin/echo $GETNOW | $JSON_PATH -b | grep temp_c | awk '{print $2}')
	fi

	# Common fields parsed once for all sources
	A_TIME=$(/bin/echo $GETNOW | $JSON_PATH -b | grep observation_epoch | awk -F'"' '{print $6}')
	A_HUMIDITY=$(/bin/echo $GETNOW | $JSON_PATH -b | grep relative_humidity | awk -F'"' '{print $6}')
	B_HUMIDITY=$(echo "$A_HUMIDITY" | grep -o "\-*[0-9]*\.*[0-9]*")
	A_WIND_DIR=$(/bin/echo $GETNOW | $JSON_PATH -b | grep wind_dir | awk -F'"' '{print $6}')
	A_PRES_IN=$(/bin/echo $GETNOW | $JSON_PATH -b | grep pressure_in | awk -F'"' '{print $6}')
	A_PRES_TREND=$(/bin/echo $GETNOW | $JSON_PATH -b | grep pressure_trend | awk -F'"' '{print $6}')
	A_DEW=$(/bin/echo $GETNOW | $JSON_PATH -b | grep dewpoint_f | awk '{print $2}')
	pressure_mb=$(/bin/echo $GETNOW | $JSON_PATH -b | grep pressure_mb | awk -F'"' '{print $6}')
	UV=$(/bin/echo $GETNOW | $JSON_PATH -b | grep UV | awk -F'"' '{print $6}')
	precip_1hr_in=$(/bin/echo $GETNOW | $JSON_PATH -b | grep precip_1hr_in | awk -F'"' '{print $6}')
	precip_1hr_metric=$(/bin/echo $GETNOW | $JSON_PATH -b | grep precip_1hr_metric | awk -F'"' '{print $6}')
	precip_today_string=$(/bin/echo $GETNOW | $JSON_PATH -b | grep precip_today_string | awk -F'"' '{print $6}')
	precip_today_in=$(/bin/echo $GETNOW | $JSON_PATH -b | grep precip_today_in | awk -F'"' '{print $6}')
	precip_today_metric=$(/bin/echo $GETNOW | $JSON_PATH -b | grep precip_today_metric | awk -F'"' '{print $6}')
	solarradiation=$(/bin/echo $GETNOW | $JSON_PATH -b | grep solarradiation | awk -F'"' '{print $6}')
	lux="0"

	return 0
}

# Log provider health to database (non-blocking)
log_health() {
	local provider="$1"
	local role="$2"
	local success="$3"
	local response_ms="$4"
	local error_reason="$5"
	local obs_age="$6"

	local p_safe="${provider//\'/\'\'}"
	local r_safe="${role//\'/\'\'}"
	local e_safe="${error_reason//\'/\'\'}"
	local err_val="NULL"
	[ -n "$error_reason" ] && err_val="'${e_safe}'"

	sqlite3 "$DATASOURCE" "INSERT INTO weather_health (timestamp, provider, role, success, response_ms, error_reason, observation_age_minutes) VALUES ('$DATE', '${p_safe}', '${r_safe}', $success, ${response_ms:-0}, ${err_val}, ${obs_age:-0});" 2>/dev/null

	# Prune old health records (>30 days)
	sqlite3 "$DATASOURCE" "DELETE FROM weather_health WHERE timestamp < datetime('now', '-30 days');" 2>/dev/null
}

# Build provider chain: primary, fallback 1, fallback 2 (skip empties and duplicates)
build_provider_chain() {
	local seen=""
	for provider in "$WEATHER_LEVEL" "$WEATHER_FALLBACK" "$WEATHER_FALLBACK_2"; do
		if [ -n "$provider" ]; then
			case "$seen" in
				*"|${provider}|"*) continue ;;
			esac
			seen="${seen}|${provider}|"
			printf '%s\n' "$provider"
		fi
	done
}

# Main execution
init_wx_vars

mapfile -t PROVIDER_CHAIN < <(build_provider_chain)
WX_SUCCESS=0
ROLE_NAMES=("primary" "fallback_1" "fallback_2")

for i in "${!PROVIDER_CHAIN[@]}"; do
	provider="${PROVIDER_CHAIN[$i]}"
	role="${ROLE_NAMES[$i]}"

	GETNOW=""
	START_TIME=$(date +%s%3N 2>/dev/null || echo $((SECONDS * 1000)))

	call_wx_provider "$provider"
	END_TIME=$(date +%s%3N 2>/dev/null || echo $((SECONDS * 1000)))
	ELAPSED_MS=$(( END_TIME - START_TIME ))

	if ! parse_wx_json; then
		loglocal "$DATE" WXFallback WARNING "Weather source '$provider' ($role) failed to return data"
		log_health "$provider" "$role" 0 "$ELAPSED_MS" "no_data" 0
		continue
	fi

	# Check observation freshness
	OBS_AGE=$(validate_observation_time "$OBSERVATIONDATETIME" "$MAX_STALE")
	STALE_RC=$?

	if [ $STALE_RC -ne 0 ] && [ -n "$OBSERVATIONDATETIME" ]; then
		loglocal "$DATE" WXFallback WARNING "Weather source '$provider' ($role) returned stale data (${OBS_AGE:-unknown} min old, max $MAX_STALE)"
		log_health "$provider" "$role" 0 "$ELAPSED_MS" "stale_${OBS_AGE}min" "${OBS_AGE:-0}"
		init_wx_vars
		continue
	fi

	# Success
	WX_SUCCESS=1
	log_health "$provider" "$role" 1 "$ELAPSED_MS" "" "${OBS_AGE:-0}"

	if [ "$role" != "primary" ]; then
		loglocal "$DATE" WXFallback INFO "Weather $role source '$provider' succeeded"
	fi
	break
done

if [ $WX_SUCCESS -eq 0 ]; then
	loglocal "$DATE" WXAmbient ERROR "All weather sources failed (tried: ${PROVIDER_CHAIN[*]})"
fi

# Validate all values
check_allow_neg A_TEMP
check B_HUMIDITY
check_allow_neg A_DEW
check_allow_neg A_TEMP_C
check A_WIND_MPH
check A_WIND_DIR
check wind_degrees
check wind_gust_mph
check wind_kph
check wind_gust_kph
check pressure_mb
check A_PRES_IN
check A_PRES_TREND
check_allow_neg weather_dewc
check solarradiation
check UV
check precip_1hr_in
check precip_1hr_metric
check precip_today_string
check precip_today_in
check precip_today_metric

echo "$WEATHER_STATIONID,$OBSERVATIONDATETIME,$A_TEMP,$B_HUMIDITY,$A_DEW,$A_TEMP_C,$A_WIND_MPH,$A_WIND_DIR,$wind_degrees,$wind_gust_mph,$wind_kph,$wind_gust_kph,$pressure_mb,$A_PRES_IN,$A_PRES_TREND,$weather_dewc,$solarradiation,$UV,$precip_1hr_in,$precip_1hr_metric,$precip_today_string,$precip_today_in,$precip_today_metric"
