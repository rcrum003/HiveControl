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
#include "cphidgetencoder.h"
#include <stdio.h>
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(Encoder)
	int i = 0;
	for (i = 0; i<ENCODER_MAXINPUTS; i++)
	{
		phid->inputState[i] = PUNK_BOOL;
	}
	for (i = 0; i<ENCODER_MAXENCODERS; i++)
	{
		phid->encoderPosition[i] = 0;
		phid->encoderTimeStamp[i] = PUNK_INT;
		phid->indexPosition[i] = PUNK_INT;
		phid->enableStateEcho[i] = PUNI_BOOL;
	}
	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(Encoder)
	int i = 0;
	unsigned char buffer[8] = { 0 };
	int result;

	TESTPTR(phid);

	//Make sure no old writes are still pending
	phid->outputPacketLen = 0;

	//set data arrays to unknown, initial states
	for (i = 0; i<phid->phid.attr.encoder.numInputs; i++)
	{
		phid->inputState[i] = PUNK_BOOL;
	}
	for (i = 0; i<phid->phid.attr.encoder.numEncoders; i++)
	{
		phid->encoderPosition[i] = 0; //position set to zero on each attach
		phid->encoderTimeStamp[i] = PUNK_INT;
		phid->indexPosition[i] = PUNK_INT;
		switch(phid->phid.deviceIDSpec) {
			case PHIDID_ENCODER_HS_4ENCODER_4INPUT:
				phid->enableState[i] = PUNK_BOOL;
				phid->enableStateEcho[i] = PUNK_BOOL;
				break;
			case PHIDID_ENCODER_1ENCODER_1INPUT:
			case PHIDID_ENCODER_HS_1ENCODER:
				phid->enableState[i] = PTRUE;
				phid->enableStateEcho[i] = PTRUE;
				break;
			default:
				break;
		}
	}

	//send out any initial pre-read packets
	switch(phid->phid.deviceIDSpec) {
		case PHIDID_ENCODER_1ENCODER_1INPUT:
			if (phid->phid.deviceVersion <= 100)
			{
				ZEROMEM(buffer,8);
				LOG(PHIDGET_LOG_INFO,"Sending workaround startup packet");
				if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
					return result;
			}
			break;
		case PHIDID_ENCODER_HS_1ENCODER:
		case PHIDID_ENCODER_HS_4ENCODER_4INPUT:
		default:
			break;
	}

	//issue a read on the 4-input HS to get enable states
	switch(phid->phid.deviceIDSpec) {
		case PHIDID_ENCODER_HS_4ENCODER_4INPUT:
			if (phid->phid.deviceVersion >= 100 && phid->phid.deviceVersion < 200)
			{
				CPhidget_read((CPhidgetHandle)phid);
			}
			break;
		case PHIDID_ENCODER_1ENCODER_1INPUT:
		case PHIDID_ENCODER_HS_1ENCODER:
		default:
			break;
	}

	//fill in enabled states
	for (i = 0; i<phid->phid.attr.encoder.numEncoders; i++)
	{
		phid->enableState[i] = phid->enableStateEcho[i];
	}

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(Encoder)
	int i=0,j=0;

	unsigned char input[ENCODER_MAXINPUTS];
	unsigned char indexTrue[ENCODER_MAXINPUTS];
	unsigned char enabledEcho[ENCODER_MAXINPUTS];
	short positionChange[ENCODER_MAXENCODERS];
	short indexChange[ENCODER_MAXENCODERS];
	int packetTime[ENCODER_MAXENCODERS];
	int curTime[ENCODER_MAXENCODERS];
	unsigned short timeChange = 0, timeStamp = 0;
	int timeChangeInt[ENCODER_MAXENCODERS];

	unsigned char lastInputState[ENCODER_MAXINPUTS];

	if (length<0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	ZEROMEM(input, sizeof(input));
	ZEROMEM(positionChange, sizeof(positionChange));
	ZEROMEM(lastInputState, sizeof(lastInputState));
	ZEROMEM(indexTrue, sizeof(indexTrue));
	ZEROMEM(indexChange, sizeof(indexChange));
	ZEROMEM(packetTime, sizeof(packetTime));
	ZEROMEM(curTime, sizeof(curTime));
	ZEROMEM(timeChangeInt, sizeof(timeChangeInt));

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_ENCODER_1ENCODER_1INPUT:
			/* OLD version here... */
			if (phid->phid.deviceVersion < 110)
			{
				if (buffer[1] & 0x04)
					input[0] = PFALSE;
				else
					input[0] = PTRUE;

				positionChange[0] = (signed char)buffer[2];

				timeStamp = (((unsigned short)buffer[4]) << 8) + buffer[3];
			}
			/* NEW version 1-encoder = 1.10+*/
			else if (phid->phid.deviceVersion >= 110 && phid->phid.deviceVersion < 300)
			{
				if (buffer[1] & 0x01)
					input[0] = PFALSE;
				else
					input[0] = PTRUE;
				
				positionChange[0] = (signed char)buffer[4];

				timeStamp = (((unsigned short)buffer[3]) << 8) + buffer[2];
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_ENCODER_HS_1ENCODER:
			/* high speed encoder */
			if (phid->phid.deviceVersion >= 300 && phid->phid.deviceVersion < 400) {

				//this will work for 8 inputs before we need to change the protocol
				//currently no high-speed encoder has any inputs
				for (i = 0, j = 1; i < (phid->phid.attr.encoder.numInputs); i++, j<<=1)
				{
					if (buffer[1] & j)
						input[i] = PFALSE;
					else
						input[i] = PTRUE;
				}
				
				//this will work for two encoders before we need to change the protocol
				for(i=0;i<phid->phid.attr.encoder.numEncoders;i++) {
					positionChange[i] = (((unsigned short)buffer[2*i+5]) << 8) + buffer[2*i+4];
				}

				timeStamp = (((unsigned short)buffer[3]) << 8) + buffer[2];
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_ENCODER_HS_4ENCODER_4INPUT:
			/* high speed encoder 4 input, with enable and index */
			if (phid->phid.deviceVersion >= 100 && phid->phid.deviceVersion < 200)
			{
				//this will work for two encoders before we need to change the protocol
				for(i=0;i<phid->phid.attr.encoder.numEncoders;i++) {
					positionChange[i] = (((unsigned short)buffer[10*i+0]) << 8) + buffer[10*i+1] - 0x7fff;
					indexChange[i] = (((unsigned short)buffer[10*i+2]) << 8) + buffer[10*i+3] - 0x7fff;
					packetTime[i] = (((int)buffer[10*i+4]) << 16) + (((unsigned short)buffer[10*i+5]) << 8) + buffer[10*i+6];
					curTime[i] = (((unsigned short)buffer[10*i+7]) << 8) + buffer[10*i+8];
					indexTrue[i] = (buffer[10*i+9] & 0x01) ? PTRUE : PFALSE;
					enabledEcho[i] = (buffer[10*i+9] & 0x02) ? PTRUE : PFALSE;
					if (buffer[10*i+9] & 0x04)
						input[i] = PFALSE;
					else
						input[i] = PTRUE;
				}
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//Make sure values are within defined range, and store to structure
	for (i = 0; i < phid->phid.attr.encoder.numInputs; i++)
	{
		lastInputState[i] = phid->inputState[i];
		phid->inputState[i] = input[i];
	}
	for(i=0; i < phid->phid.attr.encoder.numEncoders; i++) 
	{
		//check for over/undershoots
		if((positionChange[i] > 0 && (phid->encoderPosition[i] + positionChange[i]) < phid->encoderPosition[i])
			|| (positionChange[i] < 0 && (phid->encoderPosition[i] + positionChange[i]) > phid->encoderPosition[i]))
		{
			char error_buffer[50];
			FIRE_ERROR(EEPHIDGET_WRAP, "Encoder %d position is wrapping around.", i);
		}

		phid->encoderPosition[i] += positionChange[i];

		//Different timing stuff for different devices
		switch(phid->phid.deviceIDSpec)
		{
			case PHIDID_ENCODER_1ENCODER_1INPUT:
			case PHIDID_ENCODER_HS_1ENCODER:
				//this handles wraparounds because we're using unsigned shorts
				timeChange = (timeStamp - phid->encoderTimeStamp[i]);

				//TODO: this misses the case where timeChange > 65 seconds - we would need to add timing to the library
				if (timeChange > 30000 || phid->encoderTimeStamp[i] == PUNK_INT)
					timeChangeInt[i] = PUNK_INT;
				else
					timeChangeInt[i] = timeChange;

				phid->encoderTimeStamp[i] = timeStamp;
				break;
			case PHIDID_ENCODER_HS_4ENCODER_4INPUT:
				//timing
				if (phid->encoderTimeStamp[i] == PUNK_INT)
					phid->encoderTimeStamp[i] = 0;

				phid->encoderTimeStamp[i] += packetTime[i];

				if(positionChange[i] != 0)
				{
					timeChangeInt[i] = phid->encoderTimeStamp[i] - curTime[i];
					// convert 1/12,000,000 to microseconds
					timeChangeInt[i] = round(timeChangeInt[i] / 12.0);

					phid->encoderTimeStamp[i] = curTime[i];
				}
				//index
				if(indexTrue[i])
					phid->indexPosition[i] = phid->encoderPosition[i] - indexChange[i];
				//enabled echo
				phid->enableStateEcho[i] = enabledEcho[i];
				break;
			default:
				return EPHIDGET_UNEXPECTED;
		}
	}

	//send out any events for changed data
	for (i = 0; i < phid->phid.attr.encoder.numInputs; i++)
	{
		if(phid->inputState[i] != lastInputState[i])
			FIRE(InputChange, i, phid->inputState[i]);
	}
	for(i=0; i < phid->phid.attr.encoder.numEncoders; i++) 
	{
		if(positionChange[i] != 0)
			FIRE(PositionChange, i, timeChangeInt[i], positionChange[i]);
		if(indexTrue[i])
			FIRE(Index, i, phid->indexPosition[i]);
	}

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(Encoder)

	for (i = 0; i < phid->phid.attr.encoder.numInputs; i++)
	{
		if (phid->inputState[i] != PUNK_BOOL)
		{
			FIRE(InputChange, i, phid->inputState[i]);
		}
	}
	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET_BUF(Encoder)

//sendpacket - sends a packet to the device asynchronously, blocking if the 1-packet queue is full
CSENDPACKET_BUF(Encoder)

//makePacket - constructs a packet using current device state
CMAKEPACKET(Encoder)
	int i = 0, j = 0;

	TESTPTRS(phid, buffer);

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_ENCODER_HS_4ENCODER_4INPUT:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion  < 200))
			{
				//have to make sure that everything to be sent has some sort of default value if the user hasn't set a value
				for (i = 0; i<phid->phid.attr.encoder.numEncoders; i++)
				{
					if (phid->enableState[i] == PUNK_BOOL)
						phid->enableState[i] = PFALSE;
				}

				//construct the packet
				for (i = 0, j = 1; i < phid->phid.attr.encoder.numEncoders; i++, j<<=1)
				{
					if (phid->enableState[i])
						buffer[0] |= j;
				}
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_ENCODER_HS_1ENCODER:
		case PHIDID_ENCODER_1ENCODER_1INPUT:
			return EPHIDGET_UNSUPPORTED; //these versions don't have OUT packets
		default:
			return EPHIDGET_UNEXPECTED;
	}
	return EPHIDGET_OK;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(Encoder, PHIDCLASS_ENCODER)

//event setup functions
CFHANDLE(Encoder, InputChange, int, int)
CFHANDLE(Encoder, PositionChange, int, int, int)
CFHANDLE(Encoder, Index, int, int)

CGET(Encoder,InputCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ENCODER)
	TESTATTACHED

	MASGN(phid.attr.encoder.numInputs)
}

CGETINDEX(Encoder,InputState,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ENCODER)
	TESTATTACHED
	TESTINDEX(phid.attr.encoder.numInputs)
	TESTMASGN(inputState[Index], PUNK_BOOL)

	MASGN(inputState[Index])
}

