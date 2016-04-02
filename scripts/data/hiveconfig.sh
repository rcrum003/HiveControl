#!/bin/bash
# Script to get variales from db to use in our scripts
# Version 2

source /home/HiveControl/scripts/data/logger.inc

#Set some default variables
LOCALDATABASE=/home/HiveControl/data/hive-data.db
CONFIGOUT="/home/HiveControl/scripts/hiveconfig.inc"
source $CONFIGOUT

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

# Check to see if the version has changed, if not , stop running

FILEVERSION=`cat $CONFIGOUT |grep -i version |awk -F\" '{print $2}'`
DBVERSION=`sqlite3 $LOCALDATABASE "select version from hiveconfig;"`

if [ $DBVERSION -eq $FILEVERSION ]; then
	echo "No Change - Versions are the same"
	exit 1
fi
if [ $FILEVERSION -gt $DBVERSION ]; then
	loglocal "$DATE" CONFIG ERROR "File Version is higher than DB Version - Did you edit manually"
	echo "ERROR: File Version is higher than DB Version - Did you edit manually?"
	exit 1
fi
# If we passed the two tests above, we can continue
# Dump to a tempfile
sqlite3 -header -line $LOCALDATABASE "SELECT * from hiveconfig INNER JOIN hiveequipmentweight on hiveconfig.id=hiveequipmentweight.id;" |sort | uniq > tempout
#Clean up said file

cat tempout |awk '{ gsub(/ = /, "=\""); print }' | sed 's/^ *//g' |awk '{print $0"\""}' > $CONFIGOUT 

loglocal "$DATE" CONFIG SUCCESS "Updated Config to Version $DBVERSION"



