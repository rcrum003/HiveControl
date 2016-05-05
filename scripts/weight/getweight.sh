#!/bin/bash
# version 0.8
# reads the Weight boards or other scale boards and scales the output
# Updated to remove the percentage check, since that seems to cause a lot of bad values
# Now we just check for expected weights within range between 0 and 1500 lbs.
# Added logic to check to see if we have set our intercepts.

WEIGHTRUNDIR=/home/HiveControl/scripts/weight
# Get some variables from our central file
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

if [[ -z "$HIVE_WEIGHT_INTERCEPT" ]] ; then
	#echo "Weight Intercept Not Set"
	loglocal "$DATE" WEIGHT ERROR "Please set Weight Zero Value in the Instruments page"
	HIVE_WEIGHT_INTERCEPT=1	
fi
if [[ -z $HIVE_WEIGHT_SLOPE ]] || [[ $HIVE_WEIGHT_INTERCEPT = "0" ]]; then
	#echo "Weight Slope Not Set"
	loglocal "$DATE" WEIGHT ERROR "Please set Weight Slope in the Instruments page"
	HIVE_WEIGHT_SLOPE=1
fi


if [ "$SCALETYPE" = "hx711" ]; then
	#echo "getting scale values"
	HX711_ZERO="$HIVE_WEIGHT_INTERCEPT"	
	HX711_CALI="$HIVE_WEIGHT_SLOPE"
	#echo "Passing Zero = $HX711_ZERO, CALI = $HX711_CALI"
	RAWWEIGHT=$($WEIGHTRUNDIR/hx711.sh $HX711_ZERO $HX711_CALI)
	#echo "Got RAWWEIGHT as $RAWWEIGHT"
elif [ "$SCALETYPE" = "phidget1046" ]; then
        RAWWEIGHT=`$WEIGHTRUNDIR/phidget1046.sh`
elif [ "$SCALETYPE" = "cpw200plus" ]; then
	RAWWEIGHT=`$WEIGHTRUNDIR/cpw200plus.sh`
else
	loglocal "$DATE" WEIGHT ERROR "No scale type specified"
	echo "0 0"
	exit
fi

# Test our values for expected range

	RAW_MINTEST=$(echo "$RAWWEIGHT < 0" | bc)
    RAW_MAXTEST=$(echo "$RAWWEIGHT > 1500" | bc)

if [[ $RAW_MINTEST == "0" ]] && [[ $RAW_MAXTEST == "0" ]]; then 
	# Success - Let's get the other data
	#================
	# Calc Hive Weight - Equipment Weight
	#================
	BASEWEIGHT=`echo "($HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT * $NUM_HIVE_BASE_SOLID_BOTTOM_BOARD) + ($HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT * $NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD) + ($HIVE_FEEDER_WEIGHT * $NUM_HIVE_FEEDER) + ($HIVE_TOP_INNER_COVER_WEIGHT * $NUM_HIVE_TOP_INNER_COVER) + ($HIVE_TOP_TELE_COVER_WEIGHT * $NUM_HIVE_TOP_TELE_COVER) + ($HIVE_TOP_MIGRATORY_COVER_WEIGHT * $NUM_HIVE_TOP_MIGRATORY_COVER)" |bc`
	BODYWEIGHT=`echo "($HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT * $NUM_HIVE_BODY_MEDIUM_FOUNDATION) + ($HIVE_BODY_MEDIUM_FOUNDATION_LESS_WEIGHT * $NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS) + ($HIVE_BODY_DEEP_FOUNDATION_WEIGHT * $NUM_HIVE_BODY_DEEP_FOUNDATION) + ($HIVE_BODY_DEEP_FOUNDATION_LESS_WEIGHT * $NUM_HIVE_BODY_DEEP_FOUNDATION_LESS) + ($HIVE_BODY_SHAL_FOUNDATION_WEIGHT * $NUM_HIVE_BODY_SHAL_FOUNDATION) + ($HIVE_BODY_SHAL_FOUNDATION_LESS_WEIGHT * $NUM_HIVE_BODY_SHAL_FOUNDATION_LESS)" |bc`
	MISCWEIGHT=`echo "$HIVE_TOP_WEIGHT + $HIVE_COMPUTER_WEIGHT + $HIVE_MISC_WEIGHT" |bc`
	COMBINEDWEIGHT=`echo "$BASEWEIGHT + $BODYWEIGHT + $MISCWEIGHT" |bc`
	HIVEWEIGHT=`echo "$RAWWEIGHT - $COMBINEDWEIGHT" |bc`
	RAW2=$(echo "scale=2; ($RAWWEIGHT/1)" |bc)
else
	#Something bad happened, set values to zero and exit
        loglocal "$DATE" WEIGHT ERROR "Raw weight was $RAWWEIGHT which exceeded 1500 or was less than 0, setting value to 0"	
         RAW2="0"
         HIVEWEIGHT="0"
fi

if [[ -z "$RAW2" ]] || [[ -z "$HIVEWEIGHT" ]];  then
	 loglocal "$DATE" WEIGHT ERROR "Unknown Error, raw2 was $RAW2 and hiveweight was $HIVEWEIGHT"	
	 #echo "unknown error, raw2 was $RAW2 and hiveweight was $HIVEWEIGHT"
	echo "0 0"
	exit
else
echo "$RAW2 $HIVEWEIGHT"
fi

