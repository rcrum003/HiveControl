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
#include "cphidgetaccelerometer.h"
#include "cusb.h"
#include "math.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(Accelerometer)
	int i = 0;

	phid->accelerationMax = PUNI_DBL;
	phid->accelerationMin = PUNI_DBL;

	for (i = 0; i<ACCEL_MAXAXES; i++)
	{
		phid->axis[i] = PUNI_DBL;
		phid->axisLastTrigger[i] = PUNK_DBL;
		phid->axisChangeTrigger[i] = PUNI_DBL;
	}
	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(Accelerometer)
	int i = 0;

	TESTPTR(phid);

	//Setup max/min values
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_ACCELEROMETER_2AXIS:
			if (phid->phid.deviceVersion < 200)
			{
				phid->accelerationMax = 2.1;
				phid->accelerationMin = -2.1;
			}
			else if ((phid->phid.deviceVersion  >= 200) && (phid->phid.deviceVersion  < 300))
			{
				phid->accelerationMax = 10.1;
				phid->accelerationMin = -10.1;
			}
			else if ((phid->phid.deviceVersion  >= 300) && (phid->phid.deviceVersion  < 400))
			{
				phid->accelerationMax = 5.1;
				phid->accelerationMin = -5.1;
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		case PHIDID_ACCELEROMETER_3AXIS:
			if ((phid->phid.deviceVersion  >= 400) && (phid->phid.deviceVersion  < 500))
			{
				phid->accelerationMax = 3.1;
				phid->accelerationMin = -3.1;
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//initialize triggers, set data arrays to unknown
	for (i = 0; i<phid->phid.attr.accelerometer.numAxis; i++)
	{
		phid->axis[i] = PUNK_DBL;
		phid->axisLastTrigger[i] = PUNK_DBL;
		phid->axisChangeTrigger[i] = 0.001;
	}

	//issue one read
	CPhidget_read((CPhidgetHandle)phid);

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(Accelerometer)
	int i = 0;
	double axis[ACCEL_MAXAXES];

	if (length<0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);
	
	ZEROMEM(axis, sizeof(axis));

	//Parse device packets - store data locally
	switch(phidG->deviceIDSpec)
	{
		case PHIDID_ACCELEROMETER_2AXIS:
			if (phidG->deviceVersion < 200)
			{
				int data = 0;
				data = (signed short)((unsigned short)buffer[0]+((unsigned short)buffer[1]<<8));
				axis[0] = round_double((((double)data-16384) / 2000), 4);
				data = (signed short)((unsigned short)buffer[2]+((unsigned short)buffer[3]<<8));
				axis[1] = round_double((((double)data-16384) / 2000), 4);
			}
			else if ((phidG->deviceVersion  >= 200) && (phidG->deviceVersion  < 300))
			{
				int data = 0;
				data = (signed short)((unsigned short)buffer[0]+((unsigned short)buffer[1]<<8));
				axis[0] = round_double((((double)data-16384) / 650), 4);
				data = (signed short)((unsigned short)buffer[2]+((unsigned short)buffer[3]<<8));
				axis[1] = round_double((((double)data-16384) / 650), 4);
			}
			else if ((phidG->deviceVersion  >= 300) && (phidG->deviceVersion  < 400))
			{
				int data = 0;
				data = ((unsigned short)buffer[0]+((unsigned short)buffer[1]<<8));
				axis[0] = round_double((((double)(data-32768)) / 4000), 5);
				data = ((unsigned short)buffer[2]+((unsigned short)buffer[3]<<8));
				axis[1] = round_double((((double)(data-32768)) / 4000), 5);
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		case PHIDID_ACCELEROMETER_3AXIS:
			if ((phidG->deviceVersion  >= 400) && (phidG->deviceVersion  < 500))
			{
				int data = 0;
				data = ((unsigned short)buffer[0]+((unsigned short)buffer[1]<<8));
				axis[0] = round_double((((double)(data-32768)) / 6553.6), 5);
				data = ((unsigned short)buffer[2]+((unsigned short)buffer[3]<<8));
				axis[1] = round_double((((double)(data-32768)) / 6553.6), 5);
				data = ((unsigned short)buffer[4]+((unsigned short)buffer[5]<<8));
				axis[2] = round_double((((double)(data-32768)) / 6553.6), 5);
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//Make sure values are within defined range, and store to structure
	for (i = 0; i<phid->phid.attr.accelerometer.numAxis; i++)
	{
		if(axis[i] > phid->accelerationMax) axis[i] = phid->accelerationMax;
		if(axis[i] < phid->accelerationMin) axis[i] = phid->accelerationMin;
		phid->axis[i] = axis[i];
	}
	
	//send out any events that exceed or match the trigger
	for (i = 0; i<phid->phid.attr.accelerometer.numAxis; i++)
	{
		if (fabs(phid->axis[i] - phid->axisLastTrigger[i]) >= phid->axisChangeTrigger[i]
			|| phid->axisLastTrigger[i] == PUNK_DBL)
		{
			FIRE(AccelerationChange, i, phid->axis[i]);
			phid->axisLastTrigger[i] = phid->axis[i];
		}
	}

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(Accelerometer)

	for (i = 0; i<phid->phid.attr.accelerometer.numAxis; i++)
	{
		if(phid->axis[i] != PUNK_DBL)
		{
			FIRE(AccelerationChange, i, phid->axis[i]);
			phid->axisLastTrigger[i] = phid->axis[i];
		}
	}

	return EPHIDGET_OK;
}

//getPacket - not used for accelerometer
CGETPACKET(Accelerometer)
	return EPHIDGET_UNEXPECTED;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(Accelerometer, PHIDCLASS_ACCELEROMETER)

//event setup functions
CFHANDLE(Accelerometer, AccelerationChange, int, double)

CGET(Accelerometer,AxisCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ACCELEROMETER)
	TESTATTACHED

	MASGN(phid.attr.accelerometer.numAxis)
}

CGETINDEX(Accelerometer,Acceleration,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_ACCELEROMETER)
	TESTATTACHED
	TESTINDEX(phid.attr.accelerometer.numAxis)
	TESTMASGN(axis[Index], PUNK_DBL)

	MASGN(axis[Index])
}

CGETINDEX(Accelerometer,AccelerationMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_ACCELEROMETER)
	TESTATTACHED
	TESTINDEX(phid.attr.accelerometer.numAxis)
	TESTMASGN(accelerationMax, PUNK_DBL)

	MASGN(accelerationMax)
}

CGETINDEX(Accelerometer,AccelerationMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_ACCELEROMETER)
	TESTATTACHED
	TESTINDEX(phid.attr.accelerometer.numAxis)
	TESTMASGN(accelerationMin, PUNK_DBL)

	MASGN(accelerationMin)
}

CGETINDEX(Accelerometer,AccelerationChangeTrigger,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_ACCELEROMETER)
	TESTATTACHED
	TESTINDEX(phid.attr.accelerometer.numAxis)
	TESTMASGN(axisChangeTrigger[Index], PUNK_DBL)

	MASGN(axisChangeTrigger[Index])
}
CSETINDEX(Accelerometer,AccelerationChangeTrigger,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_ACCELEROMETER)
	TESTATTACHED
	TESTINDEX(phid.attr.accelerometer.numAxis)
	TESTRANGE(0, phid->accelerationMax - phid->accelerationMin)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Trigger, "%lE", axisChangeTrigger);
	else
		phid->axisChangeTrigger[Index] = newVal;

	return EPHIDGET_OK;
}

// === Deprecated Functions === //

CGET(Accelerometer,NumAxis,int)
	return CPhidgetAccelerometer_getAxisCount(phid, pVal);
}
