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

#ifndef PHIDGET_JNI_H
#define PHIDGET_JNI_H

#include <jni.h>

#ifdef _ANDROID
#define JNIEnvPtr const struct JNINativeInterface ***
#else
#define JNIEnvPtr void **
#endif

extern JavaVM *ph_vm;

extern jclass phidget_class;
extern jmethodID phidget_cons;

extern jclass ph_exception_class;
extern jmethodID ph_exception_cons;

extern jclass attachEvent_class;
extern jmethodID attachEvent_cons;

extern jclass detachEvent_class;
extern jmethodID detachEvent_cons;

extern jclass errorEvent_class;
extern jmethodID errorEvent_cons;

extern jclass serverConnectEvent_class;
extern jmethodID serverConnectEvent_cons;

extern jclass serverDisconnectEvent_class;
extern jmethodID serverDisconnectEvent_cons;

void com_phidgets_Phidget_OnLoad(JNIEnv *);
void com_phidgets_AccelerometerPhidget_OnLoad(JNIEnv *);
void com_phidgets_AdvancedServoPhidget_OnLoad(JNIEnv *);
void com_phidgets_AnalogPhidget_OnLoad(JNIEnv *);
void com_phidgets_BridgePhidget_OnLoad(JNIEnv *);
void com_phidgets_EncoderPhidget_OnLoad(JNIEnv *);
void com_phidgets_FrequencyCounterPhidget_OnLoad(JNIEnv *);
void com_phidgets_GPSPhidget_OnLoad(JNIEnv *);
void com_phidgets_InterfaceKitPhidget_OnLoad(JNIEnv *);
void com_phidgets_IRPhidget_OnLoad(JNIEnv *);
void com_phidgets_LEDPhidget_OnLoad(JNIEnv *);
void com_phidgets_MotorControlPhidget_OnLoad(JNIEnv *);
void com_phidgets_PHSensorPhidget_OnLoad(JNIEnv *);
void com_phidgets_RFIDPhidget_OnLoad(JNIEnv *);
void com_phidgets_ServoPhidget_OnLoad(JNIEnv *);
void com_phidgets_SpatialPhidget_OnLoad(JNIEnv *);
void com_phidgets_StepperPhidget_OnLoad(JNIEnv *);
void com_phidgets_TemperatureSensorPhidget_OnLoad(JNIEnv *);
void com_phidgets_TextLCDPhidget_OnLoad(JNIEnv *);
void com_phidgets_TextLEDPhidget_OnLoad(JNIEnv *);
void com_phidgets_WeightSensorPhidget_OnLoad(JNIEnv *);
void com_phidgets_Manager_OnLoad(JNIEnv *);
void com_phidgets_Dictionary_OnLoad(JNIEnv *);
void com_phidgets_DictionaryKeyListener_OnLoad(JNIEnv *);

#ifdef _ANDROID
extern int AndroidUsbJarAvailable;
int com_phidgets_usb_Manager_OnLoad(JNIEnv *);
int com_phidgets_usb_Phidget_OnLoad(JNIEnv *);
#endif

jlong updateGlobalRef(JNIEnv *env, jobject obj, jfieldID fid, jboolean b);

#define JNI_ABORT_STDERR(...) \
do { \
	LOG_STDERR(PHIDGET_LOG_CRITICAL, __VA_ARGS__); \
	(*env)->ExceptionDescribe(env); \
	(*env)->ExceptionClear(env); \
	abort(); \
} while(0)

#define EVENT_VARS(event, Event) static jmethodID event##Event_cons; \
static jmethodID fire##Event##_mid; \
static jclass event##Event_class; \
static jfieldID native##Event##Handler_fid;

