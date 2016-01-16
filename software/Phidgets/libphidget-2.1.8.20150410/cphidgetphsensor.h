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

#ifndef __CPHIDGETPHSENSOR
#define __CPHIDGETPHSENSOR
#include "cphidget.h"

/** \defgroup phidph Phidget PH Sensor 
 * \ingroup phidgets
 * These calls are specific to the Phidget PH Sensor object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(PHSensor)
CHDRSTANDARD(PHSensor)

/**
 * Gets the sensed PH.
 * @param phid An attached phidget ph sensor handle.
 * @param ph The PH.
 */
CHDRGET(PHSensor,PH,double *ph)
/**
 * Gets the maximum PH that the sensor could report.
 * @param phid An attached phidget ph sensor handle.
 * @param max The maximum PH.
 */
CHDRGET(PHSensor,PHMax,double *max)
/**
 * Gets the minimum PH that the sensor could report.
 * @param phid An attached phidget ph sensor handle.
 * @param min The minimum PH.
 */
CHDRGET(PHSensor,PHMin,double *min)
/**
 * Set a PH change handler. This is called when the PH changes by more then the change trigger.
 * @param phid An attached phidget ph sensor handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENT(PHSensor,PHChange,double ph)
/**
 * Gets the PH change trigger.
 * @param phid An attached phidget ph sensor handle.
 * @param trigger The change trigger.
 */
CHDRGET(PHSensor,PHChangeTrigger,double *trigger)
/**
 * Sets the PH change trigger.
 * @param phid An attached phidget ph sensor handle.
 * @param trigger The change trigger.
 */
CHDRSET(PHSensor,PHChangeTrigger,double trigger)

/**
 * Gets the sensed potential.
 * @param phid An attached phidget ph sensor handle.
 * @param potential The potential.
 */
CHDRGET(PHSensor,Potential,double *potential)
/**
 * Gets the maximum potential that can be sensed.
 * @param phid An attached phidget ph sensor handle.
 * @param max The maximum potential.
 */
CHDRGET(PHSensor,PotentialMax,double *max)
/**
 * Gets the minimum potential that can be sensed.
 * @param phid An attached phidget ph sensor handle.
 * @param min The minimum potential.
 */
CHDRGET(PHSensor,PotentialMin,double *min)

/**
 * Sets the temperature to be used for PH calculations.
 * @param phid An attached phidget ph sensor handle.
 * @param temperature The temperature (degrees celcius). By default this is 20.
 */
CHDRSET(PHSensor,Temperature,double temperature)

#ifndef EXTERNALPROTO
struct _CPhidgetPHSensor {
	CPhidget phid;

	int (CCONV *fptrPHChange)(CPhidgetPHSensorHandle, void *, double);

	void *fptrPHChangeptr;

	double PH, Potential;
	double PHLastTrigger;
	double PHChangeTrigger;

	double Temperature;

	double phMax, phMin;
	double potentialMax, potentialMin;
} typedef CPhidgetPHSensorInfo;
#endif

/** @} */

#endif
