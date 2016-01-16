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
#include "com_phidgets_FrequencyCounterPhidget.h"
#include "../cphidgetfrequencycounter.h"

EVENT_VARS(frequencyCounterCount, FrequencyCounterCount)

JNI_LOAD(frequencyCounter, FrequencyCounter)
	EVENT_VAR_SETUP(frequencyCounter, frequencyCounterCount, FrequencyCounterCount, III, V)
}

EVENT_HANDLER_INDEXED2(FrequencyCounter, frequencyCounterCount, FrequencyCounterCount, 
					  CPhidgetFrequencyCounter_set_OnCount_Handler, int, int)

JNI_CREATE(FrequencyCounter)

JNI_GETFUNC(FrequencyCounter, FrequencyInputCount, FrequencyInputCount, jint)
JNI_INDEXED_GETFUNC(FrequencyCounter, Frequency, Frequency, jdouble)
JNI_INDEXED_GETFUNC(FrequencyCounter, TotalTime, TotalTime, jlong)
JNI_INDEXED_GETFUNC(FrequencyCounter, TotalCount, TotalCount, jlong)
JNI_INDEXED_GETFUNC(FrequencyCounter, Timeout, Timeout, jint)
JNI_INDEXED_SETFUNC(FrequencyCounter, Timeout, Timeout, jint)
JNI_INDEXED_GETFUNCBOOL(FrequencyCounter, Enabled, Enabled)
JNI_INDEXED_SETFUNC(FrequencyCounter, Enabled, Enabled, jboolean)

JNIEXPORT jint JNICALL
Java_com_phidgets_FrequencyCounterPhidget_getFilter(JNIEnv *env, jobject obj, jint index)
{
	CPhidgetFrequencyCounterHandle h = (CPhidgetFrequencyCounterHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);
	int error;
	CPhidgetFrequencyCounter_FilterType v;
	if ((error = CPhidgetFrequencyCounter_getFilter(h, index, &v)))
		PH_THROW(error);
	return (jint)v;
}

JNI_INDEXED_SETFUNC(FrequencyCounter, Filter, Filter, jint)

JNIEXPORT void JNICALL
Java_com_phidgets_FrequencyCounterPhidget_reset(JNIEnv *env, jobject obj, jint index)
{
	CPhidgetFrequencyCounterHandle h = (CPhidgetFrequencyCounterHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);
	int error;

	if ((error = CPhidgetFrequencyCounter_reset(h, index)))
		PH_THROW(error);
}