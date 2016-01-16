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
#include "cphidgetstepper.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(Stepper)
	int i = 0;

	phid->motorSpeedMax = PUNI_DBL;
	phid->motorSpeedMin = PUNI_DBL;
	phid->accelerationMax = PUNI_DBL;
	phid->accelerationMin = PUNI_DBL;
	phid->motorPositionMax = PUNI_INT64;
	phid->motorPositionMin = PUNI_INT64;
	phid->currentMax = PUNI_DBL;
	phid->currentMin = PUNI_DBL;

	for (i = 0; i<STEPPER_MAXINPUTS; i++)
	{
		phid->inputState[i] = PUNI_BOOL;
	}

	for (i = 0; i<STEPPER_MAXSTEPPERS; i++)
	{
		phid->motorSpeedEcho[i] = PUNI_DBL;
		phid->motorPositionEcho[i] = PUNI_INT64;
		phid->motorSensedCurrent[i] = PUNI_DBL;
		phid->motorEngagedStateEcho[i] = PUNI_BOOL;
		phid->motorStoppedState[i] = PUNI_BOOL;
		phid->packetCounterEcho[i] = PUNK_INT;
		
		phid->motorPosition[i] = PUNI_INT64;
		phid->motorPositionReset[i] = PUNK_INT64;
		phid->motorEngagedState[i] = PUNK_BOOL;
		phid->motorAcceleration[i] = PUNK_DBL;
		phid->motorSpeed[i] = PUNK_DBL;
		phid->motorCurrentLimit[i] = PUNK_DBL;
		phid->packetCounter[i] = PUNK_INT;
	}
	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(Stepper)
	int i = 0;

	TESTPTR(phid);

	//Make sure no old writes are still pending
	phid->outputPacketLen = 0;
	
	//Setup max/min values
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_STEPPER_UNIPOLAR_4MOTOR:
			phid->microSteps = 2;
			phid->motorSpeedMax = ((phid->microSteps * 0x100) - 1) * 0.75;
			phid->motorSpeedMin = 0;
			phid->accelerationMax = (250 * (0.75 * 0.75)) * 0x3f;
			phid->accelerationMin = (250 * (0.75 * 0.75));
			phid->motorPositionMax = 0x7FFFFFFFFFLL;
			phid->motorPositionMin = -0x7FFFFFFFFFLL;
			phid->currentMax = PUNK_DBL;
			phid->currentMin = PUNK_DBL;
			break;
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR:
			phid->microSteps = 16;
			phid->motorSpeedMax = 8 * phid->microSteps * 0x100; //0x8000
			phid->motorSpeedMin = 0;
			phid->accelerationMax = 4000 * 0xff;
			phid->accelerationMin = 4000;
			phid->motorPositionMax = 0x7FFFFFFFFFLL;
			phid->motorPositionMin = -0x7FFFFFFFFFLL;
			phid->currentMax = 2.492;
			phid->currentMin = 0.0542;
			break;
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR_M3:
			phid->microSteps = 16;
			phid->motorSpeedMax = 250000; //nice round number
			phid->motorSpeedMin = 0;
			phid->accelerationMax = 10000000;
			phid->accelerationMin = 2;
			phid->motorPositionMax = 1000000000000000LL; // +-1 Quadrillion - enough for 211 years at max speed
			phid->motorPositionMin = -1000000000000000LL;
			phid->currentMax = 4;
			phid->currentMin = 0;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//set data arrays to unknown
	for (i = 0; i<phid->phid.attr.stepper.numInputs; i++)
	{
		phid->inputState[i] = PUNK_BOOL;
	}
	for (i = 0; i<phid->phid.attr.stepper.numMotors; i++)
	{
		phid->motorSpeedEcho[i] = PUNK_DBL;
		phid->motorPositionEcho[i] = PUNK_INT64;
		phid->motorSensedCurrent[i] = PUNK_DBL;
		phid->motorEngagedStateEcho[i] = PUNK_BOOL;
		phid->motorStoppedState[i] = PUNK_BOOL;
		phid->packetCounterEcho[i] = PUNK_INT;
		phid->packetCounter[i] = PUNK_INT;
	}

	//read in initial state - only one packet needed
	CPhidget_read((CPhidgetHandle)phid);

	//At this point, we can only recover (maybe) the position, engaged state, set others to unknown
	for (i = 0; i<phid->phid.attr.stepper.numMotors; i++)
	{
		phid->motorPosition[i] = phid->motorPositionEcho[i];
		phid->motorEngagedState[i] = phid->motorEngagedStateEcho[i];
		phid->motorAcceleration[i] = PUNK_DBL;
		phid->motorSpeed[i] = PUNK_DBL;
		phid->motorCurrentLimit[i] = PUNK_DBL;
		phid->packetCounter[i] = phid->packetCounterEcho[i];

		if(phid->motorStoppedState[i] == PUNK_BOOL)
		{
			if(phid->motorSpeedEcho[i] == 0 || phid->motorEngagedStateEcho[i] == PFALSE)
				phid->motorStoppedState[i] = PTRUE;
			else
				phid->motorStoppedState[i] = PFALSE;
		}
	}
	
	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(Stepper)
	int i = 0, j = 0;

	double speed[STEPPER_MAXSTEPPERS], current[STEPPER_MAXSTEPPERS];
	double lastSpeed[STEPPER_MAXSTEPPERS], lastCurrent[STEPPER_MAXSTEPPERS];
	__int64 position[STEPPER_MAXSTEPPERS], lastPosition[STEPPER_MAXSTEPPERS];
	unsigned char input[STEPPER_MAXINPUTS], lastInput[STEPPER_MAXINPUTS];
	unsigned char motorEngaged[STEPPER_MAXSTEPPERS], motorDone[STEPPER_MAXSTEPPERS], justStopped[STEPPER_MAXSTEPPERS];

	if (length < 0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	ZEROMEM(input, sizeof(input));
	ZEROMEM(lastInput, sizeof(lastInput));

	//Parse device packets - store data locally
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_STEPPER_UNIPOLAR_4MOTOR:
			for (i = 0; i < phid->phid.attr.stepper.numMotors; i++)
			{
				phid->packetCounterEcho[i] = buffer[0+(i*9)] & 0x0F;
				motorEngaged[i] = (buffer[0+(i*9)] & MOTOR_DISABLED_STEPPER) ? PFALSE : PTRUE;
				motorDone[i] = (buffer[0+(i*9)] & MOTOR_DONE_STEPPER) ? PTRUE : PFALSE;

				speed[i] = (double)((signed short)((buffer[1+(i*9)] << 8) | buffer[2+(i*9)]));
				speed[i] = (double)((speed[i] / 511.0) * phid->motorSpeedMax);

				position[i] = ((((__int64)(signed char)buffer[3+(i*9)]) << 40) +
					(((__uint64)buffer[4+(i*9)]) << 32) +
					(((__uint64)buffer[5+(i*9)]) << 24) +
					(((__uint64)buffer[6+(i*9)]) << 16) +
					(((__uint64)buffer[7+(i*9)]) << 8) +
					((__uint64)buffer[8+(i*9)]));

				position[i] -= 0x20; //round
				position[i] >>= 6;

				//current is not returned
				current[i] = PUNK_DBL;
			}
			break;
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR:
			{
				double Vad;
				phid->packetCounterEcho[0] = buffer[0] & 0x0F;
				motorEngaged[0] = (buffer[0] & MOTOR_DISABLED_STEPPER) ? PFALSE : PTRUE;
				motorDone[0] = (buffer[0] & MOTOR_DONE_STEPPER) ? PTRUE : PFALSE;

				speed[0] = (double)((signed short)((buffer[1] << 8) | buffer[2]));
				speed[0] = (double)((speed[i] / 4096.0) * phid->motorSpeedMax);

				position[0] = ((((__int64)(signed char)buffer[3]) << 40) +
					(((__uint64)buffer[4]) << 32) +
					(((__uint64)buffer[5]) << 24) +
					(((__uint64)buffer[6]) << 16) +
					(((__uint64)buffer[7]) << 8) +
					((__uint64)buffer[8]));

				position[0] -= 0x04; //round
				position[0] >>= 3;

				for (i = 0, j=0x01; i<phid->phid.attr.stepper.numInputs; i++, j<<=1)
				{
					if ((buffer[9] & j))
						input[i] = PFALSE;
					else
						input[i] = PTRUE;
				}

				//value is 0-4.16 v in 8 bits 
				//(4.16 = (Vbandgap + 1.6) * 2, min 4.096v, max 4.224v according to bandgap voltage min, max)
				//ie +-1.5% error
				Vad = (((unsigned char)buffer[10] * 4.16 ) / 255.0); // The voltage sensed, to a max of 4.16v
				current[0] = round_double((Vad / (BIPOLAR_STEPPER_CURRENT_SENSE_GAIN * BIPOLAR_STEPPER_CURRENT_LIMIT_Rs)), 3);
			}
			break;
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR_M3:
			{
				phid->packetCounterEcho[0] = buffer[0] & 0x0F;
				motorEngaged[0] = (buffer[0] & MOTOR_DISABLED_STEPPER) ? PFALSE : PTRUE;
				motorDone[0] = (buffer[0] & MOTOR_DONE_STEPPER) ? PTRUE : PFALSE;

				//24.8 floating point format
				speed[0] = (double)( ((signed int)((((signed char)buffer[1]) << 24) | (buffer[2] << 16) | (buffer[3] << 8) | buffer[4])) / 256.0);

				position[0] = ((((__int64)(signed char)buffer[5]) << 56) +
					(((__uint64)buffer[6]) << 48) +
					(((__uint64)buffer[7]) << 40) +
					(((__uint64)buffer[8]) << 32) +
					(((__uint64)buffer[9]) << 24) +
					(((__uint64)buffer[10]) << 16) +
					(((__uint64)buffer[11]) << 8) +
					((__uint64)buffer[12]));
			}
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}
	
	//Make sure values are within defined range, and store to structure
	for (i = 0; i < phid->phid.attr.stepper.numMotors; i++)
	{
		lastPosition[i] = phid->motorPositionEcho[i];
		lastSpeed[i] = phid->motorSpeedEcho[i];
		lastCurrent[i] = phid->motorSensedCurrent[i];

		if(position[i] > phid->motorPositionMax || position[i] < phid->motorPositionMin)
			phid->motorPositionEcho[i] = PUNK_INT64;
		else
			phid->motorPositionEcho[i] = position[i];

		if(speed[i] > phid->motorSpeedMax || speed[i] < -phid->motorSpeedMax)
			LOG(PHIDGET_LOG_WARNING, "Phidget stepper received out of range speed data: %lE", speed[i]);
		else
			phid->motorSpeedEcho[i] = speed[i];

		phid->motorSensedCurrent[i] = current[i];

		phid->motorEngagedStateEcho[i] = motorEngaged[i];
	}

	for (i = 0; i < phid->phid.attr.stepper.numInputs; i++)
	{
		lastInput[i] = phid->inputState[i];
		phid->inputState[i] = input[i];
	}

	//make sure phid->motorStoppedState isn't updated until the other data is filled in
	CThread_mutex_lock(&phid->phid.writelock);
	for (i = 0; i < phid->phid.attr.stepper.numMotors; i++)
	{
		//if we are up to date, and the motor is done, set stopped to true - this is the only place that this gets set true;
		justStopped[i] = PFALSE;
		if(phid->packetCounter[i] == phid->packetCounterEcho[i] && motorDone[i] == PTRUE
			&& ((phid->motorPositionEcho[i] == phid->motorPosition[i] && phid->motorSpeedEcho[i] == 0) || phid->motorEngagedStateEcho[i] == PFALSE))
		{
			if(phid->motorStoppedState[i] == PFALSE)
				justStopped[i] = PTRUE;
			phid->motorStoppedState[i] = PTRUE;
		}
		else if(motorDone[i] == PFALSE)
			phid->motorStoppedState[i] = PFALSE;
	}
	//LOG(PHIDGET_LOG_VERBOSE, " ECHO: %d %d %d %d",phid->packetCounterEcho[0],phid->packetCounterEcho[1],phid->packetCounterEcho[2],phid->packetCounterEcho[3]);
	CThread_mutex_unlock(&phid->phid.writelock);

	//send out any events for changed data
	for (i = 0; i < phid->phid.attr.stepper.numInputs; i++)
	{
		if(phid->inputState[i] != PUNK_BOOL && phid->inputState[i] != lastInput[i])
			FIRE(InputChange, i, phid->inputState[i]);
	}

	for (i = 0; i < phid->phid.attr.stepper.numMotors; i++)
	{
		if(phid->motorPositionEcho[i] != PUNK_INT64 && phid->motorEngagedStateEcho[i] == PTRUE
			&& (phid->motorPositionEcho[i] != lastPosition[i] || justStopped[i] == PTRUE) )
		{
			FIRE(PositionChange, i, phid->motorPositionEcho[i]);
			FIRE(PositionChange32, i, (int)phid->motorPositionEcho[i]);
		}

		if(phid->motorSpeedEcho[i] != PUNK_DBL && phid->motorSpeedEcho[i] != lastSpeed[i])
			FIRE(VelocityChange, i, phid->motorSpeedEcho[i]);

		if(phid->motorSensedCurrent[i] != PUNK_DBL && phid->motorSensedCurrent[i] != lastCurrent[i])
			FIRE(CurrentChange, i, phid->motorSensedCurrent[i]);
	}

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(Stepper)

	for (i = 0; i<phid->phid.attr.stepper.numInputs; i++)
	{
		if(phid->inputState[i] != PUNK_BOOL)
			FIRE(InputChange, i, phid->inputState[i]);
	}
	for (i = 0; i<phid->phid.attr.stepper.numMotors; i++)
	{
		if(phid->motorSensedCurrent[i] != PUNK_DBL)
			FIRE(CurrentChange, i, phid->motorSensedCurrent[i]);
		if(phid->motorSpeedEcho[i] != PUNK_DBL)
			FIRE(VelocityChange, i, phid->motorSpeedEcho[i]);
		if(phid->motorPositionEcho[i] != PUNK_INT64 && phid->motorEngagedStateEcho[i] == PTRUE)
		{
			FIRE(PositionChange, i, phid->motorPositionEcho[i]);
			FIRE(PositionChange32, i, (int)phid->motorPositionEcho[i]);
		}
	}

	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET_BUF(Stepper)

