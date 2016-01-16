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

#include <string.h>
#include <assert.h>
#include "stdafx.h"
#include "cphidgetmotorcontrol.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(MotorControl)
	int i = 0;

	phid->accelerationMax = PUNI_DBL;
	phid->accelerationMin = PUNI_DBL;

	for (i = 0; i<MOTORCONTROL_MAXINPUTS; i++)
	{
		phid->inputState[i] = PUNI_BOOL;
	}
	for (i = 0; i<MOTORCONTROL_MAXMOTORS; i++)
	{
		phid->motorSpeedEcho[i] = PUNI_DBL;
		phid->motorSensedCurrent[i] = PUNI_DBL;
		phid->motorSensedBackEMF[i] = PUNI_DBL;
		phid->motorSetSpeedEcho[i] = PUNK_DBL;
		phid->motorAccelerationEcho[i] = PUNI_DBL;
		phid->motorBrakingEcho[i] = PUNI_DBL;
		phid->backEMFSensingStateEcho[i] = PUNI_BOOL;

		phid->motorSpeed[i] = PUNK_DBL;
		phid->motorAcceleration[i] = PUNK_DBL;
		phid->motorBraking[i] = PUNK_DBL;
		phid->backEMFSensingState[i] = PUNK_BOOL;
		phid->motorErrors[i] = 0;
	}
	phid->supplyVoltage = PUNI_DBL;
	for(i = 0; i<MOTORCONTROL_MAXENCODERS; i++)
	{
		phid->encoderPositionEcho[i] = 0;
		phid->encoderTimeStamp[i] = 0;
		phid->encoderPositionDelta[i] = 0;
	}
	for(i = 0; i<MOTORCONTROL_MAXSENSORS; i++)
	{
		phid->sensorRawValue[i] = PUNI_INT;
		phid->sensorValue[i] = PUNI_INT;
	}
	phid->ratiometricEcho = PUNI_BOOL;
	phid->ratiometric = PUNK_BOOL;

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(MotorControl)
	int i = 0;
	int readtries = 0;

	TESTPTR(phid);

	//Make sure no old writes are still pending
	phid->outputPacketLen = 0;

	//Setup max/min values
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				phid->accelerationMax = 100;
				phid->accelerationMin = round_double((1.0 / 10.23), 2);
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		case PHIDID_MOTORCONTROL_1MOTOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				phid->accelerationMax = 100/0.016; //0-100 in 16ms
				phid->accelerationMin = round_double(100/4.08, 2); //0-100 in 4.08 seconds
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//initialize triggers, set data arrays to unknown
	for (i = 0; i<phid->phid.attr.motorcontrol.numInputs; i++)
	{
		phid->inputState[i] = PUNK_BOOL;
	}
	for (i = 0; i<phid->phid.attr.motorcontrol.numMotors; i++)
	{
		phid->motorSpeedEcho[i] = PUNK_DBL;
		phid->motorSensedCurrent[i] = PUNK_DBL;
		phid->motorSensedBackEMF[i] = PUNK_DBL;
		phid->motorSetSpeedEcho[i] = PUNK_DBL;
		phid->motorAccelerationEcho[i] = PUNK_DBL;
		//set some defaults for devices that don't support these
		switch(phid->phid.deviceIDSpec)
		{
			case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
			case PHIDID_MOTORCONTROL_HC_2MOTOR:
				phid->motorBrakingEcho[i] = 0;
				phid->backEMFSensingStateEcho[i] = PFALSE;
				break;
			case PHIDID_MOTORCONTROL_1MOTOR:
				phid->motorBrakingEcho[i] = PUNK_DBL;
				phid->backEMFSensingStateEcho[i] = PUNK_BOOL;
				break;
			default:
				break;
		}
		phid->motorErrors[i] = 0;
	}
	phid->supplyVoltage = PUNK_DBL;
	phid->lastVoltage = PUNK_DBL;
	for(i = 0; i<phid->phid.attr.motorcontrol.numEncoders; i++)
	{
		phid->encoderPositionEcho[i] = 0;
		phid->encoderTimeStamp[i] = 0;
		phid->encoderPositionDelta[i] = 0;
	}
	for(i = 0; i<phid->phid.attr.motorcontrol.numSensors; i++)
	{
		phid->sensorRawValue[i] = PUNK_INT;
		phid->sensorValue[i] = PUNK_INT;
	}
	phid->ratiometricEcho = PUNK_BOOL;

	phid->lastPacketCount = PUNK_INT;

	//read in initial state
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
		case PHIDID_MOTORCONTROL_1MOTOR:
			readtries = 1;
			break;
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
			readtries = phid->phid.attr.motorcontrol.numMotors * 2;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}
	while(readtries-- > 0)
	{
		CPhidget_read((CPhidgetHandle)phid);
		for (i = 0; i<phid->phid.attr.motorcontrol.numMotors; i++)
			if(phid->motorSpeedEcho[i] == PUNK_DBL)
				break;
		if(i==phid->phid.attr.motorcontrol.numMotors) break;
	}

	//recover what we can, set others to unknown
	for (i = 0; i<phid->phid.attr.motorcontrol.numMotors; i++)
	{
		if(phid->motorSetSpeedEcho[i] != PUNK_DBL)
			phid->motorSpeed[i] = phid->motorSetSpeedEcho[i];
		else
			phid->motorSpeed[i] = phid->motorSpeedEcho[i];
		phid->motorAcceleration[i] = phid->motorAccelerationEcho[i];
		phid->motorBraking[i] = phid->motorBrakingEcho[i];
		phid->backEMFSensingState[i] = phid->backEMFSensingStateEcho[i];
	}
	for(i = 0; i<phid->phid.attr.motorcontrol.numEncoders; i++)
	{
		phid->encoderPositionDelta[i] = phid->encoderPositionEcho[i];
	}
	phid->ratiometric = phid->ratiometricEcho;

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(MotorControl)
	int i = 0, j = 0;

	if (length < 0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	switch(phid->phid.deviceIDSpec)
	{
		/* Original Motor controller */
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				double speed[MOTORCONTROL_MAXMOTORS];
				double lastSpeed[MOTORCONTROL_MAXMOTORS];
				unsigned char input[MOTORCONTROL_MAXINPUTS];
				unsigned char lastInputState[MOTORCONTROL_MAXINPUTS];
				unsigned char error[MOTORCONTROL_MAXINPUTS];

				ZEROMEM(speed, sizeof(speed));
				ZEROMEM(lastSpeed, sizeof(lastSpeed));
				ZEROMEM(input, sizeof(input));
				ZEROMEM(lastInputState, sizeof(lastInputState));
				ZEROMEM(error, sizeof(error));

				//Parse device packet - store data locally
				for (i = 0, j = 1; i < phid->phid.attr.motorcontrol.numInputs; i++, j<<=1)
				{
					if (buffer[0] & j)
						input[i] = PTRUE;
					else
						input[i] = PFALSE;
				}
				for (i = 0, j = 1; i < phid->phid.attr.motorcontrol.numMotors; i++, j<<=1)
				{
					speed[i] = (char)buffer[4 + i];
					speed[i] = round_double(((speed[i] * 100) / 127.0), 2);

					//errors
					if (buffer[1] & j)
					{
						error[i] = PTRUE;
					}
				}
				
				//Make sure values are within defined range, and store to structure
				for (i = 0; i < phid->phid.attr.motorcontrol.numInputs; i++)
				{
					lastInputState[i] = phid->inputState[i];
					phid->inputState[i] = input[i];
				}
				for (i = 0; i<phid->phid.attr.motorcontrol.numMotors; i++)
				{
					lastSpeed[i] = phid->motorSpeedEcho[i];
					phid->motorSpeedEcho[i] = speed[i];
				}

				//send out any events for changed data
				for (i = 0; i < phid->phid.attr.motorcontrol.numInputs; i++)
				{
					if(phid->inputState[i] != PUNK_BOOL && phid->inputState[i] != lastInputState[i])
						FIRE(InputChange, i, phid->inputState[i]);
				}
				for (i = 0; i<phid->phid.attr.motorcontrol.numMotors; i++)
				{
					if(phid->motorSpeedEcho[i] != PUNK_DBL && phid->motorSpeedEcho[i] != lastSpeed[i])
					{
						FIRE(VelocityChange, i, phid->motorSpeedEcho[i]);
						//Deprecated
						FIRE(MotorChange, i, phid->motorSpeedEcho[i]);
					}
					if(error[i])
					{
						char error_buffer[50];
						FIRE_ERROR(EEPHIDGET_OVERCURRENT, "Motor %d exceeded 1.5 Amp current limit.", i);
					}
				}

			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		/* HC motor controller - packets are indexed */
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				int index = 0;
				int error[MOTORCONTROL_MAXINPUTS];
				double speed = 0, current = 0;
				double lastSpeed = 0, lastCurrent = 0;

				ZEROMEM(error, sizeof(error));
				
				//Parse device packet - store data locally
				index = buffer[3];

				speed = (signed char)buffer[4];
				speed = round_double(((speed * 100) / 127.0), 2);

				//NOTE: current sense is only accurate at +100 and -100 velocity
				current = (unsigned int)(((unsigned char)buffer[6] << 8) | (unsigned char)buffer[7]);
				current -= 5;
				if(current < 0) current = 0;
				current /= 51.2; //volts
				current = (current * 11370) / 1500; //amps

				if (!(buffer[1] & 0x10)) error[0] |= 0x01;
				if (!(buffer[1] & 0x20)) error[1] |= 0x01;
				if (!(buffer[1] & 0x40)) error[0] |= 0x02;
				if (!(buffer[1] & 0x80)) error[1] |= 0x02;

				//Make sure values are within defined range, and store to structure
				lastSpeed = phid->motorSpeedEcho[index];
				phid->motorSpeedEcho[index] = speed;
				lastCurrent = phid->motorSensedCurrent[index];
				phid->motorSensedCurrent[index] = current;
				
				//send out any events for changed data
				if(phid->motorSpeedEcho[index] != PUNK_DBL && phid->motorSpeedEcho[index] != lastSpeed)
				{
					FIRE(VelocityChange, index, phid->motorSpeedEcho[index]);
					//Deprecated
					FIRE(MotorChange, index, phid->motorSpeedEcho[index]);
				}
				if(phid->motorSensedCurrent[index] != PUNK_DBL && phid->motorSensedCurrent[index] != lastCurrent)
					FIRE(CurrentChange, index, phid->motorSensedCurrent[index]);
				if(phid->motorSensedCurrent[index] != PUNK_DBL)
					FIRE(CurrentUpdate, index, phid->motorSensedCurrent[index]);
				for (i = 0; i<phid->phid.attr.motorcontrol.numMotors; i++)
				{
					char error_buffer[50];
					//There are two error conditions - but as far as I can tell they both mean the same thing
					if(error[i])
					{
						FIRE_ERROR(EEPHIDGET_OVERTEMP, "Motor %d overtemperature or short detected.", i);
					}
				}
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		/* 1-Motor controller with encoder and sensor inputs */
		case PHIDID_MOTORCONTROL_1MOTOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				int i=0,j=0;
				double speed = 0, current = 0, backEmf = 0, backEmfPos = 0, backEmfNeg = 0, supplyVoltage = 0, setSpeed = 0, setAccel = 0, setBraking = 0;
				double lastSpeed = 0, lastCurrent = 0;
				int encoderPos = 0, timeChangeInt = 0;
				unsigned short encoderTimeChange = 0, encoderTime = 0;
				int packetCount = 0;
				unsigned char ratioEn = 0, dir = 0, bEMFValid = 0, bEMFenabled = 0, lowVoltageErr = 0, overTempErr = 0, setDir = 0;
				int sensor[MOTORCONTROL_MAXSENSORS], sensorRaw[MOTORCONTROL_MAXSENSORS];
				unsigned char input[MOTORCONTROL_MAXINPUTS];
				unsigned char lastInputState[MOTORCONTROL_MAXINPUTS];
				int encoderPositionChange = 0;
				char error_buffer[128];
				ZEROMEM(sensorRaw, sizeof(sensorRaw));
				ZEROMEM(sensor, sizeof(sensor));
				ZEROMEM(input, sizeof(input));
				ZEROMEM(lastInputState, sizeof(lastInputState));

				//Read in data
				for (i = 0, j = 1; i < phid->phid.attr.motorcontrol.numInputs; i++, j<<=1)
				{
					if (buffer[0] & j)
						input[i] = PTRUE;
					else
						input[i] = PFALSE;
				}

				overTempErr = ((buffer[1] & 0x01) ? PTRUE : PFALSE);
				lowVoltageErr = ((buffer[1] & 0x02) ? PTRUE : PFALSE);
				bEMFValid = ((buffer[1] & 0x04) ? PTRUE : PFALSE);
				dir = ((buffer[1] & 0x08) ? PTRUE : PFALSE);
				ratioEn = ((buffer[1] & 0x10) ? PTRUE : PFALSE);
				bEMFenabled = ((buffer[1] & 0x20) ? PTRUE : PFALSE);
				setDir = ((buffer[1] & 0x40) ? PTRUE : PFALSE);

				for (i = 0; i < phid->phid.attr.motorcontrol.numSensors; i++)
				{
					sensorRaw[i] = ((unsigned char)buffer[i*2 + 4] << 8) + (unsigned char)buffer[i*2 + 5];
					sensorRaw[i] = round(sensorRaw[i] * 1.001);
					if(sensorRaw[i] > 0xfff)
						sensorRaw[i] = 0xfff;
					sensor[i] = round((double)sensorRaw[i] / 4.095);
				}

				//TODO: check this calculation
				current = (unsigned int)(((unsigned char)buffer[2] << 8) + (unsigned char)buffer[3]);
				current = ((current * 5) / 4096.0); //voltage on adc
				current = (current / 150.0); //Current through resistor (0.24% of current through motor)
				current = round_double(current / 0.0024, 3); //Normalized Current

				backEmfPos = (unsigned int)(((unsigned char)buffer[10] << 8) + (unsigned char)buffer[11]);
				backEmfNeg = (unsigned int)(((unsigned char)buffer[12] << 8) + (unsigned char)buffer[13]);
				backEmf = backEmfPos - backEmfNeg;
				backEmf = round_double(((backEmf * 80) / 4096.0), 3);

				supplyVoltage = (unsigned int)(((unsigned char)buffer[8] << 8) + (unsigned char)buffer[9]);
				supplyVoltage = round_double(((supplyVoltage * 80) / 2048.0), 3); //80v full scale on ADC

				//LOG(PHIDGET_LOG_DEBUG, "Supply Voltage: %0.3lf", supplyVoltage);

				speed = ((unsigned char)buffer[20] / 255.0) * 100.0;
				if(!dir)
					speed = -speed;

				setSpeed = ((unsigned char)buffer[21] / 255.0) * 100.0;
				if(!setDir)
					setSpeed = -setSpeed;

				setAccel = ((unsigned char)buffer[22] / 255.0) * phid->accelerationMax;
				setBraking = ((unsigned char)buffer[23] / 255.0) * 100.0;

				packetCount = (unsigned char)buffer[24];

				encoderPos = (signed int)(((unsigned char)buffer[14] << 24) 
					+ ((unsigned char)buffer[15] << 16) 
					+ ((unsigned char)buffer[16] << 8) 
					+ (unsigned char)buffer[17]);

				encoderTime = (unsigned int)(((unsigned char)buffer[18] << 8) + (unsigned char)buffer[19]);

				//Do stuff with the data
				//Make sure values are within defined range, and store to structure
				for (i = 0; i < phid->phid.attr.motorcontrol.numInputs; i++)
				{
					lastInputState[i] = phid->inputState[i];
					phid->inputState[i] = input[i];
				}

				lastSpeed = phid->motorSpeedEcho[0];
				phid->motorSpeedEcho[0] = speed;

				lastCurrent = phid->motorSensedCurrent[0];
				phid->motorSensedCurrent[0] = current;

				phid->motorSetSpeedEcho[0] = setSpeed;
				phid->motorAccelerationEcho[0] = setAccel;
				phid->motorBrakingEcho[0] = setBraking;

				if(bEMFValid)
					phid->motorSensedBackEMF[0] = backEmf;
				if(!bEMFenabled)
					phid->motorSensedBackEMF[0] = PUNK_DBL;
				phid->backEMFSensingStateEcho[0] = bEMFenabled;

				phid->supplyVoltage = supplyVoltage;

				//check for over/undershoots on encoder
				if(((int)(phid->encoderPositionEcho[0] - phid->encoderPositionDelta[0]) > 2000000000 
					&& (int)(encoderPos - phid->encoderPositionDelta[0]) < -2000000000)
					|| ((int)(phid->encoderPositionEcho[0] - phid->encoderPositionDelta[0]) < -2000000000 
					&& (int)(encoderPos - phid->encoderPositionDelta[0]) > 2000000000))
				{
					char error_buffer[50];
					FIRE_ERROR(EEPHIDGET_WRAP, "Encoder position is wrapping around.");
				}

				encoderPositionChange = encoderPos - phid->encoderPositionEcho[0];
				phid->encoderPositionEcho[0] = encoderPos;
					
				//this handles wraparounds because we're using unsigned shorts
				encoderTimeChange = (encoderTime - phid->encoderTimeStamp[0]);

				//timeout is 20 seconds
				if (encoderTimeChange > 60000 || phid->encoderTimeStamp[0] == PUNK_INT)
					timeChangeInt = PUNK_INT;
				else
					timeChangeInt = encoderTimeChange;

				phid->encoderTimeStamp[0] = encoderTime;

				for (i = 0; i < phid->phid.attr.motorcontrol.numSensors; i++)
				{
					phid->sensorValue[i] = sensor[i];
					phid->sensorRawValue[i] = sensorRaw[i];
				}
				phid->ratiometricEcho = ratioEn;

				//Send out events
				for (i = 0; i < phid->phid.attr.motorcontrol.numInputs; i++)
				{
					if(phid->inputState[i] != PUNK_BOOL && phid->inputState[i] != lastInputState[i])
						FIRE(InputChange, i, phid->inputState[i]);
				}
				if(phid->motorSpeedEcho[0] != PUNK_DBL && phid->motorSpeedEcho[0] != lastSpeed)
				{
					FIRE(VelocityChange, 0, phid->motorSpeedEcho[0]);
					//Deprecated
					FIRE(MotorChange, 0, phid->motorSpeedEcho[0]);
				}
				if(phid->motorSensedCurrent[0] != PUNK_DBL && phid->motorSensedCurrent[0] != lastCurrent)
					FIRE(CurrentChange, 0, phid->motorSensedCurrent[0]);

				if(phid->motorSensedCurrent[0] != PUNK_DBL)
					FIRE(CurrentUpdate, 0, phid->motorSensedCurrent[0]);

				for (i = 0; i < phid->phid.attr.motorcontrol.numSensors; i++)
					FIRE(SensorUpdate, i, phid->sensorValue[i]);
				
				if(encoderPositionChange != 0)
					FIRE(EncoderPositionChange, 0, timeChangeInt, encoderPositionChange);
				FIRE(EncoderPositionUpdate, 0, encoderPositionChange);

				if(bEMFValid)
					FIRE(BackEMFUpdate, 0, backEmf);

				//Error Events:

				//This is true when MC33931 status pin is LOW (active)
				// could be under-voltage lockout, over-temperature, or short-circuit
				// over-temperature and short-circuit conditions need to be cleared by toggling D1 or D2
				if(overTempErr && !(phid->motorErrors[0] & MOTORCONTROL_ERRORFLAG_OVERTEMP))
				{
					phid->motorErrors[0] |= MOTORCONTROL_ERRORFLAG_OVERTEMP;
					FIRE_ERROR(EEPHIDGET_OVERTEMP, "Over-temperature / Short-circuit condition detected.");
				}
				else if(!overTempErr && (phid->motorErrors[0] & MOTORCONTROL_ERRORFLAG_OVERTEMP))
				{
					phid->motorErrors[0] &= ~MOTORCONTROL_ERRORFLAG_OVERTEMP;
					FIRE_ERROR(EEPHIDGET_OK, "Over-temperature / Short-circuit condition ended.");
				}
				//if supply voltage is >5v, then we have likely already recovered from this error. This happens when the power is first plugged in
				//This error happens when the power supply voltage falls, not when it starts low
				if(lowVoltageErr && supplyVoltage < 5 && !(phid->motorErrors[0] & MOTORCONTROL_ERRORFLAG_UNDERVOLTAGE_LOCKOUT))
				{
					phid->motorErrors[0] |= MOTORCONTROL_ERRORFLAG_UNDERVOLTAGE_LOCKOUT;
					FIRE_ERROR(EEPHIDGET_BADPOWER, "Under-voltage Lockout condition detected. Supply voltage is %0.2lfv", supplyVoltage);
				}
				else if(!lowVoltageErr && (phid->motorErrors[0] & MOTORCONTROL_ERRORFLAG_UNDERVOLTAGE_LOCKOUT))
				{
					phid->motorErrors[0] &= ~MOTORCONTROL_ERRORFLAG_UNDERVOLTAGE_LOCKOUT;
					FIRE_ERROR(EEPHIDGET_OK, "Under-voltage Lockout condition ended.");
				}
				//Errors I detect - not error flags from the Motor Control IC:
				if(supplyVoltage < 1 && !(phid->motorErrors[0] & MOTORCONTROL_ERRORFLAG_NOPOWER))
				{
					phid->motorErrors[0] |= MOTORCONTROL_ERRORFLAG_NOPOWER;
					FIRE_ERROR(EEPHIDGET_BADPOWER, "Power supply is unplugged or non-functional. Supply voltage is %0.2lfv", supplyVoltage);
				}
				else if(supplyVoltage <= 7 && !(phid->motorErrors[0] & MOTORCONTROL_ERRORFLAG_LOWPOWER))
				{
					phid->motorErrors[0] |= MOTORCONTROL_ERRORFLAG_LOWPOWER;
					FIRE_ERROR(EEPHIDGET_BADPOWER, "Power supply voltage is too low. Supply voltage is %0.2lfv", supplyVoltage);
				}
				if(supplyVoltage >= 40 && !(phid->motorErrors[0] & MOTORCONTROL_ERRORFLAG_DANGERPOWER))
				{
					phid->motorErrors[0] |= MOTORCONTROL_ERRORFLAG_DANGERPOWER;
					FIRE_ERROR(EEPHIDGET_BADPOWER, "DANGER: Over-voltage condition, braking motor. Supply voltage is %0.2lfv", supplyVoltage);
				}
				else if(supplyVoltage >= 34 && !(phid->motorErrors[0] & MOTORCONTROL_ERRORFLAG_HIGHPOWER))
				{
					phid->motorErrors[0] |= MOTORCONTROL_ERRORFLAG_HIGHPOWER;
					FIRE_ERROR(EEPHIDGET_BADPOWER, "Power supply voltage is too high. Supply voltage is %0.2lfv", supplyVoltage);
				}
				if(supplyVoltage > 7.5 && supplyVoltage < 33 
					&& (phid->motorErrors[0] & (MOTORCONTROL_ERRORFLAG_NOPOWER | MOTORCONTROL_ERRORFLAG_LOWPOWER | MOTORCONTROL_ERRORFLAG_HIGHPOWER | MOTORCONTROL_ERRORFLAG_DANGERPOWER)))
				{
					phid->motorErrors[0] &= ~(MOTORCONTROL_ERRORFLAG_NOPOWER | MOTORCONTROL_ERRORFLAG_LOWPOWER | MOTORCONTROL_ERRORFLAG_HIGHPOWER | MOTORCONTROL_ERRORFLAG_DANGERPOWER);
					FIRE_ERROR(EEPHIDGET_OK, "Power supply voltage has returned to valid range. Supply voltage is %0.2lfv", supplyVoltage);
				}

				if((phid->lastPacketCount != PUNK_INT) && ((phid->lastPacketCount+1)&0xFF) != packetCount)
				{
					FIRE_ERROR_NOQUEUE(EEPHIDGET_PACKETLOST, "One or more data packets were lost");
				}
				phid->lastPacketCount = packetCount;
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		default:
			return EPHIDGET_UNEXPECTED;

	}

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(MotorControl)

	for (i = 0; i<phid->phid.attr.motorcontrol.numInputs; i++)
	{
		if(phid->inputState[i] != PUNK_BOOL)
			FIRE(InputChange, i, phid->inputState[i]);
	}
	for (i = 0; i<phid->phid.attr.motorcontrol.numMotors; i++)
	{
		if(phid->motorSpeedEcho[i] != PUNK_DBL)
		{
			FIRE(VelocityChange, i, phid->motorSpeedEcho[i]);
			//Deprecated
			FIRE(MotorChange, i, phid->motorSpeedEcho[i]);
		}
		if(phid->motorSensedCurrent[i] != PUNK_DBL && phid->motorSensedCurrent[i] != PUNI_DBL)
			FIRE(CurrentChange, i, phid->motorSensedCurrent[i]);
	}

	//Note: don't send BackEMFUpdate, SensorUpdate, CurrentUpdate events - they come at a set rate.

	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET_BUF(MotorControl)

//sendpacket - sends a packet to the device asynchronously, blocking if the 1-packet queue is full
CSENDPACKET_BUF(MotorControl)

//makePacket - constructs a packet using current device state
CMAKEPACKETINDEXED(MotorControl)
	int velocity = 0, accel = 0, braking = 0;
	unsigned char dir = 0;

	TESTPTRS(phid, buffer);

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				//have to make sure that everything to be sent has some sort of default value if the user hasn't set a value
				if(phid->motorSpeed[Index] == PUNK_DBL)
					phid->motorSpeed[Index] = 0; //not moving
				if(phid->motorAcceleration[Index] == PUNK_DBL)
					phid->motorAcceleration[Index] = phid->accelerationMax / 2; //mid-range

				velocity = (int)round((phid->motorSpeed[Index] * 127.0) / 100.0);
				accel = (int)round(phid->motorAcceleration[Index] * 10.23);

				buffer[0] = (unsigned char)Index;
				buffer[1] = (unsigned char)(velocity & 0xff);
				buffer[2] = (unsigned char)((accel >> 8) & 0x0f);
				buffer[3] = (unsigned char)(accel & 0xff);
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_MOTORCONTROL_1MOTOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				//have to make sure that everything to be sent has some sort of default value if the user hasn't set a value
				if(phid->motorSpeed[Index] == PUNK_DBL)
					phid->motorSpeed[Index] = 0; //not moving
				if(phid->motorBraking[Index] == PUNK_DBL)
					phid->motorBraking[Index] = 0; //not braking
				if(phid->motorAcceleration[Index] == PUNK_DBL)
					phid->motorAcceleration[Index] = phid->accelerationMax / 2; //mid-range
				if(phid->motorSpeed[Index] >= 0)
				{
					velocity = (int)round((phid->motorSpeed[Index] * 255.0) / 100.0);
					dir = 1;
				}
				else
				{
					velocity = (int)round((-phid->motorSpeed[Index] * 255.0) / 100.0);
					dir = 0;
				}
				accel = (int)round((phid->motorAcceleration[Index] / phid->accelerationMax) * 255);
				braking = (int)round((phid->motorBraking[Index] * 255.0) / 100.0);
				if(phid->ratiometric == PUNK_BOOL)
					phid->ratiometric = PTRUE;
				if(phid->backEMFSensingState[0] == PUNK_BOOL)
					phid->backEMFSensingState[0] = PFALSE;

				buffer[0] = (unsigned char)(dir | (phid->ratiometric==PTRUE?0x04:0x00) | (phid->backEMFSensingState[0]==PTRUE?0x02:0x00));
				buffer[1] = (unsigned char)(velocity & 0xff);
				buffer[2] = (unsigned char)(accel & 0xff);
				buffer[3] = (unsigned char)(braking & 0xff);
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
CCREATE(MotorControl, PHIDCLASS_MOTORCONTROL)

//event setup functions
CFHANDLE(MotorControl, InputChange, int, int)
CFHANDLE(MotorControl, VelocityChange, int, double)
CFHANDLE(MotorControl, CurrentChange, int, double)
CFHANDLE(MotorControl, EncoderPositionChange, int, int, int)
CFHANDLE(MotorControl, EncoderPositionUpdate, int, int)
CFHANDLE(MotorControl, BackEMFUpdate, int, double)
CFHANDLE(MotorControl, SensorUpdate, int, int)
CFHANDLE(MotorControl, CurrentUpdate, int, double)

CGET(MotorControl,MotorCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED

	MASGN(phid.attr.motorcontrol.numMotors)
}

CGETINDEX(MotorControl,Velocity,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numMotors)
	TESTMASGN(motorSpeedEcho[Index], PUNK_DBL)

	MASGN(motorSpeedEcho[Index])
}
CSETINDEX(MotorControl,Velocity,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numMotors)
	TESTRANGE(-100, 100)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Velocity, "%lE", motorSpeed);
	else
		SENDPACKETINDEXED(MotorControl, motorSpeed[Index], Index);

	return EPHIDGET_OK;
}

