#!/bin/bash
#Upgrade script HiveControl updates
# Used when going from version to version
# Includes methods to upgrade databases, as well as pull new files
# Gets the current version installed and brings up to the latest version when running the script

#Move all of this code into a script that checks for new code once a day.
# If new code is available, trigger an alert in the UI. Clicking gives instructions on how to upgrade.

#Get the latest upgrade script

Upgrade_ver="122"

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc


SKIP_CHECKUPGRADE=$1

if [[ $SKIP_CHECKUPGRADE != "SKIP" ]]; then
	#Check to see if the update.sh is at the latest version available
	checkupgrade=$( /home/HiveControl/scripts/system/checkupgrades.sh |head -1 )
		if [[ $checkupgrade = "NEWUPGRADE" ]]; then
						echo "running new upgrade file"
						AREWENEW=$(cat /home/HiveControl/upgrade.sh |grep Upgrade_ver | head -1 | awk -F\" '{print $2}')
						if [[ $AREWENEW > $Upgrade_ver ]]; then
							#We can execute
							loglocal "$DATE" UPGRADE ERROR "Using new upgrade.sh file to upgrade your hive."
							#Preventing an upgrade loop here.
							exec /home/HiveControl/upgrade.sh SKIP
							exit 1
						else
							ERROR="Upgrade attempted, but didn't have a new upgrade file"
							loglocal "$DATE" UPGRADE ERROR "$ERROR"
							echo "$ERROR"
							exit 1
						fi
		fi
fi

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

#Check to see if we are latest version
Installed_Ver=$(cat /home/HiveControl/VERSION)
Latest_Ver=$(curl -s -L -H "Cache-Control: no-cache" https://raw.githubusercontent.com/rcrum003/HiveControl/master/VERSION)

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

# Ensure required directories exist (every upgrade, not version-gated)
mkdir -p /home/HiveControl/logs
mkdir -p /home/HiveControl/data/backups

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
softwareSource="/home/HiveControl/upgrade/HiveControl/software"
softwareDest="/home/HiveControl/software"
#Remove some initial installation files from repository for upgrade
#Remove the offending file, since we don't want to upgrade these 
rm -rf $WWWTempRepo/include/db-connect.php
rm -rf $WWWTempRepo/data/* 
	echo "....... Storing it in /home/HiveControl/upgrade"
echo "============================================="


#Upgrade our DB FIRST — schema must be ready before new scripts land
#(cron may fire currconditions.sh between file copy and patch application)
#Get DBVersion
DB_ver=$(cat /home/HiveControl/data/DBVERSION)
DBPatches="/home/HiveControl/upgrade/HiveControl/patches/database"
	#Get the version available
	DB_latest_ver=$(curl -s -L -H "Cache-Control: no-cache" https://raw.githubusercontent.com/rcrum003/HiveControl/master/data/DBVERSION)

	#if [[ "$DB_ver" -lt "$DB_latest_ver" ]]; then
		echo "Checking for DB Upgrades"
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
			#Update SUDOERs drop-in
			sudo cp /home/HiveControl/upgrade/HiveControl/install/sudoers.d/hivecontrol.sudoers /etc/sudoers.d/hivecontrol
			sudo chown root:root /etc/sudoers.d/hivecontrol
			sudo chmod 440 /etc/sudoers.d/hivecontrol
			CHECKSUDO=$(sudo visudo -c 2>&1 | grep -c "parsed OK")
				if [[ $CHECKSUDO -gt 0 ]]; then
					echo "Sudoers update SUCCESS"
				else
					echo "Something went wrong with our SUDOERS file, removing it"
					sudo rm -f /etc/sudoers.d/hivecontrol
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
		if [[ $DB_ver -eq "21" ]]; then
			echo "Applying DB Ver 22 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_30
			let DB_ver="22"
		fi
		if [[ $DB_ver -eq "22" ]]; then
			echo "Applying DB Ver 23 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_31
			let DB_ver="23"
			AIR_QUALITY_NULL=$(sqlite3 $DestDB "SELECT ENABLE_AIR from hiveconfig;")			
			if [[ $AIR_QUALITY_NULL == "" || $AIR_QUALITY_NULL == "NULL" ]]; then
				#statements
				sqlite3 $DestDB "UPDATE hiveconfig SET ENABLE_AIR = 'no';"
			fi
		fi
		if [[ $DB_ver -eq "23" ]]; then
			echo "Applying DB Ver 24 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_32
			let DB_ver="24"
		fi
		if [[ $DB_ver -eq "24" ]]; then
			echo "Applying DB Ver 25 Upgrades - Weight drift compensation"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_33
			let DB_ver="25"
		fi
		if [[ $DB_ver -eq "25" ]]; then
			echo "Applying DB Ver 26 Upgrades - Weather fallback support"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_34
			let DB_ver="26"
		fi
		if [[ $DB_ver -eq "26" ]]; then
			echo "Applying DB Ver 27 Upgrades - Alert system"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_35
			let DB_ver="27"
		fi
		if [[ $DB_ver -eq "27" ]]; then
			echo "Applying DB Ver 28 Upgrades - Weather reliability (cascading fallback, health tracking)"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_36
			let DB_ver="28"
		fi
		if [[ $DB_ver -eq "28" ]]; then
			echo "Applying DB Ver 29 Upgrades - Per-provider API key storage"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_37
			let DB_ver="29"
		fi
		if [[ $DB_ver -eq "29" ]]; then
			echo "Applying DB Ver 30 Upgrades - Pollen fallback provider keys"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_38
			let DB_ver="30"
		fi
		if [[ $DB_ver -eq "30" ]]; then
			echo "Applying DB Ver 31 Upgrades - Expanded air quality"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_39
			let DB_ver="31"
		fi
		if [[ $DB_ver -eq "31" ]]; then
			echo "Applying DB Ver 32 Upgrades - EPA air quality table"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_40
			let DB_ver="32"
		fi
		if [[ $DB_ver -eq "32" ]]; then
			echo "Applying DB Ver 33 Upgrades - Air quality trend line configs"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_41
			let DB_ver="33"
		fi
		if [[ $DB_ver -eq "33" ]]; then
			echo "Applying DB Ver 34 Upgrades - Hive body stack order and sensor placement"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_42 2>/dev/null || true
			let DB_ver="34"
		fi
		if [[ $DB_ver -eq "34" ]]; then
			echo "Applying DB Ver 35 Upgrades - WX Underground API key"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_43 2>/dev/null || true
			let DB_ver="35"
		fi
		if [[ $DB_ver -eq "35" ]]; then
			echo "Applying DB Ver 36 Upgrades - Pollen enable/disable toggle"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_44 2>/dev/null || true
			let DB_ver="36"
		fi
		if [[ $DB_ver -eq "36" ]]; then
			echo "Applying DB Ver 37 Upgrades - In-hive frame feeder placement"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_45 2>/dev/null || true
			let DB_ver="37"
		fi
		if [[ $DB_ver -eq "37" ]]; then
			echo "Applying DB Ver 38 Upgrades - ZIP code for pollen and location services"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_46 2>/dev/null || true
			let DB_ver="38"
		fi

	#else
	#	echo "Skipping DB, no new database upgrades available"
	#fi
	sudo echo $DB_ver > /home/HiveControl/data/DBVERSION

# Validate schema — catches any columns missed by the patch chain
if [ -f "/home/HiveControl/upgrade/HiveControl/scripts/data/schema_validate.sh" ]; then
	bash /home/HiveControl/upgrade/HiveControl/scripts/data/schema_validate.sh "$DestDB"
fi
echo "============================================="

#Now that DB schema is up to date, copy new scripts and web files
echo "Upgrading WWW pages"
cp -Rp $WWWTempRepo/pages/* $DestWWWRepo/pages/
cp -Rp $WWWTempRepo/admin/* $DestWWWRepo/admin/
cp -Rp $WWWTempRepo/include/* $DestWWWRepo/include/
cp -Rp $WWWTempRepo/errors/* $DestWWWRepo/errors/
cp -Rp $WWWTempRepo/js/* $DestWWWRepo/js/
mkdir -p $DestWWWRepo/css
cp -Rp $WWWTempRepo/css/* $DestWWWRepo/css/
mkdir -p $DestWWWRepo/bower_components/jquery-ui/dist
cp -Rp $WWWTempRepo/bower_components/jquery-ui/dist/* $DestWWWRepo/bower_components/jquery-ui/dist/
echo "============================================="

echo "Upgrading our shell scripts"
rm -rf $scriptsource/hiveconfig.inc
cp -Rp $scriptsource/* $scriptDest/
cd $scriptDest
find . -name '*.sh' -exec chmod u+x {} +
echo "============================================="

echo "Upgrading our binaries"
cp -Rp $softwareSource/* $softwareDest/
echo "============================================="

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
	#Update SUDOERs drop-in
	sudo cp /home/HiveControl/upgrade/HiveControl/install/sudoers.d/hivecontrol.sudoers /etc/sudoers.d/hivecontrol
	sudo chown root:root /etc/sudoers.d/hivecontrol
	sudo chmod 440 /etc/sudoers.d/hivecontrol
	CHECKSUDO=$(sudo visudo -c 2>&1 | grep -c "parsed OK")

	if [[ $CHECKSUDO -gt 0 ]]; then
		echo "Sudoers update SUCCESS"
	else
		echo "Something went wrong with our SUDOERS file, removing it"
		sudo rm -f /etc/sudoers.d/hivecontrol
	fi

	#Installing DHTXX Code
	cd /home/HiveControl/software
	sudo mkdir DHTXXD
	cd DHTXXD
	# SECURITY FIX: Use HTTPS to prevent man-in-the-middle attacks
	# TODO: Verify checksum of downloaded archive before extracting
	sudo wget https://abyz.co.uk/rpi/pigpio/code/DHTXXD.zip
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

		if [[ $DB_ver -eq "20" ]]; then
			echo "Applying DB Ver21 Upgrades"
			sqlite3 $DestDB < $DBPatches/DB_PATCH_28
			let DB_ver="21"
			sudo echo $DB_ver > /home/HiveControl/data/DBVERSION
		fi

fi

if [[ "$Installed_Ver" < "1.93" ]]; then
	sudo chmod u+x /home/HiveControl/scripts/weather/wxunderground/*
fi

if [[ "$Installed_Ver" < "1.95" ]]; then
	#Set our new scripts to executable.
	sudo chmod u+x /home/HiveControl/scripts/air/*	
fi

if [[ "$Installed_Ver" < "1.98" ]]; then
	#Update PigPIO to support Raspi4
	sudo killall pigpio 	
	sudo apt purge pigpio
	sudo apt install python-setuptools python3-setuptools -y
	cd /home/HiveControl/software
	sudo git clone https://github.com/rcrum003/pigpio
	cd pigpio/
	sudo make
	sudo make install
	sudo cp /usr/local/bin/pigpiod /usr/bin/
	sudo pigpiod
fi

if [[ "$Installed_Ver" < "2.00" ]]; then

	#For some reason, who owned our data directory got changed, so let's make sure everyone is set to the right version
	chown www-data:www-data /home/HiveControl/data
	chown www-data:www-data /home/HiveControl/data/hive-data.db

	#Phidget hasn't updated to buster yet, and it's breaking everything, so let's leave it at stretch
	echo "deb http://www.phidgets.com/debian stretch main" > /etc/apt/sources.list.d/phidgets.list

	#If you got a Rasp 4 - you need an EEPROM update
	sudo apt update -y && sudo apt upgrade -y && sudo apt install rpi-eeprom rpi-eeprom-images -y
	rpi-eeprom-update -a

fi

if [[ "$Installed_Ver" < "2.02" ]]; then

	#We added support for BroodMinder, which talks via Bluetooth, so we need those tools installed.

		#Upgrade PIP, while we are at it
		sudo pip install --upgrade pip

		#Install bluepy
		sudo pip install bluepy
fi

if [[ "$Installed_Ver" < "2.04" ]]; then
	sudo chmod u+x /home/HiveControl/install/*
fi


if [[ "$Installed_Ver" < "2.06" ]]; then
	#need to update certs and stop trusting DST_Root_CA_X3
	sudo sed -i '/^mozilla\/DST_Root_CA_X3.crt$/ s/^/!/' /etc/ca-certificates.conf
	sudo update-ca-certificates -f
fi

if [[ "$Installed_Ver" < "2.10" ]]; then
	#Create a Backup folder
	mkdir /home/HiveControl/data/backups
	#Make www-data the owner
	chown www-data:www-data /home/HiveControl/data/backups


fi

if [[ "$Installed_Ver" < "2.11" ]]; then
	# Weight drift compensation scripts
	sudo chmod u+x /home/HiveControl/scripts/weight/weight_monitor.sh
	sudo chmod u+x /home/HiveControl/scripts/weight/calibrate_env_compensation.sh
	sudo chmod u+x /home/HiveControl/scripts/weight/compute_compensation.py

	# Install weight monitor cron job (every 15 min) if not already present
	CRON_ENTRY="*/15 * * * * /home/HiveControl/scripts/weight/weight_monitor.sh >> /home/HiveControl/logs/weight_monitor.log 2>&1"
	(sudo crontab -u root -l 2>/dev/null | grep -v "weight_monitor.sh"; echo "$CRON_ENTRY") | sudo crontab -u root -

	# Ensure log file exists
	sudo touch /home/HiveControl/logs/weight_monitor.log
	sudo chown root:root /home/HiveControl/logs/weight_monitor.log
fi

if [[ "$Installed_Ver" < "2.12" ]]; then
	# Detect OS architecture
	OS_ARCH=$(uname -m)
	case "$OS_ARCH" in
		aarch64|arm64|x86_64|amd64) IS_64BIT=true ;;
		*) IS_64BIT=false ;;
	esac

	PI_VERSION=$(grep -oP 'Raspberry Pi \K\d+' /proc/device-tree/model 2>/dev/null || echo "0")
	echo "Detected: $(cat /proc/device-tree/model 2>/dev/null), OS arch: $OS_ARCH"

	if [ "$IS_64BIT" = true ] || [ "$PI_VERSION" -ge 5 ]; then
		echo "64-bit OS or Pi 5+ detected — migrating from pigpio to lgpio"

		# Install lgpio
		sudo apt install python3-lgpio -y
		sudo pip3 install smbus2 spidev --break-system-packages 2>/dev/null || sudo pip3 install smbus2 spidev

		# Stop and remove pigpio
		sudo killall pigpiod 2>/dev/null || true
		sudo apt remove python3-pigpio -y 2>/dev/null || true

		# Remove pigpiod from cron
		(sudo crontab -u root -l 2>/dev/null | grep -v "pigpiod") | sudo crontab -u root -
	else
		echo "32-bit OS on Pi 4 or older — keeping pigpio, installing lgpio alongside"
		sudo apt install python3-lgpio -y 2>/dev/null || true
	fi

	# Make DHT readers executable regardless of architecture
	sudo chmod u+x /home/HiveControl/software/DHTXXD/DHTXXD_lgpio.py
	sudo chmod u+x /home/HiveControl/software/DHTXXD/DHTXXD_kernel.py

	# Make updated DHT scripts executable
	sudo chmod u+x /home/HiveControl/scripts/temp/dht22.sh
	sudo chmod u+x /home/HiveControl/scripts/temp/dht21.sh

	# Migrate legacy timezone names to canonical IANA names (Trixie removed symlinks)
	echo "Updating timezone to canonical IANA name"
	CURRENT_TZ=$(sqlite3 /home/HiveControl/data/hive-data.db "SELECT TIMEZONE FROM hiveconfig WHERE id=1")
	case "$CURRENT_TZ" in
		US/Alaska)              NEW_TZ="America/Anchorage" ;;
		US/Arizona)             NEW_TZ="America/Phoenix" ;;
		US/Mountain)            NEW_TZ="America/Denver" ;;
		US/Central)             NEW_TZ="America/Chicago" ;;
		US/Eastern)             NEW_TZ="America/New_York" ;;
		US/East-Indiana)        NEW_TZ="America/Indiana/Indianapolis" ;;
		Canada/Saskatchewan)    NEW_TZ="America/Regina" ;;
		Canada/Atlantic)        NEW_TZ="America/Halifax" ;;
		Canada/Newfoundland)    NEW_TZ="America/St_Johns" ;;
		Pacific/Samoa)          NEW_TZ="Pacific/Pago_Pago" ;;
		America/Godthab)        NEW_TZ="America/Nuuk" ;;
		Etc/Greenwich)          NEW_TZ="Etc/GMT" ;;
		Asia/Calcutta)          NEW_TZ="Asia/Kolkata" ;;
		Asia/Katmandu)          NEW_TZ="Asia/Kathmandu" ;;
		Asia/Rangoon)           NEW_TZ="Asia/Yangon" ;;
		Asia/Chongqing)         NEW_TZ="Asia/Shanghai" ;;
		Asia/Ulan_Bator)        NEW_TZ="Asia/Ulaanbaatar" ;;
		*)                      NEW_TZ="" ;;
	esac
	if [ -n "$NEW_TZ" ]; then
		sqlite3 /home/HiveControl/data/hive-data.db "UPDATE hiveconfig SET TIMEZONE='$NEW_TZ' WHERE id=1"
		echo "Timezone updated: $CURRENT_TZ -> $NEW_TZ"
	fi

	echo "lgpio migration complete"
