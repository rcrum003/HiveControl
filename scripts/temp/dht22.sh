#!/bin/bash
# read the DHT22
# Version 1.2
# Supporting the Hivetool project
# gets and returns data like the temperhum



source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATA_GOOD=0
COUNTER=1
while [ $COUNTER -lt 11 ] && [ $DATA_GOOD -eq 0 ]
do
      DATE2=$(TZ=":$TIMEZONE" date '+%F %T')
      #S = Simple
      DHT22=$(nice --20 /usr/local/bin/Seeed_DHT22 $HIVE_TEMP_GPIO S)
      if [[ -n $DHT22 ]]
      then
        HUMIDITY=$(echo $DHT22 | awk '{print $2}')
        TEMPC=$(echo $DHT22 | awk '{print $1}')
        #Convert C to F
        TEMPF=$(echo "scale=1; (($TEMPC*9)/5)+32" |bc )
        dewpoint_f=$(echo "scale=2; ($TEMPF - ( 9* (100 - $HUMIDITY)) /25)" |bc)
        dewpoint_c=$(echo "scale=1;(($dewpoint_f-32)*5)/9" | bc -l) 
        HUMIDITY_MINTEST=`echo "$HUMIDITY < 0" | bc`
        TEMP_MINTEST=`echo "$TEMPF > -50" | bc`
        HUMIDITY_MAXTEST=`echo "$HUMIDITY > 100" | bc`
        TEMP_MAXTEST=`echo "$TEMPF > 150" | bc`
        if [ $HUMIDITY_MINTEST == "0" ] && [ $TEMP_MINTEST != "0" ] && [ $HUMIDITY_MAXTEST == "0" ] && [ $TEMP_MAXTEST == "0" ]
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
  #echo "$DATE2-DHT22-ERROR-Error Reading $HIVEDEVICE" >> $LOG
  loglocal "$DATE2" TEMP ERROR "DHT22 Error Reading GPIO $HIVE_TEMP_GPIO"
         HUMIDITY="0"
         TEMPF="0"
         TEMPC="0"
         dewpoint_f="0"
fi

if test $COUNTER -gt 5
then
  #echo "$DATE2--WARNING-Failed reading GPIO:$HIVE_TEMP_GPIO: retried $COUNTER times" >> $LOG
  loglocal "$DATE2" TEMP WARNING "DHT22 Failed reading GPIO $HIVE_TEMP_GPIO: Retried $COUNTER times"
        HUMIDITY="0"
        TEMPF="0"
        TEMPC="0"
        dewpoint_f="0"
fi

echo $TEMPF $HUMIDITY $dewpoint_f $TEMPC



