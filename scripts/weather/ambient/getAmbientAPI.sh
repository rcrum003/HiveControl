
#!/bin/bash
# Version 6
# Script to output API.Ambientweather.net to a JSON and XML file that matches WUNDERGROUND file format
#
# Revision 2
# 2020-05-26
# 
# 	


#Get our variables

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc

#move to db
ambient_APPID="99d38400a7ff405a9389b113423f3416440cf4e7738841709862a048d7faaa8a"
ambient_APIKEY=$KEY
ambient_deviceMAC=$WXSTATION
ambient_device_url="https://api.ambientweather.net/v1/devices"

#Specific Device
JSON_PATH="/home/HiveControl/scripts/weather/JSON.sh"

DATE=$(TZ=":$TIMEZONE" date '+%F %R')

#Added Sleep Function Here, all in seconds
SLEEPTIME=$((1 + RANDOM % 30))


WXSTATION="AMW_$ambient_deviceMAC"

TRYCOUNTER="1" 
DATA_GOOD="0" 

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do
	
	GETDATA=$(/usr/bin/curl --silent "$ambient_device_url/$ambient_deviceMAC?apiKey=$ambient_APIKEY&applicationKey=$ambient_APPID&limit=1")
	
	CHECKERROR=$(/bin/echo $GETDATA | $JSON_PATH -b | grep tempf |awk '{print $2}')
	 
	if [ -z "$CHECKERROR"  ]; then
		#Clean up bad characters
		ERROR_MSG=$(sed 's/\"//g' <<<"$GETDATA")
		loglocal "$DATE" WEATHER INFO "Did not get a proper response from AmbientWeather.net, trying again, $ERROR_MSG"
		sleep $SLEEPTIME
		let TRYCOUNTER+=1
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD = "0" ]; then
	ERROR_MSG=$(sed 's/\"//g' <<<"$GETDATA")
	loglocal "$DATE" WEATHER ERROR "Did not get a proper response from AmbientWeather - $ERROR_MSG"
	exit
fi


#Now that we are successful, do the needful.
DATEUTC=$(/bin/echo $GETDATA | $JSON_PATH -b | grep dateutc |awk '{print $2}')
WIND_DIR=$(/bin/echo $GETDATA | $JSON_PATH -b | grep winddir |awk '{print $2}')
WIND_SPEED_MPH=$(/bin/echo $GETDATA | $JSON_PATH -b | grep windspeedmph |awk '{print $2}')
WIND_GUST_MPH=$(/bin/echo $GETDATA | $JSON_PATH -b | grep windgustmph |awk '{print $2}')
WIND_MAX_DAILY_GUST=$(/bin/echo $GETDATA | $JSON_PATH -b | grep windgustmph |awk '{print $2}')
TEMP_F=$(/bin/echo $GETDATA | $JSON_PATH -b | grep tempf |awk '{print $2}')
RAIN_HOURLY_IN=$(/bin/echo $GETDATA | $JSON_PATH -b | grep hourlyrainin |awk '{print $2}')
RAIN_EVENT_IN=$(/bin/echo $GETDATA | $JSON_PATH -b | grep eventrainin |awk '{print $2}')
RAIN_DAILY_IN=$(/bin/echo $GETDATA | $JSON_PATH -b | grep dailyrainin |awk '{print $2}')
BAROM_REL_IN=$(/bin/echo $GETDATA | $JSON_PATH -b | grep baromrelin |awk '{print $2}')
BAROM_ABS_IN=$(/bin/echo $GETDATA | $JSON_PATH -b | grep baromabsin |awk '{print $2}')
HUMIDITY=$(/bin/echo $GETDATA | $JSON_PATH -b | grep humidity |grep -v humidityin |awk '{print $2}')
UV=$(/bin/echo $GETDATA | $JSON_PATH -b | grep uv |awk '{print $2}')
SOLARRADIATION=$(/bin/echo $GETDATA | $JSON_PATH -b | grep solarradiation |awk '{print $2}')
DATE_NORM=$(/bin/echo $GETDATA | $JSON_PATH -b | grep "date" |grep -v "dateutc" |awk '{print $2}')

# Do some conversions to
# AmbientWeather seems to only supply F - below we convert to C
temp_c=$(echo "scale=1;(($TEMP_F-32)*5)/9" | bc -l)


# Convert Wind then DIRECTION Degrees into Text

if [ 1 -eq "$(echo "$WIND_DIR >= 0" | bc)" ] && [ 1 -eq "$(echo "$WIND_DIR < 22.5" | bc)" ]; 
	then 
		DIRECTION="North" 
elif [ 1 -eq "$(echo "$WIND_DIR >= 22.5" | bc)" ] && [ 1 -eq "$(echo "$WIND_DIR < 67.5" | bc)" ]; 
	then 
		DIRECTION="NE" 
elif [ 1 -eq "$(echo "$WIND_DIR >= 67.5" | bc)" ] && [ 1 -eq "$(echo "$WIND_DIR < 112.5" | bc)" ]; 
	then 
		DIRECTION="East" 
elif [ 1 -eq "$(echo "$WIND_DIR >= 112.5" | bc)" ] && [ 1 -eq "$(echo "$WIND_DIR < 157.5" | bc)" ]; 
	then 
		DIRECTION="SE" 
