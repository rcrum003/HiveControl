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
#include "cphidgetadvancedservo.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //
static int CPhidgetAdvancedServo_makePacket(CPhidgetAdvancedServoHandle phid, unsigned char *buffer, int Index);

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(AdvancedServo)
	int i = 0;

	phid->motorPositionMaxLimit = PUNI_DBL;
	phid->motorPositionMinLimit = PUNI_DBL;
	phid->velocityMaxLimit = PUNI_DBL;
	phid->velocityMin = PUNI_DBL;
	phid->accelerationMax = PUNI_DBL;
	phid->accelerationMin = PUNI_DBL;
	phid->PGoodErrState = PFALSE;

	for (i = 0; i<ADVSERVO_MAXSERVOS; i++)
	{
		phid->motorVelocityEcho[i] = PUNI_DBL;
		phid->motorPositionEcho[i] = PUNI_DBL;
		phid->motorSensedCurrent[i] = PUNI_DBL;
		phid->motorSpeedRampingStateEcho[i] = PUNI_BOOL;
		phid->motorStoppedState[i] = PUNI_BOOL;
		phid->motorEngagedStateEcho[i] = PUNI_BOOL;
		phid->packetCounterEcho[i] = PUNK_INT;
		
		phid->motorPosition[i] = PUNK_DBL;
		phid->motorAcceleration[i] = PUNK_DBL;
		phid->motorVelocity[i] = PUNK_DBL;
		phid->motorSpeedRampingState[i] = PUNK_BOOL;
		phid->motorEngagedState[i] = PUNK_BOOL;
		phid->packetCounter[i] = PUNK_INT;

		phid->velocityMax[i] = PUNI_DBL;
		phid->motorPositionMax[i] = PUNI_DBL;
		phid->motorPositionMin[i] = PUNI_DBL;
		
		phid->servoParams[i].state = PUNI_BOOL;
	}
	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(AdvancedServo)
	int i = 0, result = 0, readcount = 100;
	unsigned char buffer[8];

	TESTPTR(phid);
	ZEROMEM(buffer, 8);

	//Make sure no old writes are still pending
	phid->outputPacketLen = 0;

	//Setup max/min values
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_ADVANCEDSERVO_8MOTOR:
		case PHIDID_ADVANCEDSERVO_1MOTOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 400))
			{				
				phid->motorPositionMaxLimit = 32768/12.0;
				phid->motorPositionMinLimit = 1/12.0;
				phid->velocityMaxLimit = (50/12.0) * 16384; //68266.67 us/s
				phid->velocityMin = 0;
				phid->accelerationMax = (50/12.0) / 0.02 * 16384; //3413333.33 us/s^2
				phid->accelerationMin = (50/12.0) / 0.02; //min velocity over time integration increment - 208.33 us/s^2
				
				phid->PGoodErrState = PFALSE;
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//initialize triggers, set data arrays to unknown
	for (i = 0; i<phid->phid.attr.advancedservo.numMotors; i++)
	{
		phid->motorVelocityEcho[i] = PUNK_DBL;
		phid->motorPositionEcho[i] = PUNK_DBL;
		phid->motorSensedCurrent[i] = PUNK_DBL;
		phid->motorSpeedRampingStateEcho[i] = PUNK_BOOL;
		phid->motorStoppedState[i] = PUNK_BOOL;
		phid->motorEngagedStateEcho[i] = PUNK_BOOL;
		phid->packetCounterEcho[i] = PUNK_INT;
		phid->packetCounter[i] = PUNK_INT;
		//Using this for historical reasons
		phid->servoParams[i] = Phid_Servo_Types[getServoParameterIndex(PHIDGET_SERVO_DEFAULT)];
	}

	//read in initial state - only one packet needed
	CPhidget_read((CPhidgetHandle)phid);

	//At this point, we can only recover (maybe) the position, speed ramping, engaged state.
	for (i = 0; i<phid->phid.attr.advancedservo.numMotors; i++)
	{
		phid->motorPosition[i] = phid->motorPositionEcho[i];
		phid->motorAcceleration[i] = PUNK_DBL;
		phid->motorVelocity[i] = PUNK_DBL;
		phid->motorSpeedRampingState[i] = phid->motorSpeedRampingStateEcho[i];
		phid->motorEngagedState[i] = phid->motorEngagedStateEcho[i];
		phid->packetCounter[i] = phid->packetCounterEcho[i];

		if(phid->motorStoppedState[i] == PUNK_BOOL)
		{
			if(phid->motorVelocityEcho[i] == 0 || phid->motorEngagedStateEcho[i] == PFALSE)
				phid->motorStoppedState[i] = PTRUE;
			else
				phid->motorStoppedState[i] = PFALSE;
		}
	}

	//we need to set the min/max positions for each motor according to the servo settings
	for (i = 0; i<phid->phid.attr.advancedservo.numMotors; i++)
	{
		phid->velocityMax[i] = phid->servoParams[i].max_us_per_s;

		phid->motorPositionMin[i] = phid->servoParams[i].min_us;
		if(phid->servoParams[i].max_us > phid->motorPositionMaxLimit)
			phid->motorPositionMax[i] = phid->motorPositionMaxLimit;
		else
			phid->motorPositionMax[i] = phid->servoParams[i].max_us;

		if((result = CPhidgetAdvancedServo_makePacket(phid, buffer, i + ADVSERVO_MINMAX_PACKET)))
			return result;
		if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)))
			return result;
	}

	//read till the packet counters are in sync (max of 100 reads)
	while(1)
	{
		int again = PFALSE;
		for (i = 0; i<phid->phid.attr.advancedservo.numMotors; i++)
		{
			if(phid->packetCounter[i] != phid->packetCounterEcho[i])
				again = PTRUE;
		}
		if(again)
			CPhidget_read((CPhidgetHandle)phid);
		else
			break;
		
		readcount--;
		if(readcount == 0)
			return EPHIDGET_UNEXPECTED;
	}

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(AdvancedServo)
	int i=0;

	unsigned char speedRamping[ADVSERVO_MAXSERVOS], motorEngaged[ADVSERVO_MAXSERVOS], motorDone[ADVSERVO_MAXSERVOS], justStopped[ADVSERVO_MAXSERVOS];
	double velocity[ADVSERVO_MAXSERVOS], position[ADVSERVO_MAXSERVOS], current[ADVSERVO_MAXSERVOS];
	double lastVelocity[ADVSERVO_MAXSERVOS], lastPosition[ADVSERVO_MAXSERVOS], lastCurrent[ADVSERVO_MAXSERVOS];
	int pwmEcho[ADVSERVO_MAXSERVOS];
	char error_buffer[128];

	if (length < 0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	//Parse device packets - store data locally
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_ADVANCEDSERVO_8MOTOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				for (i = 0; i < phid->phid.attr.advancedservo.numMotors; i++)
				{
					phid->packetCounterEcho[i] = buffer[0+(i*7)] & 0x0F;
					motorEngaged[i] = (buffer[0+(i*7)] & MOTOR_DISABLED_ADVSERVO) ? PFALSE : PTRUE;
					speedRamping[i] = (buffer[0+(i*7)] & NO_RAMPING_FLAG_ADVSERVO) ? PFALSE : PTRUE;
					motorDone[i] = (buffer[0+(i*7)] & MOTOR_DONE_ADVSERVO) ? PTRUE : PFALSE;

					pwmEcho[i] = (unsigned short)(buffer[1+(i*7)] << 8) + (unsigned char)(buffer[2+(i*7)]);
					position[i] = pwmEcho[i]/12.0;

					velocity[i] = (signed short)(buffer[3+(i*7)] << 8) + (unsigned char)(buffer[4+(i*7)]);
					velocity[i] = round_double(((velocity[i]/16384.0)*phid->velocityMaxLimit), 2);

					current[i] = (unsigned short)(buffer[5+(i*7)] << 8) + (unsigned char)(buffer[6+(i*7)]);
					current[i] = round_double(((50.0/11.0) * (current[i]/16384.0)), 4);
				}
			}
			//overcurrent detect
			else if ((phid->phid.deviceVersion >= 200) && (phid->phid.deviceVersion < 300))
			{
				unsigned char powerGoodEcho;

				for (i = 0; i < phid->phid.attr.advancedservo.numMotors; i++)
				{
					phid->packetCounterEcho[i] = buffer[0+(i*7)] & 0x0F;
					motorEngaged[i] = (buffer[0+(i*7)] & MOTOR_DISABLED_ADVSERVO) ? PFALSE : PTRUE;
					speedRamping[i] = (buffer[0+(i*7)] & NO_RAMPING_FLAG_ADVSERVO) ? PFALSE : PTRUE;
					motorDone[i] = (buffer[0+(i*7)] & MOTOR_DONE_ADVSERVO) ? PTRUE : PFALSE;

					pwmEcho[i] = (unsigned short)(buffer[1+(i*7)] << 8) + (unsigned char)(buffer[2+(i*7)]);
					position[i] = pwmEcho[i]/12.0;

					velocity[i] = (signed short)(buffer[3+(i*7)] << 8) + (unsigned char)(buffer[4+(i*7)]);
					velocity[i] = round_double(((velocity[i]/16384.0)*phid->velocityMaxLimit), 2);

					current[i] = (unsigned short)(buffer[5+(i*7)] << 8) + (unsigned char)(buffer[6+(i*7)]);
					current[i] = round_double(((50.0/11.0) * (current[i]/16384.0)), 4);
				}
				
				//PowerGood
				if(buffer[56] & ADVSERVO_PGOOD_FLAG)
				{
					phid->PGoodErrState = PFALSE;
					powerGoodEcho = PTRUE;
				}
				else
				{
					powerGoodEcho = PFALSE;
				}

				if(!powerGoodEcho && phid->PGoodErrState == PFALSE)
				{
					phid->PGoodErrState = PTRUE;
					FIRE_ERROR(EEPHIDGET_BADPOWER, "Bad power supply detected - undervoltage or overcurrent.");
				}
			}
			//different current sense formula
			else if ((phid->phid.deviceVersion >= 300) && (phid->phid.deviceVersion < 400))
			{
				unsigned char powerGoodEcho;

				for (i = 0; i < phid->phid.attr.advancedservo.numMotors; i++)
				{
					phid->packetCounterEcho[i] = buffer[0+(i*7)] & 0x0F;
					motorEngaged[i] = (buffer[0+(i*7)] & MOTOR_DISABLED_ADVSERVO) ? PFALSE : PTRUE;
					speedRamping[i] = (buffer[0+(i*7)] & NO_RAMPING_FLAG_ADVSERVO) ? PFALSE : PTRUE;
					motorDone[i] = (buffer[0+(i*7)] & MOTOR_DONE_ADVSERVO) ? PTRUE : PFALSE;

					pwmEcho[i] = (unsigned short)(buffer[1+(i*7)] << 8) + (unsigned char)(buffer[2+(i*7)]);
					position[i] = pwmEcho[i]/12.0;

					velocity[i] = (signed short)(buffer[3+(i*7)] << 8) + (unsigned char)(buffer[4+(i*7)]);
					velocity[i] = round_double(((velocity[i]/16384.0)*phid->velocityMaxLimit), 2);

					current[i] = (unsigned short)(buffer[5+(i*7)] << 8) + (unsigned char)(buffer[6+(i*7)]);
					current[i] = round_double((((5.0/11.0)/0.022) * (current[i]/16384.0)), 4);
				}
				
				//PowerGood
				if(buffer[56] & ADVSERVO_PGOOD_FLAG)
				{
					phid->PGoodErrState = PFALSE;
					powerGoodEcho = PTRUE;
				}
				else
				{
					powerGoodEcho = PFALSE;
				}

				if(!powerGoodEcho && phid->PGoodErrState == PFALSE)
				{
					phid->PGoodErrState = PTRUE;
					FIRE_ERROR(EEPHIDGET_BADPOWER, "Bad power supply detected - undervoltage or overcurrent.");
				}
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_ADVANCEDSERVO_1MOTOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				phid->packetCounterEcho[i] = buffer[0] & 0x0F;
				motorEngaged[i] = (buffer[0] & MOTOR_DISABLED_ADVSERVO) ? PFALSE : PTRUE;
				speedRamping[i] = (buffer[0] & NO_RAMPING_FLAG_ADVSERVO) ? PFALSE : PTRUE;
				motorDone[i] = (buffer[0] & MOTOR_DONE_ADVSERVO) ? PTRUE : PFALSE;

				pwmEcho[i] = (unsigned short)(buffer[1] << 8) + (unsigned char)(buffer[2]);
				position[i] = pwmEcho[i]/12.0;

				velocity[i] = (signed short)(buffer[3] << 8) + (unsigned char)(buffer[4]);
				velocity[i] = round_double(((velocity[i]/16384.0)*phid->velocityMaxLimit), 2);

				current[i] = (unsigned short)(buffer[5] << 8) + (unsigned char)(buffer[6]);
				current[i] = round_double((current[i]/2068.0), 4);
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//Make sure values are within defined range, and store to structure
	for (i = 0; i < phid->phid.attr.advancedservo.numMotors; i++)
	{
		lastPosition[i] = phid->motorPositionEcho[i];
		lastVelocity[i] = phid->motorVelocityEcho[i];
		lastCurrent[i] = phid->motorSensedCurrent[i];

		if(position[i] > phid->motorPositionMaxLimit || position[i] < phid->motorPositionMinLimit)
			phid->motorPositionEcho[i] = PUNK_DBL;
		else
			phid->motorPositionEcho[i] = position[i];
		if(velocity[i] > phid->velocityMaxLimit || velocity[i] < -phid->velocityMaxLimit)
			LOG(PHIDGET_LOG_WARNING, "Phidget advanced servo received out of range velocity data: %lE", velocity[i]);
		else
			phid->motorVelocityEcho[i] = velocity[i];

		phid->motorSensedCurrent[i] = current[i];

		phid->motorSpeedRampingStateEcho[i] = speedRamping[i];
		phid->motorEngagedStateEcho[i] = motorEngaged[i];
	}

	//make sure phid->motorStoppedState isn't updated until the other data is filled in
	CThread_mutex_lock(&phid->phid.outputLock);
	for (i = 0; i < phid->phid.attr.stepper.numMotors; i++)
	{
		int pwm = round(phid->motorPosition[i] * 12.0);
		//if we are up to date, and the motor is done, set stopped to true - this is the only place that this gets set true;
		justStopped[i] = PFALSE;
		if(phid->packetCounter[i] == phid->packetCounterEcho[i] && motorDone[i] == PTRUE
			&& ((pwmEcho[i] == pwm && phid->motorVelocityEcho[i] == 0) || phid->motorEngagedStateEcho[i] == PFALSE))
		{
			if(phid->motorStoppedState[i] == PFALSE)
				justStopped[i] = PTRUE;
			phid->motorStoppedState[i] = PTRUE;
		}
		else if(motorDone[i] == PFALSE)
			phid->motorStoppedState[i] = PFALSE;
	}
	CThread_mutex_unlock(&phid->phid.outputLock);

	//send out any events for changed data
	//only send a position event if the motor is engaged
	for (i = 0; i < phid->phid.attr.advancedservo.numMotors; i++)
	{
		if(phid->motorPositionEcho[i] != PUNK_DBL && phid->motorEngagedStateEcho[i] == PTRUE
			&& (phid->motorPositionEcho[i] != lastPosition[i] || justStopped[i] == PTRUE) )
			FIRE(PositionChange, i, servo_us_to_degrees(phid->servoParams[i], phid->motorPositionEcho[i], PTRUE));

		if(phid->motorVelocityEcho[i] != PUNK_DBL && phid->motorVelocityEcho[i] != lastVelocity[i])
			FIRE(VelocityChange, i, servo_us_to_degrees_vel(phid->servoParams[i], phid->motorVelocityEcho[i], PTRUE));

		if(phid->motorSensedCurrent[i] != PUNK_DBL && phid->motorSensedCurrent[i] != lastCurrent[i])
			FIRE(CurrentChange, i, phid->motorSensedCurrent[i]);
	}
	
	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(AdvancedServo)

	for (i = 0; i<phid->phid.attr.advancedservo.numMotors; i++)
	{
		if(phid->motorSensedCurrent[i] != PUNK_DBL)
			FIRE(CurrentChange, i, phid->motorSensedCurrent[i]);
		if(phid->motorVelocityEcho[i] != PUNK_DBL)
			FIRE(VelocityChange, i, servo_us_to_degrees_vel(phid->servoParams[i], phid->motorVelocityEcho[i], PTRUE));
		if(phid->motorPositionEcho[i] != PUNK_DBL && phid->motorEngagedStateEcho[i] == PTRUE)
			FIRE(PositionChange, i, servo_us_to_degrees(phid->servoParams[i], phid->motorPositionEcho[i], PTRUE));
	}

	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET_BUF(AdvancedServo)

//sendpacket - sends a packet to the device asynchronously, blocking if the 1-packet queue is full
CSENDPACKET_BUF(AdvancedServo)

//makePacket - constructs a packet using current device state
CMAKEPACKETINDEXED(AdvancedServo)
	int pwm = 0, velocity = 0, accel = 0, minpwm=0, maxpwm=0, retval=EPHIDGET_OK;
	unsigned char flags = 0;

	int packet_type = Index & 0x10;
	Index = Index & 0x07;

	TESTPTRS(phid, buffer);

	CThread_mutex_lock(&phid->phid.outputLock);

	if(phid->packetCounter[Index] == PUNK_INT)
		phid->packetCounter[Index] = 0;

	phid->packetCounter[Index]++;
	phid->packetCounter[Index] &= 0x0F;
	
	phid->motorStoppedState[Index] = PFALSE;

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_ADVANCEDSERVO_8MOTOR:
		case PHIDID_ADVANCEDSERVO_1MOTOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 400))
			{

				if(phid->motorSpeedRampingState[Index] == PUNK_BOOL)
					phid->motorSpeedRampingState[Index] = PTRUE;
				if(phid->motorEngagedState[Index] == PUNK_BOOL)
					phid->motorEngagedState[Index] = PFALSE; //note motors are not engaged by default

				if(phid->motorSpeedRampingState[Index] == PFALSE)
					flags |= NO_RAMPING_FLAG_ADVSERVO;

				if(phid->motorEngagedState[Index] == PFALSE)
					flags |= MOTOR_DISABLED_ADVSERVO;

				//2-bit index, 2-bit packet type, 4-bit counter
				buffer[0] = (Index << 5) | packet_type | phid->packetCounter[Index];
				buffer[1] = flags;

				switch(packet_type)
				{
					case ADVSERVO_NORMAL_PACKET:
						//have to make sure that everything to be sent has some sort of default value if the user hasn't set a value
						if(phid->motorPosition[Index] == PUNK_DBL)
							phid->motorPosition[Index] = round((phid->motorPositionMax[Index] - phid->motorPositionMin[Index])/2); //mid-range valid posn (much better then 0)
						if(phid->motorVelocity[Index] == PUNK_DBL)
						{
							if(phid->servoParams[Index].servoType == PHIDGET_SERVO_DEFAULT)
								phid->motorVelocity[Index] = 128/12.0*316;//historical reasons
							else
								phid->motorVelocity[Index] = phid->servoParams[Index].max_us_per_s; //max speed of the servo Phidgets sells (0.19sec/60 degrees at 4.8V) //TODO: could bump this because it's 5V
						}
						if(phid->motorAcceleration[Index] == PUNK_DBL)
							phid->motorAcceleration[Index] = phid->accelerationMax / 2; //mid-level acceleration
						pwm = round(phid->motorPosition[Index] * 12.0);
						velocity = round((phid->motorVelocity[Index] / phid->velocityMaxLimit) * 16384);
						accel = round((phid->motorAcceleration[Index] / phid->accelerationMax) * 16384);

						buffer[2] = (unsigned char)((pwm >> 8) & 0xff);
						buffer[3] = (unsigned char)(pwm & 0xff); 
						buffer[4] = (unsigned char)((velocity >> 8) & 0xff);
						buffer[5] = (unsigned char)(velocity & 0xff);
						buffer[6] = (unsigned char)((accel >> 8) & 0xff);
						buffer[7] = (unsigned char)(accel & 0xff);
						break;
					case ADVSERVO_MINMAX_PACKET:
						minpwm = round(phid->motorPositionMin[Index] * 12.0);
						maxpwm = round(phid->motorPositionMax[Index] * 12.0);

						buffer[2] = (unsigned char)((minpwm >> 8) & 0xff);
						buffer[3] = (unsigned char)(minpwm & 0xff); 
						buffer[4] = (unsigned char)((maxpwm >> 8) & 0xff);
						buffer[5] = (unsigned char)(maxpwm & 0xff);
						buffer[6] = 0;
						buffer[7] = 0;
						break;
					default:
						retval = EPHIDGET_UNEXPECTED;
						break;
				}
			}
			else
				retval = EPHIDGET_UNEXPECTED;
			break;
		default:
			retval = EPHIDGET_UNEXPECTED;
			break;
	}

	CThread_mutex_unlock(&phid->phid.outputLock);
	return retval;
}

