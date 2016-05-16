#!/bin/bash
# Server Status
# Version 2
# Original Author: Hivetool.org
#

# get WAN IP from router
hostname
date
echo
echo "                          * * * DISK SPACE * * *"
/bin/df
echo
echo "                           * * * INTERNET * * *"
/sbin/iwconfig wlan0
echo
/sbin/ifconfig wlan0
echo 
/sbin/ifconfig eth0
echo
echo "                        * * * NTP CLOCK SYNCH * * *"
/usr/bin/ntpq -p
echo
echo "                          * * * CPU & MEMORY * * *"
/usr/bin/top -b -n 1 | head -n 5
echo
echo "                            * * * PROCESSES * * *"
#ps -ef | grep  -e icecast -e apache -e ices | grep -v root
#/usr/bin/top -b -n 1 | grep  -e beecounter -e apache -e mjpg | grep -v root
echo
echo "                           * * * USB DEVICES * * *"
/usr/bin/lsusb
echo
echo "                            * * * IPTABLES * * *"
/sbin/iptables -L INPUT|grep DROP
echo