elif [ 1 -eq "$(echo "$WIND_DIR >= 157.5" | bc)" ] && [ 1 -eq "$(echo "$WIND_DIR < 202.5" | bc)" ];
	then 
		DIRECTION="South" 
elif [ 1 -eq "$(echo "$WIND_DIR >= 202.5" | bc)" ] && [ 1 -eq "$(echo "$WIND_DIR < 247.5" | bc)" ];
	then 
		DIRECTION="SW" 
elif [ 1 -eq "$(echo "$WIND_DIR >= 247.5" | bc)" ] && [ 1 -eq "$(echo "$WIND_DIR < 292.5" | bc)" ];
	then 
		DIRECTION="West" 
elif [ 1 -eq "$(echo "$WIND_DIR >= 292.5" | bc)" ] && [ 1 -eq "$(echo "$WIND_DIR < 337.5" | bc)" ];
	then 
		DIRECTION="NW" 
else DIRECTION="North"
fi 

# convert MPH to kph per hour
wind_kph="$(echo "scale=2; ($WIND_SPEED_MPH * 1.609344)/1" | bc)"
wind_gust_kph="$(echo "scale=2; ($WIND_GUST_MPH * 1.609344)/1" | bc)"


#Convert Pressure in to MB
pressure_mb="$(echo "scale=0; ($BAROM_ABS_IN * 33.8637526)/1" | bc)"

#Calculate the dewpoint F
# Formula:  TDEW = T - 9(100-RH%) / 25
# Get the integers of the temperature and humidity
dewpoint_f="$(echo "scale=2; (${TEMP_F} - ( 9* (100 - ${HUMIDITY})) /25)" |bc -l)"
dewpoint_c=$(echo "scale=1;((${dewpoint_f}-32)*5)/9" | bc -l)

#Convert Rain Inches for Hour and Day to Metric
precip_1hr_metric="$(echo "scale=2; ($RAIN_HOURLY_IN / 0.039370)" |bc)"
precip_today_metric="$(echo "scale=2; ($RAIN_DAILY_IN / 0.039370)" |bc)"

if [[ -z "$TEMP_F" ]]; then
	ERROR_MSG=$(sed 's/\"//g' <<<"$GETDATA")
	loglocal "$DATE" WEATHER ERROR "Did not get a proper outTemp, even after counter, $ERROR_MSG"
	#Since we didn't get a proper file, we should assume the whole thing is borked
	exit
fi

#ConvertDate
WX_DATE_TIME=$(date -d @$(($DATEUTC/1000)) '+%Y-%m-%d %H:%M:%S')

#B4:E6:2D:07:87:49,1562721300000,72.9,80,66.36,22.7,0,0,224,0,0,0,1014,29.95,0,19.0,0,0,0,0,0,0,0
#LOCALSENSOR,2019-07-09 21:20,73.8,45.20,54.08,20.9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	
# Return a JSON file to mimic WUNderground file format
echo "{"
echo "\"response\": {"
echo "\"version\":\"0.1\","
echo "\"features\": {"
echo "\"conditions\": 1"
echo "  }"
echo "	}"
echo "  ,	\"current_observation\": {"
echo "		\"station_id\":\"$WXSTATION\","
echo "		\"observation_time\":\"$WX_DATE_TIME\","
echo "		\"temperature_string\":\"$TEMP_F F ($temp_c C)\","
echo "		\"temp_f\":\"$TEMP_F\","
echo "		\"temp_c\":\"$temp_c\","
echo "		\"relative_humidity\":\"$HUMIDITY%\","
echo "		\"wind_string\":\" $DIRECTION at $WIND_SPEED_MPH MPH, Gust to $WIND_GUST_MPH MPH\","
echo "		\"wind_dir\":\"$DIRECTION\","
echo "		\"wind_degrees\":\"$WIND_DIR\","
echo "		\"wind_mph\":\"$WIND_SPEED_MPH\","
echo "		\"wind_gust_mph\":\"$WIND_GUST_MPH\","
echo "		\"wind_kph\":\"$wind_kph\","
echo "		\"wind_gust_kph\":\"$wind_gust_kph\","
echo "		\"pressure_mb\":\"$pressure_mb\","
echo "		\"pressure_in\":\"$BAROM_ABS_IN\","
echo "		\"pressure_trend\":\"-\","
echo "		\"dewpoint_f\":\"$dewpoint_f\","
echo "		\"dewpoint_c\":\"$dewpoint_c\","
echo "		\"windchill_string\":\"NA\","
echo "		\"windchill_f\":\"NA\","
echo "		\"windchill_c\":\"NA\","
echo "		\"solarradiation\":\"$SOLARRADIATION\","
echo "		\"UV\":\"$UV\","
echo "		\"precip_1hr_in\":\"$RAIN_HOURLY_IN\","
echo "		\"precip_1hr_metric\":\"$precip_1hr_metric\","
echo "		\"precip_today_string\":\"$RAIN_DAILY_IN in ($precip_today_metric mm)\","
echo "		\"precip_today_in\":\"$RAIN_DAILY_IN\","
echo "		\"precip_today_metric\":\"$precip_today_metric\""
echo " }"
echo "}"


