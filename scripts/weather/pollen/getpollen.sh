#!/bin/bash
#
# Fetch daily pollen data and store in database
#
# Provider chain:
#   1. Pollen.com (US, ZIP-based, free, no key) — best for North America
#   2. Tomorrow.io (global, coordinate-based, requires KEY_TOMORROW)
#   3. Open-Meteo Air Quality (global/Europe, coordinate-based, free, no key)
#   4. Ambee Pollen API (global, coordinate-based, requires KEY_AMBEE)
#
# On first run or fresh install, backfills the current year from
# pollen.com (3-day window) and Open-Meteo historical air quality.
#
# Author: Ryan Crum
# Version: 2.0

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F')
LOGDATE=$(TZ=":$TIMEZONE" date '+%F %T')
DATASOURCE="$HOMEDIR/data/hive-data.db"
TEST_MODE=0
if [ "${1:-}" = "--test" ]; then
	TEST_MODE=1
fi

# --- Check if pollen collection is enabled ---
if [ "$ENABLE_POLLEN" = "no" ] && [ $TEST_MODE -eq 0 ]; then
	exit 0
fi

# --- Check if today already has data ---
if [ $TEST_MODE -eq 0 ]; then
	existing=$(sqlite3 "$DATASOURCE" "SELECT COUNT(*) FROM pollen WHERE date = '$DATE';")
	if [ "$existing" -gt 0 ]; then
		loglocal "$LOGDATE" POLLEN INFO "Today ($DATE) already has pollen data, skipping"
		exit 0
	fi
fi

# --- Severity/index helpers for Open-Meteo grains/m³ ---
map_grass_severity() {
	local val=$1
	if [ -z "$val" ] || [ "$val" = "null" ] || [ "$val" = "0" ]; then echo 0; return; fi
	if [ 1 -eq "$(echo "$val < 5" | bc)" ]; then echo 1
	elif [ 1 -eq "$(echo "$val < 25" | bc)" ]; then echo 2
	elif [ 1 -eq "$(echo "$val < 100" | bc)" ]; then echo 3
	else echo 3; fi
}

map_ragweed_severity() {
	local val=$1
	if [ -z "$val" ] || [ "$val" = "null" ] || [ "$val" = "0" ]; then echo 0; return; fi
	if [ 1 -eq "$(echo "$val < 10" | bc)" ]; then echo 1
	elif [ 1 -eq "$(echo "$val < 50" | bc)" ]; then echo 2
	elif [ 1 -eq "$(echo "$val < 200" | bc)" ]; then echo 3
	else echo 3; fi
}

map_tree_severity() {
	local val=$1
	if [ -z "$val" ] || [ "$val" = "null" ] || [ "$val" = "0" ]; then echo 0; return; fi
	if [ 1 -eq "$(echo "$val < 15" | bc)" ]; then echo 1
	elif [ 1 -eq "$(echo "$val < 90" | bc)" ]; then echo 2
	elif [ 1 -eq "$(echo "$val < 400" | bc)" ]; then echo 3
	else echo 3; fi
}

severity_to_index() {
	local severity=$1
	case $severity in
		0) echo 0 ;; 1) echo 4 ;; 2) echo 8 ;; 3) echo 12 ;; *) echo 0 ;;
	esac
}

max_of() {
	local max=0
	for s in "$@"; do
		[ "$s" -gt "$max" ] && max=$s
	done
	echo $max
}

risk_to_severity() {
	case "$1" in
		Low) echo 1 ;; Moderate) echo 2 ;; High|"Very High") echo 3 ;; *) echo 0 ;;
	esac
}

# Tomorrow.io 0-5 scale to 0-3 severity
tomorrow_to_severity() {
	local idx=$1
	if [ "$idx" -le 0 ]; then echo 0
	elif [ "$idx" -le 1 ]; then echo 1
	elif [ "$idx" -le 3 ]; then echo 2
	else echo 3; fi
}

