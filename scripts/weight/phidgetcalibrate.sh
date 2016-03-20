#!/bin/bash

# Get 2000 samples
count=2

while [ $count -lt 2000 ]; do

# Get base numbers from Phidget 1046 device
SCALE=`python /home/HiveControl/scripts/weight/getraw.py`
INPUT_0=`echo $SCALE | grep  -o "Weight0 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*"`
INPUT_1=`echo $SCALE | grep  -o "Weight1 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*"`
INPUT_2=`echo $SCALE | grep  -o "Weight2 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*"`
INPUT_3=`echo $SCALE | grep  -o "Weight3 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*"`


RAWWEIGHT=`echo "scale=2; ($INPUT_0 + $INPUT_1 + $INPUT_2 + $INPUT_3)" | bc`

echo "$INPUT_0, $INPUT_1, $INPUT_2, $INPUT_3, $RAWWEIGHT"
echo "$INPUT_0, $INPUT_1, $INPUT_2, $INPUT_3, $RAWWEIGHT" >> calibrate.log

let count=$count+1
echo $count
done

#echo $RAWWEIGHT

