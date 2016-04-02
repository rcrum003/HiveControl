#!/bin/bash
# read the TSL2561 
# Version 1
# For Hivetool command set

# We don't actually have this version, but this is a placeholder if you do.
# Let us know, and we'll add support. 

# Source variables
source /home/HiveControl/scripts/hiveconfig.inc
source /home/HiveControl/scripts/data/logger.inc

loglocal "$DATE" LIGHT ERROR "TSL2561 Not Supported"

#echo "Using an TS2561 - Not supported" >> $LOG
echo 0



