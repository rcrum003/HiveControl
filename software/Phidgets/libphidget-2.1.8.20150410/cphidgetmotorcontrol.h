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

#ifndef __CPHIDGETMOTORCONTROL
#define __CPHIDGETMOTORCONTROL
#include "cphidget.h"

/** \defgroup phidmotorcontrol Phidget Motor Control 
 * \ingroup phidgets
 * These calls are specific to the Phidget Motor Control object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(MotorControl)
CHDRSTANDARD(MotorControl)

/**
 * Gets the number of motors supported by this controller.
 * @param phid An attached phidget motor control handle.
 * @param count The motor count.
 */
CHDRGET(MotorControl,MotorCount,int *count)

/**
 * Gets the current velocity of a motor.
 * @param phid An attached phidget motor control handle.
 * @param index The motor index.
 * @param velocity The current velocity.
 */
CHDRGETINDEX(MotorControl,Velocity,double *velocity)
/**
 * Sets the velocity of a motor.
 * @param phid An attached phidget motor control handle.
 * @param index The motor index.
 * @param velocity The velocity.
 */
CHDRSETINDEX(MotorControl,Velocity,double velocity)
/**
 * Sets a velocity change event handler. This is called when the velocity changes.
 * @param phid An attached phidget motor control handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(MotorControl,VelocityChange,double velocity)

/**
 * Gets the last set acceleration of a motor.
 * @param phid An attached phidget motor control handle.
 * @param index The motor index.
 * @param acceleration The acceleration.
 */
CHDRGETINDEX(MotorControl,Acceleration,double *acceleration)
/**
 * Sets the last set acceleration of a motor.
 * @param phid An attached phidget motor control handle.
 * @param index The motor index.
 * @param acceleration The acceleration.
 */
CHDRSETINDEX(MotorControl,Acceleration,double acceleration)
/**
 * Gets the maximum acceleration supported by a motor
 * @param phid An attached phidget motor control handle
 * @param index The motor index.
 * @param max The maximum acceleration.
 */
CHDRGETINDEX(MotorControl,AccelerationMax,double *max)
/**
 * Gets the minimum acceleration supported by a motor.
 * @param phid An attached phidget motor control handle
 * @param index The motor index.
 * @param min The minimum acceleration
 */
CHDRGETINDEX(MotorControl,AccelerationMin,double *min)


/**
 * Gets the current current draw for a motor.
 * @param phid An attached phidget motor control handle
 * @param index The motor index.
 * @param current The current.
 */
CHDRGETINDEX(MotorControl,Current,double *current)
/**
 * Sets a current change event handler. This is called when the current draw changes.
 * @param phid An attached phidget motor control handle
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(MotorControl,CurrentChange,double current)

/**
 * Gets the number of digital inputs supported by this board.
 * @param phid An attached phidget motor control handle.
 * @param count The ditial input count.
 */
CHDRGET(MotorControl,InputCount,int *count)
/**
 * Gets the state of a digital input.
 * @param phid An attached phidget motor control handle.
 * @param index The input index.
 * @param inputState The input state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(MotorControl,InputState,int *inputState)
/**
 * Set a digital input change handler. This is called when a digital input changes.
 * @param phid An attached phidget motor control handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(MotorControl,InputChange,int inputState)

/**
 * Gets the number of encoder inputs supported by this board.
 * @param phid An attached phidget motor control handle.
 * @param count The encoder input count.
 */
CHDRGET(MotorControl,EncoderCount,int *count)
/**
 * Gets the position of an encoder. This position starts at 0 every time the phidget is opened.
 * @param phid An attached phidget motor control handle.
 * @param index The encoder index.
 * @param position The encoder position.
 */
CHDRGETINDEX(MotorControl,EncoderPosition,int *position)
/**
 * Sets the encoder position. This can be used to set the position to a known value, and should only be called when the encoder is not moving.
 * @param phid An attached phidget motor control handle.
 * @param index The encoder index.
 * @param position The encoder position.
 */
CHDRSETINDEX(MotorControl,EncoderPosition,int position)
/**
 * Set an encoder position change handler. This is called when the encoder position changes.
 * @param phid An attached phidget motor control handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(MotorControl,EncoderPositionChange,int time,int positionChange)
/**
 * Set an encoder position update handler. This is called at a constant rate; every 8ms, whether the encoder position has changed or not.
 * @param phid An attached phidget motor control handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(MotorControl,EncoderPositionUpdate,int positionChange)

/**
 * Gets the Back EMF sensing state for a motor.
 * @param phid An attached phidget motor control handle.
 * @param index The motor index.
 * @param bEMFState The back EMF sensing state.
 */
