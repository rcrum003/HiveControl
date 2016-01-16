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
#include "csocket.h"
#include "csocketevents.h"
#include "utils/utils.h"
#include "math.h"

#include "cphidget.h"
#include "cphidgetdictionary.h"
#include "csocket.h"
#include "clog.h"
#include "cphidgetaccelerometer.h"
#include "cphidgetadvancedservo.h"
#include "cphidgetanalog.h"
#include "cphidgetbridge.h"
#include "cphidgetencoder.h"
#include "cphidgetfrequencycounter.h"
#include "cphidgetgps.h"
#include "cphidgetinterfacekit.h"
#include "cphidgetir.h"
#include "cphidgetmanager.h"
#include "cphidgetled.h"
#include "cphidgetmotorcontrol.h"
#include "cphidgetphsensor.h"
#include "cphidgetrfid.h"
#include "cphidgetservo.h"
#include "cphidgetspatial.h"
#include "cphidgetstepper.h"
#include "cphidgettemperaturesensor.h"
#include "cphidgettextlcd.h"
#include "cphidgettextled.h"
#include "cphidgetweightsensor.h"
#include "cphidgetgeneric.h"

regex_t phidgetsetex;
regex_t managerex;
regex_t managervalex;

PWC_SETKEYS(Accelerometer)
	if(KEYNAME("NumberOfAxes"))
	{
		GET_INT_VAL;
		phid->phid.attr.accelerometer.numAxis = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("Acceleration"))
	{
		if(CHKINDEX(accelerometer.numAxis,ACCEL_MAXAXES))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(axis[index], PUNI_DBL)
			phid->axis[index] = value;
			if(value != PUNK_DBL)
				FIRE(AccelerationChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("AccelerationMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(accelerationMin, PUNI_DBL)
		phid->accelerationMin = value;
	}
	else if(KEYNAME("AccelerationMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(accelerationMax, PUNI_DBL)
		phid->accelerationMax = value;
	}
	else if(KEYNAME("Trigger"))
	{
		if(CHKINDEX(accelerometer.numAxis,ACCEL_MAXAXES))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(axisChangeTrigger[index], PUNI_DBL)
			phid->axisChangeTrigger[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(Accelerometer);
	}
	return ret;
}

PWC_SETKEYS(AdvancedServo)
	if(KEYNAME("NumberOfMotors"))
	{
		GET_INT_VAL;
		phid->phid.attr.advancedservo.numMotors = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("Position"))
	{
		if(index < phid->phid.attr.advancedservo.numMotors?phid->phid.attr.advancedservo.numMotors:ADVSERVO_MAXSERVOS)
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorPositionEcho[index], PUNI_DBL)
			phid->motorPositionEcho[index] = value;
			if(value != PUNK_DBL)
				FIRE(PositionChange, index, servo_us_to_degrees(phid->servoParams[index], value, PTRUE));
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("PositionMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(motorPositionMin[index], PUNI_DBL)
		phid->motorPositionMin[index] = value;
	}
	else if(KEYNAME("PositionMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(motorPositionMax[index], PUNI_DBL)
		phid->motorPositionMax[index] = value;
	}
	else if(KEYNAME("PositionMinLimit"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(motorPositionMinLimit, PUNI_DBL)
		phid->motorPositionMinLimit = value;
	}
	else if(KEYNAME("PositionMaxLimit"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(motorPositionMaxLimit, PUNI_DBL)
		phid->motorPositionMaxLimit = value;
	}
	// initial acceleration limit always unknown so don't keyCount++
	else if(KEYNAME("Acceleration"))
	{
		if(CHKINDEX(advancedservo.numMotors, ADVSERVO_MAXSERVOS))
		{
			GET_DOUBLE_VAL;
			phid->motorAcceleration[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("AccelerationMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(accelerationMax, PUNI_DBL)
		phid->accelerationMax = value;
	}
	else if(KEYNAME("AccelerationMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(accelerationMin, PUNI_DBL)
		phid->accelerationMin = value;
	}
	else if(KEYNAME("Current"))
	{
		if(CHKINDEX(advancedservo.numMotors, ADVSERVO_MAXSERVOS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorSensedCurrent[index], PUNI_DBL)
			phid->motorSensedCurrent[index] = value;
			if(value != PUNK_DBL)
				FIRE(CurrentChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	// initial velocity limit always unknown so don't keyCount++
	else if(KEYNAME("VelocityLimit"))
	{
		if(CHKINDEX(advancedservo.numMotors, ADVSERVO_MAXSERVOS))
		{
			GET_DOUBLE_VAL;
			phid->motorVelocity[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Velocity"))
	{
		if(CHKINDEX(advancedservo.numMotors, ADVSERVO_MAXSERVOS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorVelocityEcho[index], PUNI_DBL)
			phid->motorVelocityEcho[index] = value;
			if(value != PUNK_DBL)
				FIRE(VelocityChange, index, servo_us_to_degrees_vel(phid->servoParams[index], value, PTRUE));
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("VelocityMax"))
	{
		if(CHKINDEX(advancedservo.numMotors, ADVSERVO_MAXSERVOS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(velocityMax[index], PUNI_DBL)
			phid->velocityMax[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("VelocityMaxLimit"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(velocityMaxLimit, PUNI_DBL)
		phid->velocityMaxLimit = value;
	}
	else if(KEYNAME("VelocityMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(velocityMin, PUNI_DBL)
		phid->velocityMin = value;
	}
	else if(KEYNAME("Engaged"))
	{
		if(CHKINDEX(advancedservo.numMotors, ADVSERVO_MAXSERVOS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(motorEngagedStateEcho[index], PUNI_BOOL)
			phid->motorEngagedStateEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("SpeedRampingOn"))
	{
		if(CHKINDEX(advancedservo.numMotors, ADVSERVO_MAXSERVOS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(motorSpeedRampingStateEcho[index], PUNI_BOOL)
			phid->motorSpeedRampingStateEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Stopped"))
	{
		if(CHKINDEX(advancedservo.numMotors, ADVSERVO_MAXSERVOS))
		{
			GET_INT_VAL;
			unsigned char lastStoppedState = phid->motorStoppedState[index];
			INC_KEYCOUNT(motorStoppedState[index], PUNI_BOOL)
			phid->motorStoppedState[index] = value;
			//If changed, re-run position/velocity events, so the stopped change will be noticed
			if(lastStoppedState != value)
			{
				if(phid->motorVelocityEcho[index] != PUNK_DBL)
					FIRE(VelocityChange, index, servo_us_to_degrees_vel(phid->servoParams[index], phid->motorVelocityEcho[index], PTRUE));
				if(phid->motorPositionEcho[index] != PUNK_DBL)
					FIRE(PositionChange, index, servo_us_to_degrees(phid->servoParams[index], phid->motorPositionEcho[index], PTRUE));
			}
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("ServoParameters"))
	{
		if(CHKINDEX(advancedservo.numMotors, ADVSERVO_MAXSERVOS))
		{
			CPhidgetServoParameters params;
			char *endptr;
			params.servoType = strtol(state, &endptr, 10);
			params.min_us = strtod(endptr+1, &endptr);
			params.max_us = strtod(endptr+1, &endptr);
			params.us_per_degree = strtod(endptr+1, &endptr);
			params.max_us_per_s = strtod(endptr+1, NULL);
			params.state = PTRUE;

			INC_KEYCOUNT(servoParams[index].state, PUNI_BOOL)

			phid->servoParams[index] = params;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(AdvancedServo);
	}
	return ret;
}

PWC_SETKEYS(Analog)
	if(KEYNAME("NumberOfOutputs"))
	{
		GET_INT_VAL;
		phid->phid.attr.analog.numAnalogOutputs = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("Voltage"))
	{
		if(CHKINDEX(analog.numAnalogOutputs, ANALOG_MAXOUTPUTS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(voltageEcho[index], PUNI_DBL)
			phid->voltageEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("VoltageMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(voltageMin, PUNI_DBL)
		phid->voltageMin = value;
	}
	else if(KEYNAME("VoltageMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(voltageMax, PUNI_DBL)
		phid->voltageMax = value;
	}
	else if(KEYNAME("Enabled"))
	{
		if(CHKINDEX(analog.numAnalogOutputs, ANALOG_MAXOUTPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(enabledEcho[index], PUNI_BOOL)
			phid->enabledEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(Analog);
	}
	return ret;
}

PWC_SETKEYS(Bridge)
	if(KEYNAME("NumberOfInputs"))
	{
		GET_INT_VAL;
		phid->phid.attr.bridge.numBridgeInputs = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("DataRate"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(dataRateEcho, PUNI_INT)
		phid->dataRateEcho = value;
	}
	else if(KEYNAME("DataRateMin"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(dataRateMin, PUNI_INT)
		phid->dataRateMin = value;
	}
	else if(KEYNAME("DataRateMax"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(dataRateMax, PUNI_INT)
		phid->dataRateMax = value;
	}
	else if(KEYNAME("Enabled"))
	{
		if(CHKINDEX(bridge.numBridgeInputs, BRIDGE_MAXINPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(enabledEcho[index], PUNI_BOOL)
			phid->enabledEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("BridgeMax"))
	{
		if(CHKINDEX(bridge.numBridgeInputs, BRIDGE_MAXINPUTS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(bridgeMax[index], PUNI_DBL)
			phid->bridgeMax[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("BridgeMin"))
	{
		if(CHKINDEX(bridge.numBridgeInputs, BRIDGE_MAXINPUTS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(bridgeMin[index], PUNI_DBL)
			phid->bridgeMin[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Gain"))
	{
		if(CHKINDEX(bridge.numBridgeInputs, BRIDGE_MAXINPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(gainEcho[index], PHIDGET_BRIDGE_GAIN_UNKNOWN)
			phid->gainEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("BridgeValue"))
	{
		if(CHKINDEX(bridge.numBridgeInputs, BRIDGE_MAXINPUTS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(bridgeValue[index], PUNI_DBL)
			phid->bridgeValue[index] = value;
			if(value != PUNK_DBL)
				FIRE(BridgeData, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(Bridge);
	}
	return ret;
}

PWC_SETKEYS(Encoder)
	if(KEYNAME("NumberOfEncoders"))
	{
		GET_INT_VAL;
		phid->phid.attr.encoder.numEncoders = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("NumberOfInputs"))
	{
		GET_INT_VAL;
		phid->phid.attr.encoder.numInputs = value;
		phid->phid.keyCount++;
	}
	// initial Input is unknown so don't keyCount++
	else if(KEYNAME("Input"))
	{
		if(CHKINDEX(encoder.numInputs, ENCODER_MAXINPUTS))
		{
			GET_INT_VAL;
			phid->inputState[index] = value;
			if(value != PUNK_BOOL)
				FIRE(InputChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	// initial Position is unknown so don't keyCount++
	else if(KEYNAME("ResetPosition"))
	{
		if(CHKINDEX(encoder.numEncoders, ENCODER_MAXENCODERS))
		{
			GET_INT_VAL;
			phid->encoderPosition[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Position"))
	{
		if(CHKINDEX(encoder.numEncoders, ENCODER_MAXENCODERS))
		{
			char *endPtr;
			int value = strtol(state, &endPtr, 10);
			int posnchange = strtol(endPtr+1, &endPtr, 10);
			int posn = strtol(endPtr+1, &endPtr, 10);
			
			phid->encoderPosition[index] = posn;
			
			FIRE(PositionChange, index, (unsigned short)value, posnchange);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("IndexPosition"))
	{
		if(CHKINDEX(encoder.numEncoders, ENCODER_MAXENCODERS))
		{
			GET_INT_VAL;
			phid->indexPosition[index] = value;
			
			FIRE(Index, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Enabled"))
	{
		if(CHKINDEX(encoder.numEncoders, ENCODER_MAXENCODERS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(enableStateEcho[index], PUNI_BOOL)
			phid->enableStateEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(Encoder);
	}
	return ret;
}

PWC_SETKEYS(FrequencyCounter)
	if(KEYNAME("NumberOfInputs"))
	{
		GET_INT_VAL;
		phid->phid.attr.frequencycounter.numFreqInputs = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("Count") || KEYNAME("CountReset"))
	{
		if(CHKINDEX(frequencycounter.numFreqInputs, FREQCOUNTER_MAXINPUTS))
		{
			char *endPtr;
			__int64 totTime = strtoll(state, &endPtr, 10);
			__int64 totCount = strtoll(endPtr+1, &endPtr, 10);
			double freq = strtod(endPtr+1, NULL);

			int timeChange = (int)(totTime - phid->totalTime[index]);
			int cntChange = (int)(totCount - phid->totalCount[index]);

			INC_KEYCOUNT(frequency[index], PUNI_INT);

			phid->totalTime[index] = totTime;
			phid->totalCount[index] = totCount;
			phid->frequency[index] = freq;

			//no event on first time or reset
			if(phid->countsGood[index] == PTRUE && KEYNAME("Count"))
			{
				FIRE(Count, index, timeChange, cntChange);
			}

			phid->countsGood[index] = PTRUE;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Enabled"))
	{
		if(CHKINDEX(frequencycounter.numFreqInputs, FREQCOUNTER_MAXINPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(enabledEcho[index], PUNI_BOOL)
			phid->enabledEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Timeout"))
	{
		if(CHKINDEX(frequencycounter.numFreqInputs, FREQCOUNTER_MAXINPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(timeout[index], PUNI_INT)
			phid->timeout[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Filter"))
	{
		if(CHKINDEX(frequencycounter.numFreqInputs, FREQCOUNTER_MAXINPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(filterEcho[index], PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_UNKNOWN)
			phid->filterEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(FrequencyCounter);
	}
	return ret;
}

PWC_SETKEYS(Generic)
	phid = 0;
	return ret;
}

PWC_SETKEYS(GPS)
	if(KEYNAME("Position"))
	{
		char *endPtr;
		double lat = strtod(state, &endPtr);
		double lon = strtod(endPtr+1, &endPtr);
		double alt = strtod(endPtr+1, NULL);
		phid->latitude = lat;
		phid->longitude = lon;
		phid->altitude = alt;
		FIRE(PositionChange, lat, lon, alt);
	}
	else if(KEYNAME("PositionFix"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(fix, PUNI_BOOL)
		phid->fix = value;
		FIRE(PositionFixStatusChange, value);
	}
	else if(KEYNAME("Velocity"))
	{
		GET_DOUBLE_VAL;
		phid->velocity = value;
	}
	else if(KEYNAME("Heading"))
	{
		GET_DOUBLE_VAL;
		phid->heading = value;
	}
	else if(KEYNAME("DateTime"))
	{
		GPSTime time;
		GPSDate date;
		char *endPtr;
		date.tm_year = (short)strtol(state, &endPtr, 10);
		date.tm_mon = (short)strtol(endPtr+1, &endPtr, 10);
		date.tm_mday = (short)strtol(endPtr+1, &endPtr, 10);
		time.tm_hour = (short)strtol(endPtr+1, &endPtr, 10);
		time.tm_min = (short)strtol(endPtr+1, &endPtr, 10);
		time.tm_sec = (short)strtol(endPtr+1, &endPtr, 10);
		time.tm_ms = (short)strtol(endPtr+1, NULL, 10);

		phid->GPSData.GGA.time = time;
		phid->haveTime = PTRUE;
		phid->GPSData.RMC.date = date;
		phid->haveDate = PTRUE;
	}
	else{
		PWC_BAD_SETTYPE(GPS);
	}
	return ret;
}

PWC_SETKEYS(InterfaceKit)
	if(KEYNAME("NumberOfSensors"))
	{
		GET_INT_VAL;
		phid->phid.attr.ifkit.numSensors = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("NumberOfInputs"))
	{
		GET_INT_VAL;
		phid->phid.attr.ifkit.numInputs = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("NumberOfOutputs"))
	{
		GET_INT_VAL;
		phid->phid.attr.ifkit.numOutputs = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("Input"))
	{
		if(CHKINDEX(ifkit.numInputs, IFKIT_MAXINPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(physicalState[index], PUNI_BOOL)
			phid->physicalState[index] = value;
			if(value != PUNK_BOOL)
				FIRE(InputChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Sensor"))
	{
		if(CHKINDEX(ifkit.numSensors, IFKIT_MAXSENSORS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(sensorValue[index], PUNI_INT)
			phid->sensorValue[index] = value;
			if(value != PUNK_INT)
				FIRE(SensorChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("RawSensor"))
	{
		if(CHKINDEX(ifkit.numSensors, IFKIT_MAXSENSORS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(sensorRawValue[index], PUNI_INT)
			phid->sensorRawValue[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Output"))
	{
		if(CHKINDEX(ifkit.numOutputs, IFKIT_MAXOUTPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(outputEchoStates[index], PUNI_BOOL)
			phid->outputEchoStates[index] = value;
			if(value != PUNK_BOOL)
				FIRE(OutputChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Trigger"))
	{
		if(CHKINDEX(ifkit.numSensors, IFKIT_MAXSENSORS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(sensorChangeTrigger[index], PUNI_INT)
			phid->sensorChangeTrigger[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("DataRate"))
	{
		if(CHKINDEX(ifkit.numSensors, IFKIT_MAXSENSORS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(dataRate[index], PUNI_INT)
			phid->dataRate[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("DataRateMin"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(dataRateMin, PUNI_INT)
		phid->dataRateMin = value;
	}
	else if(KEYNAME("DataRateMax"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(dataRateMax, PUNI_INT)
		phid->dataRateMax = value;
	}
	else if(KEYNAME("InterruptRate"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(interruptRate, PUNI_INT)
		phid->interruptRate = value;
		phid->maxDataPerPacket = phid->phid.attr.ifkit.numSensors?phid->phid.attr.ifkit.numSensors:IFKIT_MAXSENSORS;
	}
	else if(KEYNAME("Ratiometric"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(ratiometric, PUNI_BOOL)
		phid->ratiometric = value;
		phid->ratiometricEcho = value;
	}
	else{
		PWC_BAD_SETTYPE(InterfaceKit);
	}
	return ret;
}

PWC_SETKEYS(IR)
	if(KEYNAME("Code"))
	{
		unsigned char data[IR_MAX_CODE_DATA_LENGTH];
		int bitCount, repeat, length = IR_MAX_CODE_DATA_LENGTH;
		char *endPtr;

		//this will stop at the first ','
		stringToByteArray((char *)state, data, &length);
		bitCount = strtol(state+length*2+1, &endPtr, 10);
		repeat = strtol(endPtr+1, &endPtr, 10);

		//send out the code event!
		FIRE(Code, data, length, bitCount, repeat);

		//store to last code
		ZEROMEM(phid->lastCode, sizeof(phid->lastCode));
		memcpy(phid->lastCode, data, length);
		phid->lastCodeInfo.bitCount = bitCount;
		phid->lastRepeat = repeat;
		phid->lastCodeKnown = PTRUE;
	}
	else if(KEYNAME("Learn"))
	{
		unsigned char data[IR_MAX_CODE_DATA_LENGTH];
		CPhidgetIR_CodeInfo codeInfo;
		int length=IR_MAX_CODE_DATA_LENGTH;

		stringToCodeInfo((char *)state, &codeInfo);
		stringToByteArray((char *)(state+sizeof(CPhidgetIR_CodeInfo)*2), data, &length);

		//send the event
		FIRE(Learn, data, length, &codeInfo);

		//store to last code
		ZEROMEM(phid->lastLearnedCode, sizeof(phid->lastLearnedCode));
		memcpy(phid->lastLearnedCode, data, length);
		phid->lastLearnedCodeInfo = codeInfo;
		phid->lastLearnedCodeKnown = PTRUE;
	}
	else if(KEYNAME("RawData"))
	{
		//TODO: 
		//what about multiple clients?
		// -if a faster client is getting all the data and acking it, then the slower client will end up missing chunks...
		// -we are assuming that data comes in in the same order as it is sent, so we don't deal with out of order keys

		//only respond to new raw data
		if(reason != PDR_CURRENT_VALUE)
		{
			int i;
			int data[IR_MAX_CODE_DATA_LENGTH];
			char key[1024], val[1024];
			int rawDataSendCnt, length = IR_MAX_CODE_DATA_LENGTH;
			char *endPtr;

			//this will stop at the first ','
			stringToWordArray((char *)state, data, &length);
			rawDataSendCnt = strtol(state+length*5+1, &endPtr, 10);

			//send an ACK for this count
			CThread_mutex_lock(&phid->phid.lock);
			snprintf(key, sizeof(key), "/PCK/%s/%d/RawDataAck/%d", phid->phid.deviceType, phid->phid.serialNumber, index);
			snprintf(val, sizeof(val), "%d", rawDataSendCnt);
			pdc_async_set(phid->phid.networkInfo->server->pdcs, key, val, (int)strlen(val), PFALSE, internal_async_network_error_handler, &phid->phid);
			CThread_mutex_unlock(&phid->phid.lock);

			//see if we lost a packet
			if(phid->rawDataSendWSCounter != PUNK_INT && phid->rawDataSendWSCounter + 1 != rawDataSendCnt)
			{
				char error_buffer[127];
				FIRE_ERROR_NOQUEUE(EEPHIDGET_PACKETLOST, "A piece on PhidgetIR Raw Data was lost. Be careful if decoding RawData manually.");

				//reset data pointers
				phid->dataWritePtr = 0;
				phid->userReadPtr = 0;
			}
			phid->rawDataSendWSCounter = rawDataSendCnt;

			//send the event
			FIRE(RawData, data, length);

			//TODO: store the raw data array
			for(i=0;i<length;i++)
			{
				phid->dataBuffer[phid->dataWritePtr] = data[i];

				phid->dataWritePtr++;
				phid->dataWritePtr &= IR_DATA_ARRAY_MASK;
				//if we run into data that hasn't been read... too bad, we overwrite it and adjust the read pointer
				if(phid->dataWritePtr == phid->userReadPtr)
				{
					phid->userReadPtr++;
					phid->userReadPtr &= IR_DATA_ARRAY_MASK;
				}
			}

		}
	}
	else{
		PWC_BAD_SETTYPE(IR);
	}
	return ret;
}

PWC_SETKEYS(LED)
	if(KEYNAME("NumberOfLEDs"))
	{
		GET_INT_VAL;
		phid->phid.attr.led.numLEDs = value;
		phid->phid.keyCount++;
	}
	// initial brightness is unknown so don't keyCount++
	else if(KEYNAME("Brightness"))
	{
		if(CHKINDEX(led.numLEDs, LED_MAXLEDS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(LED_Power[index], PUNI_DBL)
			phid->LED_Power[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Voltage"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(voltageEcho, -1)
		phid->voltageEcho = value;
	}
	else if(KEYNAME("CurrentLimit"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(currentLimitEcho, -1)
		phid->currentLimitEcho = value;
	}
	else if(KEYNAME("CurrentLimitIndexed"))
	{
		if(CHKINDEX(led.numLEDs, LED_MAXLEDS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(LED_CurrentLimit[index], PUNI_DBL)
			phid->LED_CurrentLimit[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(LED);
	}
	return ret;
}

PWC_SETKEYS(MotorControl)
	if(KEYNAME("NumberOfMotors"))
	{
		GET_INT_VAL;
		phid->phid.attr.motorcontrol.numMotors = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("NumberOfInputs"))
	{
		GET_INT_VAL;
		phid->phid.attr.motorcontrol.numInputs = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("NumberOfEncoders"))
	{
		GET_INT_VAL;
		phid->phid.attr.motorcontrol.numEncoders = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("NumberOfSensors"))
	{
		GET_INT_VAL;
		phid->phid.attr.motorcontrol.numSensors = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("Input"))
	{
		if(CHKINDEX(motorcontrol.numInputs, MOTORCONTROL_MAXINPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(inputState[index], PUNI_BOOL)
			phid->inputState[index] = value;
			if(value != PUNK_BOOL)
				FIRE(InputChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Sensor"))
	{
		if(CHKINDEX(motorcontrol.numSensors, MOTORCONTROL_MAXSENSORS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(sensorValue[index], PUNI_INT)
			phid->sensorValue[index] = value;
			if(value != PUNK_INT)
				FIRE(SensorUpdate, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("RawSensor"))
	{
		if(CHKINDEX(motorcontrol.numSensors, MOTORCONTROL_MAXSENSORS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(sensorRawValue[index], PUNI_INT)
			phid->sensorRawValue[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Ratiometric"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(ratiometricEcho, PUNI_BOOL)
		phid->ratiometricEcho = value;
	}
	else if(KEYNAME("Braking"))
	{
		if(CHKINDEX(motorcontrol.numMotors, MOTORCONTROL_MAXMOTORS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorBrakingEcho[index], PUNI_DBL)
			phid->motorBrakingEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("BackEMF"))
	{
		if(CHKINDEX(motorcontrol.numMotors, MOTORCONTROL_MAXMOTORS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorSensedBackEMF[index], PUNI_DBL)
			phid->motorSensedBackEMF[index] = value;
			if(value != PUNK_DBL)
				FIRE(BackEMFUpdate, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("BackEMFState"))
	{
		if(CHKINDEX(motorcontrol.numMotors, MOTORCONTROL_MAXMOTORS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(backEMFSensingStateEcho[index], PUNI_BOOL)
			phid->backEMFSensingStateEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("SupplyVoltage"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(supplyVoltage, PUNI_DBL)
		phid->supplyVoltage = value;
	}
	else if(KEYNAME("Velocity"))
	{
		if(CHKINDEX(motorcontrol.numMotors, MOTORCONTROL_MAXMOTORS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorSpeedEcho[index], PUNI_DBL)
			phid->motorSpeedEcho[index] = value;
			if(value != PUNK_DBL)
				FIRE(VelocityChange, index, value);
			//Deprecated
			if(value != PUNK_DBL)
				FIRE(MotorChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Current"))
	{
		if(CHKINDEX(motorcontrol.numMotors, MOTORCONTROL_MAXMOTORS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorSensedCurrent[index], PUNI_DBL)
			phid->motorSensedCurrent[index] = value;
			if(value != PUNK_DBL)
				FIRE(CurrentChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("CurrentUpdate"))
	{
		if(CHKINDEX(motorcontrol.numMotors, MOTORCONTROL_MAXMOTORS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorSensedCurrent[index], PUNI_DBL)
			phid->motorSensedCurrent[index] = value;
			if(value != PUNK_DBL)
				FIRE(CurrentUpdate, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	// initial acceleration is unknown so don't keyCount++
	else if(KEYNAME("Acceleration"))
	{
		if(CHKINDEX(motorcontrol.numMotors, MOTORCONTROL_MAXMOTORS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorAccelerationEcho[index], PUNI_DBL)
			phid->motorAccelerationEcho[index] = value;
			phid->motorAcceleration[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("AccelerationMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(accelerationMin, PUNI_DBL)
		phid->accelerationMin = value;
	}
	else if(KEYNAME("AccelerationMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(accelerationMax, PUNI_DBL)
		phid->accelerationMax = value;
	}
	// initial Position is unknown so don't keyCount++
	else if(KEYNAME("ResetEncoderPosition"))
	{
		if(CHKINDEX(motorcontrol.numEncoders, MOTORCONTROL_MAXENCODERS))
		{
			GET_INT_VAL;
			phid->encoderPositionDelta[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("EncoderPosition"))
	{
		if(CHKINDEX(motorcontrol.numEncoders, MOTORCONTROL_MAXENCODERS))
		{
			char *endPtr;
			int time = strtol(state, &endPtr, 10);
			int posn = strtol(endPtr+1, NULL, 10);
			int posnChange = posn - phid->encoderPositionEcho[index];
			int encoderTimeChange = time - phid->encoderTimeStamp[index];

			//timeout is 20 seconds
			if (encoderTimeChange > 60000)
				encoderTimeChange = PUNK_INT;

			phid->encoderPositionEcho[index] = posn;
			phid->encoderTimeStamp[index] = time;
			
			FIRE(EncoderPositionChange, index, encoderTimeChange, posnChange);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("EncoderPositionUpdate"))
	{
		if(CHKINDEX(motorcontrol.numEncoders, MOTORCONTROL_MAXENCODERS))
		{
			int posn = strtol(state, NULL, 10);
			int posnChange = posn - phid->encoderPositionUpdates[index];

			phid->encoderPositionUpdates[index] = posn;
			
			FIRE(EncoderPositionUpdate, index, posnChange);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(MotorControl);
	}
	return ret;
}

PWC_SETKEYS(PHSensor)
	if(KEYNAME("PH"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(PH, PUNI_DBL)
		phid->PH = value;
		if(value != PUNK_DBL)
			FIRE(PHChange, value);
	}
	else if(KEYNAME("PHMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(phMin, PUNI_DBL)
		phid->phMin = value;
	}
	else if(KEYNAME("PHMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(phMax, PUNI_DBL)
		phid->phMax = value;
	}
	else if(KEYNAME("Trigger"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(PHChangeTrigger, PUNI_DBL)
		phid->PHChangeTrigger = value;
	}
	else if(KEYNAME("Potential"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(Potential, PUNI_DBL)
		phid->Potential = value;
	}
	else if(KEYNAME("PotentialMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(potentialMin, PUNI_DBL)
		phid->potentialMin = value;
	}
	else if(KEYNAME("PotentialMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(potentialMax, PUNI_DBL)
		phid->potentialMax = value;
	}
	else{
		PWC_BAD_SETTYPE(PHSensor);
	}
	return ret;
}

PWC_SETKEYS(RFID)
	if(KEYNAME("NumberOfOutputs"))
	{
		GET_INT_VAL;
		phid->phid.attr.rfid.numOutputs = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("LastTag"))
	{
		char *endPtr;
		CPhidgetRFID_Tag tag = {0};
		tag.protocol = (CPhidgetRFID_Protocol)strtol(state, &endPtr, 10);
		strncpy(tag.tagString, endPtr+1, 25);

		INC_KEYCOUNT(lastTagValid, PUNI_BOOL)
		if(tag.protocol != 0)
			phid->lastTagValid = PTRUE;
		else
			phid->lastTagValid = PFALSE;
		
		if(tag.protocol == PHIDGET_RFID_PROTOCOL_EM4100)
		{
			tag.tagData[0] = (hexval(tag.tagString[0])<<4)|hexval(tag.tagString[1]);
			tag.tagData[1] = (hexval(tag.tagString[2])<<4)|hexval(tag.tagString[3]);
			tag.tagData[2] = (hexval(tag.tagString[4])<<4)|hexval(tag.tagString[5]);
			tag.tagData[3] = (hexval(tag.tagString[6])<<4)|hexval(tag.tagString[7]);
			tag.tagData[4] = (hexval(tag.tagString[8])<<4)|hexval(tag.tagString[9]);
		}
		
		phid->lastTag = tag;
	}
	else if(KEYNAME("Tag2"))
	{
		char *endPtr;
		CPhidgetRFID_Tag tag = {0};
		tag.protocol = (CPhidgetRFID_Protocol)strtol(state, &endPtr, 10);
		strncpy(tag.tagString, endPtr+1, 25);

		INC_KEYCOUNT(tagPresent, PUNI_BOOL)
		phid->tagPresent = PTRUE;
		
		INC_KEYCOUNT(lastTagValid, PUNI_BOOL)
		phid->lastTagValid = PTRUE;

		if(tag.protocol == PHIDGET_RFID_PROTOCOL_EM4100)
		{
			tag.tagData[0] = (hexval(tag.tagString[0])<<4)|hexval(tag.tagString[1]);
			tag.tagData[1] = (hexval(tag.tagString[2])<<4)|hexval(tag.tagString[3]);
			tag.tagData[2] = (hexval(tag.tagString[4])<<4)|hexval(tag.tagString[5]);
			tag.tagData[3] = (hexval(tag.tagString[6])<<4)|hexval(tag.tagString[7]);
			tag.tagData[4] = (hexval(tag.tagString[8])<<4)|hexval(tag.tagString[9]);
			FIRE(Tag, tag.tagData);
		}

		FIRE(Tag2, tag.tagString, tag.protocol);
		
		phid->lastTag = tag;
	}
	else if(KEYNAME("TagLoss2"))
	{
		INC_KEYCOUNT(tagPresent, PUNI_BOOL)
		phid->tagPresent = PFALSE;

		if(phid->lastTag.protocol == PHIDGET_RFID_PROTOCOL_EM4100)
			FIRE(TagLost, phid->lastTag.tagData);
		FIRE(TagLost2, phid->lastTag.tagString, phid->lastTag.protocol);
	}
	else if(KEYNAME("TagState"))
	{			
		GET_INT_VAL;

		INC_KEYCOUNT(tagPresent, PUNI_BOOL)
		phid->tagPresent = value;
	}
	else if(KEYNAME("Output"))
	{
		if(CHKINDEX(rfid.numOutputs, RFID_MAXOUTPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(outputEchoState[index], PUNI_BOOL)
			phid->outputEchoState[index] = value;
			if(value != PUNK_BOOL)
				FIRE(OutputChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("AntennaOn"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(antennaEchoState, PUNI_BOOL)
		phid->antennaEchoState = value;
	}
	else if(KEYNAME("LEDOn"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(ledEchoState, PUNI_BOOL)
		phid->ledEchoState = value;
	}
	else{
		PWC_BAD_SETTYPE(RFID);
	}
	return ret;
}

PWC_SETKEYS(Servo)
	if(KEYNAME("NumberOfMotors"))
	{
		GET_INT_VAL;
		phid->phid.attr.led.numLEDs = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("Position"))
	{
		if(CHKINDEX(servo.numMotors, SERVO_MAXSERVOS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorPositionEcho[index], PUNI_DBL)
			phid->motorPositionEcho[index]= value;
			if(value != PUNK_DBL)
				FIRE(PositionChange, index, servo_us_to_degrees(phid->servoParams[index], value, PTRUE));
			//Deprecated
			if(value != PUNK_DBL)
				FIRE(MotorPositionChange, index, servo_us_to_degrees(phid->servoParams[index], value, PTRUE));
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Engaged"))
	{
		if(CHKINDEX(servo.numMotors, SERVO_MAXSERVOS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(motorEngagedStateEcho[index], PUNI_BOOL)
			phid->motorEngagedStateEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("PositionMinLimit"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(motorPositionMinLimit, PUNI_DBL)
		phid->motorPositionMinLimit = value;
	}
	else if(KEYNAME("PositionMaxLimit"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(motorPositionMaxLimit, PUNI_DBL)
		phid->motorPositionMaxLimit = value;
	}
	else if(KEYNAME("ServoParameters"))
	{
		if(CHKINDEX(servo.numMotors, SERVO_MAXSERVOS))
		{
			CPhidgetServoParameters params;
			char *endptr;
			params.servoType = strtol(state, &endptr, 10);
			params.min_us = strtod(endptr+1, &endptr);
			params.max_us = strtod(endptr+1, &endptr);
			params.us_per_degree = strtod(endptr+1, NULL);
			params.state = PTRUE;

			INC_KEYCOUNT(servoParams[index].state, PUNI_BOOL)

			phid->servoParams[index] = params;

			//Set the max/min
			//make sure we don't set max higher then the limit
			if(params.max_us > phid->motorPositionMaxLimit)
				phid->motorPositionMax[index] = phid->motorPositionMaxLimit;
			else
				phid->motorPositionMax[index] = params.max_us;

			phid->motorPositionMin[index] = params.min_us;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(Servo);
	}
	return ret;
}

PWC_SETKEYS(Spatial)
	if(KEYNAME("AccelerationAxisCount"))
	{
		GET_INT_VAL;
		phid->phid.attr.spatial.numAccelAxes = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("GyroAxisCount"))
	{
		GET_INT_VAL;
		phid->phid.attr.spatial.numGyroAxes = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("CompassAxisCount"))
	{
		GET_INT_VAL;
		phid->phid.attr.spatial.numCompassAxes = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("DataRate"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(dataRate, PUNI_INT)
		phid->dataRate = value;
	}
	else if(KEYNAME("DataRateMin"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(dataRateMin, PUNI_INT)
		phid->dataRateMin = value;
	}
	else if(KEYNAME("DataRateMax"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(dataRateMax, PUNI_INT)
		phid->dataRateMax = value;
	}
	else if(KEYNAME("InterruptRate"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(interruptRate, PUNI_INT)
		phid->interruptRate = value;
	}
	else if(KEYNAME("AccelerationMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(accelerationMin, PUNI_DBL)
		phid->accelerationMin = value;
	}
	else if(KEYNAME("AccelerationMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(accelerationMax, PUNI_DBL)
		phid->accelerationMax = value;
	}
	else if(KEYNAME("AngularRateMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(angularRateMin, PUNI_DBL)
		phid->angularRateMin = value;
	}
	else if(KEYNAME("AngularRateMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(angularRateMax, PUNI_DBL)
		phid->angularRateMax = value;
	}
	else if(KEYNAME("MagneticFieldMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(magneticFieldMin, PUNI_DBL)
		phid->magneticFieldMin = value;
	}
	else if(KEYNAME("MagneticFieldMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(magneticFieldMax, PUNI_DBL)
		phid->magneticFieldMax = value;
	}
	else if(KEYNAME("SpatialData"))
	{
		CPhidgetSpatial_SpatialEventDataHandle eventData[1];
		CPhidgetSpatial_SpatialEventData spatialData;
		int i;
		char *endptr = (char *)state-1;

		INC_KEYCOUNT(spatialDataNetwork, PUNI_BOOL)
		phid->spatialDataNetwork = PTRUE;
		
		for(i=0;i<SPATIAL_MAX_ACCELAXES;i++)
		{
			phid->accelAxis[i] = spatialData.acceleration[i] = strtod(endptr+1, &endptr);
		}
		for(i=0;i<SPATIAL_MAX_GYROAXES;i++)
		{
			phid->gyroAxis[i] = spatialData.angularRate[i] = strtod(endptr+1, &endptr);
		}
		for(i=0;i<SPATIAL_MAX_COMPASSAXES;i++)
		{
			phid->compassAxis[i] = spatialData.magneticField[i] = strtod(endptr+1, &endptr);
		}

		spatialData.timestamp.seconds = strtol(endptr+1, &endptr, 10);
		spatialData.timestamp.microseconds = strtol(endptr+1, NULL, 10);
		
		eventData[0] = &spatialData;
		FIRE(SpatialData, eventData, 1);
	}
	else{
		PWC_BAD_SETTYPE(Spatial);
	}
	return ret;
}

PWC_SETKEYS(Stepper)
	if(KEYNAME("NumberOfMotors"))
	{
		GET_INT_VAL;
		phid->phid.attr.stepper.numMotors = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("NumberOfInputs"))
	{
		GET_INT_VAL;
		phid->phid.attr.stepper.numInputs = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("Input"))
	{
		if(CHKINDEX(stepper.numInputs, STEPPER_MAXINPUTS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(inputState[index], PUNI_BOOL)
			phid->inputState[index] = value;
			if(value != PUNK_BOOL)
				FIRE(InputChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("CurrentPosition"))
	{
		if(CHKINDEX(stepper.numMotors, STEPPER_MAXSTEPPERS))
		{
			GET_INT64_VAL;
			INC_KEYCOUNT(motorPositionEcho[index], PUNI_INT64)
			phid->motorPositionEcho[index] = value;
			if(value != PUNK_INT64)
				FIRE(PositionChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	// initial target position isn't a keyCount++ sort of thing
	else if(KEYNAME("TargetPosition"))
	{
		if(CHKINDEX(stepper.numMotors, STEPPER_MAXSTEPPERS))
		{
			GET_INT64_VAL;
			INC_KEYCOUNT(motorPosition[index], PUNI_INT64)
			phid->motorPosition[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("PositionMin"))
	{
		GET_INT64_VAL;
		INC_KEYCOUNT(motorPositionMin, PUNI_INT64)
		phid->motorPositionMin = value;
	}
	else if(KEYNAME("PositionMax"))
	{
		GET_INT64_VAL;
		INC_KEYCOUNT(motorPositionMax, PUNI_INT64)
		phid->motorPositionMax = value;
	}
	// initial acceleration is unknown so dont' keyCount++
	else if(KEYNAME("Acceleration"))
	{
		if(CHKINDEX(stepper.numMotors, STEPPER_MAXSTEPPERS))
		{
			GET_DOUBLE_VAL;
			phid->motorAcceleration[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("AccelerationMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(accelerationMin, PUNI_DBL)
		phid->accelerationMin = value;
	}
	else if(KEYNAME("AccelerationMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(accelerationMax, PUNI_DBL)
		phid->accelerationMax = value;
	}
	// initial current limit is unknown so dont' keyCount++
	else if(KEYNAME("CurrentLimit"))
	{
		if(CHKINDEX(stepper.numMotors, STEPPER_MAXSTEPPERS))
		{
			GET_DOUBLE_VAL;
			phid->motorCurrentLimit[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Current"))
	{
		if(CHKINDEX(stepper.numMotors, STEPPER_MAXSTEPPERS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorSensedCurrent[index], PUNI_DBL)
			phid->motorSensedCurrent[index] = value;
			if(value != PUNK_DBL)
				FIRE(CurrentChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("CurrentMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(currentMin, PUNI_DBL)
		phid->currentMin = value;
	}
	else if(KEYNAME("CurrentMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(currentMax, PUNI_DBL)
		phid->currentMax = value;
	}
	// initial velocity limit is unknown so dont' keyCount++
	else if(KEYNAME("VelocityLimit"))
	{
		GET_DOUBLE_VAL;
		phid->motorSpeed[index] = value;
	}
	else if(KEYNAME("Velocity"))
	{
		if(CHKINDEX(stepper.numMotors, STEPPER_MAXSTEPPERS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(motorSpeedEcho[index], PUNI_DBL)
			phid->motorSpeedEcho[index] = value;
			if(value != PUNK_DBL)
				FIRE(VelocityChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("VelocityMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(motorSpeedMin, PUNI_DBL)
		phid->motorSpeedMin = value;
	}
	else if(KEYNAME("VelocityMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(motorSpeedMax, PUNI_DBL)
		phid->motorSpeedMax = value;
	}
	else if(KEYNAME("Engaged"))
	{
		if(CHKINDEX(stepper.numMotors, STEPPER_MAXSTEPPERS))
		{
			GET_INT_VAL;
			INC_KEYCOUNT(motorEngagedStateEcho[index], PUNI_BOOL)
			phid->motorEngagedStateEcho[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("Stopped"))
	{
		if(CHKINDEX(stepper.numMotors, STEPPER_MAXSTEPPERS))
		{
			GET_INT_VAL;
			unsigned char lastStoppedState = phid->motorStoppedState[index];
			INC_KEYCOUNT(motorStoppedState[index], PUNI_BOOL)
			phid->motorStoppedState[index] = value;
			//If changed, re-run position/velocity events, so the stopped change will be noticed
			if(lastStoppedState != value)
			{
				if(phid->motorSpeedEcho[index] != PUNK_DBL)
					FIRE(VelocityChange, index, phid->motorSpeedEcho[index]);
				if(phid->motorPositionEcho[index] != PUNK_INT64)
					FIRE(PositionChange, index, phid->motorPositionEcho[index]);
			}
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(Stepper);
	}
	return ret;
}

PWC_SETKEYS(TemperatureSensor)
	if(KEYNAME("NumberOfSensors"))
	{
		GET_INT_VAL;
		phid->phid.attr.temperaturesensor.numTempInputs = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("Potential"))
	{
		if(CHKINDEX(temperaturesensor.numTempInputs, TEMPSENSOR_MAXSENSORS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(Potential[index], PUNI_DBL)
			phid->Potential[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("PotentialMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(potentialMin, PUNI_DBL)
		phid->potentialMin = value;
	}
	else if(KEYNAME("PotentialMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(potentialMax, PUNI_DBL)
		phid->potentialMax = value;
	}
	else if(KEYNAME("Temperature"))
	{
		if(CHKINDEX(temperaturesensor.numTempInputs, TEMPSENSOR_MAXSENSORS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(Temperature[index], PUNI_DBL)
			phid->Temperature[index] = value;
			if(value != PUNK_DBL)
				FIRE(TemperatureChange, index, value);
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else if(KEYNAME("TemperatureMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(temperatureMin[index], PUNI_DBL)
		phid->temperatureMin[index] = value;
	}
	else if(KEYNAME("TemperatureMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(temperatureMax[index], PUNI_DBL)
		phid->temperatureMax[index] = value;
	}
	else if(KEYNAME("AmbientTemperature"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(AmbientTemperature, PUNI_DBL)
		phid->AmbientTemperature = value;
	}
	else if(KEYNAME("AmbientTemperatureMin"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(ambientTemperatureMin, PUNI_DBL)
		phid->ambientTemperatureMin = value;
	}
	else if(KEYNAME("AmbientTemperatureMax"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(ambientTemperatureMax, PUNI_DBL)
		phid->ambientTemperatureMax = value;
	}
	else if(KEYNAME("ThermocoupleType"))
	{
		GET_INT_VAL;
		INC_KEYCOUNT(ThermocoupleType[index], -1)
		phid->ThermocoupleType[index] = value;
	}
	else if(KEYNAME("Trigger"))
	{
		if(CHKINDEX(temperaturesensor.numTempInputs, TEMPSENSOR_MAXSENSORS))
		{
			GET_DOUBLE_VAL;
			INC_KEYCOUNT(TempChangeTrigger[index], PUNI_DBL)
			phid->TempChangeTrigger[index] = value;
		}
		else
			ret = EPHIDGET_OUTOFBOUNDS;
	}
	else{
		PWC_BAD_SETTYPE(TemperatureSensor);
	}
	return ret;
}

PWC_SETKEYS(TextLCD)
	GET_INT_VAL;

	if(KEYNAME("NumberOfRows"))
	{
		INC_KEYCOUNT(rowCount[index], PUNI_INT)
		phid->rowCount[index] = value;
	}
	else if(KEYNAME("NumberOfColumns"))
	{
		INC_KEYCOUNT(columnCount[index], PUNI_INT)
		phid->columnCount[index] = value;
	}
	else if(KEYNAME("NumberOfScreens"))
	{
		phid->phid.attr.textlcd.numScreens = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("Backlight"))
	{
		INC_KEYCOUNT(backlightEcho[index], PUNI_BOOL)
		phid->backlightEcho[index] = (unsigned char)value;
	}
	else if(KEYNAME("CursorOn"))
	{
		phid->cursorOn[index] = (unsigned char)value;
	}
	else if(KEYNAME("CursorBlink"))
	{
		phid->cursorBlink[index] = (unsigned char)value;
	}
	else if(KEYNAME("Contrast"))
	{
		INC_KEYCOUNT(contrastEcho[index], PUNI_INT)
		phid->contrastEcho[index] = (unsigned char)value;
	}
	else if(KEYNAME("Brightness"))
	{
		INC_KEYCOUNT(brightnessEcho[index], PUNI_INT)
		phid->brightnessEcho[index] = (unsigned char)value;
	}
	else if(KEYNAME("ScreenSize"))
	{
		INC_KEYCOUNT(screenSize[index], -1)
		phid->screenSize[index] = value;
	}
	else{
		PWC_BAD_SETTYPE(TextLCD);
	}
	return ret;
}

PWC_SETKEYS(TextLED)
	GET_INT_VAL;
	if(KEYNAME("NumberOfRows"))
	{
		phid->phid.attr.textled.numRows = value;
		phid->phid.keyCount++;
	}
	else if(KEYNAME("NumberOfColumns"))
	{
		phid->phid.attr.textled.numColumns = value;
		phid->phid.keyCount++;
	}
	// this is unknown at attach, so no keyCount++
	else if(KEYNAME("Brightness"))
	{
		phid->brightness = value;
	}
	else{
		PWC_BAD_SETTYPE(TextLED);
	}
	return ret;
}

PWC_SETKEYS(WeightSensor)
	if(KEYNAME("Weight"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(Weight, PUNI_DBL)
		phid->Weight = value;
		if(value != PUNK_DBL)
			FIRE(WeightChange, value);
	}
	else if(KEYNAME("Trigger"))
	{
		GET_DOUBLE_VAL;
		INC_KEYCOUNT(WeightChangeTrigger, PUNI_DBL)
		phid->WeightChangeTrigger = value;
	}
	else{
		PWC_BAD_SETTYPE(WeightSensor);
	}
	return ret;
}

int(*fptrSet[PHIDGET_DEVICE_CLASS_COUNT])(CPhidgetHandle generic_phid, const char *setThing, int index, const char *state, pdict_reason_t reason) = {
NULL,
NULL,
phidgetAccelerometer_set,
phidgetAdvancedServo_set,
phidgetEncoder_set,
phidgetGPS_set,
NULL,//old gyro,
phidgetInterfaceKit_set,
phidgetLED_set,
phidgetMotorControl_set,
phidgetPHSensor_set,
phidgetRFID_set,
phidgetServo_set,
phidgetStepper_set,
phidgetTemperatureSensor_set,
phidgetTextLCD_set,
phidgetTextLED_set,
phidgetWeightSensor_set,
phidgetGeneric_set,
phidgetIR_set,
phidgetSpatial_set,
phidgetFrequencyCounter_set,
phidgetAnalog_set,
phidgetBridge_set};
void network_phidget_event_handler(const char *key, const char *val, unsigned int len, pdict_reason_t reason, void *ptr)
{
	CPhidgetHandle phid = (CPhidgetHandle)ptr;
	regmatch_t pmatch[7];
	char *setThing = NULL;
	char *index = NULL;
	char *serial = NULL;
	char *label = NULL, *l = NULL;
	char errbuf[1024];
	
	int serialNumber;

	int res, ind = PUNK_INT, i, ret = EPHIDGET_OK;

	if(!strncmp(val, "\001", 1) && len == 1)
	{
		memset((char *)val,0,1);
	}


	if(!CPhidget_statusFlagIsSet(phid->status, PHIDGET_SERVER_CONNECTED_FLAG))
	{
		LOG(PHIDGET_LOG_VERBOSE, "network_phidget_event_handler: key ignored: %s Value: %s",key,val);
	}
	else if(reason!=PDR_ENTRY_REMOVING || !strncmp(val, "Detached", sizeof("Detached")))
	{
		LOG(PHIDGET_LOG_VERBOSE, "network_phidget_event_handler: key: %s Value: %s",key,val);
		if ((res = regexec(&phidgetsetex, key, 7, pmatch, 0)) != 0) {
			LOG(PHIDGET_LOG_DEBUG,"Error in network_phidget_event_handler - pattern not met for key: \"%s\"",key);
			return;
		}
		getmatchsub(key, &label, pmatch, 2);
		getmatchsub(key, &serial, pmatch, 3);
		getmatchsub(key, &setThing, pmatch, 4);
		getmatchsub(key, &index, pmatch, 5);
		
		if(l)
		{
			if (!unescape(l, &label, NULL))
			{
				LOG(PHIDGET_LOG_ERROR, "Error in unescape");
				return;
			}
		}

		serialNumber = strtol(serial, NULL, 10);
			
		if(phid->specificDevice == PHIDGETOPEN_ANY && strncmp(val, "Detached", sizeof("Detached")))
		{
			phid->specificDevice = PHIDGETOPEN_ANY_ATTACHED;
			phid->serialNumber = serialNumber;
		}

		if(phid->specificDevice == PHIDGETOPEN_LABEL && strncmp(val, "Detached", sizeof("Detached")))
		{
			phid->serialNumber = serialNumber;
		}
		
		if(serialNumber == phid->serialNumber && setThing)
		{
			if(KEYNAME("Label"))
			{
				strncpy(phid->label, val, MAX_LABEL_STORAGE);
				phid->keyCount++;
			}
			else if(KEYNAME("InitKeys"))
			{
				phid->initKeys = strtol(val, NULL, 10);
				phid->keyCount++;
			}
			else if(KEYNAME("Version"))
			{
				phid->deviceVersion = strtol(val, NULL, 10);
				phid->keyCount++;
			}
			else if(KEYNAME("ID"))
			{
				phid->deviceIDSpec = strtol(val, NULL, 10);
				phid->deviceType = Phid_DeviceName[phid->deviceID];
				phid->keyCount++;

				for(i = 1;i<PHIDGET_DEVICE_COUNT;i++)
				{
					if(Phid_Device_Def[i].pdd_sdid == phid->deviceIDSpec)
					{
						phid->deviceDef = &Phid_Device_Def[i];
						phid->attr = Phid_Device_Def[i].pdd_attr;
						break;
					}
				}
			}
			else if(KEYNAME("Name"))
			{
				phid->keyCount++;
			}
			else if(KEYNAME("Error"))
			{
				char *endptr;
				int errcode = strtol(val, &endptr, 10);
				char *errstr = endptr+1;
				FIRE_ERROR_BUF(errcode, errstr);
			}
			else if(KEYNAME("Status"))
			{
				if(!strncmp(val, "Attached", sizeof("Attached")))
				{
					phid->keyCount++;
				}
				else if(!strncmp(val, "Detached", sizeof("Detached")))
				{
					CThread_mutex_lock(&phid->lock);
					phid->keyCount = 0;
					if(phid->specificDevice == PHIDGETOPEN_ANY_ATTACHED)
					{
						phid->specificDevice = PHIDGETOPEN_ANY;
					}
					CPhidget_clearStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, NULL);
					CPhidget_setStatusFlag(&phid->status, PHIDGET_DETACHING_FLAG, NULL);
					CThread_mutex_unlock(&phid->lock);

					if (phid->fptrDetach)
					{
						SET_RUNNING_EVENT(phid)
						phid->fptrDetach((CPhidgetHandle)phid, phid->fptrDetachptr);
						CLEAR_RUNNING_EVENT(phid)
					}

					//clear all variables
					phid->fptrClear((CPhidgetHandle)phid);
					
					//if mDNS & any server, disconnect
#ifdef USE_ZEROCONF
					CThread_mutex_lock(&phid->lock);
					if(phid->networkInfo && !phid->networkInfo->requested_address && !phid->networkInfo->requested_serverID)
					{
						CThread DisconnectPhidgetThread;
						CThread_mutex_unlock(&phid->lock);
						CThread_mutex_lock(&zeroconfPhidgetsLock);
						CList_removeFromList((CListHandle *)&zeroconfPhidgets, phid, CPhidget_areExtraEqual, TRUE, CPhidget_free);
						CThread_mutex_unlock(&zeroconfPhidgetsLock);
						CThread_create_detached(&DisconnectPhidgetThread, DisconnectPhidgetThreadFunction, phid);
					}
					CThread_mutex_unlock(&phid->lock);
#endif
					
					CPhidget_clearStatusFlag(&phid->status, PHIDGET_DETACHING_FLAG, NULL);
						
					phid->deviceIDSpec = 0;
					phid->deviceUID = 0;
					ZEROMEM(&phid->attr, sizeof(CPhidgetAttr));
					if(phid->specificDevice != PHIDGETOPEN_LABEL)
						ZEROMEM(phid->label, MAX_LABEL_STORAGE);
					phid->deviceVersion = 0;
					phid->initKeys = PUNK_INT;

				}
				else
				{
					throw_error_event(phid, "Bad Message type for Status set", EEPHIDGET_NETWORK);
				}
			}
			else if(fptrSet[phid->deviceID] && setThing)
			{
				if(index)
					ind = strtol(index, NULL, 10);

				SET_RUNNING_EVENT(phid)
				ret = fptrSet[phid->deviceID](phid, setThing, ind, val, reason);
				CLEAR_RUNNING_EVENT(phid)
			}
			
			if((phid->initKeys != PUNK_INT) 
				&& (phid->keyCount >= phid->initKeys) 
				&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
				&& CPhidget_statusFlagIsSet(phid->status, PHIDGET_OPENED_FLAG)
				&& CPhidget_statusFlagIsSet(phid->status, PHIDGET_SERVER_CONNECTED_FLAG))
			{
				LOG(PHIDGET_LOG_VERBOSE, "Got all initkeys, run attach - %d/%d", phid->keyCount, phid->initKeys);

				//Set UID
				phid->deviceUID = CPhidget_getUID(phid->deviceIDSpec, phid->deviceVersion);

				CPhidget_setStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);

				SET_RUNNING_EVENT(phid)
				if (phid->fptrAttach)
					phid->fptrAttach(phid, phid->fptrAttachptr);
				phid->fptrEvents((CPhidgetHandle)phid);
				CLEAR_RUNNING_EVENT(phid)
			}
			else
				LOG(PHIDGET_LOG_VERBOSE, "Accumulating initkeys for attach - %d/%d", phid->keyCount, phid->initKeys == PUNK_INT ? 0 : phid->initKeys);

			//LOG(PHIDGET_LOG_DEBUG, "Message: %s(%s)=%s (%d of %d)", setThing, index, val, phid->keyCount, phid->initKeys);
		}

		free(setThing); setThing = NULL;
		free(index); index = NULL;
		free(serial); serial = NULL;
	}
	
	if(ret)
	{
		snprintf(errbuf, 1024, "Problem during Network set (Phidget): %s\n (Key:\"%s\", Val:\"%s\"", CPhidget_strerror(ret), key, val);
		throw_error_event(phid, errbuf, EEPHIDGET_NETWORK);
	}
}

void network_manager_event_handler(const char *key, const char *val, unsigned int vallen, pdict_reason_t reason, void *ptr)
{
	CPhidgetManagerHandle phidm = (CPhidgetManagerHandle)ptr;
	regmatch_t keymatch[6], valmatch[6];
	char *attachDetach = NULL;
	char *deviceType = NULL;
	char *serial = NULL;
	char *version = NULL;
	char *deviceIDSpec = NULL;
	char *label = NULL;
	char errbuf[1024];
	
	int serialNumber;
	CPhidgetHandle phid;

	//BL:Changed to init Len before use
	int len = 0;

	int res, ret = EPHIDGET_OK;

	int i;
	
	if(!phidm) return;

	if(!strncmp(val, "\001", 1) && (len == 1))
	{
		memset((char *)val,0,1);
	}

	if(reason!=PDR_ENTRY_REMOVING)
	{
		if ((res = regexec(&managerex, key, 3, keymatch, 0)) != 0) {
			LOG(PHIDGET_LOG_DEBUG,"Error in network_manager_event_handler - key pattern not met");
			return;
		}
		if ((res = regexec(&managervalex, val, 5, valmatch, 0)) != 0) {
			LOG(PHIDGET_LOG_DEBUG,"Error in network_manager_event_handler - val pattern not met");
			return;
		}
		getmatchsub(key, &deviceType, keymatch, 1);
		getmatchsub(key, &serial, keymatch, 2);

		getmatchsub(val, &attachDetach, valmatch, 1);
		getmatchsub(val, &version, valmatch, 2);
		getmatchsub(val, &deviceIDSpec, valmatch, 3);
		getmatchsub(val, &label, valmatch, 4);
		
		serialNumber = strtol(serial, NULL, 10);
	
		if((CPhidget_create(&phid))) return;

		phid->deviceID = phidget_type_to_id(deviceType);
		phid->deviceType = Phid_DeviceName[phid->deviceID];
		phid->serialNumber = serialNumber;
		phid->deviceIDSpec = (unsigned short)strtol(deviceIDSpec, NULL, 10);
		phid->deviceVersion = strtol(version, NULL, 10);
		phid->deviceUID = CPhidget_getUID(phid->deviceIDSpec, phid->deviceVersion);
		phid->specificDevice = PHIDGETOPEN_SERIAL; //so it actually compares the serial

		for(i = 1;i<PHIDGET_DEVICE_COUNT;i++)
			if(phid->deviceIDSpec == Phid_Device_Def[i].pdd_sdid) break;
		phid->deviceDef = &Phid_Device_Def[i];
		phid->attr = Phid_Device_Def[i].pdd_attr;

		//so se can get address, etc. from devices.
		phid->networkInfo = phidm->networkInfo;
		CPhidget_setStatusFlag(&phid->status, PHIDGET_REMOTE_FLAG, &phid->lock);
		CPhidget_setStatusFlag(&phid->status, PHIDGET_SERVER_CONNECTED_FLAG, &phid->lock);

		if(label)
		{
			strncpy(phid->label, label, MAX_LABEL_STORAGE);
		}

		if(!strncmp(attachDetach, "Attached", sizeof("Attached")))
		{
			CPhidget_setStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
		
			CList_addToList((CListHandle *)&phidm->AttachedPhidgets, phid, CPhidget_areEqual);

			if (phidm->fptrAttachChange && phidm->state == PHIDGETMANAGER_ACTIVE)
			{
				SET_RUNNING_EVENT(phidm)
				phidm->fptrAttachChange((CPhidgetHandle)phid, phidm->fptrAttachChangeptr);
				CLEAR_RUNNING_EVENT(phidm)
			}
		}
		
		if(!strncmp(attachDetach, "Detached", sizeof("Detached")))
		{
			CPhidget_clearStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
			CPhidget_setStatusFlag(&phid->status, PHIDGET_DETACHING_FLAG, &phid->lock);
			if(CList_findInList((CListHandle)phidm->AttachedPhidgets, phid, CPhidget_areEqual, NULL) == EPHIDGET_OK)
			{
				if (phidm->fptrDetachChange && phidm->state == PHIDGETMANAGER_ACTIVE)
				{
					SET_RUNNING_EVENT(phidm)
					phidm->fptrDetachChange((CPhidgetHandle)phid, phidm->fptrDetachChangeptr);
					CLEAR_RUNNING_EVENT(phid)
				}

				CList_removeFromList((CListHandle *)&phidm->AttachedPhidgets, phid, CPhidget_areEqual, PTRUE, CPhidget_free);
			}
			CPhidget_clearStatusFlag(&phid->status, PHIDGET_DETACHING_FLAG, &phid->lock);
			CPhidget_free(phid); phid = NULL;
		}

		free(deviceType); deviceType = NULL;
		free(label); label = NULL;
		free(attachDetach); attachDetach = NULL;
		free(serial); serial = NULL;
		free(version); version = NULL;
		free(deviceIDSpec); deviceIDSpec = NULL;
	}
	
	if(ret)
	{
		snprintf(errbuf, 1024, "Problem during Network set (Manager): %s\n (Key:\"%s\", Val:\"%s\"", CPhidget_strerror(ret), key, val);
		throw_error_event((CPhidgetHandle)phidm, errbuf, EEPHIDGET_NETWORK);
	}
}

void network_heartbeat_event_handler(const char *key, const char *val, unsigned int len, pdict_reason_t reason, void *ptr)
{
	CPhidgetSocketClientHandle server = (CPhidgetSocketClientHandle)ptr;
	double duration = timeSince(&server->lastHeartbeatTime);

	//Keeps a rolling average of the last 5 times
	if(server->avgHeartbeatTimeCount > 5)
	{
		double avg = server->avgHeartbeatTime / server->avgHeartbeatTimeCount;
		server->avgHeartbeatTime -= avg;
		server->avgHeartbeatTimeCount--;
	}
	server->avgHeartbeatTime += duration;
	server->avgHeartbeatTimeCount++;

	server->heartbeatCount++;

	setTimeNow(&server->lastHeartbeatTime);
	server->waitingForHeartbeat = PFALSE;
}
