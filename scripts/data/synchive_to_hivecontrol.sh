#!/bin/bash
# Script to sync data from local beehives to the yard controller
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
LOCALDATABASE=/home/HiveControl/data/hive-data.db
RUNDIR=/home/HiveControl/data/

#echo "$TIMEZONE"
DATE=$(TZ=":$TIMEZONE" date '+%F %T')
#echo "$DATE"


if [ -z "$HIVEAPI" ]; then
	echo "--- Unable to share with hivecontrol.org because HIVEAPI is not set"


else
		#echo "--- Sharing with hivecontrol.org ---"
		CLOUD_URL="https://www.hivecontrol.org/api/v1"
		SYNCAPI_URL="$CLOUD_URL/hive/data/sync"
		
	#Set the HIVEID on all old records
	sqlite3 $LOCALDATABASE "UPDATE allhivedata SET hiveid=$HIVEID;"

	#===========
	# Get the count of records we want to send
	#===========

	REC_COUNT=$(sqlite3 $LOCALDATABASE "SELECT COUNT(hiveid) as count from allhivedata;")
	echo "SYNCING $REC_COUNT Rows, this may take awhile"

	i=1
	while [ "$i" -lt "$REC_COUNT" ]
		do
		#=========================================
		# Query the database and pull the fields that need to be sync'd
		#=========================================
		# Export out to a file
		sqlite3 -header -csv $LOCALDATABASE "SELECT *, '$HIVENAME' as hive_name from allhivedata LIMIT 500 OFFSET $i;" > $RUNDIR/syncout.csv


		#Convert CSV to JSON
		# Note, the CSV mapping here is one less, since jq uses java and java starts numbering at 0... stupid java.
		/usr/bin/jq --slurp --raw-input --raw-output -c 'split("\r\n") | .[1:] | map(split(",")) |
		map({"hive_id": .[0], "hive_name": .[37],"hive_observation_time_local": .[1], "hive_temp_f": .[2], "hive_temp_c": .[3], "hive_humidity": .[4],"hive_weight_lbs": .[6], "wx_station_id": .[9], "wx_observation_time_rfc822": .[10], "wx_temp_f": .[11], "wx_relative_humidity": .[12], "wx_dewpoint_f": .[13], "wx_temp_c": .[14], "wx_wind_dir": .[15], "wx_wind_degrees": .[16], "wx_wind_gust_mph": .[17], "wx_pressure_mb": .[20], "wx_pressure_in": .[21],  "wx_dewpoint_c": .[23], "wx_solar_radiation": .[24], "wx_precip_1hr_in": .[26], "wx_precip_1hr_metric": .[27], "wx_precip_today_in": .[29], "wx_precip_today_metric": .[30], "wx_wind_mph": .[31], "hive_flight_in": .[34], "hive_flight_out": .[35], "record_id": .[36]})' \
		$RUNDIR/syncout.csv > $RUNDIR/syncout.json

		#=========================================
		# Connect to the cloud API and insert our data
		#=========================================
		SHARE_API_STATUS=$(/usr/bin/curl --silent --retry 5 $SYNCAPI_URL --data-binary @$RUNDIR/syncout.json -H 'Content-Type: application/json' -H "X-Requested-With: XMLHttpRequest" -H "Authorization: Bearer $HIVEAPI" ) 

		echo $SHARE_API_STATUS

		exit
		SYNCED_COUNT=$(($SYNCED_COUNT + $start))
		#Set I to get next 500 records
		i=$(($i + 500))



		echo "$DATE $SHARE_API_STATUS" >> $RUNDIR/sync.log
		loglocal "$DATE" 
	done

	#Delete old records
	rm -rf $RUNDIR/syncout.csv
	rm -rf $RUNDIR/syncout.json

	#Log that we are done.
	loglocal "$DATE" HIVECONTROL SYNC "SYNCED $SYNCED_COUNT/$REC_COUNT records"

fi
