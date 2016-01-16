# getpollen.sh
#/bin/bash
# Script to get pollen level to support hivetool.net
# Ryan Crum
# May 15, 2015

# Only run once per day at noon. The service we call changes at a time unknown after 4pm to show the next day and not the current day


#Parameters
SHELL=/bin/bash
PATH=/usr/local/sbin:/usr/local/bin:/bin:/sbin:/usr/sbin:/usr/bin:/home/hivetool/weather

ZIP="06824"
HOST=`hostname`
DATE=`date -R`

#Data Fetchers

# Pollen Data
# Only place that is publishing a pollen count SVC/API as of May 15, 2015
GETPollen=`/usr/bin/curl http://www.claritin.com/weatherpollenservice/weatherpollenservice.svc/getforecast/$ZIP`

# Data Parsers
#Pollen
POLLEN=`echo $GETPollen | ./JSON.sh -b |awk -F"\"" '{print $18}' |cut -c 3-14 |awk -F "," '{print $1}'`
POLLENTYPE=`echo $GETPollen | ./JSON.sh -b |awk -F"\"" '{print $21}' | rev | cut -c 3- | rev`

#echo POLLEN = $POLLEN
#echo TYPE = $POLLENTYPE
#echo -----------------------

# Troubleshooting

# Data Printers
echo $HOST, $DATE, $POLLEN, $POLLENTYPE >> pollen.log

# Data Inserter into Databases

