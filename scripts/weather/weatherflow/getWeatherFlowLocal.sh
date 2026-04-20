#!/bin/bash
# Version 8
# Script to output weatherflow data to a JSON and XML file that matches WUNDERGROUND file format
#
# Listens on the local network for UDP broadcast of the weatherstation.
#
# Revision 2
# 2026-01-17 - Fixed error handling when no data received
#
#



#Get our variables

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc
source /home/HiveControl/scripts/weather/wx_helpers.inc

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
	loglocal "$DATE" WEATHER ERROR "Did not get a proper response from WeatherFlow - $GETDATA"
	# Output empty JSON and exit - getwx.sh expects JSON format
	exit 1
fi

#jq '.obs[] | .[0]'

#Now that we are successful, do the needful.
WXSTATION=$(/bin/echo $GETDATA | jq --raw-output '.serial_number' 2>/dev/null)
DATEUTC=$(/bin/echo $GETDATA | jq '.obs[] | .[0]' 2>/dev/null)
WIND_DIR=$(/bin/echo $GETDATA | jq '.obs[] | .[4]' 2>/dev/null)
WIND_SPEED_MS=$(/bin/echo $GETDATA | jq '.obs[] | .[2]' 2>/dev/null)
WIND_GUST_MS=$(/bin/echo $GETDATA | jq '.obs[] | .[3]' 2>/dev/null)
#WIND_MAX_DAILY_GUST=$(/bin/echo $GETDATA | jq '.obs[] | .[0]' 2>/dev/null)
TEMP_C=$(/bin/echo $GETDATA | jq '.obs[] | .[7]' 2>/dev/null)
#RAIN_HOURLY_IN=$(/bin/echo $GETDATA | jq '.obs[] | .[0]' 2>/dev/null)
#RAIN_EVENT_IN=$(/bin/echo $GETDATA | jq '.obs[] | .[0]' 2>/dev/null)
RAIN_DAILY_MM=$(/bin/echo $GETDATA | jq '.obs[] | .[12]' 2>/dev/null)
#BAROM_REL_IN=$(/bin/echo $GETDATA | jq '.obs[] | .[6]' 2>/dev/null)
pressure_mb=$(/bin/echo $GETDATA | jq '.obs[] | .[6]' 2>/dev/null)
HUMIDITY=$(/bin/echo $GETDATA | jq '.obs[] | .[8]' 2>/dev/null)
UV=$(/bin/echo $GETDATA | jq '.obs[] | .[10]' 2>/dev/null)
SOLARRADIATION=$(/bin/echo $GETDATA | jq '.obs[] | .[11]' 2>/dev/null)
LUX=$(/bin/echo $GETDATA | jq '.obs[] | .[9]' 2>/dev/null)
LIGHTNING_STRIKE_COUNT=$(/bin/echo $GETDATA | jq '.obs[] | .[15]' 2>/dev/null)
LIGHTNING_STRIKE_DISTANCE=$(/bin/echo $GETDATA | jq '.obs[] | .[14]' 2>/dev/null) #km
#DEWPOINT_F=$(/bin/echo $GETDATA | jq '.obs[] | .[0]' 2>/dev/null)
#DATE_NORM=$(/bin/echo $GETDATA | jq '.obs[] | .[0]' 2>/dev/null)

# Check if we have required data - if TEMP_C is null/empty, data is bad
if [[ -z "$TEMP_C" ]] || [[ "$TEMP_C" == "null" ]]; then
	loglocal "$DATE" WEATHER ERROR "Did not get valid temperature from WeatherFlow"
	exit 1
fi

# Do some conversions to
# WeatherFlow supplies most things in metric-ish settings, need to convert to imperials
TEMP_F=$(c_to_f "$TEMP_C")

# Convert Wind then DIRECTION Degrees into Text
# Set default values if empty
WIND_DIR="${WIND_DIR:-0}"
WIND_SPEED_MS="${WIND_SPEED_MS:-0}"
WIND_GUST_MS="${WIND_GUST_MS:-0}"
pressure_mb="${pressure_mb:-0}"
HUMIDITY="${HUMIDITY:-0}"
RAIN_DAILY_MM="${RAIN_DAILY_MM:-0}"
UV="${UV:-0}"
SOLARRADIATION="${SOLARRADIATION:-0}"

DIRECTION=$(degrees_to_cardinal "$WIND_DIR")

# convert Meters Per Second (MS) to mph
WIND_SPEED_MPH=$(ms_to_mph "$WIND_SPEED_MS")
WIND_GUST_MPH=$(ms_to_mph "$WIND_GUST_MS")

# convert MS to kph
wind_kph=$(ms_to_kph "$WIND_SPEED_MS")
wind_gust_kph=$(ms_to_kph "$WIND_GUST_MS")

#Convert MB pressure to IN
pressure_in=$(mb_to_in "$pressure_mb")

#Calculate the dewpoint F/C
dewpoint_f=$(calc_dewpoint_f "$TEMP_F" "$HUMIDITY")
dewpoint_c=$(f_to_c "$dewpoint_f")

#Convert Rain MM to Inches
RAIN_DAILY_IN=$(mm_to_in "$RAIN_DAILY_MM")
precip_today_metric=$RAIN_DAILY_MM

# Final check - if TEMP_F is still empty or null after all conversions, exit with error
if [[ -z "$TEMP_F" ]] || [[ "$TEMP_F" == "null" ]]; then
	loglocal "$DATE" WEATHER ERROR "Did not get a proper outTemp, even after counter, $GETDATA"
	exit 1
fi

#ConvertDate
WX_DATE_TIME=$(date -d @$(($DATEUTC)) '+%Y-%m-%d %H:%M:%S')

#B4:E6:2D:07:87:49,1562721300000,72.9,80,66.36,22.7,0,0,224,0,0,0,1014,29.95,0,19.0,0,0,0,0,0,0,0
#LOCALSENSOR,2019-07-09 21:20,73.8,45.20,54.08,20.9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
# Return a JSON file to mimic WUNderground file format
WX_STATION_ID="$WXSTATION"
WX_OBS_TIME="$WX_DATE_TIME"
WX_TEMP_F="$TEMP_F"
WX_TEMP_C="$TEMP_C"
WX_HUMIDITY="$HUMIDITY"
WX_WIND_DIR_TEXT="$DIRECTION"
WX_WIND_DIR_DEG="$WIND_DIR"
WX_WIND_MPH="$WIND_SPEED_MPH"
WX_WIND_GUST_MPH="$WIND_GUST_MPH"
WX_WIND_KPH="$wind_kph"
WX_WIND_GUST_KPH="$wind_gust_kph"
WX_PRESSURE_MB="$pressure_mb"
WX_PRESSURE_IN="$pressure_in"
WX_PRESSURE_TREND="-"
WX_DEWPOINT_F="$dewpoint_f"
WX_DEWPOINT_C="$dewpoint_c"
WX_SOLARRADIATION="$SOLARRADIATION"
WX_UV="$UV"
WX_PRECIP_1HR_IN="NA"
WX_PRECIP_1HR_MM="NA"
WX_PRECIP_TODAY_IN="$RAIN_DAILY_IN"
WX_PRECIP_TODAY_MM="$precip_today_metric"
output_wx_json