#!/bin/bash
#
# Script to get weather data from Weather Underground PWS API
# Uses the api.weather.com/v2/pws/observations/current endpoint
#
# Revision 3
# 2026-04-22
#
# The old XML endpoint (api.wunderground.com) was killed by IBM.
# PWS owners who upload data get a free API key for the new JSON endpoint.

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %R')

if [ -z "$WXAPIKEY" ]; then
	loglocal "$DATE" WEATHER ERROR "WXAPIKEY not configured — cannot fetch from WX Underground PWS API"
	exit 1
fi

if [ -z "$WXSTATION" ]; then
	loglocal "$DATE" WEATHER ERROR "WXSTATION not configured"
	exit 1
fi

TRYCOUNTER="1"
DATA_GOOD="0"

TMPFILE=$(mktemp /tmp/getwxxml_XXXXXX.json)
CURLCFG=$(mktemp /tmp/curlcfg_XXXXXX)
chmod 600 "$CURLCFG"
trap 'rm -f "$TMPFILE" "$CURLCFG"' EXIT

printf 'url = "https://api.weather.com/v2/pws/observations/current?stationId=%s&format=json&units=e&apiKey=%s&numericPrecision=decimal"\n' \
	"$WXSTATION" "$WXAPIKEY" > "$CURLCFG"

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do
	/usr/bin/curl --retry 3 --max-time 30 --silent --config "$CURLCFG" \
		> "$TMPFILE"

	CHECKERROR=$?

	if [ "$CHECKERROR" -ne "0" ]; then
		loglocal "$DATE" WEATHER INFO "Did not get a proper response from WX Underground PWS API, trying again"
		let TRYCOUNTER+=1
	else
		# Verify we got valid JSON with observations
		if jq -e '.observations[0]' "$TMPFILE" > /dev/null 2>&1; then
			DATA_GOOD=1
		else
			loglocal "$DATE" WEATHER INFO "WX Underground PWS API returned invalid data, trying again"
			let TRYCOUNTER+=1
		fi
	fi
done

if [ $DATA_GOOD = "0" ]; then
	loglocal "$DATE" WEATHER ERROR "Did not get a proper response from WX Underground PWS API after retries"
	exit 1
fi

# Parse JSON response using jq
OBS=$(cat "$TMPFILE")

DATEUTC=$(echo "$OBS" | jq -r '.observations[0].obsTimeUtc // empty')
TEMP_F=$(echo "$OBS" | jq -r '.observations[0].imperial.temp // empty')
HUMIDITY=$(echo "$OBS" | jq -r '.observations[0].humidity // empty')
WIND_DIR=$(echo "$OBS" | jq -r '.observations[0].winddir // empty')
WIND_SPEED_MPH=$(echo "$OBS" | jq -r '.observations[0].imperial.windSpeed // empty')
WIND_GUST_MPH=$(echo "$OBS" | jq -r '.observations[0].imperial.windGust // empty')
PRESSURE_IN=$(echo "$OBS" | jq -r '.observations[0].imperial.pressure // empty')
DEWPOINT_F=$(echo "$OBS" | jq -r '.observations[0].imperial.dewpt // empty')
RAIN_HOURLY_IN=$(echo "$OBS" | jq -r '.observations[0].imperial.precipRate // empty')
RAIN_DAILY_IN=$(echo "$OBS" | jq -r '.observations[0].imperial.precipTotal // empty')
SOLAR_RADIATION=$(echo "$OBS" | jq -r '.observations[0].solarRadiation // empty')
UV=$(echo "$OBS" | jq -r '.observations[0].uv // empty')

if [ -z "$TEMP_F" ]; then
	loglocal "$DATE" WEATHER ERROR "Did not get a proper Temp from WX Underground PWS API"
	exit 1
fi

# Derived values
temp_c=$(f_to_c "$TEMP_F")
DEWPOINT_C=$(f_to_c "$DEWPOINT_F")
PRESSURE_MB=$(in_to_mb "$PRESSURE_IN")
DIRECTION=$(degrees_to_cardinal "$WIND_DIR")
wind_kph=$(mph_to_kph "$WIND_SPEED_MPH")
wind_gust_kph=$(mph_to_kph "$WIND_GUST_MPH")
precip_1hr_metric=$(in_to_mm "$RAIN_HOURLY_IN")
precip_today_metric=$(in_to_mm "$RAIN_DAILY_IN")

# Output standardized WunderGround-format JSON
WX_STATION_ID="$WXSTATION"
WX_OBS_TIME="${DATEUTC:-$DATE}"
WX_TEMP_F="$TEMP_F"
WX_TEMP_C="$temp_c"
WX_HUMIDITY="$HUMIDITY"
WX_WIND_DIR_TEXT="$DIRECTION"
WX_WIND_DIR_DEG="$WIND_DIR"
WX_WIND_MPH="$WIND_SPEED_MPH"
WX_WIND_GUST_MPH="$WIND_GUST_MPH"
WX_WIND_KPH="$wind_kph"
WX_WIND_GUST_KPH="$wind_gust_kph"
WX_PRESSURE_MB="$PRESSURE_MB"
WX_PRESSURE_IN="$PRESSURE_IN"
WX_PRESSURE_TREND="-"
WX_DEWPOINT_F="$DEWPOINT_F"
WX_DEWPOINT_C="$DEWPOINT_C"
WX_SOLARRADIATION="$SOLAR_RADIATION"
WX_UV="$UV"
WX_PRECIP_1HR_IN="$RAIN_HOURLY_IN"
WX_PRECIP_1HR_MM="$precip_1hr_metric"
WX_PRECIP_TODAY_IN="$RAIN_DAILY_IN"
WX_PRECIP_TODAY_MM="$precip_today_metric"
output_wx_json
