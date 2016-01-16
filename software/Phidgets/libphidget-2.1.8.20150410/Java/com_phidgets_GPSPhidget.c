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
#include "com_phidgets_GPSPhidget.h"
#include "../cphidgetgps.h"

static jclass irCode_class;
static jmethodID irCode_getInstance;
static jmethodID irCode_set;

static jclass javaTimeZone_class; //class
static jmethodID javaTimeZone_getTimeZone;

EVENT_VARS(gpsPositionFixStatusChange, GPSPositionFixStatusChange)
EVENT_VARS(gpsPositionChange, GPSPositionChange)

JNI_LOAD(gps, GPS)

	if (!(irCode_class = (*env)->FindClass(env,"java/util/Calendar")))
		JNI_ABORT_STDERR("Couldn't FindClass java.util.Calendar");
	if (!(irCode_class = (jclass)(*env)->NewGlobalRef(env, irCode_class)))
		JNI_ABORT_STDERR("Couldn't create global ref irCode_class");
	if (!(irCode_getInstance = (*env)->GetStaticMethodID(env, irCode_class, "getInstance", "(Ljava/util/TimeZone;)Ljava/util/Calendar;")))
		JNI_ABORT_STDERR("Couldn't get method ID getInstance");
	if (!(irCode_set = (*env)->GetMethodID(env, irCode_class, "set", "(II)V")))
		JNI_ABORT_STDERR("Couldn't get method ID set");

	if (!(javaTimeZone_class = (*env)->FindClass(env,"java/util/TimeZone")))
		JNI_ABORT_STDERR("Couldn't FindClass java.util.TimeZone");
	if (!(javaTimeZone_class = (jclass)(*env)->NewGlobalRef(env, javaTimeZone_class)))
		JNI_ABORT_STDERR("Couldn't create global ref java.util.TimeZone");
	if (!(javaTimeZone_getTimeZone = (*env)->GetStaticMethodID(env, javaTimeZone_class, "getTimeZone", "(Ljava/lang/String;)Ljava/util/TimeZone;")))
		JNI_ABORT_STDERR("Couldn't get method ID  from getTimeZone");
	
	EVENT_VAR_SETUP(gps, gpsPositionFixStatusChange, GPSPositionFixStatusChange, Z, V)
	EVENT_VAR_SETUP(gps, gpsPositionChange, GPSPositionChange, DDD, V)
}

EVENT_HANDLER(GPS, gpsPositionFixStatusChange, GPSPositionFixStatusChange, CPhidgetGPS_set_OnPositionFixStatusChange_Handler, int)
EVENT_HANDLER_3(GPS, gpsPositionChange, GPSPositionChange, CPhidgetGPS_set_OnPositionChange_Handler, double, double, double)

JNI_CREATE(GPS)

JNI_GETFUNC(GPS, Latitude, Latitude, jdouble)
JNI_GETFUNC(GPS, Longitude, Longitude, jdouble)
JNI_GETFUNC(GPS, Altitude, Altitude, jdouble)
JNI_GETFUNC(GPS, Heading, Heading, jdouble)
JNI_GETFUNC(GPS, Velocity, Velocity, jdouble)
JNI_GETFUNCBOOL(GPS, PositionFixStatus, PositionFixStatus)

JNIEXPORT jobject JNICALL
Java_com_phidgets_GPSPhidget_getDateAndTime(JNIEnv *env, jobject obj)
{
	CPhidgetGPSHandle h = (CPhidgetGPSHandle)(uintptr_t)(*env)->GetLongField( env, obj, handle_fid);
	int error;
	GPSTime cGPSTime;
	GPSDate cGPSDate;
	jobject GPSTimeAndDate, GPSTimeZone;
	
	char* buf;

	if ((error = CPhidgetGPS_getTime(h, &cGPSTime)))
	{
		PH_THROW(error);
		return NULL;
	}

	if ((error = CPhidgetGPS_getDate(h, &cGPSDate)))
	{
		PH_THROW(error);
		return NULL;
	}

	buf = "PST\0";
	//create a Java abstract TimeZone object with a time zone of UTC 
	if(!(GPSTimeZone=(*env)->CallStaticObjectMethod(env, javaTimeZone_class, javaTimeZone_getTimeZone, 
		(*env)->NewStringUTF(env, buf)))){
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}

	//calls the Java static method Calendar.getInstance object said TimeZone
	if (!(GPSTimeAndDate = (*env)->CallStaticObjectMethod(env, irCode_class, irCode_getInstance, GPSTimeZone)))
	{ 
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}
	(*env)->DeleteLocalRef (env, GPSTimeZone);

	(*env)->CallVoidMethod(env, GPSTimeAndDate, irCode_set, 1, cGPSDate.tm_year);  //year
	(*env)->CallVoidMethod(env, GPSTimeAndDate, irCode_set, 2, (cGPSDate.tm_mon-1)); //month(0-11)  
	(*env)->CallVoidMethod(env, GPSTimeAndDate, irCode_set, 5,  cGPSDate.tm_mday);  //day(1-31)

	(*env)->CallVoidMethod(env, GPSTimeAndDate, irCode_set, 11, (jint)cGPSTime.tm_hour);  //hour(0-23)
	(*env)->CallVoidMethod(env, GPSTimeAndDate, irCode_set, 12, cGPSTime.tm_min); //minute(0-59)
	(*env)->CallVoidMethod(env, GPSTimeAndDate, irCode_set, 13,  cGPSTime.tm_sec);  //second(0-59)
	(*env)->CallVoidMethod(env, GPSTimeAndDate, irCode_set, 14,  cGPSTime.tm_ms);  //milliseconds
	
	return GPSTimeAndDate;
}



