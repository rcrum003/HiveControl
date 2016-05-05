# getpollen.sh
#!/bin/bash
# Script to get pollen level to support hivetool.net
# This script uses pollen.com
# Ryan Crum
# May 1, 2016

# Only run once per day at noon. The service we call changes at a time unknown after 4pm to show the next day and not the current day

#Parameters
SHELL=/bin/bash
PATH=/usr/local/sbin:/usr/local/bin:/bin:/sbin:/usr/sbin:/usr/bin:/home/HiveControl/scripts/weather

#source /home/HiveControl/scripts/hiveconfig.inc
#source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F')
DATASOURCE="$HOMEDIR/data/hive-data.db"

ZIP=06824

#Check to see if our variables are set
if [ -z "${ZIP+xxx}" ]; then 
	echo "VAR is not set at all"
	exit
elif [ -z "$ZIP" ] && [ "${ZIP+xxx}" = "xxx" ]; then 
	echo VAR is set but empty;
	exit 
fi

#Data Fetchers 
#GETPollen=$(curl -s 'https://www.pollen.com/api/forecast/current/pollen/'$ZIP'' -H 'DNT: 1'  -H 'User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.86 Safari/537.36' -H 'Referer: https://www.pollen.com/forecast/current/pollen/')

#Parse Data
#srcdata=$(cat pollenhistory.json | ./JSON.sh -b )

i=0

#Get the Period Data
while [[ $i < "90"  ]]; do
	period=$(cat pollenhistory.json | ./JSON.sh -b | grep ",$i," |grep Period |awk '{print $2}' |awk -FT '{print $1}')
	period=${period//\"/}
	index=$(cat pollenhistory.json | ./JSON.sh -b | grep ",$i," |grep Index |awk '{print $2}')
	echo "$period,$index,"
	let i=i+1
done

#srcdata=$(cat pollenhistory.json | ./JSON.sh -b); i=0;  data=$(echo $srcdata | grep ",$i," |grep Period |awk '{print $2}' |awk -FT '{print $1}'); echo "$data"

