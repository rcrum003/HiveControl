#!/bin/bash
# Script to get variables from db to use in our scripts
# Version 2019071101

source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/cloud.inc
source /home/HiveControl/scripts/data/check.inc


#Set some default variables
LOCALDATABASE="/home/HiveControl/data/hive-data.db"
CONFIGOUT="/home/HiveControl/scripts/hiveconfig.inc"

function dump_config_to_file() {
	# Generate config from database
	# Use LEFT JOIN to ensure we get hiveconfig data even if hiveequipmentweight table is empty (fresh install)
	sqlite3 -header -line $LOCALDATABASE "SELECT * from hiveconfig LEFT JOIN hiveequipmentweight on hiveconfig.id=hiveequipmentweight.id;" |sort | uniq > tempout

	# Check if tempout has data
	if [ ! -s tempout ]; then
		echo "ERROR: tempout file is empty or doesn't exist"
		return 1
	fi

	#Clean up said file and write to config
	cat tempout |awk '{ gsub(/ = /, "=\""); print }' | sed 's/^ *//g' |awk '{print $0"\""}' > $CONFIGOUT

	# Verify config file was created successfully
	if [ ! -s $CONFIGOUT ]; then
		echo "ERROR: Failed to create $CONFIGOUT"
		return 1
	fi
}

if [ ! -f "$CONFIGOUT" ]; then
   #File Doesnt exist so we need to generate it
    dump_config_to_file
else
   # Check if file is too small (corrupted) - should be at least 100 bytes for a valid config
   FILESIZE=$(stat -c%s "$CONFIGOUT" 2>/dev/null || stat -f%z "$CONFIGOUT" 2>/dev/null)
   if [ "$FILESIZE" -lt 100 ]; then
      echo "Config file appears corrupted (only $FILESIZE bytes), regenerating..."
      dump_config_to_file
   fi
fi

# Source the config file to load all variables into the environment
source $CONFIGOUT

