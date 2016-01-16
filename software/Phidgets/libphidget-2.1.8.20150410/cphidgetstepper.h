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

#ifndef __CPHIDGETSTEPPER
#define __CPHIDGETSTEPPER
#include "cphidget.h"

/** \defgroup phidstepper Phidget Stepper 
 * \ingroup phidgets
 * These calls are specific to the Phidget Stepper object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(Stepper)
CHDRSTANDARD(Stepper)


/**
 * Gets the number of digital inputs supported by this board.
 * @param phid An attached phidget stepper handle.
 * @param count The ditial input count.
 */
CHDRGET(Stepper,InputCount,int *count)
/**
 * Gets the state of a digital input.
 * @param phid An attached phidget stepper handle.
 * @param index The input index.
 * @param inputState The input state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(Stepper,InputState,int *inputState)
/**
 * Set a digital input change handler. This is called when a digital input changes.
 * @param phid An attached phidget stepper handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(Stepper,InputChange,int inputState)

/**
 * Gets the number of motors supported by this controller
 * @param phid An attached phidget stepper handle.
 * @param count The motor count.
 */
CHDRGET(Stepper,MotorCount,int *count)

/**
 * Gets the last set acceleration for a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param acceleration The acceleration
 */
CHDRGETINDEX(Stepper,Acceleration,double *acceleration)
/**
 * Sets the acceleration for a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param acceleration The acceleration
 */
CHDRSETINDEX(Stepper,Acceleration,double acceleration)
/**
 * Gets the maximum acceleration supported by a motor
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param max The maximum acceleration.
 */
CHDRGETINDEX(Stepper,AccelerationMax,double *max)
/**
 * Gets the minimum acceleration supported by a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param min The minimum acceleration
 */
CHDRGETINDEX(Stepper,AccelerationMin,double *min)

/**
 * Gets the last set velocity limit for a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param limit The velocity limit.
 */
CHDRGETINDEX(Stepper,VelocityLimit,double *limit)
/**
 * Sets the velocity limit for a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param limit The velocity limit.
 */
CHDRSETINDEX(Stepper,VelocityLimit,double limit)
/**
 * Gets the current velocity of a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param velocity The current velocity.
 */
CHDRGETINDEX(Stepper,Velocity,double *velocity)
/**
 * Gets the maximum velocity that can be set for a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param max The maximum velocity
 */
CHDRGETINDEX(Stepper,VelocityMax,double *max)
/**
 * Gets the minimum velocity that can be set for a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param min The minimum velocity.
 */
CHDRGETINDEX(Stepper,VelocityMin,double *min)
/**
 * Sets a velocity change event handler. This is called when the velocity changes.
 * @param phid An attached phidget stepper handle
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(Stepper,VelocityChange,double velocity)

/**
 * Gets the last set target position of a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param position The position.
 */
CHDRGETINDEX(Stepper,TargetPosition,__int64 *position)
/**
 * Sets the target position of a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param position The position.
 */
CHDRSETINDEX(Stepper,TargetPosition,__int64 position)
/**
 * Gets the current position of a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param position The position.
 */
CHDRGETINDEX(Stepper,CurrentPosition,__int64 *position)
/**
 * Sets the current position of a motor. This will not move the motor, just update the position value.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param position The position.
 */
CHDRSETINDEX(Stepper,CurrentPosition,__int64 position)
/**
 * Gets the maximum position that a motor can go to.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param max The maximum position.
 */
CHDRGETINDEX(Stepper,PositionMax,__int64 *max)
/**
 * Gets the minimum position that a motor can go to.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param min The minimum position
 */
CHDRGETINDEX(Stepper,PositionMin,__int64 *min)
/**
 * Sets a position change event handler. This is called when the position changes.
 * @param phid An attached phidget stepper handle
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(Stepper,PositionChange,__int64 position)

/**
 * Gets the current limit for a motor.
 * @param phid An attached phidget stepper handle.
 * @param index The motor index.
 * @param limit The current limit.
 */
CHDRGETINDEX(Stepper,CurrentLimit,double *limit)
/**
 * Sets the current limit for a motor.
 * @param phid An attached phidget stepper handle.
 * @param index The motor index.
 * @param limit The current limit.
 */
