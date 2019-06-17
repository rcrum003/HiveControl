#!/bin/bash
# read the AdaFruit BME680 Temp/Humidity/Gas Sensor
# Version 1.0
# Supporting the HiveControl project
# gets and returns data like the temperhum


# Requires that the binary bme680 is in /usr/local/bin

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
# Try 5 times for a good reading
############################################################
while [ $COUNTER -lt 3 ] && [ $DATA_GOOD -eq 0 ]
do
        DATE2=$(TZ=":$TIMEZONE" date '+%F %T')
        #Run the C Code to read the sensor
        
        bme680_out=$(sudo /usr/local/bin/bme680 5 1 2>&1)
        
        #echo " Output was $bme680_out"

        #Check for error status, which is included in the C code
          BME680STATUS=$(echo $bme680_out |awk '{print $1}')
          #echo "Status was $SHT31DSTATUS"

          if [[ "$BME680STATUS" != "Error" ]]; then
           #Must have been good
            #echo "Doing Else.............."
            #IF we had a good value, then run the conversions and such
                  
                  TEMPF_RAW=$(echo $bme680_out | awk '{print $1}')
                  TEMPC_RAW=$(echo $bme680_out | awk '{print $2}')
                  HUMIDITY=$(echo $bme680_out | awk '{print $3}')
                  PRESSURE=$(echo $bme680_out | awk '{print $4}')
                  GAS=$(echo $bme680_out | awk '{print $5}')

                  TEMPF=$(echo "scale=1; ($TEMPF_RAW/1)" |bc)
                  TEMPC=$(echo "scale=1; ($TEMPC_RAW/1)" |bc)

                  #echo "Humidity was $HUMIDITY, Temp was $TEMPF"
      
                  #Calculate dewpoint
                  dewpoint_f=$(echo "scale=2; ($TEMPF - ( 9* (100 - $HUMIDITY)) /25)" |bc)
                  dewpoint_c=$(echo "scale=1;(($dewpoint_f-32)*5)/9" | bc -l) 
                  
                  #Test our values to see if they were actually any good.
                  HUMIDITY_MINTEST=$(echo "$HUMIDITY < 0" | bc)
                  TEMP_MINTEST=$(echo "$TEMPF > -50" | bc) #If you are below -50F you shouldn't have bees
                  HUMIDITY_MAXTEST=$(echo "$HUMIDITY > 101" | bc)  #100 means it's raining
                  TEMP_MAXTEST=$(echo "$TEMPF > 150" | bc) #That would be hot
                  if [ $HUMIDITY_MINTEST == "0" ] && [ $TEMP_MINTEST != "0" ] && [ $HUMIDITY_MAXTEST == "0" ] && [ $TEMP_MAXTEST == "0" ] 
                    then
                    #Return a value for use by other scripts
                   echo $TEMPF $HUMIDITY $dewpoint_f $TEMPC $PRESSURE $GAS
                  else
                      $message="ERROR Temp readings were good, but the values failed validation"
                      return_error
                      exit
                  fi
                DATA_GOOD=1
              
          else

            #Check for Error states, if so, try again
            message="$bme680_out"
            DATA_GOOD=0
            

          fi

         #do C code stuff
      let "COUNTER += 1"
      sleep $COUNTER
done

if [[ $COUNTER -gt "2" ]]; then
  return_error
fi




