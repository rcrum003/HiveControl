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

#ifndef __CPHIDGETADVANCEDSERVO
#define __CPHIDGETADVANCEDSERVO
#include "cphidget.h"

/** \defgroup phidadvservo Phidget Advanced Servo 
 * \ingroup phidgets
 * These calls are specific to the Phidget Advanced Servo object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(AdvancedServo)
CHDRSTANDARD(AdvancedServo)


/**
 * The Phidget Servo Type sets the relationship of degrees to PCM width
 */
typedef enum {
	PHIDGET_SERVO_DEFAULT = 1,			/**< Default - This is what the servo API been historically used, originally based on the Futaba FP-S148 */
	PHIDGET_SERVO_RAW_us_MODE,			/**< Raw us mode - all position, velocity, acceleration functions are specified in microseconds rather then degrees */
	PHIDGET_SERVO_HITEC_HS322HD,		/**< HiTec HS-322HD Standard Servo */
	PHIDGET_SERVO_HITEC_HS5245MG,		/**< HiTec HS-5245MG Digital Mini Servo */
	PHIDGET_SERVO_HITEC_805BB,			/**< HiTec HS-805BB Mega Quarter Scale Servo */
	PHIDGET_SERVO_HITEC_HS422,			/**< HiTec HS-422 Standard Servo */
	PHIDGET_SERVO_TOWERPRO_MG90,		/**< Tower Pro MG90 Micro Servo */
	PHIDGET_SERVO_HITEC_HSR1425CR,		/**< HiTec HSR-1425CR Continuous Rotation Servo */
	PHIDGET_SERVO_HITEC_HS785HB,		/**< HiTec HS-785HB Sail Winch Servo */
	PHIDGET_SERVO_HITEC_HS485HB,		/**< HiTec HS-485HB Deluxe Servo */
	PHIDGET_SERVO_HITEC_HS645MG,		/**< HiTec HS-645MG Ultra Torque Servo */
	PHIDGET_SERVO_HITEC_815BB,			/**< HiTec HS-815BB Mega Sail Servo */
	PHIDGET_SERVO_FIRGELLI_L12_30_50_06_R,	/**< Firgelli L12 Linear Actuator 30mm 50:1*/
	PHIDGET_SERVO_FIRGELLI_L12_50_100_06_R,	/**< Firgelli L12 Linear Actuator 50mm 100:1*/
	PHIDGET_SERVO_FIRGELLI_L12_50_210_06_R,	/**< Firgelli L12 Linear Actuator 50mm 210:1*/
	PHIDGET_SERVO_FIRGELLI_L12_100_50_06_R,	/**< Firgelli L12 Linear Actuator 100mm 50:1*/
	PHIDGET_SERVO_FIRGELLI_L12_100_100_06_R,/**< Firgelli L12 Linear Actuator 100mm 100:1*/
	PHIDGET_SERVO_SPRINGRC_SM_S2313M,	/**< SpringRC SM-S2313M Micro Servo*/
	PHIDGET_SERVO_SPRINGRC_SM_S3317M,	/**< SpringRC SM-S3317M Small Servo*/
	PHIDGET_SERVO_SPRINGRC_SM_S3317SR,	/**< SpringRC SM-S3317SR Small Continuous Rotation Servo*/
	PHIDGET_SERVO_SPRINGRC_SM_S4303R,	/**< SpringRC SM-S4303R Standard Continuous Rotation Servo*/
	PHIDGET_SERVO_SPRINGRC_SM_S4315M,	/**< SpringRC SM-S4315M High Torque Servo*/
	PHIDGET_SERVO_SPRINGRC_SM_S4315R,	/**< SpringRC SM-S4315R High Torque Continuous Rotation Servo*/
	PHIDGET_SERVO_SPRINGRC_SM_S4505B,	/**< SpringRC SM-S4505B Standard Servo*/
	PHIDGET_SERVO_USER_DEFINED
}  CPhidget_ServoType;

/**
 * Gets the number of motors supported by this controller
 * @param phid An attached phidget advanced servo handle.
 * @param count The motor count.
 */
CHDRGET(AdvancedServo,MotorCount,int *count)

/**
 * Gets the last set acceleration for a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param acceleration The acceleration
 */
