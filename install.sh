#!/bin/bash

# ==================================================
# Script to automate the install of all the dependencies
# v4.1 - for HiveControl on Raspberry Pi 5 OS (64-bit)
#
# Updated January 18, 2026
#
# Must run under root
# sudo bash
# Usage: sudo ./install.sh
# ==================================================



function show_help {
	echo "--------------------"
	echo "HiveControl Installation Help"
	echo "--------------------"
	echo " -b  Use RaspiCam as a Beecounter Setup (Warning: Can take up to 8 hrs to run)"
	echo " -w  Use RaspiCam as Livestream"
	echo " -x  Install XRDP for you Windows Users"
	echo " -k  Touch Screen Keyboard Install"
	echo " -h or -?  This help message"
	echo " -d  Turn on Debugging"
	echo ""

}


function enablePICam() {
	if grep "start_x=1" /boot/config.txt
		then
		        echo "Start_x already set"
		elif grep "start_x=0" /boot/config.txt
			then
		        sed -i "s/start_x=0/start_x=1/g" /boot/config.txt
		        #reboot
		else
			 	echo "start_x=1" >> /boot/config.txt
		fi

		if grep "disable_camera_led=1" /boot/config.txt
		then
		        echo "LED Disable already set"
		else
		        sed -i "s/disable_camera_led=0/disable_camera_led=1/g" /boot/config.txt
		        echo "disable_camera_led=1" >> /boot/config.txt
		        #reboot
		        gpu_mem=128
		fi

		if grep "gpu_mem=128" /boot/config.txt
		then
				sed -i "s/gpu_mem=128/gpu_mem=256/g" /boot/config.txt
		        echo "Setting GPU to 256m"
		elif grep "gpu_mem=256" /boot/config.txt
			then
				echo "gpu_mem already set to 256"
		else
		        echo "gpu_mem=256" >> /boot/config.txt
		        #reboot
		fi

}

function installBeeCount() {
   #Enable the camera and turn off the LED
	enablePICam
	#Install the software
    /home/HiveControl/install/setupbeecounter.sh
    return

}

function install_hivecam() {
	   #Enable the camera and turn off the LED
		enablePICam
		#Install the software


}
# A POSIX variable
OPTIND=1         # Reset in case getopts has been used previously in the shell.

while getopts "h?bwdxk" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    b)  BEECOUNTER="true"
        ;;
    w)  HIVECAM="true"
    	;;
    d)  DEBUG="true"
	    ;;
	x) XRDP="true"
	    ;;
	k) KEYBOARD="true"
	    ;;
    esac
done

shift $((OPTIND-1))

[ "$1" = "--" ] && shift

if [[ $DEBUG = true ]]; then
    set -x
fi

if [[ $BEECOUNTER = true && $HIVECAM = true ]]; then
	#All hell no, can't setup the cam to be both fool, tell the user and bomb
	echo "Sorry - can't setup RaspiCam to be both a BeeCounter and HiveCam - Pick one only"
	exit 1
fi

if [[ -z $BEECOUNTER ]]; then
	#echo "Lets install the hivecam for you"
	install_hivecam
fi


#Sorry, need to run as root due to some compiling errors we get when we aren't root
whoami=$(whoami)
if [[ $whoami -ne "root" ]]; then
	echo "You must be root to install this"
	echo "run sudo bash, then rerun this script"
	exit
fi


#resize the drive, since the initial write doesn't
#Check the size of the disk
# If not big enough, then say you need to run rasp-
# Need 500,000 to install, with room for data
HDsize=$(df -BM / | awk 'NR==2 {print $4}' | sed 's/M//')
HDNeed="500"
if [[ $HDsize -lt $HDNeed ]]; then
	echo "We need at least 500 MB to install all the needed software."
	echo "Please free up more space or run raspi-config to expand your file system"
	exit
fi
# update this OS
sudo apt-get update -y
sudo apt-get upgrade -y

#Running update again because upgrade breaks it for some reason
sudo apt-get update -y


