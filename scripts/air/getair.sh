#!/bin/bash
# version 2.0, 2026-04-20
# Air quality dispatcher — routes to the configured PurpleAir provider
# Output: DATE,AIR_TEMP,AIR_HUMIDITY,PM1_0,PM2_5,PM10,PM2_5_AQI,PM10_AQI,PM1_RAW,PM2_5_RAW,PM10_RAW,PRESSURE

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

case $AIR_TYPE in
	purpleapi|purple)
		GETAIR_DATA=$($HOMEDIR/scripts/air/purpleapi.sh)
		;;
	purplelocal)
		GETAIR_DATA=$($HOMEDIR/scripts/air/purpleairlocal.sh)
		;;
	*)
		;;
esac

if [[ $GETAIR_DATA = "ERROR" ]] || [[ -z "$GETAIR_DATA" ]]; then
	echo "null,null,null,null,null,null,null,null,null,null,null,null"
else
	echo "$GETAIR_DATA"
fi
