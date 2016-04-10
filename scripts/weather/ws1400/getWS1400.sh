
#!/bin/bash
# Version 4
# Script to output WS-1400-IP to a JSON and XML file that matches WUNDERGROUND file format
#
# Revision 3 - Had to make only one pull to the WS1400, as it seems to not like 3 hives all pulling 3 html requests at once.
# Pretty lame, if you ask me, but it's the weather station i have.
#
#Set some variables
# Url of your WS-1400
URL="http://ws1400ip/livedata.htm"
# Record the date we pulled it - ignore what time the receiver says so we stay in sync with the local computer time
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

curl -s $URL -o webpage.html

# Get the data from the device
eval $(cat webpage.html | egrep "(in|out)(Temp|Humi)" | cut -f 4,14 -d'"' --output-delimiter='=')
eval $(cat webpage.html | egrep "(rainofhourly|uvi|AbsPress|RelPress|windir|avgwind|gustspeed|solarrad)" |cut -f 4,14 -d'"' --output-delimiter='=')
eval $(cat webpage.html | egrep "(rainofdaily)" |head -n 1 |cut -f 4,12 -d'"' --output-delimiter='=')

rm -rf webpage.html

# Do some conversions to
# My WS is setup to display F by default - below we convert to C
outTempC=$(echo "scale=1;((${outTemp}-32)*5)/9" | bc -l)


# Convert Wind then DIRECTION Degrees into Text

if [ 1 -eq "$(echo "$windir >= 0" | bc)" ] && [ 1 -eq "$(echo "$windir < 22.5" | bc)" ]; 
	then 
		DIRECTION="North" 
elif [ 1 -eq "$(echo "$windir >= 22.5" | bc)" ] && [ 1 -eq "$(echo "$windir < 67.5" | bc)" ]; 
	then 
		DIRECTION="NE" 
elif [ 1 -eq "$(echo "$windir >= 67.5" | bc)" ] && [ 1 -eq "$(echo "$windir < 112.5" | bc)" ]; 
	then 
		DIRECTION="East" 
elif [ 1 -eq "$(echo "$windir >= 112.5" | bc)" ] && [ 1 -eq "$(echo "$windir < 157.5" | bc)" ]; 
	then 
		DIRECTION="SE" 
elif [ 1 -eq "$(echo "$windir >= 157.5" | bc)" ] && [ 1 -eq "$(echo "$windir < 202.5" | bc)" ];
	then 
		DIRECTION="South" 
elif [ 1 -eq "$(echo "$windir >= 202.5" | bc)" ] && [ 1 -eq "$(echo "$windir < 247.5" | bc)" ];
	then 
		DIRECTION="SW" 
elif [ 1 -eq "$(echo "$windir >= 247.5" | bc)" ] && [ 1 -eq "$(echo "$windir < 292.5" | bc)" ];
	then 
		DIRECTION="West" 
elif [ 1 -eq "$(echo "$windir >= 292.5" | bc)" ] && [ 1 -eq "$(echo "$windir < 337.5" | bc)" ];
	then 
		DIRECTION="NW" 
else DIRECTION="North"
fi 

# convert MPH to kph per hour
wind_kph="$(echo "scale=2; ($avgwind * 1.609344)/1" | bc)"
wind_gust_kph="$(echo "scale=2; ($gustspeed * 1.609344)/1" | bc)"


#Convert Pressure in to MB
pressure_mb="$(echo "scale=0; ($AbsPress * 33.8637526)/1" | bc)"

#Calculate the dewpoint F
# Formula:  TDEW = T - 9(100-RH%) / 25
# Get the integers of the temperature and humidity

dewpoint_f="$(echo "scale=2; (${outTemp} - ( 9* (100 - ${outHumi})) /25)" |bc)"
dewpoint_c=$(echo "scale=1;((${dewpoint_f}-32)*5)/9" | bc -l)


#Convert Rain Inches for Hour and Day to Metric
precip_1hr_metric="$(echo "scale=2; (${rainofhourly} / 0.039370)" |bc)"
precip_today_metric="$(echo "scale=2; (${rainofdaily} / 0.039370)" |bc)"

if [[ -z "${outTemp}" ]]; then
	loglocal "$DATE" WEATHER ERROR "Did not get a proper outTemp" 
	
	#Since we didn't get a proper file, we should assume the whole thing is borked
	# so we exit
	exit
