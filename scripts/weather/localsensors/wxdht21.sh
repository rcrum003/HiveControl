#!/bin/bash
# read the DHT21
# Version 2
# Supporting the Hivetool project
# gets and returns data like the temperhum

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc


function return_error {
    #Function to return an error value
         loglocal "$DATE2" WXTEMP ERROR "WX $message"
         HUMIDITY="0"
         TEMPF="0"
         TEMPC="0"
         dewpoint_f="0"
         echo $TEMPF $HUMIDITY $dewpoint_f $TEMPC
} 

DATA_GOOD=0
COUNTER=1
############################################################
#Set some values        
        #Conversion from wiringPI GPIO pin numbers to BCM Pin Numbers
        # The pigpio library uses BCM Pin Numbers
        #It's not the prettiest code, but works.
        #All GPIO setings in our software references the wiringPI GPIO
        case $WX_TEMP_GPIO in
          2)
          GPIO="27"
          ;;
          3)
          GPIO="22"
          ;;
          0)
          GPIO="17"
          ;;
          1)
          GPIO="18"
          ;;
          4)
          GPIO="23"
          ;;
          5)
          GPIO="24"
          ;;
          6)
          GPIO="25"
          ;;
          7)
          GPIO="4"
          ;;
          21)
          GPIO="5"
          ;;
          22)
          GPIO="6"
          ;;
          23)
          GPIO="13"
          ;;
          24)
          GPIO="19"
          ;;
          25)
          GPIO="26"
          ;;
          26)
          GPIO="12"
          ;;
          27)
          GPIO="16"
          ;;
          28)
          GPIO="20"
          ;;
          29)
          GPIO="21"
          ;;
          esac
############################################################
#Read the sensor
# Try 5 times for a good reading
############################################################
while [ $COUNTER -lt 6 ] && [ $DATA_GOOD -eq 0 ]
do
        DATE2=$(TZ=":$TIMEZONE" date '+%F %T')
        #Run the C Code to read the sensor
        DHT22=$(sudo /usr/local/bin/DHTXXD -g$GPIO)
          
        #Check for error status, which is included in the C code
          DHT_STATUS=$(echo $DHT22 |awk '{print $1}')
          case $DHT_STATUS in
            0 )
              #0 Read data was good!
                DATA_GOOD=1
                  #IF we had a good value, then run the conversions and such
                  HUMIDITY=$(echo $DHT22 | awk '{print $3}')
                  TEMPC=$(echo $DHT22 | awk '{print $2}')
                  #Convert C to F
                  TEMPF=$(echo "scale=1; (($TEMPC*9)/5)+32" |bc)
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
                   echo $TEMPF $HUMIDITY $dewpoint_f $TEMPC
                  else
                      $message="Temp readings were good, but the values failed validation"
                      return_error
                      exit
                  fi
            ;;
            1 )
              #DHT_BAD_CHECKSUM
              message="DHT Bad Checksum, returning 0,0 for all values"
              DATA_GOOD=0
            ;;
            2 )
              #Do
              message="DHT Bad Data, returning 0,0 for all values"
              DATA_GOOD=0
            ;;
            3 )
              #Do
              message="DHT Timeout, make sure you have proper GPIO set returning 0,0 for all values"
              DATA_GOOD=0
            ;;
          esac
        #do C code stuff
      let "COUNTER += 1"
      sleep $COUNTER
done

if [[ $COUNTER -gt "5" ]]; then
  return_error
fi



