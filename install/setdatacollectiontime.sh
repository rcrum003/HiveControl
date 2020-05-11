
########################
# Welcome to HiveControl
#########################

#Script to set your preferred timing of data collection.

function show_help {
	echo "--------------------"
	echo "Set Collection Time Script Help"
	echo "--------------------"
	echo " -t 1-60 time_in_minutes you want to collect"
	echo " -h or -?  This help message"
	echo " -d  Turn on Debugging"
	echo ""
	echo "Example Usage: $0 -t 5"
	echo ""

}

# A POSIX variable
OPTIND=1         # Reset in case getopts has been used previously in the shell.

while getopts "ht?d" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    t)  TIME="$2"
        ;;    
    d)  DEBUG="true"
	    ;;
	*)  
		show_help
		exit 0
		;;
	esac
done

shift $((OPTIND-1))

[ "$1" = "--" ] && shift

if [[ $DEBUG = true ]]; then
    set -x
fi

#Check to see if we got passed a variable for time
if [ -z ${TIME+x} ]; then 
	#No variable was set
	show_help
	exit 0
fi

#Error Checking
#Check to see if the value was proper

if echo $TIME | egrep -q '^[0-9]+$'; then

	if [ $TIME -gt 0 ] && [ $TIME -lt 61 ] 2>/dev/null; then
		#Value was set between 1-60
		echo "Setting Collection time to be every $TIME minutes"
		#Create new Crontab file with the correct time
		{ printf '\n%s\n' "#Check Sensors every $TIME Minutes"; printf '%s' "*/$TIME * * * * /home/HiveControl/scripts/system/currconditions.sh > /dev/null 2>&1"; sudo crontab -l | grep -v "currconditions" |grep -v "Check Sensor"; } > /home/HiveControl/install/cron/cron1.new	
		sudo crontab /home/HiveControl/install/cron/cron1.new
	else 
		echo "Error: Time values must be between 1-60 minutes"
		show_help
		exit 0
	fi
else
	echo "Error: Time values must be a number"
        show_help
        exit 0
fi


echo "========================================================"
echo "Completed - Updated Crontab"
echo "========================================================"