CGET(Encoder,EncoderCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ENCODER)
	TESTATTACHED

	MASGN(phid.attr.encoder.numEncoders)
}

CGETINDEX(Encoder,Position,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ENCODER)
	TESTATTACHED
	TESTINDEX(phid.attr.encoder.numEncoders)

	MASGN(encoderPosition[Index])
}
CSETINDEX(Encoder,Position,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ENCODER)
	TESTATTACHED
	TESTINDEX(phid.attr.encoder.numEncoders)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(ResetPosition, "%d", encoderPosition);
	else
		phid->encoderPosition[Index] = newVal;

	return EPHIDGET_OK;
}

CGETINDEX(Encoder,IndexPosition,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ENCODER)
	TESTATTACHED
	TESTINDEX(phid.attr.encoder.numEncoders)
	TESTMASGN(indexPosition[Index], PUNK_INT)

	MASGN(indexPosition[Index])
}

CGETINDEX(Encoder,Enabled,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ENCODER)
	TESTATTACHED
	TESTINDEX(phid.attr.encoder.numEncoders)
	TESTMASGN(enableStateEcho[Index], PUNK_BOOL)

	MASGN(enableStateEcho[Index])
}
CSETINDEX(Encoder,Enabled,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ENCODER)
	TESTATTACHED
	
	//Only supported on the new 4-encoder board
	switch(phid->phid.deviceIDSpec) {
		case PHIDID_ENCODER_HS_4ENCODER_4INPUT:
			TESTINDEX(phid.attr.encoder.numEncoders)
			TESTRANGE(PFALSE, PTRUE)
			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
				ADDNETWORKKEYINDEXED(Enabled, "%d", enableState);
			else
				SENDPACKET(Encoder, enableState[Index]);
			return EPHIDGET_OK;
		case PHIDID_ENCODER_1ENCODER_1INPUT:
		case PHIDID_ENCODER_HS_1ENCODER:
		default:
			return EPHIDGET_UNSUPPORTED;
	}

}

// === Deprecated Functions === //

CGETINDEX(Encoder,EncoderPosition,int)
	return CPhidgetEncoder_getPosition(phid, Index, pVal);
}
CSETINDEX(Encoder,EncoderPosition,int)
	return CPhidgetEncoder_setPosition(phid, Index, newVal);
}
CGET(Encoder,NumInputs,int)
	return CPhidgetEncoder_getInputCount(phid, pVal);
}
CGET(Encoder,NumEncoders,int)
	return CPhidgetEncoder_getEncoderCount(phid, pVal);
}