fi

if [[ "$Installed_Ver" < "2.13" ]]; then
	# Increase data collection frequency from 60min to 15min and offset weight_monitor
	echo "Updating cron intervals"
	(sudo crontab -u root -l 2>/dev/null \
		| sed 's|^\*/60 \* \* \* \* .*/currconditions\.sh.*|*/15 * * * * /home/HiveControl/scripts/system/currconditions.sh >> /home/HiveControl/logs/currconditions.log 2>\&1|' \
		| sed 's|^\*/15 \* \* \* \* .*/weight_monitor\.sh.*|5,20,35,50 * * * * /home/HiveControl/scripts/weight/weight_monitor.sh >> /home/HiveControl/logs/weight_monitor.log 2>\&1|' \
	) | sudo crontab -u root -
	echo "Cron intervals updated: currconditions=15min, weight_monitor=offset by 5min"

	# Make new weather scripts executable
	sudo chmod u+x /home/HiveControl/scripts/weather/getwx.sh
	sudo chmod u+x /home/HiveControl/scripts/weather/openmeteo/getopenmeteo.sh
	sudo chmod u+x /home/HiveControl/scripts/weather/nws/getnws.sh
	sudo chmod u+x /home/HiveControl/scripts/weather/openweathermap/getopenweathermap.sh
	sudo chmod u+x /home/HiveControl/scripts/weather/weatherapi/getweatherapi.sh
	sudo chmod u+x /home/HiveControl/scripts/weather/visualcrossing/getvisualcrossing.sh
