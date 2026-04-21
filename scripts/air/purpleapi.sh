
#!/bin/bash
###################################
# Purple Air — Cloud API (api.purpleair.com)
#
# Pulls air quality data from PurpleAir v1 API
# Outputs expanded CSV: DATE,AIR_TEMP,AIR_HUMIDITY,PM1_0,PM2_5,PM10,PM2_5_AQI,PM10_AQI,PM1_RAW,PM2_5_RAW,PM10_RAW,PRESSURE

# v 2026042001

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/air/air_helpers.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

AIR_URL="https://api.purpleair.com/v1/sensors"

PURPLE_API_KEY="${AIR_API:-}"

if [ -z "$PURPLE_API_KEY" ]; then
	loglocal "$DATE" AIR ERROR "No PurpleAir API key configured (AIR_API). Set it in the instrument config."
	echo "ERROR"
	exit 1
fi

TRYCOUNTER="1"
DATA_GOOD="0"

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do
	API_RESULT=$(/usr/bin/curl -s "$AIR_URL/$AIR_ID" --header "X-API-Key: $PURPLE_API_KEY")

	# Mass concentrations from API (ug/m3)
	PM2_5=$(jq --raw-output '.sensor."pm2.5" // empty' <<< "$API_RESULT")
	PM1_0=$(jq --raw-output '.sensor."pm1.0" // empty' <<< "$API_RESULT")
	PM10=$(jq --raw-output '.sensor."pm10.0" // empty' <<< "$API_RESULT")

	# These are the same values on the cloud API (ATM-corrected)
	PM2_5_RAW="${PM2_5}"
	PM10_RAW="${PM10}"
	PM1_RAW="${PM1_0}"

	# Environmental readings
	AIR_TEMP=$(jq --raw-output '.sensor.temperature // empty' <<< "$API_RESULT")
	AIR_HUMIDITY=$(jq --raw-output '.sensor.humidity // empty' <<< "$API_RESULT")
	PRESSURE=$(jq --raw-output '.sensor.pressure // empty' <<< "$API_RESULT")

	if [ -z "$PM2_5" ]; then
		loglocal "$DATE" AIR INFO "Did not get a proper response from PurpleAir API, trying again"
		let TRYCOUNTER+=1
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD = "0" ]; then
	loglocal "$DATE" AIR ERROR "Did not get a proper response from PurpleAir API"
	echo "ERROR"
	exit
fi

# Compute EPA AQI
PM2_5_AQI=$(pm25_to_aqi "${PM2_5_RAW:-0}")
PM10_AQI=$(pm10_to_aqi "${PM10_RAW:-0}")

# Default any empty values to null
PM1_RAW="${PM1_RAW:-null}"
PM2_5_RAW="${PM2_5_RAW:-null}"
PM10_RAW="${PM10_RAW:-null}"
PRESSURE="${PRESSURE:-null}"
PM2_5_AQI="${PM2_5_AQI:-null}"
PM10_AQI="${PM10_AQI:-null}"

echo "$DATE,$AIR_TEMP,$AIR_HUMIDITY,$PM1_0,$PM2_5,$PM10,$PM2_5_AQI,$PM10_AQI,$PM1_RAW,$PM2_5_RAW,$PM10_RAW,$PRESSURE"