compute_pollen_from_species() {
	local alder=$1 birch=$2 grass=$3 mugwort=$4 olive=$5 ragweed=$6

	{ [ "$alder" = "null" ] || [ -z "$alder" ]; } && alder=0
	{ [ "$birch" = "null" ] || [ -z "$birch" ]; } && birch=0
	{ [ "$grass" = "null" ] || [ -z "$grass" ]; } && grass=0
	{ [ "$mugwort" = "null" ] || [ -z "$mugwort" ]; } && mugwort=0
	{ [ "$olive" = "null" ] || [ -z "$olive" ]; } && olive=0
	{ [ "$ragweed" = "null" ] || [ -z "$ragweed" ]; } && ragweed=0

	local sev_a=$(map_tree_severity "$alder")
	local sev_b=$(map_tree_severity "$birch")
	local sev_g=$(map_grass_severity "$grass")
	local sev_m=$(map_grass_severity "$mugwort")
	local sev_o=$(map_tree_severity "$olive")
	local sev_r=$(map_ragweed_severity "$ragweed")

	local level=$(severity_to_index "$(max_of $sev_a $sev_b $sev_g $sev_m $sev_o $sev_r)")

	local types=""
	[ "$(echo "$alder > 0" | bc)" -eq 1 ] && types="${types}Alder "
	[ "$(echo "$birch > 0" | bc)" -eq 1 ] && types="${types}Birch "
	[ "$(echo "$grass > 0" | bc)" -eq 1 ] && types="${types}Grass "
	[ "$(echo "$mugwort > 0" | bc)" -eq 1 ] && types="${types}Mugwort "
	[ "$(echo "$olive > 0" | bc)" -eq 1 ] && types="${types}Olive "
	[ "$(echo "$ragweed > 0" | bc)" -eq 1 ] && types="${types}Ragweed "
	types=$(echo "$types" | sed 's/ $//')
	[ -z "$types" ] && types="None"

	echo "$level|$types"
}

# --- Log to weather_health table ---
log_pollen_health() {
	local provider="$1" success="$2" response_ms="$3" error_reason="$4"
	local p_safe="${provider//\'/\'\'}"
	local e_safe="${error_reason//\'/\'\'}"
	local err_val="NULL"
	[ -n "$error_reason" ] && err_val="'${e_safe}'"
	sqlite3 "$DATASOURCE" "INSERT INTO weather_health (timestamp, provider, role, success, response_ms, error_reason, observation_age_minutes) VALUES ('$LOGDATE', '${p_safe}', 'pollen', $success, ${response_ms:-0}, ${err_val}, 0);" 2>/dev/null
}

# --- Provider 1: Pollen.com (US, ZIP-based) ---
fetch_pollencom() {
	if [ -z "$ZIP" ]; then
		return 1
	fi

	local API_URL="https://www.pollen.com/api/forecast/current/pollen/${ZIP}"
	local TRYCOUNTER=1
	local API_RESULT=""
	local start_ms=$(date +%s%N 2>/dev/null || echo 0)

	while [ $TRYCOUNTER -le 3 ]; do
		API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL" \
			-H 'Referer: https://www.pollen.com/forecast/current/pollen/' \
			-H 'User-Agent: Mozilla/5.0 (X11; Linux aarch64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36' \
			2>/dev/null)

		local check=$(echo "$API_RESULT" | jq -r '.Location.periods // empty' 2>/dev/null)
		if [ -n "$check" ]; then
			break
		fi
		loglocal "$LOGDATE" POLLEN INFO "Pollen.com: no valid response, attempt $TRYCOUNTER"
		TRYCOUNTER=$((TRYCOUNTER + 1))
		sleep 5
	done

	local end_ms=$(date +%s%N 2>/dev/null || echo 0)
	local elapsed_ms=$(( (end_ms - start_ms) / 1000000 ))

	if [ -z "$(echo "$API_RESULT" | jq -r '.Location.periods // empty' 2>/dev/null)" ]; then
		log_pollen_health "pollen.com" 0 "$elapsed_ms" "No valid response after retries"
		return 1
	fi

	# periods[1] = Today (periods[0] = Yesterday, periods[2] = Tomorrow)
	local index=$(echo "$API_RESULT" | jq -r '.Location.periods[1].Index // empty' 2>/dev/null)
	if [ -z "$index" ] || [ "$index" = "null" ]; then
		log_pollen_health "pollen.com" 0 "$elapsed_ms" "No today index in response"
		return 1
	fi

	# Round to nearest integer for storage
	POL_LEVEL=$(echo "$index" | awk '{printf "%.0f", $1}')

	# Extract trigger names
	local triggers=$(echo "$API_RESULT" | jq -r '.Location.periods[1].Triggers[]?.Name // empty' 2>/dev/null)
	POL_TYPES=""
	while IFS= read -r name; do
		[ -n "$name" ] && POL_TYPES="${POL_TYPES}${name} "
	done <<< "$triggers"
	POL_TYPES=$(echo "$POL_TYPES" | sed 's/ $//')
	[ -z "$POL_TYPES" ] && POL_TYPES="None"

	log_pollen_health "pollen.com" 1 "$elapsed_ms" ""
	loglocal "$LOGDATE" POLLEN INFO "Pollen.com: Level=$POL_LEVEL Types=$POL_TYPES"
	POLLEN_SOURCE="pollen.com"
	return 0
}

