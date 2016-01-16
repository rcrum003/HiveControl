#phidget.py
#! /usr/bin/python

"""Copyright 2011 Phidgets Inc.
This work is licensed under the Creative Commons Attribution 2.5 Canada License.
To view a copy of this license, visit http://creativecommons.org/licenses/by/2.5/ca/
"""

__author__="Adam Stelmack"
__version__="2.1.8"
__date__ ="14-Jan-2011 2:29:14 PM"

#Basic imports
import sys
from time import sleep

#Phidget specific imports
from Phidgets.Devices.Bridge import Bridge, BridgeGain
from Phidgets.PhidgetException import PhidgetException
from Phidgets.Phidget import PhidgetLogLevel

# declare and initialize some variables
# debug   0=suppress output messages   1=display messages
# loop    0=run once                   1=continuously loop
global debug, loop
debug=0
loop=0

#Create a bridge object
try:
    bridge = Bridge()
except RuntimeError as e:
    print("Runtime Exception: %s" % e.details)
    print("Exiting....")
    exit(1)

#Information Display Function
def displayDeviceInfo():
    global debug
    if debug:
       print("|------------|----------------------------------|--------------|------------|")
       print("|- Attached -|-              Type              -|- Serial No. -|-  Version -|")
       print("|------------|----------------------------------|--------------|------------|")
       print("|- %8s -|- %30s -|- %10d -|- %8d -|" % (bridge.isAttached(), bridge.getDeviceName(), bridge.getSerialNum(), bridge.getDeviceVersion()))
       print("|------------|----------------------------------|--------------|------------|")
       print("Number of bridge inputs: %i" % (bridge.getInputCount()))
       print("Data Rate Max: %d" % (bridge.getDataRateMax()))
       print("Data Rate Min: %d" % (bridge.getDataRateMin()))
       print("Input Value Max: %d" % (bridge.getBridgeMax(0)))
       print("Input Value Min: %d" % (bridge.getBridgeMin(0)))

#Event Handler Callback Functions
def BridgeAttached(e):
    global debug
    global count
    global channel
    count = 0
    channel=[0,0,0,0]

    attached = e.device
    if debug:
       print("Bridge %i Attached!" % (attached.getSerialNum()))

def BridgeDetached(e):
    global debug
    detached = e.device
    if debug:
      print("Bridge %i Detached!" % (detached.getSerialNum()))

def BridgeError(e):
    global debug
    try:
        source = e.device
        if debug:
           print("Bridge %i: Phidget Error %i: %s" % (source.getSerialNum(), e.eCode, e.description))
    except PhidgetException as e:
        if debug:
           print("Phidget Exception %i: %s" % (e.code, e.details))

def BridgeData(e):
    global count
    global channel
    offset0 = 0
    offset1 = 0
    offset2 = 0
    offset3 = 0
    factor = 1
    total_weight = 0
    global loop
    source = e.device
    e.value = round(e.value, 3)
    # fix offsets
    print("=====================")
    print("Starting Loop %i" % (e.index))
    if e.index == 0:
	print("Starting Value= %f" % (e.value))
	print("Minus an offset of %f" % (offset0))
     	e.value = e.value - offset0
	print("Input - offset =%f " % (e.value))
        weight0 = e.value * factor
        print("Weight0 = %f" % (weight0))
    if e.index == 1:
        print("Starting Value= %f" % (e.value))
        print("Minus an offset of %f" % (offset1))
        e.value = e.value - offset1
        print("Input - offset =%f " % (e.value))
        weight1 = e.value * factor
        print("Weight1 = %f" % (weight1))
    if e.index == 2:
        print("Starting Value= %f" % (e.value))
        print("Minus an offset of %f" % (offset2))
        e.value = e.value - offset2
        print("Input - offset =%f " % (e.value))
        weight2 = e.value * factor
        print("Weight2 = %f" % (weight2))
    if e.index == 3:
        print("Starting Value= %f" % (e.value))
        print("Minus an offset of %f" % (offset3))
        e.value = e.value - offset3
        print("Input - offset =%f " % (e.value))
	weight3 = e.value * factor
        print("Weight3 = %f" % (weight3))
    # try to sum the channels
    channel[e.index] = round(e.value, 3)
    count=count+1
    if count > 3:
       count = 0
       channel_total = channel[0]+channel[1]+channel[2]+channel[3]
       print("Bridge Total: %f" % (channel_total))
       # solve for knownweight = K x Bridge Total (since we include offset above)       
       if loop<1:
