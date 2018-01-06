#!/bin/bash
# Script to gather Current_Conditions to monitor beehives
# see hivecontrol.org
# Author: Ryan Crum
# Version 2.5

#############################################
# Get Config parameters from DB and set some basics
#############################################
clear

echo "##########################################################"
echo "# Starting HiveControl Current Conditions Collection #"
echo "##########################################################"
echo "Getting Latest Configurations"
echo ""

SHELL=/bin/bash
/home/HiveControl/scripts/data/hiveconfig.sh


# Set some basics
PATH=/usr/local/sbin:/usr/local/bin:/bin:/sbin:/usr/sbin:/usr/bin:/home/HiveControl/scripts/weather:/home/HiveControl/scripts/system
HOST=`hostname`
#Load the results of the script above
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/check.inc
source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/cloud.inc


DATE=$(TZ=":$TIMEZONE" date '+%F %T')

echo "##########################################################"


############################################################################################################
# Used to determine if we should run or not - allows us to pause the collection during manipulation
############################################################################################################
if [ $RUN = "no" ]; then
	loglocal "$DATE" MAIN INFO "Execution of main script is disabled - renable in System Commands screen"
	echo "This script has been disabled in the UI, under System Commands"
	echo "Exiting...."
	exit;
fi

####################################################################################
# ------ GET HIVE WEIGHT ----
####################################################################################
	
	if [ $ENABLE_HIVE_WEIGHT_CHK = "yes" ]; then
		echo " --- GETTING WEIGHT ---      "
		HIVEWEIGHTSRC=$($HOMEDIR/scripts/weight/getweight.sh)
		HIVEWEIGHT=$(echo $HIVEWEIGHTSRC |awk '{print $2}')
		HIVERAWWEIGHT=$(echo $HIVEWEIGHTSRC |awk '{print $1}')
		check HIVEWEIGHTSRC
		check HIVEWEIGHT
		check HIVERAWWEIGHT		
	echo "Gross=$HIVERAWWEIGHT, Net=$HIVEWEIGHT "
		#echo "--- WEIGHT DONE ---"
	else
		echo " --- WEIGHT CHECK DISABLED--- 	"
		HIVEWEIGHT="null"
		HIVERAWWEIGHT="null"
	fi
	echo "		"
	echo "##########################################################"
##################################################################
# ------ GET HIVE TEMP ------
##################################################################
	if [ $ENABLE_HIVE_TEMP_CHK = "yes" ]; then
		echo " --- GETTING HIVE TEMP ---	 "
		HIVETEMPSRC=$($HOMEDIR/scripts/temp/gettemp.sh)
		HIVETEMPF=$(echo $HIVETEMPSRC |awk '{print $1}')
		HIVETEMPC=$(echo $HIVETEMPSRC |awk '{print $4}')
		HIVEHUMIDITY=$(echo $HIVETEMPSRC |awk '{print $2}')
		HIVEDEW=$(echo $HIVETEMPSRC |awk '{print $3}')
		echo "Calculated: TEMPF=$HIVETEMPF, TEMPC=$HIVETEMPC, HUMI=$HIVEHUMIDITY"
	else
		echo "--- HIVE TEMP DISABLED ---"
		HIVETEMPF="null"
		HIVETEMPC="null"
		HIVEHUMIDITY="null"
		HIVEDEW="null"
	fi
	#echo "--- TEMP DONE ---"
	echo "		"
	echo "##########################################################"

##########################################################################################
# --------- Get Beecount -------------
##########################################################################################
if [[ $ENABLE_BEECOUNTER = "yes" ]]; then
	echo "--- Counting Bee Flights ---"
	INOUT=$($HOMEDIR/scripts/beecount/countbees.sh)
	#INOUT=$($HOMEDIR/scripts/beecounter/countbees.sh)
	IN_COUNT=$(echo $INOUT | awk -F, '{print $1}')
	OUT_COUNT=$(echo $INOUT | awk -F, '{print $2}')
	check IN_COUNT
	check OUT_COUNT
	echo "IN=$IN_COUNT, OUT=$OUT_COUNT"
	#echo "--- Counting Bee Flights Done ---"
else
	IN_COUNT="null"
	OUT_COUNT="null"
fi
	echo "		"
	echo "##########################################################"