fi

if [[ "$Installed_Ver" < "2.15" ]]; then
	# Pollen provider chain refactor — make new pollen scripts executable
	echo "Setting up pollen provider chain scripts"
	sudo chmod u+x /home/HiveControl/scripts/weather/pollen/getpollen.sh

	# Copy new pollen icon to web images
	sudo cp /home/HiveControl/upgrade/HiveControl/www/public_html/images/pollen.png /home/HiveControl/www/public_html/images/

	# Copy new CSS (includes col-lg-fifth for 5-column dashboard)
	sudo cp /home/HiveControl/upgrade/HiveControl/www/public_html/dist/css/sb-admin-2.css /home/HiveControl/www/public_html/dist/css/sb-admin-2.css
fi

if [[ "$Installed_Ver" < "2.16" ]]; then
	# Air quality overhaul — AirNow EPA integration, expanded PurpleAir data
	echo "Setting up expanded air quality scripts"
	sudo chmod u+x /home/HiveControl/scripts/air/getairnow.sh
	sudo chmod u+x /home/HiveControl/scripts/air/air_helpers.inc

	# Update sudoers to allow www-data to run getairnow.sh (for wizard test button)
	echo "Updating sudoers for air quality scripts..."
	sudo cp /home/HiveControl/upgrade/HiveControl/install/sudoers.d/hivecontrol.sudoers /etc/sudoers.d/hivecontrol
	sudo chown root:root /etc/sudoers.d/hivecontrol
	sudo chmod 440 /etc/sudoers.d/hivecontrol

	# Add hourly AirNow cron job (runs at minute 7 to avoid overlap with currconditions)
	AIRNOW_CRON="7 * * * * mkdir -p /home/HiveControl/logs && /home/HiveControl/scripts/air/getairnow.sh >> /home/HiveControl/logs/airnow.log 2>&1"
	(sudo crontab -u root -l 2>/dev/null | grep -v "getairnow.sh"; echo "$AIRNOW_CRON") | sudo crontab -u root -
