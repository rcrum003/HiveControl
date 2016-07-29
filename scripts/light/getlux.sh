#!/bin/bash
# version 0.9
# reads the LUX sensors as one script


# Get some variables from our central file
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')
			
if [ $LUX_SOURCE = "tsl2591" ]; then
	lux=$($HOMEDIR/scripts/light/tsl2591.sh)
elif [[ $LUX_SOURCE = "tsl2561" ]]; then
	lux=$($HOMEDIR/scripts/light/tsl2561.sh)
elif [ $LUX_SOURCE = "wx" ]; then
	#echo "- getting solarradiation from weatherstation"
	lux="null"
	echo "null"
	exit
fi
#echo "RAW LUX = $lux1"
#check lux1

if [[ $lux != "null" ]]; then
	#Use Slope/Intercept
	if [[ -z "$HIVE_LUX_SLOPE" ]] || [[ "$HIVE_LUX_SLOPE" == "0" ]];  then
		HIVE_LUX_SLOPE=1
	fi

	if [[ -z "$HIVE_LUX_INTERCEPT" ]];  then
		HIVE_LUX_INTERCEPT=0
	fi
	lux=`echo "scale=2; (($lux * $HIVE_LUX_SLOPE) + $HIVE_LUX_INTERCEPT)" | bc`
fi
echo $lux



