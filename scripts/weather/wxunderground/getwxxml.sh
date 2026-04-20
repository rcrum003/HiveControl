
#!/bin/bash
#
# Script to get output from WX-Underground XML file that matches WUNDERGROUND file format
#
# Revision 2
# 2019-07-07
#
# Not sure how this script will last, now that IBM is the evil overlord of WXUnderground


#Get our variables

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

#set -x
DATE=$(TZ=":$TIMEZONE" date '+%F %R')


TRYCOUNTER="1"
DATA_GOOD="0"

TMPFILE=$(mktemp /tmp/getwxxml_XXXXXX.xml)
trap 'rm -f "$TMPFILE"' EXIT

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do

	#GETDATA=$(/usr/bin/curl --silent "$ambient_device_url/$ambient_deviceMAC?apiKey=$ambient_APIKEY&applicationKey=$ambient_APPID&limit=1")
	/usr/bin/curl --retry 5 --max-time 30 "http://api.wunderground.com/weatherstation/WXCurrentObXML.asp?ID=$WXSTATION" > $TMPFILE

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


# Convert Wind DIRECTION Degrees into Text
DIRECTION=$(degrees_to_cardinal "$WIND_DIR")

# Convert MPH to KPH
wind_kph=$(mph_to_kph "$WIND_SPEED_MPH")
wind_gust_kph=$(mph_to_kph "$WIND_GUST_MPH")


if [[ -z "$TEMP_F" ]]; then
	loglocal "$DATE" WEATHER ERROR "Did not get a proper Temp from WXUnderground, even after counter, $GETDATA"
	#Since we didn't get a proper file, we should assume the whole thing is borked
	exit
fi


# Return a JSON file to mimic WUNderground file format
WX_STATION_ID="$WXSTATION"
WX_OBS_TIME="$DATEUTC"
WX_TEMP_F="$TEMP_F"
WX_TEMP_C="$temp_c"
WX_HUMIDITY="$HUMIDITY"
WX_WIND_DIR_TEXT="$DIRECTION"
WX_WIND_DIR_DEG="$WIND_DIR"
WX_WIND_MPH="$WIND_SPEED_MPH"
WX_WIND_GUST_MPH="$WIND_GUST_MPH"
WX_WIND_KPH="$wind_kph"
WX_WIND_GUST_KPH="$wind_gust_kph"
WX_PRESSURE_MB="$PRESSURE_MB"
WX_PRESSURE_IN="$PRESSURE_IN"
WX_PRESSURE_TREND="-"
WX_DEWPOINT_F="$DEWPOINT_F"
WX_DEWPOINT_C="$DEWPOINT_C"
WX_SOLARRADIATION="$SOLAR_RADIATION"
WX_UV="$UV"
WX_PRECIP_1HR_IN="$RAIN_HOURLY_IN"
WX_PRECIP_1HR_MM="$precip_1hr_metric"
WX_PRECIP_TODAY_IN="$RAIN_DAILY_IN"
WX_PRECIP_TODAY_MM="$precip_today_metric"
output_wx_json
