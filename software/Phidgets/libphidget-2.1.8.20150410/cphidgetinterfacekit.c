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
#include "cphidgetinterfacekit.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

static int CPhidgetInterfaceKit_getPacket(CPhidgetHandle, unsigned char *, unsigned int *);

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(InterfaceKit)
	int i = 0;

	phid->ratiometric = PUNI_BOOL;
	phid->ratiometricEcho = PUNK_BOOL;
	phid->lastChangedOutput = PUNK_INT;
	phid->fullStateEcho = PUNK_BOOL;
	phid->ratiometricSwitching = 0;
	phid->dataRateMin = PUNI_INT;
	phid->dataRateMax = PUNI_INT;
	phid->interruptRate = PUNI_INT;

	for (i = 0; i<IFKIT_MAXINPUTS; i++)
		phid->physicalState[i] = PUNI_BOOL;
	for (i = 0; i<IFKIT_MAXSENSORS; i++)
	{
		phid->sensorChangeTrigger[i] = PUNI_INT;
		phid->sensorRawValue[i] = PUNI_INT;
		phid->sensorValue[i] = PUNI_INT;
		phid->sensorLastValue[i] = PUNK_INT;
		phid->dataRate[i] = PUNI_INT;
	}
	for (i = 0; i<IFKIT_MAXOUTPUTS; i++)
	{
		phid->nextOutputStates[i] = PUNK_BOOL;
		phid->outputEchoStates[i] = PUNI_BOOL;
		phid->outputStates[i] = PUNK_BOOL;
		phid->changedOutputs[i] = PUNK_BOOL;
	}

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(InterfaceKit)
	int i = 0, j = 0;
	int readtries = 0;
	unsigned char buffer[8] = { 0 };
	unsigned int len = 8;
	int result;

	TESTPTR(phid);

	//init device specific attrs
	phid->phid.awdc_enabled = FALSE;
	phid->fullStateEcho = FALSE;
	phid->maxDataPerPacket = phid->phid.attr.ifkit.numSensors;
	phid->ratiometricSwitching = 0;
	phid->lastPacketCount = -1;
	phid->dataSinceAttach = 0;
	switch(phid->phid.deviceIDSpec) {
		case PHIDID_INTERFACEKIT_8_8_8:
			if (phid->phid.deviceVersion < 900) {
				phid->interruptRate = 16;
				phid->dataRateMin = IFKIT_MIN_DATA_RATE/phid->interruptRate*phid->interruptRate;
				phid->dataRateMax = 16; //actual data rate
			}
			if (phid->phid.deviceVersion >= 821 && phid->phid.deviceVersion < 900) {
				phid->phid.awdc_enabled = PTRUE;
				phid->fullStateEcho = TRUE;
			}
			if (phid->phid.deviceVersion >= 900 && phid->phid.deviceVersion < 1000) {
				phid->phid.awdc_enabled = PTRUE; //To help with pre v902 bug (see firmware)
				phid->fullStateEcho = TRUE;
				phid->interruptRate = 8;
				phid->dataRateMin = IFKIT_MIN_DATA_RATE/phid->interruptRate*phid->interruptRate;
				phid->dataRateMax = 1; //actual data rate
				phid->maxDataPerPacket = 36;
			}
			break;
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:	
			if (phid->phid.deviceVersion < 200) {
				phid->interruptRate = 16;
				phid->dataRateMin = IFKIT_MIN_DATA_RATE/phid->interruptRate*phid->interruptRate;
				phid->dataRateMax = 16; //actual data rate
			}
			if (phid->phid.deviceVersion >= 120 && phid->phid.deviceVersion < 200) {
				phid->phid.awdc_enabled = PTRUE;
				phid->fullStateEcho = TRUE;
			}
			if (phid->phid.deviceVersion >= 200 && phid->phid.deviceVersion < 300) {
				phid->phid.awdc_enabled = PTRUE; //To help with pre v902 bug (see firmware)
				phid->fullStateEcho = TRUE;
				phid->interruptRate = 8;
				phid->dataRateMin = IFKIT_MIN_DATA_RATE/phid->interruptRate*phid->interruptRate;
				phid->dataRateMax = 1; //actual data rate
				phid->maxDataPerPacket = 36;
			}
			break;
		case PHIDID_INTERFACEKIT_2_2_2:	
			if (phid->phid.deviceVersion < 200) {
				phid->fullStateEcho = TRUE;
				phid->interruptRate = 8;
				phid->dataRateMin = IFKIT_MIN_DATA_RATE/phid->interruptRate*phid->interruptRate;
				phid->dataRateMax = 1; //actual data rate
				phid->maxDataPerPacket = 16; //TODO: check this
			}
			break;
		case PHIDID_INTERFACEKIT_0_0_4:
			if (phid->phid.deviceVersion >= 704 && phid->phid.deviceVersion < 800)
				phid->fullStateEcho = TRUE;
			break;
		case PHIDID_INTERFACEKIT_0_0_8:
			phid->fullStateEcho = TRUE;
			break;
		case PHIDID_INTERFACEKIT_0_16_16:
			if (phid->phid.deviceVersion >= 601 && phid->phid.deviceVersion < 700)
				phid->fullStateEcho = TRUE;
			break;
		case PHIDID_ROTARY_TOUCH:
		case PHIDID_LINEAR_TOUCH:
		case PHIDID_INTERFACEKIT_0_5_7:
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_4_8_8:
		default:
			break;
	}

	//initialize triggers, set data arrays to unknown
	phid->ratiometric = PUNK_BOOL;
	phid->ratiometricEcho = PUNK_BOOL;
	for (j = 0; j<phid->phid.attr.ifkit.numInputs; j++)
		phid->physicalState[j] = PUNK_BOOL;
	for (j = 0; j<phid->phid.attr.ifkit.numSensors; j++)
	{
		switch(phid->phid.deviceIDSpec) {
			case PHIDID_ROTARY_TOUCH:
			case PHIDID_LINEAR_TOUCH:
				//minimum change is 7-8, so we want full accuracy - 10 would not make sense
				phid->sensorChangeTrigger[j] = 1;
				break;
			default:
				phid->sensorChangeTrigger[j] = 10;
				break;
		}
		//Only the new devices go into dataRate event mode by default
		//default mode is change trigger
		phid->eventMode[j] = EVENTMODE_CHANGETRIGGER;
		phid->dataRate[j] = phid->interruptRate;
		switch(phid->phid.deviceIDSpec) {
			case PHIDID_INTERFACEKIT_8_8_8:
				if (phid->phid.deviceVersion >= 900 && phid->phid.deviceVersion < 1000) {
					phid->eventMode[j] = EVENTMODE_DATARATE;
				}
				break;
			case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
				if (phid->phid.deviceVersion >= 200 && phid->phid.deviceVersion < 300) {
					phid->eventMode[j] = EVENTMODE_DATARATE;
				}
				break;
			case PHIDID_INTERFACEKIT_2_2_2:
			if (phid->phid.deviceVersion < 200) {
					phid->eventMode[j] = EVENTMODE_DATARATE;
				}
				break;
			default:
				break;
		}
		phid->sensorRawValue[j] = PUNK_INT;
		phid->sensorValue[j] = PUNK_INT;
		phid->sensorLastValue[j] = PUNK_INT;
		phid->sensorRawValueAccumulator[j] = 0;
		phid->sensorRawValueAccumulatorCount[j] = 0;
	}
	for (j = 0; j<phid->phid.attr.ifkit.numOutputs; j++)
	{
		phid->nextOutputStates[j] = PUNK_BOOL;
		phid->outputEchoStates[j] = PUNK_BOOL;
		phid->changedOutputs[j] = PFALSE;
	}
	phid->lastChangedOutput = 0;

	//send out any initial pre-read packets
	switch(phid->phid.deviceIDSpec) {
		case PHIDID_ROTARY_TOUCH:
			{
				//performs initial calibration
				buffer[0] = 0x01;
				if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
					return result;
			}
			break;
		case PHIDID_LINEAR_TOUCH:
			{
				//performs initial calibration
				buffer[0] = 0x01;
				if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
					return result;
				SLEEP(100);
				buffer[0] = 0x02;
				if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
					return result;
			}
			break;
		case PHIDID_INTERFACEKIT_4_8_8:
			if (phid->phid.deviceVersion <= 100)
			{
				ZEROMEM(buffer,8);
				LOG(PHIDGET_LOG_INFO,"Sending workaround startup packet");
				if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
					return result;
			}
			break;
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_2_2_2:
		case PHIDID_INTERFACEKIT_0_0_4:
		case PHIDID_INTERFACEKIT_0_0_8:
		case PHIDID_INTERFACEKIT_0_16_16:
		case PHIDID_INTERFACEKIT_0_5_7:
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
		default:
			break;
	}

	//read in device state - don't issue reads on devices that block
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_INTERFACEKIT_0_16_16:
		case PHIDID_INTERFACEKIT_0_0_8:
		case PHIDID_INTERFACEKIT_0_0_4:
			if(phid->fullStateEcho)
				CPhidget_read((CPhidgetHandle)phid);
			break;
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_2_2_2:
		case PHIDID_INTERFACEKIT_4_8_8:
			readtries = 4;
			while(readtries-- > 0)
			{
				CPhidget_read((CPhidgetHandle)phid);
				for (i = 0; i<phid->phid.attr.ifkit.numSensors; i++)
					if(phid->sensorValue[i] == PUNK_INT)
						break;
				if(i==phid->phid.attr.ifkit.numSensors) break;
			}
			break;
		case PHIDID_LINEAR_TOUCH:
		case PHIDID_ROTARY_TOUCH:
			CPhidget_read((CPhidgetHandle)phid); //clear any bad data...
			CPhidget_read((CPhidgetHandle)phid);
			break;
		//these only send data on a change - so we have no way of knowing initial state, or even state at all, until an input changes state
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_0_5_7:
		default:
			break;
	}

	//initialize outputs
	for (j = 0; j<phid->phid.attr.ifkit.numOutputs; j++) {
		phid->outputStates[j] = phid->outputEchoStates[j];
	}

	//send out any initial post-read packets
	switch(phid->phid.deviceIDSpec) {
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
			if ((phid->phid.deviceVersion < 200 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8_w_LCD)
				|| (phid->phid.deviceVersion < 900 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8))
			{
				//always turn on ratiometric because there is no way to read it's state from the device, and most
				// sensors need ratiometric
				// set this in post read because the outputs are set in the same packet as ratiometric, and we want to preserve their state.
				phid->ratiometric = PTRUE;
				if ((result = CPhidgetInterfaceKit_getPacket((CPhidgetHandle)phid, (unsigned char *)&buffer, &len)) != EPHIDGET_OK)
					return result;
				if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
					return result;
			}
			//newer versions send back their ratiometric state, and default to ratiometric enabled.
			break;
		case PHIDID_INTERFACEKIT_2_2_2:
		case PHIDID_ROTARY_TOUCH:
		case PHIDID_LINEAR_TOUCH:
		case PHIDID_INTERFACEKIT_0_0_4:
		case PHIDID_INTERFACEKIT_0_0_8:
		case PHIDID_INTERFACEKIT_0_16_16:
		case PHIDID_INTERFACEKIT_0_5_7:
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_4_8_8:
		default:
			break;
	}

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(InterfaceKit)
	int j = 0, i = 0, k = 0;

	unsigned char outputState[IFKIT_MAXOUTPUTS], lastOutputState[IFKIT_MAXOUTPUTS];
	unsigned char inputState[IFKIT_MAXINPUTS], lastInputState[IFKIT_MAXINPUTS];
	int sensorValue[IFKIT_MAXSENSORS][IFKIT_MAX_DATA_PER_PACKET];
	int sensorRawValue[IFKIT_MAXSENSORS][IFKIT_MAX_DATA_PER_PACKET];
	int sensorDataCount[IFKIT_MAXSENSORS];
	unsigned char ratiometricEcho = PUNK_BOOL;

	if (length<0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);
	
	if(CPhidget_statusFlagIsSet(((CPhidgetHandle)phid)->status, PHIDGET_ATTACHED_FLAG))
		if(phid->dataSinceAttach < 100)
			phid->dataSinceAttach++;

	for (j = 0; j<phid->phid.attr.ifkit.numInputs; j++)
	{
		inputState[j] = PUNK_BOOL;
		lastInputState[j] = phid->physicalState[j];
	}
	for (j = 0; j<phid->phid.attr.ifkit.numSensors; j++)
	{
		for(i=0;i<IFKIT_MAX_DATA_PER_PACKET;i++)
		{
			sensorRawValue[j][i] = PUNK_INT;
			sensorValue[j][i] = PUNK_INT;
		}
		sensorDataCount[j] = 0;
	}
	for (j = 0; j<phid->phid.attr.ifkit.numOutputs; j++)
	{
		outputState[j] = PUNK_BOOL;
		lastOutputState[j] = phid->outputEchoStates[j];
	}
		
	//Parse device packets - store data locally
	switch (phid->phid.deviceIDSpec)
	{
		case PHIDID_INTERFACEKIT_4_8_8:
			//Sensors
			sensorRawValue[0][0] = ((unsigned char)buffer[3] + ((unsigned char)buffer[2] & 0x0f) * 256);
			sensorDataCount[0]++;
			sensorRawValue[1][0] = ((unsigned char)buffer[1] + ((unsigned char)buffer[2] & 0xf0) * 16);
			sensorDataCount[1]++;
			sensorRawValue[2][0] = ((unsigned char)buffer[6] + ((unsigned char)buffer[5] & 0x0f) * 256);
			sensorDataCount[2]++;
			sensorRawValue[3][0] = ((unsigned char)buffer[4] + ((unsigned char)buffer[5] & 0xf0) * 16);
			sensorDataCount[3]++;

			for (i = 0; i<phid->phid.attr.ifkit.numSensors; i++)
				sensorValue[i][0] = round((double)sensorRawValue[i][0] / 4.095);

			//Inputs
			for (i = 0, j = 0x80; i < 4; i++, j >>= 1)
			{
				if ((buffer[0] & j) != 0)
					inputState[i] = PFALSE;
				else
					inputState[i] = PTRUE;
			}
			for (i = 4, j = 0x01; i < 8; i++, j <<= 1)
			{
				if ((buffer[0] & j) != 0)
					inputState[i] = PFALSE;
				else
					inputState[i] = PTRUE;
			}

			break;
		case PHIDID_INTERFACEKIT_0_16_16:
			//Inputs
			for (k = 0; k<2; k++)
			{
				for (i = 0, j = 0x01; i<8; i++, j <<= 1)
				{
					if ((buffer[k] & j) == 0)
						inputState[i+k*8] = PFALSE;
					else
						inputState[i+k*8] = PTRUE;
				}
			}

			//Outputs
			if (phid->fullStateEcho)
			{
				if(phid->phid.deviceVersion == 601) //top 8 bits are flipped
				{
					for (i = 0, j = 0x01; i<8; i++, j <<= 1)
					{
						if ((buffer[2] & j) == 0)
							outputState[i] = PFALSE;
						else
							outputState[i] = PTRUE;
					}
					for (i = 7, j = 0x01; i>=0; i--, j <<= 1)
					{
						if ((buffer[3] & j) == 0)
							outputState[i+8] = PFALSE;
						else
							outputState[i+8] = PTRUE;
					}
				}
				else
				{
					for (k = 0; k<2; k++)
					{
						for (i = 0, j = 0x01; i<8; i++, j <<= 1)
						{
							if ((buffer[k+2] & j) == 0)
								outputState[i+k*8] = PFALSE;
							else
								outputState[i+k*8] = PTRUE;
						}
					}
				}
			}

			break;
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_2_2_2:
			if ((phid->phid.deviceVersion < 200 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8_w_LCD)
				|| (phid->phid.deviceVersion < 900 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8))
			{
				//Inputs
				for (i = 0, j = 0x01; i < phid->phid.attr.ifkit.numInputs; i++, j <<= 1)
				{
					if (buffer[1] & j)
						inputState[i] = PFALSE;
					else
						inputState[i] = PTRUE;
				}

				//there are two types of packets
				if (!(buffer[0] & 0x01))
				{
					//Sensors
					sensorRawValue[0][0] = ((unsigned char)buffer[2] + ((unsigned char)buffer[3] & 0x0f) * 256);
					sensorDataCount[0]++;
					sensorRawValue[1][0] = ((unsigned char)buffer[4] + ((unsigned char)buffer[3] & 0xf0) * 16);
					sensorDataCount[1]++;
					sensorRawValue[2][0] = ((unsigned char)buffer[5] + ((unsigned char)buffer[6] & 0x0f) * 256);
					sensorDataCount[2]++;
					sensorRawValue[3][0] = ((unsigned char)buffer[7] + ((unsigned char)buffer[6] & 0xf0) * 16);
					sensorDataCount[3]++;
					for (i = 0; i<4; i++)
						sensorValue[i][0] = round((double)sensorRawValue[i][0] / 4.095);

					//Outputs
					if (phid->fullStateEcho)
					{
						for (i = 0, j = 0x10; i<4; i++, j <<= 1)
						{
							if ((buffer[0] & j) == 0)
								outputState[i] = PFALSE;
							else
								outputState[i] = PTRUE;
						}
					}
				}
				else
				{
					//Sensors
					sensorRawValue[4][0] = ((unsigned char)buffer[2] + ((unsigned char)buffer[3] & 0x0f) * 256);
					sensorDataCount[4]++;
					sensorRawValue[5][0] = ((unsigned char)buffer[4] + ((unsigned char)buffer[3] & 0xf0) * 16);
					sensorDataCount[5]++;
					sensorRawValue[6][0] = ((unsigned char)buffer[5] + ((unsigned char)buffer[6] & 0x0f) * 256);
					sensorDataCount[6]++;
					sensorRawValue[7][0] = ((unsigned char)buffer[7] + ((unsigned char)buffer[6] & 0xf0) * 16);
					sensorDataCount[7]++;
					for (i = 4; i<8; i++)
						sensorValue[i][0] = round((double)sensorRawValue[i][0] / 4.095);

					//Outputs
					if (phid->fullStateEcho)
					{
						for (i = 4, j = 0x10; i<8; i++, j <<= 1)
						{
							if ((buffer[0] & j) == 0)
								outputState[i] = PFALSE;
							else
								outputState[i] = PTRUE;
						}
					}
				}
				break;
			}
			else if((phid->phid.deviceVersion >= 900 && phid->phid.deviceVersion < 1000 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8)
				|| (phid->phid.deviceVersion >= 200 && phid->phid.deviceVersion < 300 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8_w_LCD)
				|| (phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_2_2_2))
			{
				int overrunBits, overrunPtr, countPtr, packetCount, channelCount[IFKIT_MAXSENSORS], overrunCount[IFKIT_MAXSENSORS];
				unsigned char overcurrentFlag = 0;
				int datacount = 0;
				int flip, bufindx;
				char error_buffer[127];

				//counters, etc.
				packetCount = (buffer[0] >> 6) & 0x03;
				overcurrentFlag = (buffer[0] >> 5) & 0x01;
				ratiometricEcho = (buffer[0] >> 4) & 0x01;
				overrunBits = buffer[0] & 0x0f;

				//Inputs
				for (i = 0, j = 0x01; i < phid->phid.attr.ifkit.numInputs; i++, j <<= 1)
				{
					if (buffer[1] & j)
						inputState[i] = PFALSE;
					else
						inputState[i] = PTRUE;
				}

				//Outputs
				for (i = 0, j = 0x01; i < phid->phid.attr.ifkit.numOutputs; i++, j <<= 1)
				{
					if ((buffer[2] & j) == 0)
						outputState[i] = PFALSE;
					else
						outputState[i] = PTRUE;
				}

				//Sensors
				//Overruns
				overrunPtr = 3;
				for (i = 0; i<phid->phid.attr.ifkit.numSensors; i++)
				{
					overrunCount[i] = 0;
				}
				if(overrunBits & 0x01)
				{
					overrunCount[0] = buffer[overrunPtr] >> 4;
					overrunCount[1] = buffer[overrunPtr] & 0x0f;
					overrunPtr++;
				}
				if(overrunBits & 0x02)
				{
					overrunCount[2] = buffer[overrunPtr] >> 4;
					overrunCount[3] = buffer[overrunPtr] & 0x0f;
					overrunPtr++;
				}
				if(overrunBits & 0x04)
				{
					overrunCount[4] = buffer[overrunPtr] >> 4;
					overrunCount[5] = buffer[overrunPtr] & 0x0f;
					overrunPtr++;
				}
				if(overrunBits & 0x08)
				{
					overrunCount[6] = buffer[overrunPtr] >> 4;
					overrunCount[7] = buffer[overrunPtr] & 0x0f;
					overrunPtr++;
				}

				//Counts
				countPtr = overrunPtr;
				for (i = 0; i<phid->phid.attr.ifkit.numSensors; i++)
				{
					if(i%2)
					{
						channelCount[i] = buffer[countPtr] & 0x0F;
						countPtr++;
					}
					else
					{
						channelCount[i] = buffer[countPtr] >> 4;
					}
					datacount+=channelCount[i];
				}

				//Data
				j=0;
				flip = 0;
				bufindx = countPtr;
				while(datacount>0)
				{
					for (i = 0; i<phid->phid.attr.ifkit.numSensors; i++)
					{
						if(channelCount[i]>j)
						{
							if(!flip)
							{
								sensorRawValue[i][j] = ((unsigned char)buffer[bufindx] + (((unsigned char)buffer[bufindx+1] & 0xf0) << 4));
								bufindx+=2;
							}
							else
							{
								sensorRawValue[i][j] = ((unsigned char)buffer[bufindx] + (((unsigned char)buffer[bufindx-1] & 0x0f) << 8));
								bufindx++;
							}
							//compensating for resistors, etc. - on earlier versions, this was done in Firmware.
							sensorRawValue[i][j] = round(sensorRawValue[i][j] * 1.001);
							if(sensorRawValue[i][j] > 0xfff)
								sensorRawValue[i][j] = 0xfff;
							sensorValue[i][j] = round((double)sensorRawValue[i][j] / 4.095);
							sensorDataCount[i]++;
							flip^=0x01;
							datacount--;
						}
					}
					j++;
				}
				if(datacount < 0)
					LOG(PHIDGET_LOG_DEBUG, "Datacount error");

				//Send out some errors - overruns/lost packets
				for (i = 0; i<phid->phid.attr.ifkit.numSensors; i++)
				{
					if(overrunCount[i])
					{
						if(phid->dataSinceAttach >= 10)
						{
							FIRE_ERROR_NOQUEUE(EEPHIDGET_OVERRUN, "Channel %d: %d sample overrun detected.", i, overrunCount[i]);
						}
					}
				}
				if((phid->lastPacketCount >= 0) && ((phid->lastPacketCount+1)&0x03) != packetCount)
				{
					FIRE_ERROR_NOQUEUE(EEPHIDGET_PACKETLOST, "One or more data packets were lost");
				}
				if(overcurrentFlag)
				{
					FIRE_ERROR(EEPHIDGET_OVERCURRENT, "Analog input overcurrent detected.");
				}

				phid->lastPacketCount = packetCount;

				break;
			}
			else
				return EPHIDGET_UNEXPECTED;
		case PHIDID_INTERFACEKIT_0_5_7:
			//Inputs
			for (i = 0, j = 0x01; i<5; i++, j <<= 1)
			{
				if ((buffer[0] & j) == 0)
					inputState[i] = PFALSE;
				else
					inputState[i] = PTRUE;
			}
			break;
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
			//Inputs
			for (i = 0, j = 0x01; i<8; i++, j <<= 1)
			{
				if ((buffer[0] & j) == 0)
					inputState[i] = PFALSE;
				else
					inputState[i] = PTRUE;
			}
			break;
		case PHIDID_INTERFACEKIT_0_0_4:
		case PHIDID_INTERFACEKIT_0_0_8:
			//Outputs
			if (phid->fullStateEcho)
			{
				for (i = 0, j = 0x01; i < phid->phid.attr.ifkit.numOutputs; i++, j <<= 1)
				{
					if ((buffer[0] & j) == 0)
						outputState[i] = PFALSE;
					else
						outputState[i] = PTRUE;
				}
			}
			break;
		case PHIDID_ROTARY_TOUCH:
		case PHIDID_LINEAR_TOUCH:
			//Top bit of buffer[0] indicates a touch
			if (buffer[0] & 0x80) //touch detected
			{
				inputState[0] = PTRUE;
				inputState[1] = PTRUE;

				sensorRawValue[0][0] = buffer[0] & 0x7f;
				sensorDataCount[0]++;
				sensorValue[0][0] = (int)((double)sensorRawValue[0][0] * 7.875);
			}
			else //no touch detected
			{
				//set sensorValue back to unknown
				phid->sensorRawValue[0] = PUNK_INT;
				phid->sensorValue[0] = PUNK_INT;
			
				//Touch
				inputState[0] = PFALSE;

				//Proximity
				if (buffer[0] & 0x01)
					inputState[1] = PTRUE;
				else
					inputState[1] = PFALSE;
			}
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//this is set after the sensor data, so users can poll ratiometric after changing it, to know when to to read sensors.
	if(ratiometricEcho != PUNK_BOOL)
	{
		if(phid->ratiometric == PUNK_BOOL)
			phid->ratiometric = ratiometricEcho;
			
		if(phid->ratiometricSwitching > 0 && ratiometricEcho == phid->ratiometric)
			phid->ratiometricSwitching--;
		if(!phid->ratiometricSwitching)
			phid->ratiometricEcho = ratiometricEcho;
	}
	//Don't do any skipping unless ratiometric state is echoed back
	else
	{
		if(phid->ratiometricSwitching > 0)
			phid->ratiometricSwitching = 0;
	}

	//Make sure values are within defined range, and store to structure
	if(!phid->ratiometricSwitching)
	{
		for (i = 0; i<phid->phid.attr.ifkit.numSensors; i++)
		{
			int rawSensorAccumulate = 0, sensorAccumulate = 0, sensorAccumulateCount = 0;
			for(j=0; j<sensorDataCount[i]; j++)
			{
				if(sensorValue[i][j] != PUNK_INT && sensorRawValue[i][j] != PUNK_INT)
				{
					rawSensorAccumulate += sensorRawValue[i][j];
					if(sensorValue[i][j] < 0) sensorValue[i][j] = 0;
					if(sensorValue[i][j] > 1000) sensorValue[i][j] = 1000;
					sensorAccumulate += sensorValue[i][j];
					sensorAccumulateCount++;
				}
			}
			if(sensorAccumulateCount > 0)
			{
				phid->sensorRawValue[i] = round(rawSensorAccumulate / (double)sensorAccumulateCount);
				phid->sensorValue[i] = round(sensorAccumulate / (double)sensorAccumulateCount);
			}
		}
	}
	for (i = 0; i<phid->phid.attr.ifkit.numInputs; i++)
	{
		if(inputState[i] != PUNK_BOOL)
			phid->physicalState[i] = inputState[i];
	}
	for (i = 0; i<phid->phid.attr.ifkit.numOutputs; i++)
	{
		if(outputState[i] != PUNK_BOOL)
		{
			phid->outputEchoStates[i] = outputState[i];
			CThread_mutex_lock(&phid->phid.outputLock);
			if (phid->outputStates[i] == PUNK_BOOL)
				phid->outputStates[i] = outputState[i];
			CThread_mutex_unlock(&phid->phid.outputLock);
		}
	}

	//send out any events for changed data
	//only if not switching ratiometric
	if(!phid->ratiometricSwitching)
	{
		for (i = 0; i < phid->phid.attr.ifkit.numSensors; i++)
		{	
			if(sensorDataCount[i] > 0)
			{
				switch(phid->eventMode[i])
				{
					case EVENTMODE_CHANGETRIGGER:
						if(phid->sensorValue[i] != PUNK_INT)
						{
							if (abs(phid->sensorLastValue[i] - phid->sensorValue[i]) >= phid->sensorChangeTrigger[i]
								|| (phid->sensorLastValue[i] == PUNK_INT))
							{
								FIRE(SensorChange, i, phid->sensorValue[i]);
								phid->sensorLastValue[i] = phid->sensorValue[i];
							}
						}
						break;
					case EVENTMODE_DATARATE:
						{
							//Case 1: datarate > interrupt rate, just send out all data that came in
							if(phid->dataRate[i] <= phid->interruptRate)
							{
								for(j=0;j<sensorDataCount[i];j++)
								{
									//we still follow the changeTrigger rules
									if (abs(phid->sensorLastValue[i] - phid->sensorValue[i]) >= phid->sensorChangeTrigger[i]
										|| (phid->sensorLastValue[i] == PUNK_INT))
									{
										FIRE(SensorChange, i, sensorValue[i][j]);
										phid->sensorLastValue[i] = sensorValue[i][j];
									}
								}
							}
							//Case 2: data is sent out slower then interrupt rate, so we need to accumulate the data and send it out only sometimes.
							else
							{
								int dataPerEvent = phid->dataRate[i] / phid->interruptRate;
								phid->sensorRawValueAccumulator[i] += phid->sensorValue[i];
								phid->sensorRawValueAccumulatorCount[i]++;
								if(phid->sensorRawValueAccumulatorCount[i] == dataPerEvent)
								{
									int eventVal = round(phid->sensorRawValueAccumulator[i] / (double)phid->sensorRawValueAccumulatorCount[i]);
									//we still follow the changeTrigger rules
									if (abs(phid->sensorLastValue[i] - eventVal) >= phid->sensorChangeTrigger[i]
										|| (phid->sensorLastValue[i] == PUNK_INT))
									{
										FIRE(SensorChange, i, eventVal);
										phid->sensorLastValue[i] = eventVal;
									}
									phid->sensorRawValueAccumulator[i]=0;
									phid->sensorRawValueAccumulatorCount[i]=0;
								}
							}
						}
						break;
					default:
						break;
				}
			}
		}
	}
	for (i = 0; i < phid->phid.attr.ifkit.numInputs; i++)
	{
		if(phid->physicalState[i] != PUNK_BOOL && phid->physicalState[i] != lastInputState[i])
			FIRE(InputChange, i, phid->physicalState[i]);
	}
	for (i = 0; i < phid->phid.attr.ifkit.numOutputs; i++)
	{
		if (phid->outputEchoStates[i] != PUNK_BOOL && phid->outputEchoStates[i] != lastOutputState[i])
			FIRE(OutputChange, i, phid->outputEchoStates[i]);
	}

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(InterfaceKit)

	for (i = 0; i < phid->phid.attr.ifkit.numInputs; i++)
	{
		if (phid->physicalState[i] != PUNK_BOOL)
		{
			FIRE(InputChange, i, phid->physicalState[i]);
		}
	}
	for (i = 0; i < phid->phid.attr.ifkit.numOutputs; i++)
	{
		if (phid->outputEchoStates[i] != PUNK_BOOL)
		{
			FIRE(OutputChange, i, phid->outputEchoStates[i]);
		}
	}
	for (i = 0; i < phid->phid.attr.ifkit.numSensors; i++)
	{
		if (phid->sensorValue[i] != PUNK_INT)
		{
			FIRE(SensorChange, i, phid->sensorValue[i]);
		}
	}

	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET(InterfaceKit)
	int i, j, k;

	CPhidgetInterfaceKitHandle phid = (CPhidgetInterfaceKitHandle)phidG;

	TESTPTRS(phid, buf)
	TESTPTR(lenp)

	if (*lenp < phid->phid.outputReportByteLength)
		return EPHIDGET_INVALIDARG;

	ZEROMEM(buf, *lenp)

	CThread_mutex_lock(&phid->phid.outputLock);

	for (i = 0; i < phid->phid.attr.ifkit.numOutputs; i++)
	{
		//Checks for queued up output changes, and propagates them into the output packet
		if(phid->changedOutputs[i]) {
			phid->outputStates[i] = phid->nextOutputStates[i];
			phid->changedOutputs[i] = 0;
			phid->nextOutputStates[i] = PUNK_BOOL;
		}
		//set unknown outputs to false
		if(phid->outputStates[i] == PUNK_BOOL)
			phid->outputStates[i] = PFALSE;
		//set output echoes to output states for non-echoing devices
		if (!phid->fullStateEcho)
			phid->outputEchoStates[i] = phid->outputStates[i];
	}

	//fill in the buffer and length
	switch(phid->phid.deviceIDSpec) {
		case PHIDID_INTERFACEKIT_4_8_8:
			{
				unsigned const char phid488_lookup[8] = {0x00, 0x01, 0x02, 0x03, 0x07, 0x06, 0x05, 0x04};
				buf[0] = phid488_lookup[phid->lastChangedOutput];
				if (phid->outputStates[phid->lastChangedOutput])
					buf[0] |= 0x08;
			}
			break;
		case PHIDID_INTERFACEKIT_0_16_16:
			for (k = 0; k<2; k++)
			{
				for (i = 0, j = 1; i<8; i++, j <<= 1)
				{
					if (phid->outputStates[k * 8 + i])
						buf[k] |= j;
				}
			}
			break;
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_2_2_2:
			if(phid->ratiometric == PUNK_BOOL)
				phid->ratiometric = PTRUE;
			if ((phid->phid.deviceVersion < 200 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8_w_LCD)
				|| (phid->phid.deviceVersion < 900 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8))
			{
				for (k = 0, j = 1; k<8; k++, j <<= 1)
				{
					if (phid->outputStates[k])
						buf[0] |= j;
				}
				buf[3] = (char)phid->ratiometric;
			}
			//Newer version with dataRate
			else if((phid->phid.deviceVersion >= 900 && phid->phid.deviceVersion < 1000 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8)
				|| (phid->phid.deviceVersion >= 200 && phid->phid.deviceVersion < 300 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8_w_LCD)
				|| (phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_2_2_2))
			{
				for (k = 0, j = 1; k<phid->phid.attr.ifkit.numOutputs; k++, j <<= 1)
				{
					//outputs
					if (phid->outputStates[k])
						buf[0] |= j;
				}
				for (k = 0, j = 1; k<phid->phid.attr.ifkit.numSensors; k++, j <<= 1)
				{
					//datarate
					int datarate = (phid->dataRate[k] > phid->interruptRate ? phid->interruptRate : phid->dataRate[k]);
					datarate--; //so that 8ms fits in 3 bits
					//odd ones are shifted
					if(k%2) datarate <<= 4;
					buf[k/2+1] |= datarate;
				}
				buf[k/2+1] = (char)phid->ratiometric;
			}

			break;
		case PHIDID_INTERFACEKIT_0_0_4:
		case PHIDID_INTERFACEKIT_0_0_8:
			for (k = 0, j = 1; k < phid->phid.attr.ifkit.numOutputs; k++, j <<= 1)
			{
				if (phid->outputStates[k])
					buf[0] |= j;
			}
			break;
		case PHIDID_INTERFACEKIT_0_5_7:
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
			for (i = 0, j=1; i<8; i++, j <<= 1)
			{
				if (phid->outputStates[i])
					buf[0] |= j;
			}
			buf[7] = 0x10;  /* Signal an output */
			break;
		case PHIDID_ROTARY_TOUCH:
		case PHIDID_LINEAR_TOUCH:
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	*lenp = phid->phid.outputReportByteLength;
	
	CThread_mutex_unlock(&phid->phid.outputLock);

	return EPHIDGET_OK;
}

//sendpacket - sends a packet to the device asynchronously, blocking if the 1-packet queue is full
// adds output data to the packet in the queue if possible to combine up to numoutput calls to setoutput into one sent packet
static int CCONV CPhidgetInterfaceKit_sendpacket(CPhidgetInterfaceKitHandle phid,
    unsigned int Index, unsigned int newVal)
{
	int waitReturn;

	CThread_mutex_lock(&phid->phid.writelock);
	phid->lastChangedOutput = Index;

again:
	if (!CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_ATTACHED_FLAG))
	{
		CThread_mutex_unlock(&phid->phid.writelock);
		return EPHIDGET_NOTATTACHED;
	}
	CThread_mutex_lock(&phid->phid.outputLock);
	if (phid->changedOutputs[Index]) {
		if (phid->nextOutputStates[Index] != newVal) {
			CThread_mutex_unlock(&phid->phid.outputLock);
			waitReturn = CThread_wait_on_event(&phid->phid.writtenEvent, 2500);
			switch(waitReturn)
			{
			case WAIT_OBJECT_0:
				break;
			case WAIT_ABANDONED:
				CThread_mutex_unlock(&phid->phid.writelock);
				return EPHIDGET_UNEXPECTED;
			case WAIT_TIMEOUT:
				CThread_mutex_unlock(&phid->phid.writelock);
				return EPHIDGET_TIMEOUT;
			}
			goto again;
		} else {
			CThread_mutex_unlock(&phid->phid.outputLock);
			CThread_mutex_unlock(&phid->phid.writelock);
			return EPHIDGET_OK;
		}
	} else {
		if (phid->outputStates[Index] == newVal) {
			CThread_mutex_unlock(&phid->phid.outputLock);
			CThread_mutex_unlock(&phid->phid.writelock);
			return EPHIDGET_OK;
		}
		phid->changedOutputs[Index] = PTRUE;
		phid->nextOutputStates[Index] = newVal;
		CThread_reset_event(&phid->phid.writtenEvent);
		CThread_mutex_unlock(&phid->phid.outputLock);
		CThread_set_event(&phid->phid.writeAvailableEvent);
	}

	//send output events for devices that don't echo
	if (!(phid->fullStateEcho)) {
		if (phid->outputEchoStates[Index] != (unsigned char)newVal)
		{
			phid->outputEchoStates[Index] = (unsigned char)newVal;
			FIRE(OutputChange, Index, newVal);
		}
	}

	CThread_mutex_unlock(&phid->phid.writelock);

	return EPHIDGET_OK;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(InterfaceKit, PHIDCLASS_INTERFACEKIT)

//event setup functions
CFHANDLE(InterfaceKit, InputChange, int, int)
CFHANDLE(InterfaceKit, OutputChange, int, int)
CFHANDLE(InterfaceKit, SensorChange, int, int)

CGET(InterfaceKit, InputCount, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED

	MASGN(phid.attr.ifkit.numInputs)
}

CGETINDEX(InterfaceKit, InputState, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED
	TESTINDEX(phid.attr.ifkit.numInputs)
	TESTMASGN(physicalState[Index], PUNK_BOOL)

	MASGN(physicalState[Index])
}

CGET(InterfaceKit, OutputCount, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED

	MASGN(phid.attr.ifkit.numOutputs)
}

CGETINDEX(InterfaceKit, OutputState, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED
	TESTINDEX(phid.attr.ifkit.numOutputs)
	TESTMASGN(outputEchoStates[Index], PUNK_BOOL)

	MASGN(outputEchoStates[Index])
}
CSETINDEX(InterfaceKit, OutputState, int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED
	TESTRANGE(PFALSE, PTRUE)
	TESTINDEX(phid.attr.ifkit.numOutputs)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Output, "%d", nextOutputStates);
	else
		return CPhidgetInterfaceKit_sendpacket(phid, Index, newVal);

	return EPHIDGET_OK;
}