CHDRGETINDEX(AdvancedServo,Acceleration,double *acceleration)
/**
 * Sets the acceleration for a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param acceleration The acceleration
 */
CHDRSETINDEX(AdvancedServo,Acceleration,double acceleration)
/**
 * Gets the maximum acceleration supported by a motor
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param max The maximum acceleration.
 */
CHDRGETINDEX(AdvancedServo,AccelerationMax,double *max)
/**
 * Gets the minimum acceleration supported by a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param min The minimum acceleration
 */
CHDRGETINDEX(AdvancedServo,AccelerationMin,double *min)

//set velocity
/**
 * Gets the last set velocity limit for a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param limit The velocity limit.
 */
CHDRGETINDEX(AdvancedServo,VelocityLimit,double *limit)
/**
 * Sets the velocity limit for a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param limit The velocity limit.
 */
CHDRSETINDEX(AdvancedServo,VelocityLimit,double limit)
//Actual velocity
/**
 * Gets the current velocity of a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param velocity The current velocity.
 */
CHDRGETINDEX(AdvancedServo,Velocity,double *velocity)
/**
 * Gets the maximum velocity that can be set for a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param max The maximum velocity
 */
CHDRGETINDEX(AdvancedServo,VelocityMax,double *max)
/**
 * Gets the minimum velocity that can be set for a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param min The minimum velocity.
 */
CHDRGETINDEX(AdvancedServo,VelocityMin,double *min)
/**
 * Sets a velocity change event handler. This is called when the velocity changes.
 * @param phid An attached phidget advanced servo handle
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(AdvancedServo,VelocityChange,double velocity)

/**
 * Gets the current position of a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param position The position.
 */
CHDRGETINDEX(AdvancedServo,Position,double *position)
/**
 * Sets the position of a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param position The position.
 */
CHDRSETINDEX(AdvancedServo,Position,double position)
/**
 * Gets the maximum position that a motor can go to.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param max The maximum position.
 */
CHDRGETINDEX(AdvancedServo,PositionMax,double *max)
/**
 * Sets the maximum position that a motor can go to.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param max The Maximum position.
 */
CHDRSETINDEX(AdvancedServo,PositionMax,double max)
/**
 * Gets the minimum position that a motor can go to.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param min The minimum position
 */
CHDRGETINDEX(AdvancedServo,PositionMin,double *min)
/**
 * Sets the minimum position that a motor can go to.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param min The minimum position
 */
CHDRSETINDEX(AdvancedServo,PositionMin,double min)
/**
 * Sets a position change event handler. This is called when the position changes.
 * @param phid An attached phidget advanced servo handle
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(AdvancedServo,PositionChange,double position)

/**
 * Gets the current current draw for a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param current The current.
 */
CHDRGETINDEX(AdvancedServo,Current,double *current)
/**
 * Sets a current change event handler. This is called when the current draw changes.
 * @param phid An attached phidget advanced servo handle
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(AdvancedServo,CurrentChange,double current)

/**
 * Gets the speed ramping state for a motor. This is whether or not velocity and acceleration are used.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param rampingState The speed ramping state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(AdvancedServo,SpeedRampingOn,int *rampingState)
/**
 * Sets the speed ramping state for a motor. This is whether or not velocity and acceleration are used.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param rampingState The speed ramping state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSETINDEX(AdvancedServo,SpeedRampingOn,int rampingState)
/**
 * Gets the engaged state of a motor. This is whether the motor is powered or not.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param engagedState The engaged state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(AdvancedServo,Engaged,int *engagedState)
/**
 * Sets the engaged state of a motor. This is whether the motor is powered or not.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param engagedState The engaged state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSETINDEX(AdvancedServo,Engaged,int engagedState)
/**
 * Gets the stopped state of a motor. This is true when the motor is not moving and there are no outstanding commands.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param stoppedState The stopped state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(AdvancedServo,Stopped,int *stoppedState)
/**
 * Gets the servo type of a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param servoType The servo type.
 */
CHDRGETINDEX(AdvancedServo,ServoType,CPhidget_ServoType *servoType)
/**
 * Sets the servo type of a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param servoType The servo type.
 */
