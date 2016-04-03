

# HiveControl
This is a supporting application to the Hivetool.org software.

[![Gitter](https://badges.gitter.im/rcrum003/HiveControl.svg)](https://gitter.im/rcrum003/HiveControl?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

## Current version supports:
Weight, Temperature, Webcam, Weather, GDD, LUX

[Supported Sensors](https://github.com/rcrum003/HiveControl/wiki/Sensor-Support)

## Installation Instructions
To install, download and run [install.sh](https://raw.githubusercontent.com/rcrum003/HiveControl/master/install.sh) as root from your Raspberry PI.
Easiest way is to run this command from your command line:
curl https://raw.githubusercontent.com/rcrum003/HiveControl/master/install.sh -o install.sh
sudo ./install.sh

-----------------------
Weight Logging
-----------------------
Weight logging modules currently supports HX711 (included with Hivetool Developer Kit) or the Phidget 1046 module.

Installing:
1. You will need to calibrate your scale (ie, zero it out)
2. See the /scripts/weight/ directory for how we do this.


------------------------
Temperature
------------------------
Temperature logging currently supports the TemperHUM sensors.


-------------------------
Weather
-------------------------