# --- Provider 2: Open-Meteo Air Quality (Europe/global) ---
fetch_openmeteo_pollen() {
	if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
		return 1
	fi

	local API_URL="https://air-quality-api.open-meteo.com/v1/air-quality?latitude=${LATITUDE}&longitude=${LONGITUDE}&current=alder_pollen,birch_pollen,grass_pollen,mugwort_pollen,olive_pollen,ragweed_pollen&timezone=auto"

	local TRYCOUNTER=1
	local API_RESULT=""
	local start_ms=$(date +%s%N 2>/dev/null || echo 0)

	while [ $TRYCOUNTER -le 3 ]; do
		API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL" 2>/dev/null)
		local check=$(echo "$API_RESULT" | jq -r '.current // empty' 2>/dev/null)
		if [ -n "$check" ]; then
			break
		fi
		loglocal "$LOGDATE" POLLEN INFO "Open-Meteo Air Quality: no valid response, attempt $TRYCOUNTER"
		TRYCOUNTER=$((TRYCOUNTER + 1))
		sleep 5
	done

	local end_ms=$(date +%s%N 2>/dev/null || echo 0)
	local elapsed_ms=$(( (end_ms - start_ms) / 1000000 ))

	if [ -z "$(echo "$API_RESULT" | jq -r '.current // empty' 2>/dev/null)" ]; then
		log_pollen_health "openmeteo_pollen" 0 "$elapsed_ms" "No valid response after retries"
		return 1
	fi

	local alder=$(echo "$API_RESULT" | jq -r '.current.alder_pollen // 0')
	local birch=$(echo "$API_RESULT" | jq -r '.current.birch_pollen // 0')
	local grass=$(echo "$API_RESULT" | jq -r '.current.grass_pollen // 0')
	local mugwort=$(echo "$API_RESULT" | jq -r '.current.mugwort_pollen // 0')
	local olive=$(echo "$API_RESULT" | jq -r '.current.olive_pollen // 0')
	local ragweed=$(echo "$API_RESULT" | jq -r '.current.ragweed_pollen // 0')

	# If all species are null/0, Open-Meteo doesn't have pollen data for this region
	{ [ "$alder" = "null" ] || [ -z "$alder" ]; } && alder=0
	{ [ "$birch" = "null" ] || [ -z "$birch" ]; } && birch=0
	{ [ "$grass" = "null" ] || [ -z "$grass" ]; } && grass=0
	{ [ "$mugwort" = "null" ] || [ -z "$mugwort" ]; } && mugwort=0
	{ [ "$olive" = "null" ] || [ -z "$olive" ]; } && olive=0
	{ [ "$ragweed" = "null" ] || [ -z "$ragweed" ]; } && ragweed=0

	local total=$(echo "$alder + $birch + $grass + $mugwort + $olive + $ragweed" | bc)
	if [ "$total" = "0" ]; then
		log_pollen_health "openmeteo_pollen" 0 "$elapsed_ms" "No pollen data for this region"
		return 1
	fi

	local result=$(compute_pollen_from_species "$alder" "$birch" "$grass" "$mugwort" "$olive" "$ragweed")
	POL_LEVEL="${result%%|*}"
	POL_TYPES="${result#*|}"

	log_pollen_health "openmeteo_pollen" 1 "$elapsed_ms" ""
	loglocal "$LOGDATE" POLLEN INFO "Open-Meteo: Level=$POL_LEVEL Types=$POL_TYPES (alder=$alder birch=$birch grass=$grass mugwort=$mugwort olive=$olive ragweed=$ragweed)"
	POLLEN_SOURCE="openmeteo"
	return 0
}

