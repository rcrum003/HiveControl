# getpollen.sh
#!/bin/bash
# Script to get pollen level to support hivetool.net
# This script uses pollen.com
# Ryan Crum
# May 1, 2016
# v1.1

# Only run once per day at noon. The service we call changes at a time unknown after 4pm to show the next day and not the current day

#Parameters
SHELL=/bin/bash
PATH=/usr/local/sbin:/usr/local/bin:/bin:/sbin:/usr/sbin:/usr/bin:/home/HiveControl/scripts/weather

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F')
DATASOURCE="$HOMEDIR/data/hive-data.db"
LOGDATE=$(TZ=":$TIMEZONE" date '+%F %T')
#Check to see if our variables are set
if [ -z "${ZIP+xxx}" ]; then 
	loglocal "$LOGDATE" POLLEN ERROR "ZIP Code is not set in Settings->Basic"
	exit
elif [ -z "$ZIP" ] && [ "${ZIP+xxx}" = "xxx" ]; then
	loglocal "$LOGDATE" POLLEN ERROR "ZIP Code is not set in Settings->Basic" 
	exit 
fi

#Data Fetchers 
GETPollen=$(curl -s 'https://www.pollen.com/api/forecast/current/pollen/'$ZIP'' -H 'DNT: 1'  -H 'User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.86 Safari/537.36' -H 'Referer: https://www.pollen.com/forecast/current/pollen/')

#Parse Data
POL_LEVEL=$(echo $GETPollen | ./JSON.sh -b |grep \"periods\",1,\"Index\" |awk '{print $2}')

	#Pollen Type Parsing
	POL_TYPE_SRC=$(echo $GETPollen | ./JSON.sh -b |grep \"periods\",1,\"Triggers\" |grep Name |awk '{print $2}')

	#Strip "
	POL_TYPE_SRC=${POL_TYPE_SRC//\"/}
	POL_TYPES=""
	while read -r POL_TYPE_SRC; do
    	#echo "... $POLLEN ..."
    	POL_TYPES+=$POL_TYPE_SRC
    	POL_TYPES+=" "
	done <<< "$POL_TYPE_SRC"

# Troubleshooting

# Data Printers, only useful if you run to see if its working
echo \"$DATE\", \"$POL_LEVEL\", \"$POL_TYPES\" 

# Data Inserter into Databases
sqlite3 $DATASOURCE "insert into pollen (date,pollenlevel,pollentypes) values (\"$DATE\",\"$POL_LEVEL\",\"$POL_TYPES\");"





