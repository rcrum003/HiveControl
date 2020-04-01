#!/bin/bash
# read the BroodMinder Temp/Humidity Sensor
# Version 1.0
# Supporting the HiveControl project
# gets and returns data like the temperhum


# Requires that the python script BM_Scan.py is in /home/HiveControl/software/broodminder

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
 #      1    2         3    4    5      6    7     8   9 
#DeviceId, DeviceMAC,RSSI,Date,Record,TempC,TempF,Hum,Battery
#42:1C:8A,06:09:16:42:1c:8a,-72,2020-03-31 22:01:04,66,22.9,73.2,41,100


############################################################
#Read the sensor
# Try 5 times for a good reading
############################################################
while [ $COUNTER -lt 3 ] && [ $DATA_GOOD -eq 0 ]
do
        DATE2=$(TZ=":$TIMEZONE" date '+%F %T')
        #Run the Python Code to read the sensor
        
        #Get the varialble on which device they want

        broodminder_out=$(sudo python /home/HiveControl/software/broodminder/BM_Scan.py |grep -i $HIVEDEVICE | tail -1 2>&1)
        #echo " Output was $broodminder_out"

        #Check for Error or Empty by checking to see if first field contains our hivedevice
        CHECK_DEVICE_STATUS=$(echo $broodminder_out |awk -F, '{print $1}')


        #Check for error status
          if [[ $CHECK_DEVICE_STATUS = $HIVEDEVICE ]]; then
           #Must have been good
            #IF we had a good value, then run the conversions and such
                  
                  TEMPF_RAW=$(echo $broodminder_out | awk -F, '{print $7}')
                  TEMPC_RAW=$(echo $broodminder_out | awk -F, '{print $6}')
                  HUMIDITY=$(echo $broodminder_out | awk -F, '{print $8}')
                  BATTERY=$(echo $broodminder_out | awk -F, '{print $9}')
                  
                  #Check Battery
                  if [[ $BATTERY -lt "5" ]]; then
                    #Battery is getting low
                    message="BroodMinder Battery is at $BATTERY%, change soon"
                    loglocal "$DATE2" TEMP ERROR "$message"
                  elif [[ $BATTERY -lt "2" ]]; then
                    #Battery is basically dead
                    message="BroodMinder Battery is at $BATTERY%, stopping collection"
                    return_error
                    exit
                  fi

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
                   echo $TEMPF $HUMIDITY $dewpoint_f $TEMPC 
                  else
                      message="Temp readings were good, but the values failed validation"
                      return_error
                      exit
                  fi
                DATA_GOOD=1
              
          else
            #Check for Error states, if so, try again            
                    #Looks like we had an error
                    #message="Attempt $COUNTER - Failed to find BroodMinder Device $HIVEDEVICE, $broodminder_out"
                    #loglocal "$DATE2" TEMP ERROR "$message"  
              DATA_GOOD=0
            

          fi

         #do C code stuff
      let "COUNTER += 1"
      sleep $COUNTER
done

if [[ $COUNTER -gt "2" ]]; then
  message="Failed to find BroodMinder Device $HIVEDEVICE $broodminder_out"
  return_error
fi




