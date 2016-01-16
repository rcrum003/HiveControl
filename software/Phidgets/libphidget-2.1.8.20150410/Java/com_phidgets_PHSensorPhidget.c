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
#include "com_phidgets_PHSensorPhidget.h"
#include "../cphidgetphsensor.h"

EVENT_VARS(phChange, PHChange)

JNI_LOAD(ph, PHSensor)
	EVENT_VAR_SETUP(ph, phChange, PHChange, D, V)
}

EVENT_HANDLER(PHSensor, phChange, PHChange, 
			  CPhidgetPHSensor_set_OnPHChange_Handler, double)

JNI_CREATE(PHSensor)
JNI_GETFUNC(PHSensor, PHChangeTrigger, PHChangeTrigger, jdouble)
JNI_SETFUNC(PHSensor, PHChangeTrigger, PHChangeTrigger, jdouble)
JNI_GETFUNC(PHSensor, PH, PH, jdouble)
JNI_GETFUNC(PHSensor, PHMin, PHMin, jdouble)
JNI_GETFUNC(PHSensor, PHMax, PHMax, jdouble)
JNI_GETFUNC(PHSensor, Potential, Potential, jdouble)
JNI_GETFUNC(PHSensor, PotentialMin, PotentialMin, jdouble)
JNI_GETFUNC(PHSensor, PotentialMax, PotentialMax, jdouble)
JNI_SETFUNC(PHSensor, Temperature, Temperature, jdouble)
