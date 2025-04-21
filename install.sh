#!/bin/bash

# ==================================================
# Script to automate the install of all the dependencies
# v2.02 - for HiveControl
# 
# Must run under root
# sudo bash 
# Usage: ./install.sh
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
		elif "start_x=0"
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
#HDsize=$(df |grep /dev/root |awk '{print $4}')
#HDNeed="500000"
#if [[ $HDsize -lt $HDNeed ]]; then
#	echo "We need at least 500 MB to install all the needed software."
#	echo "Please free up more space or run raspi-config to expand your file system"
#	exit
#fi
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

echo "Installing SQLite for local storage, and mysql-client for communications"
sudo apt-get install sqlite3 php-sqlite3 -y
sudo apt-get install mysql-client -y
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
curl -sS https://getcomposer.org/installer | sudo php -- --install-dir=/usr/local/bin --filename=composer


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
# Tempered Software
####################################################################################
echo "Installing hidapi - for Tempered Temp Sensor"
#Get prerequite
# Make hidapi software
cd /home/HiveControl/software/
sudo git clone https://github.com/rcrum003/hidapi

#git clone git://github.com/signal11/hidapi.git
cd /home/HiveControl/software/hidapi
sudo ./bootstrap
sudo ./configure 
sudo make
sudo make install
# Run it again, because if we run just once, it doesn't work
# Weirdest thing ever, don't have time to figure it out
# But this works #MessageMeIfYouDo
sudo ./bootstrap
sudo ./configure 
sudo make
sudo make install

#Make Tempered software
cd /home/HiveControl/software/
sudo git clone https://github.com/rcrum003/TEMPered-v6-2015
cd TEMPered-v6-2015
sudo cmake .
sudo make all
sudo cp utils/tempered /usr/local/bin/
sudo cp utils/hid-query /usr/local/bin/

####################################################################################
# GPIO Library
####################################################################################
#WiringPI giving us errors, switching to PIGPIO
#echo "Installing wiringPI for HX711 sensor"
#cd /home/HiveControl/software/
#sudo git clone git://git.drogon.net/wiringPI
#cd wiringPI
#sudo ./build

echo "Installing PIGPIO library for DHT and HX711 Sensors"
#Kill pigpiod just in case it is already running
sudo killall pigpiod
#sudo apt-get install pigpio python-pigpio python3-pigpio -y
#Added for PigPIO

sudo apt install python-setuptools python3-setuptools -y
cd /home/HiveControl/software
sudo git clone https://github.com/rcrum003/pigpio
cd pigpio/
sudo make
sudo make install
sudo cp /usr/local/bin/pigpiod /usr/bin/
sudo apt install python-pigpio python3-pigpio
sudo pigpiod

#Allow www-data to run python and other commands
	#Update SUDOERs
	sudo cp /etc/sudoers /home/HiveControl/install/sudoers.org
	sudo cp /home/HiveControl/install/sudoers.d/hivecontrol.sudoers /etc/sudoers
	CHECKSUDO=$(visudo -c -f /etc/sudoers |grep "/etc/sudoers:" |awk '{print $3}')

	if [[ $CHECKSUDO == "OK" ]]; then
		#Copy over SUDOERs file
		echo "SUCCESS"
	else
		echo "Something went wrong with our SUDOERS file, so I didn't change anything"
		sudo cp /home/HiveControl/install/sudoers.org /etc/sudoers
	fi
####################################################################################
# DHTXXD
####################################################################################
	#Installing DHTXX Code
	echo "Installing DHT Code"
	cd /home/HiveControl/software/DHTXXD
	unzip DHTXXD.zip
	sudo gcc -Wall -pthread -o DHTXXD test_DHTXXD.c DHTXXD.c -lpigpiod_if2
	sudo cp DHTXXD /usr/local/bin/

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
# BME680 and SHT31-D Temp Sensor Support
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
		sudo make
		sudo cp sht31-d /usr/local/bin
####################################################################################


####################################################################################
# BroodMinder Temp Sensor Support
####################################################################################
echo "-------------------------------"
echo "Installing Bluetooth Support for BroodMinder Devices"
echo "-------------------------------"
sudo apt-get install python-pip -y
#sudo pip install --upgrade pip

#Install bluepy
sudo pip install bluepy
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
IP=$(ifconfig |grep "inet addr" |awk -F\: '{print $2}' |awk '{print $1}' |grep -v "127.0.0.1")
IPCount=$(ifconfig |grep "inet addr" |awk -F\: '{print $2}' |awk '{print $1}' |wc -l)
if [[ $IPCount -gt "2" ]]; then
	echo "Multiple IPs Exist, getting addresses for both eth0 and wlan0"
	IPeth0=$(ifconfig eth0 |grep "inet addr" |awk -F\: '{print $2}' |awk '{print $1}' |grep -v "127.0.0.1")
	IPwlan0=$(ifconfig wlan0 |grep "inet addr" |awk -F\: '{print $2}' |awk '{print $1}' |grep -v "127.0.0.1")
	echo "Please go to http://$IPeth0/admin/hiveconfig.php or http://$IPwlan0/admin/hiveconfig.php to setup basic options"
fi
if [[ $IPCount -lt "2" ]]; then
	echo "Please go to http://127.0.0.1/admin/hiveconfig.php to setup basic options"
fi
if [[ $IPCount -eq "2" ]]; then
	echo "Please go to http://$IP/admin/hiveconfig.php to setup basic options"
fi

#Setup Cron when we are ready to go
sudo crontab -l > /home/HiveControl/install/cron/cron.orig
sudo cp /home/HiveControl/install/cron/cron.orig /home/HiveControl/install/cron/cron.new
sudo cat /home/HiveControl/install/cron/hivetool.cron >> /home/HiveControl/install/cron/cron.new
sudo crontab /home/HiveControl/install/cron/cron.new


#Remove DHCP stuff, since it gets in the way of finding our machine
#apt-get remove isc-dhcp-client


#Install .htpasswd
echo "Please set an admin password for http://127.0.0.1/admin/ access:"
cd /home/HiveControl/www/
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
		done
fi

echo "Installing EEPROM updates"
sudo apt install rpi-eeprom rpi-eeprom-images -y
rpi-eeprom-update -a

echo "========================================================"
echo "Completed Setup of HiveControl"
echo "========================================================"
echo "A REBOOT IS REQUIRED NOW!"
echo "Press ENTER to reboot : \c"
read aok
echo "REBOOTING...."
/bin/sync
/sbin/reboot










