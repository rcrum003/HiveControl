# getwundergrd.sh
#/bin/bash

# Script to get weather from weather underground
# requires an API key
# by Ryan Crum
# in support of HiveTool Project
# May 16, 2015

#Parameters
SHELL=/bin/bash
PATH=/usr/local/sbin:/usr/local/bin:/bin:/sbin:/usr/sbin:/usr/bin:/home/hivetool/weather

KEY="6002e68ccef1de3a"
CITY="FAIRFIELD"
STATE="CT"

#Data Fetchers
# Weather Data
GETNOW=`/usr/bin/curl --silent http://api.wunderground.com/api/$KEY/conditions/q/$STATE/$CITY.json`
#HOST=`hostname`
#DATE=`date -R` # Using UTC for easier data analysis

# Data Parsers
# Weather Data

A_TEMP=`/bin/echo $GETNOW | JSON.sh -b |grep temp_f |awk '{print $2}'`
A_TIME=`/bin/echo $GETNOW | JSON.sh -b |grep observation_time_rfc822 |awk '{print $2}'`
#A_TIME=`/bin/echo $GETNOW | JSON.sh -b |grep observation_epoch |awk -F"\"" '{print $6}'`
A_HUMIDITY=`/bin/echo $GETNOW | JSON.sh -b |grep relative_humidity |awk -F"\"" '{print $6}'`
#A_WINDSTR=`/bin/echo $GETNOW | JSON.sh -b |grep wind_string |awk -F"\"" '{print $6}'`
A_WIND_DIR=`/bin/echo $GETNOW | JSON.sh -b |grep wind_dir |awk -F"\"" '{print $6}'`
A_WIND_MPH=`/bin/echo $GETNOW | JSON.sh -b |grep wind_mph |awk '{print $6}'`
A_PRES_IN=`/bin/echo $GETNOW | JSON.sh -b |grep pressure_in |awk -F"\"" '{print $6}'`
A_PRES_TREND=`/bin/echo $GETNOW | JSON.sh -b |grep pressure_trend |awk -F"\"" '{print $6}'`
A_DEW=`/bin/echo $GETNOW | JSON.sh -b |grep dewpoint_f |awk '{print $2}'`

#=`echo $GETNOW | ./JSON.sh -b |grep relative_humidity |awk -F"\"" '{print $6}'`


#Make Available to other scripts
export A_TEMP="$A_TEMP"
export A_TIME="$A_TIME"
export A_HUMIDITY="$A_HUMIDITY"
export A_WIND_DIR="$A_WIND_DIR"
export A_PRES_IN="$A_PRES_IN"
export A_PRES_TREND="$A_PRESS_TREND"
export A_DEW="$A_DEW"

# Date Printers
#echo $TEMP, $TIME, $HUMIDITY, $WINDSTR, $WIND_DIR, $PRES_IN, $PRES_TREND, $DEW >> /home/hivetool/weather/weather.log

# Data Inserter into Database
# Return to caller so they can process
#echo $HOST, $DATE, $TEMP, $TIME, $HUMIDITY, $WINDSTR, $WIND_DIR, $PRES_IN, $PRES_TREND, $DEW