CHDRSETINDEX(AdvancedServo,ServoType,CPhidget_ServoType servoType)
/**
 * Sets the servo parameters of a motor.
 * @param phid An attached phidget advanced servo handle
 * @param index The motor index.
 * @param min_us The minimum supported PCM in microseconds.
 * @param max_us The maximum supported PCM in microseconds.
 * @param degrees The degrees of rotation defined by the given PCM range.
 * @param velocity_max The maximum velocity in degrees/second.
 */
CHDRSETINDEX(AdvancedServo,ServoParameters,double min_us,double max_us,double degrees,double velocity_max)

#ifndef EXTERNALPROTO
#define ADVSERVO_MAXSERVOS 8

#define ADVSERVO_NORMAL_PACKET	0x00
#define ADVSERVO_MINMAX_PACKET	0x10

//Flags - need to match those defined in firmware
#define MOTOR_DONE_ADVSERVO	 		0x20
#define MOTOR_DISABLED_ADVSERVO		0x40
#define NO_RAMPING_FLAG_ADVSERVO	0x80

#define ADVSERVO_PGOOD_FLAG	0x01

//Define the parameters for several servo motors
struct _CPhidgetServoParameters
{
	CPhidget_ServoType servoType;
	double min_us;
	double max_us;
	double us_per_degree;
	double max_us_per_s;	//velocity in us notation
	unsigned char state;
} typedef CPhidgetServoParameters, *CPhidgetServoParametersHandle;

extern const CPhidgetServoParameters Phid_Servo_Types[];
int getServoParameterIndex(CPhidget_ServoType type);
PHIDGET21_API double CCONV servo_us_to_degrees(CPhidgetServoParameters params, double us, unsigned char round);
PHIDGET21_API double CCONV servo_degrees_to_us(CPhidgetServoParameters params, double degrees);
PHIDGET21_API double CCONV servo_us_to_degrees_vel(CPhidgetServoParameters params, double us, unsigned char round);
PHIDGET21_API double CCONV servo_degrees_to_us_vel(CPhidgetServoParameters params, double degrees);

PHIDGET21_API int CCONV setupNewAdvancedServoParams(CPhidgetAdvancedServoHandle phid, int Index, CPhidgetServoParameters newVal);

struct _CPhidgetAdvancedServo {
	CPhidget phid;

	int (CCONV *fptrPositionChange)(CPhidgetAdvancedServoHandle, void *, int, double);
	int (CCONV *fptrVelocityChange)(CPhidgetAdvancedServoHandle, void *, int, double);
	int (CCONV *fptrCurrentChange)(CPhidgetAdvancedServoHandle, void *, int, double);

	void *fptrPositionChangeptr;
	void *fptrVelocityChangeptr;
	void *fptrCurrentChangeptr;

	//all data is stored as microseconds
	
	//data from the device
	double motorVelocityEcho[ADVSERVO_MAXSERVOS];
	double motorPositionEcho[ADVSERVO_MAXSERVOS];
	double motorSensedCurrent[ADVSERVO_MAXSERVOS];
	unsigned char motorSpeedRampingStateEcho[ADVSERVO_MAXSERVOS];
	unsigned char motorEngagedStateEcho[ADVSERVO_MAXSERVOS];
	unsigned char motorStoppedState[ADVSERVO_MAXSERVOS];
	int packetCounterEcho[ADVSERVO_MAXSERVOS];

	//data from the user
	double motorPosition[ADVSERVO_MAXSERVOS];
	double motorAcceleration[ADVSERVO_MAXSERVOS];
	double motorVelocity[ADVSERVO_MAXSERVOS];
	unsigned char motorSpeedRampingState[ADVSERVO_MAXSERVOS];
	unsigned char motorEngagedState[ADVSERVO_MAXSERVOS];
	int packetCounter[ADVSERVO_MAXSERVOS];
	double motorPositionMax[ADVSERVO_MAXSERVOS];
	double motorPositionMin[ADVSERVO_MAXSERVOS];
	
	double velocityMax[ADVSERVO_MAXSERVOS], velocityMin;
	double velocityMaxLimit;
	double accelerationMax, accelerationMin;
	double motorPositionMaxLimit, motorPositionMinLimit;
	
	unsigned char PGoodErrState;

	CPhidgetServoParameters servoParams[ADVSERVO_MAXSERVOS];
	char *servoParamString[ADVSERVO_MAXSERVOS];

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;
} typedef CPhidgetAdvancedServoInfo;

#endif

/** @} */

#endif