CGETINDEX(MotorControl,Acceleration,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numMotors)
	TESTMASGN(motorAcceleration[Index], PUNK_DBL)

	MASGN(motorAcceleration[Index])
}
CSETINDEX(MotorControl,Acceleration,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numMotors)
	TESTRANGE(phid->accelerationMin, phid->accelerationMax)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Acceleration, "%lE", motorAcceleration);
	else
		SENDPACKETINDEXED(MotorControl, motorAcceleration[Index], Index);

	return EPHIDGET_OK;
}

CGETINDEX(MotorControl,AccelerationMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numMotors)
	TESTMASGN(accelerationMax, PUNK_DBL)

	MASGN(accelerationMax)
}

CGETINDEX(MotorControl,AccelerationMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numMotors)
	TESTMASGN(accelerationMin, PUNK_DBL)

	MASGN(accelerationMin)
}

CGETINDEX(MotorControl,Current,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED

	//Only supported on HC/1-motor
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
			return EPHIDGET_UNSUPPORTED;
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
		case PHIDID_MOTORCONTROL_1MOTOR:
			TESTINDEX(phid.attr.motorcontrol.numMotors)
			TESTMASGN(motorSensedCurrent[Index], PUNK_DBL)
			MASGN(motorSensedCurrent[Index])
		default:
			return EPHIDGET_UNEXPECTED;
	}
}