fi

if [[ "$Installed_Ver" < "2.17" ]]; then
	# Ensure logs directory exists (was missing from earlier installs)
	sudo mkdir -p /home/HiveControl/logs

	# Add mkdir guard to cron entries so they self-heal if logs dir is removed
	(sudo crontab -u root -l 2>/dev/null \
		| sed 's|^\*/15 \* \* \* \* /home/HiveControl/scripts/system/currconditions\.sh|*/15 * * * * mkdir -p /home/HiveControl/logs \&\& /home/HiveControl/scripts/system/currconditions.sh|' \
		| sed 's|^5,20,35,50 \* \* \* \* /home/HiveControl/scripts/weight/weight_monitor\.sh|5,20,35,50 * * * * mkdir -p /home/HiveControl/logs \&\& /home/HiveControl/scripts/weight/weight_monitor.sh|' \
	) | sudo crontab -u root -

	sudo chmod u+x /home/HiveControl/scripts/temp/broodminder.sh
fi

if [[ "$Installed_Ver" < "2.18" ]]; then
	# Enable Bluetooth for BroodMinder BLE scanning
	sudo systemctl enable --now bluetooth 2>/dev/null
	sudo rfkill unblock bluetooth 2>/dev/null

	# BroodMinder: migrate from bluepy (Python 2) to bleak (Python 3)
	sudo pip3 install bleak --break-system-packages 2>/dev/null || sudo pip3 install bleak 2>/dev/null || sudo apt install -y python3-bleak 2>/dev/null

	# Add mkdir guard to AirNow cron entry (was missing in 2.16 upgrade)
	(sudo crontab -u root -l 2>/dev/null \
		| sed 's|^\([0-9]* \* \* \* \*\) /home/HiveControl/scripts/air/getairnow\.sh|7 * * * * mkdir -p /home/HiveControl/logs \&\& /home/HiveControl/scripts/air/getairnow.sh|' \
	) | sudo crontab -u root -
