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

#include "stdafx.h"
#include "cphidgetservo.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(Servo)
	int i = 0;

	phid->motorPositionMaxLimit = PUNI_DBL;
	phid->motorPositionMinLimit = PUNI_DBL;
	phid->fullStateEcho = PUNK_BOOL;

	for (i = 0; i<SERVO_MAXSERVOS; i++)
	{
		phid->motorPositionMax[i] = PUNI_DBL;
		phid->motorPositionMin[i] = PUNI_DBL;
		phid->motorPosition[i] = PUNK_DBL;
		phid->motorEngagedState[i] = PUNK_BOOL;
		phid->motorPositionEcho[i] = PUNI_DBL;
		phid->motorEngagedStateEcho[i] = PUNI_BOOL;
		phid->servoParams[i].state = PUNI_BOOL;
	}

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(Servo)
	int i = 0;

	TESTPTR(phid);
	
	//Make sure no old writes are still pending
	phid->outputPacketLen = 0;

	//Setup max/min values
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SERVO_1MOTOR:
		case PHIDID_SERVO_4MOTOR:
			if (phid->phid.deviceVersion == 200)
			{
				phid->motorPositionMaxLimit = 2550;
				phid->motorPositionMinLimit = 1;
				phid->fullStateEcho = PFALSE;
			}
			else if ((phid->phid.deviceVersion >= 300) && (phid->phid.deviceVersion < 313))
			{
				phid->motorPositionMaxLimit = 4095;
				phid->motorPositionMinLimit = 1;
				phid->fullStateEcho = PFALSE;
			}
			else if ((phid->phid.deviceVersion >= 313) && (phid->phid.deviceVersion < 400))
			{
				phid->motorPositionMaxLimit = 4095;
				phid->motorPositionMinLimit = 1;
				phid->fullStateEcho = PTRUE;
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		case PHIDID_SERVO_1MOTOR_OLD:
		case PHIDID_SERVO_4MOTOR_OLD:
			if (phid->phid.deviceVersion == 200)
			{
				phid->motorPositionMaxLimit = 2550;
				phid->motorPositionMinLimit = 1;
				phid->fullStateEcho = PFALSE;
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}
	
	for (i = 0; i<phid->phid.attr.servo.numMotors; i++)
	{
		//set data arrays to unknown
		phid->motorPositionEcho[i] = PUNK_DBL;
		phid->motorEngagedStateEcho[i] = PUNK_BOOL;
		//init servo type and max/min
		phid->servoParams[i] = Phid_Servo_Types[getServoParameterIndex(PHIDGET_SERVO_DEFAULT)];
		if(phid->servoParams[i].max_us > phid->motorPositionMaxLimit)
			phid->motorPositionMax[i] = phid->motorPositionMaxLimit;
		else
			phid->motorPositionMax[i] = phid->servoParams[i].max_us;
		phid->motorPositionMin[i] = phid->servoParams[i].min_us;
	}

	//issue a read for devices that return output data
	if(phid->fullStateEcho)
	{
		CPhidget_read((CPhidgetHandle)phid);
	}

	//recover what we can - if anything isn't filled out, it's PUNK anyways
	for (i = 0; i<phid->phid.attr.servo.numMotors; i++)
	{
		phid->motorPosition[i] = phid->motorPositionEcho[i];
		phid->motorEngagedState[i] = phid->motorEngagedStateEcho[i];
	}

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(Servo)
	int i = 0;
	double position[SERVO_MAXSERVOS];
	double lastPosition[SERVO_MAXSERVOS];

	if (length<0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	ZEROMEM(position, sizeof(position));
	ZEROMEM(lastPosition, sizeof(lastPosition));

	//Parse device packets - store data locally
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SERVO_1MOTOR:
			if ((phid->phid.deviceVersion >= 313) && (phid->phid.deviceVersion < 400))
			{
					position[0] = (((unsigned short)buffer[0]) << 5) + buffer[1];
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_SERVO_4MOTOR:
			if ((phid->phid.deviceVersion >= 313) && (phid->phid.deviceVersion < 400))
			{
				for (i = 0; i < phid->phid.attr.servo.numMotors; i++)
				{
					position[i] = (((unsigned short)buffer[i*2]) << 5) + buffer[(i*2) + 1];
				}
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_SERVO_1MOTOR_OLD:
		case PHIDID_SERVO_4MOTOR_OLD:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//Make sure values are within defined range, and store to structure
	for (i = 0; i < phid->phid.attr.servo.numMotors; i++)
	{
		lastPosition[i] = phid->motorPositionEcho[i];
		if(position[i] > phid->motorPositionMaxLimit || position[i] < phid->motorPositionMinLimit)
		{
			if(position[i] == 0)
				phid->motorEngagedStateEcho[i] = PFALSE;
			else
				phid->motorEngagedStateEcho[i] = PUNK_BOOL;
			phid->motorPositionEcho[i] = PUNK_DBL;
		}
		else
		{
			phid->motorPositionEcho[i] = position[i];
			phid->motorEngagedStateEcho[i] = PTRUE;
		}
	}
	
	//send out any events for changed data
	for (i = 0; i < phid->phid.attr.servo.numMotors; i++)
	{
		if(phid->motorPositionEcho[i] != PUNK_DBL && phid->motorPositionEcho[i] != lastPosition[i] && phid->motorEngagedStateEcho[i] == PTRUE)
		{
			FIRE(PositionChange, i, servo_us_to_degrees(phid->servoParams[i], phid->motorPositionEcho[i], PTRUE));
			//Deprecated
			FIRE(MotorPositionChange, i, servo_us_to_degrees(phid->servoParams[i], phid->motorPositionEcho[i], PTRUE));
		}
	}

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(Servo)

	for (i = 0; i < phid->phid.attr.servo.numMotors; i++)
	{
		if(phid->motorPositionEcho[i] != PUNK_DBL && phid->motorEngagedStateEcho[i] == PTRUE)
		{
			FIRE(PositionChange, i, servo_us_to_degrees(phid->servoParams[i], phid->motorPositionEcho[i], PTRUE));
			//Deprecated
			FIRE(MotorPositionChange, i, servo_us_to_degrees(phid->servoParams[i], phid->motorPositionEcho[i], PTRUE));
		}
	}

	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET_BUF(Servo)

//sendpacket - sends a packet to the device asynchronously, blocking if the 1-packet queue is full
CSENDPACKET_BUF(Servo)

//makePacket - constructs a packet using current device state
CMAKEPACKETINDEXED(Servo)
	int i = 0;

	TESTPTRS(phid, buffer);

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_SERVO_1MOTOR:
			if (phid->phid.deviceVersion == 200) //this version may not exist in this product id
			{
				buffer[0] = 0;
				if(phid->motorPosition[0] == PUNK_DBL || phid->motorEngagedState[0] == PFALSE)
					buffer[1] = 0;
				else
					buffer[1] = (unsigned char)round(phid->motorPosition[Index] / 10.0);
			}
			else if ((phid->phid.deviceVersion >= 300) && (phid->phid.deviceVersion < 400))
			{
				int microsecondPulse = 0;

				if(phid->motorPosition[0] == PUNK_DBL || phid->motorEngagedState[0] == PFALSE)
					microsecondPulse = 0;
				else
					microsecondPulse = round(phid->motorPosition[0]);

				buffer[0] = (unsigned char)(microsecondPulse & 0xFF);
				buffer[1] = (unsigned char)(microsecondPulse >> 8);
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_SERVO_4MOTOR:
			if (phid->phid.deviceVersion == 200) //this version may not exist in this product id
			{
				if (Index == 0) buffer[0] = 2;
				if (Index == 1) buffer[0] = 3;
				if (Index == 2) buffer[0] = 0;
				if (Index == 3) buffer[0] = 1;
				if(phid->motorPosition[Index] == PUNK_DBL || phid->motorEngagedState[Index] == PFALSE)
					buffer[1] = 0;
				else
					buffer[1] = (unsigned char)round(phid->motorPosition[Index]/10.0);
			}
			else if ((phid->phid.deviceVersion >= 300) && (phid->phid.deviceVersion < 400))
			{
				int microsecondPulse[4];

				ZEROMEM(microsecondPulse, sizeof(microsecondPulse));

				for (i = 0; i<phid->phid.attr.servo.numMotors; i++)
				{
					if(phid->motorPosition[i] == PUNK_DBL || phid->motorEngagedState[i] == PFALSE)
					{
						microsecondPulse[i] = 0;
						phid->motorEngagedState[i] = PFALSE;
						if (!(phid->fullStateEcho))
							phid->motorEngagedStateEcho[i] = PFALSE;
					}
					else
					{
						microsecondPulse[i] = round(phid->motorPosition[i]);
						phid->motorEngagedState[i] = PTRUE;
						if (!(phid->fullStateEcho))
							phid->motorEngagedStateEcho[i] = PTRUE;
					}
				}

				buffer[0] = (unsigned char)(microsecondPulse[0] & 0xFF);
				buffer[1] = (unsigned char)((microsecondPulse[0] >> 8) & 0x0F);

				buffer[2] = (unsigned char)(microsecondPulse[1] & 0xFF);
				buffer[1] |= (unsigned char)((microsecondPulse[1] >> 4) & 0xF0);

				buffer[3] = (unsigned char)(microsecondPulse[2] & 0xFF);
				buffer[4] = (unsigned char)((microsecondPulse[2] >> 8) & 0x0F);

				buffer[5] = (unsigned char)(microsecondPulse[3] & 0xFF);
				buffer[4] |= (unsigned char)((microsecondPulse[3] >> 4) & 0xF0);
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_SERVO_1MOTOR_OLD:
			if (phid->phid.deviceVersion == 200)
			{
				buffer[0] = 0;
				if(phid->motorPosition[0] == PUNK_DBL || phid->motorEngagedState[0] == PFALSE)
					buffer[1] = (unsigned char)0;
				else
					buffer[1] = (unsigned char)round(phid->motorPosition[Index]/10.0);
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_SERVO_4MOTOR_OLD:
			if (phid->phid.deviceVersion == 200)
			{
				if (Index == 0) buffer[0] = 2;
				if (Index == 1) buffer[0] = 3;
				if (Index == 2) buffer[0] = 0;
				if (Index == 3) buffer[0] = 1;
				if(phid->motorPosition[Index] == PUNK_DBL || phid->motorEngagedState[Index] == PFALSE)
					buffer[1] = (unsigned char)0;
				else
					buffer[1] = (unsigned char)round(phid->motorPosition[Index]/10.0);
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	return EPHIDGET_OK;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(Servo, PHIDCLASS_SERVO)

//event setup functions
CFHANDLE(Servo, PositionChange, int, double)

CGET(Servo,MotorCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_SERVO)
	TESTATTACHED

	MASGN(phid.attr.servo.numMotors)
}

CGETINDEX(Servo,Position,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_SERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.servo.numMotors)
	TESTMASGN(motorPositionEcho[Index], PUNK_DBL)

	if(phid->motorEngagedStateEcho[Index] != PTRUE) {*pVal = PUNK_DBL; return EPHIDGET_UNKNOWNVAL;}

	*pVal = servo_us_to_degrees(phid->servoParams[Index], phid->motorPositionEcho[Index], PTRUE);
	return EPHIDGET_OK;
}
CSETINDEX(Servo,Position,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_SERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.servo.numMotors)
	TESTRANGE(servo_us_to_degrees(phid->servoParams[Index], phid->motorPositionMin[Index], PFALSE), 
		servo_us_to_degrees(phid->servoParams[Index], phid->motorPositionMax[Index], PFALSE))

	newVal = servo_degrees_to_us(phid->servoParams[Index], newVal);

	//Note: setting a position will always engage a servo
	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		if(phid->motorEngagedState[Index] != PTRUE)
			CPhidgetServo_setEngaged(phid, Index, PTRUE);
		ADDNETWORKKEYINDEXED(Position, "%lE", motorPosition);
	}
	else
	{
		phid->motorEngagedState[Index] = PTRUE;
		SENDPACKETINDEXED(Servo, motorPosition[Index], Index);
		if (!(phid->fullStateEcho))
		{
			phid->motorEngagedStateEcho[Index] = PTRUE;
			if (phid->motorPositionEcho[Index] == PUNK_BOOL || phid->motorPositionEcho[Index] != newVal)
			{
				phid->motorPositionEcho[Index] = newVal;
				{
					FIRE(PositionChange, Index, servo_us_to_degrees(phid->servoParams[Index], newVal, PTRUE));
					//Deprecated
					FIRE(MotorPositionChange, Index, servo_us_to_degrees(phid->servoParams[Index], newVal, PTRUE));
				}
			}
		}
	}

	return EPHIDGET_OK;
}

CGETINDEX(Servo,PositionMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.servo.numMotors)
	TESTMASGN(motorPositionMax[Index], PUNK_DBL)

	*pVal = servo_us_to_degrees(phid->servoParams[Index], phid->motorPositionMax[Index], PFALSE);
	return EPHIDGET_OK;
}

CGETINDEX(Servo,PositionMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_SERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.servo.numMotors)
	TESTMASGN(motorPositionMin[Index], PUNK_DBL)

	*pVal = servo_us_to_degrees(phid->servoParams[Index], phid->motorPositionMin[Index], PFALSE);
	return EPHIDGET_OK;
}

CGETINDEX(Servo,Engaged,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_SERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.servo.numMotors)
	TESTMASGN(motorEngagedStateEcho[Index], PUNK_BOOL)

	MASGN(motorEngagedStateEcho[Index])
}
CSETINDEX(Servo,Engaged,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_SERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.servo.numMotors)
	TESTRANGE(PFALSE, PTRUE)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Engaged, "%d", motorEngagedState);
	else
		SENDPACKETINDEXED(Servo, motorEngagedState[Index], Index);

	return EPHIDGET_OK;
}