# --- Provider 3: Tomorrow.io (global, coordinate-based, requires key) ---
# Free tier: 500 calls/day. Provides tree/grass/weed indices on 0-5 scale.
fetch_tomorrow_pollen() {
	local TOMORROW_KEY="${KEY_TOMORROW:-}"
	if [ -z "$TOMORROW_KEY" ]; then
		return 1
	fi

	if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
		return 1
	fi

	local API_URL="https://api.tomorrow.io/v4/timelines?location=${LATITUDE},${LONGITUDE}&fields=treeIndex,grassIndex,weedIndex&timesteps=current&apikey=${TOMORROW_KEY}"
	local start_ms=$(date +%s%N 2>/dev/null || echo 0)

	local TRYCOUNTER=1
	local API_RESULT=""

	while [ $TRYCOUNTER -le 3 ]; do
		API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL" 2>/dev/null)
		local check=$(echo "$API_RESULT" | jq -r '.data.timelines[0].intervals[0].values // empty' 2>/dev/null)
		if [ -n "$check" ]; then
			break
		fi
		loglocal "$LOGDATE" POLLEN INFO "Tomorrow.io: no valid response, attempt $TRYCOUNTER"
		TRYCOUNTER=$((TRYCOUNTER + 1))
		sleep 5
	done

	local end_ms=$(date +%s%N 2>/dev/null || echo 0)
	local elapsed_ms=$(( (end_ms - start_ms) / 1000000 ))

	local values=$(echo "$API_RESULT" | jq -r '.data.timelines[0].intervals[0].values // empty' 2>/dev/null)
	if [ -z "$values" ]; then
		log_pollen_health "tomorrow.io" 0 "$elapsed_ms" "No valid response after retries"
		return 1
	fi

	# Tomorrow.io indices: 0=None, 1=Very Low, 2=Low, 3=Medium, 4=High, 5=Very High
	local tree_idx=$(echo "$API_RESULT" | jq -r '.data.timelines[0].intervals[0].values.treeIndex // 0')
	local grass_idx=$(echo "$API_RESULT" | jq -r '.data.timelines[0].intervals[0].values.grassIndex // 0')
	local weed_idx=$(echo "$API_RESULT" | jq -r '.data.timelines[0].intervals[0].values.weedIndex // 0')

	{ [ "$tree_idx" = "null" ] || [ -z "$tree_idx" ]; } && tree_idx=0
	{ [ "$grass_idx" = "null" ] || [ -z "$grass_idx" ]; } && grass_idx=0
	{ [ "$weed_idx" = "null" ] || [ -z "$weed_idx" ]; } && weed_idx=0

	local sev_tree=$(tomorrow_to_severity "$tree_idx")
	local sev_grass=$(tomorrow_to_severity "$grass_idx")
	local sev_weed=$(tomorrow_to_severity "$weed_idx")

	POL_LEVEL=$(severity_to_index "$(max_of $sev_tree $sev_grass $sev_weed)")

	POL_TYPES=""
	[ "$sev_tree" -gt 0 ] && POL_TYPES="${POL_TYPES}Tree "
	[ "$sev_grass" -gt 0 ] && POL_TYPES="${POL_TYPES}Grass "
	[ "$sev_weed" -gt 0 ] && POL_TYPES="${POL_TYPES}Weed "
	POL_TYPES=$(echo "$POL_TYPES" | sed 's/ $//')
	[ -z "$POL_TYPES" ] && POL_TYPES="None"

	log_pollen_health "tomorrow.io" 1 "$elapsed_ms" ""
	loglocal "$LOGDATE" POLLEN INFO "Tomorrow.io: Level=$POL_LEVEL Types=$POL_TYPES (tree=$tree_idx grass=$grass_idx weed=$weed_idx)"
	POLLEN_SOURCE="tomorrow.io"
	return 0
}

