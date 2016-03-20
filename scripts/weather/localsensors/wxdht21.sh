#!/bin/bash
# read the DHT21
# Version 1
# Supporting the Hivetool project
# gets and returns data like the temperhum



source /home/hivetool2/scripts/hiveconfig.inc
source /home/hivetool2/scripts/data/logger.inc

DATA_GOOD=0
COUNTER=1
while [ $COUNTER -lt 11 ] && [ $DATA_GOOD -eq 0 ]
do
      DATE2=$(TZ=":$TIMEZONE" date '+%F %T')
      #S = Simple
      DHT21=$(nice --20 /usr/local/bin/Seeed_DHT21 $WX_TEMP_GPIO S)
      if [[ -n $DHT21 ]]
      then
        HUMIDITY=$(echo $DHT21 | awk '{print $2}')
        TEMPC=$(echo $DHT21 | awk '{print $1}')
        #Convert C to F
        TEMPF=$(echo "scale=1; (($TEMPC*9)/5)+32" |bc )

        dewpoint_f=$(echo "scale=2; ($TEMPF - ( 9* (100 - $HUMIDITY)) /25)" |bc)
        dewpoint_c=$(echo "scale=1;(($dewpoint_f-32)*5)/9" | bc -l) 
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
  #echo "$DATE2-DHT21-ERROR-Error Reading $HIVEDEVICE" >> $LOG
  loglocal "$DATE2" DHT21 ERROR "Error Reading GPIO $HIVE_TEMP_GPIO"
fi

if test $COUNTER -gt 5
then
  #echo "$DATE2--WARNING-Failed reading GPIO:$HIVE_TEMP_GPIO: retried $COUNTER times" >> $LOG
  loglocal "$DATE2" DHT21 WARNING "Failed reading GPIO $HIVE_TEMP_GPIO: Retried $COUNTER times"
fi

echo $TEMPF $HUMIDITY $dewpoint_f $TEMPC



