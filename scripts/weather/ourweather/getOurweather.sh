#!/bin/bash
# Version 2020052801
# Script to pull Ourweather (local) station's data
# C. Duncan Hudson
#
#
# 05/22/20 V 1   - Created to pull REST data from Switchdoc's Ourweather weather station, and convert to Wunderground output
#                  format to allow Hivecontrol to monitor.
# 05/26/20 V 1.1 - Changed to support metric to English as REST interface always returns metric.
# 05/26/20 V 1.2 - Neglected to add wx.xml file for hivetool	
# 5/27/20 Version 2020052701 - Adapted to HiveControl Code Standards, removed use of perl

#Get our variables

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc

URL="http://$local_wx_url"

DATE=$(TZ=":$TIMEZONE" date '+%F %R')

TRYCOUNTER="1" 
DATA_GOOD="0" 

# Poll the weather station
while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do

	GETDATA=$(/usr/bin/curl --silent "$URL" | )
	CHECKERROR=$?

	if [ "$CHECKERROR" -ne "0"  ]; then
		loglocal "$DATE" WEATHER INFO "Did not get a proper response from Ourweather, trying again"
		let TRYCOUNTER+=1
		rm -rf $TMPFILE
	else
		DATA_GOOD=1
	fi
done

#Station supports reporting in Metric or English
IS_METRIC_OR_ENGLISH=$(/bin/echo $GETDATA | jq --raw-output '.variables.EnglishOrMetric')

# Get needed info from the weather station's JSON
DATEUTC=$(/bin/echo $GETDATA | jq --raw-output '.variables.OurWeatherTime' )
#WIND_MAX_DAILY_GUST=$(/bin/echo $GETDATA | jq '.obs[] | .[0]')
TEMP_C=$(/bin/echo $GETDATA | jq --raw-output '.variables.OutdoorTemperature')
HUMIDITY=$(/bin/echo $GETDATA | jq --raw-output '.variables.OutdoorHumidity')
INDOOR_TEMP_C=$(/bin/echo $GETDATA | jq --raw-output '.variables.IndoorTemperature')
PRESSURE_PA=$(/bin/echo $GETDATA | jq --raw-output '.variables.BarometricPressure')
ALTITUDE=$(/bin/echo $GETDATA | jq --raw-output '.variables.Altitude')
WIND_SPEED_KPH=$(/bin/echo $GETDATA | jq --raw-output '.variables.CurrentWindSpeed')
WIND_GUST_KPH=$(/bin/echo $GETDATA | jq --raw-output '.variables.CurrentWindGust')
WIND_DIR=$(/bin/echo $GETDATA | jq --raw-output '.variables.CurrentWindDirection')
RAIN_DAILY_MM=$(/bin/echo $GETDATA | jq --raw-output '.variables.RainTotal' ) #RainTotal is reporting since "lastreboot"
WIND_SPEED_KPH_MIN=$(/bin/echo $GETDATA | jq --raw-output '.variables.WindSpeedMin')
WIND_SPEED_KPH_MAX=$(/bin/echo $GETDATA | jq --raw-output '.variables.WindSpeedMax' )
WIND_GUST_KPH_MIN=$(/bin/echo $GETDATA | jq --raw-output '.variables.WindGustMin')
WIND_GUST_KPH_MAX=$(/bin/echo $GETDATA | jq --raw-output '.variables.WindGustMax' )
WIND_DIR_MIN=$(/bin/echo $GETDATA | jq --raw-output '.variables.WindDirectionMin')
WIND_DIR_MAX=$(/bin/echo $GETDATA | jq --raw-output '.variables.WindDirectionMax')
OBSERVATION_DATE=$(/bin/echo $GETDATA | jq --raw-output '.variables.OurWeatherTime')
WXSTATION=$(/bin/echo $GETDATA | jq --raw-output '.name') #Can be empty
if [[ -z "$WXSTATION" ]]; then
	#No Workstation name
	WXSTATION=$local_wx_url
fi
SOLARRADIATION=$(/bin/echo $GETDATA | jq --raw-output '.variables.Sunlight')
AIR_QUALITY=$(/bin/echo $GETDATA | jq --raw-output '.variables.AirQualitySensor')

# Bail out if we don't have a basic temperature
if [[ -z "$TEMP_C" ]]; then
	ERROR_MSG=$(sed 's/\"//g' <<<"$GETDATA")
	loglocal "$DATE" WEATHER ERROR "Did not get a proper Temperature, even after counter, $ERROR_MSG"
	#Since we didn't get a proper file, we should assume the whole thing is borked
	exit
fi

# Do some necessary conversions 
# Temperature
TEMP_F=`echo "scale=1; ($TEMP_C * 9 / 5) + 32" |bc`

# Pressure
((PRESSURE_MB=$PRESSURE_PA / 100))
PRESSURE_HG=`echo "scale=2; $PRESSURE_MB / 33.8637526" | bc`
# Speed
WIND_SPEED_MPH=`echo "scale=2; ($WIND_SPEED_KPH / 1.609344)" | bc`
WIND_GUST_MPH=`echo "scale=2; ($WIND_GUST_KPH / 1.609344)" | bc`
# Depth
RAIN_DAILY_IN=`echo "scale=2; ($RAIN_DAILY_MM * 25.4)" | bc`

# Derive some new variables
DEWPOINT_F=`echo "scale=1; $TEMP_F - (100-$HUMIDITY) * 9 / 25" | bc`
DEWPOINT_C=`echo "scale=1; ($DEWPOINT_F-32) * 5 / 9" | bc`