else
	#loglocal "$DATE" WEATHER SUCCESS "We are good"
fi
	
# Return a JSON file to mimic WUNderground file format
echo "{"
echo "\"response\": {"
echo "\"version\":\"0.1\","
echo "\"features\": {"
echo "\"conditions\": 1"
echo "  }"
echo "	}"
echo "  ,	\"current_observation\": {"
echo "		\"station_id\":\"KCTFAIRF23\","
echo "		\"observation_time\":\"$DATE\","
echo "		\"temperature_string\":\"${outTemp} F ($outTempC C)\","
echo "		\"temp_f\":\"${outTemp}\","
echo "		\"temp_c\":\"$outTempC\","
echo "		\"relative_humidity\":\"${outHumi}%\","
echo "		\"wind_string\":\" $DIRECTION at $avgwind MPH, Gust to ${gustspeed}\","
echo "		\"wind_dir\":\"$DIRECTION\","
echo "		\"wind_degrees\":\"${windir}\","
echo "		\"wind_mph\":\"$avgwind\","
echo "		\"wind_gust_mph\":\"${gustspeed}\","
echo "		\"wind_kph\":\"$wind_kph\","
echo "		\"wind_gust_kph\":\"$wind_gust_kph\","
echo "		\"pressure_mb\":\"$pressure_mb\","
echo "		\"pressure_in\":\"${AbsPress}\","
echo "		\"pressure_trend\":\"-\","
echo "		\"dewpoint_f\":\"$dewpoint_f\","
echo "		\"dewpoint_c\":\"$dewpoint_c\","
echo "		\"windchill_string\":\"NA\","
echo "		\"windchill_f\":\"NA\","
echo "		\"windchill_c\":\"NA\","
echo "		\"solarradiation\":\"${solarrad}\","
echo "		\"UV\":\"${uvi}\","
echo "		\"precip_1hr_in\":\"${rainofhourly}\","
echo "		\"precip_1hr_metric\":\"$precip_1hr_metric\","
echo "		\"precip_today_string\":\"${rainofdaily} in ($precip_today_metric mm)\","
echo "		\"precip_today_in\":\"${rainofdaily}\","
echo "		\"precip_today_metric\":\"$precip_today_metric\""
echo " }"
echo "}"

# Generate an XML file for use by Hivetool, since they like that also

echo "
<response>
  <current_observation>
		<display_location>
		<full>$CITY, $STATE</full>
		<city>$CITY</city>
		<state>$STATE</state>
		<country>US</country>
		<latitude>$LATITUDE</latitude>
		<longitude>$LONGITUDE</longitude>
		</display_location>
		<station_id>$WXSTATION</station_id>
		<temperature_string>${outTemp} F ($outTempC C)</temperature_string>
		<temp_f>${outTemp}</temp_f>
		<temp_c>$outTempC</temp_c>
		<relative_humidity>${outHumi}%</relative_humidity>
		<wind_string>$DIRECTION at $avgwind MPH, Gust to ${gustspeed}</wind_string>
		<wind_dir>$DIRECTION</wind_dir>
		<wind_degrees>${windir}</wind_degrees>
		<wind_mph>${avgwind}</wind_mph>
		<wind_gust_mph>${gustspeed}</wind_gust_mph>
		<wind_kph>$wind_kph</wind_kph>
		<wind_gust_kph>$wind_gust_kph</wind_gust_kph>
		<pressure_mb>$pressure_mb</pressure_mb>
		<pressure_in>${AbsPress}</pressure_in>
		<pressure_trend>-</pressure_trend>
		<dewpoint_f>$dewpoint_f</dewpoint_f>
		<dewpoint_c>$dewpoint_c</dewpoint_c>
		<windchill_string>NA</windchill_string>
		<windchill_f>NA</windchill_f>
		<windchill_c>NA</windchill_c>
	        <solarradiation>${solarrad}</solarradiation>
		<UV>${uvi}</UV>
		<precip_1hr_in>${rainofhourly}</precip_1hr_in>
		<precip_1hr_metric>$precip_1hr_metric</precip_1hr_metric>
		<precip_today_string>${rainofdaily} in ($precip_today_metric mm)</precip_today_string>
		<precip_today_in>${rainofdaily}</precip_today_in>
		<precip_today_metric>$precip_today_metric</precip_today_metric>
	</current_observation>
</response>" > wx.xml