CGET(InterfaceKit, SensorCount, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED

	MASGN(phid.attr.ifkit.numSensors)
}

CGETINDEX(InterfaceKit, SensorValue, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED
	TESTINDEX(phid.attr.ifkit.numSensors)
	TESTMASGN(sensorValue[Index], PUNK_INT)

	MASGN(sensorValue[Index])
}

CGETINDEX(InterfaceKit, SensorRawValue, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED
	TESTINDEX(phid.attr.ifkit.numSensors)
	TESTMASGN(sensorRawValue[Index], PUNK_INT)

	MASGN(sensorRawValue[Index])
}

CGETINDEX(InterfaceKit, SensorChangeTrigger, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED
	TESTINDEX(phid.attr.ifkit.numSensors)
	TESTMASGN(sensorChangeTrigger[Index], PUNK_INT)

	MASGN(sensorChangeTrigger[Index])
}
CSETINDEX(InterfaceKit,SensorChangeTrigger,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED
	TESTINDEX(phid.attr.ifkit.numSensors)
	TESTRANGE(0, 1000)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Trigger, "%d", sensorChangeTrigger);
	else
	{
		//setting this to 0 is usually done when data rate mode is wanted
		if(newVal != 0)
			phid->eventMode[Index] = EVENTMODE_CHANGETRIGGER;
		phid->sensorChangeTrigger[Index] = newVal;
	}

	return EPHIDGET_OK;
}