#Get local versions
FILEVERSION=$(cat $CONFIGOUT |grep VERSION |awk -F\" '{print $2}')
DBVERSION=$(sqlite3 $LOCALDATABASE "select version from hiveconfig;")

DATE=$(TZ=":$TIMEZONE" date '+%F %T')
HOMEDIR=$(sqlite3 $LOCALDATABASE "select homedir from hiveconfig;")
HIVEID=$(sqlite3 $LOCALDATABASE "select HIVEID from hiveconfig;")
HIVENAME=$(sqlite3 $LOCALDATABASE "select HIVENAME from hiveconfig;")
REGISTERED=$(sqlite3 $LOCALDATABASE "select REGISTERED from hiveconfig;")
HIVEAPI=$(sqlite3 $LOCALDATABASE "select HIVEAPI from hiveconfig;")

if [[ -z "$HIVEID" ]]; then
	echo "HIVEID was blank, so setting back to default"
	HIVEID="9999"
fi


#Tired of these hives not being in sync with Version

	#Get CODE Version
	HCVersion_file=$(cat /home/HiveControl/VERSION)
	HCVersion_DB=$(cat $CONFIGOUT |grep -i hcversion |awk -F\" '{print $2}')
	if [[ "$HCVersion_DB" != "$HCVersion_file" ]]; then
		sqlite3 $LOCALDATABASE "UPDATE hiveconfig SET HCVersion=\"$HCVersion_file\";"
	fi

#If Hive API is going to be blank, we are going to set it manually
#GhCf3D26mV06a8j7TWGObqJL1NulwGVdxGhcpqCfAXh4ZZmV2zGu1TyV3dBj


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

		echo "HiveControl Version was $HIVE_CONFIG_VERSION"

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
					POST_id=$(jq -r .id $HIVE_CONFIG) #This is the actual HiveID in the system, HiveID field is null in this output
					#POST_created_at=$(jq -r .created_at $HIVE_CONFIG)
					#POST_updated_at=$(jq -r .updated_at $HIVE_CONFIG)
					POST_name=$(jq -r .name $HIVE_CONFIG)
					#echo "First was $POST_name"
					#POST_hiveid=$(jq -r .hiveid $HIVE_CONFIG) # this comes back null, as it's the same as POST_id
					POST_user_id=$(jq -r .user_id $HIVE_CONFIG)
					POST_yard_id=$(jq -r .yard_id $HIVE_CONFIG)
					#POST_hiveapi=$(jq -r .hiveapi $HIVE_CONFIG) #this comes back Null
					#POST_api_status=$(jq -r .api_status $HIVE_CONFIG) 
					POST_hc_version=$(jq -r .hc_version $HIVE_CONFIG)
					POST_timezone=$(jq -r .timezone $HIVE_CONFIG)
					POST_power=$(jq -r .power $HIVE_CONFIG)
					POST_internet=$(jq -r .internet $HIVE_CONFIG)
					POST_status=$(jq -r .status $HIVE_CONFIG)
					POST_computer=$(jq -r .computer $HIVE_CONFIG)
					POST_start_date=$(jq -r .start_date $HIVE_CONFIG)
					POST_run=$(jq -r .run $HIVE_CONFIG)
					#POST_ipaddress=$(jq -r .ipaddress $HIVE_CONFIG)
					POST_homedir=$(jq -r .homedir $HIVE_CONFIG)
					POST_public_html_dir=$(jq -r .public_html_dir $HIVE_CONFIG)
					POST_version=$(jq -r .version $HIVE_CONFIG)
					POST_check_for_upgrades=$(jq -r .check_for_upgrades $HIVE_CONFIG)
					#POST_deleted_at=$(jq -r .deleted_at $HIVE_CONFIG)
					POST_bee_race=$(jq -r .bee_race $HIVE_CONFIG)
					POST_bee_source=$(jq -r .bee_source $HIVE_CONFIG)
					#POST_hive_type=$(jq -r .hive_type $HIVE_CONFIG) #not locally configured
					POST_hive_status=$(jq -r .hive_status $HIVE_CONFIG)
					POST_queen_start=$(jq -r .queen_start $HIVE_CONFIG)
					POST_queen_color=$(jq -r .queen_color $HIVE_CONFIG)
					POST_enable_hive_temp_chk=$(jq -r .enable_hive_temp_chk $HIVE_CONFIG)
					POST_temptype=$(jq -r .temptype $HIVE_CONFIG)
					POST_hive_temp_gpio=$(jq -r .hive_temp_gpio $HIVE_CONFIG)
					POST_hive_temp_measure=$(jq -r .hive_temp_measure $HIVE_CONFIG)
					POST_hivedevice=$(jq -r .hivedevice $HIVE_CONFIG)
					POST_hive_temp_slope=$(jq -r .hive_temp_slope $HIVE_CONFIG)
					POST_hive_temp_intercept=$(jq -r .hive_temp_intercept $HIVE_CONFIG)
					POST_hive_humidity_slope=$(jq -r .hive_humidity_slope $HIVE_CONFIG)
					POST_hive_humidity_intercept=$(jq -r .hive_humidity_intercept $HIVE_CONFIG)
					POST_enable_hive_weight_chk=$(jq -r .enable_hive_weight_chk $HIVE_CONFIG)
					POST_scaletype=$(jq -r .scaletype $HIVE_CONFIG)
					POST_hive_weight_gpio=$(jq -r .hive_weight_gpio $HIVE_CONFIG)
					POST_hive_weight_slope=$(jq -r .hive_weight_slope $HIVE_CONFIG)
					POST_hive_weight_intercept=$(jq -r .hive_weight_intercept $HIVE_CONFIG)
					POST_enable_lux=$(jq -r .enable_lux $HIVE_CONFIG)
					POST_lux_source=$(jq -r .lux_source $HIVE_CONFIG)
					POST_hive_lux_gpio=$(jq -r .hive_lux_gpio $HIVE_CONFIG)
					POST_hive_lux_slope=$(jq -r .hive_lux_slope $HIVE_CONFIG)
					POST_hive_lux_intercept=$(jq -r .hive_lux_intercept $HIVE_CONFIG)
					POST_enable_hive_camera=$(jq -r .enable_hive_camera $HIVE_CONFIG)
					POST_enable_beecounter=$(jq -r .enable_beecounter $HIVE_CONFIG)
					POST_cameratype=$(jq -r .cameratype $HIVE_CONFIG)
					POST_cameramode=$(jq -r .cameramode $HIVE_CONFIG)
					POST_countertype=$(jq -r .countertype $HIVE_CONFIG)
					POST_NUM_HIVE_BASE_SOLID_BOTTOM_BOARD=$(jq -r .NUM_HIVE_BASE_SOLID_BOTTOM_BOARD $HIVE_CONFIG)
					POST_NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD=$(jq -r .NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD $HIVE_CONFIG)
					POST_NUM_HIVE_FEEDER=$(jq -r .NUM_HIVE_FEEDER $HIVE_CONFIG)
					POST_NUM_HIVE_TOP_INNER_COVER=$(jq -r .NUM_HIVE_TOP_INNER_COVER $HIVE_CONFIG)
					POST_NUM_HIVE_TOP_TELE_COVER=$(jq -r .NUM_HIVE_TOP_TELE_COVER $HIVE_CONFIG)
					POST_NUM_HIVE_TOP_MIGRATORY_COVER=$(jq -r .NUM_HIVE_TOP_MIGRATORY_COVER $HIVE_CONFIG)
					POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION=$(jq -r .NUM_HIVE_BODY_MEDIUM_FOUNDATION $HIVE_CONFIG)
					POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS=$(jq -r .NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS $HIVE_CONFIG)
					POST_NUM_HIVE_BODY_DEEP_FOUNDATION=$(jq -r .NUM_HIVE_BODY_DEEP_FOUNDATION $HIVE_CONFIG)
					POST_NUM_HIVE_BODY_DEEP_FOUNDATION_LESS=$(jq -r .NUM_HIVE_BODY_DEEP_FOUNDATION_LESS $HIVE_CONFIG)
					POST_NUM_HIVE_BODY_SHAL_FOUNDATION=$(jq -r .NUM_HIVE_BODY_SHAL_FOUNDATION $HIVE_CONFIG)
					POST_NUM_HIVE_BODY_SHAL_FOUNDATION_LESS=$(jq -r .NUM_HIVE_BODY_SHAL_FOUNDATION_LESS $HIVE_CONFIG)
					POST_gdd_base_temp=$(jq -r .gdd_base_temp $HIVE_CONFIG)
					POST_gdd_start_date=$(jq -r .gdd_start_date $HIVE_CONFIG)
					POST_latitude=$(jq -r .latitude $HIVE_CONFIG)
					POST_longitude=$(jq -r .longitude $HIVE_CONFIG)
					POST_local_wx_type=$(jq -r .local_wx_type $HIVE_CONFIG)
					POST_local_wx_url=$(jq -r .local_wx_url $HIVE_CONFIG)
					POST_weather_detail=$(jq -r .weather_detail $HIVE_CONFIG)
					POST_weather_level=$(jq -r .weather_level $HIVE_CONFIG)
					POST_wx_humidity_intercept=$(jq -r .wx_humidity_intercept $HIVE_CONFIG)
					POST_wx_humidity_slope=$(jq -r .wx_humidity_slope $HIVE_CONFIG)
					POST_wx_temp_gpio=$(jq -r .wx_temp_gpio $HIVE_CONFIG)
					POST_wx_temp_intercept=$(jq -r .wx_temp_intercept $HIVE_CONFIG)
					POST_wx_temp_slope=$(jq -r .wx_temp_slope $HIVE_CONFIG)
					POST_wx_temper_device=$(jq -r .wx_temper_device $HIVE_CONFIG)
					POST_wx_station=$(jq -r .wx_station $HIVE_CONFIG)
					POST_wx_temp_type=$(jq -r .wx_temp_type $HIVE_CONFIG)
					
					POST_enable_hive_air_chk=$(jq -r .enable_hive_air_chk $HIVE_CONFIG)
					POST_hive_air_type=$(jq -r .hive_air_type $HIVE_CONFIG)
					POST_hive_air_id=$(jq -r .hive_air_id $HIVE_CONFIG)
					POST_hive_air_api=$(jq -r .hive_air_api $HIVE_CONFIG)

			#You have to evaluate if something is blank, if so, use the local copy.
			#let's make a function
			function A {
				#Function to check if a value is null, so we dont' overwrite the local DB
				# $1 remote_variable value to check
				# $2 local_variable to use if it was blank
				# Usage " A $POST_name name"
				#remote_var="$1"
				local_var="$2"

				if [[ ${!1} == "null" || ${!1} == "" ]]; then
					#Remote Value was null or blank
					#Get local value
					echo "Checking value..... ${1}"
					LOCAL_VAR_VAL=$(sqlite3 $LOCALDATABASE "SELECT $local_var from hiveconfig;")
					#Set the remote_var
					if [[ $LOCAL_VAR_VAL == "" ]]; then
						let LOCAL_VAR_VAL="null"
					else
						let ${1}="$LOCAL_VAR_VAL"
					fi
				fi

			}



			##############################################
			#
			# If a value is detected locally and we get a null result from the 
			#
			###############################################
			A POST_id hiveid
			#A POST_created_at created_at
			#A POST_updated_at updated_at
			A POST_name hivename
			A POST_user_id user_id
			A POST_yard_id yard_id
			#A POST_hiveid hiveid
			#A POST_hiveapi hiveapi
			#A POST_api_status api_status
			A POST_hc_version hc_version
			A POST_timezone timezone
			A POST_power power
			A POST_internet internet
			A POST_status status
			A POST_computer computer
			A POST_start_date start_date
			A POST_run run
			#A POST_ipaddress ipaddress
			A POST_homedir homedir
			A POST_public_html_dir public_html_dir
			A POST_version version
			A POST_check_for_upgrades check_for_upgrades
			#A POST_deleted_at deleted_at
			A POST_bee_race bee_race
			A POST_bee_source bee_source
			#A POST_hive_type hive_type
			A POST_hive_status hive_status
			A POST_queen_start queen_start
			A POST_queen_color queen_color
			A POST_enable_hive_temp_chk enable_hive_temp_chk
			A POST_temptype temptype
			A POST_hive_temp_gpio hive_temp_gpio
			A POST_hive_temp_measure hive_temp_measure
			A POST_hivedevice hivedevice
			A POST_hive_temp_slope hive_temp_slope
			A POST_hive_temp_intercept hive_temp_intercept
			A POST_hive_humidity_slope hive_humidity_slope
			A POST_hive_humidity_intercept hive_humidity_intercept
			A POST_enable_hive_weight_chk enable_hive_weight_chk
			A POST_scaletype scaletype
			A POST_hive_weight_gpio hive_weight_gpio
			A POST_hive_weight_slope hive_weight_slope
			A POST_hive_weight_intercept hive_weight_intercept
			A POST_enable_lux enable_lux
			A POST_lux_source lux_source
			A POST_hive_lux_gpio hive_lux_gpio
			A POST_hive_lux_slope hive_lux_slope
			A POST_hive_lux_intercept hive_lux_intercept
			A POST_enable_hive_camera enable_hive_camera
			A POST_enable_beecounter enable_beecounter
			A POST_cameratype cameratype
			A POST_cameramode cameramode
			A POST_countertype countertype
			A POST_NUM_HIVE_BASE_SOLID_BOTTOM_BOARD NUM_HIVE_BASE_SOLID_BOTTOM_BOARD
			A POST_NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD
			A POST_NUM_HIVE_FEEDER NUM_HIVE_FEEDER
			A POST_NUM_HIVE_TOP_INNER_COVER NUM_HIVE_TOP_INNER_COVER
			A POST_NUM_HIVE_TOP_TELE_COVER NUM_HIVE_TOP_TELE_COVER
			A POST_NUM_HIVE_TOP_MIGRATORY_COVER NUM_HIVE_TOP_MIGRATORY_COVER
			A POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION NUM_HIVE_BODY_MEDIUM_FOUNDATION
			A POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS
			A POST_NUM_HIVE_BODY_DEEP_FOUNDATION NUM_HIVE_BODY_DEEP_FOUNDATION
			A POST_NUM_HIVE_BODY_DEEP_FOUNDATION_LESS NUM_HIVE_BODY_DEEP_FOUNDATION_LESS
			A POST_NUM_HIVE_BODY_SHAL_FOUNDATION NUM_HIVE_BODY_SHAL_FOUNDATION
			A POST_NUM_HIVE_BODY_SHAL_FOUNDATION_LESS NUM_HIVE_BODY_SHAL_FOUNDATION_LESS
			A POST_gdd_base_temp gdd_base_temp
			A POST_gdd_start_date gdd_start_date
			A POST_latitude latitude
			A POST_longitude longitude
			A POST_local_wx_type local_wx_type
			A POST_local_wx_url local_wx_url
			A POST_weather_detail weather_detail
			A POST_weather_level weather_level
			A POST_wx_humidity_intercept wx_humidity_intercept
			A POST_wx_humidity_slope wx_humidity_slope
			A POST_wx_temp_gpio wx_temp_gpio
			A POST_wx_temp_intercept wx_temp_intercept
			A POST_wx_temp_slope wx_temp_slope
			A POST_wx_temper_device wx_temper_device
			A POST_wx_station wx_station
			A POST_wx_temp_type WXTEMPTYPE



			#Make sure all values have a value
				check_if_blank POST_id
				#check_if_blank POST_created_at
				#check_if_blank POST_updated_at
				check_if_blank POST_name
				check_if_blank POST_user_id
				check_if_blank POST_yard_id
				#check_if_blank POST_hiveid
				#check_if_blank POST_hiveapi
				#check_if_blank POST_api_status
				check_if_blank POST_hc_version
				check_if_blank POST_timezone
				check_if_blank POST_power
				check_if_blank POST_internet
				check_if_blank POST_status
				check_if_blank POST_computer
				check_if_blank POST_start_date
				check_if_blank POST_run
				#check_if_blank POST_ipaddress
				check_if_blank POST_homedir
				check_if_blank POST_public_html_dir
				check_if_blank POST_version
				check_if_blank POST_check_for_upgrades
				#check_if_blank POST_deleted_at
				check_if_blank POST_bee_race
				check_if_blank POST_bee_source
				#check_if_blank POST_hive_type
				check_if_blank POST_hive_status
				check_if_blank POST_queen_start
				check_if_blank POST_queen_color
				check_if_blank POST_enable_hive_temp_chk
				check_if_blank POST_temptype
				check_if_blank POST_hive_temp_gpio
				check_if_blank POST_hive_temp_measure
				check_if_blank POST_hivedevice
				check_if_blank POST_hive_temp_slope
				check_if_blank POST_hive_temp_intercept
				check_if_blank POST_hive_humidity_slope
				check_if_blank POST_hive_humidity_intercept
				check_if_blank POST_enable_hive_weight_chk
				check_if_blank POST_scaletype
				check_if_blank POST_hive_weight_gpio
				check_if_blank POST_hive_weight_slope
				check_if_blank POST_hive_weight_intercept
				check_if_blank POST_enable_lux
				check_if_blank POST_lux_source
				check_if_blank POST_hive_lux_gpio
				check_if_blank POST_hive_lux_slope
				check_if_blank POST_hive_lux_intercept
				check_if_blank POST_enable_hive_camera
				check_if_blank POST_enable_beecounter
				check_if_blank POST_cameratype
				check_if_blank POST_cameramode
				check_if_blank POST_countertype
				check_if_blank POST_NUM_HIVE_BASE_SOLID_BOTTOM_BOARD
				check_if_blank POST_NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD
				check_if_blank POST_NUM_HIVE_FEEDER
				check_if_blank POST_NUM_HIVE_TOP_INNER_COVER
				check_if_blank POST_NUM_HIVE_TOP_TELE_COVER
				check_if_blank POST_NUM_HIVE_TOP_MIGRATORY_COVER
				check_if_blank POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION
				check_if_blank POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS
				check_if_blank POST_NUM_HIVE_BODY_DEEP_FOUNDATION
				check_if_blank POST_NUM_HIVE_BODY_DEEP_FOUNDATION_LESS
				check_if_blank POST_NUM_HIVE_BODY_SHAL_FOUNDATION
				check_if_blank POST_NUM_HIVE_BODY_SHAL_FOUNDATION_LESS
				check_if_blank POST_gdd_base_temp
				check_if_blank POST_gdd_start_date
				check_if_blank POST_latitude
				check_if_blank POST_longitude
				check_if_blank POST_local_wx_type
				check_if_blank POST_local_wx_url
				check_if_blank POST_weather_detail
				check_if_blank POST_weather_level
				check_if_blank POST_wx_humidity_intercept
				check_if_blank POST_wx_humidity_slope
				check_if_blank POST_wx_temp_gpio
				check_if_blank POST_wx_temp_intercept
				check_if_blank POST_wx_temp_slope
				check_if_blank POST_wx_temper_device
				check_if_blank POST_wx_station
				check_if_blank POST_wx_temp_type
				

			#Update Local DB
		 	sqlite3 $LOCALDATABASE "UPDATE hiveconfig SET HIVEAPI=\"$HIVEAPI\", HIVENAME=\"$POST_name\", HIVEID=\"$POST_id\", CAMERAMODE=\"$POST_cameramode\", CAMERATYPE=\"$POST_cameratype\", check_for_upgrades=\"$POST_check_for_upgrades\", COMPUTER=\"$POST_computer\", COUNTERTYPE=\"$POST_countertype\", ENABLE_BEECOUNTER=\"$POST_enable_beecounter\", ENABLE_HIVE_CAMERA=\"$POST_enable_hive_camera\", ENABLE_HIVE_TEMP_CHK=\"$POST_enable_hive_temp_chk\", ENABLE_HIVE_WEIGHT_CHK=\"$POST_enable_hive_weight_chk\", ENABLE_LUX=\"$POST_enable_lux\", GDD_BASE_TEMP=\"$POST_gdd_base_temp\", GDD_START_DATE=\"$POST_gdd_start_date\", HCVersion=\"$POST_hc_version\", HIVE_HUMIDITY_INTERCEPT=\"$POST_hive_humidity_intercept\", HIVE_HUMIDITY_SLOPE=\"$POST_hive_humidity_slope\", HIVE_LUX_GPIO=\"$POST_hive_lux_gpio\", HIVE_LUX_INTERCEPT=\"$POST_hive_lux_intercept\", HIVE_LUX_SLOPE=\"$POST_hive_lux_slope\", HIVE_TEMP_GPIO=\"$POST_hive_temp_gpio\", HIVE_TEMP_INTERCEPT=\"$POST_hive_temp_intercept\", HIVE_TEMP_MEASURE=\"$POST_hive_temp_measure\", HIVE_TEMP_SLOPE=\"$POST_hive_temp_slope\", HIVE_WEIGHT_GPIO=\"$POST_hive_weight_gpio\", HIVE_WEIGHT_INTERCEPT=\"$POST_hive_weight_intercept\", HIVE_WEIGHT_SLOPE=\"$POST_hive_weight_slope\", HIVEDEVICE=\"$POST_hivedevice\", INTERNET=\"$POST_internet\", LATITUDE=\"$POST_latitude\", local_wx_type=\"$POST_local_wx_type\", local_wx_url=\"$POST_local_wx_url\", LONGITUDE=\"$POST_longitude\", LUX_SOURCE=\"$POST_lux_source\", NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD=\"$POST_NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD\", NUM_HIVE_BASE_SOLID_BOTTOM_BOARD=\"$POST_NUM_HIVE_BASE_SOLID_BOTTOM_BOARD\", NUM_HIVE_BODY_DEEP_FOUNDATION=\"$POST_NUM_HIVE_BODY_DEEP_FOUNDATION\", NUM_HIVE_BODY_DEEP_FOUNDATION_LESS=\"$POST_NUM_HIVE_BODY_DEEP_FOUNDATION_LESS\", NUM_HIVE_BODY_MEDIUM_FOUNDATION=\"$POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION\", NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS=\"$POST_NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS\", NUM_HIVE_BODY_SHAL_FOUNDATION=\"$POST_NUM_HIVE_BODY_SHAL_FOUNDATION\", NUM_HIVE_BODY_SHAL_FOUNDATION_LESS=\"$POST_NUM_HIVE_BODY_SHAL_FOUNDATION_LESS\", NUM_HIVE_FEEDER=\"$POST_NUM_HIVE_FEEDER\", NUM_HIVE_TOP_INNER_COVER=\"$POST_NUM_HIVE_TOP_INNER_COVER\", NUM_HIVE_TOP_MIGRATORY_COVER=\"$POST_NUM_HIVE_TOP_MIGRATORY_COVER\", NUM_HIVE_TOP_TELE_COVER=\"$POST_NUM_HIVE_TOP_TELE_COVER\", POWER=\"$POST_power\", PUBLIC_HTML_DIR=\"$POST_public_html_dir\", RUN=\"$POST_run\", SCALETYPE=\"$POST_scaletype\", START_DATE=\"$POST_start_date\", STATUS=\"$POST_status\", TEMPTYPE=\"$POST_temptype\", TIMEZONE=\"$POST_timezone\", VERSION=\"$POST_version\", WEATHER_DETAIL=\"$POST_weather_detail\", WEATHER_LEVEL=\"$POST_weather_level\", WX_HUMIDITY_INTERCEPT=\"$POST_wx_humidity_intercept\", WX_HUMIDITY_SLOPE=\"$POST_wx_humidity_slope\", WXSTATION=\"$POST_wx_station\", WX_TEMP_GPIO=\"$POST_wx_temp_gpio\", WX_TEMP_INTERCEPT=\"$POST_wx_temp_intercept\", WX_TEMP_SLOPE=\"$POST_wx_temp_slope\", WXTEMPTYPE=\"$POST_wx_temp_type\", WX_TEMPER_DEVICE=\"$POST_wx_temper_device\";"
			 #Register Success
			 	#Set DBVersion to match the HiveControl.org version
				DBVersion=$POST_version
				MESSAGE="Updated Local Config with Newer Version from HiveControl.org"
				loglocal "$DATE" CONFIG INFO "$MESSAGE"
				echo "$MESSAGE"
				PUBLIC_HTML_DIR="$HOMEDIR/www/public_html/"
				echo "$DBVERSION" > $PUBLIC_HTML_DIR/admin/hiveconfig.ver
				dump_config_to_file

				exit 1
		 	
		 	fi # 4.
			###############################################################
			#	
			# If hivecontrol.org is older, send local config to the API
			#
			###############################################################
			if [ "$DBVERSION" -gt "$HIVE_CONFIG_VERSION" ]; then #5
				#Get Local Config from Database, Dump to a tempfile that we can source
				dump_config_to_file
				#sqlite3 -header -line $LOCALDATABASE "SELECT * from hiveconfig INNER JOIN hiveequipmentweight on hiveconfig.id=hiveequipmentweight.id;" |sort | uniq > tempout
				#cat tempout |awk '{ gsub(/ = /, "=\""); print }' | sed 's/^ *//g' |awk '{print $0"\""}' > $CONFIGOUT
				source $CONFIGOUT

				###################################
				# Make sure numbers are numbers
				##################################
					check_allow_neg_return_zero HIVE_HUMIDITY_INTERCEPT
					check_allow_neg_return_zero HIVE_HUMIDITY_SLOPE
					check_allow_neg_return_zero HIVE_LUX_GPIO
					check_allow_neg_return_zero HIVE_LUX_INTERCEPT
					check_allow_neg_return_zero HIVE_LUX_SLOPE
					check_allow_neg_return_zero HIVE_TEMP_GPIO
					check_allow_neg_return_zero HIVE_TEMP_INTERCEPT
					check_allow_neg_return_zero HIVE_TEMP_MEASURE
					check_allow_neg_return_zero HIVE_TEMP_SLOPE
					check_allow_neg_return_zero HIVE_WEIGHT_GPIO
					check_allow_neg_return_zero HIVE_WEIGHT_INTERCEPT
					check_allow_neg_return_zero HIVE_WEIGHT_SLOPE
					check_allow_neg_return_zero NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD
					check_allow_neg_return_zero NUM_HIVE_BASE_SOLID_BOTTOM_BOARD
					check_allow_neg_return_zero NUM_HIVE_BODY_DEEP_FOUNDATION
					check_allow_neg_return_zero NUM_HIVE_BODY_DEEP_FOUNDATION_LESS
					check_allow_neg_return_zero NUM_HIVE_BODY_MEDIUM_FOUNDATION
					check_allow_neg_return_zero NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS
					check_allow_neg_return_zero NUM_HIVE_BODY_SHAL_FOUNDATION
					check_allow_neg_return_zero NUM_HIVE_BODY_SHAL_FOUNDATION_LESS
					check_allow_neg_return_zero NUM_HIVE_FEEDER
					check_allow_neg_return_zero NUM_HIVE_TOP_INNER_COVER
					check_allow_neg_return_zero NUM_HIVE_TOP_MIGRATORY_COVER
					check_allow_neg_return_zero NUM_HIVE_TOP_TELE_COVER
					check_allow_neg_return_zero VERSION
					check_allow_neg_return_zero GDD_BASE_TEMP
					check_allow_neg_return_zero LATITUDE
					check_allow_neg_return_zero LONGITUDE
					check_allow_neg_return_zero WX_HUMIDITY_INTERCEPT
					check_allow_neg_return_zero WX_HUMIDITY_SLOPE
					check_allow_neg_return_zero WX_TEMP_GPIO
					check_allow_neg_return_zero WX_TEMP_INTERCEPT
					check_allow_neg_return_zero WX_TEMP_SLOPE

				#Send Config to hivecontrol.org, latest config is already in file
					SEND_CONFIG_STATUS=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "cameramode=$CAMERAMODE"	-F "cameratype=$CAMERATYPE"	-F "check_for_upgrades=$check_for_upgrades"	-F "computer=$COMPUTER"	-F "countertype=$COUNTERTYPE"	-F "enable_beecounter=$ENABLE_BEECOUNTER"	-F "enable_hive_camera=$ENABLE_HIVE_CAMERA"	-F "enable_hive_temp_chk=$ENABLE_HIVE_TEMP_CHK"	-F "enable_hive_weight_chk=$ENABLE_HIVE_WEIGHT_CHK"	-F "enable_lux=$ENABLE_LUX"	-F "hc_version=$HCVersion"	-F "hive_humidity_intercept=$HIVE_HUMIDITY_INTERCEPT"	-F "hive_humidity_slope=$HIVE_HUMIDITY_SLOPE"	-F "hive_lux_gpio=$HIVE_LUX_GPIO"	-F "hive_lux_intercept=$HIVE_LUX_INTERCEPT"	-F "hive_lux_slope=$HIVE_LUX_SLOPE"	-F "hive_status=$status"	-F "hive_temp_gpio=$HIVE_TEMP_GPIO"	-F "hive_temp_intercept=$HIVE_TEMP_INTERCEPT"	-F "hive_temp_measure=$HIVE_TEMP_MEASURE"	-F "hive_temp_slope=$HIVE_TEMP_SLOPE"	-F "hive_weight_gpio=$HIVE_WEIGHT_GPIO"	-F "hive_weight_intercept=$HIVE_WEIGHT_INTERCEPT"	-F "hive_weight_slope=$HIVE_WEIGHT_SLOPE"	-F "hivedevice=$HIVEDEVICE"	-F "homedir=$HOMEDIR"	-F "internet=$INTERNET"	-F "lux_source=$LUX_SOURCE"	-F "NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD=$NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD"	-F "NUM_HIVE_BASE_SOLID_BOTTOM_BOARD=$NUM_HIVE_BASE_SOLID_BOTTOM_BOARD"	-F "NUM_HIVE_BODY_DEEP_FOUNDATION=$NUM_HIVE_BODY_DEEP_FOUNDATION"	-F "NUM_HIVE_BODY_DEEP_FOUNDATION_LESS=$NUM_HIVE_BODY_DEEP_FOUNDATION_LESS"	-F "NUM_HIVE_BODY_MEDIUM_FOUNDATION=$NUM_HIVE_BODY_MEDIUM_FOUNDATION"	-F "NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS=$NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS"	-F "NUM_HIVE_BODY_SHAL_FOUNDATION=$NUM_HIVE_BODY_SHAL_FOUNDATION"	-F "NUM_HIVE_BODY_SHAL_FOUNDATION_LESS=$NUM_HIVE_BODY_SHAL_FOUNDATION_LESS"	-F "NUM_HIVE_FEEDER=$NUM_HIVE_FEEDER"	-F "NUM_HIVE_TOP_INNER_COVER=$NUM_HIVE_TOP_INNER_COVER"	-F "NUM_HIVE_TOP_MIGRATORY_COVER=$NUM_HIVE_TOP_MIGRATORY_COVER"	-F "NUM_HIVE_TOP_TELE_COVER=$NUM_HIVE_TOP_TELE_COVER"	-F "power=$POWER"	-F "public_html_dir=$PUBLIC_HTML_DIR"	-F "run=$RUN"	-F "scaletype=$SCALETYPE"	-F "start_date=$START_DATE"	-F "status=$STATUS"	-F "temptype=$TEMPTYPE"	-F "timezone=$TIMEZONE"	-F "version=$VERSION"	-F "GDD_BASE_TEMP=$GDD_BASE_TEMP"	-F "GDD_START_DATE=$GDD_START_DATE"	-F "LATITUDE=$LATITUDE"	-F "local_wx_type=$local_wx_type"	-F "local_wx_url=$local_wx_url"	-F "LONGITUDE=$LONGITUDE"	-F "WEATHER_DETAIL=$WEATHER_DETAIL"	-F "WEATHER_LEVEL=$WEATHER_LEVEL"	-F "WX_HUMIDITY_INTERCEPT=$WX_HUMIDITY_INTERCEPT"	-F "WX_HUMIDITY_SLOPE=$WX_HUMIDITY_SLOPE"	-F "WX_TEMP_GPIO=$WX_TEMP_GPIO"	-F "WX_TEMP_INTERCEPT=$WX_TEMP_INTERCEPT"	-F "WX_TEMP_SLOPE=$WX_TEMP_SLOPE"	-F "WX_TEMPER_DEVICE=$WX_TEMPER_DEVICE"	-F "WX_STATION=$WXSTATION"	-F "WX_TEMP_TYPE=$WXTEMPTYPE"	-F "api_token=$HIVEAPI"	-F "hivename=$HIVENAME"  -F "hive_id=$HIVEID"  "$POST_UPDATECONFIG_URL" |awk -F'\"' '{print $4}')
					
				#Register Success
				check_api_send_status "$SEND_CONFIG_STATUS" "hiveconfig.sh"

			#Parse Various Response and set SHARE_API_STATUS
			# Check to see if the status was Unauthenticated	
			#SHARE_SUB_STATUS=$(/bin/echo $SHARE_API_STATUS | $HOMEDIR/scripts/system/JSON.sh -b |awk -F\" '{print $4}' |awk -F, '{print $1}')
				 dump_config_to_file

				 exit 1

				 fi	#5
	
		fi #2
	
	fi # 1.
#######################################################
# End Cloud Config
#######################################################


########################################################################
# Write the latest version to the local file, to be used by the scripts
########################################################################
dump_config_to_file

	if [ $FILEVERSION -gt $DBVERSION ]; then
		loglocal "$DATE" CONFIG ERROR "File Version is higher than DB Version - Did you edit manually"
		echo "ERROR: File Version is higher than DB Version - Did you edit manually?"
		exit 1
	fi

##########
# END
##########


