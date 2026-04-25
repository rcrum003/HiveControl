#!/bin/bash
# read the Adafruit SHT41 Trinkey Temp/Humidity Sensor (USB)
# Version 1.0
# Supporting the HiveControl project
# gets and returns data like the temperhum

# Requires that the python script SHT41_Trinkey.py is in /home/HiveControl/software/sht41_trinkey

#Get our standard includes
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc


function return_error {
    #Function to return an error value
         loglocal "$DATE2" TEMP ERROR "$message"
         HUMIDITY="null"
         TEMPF="null"
         TEMPC="null"
         dewpoint_f="null"
         echo $TEMPF $HUMIDITY $dewpoint_f $TEMPC
}

DATA_GOOD=0
COUNTER=1


############################################################
#Read the sensor
# Try 3 times for a good reading
############################################################
while [ $COUNTER -lt 3 ] && [ $DATA_GOOD -eq 0 ]
do
        DATE2=$(TZ=":$TIMEZONE" date '+%F %T')

        sht41_out=$(sudo python3 /home/HiveControl/software/sht41_trinkey/SHT41_Trinkey.py 2>&1)

        sht41STATUS=$(echo $sht41_out |awk '{print $1}')

        # Check that we got numeric output (not a traceback or error message)
        if [[ "$sht41STATUS" == "Error" ]] || [[ "$sht41STATUS" == "Traceback" ]] || [[ -z "$sht41STATUS" ]] || ! echo "$sht41STATUS" | grep -qE '^-?[0-9]'; then

            message="$sht41_out"
            DATA_GOOD=0

          else

                  TEMPF_RAW=$(echo $sht41_out | awk '{print $1}')
                  TEMPC_RAW=$(echo $sht41_out | awk '{print $2}')
                  HUMIDITY=$(echo $sht41_out | awk '{print $3}')

                  TEMPF=$(echo "scale=1; ($TEMPF_RAW/1)" |bc)
                  TEMPC=$(echo "scale=1; ($TEMPC_RAW/1)" |bc)

                  #Calculate dewpoint
                  dewpoint_f=$(echo "scale=2; ($TEMPF - ( 9* (100 - $HUMIDITY)) /25)" |bc)

                  #Test our values to see if they were actually any good.
                  HUMIDITY_MINTEST=$(echo "$HUMIDITY < 0" | bc)
                  TEMP_MINTEST=$(echo "$TEMPF > -50" | bc)
                  HUMIDITY_MAXTEST=$(echo "$HUMIDITY > 101" | bc)
                  TEMP_MAXTEST=$(echo "$TEMPF > 150" | bc)
                  if [ "$HUMIDITY_MINTEST" == "0" ] && [ "$TEMP_MINTEST" != "0" ] && [ "$HUMIDITY_MAXTEST" == "0" ] && [ "$TEMP_MAXTEST" == "0" ]
                    then
                   echo $TEMPF $HUMIDITY $dewpoint_f $TEMPC
                  else
                      message="ERROR SHT41 Trinkey readings failed validation"
                      return_error
                      exit
                  fi
                DATA_GOOD=1

          fi

      let "COUNTER += 1"
      sleep $COUNTER
done

if [[ $COUNTER -gt "2" ]]; then
  return_error
fi