# --- Provider 4: Ambee Pollen API (fallback, requires key) ---
fetch_ambee_pollen() {
	local AMBEE_KEY="${KEY_AMBEE:-}"
	if [ -z "$AMBEE_KEY" ]; then
		return 1
	fi

	if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
		return 1
	fi

	local API_URL="https://api.ambeedata.com/latest/pollen/by-lat-lng?lat=${LATITUDE}&lng=${LONGITUDE}"
	local start_ms=$(date +%s%N 2>/dev/null || echo 0)
	local API_RESULT=$(/usr/bin/curl -s --max-time 30 -H "x-api-key: ${AMBEE_KEY}" -H "Content-type: application/json" "$API_URL" 2>/dev/null)
	local end_ms=$(date +%s%N 2>/dev/null || echo 0)
	local elapsed_ms=$(( (end_ms - start_ms) / 1000000 ))

	local check=$(echo "$API_RESULT" | jq -r '.data // empty' 2>/dev/null)
	if [ -z "$check" ]; then
		log_pollen_health "ambee_pollen" 0 "$elapsed_ms" "No valid response"
		return 1
	fi

	local tree_risk=$(echo "$API_RESULT" | jq -r '.data[0].Risk.tree_pollen // "Low"')
	local grass_risk=$(echo "$API_RESULT" | jq -r '.data[0].Risk.grass_pollen // "Low"')
	local weed_risk=$(echo "$API_RESULT" | jq -r '.data[0].Risk.weed_pollen // "Low"')

	local sev_tree=$(risk_to_severity "$tree_risk")
	local sev_grass=$(risk_to_severity "$grass_risk")
	local sev_weed=$(risk_to_severity "$weed_risk")

	POL_LEVEL=$(severity_to_index "$(max_of $sev_tree $sev_grass $sev_weed)")

	POL_TYPES=""
	[ "$sev_tree" -gt 0 ] && POL_TYPES="${POL_TYPES}Tree "
	[ "$sev_grass" -gt 0 ] && POL_TYPES="${POL_TYPES}Grass "
	[ "$sev_weed" -gt 0 ] && POL_TYPES="${POL_TYPES}Weed "
	POL_TYPES=$(echo "$POL_TYPES" | sed 's/ $//')
	[ -z "$POL_TYPES" ] && POL_TYPES="None"

	log_pollen_health "ambee_pollen" 1 "$elapsed_ms" ""
	loglocal "$LOGDATE" POLLEN INFO "Ambee: Level=$POL_LEVEL Types=$POL_TYPES (tree=$tree_risk grass=$grass_risk weed=$weed_risk)"
	POLLEN_SOURCE="ambee"
	return 0
}

