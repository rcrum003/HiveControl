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
#include "cphidgetfrequencycounter.h"
#include <math.h>
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(FrequencyCounter)
	int i = 0;

	//initialize triggers, set data arrays to unknown
	for (i = 0; i<FREQCOUNTER_MAXINPUTS; i++)
	{
		phid->timeout[i] = PUNI_INT;
		phid->frequency[i] = PUNI_INT;
		phid->filterEcho[i] = PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_UNKNOWN;
		phid->enabledEcho[i] = PUNI_BOOL;

		phid->totalCount[i] = 0;
		phid->totalTime[i] = 0;

		phid->countsGood[i] = PFALSE;
	}

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(FrequencyCounter)
	int i;

	TESTPTR(phid);

	//initialize triggers, set data arrays to unknown
	for (i = 0; i<phid->phid.attr.frequencycounter.numFreqInputs; i++)
	{
		phid->timeout[i] = 1000000; //1 second (in microseconds) - detect down to 1Hz

		phid->frequency[i] = PUNK_INT;
		phid->filterEcho[i] = PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_UNKNOWN;
		phid->enabledEcho[i] = PUNK_BOOL;
	}
	phid->lastPacketCount = PUNK_INT;

	//issue one read
	CPhidget_read((CPhidgetHandle)phid);

	//fill in enabledState and filterType, set other things to 0
	for (i = 0; i<phid->phid.attr.frequencycounter.numFreqInputs; i++)
	{
		phid->filter[i] = phid->filterEcho[i];
		phid->enabled[i] = phid->enabledEcho[i];
		
		phid->totalTicksSinceLastCount[i] = PUNK_INT;

		phid->totalCount[i] = 0;
		phid->totalTime[i] = 0;
	}
	phid->lastPacketCount = PUNK_INT;

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(FrequencyCounter)
	int i, packetCount;
	int ticks, ticksAtLastCount[FREQCOUNTER_MAXINPUTS], counts[FREQCOUNTER_MAXINPUTS];
	char error_buffer[127];

	//Setup max/min values
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_FREQUENCYCOUNTER_2INPUT:
			if(phid->phid.deviceVersion < 200) 
			{
				ticks = buffer[0] + (buffer[1] << 8);

				counts[0] = buffer[2] + (buffer[3] << 8) + (buffer[4] << 16);
				ticksAtLastCount[0] = buffer[5] + (buffer[6] << 8);

				counts[1] = buffer[7] + (buffer[8] << 8) + (buffer[9] << 16);
				ticksAtLastCount[1] = buffer[10] + (buffer[11] << 8);

				//Filter type echo
				if(buffer[12] & FREQCOUNTER_FLAG_CH0_LOGIC)
					phid->filterEcho[0] = PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL;
				else
					phid->filterEcho[0] = PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING;
				if(buffer[12] & FREQCOUNTER_FLAG_CH1_LOGIC)
					phid->filterEcho[1] = PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL;
				else
					phid->filterEcho[1] = PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING;
				
				//Enabled state echo
				if(buffer[12] & FREQCOUNTER_FLAG_CH0_ENABLE)
					phid->enabledEcho[0] = PTRUE;
				else
					phid->enabledEcho[0] = PFALSE;
				if(buffer[12] & FREQCOUNTER_FLAG_CH1_ENABLE)
					phid->enabledEcho[1] = PTRUE;
				else
					phid->enabledEcho[1] = PFALSE;

				packetCount = (buffer[12] & 0xF0) >> 4;
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	if((phid->lastPacketCount != PUNK_INT) && ((phid->lastPacketCount+1)&0x0F) != packetCount)
	{
		FIRE_ERROR_NOQUEUE(EEPHIDGET_PACKETLOST, "One or more data packets were lost");
	}
	phid->lastPacketCount = packetCount;

	for(i=0;i<phid->phid.attr.frequencycounter.numFreqInputs; i++)
	{
		if(phid->enabledEcho[i] == PTRUE && phid->enabled[i] == PTRUE)
		{
			CThread_mutex_lock(&phid->resetlock);
			phid->totalTime[i] += (ticks * FREQCOUNTER_MICROSECONDS_PER_TICK);
			phid->totalCount[i] += counts[i];
			CThread_mutex_unlock(&phid->resetlock);

			if (counts[i] == 0)
			{
				//Do not accumulate if timed out
				if (phid->totalTicksSinceLastCount[i] != PUNK_INT)
					phid->totalTicksSinceLastCount[i] += ticks;

				//only accumulate counts up to timeOut
				if ((phid->totalTicksSinceLastCount[i] * FREQCOUNTER_MICROSECONDS_PER_TICK) > phid->timeout[i])
				{
					phid->frequency[i] = 0;

					//Fire one event with 0 counts to indicate that the Timeout has elapsed and frequency is now 0
					FIRE(Count, i, (phid->totalTicksSinceLastCount[i] * FREQCOUNTER_MICROSECONDS_PER_TICK), 0);
					phid->totalTicksSinceLastCount[i] = PUNK_INT;
				}
			}
			else
			{
				//1st count(s) since a timeout (or 1st read packet since opening)
				//don't try to calculate frequency because we don't to the 'ticks at first count'
				if (phid->totalTicksSinceLastCount[i] == PUNK_INT)
				{
					phid->totalTicksSinceLastCount[i] = ticks - ticksAtLastCount[i];
				}
				else
				{
					int countTimeSpan = (phid->totalTicksSinceLastCount[i] + ticksAtLastCount[i]) * FREQCOUNTER_MICROSECONDS_PER_TICK; //in microseconds
					phid->frequency[i] = (double)((double)counts[i] / ((double)countTimeSpan / 1000000.0));

					FIRE(Count, i, countTimeSpan, counts[i]);

					phid->totalTicksSinceLastCount[i] = ticks - ticksAtLastCount[i];
				}
			}
		}
		else
			phid->frequency[i] = PUNK_DBL;
	}

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(FrequencyCounter)
	phid = 0;
	return EPHIDGET_OK;
}

