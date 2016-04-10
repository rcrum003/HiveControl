#!/bin/bash
# read the TEMPerHUM
# Author: Ryan Crum
# Date: 4-9-2016
# Supporting Hivetool.org project
# Version 2
# Had to rewrite this, the while loop was useless, and too many times when no value was returned

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc


function return_value {
#Returns proper values

TEMPerHUM=$1
DATE2=$2
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
         echo $TEMPF $HUMIDITY $DEW $TEMPC
         exit
        else
           loglocal "$DATE2" TEMP ERROR "Readings Exceeded Limits: TEMP=$TEMPF, HUMDITY=$HUMIDITY"
         HUMIDITY="0"
         TEMPF="0"
         TEMPC="0"
         DEW="0"
          echo $TEMPF $HUMIDITY $DEW $TEMPC
         exit
        fi
      fi
}

DATE2=$(TZ=":$TIMEZONE" date '+%F %T')
TEMPerHUM=$(/usr/bin/timeout 5 /usr/local/bin/tempered -s F $HIVEDEVICE 2>&1)

      if [[ "$TEMPerHUM" == *"device not found"* ]]
        then
         devicesavail=$(/usr/local/bin/hid-query -e |grep -v "interface 0" |awk '{print $1}')
          #My temperhum has a bad habit of changing hidraw1 for some reason
          #if we only have one device, then we'll try to read that one as well
          devicecount=$(/usr/local/bin/hid-query -e | grep -v "interface 0" | wc | awk '{print $1}')
          if [[ $devicecount -eq "1" ]] 
            then
            TEMPerHUM=$(/usr/bin/timeout 5 /usr/local/bin/tempered -s F $devicesavail 2>&1)
            loglocal "$DATE2" TEMP WARNING "$HIVEDEVICE not found, we used $devicesavail instead"
            return_value "$TEMPerHUM" "$DATE2"
            exit
          fi
          if [[ $devicesavail == $HIVEDEVICE ]] 
           then
            loglocal "$DATE2" TEMP ERROR "$HIVEDEVICE not found, driver error of unknown kind, since you have the device configured properly"
           else 
            loglocal "$DATE2" TEMP ERROR "$HIVEDEVICE not found, looks like your device is $devicesavail"
           fi 
         HUMIDITY="0"
         TEMPF="0"
         TEMPC="0"
         dewpoint_f="0"
         echo $TEMPF $HUMIDITY $DEW $TEMPC
         exit
        fi
      if [[ $TEMPerHUM == *"devices were found"* ]]
        then
          loglocal "$DATE2" TEMP ERROR "No Devices Found - Check to see if plugged in. Run hid-query -e"
         HUMIDITY="0"
         TEMPF="0"
         TEMPC="0"
         dewpoint_f="0"
         echo $TEMPF $HUMIDITY $DEW $TEMPC
         exit
      fi

# If you made it through all that, then let's send out value to an output function defined at the top
return_value "$TEMPerHUM" "$DATE2"



