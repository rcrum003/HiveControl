#!/bin/bash
#
# read the scale
# 
#
# v3 patch 
# Changed Count to RAW for better readability
# Added a check to make sure RAW is a number to assist setups that don't always return a number
# In this case, every couple of times, we were getting "No data to consider"
# Also, commented out the Warning

source /home/HiveControl/scripts/hiveconfig.inc

HX711_ZERO=$HIVE_WEIGHT_INTERCEPT
HX711_CALI=$HIVE_WEIGHT_SLOPE
#HX711_ZERO=37800
#HX711_CALI=9928.50
#
# read the scale
DATA_GOOD=0
COUNTER=1
while [ $COUNTER -lt 5 ] && [ $DATA_GOOD -eq 0 ]; do

RAW=`/usr/bin/timeout 5 /usr/bin/sudo /usr/local/bin/hx711 $HX711_ZERO`
#echo "$RAW" >> successrate	

# Check to see if Raw is a number, if not, don't do this
if [[ $RAW =~ ^-?[0-9]+$ ]] 
then	
	#echo "Passed Test"
	WEIGHT=`echo "scale=2; ($RAW/$HX711_CALI)" | bc`
        if [ $WEIGHT ]
        then
         DATA_GOOD=1
        fi
fi
      let "COUNTER += 1"
done
#echo $COUNTER $RAW $WEIGHT

if [ $COUNTER -gt 10 ]
then
  echo "$DATE ERROR reading Scale $DEVICE"
  SCALE=-555
fi
#if test $COUNTER -gt 2
#then
#  echo "$DATE WARNING reading Scale /dev/ttyS0: retried $COUNTER"
#fi

echo "$WEIGHT"

