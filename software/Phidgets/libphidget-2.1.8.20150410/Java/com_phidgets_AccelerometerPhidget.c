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
#include "com_phidgets_AccelerometerPhidget.h"
#include "../cphidgetaccelerometer.h"

EVENT_VARS(accelerationChange, AccelerationChange)

JNI_LOAD(accel, Accelerometer)
	EVENT_VAR_SETUP(accel, accelerationChange, AccelerationChange, ID, V)
}

EVENT_HANDLER_INDEXED(Accelerometer, accelerationChange, AccelerationChange, 
					  CPhidgetAccelerometer_set_OnAccelerationChange_Handler, double)

JNI_CREATE(Accelerometer)
JNI_INDEXED_GETFUNC(Accelerometer, AccelerationChangeTrigger, AccelerationChangeTrigger, jdouble)
JNI_INDEXED_SETFUNC(Accelerometer, AccelerationChangeTrigger, AccelerationChangeTrigger, jdouble)
JNI_INDEXED_GETFUNC(Accelerometer, AccelerationMax, AccelerationMax, jdouble)
JNI_INDEXED_GETFUNC(Accelerometer, AccelerationMin, AccelerationMin, jdouble)
JNI_INDEXED_GETFUNC(Accelerometer, Acceleration, Acceleration, jdouble)
JNI_GETFUNC(Accelerometer, AxisCount, AxisCount, jint)
