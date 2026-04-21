#!/bin/bash
# WeatherAPI.com
# Free tier: 1M calls/month. Provides comprehensive current conditions.
# Requires: KEY (API key from weatherapi.com), LATITUDE, LONGITUDE in hiveconfig

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

WAPI_KEY="${KEY_WEATHERAPI:-$KEY}"

if [ -z "$WAPI_KEY" ]; then
	loglocal "$DATE" WEATHER ERROR "WeatherAPI.com requires an API key. Set it in instrument config."
	exit 1
fi

if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
	loglocal "$DATE" WEATHER ERROR "WeatherAPI.com requires LATITUDE and LONGITUDE."
	exit 1
fi

API_URL="https://api.weatherapi.com/v1/current.json?key=${WAPI_KEY}&q=${LATITUDE},${LONGITUDE}"

TRYCOUNTER=1
DATA_GOOD=0

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]]; do
	API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL")

	TEMP_CHECK=$(echo "$API_RESULT" | jq -r '.current.temp_f // empty' 2>/dev/null)

	if [ -z "$TEMP_CHECK" ]; then
		ERROR_MSG=$(echo "$API_RESULT" | jq -r '.error.message // "unknown error"' 2>/dev/null)
		loglocal "$DATE" WEATHER INFO "WeatherAPI.com: no valid response ($ERROR_MSG), attempt $TRYCOUNTER"
		let TRYCOUNTER+=1
		sleep 5
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD -eq 0 ]; then
	loglocal "$DATE" WEATHER ERROR "WeatherAPI.com: failed after retries"
	exit 1
fi

# Parse — WeatherAPI provides both imperial and metric
TEMP_F=$(echo "$API_RESULT" | jq -r '.current.temp_f')
TEMP_C=$(echo "$API_RESULT" | jq -r '.current.temp_c')
HUMIDITY=$(echo "$API_RESULT" | jq -r '.current.humidity')
WIND_MPH=$(echo "$API_RESULT" | jq -r '.current.wind_mph')
WIND_KPH=$(echo "$API_RESULT" | jq -r '.current.wind_kph')
WIND_DIR_DEG=$(echo "$API_RESULT" | jq -r '.current.wind_degree')
WIND_DIR_TEXT=$(echo "$API_RESULT" | jq -r '.current.wind_dir')
GUST_MPH=$(echo "$API_RESULT" | jq -r '.current.gust_mph // 0')
GUST_KPH=$(echo "$API_RESULT" | jq -r '.current.gust_kph // 0')
PRESSURE_MB=$(echo "$API_RESULT" | jq -r '.current.pressure_mb')
PRESSURE_IN=$(echo "$API_RESULT" | jq -r '.current.pressure_in')
PRECIP_MM=$(echo "$API_RESULT" | jq -r '.current.precip_mm // 0')
PRECIP_IN=$(echo "$API_RESULT" | jq -r '.current.precip_in // 0')
UV_INDEX=$(echo "$API_RESULT" | jq -r '.current.uv // 0')
DEWPOINT_F=$(echo "$API_RESULT" | jq -r '.current.dewpoint_f // empty' 2>/dev/null)
DEWPOINT_C=$(echo "$API_RESULT" | jq -r '.current.dewpoint_c // empty' 2>/dev/null)
LAST_UPDATED=$(echo "$API_RESULT" | jq -r '.current.last_updated')
LOCATION=$(echo "$API_RESULT" | jq -r '.location.name // "WAPI"')

[ "$GUST_MPH" = "null" ] && GUST_MPH="0"
[ "$GUST_KPH" = "null" ] && GUST_KPH="0"
[ "$PRECIP_MM" = "null" ] && PRECIP_MM="0"
[ "$PRECIP_IN" = "null" ] && PRECIP_IN="0"
[ "$UV_INDEX" = "null" ] && UV_INDEX="0"

# Calculate dewpoint if not provided
if [ -z "$DEWPOINT_F" ] || [ "$DEWPOINT_F" = "null" ]; then
	DEWPOINT_F=$(calc_dewpoint_f "$TEMP_F" "$HUMIDITY")
	DEWPOINT_C=$(f_to_c "$DEWPOINT_F")
fi

WX_STATION_ID="WAPI_${LOCATION}"
WX_OBS_TIME="$LAST_UPDATED"
WX_TEMP_F="$TEMP_F"
WX_TEMP_C="$TEMP_C"
WX_HUMIDITY="$HUMIDITY"
WX_WIND_DIR_DEG="$WIND_DIR_DEG"
WX_WIND_DIR_TEXT="$WIND_DIR_TEXT"
WX_WIND_MPH="$WIND_MPH"
WX_WIND_GUST_MPH="$GUST_MPH"
WX_WIND_KPH="$WIND_KPH"
WX_WIND_GUST_KPH="$GUST_KPH"
WX_PRESSURE_MB="$PRESSURE_MB"
WX_PRESSURE_IN="$PRESSURE_IN"
WX_PRESSURE_TREND="-"
WX_DEWPOINT_F="$DEWPOINT_F"
WX_DEWPOINT_C="$DEWPOINT_C"
WX_SOLARRADIATION="0"
WX_UV="$UV_INDEX"
WX_PRECIP_1HR_IN="$PRECIP_IN"
WX_PRECIP_1HR_MM="$PRECIP_MM"
WX_PRECIP_TODAY_IN="0"
WX_PRECIP_TODAY_MM="0"

output_wx_json