fi

if [[ "$Installed_Ver" < "2.19" ]]; then
	# Ensure Bluetooth enabled and bleak installed (was missed in 2.18 upgrades)
	sudo systemctl enable --now bluetooth 2>/dev/null
	sudo rfkill unblock bluetooth 2>/dev/null
	sudo pip3 install bleak --break-system-packages 2>/dev/null || sudo pip3 install bleak 2>/dev/null || sudo apt install -y python3-bleak 2>/dev/null
fi

if [[ "$Installed_Ver" < "2.24" ]]; then
	# Update sudoers to allow www-data to run reboot/shutdown from web UI
	echo "Updating sudoers for restart/shutdown support"
	sudo cp /home/HiveControl/upgrade/HiveControl/install/sudoers.d/hivecontrol.sudoers /etc/sudoers.d/hivecontrol
	sudo chown root:root /etc/sudoers.d/hivecontrol
	sudo chmod 440 /etc/sudoers.d/hivecontrol
	CHECKSUDO=$(sudo visudo -c 2>&1 | grep -c "parsed OK")
	if [[ $CHECKSUDO -gt 0 ]]; then
		echo "Sudoers update SUCCESS"
	else
		echo "Something went wrong with our SUDOERS file, removing it"
		sudo rm -f /etc/sudoers.d/hivecontrol
	fi
