#!/bin/bash
#Upgrade script HiveControl updates
# Used when going from version to version
# Includes methods to upgrade databases, as well as pull new files
# Gets the current version installed and brings up to the latest version when running the script

#Move all of this code into a script that checks for new code once a day.
# If new code is available, trigger an alert in the UI. Clicking gives instructions on how to upgrade.

#Get the latest upgrade script
Upgrade_ver="9"

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

#Check to see if we are latest version
Installed_Ver=$(cat /home/HiveControl/VERSION)
Latest_Ver=$(curl -s https://raw.githubusercontent.com/rcrum003/HiveControl/master/VERSION)

if [[  $(echo "$Installed_Ver == $Latest_Ver" | bc) -eq 1 ]]; then
		echo "Nothing to do, you are at the latest version"
		loglocal "$DATE" UPGRADE WARNING "Upgrade attempted, but nothing to upgrade. Installed is latest"
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
cp -Rup $scriptsource/* $scriptDest/*
find . -name '$scriptDest/*.sh' -exec chmod u+x {} +

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
	else
		echo "Skipping DB, no new database upgrades available"
	fi
	sudo echo $DB_ver > /home/HiveControl/data/DBVERSION

echo "============================================="

#Cleanup and set the flag in the DB
loglocal "$DATE" UPGRADE SUCCESS "Upgraded to HiveControl ver $Latest_Ver"

#Move the VERSION
cp /home/HiveControl/upgrade/HiveControl/VERSION /home/HiveControl/




