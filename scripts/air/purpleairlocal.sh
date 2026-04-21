
#!/bin/bash
###################################
# Purple Air - Local sensor via LAN
#
# Pulls air quality data from a PurpleAir sensor on local network
# Outputs expanded CSV: DATE,AIR_TEMP,AIR_HUMIDITY,PM1_0,PM2_5,PM10,PM2_5_AQI,PM10_AQI,PM1_RAW,PM2_5_RAW,PM10_RAW,PRESSURE

# v 2026042001

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/air/air_helpers.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

AIR_URL="$AIR_LOCAL_URL"
TEMPFILE=$(mktemp /tmp/purpleairlocal_XXXXXX.json)
trap 'rm -f "$TEMPFILE"' EXIT

TRYCOUNTER="1"
DATA_GOOD="0"

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do
	RESULT=$(/usr/bin/curl -s -w %{http_code} "$AIR_URL" -o $TEMPFILE)

	# Particle counts (p_X_um) — count-based, used for legacy compatibility
	PM2_5=$(jq --raw-output '.p_2_5_um // empty' <$TEMPFILE)
	PM1_0=$(jq --raw-output '.p_1_0_um // empty' <$TEMPFILE)
	PM10=$(jq --raw-output '.p_10_0_um // empty' <$TEMPFILE)

	# ATM-corrected mass concentrations (ug/m3) — input for EPA AQI calculation
	PM2_5_RAW=$(jq --raw-output '.pm2_5_atm // empty' <$TEMPFILE)
	PM10_RAW=$(jq --raw-output '.pm10_0_atm // empty' <$TEMPFILE)
	PM1_RAW=$(jq --raw-output '.pm1_0_atm // empty' <$TEMPFILE)

	# Environmental readings from onboard BME280
	AIR_TEMP=$(jq --raw-output '.current_temp_f // empty' <$TEMPFILE)
	AIR_HUMIDITY=$(jq --raw-output '.current_humidity // empty' <$TEMPFILE)
	PRESSURE=$(jq --raw-output '.pressure // empty' <$TEMPFILE)

	if [ -z "$PM2_5" ]; then
		loglocal "$DATE" AIR INFO "Did not get a proper value for PM2_5 from $AIR_URL, trying again"
		let TRYCOUNTER+=1
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD = "0" ]; then
	loglocal "$DATE" AIR ERROR "Did not get a proper response from $AIR_URL- $RESULT"
	echo "ERROR"
	exit
fi

# Compute EPA AQI from ATM-corrected concentrations
PM2_5_AQI=$(pm25_to_aqi "${PM2_5_RAW:-0}")
PM10_AQI=$(pm10_to_aqi "${PM10_RAW:-0}")

# Default any empty values to null
PM1_RAW="${PM1_RAW:-null}"
PM2_5_RAW="${PM2_5_RAW:-null}"
PM10_RAW="${PM10_RAW:-null}"
PRESSURE="${PRESSURE:-null}"
PM2_5_AQI="${PM2_5_AQI:-null}"
PM10_AQI="${PM10_AQI:-null}"

rm -f "$TEMPFILE"

echo "$DATE,$AIR_TEMP,$AIR_HUMIDITY,$PM1_0,$PM2_5,$PM10,$PM2_5_AQI,$PM10_AQI,$PM1_RAW,$PM2_5_RAW,$PM10_RAW,$PRESSURE"
