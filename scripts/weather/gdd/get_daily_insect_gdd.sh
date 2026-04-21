#!/bin/bash
#
# Calculate daily insect GDD (Growing Degree Days) and accumulate seasonal totals
#
# Note: This is insect GDD (air temperature), not plant GDD (soil temperature)
# Author: Ryan Crum
# Version: 3.0
#
# Standard insect GDD parameters:
#   Base temp: 50°F (universally accepted for general insect development)
#   Season start: January 1 of the current year
#   Max cap: 86°F (above this, insect development plateaus)
#
# On first run or fresh install, backfills the entire year from Open-Meteo
# historical API (free, no key required) in a single request.
# Daily cron then adds one day at a time going forward.
#
# Database Note:
# The DB only accepts one GDD row per date (INSERT OR IGNORE).
# This protects against duplicate entries from re-runs.

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')
DATASOURCE="$HOMEDIR/data/hive-data.db"

GDD_BASE_TEMP=50
CURRENT_YEAR=$(date '+%Y')
SEASON_START="${CURRENT_YEAR}-01-01"
YESTERDAY=$(date --date="yesterday" +%Y-%m-%d)

# --- GDD calculation for a single day ---
calc_gdd() {
	local max_temp="$1"
	local min_temp="$2"

	# Cap max at 86°F
	if [[ $(echo "$max_temp > 86" | bc) -eq 1 ]]; then
		max_temp="86"
	fi

	local gdd=$(echo "scale=1; (($max_temp + $min_temp) / 2) - $GDD_BASE_TEMP" | bc)

	if [[ $(echo "$gdd < 0" | bc) -eq 1 ]]; then
		gdd=0
	fi

	echo "$gdd"
}

# --- Backfill missing days from Open-Meteo historical API ---
backfill_from_openmeteo() {
	if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
		loglocal "$DATE" GDD ERROR "Cannot backfill: LATITUDE/LONGITUDE not configured"
		return 1
	fi

	# Find the first missing date this year
	local last_date=$(sqlite3 "$DATASOURCE" "SELECT MAX(gdddate) FROM gdd WHERE gdddate >= '$SEASON_START';")

	local start_date="$SEASON_START"
	if [ -n "$last_date" ]; then
		start_date=$(date --date="$last_date + 1 day" +%Y-%m-%d)
	fi

	# Nothing to backfill if we're caught up
	if [[ "$start_date" > "$YESTERDAY" ]]; then
		return 0
	fi

	loglocal "$DATE" GDD INFO "Backfilling GDD from $start_date to $YESTERDAY via Open-Meteo"

	local API_URL="https://archive-api.open-meteo.com/v1/archive?latitude=${LATITUDE}&longitude=${LONGITUDE}&daily=temperature_2m_max,temperature_2m_min&temperature_unit=fahrenheit&start_date=${start_date}&end_date=${YESTERDAY}&timezone=auto"
	local API_RESULT=$(/usr/bin/curl -s --max-time 60 "$API_URL" 2>/dev/null)

	if [ -z "$API_RESULT" ]; then
		loglocal "$DATE" GDD ERROR "Backfill: Open-Meteo returned empty response"
		return 1
	fi

	local dates_json=$(echo "$API_RESULT" | jq -r '.daily.time // empty' 2>/dev/null)
	if [ -z "$dates_json" ]; then
		loglocal "$DATE" GDD ERROR "Backfill: Could not parse Open-Meteo response"
		return 1
	fi

	local count=$(echo "$API_RESULT" | jq -r '.daily.time | length' 2>/dev/null)
	local filled=0

	# Get existing dates to skip, and current season total
	local existing_dates=$(sqlite3 "$DATASOURCE" "SELECT gdddate FROM gdd WHERE gdddate >= '$SEASON_START';")
	local running_total=$(sqlite3 "$DATASOURCE" "SELECT COALESCE(SUM(daygdd), 0) FROM gdd WHERE gdddate >= '$SEASON_START';" 2>/dev/null)
	running_total="${running_total:-0}"

	# Build all inserts as a single transaction
	local sql="BEGIN TRANSACTION;"

	for (( i=0; i<count; i++ )); do
		local day_date=$(echo "$API_RESULT" | jq -r ".daily.time[$i]" 2>/dev/null)
		local day_max=$(echo "$API_RESULT" | jq -r ".daily.temperature_2m_max[$i] // empty" 2>/dev/null)
		local day_min=$(echo "$API_RESULT" | jq -r ".daily.temperature_2m_min[$i] // empty" 2>/dev/null)

		if [ -z "$day_max" ] || [ -z "$day_min" ]; then
			continue
		fi

		# Skip dates that already have data
		if echo "$existing_dates" | grep -q "^${day_date}$"; then
			# Still accumulate its GDD into the running total for correct season sums
			local existing_gdd=$(sqlite3 "$DATASOURCE" "SELECT daygdd FROM gdd WHERE gdddate = '$day_date';")
			running_total=$(echo "$running_total + ${existing_gdd:-0}" | bc)
			continue
		fi

		local day_gdd=$(calc_gdd "$day_max" "$day_min")
		running_total=$(echo "$running_total + $day_gdd" | bc)

		sql="${sql} INSERT INTO gdd (calcdate, gdddate, daygdd, seasongdd) VALUES ('${day_date} 23:59:59', '$day_date', $day_gdd, $running_total);"
		filled=$((filled + 1))
	done

	sql="${sql} COMMIT;"

	if [ "$filled" -gt 0 ]; then
		sqlite3 "$DATASOURCE" "$sql"
	fi

	loglocal "$DATE" GDD INFO "Backfilled $filled days of GDD data ($start_date to $YESTERDAY)"
	return 0
}

