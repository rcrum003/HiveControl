/*
 * This file is part of libphidget21
 *
 * Copyright 2006-2015 Phidgets Inc <patrick@phidgets.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see 
 * <http://www.gnu.org/licenses/>
 */

#ifndef __CPHIDGETINTERFACEKIT
#define __CPHIDGETINTERFACEKIT
#include "cphidget.h"

/** \defgroup phidifkit Phidget InterfaceKit 
 * \ingroup phidgets
 * These calls are specific to the Phidget InterfaceKit object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(InterfaceKit)
CHDRSTANDARD(InterfaceKit)

/**
 * Gets the number of digital inputs supported by this board.
 * @param phid An attached phidget interface kit handle.
 * @param count The ditial input count.
 */
CHDRGET(InterfaceKit, InputCount, int *count)
/**
 * Gets the state of a digital input.
 * @param phid An attached phidget interface kit handle.
 * @param index The input index.
 * @param inputState The input state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(InterfaceKit, InputState, int *inputState)
/**
 * Set a digital input change handler. This is called when a digital input changes.
 * @param phid An attached phidget interface kit handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(InterfaceKit, InputChange, int inputState)

/**
 * Gets the number of digital outputs supported by this board.
 * @param phid An attached phidget interface kit handle.
 * @param count The output count.
 */
CHDRGET(InterfaceKit, OutputCount, int *count)
/**
 * Gets the state of a digital output.
 * @param phid An attached phidget interface kit handle.
 * @param index The output index.
 * @param outputState The output state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(InterfaceKit, OutputState, int *outputState)
/**
 * Sets the state of a digital output.
 * @param phid An attached phidget interface kit handle.
 * @param index The otuput index.
 * @param outputState The output state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSETINDEX(InterfaceKit, OutputState, int outputState)
/**
 * Set a digital output change handler. This is called when a digital output changes.
 * @param phid An attached phidget interface kit handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(InterfaceKit, OutputChange, int outputState)

/**
 * Gets the number of sensor (analog) inputs supported by this board.
 * @param phid An attached phidget interface kit handle.
 * @param count The sensor input count.
 */
CHDRGET(InterfaceKit, SensorCount, int *count)
/**
 * Gets a sensor value (0-1000).
 * @param phid An attached phidget interface kit handle.
 * @param index The sensor index.
 * @param sensorValue The sensor value.
 */
CHDRGETINDEX(InterfaceKit, SensorValue, int *sensorValue)
/**
 * Gets a sensor raw value (12-bit).
 * @param phid An attached phidget interface kit handle.
 * @param index The sensor index.
 * @param sensorRawValue The sensor value.
 */
CHDRGETINDEX(InterfaceKit, SensorRawValue, int *sensorRawValue)
/**
 * Set a sensor change handler. This is called when a sensor value changes by more then the change trigger.
 * @param phid An attached phidget interface kit handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(InterfaceKit, SensorChange, int sensorValue)
/**
 * Gets a sensor change trigger.
 * @param phid An attached phidget interface kit handle.
 * @param index The sensor index.
 * @param trigger The change trigger.
 */
CHDRGETINDEX(InterfaceKit, SensorChangeTrigger, int *trigger)
/**
 * Sets a sensor change trigger.
 * @param phid An attached phidget interface kit handle.
 * @param index The sensor index.
 * @param trigger The change trigger.
 */
CHDRSETINDEX(InterfaceKit, SensorChangeTrigger, int trigger)
/**
 * Gets the ratiometric state for this board.
 * @param phid An attached phidget interface kit handle.
 * @param ratiometric The ratiometric state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGET(InterfaceKit, Ratiometric, int *ratiometric)
/**
 * Sets the ratiometric state for this board.
 * @param phid An attached phidget interface kit handle.
 * @param ratiometric The ratiometric state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSET(InterfaceKit, Ratiometric, int ratiometric)

//This is the event rate
//since we're not going to run an extra thread, the accuracy of the data rate is limited by the interrupt endpoint data rate (>=8ms)
/**
 * Gets the Data Rate for an analog input.
 * @param phid An attached phidget interface kit handle.
 * @param index The sensor index.
 * @param milliseconds Data rate in ms.
 */
