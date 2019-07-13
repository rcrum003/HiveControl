#!/bin/bash
# version 1.0, 2019-07-13
# reads the temp sensors as one script


# Get some variables from our central file
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

# Data Fetchers/Parsers in one

case $air_type in
	purple)
		GETAIR_DATA=$($HOMEDIR/scripts/air/purpleair.sh)
		;;
	*)
		#all others
		;;
esac


if [[ $GETAIR_DATA = "ERROR" ]]; then
	#set values to null
		AIR_DATE="null"
		AIR_TEMP="null"
		AIR_HUMIDITY="null"
		AIR_PM1="null"
		AIR_PM2_5="null"
		AIR_PM10="null"

fi

#echo "DATE,AIR_TEMP,AIR_HUMIDITY,PM1_0,PM2_5,PM10"
echo "$GETAIR_DATA"
	
	#AIR_DATE=$(echo $GETAIR_DATA |awk -F, '{print $1}')
	#AIR_TEMP=$(echo $GETAIR_DATA |awk -F, '{print $2}')
	#AIR_HUMIDITY=$(echo $GETAIR_DATA |awk -F, '{print $3}')
	#AIR_PM1=$(echo $GETAIR_DATA |awk -F, '{print $4}')
	#AIR_PM2_5=$(echo $GETAIR_DATA |awk -F, '{print $5}')
	#AIR_PM10=$(echo $GETAIR_DATA |awk -F, '{print $6}')
	
		#echo "RAW - TEMPF=$HIVETEMPF, TEMPC=$HIVETEMPC, HUMI=$HIVEHUMIDITY"
		#Use TEMP Slope/Intercept
		