CGETINDEX(MotorControl,BackEMFSensingState,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numMotors)
	TESTMASGN(backEMFSensingStateEcho[Index], PUNK_BOOL)

	//Note: will return false on devices that don't support backEMF sensing
	MASGN(backEMFSensingStateEcho[Index])
}
CSETINDEX(MotorControl,BackEMFSensingState,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numMotors)
	TESTRANGE(PFALSE, PTRUE)

	//Only supported on 1-motor
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
			return EPHIDGET_UNSUPPORTED;
		case PHIDID_MOTORCONTROL_1MOTOR:
			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
				ADDNETWORKKEYINDEXED(BackEMFState, "%d", backEMFSensingState);
			else
				SENDPACKETINDEXED(MotorControl, backEMFSensingState[Index], Index);
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	return EPHIDGET_OK;
}

CGETINDEX(MotorControl,BackEMF,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED

	//Only supported on 1-motor
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
			return EPHIDGET_UNSUPPORTED;
		case PHIDID_MOTORCONTROL_1MOTOR:
			TESTINDEX(phid.attr.motorcontrol.numMotors)
			TESTMASGN(motorSensedBackEMF[Index], PUNK_DBL)
			MASGN(motorSensedBackEMF[Index])
		default:
			return EPHIDGET_UNEXPECTED;
	}
}

