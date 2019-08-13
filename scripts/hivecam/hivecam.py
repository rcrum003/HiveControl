#!/usr/bin/env python
#
#  hivecam.py
#
#
# sudo python /your/file/location/raspiLapseCam.py (add &) if you wish to run as a
# background task. A process ID will be shown which can be ended with

# sudo kill XXXX (XXXX = process number)

# Based on your settings the application will not begin capturing images
# saving them to your chosen file location (same as current location of this file as default.

# Import some frameworks
import os
import time
import RPi.GPIO as GPIO
from datetime import datetime

# Define the location where you wish to save files. Set to HOME as default. 
# If you run a local web server on Apache you could set this to /var/www/ to make them 
# accessible via web browser.
folderToSave = "/home/hivecam/timelapse"
#os.mkdir(folderToSave)

#os.makedirs(folderToSave,exist_ok=True)

try: 
    os.makedirs(folderToSave)
except OSError:
    if not os.path.isdir(folderToSave):
        raise


# Set the initial serial for saved images to 1
fileSerial = 1
#fileSerial = "str(initYear) + str(initMonth) + str(initDate) + str(initHour) + str(initMins)"

# Run a WHILE Loop of infinitely
while True:
        
      d = datetime.now()

      # Set FileSerialNumber to 000X using four digits
      fileSerialNumber = "%04d" % (fileSerial)
        
      # Capture the CURRENT time (not start time as set above) to insert into each capture image filename
      year = "%04d" % (d.year)
      month = "%02d" % (d.month)
      day = "%02d" % (d.day)
      hour = "%02d" % (d.hour)
      mins = "%02d" % (d.minute)
        
      # Define the size of the image you wish to capture. 
      imgWidth = 800 # Max = 2592 
      imgHeight = 600 # Max = 1944
      #print " ====================================== Saving file at " + hour + ":" + mins
        
      # Turn on light before we capture image
      os.system("sudo gpio mode 7 up")

      #-n -ex night -awb -mm average -drc medium -q 50
      # Capture the image using raspistill. Set to capture with added sharpening, auto white balance and average metering mode
      # Change these settings where you see fit and to suit the conditions you are using the camera in
      os.system("raspistill -w " + str(imgWidth) + " -h " + str(imgHeight) + " -o " + str(folderToSave) + "/" + str(fileSerialNumber) + "_" + str(year) + str(month) + str(day) + str(hour) + str(mins) +  ".jpg  -n -ex night -awb auto -mm average -drc high -q 50 -l /var/www/html/hivecam/current.jpg")

      # Turn off light
      os.system("sudo gpio mode 7 down")
        
      # Increment the fileSerial
      fileSerial += 1
        
      # Wait 600 seconds (10 minutes) before next capture
      time.sleep(600)
      
