# getpollen.sh
#!/bin/bash
# Script to get pollen level to support hivetool.net
# This script uses Claritin.com to get pollen data.
# Ryan Crum
# May 1, 2016

# Only run once per day at noon. The service we call changes at a time unknown after 4pm to show the next day and not the current day

#Parameters
SHELL=/bin/bash
PATH=/usr/local/sbin:/usr/local/bin:/bin:/sbin:/usr/sbin:/usr/bin:/home/HiveControl/scripts/weather

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

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


#Data Fetchers[
# Pollen Data
# Only place that is publishing a pollen count SVC/API as of May 1, 2016
#GETPollen=`/usr/bin/curl http://www.claritin.com/weatherpollenservice/weatherpollenservice.svc/getforecast/$ZIP`
GETPollen=$(curl -s 'https://www.claritin.com/webservice/allergyforecast.php?zip='$ZIP'&_=' -H 'User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2623.112 Safari/537.36'  -H 'Referer: https://www.claritin.com/allergy-forecast/')
#GETPollen=$(curl 'https://www.pollen.com/api/forecast/current/pollen/'$ZIP'' -H 'DNT: 1'  -H 'User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.86 Safari/537.36' -H 'Referer: https://www.pollen.com/forecast/current/pollen/')
#CleanData
# first, strip [
CLEAN=${GETPollen//\"[/}
# then strip ]
CLEAN=${CLEAN//]\"/}
# Remove \
CLEAN=${CLEAN//\\/}

#Stupid Claritin people have a UTF8 code stuck here, this cleans it up
POLLEN=$(echo $CLEAN | sed 's/\xEF\xBB\xBF//')
echo $POLLEN > foo.json

# Data Parsing
POL_LEVEL=$(echo $POLLEN | ./JSON.sh -b |grep 'pollenForecast\",\"forecast\",0]' |awk '{print $2}')
POL_TYPES=$(echo $POLLEN | ./JSON.sh -b |grep pollenForecast |grep pp |awk -F\" '{print $6}')

# Troubleshooting

# Data Printers, only useful if you run to see if its working
echo \"$DATE\", \"$POL_LEVEL\", \"$POL_TYPES\" 

# Data Inserter into Databases
sqlite3 $DATASOURCE "insert into pollen (date,pollenlevel,pollentypes) values (\"$DATE\",\"$POL_LEVEL\",\"$POL_TYPES\");"