CGET(MotorControl,SupplyVoltage,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED

	//Only supported on 1-motor
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
			return EPHIDGET_UNSUPPORTED;
		case PHIDID_MOTORCONTROL_1MOTOR:
			TESTMASGN(supplyVoltage, PUNK_DBL)
			MASGN(supplyVoltage)
		default:
			return EPHIDGET_UNEXPECTED;
	}
}

CGETINDEX(MotorControl,Braking,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numMotors)
	TESTMASGN(motorBrakingEcho[Index], PUNK_DBL)

	MASGN(motorBrakingEcho[Index])
}
CSETINDEX(MotorControl,Braking,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numMotors)
	TESTRANGE(0, 100)

	//Only supported on 1-motor
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
			return EPHIDGET_UNSUPPORTED;
		case PHIDID_MOTORCONTROL_1MOTOR:
			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
				ADDNETWORKKEYINDEXED(Braking, "%lE", motorBraking);
			else
				SENDPACKETINDEXED(MotorControl, motorBraking[Index], Index);
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	return EPHIDGET_OK;
}

CGET(MotorControl,InputCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED

	MASGN(phid.attr.motorcontrol.numInputs)
}

CGETINDEX(MotorControl,InputState,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numInputs)
	TESTMASGN(inputState[Index], PUNK_BOOL)

	MASGN(inputState[Index])
}

