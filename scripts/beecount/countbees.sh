#/bin/bash
# Sum BeeCounter
# Checks the running total for Beecounter every 5minutes
# Author: Ryan Crum
# Date: 4-9-2016
# Supporting Hivetool.org project
# Version 1.1

BEECOUNTDIR="/home/HiveControl/scripts/beecount"
#Source some standard variables/functions
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc


DATE=$(TZ=":$TIMEZONE" date '+%F %T')


IN=$(cat $BEECOUNTDIR/myfile |awk -F"," '{print $1}')
OUT=$(cat $BEECOUNTDIR/myfile |awk -F"," '{print $2}')

#Get last value so we can calculate how many came in out during this round
LAST_IN=$(cat $BEECOUNTDIR/lastcount |awk -F"," '{print $1}')
LAST_OUT=$(cat $BEECOUNTDIR/lastcount |awk -F"," '{print $2}')

NEW_IN_COUNT=$(echo "$IN - $LAST_IN" | bc)
NEW_OUT_COUNT=$(echo "$OUT - $LAST_OUT" | bc)

if [[ $NEW_IN_COUNT < "0" ]]; then
	#check to see if the number is negative because of a restart
	#if so, restart the lastcount
	loglocal "$DATE" BEECOUNT ERROR "IN Count was less than zero"
	NEW_IN_COUNT=0
elif [[ $NEW_OUT_COUNT < "0"]]; then
	loglocal "$DATE" BEECOUNT ERROR "OUT Count was less than zero"
	NEW_OUT_COUNT=0
fi

#echo "IN = $NEW_IN_COUNT"
#echo "OUT = $NEW_OUT_COUNT"

echo "$IN,$OUT" > $BEECOUNTDIR/lastcount

echo "$NEW_IN_COUNT,$NEW_OUT_COUNT"