
#!/bin/bash
# Version 1.2
# Script to output localized DHT22 and RainSensor that matches WUNDERGROUND file format
#
# 

#set -x #echo on
# Get config data
# Mainly DHT GPIO and the local Rain bucket GPIO.
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %R')

#Set some default values for the sensors that don't provide it
WXPRESSURE_MB="0"
WXPRESSURE_IN="0"


# =======================================
# Get the data from the device
# =======================================
# ------ GET TEMP/Humidity ------
if [[ $WXTEMPTYPE == "temperhum" ]]; then
	WXTEMP=`$HOMEDIR/scripts/weather/localsensors/wxtemperhum.sh`
elif [[ $WXTEMPTYPE == "dht22" ]]; then
	WXTEMP=`$HOMEDIR/scripts/weather/localsensors/wxdht22.sh `
elif [[ $WXTEMPTYPE == "dht21" ]]; then
	WXTEMP=`$HOMEDIR/scripts/weather/localsensors/wxdht21.sh`
elif [[ $WXTEMPTYPE = "sht31d" ]]; then
	WXTEMP=$($HOMEDIR/scripts/weather/localsensors/wxsht31-d.sh)
elif [[ $WXTEMPTYPE = "bme680" ]]; then
    WXTEMP=$($HOMEDIR/scripts/weather/localsensors/wxbme680.sh)
    WXPRESSURE_MB=$(echo $WXTEMP |awk '{print $5}')
    WXGAS=$(echo $WXTEMP |awk '{print $6}')
    #To convert millibars to inches of mercury, multiply the millibar value by 0.0295301
    WXPRESSURE_IN=$(echo "scale=2; ($WXPRESSURE_MB * 0.0295301)" | bc )
fi


# ------END GET TEMP/Humidity ------

# ------ GET Rain Data ------
raininhourly=0
rainmetrichour=0
rainindaily=0
rainmetricdaily=0

# ------ END GET Rain Data ------

#============================================

if [[ -z "$WXTEMP" ]] || [[ "$WXTEMP" == "" ]];  then
			loglocal "$DATE" WXAmbient  ERROR "Error connecting to Local Sensors for Ambient Weather, skipping collection..."		
		WXTEMPF="null"
		WXTEMPC="null"
		WXHUMIDITY="0"
		WXDEW="0"
		else 
			WXTEMPF=$(echo $WXTEMP |awk '{print $1}')
			WXTEMPC=$(echo $WXTEMP |awk '{print $4}')
			WXHUMIDITY=$(echo $WXTEMP |awk '{print $2}')
			WXDEW=$(echo $WXTEMP |awk '{print $3}')
fi

#Convert Rain Inches for Hour and Day to Metric
#precip_1hr_metric="$(echo "scale=2; (${rainofhourly} / 0.039370)" |bc)"
#precip_today_metric="$(echo "scale=2; (${rainofdaily} / 0.039370)" |bc)"

	
# Return a JSON file to mimic WUNderground file format
echo "{"
echo "\"response\": {"
echo "\"version\":\"0.1\","
echo "\"features\": {"
echo "\"conditions\": 1"
echo "  }"
echo "	}"
echo "  ,	\"current_observation\": {"
echo "		\"station_id\":\"LOCALSENSOR\","
echo "		\"observation_time\":\"$DATE\","
echo "		\"temperature_string\":\"$WXTEMPF F ($WXTEMPC C)\","
echo "		\"temp_f\":$WXTEMPF,"
echo "		\"temp_c\":$WXTEMPC,"
echo "		\"relative_humidity\":\"$WXHUMIDITY%\","
echo "		\"wind_string\":\"NA\","
echo "		\"wind_dir\":\"NA\","
echo "		\"wind_degrees\":\"0\","
echo "		\"wind_mph\":\"0\","
echo "		\"wind_gust_mph\":\"0\","
echo "		\"wind_kph\":\"0\","
echo "		\"wind_gust_kph\":\"0\","
echo "		\"pressure_mb\":\"$WXPRESSURE_MB\","
echo "		\"pressure_in\":\"$WXPRESSURE_IN\","
echo "		\"pressure_trend\":\"-\","
echo "		\"dewpoint_f\":$WXDEW,"
echo "		\"dewpoint_c\":0,"
echo "		\"windchill_string\":\"NA\","
echo "		\"windchill_f\":\"NA\","
echo "		\"windchill_c\":\"NA\","
echo "		\"solarradiation\":\"--\","
echo "		\"UV\":\"0\","
echo "		\"precip_1hr_in\":\"$raininhourly\","
echo "		\"precip_1hr_metric\":\"$rainmetrichour\","
echo "		\"precip_today_string\":\"NA\","
echo "		\"precip_today_in\":\"$rainindaily\","
echo "		\"precip_today_metric\":\"$rainmetricdaily\""
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
		<station_id>LOCALSENSOR</station_id>
		<temperature_string>$WXTEMPF F ($WXTEMPC C)</temperature_string>
		<temp_f>$WXTEMPF</temp_f>
		<temp_c>$WXTEMPC</temp_c>
		<relative_humidity>$WXHUMIDITY%</relative_humidity>
		<wind_string>NA</wind_string>
		<wind_dir>0</wind_dir>
		<wind_degrees>0</wind_degrees>
		<wind_mph>0</wind_mph>
		<wind_gust_mph>0</wind_gust_mph>
		<wind_kph>0</wind_kph>
		<wind_gust_kph>0</wind_gust_kph>
		<pressure_mb>$WXPRESSURE_MB</pressure_mb>
		<pressure_in>$WXPRESSURE_IN</pressure_in>
		<pressure_trend>-</pressure_trend>
		<dewpoint_f>$WXDEW</dewpoint_f>
		<dewpoint_c>0</dewpoint_c>
		<windchill_string>NA</windchill_string>
		<windchill_f>NA</windchill_f>
		<windchill_c>NA</windchill_c>
	        <solarradiation>--</solarradiation>
		<UV>0</UV>
		<precip_1hr_in>$raininhourly</precip_1hr_in>
		<precip_1hr_metric>$rainmetrichour</precip_1hr_metric>
		<precip_today_string>NA</precip_today_string>
		<precip_today_in>$rainindaily</precip_today_in>
		<precip_today_metric>$rainmetricdaily</precip_today_metric>
	</current_observation>
</response>" > wx.xml
