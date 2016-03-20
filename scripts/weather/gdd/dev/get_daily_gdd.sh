#!/bin/bash
# 
# Calculate every day the GDD value to add to our database
#
# Seperate script used to determine the actual value
#

# Notes
# We calculate three different types of GDD
# 
# 1. Micro Weather - Based on your Hive Yard Sensors or yard weather station
#	
# 2. Macro Weather - Based on closet weatherunderground.com weather station

# 3. Soil GDD  # Future

# Important parameters to understand and set.

#---------------------------------------------------------------------
# Parameters

source /home/HiveControl/scripts/hivetool.inc

# Base our source based on the weather source you are  using

# Chose Weather Source 
# 1 = Micro, use your temphum weather for the day
# 2 = Macro, use weatherunderground.com weather station

WEATHER_SOURCE="1"


if [ $WEATHER_LEVEL = "hive" ]; then

# Micro Settings


# Macro Settings
WX_KEY="6002e68ccef1de3a"		# Need API key for api.wunderground.com, get yours and put it here.
WX_STATION="KCTFAIRF10"
WX_CITY="Fairfield"
WX_STATE="CT"

YESTERDAY=`date  --date="yesterday" +%Y%m%d`

# Format http://api.wunderground.com/api/6002e68ccef1de3a/history_20150525/q/pws:KCTFAIRF10.json

#Data Fetchers
# Weather Data
GETNOW=`/usr/bin/curl --silent http://api.wunderground.com/api/$WX_KEY/history_$YESTERDAY/q/pws:$WX_STATION.json`
echo $GETNOW > /home/newhive/data/weather/weather_history_$YESTERDAY.json
echo $GETNOW > /var/www/weather_yesterday.json

# Data Parsers
# Weather Data

#Get Degree Days
A_GDEGREEDAYS=`/bin/echo $GETNOW | JSON.sh -b |grep "\[\"history\",\"dailysummary\",0,\"gdegreedays\"\]" |awk -F"\"" '{print $8}'`

#Store in a file so we can accumulate degree days
echo "$WX_CITY,$WX_STATE,$YESTERDAY,$A_GDEGREEDAYS" >> /home/newhive/data/weather/GDD_data.log


#Make Available to other scripts
export A_GDEGREEDAYS="$A_GDEGREEDAYS"
echo $A_GDEGREEDAYS

# Data Printers

# Data Inserter into Database


