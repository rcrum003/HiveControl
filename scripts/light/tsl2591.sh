#!/bin/bash
# read the TSL2591 
# Version 2
# For Hivetool command set
# This is the version used for the hivecontroller base
# Requires 2591 code to be located in /usr/local/bin/2591
 

# Source variables
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc


DATA_GOOD=0
COUNTER=1

while [ $COUNTER -lt 11 ] && [ $DATA_GOOD -eq 0 ]
do
      DATE2=$(TZ=":$TIMEZONE" date '+%F %T')
      TSL2591=`/usr/bin/timeout 5 sudo /usr/local/bin/2591`
      #echo -ne "$DATE2 $COUNTER $? $TSL2591 \n" >> $LOG
      if [[ -n $TSL2591 ]]
      then
        LUX=`echo $TSL2591 |grep Lux | grep -o "[0-9]*$"`
        if [ $LUX -gt -1 ] && [ $LUX -lt 120000 ]
        then
         DATA_GOOD=1
        else
         LUX="0"
        fi
      fi
      let "COUNTER += 1"
      sleep $COUNTER
done
#echo $COUNTER $TEMP $HUMIDITY

if [[ $COUNTER -gt 11 ]]
then
  loglocal "$DATE2" LIGHT ERROR "Error Reading $DEVICE" 
  LUX="0"
fi

if test $COUNTER -gt 2
then
  loglocal "$DATE2" LIGHT WARNING "Failed reading TSL2591 - Retried $COUNTER times"
  LUX="0"
fi

echo $LUX



