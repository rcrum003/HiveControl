#!/bin/bash
#Upgrade script HiveControl updates
# Used when going from version to version
# Includes methods to upgrade databases, as well as pull new files
# Gets the current version installed and brings up to the latest version when running the script

#Get the latest upgrade script
Upgrade_ver="1"
	#Get the version available
	Upgrade_latest_ver=$(curl -s https://raw.githubusercontent.com/rcrum003/HiveControl/master/upgrade.sh)

#Get our current HiveControl Version
Installed_Ver=$(cat /home/HiveControl/VERSION)
echo "Installed $Installed_Ver"

#Get the latest HiveControl version
Latest_Ver=$(curl -s https://raw.githubusercontent.com/rcrum003/HiveControl/master/VERSION)
echo "Newest version is $Latest_Ver"

if [[  $(echo "$Installed_Ver == $Latest_Ver" | bc) -eq 1 ]]; then
		echo "Nothing to do, you are at the latest version"
		exit
fi
if [[  $(echo "$Installed_Ver > $Latest_Ver" | bc) -eq 1 ]]; then
		echo "Hey, you are running a newer version that we have in the repository! - Exiting"
		exit
fi
echo "We have some upgrading to do!"

#Back everything up, just in case (mainly the database)
echo "============================================="
echo "Backing up Database to hive-data.bckup"
sudo cp /home/HiveControl/data/hive-data.db /home/HiveControl/data/hive-data.bckup
echo "============================================="

#Set some variables
WWWTempRepo="/home/HiveControl/upgrade/HiveControl/www/public_html"
DBRepo="/home/HiveControl/upgrade/data"


# Get the latest code from github into a temporary repository
echo "Getting Latest Code"
#Remove any remnants of past code upgrades
	sudo rm -rf /home/HiveControl/upgrade
#Make us a fresh directory
	sudo mkdir /home/HiveControl/upgrade
#Start in our directory
	sudo cd /home/HiveControl/upgrade
#Get the code
	sudo git clone https://github.com/rcrum003/HiveControl

#Remove some initial installation files from repository for upgrade
#Remove the offending file, since we don't want to upgrade these 
rm -rf $WWWTempRepo/include/db-connect.php
rm -rf $DBRepo/hive-data.db
rm -rf $WWWTempRepo/data/*

	echo "....... Storing it in /home/HiveControl/upgrade"
echo "============================================="


#Upgrade www
echo "Upgrading WWW pages"
#Update Includes, but not dbconnect.db

cp $WWWTempRepo 

your code here

echo "============================================="

#Upgrade our code
echo "Doing Next Thing"

your code here

echo "============================================="

#Upgrade our DB
echo "Doing Next Thing"

your code here

echo "============================================="

#Cleanup and set the flag in the DB


