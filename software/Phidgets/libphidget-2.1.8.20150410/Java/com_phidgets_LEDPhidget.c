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
#include "com_phidgets_LEDPhidget.h"
#include "../cphidgetled.h"

JNI_LOAD(accel, LED)
}

JNI_CREATE(LED)
JNI_INDEXED_GETFUNC(LED, DiscreteLED, DiscreteLED, jint)
JNI_INDEXED_SETFUNC(LED, DiscreteLED, DiscreteLED, jint)
JNI_INDEXED_GETFUNC(LED, Brightness, Brightness, jdouble)
JNI_INDEXED_SETFUNC(LED, Brightness, Brightness, jdouble)
JNI_INDEXED_GETFUNC(LED, CurrentLimit__I, CurrentLimitIndexed, jdouble)
JNI_INDEXED_SETFUNC(LED, CurrentLimit__ID, CurrentLimitIndexed, jdouble)
JNI_GETFUNC(LED, LEDCount, LEDCount, jint)

JNIEXPORT jint JNICALL
Java_com_phidgets_LEDPhidget_getCurrentLimit__(JNIEnv *env, jobject obj)
{
	CPhidgetLEDHandle h = (CPhidgetLEDHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);
	int error;
	CPhidgetLED_CurrentLimit v;
	if ((error = CPhidgetLED_getCurrentLimit(h, &v)))
		PH_THROW(error);
	return (jint)v;
}
JNI_SETFUNC(LED, CurrentLimit__I, CurrentLimit, jint)

JNIEXPORT jint JNICALL
Java_com_phidgets_LEDPhidget_getVoltage(JNIEnv *env, jobject obj)
{
	CPhidgetLEDHandle h = (CPhidgetLEDHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);
	int error;
	CPhidgetLED_Voltage v;
	if ((error = CPhidgetLED_getVoltage(h, &v)))
		PH_THROW(error);
	return (jint)v;
}
JNI_SETFUNC(LED, Voltage, Voltage, jint)