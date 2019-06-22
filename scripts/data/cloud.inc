#!/bin/bash
# Cloud Settings and variables to be shared with multiple scripts
# Version 2.1
# 2019062201

#Connection information

CLOUD_URL="https://www.hivecontrol.org/api/v1"


#API URLs
SYNCAPI_URL="$CLOUD_URL/hive/data/sync"
CHECK_URL="$CLOUD_URL/hive/check"
POST_DATA_URL="$CLOUD_URL/hive/data"
REG_HIVE_URL="$CLOUD_URL/hive/new"


#Config URLs
POST_CONFIGVERSION_URL="$CLOUD_URL/hive/config/version"
POST_GETCONFIG_URL="$CLOUD_URL/hive/config"
POST_UPDATECONFIG_URL="$CLOUD_URL/hive/config/update"


#Common functions:
function hc_senddata () {	
	#Try Sending the fields.
	
	#Need to convert one of our dates to a more friendly format
	#CONVERTED_DATE=$( echo $OBSERVATIONDATETIME	HIVETEMPF=$(echo "scale=2; (($HIVETEMPF * $HIVE_TEMP_SLOPE) + $HIVE_TEMP_INTERCEPT)" | bc)
	WX_DATE_TIME=$(date -d @$(($OBSERVATIONDATETIME/1000)) '+%Y-%m-%d %H:%M:%S')
	#echo "WX_DATE_TIME = $WX_DATE_TIME"
	SHARE_API_STATUS=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hive_id=$HIVEID" -F "hivename=$HIVENAME" -F "hive_observation_time_local=$DATE" -F "hive_temp_f=$HIVETEMPF" -F "hive_temp_c=$HIVETEMPC" -F "hive_humidity=$HIVEHUMIDITY" -F "hive_weight_lbs=$HIVEWEIGHT" -F "hive_flight_in=$IN_COUNT" -F "hive_flight_out=$OUT_COUNT" -F "wx_station_id=$WEATHER_STATIONID" -F "wx_observation_time_rfc822=$WX_DATE_TIME" -F "wx_temp_f=$A_TEMP" -F "wx_relative_humidity=$B_HUMIDITY" -F "wx_dewpoint_f=$A_DEW" -F "wx_temp_c=$A_TEMP_C" -F "wx_wind_mph=$A_WIND_MPH" -F "wx_wind_dir=$A_WIND_DIR" -F "wx_wind_degrees=$wind_degrees" -F "wx_wind_gust_mpg=$wind_gust_mph" -F "wx_pressure_mb=$pressure_mb" -F "wx_pressure_in=$A_PRES_IN" -F "wx_dewpoint_c=$weather_dewc" -F "wx_solar_radiation=$solarradiation" -F "wx_precip_1hr_in=$precip_1hr_in" -F "wx_precip_1hr_metric=$precip_1hr_metric" -F "wx_precip_today_in=$precip_today_in" -F "wx_precip_today_metric=$precip_today_metric" -F "record_id=$record_id" "$POST_DATA_URL")

	#Parse Various Response and set SHARE_API_STATUS
		# Check to see if the status was Unauthenticated	
		SHARE_SUB_STATUS=$(/bin/echo $SHARE_API_STATUS | $HOMEDIR/scripts/system/JSON.sh -b |awk -F\" '{print $4}' |awk -F, '{print $1}')

		check_api_send_status $SHARE_SUB_STATUS hc_senddata

	}

#Check Registration

function register_hive () {

# Gets a webpage you specify
# This script takes 4 inputs 
# 1 - HIVENAME 
# 2 - HIVEID

CHECK_HIVENAME="$1"
CHECK_HIVEID="$2"

HIVE_REG_CHECK=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hivename=$HIVENAME" "$CHECK_URL" | jq -r .error)

echo "hive_reg_check was $HIVE_REG_CHECK"
case $HIVE_REG_CHECK in
	"NOT_REGISTERED" )
		#Do registration
		#Register our hive now
				#You look like a new hive, so we will register you

				REG_HIVE_STATUS=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "name=$HIVENAME" -F "hc_version=$HCVersion" -F "timezone=$TIMEZONE" -F "power=$POWER" -F "internet=$INTERNET" -F "status=$STATUS" -F "computer=$COMPUTER" -F "start_date=$DATE"  "$REG_HIVE_URL" | jq -r .result)

				case $REG_HIVE_STATUS in
					SUCCESS)
						loglocal "$DATE" HIVECONTROL INFO "Successfully registered as a new hive at hivecontrol.org"
						REG_HIVE_RESULT="REG_VALID"
						set_hiveid 

					;;
					INVALID_DATA)
						loglocal "$DATE" HIVECONTROL ERROR "Invalid data posted to HiveControl for Registration"
						exit 1
					;;
					DUPLICATE)
						loglocal "$DATE" HIVECONTROL ERROR "Duplicate HiveName exists under your account"
						exit 1
					;;
					*)
						#Invalid_data
						loglocal "$DATE" HIVECONTROL ERROR "Unknown error has occured in register_hive 102"
					;;		
					esac
		;;
	"REG_VALID")
		#Do nothing
		REG_HIVE_RESULT="REG_VALID"
		;;
	"UPDATE_HIVEID")
		#Calling API again to get the HIVEID, since we didn't store in the initial check
		set_hiveid
		loglocal "$DATE" HIVECONTROL INFO "Updated HiveID based on HiveName from hivecontrol.org"
		REG_HIVE_RESULT="REG_VALID"
		;;
	"UNKNOWN_ERROR")
		loglocal "$DATE" HIVECONTOL ERROR "HiveControl Returned an unknown error Registration Routine 101"
		;;
	*)
		loglocal "$DATE" HIVECONTROL ERROR "HIVE_REG_CHECK returned $HIVE_REG_CHECK"
		;;