# --- Backfill missing days using pollen.com (limited to 3-day window) ---
# Pollen.com only gives yesterday/today/tomorrow, so backfill is minimal.
# For longer gaps, Open-Meteo historical air quality is used (Europe only).
backfill_pollen() {
	local CURRENT_YEAR=$(date '+%Y')
	local SEASON_START="${CURRENT_YEAR}-01-01"
	local YESTERDAY=$(date --date="yesterday" +%Y-%m-%d 2>/dev/null || date -v-1d +%Y-%m-%d)

	# Backfill yesterday from pollen.com if missing (most common gap)
	if [ -n "$ZIP" ]; then
		local yesterday_exists=$(sqlite3 "$DATASOURCE" "SELECT COUNT(*) FROM pollen WHERE date = '$YESTERDAY';")
		if [ "$yesterday_exists" -eq 0 ]; then
			local API_URL="https://www.pollen.com/api/forecast/current/pollen/${ZIP}"
			local API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL" \
				-H 'Referer: https://www.pollen.com/forecast/current/pollen/' \
				-H 'User-Agent: Mozilla/5.0 (X11; Linux aarch64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36' \
				2>/dev/null)

			local yesterday_index=$(echo "$API_RESULT" | jq -r '.Location.periods[0].Index // empty' 2>/dev/null)
			if [ -n "$yesterday_index" ] && [ "$yesterday_index" != "null" ]; then
				local level=$(echo "$yesterday_index" | awk '{printf "%.0f", $1}')
				local triggers=$(echo "$API_RESULT" | jq -r '.Location.periods[0].Triggers[]?.Name // empty' 2>/dev/null)
				local types=""
				while IFS= read -r name; do
					[ -n "$name" ] && types="${types}${name} "
				done <<< "$triggers"
				types=$(echo "$types" | sed 's/ $//')
				[ -z "$types" ] && types="None"
				local t_safe="${types//\'/\'\'}"

				sqlite3 "$DATASOURCE" "INSERT OR IGNORE INTO pollen (date, pollenlevel, pollentypes) VALUES ('$YESTERDAY', $level, '$t_safe');"
				loglocal "$LOGDATE" POLLEN INFO "Backfilled yesterday ($YESTERDAY) from pollen.com: Level=$level Types=$types"
			fi
		fi
	fi

	# For longer gaps, try Open-Meteo historical air quality (European coverage only)
	if [ -n "$LATITUDE" ] && [ -n "$LONGITUDE" ]; then
		local last_date=$(sqlite3 "$DATASOURCE" "SELECT MAX(date) FROM pollen WHERE date >= '$SEASON_START';")
		local start_date="$SEASON_START"
		if [ -n "$last_date" ]; then
			start_date=$(date --date="$last_date + 1 day" +%Y-%m-%d 2>/dev/null || date -j -v+1d -f "%Y-%m-%d" "$last_date" +%Y-%m-%d)
		fi

		# Only attempt if there's a gap of more than 2 days (yesterday already handled above)
		local two_days_ago=$(date --date="2 days ago" +%Y-%m-%d 2>/dev/null || date -v-2d +%Y-%m-%d)
		if [[ "$start_date" < "$two_days_ago" ]]; then
			loglocal "$LOGDATE" POLLEN INFO "Attempting Open-Meteo backfill from $start_date to $two_days_ago"

			local API_URL="https://air-quality-api.open-meteo.com/v1/air-quality?latitude=${LATITUDE}&longitude=${LONGITUDE}&hourly=alder_pollen,birch_pollen,grass_pollen,mugwort_pollen,olive_pollen,ragweed_pollen&start_date=${start_date}&end_date=${two_days_ago}&timezone=auto"
			local API_RESULT=$(/usr/bin/curl -s --max-time 120 "$API_URL" 2>/dev/null)

			local hours_count=$(echo "$API_RESULT" | jq -r '.hourly.time | length' 2>/dev/null)
			if [ -n "$hours_count" ] && [ "$hours_count" != "null" ] && [ "$hours_count" -gt 0 ]; then
				local daily_data=$(echo "$API_RESULT" | jq -r '
					[.hourly.time, .hourly.alder_pollen, .hourly.birch_pollen, .hourly.grass_pollen, .hourly.mugwort_pollen, .hourly.olive_pollen, .hourly.ragweed_pollen]
					| transpose
					| map({
						date: (.[0] | split("T")[0]),
						alder: (.[1] // 0),
						birch: (.[2] // 0),
						grass: (.[3] // 0),
						mugwort: (.[4] // 0),
						olive: (.[5] // 0),
						ragweed: (.[6] // 0)
					})
					| group_by(.date)
					| map({
						date: .[0].date,
						alder: (map(.alder) | max),
						birch: (map(.birch) | max),
						grass: (map(.grass) | max),
						mugwort: (map(.mugwort) | max),
						olive: (map(.olive) | max),
						ragweed: (map(.ragweed) | max)
					})
					| .[]
					| [.date, .alder, .birch, .grass, .mugwort, .olive, .ragweed]
					| @tsv
				' 2>/dev/null)

				if [ -n "$daily_data" ]; then
					local existing_dates=$(sqlite3 "$DATASOURCE" "SELECT date FROM pollen WHERE date >= '$SEASON_START';")
					local sql="BEGIN TRANSACTION;"
					local filled=0

					while IFS=$'\t' read -r day_date alder birch grass mugwort olive ragweed; do
						[ -z "$day_date" ] && continue
						[[ ! "$day_date" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]] && continue

						if echo "$existing_dates" | grep -q "^${day_date}$"; then
							continue
						fi

						# Skip if all species are 0/null (no coverage for this region)
						{ [ "$alder" = "null" ] || [ -z "$alder" ]; } && alder=0
						{ [ "$birch" = "null" ] || [ -z "$birch" ]; } && birch=0
						{ [ "$grass" = "null" ] || [ -z "$grass" ]; } && grass=0
						{ [ "$mugwort" = "null" ] || [ -z "$mugwort" ]; } && mugwort=0
						{ [ "$olive" = "null" ] || [ -z "$olive" ]; } && olive=0
						{ [ "$ragweed" = "null" ] || [ -z "$ragweed" ]; } && ragweed=0
						local total=$(echo "$alder + $birch + $grass + $mugwort + $olive + $ragweed" | bc 2>/dev/null)
						if [ "$total" = "0" ] || [ -z "$total" ]; then
							continue
						fi

						local result=$(compute_pollen_from_species "$alder" "$birch" "$grass" "$mugwort" "$olive" "$ragweed")
						local level="${result%%|*}"
						local types="${result#*|}"
						[[ ! "$level" =~ ^[0-9]+$ ]] && continue
						local t_safe="${types//\'/\'\'}"

						sql="${sql} INSERT OR IGNORE INTO pollen (date, pollenlevel, pollentypes) VALUES ('$day_date', $level, '$t_safe');"
						filled=$((filled + 1))
					done <<< "$daily_data"

					sql="${sql} COMMIT;"

					if [ "$filled" -gt 0 ]; then
						sqlite3 "$DATASOURCE" "$sql"
						loglocal "$LOGDATE" POLLEN INFO "Open-Meteo backfilled $filled days of pollen data"
					fi
				fi
			fi
		fi
	fi
}

# --- Main execution ---

# Step 1: Backfill any missing days
backfill_pollen

# Step 2: Check again if today got filled
existing=$(sqlite3 "$DATASOURCE" "SELECT COUNT(*) FROM pollen WHERE date = '$DATE';")
if [ "$existing" -gt 0 ]; then
	exit 0
fi

# Step 3: Fetch today's pollen data
POL_LEVEL=""
POL_TYPES=""
POLLEN_SOURCE=""

fetch_pollencom

if [ -z "$POL_LEVEL" ]; then
	fetch_tomorrow_pollen
fi

if [ -z "$POL_LEVEL" ]; then
	fetch_openmeteo_pollen
fi

if [ -z "$POL_LEVEL" ]; then
	fetch_ambee_pollen
fi

if [ -z "$POL_LEVEL" ] || [[ ! "$POL_LEVEL" =~ ^[0-9]+$ ]]; then
	echo "ERROR: Could not get valid pollen data from any source (level='$POL_LEVEL')"
	loglocal "$LOGDATE" POLLEN ERROR "Could not get valid pollen data from any source (level='$POL_LEVEL')"
	exit 1
fi

# Step 4: Insert into database
POL_TYPES_SAFE="${POL_TYPES//\'/\'\'}"
sqlite3 "$DATASOURCE" "INSERT OR IGNORE INTO pollen (date, pollenlevel, pollentypes) VALUES ('$DATE', $POL_LEVEL, '$POL_TYPES_SAFE');"

echo "Date: $DATE"
echo "Source: ${POLLEN_SOURCE:-unknown}"
echo "Level: $POL_LEVEL"
echo "Types: ${POL_TYPES:-N/A}"

loglocal "$LOGDATE" POLLEN INFO "Stored: Date=$DATE Level=$POL_LEVEL Types=$POL_TYPES Source=$POLLEN_SOURCE"
