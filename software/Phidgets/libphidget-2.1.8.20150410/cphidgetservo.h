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

#ifndef __CPHIDGETSERVO
#define __CPHIDGETSERVO
#include "cphidget.h"
#include "cphidgetadvancedservo.h"

/** \defgroup phidservo Phidget Servo 
 * \ingroup phidgets
 * These calls are specific to the Phidget Servo object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(Servo)
CHDRSTANDARD(Servo)

/**
 * Gets the number of motors supported by this controller.
 * @param phid An attached phidget servo handle.
 * @param count The motor count.
 */
CHDRGET(Servo,MotorCount,int *count)

/**
 * Gets the current position of a motor.
 * @param phid An attached phidget servo handle.
 * @param index The motor index.
 * @param position The motor position.
 */
CHDRGETINDEX(Servo,Position,double *position)
/**
 * Sets the current position of a motor.
 * @param phid An attached phidget servo handle.
 * @param index The motor index.
 * @param position The motor position.
 */
CHDRSETINDEX(Servo,Position,double position)
/**
 * Gets the maximum position that a motor can be set to.
 * @param phid An attached phidget servo handle.
 * @param index The motor index.
 * @param max The maximum position.
 */
CHDRGETINDEX(Servo,PositionMax,double *max)
/**
 * Gets the minimum position that a motor can be set to.
 * @param phid An attached phidget servo handle.
 * @param index The motor index.
 * @param min The minimum position.
 */
CHDRGETINDEX(Servo,PositionMin,double *min)
/**
 * Sets a position change event handler. This is called when the position changes.
 * @param phid An attached phidget servo handle
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(Servo,PositionChange,double position)
/**
 * Gets the engaged state of a motor. This is whether the motor is powered or not.
 * @param phid An attached phidget servo handle
 * @param index The motor index.
 * @param engagedState The engaged state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(Servo,Engaged,int *engagedState)
/**
 * Sets the engaged state of a motor. This is whether the motor is powered or not.
 * @param phid An attached phidget servo handle
 * @param index The motor index.
 * @param engagedState The engaged state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSETINDEX(Servo,Engaged,int engagedState)
/**
 * Gets the servo type of a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param servoType The servo type.
 */
CHDRGETINDEX(Servo,ServoType,CPhidget_ServoType *servoType)
/**
 * Sets the servo type of a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param servoType The servo type.
 */
CHDRSETINDEX(Servo,ServoType,CPhidget_ServoType servoType)
/**
 * Sets the servo parameters of a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param min_us The minimum supported PCM in microseconds.
 * @param max_us The maximum supported PCM in microseconds.
 * @param degrees The degrees of rotation defined by the given PCM range.
 */
CHDRSETINDEX(Servo,ServoParameters,double min_us,double max_us,double degrees)

#ifndef REMOVE_DEPRECATED
DEP_CHDRGET("Deprecated - use CPhidgetServo_getMotorCount",Servo,NumMotors,int *)
DEP_CHDRSETINDEX("Deprecated - use CPhidgetServo_setPosition",Servo,MotorPosition,double)
DEP_CHDRGETINDEX("Deprecated - use CPhidgetServo_getPosition",Servo,MotorPosition,double *)
DEP_CHDRGETINDEX("Deprecated - use CPhidgetServo_getPositionMax",Servo,MotorPositionMax,double *)
DEP_CHDRGETINDEX("Deprecated - use CPhidgetServo_getPositionMin",Servo,MotorPositionMin,double *)
DEP_CHDREVENTINDEX("Deprecated - use CPhidgetServo_set_OnPositionChange_Handler",Servo,MotorPositionChange,double motorPosition)
DEP_CHDRSETINDEX("Deprecated - use CPhidgetServo_setEngaged",Servo,MotorOn,int)
DEP_CHDRGETINDEX("Deprecated - use CPhidgetServo_getEngaged",Servo,MotorOn,int *)
#endif

#ifndef EXTERNALPROTO

PHIDGET21_API int CCONV setupNewServoParams(CPhidgetServoHandle phid, int Index, CPhidgetServoParameters params);

#define SERVO_MAXSERVOS 4
struct _CPhidgetServo {
	CPhidget phid;

	int (CCONV *fptrPositionChange)(CPhidgetServoHandle, void *, int, double);           
	void *fptrPositionChangeptr;

	//Deprecated
	int (CCONV *fptrMotorPositionChange)(CPhidgetServoHandle, void *, int, double);           
	void *fptrMotorPositionChangeptr;

	double motorPositionEcho[SERVO_MAXSERVOS];
	unsigned char motorEngagedStateEcho[SERVO_MAXSERVOS];

	double motorPosition[SERVO_MAXSERVOS];
	unsigned char motorEngagedState[SERVO_MAXSERVOS];

	unsigned char fullStateEcho;

	double motorPositionMax[SERVO_MAXSERVOS], motorPositionMin[SERVO_MAXSERVOS];
	double motorPositionMaxLimit, motorPositionMinLimit;

	CPhidgetServoParameters servoParams[ADVSERVO_MAXSERVOS];
	char *servoParamString[ADVSERVO_MAXSERVOS];

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;
} typedef CPhidgetServoInfo;
#endif

/** @} */

#endif
