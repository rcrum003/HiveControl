#!/bin/bash
# Script to check our various error files and display in the webapage
# Also includes functions to store in the local SQLite databases
# Future version will transmit to YardController
# Author: Ryan Crum
# Date: December 2015
# Supporting Hivetool.org
# Version: 1.0


#source common variables
source /home/HiveControl/scripts/hiveconfig.inc
DATETIME=$(TZ=":$TIMEZONE" date '+%F %T')

DATE=$(TZ=":$TIMZEONE" date '+%F')
#LOG="/home/HiveControl/logs/hivetooltest.log"

#Let's inject this data into the DB, and do all of our logic in the webpage




#First let's get successes
# Get the last Success Line, and then check the date/time
SYNC_SUCCESS=$(cat $LOG |grep SYNCHIVE |grep SUCCESS | tail -1 |awk -F- '{print $1"-"$2"-"$3}')

#Set a date to an hour ago and see if we can compare that
REPORTING=$(date -d '1 hour ago' '+%F %T' )

#Do the actual check
if [[ "$SYNC_SUCCESS" > "$REPORTING" ]]; then
	echo "Yay, within the past hour"
	echo "SYNC_SUCCESS is $SYNC_SUCCESS"
	echo "Reporting was $REPORTING"
else
	echo "Uh oh, not within past 1 hr"
	echo "SYNC_SUCCESS is $SYNC_SUCCESS"
	echo "Had to be > $REPORTING"
fi

#echo "DATETIME is $DATETIME"
#echo "REPORTING = $REPORTING"











