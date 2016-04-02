#!/bin/bash
# version 0.3
# reads the Phidget Bridge board or other scale boards and scales the output
# The code below that checks for difference is just to catch when the numbers go horribly wrong, usually
# because of a sensor error. If it goes that wrong, we want the code to try again until the code is more reasonable
# Need to monitor future data sets when we go into the yard, because unknown how much of a swing will happen when we 
# remove a super.
#echo "Started"
WEIGHTRUNDIR=/home/HiveControl/scripts/weight
# Get some variables from our central file
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc


#echo "Got my source"
COUNTER=0
SUBCOUNTER=0
while [  $COUNTER -eq 0 ]; do
  #echo The counter is $COUNTER
#echo $SCALETYPE

if [ "$SCALETYPE" = "hx711" ]; then
	RAWWEIGHT=`$WEIGHTRUNDIR/hx711.sh`
elif [ "$SCALETYPE" = "phidget1046" ]; then
        RAWWEIGHT=`$WEIGHTRUNDIR/phidget1046.sh`
elif [ "$SCALETYPE" = "cpw200plus" ]; then
	RAWWEIGHT=`$WEIGHTRUNDIR/cpw200plus.sh`
else
	echo "No Scale Type Specified"
	break
fi

#echo "Made it past the if statements"
#echo "Got $RAWWEIGHT"

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

#=========
# Get last known weight
#=========
OLDWEIGHT=`cat  $WEIGHTRUNDIR/lastweight`
#Calc Percent Difference
DIFF=$(echo "scale=2; $RAWWEIGHT-$OLDWEIGHT" | bc)
CHANGE=$(echo "scale=2; $DIFF/$OLDWEIGHT" |bc)
# Set allowed difference
ALLOWDIFF="-.25"

if [[ $( echo "$CHANGE < $ALLOWDIFF" |bc) -eq 1 ]]; then 
		loglocal "$DATE" WEIGHT WARNING "PERCENTAGE OF CHANGE EXCEEDS $ALLOWDIFF"
        
	sleep 30s
	let SUBCOUNTER=SUBCOUNTER+1
	#echo SUB is $SUBCOUNTER
	if [[  $SUBCOUNTER -gt "5" ]]; then
		loglocal "$DATE" WEIGHT ERROR "Exceeded Retries to get a good weight"
	RAWWEIGHT="$OLDWEIGHT"
	let COUNTER=COUNTER+1			
	fi
else
# Success - Let's get the other data
#================
# Calc Hive Weight - Equipment Weight
#================
#echo "Starting weight dance"
BASEWEIGHT=`echo "($HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT * $NUM_HIVE_BASE_SOLID_BOTTOM_BOARD) + ($HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT * $NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD) + ($HIVE_FEEDER_WEIGHT * $NUM_HIVE_FEEDER) + ($HIVE_TOP_INNER_COVER_WEIGHT * $NUM_HIVE_TOP_INNER_COVER) + ($HIVE_TOP_TELE_COVER_WEIGHT * $NUM_HIVE_TOP_TELE_COVER) + ($HIVE_TOP_MIGRATORY_COVER_WEIGHT * $NUM_HIVE_TOP_MIGRATORY_COVER)" |bc`
#echo "Base = $BASEWEIGHT"
BODYWEIGHT=`echo "($HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT * $NUM_HIVE_BODY_MEDIUM_FOUNDATION) + ($HIVE_BODY_MEDIUM_FOUNDATION_LESS_WEIGHT * $NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS) + ($HIVE_BODY_DEEP_FOUNDATION_WEIGHT * $NUM_HIVE_BODY_DEEP_FOUNDATION) + ($HIVE_BODY_DEEP_FOUNDATION_LESS_WEIGHT * $NUM_HIVE_BODY_DEEP_FOUNDATION_LESS) + ($HIVE_BODY_SHAL_FOUNDATION_WEIGHT * $NUM_HIVE_BODY_SHAL_FOUNDATION) + ($HIVE_BODY_SHAL_FOUNDATION_LESS_WEIGHT * $NUM_HIVE_BODY_SHAL_FOUNDATION_LESS)" |bc`
#echo "Body = $BODYWEIGHT"
MISCWEIGHT=`echo "$HIVE_TOP_WEIGHT + $HIVE_COMPUTER_WEIGHT + $HIVE_MISC_WEIGHT" |bc`
#echo "MISC = $MISCWEIGHT"
COMBINEDWEIGHT=`echo "$BASEWEIGHT + $BODYWEIGHT + $MISCWEIGHT" |bc`
#echo "Combined = $COMBINEDWEIGHT"
HIVEWEIGHT=`echo "$RAWWEIGHT - $COMBINEDWEIGHT" |bc`
#echo "HIVEWEIGHT $HIVEWEIGHT"
# End WHILE loop, if Successfull
let COUNTER=COUNTER+1
printf "$RAWWEIGHT" > $WEIGHTRUNDIR/lastweight 

#echo "Made it here"
# Clean up decimal points

RAW2=$(echo "scale=2; ($RAWWEIGHT/1)" |bc)

echo "$RAW2 $HIVEWEIGHT"
#echo "$DATE - WEIGHT - SUCCESS: $INPUT_0,$INPUT_1,$INPUT_2,$INPUT_3,$RAWWEIGHT,$HIVEWEIGHT" >>  $LOG
fi
done