CGET(InterfaceKit, Ratiometric, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_2_2_2:
			if ((phid->phid.deviceVersion < 200 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8_w_LCD)
				|| (phid->phid.deviceVersion < 900 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8))
			{
				TESTMASGN(ratiometric, PUNK_BOOL)
				MASGN(ratiometric)
			}
			else if((phid->phid.deviceVersion >= 900 && phid->phid.deviceVersion < 1000 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8)
				|| (phid->phid.deviceVersion >= 200 && phid->phid.deviceVersion < 300 && phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_8_8_8_w_LCD)
				|| (phid->phid.deviceIDSpec == PHIDID_INTERFACEKIT_2_2_2))
			{
				TESTMASGN(ratiometricEcho, PUNK_BOOL)
				MASGN(ratiometricEcho)
			}
			else
				return EPHIDGET_UNEXPECTED;
		case PHIDID_INTERFACEKIT_0_5_7:
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_4_8_8:
		case PHIDID_INTERFACEKIT_0_0_4:
		case PHIDID_INTERFACEKIT_0_0_8:
		case PHIDID_INTERFACEKIT_0_16_16:
		case PHIDID_ROTARY_TOUCH:
		case PHIDID_LINEAR_TOUCH:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}
}
CSET(InterfaceKit, Ratiometric, int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_2_2_2:
			TESTRANGE(PFALSE, PTRUE)

			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
				ADDNETWORKKEY(Ratiometric, "%d", ratiometric);
			else
			{
				//this just signals the write thread that a write is available
				CThread_mutex_lock(&phid->phid.writelock);
				CThread_mutex_lock(&phid->phid.outputLock);
				phid->ratiometric = newVal;
				phid->ratiometricSwitching = 2; //this causes one data packet to be skipped for devices that echo ratiometric
				CThread_mutex_unlock(&phid->phid.outputLock);
				CThread_set_event(&phid->phid.writeAvailableEvent);
				CThread_mutex_unlock(&phid->phid.writelock);
			}
			break;
		case PHIDID_INTERFACEKIT_0_5_7:
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_4_8_8:
		case PHIDID_INTERFACEKIT_0_0_4:
		case PHIDID_INTERFACEKIT_0_0_8:
		case PHIDID_INTERFACEKIT_0_16_16:
		case PHIDID_ROTARY_TOUCH:
		case PHIDID_LINEAR_TOUCH:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	return EPHIDGET_OK;
}


