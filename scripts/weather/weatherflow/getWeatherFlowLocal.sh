#!/bin/bash
# Version 6
# Script to output weatherflow data to a JSON and XML file that matches WUNDERGROUND file format
#
# Listens on the local network for UDP broadcast of the weatherstation.
#
# Revision 1
# 2020-05-26
# 
# 	



#Get our variables

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc

#JSON Parser Script
JSON_PATH="/home/HiveControl/scripts/weather/JSON.sh"

DATE=$(TZ=":$TIMEZONE" date '+%F %R')

#Added Sleep Function Here, all in seconds
#SLEEPTIME=$((1 + RANDOM % 30))
#sleep $SLEEPTIME


function listen_for_event {
 
 timeout 60 nc -kluw 0 50222 | (
    cnt=0
    line=
    while read -N 1 c; do
        line="$line$c"
        if [ "$c" = "{" ]; then
            cnt=$((cnt+1))
        elif [ "$c" = "}" ]; then
            cnt=$((cnt-1))
            if [ $cnt -eq 0 ]; then
                printf "%s\n" "$line"
                line=
            fi
        fi
    done
) | grep "obs" |grep "$WXSTATION" | tail -1

}

TRYCOUNTER="1" 
DATA_GOOD="0" 

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do
	
	GETDATA=$(listen_for_event)
	
	CHECKERROR=$(/bin/echo $GETDATA | jq '.obs[] | .[0]')
	 
	if [ -z "$CHECKERROR"  ]; then
		loglocal "$DATE" WEATHER INFO "Did not get a proper response from AmbientWeather.net, trying again"
		let TRYCOUNTER+=1
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD = "0" ]; then
	loglocal "$DATE" WEATHER ERROR "Did not get a proper response from AmbientWeather - $GETDATA"

fi

#jq '.obs[] | .[0]'

#Now that we are successful, do the needful.
WXSTATION=$(/bin/echo $GETDATA | jq --raw-output '.serial_number')
DATEUTC=$(/bin/echo $GETDATA | jq '.obs[] | .[0]')
WIND_DIR=$(/bin/echo $GETDATA | jq '.obs[] | .[4]')
WIND_SPEED_MS=$(/bin/echo $GETDATA | jq '.obs[] | .[2]')
WIND_GUST_MS=$(/bin/echo $GETDATA | jq '.obs[] | .[3]')
#WIND_MAX_DAILY_GUST=$(/bin/echo $GETDATA | jq '.obs[] | .[0]')
TEMP_C=$(/bin/echo $GETDATA | jq '.obs[] | .[7]')
#RAIN_HOURLY_IN=$(/bin/echo $GETDATA | jq '.obs[] | .[0]')
#RAIN_EVENT_IN=$(/bin/echo $GETDATA | jq '.obs[] | .[0]')
RAIN_DAILY_MM=$(/bin/echo $GETDATA | jq '.obs[] | .[12]')
#BAROM_REL_IN=$(/bin/echo $GETDATA | jq '.obs[] | .[6]')
pressure_mb=$(/bin/echo $GETDATA | jq '.obs[] | .[6]')
HUMIDITY=$(/bin/echo $GETDATA | jq '.obs[] | .[8]')
UV=$(/bin/echo $GETDATA | jq '.obs[] | .[10]')
SOLARRADIATION=$(/bin/echo $GETDATA | jq '.obs[] | .[11]')
LUX=$(/bin/echo $GETDATA | jq '.obs[] | .[9]')
LIGHTNING_STRIKE_COUNT=$(/bin/echo $GETDATA | jq '.obs[] | .[15]')
LIGHTNING_STRIKE_DISTANCE=$(/bin/echo $GETDATA | jq '.obs[] | .[14]') #km
#DEWPOINT_F=$(/bin/echo $GETDATA | jq '.obs[] | .[0]')
#DATE_NORM=$(/bin/echo $GETDATA | jq '.obs[] | .[0]')