// === Servo Definitions === //
/* Updating this list:
 *  -update the list in AS3 - AS3 has it's own version of all these numbers
 *  -updated constants/enums in .NET, COM, Java (2 files), AS3 (3 files), AppleScript
 *  -don't updated webservice version - webservice sends everything in us, and one message for servo params.
 */
const CPhidgetServoParameters Phid_Servo_Types[] = {
	{PHIDGET_SERVO_DEFAULT,					23 * 128/12.0,	243 * 128/12.0,		128/12.0,		50/12.0*16384},	//245.3us(0)-2592us(220), 10.666us/degree, max vel
	{PHIDGET_SERVO_RAW_us_MODE,				0,				10000,				1,				50/12.0*16384},	//0us-MAXus, 1us/us, max vel
	{PHIDGET_SERVO_HITEC_HS322HD,			630,			2310,				1680/180.0,		1680/180.0*316},//180 degrees, 316deg/s
	{PHIDGET_SERVO_HITEC_HS5245MG,			765,			2185,				1420/145.0,		1420/145.0*400},//145 degrees, 400deg/s
	{PHIDGET_SERVO_HITEC_805BB,				650,			2350,				1700/180.0,		1700/180.0*316},//180 degrees, 316deg/s
	{PHIDGET_SERVO_HITEC_HS422,				650,			2450,				1800/180.0,		1800/180.0*286},//180 degrees, 286deg/s
	{PHIDGET_SERVO_TOWERPRO_MG90,			485,			2335,				1850/175.0,		1850/175.0*545},//175 degrees, 545deg/s
	{PHIDGET_SERVO_HITEC_HSR1425CR,			1300,			1740,				440/100.0,		50/12.0*16384}, //"100" degrees (stopped at 50), max vel
	{PHIDGET_SERVO_HITEC_HS785HB,			700,			2550,				1850/2880.0,	1850/2880.0*225},//2881 degrees, 225deg/s
	{PHIDGET_SERVO_HITEC_HS485HB,			580,			2400,				1820/180.0,		1820/180.0*272},//180 degrees, 272deg/s
	{PHIDGET_SERVO_HITEC_HS645MG,			580,			2330,				1750/180.0,		1750/180.0*300},//180 degrees, 300deg/s
	{PHIDGET_SERVO_HITEC_815BB,				980,			2050,				1070/180.0,		1070/180.0*250},//180 degrees, 250deg/s
	{PHIDGET_SERVO_FIRGELLI_L12_30_50_06_R,	1000,			2000,				1000/30.0,		1000/30.0*23},	//30mm, 23mm/s
	{PHIDGET_SERVO_FIRGELLI_L12_50_100_06_R,1000,			2000,				1000/50.0,		1000/50.0*12},	//50mm, 12mm/s
	{PHIDGET_SERVO_FIRGELLI_L12_50_210_06_R,1000,			2000,				1000/50.0,		1000/50.0*5},	//50mm, 5mm/s
	{PHIDGET_SERVO_FIRGELLI_L12_100_50_06_R,1000,			2000,				1000/100.0,		1000/100.0*23},	//100mm, 23mm/s
	{PHIDGET_SERVO_FIRGELLI_L12_100_100_06_R,1000,			2000,				1000/100.0,		1000/100.0*12},	//100mm, 12mm/s
	{PHIDGET_SERVO_SPRINGRC_SM_S2313M,		535,			2210,				1675/180.0,		1675/180.0*600},//180 degrees, 600deg/s
	{PHIDGET_SERVO_SPRINGRC_SM_S3317M,		565,			2365,				1800/180.0,		1800/180.0*375},//180 degrees, 375deg/s
	{PHIDGET_SERVO_SPRINGRC_SM_S3317SR,		1125,			1745,				620/100.0,		50/12.0*16384},	//"100" degrees (stopped at 50 (1435)), max vel
	{PHIDGET_SERVO_SPRINGRC_SM_S4303R,		1050,			1950,				910/100.0,		50/12.0*16384},	//"100" degrees (stopped at 50 (1495)), max vel
	{PHIDGET_SERVO_SPRINGRC_SM_S4315M,		630,			2370,				1740/180.0,		1740/180.0*285},//180 degrees, 285deg/s
	{PHIDGET_SERVO_SPRINGRC_SM_S4315R,		1150,			1800,				650/100.0,		50/12.0*16384},	//"100" degrees (stopped at 50 (1475)), max vel
	{PHIDGET_SERVO_SPRINGRC_SM_S4505B,		665,			2280,				1615/180.0,		1615/180.0*400},//180 degrees, 400deg/s
	{PHIDGET_SERVO_USER_DEFINED,0,0,0,0},
	{0,0,0,0,0}//null
};

