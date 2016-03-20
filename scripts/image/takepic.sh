#!/bin/bash
# Take a picture every 5 minutes and upload the series once an hour for analysis to the yardcontroller

source /home/HiveControl/scripts/hiveconfig.inc

#Take a full picture
raspistill -drc high -ex auto -awb auto -o $PUBLIC_HTML_DIR/images/current.jpg

#Take a preview, might not be good to wait a milisecond
raspistill -w 275 -h 100 -drc high -ex auto -awb auto -o $PUBLIC_HTML_DIR/images/currentsm.jpg