CHDRGETINDEX(MotorControl,BackEMFSensingState,int *bEMFState)
/**
 * Sets the Back EMF sensing state for a motor.
 * @param phid An attached phidget motor control handle.
 * @param index The motor index.
 * @param bEMFState The back EMF sensing state.
 */
CHDRSETINDEX(MotorControl,BackEMFSensingState,int bEMFState)
/**
 * Gets the Back EMF voltage for a motor.
 * @param phid An attached phidget motor control handle.
 * @param index The motor index.
 * @param voltage The back EMF voltage, in volts.
 */
CHDRGETINDEX(MotorControl,BackEMF,double *voltage)
/**
 * Set a back EMF update handler. This is called at a constant rate; every 16ms, when back EMF sensing is enabled for that motor.
 * @param phid An attached phidget motor control handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(MotorControl,BackEMFUpdate,double voltage)

/**
 * Gets the Supply voltage for the motors. This could be higher then the actual supply voltage.
 * @param phid An attached phidget motor control handle.
 * @param supplyVoltage The supply voltage, in volts.
 */
CHDRGET(MotorControl,SupplyVoltage,double *supplyVoltage)

/**
 * Gets the Braking value for a motor.
 * @param phid An attached phidget motor control handle.
 * @param index The motor index.
 * @param braking The braking value, in percent.
 */
CHDRGETINDEX(MotorControl,Braking,double *braking)
/**
 * Sets the Braking value for a motor. This is applied when velocity is 0. Default is 0%.
 * @param phid An attached phidget motor control handle.
 * @param index The motor index.
 * @param braking The braking value, in percent.
 */
CHDRSETINDEX(MotorControl,Braking,double braking)

/**
 * Gets the number of sensor inputs supported by this board.
 * @param phid An attached phidget motor control handle.
 * @param count The sensor input count.
 */
CHDRGET(MotorControl, SensorCount, int *count)
/**
 * Gets the value of a sensor.
 * @param phid An attached phidget motor control handle.
 * @param index The sensor index.
 * @param sensorValue The sensor value, range: 0-1000.
 */
CHDRGETINDEX(MotorControl, SensorValue, int *sensorValue)
/**
 * Gets the raw value of a sensor (12-bit).
 * @param phid An attached phidget motor control handle.
 * @param index The sensor index.
 * @param sensorRawValue The sensor value, range: 0-4096.
 */
CHDRGETINDEX(MotorControl, SensorRawValue, int *sensorRawValue)
/**
 * Set a sensor update handler. This is called at a constant rate; every 8ms.
 * @param phid An attached phidget motor control handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(MotorControl, SensorUpdate, int sensorValue)
/**
 * Gets the ratiometric state.
 * @param phid An attached phidget motor control handle.
 * @param ratiometric The ratiometric state.
 */
CHDRGET(MotorControl, Ratiometric, int *ratiometric)
/**
 * Sets the ratiometric state. This control the voltage reference used for sampling the analog sensors.
 * @param phid An attached phidget motor control handle.
 * @param ratiometric The ratiometric state.
 */
CHDRSET(MotorControl, Ratiometric, int ratiometric)

