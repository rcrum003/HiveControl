#!/bin/bash
# 
# Calculate every day the GDD value to add to our database
#
# Note: This is considered Insect GDD and not plant GDD (plant should be based on soil temp)
# Once we get our soil temp sensors working, we will add another script
# Author: Ryan Crum
# Version: 1

# Installation instructions:
# You will have to prepopulate with the Populate_GDD.sh script, which will require a WeatherUnderground API key.
# Or you can chose to have the data be wrong for one season

# Database Note:
# The DB only accepts one GDD row per date, no matter how many times you run the script.
# This protects against run-away scripts, and ensures our Seasonal GDD stays accurate

set -x

source /home/hivetool2/scripts/hiveconfig.inc
DATE=$(TZ=":$TIMEZONE" date '+%F %T')
DATASOURCE="$HOMEDIR/data/hive-data.db"

# Base our source based on the weather source you are  using
# We get this from your hive weather data.
# An alternative approach is to calculate once per yard, but that requires a yard controller

# We run this script once a day for yesterday's data.

YESTERDAY=`date  --date="yesterday" +%Y%m%d`
# Get the data from the DB
MAXTEMP=$(sqlite3 $DATASOURCE "SELECT MAX(weather_tempf) from allhivedata WHERE strftime('%Y%m%d', datetime(date,'localtime')) is '$YESTERDAY';")
MINTEMP=$(sqlite3 $DATASOURCE "SELECT MIN(weather_tempf) from allhivedata WHERE strftime('%Y%m%d', datetime(date,'localtime')) is '$YESTERDAY';")
#GDD=$(sqlite3 $DATASOURCE "SELECT (ROUND((MAX(weather_tempf) + MIN(weather_tempf))/2)  - $GDD_BASE_TEMP) as GDD from allhivedata WHERE strftime('%Y%m%d', datetime(date,'localtime')) is '$YESTERDAY';")

if [[ $(echo "$MAXTEMP > 86" |bc) -eq 1 ]]; then
	echo "Max Temp was higher than 86"
	let MAXTEMP="86"
fi

#DO GDD Calc
GDD=$(echo "scale=0; (($MAXTEMP + $MINTEMP)/2) - $GDD_BASE_TEMP" |bc)

echo "GDD for yesterday is $GDD"

if [[ $( echo "$GDD < 0" |bc) -eq 1 ]]; then 
	let GDD=0;
	#echo "GDD was less than 0, so we set gdd to 0"
fi

# Now that we have yesterdays daily gdd, lets calculate season GDD
SEASONGDD=$(sqlite3 $DATASOURCE "SELECT SUM(daygdd) from gdd WHERE gdddate BETWEEN '$GDD_START_DATE' AND '$YESTERDAY';")
SEASONGDD2=$(echo "$SEASONGDD + $GDD" | bc )  


echo $DATE,$GDD,$SEASONGDD2

echo "SeasonGDD = $SEASONGDD"
echo "SEASONGDD2 = $SEASONGDD2"

# Insert Weather Data into local DB, date, daygdd, seasongdd
sqlite3 $DATASOURCE "insert into gdd (calcdate, gdddate,daygdd,seasongdd) values ('$DATE','$YESTERDAY',$GDD,$SEASONGDD2);"

# Dump to a Webfile so we can see it on the dashboard
echo "$GDD" > $PUBLIC_HTML_DIR/data/todaygdd.txt
echo "$SEASONGDD2" > $PUBLIC_HTML_DIR/data/seasongdd.txt
