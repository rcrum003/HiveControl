
#!/bin/bash
# Version 7
# Script to output API.Ambientweather.net to a JSON and XML file that matches WUNDERGROUND file format
#
# Revision 2
# 2020-05-26
#


#Get our variables

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

#move to db - should eventually move to config
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

	GETDATA=$(/usr/bin/curl --silent --max-time 30 "$ambient_device_url/$ambient_deviceMAC?apiKey=$ambient_APIKEY&applicationKey=$ambient_APPID&limit=1")

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

# Do some conversions
# AmbientWeather seems to only supply F - below we convert to C
temp_c=$(f_to_c "$TEMP_F")

# Convert Wind DIRECTION Degrees into Text
DIRECTION=$(degrees_to_cardinal "$WIND_DIR")

# Convert MPH to KPH
wind_kph=$(mph_to_kph "$WIND_SPEED_MPH")
wind_gust_kph=$(mph_to_kph "$WIND_GUST_MPH")

# Convert Pressure in to MB
pressure_mb=$(in_to_mb "$BAROM_ABS_IN")

# Calculate the dewpoint F
dewpoint_f=$(calc_dewpoint_f "$TEMP_F" "$HUMIDITY")
dewpoint_c=$(f_to_c "$dewpoint_f")

# Convert Rain Inches for Hour and Day to Metric
precip_1hr_metric=$(in_to_mm "$RAIN_HOURLY_IN")
precip_today_metric=$(in_to_mm "$RAIN_DAILY_IN")

if [[ -z "$TEMP_F" ]]; then
	ERROR_MSG=$(sed 's/\"//g' <<<"$GETDATA")
	loglocal "$DATE" WEATHER ERROR "Did not get a proper outTemp, even after counter, $ERROR_MSG"
	#Since we didn't get a proper file, we should assume the whole thing is borked
	exit
fi

#ConvertDate
WX_DATE_TIME=$(date -d @$(($DATEUTC/1000)) '+%Y-%m-%d %H:%M:%S')

# Return a JSON file to mimic WUNderground file format
WX_STATION_ID="$WXSTATION"
WX_OBS_TIME="$WX_DATE_TIME"
WX_TEMP_F="$TEMP_F"
WX_TEMP_C="$temp_c"
WX_HUMIDITY="$HUMIDITY"
WX_WIND_DIR_TEXT="$DIRECTION"
WX_WIND_DIR_DEG="$WIND_DIR"
WX_WIND_MPH="$WIND_SPEED_MPH"
WX_WIND_GUST_MPH="$WIND_GUST_MPH"
WX_WIND_KPH="$wind_kph"
WX_WIND_GUST_KPH="$wind_gust_kph"
WX_PRESSURE_MB="$pressure_mb"
WX_PRESSURE_IN="$BAROM_ABS_IN"
WX_PRESSURE_TREND="-"
WX_DEWPOINT_F="$dewpoint_f"
WX_DEWPOINT_C="$dewpoint_c"
WX_SOLARRADIATION="$SOLARRADIATION"
WX_UV="$UV"
WX_PRECIP_1HR_IN="$RAIN_HOURLY_IN"
WX_PRECIP_1HR_MM="$precip_1hr_metric"
WX_PRECIP_TODAY_IN="$RAIN_DAILY_IN"
WX_PRECIP_TODAY_MM="$precip_today_metric"
output_wx_json