CGET(MotorControl,EncoderCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED

	MASGN(phid.attr.motorcontrol.numEncoders)
}

CGETINDEX(MotorControl,EncoderPosition,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numEncoders)

	*pVal = (phid->encoderPositionEcho[Index] - phid->encoderPositionDelta[Index]);
	return EPHIDGET_OK;
}
CSETINDEX(MotorControl,EncoderPosition,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numEncoders)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		//newVal = phid->encoderPositionEcho[Index] + newVal;
		ADDNETWORKKEYINDEXED(ResetEncoderPosition, "%d", encoderPositionDelta);
	}
	else
		phid->encoderPositionDelta[Index] = phid->encoderPositionEcho[Index] + newVal;

	return EPHIDGET_OK;
}

CGET(MotorControl, SensorCount, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED

	MASGN(phid.attr.motorcontrol.numSensors)
}

CGETINDEX(MotorControl, SensorValue, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numSensors)
	TESTMASGN(sensorValue[Index], PUNK_INT)

	MASGN(sensorValue[Index])
}

CGETINDEX(MotorControl, SensorRawValue, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED
	TESTINDEX(phid.attr.motorcontrol.numSensors)
	TESTMASGN(sensorRawValue[Index], PUNK_INT)

	MASGN(sensorRawValue[Index])
}
CGET(MotorControl, Ratiometric, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED

	//Only supported on 1-motor
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
			return EPHIDGET_UNSUPPORTED;
		case PHIDID_MOTORCONTROL_1MOTOR:
			TESTMASGN(ratiometricEcho, PUNK_BOOL)
			MASGN(ratiometricEcho)
		default:
			return EPHIDGET_UNEXPECTED;
	}
}
CSET(MotorControl, Ratiometric, int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_MOTORCONTROL)
	TESTATTACHED

	//Only supported on 1-motor
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT:
		case PHIDID_MOTORCONTROL_HC_2MOTOR:
			return EPHIDGET_UNSUPPORTED;
		case PHIDID_MOTORCONTROL_1MOTOR:
			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
				ADDNETWORKKEY(Ratiometric, "%d", ratiometric);
			else
				SENDPACKETINDEXED(MotorControl, ratiometric, 0);
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	return EPHIDGET_OK;
}

// === Deprecated Functions === //

CFHANDLE(MotorControl, MotorChange, int, double)
CGETINDEX(MotorControl,MotorSpeed,double)
	return CPhidgetMotorControl_getVelocity(phid, Index, pVal);
}
CSETINDEX(MotorControl,MotorSpeed,double)
	return CPhidgetMotorControl_setVelocity(phid, Index, newVal);
}
CGET(MotorControl,NumMotors,int)
	return CPhidgetMotorControl_getMotorCount(phid, pVal);
}
CGET(MotorControl,NumInputs,int)
	return CPhidgetMotorControl_getInputCount(phid, pVal);
}
