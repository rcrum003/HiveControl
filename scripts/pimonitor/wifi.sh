# Script to pull signal strenth

#First let's get Signal Strength
SIGNAL=$(iwconfig wlan0 |grep Link |awk '{print $4}' |awk -F\= '{print $2}' |awk -F\/ '{print $1}')

#Let's echo it for anyone calling this script
echo "Signal is $SIGNAL"

