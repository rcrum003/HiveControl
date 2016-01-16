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
#include "cphidgetanalog.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(Analog)
	int i = 0;

	for(i=0;i<ANALOG_MAXOUTPUTS;i++)
	{
		phid->changedVoltage[i] = PUNK_BOOL;
		phid->nextVoltage[i] = PUNK_DBL;

		phid->changedEnabled[i] = PUNK_BOOL;
		phid->nextEnabled[i] = PUNK_BOOL;

		phid->enabled[i] = PUNK_BOOL;
		phid->voltage[i] = PUNK_DBL;

		phid->voltageEcho[i] = PUNI_DBL;
		phid->enabledEcho[i] = PUNI_BOOL;
	}
	phid->voltageMax = PUNI_DBL;
	phid->voltageMin = PUNI_DBL;

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(Analog)
	int i = 0;

	TESTPTR(phid);

	//set data arrays to unknown
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_ANALOG_4OUTPUT:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				for(i=0;i<phid->phid.attr.analog.numAnalogOutputs;i++)
				{
					phid->changedVoltage[i] = PFALSE;
					phid->nextVoltage[i] = PUNK_DBL;

					phid->changedEnabled[i] = PFALSE;
					phid->nextEnabled[i] = PUNK_BOOL;

					phid->enabled[i] = PUNK_BOOL;
					phid->voltage[i] = PUNK_DBL;

					phid->voltageEcho[i] = PUNK_DBL;
					phid->enabledEcho[i] = PUNK_BOOL;

					phid->lastOvercurrent[i] = PFALSE;
				}
				phid->voltageMax = 10;
				phid->voltageMin = -10;

				phid->lastTsd = PFALSE;
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}
	phid->controlPacketWaiting = PFALSE;
	
	//issue a read - fill in data
	CPhidget_read((CPhidgetHandle)phid);
	for(i=0;i<phid->phid.attr.analog.numAnalogOutputs;i++)
	{
		phid->voltage[i] = phid->voltageEcho[i];
		phid->enabled[i] = phid->enabledEcho[i];
	}

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(Analog)
	int i = 0;
	char error_buffer[128];
	unsigned char overcurrent[ANALOG_MAXOUTPUTS] = {PFALSE};
	unsigned char tsd = PFALSE;
	int iVoltage;

	if (length < 0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_ANALOG_4OUTPUT:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				// Bit-->	|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
				//Byte 0	| oc[3] | oc[2] | oc[1] | oc[0] | en[3] | en[2] | en[1] | en[0] |
				//Byte 1	|   0   |   0   |   0   |   0   |   0   |   0   |   0   |  TSD  |
				//Byte 2	|       voltage[0] bits 3-0     |   0   |   0   |   0   |   0   |
				//Byte 3	|                   voltage[0] bits 11-4                        |
				//Byte 4	|       voltage[1] bits 3-0     |   0   |   0   |   0   |   0   |
				//Byte 5	|                   voltage[1] bits 11-4                        |
				//Byte 6	|       voltage[2] bits 3-0     |   0   |   0   |   0   |   0   |
				//Byte 7	|                   voltage[2] bits 11-4                        |
				//Byte 8	|       voltage[3] bits 3-0     |   0   |   0   |   0   |   0   |
				//Byte 9	|                   voltage[3] bits 11-4                        |
				for (i = 0; i < phid->phid.attr.analog.numAnalogOutputs; i++)
				{
					iVoltage =  (((signed char)buffer[i*2 + 3]) << 4) + (buffer[i*2 + 2] >> 4);
					phid->voltageEcho[i] = round_double((iVoltage * 10 / 2047.0), 3);
					phid->enabledEcho[i] = (buffer[0] & (0x01 << i)) ? PTRUE : PFALSE;
					overcurrent[i] = (buffer[0] & (0x10 << i)) ? PTRUE : PFALSE;
				}
				tsd = buffer[1];
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}
	
	for (i = 0; i < phid->phid.attr.analog.numAnalogOutputs; i++)
	{
		if(overcurrent[i] && !phid->lastOvercurrent[i])
		{
			FIRE_ERROR(EEPHIDGET_OVERCURRENT, "Output %d is trying to draw > 20mA - possible short circuit.", i);
		}
		else if(!overcurrent[i] && phid->lastOvercurrent[i])
		{
			FIRE_ERROR(EEPHIDGET_OK, "Output %d overcurrent state ended.", i);
		}
		phid->lastOvercurrent[i] = overcurrent[i];
	}
	if(tsd && !phid->lastTsd)
	{
		FIRE_ERROR(EEPHIDGET_OVERTEMP, "Thermal shutdown detected. All outputs have been disabled.");
	}
	else if(!tsd && phid->lastTsd)
	{
		FIRE_ERROR(EEPHIDGET_OK, "Thermal shutdown state ended.");
	}
	phid->lastTsd = tsd;

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization - not used
CPHIDGETINITEVENTS(Analog)
	phid = 0;
	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET(Analog)
	int i = 0;
	int iVoltage;

	CPhidgetAnalogHandle phid = (CPhidgetAnalogHandle)phidG;

	TESTPTRS(phid, buf)
	TESTPTR(lenp)

	if (*lenp < phid->phid.outputReportByteLength)
		return EPHIDGET_INVALIDARG;

	CThread_mutex_lock(&phid->phid.outputLock);
	
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_ANALOG_4OUTPUT:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{	
				// Bit-->	|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
				//Byte 0	|       voltage[0] bits 3-0     | en[3] | en[2] | en[1] | en[0] |
				//Byte 1	|                   voltage[0] bits 11-4                        |
				//Byte 2	|       voltage[1] bits 3-0     |   0   |   0   |   0   |   0   |
				//Byte 3	|                   voltage[1] bits 11-4                        |
				//Byte 4	|       voltage[2] bits 3-0     |   0   |   0   |   0   |   0   |
				//Byte 5	|                   voltage[2] bits 11-4                        |
				//Byte 6	|       voltage[3] bits 3-0     |   0   |   0   |   0   |   0   |
				//Byte 7	|                   voltage[3] bits 11-4                        |
				for (i = 0; i < phid->phid.attr.analog.numAnalogOutputs; i++)
				{
					if (phid->changedVoltage[i]) {
						phid->voltage[i] = phid->nextVoltage[i];
						phid->changedVoltage[i] = PFALSE;
						phid->nextVoltage[i] = PUNK_DBL;
					}
					if (phid->changedEnabled[i]) {
						phid->enabled[i] = phid->nextEnabled[i];
						phid->changedEnabled[i] = PFALSE;
						phid->nextEnabled[i] = PUNK_BOOL;
					}
					//fill in buffer
					iVoltage = round(phid->voltage[i] * 2047 / 10.0);
					buf[i*2] = iVoltage << 4;
					buf[i*2 + 1] = iVoltage >> 4;
					if(phid->enabled[i])
						buf[0] |= 1<<i;
				}
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	*lenp = phid->phid.outputReportByteLength;

	CThread_mutex_unlock(&phid->phid.outputLock);

	return EPHIDGET_OK;
}

