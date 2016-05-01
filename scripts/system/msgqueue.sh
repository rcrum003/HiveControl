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


#sqlite3 $DB "insert into msgqueue (date,message,status) values (\"$DATE\",\"upgrade\",\"new\");"


#First get the command
cmd=$(sqlite3 $DB "SELECT message from msgqueue WHERE status='new' LIMIT 1";)
cmdid=$(sqlite3 $DB "SELECT id from msgqueue WHERE status='new' LIMIT 1";)

#Next sanitize for any stupid , should happen at the website, but you never know.
cmd=${cmd//[^a-zA-Z0-9_]/}

#Run through our case switch
case "$cmd" in
        upgrade)
            loglocal "$DATE" MSGQUEUE SUCCESS "Message Queue received upgrade command"
            #Set status to processing
            sqlite3 $DB "UPDATE msgqueue SET status='processing' WHERE id=$cmdid;"
            cd $HOMEDIR
            result=$(/home/HiveControl/upgrade.sh | tail -1)
            #result=$(/home/HiveControl/scripts/system/foo.sh | tail -1)
            #Check to see if we ran
            if [[ "$result" == "success" ]]; then
            	sqlite3 $DB "UPDATE msgqueue SET response='$result', status='complete' WHERE id=$cmdid;"
            else
            	sqlite3 $DB "UPDATE msgqueue SET response='$result', status='error' WHERE id=$cmdid;"
            fi
            ;;
        cleardata)
            loglocal "$DATE" MSGQUEUE SUCCESS "Message Queue received cleardata command"
            ;;            
        null)
            somecommand
            ;;
        *)
		#Not a valid command, so clear the queue
		#loglocal "$DATE" MSGQUEUE ERROR "Message Queue received an invalid command"
            exit 1
esac




