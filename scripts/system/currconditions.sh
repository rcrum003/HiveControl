#!/bin/bash
# Script to gather Current_Conditions to monitor beehives
# see hivetool.net
# Version 1.4

# Get Variables from central file
SHELL=/bin/bash

#Get Config parameters from DB
/home/HiveControl/scripts/data/hiveconfig.sh

# Set some basics
PATH=/usr/local/sbin:/usr/local/bin:/bin:/sbin:/usr/sbin:/usr/bin:/home/HiveControl/scripts/weather:/home/HiveControl/scripts/system
HOST=`hostname`
#Load the results of the script above
source /home/HiveControl/scripts/hiveconfig.inc
DATE=$(TZ=":$TIMEZONE" date '+%F %T')

# ------ GET HIVE WEIGHT ----
echo "--- WEIGHT --- "
if [ $ENABLE_HIVE_WEIGHT_CHK = "yes" ]; then
	#echo "Checking Weight" >> $LOG
	#Get the weight fool
	HIVEWEIGHTSRC=`$HOMEDIR/scripts/weight/getweight.sh`
	HIVEWEIGHT=$(echo $HIVEWEIGHTSRC |awk '{print $2}')
	HIVERAWWEIGHT=$(echo $HIVEWEIGHTSRC |awk '{print $1}')
fi
if [ $ENABLE_HIVE_WEIGHT_CHK = "no" ]; then
	HIVEWEIGHT=0
	HIVERAWWEIGHT=0
fi
echo "--- WEIGHT DONE ---"


# ------ GET HIVE TEMP ------
echo "--- TEMP ---"
if [ $ENABLE_HIVE_TEMP_CHK = "yes" ]; then
	# Data Fetchers/Parsers in one
	#echo "Checking TEMP" >> $LOG
	if [ $TEMPTYPE = "temperhum" ]; then
	GETTEMP=`$HOMEDIR/scripts/temp/temperhum.sh $HIVEDEVICE`
	elif [[ $TEMPTYPE = "dht22" ]]; then
	GETTEMP=`$HOMEDIR/scripts/temp/dht22.sh $HIVE_TEMP_GPIO`
	elif [[ $TEMPTYPE = "dht21" ]]; then
	GETTEMP=`$HOMEDIR/scripts/temp/dht21.sh`
	fi
	HIVETEMPF=$(echo $GETTEMP |awk '{print $1}')
	HIVETEMPC=$(echo $GETTEMP |awk '{print $4}')
	HIVEHUMIDITY=$(echo $GETTEMP |awk '{print $2}')
	HIVEDEW=$(echo $GETTEMP |awk '{print $3}')
fi
if [ $ENABLE_HIVE_TEMP_CHK = "no" ]; then
	HIVETEMPF=0
	HIVETEMPC=0
	HIVEHUMIDITY=0
	HIVEDEW=0
fi
echo "--- TEMP DONE ---"

# -------- END GET HIVE TEMP ----------

#---------- Get Ambient Weather--------
# Variables come from variable.inc
# Weather Data
echo "--- WX ---"
if [ $WEATHER_LEVEL = "hive" ]; then
echo "Getting from Wunderground"
GETNOW=`/usr/bin/curl --silent http://api.wunderground.com/api/$KEY/conditions/q/pws:$WXSTATION.json`

A_WIND_MPH=`/bin/echo $GETNOW | JSON.sh -b |grep wind_mph |awk '{print $2}'`
OBSERVATIONEPOCH=`/bin/echo $GETNOW | JSON.sh -b |grep observation_epoch |awk -F"\"" '{print $6}'`
OBSERVATIONDATETIME=`date -d @$OBSERVATIONEPOCH '+%F %T %Z'`
wind_degrees=`/bin/echo $GETNOW | JSON.sh -b |grep wind_degrees |awk  '{print $2}'`
wind_gust_mph=`/bin/echo $GETNOW | JSON.sh -b |grep wind_gust_mph |awk '{print $2}'`
wind_kph=`/bin/echo $GETNOW | JSON.sh -b |grep wind_kph |awk '{print $2}'`
wind_gust_kph=`/bin/echo $GETNOW | JSON.sh -b |grep wind_gust_kph |awk '{print $2}'`
weather_dewc=`/bin/echo $GETNOW | JSON.sh -b |grep dewpoint_c |awk '{print $2}'`