//sendpacket - sends a packet to the device asynchronously, blocking if the 1-packet queue is full
CSENDPACKET_BUF(Stepper)

//makePacket - constructs a packet using current device state
CMAKEPACKETINDEXED(Stepper)
	unsigned char accel = 0, flags = 0;
	unsigned short speed = 0;
	__int64 position = 0;

	int packet_type = Index & 0x30;
	Index = Index & 0x0F;

	TESTPTRS(phid, buffer);

	if(phid->packetCounter[Index] == PUNK_INT)
		phid->packetCounter[Index] = 0;

	phid->packetCounter[Index]++;
	phid->packetCounter[Index] &= 0x0F;
	phid->motorStoppedState[Index] = PFALSE;

	//LOG(PHIDGET_LOG_VERBOSE, " OUT %d: %d",Index, phid->packetCounter[Index]);

	//have to make sure that everything to be sent has some sort of default value if the user hasn't set a value
	if(phid->motorEngagedState[Index] == PUNK_BOOL)
		phid->motorEngagedState[Index] = PFALSE; //note motors are not engaged by default

	if(phid->motorEngagedState[Index] == PFALSE)
		flags |= MOTOR_DISABLED_STEPPER;

	switch(packet_type)
	{
		case STEPPER_POSITION_PACKET:
			break;
		case STEPPER_VEL_ACCEL_PACKET:
			if(phid->motorSpeed[Index] == PUNK_DBL)
				phid->motorSpeed[Index] = phid->motorSpeedMax / 2;
			if(phid->motorAcceleration[Index] == PUNK_DBL)
				phid->motorAcceleration[Index] = phid->accelerationMax / 2; //mid-level acceleration
			break;
		case STEPPER_RESET_PACKET:
			phid->motorPosition[Index] = phid->motorPositionReset[Index];
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_STEPPER_UNIPOLAR_4MOTOR:
			//2-bit index, 2-bit packet type, 4-bit counter 
			buffer[0] = (Index << 6) | packet_type | phid->packetCounter[Index]; 
			//flags
			buffer[1] = flags; 

			switch(packet_type)
			{
				case STEPPER_POSITION_PACKET:
					position = phid->motorPosition[Index] << 6;
					position += 0x20;

					//48-bit position
					buffer[2] = (unsigned char)(position >> 40);
					buffer[3] = (unsigned char)(position >> 32);
					buffer[4] = (unsigned char)(position >> 24);
					buffer[5] = (unsigned char)(position >> 16);
					buffer[6] = (unsigned char)(position >> 8);
					buffer[7] = (unsigned char)(position);
					break;
				case STEPPER_VEL_ACCEL_PACKET:
					accel = (unsigned char)round((phid->motorAcceleration[Index]/phid->accelerationMax) * 63.0);
					speed = (unsigned short)round((phid->motorSpeed[Index]/phid->motorSpeedMax) * 511.0);

					//6-bit acceleration
					buffer[2] = accel;
					//9-bit speed
					buffer[3] = speed >> 8;
					buffer[4] = speed & 0xFF;
					//not used
					buffer[5] = 0;
					buffer[6] = 0;
					buffer[7] = 0;
					break;
				case STEPPER_RESET_PACKET:
					position = phid->motorPositionReset[Index] << 6;
					position += 0x20;

					//48-bit position
					buffer[2] = (unsigned char)(position >> 40);
					buffer[3] = (unsigned char)(position >> 32);
					buffer[4] = (unsigned char)(position >> 24);
					buffer[5] = (unsigned char)(position >> 16);
					buffer[6] = (unsigned char)(position >> 8);
					buffer[7] = (unsigned char)(position);
					break;
				default:
					return EPHIDGET_UNEXPECTED;
			}
			break;
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR:
			{
				unsigned char currentLimit = 0;
				double Vref;

				//2-bit index, 2-bit packet type, 4-bit counter 
				buffer[0] = (Index << 6) | packet_type | phid->packetCounter[Index]; 
				//flags
				buffer[1] = flags; 

				switch(packet_type)
				{
					case STEPPER_POSITION_PACKET:
						position = phid->motorPosition[Index] << 3;
						position += 0x04;

						//48-bit position
						buffer[2] = (unsigned char)(position >> 40);
						buffer[3] = (unsigned char)(position >> 32);
						buffer[4] = (unsigned char)(position >> 24);
						buffer[5] = (unsigned char)(position >> 16);
						buffer[6] = (unsigned char)(position >> 8);
						buffer[7] = (unsigned char)(position);
						break;
					case STEPPER_VEL_ACCEL_PACKET:
						if(phid->motorCurrentLimit[Index] == PUNK_DBL)
							phid->motorCurrentLimit[Index] = 0.50; //choose 500mA - should at least work for the most part

						accel = (unsigned char)round((phid->motorAcceleration[Index]/phid->accelerationMax) * 255.0);
						speed = (unsigned short)round((phid->motorSpeed[Index]/phid->motorSpeedMax) * 4095.0);

						// highest Vref is 3v (2.5A limit as defined by stepping chip)
						// The 8 is defined by the stepping chip - (ItripMAX = Vref/8Rs)
						Vref = phid->motorCurrentLimit[Index] * 8 * BIPOLAR_STEPPER_CURRENT_LIMIT_Rs;
						// DAC output is 0-63 = 0-4.16v, highest value is 45 (3v)
						// 2.08 = Vbandgap * 1.6
						currentLimit = (unsigned char)round( (((Vref - 2.08)/2.08) * 32) + 31 );

						//8-bit acceleration
						buffer[2] = accel;
						//12-bit speed
						buffer[3] = speed >> 8;
						buffer[4] = speed & 0xFF;
						//6-bit current limit
						buffer[5] = currentLimit;
						//not used
						buffer[6] = 0;
						buffer[7] = 0;
						break;
					case STEPPER_RESET_PACKET:
						position = phid->motorPositionReset[Index] << 3;
						position += 0x04;

						//48-bit position
						buffer[2] = (unsigned char)(position >> 40);
						buffer[3] = (unsigned char)(position >> 32);
						buffer[4] = (unsigned char)(position >> 24);
						buffer[5] = (unsigned char)(position >> 16);
						buffer[6] = (unsigned char)(position >> 8);
						buffer[7] = (unsigned char)(position);
						break;
					default:
						return EPHIDGET_UNEXPECTED;
				}
			}
			break;
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR_M3:
			{
				unsigned char currentLimit = 0;
				int accelInt, velInt;

				//2-bit index, 2-bit packet type, 4-bit counter 
				buffer[0] = (Index << 6) | packet_type | phid->packetCounter[Index]; 
				//flags
				buffer[1] = flags; 

				switch(packet_type)
				{
					case STEPPER_POSITION_PACKET:
						//64-bit position
						buffer[2] = (unsigned char)(phid->motorPosition[Index] >> 56);
						buffer[3] = (unsigned char)(phid->motorPosition[Index] >> 48);
						buffer[4] = (unsigned char)(phid->motorPosition[Index] >> 40);
						buffer[5] = (unsigned char)(phid->motorPosition[Index] >> 32);
						buffer[6] = (unsigned char)(phid->motorPosition[Index] >> 24);
						buffer[7] = (unsigned char)(phid->motorPosition[Index] >> 16);
						buffer[8] = (unsigned char)(phid->motorPosition[Index] >> 8);
						buffer[9] = (unsigned char)(phid->motorPosition[Index]);
						break;
					case STEPPER_VEL_ACCEL_PACKET:
						if(phid->motorCurrentLimit[Index] == PUNK_DBL)
							phid->motorCurrentLimit[Index] = 0.50; //choose 500mA - should at least work for the most part

						accelInt = (int)phid->motorAcceleration[Index];
						velInt = (int)phid->motorSpeed[Index];
						currentLimit = (unsigned char)round((phid->motorCurrentLimit[Index]/phid->currentMax) * 255.0);

						//3-byte acceleration
						buffer[2] = accelInt >> 16;
						buffer[3] = accelInt >> 8;
						buffer[4] = accelInt;

						//3-byte velocity
						buffer[5] = velInt >> 16;
						buffer[6] = velInt >> 8;
						buffer[7] = velInt;

						//8-bit current limit
						buffer[8] = currentLimit;

						//not used
						buffer[9] = 0;
						break;
					case STEPPER_RESET_PACKET:
						//64-bit position
						buffer[2] = (unsigned char)(phid->motorPositionReset[Index] >> 56);
						buffer[3] = (unsigned char)(phid->motorPositionReset[Index] >> 48);
						buffer[4] = (unsigned char)(phid->motorPositionReset[Index] >> 40);
						buffer[5] = (unsigned char)(phid->motorPositionReset[Index] >> 32);
						buffer[6] = (unsigned char)(phid->motorPositionReset[Index] >> 24);
						buffer[7] = (unsigned char)(phid->motorPositionReset[Index] >> 16);
						buffer[8] = (unsigned char)(phid->motorPositionReset[Index] >> 8);
						buffer[9] = (unsigned char)(phid->motorPositionReset[Index]);
						break;
					default:
						return EPHIDGET_UNEXPECTED;
				}
			}
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	return EPHIDGET_OK;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(Stepper, PHIDCLASS_STEPPER)

//event setup functions
CFHANDLE(Stepper, InputChange, int, int)
CFHANDLE(Stepper, PositionChange, int, __int64)
CFHANDLE(Stepper, PositionChange32, int, int)
CFHANDLE(Stepper, VelocityChange, int, double)
CFHANDLE(Stepper, CurrentChange, int, double)

CGET(Stepper,InputCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED

	MASGN(phid.attr.stepper.numInputs)
}

CGETINDEX(Stepper,InputState,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numInputs)
	TESTMASGN(inputState[Index], PUNK_BOOL)

	MASGN(inputState[Index])
}

