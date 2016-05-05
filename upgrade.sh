#!/bin/bash
#Upgrade script HiveControl updates
# Used when going from version to version
# Includes methods to upgrade databases, as well as pull new files
# Gets the current version installed and brings up to the latest version when running the script

#Move all of this code into a script that checks for new code once a day.
# If new code is available, trigger an alert in the UI. Clicking gives instructions on how to upgrade.

#Get the latest upgrade script
Upgrade_ver="32"

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

#Check to see if we are latest version
Installed_Ver=$(cat /home/HiveControl/VERSION)
Latest_Ver=$(curl -s https://raw.githubusercontent.com/rcrum003/HiveControl/master/VERSION)

if [[  $(echo "$Installed_Ver == $Latest_Ver" | bc) -eq 1 ]]; then
		echo "Nothing to do, you are at the latest version"
		loglocal "$DATE" UPGRADE WARNING "Upgrade attempted, but nothing to upgrade. Installed is latest"
		echo "Error: Nothing to Upgrade"
		exit
fi

#Back everything up, just in case (mainly the database)
echo "Backing up Database to hive-data.bckup"
cp /home/HiveControl/data/hive-data.db /home/HiveControl/data/hive-data.bckup
echo "============================================="



# Get the latest code from github into a temporary repository
echo "Getting Latest Code"
#Remove any remnants of past code upgrades
	rm -rf /home/HiveControl/upgrade
#Make us a fresh directory
	mkdir /home/HiveControl/upgrade
#Start in our directory
	cd /home/HiveControl/upgrade
#Get the code
	git clone https://github.com/rcrum003/HiveControl &> /dev/null


#Set some variables
WWWTempRepo="/home/HiveControl/upgrade/HiveControl/www/public_html"
DestWWWRepo="/home/HiveControl/www/public_html"
DestDB="/home/HiveControl/data/hive-data.db"
scriptsource="/home/HiveControl/upgrade/HiveControl/scripts"
scriptDest="/home/HiveControl/scripts"

#Remove some initial installation files from repository for upgrade
#Remove the offending file, since we don't want to upgrade these 
rm -rf $WWWTempRepo/include/db-connect.php
rm -rf $WWWTempRepo/data/* 
	echo "....... Storing it in /home/HiveControl/upgrade"
echo "============================================="


#Upgrade www
echo "Upgrading WWW pages"
cp -Rp $WWWTempRepo/pages/* $DestWWWRepo/pages/
cp -Rp $WWWTempRepo/admin/* $DestWWWRepo/admin/
cp -Rp $WWWTempRepo/include/* $DestWWWRepo/include/
cp -Rp $WWWTempRepo/errors/* $DestWWWRepo/errors/
echo "============================================="

#Upgrade our code

echo "Upgrading our shell scripts"
#cp -R /home/HiveControl/scripts/
rm -rf $scriptsource/hiveconfig.inc
cp -Rp $scriptsource/* $scriptDest/
cd $scriptDest
find . -name '*.sh' -exec chmod u+x {} +

echo "============================================="

#Upgrade our DB
#Get DBVersion
#Get the latest upgrade script
DB_ver=$(cat /home/HiveControl/data/DBVERSION)
DBPatches="/home/HiveControl/upgrade/HiveControl/patches/database"
	#Get the version available
	DB_latest_ver=$(curl -s https://raw.githubusercontent.com/rcrum003/HiveControl/master/data/DBVERSION)

	if [[ $( echo "$DB_ver < $DB_latest_ver" | bc) -eq 1 ]]; then
		echo "Upgrading DBs"
		if [[ $DB_ver -eq "0" ]]; then
			#Upgarding to version 1
			echo "Applying DB Ver1 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_6 
			#Set DB Ver to the next
			let DB_ver="1"		
		fi
		if [[ $DB_ver -eq "1" ]]; then
			#Upgarding to version 2
			echo "Applying DB Ver2 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_7 
			#Set DB Ver to the next
			let DB_ver="2"
		fi
		if [[ $DB_ver -eq "2" ]]; then
			#Upgarding to version 2
			echo "Applying DB Ver3 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_8 
			#Set DB Ver to the next
			let DB_ver="3"
		fi
		if [[ $DB_ver -eq "3" ]]; then
			#Upgarding to version 2
			echo "Applying DB Ver4 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_9 
			#Set DB Ver to the next
			let DB_ver="4"
		fi
		if [[ $DB_ver -eq "4" ]]; then
			#Upgarding to version 2
			echo "Applying DB Ver5 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_10
			#Set DB Ver to the next
			let DB_ver="5"
		fi
		if [[ $DB_ver -eq "5" ]]; then
			#Upgarding to next version 
			echo "Applying DB Ver6 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_11
			sqlite3 $DestDB "UPDATE allhivedata SET IN_COUNT=0 WHERE OUT_COUNT is null"
			sqlite3 $DestDB "UPDATE allhivedata SET OUT_COUNT=0 WHERE OUT_COUNT is null" 
			#Set DB Ver to the next
			let DB_ver="6"
		fi
		if [[ $DB_ver -eq "6" ]]; then
			#Upgarding to next version 
			echo "Applying DB Ver7 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_12
			sqlite3 $DestDB "UPDATE allhivedata SET IN_COUNT=0 WHERE OUT_COUNT is null"
			sqlite3 $DestDB "UPDATE allhivedata SET OUT_COUNT=0 WHERE OUT_COUNT is null" 
			#Set DB Ver to the next
			let DB_ver="7"
		fi
		if [[ $DB_ver -eq "7" ]]; then
			#Upgarding to next version 
			echo "Applying DB Ver8 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_13
			#Set DB Ver to the next
			#Upgrade Cron for Message Queue and Pollen Counter
			#Get Crontab as it is
			sudo crontab -l > /home/HiveControl/install/cron/cron1.orig 
			#Echo our new content into a new crontab file with the old
			sudo cat /home/HiveControl/upgrade/HiveControl/patches/cron/CRON_PATCH_1 >> /home/HiveControl/install/cron/cron1.orig
			sudo crontab /home/HiveControl/install/cron/cron1.orig
			#Copy new images related to this feature set
			sudo cp /home/HiveControl/upgrade/HiveControl/www/public_html/images/* /home/HiveControl/www/public_html/images/
			let DB_ver="8"
		fi
			if [[ $DB_ver -eq "8" ]]; then
			#Upgarding to next version 
			echo "Applying DB Ver9 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_14
			#Set DB Ver to the next
			let DB_ver="9"
		fi
			if [[ $DB_ver -eq "9" ]]; then
			#Upgarding to next version 
			echo "Applying DB Ver10 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_15
			sqlite3 $DestDB < $DBPatches/DB_PATCH_16
			#Set DB Ver to the next
			let DB_ver="10"
		fi


	else
		echo "Skipping DB, no new database upgrades available"
	fi
	sudo echo $DB_ver > /home/HiveControl/data/DBVERSION

echo "============================================="
echo "success"
#Cleanup and set the flag in the DB
loglocal "$DATE" UPGRADE SUCCESS "Upgraded to HiveControl ver $Latest_Ver"
sqlite3 $DestDB "UPDATE hiveconfig SET upgrade_available=\"no\" WHERE id=1"
sqlite3 $DestDB "UPDATE hiveconfig SET HCVersion=$Latest_Ver WHERE id=1"

#Move the VERSION
cp /home/HiveControl/upgrade/HiveControl/VERSION /home/HiveControl/




