
#!/bin/bash
###################################
# Purple Air
#
# Script to pull air quality reports from a Purple Air sensor api.purpleair.com
#
# You can pull using one hive, but any more and PurpleAir may block you.
# Alternatively, setup a main/sub structure

# v 2022080101

#Pull configs and logger library
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
#source /home/HiveControl/scripts/data/curl.inc

#Store date in a variable
DATE=$(TZ=":$TIMEZONE" date '+%F %T')

JSON_PATH="/home/HiveControl/scripts/weather/JSON.sh"


#Instructions
#Documentation here:
#api.purpleair.com


#####################
# Main
#####################

AIR_URL="https://api.purpleair.com/v1/sensors"
TEMPFILE="/home/HiveControl/scripts/air/output.json"

#Read Key: 251486F2-9B9F-11EB-912F-42010A800259
#Write Key: 25158438-9B9F-11EB-912F-42010A800259


#GET https://api.purpleair.com/v1/sensors/7634  
#X-API-Key: 251486F2-9B9F-11EB-912F-42010A800259

PURPLE_API_KEY="251486F2-9B9F-11EB-912F-42010A800259"

#TEMPFILE="/home/pi/purpleair/output.json"

#Get all sensors
#https://www.purpleair.com/json 

#If PurpleSensorID is set, then use that one
#IF not, Find Air Quality Closet to you by latitude/longitude

#Pull Sensor Data
#https://www.purpleair.com/json?show=<ID>
while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do
	#https://www.purpleair.com/json?show=7634
	API_RESULT=$(/usr/bin/curl -s "$AIR_URL/$AIR_ID" --header "X-API-Key: $PURPLE_API_KEY")
		
	#Get Stats:
	PM2_5=$(jq --raw-output '.sensor."pm2.5"' <<<  "$API_RESULT")
	PM1_0=$(jq --raw-output '.sensor."pm1.0"' <<<  "$API_RESULT")
	PM10=$(jq --raw-output '.sensor."pm10.0"' <<<  "$API_RESULT")
	AIR_TEMP=$(jq --raw-output '.sensor.temperature' <<<  "$API_RESULT")
	AIR_HUMIDITY=$(jq --raw-output '.sensor.humidity' <<<  "$API_RESULT")
	AIR_PRESURE=$(jq --raw-output '.sensor.pressure' <<<  "$API_RESULT")

#New Factors:
#http://vista.cira.colostate.edu/Improve/visibility-basics/
	SCAT_COEFF=$(jq --raw-output '.sensor.scattering_coefficient' <<<  "$API_RESULT")
	DECIVIEWS=$(jq --raw-output '.sensor.deciviews' <<<  "$API_RESULT")
	VISUAL_RANGE=$(jq --raw-output '.sensor.visual_range' <<<  "$API_RESULT")

#Stats
	PM2_5_10M=$(jq --raw-output '.sensor.stats."pm2.5_10minute"' <<<  "$API_RESULT")
	PM2_5_30M=$(jq --raw-output '.sensor.stats."pm2.5_30minute"' <<<  "$API_RESULT")
	PM2_5_60M=$(jq --raw-output '.sensor.stats."pm2.5_60minute"' <<<  "$API_RESULT")

if [ -z "$PM2_5"  ]; then
		loglocal "$DATE" AIR INFO "Did not get a proper response from PurpleAir, trying again"
		let TRYCOUNTER+=1
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD = "0" ]; then
	loglocal "$DATE" AIR ERROR "Did not get a proper response from PurpleAir, stopping. $RESULT"
	echo "ERROR"
	exit
fi

#Do Conversion to AQI for US Market
#https://en.wikipedia.org/wiki/Air_quality_index

#Remove tempfile
rm -rf $TEMPFILE

#Return output for main script

#Note: last value is the derived 
#echo "DATE,AIR_TEMP,AIR_HUMIDITY,PM1_0,PM2_5,PM10,AQI"
echo "$DATE,$AIR_TEMP,$AIR_HUMIDITY,$PM1_0,$PM2_5,$PM10,0,$PM2_5_10M,$PM2_5_30M,$PM2_5_60M,$SCAT_COEFF,$DECIVIEWS,$VISUAL_RANGE"



#         _
#        /_/_      .'''.
#     =O(_)))) ...'     `.
#        \_\              `.    .'''
#                           `..'



























