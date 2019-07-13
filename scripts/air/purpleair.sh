
#!/bin/bash
###################################
# Purple Air
#
# Script to pull air quality reports from a Purple Air sensor
#
# v 2019070301

#Pull configs and logger library
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
#source /home/HiveControl/scripts/data/curl.inc

#Store date in a variable
DATE=$(TZ=":$TIMEZONE" date '+%F %T')

JSON_PATH="/home/HiveControl/scripts/weather/JSON.sh"

air_id="7634"

#Instructions
#Documentation here:
#https://docs.google.com/document/d/15ijz94dXJ-YAZLi9iZ_RaBwrZ4KtYeCy08goGBwnbCU/edit

#One entry: https://www.purpleair.com/json?show=<ID> where ID is the “ID” of the sensor you want (in the case of dual laser where ParentID is “null”).

#Pulling from THINKSPEAK because they give us better values


#How to find my closet purpleair
# Curl 
# jq --compact-output --raw-output '.results[] | [.ID,.ParentID,.Label,.Lat,.Lon,.LastSeen] | @csv' <bar.json > purpleid.csv
# Load into table

#Query
#select id, label, lat, lon, from_unixtime(lastseen) as lastseen_date from purple_air where parent_id is null AND lat BETWEEN "41" AND "42" AND lon BETWEEN "-74" AND "-73" AND from_unixtime(lastseen) >= DATE(NOW() - INTERVAL 7 DAY);

#SQL Query to find which are the closest.

#SELECT id, label, latitude, longitude, from_unixtime(lastseen) as lastseen_date, ( 3959 * acos( cos( radians( 41.17054100 ) ) * cos( radians(latitude) ) * 
#cos( radians(longitude) - radians( -73.25438300 ) ) + sin( radians( 41.17054100 ) ) * 
#sin( radians(latitude) ) ) ) AS distance FROM purple_air WHERE from_unixtime(lastseen) >= DATE(NOW() - INTERVAL 7 DAY)  HAVING
#distance < 10  ORDER BY distance;

#####################
# Main
#####################
#set -x

#TEMPFILE="/home/HiveControl/scripts/air/output.json"
TEMPFILE="/home/pi/purpleair/output.json"

#Get all sensors
#https://www.purpleair.com/json 

#If PurpleSensorID is set, then use that one
#IF not, Find Air Quality Closet to you by latitude/longitude

#Pull Sensor Data
#https://www.purpleair.com/json?show=<ID>
while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do
	#https://www.purpleair.com/json?show=7634
	RESULT=$(/usr/bin/curl -s -w %{http_code} "https://www.purpleair.com/json?show=$air_id" -o $TEMPFILE)
		
	#Get Channel A	
	THINKSPEAK_A_ID=$(jq --compact-output --raw-output '.results[] | .THINGSPEAK_PRIMARY_ID' <$TEMPFILE | head -1)
	THINKSPEAK_A_API=$(jq --compact-output --raw-output '.results[] | .THINGSPEAK_PRIMARY_ID_READ_KEY' <$TEMPFILE | head -1)
	THINKSPEAK_B_ID=$(jq --compact-output --raw-output '.results[] | .THINGSPEAK_SECONDARY_ID' <$TEMPFILE | head -1)
	THINKSPEAK_B_API=$(jq --compact-output --raw-output '.results[] | .THINGSPEAK_SECONDARY_ID_READ_KEY' <$TEMPFILE | head -1)


	if [[ -z $THINKSPEAK_A_ID ]]; then
		#Looks like something failed
		loglocal "$DATE" AIR ERROR "Purple ID cannot be found, please check your entry of $air_id, CURL $RESULT"
		echo "ERROR"
		exit 1
	fi

	###################
	# Get Channel A
	###################
	RESULT=$(/usr/bin/curl -s -w %{http_code} "https://api.thingspeak.com/channels/$THINKSPEAK_A_ID/feeds.json?api_key=$THINKSPEAK_A_API&results=1" -o $TEMPFILE)
	
	PM2_5=$(jq --raw-output '.feeds[] | .field8' <$TEMPFILE) #PM2.5 (CF=1)
	AIR_TEMP=$(jq --raw-output '.feeds[] | .field6' <$TEMPFILE)
	AIR_HUMIDITY=$(jq --raw-output '.feeds[] | .field7' <$TEMPFILE)

	####################
	#Get Channel B
	####################
	RESULT=$(/usr/bin/curl -s -w %{http_code} "https://api.thingspeak.com/channels/$THINKSPEAK_B_ID/feeds.json?api_key=$THINKSPEAK_B_API&results=1" -o $TEMPFILE)
	#curl_get $SRC_URL $TEMPFILE $JOB_NAME

	PM1_0=$(jq --raw-output '.feeds[] | .field7' <$TEMPFILE)
	PM10=$(jq --raw-output '.feeds[] | .field8' <$TEMPFILE)

	if [ -z "$PM2_5"  ]; then
		loglocal "$DATE" AIR INFO "Did not get a proper response from PurpleAir/ThinkSpeak, trying again"
		let TRYCOUNTER+=1
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD = "0" ]; then
	loglocal "$DATE" AIR ERROR "Did not get a proper response from PurpleAir/ThinkSpeak- $RESULT"
	echo "ERROR"
	exit
fi

#Do Conversion to AQI for US Market
#https://en.wikipedia.org/wiki/Air_quality_index

#Return output for main script

#echo "DATE,AIR_TEMP,AIR_HUMIDITY,PM1_0,PM2_5,PM10"
echo "$DATE,$AIR_TEMP,$AIR_HUMIDITY,$PM1_0,$PM2_5,$PM10"



#         _
#        /_/_      .'''.
#     =O(_)))) ...'     `.
#        \_\              `.    .'''
#                           `..'



