CGET(Stepper,MotorCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED

	MASGN(phid.attr.stepper.numMotors)
}
CGETINDEX(Stepper,Acceleration,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorAcceleration[Index], PUNK_DBL)

	MASGN(motorAcceleration[Index])
}
CSETINDEX(Stepper,Acceleration,double)
	TESTPTR(phid) 
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTRANGE(phid->accelerationMin, phid->accelerationMax)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Acceleration, "%lE", motorAcceleration);
	else
		SENDPACKETINDEXED(Stepper, motorAcceleration[Index], Index + STEPPER_VEL_ACCEL_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(Stepper,AccelerationMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(accelerationMax, PUNK_DBL)

	MASGN(accelerationMax)
}

CGETINDEX(Stepper,AccelerationMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(accelerationMin, PUNK_DBL)

	MASGN(accelerationMin)
}

CGETINDEX(Stepper,Velocity,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorSpeedEcho[Index], PUNK_DBL)

	MASGN(motorSpeedEcho[Index])
}

CGETINDEX(Stepper,VelocityLimit,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorSpeed[Index], PUNK_DBL)

	MASGN(motorSpeed[Index])
}
CSETINDEX(Stepper,VelocityLimit,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTRANGE(phid->motorSpeedMin, phid->motorSpeedMax)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(VelocityLimit, "%lE", motorSpeed);
	else
		SENDPACKETINDEXED(Stepper, motorSpeed[Index], Index + STEPPER_VEL_ACCEL_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(Stepper,VelocityMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorSpeedMax, PUNK_DBL)

	MASGN(motorSpeedMax)
}

CGETINDEX(Stepper,VelocityMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorSpeedMin, PUNK_DBL)

	MASGN(motorSpeedMin)
}