fi

if [[ "$Installed_Ver" < "2.25" ]]; then
	# Recompile TSL2561 light sensor binary for the current OS architecture
	# (previous versions shipped a pre-compiled 32-bit ARM binary that fails on 64-bit OS)
	OS_ARCH=$(uname -m)
	case "$OS_ARCH" in
		aarch64|arm64|x86_64|amd64) IS_64BIT=true ;;
		*) IS_64BIT=false ;;
	esac

	echo "Recompiling TSL2561 light sensor binary for $OS_ARCH"
	cd /home/HiveControl/software/tsl2561
	if [ -f Makefile ]; then
		make clean
		if make; then
			sudo make install
			echo "TSL2561 compiled and installed to /usr/local/bin/2561"
		else
			echo "Warning: TSL2561 compilation failed"
			if [ "$IS_64BIT" = false ]; then
				echo "Falling back to pre-compiled 32-bit binary"
				sudo cp /home/HiveControl/software/tsl2561/2561 /usr/local/bin/2561
			else
				echo "Error: No pre-compiled 64-bit binary available — TSL2561 sensor will not work"
			fi
		fi
	else
		echo "Warning: TSL2561 Makefile not found, compiling manually"
		gcc -Wall -O2 -o TSL2561.o -c TSL2561.c && \
		gcc -Wall -O2 -o 2561.o -c 2561.c && \
		gcc -Wall -O2 -o 2561 TSL2561.o 2561.o && \
		sudo install -m 755 2561 /usr/local/bin/2561 && \
		echo "TSL2561 compiled and installed" || \
		echo "Warning: TSL2561 compilation failed"
		rm -f *.o
	fi

	echo "Recompiling TSL2591 light sensor binary for $OS_ARCH"
	cd /home/HiveControl/software/tsl2591
	if [ -f Makefile ]; then
		make clean
		if make; then
			sudo make install
			echo "TSL2591 compiled and installed to /usr/local/bin/2591"
		else
			echo "Warning: TSL2591 compilation failed"
			if [ "$IS_64BIT" = false ]; then
				echo "Falling back to pre-compiled 32-bit binary"
				sudo cp /home/HiveControl/software/binaries/2591 /usr/local/bin/2591
			else
				echo "Error: No pre-compiled 64-bit binary available — TSL2591 sensor will not work"
			fi
		fi
	else
		echo "Warning: TSL2591 Makefile not found, compiling manually"
		gcc -Wall -O2 -o 2591 2591.c && \
		sudo install -m 755 2591 /usr/local/bin/2591 && \
		echo "TSL2591 compiled and installed" || \
		echo "Warning: TSL2591 compilation failed"
	fi