##################################################################
#---------- Get Ambient Weather--------
# Variables come from variable.inc
# Weather Data
##################################################################
	echo "--- Getting Ambient Weather ---"
	WXSOURCE=$($HOMEDIR/scripts/weather/getwx.sh)
	
	WEATHER_STATIONID=$(echo $WXSOURCE | awk -F, '{print $1}')
	OBSERVATIONDATETIME=$(echo $WXSOURCE | awk -F, '{print $2}')
	A_TEMP=$(echo $WXSOURCE | awk -F, '{print $3}')
	B_HUMIDITY=$(echo $WXSOURCE | awk -F, '{print $4}')
	A_DEW=$(echo $WXSOURCE | awk -F, '{print $5}')
	A_TEMP_C=$(echo $WXSOURCE | awk -F, '{print $6}')
	A_WIND_MPH=$(echo $WXSOURCE | awk -F, '{print $7}')
	A_WIND_DIR=$(echo $WXSOURCE | awk -F, '{print $8}')
	wind_degrees=$(echo $WXSOURCE | awk -F, '{print $9}')
	wind_gust_mph=$(echo $WXSOURCE | awk -F, '{print $10}')
	wind_kph=$(echo $WXSOURCE | awk -F, '{print $11}')
	wind_gust_kph=$(echo $WXSOURCE | awk -F, '{print $12}')
	pressure_mb=$(echo $WXSOURCE | awk -F, '{print $13}')
	A_PRES_IN=$(echo $WXSOURCE | awk -F, '{print $14}')
	A_PRES_TREND=$(echo $WXSOURCE | awk -F, '{print $15}')
	weather_dewc=$(echo $WXSOURCE | awk -F, '{print $16}')
	solarradiation=$(echo $WXSOURCE | awk -F, '{print $17}')
	UV=$(echo $WXSOURCE | awk -F, '{print $18}')
	precip_1hr_in=$(echo $WXSOURCE | awk -F, '{print $19}')
	precip_1hr_metric=$(echo $WXSOURCE | awk -F, '{print $20}')
	precip_today_string=$(echo $WXSOURCE | awk -F, '{print $21}')
	precip_today_in=$(echo $WXSOURCE | awk -F, '{print $22}')
	precip_today_metric=$(echo $WXSOURCE | awk -F, '{print $23}')
	
	echo "$WEATHER_STATIONID,'$OBSERVATIONDATETIME', $A_TEMP ,$B_HUMIDITY,$A_DEW,$A_TEMP_C,'$A_WIND_MPH','$A_WIND_DIR','$wind_degrees','$wind_gust_mph','$wind_kph','$wind_gust_kph','$pressure_mb','$A_PRES_IN','$A_PRES_TREND','$weather_dewc','$solarradiation','$UV','$precip_1hr_in','$precip_1hr_metric','$precip_today_string','$precip_today_in','$precip_today_metric'"
	
	echo "		"
	echo "##########################################################"

######################################################################
# ------ GET LUX -----------
# Two ways to get Light Levels
# From a local sensor on the hive, OR
# from a local weather station
######################################################################
	if [ $ENABLE_LUX = "yes" ]; then
		echo "--- GETTING LUX --- "
		lux=$($HOMEDIR/scripts/light/getlux.sh)
	fi
	check lux
	echo "Calculated LUX = $lux"
	#echo "--- LUX DONE --- "
	echo "		"
	echo "##########################################################"


######################################################################
# Storing Data in our database
######################################################################
echo "--- Storing in the Database ---"
#set -x
sqlite3 $HOMEDIR/data/hive-data.db "insert into allhivedata (hiveid,date,hivetempf,hivetempc,hiveHum,hiveweight,hiverawweight,yardid,sync,beekeeperid,weather_stationID,observationDateTime,weather_tempf,weather_humidity,weather_dewf,weather_tempc,wind_mph,wind_dir,wind_degrees,wind_gust_mph,wind_kph,wind_gust_kph,pressure_mb,pressure_in,pressure_trend,weather_dewc,solarradiation,UV,precip_1hr_in,precip_1hr_metric,precip_today_string,precip_today_in,precip_today_metric,lux,IN_COUNT,OUT_COUNT) \
values (\"$HIVEID\",\"$DATE\",\"$HIVETEMPF\",\"$HIVETEMPC\",\"$HIVEHUMIDITY\",\"$HIVEWEIGHT\",\"$HIVERAWWEIGHT\",\"$YARDID\",1,\"$BEEKEEPERID\", \"$WEATHER_STATIONID\",\"$OBSERVATIONDATETIME\",\"$A_TEMP\",\"$B_HUMIDITY\",\"$A_DEW\",\"$A_TEMP_C\",\"$A_WIND_MPH\",\"$A_WIND_DIR\",\"$wind_degrees\",\"$wind_gust_mph\",\"$wind_kph\",\"$wind_gust_kph\",\"$pressure_mb\",\"$A_PRES_IN\",\"$A_PRES_TREND\",\"$weather_dewc\",\"$solarradiation\",\"$UV\",\"$precip_1hr_in\",\"$precip_1hr_metric\",\"$precip_today_string\",\"$precip_today_in\",\"$precip_today_metric\",\"$lux\",\"$IN_COUNT\",\"$OUT_COUNT\");"
	#echo "Success AAD"
