#!/bin/bash
# Open-Meteo Weather API
# Free, no API key required. Uses ECMWF/DWD/NOAA model data.
# Requires: LATITUDE, LONGITUDE in hiveconfig

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
	loglocal "$DATE" WEATHER ERROR "Open-Meteo requires LATITUDE and LONGITUDE. Set them in site config."
	exit 1
fi

API_URL="https://api.open-meteo.com/v1/forecast?latitude=${LATITUDE}&longitude=${LONGITUDE}&current=temperature_2m,relative_humidity_2m,dew_point_2m,wind_speed_10m,wind_direction_10m,wind_gusts_10m,surface_pressure,precipitation,rain,shortwave_radiation,uv_index&daily=precipitation_sum&temperature_unit=fahrenheit&wind_speed_unit=mph&precipitation_unit=inch&forecast_days=1&timezone=auto"

TRYCOUNTER=1
DATA_GOOD=0

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]]; do
	API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL")

	TEMP_CHECK=$(echo "$API_RESULT" | jq -r '.current.temperature_2m // empty' 2>/dev/null)

	if [ -z "$TEMP_CHECK" ]; then
		loglocal "$DATE" WEATHER INFO "Open-Meteo: no valid response, attempt $TRYCOUNTER"
		let TRYCOUNTER+=1
		sleep 5
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD -eq 0 ]; then
	loglocal "$DATE" WEATHER ERROR "Open-Meteo: failed after retries"
	exit 1
fi

TEMP_F=$(echo "$API_RESULT" | jq -r '.current.temperature_2m')
HUMIDITY=$(echo "$API_RESULT" | jq -r '.current.relative_humidity_2m')
DEWPOINT_F=$(echo "$API_RESULT" | jq -r '.current.dew_point_2m')
WIND_MPH=$(echo "$API_RESULT" | jq -r '.current.wind_speed_10m')
WIND_DIR_DEG=$(echo "$API_RESULT" | jq -r '.current.wind_direction_10m')
WIND_GUST_MPH=$(echo "$API_RESULT" | jq -r '.current.wind_gusts_10m')
PRESSURE_MB=$(echo "$API_RESULT" | jq -r '.current.surface_pressure')
PRECIP_IN=$(echo "$API_RESULT" | jq -r '.current.precipitation')
RAIN_IN=$(echo "$API_RESULT" | jq -r '.current.rain')
SOLAR_RAD=$(echo "$API_RESULT" | jq -r '.current.shortwave_radiation')
UV_INDEX=$(echo "$API_RESULT" | jq -r '.current.uv_index')
OBS_TIME=$(echo "$API_RESULT" | jq -r '.current.time' | sed 's/T/ /')
PRECIP_TODAY_IN=$(echo "$API_RESULT" | jq -r '.daily.precipitation_sum[0] // 0')

# Handle null values
[ "$WIND_GUST_MPH" = "null" ] && WIND_GUST_MPH="0"
[ "$SOLAR_RAD" = "null" ] && SOLAR_RAD="0"
[ "$UV_INDEX" = "null" ] && UV_INDEX="0"
[ "$PRECIP_IN" = "null" ] && PRECIP_IN="0"
[ "$PRECIP_TODAY_IN" = "null" ] && PRECIP_TODAY_IN="0"

WX_STATION_ID="OPENMETEO"
WX_OBS_TIME="$OBS_TIME"
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
WX_SOLARRADIATION="$SOLAR_RAD"
WX_UV="$UV_INDEX"
WX_PRECIP_1HR_IN="$PRECIP_IN"
WX_PRECIP_1HR_MM=$(in_to_mm "$PRECIP_IN")
WX_PRECIP_TODAY_IN="$PRECIP_TODAY_IN"
WX_PRECIP_TODAY_MM=$(in_to_mm "$PRECIP_TODAY_IN")

output_wx_json
