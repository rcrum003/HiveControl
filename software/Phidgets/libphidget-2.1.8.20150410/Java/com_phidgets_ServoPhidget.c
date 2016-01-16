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
#include "com_phidgets_ServoPhidget.h"
#include "../cphidgetservo.h"

EVENT_VARS(servoPositionChange, ServoPositionChange)

JNI_LOAD(servo, Servo)
	EVENT_VAR_SETUP(servo, servoPositionChange, ServoPositionChange, ID, V)
}

EVENT_HANDLER_INDEXED(Servo, servoPositionChange, ServoPositionChange, 
					  CPhidgetServo_set_OnPositionChange_Handler, double)

JNI_CREATE(Servo)
JNI_INDEXED_GETFUNC(Servo, Position, Position, jdouble)
JNI_INDEXED_SETFUNC(Servo, Position, Position, jdouble)
JNI_INDEXED_GETFUNC(Servo, PositionMin, PositionMin, jdouble)
JNI_INDEXED_GETFUNC(Servo, PositionMax, PositionMax, jdouble)
JNI_INDEXED_GETFUNCBOOL(Servo, Engaged, Engaged)
JNI_INDEXED_SETFUNC(Servo, Engaged, Engaged, jboolean)
JNI_GETFUNC(Servo, MotorCount, MotorCount, jint)
JNIEXPORT jint JNICALL
Java_com_phidgets_ServoPhidget_getServoType(JNIEnv *env, jobject obj, jint index)
{
	CPhidgetServoHandle h = (CPhidgetServoHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);
	int error;
	CPhidget_ServoType v;
	if ((error = CPhidgetServo_getServoType(h, index, &v)))
		PH_THROW(error);
	return (jint)v;
}
JNI_INDEXED_SETFUNC(Servo, ServoType, ServoType, jint)
JNIEXPORT void JNICALL
Java_com_phidgets_ServoPhidget_setServoParameters(JNIEnv *env, jobject obj, jint index, jdouble minUs, jdouble maxUs, jdouble degrees)
{
	CPhidgetServoHandle h = (CPhidgetServoHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);
	int error;
	if ((error = CPhidgetServo_setServoParameters(h, index, minUs, maxUs, degrees)))
		PH_THROW(error);
}

//Deprecated
JNI_INDEXED_GETFUNCBOOL(Servo, MotorOn, Engaged)
JNI_INDEXED_SETFUNC(Servo, MotorOn, Engaged, jboolean)