CHDRSETINDEX(Stepper,CurrentLimit,double limit)
/**
 * Gets the current current draw for a motor.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param current The current.
 */
CHDRGETINDEX(Stepper,Current,double *current)
/**
 * Gets the maximum current limit.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param max The maximum current limit.
 */
CHDRGETINDEX(Stepper,CurrentMax,double *max)
/**
 * Gets the minimum current limit.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param min The minimum current limit.
 */
CHDRGETINDEX(Stepper,CurrentMin,double *min)
/**
 * Sets a current change event handler. This is called when the current draw changes.
 * @param phid An attached phidget stepper handle
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(Stepper,CurrentChange,double current)

/**
 * Gets the engaged state of a motor. This is whether the motor is powered or not.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param engagedState The engaged state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(Stepper,Engaged,int *engagedState)
/**
 * Sets the engaged state of a motor. This is whether the motor is powered or not.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param engagedState The engaged state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSETINDEX(Stepper,Engaged,int engagedState)
/**
 * Gets the stopped state of a motor. This is true when the motor is not moving and there are no outstanding commands.
 * @param phid An attached phidget stepper handle
 * @param index The motor index.
 * @param stoppedState The stopped state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(Stepper,Stopped,int *stoppedState)

#ifndef EXTERNALPROTO

#define STEPPER_MAXSTEPPERS 8
#define STEPPER_MAXINPUTS 8

#define BIPOLAR_STEPPER_CURRENT_SENSE_GAIN 8.5
#define BIPOLAR_STEPPER_CURRENT_LIMIT_Rs 0.150

//flags - make sure these are in the upper 4 bits
#define MOTOR_DONE_STEPPER	 	0x10
#define MOTOR_DISABLED_STEPPER	0x20

//packet types - room for one more
#define STEPPER_POSITION_PACKET		0x00
#define STEPPER_VEL_ACCEL_PACKET	0x10
#define STEPPER_RESET_PACKET		0x20

struct _CPhidgetStepper{
	CPhidget phid;
          
	int (CCONV *fptrPositionChange)(CPhidgetStepperHandle, void *, int, __int64);       
	int (CCONV *fptrPositionChange32)(CPhidgetStepperHandle, void *, int, int);    
	int (CCONV *fptrVelocityChange)(CPhidgetStepperHandle, void *, int, double);                   
	int (CCONV *fptrCurrentChange)(CPhidgetStepperHandle, void *, int, double);       
	int (CCONV *fptrInputChange)(CPhidgetStepperHandle, void *, int, int);           

	void *fptrPositionChangeptr;
	void *fptrPositionChange32ptr;
	void *fptrInputChangeptr;
	void *fptrCurrentChangeptr;
	void *fptrVelocityChangeptr;

	//data from the device
	unsigned char inputState[STEPPER_MAXINPUTS];
	__int64 motorPositionEcho[STEPPER_MAXSTEPPERS];
	double motorSpeedEcho[STEPPER_MAXSTEPPERS];
	double motorSensedCurrent[STEPPER_MAXSTEPPERS];
	unsigned char motorEngagedStateEcho[STEPPER_MAXSTEPPERS];
	unsigned char motorStoppedState[STEPPER_MAXSTEPPERS];
	int packetCounterEcho[STEPPER_MAXSTEPPERS];

	//data from the user
	__int64 motorPosition[STEPPER_MAXSTEPPERS];
	__int64 motorPositionReset[STEPPER_MAXSTEPPERS];
	double motorAcceleration[STEPPER_MAXSTEPPERS];
	double motorSpeed[STEPPER_MAXSTEPPERS];
	double motorCurrentLimit[STEPPER_MAXSTEPPERS];
	unsigned char motorEngagedState[STEPPER_MAXSTEPPERS];
	int packetCounter[STEPPER_MAXSTEPPERS];
	
	double motorSpeedMax, motorSpeedMin;
	double accelerationMax, accelerationMin;
	__int64 motorPositionMax, motorPositionMin;
	double currentMax, currentMin;
	int microSteps;

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;
} typedef CPhidgetStepperInfo;
#endif

/** @} */

#endif

