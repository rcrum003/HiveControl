#!/bin/bash
# Check for upgrades
# Only running once a day, when we load GDD
# v4

#Check our upgrade script
#Get the current version
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

Upgrade_ver=$(cat /home/HiveControl/upgrade.sh | grep "Upgrade_ver" |awk -F\" '{print $2}')
	#Get the version available
	Upgrade_latest_ver=$(curl -s https://raw.githubusercontent.com/rcrum003/HiveControl/master/upgrade.sh |grep "Upgrade_ver" |awk -F\" '{print $2}')

	if [[ $( echo "$Upgrade_ver < $Upgrade_latest_ver" | bc) -eq 1 ]]; then
			echo "Found a new version of upgrade.sh, downloading.."
			loglocal "$DATE" UPGRADE SUCCESS "Downloaded upgrade.sh to version - $Upgrade_latest_ver"
			curl -s https://raw.githubusercontent.com/rcrum003/HiveControl/master/upgrade.sh -o /home/HiveControl/upgrade.sh
	fi


#Get our current HiveControl Version
Installed_Ver=$(cat /home/HiveControl/VERSION)
echo "Installed $Installed_Ver"

Db="/home/HiveControl/data/hive-data.db"
#Get the latest HiveControl version
Latest_Ver=$(curl -s https://raw.githubusercontent.com/rcrum003/HiveControl/master/VERSION)
echo "Newest version is $Latest_Ver"

if [[  $(echo "$Installed_Ver == $Latest_Ver" | bc) -eq 1 ]]; then
		echo "Nothing to do, you are at the latest version"
		sqlite3 $Db "UPDATE hiveconfig SET upgrade_available=\"no\" WHERE id=1"
		exit
fi
if [[  $(echo "$Installed_Ver > $Latest_Ver" | bc) -eq 1 ]]; then
		loglocal "$DATE" UPGRADE WARNING "Upgrading disabled, You are running newer version than we have"
		echo "Hey, you are running a newer version that we have in the repository! - Exiting"
		sqlite3 $Db "UPDATE hiveconfig SET upgrade_available=\"no\" WHERE id=1"
		exit
fi
echo "We have some upgrading to do"
sqlite3 $Db "UPDATE hiveconfig SET upgrade_available='$Latest_Ver' WHERE id=1"




