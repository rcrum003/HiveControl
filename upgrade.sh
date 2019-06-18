#!/bin/bash
#Upgrade script HiveControl updates
# Used when going from version to version
# Includes methods to upgrade databases, as well as pull new files
# Gets the current version installed and brings up to the latest version when running the script

#Move all of this code into a script that checks for new code once a day.
# If new code is available, trigger an alert in the UI. Clicking gives instructions on how to upgrade.

#Get the latest upgrade script

Upgrade_ver="84"

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc


#Check to see if the update.sh is at the latest version available
checkupgrade=$(/home/HiveControl/scripts/system/checkupgrades.sh |head -1)
	if [[ $checkupgrade = "NEWUPGRADE" ]]; then
					echo "running new upgrade file"
					exec /home/HiveControl/upgrade.sh
					exit
	fi


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
			if [[ $DB_ver -eq "10" ]]; then
			echo "Ok, so not a DB upgrade, but only needs to be once and this was the best place for it"
			sudo echo "www-data ALL=(ALL) NOPASSWD: /usr/local/bin/hx711" >> /etc/sudoers
			let DB_ver="11"
		fi
		if [[ $DB_ver -eq "11" ]]; then
			echo "Applying DB Ver12 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_17
			let DB_ver="12"
		fi
		if [[ $DB_ver -eq "12" ]]; then
			echo "Applying DB Ver13 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_18 
			sudo crontab -l > /home/HiveControl/install/cron/cron2.orig 
			#Echo our new content into a new crontab file with the old
			sudo cat /home/HiveControl/upgrade/HiveControl/patches/cron/CRON_PATCH_2 >> /home/HiveControl/install/cron/cron2.orig
			sudo crontab /home/HiveControl/install/cron/cron2.orig
			#Copy new images related to this feature set
			sudo cp /home/HiveControl/upgrade/HiveControl/www/public_html/images/* /home/HiveControl/www/public_html/images/
			let DB_ver="13"
		fi

		if [[ $DB_ver -eq "13" ]]; then
			echo "Applying DB Ver14 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_19 
			sudo sqlite3 $DestDB "UPDATE hiveconfig SET RUN=\"yes\";"
			sudo cp /home/HiveControl/upgrade/HiveControl/www/public_html/images/* /home/HiveControl/www/public_html/images/
			let DB_ver="14"
		fi

		if [[ $DB_ver -eq "14" ]]; then
			echo "Applying DB Ver15 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_20 
			let DB_ver="15"
		fi
		if [[ $DB_ver -eq "15" ]]; then
			echo "Applying DB Ver16 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_21 
			sqlite3 $DestDB < $DBPatches/DB_PATCH_22
			sqlite3 $DestDB < $DBPatches/DB_PATCH_23
			sqlite3 $DestDB < $DBPatches/DB_PATCH_24
			#Update SUDOERs
			sudo cp /etc/sudoers /home/HiveControl/install/sudoers.org
			sudo cp /home/HiveControl/upgrade/HiveControl/install/sudoers.d/hivecontrol.sudoers /etc/sudoers
			CHECKSUDO=$(sudo visudo -c -f /etc/sudoers |grep "/etc/sudoers:" |awk '{print $3}')
				if [[ $CHECKSUDO == "OK" ]]; then
					#Copy over SUDOERs file
					echo "SUCCESS"
				else
					echo "Something went wrong with our SUDOERS file, so I didn't change anything"
					echo $CHECKSUDO >> /home/HiveControl/sudoerror
					sudo cp /home/HiveControl/install/sudoers.org /etc/sudoers
				fi
			let DB_ver="16"
		fi
		if [[ $DB_ver -eq "16" ]]; then
			echo "Applying DB Ver17 Upgrades"
			sudo cp /home/HiveControl/upgrade/HiveControl/software/tsl2561/2561 /usr/local/bin
			let DB_ver="17"
		fi
		if [[ $DB_ver -eq "17" ]]; then
			echo "Applying DB Ver18 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_25
			let DB_ver="18"
		fi
		if [[ $DB_ver -eq "18" ]]; then
			echo "Applying DB Ver19 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_26
			let DB_ver="19"
		fi
		if [[ $DB_ver -eq "19" ]]; then
			echo "Applying DB Ver20 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_27
			let DB_ver="20"
		fi
		if [[ $DB_ver -eq "20" ]]; then
			echo "Applying DB Ver21 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_28
			let DB_ver="21"
		fi

	else
		echo "Skipping DB, no new database upgrades available"
	fi
	sudo echo $DB_ver > /home/HiveControl/data/DBVERSION

#Update install stuff
sudo cp -Rp /home/HiveControl/upgrade/HiveControl/install/* /home/HiveControl/install/

if [[ "$Installed_Ver" < "1.62" ]]; then
	#Only run this if we haven't got the latest code
	mkdir /home/HiveControl/install/init.d
	sudo cp -Rp /home/HiveControl/upgrade/HiveControl/install/init.d/* /home/HiveControl/install/init.d/
	sudo mkdir /home/HiveControl/software/beecamcounter
	sudo cp -Rp /home/HiveControl/upgrade/HiveControl/software/beecamcounter/* /home/HiveControl/software/beecamcounter/ 
fi
if [[ "$Installed_Ver" < "1.68" ]]; then
	#Only run this if we haven't got the latest code
	#Install PIGPIO
	####################################################################################
	# GPIO Library
	####################################################################################

	echo "Installing PIGPIO library for DHT and HX711 Sensors"
	#Kill pigpiod just in case it is already running
	sudo killall pigpiod
	#Get code
	cd /home/HiveControl/software
	sudo wget https://github.com/joan2937/pigpio/archive/master.zip
	sudo unzip master.zip
	cd pigpio-master
	make -j4
	sudo make install

	#sudo apt-get install python-pigpio python3-pigpio -y 
	#Update SUDOERs
	sudo cp /etc/sudoers /home/HiveControl/install/sudoers.org
	sudo cp /home/HiveControl/upgrade/HiveControl/install/sudoers.d/hivecontrol.sudoers /etc/sudoers
	CHECKSUDO=$(visudo -c -f /etc/sudoers |grep "/etc/sudoers:" |awk '{print $3}')

	if [[ $CHECKSUDO == "OK" ]]; then
		#Copy over SUDOERs file
		echo "SUCCESS"
	else
		echo "Something went wrong with our SUDOERS file, so I didn't change anything"
		sudo cp /home/HiveControl/install/sudoers.org /etc/sudoers
	fi

	#Installing DHTXX Code
	cd /home/HiveControl/software
	sudo mkdir DHTXXD
	cd DHTXXD
	sudo wget http://abyz.co.uk/rpi/pigpio/code/DHTXXD.zip
	unzip DHTXXD.zip
	sudo gcc -Wall -pthread -o DHTXXD test_DHTXXD.c DHTXXD.c -lpigpiod_if2
	sudo cp DHTXXD /usr/local/bin/
fi

if [[ "$Installed_Ver" < "1.71" ]]; then
	sudo crontab -l > /home/HiveControl/install/cron/cron.orig
	sudo cp /home/HiveControl/install/cron/cron.orig /home/HiveControl/install/cron/cron.new
	sudo echo "@reboot           /usr/local/bin/pigpiod" >> /home/HiveControl/install/cron/cron.new
	sudo crontab /home/HiveControl/install/cron/cron.new
fi

if [[ "$Installed_Ver" < "1.85" ]]; then
	sudo apt-get update
	#Install Jq to work better with JSON data
	sudo apt-get install jq -y
	
fi

if [[ "$Installed_Ver" < "1.90" ]]; then
	#Install 1.90 Version
		
		#Lets update all of our libraries at the end, so the reboot don't mess with our Jo-Jo
		sudo apt-get update

				#install i2c - do this last becasue we may reboot
		sudo chmod u+x /home/HiveControl/install/setup_i2c.sh
		sudo /home/HiveControl/install/setup_i2c.sh
		
		echo "-------------------------------"
		echo "Installing new BME680 Drivers"
		echo "-------------------------------"
		#Copy the code, #BME Drive includes modified code for our specific output.
		cd /home/HiveControl/software
		sudo git clone https://github.com/rcrum003/BME680_driver
		cd BME680_driver
		sudo gcc bme680_main.c bme680.c -o bme680
		sudo cp bme680 /usr/local/bin

		echo "-------------------------------"
		echo "Installing new SHT Drivers"
		echo "-------------------------------"
		cd /home/HiveControl/software
		sudo git clone https://github.com/rcrum003/Adafruit-sht31-for-PI
		cd Adafruit-sht31-for-PI/
		sudo make
		sudo cp sht31-d /usr/local/bin

fi


echo "============================================="
echo "success"
#Cleanup and set the flag in the DB
loglocal "$DATE" UPGRADE SUCCESS "Upgraded to HiveControl ver $Latest_Ver"
sqlite3 $DestDB "UPDATE hiveconfig SET upgrade_available=\"no\" WHERE id=1"
sqlite3 $DestDB "UPDATE hiveconfig SET HCVersion=$Latest_Ver WHERE id=1"

#Move the VERSION
cp /home/HiveControl/upgrade/HiveControl/VERSION /home/HiveControl/




