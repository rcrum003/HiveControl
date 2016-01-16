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
#include "com_phidgets_Phidget.h"
#include "../cphidget.h"
#include "../csocket.h"
#include "phidget_jni.h"

static int CCONV attach_handler(CPhidgetHandle h, void *arg);
static int CCONV detach_handler(CPhidgetHandle h, void *arg);
static int CCONV serverConnect_handler(CPhidgetHandle h, void *arg);
static int CCONV serverDisconnect_handler(CPhidgetHandle h, void *arg);
static int CCONV error_handler(CPhidgetHandle h, void *, int l, const char *arg);

jfieldID handle_fid;
jfieldID managerPhidget_fid;
static jfieldID nativeAttachHandler_fid;
static jfieldID nativeDetachHandler_fid;
static jfieldID nativeErrorHandler_fid;
static jfieldID nativeServerConnectHandler_fid;
static jfieldID nativeServerDisconnectHandler_fid;

static jmethodID fireAttach_mid;
static jmethodID fireDetach_mid;
static jmethodID fireError_mid;
static jmethodID fireServerConnect_mid;
static jmethodID fireServerDisconnect_mid;

void
com_phidgets_Phidget_OnLoad(JNIEnv *env)
{
	if (!(handle_fid = (*env)->GetFieldID(env, phidget_class, "handle", "J")))
		JNI_ABORT_STDERR("Couldn't get Field ID handle from phidget_class");
	if (!(managerPhidget_fid = (*env)->GetFieldID(env, phidget_class, "managerPhidget", "Z")))
		JNI_ABORT_STDERR("Couldn't get Field ID managerPhidget from phidget_class");
	if (!(nativeAttachHandler_fid = (*env)->GetFieldID(env, phidget_class, "nativeAttachHandler", "J")))
		JNI_ABORT_STDERR("Couldn't get Field ID nativeAttachHandler from phidget_class");
	if (!(nativeDetachHandler_fid = (*env)->GetFieldID(env, phidget_class, "nativeDetachHandler", "J")))
		JNI_ABORT_STDERR("Couldn't get Field ID nativeDetachHandler from phidget_class");
	if (!(nativeErrorHandler_fid = (*env)->GetFieldID(env, phidget_class, "nativeErrorHandler", "J")))
		JNI_ABORT_STDERR("Couldn't get Field ID nativeErrorHandler from phidget_class");
	if (!(nativeServerConnectHandler_fid = (*env)->GetFieldID(env, phidget_class, "nativeServerConnectHandler", "J")))
		JNI_ABORT_STDERR("Couldn't get Field ID nativeServerConnectHandler from phidget_class");
	if (!(nativeServerDisconnectHandler_fid = (*env)->GetFieldID(env, phidget_class, "nativeServerDisconnectHandler", "J")))
		JNI_ABORT_STDERR("Couldn't get Field ID nativeServerDisconnectHandler from phidget_class");

	if (!(fireAttach_mid = (*env)->GetMethodID(env, phidget_class, "fireAttach", "(Lcom/phidgets/event/AttachEvent;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID fireAttach from phidget_class");
	if (!(fireDetach_mid = (*env)->GetMethodID(env, phidget_class, "fireDetach", "(Lcom/phidgets/event/DetachEvent;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID fireDetach from phidget_class");
	if (!(fireError_mid = (*env)->GetMethodID(env, phidget_class, "fireError", "(Lcom/phidgets/event/ErrorEvent;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID fireError from phidget_class");
	if (!(fireServerConnect_mid = (*env)->GetMethodID(env, phidget_class, "fireServerConnect", "(Lcom/phidgets/event/ServerConnectEvent;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID fireServerConnect from phidget_class");
	if (!(fireServerDisconnect_mid = (*env)->GetMethodID(env, phidget_class, "fireServerDisconnect", "(Lcom/phidgets/event/ServerDisconnectEvent;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID fireServerDisconnect from phidget_class");
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_enableAttachEvents(JNIEnv *env, jobject obj,
											 jboolean b)
{
	jlong gr = updateGlobalRef(env, obj, nativeAttachHandler_fid, b);
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);

	CPhidget_set_OnAttach_Handler(h, b ? attach_handler : 0,
		(void *)(uintptr_t)gr);
}

static int CCONV
attach_handler(CPhidgetHandle h, void *arg)
{
	JNIEnv *env;
	jobject obj;
	jobject attachEvent;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject)arg;

	if (!(attachEvent = (*env)->NewObject(env, attachEvent_class, attachEvent_cons,
		obj)))
		return -1;
	(*env)->CallVoidMethod(env, obj, fireAttach_mid, attachEvent);
	(*env)->DeleteLocalRef(env, attachEvent);

	return 0;
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_enableDetachEvents(JNIEnv *env, jobject obj,
											 jboolean b)
{
	jlong gr = updateGlobalRef(env, obj, nativeDetachHandler_fid, b);
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);

	CPhidget_set_OnDetach_Handler(h, b ? detach_handler : 0, (void *)(uintptr_t)gr);
}

static int CCONV
detach_handler(CPhidgetHandle h, void *arg)
{
	JNIEnv *env;
	jobject obj;
	jobject detachEvent;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject)arg;

	if (!(detachEvent = (*env)->NewObject(env, detachEvent_class, detachEvent_cons,
		obj)))
		return -1;
	(*env)->CallVoidMethod(env, obj, fireDetach_mid, detachEvent);
	(*env)->DeleteLocalRef(env, detachEvent);

	return 0;
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_enableErrorEvents(JNIEnv *env, jobject obj,
											jboolean b)
{
	jlong gr = updateGlobalRef(env, obj, nativeErrorHandler_fid, b);
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);

	CPhidget_set_OnError_Handler(h, b ? error_handler : 0, (void *)(uintptr_t)gr);
}

static int CCONV
error_handler(CPhidgetHandle h, void *arg, int l, const char *v)
{
	JNIEnv *env;
	jobject obj;
	jobject errorEvent;
	jobject eobj;
	jstring edesc;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject)arg;

	if (!(edesc = (*env)->NewStringUTF(env, v)))
		return -1;

	if (!(eobj = (*env)->NewObject(env, ph_exception_class, ph_exception_cons, l, edesc)))
		return -1;

	if (!(errorEvent = (*env)->NewObject(env, errorEvent_class, errorEvent_cons, obj, eobj)))
		return -1;

	(*env)->DeleteLocalRef (env, edesc);
	(*env)->CallVoidMethod(env, obj, fireError_mid, errorEvent);
	(*env)->DeleteLocalRef(env, errorEvent);

	return 0;
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_enableServerConnectEvents(JNIEnv *env, jobject obj,
													jboolean b)
{
	jlong gr = updateGlobalRef(env, obj, nativeServerConnectHandler_fid, b);
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);

	CPhidget_set_OnServerConnect_Handler(h, b ? serverConnect_handler : 0,
		(void *)(uintptr_t)gr);
}

static int CCONV
serverConnect_handler(CPhidgetHandle h, void *arg)
{
	JNIEnv *env;
	jobject obj;
	jobject serverConnectEvent;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject)arg;

	if (!(serverConnectEvent = (*env)->NewObject(env, serverConnectEvent_class, serverConnectEvent_cons,
		obj)))
		return -1;
	(*env)->CallVoidMethod(env, obj, fireServerConnect_mid, serverConnectEvent);
	(*env)->DeleteLocalRef(env, serverConnectEvent);

	return 0;
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_enableServerDisconnectEvents(JNIEnv *env, jobject obj,
													   jboolean b)
{
	jlong gr = updateGlobalRef(env, obj, nativeServerDisconnectHandler_fid, b);
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);

	CPhidget_set_OnServerDisconnect_Handler(h, b ? serverDisconnect_handler : 0,
		(void *)(uintptr_t)gr);
}

static int CCONV
serverDisconnect_handler(CPhidgetHandle h, void *arg)
{
	JNIEnv *env;
	jobject obj;
	jobject serverDisconnectEvent;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject)arg;

	if (!(serverDisconnectEvent = (*env)->NewObject(env, serverDisconnectEvent_class, serverDisconnectEvent_cons,
		obj)))
		return -1;
	(*env)->CallVoidMethod(env, obj, fireServerDisconnect_mid, serverDisconnectEvent);
	(*env)->DeleteLocalRef(env, serverDisconnectEvent);

	return 0;
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_nativeOpen(JNIEnv *env, jobject obj, jint ser)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;

	if ((error = CPhidget_open(h, ser)))
		PH_THROW(error);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_nativeOpenRemote(JNIEnv *env, jobject obj, jint ser, jstring serverID, jstring pass)
{
	int error;
	jboolean iscopy;
	const char *serverIDcopy = serverID?(*env)->GetStringUTFChars(
		env, serverID, &iscopy):0;
	const char *passcopy = (*env)->GetStringUTFChars(
		env, pass, &iscopy);

	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)
		(*env)->GetLongField(env, obj, handle_fid);

	if ((error = CPhidget_openRemote(h, ser, serverIDcopy, passcopy)))
		PH_THROW(error);

	if(serverID)
		(*env)->ReleaseStringUTFChars(env, serverID, serverIDcopy);
	(*env)->ReleaseStringUTFChars(env, pass, passcopy);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_nativeOpenRemoteIP(JNIEnv *env, jobject obj, jint ser, jstring ipAddr, jint port, jstring pass)
{
	int error;
	jboolean iscopy;
	const char *ipAddrcopy = (*env)->GetStringUTFChars(
		env, ipAddr, &iscopy);
	const char *passcopy = (*env)->GetStringUTFChars(
		env, pass, &iscopy);

	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)
		(*env)->GetLongField(env, obj, handle_fid);

	if ((error = CPhidget_openRemoteIP(h, ser, ipAddrcopy, port, passcopy)))
		PH_THROW(error);

	(*env)->ReleaseStringUTFChars(env, ipAddr, ipAddrcopy);
	(*env)->ReleaseStringUTFChars(env, pass, passcopy);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_nativeOpenLabel(JNIEnv *env, jobject obj, jstring label)
{
	int error;
	jboolean iscopy;
	const char *labelcopy = (*env)->GetStringUTFChars(
		env, label, &iscopy);

	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);

	if ((error = CPhidget_openLabel(h, labelcopy)))
		PH_THROW(error);

	(*env)->ReleaseStringUTFChars(env, label, labelcopy);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_nativeOpenLabelRemote(JNIEnv *env, jobject obj, jstring label, jstring serverID, jstring pass)
{
	int error;
	jboolean iscopy;
	const char *serverIDcopy = serverID?(*env)->GetStringUTFChars(
		env, serverID, &iscopy):0;
	const char *passcopy = (*env)->GetStringUTFChars(
		env, pass, &iscopy);
	const char *labelcopy = (*env)->GetStringUTFChars(
		env, label, &iscopy);

	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)
		(*env)->GetLongField(env, obj, handle_fid);

	if ((error = CPhidget_openLabelRemote(h, labelcopy, serverIDcopy, passcopy)))
		PH_THROW(error);

	if(serverID)
		(*env)->ReleaseStringUTFChars(env, serverID, serverIDcopy);
	(*env)->ReleaseStringUTFChars(env, pass, passcopy);
	(*env)->ReleaseStringUTFChars(env, label, labelcopy);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_nativeOpenLabelRemoteIP(JNIEnv *env, jobject obj, jstring label, jstring ipAddr, jint port, jstring pass)
{
	int error;
	jboolean iscopy;
	const char *ipAddrcopy = (*env)->GetStringUTFChars(
		env, ipAddr, &iscopy);
	const char *passcopy = (*env)->GetStringUTFChars(
		env, pass, &iscopy);
	const char *labelcopy = (*env)->GetStringUTFChars(
		env, label, &iscopy);

	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)
		(*env)->GetLongField(env, obj, handle_fid);

	if ((error = CPhidget_openLabelRemoteIP(h, labelcopy, ipAddrcopy, port, passcopy)))
		PH_THROW(error);

	(*env)->ReleaseStringUTFChars(env, ipAddr, ipAddrcopy);
	(*env)->ReleaseStringUTFChars(env, pass, passcopy);
	(*env)->ReleaseStringUTFChars(env, label, labelcopy);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_nativeClose(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;

	if ((error = CPhidget_close(h)))
		PH_THROW(error);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_nativeDelete(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;

	if ((error = CPhidget_delete(h)))
		PH_THROW(error);
}

JNIEXPORT jint JNICALL
Java_com_phidgets_Phidget_getSerialNumber(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;
	int ser;

	if ((error = CPhidget_getSerialNumber(h, &ser)))
		PH_THROW(error);

	return ser;
}

JNIEXPORT jint JNICALL
Java_com_phidgets_Phidget_getServerPort(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;
	int port;
	const char *addr;

	if ((error = CPhidget_getServerAddress(h, &addr, &port)))
		PH_THROW(error);

	return port;
}

JNIEXPORT jstring JNICALL
Java_com_phidgets_Phidget_getServerAddress(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;
	int port;
	const char *addr;

	if ((error = CPhidget_getServerAddress(h, &addr, &port)))
		PH_THROW(error);

	return (*env)->NewStringUTF(env, addr);
}

JNIEXPORT jstring JNICALL
Java_com_phidgets_Phidget_getServerID(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;
	const char *id;

	if ((error = CPhidget_getServerID(h, &id)))
		PH_THROW(error);

	return (*env)->NewStringUTF(env, id);
}

JNIEXPORT jint JNICALL
Java_com_phidgets_Phidget_getDeviceID(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;
	CPhidget_DeviceID id;

	if ((error = CPhidget_getDeviceID(h, &id)))
		PH_THROW(error);

	return (int)id;
}

JNIEXPORT jint JNICALL
Java_com_phidgets_Phidget_getDeviceClass(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;
	CPhidget_DeviceClass cls;

	if ((error = CPhidget_getDeviceClass(h, &cls)))
		PH_THROW(error);

	return (int)cls;
}

JNIEXPORT jint JNICALL
Java_com_phidgets_Phidget_getDeviceVersion(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;
	int ver;

	if ((error = CPhidget_getDeviceVersion(h, &ver)))
		PH_THROW(error);

	return ver;
}

JNIEXPORT jstring JNICALL
Java_com_phidgets_Phidget_getDeviceType(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	const char *type;
	int error;

	if ((error = CPhidget_getDeviceType(h, &type)))
		PH_THROW(error);

	return (*env)->NewStringUTF(env, type);
}

JNIEXPORT jstring JNICALL
Java_com_phidgets_Phidget_getDeviceName(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	const char *type;
	int error;

	if ((error = CPhidget_getDeviceName(h, &type)))
		PH_THROW(error);

	return (*env)->NewStringUTF(env, type);
}

JNIEXPORT jstring JNICALL
Java_com_phidgets_Phidget_getDeviceLabel(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	const char *label;
	int error;

	if ((error = CPhidget_getDeviceLabel(h, &label)))
		PH_THROW(error);

	return (*env)->NewStringUTF(env, label);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_setDeviceLabel(JNIEnv *env, jobject obj, jstring v)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)
		(*env)->GetLongField(env, obj, handle_fid);
	int error;
	jboolean iscopy;
	const char *textString = (*env)->GetStringUTFChars(
		env, v, &iscopy);
	if ((error = CPhidget_setDeviceLabel(h, (char *)textString)))
		PH_THROW(error);
	(*env)->ReleaseStringUTFChars(env, v, textString);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_nativeEnableLogging(JNIEnv *env, jclass cls, jint level, jstring file)
{
	int error;
	jboolean iscopy;
	const char *textString = file ? (*env)->GetStringUTFChars(env, file, &iscopy) : NULL;
	if ((error = CPhidget_enableLogging(level, (char *)textString)))
		PH_THROW(error);
	if(textString != NULL)
		(*env)->ReleaseStringUTFChars(env, file, textString);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_disableLogging(JNIEnv *env, jclass cls)
{
	int error;
	if ((error = CPhidget_disableLogging()))
		PH_THROW(error);
}

JNIEXPORT void JNICALL
Java_com_phidgets_Phidget_nativeLog(JNIEnv *env, jclass cls, jint level, jstring id, jstring log)
{
	int error;
	jboolean iscopy1;
	const char *textString1 = (*env)->GetStringUTFChars(
		env, id, &iscopy1);
	jboolean iscopy2;
	const char *textString2 = (*env)->GetStringUTFChars(
		env, log, &iscopy2);
	if ((error = CPhidget_log(level, (char *)textString1, (char *)textString2)))
		PH_THROW(error);
	(*env)->ReleaseStringUTFChars(env, id, textString1);
	(*env)->ReleaseStringUTFChars(env, log, textString2);
}

JNIEXPORT jstring JNICALL
Java_com_phidgets_Phidget_getLibraryVersion(JNIEnv *env, jclass cls)
{
	const char *ver;

	CPhidget_getLibraryVersion(&ver);

	return (*env)->NewStringUTF(env, ver);
}

JNIEXPORT jboolean JNICALL
Java_com_phidgets_Phidget_isAttached(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;
	int s;

	if ((error = CPhidget_getDeviceStatus(h, &s)))
		PH_THROW(error);

	return s == PHIDGET_ATTACHED;
}

JNIEXPORT jboolean JNICALL
Java_com_phidgets_Phidget_isAttachedToServer(JNIEnv *env, jobject obj)
{
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env,
		obj, handle_fid);
	int error;
	int s;

	if ((error = CPhidget_getServerStatus(h, &s)))
		PH_THROW(error);

	return s == PHIDGET_ATTACHED;
}

JNIEXPORT void JNICALL 
Java_com_phidgets_Phidget_waitForAttachment(JNIEnv *env, jobject obj, jint timeout)
{
	int error;
	CPhidgetHandle h = (CPhidgetHandle)(uintptr_t)(*env)->GetLongField(env, obj, handle_fid);

	if ((error = CPhidget_waitForAttachment(h, timeout)))
		PH_THROW(error);

	return;
}
