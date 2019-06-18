#!/bin/bash
# Script to get variables from db to use in our scripts
# Version 2019061801

source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/cloud.inc


#Set some default variables
LOCALDATABASE=/home/HiveControl/data/hive-data.db
CONFIGOUT="/home/HiveControl/scripts/hiveconfig.inc"
source $CONFIGOUT

#Get local versions
FILEVERSION=`cat $CONFIGOUT |grep VERSION |awk -F\" '{print $2}'`
DBVERSION=`sqlite3 $LOCALDATABASE "select version from hiveconfig;"`

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

HIVEID=$(sqlite3 $LOCALDATABASE "select HIVEID from hiveconfig;")

if [[ -z "$HIVEID" ]]; then
	echo "HIVEID was blank, so setting back to default"
	HIVEID="9999"
fi

HIVENAME=$(sqlite3 $LOCALDATABASE "select HIVENAME from hiveconfig;")
REGISTERED=$(sqlite3 $LOCALDATABASE "select REGISTERED from hiveconfig;")




#######################
#Check Cloud Config
#######################
if [ -z "$HIVEAPI" ]; then # 1.
	#API isn't set, so moving on
	echo "--- Reading Local Config ONLY, because HIVEAPI is not set"

else # 1.
	if [[ -z "$REGISTERED" ]]; then # 2.
			##############################################
			#Hive registration not set, so register hive
			#Only gets set when hivename/hiveid all match for a user, based on API key
			##############################################
			register_hive "$HIVENAME" "$HIVEID"
			#HIVE_REG_CHECK=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hivename=$HIVENAME" "$CHECK_URL" | jq .error)
			
			if [[ "$REG_HIVE_RESULT" = "REG_VALID" ]]; then #3.
				DBVERSION=$(sqlite3 $LOCALDATABASE "select version from hiveconfig;")
				DBVERSION=$((DBVERSION+1))
				#Set Registered to yes
				sqlite3 $LOCALDATABASE "UPDATE hiveconfig SET version=\"$DBVERSION\", REGISTERED=\"yes\";"
							
				#statements
				fi #3.
	else # 2. 
		echo "We are registered, so do the needful....."
		##############################################
		# We are registered, so let's do the needful
		##############################################	
		echo "--- Syncing Local Hive Config with hivecontrol.org ---"
		#Check to see what version HiveControl Has
		#POST_CONFIGVERSION_URL - Returns {"version":"23"}
		
		#Get the hivecontrol.org version number
		HIVE_CONFIG_VERSION=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hive_id=$HIVEID" -F "hivename=$HIVENAME" "$POST_CONFIGVERSION_URL" | jq -r .version)

		echo "HiveConfig Version was $HIVE_CONFIG_VERSION"

		if [[ "$HIVE_CONFIG_VERSION" == "null" ]]; then
			#Something went wrong, let's try that again and get the error code
			echo "Hive_Config version was null........"
			register_hive "$HIVENAME" "$HIVEID"

			#Get Hive_Config_version, because last time it was blank
			HIVE_CONFIG_VERSION=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hive_id=$HIVEID" -F "hivename=$HIVENAME" "$POST_CONFIGVERSION_URL" | jq -r .version)

			fi

		#####################################################################
		# If hivecontrol.org is newer, import that as our new DB config
		#####################################################################
		if [ "$HIVE_CONFIG_VERSION" -gt "$DBVERSION" ]; then # 4.
			echo "HiveControl.org is newer, importing that as our DBConfig"
			HIVE_CONFIG="/home/HiveControl/scripts/data/temp.config"
			#Pull the config into a file
			/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hive_id=$HIVEID" -F "hivename=$HIVENAME" "$POST_GETCONFIG_URL" -o $HIVE_CONFIG
	 		
	 		#Assign JSON variables to BASH variables
					POST_id=$(jq .id $HIVE_CONFIG)
					#POST_created_at=$(jq .created_at $HIVE_CONFIG)
					#POST_updated_at=$(jq .updated_at $HIVE_CONFIG)
					POST_name=$(jq .name $HIVE_CONFIG)
					POST_user_id=$(jq .user_id $HIVE_CONFIG)
					POST_yard_id=$(jq .yard_id $HIVE_CONFIG)
					#POST_hiveid=$(jq .hiveid $HIVE_CONFIG)
					#POST_hiveapi=$(jq .hiveapi $HIVE_CONFIG)
					POST_api_status=$(jq .api_status $HIVE_CONFIG)
					POST_hc_version=$(jq .hc_version $HIVE_CONFIG)
					POST_timezone=$(jq .timezone $HIVE_CONFIG)
					POST_power=$(jq .power $HIVE_CONFIG)
					POST_internet=$(jq .internet $HIVE_CONFIG)
					POST_status=$(jq .status $HIVE_CONFIG)
					POST_computer=$(jq .computer $HIVE_CONFIG)
					POST_start_date=$(jq .start_date $HIVE_CONFIG)
					POST_run=$(jq .run $HIVE_CONFIG)
					POST_ipaddress=$(jq .ipaddress $HIVE_CONFIG)
					POST_homedir=$(jq .homedir $HIVE_CONFIG)
					POST_public_html_dir=$(jq .public_html_dir $HIVE_CONFIG)
					POST_version=$(jq .version $HIVE_CONFIG)
					POST_check_for_upgrades=$(jq .check_for_upgrades $HIVE_CONFIG)
					#POST_deleted_at=$(jq .deleted_at $HIVE_CONFIG)
					POST_bee_race=$(jq .bee_race $HIVE_CONFIG)
					POST_bee_source=$(jq .bee_source $HIVE_CONFIG)
					POST_hive_type=$(jq .hive_type $HIVE_CONFIG)
					POST_hive_status=$(jq .hive_status $HIVE_CONFIG)
					POST_queen_start=$(jq .queen_start $HIVE_CONFIG)
					POST_queen_color=$(jq .queen_color $HIVE_CONFIG)
					POST_enable_hive_temp_chk=$(jq .enable_hive_temp_chk $HIVE_CONFIG)
					POST_temptype=$(jq .temptype $HIVE_CONFIG)
					POST_hive_temp_gpio=$(jq .hive_temp_gpio $HIVE_CONFIG)
					POST_hive_temp_measure=$(jq .hive_temp_measure $HIVE_CONFIG)
					POST_hivedevice=$(jq .hivedevice $HIVE_CONFIG)
					POST_hive_temp_slope=$(jq .hive_temp_slope $HIVE_CONFIG)
					POST_hive_temp_intercept=$(jq .hive_temp_intercept $HIVE_CONFIG)
					POST_hive_humidity_slope=$(jq .hive_humidity_slope $HIVE_CONFIG)
					POST_hive_humidity_intercept=$(jq .hive_humidity_intercept $HIVE_CONFIG)
					POST_enable_hive_weight_chk=$(jq .enable_hive_weight_chk $HIVE_CONFIG)
					POST_scaletype=$(jq .scaletype $HIVE_CONFIG)
					POST_hive_weight_gpio=$(jq .hive_weight_gpio $HIVE_CONFIG)
					POST_hive_weight_slope=$(jq .hive_weight_slope $HIVE_CONFIG)
					POST_hive_weight_intercept=$(jq .hive_weight_intercept $HIVE_CONFIG)
					POST_enable_lux=$(jq .enable_lux $HIVE_CONFIG)
					POST_lux_source=$(jq .lux_source $HIVE_CONFIG)
					POST_hive_lux_gpio=$(jq .hive_lux_gpio $HIVE_CONFIG)
					POST_hive_lux_slope=$(jq .hive_lux_slope $HIVE_CONFIG)
					POST_hive_lux_intercept=$(jq .hive_lux_intercept $HIVE_CONFIG)
					POST_enable_hive_camera=$(jq .enable_hive_camera $HIVE_CONFIG)
					POST_enable_beecounter=$(jq .enable_beecounter $HIVE_CONFIG)
					POST_cameratype=$(jq .cameratype $HIVE_CONFIG)
					POST_cameramode=$(jq .cameramode $HIVE_CONFIG)
					POST_countertype=$(jq .countertype $HIVE_CONFIG)
					#POST_NUM_HIVE_BASE_SOLID_BOTTOM_BOARD=$(jq .NUM_HIVE_BASE_SOLID_BOTTOM_BOARD $HIVE_CONFIG)
					#POST_NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD=$(jq .NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD $HIVE_CONFIG)
					#POST_NUM_HIVE_FEEDER=$(jq .NUM_HIVE_FEEDER $HIVE_CONFIG)
					#POST_NUM_HIVE_TOP_INNER_COVER=$(jq .NUM_HIVE_TOP_INNER_COVER $HIVE_CONFIG)
					#POST_NUM_HIVE_TOP_TELE_COVER=$(jq .NUM_HIVE_TOP_TELE_COVER $HIVE_CONFIG)
					#POST_NUM_HIVE_TOP_MIGRATORY_COVER=$(jq .NUM_HIVE_TOP_MIGRATORY_COVER $HIVE_CONFIG)
					#POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION=$(jq .NUM_HIVE_BODY_MEDIUM_FOUNDATION $HIVE_CONFIG)
					#POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS=$(jq .NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS $HIVE_CONFIG)
					#POST_NUM_HIVE_BODY_DEEP_FOUNDATION=$(jq .NUM_HIVE_BODY_DEEP_FOUNDATION $HIVE_CONFIG)
					#POST_NUM_HIVE_BODY_DEEP_FOUNDATION_LESS=$(jq .NUM_HIVE_BODY_DEEP_FOUNDATION_LESS $HIVE_CONFIG)
					#POST_NUM_HIVE_BODY_SHAL_FOUNDATION=$(jq .NUM_HIVE_BODY_SHAL_FOUNDATION $HIVE_CONFIG)
					#POST_NUM_HIVE_BODY_SHAL_FOUNDATION_LESS=$(jq .NUM_HIVE_BODY_SHAL_FOUNDATION_LESS $HIVE_CONFIG)
					POST_gdd_base_temp=$(jq .gdd_base_temp $HIVE_CONFIG)
					POST_gdd_start_date=$(jq .gdd_start_date $HIVE_CONFIG)
					POST_latitude=$(jq .latitude $HIVE_CONFIG)
					POST_longitude=$(jq .longitude $HIVE_CONFIG)
					POST_local_wx_type=$(jq .local_wx_type $HIVE_CONFIG)
					POST_local_wx_url=$(jq .local_wx_url $HIVE_CONFIG)
					POST_weather_detail=$(jq .weather_detail $HIVE_CONFIG)
					POST_weather_level=$(jq .weather_level $HIVE_CONFIG)
					POST_wx_humidity_intercept=$(jq .wx_humidity_intercept $HIVE_CONFIG)
					POST_wx_humidity_slope=$(jq .wx_humidity_slope $HIVE_CONFIG)
					POST_wx_temp_gpio=$(jq .wx_temp_gpio $HIVE_CONFIG)
					POST_wx_temp_intercept=$(jq .wx_temp_intercept $HIVE_CONFIG)
					POST_wx_temp_slope=$(jq .wx_temp_slope $HIVE_CONFIG)
					POST_wx_temper_device=$(jq .wx_temper_device $HIVE_CONFIG)
					POST_wx_station=$(jq .wx_station $HIVE_CONFIG)
					POST_wx_temp_type=$(jq .wx_temp_type $HIVE_CONFIG)

			#You have to evaluate if something is blank, if so, use the local copy.
			#let's make a function
			function A {
				#Function to check if a value is null, so we dont' overwrite the local DB
				# $1 remote_variable value to check
				# $2 local_variable to use if it was blank
				# Usage " A $POST_name name"
				#remote_var="$1"
				local_var="$2"

				if [[ ${!1} == "null" ]]; then
					#Remote Value was null
					#Get local value
					echo "Checking value..... ${1}"
					LOCAL_VAR_VAL=$(sqlite3 $LOCALDATABASE "SELECT $local_var from hiveconfig;")
					#Set the remote_var
					let ${1}="$LOCAL_VAR_VAL"
				fi

			}
			##############################################
			#
			#
			#
			# Look here - it's not finished below.
			#
			#
			#
			#
			###############################################
			#try this function before we do a whole bunch
			A POST_hiveapi hive_api
			A POST_power power
			#Update Local DB
			 sqlite3 $LOCALDATABASE "INSERT OR REPLACE INTO hiveconfig(id,CAMERAMODE,CAMERATYPE,check_for_upgrades,COMPUTER,COUNTERTYPE,ENABLE_BEECOUNTER,ENABLE_HIVE_CAMERA,ENABLE_HIVE_TEMP_CHK,ENABLE_HIVE_WEIGHT_CHK,ENABLE_LUX,GDD_BASE_TEMP,GDD_START_DATE,HCVersion,HIVE_HUMIDITY_INTERCEPT,HIVE_HUMIDITY_SLOPE,HIVE_LUX_GPIO,HIVE_LUX_INTERCEPT,HIVE_LUX_SLOPE,HIVE_TEMP_GPIO,HIVE_TEMP_INTERCEPT,HIVE_TEMP_MEASURE,HIVE_TEMP_SLOPE,HIVE_TEMP_SUB,HIVE_WEIGHT_GPIO,HIVE_WEIGHT_INTERCEPT,HIVE_WEIGHT_SLOPE,HIVEDEVICE,HOMEDIR,INTERNET,LATITUDE,local_wx_type,local_wx_url,LONGITUDE,LUX_SOURCE,HIVENAME,NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD,NUM_HIVE_BASE_SOLID_BOTTOM_BOARD,NUM_HIVE_BODY_DEEP_FOUNDATION,NUM_HIVE_BODY_DEEP_FOUNDATION_LESS,NUM_HIVE_BODY_MEDIUM_FOUNDATION,NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS,NUM_HIVE_BODY_SHAL_FOUNDATION,NUM_HIVE_BODY_SHAL_FOUNDATION_LESS,NUM_HIVE_FEEDER,NUM_HIVE_TOP_INNER_COVER,NUM_HIVE_TOP_MIGRATORY_COVER,NUM_HIVE_TOP_TELE_COVER,POWER,PUBLIC_HTML_DIR,RUN,SCALETYPE,START_DATE,STATUS,TEMPTYPE,TIMEZONE,VERSION,WEATHER_DETAIL,WEATHER_LEVEL,WX_HUMIDITY_INTERCEPT,WX_HUMIDITY_SLOPE,WXSTATION,WX_TEMP_GPIO,WX_TEMP_INTERCEPT,WX_TEMP_SLOPE,WXTEMPTYPE,WX_TEMPER_DEVICE) VALUES (1,$POST_cameramode,$POST_cameratype,$POST_check_for_upgrades,$POST_computer,$POST_countertype,$POST_enable_beecounter,$POST_enable_hive_camera,$POST_enable_hive_temp_chk,$POST_enable_hive_weight_chk,$POST_enable_lux,$POST_gdd_base_temp,$POST_gdd_start_date,$POST_hc_version,$POST_hive_humidity_intercept,$POST_hive_humidity_slope,$POST_hive_lux_gpio,$POST_hive_lux_intercept,$POST_hive_lux_slope,$POST_hive_temp_gpio,$POST_hive_temp_intercept,$POST_hive_temp_measure,$POST_hive_temp_slope,$POST_hive_type,$POST_hive_weight_gpio,$POST_hive_weight_intercept,$POST_hive_weight_slope,$POST_hivedevice,$POST_homedir,$POST_internet,$POST_latitude,$POST_local_wx_type,$POST_local_wx_url,$POST_longitude,$POST_lux_source,$POST_name,$POST_NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD,$POST_NUM_HIVE_BASE_SOLID_BOTTOM_BOARD,$POST_NUM_HIVE_BODY_DEEP_FOUNDATION,$POST_NUM_HIVE_BODY_DEEP_FOUNDATION_LESS,$POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION,$POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS,$POST_NUM_HIVE_BODY_SHAL_FOUNDATION,$POST_NUM_HIVE_BODY_SHAL_FOUNDATION_LESS,$POST_NUM_HIVE_FEEDER,$POST_NUM_HIVE_TOP_INNER_COVER,$POST_NUM_HIVE_TOP_MIGRATORY_COVER,$POST_NUM_HIVE_TOP_TELE_COVER,$POST_power,$POST_public_html_dir,$POST_run,$POST_scaletype,$POST_start_date,$POST_status,$POST_temptype,$POST_timezone,$POST_version,$POST_weather_detail,$POST_weather_level,$POST_wx_humidity_intercept,$POST_wx_humidity_slope,$POST_wx_station,$POST_wx_temp_gpio,$POST_wx_temp_intercept,$POST_wx_temp_slope,$POST_wx_temp_type,$POST_wx_temper_device)"
			 exit
			#Register Success
			MESSAGE="Updating Local Config with Newer Version from HiveControl.org"
			loglocal "$DATE" CONFIG INFO "$MESSAGE"
			echo "$MESSAGE"
			echo "$DBVERSION" > $PUBLIC_HTML_DIR/admin/hiveconfig.ver
			
			
		 	fi # 4.
			###############################################################
			#	
			# If hivecontrol.org is older, send local config to the API
			#
			###############################################################
			if [ "$DBVERSION" -gt "$HIVE_CONFIG_VERSION" ]; then #5
				#Get Local Config from Database, Dump to a tempfile that we can source
				sqlite3 -header -line $LOCALDATABASE "SELECT * from hiveconfig INNER JOIN hiveequipmentweight on hiveconfig.id=hiveequipmentweight.id;" |sort | uniq > tempout

				#Send Config to hivecontrol.org, latest config is already in file
					SEND_CONFIG_STATUS=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "cameramode=$CAMERAMODE"	-F "cameratype=$CAMERATYPE"	-F "check_for_upgrades=$check_for_upgrades"	-F "computer=$COMPUTER"	-F "countertype=$COUNTERTYPE"	-F "enable_beecounter=$ENABLE_BEECOUNTER"	-F "enable_hive_camera=$ENABLE_HIVE_CAMERA"	-F "enable_hive_temp_chk=$ENABLE_HIVE_TEMP_CHK"	-F "enable_hive_weight_chk=$ENABLE_HIVE_WEIGHT_CHK"	-F "enable_lux=$ENABLE_LUX"	-F "hc_version=$HCVersion"	-F "hive_humidity_intercept=$HIVE_HUMIDITY_INTERCEPT"	-F "hive_humidity_slope=$HIVE_HUMIDITY_SLOPE"	-F "hive_lux_gpio=$HIVE_LUX_GPIO"	-F "hive_lux_intercept=$HIVE_LUX_INTERCEPT"	-F "hive_lux_slope=$HIVE_LUX_SLOPE"	-F "hive_status=$status"	-F "hive_temp_gpio=$HIVE_TEMP_GPIO"	-F "hive_temp_intercept=$HIVE_TEMP_INTERCEPT"	-F "hive_temp_measure=$HIVE_TEMP_MEASURE"	-F "hive_temp_slope=$HIVE_TEMP_SLOPE"	-F "hive_weight_gpio=$HIVE_WEIGHT_GPIO"	-F "hive_weight_intercept=$HIVE_WEIGHT_INTERCEPT"	-F "hive_weight_slope=$HIVE_WEIGHT_SLOPE"	-F "hivedevice=$HIVEDEVICE"	-F "homedir=$HOMEDIR"	-F "internet=$INTERNET"	-F "lux_source=$LUX_SOURCE"	-F "NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD=$NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD"	-F "NUM_HIVE_BASE_SOLID_BOTTOM_BOARD=$NUM_HIVE_BASE_SOLID_BOTTOM_BOARD"	-F "NUM_HIVE_BODY_DEEP_FOUNDATION=$NUM_HIVE_BODY_DEEP_FOUNDATION"	-F "NUM_HIVE_BODY_DEEP_FOUNDATION_LESS=$NUM_HIVE_BODY_DEEP_FOUNDATION_LESS"	-F "NUM_HIVE_BODY_MEDIUM_FOUNDATION=$NUM_HIVE_BODY_MEDIUM_FOUNDATION"	-F "NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS=$NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS"	-F "NUM_HIVE_BODY_SHAL_FOUNDATION=$NUM_HIVE_BODY_SHAL_FOUNDATION"	-F "NUM_HIVE_BODY_SHAL_FOUNDATION_LESS=$NUM_HIVE_BODY_SHAL_FOUNDATION_LESS"	-F "NUM_HIVE_FEEDER=$NUM_HIVE_FEEDER"	-F "NUM_HIVE_TOP_INNER_COVER=$NUM_HIVE_TOP_INNER_COVER"	-F "NUM_HIVE_TOP_MIGRATORY_COVER=$NUM_HIVE_TOP_MIGRATORY_COVER"	-F "NUM_HIVE_TOP_TELE_COVER=$NUM_HIVE_TOP_TELE_COVER"	-F "power=$POWER"	-F "public_html_dir=$PUBLIC_HTML_DIR"	-F "run=$RUN"	-F "scaletype=$SCALETYPE"	-F "start_date=$START_DATE"	-F "status=$STATUS"	-F "temptype=$TEMPTYPE"	-F "timezone=$TIMEZONE"	-F "version=$VERSION"	-F "GDD_BASE_TEMP=$GDD_BASE_TEMP"	-F "GDD_START_DATE=$GDD_START_DATE"	-F "LATITUDE=$LATITUDE"	-F "local_wx_type=$local_wx_type"	-F "local_wx_url=$local_wx_url"	-F "LONGITUDE=$LONGITUDE"	-F "WEATHER_DETAIL=$WEATHER_DETAIL"	-F "WEATHER_LEVEL=$WEATHER_LEVEL"	-F "WX_HUMIDITY_INTERCEPT=$WX_HUMIDITY_INTERCEPT"	-F "WX_HUMIDITY_SLOPE=$WX_HUMIDITY_SLOPE"	-F "WX_TEMP_GPIO=$WX_TEMP_GPIO"	-F "WX_TEMP_INTERCEPT=$WX_TEMP_INTERCEPT"	-F "WX_TEMP_SLOPE=$WX_TEMP_SLOPE"	-F "WX_TEMPER_DEVICE=$WX_TEMPER_DEVICE"	-F "WX_STATION=$WXSTATION"	-F "WX_TEMP_TYPE=$WXTEMPTYPE"	-F "api_token=$HIVEAPI"	-F "hivename=$HIVENAME"  -F "hive_id=$HIVEID"  "$POST_UPDATECONFIG_URL")
				#Register Success
				check_api_send_status $SEND_CONFIG_STATUS

			#Parse Various Response and set SHARE_API_STATUS
			# Check to see if the status was Unauthenticated	
			#SHARE_SUB_STATUS=$(/bin/echo $SHARE_API_STATUS | $HOMEDIR/scripts/system/JSON.sh -b |awk -F\" '{print $4}' |awk -F, '{print $1}')
				 
				 fi	#5
	
		fi #2
	
		
		###############################################################
		# 
		# Finish up by exporting to a flat file for the scripts to use
		#
		###############################################################
		# If we passed the two tests above, we can continue
		# Dump to a tempfile
		sqlite3 -header -line $LOCALDATABASE "SELECT * from hiveconfig INNER JOIN hiveequipmentweight on hiveconfig.id=hiveequipmentweight.id;" |sort | uniq > tempout
		#Clean up said file

		cat tempout |awk '{ gsub(/ = /, "=\""); print }' | sed 's/^ *//g' |awk '{print $0"\""}' > $CONFIGOUT 

		

	fi # 1.
#######################################################
# End Cloud Config
#######################################################


########################################################################
# Write the latest version to the local file, to be used by the scripts
########################################################################

	if [ $DBVERSION -eq $FILEVERSION ]; then
		echo "No Change - Versions are the same"
		exit 1
		fi
	if [ $FILEVERSION -gt $DBVERSION ]; then
		loglocal "$DATE" CONFIG ERROR "File Version is higher than DB Version - Did you edit manually"
		echo "ERROR: File Version is higher than DB Version - Did you edit manually?"
		exit 1
		fi
	# If we passed the two tests above, we can continue
	# Dump to a tempfile
	sqlite3 -header -line $LOCALDATABASE "SELECT * from hiveconfig INNER JOIN hiveequipmentweight on hiveconfig.id=hiveequipmentweight.id;" |sort | uniq > tempout
	#Clean up said file

	cat tempout |awk '{ gsub(/ = /, "=\""); print }' | sed 's/^ *//g' |awk '{print $0"\""}' > $CONFIGOUT 

	#loglocal "$DATE" CONFIG SUCCESS "Updated Config to Version $DBVERSION"
  echo "$DBVERSION" > $PUBLIC_HTML_DIR/admin/hiveconfig.ver

##########
# END
##########


