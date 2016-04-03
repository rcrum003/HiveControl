#!/bin/bash

# ==================================================
# Script to automate the install of all the dependencies
# v5 - for HiveControl
# 
# Must run under sudo
# Usage: sudo ./install.sh
# ==================================================


#resize the drive, since the initial write doesn't
#Check the size of the disk
# If not big enough, then say you need to run rasp-
# Need 500,000 to install, with room for data
HDsize=$(df |grep /dev/root |awk '{print $4}')
HDNeed="500000"
if [[ $HDsize -lt $HDNeed ]]; then
	echo "We need at least 500 MB to install all the needed software."
	echo "Please run raspi-config to expand your disk"
	echo "or free up enough space"
	exit 
fi
# update this OS
sudo apt-get upgrade
sudo apt-get update


# Install the basics
# Remove Comment out if your distribution doensn't include these
#sudo apt-get install sudo -y
#sudo apt-get install nano -y
#sudo apt-get install raspi-config curl make g++ apt-utils unzip -y


# Install files needed specifically for HiveControl2

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
sudo apt-get install autotools-dev autoconf automake libtool -y


echo "Installing Python - 2.7"
#sudo apt-get install python -y


#Get the software from GIT
cd /home
sudo git clone https://github.com/rcrum003/HiveControl 


echo "Installing wiringPI"
cd /home/HiveControl/software/wiringPI/
sudo ./build

echo "Copying Binaries"
sudo cp /home/HiveControl/software/binaries/* /usr/local/bin/


echo "Installing hidapi - for Temp Sensor"
# Make hidapi software
sudo cd /home/HiveControl/software/
sudo git clone https://github.com/rcrum003/hidapi
cd /home/HiveControl/software/hidapi
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
sudo make all install
sudo cp utils/tempered /usr/local/bin/

# Make Phidget software
echo "Installing Phidget software "
sudo apt-get install libusb-dev -y
cd /home/HiveControl/software/Phidget
tar -xzvf libphidget.tar.gz
cd /home/HiveControl/software/libphidget-2.1.8.20150410
sudo ./configure; make; make install

cd ../
sudo unzip PhidgetsPython.zip
cd PhidgetsPython
sudo python setup.py install


#Install webserver 
echo "Installing Apache/PHP"
sudo apt-get install apache2 php5 libapache2-mod-php5 -y

echo "Installing SQLite for local storage, and mysql-client for communications"
sudo apt-get install sqlite3 php5-sqlite -y
sudo apt-get install mysql-client -y


# Copy Apache Configuration for LocalHost
sudo cp /etc/apache2/sites-available/000-default.conf /home/HiveControl/install/apache/default.bckup
read -p "Updating default Apache config - backup in Install/apache directory - Press Any Key to Continue..."
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


#Install .htpasswd
echo "Please set an admin password for http://127.0.0.1/admin/ access:"
cd /home/HiveControl/www/
htpasswd -c .htpasswd admin


sudo service apache2 restart

#Upgrade DB
cd /home/HiveControl/patches/Database
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_1
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_2
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_3
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_4
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_5
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_6
sudo sqlite3 /home/HiveControl/data/hive-data.db < DB_PATCH_7


echo "========================================================"
echo "Completed setup"
echo "========================================================"
IP=$(ifconfig |grep "inet addr" |awk -F\: '{print $2}' |awk '{print $1}' |grep -v "127.0.0.1")
IPCount=$(ifconfig |grep "inet addr" |awk -F\: '{print $2}' |awk '{print $1}' |wc -l)
if [[ $IPCount -gt "2" ]]; then
	echo "Multiple IPs Exist, getting addresses for both eth0 and wlan0"
	IPeth0=$(ifconfig eth0 |grep "inet addr" |awk -F\: '{print $2}' |awk '{print $1}' |grep -v "127.0.0.1")
	IPwlan0=$(ifconfig wlan0 |grep "inet addr" |awk -F\: '{print $2}' |awk '{print $1}' |grep -v "127.0.0.1")
	echo "Please go to http://$IPeth0/pages/admin/hiveconfig.php or http://$IPwlan0/pages/admin/hiveconfig.php to setup basic options"
fi
if [[ $IPCount -lt "2" ]]; then
	echo "Please go to http://127.0.0.1/pages/admin/hiveconfig.php to setup basic options"
fi
if [[ $IPCount -eq "2" ]]; then
	echo "Please go to http://$IP/pages/admin/hiveconfig.php to setup basic options"
fi
echo "Then run - enablecron.sh from this directory"


#Setup Cron when we are ready to go
sudo crontab -l > /home/HiveControl/install/cron/cron.orig
sudo cp /home/HiveControl/install/cron/cron.orig /home/HiveControl/install/cron/cron.new
sudo cat /home/HiveControl/install/cron/hivetool.cron >> /home/HiveControl/install/cron/cron.new
sudo crontab /home/HiveControl/install/cron/cron.new

#If using the Adafruit DHT22
#sudo apt-get install build-essential python-dev


#Remove DHCP stuff, since it gets in the way of finding our machine
#apt-get remove isc-dhcp-client


echo "Done Executing Install Script"