int setupNewServoParams(CPhidgetServoHandle phid, int Index, CPhidgetServoParameters params)
{
	char newVal[256];

	//Only let raw us mode set the position to 0 - servo mode forces the use of engaged
	if(params.servoType == PHIDGET_SERVO_RAW_us_MODE)
		phid->motorPositionMinLimit = 0;
	else
		phid->motorPositionMinLimit = 1;

	//Set the max/min
	//make sure we don't set mac higher then the limit
	if(params.max_us > phid->motorPositionMaxLimit)
		phid->motorPositionMax[Index] = phid->motorPositionMaxLimit;
	else
		phid->motorPositionMax[Index] = params.max_us;

	phid->motorPositionMin[Index] = params.min_us;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		//make sure webservice sends along motorPositionMinLimit which may have changed
		sprintf(newVal, "%d,%lE,%lE,%lE", params.servoType, params.min_us, params.max_us, params.us_per_degree);
		ADDNETWORKKEYINDEXED(ServoParameters, "%s", servoParamString);
	}

	phid->servoParams[Index] = params;

	return EPHIDGET_OK;
}

CGETINDEX(Servo,ServoType,CPhidget_ServoType)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_SERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.servo.numMotors)

	MASGN(servoParams[Index].servoType)
}

CSETINDEX(Servo,ServoType,CPhidget_ServoType)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_SERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.servo.numMotors)
	//can't set to User defined with this function
	TESTRANGE(PHIDGET_SERVO_DEFAULT, PHIDGET_SERVO_USER_DEFINED-1)

	setupNewServoParams(phid, Index, Phid_Servo_Types[getServoParameterIndex(newVal)]);

	return EPHIDGET_OK;
}

