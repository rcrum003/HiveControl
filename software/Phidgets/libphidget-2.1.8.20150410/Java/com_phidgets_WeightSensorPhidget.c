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

#include "../stdafx.h"
#include "phidget_jni.h"
#include "com_phidgets_WeightSensorPhidget.h"
#include "../cphidgetweightsensor.h"

EVENT_VARS(weightChange, WeightChange)

JNI_LOAD(weight, WeightSensor)
	EVENT_VAR_SETUP(weight, weightChange, WeightChange, D, V)
}

EVENT_HANDLER(WeightSensor, weightChange, WeightChange, 
			  CPhidgetWeightSensor_set_OnWeightChange_Handler, double)

JNI_CREATE(WeightSensor)
JNI_GETFUNC(WeightSensor, WeightChangeTrigger, WeightChangeTrigger, jdouble)
JNI_SETFUNC(WeightSensor, WeightChangeTrigger, WeightChangeTrigger, jdouble)
JNI_GETFUNC(WeightSensor, Weight, Weight, jdouble)