function calc_windchill_f {
    #Using BC for advanced math, if you use the -l switch
    to="$TEMP_F"
    v="$WIND_SPEED_MPH"
    vel=$(echo 'e(l('$v')*0.16)' | bc -l)
    w=$(echo "scale=3; $vel * 1" | bc -l)
    twc=$(echo "scale=0; (35.74 + (0.6215 * $to) - (35.75 * $w) + (0.4275 * $to * $w)) / 1" | bc)
    echo "$twc"
}

#If Temp is less than 70F, then no windchill
if [[ $TEMP_F < 70 ]]; then
	#Only calculate if less than 70F
	if [ $WIND_KPH > 4.8 ]; then
		#Only calculate if wind is greater than 4.8 KPH
		WINDCHILL_F=calc_windchill_f 
   		WINDCHILL_C=$(echo "scale=0;(($WINDCHILL_F-32)*5)/9" | bc -l)
	else
		   WINDCHILL_C=$TEMP_C
   		   WINDCHILL_F=$TEMP_F
	fi
else
   WINDCHILL_C=$TEMP_C
   WINDCHILL_F=$TEMP_F
fi


# Convert Wind Direction (Degrees) into corresponding cardinal point
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
echo "		\"temperature_string\":\"$TEMP_F F ($TEMP_C C)\","
echo "		\"temp_f\":\"$TEMP_F\","
echo "		\"temp_c\":\"$TEMP_C\","
echo "		\"relative_humidity\":\"$HUMIDITY%\","
echo "		\"wind_string\":\" $DIRECTION at $WIND_SPEED_MPH MPH, Gust to $WIND_GUST_MPH MPH\","
echo "		\"wind_dir\":\"$DIRECTION\","
echo "		\"wind_degrees\":\"$WIND_DIR\","
echo "		\"wind_mph\":\"$WIND_SPEED_MPH\","
echo "		\"wind_gust_mph\":\"$WIND_GUST_MPH\","
echo "		\"wind_kph\":\"$WIND_SPEED_KPH\","
echo "		\"wind_gust_kph\":\"$WIND_GUST_KPH\","
echo "		\"pressure_mb\":\"$PRESSURE_MB\","
echo "		\"pressure_in\":\"$PRESSURE_HG\","
echo "		\"pressure_trend\":\"NA\","
echo "		\"dewpoint_f\":\"$DEWPOINT_F\","
echo "		\"dewpoint_c\":\"$DEWPOINT_C\","
echo "		\"windchill_string\":\"NA\","
echo "		\"windchill_f\":\"$WINDCHILL_F\","
echo "		\"windchill_c\":\"$WINDCHILL_C\","
echo "		\"solarradiation\":\"$SOLARRADIATION\","
echo "		\"UV\":\"NA\","
echo "		\"precip_1hr_in\":\"NA\","
echo "		\"precip_1hr_metric\":\"NA\","
echo "		\"precip_today_string\":\"$RAIN_DAILY_IN in ($RAIN_DAILY_MM)\","
echo "		\"precip_today_in\":\"$RAIN_DAILY_IN\","
echo "		\"precip_today_metric\":\"$RAIN_DAILY_MM\""
echo " }"
echo "}"

# Generate an XML file for use by Hivetool, since they like that also

echo "
<response>
  <current_observation>
		<display_location>
		<full>$CITY, $STATE</full>
		<city>$CITY</city>
		<state>$STATE</state>
		<country>US</country>
		<latitude>$LATITUDE</latitude>
		<longitude>$LONGITUDE</longitude>
		</display_location>
		<station_id>$WXSTATION</station_id>
		<temperature_string>$TEMP_F F $TEMP_C C)</temperature_string>
		<temp_f>$TEMP_F</temp_f>
		<temp_c>$TEMP_C</temp_c>
		<relative_humidity>$HUMIDITY%</relative_humidity>
		<wind_string>$DIRECTION at $WIND_SPEED_MPH MPH, Gust to $WIND_GUST_MPH</wind_string>
		<wind_dir>$DIRECTION</wind_dir>
		<wind_degrees>$WINDDIR</wind_degrees>
		<wind_mph>$WIND_SPEED_MPH</wind_mph>
		<wind_gust_mph>$WIND_GUST_MPH</wind_gust_mph>
		<wind_kph>$WIND_SPEED_KPH</wind_kph>
		<wind_gust_kph>$WIND_GUST_KPH</wind_gust_kph>
		<pressure_mb>$PRESSURE_MB</pressure_mb>
		<pressure_in>$PRESSURE_HG</pressure_in>
		<pressure_trend>-</pressure_trend>
		<dewpoint_f>$DEWPOINT_F</dewpoint_f>
		<dewpoint_c>$DEWPOINT_C</dewpoint_c>
		<windchill_string>NA</windchill_string>
		<windchill_f>$WINDCHILL_F</windchill_f>
		<windchill_c>$WINDCHILL_C</windchill_c>
	        <solarradiation>$SOLARRADIATION</solarradiation>
		<UV>NA</UV>
		<precip_1hr_in>NA</precip_1hr_in>
		<precip_1hr_metric>NA</precip_1hr_metric>
		<precip_today_string>$RAIN_DAILY_IN in ($RAIN_DAILY_MM mm)</precip_today_string>
		<precip_today_in>$RAIN_DAILY_IN</precip_today_in>
		<precip_today_metric>$RAIN_DAILY_MM</precip_today_metric>
	</current_observation>
</response>" > wx.xml