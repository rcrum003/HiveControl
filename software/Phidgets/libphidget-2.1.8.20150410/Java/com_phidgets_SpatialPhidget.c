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
#include "com_phidgets_SpatialPhidget.h"
#include "../cphidgetspatial.h"

EVENT_VARS(spatialData, SpatialData)

static jclass spatialEventData_class; //class
static jmethodID spatialEventData_cons; //constructor

JNI_LOAD(spatial, Spatial)
	EVENT_VAR_SETUP(spatial, spatialData, SpatialData, [Lcom/phidgets/SpatialEventData;, V)

	if (!(spatialEventData_class = (*env)->FindClass(env,"com/phidgets/SpatialEventData")))
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/SpatialEventData");
	if (!(spatialEventData_class = (jclass)(*env)->NewGlobalRef(env, spatialEventData_class)))
		JNI_ABORT_STDERR("Couldn't create global ref spatialEventData_class");
	if (!(spatialEventData_cons = (*env)->GetMethodID(env, spatialEventData_class, "<init>", "([D[D[DII)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from spatialEventData_class");
}

//SpatialData event
static int CCONV spatialData_handler(CPhidgetSpatialHandle h, void *arg, CPhidgetSpatial_SpatialEventDataHandle *, int);
JNIEXPORT void JNICALL
Java_com_phidgets_SpatialPhidget_enableSpatialDataEvents(JNIEnv *env, jobject obj, jboolean b)
{
	jlong gr = updateGlobalRef(env, obj, nativeSpatialDataHandler_fid, b);
	CPhidgetSpatialHandle h = (CPhidgetSpatialHandle)(uintptr_t)(*env)->GetLongField(env, obj, handle_fid);
	CPhidgetSpatial_set_OnSpatialData_Handler(h, b ? spatialData_handler : 0, (void *)(uintptr_t)gr);
}
static int CCONV
spatialData_handler(CPhidgetSpatialHandle h, void *arg, CPhidgetSpatial_SpatialEventDataHandle *data, int dataLength)
{
	JNIEnv *env;
	jobject obj;
	jobject spatialDataEv;
	jobject spatialEventData;
	jobjectArray jo;
	int i, j;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject)arg;

	//create and fill in short array
	jo = (*env)->NewObjectArray(env, dataLength, spatialEventData_class, NULL);
	if (!jo)
		return -1;

	for (i=0; i<dataLength; i++)
	{
		jdoubleArray accel, gyro, mag;
		jdouble *datas;

		//create and fill in data arrays
		accel = (*env)->NewDoubleArray(env, h->phid.attr.spatial.numAccelAxes);
		gyro = (*env)->NewDoubleArray(env, h->phid.attr.spatial.numGyroAxes);
		mag = (*env)->NewDoubleArray(env, data[i]->magneticField[0]==PUNK_DBL?0:h->phid.attr.spatial.numCompassAxes);
		
		if (!accel || !gyro || !mag)
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return -1;
		}
		
		if(h->phid.attr.spatial.numAccelAxes)
		{
			datas = (*env)->GetDoubleArrayElements(env, accel, 0);
			if (!datas)
			{
				PH_THROW(EPHIDGET_UNEXPECTED);
				return -1;
			}
			for (j=0; j<h->phid.attr.spatial.numAccelAxes; j++)
				datas[j] = (jdouble)data[i]->acceleration[j];
			(*env)->ReleaseDoubleArrayElements(env, accel, datas, 0);
		}
		if(h->phid.attr.spatial.numGyroAxes)
		{
			datas = (*env)->GetDoubleArrayElements(env, gyro, 0);
			if (!datas)
			{
				PH_THROW(EPHIDGET_UNEXPECTED);
				return -1;
			}
			for (j=0; j<h->phid.attr.spatial.numGyroAxes; j++)
				datas[j] = (jdouble)data[i]->angularRate[j];
			(*env)->ReleaseDoubleArrayElements(env, gyro, datas, 0);
		}
		if(data[i]->magneticField[0]==PUNK_DBL?0:h->phid.attr.spatial.numCompassAxes)
		{
			datas = (*env)->GetDoubleArrayElements(env, mag, 0);
			if (!datas)
			{
				PH_THROW(EPHIDGET_UNEXPECTED);
				return -1;
			}
			for (j=0; j<h->phid.attr.spatial.numCompassAxes; j++)
				datas[j] = (jdouble)data[i]->magneticField[j];
			(*env)->ReleaseDoubleArrayElements(env, mag, datas, 0);
		}

		if(!(spatialEventData = (*env)->NewObject(env, spatialEventData_class, spatialEventData_cons, accel, gyro, mag, data[i]->timestamp.seconds, data[i]->timestamp.microseconds)))
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return -1;
		}
			
		(*env)->DeleteLocalRef(env, accel);
		(*env)->DeleteLocalRef(env, gyro);
		(*env)->DeleteLocalRef(env, mag);

		//create and add the SpatialEventData object to its array
		(*env)->SetObjectArrayElement(env,jo,i,spatialEventData);
		
		(*env)->DeleteLocalRef(env, spatialEventData);
	}

	if (!(spatialDataEv = (*env)->NewObject(env, spatialDataEvent_class, spatialDataEvent_cons, obj, jo)))
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return -1;
	}
	(*env)->DeleteLocalRef(env, jo);

	(*env)->CallVoidMethod(env, obj, fireSpatialData_mid, spatialDataEv);

	(*env)->DeleteLocalRef(env, spatialDataEv);

	return 0;
}