elif [ $WEATHER_LEVEL = "localws" ]; then
echo "Getting from LocalWS"
GETNOW=`$HOMEDIR/scripts/weather/ws1400/getWS1400.sh`
#Get the data fields that differ from the main set
A_WIND_MPH=`/bin/echo $GETNOW | JSON.sh -b |grep wind_mph |awk -F"\"" '{print $6}'`
OBSERVATIONDATETIME=`/bin/echo $GETNOW | JSON.sh -b |grep observation_time |awk -F"\"" '{print $6}'`
wind_degrees=`/bin/echo $GETNOW | JSON.sh -b |grep wind_degrees |awk -F"\"" '{print $6}'`
wind_gust_mph=`/bin/echo $GETNOW | JSON.sh -b |grep wind_gust_mph |awk -F"\"" '{print $6}'`
wind_kph=`/bin/echo $GETNOW | JSON.sh -b |grep wind_kph |awk -F"\"" '{print $6}'`
wind_gust_kph=`/bin/echo $GETNOW | JSON.sh -b |grep wind_gust_kph |awk -F"\"" '{print $6}'`
weather_dewc=`/bin/echo $GETNOW | JSON.sh -b |grep dewpoint_c |awk -F"\"" '{print $6}'`

elif [ $WEATHER_LEVEL = "localsensors" ]; then
echo "Getting from LocalSenrors"
GETNOW=`$HOMEDIR/scripts/weather/localsensors/localsensors.sh`
#Get the data fields that differ from the main set
A_WIND_MPH=`/bin/echo $GETNOW | JSON.sh -b |grep wind_mph |awk -F"\"" '{print $6}'`
OBSERVATIONDATETIME=`/bin/echo $GETNOW | JSON.sh -b |grep observation_time |awk -F"\"" '{print $6}'`
wind_degrees=`/bin/echo $GETNOW | JSON.sh -b |grep wind_degrees |awk -F"\"" '{print $6}'`
wind_gust_mph=`/bin/echo $GETNOW | JSON.sh -b |grep wind_gust_mph |awk -F"\"" '{print $6}'`
wind_kph=`/bin/echo $GETNOW | JSON.sh -b |grep wind_kph |awk -F"\"" '{print $6}'`
wind_gust_kph=`/bin/echo $GETNOW | JSON.sh -b |grep wind_gust_kph |awk -F"\"" '{print $6}'`
weather_dewc=`/bin/echo $GETNOW | JSON.sh -b |grep dewpoint_c |awk -F"\"" '{print $6}'`

fi

#Parse the weather 
# Data Parsers for all weather sources
A_TEMP=`/bin/echo $GETNOW | JSON.sh -b |grep temp_f |awk '{print $2}'`
A_TEMP_C=`/bin/echo $GETNOW | JSON.sh -b |grep temp_c |awk '{print $2}'`
A_TIME=`/bin/echo $GETNOW | JSON.sh -b |grep observation_epoch |awk -F"\"" '{print $6}'`
A_HUMIDITY=`/bin/echo $GETNOW | JSON.sh -b |grep relative_humidity |awk -F"\"" '{print $6}'`
B_HUMIDITY=`/bin/echo $A_HUMIDITY | grep -o "\-*[0-9]*\.*[0-9]*"`
A_WIND_DIR=`/bin/echo $GETNOW | JSON.sh -b |grep wind_dir |awk -F"\"" '{print $6}'`
A_PRES_IN=`/bin/echo $GETNOW | JSON.sh -b |grep pressure_in |awk -F"\"" '{print $6}'`
A_PRES_TREND=`/bin/echo $GETNOW | JSON.sh -b |grep pressure_trend |awk -F"\"" '{print $6}'`
A_DEW=`/bin/echo $GETNOW | JSON.sh -b |grep dewpoint_f |awk '{print $2}'`
WEATHER_STATIONID=`/bin/echo $GETNOW | JSON.sh -b |grep station_id |awk '{print $2}'`
pressure_mb=`/bin/echo $GETNOW | JSON.sh -b |grep pressure_mb |awk -F"\"" '{print $6}'`
UV=`/bin/echo $GETNOW | JSON.sh -b |grep UV |awk -F"\"" '{print $6}'`
precip_1hr_in=`/bin/echo $GETNOW | JSON.sh -b |grep precip_1hr_in |awk -F"\"" '{print $6}'`
precip_1hr_metric=`/bin/echo $GETNOW | JSON.sh -b |grep precip_1hr_metric |awk -F"\"" '{print $6}'`
precip_today_string=`/bin/echo $GETNOW | JSON.sh -b |grep precip_today_string |awk -F"\"" '{print $6}'`
precip_today_in=`/bin/echo $GETNOW | JSON.sh -b |grep precip_today_in |awk -F"\"" '{print $6}'`
precip_today_metric=`/bin/echo $GETNOW | JSON.sh -b |grep precip_today_metric |awk -F"\"" '{print $6}'`
solarradiation=`/bin/echo $GETNOW | JSON.sh -b |grep solarradiation |awk -F"\"" '{print $6}'`
lux="0"

#Check to see if solarradiation is not set from the weather stations
if [[ $solarradiation = "--" ]]; then
	echo "Solarradiation did not give us a value, so setting to zero"
	solarradiation="0"
fi

