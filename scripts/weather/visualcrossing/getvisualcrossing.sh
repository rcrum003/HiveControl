#!/bin/bash
# Visual Crossing Weather API
# Free tier: 1000 calls/day. Includes solar radiation and historical data.
# Requires: KEY (API key from visualcrossing.com), LATITUDE, LONGITUDE in hiveconfig

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

VC_KEY="${KEY_VISUALCROSSING:-$KEY}"

if [ -z "$VC_KEY" ]; then
	loglocal "$DATE" WEATHER ERROR "Visual Crossing requires an API key. Set it in instrument config."
	exit 1
fi

if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
	loglocal "$DATE" WEATHER ERROR "Visual Crossing requires LATITUDE and LONGITUDE."
	exit 1
fi

API_URL="https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/${LATITUDE},${LONGITUDE}/today?unitGroup=us&include=current,days&key=${VC_KEY}&contentType=json"

TRYCOUNTER=1
DATA_GOOD=0

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]]; do
	API_RESULT=$(/usr/bin/curl -s --max-time 30 "$API_URL")

	TEMP_CHECK=$(echo "$API_RESULT" | jq -r '.currentConditions.temp // empty' 2>/dev/null)

	if [ -z "$TEMP_CHECK" ]; then
		loglocal "$DATE" WEATHER INFO "Visual Crossing: no valid response, attempt $TRYCOUNTER"
		let TRYCOUNTER+=1
		sleep 5
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD -eq 0 ]; then
	loglocal "$DATE" WEATHER ERROR "Visual Crossing: failed after retries"
	exit 1
fi

# Parse — unitGroup=us gives F, mph, inches
TEMP_F=$(echo "$API_RESULT" | jq -r '.currentConditions.temp')
HUMIDITY=$(echo "$API_RESULT" | jq -r '.currentConditions.humidity')
DEWPOINT_F=$(echo "$API_RESULT" | jq -r '.currentConditions.dew // empty')
WIND_MPH=$(echo "$API_RESULT" | jq -r '.currentConditions.windspeed // 0')
WIND_GUST_MPH=$(echo "$API_RESULT" | jq -r '.currentConditions.windgust // 0')
WIND_DIR_DEG=$(echo "$API_RESULT" | jq -r '.currentConditions.winddir // 0')
PRESSURE_IN=$(echo "$API_RESULT" | jq -r '.currentConditions.pressure // 0')
PRECIP_IN=$(echo "$API_RESULT" | jq -r '.currentConditions.precip // 0')
SOLAR_RAD=$(echo "$API_RESULT" | jq -r '.currentConditions.solarradiation // 0')
UV_INDEX=$(echo "$API_RESULT" | jq -r '.currentConditions.uvindex // 0')
OBS_TIME=$(echo "$API_RESULT" | jq -r '.currentConditions.datetime // empty')
PRECIP_TODAY_IN=$(echo "$API_RESULT" | jq -r '.days[0].precip // 0')
LOCATION=$(echo "$API_RESULT" | jq -r '.resolvedAddress // "VC"')

# Handle nulls
[ "$WIND_GUST_MPH" = "null" ] && WIND_GUST_MPH="0"
[ "$SOLAR_RAD" = "null" ] && SOLAR_RAD="0"
[ "$UV_INDEX" = "null" ] && UV_INDEX="0"
[ "$PRECIP_IN" = "null" ] && PRECIP_IN="0"
[ "$PRECIP_TODAY_IN" = "null" ] && PRECIP_TODAY_IN="0"
[ "$PRESSURE_IN" = "null" ] && PRESSURE_IN="0"

# Calculate dewpoint if not provided
if [ -z "$DEWPOINT_F" ] || [ "$DEWPOINT_F" = "null" ]; then
	DEWPOINT_F=$(calc_dewpoint_f "$TEMP_F" "$HUMIDITY")
fi

# Format observation time as full datetime
TODAY=$(TZ=":$TIMEZONE" date '+%Y-%m-%d')
if [ -n "$OBS_TIME" ] && [ "$OBS_TIME" != "null" ]; then
	WX_OBS_TIME="${TODAY} ${OBS_TIME}"
else
	WX_OBS_TIME="$DATE"
fi

WX_STATION_ID="VC_${LOCATION// /_}"
WX_TEMP_F="$TEMP_F"
WX_TEMP_C=$(f_to_c "$TEMP_F")
WX_HUMIDITY="$HUMIDITY"
WX_WIND_DIR_DEG="$WIND_DIR_DEG"
WX_WIND_DIR_TEXT=$(degrees_to_cardinal "$WIND_DIR_DEG")
WX_WIND_MPH="$WIND_MPH"
WX_WIND_GUST_MPH="$WIND_GUST_MPH"
WX_WIND_KPH=$(mph_to_kph "$WIND_MPH")
WX_WIND_GUST_KPH=$(mph_to_kph "$WIND_GUST_MPH")
WX_PRESSURE_MB=$(in_to_mb "$PRESSURE_IN")
WX_PRESSURE_IN="$PRESSURE_IN"
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