#          bridge.setEnabled(0, False)
#          bridge.setEnabled(1, False)
#          bridge.setEnabled(2, False)
#          bridge.setEnabled(3, False)
          bridge.closePhidget()
          exit(1)

#Main Program Code
try:
    bridge.setOnAttachHandler(BridgeAttached)
    bridge.setOnDetachHandler(BridgeDetached)
    bridge.setOnErrorhandler(BridgeError)
    bridge.setOnBridgeDataHandler(BridgeData)
except PhidgetException as e:
    if debug:
       print("Phidget Exception %i: %s" % (e.code, e.details))
       print("Exiting....")
    exit(1)

if debug:
   print("Opening phidget object....")
count = 0
channel=[0,0,0,0]

try:
    bridge.openPhidget()
except PhidgetException as e:
    if debug:
       print("Phidget Exception %i: %s" % (e.code, e.details))
       print("Exiting....")
    exit(1)

if debug:
   print("Waiting for attach....")

try:
    bridge.waitForAttach(10000)
except PhidgetException as e:
    if debug:
       print("Phidget Exception %i: %s" % (e.code, e.details))
    try:
        bridge.closePhidget()
    except PhidgetException as e:
        if debug:
           print("Phidget Exception %i: %s" % (e.code, e.details))
           print("Exiting....")
        exit(1)
    if debug:
       print("Exiting....")
    exit(1)
else:
    displayDeviceInfo()

try:
    if debug:
       print("Set data rate to 1000ms ...")
    bridge.setDataRate(500)
    sleep(2)

    if debug:
       print("Set Gain to 128...")
    bridge.setGain(0, BridgeGain.PHIDGET_BRIDGE_GAIN_128)
    bridge.setGain(1, BridgeGain.PHIDGET_BRIDGE_GAIN_128)
    bridge.setGain(2, BridgeGain.PHIDGET_BRIDGE_GAIN_128)
    bridge.setGain(3, BridgeGain.PHIDGET_BRIDGE_GAIN_128)
    sleep(2)

    if debug:
       print("Enable the Bridge input for reading data...")
    bridge.setEnabled(0, True)
    bridge.setEnabled(1, True)
    bridge.setEnabled(2, True)
    bridge.setEnabled(3, True)
    sleep(2)

except PhidgetException as e:
    if debug:
       print("Phidget Exception %i: %s" % (e.code, e.details))
    try:
        bridge.closePhidget()
    except PhidgetException as e:
        if debug:
           print("Phidget Exception %i: %s" % (e.code, e.details))
           print("Exiting....")
        exit(1)
    if debug:
       print("Exiting....")
    exit(1)

if debug:
   print("Press Enter to quit....")

chr = sys.stdin.read(1)


if debug:
   print("Closing...")

try:
    print("Disable the Bridge input for reading data...")
    bridge.setEnabled(0, False)
    bridge.setEnabled(1, False)
    bridge.setEnabled(2, False)
    bridge.setEnabled(3, False)
    sleep(2)
except PhidgetException as e:
    print("Phidget Exception %i: %s" % (e.code, e.details))
    try:
        bridge.closePhidget()
    except PhidgetException as e:
        print("Phidget Exception %i: %s" % (e.code, e.details))
        print("Exiting....")
        exit(1)
    print("Exiting....")
    exit(1)

try:
    bridge.closePhidget()
except PhidgetException as e:
    print("Phidget Exception %i: %s" % (e.code, e.details))
    print("Exiting....")
    exit(1)

if debug:
   print("Done.")
exit(0)