# --- Fetch yesterday's daily high/low from configured weather API ---
fetch_daily_from_api() {
	local MAXTEMP=""
	local MINTEMP=""

	case "$WEATHER_LEVEL" in
		openmeteo)
			if [ -n "$LATITUDE" ] && [ -n "$LONGITUDE" ]; then
				local API_URL="https://api.open-meteo.com/v1/forecast?latitude=${LATITUDE}&longitude=${LONGITUDE}&daily=temperature_2m_max,temperature_2m_min&temperature_unit=fahrenheit&past_days=1&forecast_days=0&timezone=auto"
				local API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL" 2>/dev/null)
				MAXTEMP=$(echo "$API_RESULT" | jq -r '.daily.temperature_2m_max[0] // empty' 2>/dev/null)
				MINTEMP=$(echo "$API_RESULT" | jq -r '.daily.temperature_2m_min[0] // empty' 2>/dev/null)
			fi
			;;
		weatherapi)
			local WAPI_KEY="${KEY_WEATHERAPI:-$KEY}"
			if [ -n "$WAPI_KEY" ] && [ -n "$LATITUDE" ] && [ -n "$LONGITUDE" ]; then
				local API_URL="https://api.weatherapi.com/v1/history.json?key=${WAPI_KEY}&q=${LATITUDE},${LONGITUDE}&dt=${YESTERDAY}"
				local API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL" 2>/dev/null)
				MAXTEMP=$(echo "$API_RESULT" | jq -r '.forecast.forecastday[0].day.maxtemp_f // empty' 2>/dev/null)
				MINTEMP=$(echo "$API_RESULT" | jq -r '.forecast.forecastday[0].day.mintemp_f // empty' 2>/dev/null)
			fi
			;;
		visualcrossing)
			local VC_KEY="${KEY_VISUALCROSSING:-$KEY}"
			if [ -n "$VC_KEY" ] && [ -n "$LATITUDE" ] && [ -n "$LONGITUDE" ]; then
				local API_URL="https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/${LATITUDE},${LONGITUDE}/${YESTERDAY}?unitGroup=us&include=days&key=${VC_KEY}&contentType=json"
				local API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL" 2>/dev/null)
				MAXTEMP=$(echo "$API_RESULT" | jq -r '.days[0].tempmax // empty' 2>/dev/null)
				MINTEMP=$(echo "$API_RESULT" | jq -r '.days[0].tempmin // empty' 2>/dev/null)
			fi
			;;
		pirateweather)
			local PW_KEY="${KEY_PIRATEWEATHER:-$KEY}"
			if [ -n "$PW_KEY" ] && [ -n "$LATITUDE" ] && [ -n "$LONGITUDE" ]; then
				local YESTERDAY_EPOCH=$(date --date="yesterday 12:00" +%s)
				local API_URL="https://api.pirateweather.net/forecast/${PW_KEY}/${LATITUDE},${LONGITUDE},${YESTERDAY_EPOCH}?units=us&exclude=currently,minutely,hourly,alerts"
				local API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL" 2>/dev/null)
				MAXTEMP=$(echo "$API_RESULT" | jq -r '.daily.data[0].temperatureMax // empty' 2>/dev/null)
				MINTEMP=$(echo "$API_RESULT" | jq -r '.daily.data[0].temperatureMin // empty' 2>/dev/null)
			fi
			;;
		openweathermap|nws)
			# No free historical daily endpoints — fall through to fallbacks
			;;
	esac

	if [ -n "$MAXTEMP" ] && [ -n "$MINTEMP" ]; then
		echo "$MAXTEMP $MINTEMP"
		return 0
	fi
	return 1
}

# --- Try fallback weather API ---
fetch_daily_from_fallback() {
	local orig_level="$WEATHER_LEVEL"

	if [ -n "$WEATHER_FALLBACK" ] && [ "$WEATHER_FALLBACK" != "$orig_level" ]; then
		WEATHER_LEVEL="$WEATHER_FALLBACK"
		local result=$(fetch_daily_from_api)
		WEATHER_LEVEL="$orig_level"
		if [ -n "$result" ]; then
			loglocal "$DATE" GDD INFO "Used fallback '$WEATHER_FALLBACK' for daily min/max"
			echo "$result"
			return 0
		fi
	fi

	if [ -n "$WEATHER_FALLBACK_2" ] && [ "$WEATHER_FALLBACK_2" != "$orig_level" ] && [ "$WEATHER_FALLBACK_2" != "$WEATHER_FALLBACK" ]; then
		WEATHER_LEVEL="$WEATHER_FALLBACK_2"
		local result=$(fetch_daily_from_api)
		WEATHER_LEVEL="$orig_level"
		if [ -n "$result" ]; then
			loglocal "$DATE" GDD INFO "Used fallback_2 '$WEATHER_FALLBACK_2' for daily min/max"
			echo "$result"
			return 0
		fi
	fi

	return 1
}