CGETINDEX(Stepper,TargetPosition,__int64)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorPosition[Index], PUNK_INT64)

	MASGN(motorPosition[Index])
}
CSETINDEX(Stepper,TargetPosition,__int64)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTRANGE(phid->motorPositionMin, phid->motorPositionMax)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(TargetPosition, "%lld", motorPosition);
	else
		SENDPACKETINDEXED(Stepper, motorPosition[Index], Index + STEPPER_POSITION_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(Stepper,CurrentPosition,__int64)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorPositionEcho[Index], PUNK_INT64)

	MASGN(motorPositionEcho[Index])
}
CSETINDEX(Stepper,CurrentPosition,__int64)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTRANGE(phid->motorPositionMin, phid->motorPositionMax)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(CurrentPosition, "%lld", motorPositionReset);
	else
		SENDPACKETINDEXED(Stepper, motorPositionReset[Index], Index + STEPPER_RESET_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(Stepper,PositionMax,__int64)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorPositionMax, PUNK_INT64)

	MASGN(motorPositionMax)
}

CGETINDEX(Stepper,PositionMin,__int64)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorPositionMin, PUNK_INT64)

	MASGN(motorPositionMin)
}
CGETINDEX(Stepper,TargetPosition32,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorPosition[Index], PUNK_INT)

	*pVal = (int)phid->motorPosition[Index]; return EPHIDGET_OK;
}
CSETINDEX(Stepper,TargetPosition32,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTRANGE(phid->motorPositionMin, phid->motorPositionMax)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(TargetPosition, "%d", motorPosition);
	else
		SENDPACKETINDEXED(Stepper, motorPosition[Index], Index + STEPPER_POSITION_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(Stepper,CurrentPosition32,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorPositionEcho[Index], PUNK_INT)

	*pVal = (int)phid->motorPositionEcho[Index]; return EPHIDGET_OK;
}
CSETINDEX(Stepper,CurrentPosition32,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTRANGE(phid->motorPositionMin, phid->motorPositionMax)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(CurrentPosition, "%d", motorPositionReset);
	else
		SENDPACKETINDEXED(Stepper, motorPositionReset[Index], Index + STEPPER_RESET_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(Stepper,PositionMax32,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)

	*pVal = -(PUNK_INT-1); return EPHIDGET_OK;
}

CGETINDEX(Stepper,PositionMin32,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)

	*pVal = (PUNK_INT-1); return EPHIDGET_OK;
}

CGETINDEX(Stepper,Current,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED

	//only the original bipolar has current sense
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR:
			TESTINDEX(phid.attr.stepper.numMotors)
			TESTMASGN(motorSensedCurrent[Index], PUNK_DBL)
			MASGN(motorSensedCurrent[Index])
			break;
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR_M3:
		case PHIDUID_STEPPER_UNIPOLAR_4MOTOR:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CGETINDEX(Stepper,CurrentLimit,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED

	//only the bipolar has currentLimit
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR:
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR_M3:
			TESTINDEX(phid.attr.stepper.numMotors)
			TESTMASGN(motorCurrentLimit[Index], PUNK_DBL)
			MASGN(motorCurrentLimit[Index])
			break;
		case PHIDUID_STEPPER_UNIPOLAR_4MOTOR:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}
CSETINDEX(Stepper,CurrentLimit,double)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED

	//only the bipolar has currentLimit
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR:
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR_M3:
			TESTINDEX(phid.attr.stepper.numMotors)
			TESTRANGE(phid->currentMin, phid->currentMax)
			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
				ADDNETWORKKEYINDEXED(CurrentLimit, "%lE", motorCurrentLimit);
			else
				SENDPACKETINDEXED(Stepper, motorCurrentLimit[Index], Index + STEPPER_VEL_ACCEL_PACKET);
			break;
		case PHIDUID_STEPPER_UNIPOLAR_4MOTOR:
		default:
			return EPHIDGET_UNSUPPORTED;
	}

	return EPHIDGET_OK;
}