int getServoParameterIndex(CPhidget_ServoType type)
{
	int i=0;
	while(Phid_Servo_Types[i].servoType)
	{
		if(Phid_Servo_Types[i].servoType == type)
			return i;
		i++;
	}
	return 0;
}
double servo_us_to_degrees(CPhidgetServoParameters params, double us, unsigned char round)
{
	if(round)
		return round_double((us - params.min_us) / params.us_per_degree, 3); 
	return (us - params.min_us) / params.us_per_degree; 
}
double servo_degrees_to_us(CPhidgetServoParameters params, double degrees)
{
	return (degrees + (params.min_us / params.us_per_degree)) * params.us_per_degree;
}
double servo_us_to_degrees_vel(CPhidgetServoParameters params, double us, unsigned char round)
{
	if(round)
		return round_double(us / params.us_per_degree, 3); 
	return us / params.us_per_degree; 
}
double servo_degrees_to_us_vel(CPhidgetServoParameters params, double degrees)
{
	return degrees * params.us_per_degree;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(AdvancedServo, PHIDCLASS_ADVANCEDSERVO)

//event setup functions
CFHANDLE(AdvancedServo, PositionChange, int, double)
CFHANDLE(AdvancedServo, VelocityChange, int, double)
CFHANDLE(AdvancedServo, CurrentChange, int, double)

CGET(AdvancedServo,MotorCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED

	MASGN(phid.attr.advancedservo.numMotors);
}

CGETINDEX(AdvancedServo,Acceleration,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(motorAcceleration[Index], PUNK_DBL)

	*pVal = servo_us_to_degrees_vel(phid->servoParams[Index], phid->motorAcceleration[Index], PTRUE);
	return EPHIDGET_OK;
}
CSETINDEX(AdvancedServo,Acceleration,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTRANGE(servo_us_to_degrees_vel(phid->servoParams[Index], phid->accelerationMin, PFALSE)-0.5, 
		servo_us_to_degrees_vel(phid->servoParams[Index], phid->accelerationMax, PFALSE)+0.5)

	newVal = servo_degrees_to_us_vel(phid->servoParams[Index], newVal);

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Acceleration, "%lE", motorAcceleration);
	else
		SENDPACKETINDEXED(AdvancedServo, motorAcceleration[Index], Index + ADVSERVO_NORMAL_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,AccelerationMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(accelerationMax, PUNK_DBL)

	*pVal = servo_us_to_degrees_vel(phid->servoParams[Index], phid->accelerationMax, PTRUE);
	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,AccelerationMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(accelerationMin, PUNK_DBL)

	*pVal = servo_us_to_degrees_vel(phid->servoParams[Index], phid->accelerationMin, PTRUE);
	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,VelocityLimit,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(motorVelocity[Index], PUNK_DBL)

	*pVal = servo_us_to_degrees_vel(phid->servoParams[Index], phid->motorVelocity[Index], PTRUE);
	return EPHIDGET_OK;
}
CSETINDEX(AdvancedServo,VelocityLimit,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTRANGE(servo_us_to_degrees_vel(phid->servoParams[Index], phid->velocityMin, PFALSE)-0.5, 
		servo_us_to_degrees_vel(phid->servoParams[Index], phid->velocityMax[Index], PFALSE)+0.5)

	newVal = servo_degrees_to_us_vel(phid->servoParams[Index], newVal);

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(VelocityLimit, "%lE", motorVelocity);
	else
		SENDPACKETINDEXED(AdvancedServo, motorVelocity[Index], Index + ADVSERVO_NORMAL_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,Velocity,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(motorVelocityEcho[Index], PUNK_DBL)

	*pVal = servo_us_to_degrees_vel(phid->servoParams[Index], phid->motorVelocityEcho[Index], PTRUE);
	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,VelocityMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(velocityMax[Index], PUNK_DBL)

	*pVal = servo_us_to_degrees_vel(phid->servoParams[Index], phid->velocityMax[Index], PTRUE);
	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,VelocityMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(velocityMin, PUNK_DBL)

	*pVal = servo_us_to_degrees_vel(phid->servoParams[Index], phid->velocityMin, PTRUE);
	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,Position,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(motorPositionEcho[Index], PUNK_DBL)

	//send unknown if the motor is not engaged
	if(phid->motorEngagedStateEcho[Index] != PTRUE) {*pVal = PUNK_DBL; return EPHIDGET_UNKNOWNVAL;}

	*pVal = servo_us_to_degrees(phid->servoParams[Index], phid->motorPositionEcho[Index], PTRUE);
	return EPHIDGET_OK;
}
CSETINDEX(AdvancedServo,Position,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTRANGE(servo_us_to_degrees(phid->servoParams[Index], phid->motorPositionMin[Index], PFALSE)-0.5, 
		servo_us_to_degrees(phid->servoParams[Index], phid->motorPositionMax[Index], PFALSE)+0.5)

	newVal = servo_degrees_to_us(phid->servoParams[Index], newVal);

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Position, "%lE", motorPosition);
	else
		SENDPACKETINDEXED(AdvancedServo, motorPosition[Index], Index + ADVSERVO_NORMAL_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,PositionMax,double)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(motorPositionMax[Index], PUNK_DBL)

	*pVal = servo_us_to_degrees(phid->servoParams[Index], phid->motorPositionMax[Index], PTRUE);
	return EPHIDGET_OK;
}
CSETINDEX(AdvancedServo,PositionMax,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	newVal = servo_degrees_to_us(phid->servoParams[Index], newVal);
	TESTRANGE(phid->motorPositionMin[Index], phid->motorPositionMaxLimit+0.5)

	if(phid->motorPosition[Index] > newVal && phid->motorPosition[Index] != PUNK_DBL)
		phid->motorPosition[Index] = newVal;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(PositionMax, "%lE", motorPositionMax);
	else
		SENDPACKETINDEXED(AdvancedServo, motorPositionMax[Index], Index + ADVSERVO_MINMAX_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,PositionMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(motorPositionMin[Index], PUNK_DBL)

	*pVal = servo_us_to_degrees(phid->servoParams[Index], phid->motorPositionMin[Index], PTRUE);
	return EPHIDGET_OK;
}
CSETINDEX(AdvancedServo,PositionMin,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	newVal = servo_degrees_to_us(phid->servoParams[Index], newVal);
	TESTRANGE(phid->motorPositionMinLimit-0.5, phid->motorPositionMax[Index])

	if(phid->motorPosition[Index] < newVal && phid->motorPosition[Index] != PUNK_DBL)
		phid->motorPosition[Index] = newVal;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(PositionMin, "%lE", motorPositionMin);
	else
		SENDPACKETINDEXED(AdvancedServo, motorPositionMin[Index], Index + ADVSERVO_MINMAX_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,Current,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(motorSensedCurrent[Index], PUNK_DBL)

	MASGN(motorSensedCurrent[Index])
}

CGETINDEX(AdvancedServo,SpeedRampingOn,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(motorSpeedRampingStateEcho[Index], PUNK_BOOL)

	MASGN(motorSpeedRampingStateEcho[Index]);
}
CSETINDEX(AdvancedServo,SpeedRampingOn,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTRANGE(PFALSE, PTRUE)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(SpeedRampingOn, "%d", motorSpeedRampingState);
	else
		SENDPACKETINDEXED(AdvancedServo, motorSpeedRampingState[Index], Index + ADVSERVO_NORMAL_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,Engaged,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(motorEngagedStateEcho[Index], PUNK_BOOL)

	MASGN(motorEngagedStateEcho[Index])
}
CSETINDEX(AdvancedServo,Engaged,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTRANGE(PFALSE, PTRUE)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Engaged, "%d", motorEngagedState);
	else
		SENDPACKETINDEXED(AdvancedServo, motorEngagedState[Index], Index + ADVSERVO_NORMAL_PACKET);

	return EPHIDGET_OK;
}

int setupNewAdvancedServoParams(CPhidgetAdvancedServoHandle phid, int Index, CPhidgetServoParameters params)
{
	int ret;
	char newVal[256];

	//Only let raw us mode set the position to 0 - servo mode forces the use of engaged
	if(params.servoType == PHIDGET_SERVO_RAW_us_MODE)
		phid->motorPositionMinLimit = 0;
	else
		phid->motorPositionMinLimit = 1/12.0;

	phid->velocityMax[Index] = params.max_us_per_s;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		//TODO: make sure webservice sends along motorPositionMinLimit and velocityMax which may have changed
		sprintf(newVal, "%d,%lE,%lE,%lE,%lE", params.servoType, params.min_us, params.max_us, params.us_per_degree, params.max_us_per_s);
		ADDNETWORKKEYINDEXED(ServoParameters, "%s", servoParamString);
	}

	phid->servoParams[Index] = params;

	//set max velocity, adjust velocityLimit if needed
	if(phid->motorVelocity[Index] > phid->velocityMax[Index])
	{
		if((ret = CPhidgetAdvancedServo_setVelocityLimit(phid, Index, servo_us_to_degrees_vel(params, phid->velocityMax[Index], PFALSE))))
			return ret;
	}

	//Set the max/min
	//make sure we don't set mac higher then the limit
	if(params.max_us > phid->motorPositionMaxLimit)
	{
		if((ret = CPhidgetAdvancedServo_setPositionMax(phid, Index, servo_us_to_degrees(params, phid->motorPositionMaxLimit, PFALSE))))
			return ret;
	}
	else
	{
		if((ret = CPhidgetAdvancedServo_setPositionMax(phid, Index, servo_us_to_degrees(params, params.max_us, PFALSE))))
			return ret;
	}

	if((ret = CPhidgetAdvancedServo_setPositionMin(phid, Index, servo_us_to_degrees(params, params.min_us, PFALSE))))
		return ret;


	return EPHIDGET_OK;
}

CGETINDEX(AdvancedServo,ServoType,CPhidget_ServoType)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)

	MASGN(servoParams[Index].servoType)
}
CSETINDEX(AdvancedServo,ServoType,CPhidget_ServoType)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	//can't set to User defined with this function
	TESTRANGE(PHIDGET_SERVO_DEFAULT, PHIDGET_SERVO_USER_DEFINED-1)

	setupNewAdvancedServoParams(phid, Index, Phid_Servo_Types[getServoParameterIndex(newVal)]);

	return EPHIDGET_OK;
}

