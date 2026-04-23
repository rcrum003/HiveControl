#!/bin/bash
# read the TSL2561 light sensor
# For HiveControl command set
# Output: integer lux value, or "null" on failure
# Binary output format: "rc, broadband, ir, lux"

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATA_GOOD=0
COUNTER=1
LUX="null"

DATE2=$(TZ=":$TIMEZONE" date '+%F %T')

while [ $COUNTER -lt 5 ] && [ $DATA_GOOD -eq 0 ]
do
      TSL2561=$(/usr/bin/timeout 5 sudo /usr/local/bin/2561 2>/dev/null)
      RC=$?
      if [ $RC -eq 0 ] && [[ -n "$TSL2561" ]]; then
        LUX=$(echo "$TSL2561" | awk -F', ' '{print $4}')
        if [[ "$LUX" =~ ^[0-9]+$ ]] && [ "$LUX" -ge 0 ] && [ "$LUX" -lt 120000 ]; then
         DATA_GOOD=1
        else
         LUX="null"
        fi
      fi
      let "COUNTER += 1"
      sleep $COUNTER
done

if [ $DATA_GOOD -eq 0 ]; then
  loglocal "$DATE2" LIGHT ERROR "Error Reading TSL2561 after $((COUNTER-1)) attempts"
  LUX="null"
fi

echo $LUX