JNI_CREATE(Spatial)

JNI_INDEXED_GETFUNC(Spatial, AccelerationMax, AccelerationMax, jdouble)
JNI_INDEXED_GETFUNC(Spatial, AccelerationMin, AccelerationMin, jdouble)
JNI_INDEXED_GETFUNC(Spatial, Acceleration, Acceleration, jdouble)
JNI_GETFUNC(Spatial, AccelerationAxisCount, AccelerationAxisCount, jint)

JNI_INDEXED_GETFUNC(Spatial, AngularRateMax, AngularRateMax, jdouble)
JNI_INDEXED_GETFUNC(Spatial, AngularRateMin, AngularRateMin, jdouble)
JNI_INDEXED_GETFUNC(Spatial, AngularRate, AngularRate, jdouble)
JNI_GETFUNC(Spatial, GyroAxisCount, GyroAxisCount, jint)

JNI_INDEXED_GETFUNC(Spatial, MagneticFieldMax, MagneticFieldMax, jdouble)
JNI_INDEXED_GETFUNC(Spatial, MagneticFieldMin, MagneticFieldMin, jdouble)
JNI_INDEXED_GETFUNC(Spatial, MagneticField, MagneticField, jdouble)
JNI_GETFUNC(Spatial, CompassAxisCount, CompassAxisCount, jint)

JNI_GETFUNC(Spatial, DataRateMin, DataRateMin, jint)
JNI_GETFUNC(Spatial, DataRateMax, DataRateMax, jint)
JNI_GETFUNC(Spatial, DataRate, DataRate, jint)
JNI_SETFUNC(Spatial, DataRate, DataRate, jint)

JNIEXPORT void JNICALL Java_com_phidgets_SpatialPhidget_zeroGyro (JNIEnv *env, jobject obj)
{
	CPhidgetSpatialHandle h = (CPhidgetSpatialHandle)(uintptr_t)(*env)->GetLongField( env, obj, handle_fid);
	int error;
	if ((error = CPhidgetSpatial_zeroGyro(h)))
		PH_THROW(error);
}

JNIEXPORT void JNICALL Java_com_phidgets_SpatialPhidget_setCompassCorrectionParameters (JNIEnv *env, jobject obj, 
	jdouble magField, 
	jdouble offset0, jdouble offset1, jdouble offset2, 
	jdouble gain0, jdouble gain1, jdouble gain2, 
	jdouble T0, jdouble T1, jdouble T2, jdouble T3, jdouble T4, jdouble T5)
{
	CPhidgetSpatialHandle h = (CPhidgetSpatialHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);
	int error;
	if ((error = CPhidgetSpatial_setCompassCorrectionParameters(h, magField, offset0, offset1, offset2, gain0, gain1, gain2, T0, T1, T2, T3, T4, T5)))
		PH_THROW(error);
}

JNIEXPORT void JNICALL Java_com_phidgets_SpatialPhidget_resetCompassCorrectionParameters (JNIEnv *env, jobject obj)
{
	CPhidgetSpatialHandle h = (CPhidgetSpatialHandle)(uintptr_t)(*env)->GetLongField( env, obj, handle_fid);
	int error;
	if ((error = CPhidgetSpatial_resetCompassCorrectionParameters(h)))
		PH_THROW(error);
}
