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
#include "cphidgetphsensor.h"
#include <math.h>
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //

static double calculate_ph(double Vad, double temperature);

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(PHSensor)
	
	phid->phMax = PUNI_DBL;
	phid->phMin = PUNI_DBL;
	phid->potentialMax = PUNI_DBL;
	phid->potentialMin = PUNI_DBL;

	phid->Temperature = PUNI_DBL;
	phid->PH = PUNI_DBL;
	phid->PHLastTrigger = PUNK_DBL;
	phid->Potential = PUNI_DBL;
	phid->PHChangeTrigger = PUNI_DBL;

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(PHSensor)
	TESTPTR(phid);

	phid->Temperature = 20; //20 degrees celcius default

	//Setup max/min values
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_PHSENSOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				phid->phMax = round_double(calculate_ph((65535.0 / 13104.0), phid->Temperature), 4);
				phid->phMin = round_double(calculate_ph(0, phid->Temperature), 4);
				phid->potentialMax = round_double(((2.5) / 4.745) * 1000.0, 2);
				phid->potentialMin = round_double(((2.5 - (65535.0 / 13104.0)) / 4.745) * 1000.0, 2);
			}
			else
				return EPHIDGET_BADVERSION;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//initialize triggers, set data arrays to unknown
	phid->PH = PUNK_DBL;
	phid->PHLastTrigger = PUNK_DBL;
	phid->Potential = PUNK_DBL;
	phid->PHChangeTrigger = 0.05;

	//issue one read
	CPhidget_read((CPhidgetHandle)phid);

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(PHSensor)
	double PH = 0, Potential = 0;

	if (length<0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);
	
	//Parse device packets - store data locally
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_PHSENSOR:
			if ((phid->phid.deviceVersion >= 100) && (phid->phid.deviceVersion < 200))
			{
				double Vad = 0, E = 0;

				Vad = ((double)((unsigned short)buffer[0]+((unsigned short)buffer[1]<<8))) / 13104.0;
				PH = round_double(calculate_ph(Vad, phid->Temperature), 4);
				
				E = (2.5 - Vad) / 4.745;
				Potential = round_double((E * 1000.0), 2);
			}
			else
				return EPHIDGET_UNEXPECTED;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}
	
	//Make sure values are within defined range, and store to structure
	if(PH < phid->phMin || PH > phid->phMax)
		phid->PH = PUNK_DBL;
	else
		phid->PH = PH;
	if(Potential < phid->potentialMin || Potential > phid->potentialMax)
		phid->Potential = PUNK_DBL;
	else
		phid->Potential = Potential;

	//send out any events that exceed or match the trigger
	if ((fabs(phid->PH - phid->PHLastTrigger) >= phid->PHChangeTrigger || phid->PHLastTrigger == PUNK_DBL) 
		&& phid->PH != PUNK_DBL)
	{
		FIRE(PHChange, phid->PH);
		phid->PHLastTrigger = phid->PH;
	}

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(PHSensor)
	if (phid->PH != PUNK_DBL)
	{
		FIRE(PHChange, phid->PH);
		phid->PHLastTrigger = phid->PH;
	}
	return EPHIDGET_OK;
}

//getPacket - not used for PHSensor
CGETPACKET(PHSensor)
	return EPHIDGET_UNEXPECTED;
}

static double calculate_ph(double Vad, double temperature)
{
	double E, E0, C, T;
	const double R=8.31441, N=1, F=96484.6;
	T=(273.15)+temperature;
	C = 2.3 * ((R*T)/(N*F));
	E0 = 7*C;
	E = (2.5 - Vad) / 4.745;
	return ((E0 - E) / C);
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(PHSensor, PHIDCLASS_PHSENSOR)

//event setup functions
CFHANDLE(PHSensor, PHChange, double)

CGET(PHSensor,PH,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_PHSENSOR)
	TESTATTACHED
	TESTMASGN(PH, PUNK_DBL)

	MASGN(PH)
}

CGET(PHSensor,PHMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_PHSENSOR)
	TESTATTACHED
	TESTMASGN(phMax, PUNK_DBL)

	MASGN(phMax)
}

CGET(PHSensor,PHMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_PHSENSOR)
	TESTATTACHED
	TESTMASGN(phMin, PUNK_DBL)

	MASGN(phMin)
}

CGET(PHSensor,PHChangeTrigger,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_PHSENSOR)
	TESTATTACHED
	TESTMASGN(PHChangeTrigger, PUNK_DBL)

	MASGN(PHChangeTrigger)
}
CSET(PHSensor,PHChangeTrigger,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_PHSENSOR)
	TESTATTACHED
	TESTRANGE(0, phid->phMax - phid->phMin)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEY(Trigger, "%lE", PHChangeTrigger);
	else
		phid->PHChangeTrigger = newVal;

	return EPHIDGET_OK;
}

CGET(PHSensor,Potential,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_PHSENSOR)
	TESTATTACHED
	TESTMASGN(Potential, PUNK_DBL)

	MASGN(Potential)
}

CGET(PHSensor,PotentialMax,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_PHSENSOR)
	TESTATTACHED
	TESTMASGN(potentialMax, PUNK_DBL)

	MASGN(potentialMax)
}

CGET(PHSensor,PotentialMin,double)
	TESTPTRS(phid,pVal) 	
	TESTDEVICETYPE(PHIDCLASS_PHSENSOR)
	TESTATTACHED
	TESTMASGN(potentialMin, PUNK_DBL)

	MASGN(potentialMin)
}

CSET(PHSensor,Temperature,double)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_PHSENSOR)
	TESTATTACHED
	TESTRANGE(-273.15, 5000) //arbitrary but reasonable range

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEY(Temperature, "%lE", Temperature);
	else
		phid->Temperature = newVal;

	phid->phMax = round_double(calculate_ph((65535.0 / 13104.0), phid->Temperature), 4);
	phid->phMin = round_double(calculate_ph(0, phid->Temperature), 4);

	return EPHIDGET_OK;
}
