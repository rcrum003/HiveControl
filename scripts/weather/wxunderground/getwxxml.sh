
#!/bin/bash
#
# Script to get output from WX-Underground XML file that matches WUNDERGROUND file format
#
# Revision 1
# 2019-07-07
# 
# Not sure how this script will last, now that IBM is the evil overlord of WXUnderground 	


#Get our variables

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc

#set -x
DATE=$(TZ=":$TIMEZONE" date '+%F %R')


TRYCOUNTER="1" 
DATA_GOOD="0" 

TMPFILE="/home/HiveControl/scripts/weather/wxunderground/temp.xml"

#Remove old file
rm -rf $TMPFILE

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do
	
	#GETDATA=$(/usr/bin/curl --silent "$ambient_device_url/$ambient_deviceMAC?apiKey=$ambient_APIKEY&applicationKey=$ambient_APPID&limit=1")
	/usr/bin/curl --retry 5 "http://api.wunderground.com/weatherstation/WXCurrentObXML.asp?ID=$WXSTATION" > $TMPFILE
	
	CHECKERROR=$?

	if [ "$CHECKERROR" -ne "0"  ]; then
		loglocal "$DATE" WEATHER INFO "Did not get a proper response from WUNDERGROUND, trying again"
		let TRYCOUNTER+=1
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD = "0" ]; then
	loglocal "$DATE" WEATHER ERROR "Did not get a proper response from Wunderground -$CHECKERROR"

fi

DATEUTC=$DATE
TEMP_F=`grep temp_f $TMPFILE | grep  -o "[0-9]*\.[0-9]*"`
temp_c=`grep temp_c $TMPFILE | grep  -o "[0-9]*\.[0-9]*"`
HUMIDITY=`grep relative_humidity $TMPFILE | grep  -o "[0-9]*"`
WIND_DIR=`grep wind_degrees $TMPFILE | grep -o "[0-9]*"`
WIND_SPEED_MPH=`grep wind_mph $TMPFILE | grep  -o "[0-9]*\.[0-9]*"`
WIND_GUST_MPH=`grep wind_gust_mph $TMPFILE |  grep  -o "[0-9]*\.[0-9]*"`
PRESSURE_MB=`grep pressure_mb $TMPFILE |  grep  -o "[0-9]*\.[0-9]*"`
DEWPOINT_F=`grep dewpoint_f $TMPFILE |  grep  -o "[0-9]*\.[0-9]*"`
DEWPOINT_C=`grep dewpoint_c $TMPFILE |  grep  -o "[0-9]*\.[0-9]*"`
RAIN_HOURLY_IN=`grep precip_1hr_in $TMPFILE |  grep  -o "[0-9]*\.[0-9]*"`
RAIN_DAILY_IN=`grep precip_today_in $TMPFILE |  grep  -o "[0-9]*\.[0-9]*"`
SOLAR_RADIATION=`grep solar_radiation $TMPFILE |  grep  -o "[0-9]*"`
UV=`grep UV $TMPFILE |  grep  -o "[0-9]*\.[0-9]*"`
precip_1hr_metric=`grep precip_1hr_metric $TMPFILE |  grep  -o "[0-9]*\.[0-9]*"`
precip_today_metric=`grep precip_1hr_metric $TMPFILE |  grep  -o "[0-9]*\.[0-9]*"`
PRESSURE_IN=`grep pressure_in $TMPFILE |  grep  -o "[0-9]*\.[0-9]*"`


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


if [[ -z "$TEMP_F" ]]; then
	loglocal "$DATE" WEATHER ERROR "Did not get a proper Temp from WXUnderground, even after counter, $GETDATA"
	#Since we didn't get a proper file, we should assume the whole thing is borked
	exit
fi

	
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
echo "		\"observation_time\":\"$DATEUTC\","
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
echo "		\"pressure_in\":\"$PRESSURE_IN\","
echo "		\"pressure_trend\":\"-\","
echo "		\"dewpoint_f\":$DEWPOINT_F,"
echo "		\"dewpoint_c\":\"$dewpoint_c\","
echo "		\"windchill_string\":\"NA\","
echo "		\"windchill_f\":\"NA\","
echo "		\"windchill_c\":\"NA\","
echo "		\"solarradiation\":\"$SOLAR_RADIATION\","
echo "		\"UV\":\"$UV\","
echo "		\"precip_1hr_in\":\"$RAIN_HOURLY_IN\","
echo "		\"precip_1hr_metric\":\"$precip_1hr_metric\","
echo "		\"precip_today_string\":\"$RAIN_DAILY_IN in ($precip_today_metric mm)\","
echo "		\"precip_today_in\":\"$RAIN_DAILY_IN\","
echo "		\"precip_today_metric\":\"$precip_today_metric\""
echo " }"
echo "}"

rm -rf $TMPFILE



