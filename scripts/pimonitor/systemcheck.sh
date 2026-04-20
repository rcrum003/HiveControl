# Script to collect PI Stats and echo to JSON for web viewing
#!/bin/bash

# Get all the settings first
#Get Temp
GETTEMPC=$(/opt/vc/bin/vcgencmd measure_temp |awk -F\= '{print $2}' |awk -F\' '{print $1}')
GETTEMPF=$(echo "scale=2; ($GETTEMPC * 9/5 + 32)" |bc)
echo "PICHIPTEMP=$GETTEMPF F"

#Get Disk Space
DISK=$(df -h |grep rootfs |awk '{print $5}')
echo "DISK=$DISK"

DISKMAX=$(df -h |grep root |awk '{print $2}')
DISKUSED=$(df -h |grep root |awk '{print $3}')
DISKAVAIL=$(df -h |grep root |awk '{print $4}')

#Get Wifi
SIGNAL=$(iwconfig wlan0 |grep Link |awk '{print $4}' |awk -F\= '{print $2}' |awk -F\/ '{print $1}')
echo "SIGNAL=$SIGNAL"
# Access Point
SSID=$(iwconfig wlan0 |grep ESSID |awk '{print $4}' |awk -F\" '{print $2}')
echo "SSID=$SSID"

#Get WAN Address
WANIP=$(dig +short myip.opendns.com @resolver1.opendns.com)
echo "WANIP=$WANIP"

LSUSB=$(/usr/bin/lsusb)
echo "LSUSB=$LSUSB"