#echo "--- Storing in DB DONE ---"
echo "		"

#Get last DB record so we can send to hivecontrol.org in order
record_id=$(sqlite3 $HOMEDIR/data/hive-data.db "select id from allhivedata WHERE date = \"$DATE\";")


######################################################################
# If sharing, create file and send to other people
######################################################################
if [ $SHARE_HIVETOOL = "yes" ]; then
	echo "--- Sharing with Hivetool.org ---"
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
		#/usr/bin/curl --silent http://api.wunderground.com/api/$KEY/conditions/q/pws:$WXSTATION.xml > $HOMEDIR/scripts/system/wx.xml
		/usr/bin/curl --silent --retry 5 http://api.wunderground.com/weatherstation/WXCurrentObXML.asp?ID=$WXSTATION > $HOMEDIR/scripts/system/wx.xml
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
	/usr/bin/curl --silent --retry 5 -k -u $HT_USERNAME:$HT_PASSWORD -X POST --data-binary @$SAVEFILE https://hivetool.org/private/log_hive.pl  -H 'Accept: application/xml' -H 'Content-Type: application/xml'
		
fi


#### Development only, will enable in future versions, in QA

if [ -z "$HIVEAPI" ]; then
	echo "--- Unable to share with hivecontrol.org because HIVEAPI is not set"