fi

if [[ "$Installed_Ver" < "2.30" ]]; then
	# AHT20 (I2C) and SHT41 Trinkey (USB) temperature/humidity sensors
	# Both use zero-dependency Python drivers — no pip packages needed
	# Files are copied by the generic software/scripts cp -Rp above;
	# just ensure the new bash wrappers are executable
	echo "Setting up AHT20 and SHT41 Trinkey sensor scripts"
	sudo chmod u+x /home/HiveControl/scripts/temp/aht20.sh 2>/dev/null || true
	sudo chmod u+x /home/HiveControl/scripts/temp/sht41trinkey.sh 2>/dev/null || true
fi

if [[ "$Installed_Ver" < "2.26" ]]; then
	# USB Camera Livestream — install mjpg-streamer and update init.d service
	echo "Installing USB camera livestream support"

	sudo apt-get install -y libjpeg62-turbo-dev 2>/dev/null || sudo apt-get install -y libjpeg-dev
	sudo apt-get install -y v4l-utils

	if [ ! -f /usr/local/bin/mjpg_streamer ]; then
		echo "Building mjpg-streamer from source..."
		cd /home/HiveControl/software
		sudo git clone https://github.com/jacksonliam/mjpg-streamer.git mjpg-streamer-build
		cd mjpg-streamer-build/mjpg-streamer-experimental
		mkdir -p _build && cd _build
		cmake .. && make && sudo make install && {
			echo "mjpg-streamer installed successfully"
		} || {
			echo "Warning: mjpg-streamer build failed — USB camera livestream will not work"
		}
		cd /home/HiveControl
		rm -rf /home/HiveControl/software/mjpg-streamer-build
	else
		echo "mjpg-streamer already installed, skipping build"
	fi

	# Update the livestream init.d service to use input_uvc for USB cameras
	sudo cp /home/HiveControl/upgrade/HiveControl/install/init.d/livestream /etc/init.d/livestream
	sudo chmod +x /etc/init.d/livestream
	sudo update-rc.d livestream defaults 2>/dev/null || true

	if [ -e /dev/video0 ]; then
		echo "USB camera detected — restarting livestream service"
		sudo /etc/init.d/livestream restart
	else
		echo "No USB camera detected — livestream service updated but not started"
	fi