CSETINDEX(InterfaceKit,DataRate,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_2_2_2:
			{
				int i, dataPerPacket = 0;
				TESTINDEX(phid.attr.ifkit.numSensors)
				TESTRANGE(phid->dataRateMax, phid->dataRateMin)

				//make sure it's a power of 2, or 1
				if(newVal < phid->interruptRate)
				{
					int temp = phid->dataRateMax;
					unsigned char good = FALSE;
					while(temp <= newVal)
					{
						if(temp == newVal)
						{
							good = TRUE;
							break;
						}
						temp *= 2;
					}
					if(!good)
						return EPHIDGET_INVALIDARG;
				}
				//make sure it's divisible by interruptRate
				else
				{
					if(newVal%phid->interruptRate)
						return EPHIDGET_INVALIDARG;
				}

				//make sure we're not asking for too much data per packet
				for(i=0;i<phid->phid.attr.ifkit.numSensors;i++)
				{
					if(i==Index)
						dataPerPacket+=phid->interruptRate/newVal;
					else
						dataPerPacket+=phid->interruptRate/phid->dataRate[i];
				}
				if(dataPerPacket > phid->maxDataPerPacket)
				{
					return EPHIDGET_INVALIDARG;
				}

				if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
					ADDNETWORKKEYINDEXED(DataRate, "%d", dataRate);
				else
				{
					//this just signals the write thread that a write is available
					CThread_mutex_lock(&phid->phid.writelock);
					CThread_mutex_lock(&phid->phid.outputLock);
					phid->eventMode[Index] = EVENTMODE_DATARATE;
					phid->dataRate[Index] = newVal;
					phid->sensorRawValueAccumulator[Index] = 0;
					phid->sensorRawValueAccumulatorCount[Index] = 0;
					CThread_mutex_unlock(&phid->phid.outputLock);
					CThread_set_event(&phid->phid.writeAvailableEvent);
					CThread_mutex_unlock(&phid->phid.writelock);
				}
			}
			break;
		case PHIDID_INTERFACEKIT_0_5_7:
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_4_8_8:
		case PHIDID_INTERFACEKIT_0_0_4:
		case PHIDID_INTERFACEKIT_0_0_8:
		case PHIDID_INTERFACEKIT_0_16_16:
		case PHIDID_ROTARY_TOUCH:
		case PHIDID_LINEAR_TOUCH:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	return EPHIDGET_OK;
}
CGETINDEX(InterfaceKit,DataRate,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_2_2_2:
			TESTINDEX(phid.attr.ifkit.numSensors)
			TESTMASGN(dataRate[Index], PUNK_INT)
			MASGN(dataRate[Index])
		case PHIDID_INTERFACEKIT_0_5_7:
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_4_8_8:
		case PHIDID_INTERFACEKIT_0_0_4:
		case PHIDID_INTERFACEKIT_0_0_8:
		case PHIDID_INTERFACEKIT_0_16_16:
		case PHIDID_ROTARY_TOUCH:
		case PHIDID_LINEAR_TOUCH:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}
}

