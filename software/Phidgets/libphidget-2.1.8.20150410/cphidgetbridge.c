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
#include "cphidgetbridge.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(Bridge)
	int i = 0;

	for (i = 0; i<BRIDGE_MAXINPUTS; i++)
	{
		phid->bridgeMin[i] = PUNI_DBL;
		phid->bridgeMax[i] = PUNI_DBL;
		phid->enabledEcho[i] = PUNI_BOOL;
		phid->gainEcho[i] = PHIDGET_BRIDGE_GAIN_UNKNOWN;
		phid->bridgeValue[i] = PUNI_DBL;

		phid->enabled[i] = PUNK_BOOL;
		phid->gain[i] = PHIDGET_BRIDGE_GAIN_UNKNOWN;
		phid->chEnabledBugNotValid[i] = PFALSE;
	}
	phid->ch0EnableOverride = 0;
	phid->dataRateEcho = PUNI_INT;
	phid->dataRateMin = PUNI_INT;
	phid->dataRateMax = PUNI_INT;

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(Bridge)
	int i;
	TESTPTR(phid);

	//Make sure no old writes are still pending
	phid->outputPacketLen = 0;

	//Setup max/min values
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_BRIDGE_4INPUT:
			switch(phid->phid.deviceUID)
			{
				case PHIDUID_BRIDGE_4INPUT_GAINBUG:
				case PHIDUID_BRIDGE_4INPUT:
					phid->dataRateMin = 1000;
					phid->dataRateMax = 8;
					break;
				default:
					return EPHIDGET_BADVERSION;
			}
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	for (i = 0; i<phid->phid.attr.bridge.numBridgeInputs; i++)
	{
		phid->bridgeMin[i] = PUNK_DBL;
		phid->bridgeMax[i] = PUNK_DBL;
		phid->bridgeValue[i] = PUNK_DBL;
		phid->enabledEcho[i] = PUNK_BOOL;
		phid->gainEcho[i] = PHIDGET_BRIDGE_GAIN_UNKNOWN;
		phid->outOfRange[i] = PFALSE;
		phid->lastOutOfRange[i] = PFALSE;
		phid->chEnabledBugNotValid[i] = PFALSE;
	}
	phid->ch0EnableOverride = 0;
	phid->dataRateEcho = PUNK_INT;

	//read in initial state - read until it returns 0
	while(CPhidget_read((CPhidgetHandle)phid)) SLEEP(8);

	//recover what we can, set others to unknown
	for (i = 0; i<phid->phid.attr.bridge.numBridgeInputs; i++)
	{
		phid->enabled[i] = phid->enabledEcho[i];
		phid->gain[i] = phid->gainEcho[i];
	}
	phid->dataRate = phid->dataRateEcho;

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(Bridge)
	int i = 0;
	int bridgeRawVal;
	double bridgeVal;
	int dataRateRaw;
	double gain;
	int roundingFactor = 0;
	unsigned char err = PFALSE;
	char error_buffer[200];
	double bridgeval[BRIDGE_MAXINPUTS];

	if (length < 0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_BRIDGE_4INPUT_GAINBUG:
		case PHIDUID_BRIDGE_4INPUT:
			{
				//Output Data Rate (Hz) = dataRateRaw * 8
				dataRateRaw = ((buffer[4] & 0x03) << 8) + buffer[5];
				//in ms
				phid->dataRateEcho = dataRateRaw * 8;

				for (i = 0; i<phid->phid.attr.bridge.numBridgeInputs; i++)
				{
					//gain
					switch(buffer[(i/2)+2] >> (i%2?0:4) & 0x0F)
					{
						case 0x00:
							phid->gainEcho[i] = PHIDGET_BRIDGE_GAIN_1;
							gain = 1;
							roundingFactor = 5;
							break;
						case 0x03:
							phid->gainEcho[i] = PHIDGET_BRIDGE_GAIN_8;
							gain = 8;
							roundingFactor = 6;
							break;
						case 0x04:
							phid->gainEcho[i] = PHIDGET_BRIDGE_GAIN_16;
							gain = 16;
							roundingFactor = 7;
							break;
						case 0x05:
							phid->gainEcho[i] = PHIDGET_BRIDGE_GAIN_32;
							gain = 32;
							roundingFactor = 7;
							break;
						case 0x06:
							phid->gainEcho[i] = PHIDGET_BRIDGE_GAIN_64;
							gain = 64;
							roundingFactor = 7;
							break;
						case 0x07:
							phid->gainEcho[i] = PHIDGET_BRIDGE_GAIN_128;
							gain = 128;
							roundingFactor = 8;
							break;
					}

					if(gain == 1)
					{
						phid->bridgeMax[i] = 1000;
						phid->bridgeMin[i] = -1000;
					}
					else
					{
						//with PGA, allow +-99.5%, because if doesn't clamp properly
						phid->bridgeMax[i] = 995/gain;
						phid->bridgeMin[i] = -995/gain;
					}

					phid->enabledEcho[i] = (buffer[0] & (0x01 << i)) ? PTRUE : PFALSE;
					if(phid->enabledEcho[i] && (buffer[0] & (0x10 << i))) //enabled and have data
					{
						//Don't count data this time - wait for next event
						if(phid->chEnabledBugNotValid[i] == PTRUE)
						{
							bridgeval[i] = PUNK_DBL;
							phid->chEnabledBugNotValid[i] = PFALSE;
						}
						else
						{
							bridgeRawVal = (buffer[i*3 + 6] << 16) + (buffer[i*3 + 7] << 8) + buffer[i*3 + 8];
							bridgeVal = round_double(((bridgeRawVal - 0x800000) / (double)0x800000) * 1000.0 / (double)gain, roundingFactor);

							err = (buffer[1] & (0x01 << i)) ? PTRUE : PFALSE;

							if(err || bridgeVal > phid->bridgeMax[i] || bridgeVal < phid->bridgeMin[i])
							{
								bridgeval[i] = PUNK_DBL;
								
								if(!phid->lastOutOfRange[i])
								{
									if(bridgeVal > phid->bridgeMax[i])
									{
										FIRE_ERROR(EEPHIDGET_OUTOFRANGE, "Overrange condition detected on input %d, try lowering the gain.",i);
									}
									else if(bridgeVal < phid->bridgeMin[i])
									{
										FIRE_ERROR(EEPHIDGET_OUTOFRANGE, "Underrange condition detected on input %d, try lowering the gain.",i);
									}
								}
								phid->lastOutOfRange[i] = PTRUE;
							}
							else
							{
								if(phid->lastOutOfRange[i])
								{
									FIRE_ERROR(EEPHIDGET_OUTOFRANGE, "Input %d out of range state ended.", i);
								}
								phid->lastOutOfRange[i] = PFALSE;

								bridgeval[i] = bridgeVal;
							}
						}
					}
					else if(!phid->enabledEcho[i])
					{
						bridgeval[i] = PUNK_DBL;
					}
					else
						bridgeval[i] = PUNI_DBL;

					//decrement until 0 - this is related to a firmware bug
					if(phid->ch0EnableOverride)
						phid->ch0EnableOverride--;
				}
			}
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	for (i = 0; i<phid->phid.attr.bridge.numBridgeInputs; i++)
	{
		if (bridgeval[i] != PUNI_DBL)
		{
			phid->bridgeValue[i] = bridgeval[i];
		}
	}

	//Send out events
	for (i = 0; i<phid->phid.attr.bridge.numBridgeInputs; i++)
	{
		if (bridgeval[i] != PUNI_DBL &&phid->bridgeValue[i] != PUNK_DBL)
		{
			FIRE(BridgeData, i, phid->bridgeValue[i]);
		}
	}

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(Bridge)

	for (i = 0; i<phid->phid.attr.bridge.numBridgeInputs; i++)
	{
		if (phid->bridgeValue[i] != PUNK_DBL)
		{
			FIRE(BridgeData, i, phid->bridgeValue[i]);
		}
	}

	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET_BUF(Bridge)

//sendpacket - sends a packet to the device asynchronously, blocking if the 1-packet queue is full
CSENDPACKET_BUF(Bridge)

//makePacket - constructs a packet using current device state
CMAKEPACKET(Bridge)
	TESTPTRS(phid, buffer);

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_BRIDGE_4INPUT_GAINBUG:
		case PHIDUID_BRIDGE_4INPUT:
			{
				int i;
				int dataRateRaw;
				int gainRaw;

				//ms->raw
				dataRateRaw = phid->dataRate / 8;

				buffer[0] = 0;
				buffer[1] = 0;
				buffer[2] = 0;
				for (i = 0; i<phid->phid.attr.bridge.numBridgeInputs; i++)
				{
					//Deal with some firmware bugs
					if(phid->phid.deviceUID == PHIDUID_BRIDGE_4INPUT_GAINBUG)
					{
						if(phid->gain[i] != phid->gainEcho[i])
						{
							//anytime we change gain on any channel, we have to enable channel 0, and leave it enabled for at least 250ms
							phid->ch0EnableOverride = 256 / 8; //USB interval is 8ms, this is decremented by one each DATA packet until it gets back to 0.
						}

						if(phid->enabledEcho[i] == PFALSE && phid->enabled[i] == PTRUE)
						{
							//1st data after a channel is enabled can be bad, so we just ignore this.
							phid->chEnabledBugNotValid[i] = PTRUE;
						}
					}

					if(phid->enabled[i])
						buffer[0] |= (0x01<<i);

					switch(phid->gain[i])
					{
						case PHIDGET_BRIDGE_GAIN_1:
						case PHIDGET_BRIDGE_GAIN_UNKNOWN:
							gainRaw = 0x00;
							break;
						case PHIDGET_BRIDGE_GAIN_8:
							gainRaw = 0x03;
							break;
						case PHIDGET_BRIDGE_GAIN_16:
							gainRaw = 0x04;
							break;
						case PHIDGET_BRIDGE_GAIN_32:
							gainRaw = 0x05;
							break;
						case PHIDGET_BRIDGE_GAIN_64:
							gainRaw = 0x06;
							break;
						case PHIDGET_BRIDGE_GAIN_128:
							gainRaw = 0x07;
							break;
					}
					buffer[(i/2)+1] |= gainRaw << (i%2?0:4);
				}

				//Deal with some firmware bugs
				if(phid->phid.deviceUID == PHIDUID_BRIDGE_4INPUT_GAINBUG && phid->ch0EnableOverride)
				{
					//override and enable ch0
					buffer[0] |= 0x01;
				}

				buffer[3] = dataRateRaw & 0xFF;
				buffer[4] = (dataRateRaw >> 8) & 0x03;

			}
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	return EPHIDGET_OK;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(Bridge, PHIDCLASS_BRIDGE)

//event setup functions
CFHANDLE(Bridge, BridgeData, int, double)

CGET(Bridge,InputCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED

	MASGN(phid.attr.bridge.numBridgeInputs)
}

CGETINDEX(Bridge,BridgeValue,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTINDEX(phid.attr.bridge.numBridgeInputs)
	TESTMASGN(bridgeValue[Index], PUNK_DBL)

	MASGN(bridgeValue[Index])
}

CGETINDEX(Bridge,BridgeMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTINDEX(phid.attr.bridge.numBridgeInputs)
	TESTMASGN(bridgeMax[Index], PUNK_DBL)

	MASGN(bridgeMax[Index])
}

CGETINDEX(Bridge,BridgeMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTINDEX(phid.attr.bridge.numBridgeInputs)
	TESTMASGN(bridgeMin[Index], PUNK_DBL)

	MASGN(bridgeMin[Index])
}

CGETINDEX(Bridge,Enabled,int)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTINDEX(phid.attr.bridge.numBridgeInputs)
	TESTMASGN(enabledEcho[Index], PUNK_BOOL)

	MASGN(enabledEcho[Index])
}
CSETINDEX(Bridge,Enabled,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTRANGE(PFALSE, PTRUE)
	TESTINDEX(phid.attr.bridge.numBridgeInputs)

	if(newVal == PFALSE) phid->bridgeValue[Index] = PUNK_DBL;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Enabled, "%d", enabled);
	else
		SENDPACKET(Bridge, enabled[Index]);

	return EPHIDGET_OK;
}