esac

}

function set_hiveid () {
		HIVEID=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hivename=$HIVENAME" "$CHECK_URL" | jq -r .value)
		#Update HiveID in DB, including version
		DBVERSION=$(sqlite3 $HOMEDIR/data/hive-data.db "select version from hiveconfig;")
		DBVERSION=$((DBVERSION+1))
		sqlite3 $HOMEDIR/data/hive-data.db "UPDATE hiveconfig SET hiveid=\"$HIVEID\", version=\"$DBVERSION\", registered=\"yes\";"
}

function check_api_send_status () {
	#used to parse responses back from the API
	#Usage "check_api_send_status $STATUSRESULT"
	SHARE_SUB_STATUS="$1"
	CALLING_FUNCTION="$2" #used to recall the function

	
	case $SHARE_SUB_STATUS in
		"NOT_REGISTERED")
			#Register our hive now
				#You look like a new hive, so we will register you
				register_hive $HIVENAME $HIVEID
			;;
		"1")
			#Success
			echo "Successfully Posted to HiveControl.org"
			;;
		"Unauthenticated.")
			#Unauthenticated
			echo "No API Key, or API is Invalid - basically you failed to authenticate"
			loglocal "$DATE" SHARE_API ERROR "No API Key, or API is Invalid - Please get an API key from Settings->API at https://www.hivecontrol.org"
			;;
		"INVALID_DATA")
			#Invalid_data
			echo "Data Posted was Invalid"
			loglocal "$DATE" SHARE_API ERROR "$CALLING_FUNCTION Data Posted to hivecontrol.org was Invalid"
			;;
		"UPDATE_HIVEID") 
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
				#SHARE_API_STATUS=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hive_id=$HIVEID" -F "hivename=$HIVENAME" -F "hive_observation_time_local=$DATE" -F "hive_temp_f=$HIVETEMPF" -F "hive_temp_c=$HIVETEMPC" -F "hive_humidity=$HIVEHUMIDITY" -F "hive_weight_lbs=$HIVEWEIGHT" -F "hive_flight_in=$IN_COUNT" -F "hive_flight_out=$OUT_COUNT" -F "wx_station_id=$WEATHER_STATIONID" -F "wx_observation_time_rfc822=$OBSERVATIONDATETIME" -F "wx_temp_f=$A_TEMP" -F "wx_relative_humidity=$B_HUMIDITY" -F "wx_dewpoint_f=$A_DEW" -F "wx_temp_c=$A_TEMP_C" -F "wx_wind_mph=$A_WIND_MPH" -F "wx_wind_dir=$A_WIND_DIR" -F "wx_wind_degrees=$wind_degrees" -F "wx_wind_gust_mpg=$wind_gust_mph" -F "wx_pressure_mb=$pressure_mb" -F "wx_pressure_in=$A_PRES_IN" -F "wx_dewpoint_c=$weather_dewc" -F "wx_solar_radiation=$solarradiation" -F "wx_precip_1hr_in=$precip_1hr_in" -F "wx_precip_1hr_metric=$precip_1hr_metric" -F "wx_precip_today_in=$precip_today_in" -F "wx_precip_today_metric=$precip_today_metric" -F "record_id=$record_id" "$POST_DATA_URL")
			
			#Try to do the function you tried originally
			if [ -z ${CALLING_FUNCTION+x} ]; then 
				echo "No function called this function"; 
			else 
				$CALLING_FUNCTION; 
			fi
			;;
		*)
			#default - unknown error
			loglocal "$DATE" SHARE_API ERROR "$CALLING_FUNCTION Error: $SHARE_SUB_STATUS "
		;;
	esac
}

