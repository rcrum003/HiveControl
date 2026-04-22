#!/bin/bash
#
# Development Branch Upgrade Script for HiveControl
#
# Pulls from the 'dev' branch instead of 'master', bypasses the version
# check, and runs the full upgrade pipeline. Use this on test hives to
# validate changes before merging to master.
#
# Usage:
#   sudo ./upgrade-dev.sh              # pull from 'dev' branch
#   sudo ./upgrade-dev.sh feature/foo  # pull from a specific branch
#

set -eo pipefail

BRANCH="${1:-dev}"
REPO="https://github.com/rcrum003/HiveControl"

source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE=$(TZ=":$TIMEZONE" date '+%F %T')
Installed_Ver=$(cat /home/HiveControl/VERSION)

echo "============================================="
echo "  HiveControl DEV Upgrade"
echo "  Branch:    $BRANCH"
echo "  Installed: $Installed_Ver"
echo "============================================="

# Back up the database
echo "Backing up database..."
cp /home/HiveControl/data/hive-data.db /home/HiveControl/data/hive-data.bckup
echo "Backup saved to hive-data.bckup"
echo "============================================="

mkdir -p /home/HiveControl/logs
mkdir -p /home/HiveControl/data/backups

# Clone the dev branch
echo "Getting code from branch: $BRANCH"
rm -rf /home/HiveControl/upgrade
mkdir /home/HiveControl/upgrade
cd /home/HiveControl/upgrade
git clone --branch "$BRANCH" --single-branch "$REPO" 2>&1 | tail -1

if [ ! -d "/home/HiveControl/upgrade/HiveControl" ]; then
    echo "ERROR: git clone failed — branch '$BRANCH' may not exist"
    loglocal "$DATE" UPGRADE ERROR "Dev upgrade failed: branch '$BRANCH' not found"
    exit 1
fi

Latest_Ver=$(cat /home/HiveControl/upgrade/HiveControl/VERSION)
echo "Branch version: $Latest_Ver"
echo "============================================="

# Set path variables (same as upgrade.sh)
WWWTempRepo="/home/HiveControl/upgrade/HiveControl/www/public_html"
DestWWWRepo="/home/HiveControl/www/public_html"
DestDB="/home/HiveControl/data/hive-data.db"
scriptsource="/home/HiveControl/upgrade/HiveControl/scripts"
scriptDest="/home/HiveControl/scripts"
softwareSource="/home/HiveControl/upgrade/HiveControl/software"
softwareDest="/home/HiveControl/software"
DBPatches="/home/HiveControl/upgrade/HiveControl/patches/database"

# Don't overwrite local db-connect.php or data
rm -rf $WWWTempRepo/include/db-connect.php
rm -rf $WWWTempRepo/data/*

# ─── Database Patches ───────────────────────────────────────
DB_ver=$(cat /home/HiveControl/data/DBVERSION)
echo "Current DB version: $DB_ver"
echo "Checking for DB upgrades..."

if [[ $DB_ver -eq "33" ]]; then
    echo "Applying DB_PATCH_42 - Hive body stack order and sensor placement"
    sqlite3 $DestDB < $DBPatches/DB_PATCH_42
    let DB_ver="34"
fi
if [[ $DB_ver -eq "34" ]]; then
    echo "Applying DB_PATCH_43 - WX Underground API key"
    sqlite3 $DestDB < $DBPatches/DB_PATCH_43
    let DB_ver="35"
fi

echo $DB_ver > /home/HiveControl/data/DBVERSION
echo "DB version now: $DB_ver"
echo "============================================="

# ─── Copy Files ─────────────────────────────────────────────
echo "Upgrading WWW pages..."
cp -Rp $WWWTempRepo/pages/* $DestWWWRepo/pages/
cp -Rp $WWWTempRepo/admin/* $DestWWWRepo/admin/
cp -Rp $WWWTempRepo/include/* $DestWWWRepo/include/
cp -Rp $WWWTempRepo/errors/* $DestWWWRepo/errors/
cp -Rp $WWWTempRepo/js/* $DestWWWRepo/js/
mkdir -p $DestWWWRepo/css
cp -Rp $WWWTempRepo/css/* $DestWWWRepo/css/
mkdir -p $DestWWWRepo/bower_components/jquery-ui/dist
cp -Rp $WWWTempRepo/bower_components/jquery-ui/dist/* $DestWWWRepo/bower_components/jquery-ui/dist/
echo "============================================="

echo "Upgrading shell scripts..."
rm -rf $scriptsource/hiveconfig.inc
cp -Rp $scriptsource/* $scriptDest/
cd $scriptDest
find . -name '*.sh' -exec chmod u+x {} +
echo "============================================="

echo "Upgrading binaries..."
cp -Rp $softwareSource/* $softwareDest/
echo "============================================="

# Update install files
cp -Rp /home/HiveControl/upgrade/HiveControl/install/* /home/HiveControl/install/

# Update sudoers (for restart/shutdown web UI support)
echo "Updating sudoers..."
sudo cp /home/HiveControl/upgrade/HiveControl/install/sudoers.d/hivecontrol.sudoers /etc/sudoers.d/hivecontrol
sudo chown root:root /etc/sudoers.d/hivecontrol
sudo chmod 440 /etc/sudoers.d/hivecontrol
CHECKSUDO=$(sudo visudo -c 2>&1 | grep -c "parsed OK")
if [[ $CHECKSUDO -gt 0 ]]; then
    echo "Sudoers update SUCCESS"
else
    echo "Something went wrong with sudoers, removing it"
    sudo rm -f /etc/sudoers.d/hivecontrol
fi

# ─── Finalize ───────────────────────────────────────────────
loglocal "$DATE" UPGRADE SUCCESS "Dev upgrade to $Latest_Ver from branch $BRANCH"
sqlite3 $DestDB "UPDATE hiveconfig SET upgrade_available=\"no\" WHERE id=1"

cp /home/HiveControl/upgrade/HiveControl/VERSION /home/HiveControl/

source /home/HiveControl/scripts/hiveconfig.inc
((VERSION++))
sqlite3 $DestDB "UPDATE hiveconfig SET HCVersion=$Latest_Ver, VERSION=$VERSION WHERE id=1"

# Regenerate hiveconfig.inc so new columns are available to scripts
/home/HiveControl/scripts/data/hiveconfig.sh

echo "============================================="
echo "  DEV UPGRADE COMPLETE"
echo "  Branch:  $BRANCH"
echo "  Version: $Latest_Ver"
echo "  DB:      $DB_ver"
echo "============================================="
echo ""
echo "To roll back: cp /home/HiveControl/data/hive-data.bckup /home/HiveControl/data/hive-data.db"
