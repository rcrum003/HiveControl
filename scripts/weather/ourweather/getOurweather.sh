
#!/bin/bash
# Version 1
# Script to pull Ourweather (local) station's data
# C. Duncan Hudson
#
#
# 05/22/20 V 1   - Created to pull REST data from Switchdoc's Ourweather weather station, and convert to Wunderground output
#                  format to allow Hivecontrol to monitor.
# 05/26/20 V 1.1 - Changed to support metric to English as REST interface always returns metric.
# 05/26/20 V 1.2 - Neglected to add wx.xml file for hivetool	

#Get our variables

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/check.inc

URL="http://$local_wx_url"


DATE=$(TZ=":$TIMEZONE" date '+%F %R')

TRYCOUNTER="1" 
DATA_GOOD="0" 
TMPFILE="/home/HiveControl/scripts/weather/ourweather/weather.tmp"

#Remove old file
rm -rf $TMPFILE

# Poll the weather station
while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do
	/usr/bin/curl --retry 5 $URL > $TMPFILE
	CHECKERROR=$?

	if [ "$CHECKERROR" -ne "0"  ]; then
		loglocal "$DATE" WEATHER INFO "Did not get a proper response from ourweather, trying again"
		let TRYCOUNTER+=1
	else
		DATA_GOOD=1
	fi
done

# Get needed info from the weather station's JSON
DATEUTC=$DATE
TEMP_C=`cat $TMPFILE | jq '.variables.OutdoorTemperature'`
HUMIDITY=`cat $TMPFILE | jq '.variables.OutdoorHumidity'`
WIND_DIR=`cat $TMPFILE | jq '.variables.CurrentWindDirection'`
WIND_SPEED_KPH=`cat $TMPFILE | jq '.variables.CurrentWindSpeed'`
WIND_GUST_KPH=`cat $TMPFILE | jq '.variables.CurrentWindGust'`
PRESSURE_PA=`cat $TMPFILE | jq '.variables.BarometricPressure'`
RAIN_DAILY_MM=`cat $TMPFILE | jq '.variables.RainTotal'`
SOLAR_RADIATION=`cat $TMPFILE | jq '.variables.Sunlight'`

# Bail out if we don't have a basic temperature
if [[ -z "$TEMP_C" ]]; then
	loglocal "$DATE" WEATHER ERROR "Did not get a proper outTemp, even after counter, $GETDATA"
	#Since we didn't get a proper file, we should assume the whole thing is borked
	exit
fi

# Do some necessary conversions 
# Temperature
TEMP_F=`echo "scale=1; ($TEMP_C * 9 / 5) + 32" |bc`

# Pressure
((PRESSURE_MB=$PRESSURE_PA / 100))
PRESSURE_HG=`echo "scale=2; $PRESSURE_MB / 33.864" | bc`
# Speed

WIND_SPEED_MPH=`echo "scale=2; ($WIND_SPEED_KPH / 1.609344)" | bc`
WIND_GUST_MPH=`echo "scale=2; ($WIND_GUST_KPH / 1.609344)" | bc`
# Depth
RAIN_DAILY_IN=`echo "scale=2; ($RAIN_DAILY_MM * 25.4)" | bc`

# Derive some new variables
DEWPOINT_F=`echo "scale=1; $TEMP_F - (100-$HUMIDITY) * 9 / 25" | bc`
DEWPOINT_C=`echo "scale=1; ($DEWPOINT_F-32) * 5 / 9" | bc`
# bash can't do exponents & bc can only do integer exponents, so we need to call a more powerful utility to deal with this math
if [ $WIND_KPH > 4.8 ]; then
   WINDCHILL_C=`perl -e 'print 13.12 + (.6215 * $TEMP_C) - (11.37 * $WIND_KPH ** 0.16) + (.3965 * $TEMP_C * $WIND_KPH ** 0.16)'`
   WINDCHILL_F=`echo "scale=1; $WINDCHILL_C * 9 / 5 + 32" | bc`
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
echo "		\"station_id\":\"$OURWXIP\","
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
echo "		\"dewpoint_f\":$DEWPOINT_F,"
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
		<station_id>$OURWXIP</station_id>
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
	        <solarradiation>${solarrad}</solarradiation>
		<UV>${uvi}</UV>
		<precip_1hr_in>NA</precip_1hr_in>
		<precip_1hr_metric>NA</precip_1hr_metric>
		<precip_today_string>$RAIN_DAILY_IN} in ($RAIN_DAILY_MM mm)</precip_today_string>
		<precip_today_in>$RAIN_DAILY_IN</precip_today_in>
		<precip_today_metric>$RAIN_DAILY_MM</precip_today_metric>
	</current_observation>
</response>" > wx.xml