PHIDGET21_API int CCONV CPhidgetAdvancedServo_setServoParameters(CPhidgetAdvancedServoHandle phid, int Index, double min_us, double max_us, double degrees, double velocity_max)
{
	CPhidgetServoParameters params;
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)

	//Sanity checking of the values
	if(min_us < phid->motorPositionMinLimit)
		return EPHIDGET_INVALIDARG;
	if(max_us > phid->motorPositionMaxLimit)
		return EPHIDGET_INVALIDARG;
	if(max_us <= min_us)
		return EPHIDGET_INVALIDARG;
	if(degrees <= 0 || degrees > 1440)
		return EPHIDGET_INVALIDARG;
	if(velocity_max <= 0 || velocity_max > phid->velocityMaxLimit)
		return EPHIDGET_INVALIDARG;

	params.servoType = PHIDGET_SERVO_USER_DEFINED;
	params.min_us = min_us;
	params.max_us = max_us;
	params.us_per_degree = (max_us - min_us)/degrees;
	params.max_us_per_s = params.us_per_degree*velocity_max;

	return setupNewAdvancedServoParams(phid, Index, params);
}

CGETINDEX(AdvancedServo,Stopped,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ADVANCEDSERVO)
	TESTATTACHED
	TESTINDEX(phid.attr.advancedservo.numMotors)
	TESTMASGN(motorStoppedState[Index], PUNK_BOOL)

	MASGN(motorStoppedState[Index])
}
