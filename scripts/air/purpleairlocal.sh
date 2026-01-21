
#!/bin/bash
###################################
# Purple Air - Local, because ThinkSpeak limits API calls - Those Bastards!
#
# Script to pull air quality reports from a Purple Air sensor
#
# Make a new script to pull from local device http://192.168.110.177/json?live=true

# v 2025042101

#Pull configs and logger library
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
#source /home/HiveControl/scripts/data/curl.inc

#Store date in a variable
DATE=$(TZ=":$TIMEZONE" date '+%F %T')

JSON_PATH="/home/HiveControl/scripts/weather/JSON.sh"


#Instructions
#Documentation here:
#https://docs.google.com/document/d/15ijz94dXJ-YAZLi9iZ_RaBwrZ4KtYeCy08goGBwnbCU/edit


#####################
# Main
#####################

#Storing URL in AIR_LOCAL_URL field
AIR_URL="$AIR_LOCAL_URL"
TEMPFILE="/home/HiveControl/scripts/air/output.json"
#TEMPFILE="/home/pi/purpleair/output.json"

#Get all sensors
#https://www.purpleair.com/json 

#If PurpleSensorID is set, then use that one
#IF not, Find Air Quality Closet to you by latitude/longitude

#Pull Sensor Data
#https://www.purpleair.com/json?show=<ID>

#Note, PurpleAir has a rate limitation, so if you have your own Purple Station, find the IP and configure as the AIR_LOCAL_URL
#Example JSON output from http://192.168.110.127/json
# {"SensorId":"60:1:94:58:a4:28","DateTime":"2025/04/21T23:43:02z","Geo":"PurpleAir-a428","Mem":19704,"memfrag":15,"memfb":16744,
# "memcs":784,"Id":115282,"lat":41.170502,"lon":-73.254402,"Adc":0.02,"loggingrate":15,"place":"outside","version":"7.02","uptime":2621427,
# "rssi":-42,"period":120,"httpsuccess":117147,"httpsends":117175,"hardwareversion":"2.0","hardwarediscovered":"2.0+BME280+PMSX003-B+PMSX003-A",
# "current_temp_f":59,"current_humidity":47,"current_dewpoint_f":39,"pressure":1015.51,"p25aqic_b":"rgb(7,229,0)","pm2.5_aqi_b":15,"pm1_0_cf_1_b":2.05,
# "p_0_3_um_b":507.33,"pm2_5_cf_1_b":3.64,"p_0_5_um_b":150.49,"pm10_0_cf_1_b":4.56,"p_1_0_um_b":28.22,"pm1_0_atm_b":2.05,"p_2_5_um_b":4.55,"pm2_5_atm_b":3.64,
# "p_5_0_um_b":1.20,"pm10_0_atm_b":4.56,"p_10_0_um_b":0.36,"p25aqic":"rgb(2,228,0)","pm2.5_aqi":10,"pm1_0_cf_1":1.23,"p_0_3_um":400.53,"pm2_5_cf_1":2.42,"p_0_5_um":104.44,
# "pm10_0_cf_1":2.75,"p_1_0_um":25.11,"pm1_0_atm":1.23,"p_2_5_um":1.44,"pm2_5_atm":2.42,"p_5_0_um":0.51,"pm10_0_atm":2.75,"p_10_0_um":0.00,"pa_latency":188,
# "response":201,"response_date":1745278887,"latency":822,"wlstate":"Connected","status_0":2,"status_1":2,"status_2":2,"status_3":2,"status_4":0,"status_5":0,
# "status_6":2,"status_7":0,"status_8":0,"status_9":0,"ssid":"MURC_AIR"}

while [[ $TRYCOUNTER -lt 3 && $DATA_GOOD -eq 0 ]];
do
	#https://www.purpleair.com/json?show=7634
	RESULT=$(/usr/bin/curl -s -w %{http_code} "$AIR_URL" -o $TEMPFILE)
	
	PM2_5=$(jq --raw-output '.p_2_5_um' <$TEMPFILE) #PM2.5 (CF=1)
	AIR_TEMP=$(jq --raw-output '.current_temp_f' <$TEMPFILE)
	AIR_HUMIDITY=$(jq --raw-output '.current_humidity' <$TEMPFILE)
	PM1_0=$(jq --raw-output '.p_1_0_um' <$TEMPFILE)
	PM10=$(jq --raw-output '.p_10_0_um' <$TEMPFILE)
	AQI=$(jq --raw-output '.["pm2.5_aqi_b"]' <$TEMPFILE)

	if [ -z "$PM2_5"  ]; then
		loglocal "$DATE" AIR INFO "Did not get a proper value for PM2_5 from $AIR_URL, trying again"
		let TRYCOUNTER+=1
	else
		DATA_GOOD=1
	fi
done

if [ $DATA_GOOD = "0" ]; then
	loglocal "$DATE" AIR ERROR "Did not get a proper response from $AIR_URL- $RESULT"
	echo "ERROR"
	exit
fi

#Do Conversion to AQI for US Market
#https://en.wikipedia.org/wiki/Air_quality_index

#Remove downloaded file
rm -rf $TEMPFILE

#Return output for main script

#echo "DATE,AIR_TEMP,AIR_HUMIDITY,PM1_0,PM2_5,PM10"
echo "$DATE,$AIR_TEMP,$AIR_HUMIDITY,$PM1_0,$PM2_5,$PM10,$AQI"




#         _
#        /_/_      .'''.
#     =O(_)))) ...'     `.
#        \_\              `.    .'''
#                           `..'


