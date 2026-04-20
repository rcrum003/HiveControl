#!/bin/bash
# National Weather Service API (weather.gov)
# Free, no API key. US locations only.
# Requires: LATITUDE, LONGITUDE in hiveconfig
# Two-step: discover nearest station, then fetch latest observation.

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')
USER_AGENT="HiveControl/1.0 (contact: hivecontrol.org)"
NWS_CACHE="/tmp/nws_station_cache"

if [ -z "$LATITUDE" ] || [ -z "$LONGITUDE" ]; then
	loglocal "$DATE" WEATHER ERROR "NWS requires LATITUDE and LONGITUDE."
	exit 1
fi

# Step 1: Find nearest observation station (cache for 24 hours)
STATION_ID=""
if [ -f "$NWS_CACHE" ]; then
	CACHE_AGE=$(( $(date +%s) - $(stat -c %Y "$NWS_CACHE" 2>/dev/null || stat -f %m "$NWS_CACHE" 2>/dev/null || echo 0) ))
	if [ "$CACHE_AGE" -lt 86400 ]; then
		STATION_ID=$(cat "$NWS_CACHE")
	fi
fi

if [ -z "$STATION_ID" ]; then
	POINTS_RESULT=$(/usr/bin/curl -s --max-time 15 -H "User-Agent: $USER_AGENT" \
		"https://api.weather.gov/points/${LATITUDE},${LONGITUDE}")

	STATIONS_URL=$(echo "$POINTS_RESULT" | jq -r '.properties.observationStations // empty' 2>/dev/null)

	if [ -z "$STATIONS_URL" ]; then
		loglocal "$DATE" WEATHER ERROR "NWS: could not resolve location. US locations only."
		exit 1
	fi

	STATIONS_RESULT=$(/usr/bin/curl -s --max-time 15 -H "User-Agent: $USER_AGENT" "$STATIONS_URL")
	STATION_ID=$(echo "$STATIONS_RESULT" | jq -r '.features[0].properties.stationIdentifier // empty' 2>/dev/null)

	if [ -z "$STATION_ID" ]; then
		loglocal "$DATE" WEATHER ERROR "NWS: no observation stations found near ${LATITUDE},${LONGITUDE}"
		exit 1
	fi

	echo "$STATION_ID" > "$NWS_CACHE"
fi

# Step 2: Get latest observation
TRYCOUNTER=1
DATA_GOOD=0

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]]; do
	OBS_RESULT=$(/usr/bin/curl -s --max-time 30 -H "User-Agent: $USER_AGENT" \
		"https://api.weather.gov/stations/${STATION_ID}/observations/latest")

	TEMP_CHECK=$(echo "$OBS_RESULT" | jq -r '.properties.temperature.value // empty' 2>/dev/null)

	if [ -z "$TEMP_CHECK" ]; then
		loglocal "$DATE" WEATHER INFO "NWS: no valid observation from $STATION_ID, attempt $TRYCOUNTER"
		let TRYCOUNTER+=1
		sleep 5
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD -eq 0 ]; then
	loglocal "$DATE" WEATHER ERROR "NWS: failed after retries for station $STATION_ID"
	exit 1
fi

# Parse — NWS returns SI units
TEMP_C=$(echo "$OBS_RESULT" | jq -r '.properties.temperature.value')
HUMIDITY=$(echo "$OBS_RESULT" | jq -r '.properties.relativeHumidity.value // 0' | cut -d. -f1)
DEWPOINT_C=$(echo "$OBS_RESULT" | jq -r '.properties.dewpoint.value // empty')
WIND_DIR_DEG=$(echo "$OBS_RESULT" | jq -r '.properties.windDirection.value // 0')
WIND_KPH=$(echo "$OBS_RESULT" | jq -r '.properties.windSpeed.value // 0')
WIND_GUST_KPH=$(echo "$OBS_RESULT" | jq -r '.properties.windGust.value // 0')
PRESSURE_PA=$(echo "$OBS_RESULT" | jq -r '.properties.barometricPressure.value // 0')
PRECIP_MM=$(echo "$OBS_RESULT" | jq -r '.properties.precipitationLastHour.value // 0')
OBS_TIMESTAMP=$(echo "$OBS_RESULT" | jq -r '.properties.timestamp')

# Handle nulls
[ "$TEMP_C" = "null" ] && TEMP_C="0"
[ "$HUMIDITY" = "null" ] && HUMIDITY="0"
[ "$DEWPOINT_C" = "null" ] && DEWPOINT_C=""
[ "$WIND_DIR_DEG" = "null" ] && WIND_DIR_DEG="0"
[ "$WIND_KPH" = "null" ] && WIND_KPH="0"
[ "$WIND_GUST_KPH" = "null" ] && WIND_GUST_KPH="0"
[ "$PRESSURE_PA" = "null" ] && PRESSURE_PA="0"
[ "$PRECIP_MM" = "null" ] && PRECIP_MM="0"

# Convert units
TEMP_F=$(c_to_f "$TEMP_C")
WIND_MPH=$(kph_to_mph "$WIND_KPH")
WIND_GUST_MPH=$(kph_to_mph "$WIND_GUST_KPH")
PRESSURE_MB=$(pa_to_mb "$PRESSURE_PA")
PRESSURE_IN=$(pa_to_in "$PRESSURE_PA")

if [ -n "$DEWPOINT_C" ]; then
	DEWPOINT_F=$(c_to_f "$DEWPOINT_C")
else
	DEWPOINT_F=$(calc_dewpoint_f "$TEMP_F" "$HUMIDITY")
	DEWPOINT_C=$(f_to_c "$DEWPOINT_F")
fi

PRECIP_IN=$(mm_to_in "$PRECIP_MM")
OBS_TIME=$(echo "$OBS_TIMESTAMP" | sed 's/T/ /' | cut -c1-19)

WX_STATION_ID="NWS_${STATION_ID}"
WX_OBS_TIME="$OBS_TIME"
WX_TEMP_F="$TEMP_F"
WX_TEMP_C="$TEMP_C"
WX_HUMIDITY="$HUMIDITY"
WX_WIND_DIR_DEG="$WIND_DIR_DEG"
WX_WIND_DIR_TEXT=$(degrees_to_cardinal "$WIND_DIR_DEG")
WX_WIND_MPH="$WIND_MPH"
WX_WIND_GUST_MPH="$WIND_GUST_MPH"
WX_WIND_KPH="$WIND_KPH"
WX_WIND_GUST_KPH="$WIND_GUST_KPH"
WX_PRESSURE_MB="$PRESSURE_MB"
WX_PRESSURE_IN="$PRESSURE_IN"
WX_PRESSURE_TREND="-"
WX_DEWPOINT_F="$DEWPOINT_F"
WX_DEWPOINT_C="$DEWPOINT_C"
WX_SOLARRADIATION="0"
WX_UV="0"
WX_PRECIP_1HR_IN="$PRECIP_IN"
WX_PRECIP_1HR_MM="$PRECIP_MM"
WX_PRECIP_TODAY_IN="0"
WX_PRECIP_TODAY_MM="0"

output_wx_json
