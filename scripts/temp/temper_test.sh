#!/bin/bash

# redirect stdout and stderr to logfile
#rm /home/hivetool/hivetool.log
exec >>/home/hivetool/hivetool.log 2>&1

HOST=`hostname`

DATE=`date +"%Y/%m/%d %H:%M"`

   COUNTER=0
   while [[  $COUNTER -lt 10 && $DATA_GOOD -eq 0 ]]; do
         TEMPerHUM=`/usr/local/bin/tempered -s F /dev/hidraw1`
         if [[ -n $TEMPerHUM ]]
         then
           HUMIDITY=`echo $TEMPerHUM | grep  -o "[0-9]*\.[0-9]\%" | grep -o "[0-9]*\.[0-9]"`
           TEMP=`echo $TEMPerHUM | grep  -o "temperature \-*[0-9]*\.[0-9]" | grep -o "\-*[0-9]*\.[0-9]"`

          if [[ -n "$TEMP" && -n "$HUMIDITY" ]]
          # if [[ $TEMP  && $HUMIDITY ]]
           then
            DATA_GOOD=1
           fi
         fi
         let COUNTER=COUNTER+1 
   done
echo $HOST $COUNTER $TEMP $HUMIDITY

if [[ $COUNTER -gt 9 ]]
then
  echo "$DATE ERROR reading /dev/hidraw1" >> error.log
fi

if [[ $COUNTER -lt 10 && $COUNTER -gt 1 ]]
then
  echo "$DATE WARNING reading /dev/hidraw1: retried $COUNTER" >> error.log
fi