# --- Open-Meteo universal fallback (free, keyless) ---
fetch_daily_from_openmeteo() {
	if [ -n "$LATITUDE" ] && [ -n "$LONGITUDE" ]; then
		local API_URL="https://api.open-meteo.com/v1/forecast?latitude=${LATITUDE}&longitude=${LONGITUDE}&daily=temperature_2m_max,temperature_2m_min&temperature_unit=fahrenheit&past_days=1&forecast_days=0&timezone=auto"
		local API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL" 2>/dev/null)
		local MAXTEMP=$(echo "$API_RESULT" | jq -r '.daily.temperature_2m_max[0] // empty' 2>/dev/null)
		local MINTEMP=$(echo "$API_RESULT" | jq -r '.daily.temperature_2m_min[0] // empty' 2>/dev/null)
		if [ -n "$MAXTEMP" ] && [ -n "$MINTEMP" ]; then
			loglocal "$DATE" GDD INFO "Used Open-Meteo universal fallback for daily min/max"
			echo "$MAXTEMP $MINTEMP"
			return 0
		fi
	fi
	return 1
}

# --- Fall back to local DB ---
fetch_daily_from_db() {
	local YESTERDAY_SHORT=$(date --date="yesterday" +%Y%m%d)
	local MAXTEMP=$(sqlite3 "$DATASOURCE" "SELECT MAX(weather_tempf) FROM allhivedata WHERE strftime('%Y%m%d', datetime(date,'localtime')) = '$YESTERDAY_SHORT';")
	local MINTEMP=$(sqlite3 "$DATASOURCE" "SELECT MIN(weather_tempf) FROM allhivedata WHERE strftime('%Y%m%d', datetime(date,'localtime')) = '$YESTERDAY_SHORT';")
	if [ -n "$MAXTEMP" ] && [ -n "$MINTEMP" ]; then
		echo "$MAXTEMP $MINTEMP"
		return 0
	fi
	return 1
}

# --- Main execution ---

# Step 1: Backfill any missing days this year (handles fresh installs and gaps)
backfill_from_openmeteo

# Step 2: Check if yesterday already has data (backfill may have covered it)
existing=$(sqlite3 "$DATASOURCE" "SELECT COUNT(*) FROM gdd WHERE gdddate = '$YESTERDAY';")
if [ "$existing" -gt 0 ]; then
	loglocal "$DATE" GDD INFO "Yesterday ($YESTERDAY) already has GDD data, skipping"
	exit 0
fi

# Step 3: Try to get yesterday's data from configured sources
TEMPS=""
GDD_SOURCE="none"

TEMPS=$(fetch_daily_from_api)
if [ -n "$TEMPS" ]; then GDD_SOURCE="$WEATHER_LEVEL"; fi

if [ -z "$TEMPS" ]; then
	TEMPS=$(fetch_daily_from_fallback)
	if [ -n "$TEMPS" ]; then GDD_SOURCE="fallback"; fi
fi

if [ -z "$TEMPS" ]; then
	TEMPS=$(fetch_daily_from_openmeteo)
	if [ -n "$TEMPS" ]; then GDD_SOURCE="openmeteo_fallback"; fi
fi

if [ -z "$TEMPS" ]; then
	TEMPS=$(fetch_daily_from_db)
	if [ -n "$TEMPS" ]; then GDD_SOURCE="local_db"; fi
fi

if [ -z "$TEMPS" ]; then
	loglocal "$DATE" GDD ERROR "Could not get yesterday's min/max temp from any source. Skipping GDD."
	exit 1
fi

MAXTEMP=$(echo "$TEMPS" | awk '{print $1}')
MINTEMP=$(echo "$TEMPS" | awk '{print $2}')

loglocal "$DATE" GDD INFO "Source: $GDD_SOURCE, Max: $MAXTEMP, Min: $MINTEMP"

# Step 4: Calculate and store
GDD=$(calc_gdd "$MAXTEMP" "$MINTEMP")

SEASONGDD=$(sqlite3 "$DATASOURCE" "SELECT COALESCE(SUM(daygdd), 0) FROM gdd WHERE gdddate >= '$SEASON_START' AND gdddate < '$YESTERDAY';")
SEASONGDD2=$(echo "$SEASONGDD + $GDD" | bc)

sqlite3 "$DATASOURCE" "INSERT INTO gdd (calcdate, gdddate, daygdd, seasongdd) VALUES ('$DATE', '$YESTERDAY', $GDD, $SEASONGDD2);"

loglocal "$DATE" GDD INFO "GDD=$GDD, Season=$SEASONGDD2, Source=$GDD_SOURCE"