CHDRGETINDEX(InterfaceKit, DataRate, int *milliseconds)
/**
 * Sets the Data Rate for an analog input.
 * @param phid An attached phidget interface kit handle.
 * @param index The sensor index.
 * @param milliseconds Data rate in ms.
 */
CHDRSETINDEX(InterfaceKit, DataRate, int milliseconds)
/**
 * Gets the maximum supported data rate for an analog input
 * @param phid An attached phidget interface kit handle.
 * @param index The sensor index.
 * @param max Data rate in ms.
 */
CHDRGETINDEX(InterfaceKit, DataRateMax, int *max)
/**
 * Gets the minimum supported data rate for an analog input
 * @param phid An attached phidget interface kit handle.
 * @param index The sensor index.
 * @param min Data rate in ms.
 */
CHDRGETINDEX(InterfaceKit, DataRateMin, int *min)

#ifndef REMOVE_DEPRECATED
DEP_CHDRGET("Deprecated - use CPhidgetInterfaceKit_getInputCount",InterfaceKit, NumInputs, int *)
DEP_CHDRGET("Deprecated - use CPhidgetInterfaceKit_getOutputCount",InterfaceKit, NumOutputs, int *)
DEP_CHDRGET("Deprecated - use CPhidgetInterfaceKit_getSensorCount",InterfaceKit, NumSensors, int *)
#endif

#ifndef EXTERNALPROTO
#define IFKIT_MAXINPUTS 32
#define IFKIT_MAXOUTPUTS 32
#define IFKIT_MAXSENSORS 8

#define IFKIT_MAXSENSORCHANGE 1000 //BL: Had to check for this, might as well use a define

//usually it is <=8, but could be bigger if a packet gets missed.
#define IFKIT_MAX_DATA_PER_PACKET	16
//in milliseconds - this is the fastest hardware rate of any device
#define IFKIT_MAX_DATA_RATE 1
//1 second is the longest between events that we support
#define IFKIT_MIN_DATA_RATE 1000
//add 200ms for timing differences (late events, etc) - should be plenty
#define IFKIT_DATA_BUFFER_SIZE ((IFKIT_MIN_DATA_RATE + 200)/IFKIT_MAX_DATA_RATE)

struct _CPhidgetInterfaceKit {
	CPhidget phid;

	int (CCONV *fptrOutputChange)(CPhidgetInterfaceKitHandle, void *, int, int);
	int (CCONV *fptrInputChange)(CPhidgetInterfaceKitHandle, void *, int, int);
	int (CCONV *fptrSensorChange)(CPhidgetInterfaceKitHandle, void *, int, int);

	void *fptrOutputChangeptr;
	void *fptrInputChangeptr;
	void *fptrSensorChangeptr;

	unsigned char ratiometric;
	unsigned char ratiometricEcho;
	unsigned char ratiometricSwitching;

	// output fields all protected by outputLock
	unsigned char outputStates[IFKIT_MAXOUTPUTS];
	unsigned char nextOutputStates[IFKIT_MAXOUTPUTS];
	unsigned char changedOutputs[IFKIT_MAXOUTPUTS];
	int lastChangedOutput;

	unsigned char outputEchoStates[IFKIT_MAXOUTPUTS];		// Values returned from the device
	unsigned char physicalState[IFKIT_MAXINPUTS];

	int sensorChangeTrigger[IFKIT_MAXSENSORS];
	int sensorRawValue[IFKIT_MAXSENSORS];
	int sensorValue[IFKIT_MAXSENSORS];
	int sensorLastValue[IFKIT_MAXSENSORS];
	
	int sensorRawValueAccumulator[IFKIT_MAXSENSORS];
	int sensorRawValueAccumulatorCount[IFKIT_MAXSENSORS];
	
	int dataRate[IFKIT_MAXSENSORS];
	int interruptRate;
	int dataRateMax, dataRateMin;

	int lastPacketCount;
	int maxDataPerPacket;

	int dataSinceAttach;

	CPhidget_EventMode eventMode[IFKIT_MAXSENSORS];

	unsigned char fullStateEcho;
} typedef CPhidgetInterfaceKitInfo;
#endif

/** @} */

#endif
