#!/bin/bash
# Check for upgrades
# Only running once a day, when we load GDD
# v5

#Check our upgrade script
#Get the current version
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

# Validate that a GitHub response looks like the expected file content, not an error page
is_valid_github_response() {
	local content="$1"
	if [ -z "$content" ]; then
		return 1
	fi
	if echo "$content" | grep -qi "terms of service\|rate limit\|<html\|<body\|<!DOCTYPE\|scraping GitHub"; then
		return 1
	fi
	return 0
}

Upgrade_ver_file=$(cat /home/HiveControl/upgrade.sh | grep "Upgrade_ver" |head -1 |awk -F\" '{print $2}')
	#Get the version available
	upgrade_sh_response=$(curl -s -L -H "Cache-Control: no-cache" "https://raw.githubusercontent.com/rcrum003/HiveControl/master/upgrade.sh?$(date +%s)")

	if ! is_valid_github_response "$upgrade_sh_response"; then
		echo "WARNING: GitHub returned an error page when fetching upgrade.sh, skipping upgrade check"
		loglocal "$DATE" UPGRADE WARNING "GitHub rate-limited or returned error page for upgrade.sh"
		exit 0
	fi

	Upgrade_latest_ver=$(echo "$upgrade_sh_response" | grep "Upgrade_ver" | head -1 | awk -F\" '{print $2}')

	if [[ $( echo "$Upgrade_ver_file < $Upgrade_latest_ver" | bc) -eq 1 ]]; then
			echo "NEWUPGRADE"
			echo "Found a new version of upgrade.sh, downloading.."
			loglocal "$DATE" UPGRADE SUCCESS "Downloaded upgrade.sh to version - $Upgrade_latest_ver"
			echo "$upgrade_sh_response" > /home/HiveControl/upgrade.sh
	fi


#Get our current HiveControl Version
Installed_Ver=$(cat /home/HiveControl/VERSION)
echo "Installed $Installed_Ver"

Db="/home/HiveControl/data/hive-data.db"
#Get the latest HiveControl version
Latest_Ver=$(curl -s -L -H "Cache-Control: no-cache" "https://raw.githubusercontent.com/rcrum003/HiveControl/master/VERSION?$(date +%s)")

if ! is_valid_github_response "$Latest_Ver"; then
	echo "WARNING: GitHub returned an error page when fetching VERSION, skipping upgrade check"
	loglocal "$DATE" UPGRADE WARNING "GitHub rate-limited or returned error page for VERSION"
	exit 0
fi

# VERSION file should be a simple number
if ! echo "$Latest_Ver" | grep -qE '^[0-9]+\.?[0-9]*$'; then
	echo "WARNING: VERSION response is not a valid version number: $Latest_Ver"
	loglocal "$DATE" UPGRADE WARNING "GitHub returned invalid VERSION content"
	exit 0
fi

echo "Newest version is $Latest_Ver"

if [[  $(echo "$Installed_Ver == $Latest_Ver" | bc) -eq 1 ]]; then
		echo "Nothing to do, you are at the latest version"
		sqlite3 $Db "UPDATE hiveconfig SET upgrade_available=\"no\" WHERE id=1"
		exit
fi
if [[  $(echo "$Installed_Ver > $Latest_Ver" | bc) -eq 1 ]]; then
		loglocal "$DATE" UPGRADE WARNING "Upgrading disabled, You are running newer version than we have"
		echo "Hey, you are running a newer version that we have in the repository! - Exiting"
		sqlite3 $Db "UPDATE hiveconfig SET upgrade_available=\"no\" WHERE id=1"
		exit
fi
echo "We have some upgrading to do"
sqlite3 $Db "UPDATE hiveconfig SET upgrade_available='$Latest_Ver' WHERE id=1"