# Do some conversions to
# WeatherFlow supplies most things in metric-ish settings, need to convert to imperials
TEMP_F=$(echo "scale=1;((9/5) * $TEMP_C) + 32" | bc -l)

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

# convert Meters Per Second (MS) to mph
# Formula - Wind(MPH) = 2.23694 * Wind(MS)
WIND_SPEED_MPH="$(echo "scale=2; ($WIND_SPEED_MS * 2.23694)/1" | bc)"
WIND_GUST_MPH="$(echo "scale=2; ($WIND_GUST_MS * 2.23694)/1" | bc)"

# convert MPH to kph per hour
wind_kph="$(echo "scale=2; ($WIND_SPEED_MPH * 1.609344)/1" | bc)"
wind_gust_kph="$(echo "scale=2; ($WIND_GUST_MPH * 1.609344)/1" | bc)"

#Convert MB pressure to IN
pressure_in="$(echo "scale=0; ($pressure_mb / 33.8637526)/1" | bc)"

#Calculate the dewpoint F/C
# Formula:  TDEW = T - 9(100-RH%) / 25
# Get the integers of the temperature and humidity
dewpoint_f="$(echo "scale=2; (${TEMP_F} - ( 9*(100 - ${HUMIDITY})) /25)" |bc -l)"
dewpoint_c=$(echo "scale=1;((${dewpoint_f}-32)*5)/9" | bc -l)

#Convert Rain MM to Inches
#precip_1hr_metric="$(echo "scale=2; ($RAIN_HOURLY_IN * 0.039370)" |bc)"
RAIN_DAILY_IN="$(echo "scale=2; ($RAIN_DAILY_MM * 0.039370)" |bc)"
precip_today_metric=$RAIN_DAILY_MM

if [[ -z "$TEMP_F" ]]; then
	loglocal "$DATE" WEATHER ERROR "Did not get a proper outTemp, even after counter, $GETDATA"
	#Since we didn't get a proper file, we should assume the whole thing is borked
	exit
fi

#ConvertDate
WX_DATE_TIME=$(date -d @$(($DATEUTC)) '+%Y-%m-%d %H:%M:%S')

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
echo "		\"temperature_string\":\"$TEMP_F F ($TEMP_C C)\","
echo "		\"temp_f\":\"$TEMP_F\","
echo "		\"temp_c\":\"$TEMP_C\","
echo "		\"relative_humidity\":\"$HUMIDITY%\","
echo "		\"wind_string\":\" $DIRECTION at $WIND_SPEED_MPH MPH, Gust to $WIND_GUST_MPH MPH\","
echo "		\"wind_dir\":\"$DIRECTION\","
echo "		\"wind_degrees\":\"$WIND_DIR\","
echo "		\"wind_mph\":\"$WIND_SPEED_MPH\","
echo "		\"wind_gust_mph\":\"$WIND_GUST_MPH\","
echo "		\"wind_kph\":\"$wind_kph\","
echo "		\"wind_gust_kph\":\"$wind_gust_kph\","
echo "		\"pressure_mb\":\"$pressure_mb\","
echo "		\"pressure_in\":\"$pressure_in\","
echo "		\"pressure_trend\":\"-\","
echo "		\"dewpoint_f\":\"$dewpoint_f\","
echo "		\"dewpoint_c\":\"$dewpoint_c\","
echo "		\"windchill_string\":\"NA\","
echo "		\"windchill_f\":\"NA\","
echo "		\"windchill_c\":\"NA\","
echo "		\"solarradiation\":\"$SOLARRADIATION\","
echo "		\"UV\":\"$UV\","
echo "		\"precip_1hr_in\":\"NA\","
echo "		\"precip_1hr_metric\":\"NA\","
echo "		\"precip_today_string\":\"$RAIN_DAILY_IN in ($precip_today_metric mm)\","
echo "		\"precip_today_in\":\"$RAIN_DAILY_IN\","
echo "		\"precip_today_metric\":\"$precip_today_metric\""
echo " }"
echo "}"