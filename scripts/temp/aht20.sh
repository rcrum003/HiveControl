#!/bin/bash
# read the Adafruit AHT20 Temp/Humidity Sensor
# Version 1.0
# Supporting the HiveControl project
# gets and returns data like the temperhum

# Requires that the python script AHT20.py is in /home/HiveControl/software/aht20

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

        aht20_out=$(sudo python3 /home/HiveControl/software/aht20/AHT20.py 2>&1)

        aht20STATUS=$(echo $aht20_out |awk '{print $1}')

        if [[ "$aht20STATUS" != "Error" ]]; then

                  TEMPF_RAW=$(echo $aht20_out | awk '{print $1}')
                  TEMPC_RAW=$(echo $aht20_out | awk '{print $2}')
                  HUMIDITY=$(echo $aht20_out | awk '{print $3}')

                  TEMPF=$(echo "scale=1; ($TEMPF_RAW/1)" |bc)
                  TEMPC=$(echo "scale=1; ($TEMPC_RAW/1)" |bc)

                  #Calculate dewpoint
                  dewpoint_f=$(echo "scale=2; ($TEMPF - ( 9* (100 - $HUMIDITY)) /25)" |bc)

                  #Test our values to see if they were actually any good.
                  HUMIDITY_MINTEST=$(echo "$HUMIDITY < 0" | bc)
                  TEMP_MINTEST=$(echo "$TEMPF > -50" | bc)
                  HUMIDITY_MAXTEST=$(echo "$HUMIDITY > 101" | bc)
                  TEMP_MAXTEST=$(echo "$TEMPF > 150" | bc)
                  if [ $HUMIDITY_MINTEST == "0" ] && [ $TEMP_MINTEST != "0" ] && [ $HUMIDITY_MAXTEST == "0" ] && [ $TEMP_MAXTEST == "0" ]
                    then
                   echo $TEMPF $HUMIDITY $dewpoint_f $TEMPC
                  else
                      message="ERROR AHT20 readings failed validation"
                      return_error
                      exit
                  fi
                DATA_GOOD=1

          else

            message="$aht20_out"
            DATA_GOOD=0

          fi

      let "COUNTER += 1"
      sleep $COUNTER
done

if [[ $COUNTER -gt "2" ]]; then
  return_error
fi
