

# HiveControl
This is a supporting application to the Hivetool.org software.


## Release Notes
Latest Release = 2.00
[Release Notes](https://raw.githubusercontent.com/rcrum003/HiveControl/master/RELEASE)

## Update
We currently support all Raspberry PI hardward/software platforms, INCLUDING the new PI4, Raspbian Buster
Verified install.sh works as expected on Raspbian Buster as of 8/13/2019.

## Now includes beecounter code
Only use these instructions if you have version 1.61 and below.
Follow the install instructions if you are setting up from scratch.
[Beecounter Setup](https://github.com/rcrum003/HiveControl/wiki/BeeCounter-Setup)

## Current version supports:
Weight, Temperature, Webcam, Weather, GDD, LUX, Purple Air Pollution

[Supported Sensors](https://github.com/rcrum003/HiveControl/wiki/Sensor-Support)

## Basic Installation
To install, download and run [install.sh](https://raw.githubusercontent.com/rcrum003/HiveControl/master/install.sh) as root from your Raspberry PI.
Easiest way is to run this command from your command line:

      curl https://raw.githubusercontent.com/rcrum003/HiveControl/master/install.sh -o install.sh
      chmod u+x install.sh
      sudo ./install.sh

## Custom Installation
For those that have requested specific things to be included that I didn't feel was best for everyone (or needed), I have included a method to be able to install from this script.
	
	Run the following ./install -option

	 -b  Beecounter Webcam Setup (Warning: Can take up to 8 hrs to run)
	 -x  Install XRDP for you Windows Users
	 -k  Touch Screen Keyboard Install
	 -h  or -?  This help message
	 -d  Turn on Debugging

To install all the options at once:
	
	./install.s -bxk


## Upgrading/Updating
To upgrade or update to the latest version, after you have installed, run:

      sudo /home/HiveControl/upgrade.sh

Note: this will overwrite any code changes you may have made, including your scripts directory.
It will not overwrite your data or configurations made thorough the Settings Page.
