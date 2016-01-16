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
#include <stdlib.h>
#include "../cphidget.h"
#include "phidget_jni.h"
#include <stdio.h>

JavaVM *ph_vm = 0;

jclass phidget_class;
jmethodID phidget_cons;

jclass ph_exception_class;
jmethodID ph_exception_cons;

jclass attachEvent_class;
jmethodID attachEvent_cons;

jclass detachEvent_class;
jmethodID detachEvent_cons;

jclass errorEvent_class;
jmethodID errorEvent_cons;

jclass serverConnectEvent_class;
jmethodID serverConnectEvent_cons;

jclass serverDisconnectEvent_class;
jmethodID serverDisconnectEvent_cons;

#ifdef _WINDOWS

extern int useThreadSecurity;
extern void destroyThreadSecurityAttributes();

#endif

static void detachCurrentThreadFromJavaVM(void)
{
	JNIEnv *env;
	if(ph_vm != NULL && *ph_vm != NULL)
	{
		if((*ph_vm)->GetEnv(ph_vm, (void **)&env, JNI_VERSION_1_4) != JNI_EDETACHED)
		{
			(*ph_vm)->DetachCurrentThread(ph_vm);
		}
	}
}

jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv *env;
	jint version = 0;

	ph_vm = vm;

	// Don't set thread security when we're using Java -  this break events.
	#ifdef _WINDOWS
		useThreadSecurity = PFALSE;
		destroyThreadSecurityAttributes();
	#endif
	
	if((*ph_vm)->GetEnv(ph_vm, (void **)&env, JNI_VERSION_1_4) != JNI_OK)
	{
		return -1;
	}
	
	if(!(version = (*env)->GetVersion(env)))
		JNI_ABORT_STDERR("Couldn't get version");
	LOG(PHIDGET_LOG_DEBUG,"JNI Version: %08x",version);

	//Load all Phidget classes, and needed methods and fields...
	//Phidget
	if (!(phidget_class = (*env)->FindClass(env, "com/phidgets/Phidget")))
		JNI_ABORT_STDERR("");
	if (!(phidget_class = (jclass)(*env)->NewGlobalRef(env, phidget_class)))
		JNI_ABORT_STDERR("");
	if (!(phidget_cons = (*env)->GetMethodID(env, phidget_class, "<init>", "(J)V")))
		JNI_ABORT_STDERR("");

	//PhidgetException
	if (!(ph_exception_class = (*env)->FindClass(env, "com/phidgets/PhidgetException")))
		JNI_ABORT_STDERR("Coulnd't FindClass com/phidgets/PhidgetException");
	if (!(ph_exception_class = (jclass)(*env)->NewGlobalRef(env, ph_exception_class)))
		JNI_ABORT_STDERR("Couldn't create global ref ph_exception_class");
	if (!(ph_exception_cons = (*env)->GetMethodID(env, ph_exception_class, "<init>", "(ILjava/lang/String;)V")))
		JNI_ABORT_STDERR("Couldn't get Method ID <init> from ph_exception_class");

	//AttachEvent
	if (!(attachEvent_class = (*env)->FindClass(env, "com/phidgets/event/AttachEvent")))
		JNI_ABORT_STDERR("Coulnd't FindClass com/phidgets/AttachEvent");
	if (!(attachEvent_class = (jclass)(*env)->NewGlobalRef(env, attachEvent_class)))
		JNI_ABORT_STDERR("Couldn't create global ref attachEvent_class");
	if (!(attachEvent_cons = (*env)->GetMethodID(env, attachEvent_class, "<init>", "(Lcom/phidgets/Phidget;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from attachEvent_class");

	//DetachEvent
	if (!(detachEvent_class = (*env)->FindClass(env, "com/phidgets/event/DetachEvent")))
		JNI_ABORT_STDERR("Coulnd't FindClass com/phidgets/DetachEvent");
	if (!(detachEvent_class = (jclass)(*env)->NewGlobalRef(env, detachEvent_class)))
		JNI_ABORT_STDERR("Couldn't create global ref detachEvent_class");
	if (!(detachEvent_cons = (*env)->GetMethodID(env, detachEvent_class, "<init>", "(Lcom/phidgets/Phidget;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from detachEvent_class");

	//ErrorEvent
	if (!(errorEvent_class = (*env)->FindClass(env, "com/phidgets/event/ErrorEvent")))
		JNI_ABORT_STDERR("Coulnd't FindClass com/phidgets/ErrorEvent");
	if (!(errorEvent_class = (jclass)(*env)->NewGlobalRef(env, errorEvent_class)))
		JNI_ABORT_STDERR("Couldn't create global ref errorEvent_class");
	if (!(errorEvent_cons = (*env)->GetMethodID(env, errorEvent_class, "<init>", "(Lcom/phidgets/Phidget;Lcom/phidgets/PhidgetException;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from errorEvent_class");

	//ServerConnectEvent
	if (!(serverConnectEvent_class = (*env)->FindClass(env, "com/phidgets/event/ServerConnectEvent")))
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/ServerConnectEvent");
	if (!(serverConnectEvent_class = (jclass)(*env)->NewGlobalRef(env, serverConnectEvent_class)))
		JNI_ABORT_STDERR("Couldn't create global ref serverConnectEvent_class");
	if (!(serverConnectEvent_cons = (*env)->GetMethodID(env, serverConnectEvent_class, "<init>", "(Ljava/lang/Object;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from serverConnectEvent_class");

	//ServerDisconnectEvent
	if (!(serverDisconnectEvent_class = (*env)->FindClass(env, "com/phidgets/event/ServerDisconnectEvent")))
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/ServerDisconnectEvent");
	if (!(serverDisconnectEvent_class = (jclass)(*env)->NewGlobalRef(env, serverDisconnectEvent_class)))
		JNI_ABORT_STDERR("Couldn't create global ref serverDisconnectEvent_class");
	if (!(serverDisconnectEvent_cons = (*env)->GetMethodID(env, serverDisconnectEvent_class, "<init>", "(Ljava/lang/Object;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from serverDisconnectEvent_class");

	com_phidgets_Phidget_OnLoad(env);
	com_phidgets_Manager_OnLoad(env);
	com_phidgets_Dictionary_OnLoad(env);
	com_phidgets_DictionaryKeyListener_OnLoad(env);
	com_phidgets_AccelerometerPhidget_OnLoad(env);
	com_phidgets_AdvancedServoPhidget_OnLoad(env);
	com_phidgets_AnalogPhidget_OnLoad(env);
	com_phidgets_BridgePhidget_OnLoad(env);
	com_phidgets_EncoderPhidget_OnLoad(env);
    com_phidgets_FrequencyCounterPhidget_OnLoad(env);
    com_phidgets_GPSPhidget_OnLoad(env);
	com_phidgets_InterfaceKitPhidget_OnLoad(env);
	com_phidgets_IRPhidget_OnLoad(env);
	com_phidgets_LEDPhidget_OnLoad(env);
	com_phidgets_MotorControlPhidget_OnLoad(env);
	com_phidgets_PHSensorPhidget_OnLoad(env);
	com_phidgets_RFIDPhidget_OnLoad(env);
	com_phidgets_ServoPhidget_OnLoad(env);
	com_phidgets_SpatialPhidget_OnLoad(env);
	com_phidgets_StepperPhidget_OnLoad(env);
	com_phidgets_TemperatureSensorPhidget_OnLoad(env);
	com_phidgets_TextLCDPhidget_OnLoad(env);
	com_phidgets_TextLEDPhidget_OnLoad(env);
	com_phidgets_WeightSensorPhidget_OnLoad(env);

	//So the main library can detach threads from java before they exit.
	fptrJavaDetachCurrentThread = detachCurrentThreadFromJavaVM;
	
#ifdef _ANDROID
	if(com_phidgets_usb_Phidget_OnLoad(env) && com_phidgets_usb_Manager_OnLoad(env))
		AndroidUsbJarAvailable = PTRUE;
	else
		AndroidUsbJarAvailable = PFALSE;
#endif

	return JNI_VERSION_1_4;
}

jlong
updateGlobalRef(JNIEnv *env, jobject obj, jfieldID fid, jboolean b)
{
	/*
	 * Manages the global reference held by phidget21 to the handler
	 * target.
	 */
	jlong gr;

	if ((gr = (*env)->GetLongField(env, obj, fid)) != 0)
		(*env)->DeleteGlobalRef(env, (jobject)(uintptr_t)gr);
	gr = b ? (jlong)(uintptr_t)(*env)->NewGlobalRef(env, obj) : 0;
	(*env)->SetLongField(env, obj, fid, gr);

	return gr;
}
