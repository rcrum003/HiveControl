#!/bin/bash
# Get base numbers from Phidget 1046 device
# Version 2 
# Supports setting the Zero/Multiplier via the webpage

source /home/HiveControl/scripts/hiveconfig.inc

ZERO="$HIVE_WEIGHT_INTERCEPT"
MULTIPLIER="$HIVE_WEIGHT_SLOPE"

#ZERO="0.05700"
#MULTIPLIER="57.4545"


SCALE=`python /home/HiveControl/scripts/weight/getrawphidget.py`
INPUT_0=`echo $SCALE | grep  -o "Weight0 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*"`
INPUT_1=`echo $SCALE | grep  -o "Weight1 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*"`
INPUT_2=`echo $SCALE | grep  -o "Weight2 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*"`
INPUT_3=`echo $SCALE | grep  -o "Weight3 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*"`

RAWVALUE=`echo "scale=2; ($INPUT_0 + $INPUT_1 + $INPUT_2 + $INPUT_3)" | bc`
CALCWEIGHT=`echo "scale=2; (( $RAWVALUE - $ZERO ) * $MULTIPLIER)" | bc`
RAWWEIGHT=$(echo "scale=2; ($CALCWEIGHT/1)" | bc)

echo $RAWWEIGHT