else
	echo "--- Sharing with hivecontrol.org ---"
	## https://www.hivecontrol.org/api/v1/hive/check


	#################################################################
	# Try posting with our current hive_id
	# If it exists, it'll post.
	# We'll also be taking the hive_id setting away from the user
	##################################################################

	function SendData {	
	#Finish sending the fields.
	SHARE_API_STATUS=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hive_id=$HIVEID" -F "hivename=$HIVENAME" -F "hive_observation_time_local=$DATE" -F "hive_temp_f=$HIVETEMPF" -F "hive_temp_c=$HIVETEMPC" -F "hive_humidity=$HIVEHUMIDITY" -F "hive_weight_lbs=$HIVEWEIGHT" -F "hive_flight_in=$IN_COUNT" -F "hive_flight_out=$OUT_COUNT" -F "wx_station_id=$WEATHER_STATIONID" -F "wx_observation_time_rfc822=$OBSERVATIONDATETIME" -F "wx_temp_f=$A_TEMP" -F "wx_relative_humidity=$B_HUMIDITY" -F "wx_dewpoint_f=$A_DEW" -F "wx_temp_c=$A_TEMP_C" -F "wx_wind_mph=$A_WIND_MPH" -F "wx_wind_dir=$A_WIND_DIR" -F "wx_wind_degrees=$wind_degrees" -F "wx_wind_gust_mpg=$wind_gust_mph" -F "wx_pressure_mb=$pressure_mb" -F "wx_pressure_in=$A_PRES_IN" -F "wx_dewpoint_c=$weather_dewc" -F "wx_solar_radiation=$solarradiation" -F "wx_precip_1hr_in=$precip_1hr_in" -F "wx_precip_1hr_metric=$precip_1hr_metric" -F "wx_precip_today_in=$precip_today_in" -F "wx_precip_today_metric=$precip_today_metric" -F "record_id=$record_id" "$POST_DATA_URL")

	#Parse Various Response and set SHARE_API_STATUS
		# Check to see if the status was Unauthenticated	
		SHARE_SUB_STATUS=$(/bin/echo $SHARE_API_STATUS | $HOMEDIR/scripts/system/JSON.sh -b |awk -F\" '{print $4}' |awk -F, '{print $1}')


	#Depending on STATUS Number, do something with it
	case $SHARE_SUB_STATUS in
		NOT_REGISTERED)
			#Register our hive now
				#You look like a new hive, so we will register you
				echo "trying to register hive"
				REG_HIVE_STATUS=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hive_id=$HIVEID" -F "name=$HIVENAME" -F "hc_version=$HCVersion" -F "timezone=$TIMEZONE" -F "power=$POWER" -F "internet=$INTERNET" -F "status=$STATUS" -F "computer=$COMPUTER" -F "start_date=$$DATE"  "$REG_HIVE_URL")
				REG_HIVE_SUB_STATUS=$(/bin/echo $REG_HIVE_STATUS | $HOMEDIR/scripts/system/JSON.sh -b |awk -F\" '{print $4}' |awk -F, '{print $1}')

				case $REG_HIVE_SUB_STATUS in
					1)
						loglocal "$DATE" HIVECONTROL INFO "Successfully registered this hive at hivecontrol.org"
						echo "Successfully registered this hive at hivecontrol.org"
						#If was successful, so try reposting the data again
						SendData
					;;
					INVALID_DATA)
						ERROR="Invalid data posted to HiveControl for Registration"
						loglocal "$DATE" HIVECONTROL ERROR "$ERROR"
						echo "$ERROR"
						exit;
					;;
					DUPLICATE)
						ERROR="Duplicate HiveName exists under your account"
						loglocal "$DATE" HIVECONTROL ERROR "$ERROR"
						echo "$ERROR"
						exit;
					;;
					esac

					#Invalid_data
					echo "Data Posted was Invalid"
					loglocal "$DATE" SHARE_API ERROR "Data Posted to hivecontrol.org was Invalid"
		;;
		
		1)
			#Success
			echo "Successfully Posted to HiveControl.org"
		;;
		Unauthenticated.)
			#Unauthenticated
			echo "No API Key, or API is Invalid - basically you failed to authenticate"
			loglocal "$DATE" SHARE_API ERROR "No API Key, or API is Invalid - Please get an API key from Settings->API at https://www.hivecontrol.org"
		;;
		INVALID_DATA)
			#Invalid_data
			echo "Data Posted was Invalid"
			loglocal "$DATE" SHARE_API ERROR "Data Posted to hivecontrol.org was Invalid"
		;;
		UPDATE_HIVEID) 
			#Bad HiveID, update on local machine
			echo "Updating HiveID from hivecontrol.org"
			loglocal "$DATE" SHARE_API INFO "Updated HiveID based on HiveName from hivecontrol.org"
			
			HIVEID=$(/bin/echo $SHARE_API_STATUS | ./JSON.sh -b |awk -F\" '{print $4}' |awk -F, '{print $2}')
				echo "NewHiveId is $HIVEID"
			#Update HiveID in DB, including version
				DBVERSION=$(sqlite3 $HOMEDIR/data/hive-data.db "select version from hiveconfig;")
				DBVERSION=$((DBVERSION+1))
				sqlite3 $HOMEDIR/data/hive-data.db "UPDATE hiveconfig SET hiveid=\"$HIVEID\", version=\"$DBVERSION\";"
			#Try to send again with new HIVEID
				SHARE_API_STATUS=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hive_id=$HIVEID" -F "hivename=$HIVENAME" -F "hive_observation_time_local=$DATE" -F "hive_temp_f=$HIVETEMPF" -F "hive_temp_c=$HIVETEMPC" -F "hive_humidity=$HIVEHUMIDITY" -F "hive_weight_lbs=$HIVEWEIGHT" -F "hive_flight_in=$IN_COUNT" -F "hive_flight_out=$OUT_COUNT" -F "wx_station_id=$WEATHER_STATIONID" -F "wx_observation_time_rfc822=$OBSERVATIONDATETIME" -F "wx_temp_f=$A_TEMP" -F "wx_relative_humidity=$B_HUMIDITY" -F "wx_dewpoint_f=$A_DEW" -F "wx_temp_c=$A_TEMP_C" -F "wx_wind_mph=$A_WIND_MPH" -F "wx_wind_dir=$A_WIND_DIR" -F "wx_wind_degrees=$wind_degrees" -F "wx_wind_gust_mpg=$wind_gust_mph" -F "wx_pressure_mb=$pressure_mb" -F "wx_pressure_in=$A_PRES_IN" -F "wx_dewpoint_c=$weather_dewc" -F "wx_solar_radiation=$solarradiation" -F "wx_precip_1hr_in=$precip_1hr_in" -F "wx_precip_1hr_metric=$precip_1hr_metric" -F "wx_precip_today_in=$precip_today_in" -F "wx_precip_today_metric=$precip_today_metric" "$POST_DATA_URL")
		;;
		*)
			#default - unknown error
			loglocal "$DATE" SHARE_API ERROR "Error: $SHARE_API_STATUS "
		;;
	esac
	}
	#Call the function to senddata
	SendData
fi
	

# End Sharing

echo "##########################################################"
echo "Script Completed"