//Extra things to do during a free
//This is run before the other things that free does
int CPhidgetFrequencyCounter_free(CPhidgetHandle phidG)
{
	CPhidgetFrequencyCounterHandle phid = (CPhidgetFrequencyCounterHandle)phidG;
	CThread_mutex_destroy(&phid->resetlock);
	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET_BUF(FrequencyCounter)

//sendpacket - sends a packet to the device asynchronously, blocking if the 1-packet queue is full
CSENDPACKET_BUF(FrequencyCounter)

//makePacket - constructs a packet using current device state
CMAKEPACKET(FrequencyCounter)
	TESTPTRS(phid, buffer);

	//Setup max/min values
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_FREQUENCYCOUNTER_2INPUT:
			if(phid->phid.deviceVersion < 200) 
			{
				buffer[0] = 0;
				if(phid->enabled[0] == PTRUE)
					buffer[0] |= FREQCOUNTER_FLAG_CH0_ENABLE;
				else
					phid->enabled[0] = PFALSE;
				if(phid->filter[0] == PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL)
					buffer[0] |= FREQCOUNTER_FLAG_CH0_LOGIC;
				else
					phid->filter[0] = PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING;

				if(phid->enabled[1] == PTRUE)
					buffer[0] |= FREQCOUNTER_FLAG_CH1_ENABLE;
				else
					phid->enabled[1] = PFALSE;
				if(phid->filter[1] == PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL)
					buffer[0] |= FREQCOUNTER_FLAG_CH1_LOGIC;
				else
					phid->filter[1] = PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING;
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
CCREATE_EXTRA(FrequencyCounter, PHIDCLASS_FREQUENCYCOUNTER)
	CThread_mutex_init(&phid->resetlock);
	phid->phid.fptrFree = CPhidgetFrequencyCounter_free;
	return EPHIDGET_OK;
}

//event setup functions
CFHANDLE(FrequencyCounter, Count, int, int, int)

CGET(FrequencyCounter,FrequencyInputCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED

	MASGN(phid.attr.frequencycounter.numFreqInputs)
}

CGETINDEX(FrequencyCounter,Frequency,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)
	TESTMASGN(frequency[Index], PUNK_DBL)

	MASGN(frequency[Index])
}

CGETINDEX(FrequencyCounter,TotalTime,__int64)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)

	MASGN(totalTime[Index])
}

CGETINDEX(FrequencyCounter,TotalCount,__int64)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)

	MASGN(totalCount[Index])
}

CGETINDEX(FrequencyCounter,TotalTime32,int)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)

	*pVal = (int)phid->totalTime[Index]; return EPHIDGET_OK;
}

CGETINDEX(FrequencyCounter,TotalCount32,int)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)

	*pVal = (int)phid->totalCount[Index]; return EPHIDGET_OK;
}

CGETINDEX(FrequencyCounter,Timeout,int)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)

	MASGN(timeout[Index])
}
CSETINDEX(FrequencyCounter,Timeout,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTRANGE(100000, 100000000) //0.1-100 seconds
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Timeout, "%d", timeout);
	else
		SENDPACKET(FrequencyCounter, timeout[Index]);

	return EPHIDGET_OK;
}

CGETINDEX(FrequencyCounter,Enabled,int)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)
	TESTMASGN(enabledEcho[Index], PUNK_BOOL)

	MASGN(enabledEcho[Index])
}
CSETINDEX(FrequencyCounter,Enabled,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTRANGE(PFALSE, PTRUE)
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)

	if(newVal == PFALSE) phid->frequency[Index] = PUNK_DBL;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Enabled, "%d", enabled);
	else
		SENDPACKET(FrequencyCounter, enabled[Index]);

	return EPHIDGET_OK;
}

CGETINDEX(FrequencyCounter,Filter,CPhidgetFrequencyCounter_FilterType)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)
	TESTMASGN(filterEcho[Index], PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_UNKNOWN)

	MASGN(filterEcho[Index])
}
CSETINDEX(FrequencyCounter,Filter,CPhidgetFrequencyCounter_FilterType)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTRANGE(PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING, PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL)
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Filter, "%d", filter);
	else
		SENDPACKET(FrequencyCounter, filter[Index]);

	return EPHIDGET_OK;
}

PHIDGET21_API int CCONV CPhidgetFrequencyCounter_reset(CPhidgetFrequencyCounterHandle phid, int Index)
{
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_FREQUENCYCOUNTER)
	TESTATTACHED
	TESTINDEX(phid.attr.frequencycounter.numFreqInputs)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		int newVal = phid->flip[Index]^1;
		ADDNETWORKKEYINDEXED(Reset, "%d", flip);
	}
	else
	{
		CThread_mutex_lock(&phid->resetlock);
		phid->totalCount[Index] = 0;
		phid->totalTime[Index] = 0;
		CThread_mutex_unlock(&phid->resetlock);
	}

	return EPHIDGET_OK;
}
