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

#ifndef __CPHIDGETWEIGHTSENSOR
#define __CPHIDGETWEIGHTSENSOR
#include "cphidget.h"

/** \defgroup phidweight Phidget Weight Sensor 
 * \ingroup phidgets
 * These calls are specific to the Phidget Weight Sensor object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(WeightSensor)
CHDRSTANDARD(WeightSensor)

/**
 * Gets the sensed weight.
 * @param phid An attached phidget weight sensor handle.
 * @param weight The weight.
 */
CHDRGET(WeightSensor,Weight,double *weight)
/**
 * Set a weight change handler. This is called when the weight changes by more then the change trigger.
 * @param phid An attached phidget weight sensor handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENT(WeightSensor,WeightChange,double weight)
/**
 * Gets the weight change trigger.
 * @param phid An attached phidget weight sensor handle.
 * @param trigger The change trigger.
 */
CHDRGET(WeightSensor,WeightChangeTrigger,double *trigger)
/**
 * Sets the weight change trigger.
 * @param phid An attached phidget weight sensor handle.
 * @param trigger The change trigger.
 */
CHDRSET(WeightSensor,WeightChangeTrigger,double trigger)

#ifndef EXTERNALPROTO
struct _CPhidgetWeightSensor {
	CPhidget phid;

	int (CCONV *fptrWeightChange)(CPhidgetWeightSensorHandle, void *, double);           

	void *fptrWeightChangeptr;

	double Weight;
	double lastweight;
	double WeightChangeTrigger;
} typedef CPhidgetWeightSensorInfo;
#endif

/** @} */

#endif
