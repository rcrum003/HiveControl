#!/bin/bash
# read the TEMPer
# Note: this is just for a TEMPer device without a humidity sensor, not recommended, but some of us have these sitting around
# Author: Ryan Crum
# Date: 1-9-2017
# HiveControl
# Supporting Hivetool.org project
# Version 2.3
# Had to rewrite this, the while loop was useless, and too many times when no value was returned

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

#HiveDevice passed to script as $1
#TEMP_SUB2 passed to script as $2, this is because the temper can have an external source.

HIVEDEVICE="$1"
TEMP_SUB2="$2"

DATE2=$(TZ=":$TIMEZONE" date '+%F %T')

function return_value {
#Returns proper values by removing values that aren't numbers and such

TEMPerHUM=$1
      if [[ -n $TEMPerHUM ]]
      then
        TEMPF=`echo $TEMPerHUM | grep  -o "temperature \-*[0-9]*\.[0-9]" | grep -o "\-*[0-9]*\.[0-9]"`
        TEMPC=$(echo "scale=2; (($TEMPF-32)*5)/9" |bc)
        TEMP_MINTEST=`echo "$TEMPF > -50" | bc`
        TEMP_MAXTEST=`echo "$TEMPF > 150" | bc`
        #HUMIDITY=`echo $TEMPerHUM | grep  -o "\-*[0-9]*\.[0-9]\%" | grep -o "\-*[0-9]*\.[0-9]"`
        #DEW=`echo $TEMPerHUM |grep -o "point \-*[0-9]*\.[0-9]" | grep -o "\-*[0-9]*\.[0-9]"` 
        #HUMIDITY_MINTEST=`echo "$HUMIDITY < 0" | bc`
        #HUMIDITY_MAXTEST=`echo "$HUMIDITY > 100" | bc`
        if [[ $TEMP_MINTEST -ne 0 ]] && [[ $TEMP_MAXTEST -eq 0 ]]
        then
         echo $TEMPF null null $TEMPC
         exit
           #statements  
        else
           loglocal "$DATE2" TEMP ERROR "Readings Exceeded Limits: TEMP=$TEMPF"
           loglocal "$DATE2" TEMP ERROR "$TEMPerHUM"
          echo "null null null null"
         exit
        fi
      fi
}



##############
# Set the parameters for what we want to read
# If a subdevice is set, read the one specified
# If not specified, then read just the main device
###############
function read_sensor {
    if [[ -n $TEMP_SUB2 ]] 
      then 
      #Temp Sub Set, so check the sub they want
      TEMPerHUM=$(/usr/bin/timeout 5 /usr/local/bin/tempered -s F $HIVEDEVICE |grep $TEMP_SUB2: 2>&1)
    else
      #Didn't set a temp_sub so, must only be one
      TEMPerHUM=$(/usr/bin/timeout 5 /usr/local/bin/tempered -s F $HIVEDEVICE 2>&1)
    fi
}
##################################
# Attempt to read the device set,
# Keep running until we get a value of some sort
##################################
  read_sensor
    if [[ "$TEMPerHUM" == "*Could not open device*" ]]
      then
        loglocal "$DATE2" TEMP WARNING "$HIVEDEVICE: Could not open device, trying again in 5 seconds"
        sleep 5 #Sleep for 5 seconds before we try again
        read_sensor 
        if [[ "$TEMPerHUM" == *"Could not open device"* ]]
          then
            #We got an error, so return that error, then exit
            loglocal "$DATE2" TEMP ERROR "$HIVEDEVICE: Could not open device after waiting 5 seconds"
          echo "null null null null"
          exit
        fi       
    fi
#####################################################################
# Try to see if the wrong device was specified, try to find the right one
#
########################################################################
      if [[ "$TEMPerHUM" == "*device not found*" ]]
        then
         devicesavail=$(/usr/local/bin/hid-query -e |grep -v "interface 1" |awk '{print $1}')
          #My temperhum has a bad habit of changing hidraw1 for some reason
          #if we only have one device, then we'll try to read that one as well
          devicecount=$(/usr/local/bin/hid-query -e | grep -v "interface 1" | wc | awk '{print $1}')
          
          if [[ $devicecount -eq "1" ]] 
            then
            #We only had one device on Interface 1, so let's try to use that one.
            loglocal "$DATE2" TEMP WARNING "$HIVEDEVICE not found, we used $devicesavail instead"
            HIVEDEVICE=$devicesavail
            read_sensor
            return_value "$TEMPerHUM" "$DATE2"
            exit
           elif [[ $devicecount -gt "1" ]]; then
             #Looks like we have multiple devices, good for you money bags, but we need to tell the user to pick the right one
             loglocal "$DATE2" TEMP ERROR "Multiple Temper Devices found, please select the right one - $devicesavail"
             echo "null null null null"
             exit
          fi

          if [[ $devicesavail == $HIVEDEVICE ]] 
           then
            loglocal "$DATE2" TEMP ERROR "$HIVEDEVICE not found, driver error of unknown kind, since you have the device configured properly"
           else 
            loglocal "$DATE2" TEMP ERROR "$HIVEDEVICE not found, looks like your device is $devicesavail"
           fi 
        echo "null null null null"
        exit
      fi
      if [[ "$TEMPerHUM" == *"devices were found"* ]]
        then
          loglocal "$DATE2" TEMP ERROR "No Devices Found - Check to see if plugged in. Run hid-query -e"
         echo "null null null null"
         exit
      fi

# If you made it through all that, then let's send out value to an output function defined at the top
return_value "$TEMPerHUM" "$DATE2"



