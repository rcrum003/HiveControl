#!/bin/bash
# read the TSL2561 
# Version 1
# For Hivetool command set

# We don't actually have this version, but this is a placeholder if you do.
# Let us know, and we'll add support. 

# Source variables
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE2=$(TZ=":$TIMEZONE" date '+%F %T')

while [[ $COUNTER -lt 5 ]] && [[ $DATA_GOOD -eq 0 ]]
do
     
      TSL2561=$(/usr/bin/timeout 5 sudo /usr/local/bin/2561)
      #echo -ne "$DATE2 $COUNTER $? $TSL2561 \n" >> $LOG
      if [[ -n $TSL2561 ]]; 
      then
        LUX=$(echo $TSL2561 |awk '{print $4}')
        if [ $LUX -gt -1 ] && [ $LUX -lt 120000 ]
        then
         DATA_GOOD=1
        else
         LUX="null"
        fi
      fi
      let "COUNTER += 1"
      sleep $COUNTER
done

if [[ $COUNTER -gt 5 ]]
then
  loglocal "$DATE2" LIGHT ERROR "Error Reading TSL2561, $TSL2561" 
  LUX="null"
fi

echo $LUX