//sends a packet to the device asynchronously, blocking if the 1-packet queue is full
//	-every channel has its own 1 state mini-queue
static int CCONV CPhidgetAnalog_sendPacket_setVoltage(CPhidgetAnalogHandle phid, unsigned int index, double voltage)
{
	int waitReturn;
	CThread_mutex_lock(&phid->phid.writelock);
again:
	if (!CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_ATTACHED_FLAG))
	{
		CThread_mutex_unlock(&phid->phid.writelock);
		return EPHIDGET_NOTATTACHED;
	}
	CThread_mutex_lock(&phid->phid.outputLock);
	//if we have already requested a change on this channel
	if (phid->changedVoltage[index]) {
		//and it was different then this time
		if (phid->nextVoltage[index] != voltage) {
			CThread_mutex_unlock(&phid->phid.outputLock);
			//then wait for it to get written
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
			//and try again
			goto again;
		} else {
			CThread_mutex_unlock(&phid->phid.outputLock);
			CThread_mutex_unlock(&phid->phid.writelock);
			return EPHIDGET_OK;
		}
	//otherwise
	} else {
		//if it's different then current, queue it up
		if (phid->voltage[index] != voltage) {
			phid->changedVoltage[index] = PTRUE;
			phid->nextVoltage[index] = voltage;
			CThread_reset_event(&phid->phid.writtenEvent);
			CThread_mutex_unlock(&phid->phid.outputLock);
			CThread_set_event(&phid->phid.writeAvailableEvent);
		}
		//if it's the same, just return
		else
		{
			CThread_mutex_unlock(&phid->phid.outputLock);
			CThread_mutex_unlock(&phid->phid.writelock);
			return EPHIDGET_OK;
		}
	}
	CThread_mutex_unlock(&phid->phid.writelock);
	return EPHIDGET_OK;
}

