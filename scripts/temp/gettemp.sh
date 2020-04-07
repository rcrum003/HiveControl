#!/bin/bash
# version 1.34 2020-03-31
# reads the temp sensors as one script


# Get some variables from our central file
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

		# Data Fetchers/Parsers in one
		if [ $TEMPTYPE = "temperhum" ]; then
			GETTEMP=`$HOMEDIR/scripts/temp/temperhum.sh $HIVEDEVICE`
		elif [[ $TEMPTYPE = "temper" ]]; then
			GETTEMP=`$HOMEDIR/scripts/temp/temper.sh $HIVEDEVICE $HIVE_TEMP_SUB`
		elif [[ $TEMPTYPE = "dht22" ]]; then
			GETTEMP=`$HOMEDIR/scripts/temp/dht22.sh`
		elif [[ $TEMPTYPE = "dht21" ]]; then
			GETTEMP=`$HOMEDIR/scripts/temp/dht21.sh`
		elif [[ $TEMPTYPE = "sht31d" ]]; then
			GETTEMP=$($HOMEDIR/scripts/temp/sht31-d.sh)
		elif [[ $TEMPTYPE = "bme680" ]]; then
			GETTEMP=$($HOMEDIR/scripts/temp/bme680.sh)
		elif [[ $TEMPTYPE = "bme280" ]]; then
			GETTEMP=$($HOMEDIR/scripts/temp/bme280.sh)
		elif [[ $TEMPTYPE = "broodminder" ]]; then
			GETTEMP=$($HOMEDIR/scripts/temp/broodminder.sh $HIVEDEVICE)
		fi

		HIVETEMPF=$(echo $GETTEMP |awk '{print $1}')
		HIVEHUMIDITY=$(echo $GETTEMP |awk '{print $2}')
		HIVEDEW=$(echo $GETTEMP |awk '{print $3}')
		HIVETEMPC=$(echo $GETTEMP |awk '{print $4}')

		#echo "RAW - TEMPF=$HIVETEMPF, TEMPC=$HIVETEMPC, HUMI=$HIVEHUMIDITY"
		#Use TEMP Slope/Intercept
		
	if [[ $HIVETEMPF != "null" ]]; then
		if [[ -z "$HIVE_TEMP_SLOPE" ]] || [[ "$HIVE_TEMP_SLOPE" == "0" ]];  then
			HIVE_TEMP_SLOPE=1
		fi

		if [[ -z "$HIVE_TEMP_INTERCEPT" ]];  then
			HIVE_TEMP_INTERCEPT=0
		fi

		#echo "			Applying SLOPE/INTERCEPT, if  set"
		if [[ $HIVETEMPF == "0" ]]; then
			#Because if the temp is actually zero, it'll screw up this stupid slope/intercept deal
			HIVETEMPF="0.01"
		fi
		
		if [[ $HIVEHUMIDITY == "0" ]]; then
			#Because if the Humidity is actually zero, it'll screw up this stupid slope/intercept deal
			HIVEHUMIDITY="0.01"
		fi
		HIVETEMPF=$(echo "scale=2; (($HIVETEMPF * $HIVE_TEMP_SLOPE) + $HIVE_TEMP_INTERCEPT)" | bc)
		HIVETEMPC=$(echo "scale=2; (($HIVETEMPF - 32) * (5/9))" | bc )

		#Use Humidity Slope/Intercept
		if [[ -z "$HIVE_HUMIDITY_SLOPE" ]] || [[ "$HIVE_HUMIDITY_SLOPE" == "0" ]];  then
			HIVE_HUMIDITY_SLOPE=1
		fi
		
		if [[ -z "$HIVE_HUMIDITY_INTERCEPT" ]];  then
			HIVE_HUMIDITY_INTERCEPT=0
		fi
		HIVEHUMIDITY=$(echo "scale=2; (($HIVEHUMIDITY * $HIVE_HUMIDITY_SLOPE) + $HIVE_HUMIDITY_INTERCEPT)" | bc)
		#echo "Calculated: TEMPF=$HIVETEMPF, TEMPC=$HIVETEMPC, HUMI=$HIVEHUMIDITY"
	fi
echo "$HIVETEMPF $HIVEHUMIDITY $HIVEDEW $HIVETEMPC"


