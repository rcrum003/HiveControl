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
#include "com_phidgets_TemperatureSensorPhidget.h"
#include "../cphidgettemperaturesensor.h"

EVENT_VARS(temperatureChange, TemperatureChange)

JNI_LOAD(temp, TemperatureSensor)
	EVENT_VAR_SETUP(temp, temperatureChange, TemperatureChange, ID, V)
}

EVENT_HANDLER_INDEXED(TemperatureSensor, temperatureChange, TemperatureChange, 
					  CPhidgetTemperatureSensor_set_OnTemperatureChange_Handler, double)

JNI_CREATE(TemperatureSensor)
JNI_INDEXED_GETFUNC(TemperatureSensor, TemperatureChangeTrigger, TemperatureChangeTrigger, jdouble)
JNI_INDEXED_SETFUNC(TemperatureSensor, TemperatureChangeTrigger, TemperatureChangeTrigger, jdouble)
JNIEXPORT jint JNICALL
Java_com_phidgets_TemperatureSensorPhidget_getThermocoupleType(JNIEnv *env, jobject obj, jint index)
{
	CPhidgetTemperatureSensorHandle h = (CPhidgetTemperatureSensorHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);
	int error;
	CPhidgetTemperatureSensor_ThermocoupleType v;
	if ((error = CPhidgetTemperatureSensor_getThermocoupleType(h, index, &v)))
		PH_THROW(error);
	return (jint)v;
}
JNI_INDEXED_SETFUNC(TemperatureSensor, ThermocoupleType, ThermocoupleType, jint)
JNI_INDEXED_GETFUNC(TemperatureSensor, Temperature, Temperature, jdouble)
JNI_INDEXED_GETFUNC(TemperatureSensor, TemperatureMin, TemperatureMin, jdouble)
JNI_INDEXED_GETFUNC(TemperatureSensor, TemperatureMax, TemperatureMax, jdouble)
JNI_INDEXED_GETFUNC(TemperatureSensor, Potential, Potential, jdouble)
JNI_INDEXED_GETFUNC(TemperatureSensor, PotentialMin, PotentialMin, jdouble)
JNI_INDEXED_GETFUNC(TemperatureSensor, PotentialMax, PotentialMax, jdouble)
JNI_GETFUNC(TemperatureSensor, AmbientTemperature, AmbientTemperature, jdouble)
JNI_GETFUNC(TemperatureSensor, AmbientTemperatureMin, AmbientTemperatureMin, jdouble)
JNI_GETFUNC(TemperatureSensor, AmbientTemperatureMax, AmbientTemperatureMax, jdouble)
JNI_GETFUNC(TemperatureSensor, TemperatureInputCount, TemperatureInputCount, jint)

//Deprecated
JNI_GETFUNC(TemperatureSensor, SensorCount, TemperatureInputCount, jint)
