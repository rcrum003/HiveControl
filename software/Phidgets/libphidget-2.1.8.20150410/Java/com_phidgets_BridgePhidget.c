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
#include "com_phidgets_BridgePhidget.h"
#include "../cphidgetbridge.h"

EVENT_VARS(bridgeData, BridgeData)

JNI_LOAD(bridge, Bridge)
	EVENT_VAR_SETUP(bridge, bridgeData, BridgeData, ID, V)
}

EVENT_HANDLER_INDEXED(Bridge, bridgeData, BridgeData, 
					  CPhidgetBridge_set_OnBridgeData_Handler, double)

JNI_CREATE(Bridge)

JNI_GETFUNC(Bridge, InputCount, InputCount, jint)
JNI_INDEXED_GETFUNC(Bridge, BridgeValue, BridgeValue, jdouble)
JNI_INDEXED_GETFUNC(Bridge, BridgeMin, BridgeMin, jdouble)
JNI_INDEXED_GETFUNC(Bridge, BridgeMax, BridgeMax, jdouble)
JNI_INDEXED_GETFUNCBOOL(Bridge, Enabled, Enabled)
JNI_INDEXED_SETFUNC(Bridge, Enabled, Enabled, jboolean)

JNIEXPORT jint JNICALL
Java_com_phidgets_BridgePhidget_getGain(JNIEnv *env, jobject obj, jint index)
{
	CPhidgetBridgeHandle h = (CPhidgetBridgeHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);
	int error;
	CPhidgetBridge_Gain v;
	if ((error = CPhidgetBridge_getGain(h, index, &v)))
		PH_THROW(error);
	return (jint)v;
}

JNI_INDEXED_SETFUNC(Bridge, Gain, Gain, jint)
JNI_GETFUNC(Bridge, DataRate, DataRate, jint)
JNI_SETFUNC(Bridge, DataRate, DataRate, jint)
JNI_GETFUNC(Bridge, DataRateMin, DataRateMin, jint)
JNI_GETFUNC(Bridge, DataRateMax, DataRateMax, jint)