# ------ GET LUX -----------
# Two ways to get Light Levels
# From a local sensor on the hive, OR
# from a local weather station
echo "--- LUX --- "
if [ $ENABLE_LUX = "yes" ]; then
		#echo "Getting Lux" >> $LOG
		if [ $LUX_SOURCE = "tsl2591" ]; then
			echo "getting LUX from tsl2591"
			lux=$($HOMEDIR/scripts/light/tsl2591.sh)
		elif [[ $LUX_SOURCE = "tsl2561" ]]; then
			echo "Sorry, we don't have a script for TSL2561"
			lux=$($HOMEDIR/scripts/light/tsl2561.sh)
		elif [ $LUX_SOURCE = "wx" ]; then
			echo "getting solarradiation from weatherstation"
			lux="0"
		fi
elif [ $ENABLE_LUX = "no" ]; then
	echo "Not getting LUX - set to no"
fi
		
echo "--- LUX DONE --- "


# ----------- END GET Ambient Weather ------------
#echo "Storing Data in our database"
sqlite3 $HOMEDIR/data/hive-data.db "insert into allhivedata (hiveid,date,hivetempf,hivetempc,hiveHum,hiveweight,hiverawweight,yardid,sync,beekeeperid,weather_stationID,observationDateTime,weather_tempf,weather_humidity,weather_dewf,weather_tempc,wind_mph,wind_dir,wind_degrees,wind_gust_mph,wind_kph,wind_gust_kph,pressure_mb,pressure_in,pressure_trend,weather_dewc,solarradiation,UV,precip_1hr_in,precip_1hr_metric,precip_today_string,precip_today_in,precip_today_metric,lux) \
values (\"$HIVEID\",\"$DATE\",\"$HIVETEMPF\",\"$HIVETEMPC\",\"$HIVEHUMIDITY\",\"$HIVEWEIGHT\",\"$HIVERAWWEIGHT\",\"$YARDID\",1,\"$BEEKEEPERID\", $WEATHER_STATIONID,'$OBSERVATIONDATETIME',$A_TEMP,$B_HUMIDITY,$A_DEW,$A_TEMP_C,'$A_WIND_MPH','$A_WIND_DIR','$wind_degrees','$wind_gust_mph','$wind_kph','$wind_gust_kph','$pressure_mb','$A_PRES_IN','$A_PRES_TREND','$weather_dewc','$solarradiation','$UV','$precip_1hr_in','$precip_1hr_metric','$precip_today_string','$precip_today_in','$precip_today_metric','$lux');"
#echo "Success AAD"


#-------------------------------------
# If sharing, create file and send to other people
#-------------------------------------
if [ $SHARE_HIVETOOL = "yes" ]; then
echo "Sending to Hivetool"
	# Create XML file, since that is what they like to get
	SAVEFILE=$HOMEDIR/scripts/system/transmit.xml
	echo "<hive_data>" > $SAVEFILE
	echo "        <hive_observation>" >> $SAVEFILE
	echo "                <hive_id>$HIVENAME</hive_id>" >> $SAVEFILE
	echo "                <hive_observation_time>$DATE</hive_observation_time>" >> $SAVEFILE
	echo "                <hive_weight_lbs>$HIVERAWWEIGHT</hive_weight_lbs>" >> $SAVEFILE
	echo "                <hive_temp_c>$HIVETEMPC</hive_temp_c>" >> $SAVEFILE
	echo "                <hive_relative_humidity>$HIVEHUMIDITY</hive_relative_humidity>" >> $SAVEFILE
	echo "                <hive_ambient_temp_c>$A_TEMP_C</hive_ambient_temp_c>" >> $SAVEFILE
	echo "                <hive_ambient_relative_humidity>$B_HUMIDITY</hive_ambient_relative_humidity>" >> $SAVEFILE
	echo "        </hive_observation>" >> $SAVEFILE
	# hivetool likes to get straight wunderground data
	# so we make another call, TODO will be to parse the JSON we already collected, and send them XML
	# Maybe we can convince them to support JSON as well
        if [ $WEATHER_LEVEL = "hive" ]; then
	/usr/bin/curl --silent http://api.wunderground.com/api/$KEY/conditions/q/pws:$WXSTATION.xml > $HOMEDIR/scripts/system/wx.xml
	fi
	if [ $WEATHER_LEVEL = "localws" ]; then
	echo "Local ws Send"
	rm -rf $HOMEDIR/scripts/system/wx.xml
	cp $HOMEDIR/scripts/weather/ws1400/wx.xml $HOMEDIR/scripts/system/wx.xml 
	fi
	/usr/bin/xmlstarlet sel -t -c "/response/current_observation" $HOMEDIR/scripts/system/wx.xml >> $SAVEFILE
	echo "</hive_data>" >> $SAVEFILE


#====================
# Try to send to hivetool
#====================
/usr/bin/curl --silent --retry 5 -k -u $HT_USERNAME:$HT_PASSWORD -X POST --data-binary @$SAVEFILE https://hivetool.org/private/log_hive.pl  -H 'Accept: application/xml' -H 'Content-Type: application/xml' 1>$HOMEDIR/logs/hivetool-error.log
	

fi

# End Sharing
