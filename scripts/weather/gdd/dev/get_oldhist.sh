#/bin/bash

# Script to get weather history
# from weather underground
# requires an API key
# by Ryan Crum
# in support of HiveTool Project
# Written to calculate GDD
# May 16, 2015

#Parameters

FILE=hist_dates

while read line; do
        echo  "$line"

KEY="6002e68ccef1de3a"
CITY="FAIRFIELD"
STATE="CT"
YESTERDAY=$line

#Data Fetchers
# Weather Data
GETNOW=`/usr/bin/curl --silent http://api.wunderground.com/api/$KEY/history_$YESTERDAY/q/$STATE/$CITY.json`
echo $GETNOW > /home/newhive/data/weather/fulldata/weather_history_$YESTERDAY.json


# Data Parsers
# Weather Data

#Get Degree Days
A_GDEGREEDAYS=`/bin/echo $GETNOW | ./JSON.sh -b |grep "\[\"history\",\"dailysummary\",0,\"gdegreedays\"\]" |awk -F"\"" '{print $8}'`

#Store in a file so we can accumulate degree days
echo "$CITY,$STATE,$YESTERDAY,$A_GDEGREEDAYS" >> /home/newhive/data/weather/GDD_data.log


#Make Available to other scripts
#export A_GDEGREEDAYS="$A_GDEGREEDAYS"
#echo $A_GDEGREEDAYS

# Data Printers

# Data Inserter into Database
        sleep 6
done < $FILE
