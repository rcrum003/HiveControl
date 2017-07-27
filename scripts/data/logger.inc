#usage loglocal "$DATE" PROGRAM TYPE "Message"
# Example - loglocal "$DATE" SYNCHIVE ERROR "No Connection"
# include in script file
# source /home/HiveControl/scripts/data/logger.inc

function loglocal {

#Stores log entries locally
# This script takes 4 inputs and puts them into the DB
# 1 - Date
# 2 - Program
# 3 - Type (Error, Success, Warning)
# 4 - Message (Optional)

# Insert into data store
sqlite3 $HOMEDIR/data/hive-data.db "insert into logs (date,program,type,message) values (\"$1\",\"$2\",\"$3\",\"$4\");"

#Also logs to the Cloud
if [ -z "$HIVEAPI" ]; then
	#Not doing anything
	echo "No API Set, not sharing log data"
else
	LOGURL="https://www.hivecontrol.org/api/v1/hive/log"
	STATUS=$(/usr/bin/curl --silent --retry 3 --connect-timeout 30 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" -F "api_token=$HIVEAPI" -F "hive_id=$HIVEID" -F "program=$2" -F "type=$3" -F "message=$4" $LOGURL)
fi

}