CGETINDEX(Stepper,CurrentMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED

	//only the bipolar has current
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR:
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR_M3:
			TESTINDEX(phid.attr.stepper.numMotors)
				TESTMASGN(currentMax, PUNK_DBL)
				MASGN(currentMax)
			break;
		case PHIDUID_STEPPER_UNIPOLAR_4MOTOR:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CGETINDEX(Stepper,CurrentMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED

	//only the bipolar has current
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR:
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR_M3:
			TESTINDEX(phid.attr.stepper.numMotors)
				TESTMASGN(currentMin, PUNK_DBL)
				MASGN(currentMin)
			break;
		case PHIDUID_STEPPER_UNIPOLAR_4MOTOR:
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CGETINDEX(Stepper,Engaged,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorEngagedStateEcho[Index], PUNK_BOOL)

	MASGN(motorEngagedStateEcho[Index])
}
CSETINDEX(Stepper,Engaged,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTRANGE(PFALSE, PTRUE)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Engaged, "%d", motorEngagedState);
	else
		//hang off the STEPPER_VEL_ACCEL_PACKET because this will have the side effect of also setting these two states
		SENDPACKETINDEXED(Stepper, motorEngagedState[Index], Index + STEPPER_VEL_ACCEL_PACKET);

	return EPHIDGET_OK;
}

CGETINDEX(Stepper,Stopped,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_STEPPER)
	TESTATTACHED
	TESTINDEX(phid.attr.stepper.numMotors)
	TESTMASGN(motorStoppedState[Index], PUNK_BOOL)

	MASGN(motorStoppedState[Index])
}