CGETINDEX(InterfaceKit,DataRateMax,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_2_2_2:
			TESTINDEX(phid.attr.ifkit.numSensors)
			TESTMASGN(dataRateMax, PUNK_INT)
			MASGN(dataRateMax)
		case PHIDID_INTERFACEKIT_0_5_7:
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_4_8_8:
		case PHIDID_INTERFACEKIT_0_0_4:
		case PHIDID_INTERFACEKIT_0_0_8:
		case PHIDID_INTERFACEKIT_0_16_16:
		case PHIDID_ROTARY_TOUCH:
		case PHIDID_LINEAR_TOUCH:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}
}

CGETINDEX(InterfaceKit,DataRateMin,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_INTERFACEKIT)
	TESTATTACHED
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_INTERFACEKIT_8_8_8:
		case PHIDID_INTERFACEKIT_8_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_2_2_2:
			TESTINDEX(phid.attr.ifkit.numSensors)
			TESTMASGN(dataRateMin, PUNK_INT)
			MASGN(dataRateMin)
		case PHIDID_INTERFACEKIT_0_5_7:
		case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
		case PHIDID_INTERFACEKIT_4_8_8:
		case PHIDID_INTERFACEKIT_0_0_4:
		case PHIDID_INTERFACEKIT_0_0_8:
		case PHIDID_INTERFACEKIT_0_16_16:
		case PHIDID_ROTARY_TOUCH:
		case PHIDID_LINEAR_TOUCH:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}
}

// === Deprecated Functions === //

CGET(InterfaceKit, NumInputs, int)
	return CPhidgetInterfaceKit_getInputCount(phid, pVal);
}
CGET(InterfaceKit, NumOutputs, int)
	return CPhidgetInterfaceKit_getOutputCount(phid, pVal);
}
CGET(InterfaceKit, NumSensors, int)
	return CPhidgetInterfaceKit_getSensorCount(phid, pVal);
}
