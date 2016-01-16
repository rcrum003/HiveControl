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
#include "com_phidgets_InterfaceKitPhidget.h"
#include "../cphidgetinterfacekit.h"

EVENT_VARS(inputChange, InputChange)
EVENT_VARS(outputChange, OutputChange)
EVENT_VARS(sensorChange, SensorChange)

JNI_LOAD(ifkit, InterfaceKit)
	EVENT_VAR_SETUP(ifkit, inputChange, InputChange, IZ, V)
	EVENT_VAR_SETUP(ifkit, outputChange, OutputChange, IZ, V)
	EVENT_VAR_SETUP(ifkit, sensorChange, SensorChange, II, V)
}

EVENT_HANDLER_INDEXED(InterfaceKit, inputChange, InputChange, CPhidgetInterfaceKit_set_OnInputChange_Handler, int)
EVENT_HANDLER_INDEXED(InterfaceKit, outputChange, OutputChange, CPhidgetInterfaceKit_set_OnOutputChange_Handler, int)
EVENT_HANDLER_INDEXED(InterfaceKit, sensorChange, SensorChange, CPhidgetInterfaceKit_set_OnSensorChange_Handler, int)

JNI_CREATE(InterfaceKit)
JNI_GETFUNC(InterfaceKit, OutputCount, OutputCount, jint)
JNI_GETFUNC(InterfaceKit, InputCount, InputCount, jint)
JNI_GETFUNC(InterfaceKit, SensorCount, SensorCount, jint)
JNI_INDEXED_GETFUNCBOOL(InterfaceKit, InputState, InputState)
JNI_INDEXED_GETFUNCBOOL(InterfaceKit, OutputState, OutputState)
JNI_INDEXED_GETFUNC(InterfaceKit, SensorValue, SensorValue, jint)
JNI_INDEXED_GETFUNC(InterfaceKit, SensorRawValue, SensorRawValue, jint)
JNI_INDEXED_GETFUNC(InterfaceKit, DataRateMin, DataRateMin, jint)
JNI_INDEXED_GETFUNC(InterfaceKit, DataRateMax, DataRateMax, jint)
JNI_INDEXED_GETFUNC(InterfaceKit, DataRate, DataRate, jint)
JNI_INDEXED_GETFUNC(InterfaceKit, SensorChangeTrigger, SensorChangeTrigger, jint)
JNI_GETFUNCBOOL(InterfaceKit, Ratiometric, Ratiometric)
JNI_INDEXED_SETFUNC(InterfaceKit, OutputState, OutputState, jboolean)
JNI_INDEXED_SETFUNC(InterfaceKit, DataRate, DataRate, jint)
JNI_INDEXED_SETFUNC(InterfaceKit, SensorChangeTrigger, SensorChangeTrigger, jint)
JNI_SETFUNC(InterfaceKit, Ratiometric, Ratiometric, jboolean)
