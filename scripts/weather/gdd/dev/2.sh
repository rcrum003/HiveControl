#!/bin/bash
# 
# Calculate every day the GDD value to add to our database
#
# Note: This is considered Insect GDD and not plant GDD (plant should be based on soil temp)
# Once we get our soil temp sensors working, we will add another script
# Autho: Ryan Crum
# Version: 1

# Installation instructions:
# You will have to prepopulate with the Populate_GDD.sh script, which will require a WeatherUnderground API key.
# Or you can chose to have the data be wrong for one season


source /home/hivetool2/scripts/hiveconfig.inc

DATASOURCE="$HOMEDIR/data/hive-data.db"

# Base our source based on the weather source you are  using
# We get this from your hive weather data.
# An alternative approach is to calculate once per yard, but that requires a yard controller

# We run this script once a day for yesterday's data.

YESTERDAY=`date  --date="yesterday" +%Y%m%d`
# Get the data from the DB
GDD=$(sqlite3 $DATASOURCE "SELECT (ROUND(AVG(weather_tempf)) - $GDD_BASE_TEMP) as GDD from weather WHERE strftime('%Y%m%d', datetime(observationDateTime,'localtime')) is '$YESTERDAY';")

echo "GDD for yesterday is $GDD"

if [[ $( echo "$GDD < 0" |bc) -eq 1 ]]; then 
	let GDD=0;
	#echo "GDD was less than 0, so we set gdd to 0"
fi

# Now that we have yesterdays daily gdd, lets calculate season GDD
SEASONGDD=$(sqlite3 $DATASOURCE "SELECT SUM(daygdd) from gdd WHERE date BETWEEN '$GDD_START_DATE' AND '$YESTERDAY';")
SEASONGDD2=$(echo "$SEASONGDD + $GDD" | bc )  
echo $YESTERDAY,$GDD,$SEASONGDD2

# Insert Weather Data into local DB, date, daygdd, seasongdd
sqlite3 $DATASOURCE "insert into gdd (date,daygdd,seasongdd) values ($YESTERDAY,$GDD, $SEASONGDD2);"