/**
 * Set a current update handler. This is called at a constant rate; every 8ms.
 * @param phid An attached phidget motor control handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(MotorControl,CurrentUpdate,double current)

#ifndef REMOVE_DEPRECATED
DEP_CHDRGET("Deprecated - use CPhidgetMotorControl_getMotorCount",MotorControl,NumMotors,int *)
DEP_CHDRGET("Deprecated - use CPhidgetMotorControl_getInputCount",MotorControl,NumInputs,int *)
DEP_CHDRGETINDEX("Deprecated - use CPhidgetMotorControl_getVelocity",MotorControl,MotorSpeed,double *)
DEP_CHDRSETINDEX("Deprecated - use CPhidgetMotorControl_setVelocity",MotorControl,MotorSpeed,double)
DEP_CHDREVENTINDEX("Deprecated - use CPhidgetMotorControl_set_OnVelocityChange_Handler",MotorControl,MotorChange,double motorSpeed)
#endif

#ifndef EXTERNALPROTO
#define MOTORCONTROL_MAXMOTORS 2
#define MOTORCONTROL_MAXINPUTS 4
#define MOTORCONTROL_MAXENCODERS 1
#define MOTORCONTROL_MAXSENSORS 2

#define MOTORCONTROL_ERRORFLAG_OVERTEMP				0x0001
#define MOTORCONTROL_ERRORFLAG_UNDERVOLTAGE_LOCKOUT	0x0002
#define MOTORCONTROL_ERRORFLAG_NOPOWER				0x0004
#define MOTORCONTROL_ERRORFLAG_LOWPOWER				0x0008
#define MOTORCONTROL_ERRORFLAG_HIGHPOWER			0x0010
#define MOTORCONTROL_ERRORFLAG_DANGERPOWER			0x0020

struct _CPhidgetMotorControl {
	CPhidget phid;
   
	int (CCONV *fptrVelocityChange)(CPhidgetMotorControlHandle, void *, int, double);               
	int (CCONV *fptrInputChange)(CPhidgetMotorControlHandle, void *, int, int);     
	int (CCONV *fptrCurrentChange)(CPhidgetMotorControlHandle, void *, int, double);
	int (CCONV *fptrEncoderPositionChange)(CPhidgetMotorControlHandle, void *, int, int, int);
	int (CCONV *fptrEncoderPositionUpdate)(CPhidgetMotorControlHandle, void *, int, int);
	int (CCONV *fptrBackEMFUpdate)(CPhidgetMotorControlHandle, void *, int, double);       
	int (CCONV *fptrSensorUpdate)(CPhidgetMotorControlHandle, void *, int, int);    
	int (CCONV *fptrCurrentUpdate)(CPhidgetMotorControlHandle, void *, int, double);   

	void *fptrInputChangeptr;
	void *fptrVelocityChangeptr;
	void *fptrCurrentChangeptr;
	void *fptrEncoderPositionChangeptr;
	void *fptrEncoderPositionUpdateptr;
	void *fptrBackEMFUpdateptr;
	void *fptrSensorUpdateptr;
	void *fptrCurrentUpdateptr;

	//Deprecated
	int (CCONV *fptrMotorChange)(CPhidgetMotorControlHandle, void *, int, double);   
	void *fptrMotorChangeptr;

	//Returned from the device
	unsigned char inputState[MOTORCONTROL_MAXINPUTS];
	double motorSpeedEcho[MOTORCONTROL_MAXMOTORS];
	double motorSensedCurrent[MOTORCONTROL_MAXMOTORS];
	double motorSensedBackEMF[MOTORCONTROL_MAXMOTORS];
	unsigned char backEMFSensingStateEcho[MOTORCONTROL_MAXMOTORS];
	double motorSetSpeedEcho[MOTORCONTROL_MAXMOTORS];
	double motorAccelerationEcho[MOTORCONTROL_MAXMOTORS];
	double motorBrakingEcho[MOTORCONTROL_MAXMOTORS];
	double supplyVoltage;
	int encoderPositionEcho[MOTORCONTROL_MAXENCODERS];
	int encoderTimeStamp[MOTORCONTROL_MAXENCODERS];
	unsigned char ratiometricEcho;
	int sensorRawValue[MOTORCONTROL_MAXSENSORS];
	int sensorValue[MOTORCONTROL_MAXSENSORS];

	int lastPacketCount;

	//Local set data
	double motorSpeed[MOTORCONTROL_MAXMOTORS];
	double motorAcceleration[MOTORCONTROL_MAXMOTORS];
	unsigned char backEMFSensingState[MOTORCONTROL_MAXMOTORS];
	double motorBraking[MOTORCONTROL_MAXMOTORS];
	unsigned char ratiometric;

	int encoderPositionDelta[MOTORCONTROL_MAXENCODERS];
	unsigned short motorErrors[MOTORCONTROL_MAXMOTORS];

	//Constants
	double accelerationMax, accelerationMin;

	//for Webservice
	double lastVoltage;
	int encoderPositionUpdates[MOTORCONTROL_MAXENCODERS];

	unsigned char outputPacket[8];
	unsigned int outputPacketLen;
} typedef CPhidgetMotorControlInfo;
#endif

/** @} */

#endif
