# Script to collect PI Stats and echo to JSON for web viewing
#!/bin/bash

# Get all the settings first
#Get Temp
GETTEMPC=$(/opt/vc/bin/vcgencmd measure_temp |awk -F\= '{print $2}' |awk -F\' '{print $1}')
GETTEMPF=$(echo "scale=2; ($GETTEMPC * 9/5 + 32)" |bc)
echo "PITEMP=$GETTEMPF F"

#Get Disk Space
DISK=$(df -h |grep root |awk '{print $5}')
echo "Disk Usage=$DISK"