PHIDGET21_API int CCONV CPhidgetServo_setServoParameters(CPhidgetServoHandle phid, int Index, double min_us, double max_us, double degrees)
{
	CPhidgetServoParameters params;
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_SERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.servo.numMotors)

	//Sanity checking of the values
	if(min_us < phid->motorPositionMinLimit)
		return EPHIDGET_INVALIDARG;
	if(max_us > phid->motorPositionMaxLimit)
		return EPHIDGET_INVALIDARG;
	if(max_us <= min_us)
		return EPHIDGET_INVALIDARG;
	if(degrees <= 0 || degrees > 1440)
		return EPHIDGET_INVALIDARG;

	params.servoType = PHIDGET_SERVO_USER_DEFINED;
	params.min_us = min_us;
	params.max_us = max_us;
	params.us_per_degree = (max_us - min_us)/degrees;

	return setupNewServoParams(phid, Index, params);
}

// === Deprecated Functions === //

CFHANDLE(Servo, MotorPositionChange, int, double)
CGET(Servo,NumMotors,int)
	return CPhidgetServo_getMotorCount(phid, pVal);
}
CGETINDEX(Servo,MotorPosition,double)
	return CPhidgetServo_getPosition(phid, Index, pVal);
}
CSETINDEX(Servo,MotorPosition,double)
	return CPhidgetServo_setPosition(phid, Index, newVal);
}
CGETINDEX(Servo,MotorPositionMax,double)
	return CPhidgetServo_getPositionMax(phid, Index, pVal);
}
CGETINDEX(Servo,MotorPositionMin,double)
	return CPhidgetServo_getPositionMin(phid, Index, pVal);
}
CGETINDEX(Servo,MotorOn,int)
	return CPhidgetServo_getEngaged(phid, Index, pVal);
}
CSETINDEX(Servo,MotorOn,int)
	return CPhidgetServo_setEngaged(phid, Index, newVal);
}
