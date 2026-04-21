#!/bin/bash
# Pirate Weather API (Dark Sky compatible)
# Free tier: 20,000 calls/day. Uses HRRR/GFS/ERA5 models.
# Requires: KEY (API key from pirateweather.net), LATITUDE, LONGITUDE in hiveconfig

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

PW_KEY="${KEY_PIRATEWEATHER:-$KEY}"

if [ -z "$PW_KEY" ]; then
	loglocal "$DATE" WEATHER ERROR "Pirate Weather requires an API key. Get one at pirateweather.net"
	exit 1
fi

if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
	loglocal "$DATE" WEATHER ERROR "Pirate Weather requires LATITUDE and LONGITUDE."
	exit 1
fi

API_URL="https://api.pirateweather.net/forecast/${PW_KEY}/${LATITUDE},${LONGITUDE}?units=us&exclude=minutely,hourly,alerts"

TRYCOUNTER=1
DATA_GOOD=0

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]]; do
	API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL")

	TEMP_CHECK=$(echo "$API_RESULT" | jq -r '.currently.temperature // empty' 2>/dev/null)

	if [ -z "$TEMP_CHECK" ]; then
		ERROR_MSG=$(echo "$API_RESULT" | jq -r '.error // "unknown error"' 2>/dev/null)
		loglocal "$DATE" WEATHER INFO "Pirate Weather: no valid response ($ERROR_MSG), attempt $TRYCOUNTER"
		let TRYCOUNTER+=1
		sleep 5
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD -eq 0 ]; then
	loglocal "$DATE" WEATHER ERROR "Pirate Weather: failed after retries"
	exit 1
fi

TEMP_F=$(echo "$API_RESULT" | jq -r '.currently.temperature')
HUMIDITY_RAW=$(echo "$API_RESULT" | jq -r '.currently.humidity // 0')
[ "$HUMIDITY_RAW" = "null" ] || [ -z "$HUMIDITY_RAW" ] && HUMIDITY_RAW="0"
HUMIDITY=$(echo "scale=0; $HUMIDITY_RAW * 100 / 1" | bc)
DEWPOINT_F=$(echo "$API_RESULT" | jq -r '.currently.dewPoint // empty')
WIND_MPH=$(echo "$API_RESULT" | jq -r '.currently.windSpeed // 0')
WIND_DIR_DEG=$(echo "$API_RESULT" | jq -r '.currently.windBearing // 0')
WIND_GUST_MPH=$(echo "$API_RESULT" | jq -r '.currently.windGust // 0')
PRESSURE_MB=$(echo "$API_RESULT" | jq -r '.currently.pressure // 0')
PRECIP_IN=$(echo "$API_RESULT" | jq -r '.currently.precipIntensity // 0')
UV_INDEX=$(echo "$API_RESULT" | jq -r '.currently.uvIndex // 0')
OBS_TIME=$(echo "$API_RESULT" | jq -r '.currently.time // empty')

[ "$WIND_MPH" = "null" ] && WIND_MPH="0"
[ "$WIND_GUST_MPH" = "null" ] && WIND_GUST_MPH="0"
[ "$PRESSURE_MB" = "null" ] && PRESSURE_MB="0"
[ "$PRECIP_IN" = "null" ] && PRECIP_IN="0"
[ "$UV_INDEX" = "null" ] && UV_INDEX="0"

# Convert epoch to readable time
if [ -n "$OBS_TIME" ] && [ "$OBS_TIME" != "null" ]; then
	OBS_TIME_STR=$(date -d "@$OBS_TIME" '+%Y-%m-%d %H:%M' 2>/dev/null)
	[ -z "$OBS_TIME_STR" ] && OBS_TIME_STR=$(date --date="@$OBS_TIME" '+%Y-%m-%d %H:%M' 2>/dev/null)
else
	OBS_TIME_STR="$DATE"
fi

if [ -z "$DEWPOINT_F" ] || [ "$DEWPOINT_F" = "null" ]; then
	DEWPOINT_F=$(calc_dewpoint_f "$TEMP_F" "$HUMIDITY")
fi

WX_STATION_ID="PIRATEWEATHER"
WX_OBS_TIME="$OBS_TIME_STR"
WX_TEMP_F="$TEMP_F"
WX_TEMP_C=$(f_to_c "$TEMP_F")
WX_HUMIDITY="$HUMIDITY"
WX_WIND_DIR_DEG="$WIND_DIR_DEG"
WX_WIND_DIR_TEXT=$(degrees_to_cardinal "$WIND_DIR_DEG")
WX_WIND_MPH="$WIND_MPH"
WX_WIND_GUST_MPH="$WIND_GUST_MPH"
WX_WIND_KPH=$(mph_to_kph "$WIND_MPH")
WX_WIND_GUST_KPH=$(mph_to_kph "$WIND_GUST_MPH")
WX_PRESSURE_MB="$PRESSURE_MB"
WX_PRESSURE_IN=$(mb_to_in "$PRESSURE_MB")
WX_PRESSURE_TREND="-"
WX_DEWPOINT_F="$DEWPOINT_F"
WX_DEWPOINT_C=$(f_to_c "$DEWPOINT_F")
WX_SOLARRADIATION="0"
WX_UV="$UV_INDEX"
WX_PRECIP_1HR_IN="$PRECIP_IN"
WX_PRECIP_1HR_MM=$(in_to_mm "$PRECIP_IN")
WX_PRECIP_TODAY_IN="0"
WX_PRECIP_TODAY_MM="0"

output_wx_json
