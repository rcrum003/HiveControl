#!/bin/bash
# read the TEMPerHUM
# Supporting Hivetool.org project
# Version 1.2
# 

source /home/hivetool2/scripts/hiveconfig.inc
source /home/hivetool2/scripts/data/logger.inc

#echo "DEVICE = $DEVICE"

DATA_GOOD=0
COUNTER=1
while [ $COUNTER -lt 11 ] && [ $DATA_GOOD -eq 0 ]
do
      DATE2=$(TZ=":$TIMEZONE" date '+%F %T')
      TEMPerHUM=`/usr/bin/timeout 5 /usr/local/bin/tempered -s F $HIVEDEVICE`
      #echo -ne "$DATE2-TEMPER-DEBUG:  $COUNTER $? $TEMPerHUM \n" >> $LOG
      if [[ -n $TEMPerHUM ]]
      then
        HUMIDITY=`echo $TEMPerHUM | grep  -o "\-*[0-9]*\.[0-9]\%" | grep -o "\-*[0-9]*\.[0-9]"`
        TEMPF=`echo $TEMPerHUM | grep  -o "temperature \-*[0-9]*\.[0-9]" | grep -o "\-*[0-9]*\.[0-9]"`
        TEMPC=$(echo "scale=1; (($TEMPF-32)*5)/9" |bc)
        DEW=`echo $TEMPerHUM |grep -o "point \-*[0-9]*\.[0-9]" | grep -o "\-*[0-9]*\.[0-9]"` 
        HUMIDITY_MINTEST=`echo "$HUMIDITY < 0" | bc`
        TEMP_MINTEST=`echo "$TEMPF > -50" | bc`
        HUMIDITY_MAXTEST=`echo "$HUMIDITY > 100" | bc`
        TEMP_MAXTEST=`echo "$TEMPF > 150" | bc`
        if [ $HUMIDITY_MINTEST -eq 0 ] && [ $TEMP_MINTEST -ne 0 ] && [ $HUMIDITY_MAXTEST -eq 0 ] && [ $TEMP_MAXTEST -eq 0 ]
        then
         DATA_GOOD=1
        else
         HUMIDITY="0"
         TEMPF="0"
         TEMPC="0"
         dewpoint_f="0"
        fi
      fi
      let "COUNTER += 1"
      sleep $COUNTER
done
#echo $COUNTER $TEMP $HUMIDITY

if [[ $COUNTER -gt 11 ]]
then
  #echo "$DATE2-TEMPER-ERROR-Error Reading $HIVEDEVICE" >> $LOG
  loglocal "$DATE2" TEMPER ERROR "Error Reading $HIVEDEVICE"
fi

if test $COUNTER -gt 5
then
  #echo "$DATE2-TEMPER-WARNING-Failed reading $HIVEDEVICE: retried $COUNTER times" >> $LOG
  loglocal "$DATE2" TEMPER WARNING "Failed reading $HIVEDEVICE: Retried $COUNTER times"
fi

echo $TEMPF $HUMIDITY $DEW $TEMPC



