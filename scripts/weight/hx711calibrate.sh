#!/bin/bash

#Script to run a long calibrate to find zero or to find the right values for slope.

#Usage: ./hx711calibrate.sh SampleNumber Weight
# Example: ./hx711calibrate.sh 10 30 
# This would perform 10 samples, and use 30 lbs/grams/kg as the weight value.

SAMPLESIZE=$1
SAMPLEWEIGHT=$2

# Get 2000 samples
count=0

echo "Clear scale of all weights, so we can calculate zero"

read -p "Press [Enter] key to start..."

echo "Collecting $SAMPLESIZE samples, this will take awhile....."

#Clear Calibrate Log
rm -rf calibrate.log

#Start looping through our count
while [ $count -lt $SAMPLESIZE ]; do

/usr/bin/sudo /usr/bin/python /home/HiveControl/scripts/weight/HX711.py  >> calibrate.log

let count=$count+1
echo "Taking Empty Sample, $count"
done

echo "**************************************************"
echo "Done Collecting Samples, when scale is empty"
echo "**************************************************"

echo "Calcuating Average Value...."

count=0;
total=0; 

for i in $( awk '{ print $1; }' calibrate.log )
   do 
     total=$(echo $total+$i | bc )
     ((count++))
   done

echo "Total Sample Size = $count"

VALUE=$( echo "scale=0; $total / $count" | bc)


echo "Average Value = $VALUE"

echo "******************************"


echo "Now place $SAMPLEWEIGHT lbs on scale"

read -p "Press [Enter] key to resume..."

#Clear old CalibrateWeughtLog
rm -rf calibrateweight.log

echo "Starting to calculate $SAMPLESIZE samples again"

count="0"
while [ $count -lt $SAMPLESIZE ]; do

/usr/bin/sudo /usr/bin/python /home/HiveControl/scripts/weight/HX711.py  >> calibrateweight.log

let count=$count+1
echo "Taking Weighted Sample, $count"
done

echo "*************************************************"
echo "Done Collecting Samples, with weight applied"
echo "*************************************************"

echo "Calcuating Average Value...."

count=0;
total=0; 

for i in $( awk '{ print $1; }' calibrateweight.log )
   do 
     total=$(echo $total+$i | bc )
     ((count++))
   done

echo "Total Sample Size = $count"

VALUEUNDERWEIGHT=$( echo "scale=0; $total / $count" | bc)

#Subtract Zero from Value Under Weight
VALUESLOPE=$( echo "scale=0; ($VALUEUNDERWEIGHT - $VALUE) / $SAMPLEWEIGHT" | bc )
#Divide by 10

echo "Average Value with Weight = $VALUEUNDERWEIGHT"

echo "******************************"

echo "Set Zero to $VALUE, Set Slope to $VALUESLOPE "


echo "******************************"
echo "Lets test those values"

WEIGHTVALUE=$(./hx711.sh $VALUE $VALUESLOPE)

echo "You told us you put $SAMPLEWEIGHT on scale"
echo "Scale returned a weight value of $WEIGHTVALUE"

echo "Finished"

