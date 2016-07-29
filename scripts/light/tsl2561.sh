#!/bin/bash
# read the TSL2561 
# Version 1
# For Hivetool command set

# We don't actually have this version, but this is a placeholder if you do.
# Let us know, and we'll add support. 

# Source variables
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

DATE2=$(TZ=":$TIMEZONE" date '+%F %T')

loglocal "$DATE2" LIGHT ERROR "TSL2561 Not Supported"

#echo "Using an TS2561 - Not supported" >> $LOG
echo "null"



