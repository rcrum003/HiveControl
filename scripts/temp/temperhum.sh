#!/bin/bash
# read the TEMPerHUM

while getopts d: option
do
        case "${option}"
        in
                d) DEVICE=${OPTARG};;
        esac
done

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
        TEMP=`echo $TEMPerHUM | grep  -o "temperature \-*[0-9]*\.[0-9]" | grep -o "\-*[0-9]*\.[0-9]"`
        DEW=`echo $TEMPerHUM |grep -o "point \-*[0-9]*\.[0-9]" | grep -o "\-*[0-9]*\.[0-9]"` 
	HUMIDITY_TEST=`echo "$HUMIDITY < 0" | bc`
        TEMP_TEST=`echo "$TEMP > -50" | bc`
        if [ $HUMIDITY_TEST -eq 0 ] && [ $TEMP_TEST -ne 0 ]
        then
         DATA_GOOD=1
        else
         HUMIDITY=""
         TMEP=""
        fi
      fi
      let "COUNTER += 1"
      sleep $COUNTER
done
#echo $COUNTER $TEMP $HUMIDITY

if [[ $COUNTER -gt 11 ]]
then
  echo "$DATE2-TEMPER-ERROR-Error Reading $HIVEDEVICE" >> $LOG
  loglocal "$DATE2" TEMPER ERROR "Error Reading $HIVEDEVICE"
fi

if test $COUNTER -gt 5
then
  echo "$DATE2-TEMPER-WARNING-Failed reading $HIVEDEVICE: retried $COUNTER times" >> $LOG
  loglocal "$DATE2" TEMPER WARNING "Failed reading $HIVEDEVICE: Retried $COUNTER times"
fi

echo $TEMP $HUMIDITY $DEW



