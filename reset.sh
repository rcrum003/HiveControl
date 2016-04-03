#!/bin/bash
# Reset.sh
# V1
# Resets allhivedata table 
# Useful when you finally get all the hive components working and you want to clear out the test data

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

function erasedata {

sqlite3 $HOMEDIR/data/hive-data.db "DELETE from allhivedata;"
sqlite3 $HOMEDIR/data/hive-data.db "DELETE from hivedata;"
sqlite3 $HOMEDIR/data/hive-data.db "DELETE from weather;"
loglocal "$DATE" RESET SUCCESS "Hive Data was reset"
}


echo "===================================================="
echo "WARNING: This will delete all hive readings"

echo "Are you sure you want to continue?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) erasedata;;
        No ) exit;;
    esac
done