#define JNI_LOAD(name, Pname) \
extern jfieldID handle_fid; \
static jclass name##_class; \
void com_phidgets_##Pname##Phidget_OnLoad(JNIEnv *env) \
{ \
	if (!(name##_class = (*env)->FindClass(env, "com/phidgets/" #Pname "Phidget"))) \
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/" #Pname "Phidget"); \
	if (!(name##_class = (jclass)(*env)->NewGlobalRef(env, name##_class))) \
		JNI_ABORT_STDERR("Couldn't create NewGlobalRef " #name "_class"); \

#define EVENT_VAR_SETUP(name, event, Event, parameters, returntype) \
	if (!(event##Event_class = (*env)->FindClass(env, "com/phidgets/event/" #Event "Event"))) \
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/event/" #Event "Event"); \
	if (!(event##Event_class = (jclass)(*env)->NewGlobalRef(env, event##Event_class))) \
		JNI_ABORT_STDERR("Couldn't create global ref " #event "Event_class"); \
	if (!(fire##Event##_mid = (*env)->GetMethodID(env, name##_class, "fire" #Event , "(Lcom/phidgets/event/" #Event "Event;)" #returntype ))) \
		JNI_ABORT_STDERR("Please install the latest Phidget Library. Couldn't get method ID fire" #Event); \
	if (!(event##Event_cons = (*env)->GetMethodID(env, event##Event_class, "<init>", "(Lcom/phidgets/Phidget;" #parameters ")" #returntype ))) \
		JNI_ABORT_STDERR("Couldn't get method ID <init> from " #event "Event_class"); \
	if (!(native##Event##Handler_fid = (*env)->GetFieldID(env, name##_class, "native" #Event "Handler", "J"))) \
		JNI_ABORT_STDERR("Couldn't get Field ID native" #Event "Handler from " #name "_class");

#define PH_THROW(errno) { \
	jobject eobj; \
	jstring edesc; \
 \
	if (!(edesc = (*env)->NewStringUTF(env, CPhidget_strerror(errno)))) \
		JNI_ABORT_STDERR("Couldn't get NewStringUTF"); \
	if (!(eobj = (*env)->NewObject(env, ph_exception_class, ph_exception_cons, errno, edesc))) \
		JNI_ABORT_STDERR("Couldn't get NewObject ph_exception_class"); \
	(*env)->DeleteLocalRef (env, edesc); \
	(*env)->Throw(env, (jthrowable)eobj); \
	/*(*env)->ExceptionClear(env);*/ \
}

#define JNI_INDEXED_SETFUNC(pname, fname, lfname, type) \
JNIEXPORT void JNICALL Java_com_phidgets_##pname##Phidget_set##fname(JNIEnv *env, jobject obj, jint index, type v) \
{ \
	CPhidget##pname##Handle h = (CPhidget##pname##Handle)(uintptr_t)(*env)->GetLongField(env, obj, handle_fid); \
	int error; \
	if ((error = CPhidget##pname##_set##lfname(h, index, v))) \
		PH_THROW(error); \
}

#define JNI_SETFUNC(pname, fname, lfname, type) \
JNIEXPORT void JNICALL Java_com_phidgets_##pname##Phidget_set##fname(JNIEnv *env, jobject obj, type v) \
{ \
	CPhidget##pname##Handle h = (CPhidget##pname##Handle)(uintptr_t) \
	    (*env)->GetLongField(env, obj, handle_fid); \
	int error; \
	if ((error = CPhidget##pname##_set##lfname(h, v))) \
		PH_THROW(error); \
}

#define JNI_INDEXED_GETFUNC(pname, fname, lfname, type) \
JNIEXPORT type JNICALL Java_com_phidgets_##pname##Phidget_get##fname(JNIEnv *env, jobject obj, jint index) \
{ \
	CPhidget##pname##Handle h = (CPhidget##pname##Handle)(uintptr_t) \
	    (*env)->GetLongField(env, obj, handle_fid); \
	int error; \
	type v; \
	if ((error = CPhidget##pname##_get##lfname(h, index, &v))) \
		PH_THROW(error); \
	return v; \
}

#define JNI_INDEXED_GETFUNCBOOL(pname, fname, lfname) \
JNIEXPORT jboolean JNICALL Java_com_phidgets_##pname##Phidget_get##fname(JNIEnv *env, jobject obj, jint index) \
{ \
	CPhidget##pname##Handle h = (CPhidget##pname##Handle)(uintptr_t) \
	    (*env)->GetLongField(env, obj, handle_fid); \
	int error; \
	int v; \
	if ((error = CPhidget##pname##_get##lfname(h, index, &v))) \
		PH_THROW(error); \
	if (v) return 1; \
	return 0; \
}

#define JNI_GETFUNC(pname, fname, lfname, type) \
JNIEXPORT type JNICALL Java_com_phidgets_##pname##Phidget_get##fname (JNIEnv *env, jobject obj) \
{ \
	CPhidget##pname##Handle h = (CPhidget##pname##Handle)(uintptr_t) \
	    (*env)->GetLongField( env, obj, handle_fid); \
	int error; \
	type no; \
	if ((error = CPhidget##pname##_get##lfname(h, &no))) \
		PH_THROW(error); \
	return no; \
}

#define JNI_GETFUNCBOOL(pname, fname, lfname) \
JNIEXPORT jboolean JNICALL Java_com_phidgets_##pname##Phidget_get##fname (JNIEnv *env, jobject obj) \
{ \
	CPhidget##pname##Handle h = (CPhidget##pname##Handle)(uintptr_t) \
	    (*env)->GetLongField( env, obj, handle_fid); \
	int error; \
	int no; \
	if ((error = CPhidget##pname##_get##lfname(h, &no))) \
		PH_THROW(error); \
	if(no) return 1; \
	return 0; \
}

#define JNI_CREATE(Pname) \
JNIEXPORT jlong JNICALL Java_com_phidgets_##Pname##Phidget_create(JNIEnv *env, jclass cls) \
{ \
	CPhidget##Pname##Handle phid; \
	int error; \
	if ((error = CPhidget##Pname##_create(&phid)) != EPHIDGET_OK) { \
		PH_THROW(error); \
		return 0; \
	} \
	return (jlong)(uintptr_t)phid; \
}

#define EVENT_HANDLER(pname, event, Event, cphidgetSetHandlerFunc, type) \
static int CCONV event##_handler(CPhidget##pname##Handle h, void *arg, type); \
JNIEXPORT void JNICALL Java_com_phidgets_##pname##Phidget_enable##Event##Events(JNIEnv *env, jobject obj, jboolean b) \
{ \
	jlong gr = updateGlobalRef(env, obj, native##Event##Handler_fid, b); \
	CPhidget##pname##Handle h = (CPhidget##pname##Handle)(uintptr_t) \
	    (*env)->GetLongField(env, obj, handle_fid); \
	cphidgetSetHandlerFunc(h, b ? event##_handler : 0, \
	    (void *)(uintptr_t)gr); \
} \
\
static int CCONV event##_handler(CPhidget##pname##Handle h, void *arg, type v) \
{ \
	JNIEnv *env; \
	jobject obj; \
	jobject event##Ev; \
\
	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL)) \
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread"); \
\
	obj = (jobject)arg; \
\
	if (!(event##Ev = (*env)->NewObject(env, event##Event_class, event##Event_cons, obj, v))) \
		return -1; \
	(*env)->CallVoidMethod(env, obj, fire##Event##_mid, event##Ev); \
	(*env)->DeleteLocalRef(env, event##Ev); \
\
	return 0; \
}

#define EVENT_HANDLER_INDEXED(pname, event, Event, cphidgetSetHandlerFunc, type) \
static int CCONV event##_handler(CPhidget##pname##Handle h, void *arg, int, type); \
JNIEXPORT void JNICALL Java_com_phidgets_##pname##Phidget_enable##Event##Events(JNIEnv *env, jobject obj, jboolean b) \
{ \
	jlong gr = updateGlobalRef(env, obj, native##Event##Handler_fid, b); \
	CPhidget##pname##Handle h = (CPhidget##pname##Handle)(uintptr_t) \
	    (*env)->GetLongField(env, obj, handle_fid); \
	cphidgetSetHandlerFunc(h, b ? event##_handler : 0, \
	    (void *)(uintptr_t)gr); \
} \
\
static int CCONV event##_handler(CPhidget##pname##Handle h, void *arg, int index, type v) \
{ \
	JNIEnv *env; \
	jobject obj; \
	jobject event##Ev; \
\
	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL)) \
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread"); \
\
	obj = (jobject)arg; \
\
	if (!(event##Ev = (*env)->NewObject(env, event##Event_class, event##Event_cons, obj, index, v))) \
		return -1; \
	(*env)->CallVoidMethod(env, obj, fire##Event##_mid, event##Ev); \
	(*env)->DeleteLocalRef(env, event##Ev); \
\
	return 0; \
}

#define EVENT_HANDLER_INDEXED2(pname, event, Event, cphidgetSetHandlerFunc, type, type2) \
static int CCONV event##_handler(CPhidget##pname##Handle h, void *arg, int, type, type2); \
JNIEXPORT void JNICALL Java_com_phidgets_##pname##Phidget_enable##Event##Events(JNIEnv *env, jobject obj, jboolean b) \
{ \
	jlong gr = updateGlobalRef(env, obj, native##Event##Handler_fid, b); \
	CPhidget##pname##Handle h = (CPhidget##pname##Handle)(uintptr_t) \
	    (*env)->GetLongField(env, obj, handle_fid); \
	cphidgetSetHandlerFunc(h, b ? event##_handler : 0, \
	    (void *)(uintptr_t)gr); \
} \
\
static int CCONV event##_handler(CPhidget##pname##Handle h, void *arg, int index, type v, type2 w) \
{ \
	JNIEnv *env; \
	jobject obj; \
	jobject event##Ev; \
\
	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL)) \
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread"); \
\
	obj = (jobject)arg; \
\
	if (!(event##Ev = (*env)->NewObject(env, event##Event_class, \
	    event##Event_cons, obj, index, v, w))) \
		return -1; \
	(*env)->CallVoidMethod(env, obj, fire##Event##_mid, event##Ev); \
	(*env)->DeleteLocalRef(env, event##Ev); \
\
	return 0; \
}

#define EVENT_HANDLER_3(pname, event, Event, cphidgetSetHandlerFunc, type, type2, type3) \
static int CCONV event##_handler(CPhidget##pname##Handle h, void *arg, type, type2, type3); \
JNIEXPORT void JNICALL Java_com_phidgets_##pname##Phidget_enable##Event##Events(JNIEnv *env, jobject obj, jboolean b) \
{ \
	jlong gr = updateGlobalRef(env, obj, native##Event##Handler_fid, b); \
	CPhidget##pname##Handle h = (CPhidget##pname##Handle)(uintptr_t) \
	    (*env)->GetLongField(env, obj, handle_fid); \
	cphidgetSetHandlerFunc(h, b ? event##_handler : 0, \
	    (void *)(uintptr_t)gr); \
} \
\
static int CCONV event##_handler(CPhidget##pname##Handle h, void *arg, type v, type2 w, type3 x) \
{ \
	JNIEnv *env; \
	jobject obj; \
	jobject event##Ev; \
\
	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL)) \
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread"); \
\
	obj = (jobject)arg; \
\
	if (!(event##Ev = (*env)->NewObject(env, event##Event_class, \
	    event##Event_cons, obj, v, w, x))) \
		return -1; \
	(*env)->CallVoidMethod(env, obj, fire##Event##_mid, event##Ev); \
	(*env)->DeleteLocalRef(env, event##Ev); \
\
	return 0; \
}


#endif