# Install the basics
# Remove Comment out if your distribution doensn't include these
#sudo apt-get install sudo -y
#sudo apt-get install nano -y
#sudo apt-get install raspi-config curl make g++ apt-utils unzip -y


# Install files needed specifically for HiveControl2

echo "========================================================================"
echo "Installing dependencies"
echo "========================================================================"
echo "Installing BC - for math and stuff"
sudo apt-get install bc -y

echo "Install git, because it's useful"
sudo apt-get install git -y

echo "Installing CMAKE"
sudo apt-get install cmake -y

echo "Installing hidapi required software"
sudo apt-get install libudev-dev -y
sudo apt-get install libusb-1.0-0-dev -y
sudo apt-get install libfox-1.6-dev -y
sudo apt-get install autotools-dev autoconf automake libtool dh-autoreconf -y
sudo apt-get install libusb-dev -y

# Install system hidapi library (instead of compiling from source)
echo "Installing hidapi library from system packages"
sudo apt-get install libhidapi-dev libhidapi-hidraw0 libhidapi-libusb0 -y

#echo "Installing Python - 2.7"
#sudo apt-get install python -y

echo "========================================================================"
echo "Getting HiveControl Code"
echo "========================================================================"
#Get the software from GIT
cd /home
sudo git clone https://github.com/rcrum003/HiveControl

echo "========================================================================"
echo "Setting up i2c"
echo "========================================================================"
sudo chmod u+x /home/HiveControl/install/setup_i2c.sh
sudo /home/HiveControl/install/setup_i2c.sh

echo "========================================================================"
echo "Installing Web Server Software"
echo "========================================================================"

#Install webserver
echo "Installing Apache/PHP"
sudo apt-get install apache2 php libapache2-mod-php -y

echo "Installing SQLite for local storage, and MariaDB client for communications"
sudo apt-get install sqlite3 php-sqlite3 -y
# Changed from mysql-client to mariadb-client for Raspberry Pi 5 OS compatibility
echo "Installing MariaDB client (replaces mysql-client)"
sudo apt-get install mariadb-client -y || echo "Warning: mariadb-client installation failed, skipping..."
sudo apt-get install jq -y

# Copy Apache Configuration for LocalHost
sudo cp /etc/apache2/sites-available/000-default.conf /home/HiveControl/install/apache/default.bckup
sudo cp /home/HiveControl/install/apache/000-default.conf /etc/apache2/sites-available/000-default.conf
sudo apachectl restart
# Setup Database to be able to be written to by Apache
sudo chown www-data:www-data /home/HiveControl/data/hive-data.db
# Dir needs to be owned by www-data as well
sudo chown www-data:www-data /home/HiveControl/data

#Ensure www-data owns all of our public_html files
sudo chown -R www-data:www-data /home/HiveControl/www/public_html/
#Install Composer
# SECURITY FIX: Download installer first, verify checksum, then execute
curl -sS https://getcomposer.org/installer -o /tmp/composer-setup.php
EXPECTED_CHECKSUM="$(curl -sS https://composer.github.io/installer.sig)"
ACTUAL_CHECKSUM="$(php -r "echo hash_file('sha384', '/tmp/composer-setup.php');")"
if [ "$EXPECTED_CHECKSUM" != "$ACTUAL_CHECKSUM" ]; then
    echo 'ERROR: Invalid installer checksum'
    rm /tmp/composer-setup.php
    exit 1
fi
sudo php /tmp/composer-setup.php --install-dir=/usr/local/bin --filename=composer
rm /tmp/composer-setup.php


echo "========================================================================"
echo "Setting up Database"
echo "========================================================================"

#Upgrade DB
cd /home/HiveControl/patches/database
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_21
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_22
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_23
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_24
sudo sqlite3 /home/HiveControl/data/hive-data.db "UPDATE hiveconfig SET RUN=\"yes\" WHERE id=\"1\";"
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_25
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_26
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_27
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_28
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_30
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_31
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_32
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_33
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_34
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_35
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_36
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_37
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_38
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_39
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_40
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_41
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_42
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_43
sudo sqlite3 /home/HiveControl/data/hive-data.db < /home/HiveControl/install/database/default_hiveconfig.sql
sudo echo 35 > /home/HiveControl/data/DBVERSION

