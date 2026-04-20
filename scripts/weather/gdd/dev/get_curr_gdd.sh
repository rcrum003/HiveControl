#/bin/bash
# Script to calculate current gdd
# Must have a file already created with
# City,State,Date,GDD
# Data file must start at March 1 and end Sept 30

#DATA=`tail -n +2 /home/newhive/data/weather/GDD_data.log |awk -F"," '{print $5}'`

#Get the data in Celsius
DATAC=`tail -n +2 /home/newhive/data/weather/GDD_data.log |awk -F"," '{print $5}' |awk '{s+=$1} END {print s}'`

echo "Global Degree Days in C = $DATAC"

# GET the data in Farheineit
DATAF=`tail -n +2 /home/newhive/data/weather/GDD_data.log |awk -F"," '{print $4}' |awk '{s+=$1} END {print s}'`
echo "Global Degree Days in F = $DATAF"
