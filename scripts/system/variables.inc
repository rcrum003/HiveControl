
# Variables used throughout the scripts
# Easier to set these once and include throughout our scripts
# This is broken out by section

# Location of files

HOMEDIR=/home/hivetool2

# Data elements of the hive for use in database
# Set YARDID to match your YARDID - If sharing this data with hivetool.net, please get a new
# id from hivetool.net
YARDID=1
YARDNAME="beeyard"
# The name of the hive you are monitoring, plus a unique ID
HIVENAME="Alec"
HIVEID=3
# Use the beekeeper ID given to you by hivetool.org, or just use one if you aren't syncing with them
BEEKEEPERID=1


# =======================================
# Temper Humidity Measurement
# =======================================
# Turn on/off this feature yes/no
ENABLE_HIVE_TEMP_CHK="yes"
HIVEDEVICE=/dev/hidraw1

# =======================================
# Enable Weight Check
# =======================================
ENABLE_HIVE_WEIGHT_CHK="no"


# ========================================
# Weather Data
# ========================================
# Determine if you want to run this for every hive or once for every yard
# Options are 'yard', 'hive', or 'localws'
WEATHER_LEVEL="localws"

# Options for Weather_DETAIL include PWS vs CITY
# PWS is more accurate as it allows you to specify the closet weather station, including one you may own.
# City is the fall back
WEATHER_DETAIL=pws

# We get data from WUNDERGROUND, you will need to get a KEY for yourself
KEY="6002e68ccef1de3a"

# Set the City, state
CITY="FAIRFIELD"
STATE="CT"

# if using PWS for WEATHER_DETAIL, get the ID from Weatherundergound and put it here
WXSTATION="KCTFAIRF23"


# =========================================
# Display Data Options
# Used to determine if you want to be able to see local hive data from the logging machine
# Requires you to install apache, see HOWTO documentation
# =========================================
# Display type LOCAL or REMOTE - If remote, 
DISPLAY_TYPE=local
# Set where the files should be dropped when generated - only change if you are using some other directory, instead of the default
PUBLIC_HTML_DIR=$HOMEDIR/www/public_html


# =========================================
# Share your data options
# =========================================
# Share with hivetool.net yes or no
SHARE_HIVETOOL="no"



