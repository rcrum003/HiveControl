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
if [[ $DB_ver -eq "35" ]]; then
    echo "Applying DB_PATCH_44 - Pollen enable/disable toggle"
    sqlite3 $DestDB < $DBPatches/DB_PATCH_44
    let DB_ver="36"
fi

echo $DB_ver > /home/HiveControl/data/DBVERSION
echo "DB version now: $DB_ver"

# Validate schema — catches any columns missed by the patch chain
if [ -f "/home/HiveControl/upgrade/HiveControl/scripts/data/schema_validate.sh" ]; then
    bash /home/HiveControl/upgrade/HiveControl/scripts/data/schema_validate.sh "$DestDB"
fi
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

# Recompile TSL2561 light sensor binary for current architecture
OS_ARCH=$(uname -m)
case "$OS_ARCH" in
    aarch64|arm64|x86_64|amd64) IS_64BIT=true ;;
    *) IS_64BIT=false ;;
esac

echo "Compiling TSL2561 light sensor binary for $OS_ARCH"
cd /home/HiveControl/software/tsl2561
if [ -f Makefile ]; then
    make clean
    if make; then
        sudo make install
        echo "TSL2561 compiled and installed to /usr/local/bin/2561"
    else
        echo "Warning: TSL2561 compilation failed"
        if [ "$IS_64BIT" = false ]; then
            echo "Falling back to pre-compiled 32-bit binary"
            sudo cp /home/HiveControl/software/tsl2561/2561 /usr/local/bin/2561
        else
            echo "Error: No pre-compiled 64-bit binary available — TSL2561 sensor will not work"
        fi
    fi
else
    echo "Warning: TSL2561 Makefile not found, compiling manually"
    gcc -Wall -O2 -o TSL2561.o -c TSL2561.c && \
    gcc -Wall -O2 -o 2561.o -c 2561.c && \
    gcc -Wall -O2 -o 2561 TSL2561.o 2561.o && \
    sudo install -m 755 2561 /usr/local/bin/2561 && \
    echo "TSL2561 compiled and installed" || \
    echo "Warning: TSL2561 compilation failed"
    rm -f *.o
fi
echo "Compiling TSL2591 light sensor binary for $OS_ARCH"
cd /home/HiveControl/software/tsl2591
if [ -f Makefile ]; then
    make clean
    if make; then
        sudo make install
        echo "TSL2591 compiled and installed to /usr/local/bin/2591"
    else
        echo "Warning: TSL2591 compilation failed"
        if [ "$IS_64BIT" = false ]; then
            echo "Falling back to pre-compiled 32-bit binary"
            sudo cp /home/HiveControl/software/binaries/2591 /usr/local/bin/2591
        else
            echo "Error: No pre-compiled 64-bit binary available — TSL2591 sensor will not work"
        fi
    fi
else
    echo "Warning: TSL2591 Makefile not found, compiling manually"
    gcc -Wall -O2 -o 2591 2591.c && \
    sudo install -m 755 2591 /usr/local/bin/2591 && \
    echo "TSL2591 compiled and installed" || \
    echo "Warning: TSL2591 compilation failed"
fi
echo "============================================="

# ─── USB Camera Livestream ──────────────────────────────────
echo "Installing USB camera livestream support"

sudo apt-get install -y libjpeg62-turbo-dev 2>/dev/null || sudo apt-get install -y libjpeg-dev
sudo apt-get install -y v4l-utils

if [ ! -f /usr/local/bin/mjpg_streamer ]; then
    echo "Building mjpg-streamer from source..."
    cd /home/HiveControl/software
    sudo git clone https://github.com/jacksonliam/mjpg-streamer.git mjpg-streamer-build
    cd mjpg-streamer-build/mjpg-streamer-experimental
    mkdir -p _build && cd _build
    cmake .. && make && sudo make install && {
        echo "mjpg-streamer installed successfully"
    } || {
        echo "Warning: mjpg-streamer build failed — USB camera livestream will not work"
    }
    cd /home/HiveControl
    rm -rf /home/HiveControl/software/mjpg-streamer-build
else
    echo "mjpg-streamer already installed, skipping build"
fi

sudo cp /home/HiveControl/upgrade/HiveControl/install/init.d/livestream /etc/init.d/livestream
sudo chmod +x /etc/init.d/livestream
sudo update-rc.d livestream defaults 2>/dev/null || true

if [ -e /dev/video0 ]; then
    echo "USB camera detected — restarting livestream service"
    sudo /etc/init.d/livestream restart
else
    echo "No USB camera detected — livestream service updated but not started"
fi
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

# ─── Self-update ────────────────────────────────────────────
if [ -f "/home/HiveControl/upgrade/HiveControl/upgrade-dev.sh" ]; then
    cp /home/HiveControl/upgrade/HiveControl/upgrade-dev.sh /home/HiveControl/upgrade-dev.sh
    chmod u+x /home/HiveControl/upgrade-dev.sh
    echo "upgrade-dev.sh self-updated"
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