//sends a packet to the device asynchronously, blocking if the 1-packet queue is full
//	-every channel has its own 1 state mini-queue
static int CCONV CPhidgetAnalog_sendPacket_setEnabled(CPhidgetAnalogHandle phid, unsigned int index, int enabled)
{
	int waitReturn;
	CThread_mutex_lock(&phid->phid.writelock);
again:
	if (!CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_ATTACHED_FLAG))
	{
		CThread_mutex_unlock(&phid->phid.writelock);
		return EPHIDGET_NOTATTACHED;
	}
	CThread_mutex_lock(&phid->phid.outputLock);
	//if we have already requested a change on this channel
	if (phid->changedEnabled[index]) {
		//and it was different then this time
		if (phid->nextEnabled[index] != enabled) {
			CThread_mutex_unlock(&phid->phid.outputLock);
			//then wait for it to get written
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
			//and try again
			goto again;
		} else {
			CThread_mutex_unlock(&phid->phid.outputLock);
			CThread_mutex_unlock(&phid->phid.writelock);
			return EPHIDGET_OK;
		}
	//otherwise
	} else {
		//if it's different then current, queue it up
		if (phid->enabled[index] != enabled) {
			phid->changedEnabled[index] = PTRUE;
			phid->nextEnabled[index] = enabled;
			CThread_reset_event(&phid->phid.writtenEvent);
			CThread_mutex_unlock(&phid->phid.outputLock);
			CThread_set_event(&phid->phid.writeAvailableEvent);
		}
		//if it's the same, just return
		else
		{
			CThread_mutex_unlock(&phid->phid.outputLock);
			CThread_mutex_unlock(&phid->phid.writelock);
			return EPHIDGET_OK;
		}
	}
	CThread_mutex_unlock(&phid->phid.writelock);
	return EPHIDGET_OK;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(Analog, PHIDCLASS_ANALOG)

CGET(Analog,OutputCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ANALOG)
	TESTATTACHED

	MASGN(phid.attr.analog.numAnalogOutputs)
}

CGETINDEX(Analog,Voltage,double)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_ANALOG)
	TESTATTACHED
	TESTINDEX(phid.attr.analog.numAnalogOutputs)
	TESTMASGN(voltageEcho[Index], PUNK_DBL)

	MASGN(voltageEcho[Index])
}

CSETINDEX(Analog,Voltage,double)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_ANALOG)
	TESTATTACHED
	TESTRANGE(phid->voltageMin, phid->voltageMax)
	TESTINDEX(phid.attr.analog.numAnalogOutputs)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Voltage, "%lf", voltage);
	else
		return CPhidgetAnalog_sendPacket_setVoltage(phid, Index, newVal);

	return EPHIDGET_OK;
}

CGETINDEX(Analog,VoltageMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_ANALOG)
	TESTATTACHED
	TESTINDEX(phid.attr.analog.numAnalogOutputs)
	TESTMASGN(voltageMax, PUNK_DBL)

	MASGN(voltageMax)
}

CGETINDEX(Analog,VoltageMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_ANALOG)
	TESTATTACHED
	TESTINDEX(phid.attr.analog.numAnalogOutputs)
	TESTMASGN(voltageMin, PUNK_DBL)

	MASGN(voltageMin)
}

CGETINDEX(Analog,Enabled,int)
	TESTPTRS(phid,pVal)
	TESTDEVICETYPE(PHIDCLASS_ANALOG)
	TESTATTACHED
	TESTINDEX(phid.attr.analog.numAnalogOutputs)
	TESTMASGN(enabledEcho[Index], PUNK_BOOL)

	MASGN(enabledEcho[Index])
}
CSETINDEX(Analog,Enabled,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_ANALOG)
	TESTATTACHED
	TESTRANGE(PFALSE, PTRUE)
	TESTINDEX(phid.attr.analog.numAnalogOutputs)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Enabled, "%d", enabled);
	else
		return CPhidgetAnalog_sendPacket_setEnabled(phid, Index, newVal);

	return EPHIDGET_OK;
}
