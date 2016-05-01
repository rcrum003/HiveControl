#!/bin/bash
#Script to check if we should upgrade
# Read a flat file owned by www-data
# Command "upgrade=yes" should be placed in file
# This script will run in Cron, and check every 20 seconds
# Can also look for other commnands we may want to run.

#Implement in the database
# ID = Autoincrement
# date
# message
# status = new, processing, complete, error
# response = input any messgaes or results to return to queue requestor

#Get some common variables

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc
DB=$HOMEDIR/data/hive-data.db

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

#First get the command
cmd=$(sqlite3 $DB "SELECT message from msgqueue WHERE status='new' LIMIT 1")
read -p "Press [Enter] key to start backup..."

#Next sanitize for any stupid characters
cmd=${cmd//[^a-zA-Z0-9_]/}

echo "Cleaned Command is $cmd"
#Run through our case switch

case "$cmd" in
        upgrade)
            loglocal "$DATE" MSGQUEUE SUCCESS "Message Queue received upgrade command"
            cd $HOMEDIR
            #./upgrade.sh 2>$1
            #Empty the Queue
            echo "" > $HOMEDIR/www/.msgqueue
            ;;
        cleardata)
            loglocal "$DATE" MSGQUEUE SUCCESS "Message Queue received cleardata command"
            ;;            
        null)
            somecommand
            ;;
        *)
		#Not a valid command, so clear the queue
		loglocal "$DATE" MSGQUEUE ERROR "Message Queue received an invalid command"
		echo "" > $HOMEDIR/www/.msgqueue
            exit 1
esac




