#!/bin/bash
###################################
# EPA AirNow API — Ozone (O3) and Nitrogen Dioxide (NO2)
#
# Fetches hourly observations from nearest EPA regulatory monitor
# Stores in airquality_epa table (separate from PurpleAir PM data)
#
# API: https://docs.airnowapi.org/CurrentObservationsByLatLon/query
# Free tier: 500 requests/day (running hourly = 24/day)
#
# v 2026042001

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/air/air_helpers.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')
DB_PATH="/home/HiveControl/data/hive-data.db"

if [ "${ENABLE_AIRNOW:-no}" != "yes" ]; then
    echo "AirNow disabled"
    exit 0
fi

KEY="${KEY_AIRNOW:-}"
if [ -z "$KEY" ]; then
    echo "ERROR: No AirNow API key configured. Get one at https://docs.airnowapi.org/account/request/"
    loglocal "$DATE" AIRNOW ERROR "No AirNow API key configured. Get one at https://docs.airnowapi.org/account/request/"
    exit 1
fi

LAT="${LATITUDE:-}"
LON="${LONGITUDE:-}"
if [ -z "$LAT" ] || [ -z "$LON" ]; then
    echo "ERROR: Latitude/Longitude not configured. Set them in Step 1 of the Setup Wizard."
    loglocal "$DATE" AIRNOW ERROR "Latitude/Longitude not configured"
    exit 1
fi

DISTANCE="${AIRNOW_DISTANCE:-25}"

API_URL="https://www.airnowapi.org/aq/observation/latLong/current/?format=application/json&latitude=${LAT}&longitude=${LON}&distance=${DISTANCE}&API_KEY=${KEY}"

TEMPFILE=$(mktemp /tmp/airnow_XXXXXX.json)
trap 'rm -f "$TEMPFILE"' EXIT

RETRY=0
MAX_RETRY=3
SUCCESS=0

while [ $RETRY -lt $MAX_RETRY ] && [ $SUCCESS -eq 0 ]; do
    HTTP_CODE=$(/usr/bin/curl -s -w "%{http_code}" "$API_URL" -o "$TEMPFILE" 2>/dev/null)

    if [ "$HTTP_CODE" = "200" ] && [ -s "$TEMPFILE" ]; then
        ENTRY_COUNT=$(jq 'length' "$TEMPFILE" 2>/dev/null)
        if [ -n "$ENTRY_COUNT" ] && [ "$ENTRY_COUNT" -gt 0 ] 2>/dev/null; then
            SUCCESS=1
        else
            loglocal "$DATE" AIRNOW INFO "Empty response from AirNow (no monitors in range?), retry $((RETRY+1))"
            RETRY=$((RETRY+1))
            sleep 5
        fi
    else
        loglocal "$DATE" AIRNOW INFO "HTTP $HTTP_CODE from AirNow, retry $((RETRY+1))"
        RETRY=$((RETRY+1))
        sleep 5
    fi
done

if [ $SUCCESS -eq 0 ]; then
    echo "ERROR: Failed to get data from AirNow after $MAX_RETRY attempts (last HTTP code: $HTTP_CODE)"
    loglocal "$DATE" AIRNOW ERROR "Failed to get data from AirNow after $MAX_RETRY attempts"
    sqlite3 "$DB_PATH" "INSERT OR IGNORE INTO weather_health (date, provider, status, message) VALUES ('$(date +%Y-%m-%d)', 'airnow', 'fail', 'No response after $MAX_RETRY retries');" 2>/dev/null
    exit 1
fi

# Parse observations — AirNow returns array of parameter observations
O3_AQI=$(jq -r '[.[] | select(.ParameterName=="O3")] | .[0].AQI // empty' "$TEMPFILE")
NO2_AQI=$(jq -r '[.[] | select(.ParameterName=="NO2")] | .[0].AQI // empty' "$TEMPFILE")
PM25_AQI=$(jq -r '[.[] | select(.ParameterName=="PM2.5")] | .[0].AQI // empty' "$TEMPFILE")
PM10_AQI=$(jq -r '[.[] | select(.ParameterName=="PM10")] | .[0].AQI // empty' "$TEMPFILE")
REPORTING_AREA=$(jq -r '.[0].ReportingArea // empty' "$TEMPFILE")

# AirNow returns AQI directly, not raw concentrations for current obs
# For O3, back-calculate approximate ppm from AQI using inverse breakpoints
O3_PPM="null"
if [ -n "$O3_AQI" ] && [ "$O3_AQI" != "null" ]; then
    if [ "$O3_AQI" -le 50 ] 2>/dev/null; then
        O3_PPM=$(echo "scale=3; 0.054 * $O3_AQI / 50" | bc -l 2>/dev/null)
    elif [ "$O3_AQI" -le 100 ]; then
        O3_PPM=$(echo "scale=3; 0.055 + (0.070 - 0.055) * ($O3_AQI - 51) / (100 - 51)" | bc -l 2>/dev/null)
    fi
    O3_PPM="${O3_PPM:-null}"
fi

# For NO2, back-calculate approximate ppb from AQI
NO2_PPB="null"
if [ -n "$NO2_AQI" ] && [ "$NO2_AQI" != "null" ]; then
    if [ "$NO2_AQI" -le 50 ] 2>/dev/null; then
        NO2_PPB=$(echo "scale=1; 53 * $NO2_AQI / 50" | bc -l 2>/dev/null)
    elif [ "$NO2_AQI" -le 100 ]; then
        NO2_PPB=$(echo "scale=1; 54 + (100 - 54) * ($NO2_AQI - 51) / (100 - 51)" | bc -l 2>/dev/null)
    fi
    NO2_PPB="${NO2_PPB:-null}"
fi

# Default empty values to null
O3_AQI="${O3_AQI:-null}"
NO2_AQI="${NO2_AQI:-null}"
PM25_AQI="${PM25_AQI:-null}"
PM10_AQI="${PM10_AQI:-null}"
REPORTING_AREA="${REPORTING_AREA:-unknown}"

# Construct date for the record (current hour, truncated to hour)
RECORD_DATE=$(TZ=":$TIMEZONE" date '+%F %H:00')
RECORD_DATE_UTC=$(date -u '+%F %H:00')

# Insert into EPA table (REPLACE on duplicate date+source)
SQL="INSERT OR REPLACE INTO airquality_epa (date, date_utc, o3_ppm, o3_aqi, no2_ppb, no2_aqi, pm25_aqi, pm10_aqi, reporting_area, source) VALUES ('$RECORD_DATE', '$RECORD_DATE_UTC', '$O3_PPM', '$O3_AQI', '$NO2_PPB', '$NO2_AQI', '$PM25_AQI', '$PM10_AQI', '$REPORTING_AREA', 'airnow');"

if ! sqlite3 "$DB_PATH" "$SQL" 2>&1; then
    loglocal "$DATE" AIRNOW ERROR "Failed to insert EPA data into database"
    exit 1
fi

# Log health success
sqlite3 "$DB_PATH" "INSERT OR REPLACE INTO weather_health (date, provider, status, message) VALUES ('$(date +%Y-%m-%d)', 'airnow', 'ok', 'O3=$O3_AQI NO2=$NO2_AQI PM2.5=$PM25_AQI area=$REPORTING_AREA');" 2>/dev/null

echo "AirNow: O3_AQI=$O3_AQI, NO2_AQI=$NO2_AQI, PM2.5_AQI=$PM25_AQI, Area=$REPORTING_AREA"