fi

if [[ "$Installed_Ver" < "2.33" ]]; then
	# Update sudoers to allow www-data to run getpollen.sh (for test button)
	echo "Updating sudoers for pollen test support"
	sudo cp /home/HiveControl/upgrade/HiveControl/install/sudoers.d/hivecontrol.sudoers /etc/sudoers.d/hivecontrol
	sudo chown root:root /etc/sudoers.d/hivecontrol
	sudo chmod 440 /etc/sudoers.d/hivecontrol
	CHECKSUDO=$(sudo visudo -c 2>&1 | grep -c "parsed OK")
	if [[ $CHECKSUDO -gt 0 ]]; then
		echo "Sudoers update SUCCESS"
	else
		echo "Something went wrong with our SUDOERS file, removing it"
		sudo rm -f /etc/sudoers.d/hivecontrol
	fi
fi

echo "============================================="
echo "success"
#Cleanup and set the flag in the DB
loglocal "$DATE" UPGRADE SUCCESS "Upgraded to HiveControl ver $Latest_Ver"
sqlite3 /home/HiveControl/data/hive-data.db "UPDATE hiveconfig SET upgrade_available=\"no\" WHERE id=1"

Latest_Ver=$(cat "/home/HiveControl/upgrade/HiveControl/VERSION")

#Move the VERSION
cp /home/HiveControl/upgrade/HiveControl/VERSION /home/HiveControl/

#Get latest Variable
source /home/HiveControl/scripts/hiveconfig.inc

#Advance the version
((VERSION++))

sqlite3 $DestDB "UPDATE hiveconfig SET HCVersion=$Latest_Ver, VERSION=$VERSION WHERE id=1"

#Dump our config again
/home/HiveControl/scripts/data/hiveconfig.sh



if [[ "$Installed_Ver" < "1.90" ]]; then
	#Need to reboot
		echo "========================================================"
		echo "Completed Upgrade to $Latest_Ver of HiveControl"
		echo "========================================================"
		echo "SORRY, A REBOOT IS REQUIRED NOW!"
		echo "Press ENTER to reboot : \c"
		read aok
		echo "REBOOTING...."
		/bin/sync
		/sbin/reboot

fi	

