#/bin/bash

# Script to get weather history
# from weather underground
# requires an API key
# by Ryan Crum
# in support of HiveTool Project
# Written to calculate GDD
# May 16, 2015
# Support both C and F calculations
# Note Weather Undergroun only supported F

#Parameters
#Set your base 
BASEm=10

#Data Fetchers
# Weather Data

###############

FILES=/home/newhive/data/weather/fulldata/*
for f in $FILES
do
  echo "Processing $f file..."
  # take action on each file. $f store current file name
GOTDATA=`cat $f`
DATE=`echo $f |awk -F"_" '{print $3}' |awk -F"." '{print $1}'` 
MEANTEMPm=`echo $GOTDATA | ./JSON.sh -l |grep daily |grep meantempm |awk -F"\"" '{print $8}'`
GDEGREEDAYS=`echo $GOTDATA | ./JSON.sh -b |grep "\[\"history\",\"dailysummary\",0,\"gdegreedays\"\]" |awk -F"\"" '{print $8}'`


echo $DATE, $MEANTEMPm, $BASEm, $GDEGREEDAYS

if (( $MEANTEMPm < $BASEm )); then
	GDD_C=0 
else
	GDD_C=`expr "$MEANTEMPm" - "$BASEm" | bc`
fi
echo "FAIRFIELD,CT,$DATE,$GDEGREEDAYS,$GDD_C"
echo "FAIRFIELD,CT,$DATE,$GDEGREEDAYS,$GDD_C" >> GDD_C.log

#echo $MEANTEMPm,$MEANTEMPi

#echo $GETNOW > /home/newhive/data/weather/fulldata/weather_history_$YESTERDAY.json


# Data Parsers
# Weather Data

#Get Degree Days
#A_GDEGREEDAYS=`/bin/echo $GETNOW | ./JSON.sh -b |grep "\[\"history\",\"dailysummary\",0,\"gdegreedays\"\]" |awk -F"\"" '{print $8}'`

#Store in a file so we can accumulate degree days
echo "$CITY,$STATE,$YESTERDAY,$A_GDEGREEDAYS" >> /home/newhive/data/weather/GDD_data.log


#Make Available to other scripts
#export A_GDEGREEDAYS="$A_GDEGREEDAYS"
#echo $A_GDEGREEDAYS

# Data Printers

# Data Inserter into Database
done

