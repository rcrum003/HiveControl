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
#include "com_phidgets_Dictionary.h"
#include "../cphidgetdictionary.h"
#include "../csocket.h"
#include "phidget_jni.h"

static int CCONV
key_handler(CPhidgetDictionaryHandle h, void *arg, const char *key, const char *val, CPhidgetDictionary_keyChangeReason reason);

static jfieldID dictionaryKeyListener_handle_fid;
static jfieldID nativeHandler_fid;
static jfieldID nativeListener_fid;
static jmethodID keyChangeEvent_cons; //constructor
static jmethodID keyRemovalEvent_cons;
static jmethodID fireKeyChange_mid;
static jmethodID fireKeyRemoval_mid;
static jclass dicitonaryKeyListener_class;
static jclass keyChangeEvent_class;
static jclass keyRemovalEvent_class;

void
com_phidgets_DictionaryKeyListener_OnLoad(JNIEnv *env)
{
	//DictionaryKeyListener
	if (!(dicitonaryKeyListener_class = (*env)->FindClass(env, "com/phidgets/DictionaryKeyListener")))
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/DictionaryKeyListener");
	if (!(dicitonaryKeyListener_class = (jclass)(*env)->NewGlobalRef(env, dicitonaryKeyListener_class)))
		JNI_ABORT_STDERR("Couldn't create global ref dicitonaryKeyListener_class");

	if (!(dictionaryKeyListener_handle_fid = (*env)->GetFieldID(env, dicitonaryKeyListener_class, "handle", "J")))
		JNI_ABORT_STDERR("Couldn't get Field ID handle from dicitonaryKeyListener_class");
	if (!(nativeHandler_fid = (*env)->GetFieldID(env, dicitonaryKeyListener_class, "nativeHandler", "J")))
		JNI_ABORT_STDERR("Couldn't get Field ID nativeHandler from dicitonaryKeyListener_class");
	if (!(nativeListener_fid = (*env)->GetFieldID(env, dicitonaryKeyListener_class, "listenerhandle", "J")))
		JNI_ABORT_STDERR("Couldn't get Field ID listenerhandle from dicitonaryKeyListener_class");

	if (!(fireKeyChange_mid = (*env)->GetMethodID(env, dicitonaryKeyListener_class, "fireKeyChange", "(Lcom/phidgets/event/KeyChangeEvent;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID fireKeyChange from dicitonaryKeyListener_class");
	if (!(fireKeyRemoval_mid = (*env)->GetMethodID(env, dicitonaryKeyListener_class, "fireKeyRemoval", "(Lcom/phidgets/event/KeyRemovalEvent;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID fireKeyRemoval from dicitonaryKeyListener_class");

	//KeyChangeEvent
	if (!(keyChangeEvent_class = (*env)->FindClass(env,  "com/phidgets/event/KeyChangeEvent")))
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/event/KeyChangeEvent");
	if (!(keyChangeEvent_class = (jclass)(*env)->NewGlobalRef(env, keyChangeEvent_class)))
		JNI_ABORT_STDERR("Couldn't create global ref keyChangeEvent_class");
	if (!(keyChangeEvent_cons = (*env)->GetMethodID(env, keyChangeEvent_class, "<init>", "(Lcom/phidgets/Dictionary;Ljava/lang/String;Ljava/lang/String;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from keyChangeEvent_class");

	//KeyRemovalEvent
	if (!(keyRemovalEvent_class = (*env)->FindClass(env,  "com/phidgets/event/KeyRemovalEvent")))
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/event/KeyRemovalEvent");
	if (!(keyRemovalEvent_class = (jclass)(*env)->NewGlobalRef(env, keyRemovalEvent_class)))
		JNI_ABORT_STDERR("Couldn't create global ref keyRemovalEvent_class");
	if (!(keyRemovalEvent_cons = (*env)->GetMethodID(env, keyRemovalEvent_class, "<init>", "(Lcom/phidgets/Dictionary;Ljava/lang/String;Ljava/lang/String;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from keyRemovalEvent_class");
}

JNIEXPORT jlong JNICALL
Java_com_phidgets_DictionaryKeyListener_enableDictionaryKeyEvents(JNIEnv *env, jobject obj,
  jboolean b, jstring pattern)
{
	CPhidgetDictionaryListenerHandle keylistener;
	jlong gr = updateGlobalRef(env, obj, nativeHandler_fid, b);
	
    jboolean iscopy;
    const char *textString = (*env)->GetStringUTFChars(
                env, pattern, &iscopy);

	CPhidgetDictionaryHandle h = (CPhidgetDictionaryHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, dictionaryKeyListener_handle_fid);

	if(b)
	{
		CPhidgetDictionary_set_OnKeyChange_Handler(h, &keylistener, textString, b ? key_handler : 0,
			(void *)(uintptr_t)gr);
	}
	else
	{
		keylistener = (CPhidgetDictionaryListenerHandle)(uintptr_t)
			(*env)->GetLongField(env, obj, nativeListener_fid);
		CPhidgetDictionary_remove_OnKeyChange_Handler(keylistener);
		keylistener = NULL;
	}
	
	(*env)->ReleaseStringUTFChars(env, pattern, textString);

	return (jlong)(uintptr_t)keylistener;
}

static int CCONV
key_handler(CPhidgetDictionaryHandle h, void *arg, const char *key, const char *val, CPhidgetDictionary_keyChangeReason reason)
{
	JNIEnv *env;
	jobject obj;
	jobject keyEvent;
	jstring k;
	jstring v;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject)arg;

	k=(*env)->NewStringUTF(env, key);
	v=(*env)->NewStringUTF(env, val);

	switch(reason)
	{
		case PHIDGET_DICTIONARY_ENTRY_REMOVING:
		{
			if (!(keyEvent = (*env)->NewObject(env, keyRemovalEvent_class, keyRemovalEvent_cons,
			  obj, k, v)))
				return -1;
			(*env)->CallVoidMethod(env, obj, fireKeyRemoval_mid, keyEvent);
			break;
		}
		default:
		{
			if (!(keyEvent = (*env)->NewObject(env, keyChangeEvent_class, keyChangeEvent_cons,
			  obj, k, v)))
				return -1;
			(*env)->CallVoidMethod(env, obj, fireKeyChange_mid, keyEvent);
		}
	}

	(*env)->DeleteLocalRef (env, k);
	(*env)->DeleteLocalRef (env, v);

	(*env)->DeleteLocalRef(env, keyEvent);

	return 0;
}
