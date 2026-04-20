#!/bin/bash
# OpenWeatherMap API (v2.5 - free tier, 1000 calls/day)
# Requires: KEY (API key from openweathermap.org), LATITUDE, LONGITUDE in hiveconfig

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

OWM_KEY="$KEY"

if [ -z "$OWM_KEY" ]; then
	loglocal "$DATE" WEATHER ERROR "OpenWeatherMap requires an API key. Set it in instrument config (KEY field)."
	exit 1
fi

if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
	loglocal "$DATE" WEATHER ERROR "OpenWeatherMap requires LATITUDE and LONGITUDE."
	exit 1
fi

API_URL="https://api.openweathermap.org/data/2.5/weather?lat=${LATITUDE}&lon=${LONGITUDE}&appid=${OWM_KEY}&units=imperial"

TRYCOUNTER=1
DATA_GOOD=0

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]]; do
	API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL")

	TEMP_CHECK=$(echo "$API_RESULT" | jq -r '.main.temp // empty' 2>/dev/null)

	if [ -z "$TEMP_CHECK" ]; then
		ERROR_MSG=$(echo "$API_RESULT" | jq -r '.message // "unknown error"' 2>/dev/null)
		loglocal "$DATE" WEATHER INFO "OpenWeatherMap: no valid response ($ERROR_MSG), attempt $TRYCOUNTER"
		let TRYCOUNTER+=1
		sleep 5
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD -eq 0 ]; then
	loglocal "$DATE" WEATHER ERROR "OpenWeatherMap: failed after retries"
	exit 1
fi

# Parse response — units=imperial gives temp in F, wind in mph
TEMP_F=$(echo "$API_RESULT" | jq -r '.main.temp')
HUMIDITY=$(echo "$API_RESULT" | jq -r '.main.humidity')
WIND_MPH=$(echo "$API_RESULT" | jq -r '.wind.speed')
WIND_DIR_DEG=$(echo "$API_RESULT" | jq -r '.wind.deg // 0')
WIND_GUST_MPH=$(echo "$API_RESULT" | jq -r '.wind.gust // 0')
# Pressure is always in hPa regardless of units param
PRESSURE_HPA=$(echo "$API_RESULT" | jq -r '.main.pressure')
# Rain is always in mm regardless of units param
RAIN_1H_MM=$(echo "$API_RESULT" | jq -r '.rain."1h" // 0')
RAIN_3H_MM=$(echo "$API_RESULT" | jq -r '.rain."3h" // 0')
DT=$(echo "$API_RESULT" | jq -r '.dt')
STATION_NAME=$(echo "$API_RESULT" | jq -r '.name // "OWM"')

[ "$WIND_GUST_MPH" = "null" ] && WIND_GUST_MPH="0"
[ "$RAIN_1H_MM" = "null" ] && RAIN_1H_MM="0"

WX_STATION_ID="OWM_${STATION_NAME}"
WX_OBS_TIME=$(TZ=":$TIMEZONE" date -d @"$DT" '+%Y-%m-%d %H:%M:%S' 2>/dev/null || date -r "$DT" '+%Y-%m-%d %H:%M:%S' 2>/dev/null || echo "$DATE")
WX_TEMP_F="$TEMP_F"
WX_TEMP_C=$(f_to_c "$TEMP_F")
WX_HUMIDITY="$HUMIDITY"
WX_WIND_DIR_DEG="$WIND_DIR_DEG"
WX_WIND_DIR_TEXT=$(degrees_to_cardinal "$WIND_DIR_DEG")
WX_WIND_MPH="$WIND_MPH"
WX_WIND_GUST_MPH="$WIND_GUST_MPH"
WX_WIND_KPH=$(mph_to_kph "$WIND_MPH")
WX_WIND_GUST_KPH=$(mph_to_kph "$WIND_GUST_MPH")
WX_PRESSURE_MB="$PRESSURE_HPA"
WX_PRESSURE_IN=$(mb_to_in "$PRESSURE_HPA")
WX_PRESSURE_TREND="-"
DEWPOINT_F=$(calc_dewpoint_f "$TEMP_F" "$HUMIDITY")
WX_DEWPOINT_F="$DEWPOINT_F"
WX_DEWPOINT_C=$(f_to_c "$DEWPOINT_F")
WX_SOLARRADIATION="0"
WX_UV="0"
RAIN_1H_IN=$(mm_to_in "$RAIN_1H_MM")
WX_PRECIP_1HR_IN="$RAIN_1H_IN"
WX_PRECIP_1HR_MM="$RAIN_1H_MM"
WX_PRECIP_TODAY_IN="0"
WX_PRECIP_TODAY_MM="0"

output_wx_json
