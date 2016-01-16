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
#include "com_phidgets_RFIDPhidget.h"
#include "../cphidgetrfid.h"

EVENT_VARS(outputChange, OutputChange)
EVENT_VARS(tagLoss, TagLoss)
EVENT_VARS(tagGain, TagGain)

JNI_LOAD(rfid, RFID)
	EVENT_VAR_SETUP(rfid, outputChange, OutputChange, IZ, V)
	EVENT_VAR_SETUP(rfid, tagLoss, TagLoss, Ljava/lang/String;I, V)
	EVENT_VAR_SETUP(rfid, tagGain, TagGain, Ljava/lang/String;I, V)
}

EVENT_HANDLER_INDEXED(RFID, outputChange, OutputChange, 
					  CPhidgetRFID_set_OnOutputChange_Handler, int)

static int CCONV tagLoss_handler (CPhidgetRFIDHandle h, void *arg, char *, CPhidgetRFID_Protocol);
JNIEXPORT void JNICALL Java_com_phidgets_RFIDPhidget_enableTagLossEvents (JNIEnv * env, jobject obj, jboolean b)
{
	jlong gr = updateGlobalRef (env, obj, nativeTagLossHandler_fid, b);
	CPhidgetRFIDHandle h = (CPhidgetRFIDHandle) (uintptr_t) (*env)->GetLongField (env, obj, handle_fid);
	CPhidgetRFID_set_OnTagLost2_Handler (h, b ? tagLoss_handler : 0, (void *) (uintptr_t) gr);
} static int CCONV
tagLoss_handler (CPhidgetRFIDHandle h, void *arg, char *v, CPhidgetRFID_Protocol proto)
{
	JNIEnv *env;
	jobject obj;
	jobject tagLossEv;
	jstring jb;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");
	
	obj = (jobject) arg;

	jb=(*env)->NewStringUTF(env, v);

	if (!(tagLossEv = (*env)->NewObject (env, tagLossEvent_class, tagLossEvent_cons, obj, jb, (int)proto)))
		return -1;
	(*env)->DeleteLocalRef (env, jb);

	(*env)->CallVoidMethod (env, obj, fireTagLoss_mid, tagLossEv);
	(*env)->DeleteLocalRef (env, tagLossEv);
	return 0;
}

static int CCONV tagGain_handler (CPhidgetRFIDHandle h, void *arg, char *, CPhidgetRFID_Protocol);
JNIEXPORT void JNICALL Java_com_phidgets_RFIDPhidget_enableTagGainEvents (JNIEnv * env, jobject obj, jboolean b)
{
	jlong gr = updateGlobalRef (env, obj, nativeTagGainHandler_fid, b);
	CPhidgetRFIDHandle h = (CPhidgetRFIDHandle) (uintptr_t) (*env)->GetLongField (env, obj, handle_fid);
	CPhidgetRFID_set_OnTag2_Handler (h, b ? tagGain_handler : 0, (void *) (uintptr_t) gr);
} static int CCONV
tagGain_handler (CPhidgetRFIDHandle h, void *arg, char *v, CPhidgetRFID_Protocol proto)
{
	JNIEnv *env;
	jobject obj;
	jobject tagGainEv;
	jstring jb;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject) arg;

	jb=(*env)->NewStringUTF(env, v);

	if (!(tagGainEv = (*env)->NewObject (env, tagGainEvent_class, tagGainEvent_cons, obj, jb, (int)proto)))
		return -1;
	(*env)->DeleteLocalRef (env, jb);

	(*env)->CallVoidMethod (env, obj, fireTagGain_mid, tagGainEv);
	(*env)->DeleteLocalRef (env, tagGainEv);

	return 0;
}


JNI_CREATE(RFID)
JNI_INDEXED_GETFUNCBOOL(RFID, OutputState, OutputState)
JNI_INDEXED_SETFUNC(RFID, OutputState, OutputState, jboolean)
JNI_GETFUNCBOOL(RFID, AntennaOn, AntennaOn)
JNI_SETFUNC(RFID, AntennaOn, AntennaOn, jboolean)
JNI_GETFUNCBOOL(RFID, LEDOn, LEDOn)
JNI_GETFUNCBOOL(RFID, TagStatus, TagStatus)
JNI_SETFUNC(RFID, LEDOn, LEDOn, jboolean)
JNI_GETFUNC(RFID, OutputCount, OutputCount, jint)

JNIEXPORT jstring JNICALL
Java_com_phidgets_RFIDPhidget_getLastTag (JNIEnv *env, jobject obj)
{
	CPhidgetRFIDHandle h = (CPhidgetRFIDHandle)(uintptr_t)
	    (*env)->GetLongField( env, obj, handle_fid);
	int error;
	char *tag;
	jstring jb;
	CPhidgetRFID_Protocol proto;
	if ((error = CPhidgetRFID_getLastTag2(h, &tag, &proto)))
		PH_THROW(error);

	jb=(*env)->NewStringUTF(env, tag);
	return jb;
}

JNIEXPORT jint JNICALL
Java_com_phidgets_RFIDPhidget_getLastTagProtocol (JNIEnv *env, jobject obj)
{
	CPhidgetRFIDHandle h = (CPhidgetRFIDHandle)(uintptr_t)
	    (*env)->GetLongField( env, obj, handle_fid);
	int error;
	char *tag;
	CPhidgetRFID_Protocol proto;
	if ((error = CPhidgetRFID_getLastTag2(h, &tag, &proto)))
		PH_THROW(error);

	return (jint)proto;
}

JNIEXPORT void JNICALL Java_com_phidgets_RFIDPhidget_write
  (JNIEnv *env, jobject obj, jstring tag, jint proto, jboolean lock)
{
	int error;
	
    jboolean iscopy;
    const char *tagString = (*env)->GetStringUTFChars(
                env, tag, &iscopy);

	CPhidgetRFIDHandle h = (CPhidgetRFIDHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);

	if ((error = CPhidgetRFID_write(h, (char *)tagString, (CPhidgetRFID_Protocol)proto, (int)lock)))
		PH_THROW(error);

	(*env)->ReleaseStringUTFChars(env, tag, tagString);
}