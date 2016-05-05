#!/bin/bash
#
# read the scale
# 
# v7 patch 
# Changed Count to RAW for better readability
# Added a check to make sure RAW is a number to assist setups that don't always return a number
# In this case, every couple of times, we were getting "No data to consider"
# Also, commented out the Warning
#
# Realized that patch 4 wasn't the right formula.
# This formula is now correct.

#Removed pulling new variables, as the main script does that for us
#source /home/HiveControl/scripts/hiveconfig.inc

#Added error checking, just in case a user wants to run hx711 from the command line.

HX711_ZERO=$1
HX711_CALI=$2

#-----------------------------
#Get Variables from input
#----------------------------
#Shows Help if nothing is selected
function show_help {
	echo "--------------------"
	echo "HX711 Help"
	echo "--------------------"
	echo " Usage: ./hx711.sh ZERO CALI"
	echo " Example: ./hx711.sh 4331 7332"
	echo ""

}

# A POSIX variable
OPTIND=1         # Reset in case getopts has been used previously in the shell.
if [ $# -eq 0 ]; then
    show_help
    exit 1
 fi
 
if [[ -z "$HX711_ZERO" ]] || [[ -z "$HX711_CALI" ]];  then
	 #loglocal "$DATE" WEIGHT ERROR "Unknown Error, raw2 was $RAW2 and hiveweight was $HIVEWEIGHT"	
	 show_help
	exit 1
fi
re='^-?[0-9]+([.][0-9]+)?$'
if ! [[ $HX711_ZERO =~ $re ]] || ! [[ $HX711_CALI =~ $re ]]  ; then
	show_help
   exit 1
fi

#
# read the scale
DATA_GOOD=0
COUNTER=1
while [ $COUNTER -lt 5 ] && [ $DATA_GOOD -eq 0 ]; do

RAW=$(/usr/bin/timeout 5 /usr/bin/sudo /usr/local/bin/hx711 $HX711_ZERO 2>&1)
#echo "$RAW" >> successrate	

# Check to see if Raw is a number, if not, don't do this
if [[ $RAW =~ ^-?[0-9]+$ ]] 
then	
	#echo "Passed Test"
	WEIGHT=$(echo "scale=2; ($RAW/$HX711_CALI)" | bc)
        if [ $WEIGHT ]
        then
         DATA_GOOD=1
        fi
fi
      let "COUNTER += 1"
done
#echo $COUNTER $RAW $WEIGHT

if [ $COUNTER -gt 10 ]
then
  echo "$DATE ERROR reading Scale $DEVICE"
  SCALE=-555
fi
#if test $COUNTER -gt 2
#then
#  echo "$DATE WARNING reading Scale /dev/ttyS0: retried $COUNTER"
#fi

echo "$WEIGHT"

