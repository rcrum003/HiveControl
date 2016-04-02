#usage loglocal "$DATE" PROGRAM TYPE "Message"
# Example - loglocal "$DATE" SYNCHIVE ERROR "No Connection"

function loglocal {

#Stores log entries locally
# This script takes 4 inputs and puts them into the DB
# 1 - Date
# 2 - Program
# 3 - Type (Error, Success, Warning)
# 4 - Message (Optional)

# Insert into data store
sqlite3 $HOMEDIR/data/hive-data.db "insert into logs (date,program,type,message) values (\"$1\",\"$2\",\"$3\",\"$4\");"

}

