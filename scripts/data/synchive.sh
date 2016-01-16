#!/bin/bash
# Script to sync data from local beehives to the yard controller
source /home/hivetool2/scripts/hiveconfig.inc
source /home/hivetool2/scripts/data/logger.inc
LOCALDATABASE=/home/hivetool2/data/hive-data.db
RUNDIR=/home/hivetool2/data/

echo "$TIMEZONE"
DATE=$(TZ=":$TIMEZONE" date '+%F %T')
echo "$DATE"
#=========================================
# Query the database and pull the fields that need to be sync'd
#=========================================
# Export out to a file
sqlite3 $LOCALDATABASE "select hiveid,date,hivetempf,hivetempc,hiveHum,hiveweight,yardid,sync,beekeeperid from hivedata WHERE SYNC=1;" > $RUNDIR/syncme.sql
LOCALCOUNT=`wc $RUNDIR/syncme.sql |awk '{print $1}'`

#=========================================
# Connect to the yardcontroller and insert our data
#=========================================
MYSQLCMD=/usr/bin/mysql
CONFIG=/home/hivetool2/security/.my.cnf #This is where we store our host, username, password - this is more secure
REMOTEDB=beeyard
MYSQL="$MYSQLCMD --defaults-file=$CONFIG --local-infile=1 --show-warnings $REMOTEDB"
$MYSQL -v -v -e "load data local infile '$RUNDIR/syncme.sql' into table hivedata fields terminated by '|' (hiveid,datetime,hivetempf,hivetempc,hivehum,hiveweight,yardid,recentlyupdated,beekeeperid);"
ERROR=`echo $?`

#=========================================
# If Everything was successful, update the sync flag to be 0, so we don't try to sync it again.
#=========================================
if [[ $ERROR -ne "1"  ]]; then
	#printf "We Have A Connection \n"
	# Verify all the records we sent got there
	REMOTECOUNT=`$MYSQL -s -r -e "select COUNT(*) from hivedata WHERE recentlyupdated=1 AND hiveid=$HIVEID;"|tail -1`
	if [[ $LOCALCOUNT -eq $REMOTECOUNT ]]; then
		#printf "Yeah everything matched, we will clean up now \n"
		# Clean up local
		sqlite3 $LOCALDATABASE "UPDATE hivedata SET SYNC=0 WHERE SYNC=1;"
		# Clean up Remote
		$MYSQL -e "UPDATE hivedata SET recentlyupdated=0 WHERE recentlyupdated=1 AND hiveid=$HIVEID;"
		printf "$DATE -SYNCHIVE-SUCCESS-Successfully Synced $LOCALCOUNT Records \n" >> $LOG
		loglocal "$DATE" SYNCHIVE SUCCESS "Synced $LOCALCOUNT Records"
	else
		printf "$DATE -SYNCHIVE-ERROR-Bad record count LOCAL=$LOCALCOUNT, REMOTE=$REMOTECOUNT \n" >> $LOG
	fi
else
        printf "$DATE -SYNCHIVE-ERROR-No connection to DB\n" >> $LOG
	loglocal "$DATE" SYNCHIVE ERROR "No Connection"
fi