#Setup backup directory
mkdir -p /home/HiveControl/data/backups
#Make www-data the owner
chown www-data:www-data /home/HiveControl/data/backups

#Setup logs directory
mkdir -p /home/HiveControl/logs

VER=$(cat /home/HiveControl/VERSION)
sudo sqlite3 /home/HiveControl/data/hive-data.db "UPDATE hiveconfig SET HCVersion='$VER' WHERE id=\"1\";"


#Set shell scripts to be executable
cd /home/HiveControl/
sudo find . -name '*.sh' -exec chmod u+x {} +

echo "Copying Binaries"
sudo cp /home/HiveControl/software/binaries/* /usr/local/bin/

echo "========================================================================"
echo "Compiling Code for sensors"
echo "========================================================================"


####################################################################################
# Phidget
####################################################################################
# Make Phidget software
#echo "Installing Phidget software "

#New apt-get method
#wget -qO- http://www.phidgets.com/gpgkey/pubring.gpg | apt-key add -
#Get Release
#DEB_RELEASE=$(cat /etc/os-release |grep VERSION= |awk -F\( '{print $2}' | awk -F\) '{print $1}')
#echo "deb http://www.phidgets.com/debian $DEB_RELEASE main" > /etc/apt/sources.list.d/phidgets.list
#sudo apt-get update
#sudo apt-get install libphidget22

#cd /home/HiveControl/software
#sudo mkdir Phidgets
#sudo wget https://www.phidgets.com/downloads/phidget22/libraries/any/Phidget22Python.zip
#sudo unzip Phidgets22Python.zip
#cd Phidgets22Python
#sudo python setup.py install

####################################################################################
# Tempered Software - FIXED FOR RASPBERRY PI 5
####################################################################################
echo "Installing hidapi - for Tempered Temp Sensor"
# Skip compiling hidapi from source, using system package installed earlier
echo "Using system libhidapi (already installed)"

#Make Tempered software
cd /home/HiveControl/software/
echo "Cloning TEMPered software..."
sudo git clone https://github.com/rcrum003/TEMPered-v6-2015
cd TEMPered-v6-2015

# Fix CMake to find the system hidapi library
echo "Configuring TEMPered with system hidapi..."
sudo cmake . -DHIDAPI_LIB=/usr/lib/aarch64-linux-gnu/libhidapi-libusb.so || {
    echo "Warning: CMake configuration failed. Trying alternative hidapi path..."
    sudo cmake . -DHIDAPI_LIB=/usr/lib/aarch64-linux-gnu/libhidapi-hidraw.so || {
        echo "Error: Could not configure TEMPered. Skipping tempered installation..."
        TEMPERED_FAILED=true
    }
}

if [[ ! $TEMPERED_FAILED == true ]]; then
    echo "Building TEMPered..."
    sudo make all && {
        sudo cp utils/tempered /usr/local/bin/ 2>/dev/null || echo "Warning: tempered binary not found"
        sudo cp utils/hid-query /usr/local/bin/ 2>/dev/null || echo "Warning: hid-query binary not found"
        echo "TEMPered installation completed"
    } || {
        echo "Warning: TEMPered build failed, but continuing installation..."
    }
fi

####################################################################################
# GPIO Library
####################################################################################
#WiringPI giving us errors, switching to PIGPIO
#echo "Installing wiringPI for HX711 sensor"
#cd /home/HiveControl/software/
#sudo git clone git://git.drogon.net/wiringPI
#cd wiringPI
#sudo ./build

# Detect Raspberry Pi version and OS architecture for appropriate GPIO library
PI_MODEL=$(cat /proc/device-tree/model 2>/dev/null || echo "Unknown")
PI_VERSION=$(echo "$PI_MODEL" | grep -oP 'Raspberry Pi \K\d+' || echo "0")

# Detect OS architecture (32-bit vs 64-bit)
OS_ARCH=$(uname -m)
case "$OS_ARCH" in
    aarch64|arm64|x86_64|amd64)
        IS_64BIT=true
        ;;
    *)
        IS_64BIT=false
        ;;
esac

echo "Detected: $PI_MODEL"
echo "OS Architecture: $OS_ARCH (64-bit: $IS_64BIT)"

echo "------------------------"
echo "Installing GPIO libraries"
echo "------------------------"
sudo apt install python3-lgpio -y

# Install smbus2 and spidev for I2C/SPI support
sudo pip3 install smbus2 spidev --break-system-packages 2>/dev/null || sudo pip3 install smbus2 spidev

if [ "$IS_64BIT" = true ] || [ "$PI_VERSION" -ge 5 ]; then
    echo "64-bit OS or Pi 5+ — removing pigpio (incompatible)"
    sudo killall pigpiod 2>/dev/null || true
    sudo apt remove python3-pigpio -y 2>/dev/null || true
else
    echo "32-bit OS on Pi 4 or older — keeping pigpio alongside lgpio"
fi

echo "GPIO library installation complete"

####################################################################################
# Compile architecture-specific binaries
####################################################################################
echo "------------------------"
echo "Compiling TSL2561 light sensor binary for $OS_ARCH"
echo "------------------------"
cd /home/HiveControl/software/tsl2561
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

echo "------------------------"
echo "Compiling TSL2591 light sensor binary for $OS_ARCH"
echo "------------------------"
cd /home/HiveControl/software/tsl2591
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

#Allow www-data to run python and other commands
	#Install sudoers drop-in to /etc/sudoers.d/
	echo "Installing sudoers rules for www-data..."
	sudo cp /home/HiveControl/install/sudoers.d/hivecontrol.sudoers /etc/sudoers.d/hivecontrol
	sudo chown root:root /etc/sudoers.d/hivecontrol
	sudo chmod 440 /etc/sudoers.d/hivecontrol
	CHECKSUDO=$(sudo visudo -c 2>&1 | grep -c "parsed OK")

	if [[ $CHECKSUDO -gt 0 ]]; then
		echo "Sudoers install SUCCESS"
	else
		echo "Something went wrong with our SUDOERS file, removing it"
		sudo rm -f /etc/sudoers.d/hivecontrol
	fi
####################################################################################
# DHTXXD - DHT11/DHT21/DHT22 Temperature/Humidity Sensors
####################################################################################
echo "------------------------"
echo "Installing DHT Sensor Support"
echo "------------------------"

cd /home/HiveControl/software/DHTXXD
chmod +x /home/HiveControl/software/DHTXXD/DHTXXD_lgpio.py
chmod +x /home/HiveControl/software/DHTXXD/DHTXXD_kernel.py
echo "DHT sensor support installation complete (lgpio)"

####################################################################################
# XMLStarlet used to send data to current hivetool.org Perl script
####################################################################################

#Install xmlstarlet (only needed for hivetool)
sudo apt-get install xmlstarlet -y

if [[ $XRDP == "true" ]]; then
	echo "------------------------"
	echo "Installing XRDP"
	echo "------------------------"
	#Used for folks who like to RDP to the server (aka the Nate Feature)
	sudo apt-get install xrdp -y
fi
####################################################################################
# BME680 and SHT31-D Temp Sensor Support - FIXED FOR RASPBERRY PI 5
####################################################################################
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

		# Fix missing include for ioctl
		echo "Patching sht31-d.c for missing ioctl include..."
		sudo sed -i '1i#include <sys/ioctl.h>' sht31-d.c

		sudo make && {
			sudo cp sht31-d /usr/local/bin
			echo "SHT31 driver installed successfully"
		} || {
			echo "Warning: SHT31 compilation failed, skipping..."
		}
####################################################################################


####################################################################################
# BroodMinder Temp Sensor Support - FIXED FOR RASPBERRY PI 5
####################################################################################
echo "-------------------------------"
echo "Installing Bluetooth Support for BroodMinder Devices"
echo "-------------------------------"
sudo apt-get install python3-pip -y

# Enable Bluetooth for BroodMinder BLE scanning
echo "Enabling Bluetooth..."
sudo systemctl enable --now bluetooth 2>/dev/null
sudo rfkill unblock bluetooth 2>/dev/null

# Install bleak — async BLE library for BroodMinder (replaces legacy bluepy)
echo "Installing bleak Python BLE package..."
sudo pip3 install bleak --break-system-packages 2>/dev/null || sudo pip3 install bleak 2>/dev/null || sudo apt install -y python3-bleak 2>/dev/null || echo "Warning: Could not install bleak - BroodMinder support may not work"
####################################################################################

####################################################################################
# USB Camera Livestream Support (mjpg-streamer)
####################################################################################
echo "========================================================================"
echo "Installing USB Camera Livestream Support"
echo "========================================================================"

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

sudo cp /home/HiveControl/install/init.d/livestream /etc/init.d/livestream
sudo chmod +x /etc/init.d/livestream
sudo update-rc.d livestream defaults 2>/dev/null || true

if [ -e /dev/video0 ]; then
	echo "USB camera detected at /dev/video0 — starting livestream"
	sudo /etc/init.d/livestream start
else
	echo "No USB camera detected — livestream service installed but not started"
	echo "Connect a USB camera and run: sudo /etc/init.d/livestream start"
fi

####################################################################################

if [[ $KEYBOARD = "true" ]]; then
	#Adds touch screen keyboard
echo "Installing Touch Screen Keyboard Support"
sudo apt-get install matchbox-keyboard -y
#write in the file
echo "#!/bin/bash" > /home/pi/Desktop/keyboard.sh
echo "matchbox-keyboard &" >> /home/pi/Desktop/keyboard.sh
chmod +x /home/pi/Desktop/keyboard.sh

fi

echo "========================================================"
echo "Completed Basic Setup of HiveControl"
echo "========================================================"
# Updated IP detection for modern Raspberry Pi OS
IP=$(hostname -I | awk '{print $1}')
if [[ -n $IP ]]; then
	echo "Please go to http://$IP/admin/hiveconfig.php to setup basic options"
else
	echo "Please go to http://127.0.0.1/admin/hiveconfig.php to setup basic options"
fi

#Setup Cron when we are ready to go
sudo crontab -l > /home/HiveControl/install/cron/cron.orig 2>/dev/null || touch /home/HiveControl/install/cron/cron.orig
sudo cp /home/HiveControl/install/cron/cron.orig /home/HiveControl/install/cron/cron.new
sudo cat /home/HiveControl/install/cron/hivetool.cron >> /home/HiveControl/install/cron/cron.new
sudo crontab /home/HiveControl/install/cron/cron.new


#Remove DHCP stuff, since it gets in the way of finding our machine
#apt-get remove isc-dhcp-client


#Install .htpasswd
echo "Please set an admin password for http://127.0.0.1/admin/ access:"
cd /home/HiveControl/www/
chown www-data:www-data .htpasswd
htpasswd -c .htpasswd admin
sudo service apache2 restart

#Set your timezone
dpkg-reconfigure tzdata


if [[ $BEECOUNTER = "true" ]]; then
	installBeeCount
else
	echo "-----------------------------------------------"
	echo "But wait, there is more....."
	echo "-----------------------------------------------"
	echo "Do you want to install the software required for the Bee Counter via Raspberry PI CAM?"
	echo "WARNING: This could take between 6-8 hrs due to having to compile some code!"
		select yn in "Yes" "No"; do
		    case $yn in
		        Yes ) installBeeCount; break;;
		        No ) break;;
		    esac
		done < /dev/tty
fi

echo "Installing EEPROM updates"
sudo apt install rpi-eeprom rpi-eeprom-images -y
rpi-eeprom-update -a

echo "========================================================"
echo "Completed Setup of HiveControl"
echo "========================================================"
echo "A REBOOT IS REQUIRED NOW!"
echo "Press ENTER to reboot : \c"
read aok < /dev/tty
echo "REBOOTING...."
/bin/sync
/sbin/reboot