CGETINDEX(Bridge,Gain,CPhidgetBridge_Gain)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTINDEX(phid.attr.bridge.numBridgeInputs)
	TESTMASGN(gainEcho[Index], PHIDGET_BRIDGE_GAIN_UNKNOWN)

	MASGN(gainEcho[Index])
}
CSETINDEX(Bridge,Gain,CPhidgetBridge_Gain)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTRANGE(PHIDGET_BRIDGE_GAIN_1, PHIDGET_BRIDGE_GAIN_128)
	TESTINDEX(phid.attr.bridge.numBridgeInputs)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Gain, "%d", gain);
	else
		SENDPACKET(Bridge, gain[Index]);

	return EPHIDGET_OK;
}

CSET(Bridge,DataRate,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTRANGE(phid->dataRateMax, phid->dataRateMin)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEY(DataRate, "%d", dataRate);
	else
		SENDPACKET(Bridge, dataRate);

	return EPHIDGET_OK;
}
CGET(Bridge,DataRate,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTMASGN(dataRateEcho, PUNK_INT)

	MASGN(dataRateEcho)
}

CGET(Bridge,DataRateMax,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTMASGN(dataRateMax, PUNK_INT)

	MASGN(dataRateMax)
}

CGET(Bridge,DataRateMin,int)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_BRIDGE)
	TESTATTACHED
	TESTMASGN(dataRateMin, PUNK_INT)

	MASGN(dataRateMin)
}
