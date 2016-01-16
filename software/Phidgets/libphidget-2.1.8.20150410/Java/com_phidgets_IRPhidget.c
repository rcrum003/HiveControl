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
#include "com_phidgets_IRPhidget.h"
#include "../cphidgetir.h"

EVENT_VARS(code, Code)
EVENT_VARS(learn, Learn)
EVENT_VARS(rawData, RawData)

static jclass irCode_class; //class
static jmethodID irCode_cons; //constructor
static jmethodID irCode_getData;
static jclass irCodeInfo_class; //class
static jmethodID irCodeInfo_cons; //constructor
static jmethodID irCodeInfo_getEncoding;
static jmethodID irCodeInfo_getLength;
static jmethodID irCodeInfo_getBitCount;
static jmethodID irCodeInfo_getGap;
static jmethodID irCodeInfo_getTrail;
static jmethodID irCodeInfo_getMinRepeat;
static jmethodID irCodeInfo_getCarrierFrequency;
static jmethodID irCodeInfo_getDutyCycle;
static jmethodID irCodeInfo_getHeader;
static jmethodID irCodeInfo_getZero;
static jmethodID irCodeInfo_getOne;
static jmethodID irCodeInfo_getRepeat;
static jmethodID irCodeInfo_getToggleMask;
static jclass irLearn_class; //class
static jmethodID irLearn_cons; //constructor

JNI_LOAD(ir, IR)
	EVENT_VAR_SETUP(ir, learn, Learn, Lcom/phidgets/IRLearnedCode;, V)
	EVENT_VAR_SETUP(ir, rawData, RawData, [I, V)
	EVENT_VAR_SETUP(ir, code, Code, Lcom/phidgets/IRCode;Z, V)

	if (!(irCode_class = (*env)->FindClass(env,"com/phidgets/IRCode")))
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/IRCode");
	if (!(irCode_class = (jclass)(*env)->NewGlobalRef(env, irCode_class)))
		JNI_ABORT_STDERR("Couldn't create global ref irCode_class");
	if (!(irCode_cons = (*env)->GetMethodID(env, irCode_class, "<init>", "([SI)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from irCode_class");

	if (!(irCode_getData = (*env)->GetMethodID(env, irCode_class, "getData", "()[S")))
		JNI_ABORT_STDERR("Couldn't get method ID getData from irCode_class");

	if (!(irCodeInfo_class = (*env)->FindClass(env,"com/phidgets/IRCodeInfo")))
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/IRCodeInfo");
	if (!(irCodeInfo_class = (jclass)(*env)->NewGlobalRef(env, irCodeInfo_class)))
		JNI_ABORT_STDERR("Couldn't create global ref irCodeInfo_class");
	if (!(irCodeInfo_cons = (*env)->GetMethodID(env, irCodeInfo_class, "<init>", "(II[I[I[III[IILcom/phidgets/IRCode;III)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from irCodeInfo_class");

	if (!(irCodeInfo_getEncoding = (*env)->GetMethodID(env, irCodeInfo_class, "getEncoding", "()I")))
		JNI_ABORT_STDERR("Couldn't get method ID getEncoding from irCodeInfo_class");
	if (!(irCodeInfo_getLength = (*env)->GetMethodID(env, irCodeInfo_class, "getLength", "()I")))
		JNI_ABORT_STDERR("Couldn't get method ID getLength from irCodeInfo_class");
	if (!(irCodeInfo_getBitCount = (*env)->GetMethodID(env, irCodeInfo_class, "getBitCount", "()I")))
		JNI_ABORT_STDERR("Couldn't get method ID getBitCount from irCodeInfo_class");
	if (!(irCodeInfo_getGap = (*env)->GetMethodID(env, irCodeInfo_class, "getGap", "()I")))
		JNI_ABORT_STDERR("Couldn't get method ID getGap from irCodeInfo_class");
	if (!(irCodeInfo_getTrail = (*env)->GetMethodID(env, irCodeInfo_class, "getTrail", "()I")))
		JNI_ABORT_STDERR("Couldn't get method ID getTrail from irCodeInfo_class");
	if (!(irCodeInfo_getMinRepeat = (*env)->GetMethodID(env, irCodeInfo_class, "getMinRepeat", "()I")))
		JNI_ABORT_STDERR("Couldn't get method ID getMinRepeat from irCodeInfo_class");
	if (!(irCodeInfo_getCarrierFrequency = (*env)->GetMethodID(env, irCodeInfo_class, "getCarrierFrequency", "()I")))
		JNI_ABORT_STDERR("Couldn't get method ID getCarrierFrequency from irCodeInfo_class");
	if (!(irCodeInfo_getDutyCycle = (*env)->GetMethodID(env, irCodeInfo_class, "getDutyCycle", "()I")))
		JNI_ABORT_STDERR("Couldn't get method ID getDutyCycle from irCodeInfo_class");
	if (!(irCodeInfo_getHeader = (*env)->GetMethodID(env, irCodeInfo_class, "getHeader", "()[I")))
		JNI_ABORT_STDERR("Couldn't get method ID getHeader from irCodeInfo_class");
	if (!(irCodeInfo_getZero = (*env)->GetMethodID(env, irCodeInfo_class, "getZero", "()[I")))
		JNI_ABORT_STDERR("Couldn't get method ID getZero from irCodeInfo_class");
	if (!(irCodeInfo_getOne = (*env)->GetMethodID(env, irCodeInfo_class, "getOne", "()[I")))
		JNI_ABORT_STDERR("Couldn't get method ID getOne from irCodeInfo_class");
	if (!(irCodeInfo_getRepeat = (*env)->GetMethodID(env, irCodeInfo_class, "getRepeat", "()[I")))
		JNI_ABORT_STDERR("Couldn't get method ID getRepeat from irCodeInfo_class");
	if (!(irCodeInfo_getToggleMask = (*env)->GetMethodID(env, irCodeInfo_class, "getToggleMask", "()Lcom/phidgets/IRCode;")))
		JNI_ABORT_STDERR("Couldn't get method ID getToggleMask from irCodeInfo_class");

	if (!(irLearn_class = (*env)->FindClass(env,"com/phidgets/IRLearnedCode")))
		JNI_ABORT_STDERR("Couldn't FindClass com/phidgets/IRLearnedCode");
	if (!(irLearn_class = (jclass)(*env)->NewGlobalRef(env, irLearn_class)))
		JNI_ABORT_STDERR("Couldn't create global ref irLearn_class");
	if (!(irLearn_cons = (*env)->GetMethodID(env, irLearn_class, "<init>", "(Lcom/phidgets/IRCode;Lcom/phidgets/IRCodeInfo;)V")))
		JNI_ABORT_STDERR("Couldn't get method ID <init> from irLearn_class");
}

//Code event
static int CCONV code_handler(CPhidgetIRHandle h, void *arg, unsigned char *data, int dataLength, int bitCount, int repeat);
JNIEXPORT void JNICALL
Java_com_phidgets_IRPhidget_enableCodeEvents(JNIEnv *env, jobject obj, jboolean b)
{
	jlong gr = updateGlobalRef(env, obj, nativeCodeHandler_fid, b);
	CPhidgetIRHandle h = (CPhidgetIRHandle)(uintptr_t)(*env)->GetLongField(env, obj, handle_fid);
	CPhidgetIR_set_OnCode_Handler(h, b ? code_handler : 0, (void *)(uintptr_t)gr);
}
static int CCONV
code_handler(CPhidgetIRHandle h, void *arg, unsigned char *data, int dataLength, int bitCount, int repeat)
{
	JNIEnv *env;
	jobject obj;
	jobject codeEv;
	jobject ircode;
	jshortArray js;
	jshort *datas;
	int i;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject)arg;

	//create and fill in short array
	js = (*env)->NewShortArray(env, dataLength);
	if (!js)
		return -1;
	datas = (*env)->GetShortArrayElements(env, js, 0);
	if (!datas)
		return -1;
	for (i=0; i<dataLength; i++)
		datas[i] = (jshort)data[i];
	(*env)->ReleaseShortArrayElements(env, js, datas, 0);

	if (!(ircode = (*env)->NewObject(env, irCode_class, irCode_cons, js, bitCount)))
	{
		return -1;
	}
	(*env)->DeleteLocalRef(env, js);

	if (!(codeEv = (*env)->NewObject(env, codeEvent_class,
	    codeEvent_cons, obj, ircode, repeat)))
	{
		(*env)->DeleteLocalRef(env, ircode);
		return -1;
	}
	(*env)->DeleteLocalRef(env, ircode);

	(*env)->CallVoidMethod(env, obj, fireCode_mid, codeEv);

	(*env)->DeleteLocalRef(env, codeEv);

	return 0;

}

//Learn event
static int CCONV learn_handler(CPhidgetIRHandle h, void *arg, unsigned char *data, int dataLength, CPhidgetIR_CodeInfoHandle codeInfo);
JNIEXPORT void JNICALL
Java_com_phidgets_IRPhidget_enableLearnEvents(JNIEnv *env, jobject obj, jboolean b)
{
	jlong gr = updateGlobalRef(env, obj, nativeLearnHandler_fid, b);
	CPhidgetIRHandle h = (CPhidgetIRHandle)(uintptr_t)(*env)->GetLongField(env, obj, handle_fid);
	CPhidgetIR_set_OnLearn_Handler(h, b ? learn_handler : 0, (void *)(uintptr_t)gr);
}
static int CCONV
learn_handler(CPhidgetIRHandle h, void *arg, unsigned char *data, int dataLength, CPhidgetIR_CodeInfoHandle codeInfo)
{
	JNIEnv *env;
	jobject obj;
	jobject learnEv;
	jobject ircode, ircodeinfo, irlearn, togglemask;
	jshortArray codeData;
	jintArray headerArray, zeroArray, oneArray, repeatArray;
	jshort *datas;
	jint *datai;
	int i,j;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject)arg;

	//create and fill in short array
	if(dataLength > 0){
		codeData = (*env)->NewShortArray(env, dataLength);
		if (!codeData)
			return -1;
		datas = (*env)->GetShortArrayElements(env, codeData, 0);
		if (!datas)
			return -1;
		for (i=0; i<dataLength; i++)
			datas[i] = (jshort)data[i];
		(*env)->ReleaseShortArrayElements(env, codeData, datas, 0);
	}
	else
		codeData=NULL;
	//create IRCode object
	if (!(ircode = (*env)->NewObject(env, irCode_class, irCode_cons, codeData, codeInfo->bitCount)))
	{
		return -1;
	}
	if(codeData)
		(*env)->DeleteLocalRef(env, codeData);

	//Header
	if(codeInfo->header[0])
	{
		headerArray = (*env)->NewIntArray(env, 2);
		if (!headerArray)
			return -1;
		datai = (*env)->GetIntArrayElements(env, headerArray, 0);
		if (!datai)
			return -1;
		for (i=0; i<2; i++)
			datai[i] = (jint)codeInfo->header[i];
		(*env)->ReleaseIntArrayElements(env, headerArray, datai, 0);
	}
	else
		headerArray=NULL;

	//Zero
	zeroArray = (*env)->NewIntArray(env, 2);
	if (!zeroArray)
		return -1;
	datai = (*env)->GetIntArrayElements(env, zeroArray, 0);
	if (!datai)
		return -1;
	for (i=0; i<2; i++)
		datai[i] = (jint)codeInfo->zero[i];
	(*env)->ReleaseIntArrayElements(env, zeroArray, datai, 0);

	//One
	oneArray = (*env)->NewIntArray(env, 2);
	if (!oneArray)
		return -1;
	datai = (*env)->GetIntArrayElements(env, oneArray, 0);
	if (!datai)
		return -1;
	for (i=0; i<2; i++)
		datai[i] = (jint)codeInfo->one[i];
	(*env)->ReleaseIntArrayElements(env, oneArray, datai, 0);

	//Repeat
	i=0;
	while(codeInfo->repeat[i])
		i++;
	if(i)
	{
		repeatArray = (*env)->NewIntArray(env, i);
		if (!repeatArray)
			return -1;
		datai = (*env)->GetIntArrayElements(env, repeatArray, 0);
		if (!datai)
			return -1;
		for (j=0; j<i; j++)
			datai[j] = (jint)codeInfo->repeat[j];
		(*env)->ReleaseIntArrayElements(env, repeatArray, datai, 0);
	}
	else{
		repeatArray = NULL;
	}

	//ToggleMask
	if(dataLength > 0){
		codeData = (*env)->NewShortArray(env, dataLength);
		if (!codeData)
			return -1;
		datas = (*env)->GetShortArrayElements(env, codeData, 0);
		if (!datas)
			return -1;
		for (i=0; i<dataLength; i++)
			datas[i] = (jshort)codeInfo->toggle_mask[i];
		(*env)->ReleaseShortArrayElements(env, codeData, datas, 0);
		if (!(togglemask = (*env)->NewObject(env, irCode_class, irCode_cons, codeData, codeInfo->bitCount)))
		{
			return -1;
		}
		(*env)->DeleteLocalRef(env, codeData);
	}
	else{
		togglemask=NULL;
	}

	//create IRCodeInfo object
	if (!(ircodeinfo = (*env)->NewObject(env, irCodeInfo_class, irCodeInfo_cons, 
		codeInfo->encoding, codeInfo->bitCount, headerArray, zeroArray, oneArray,
		codeInfo->trail, codeInfo->gap, repeatArray, codeInfo->min_repeat, togglemask,
		codeInfo->length, codeInfo->carrierFrequency, codeInfo->dutyCycle)))
	{
		(*env)->DeleteLocalRef(env, ircode);
		return -1;
	}

	if(headerArray)
		(*env)->DeleteLocalRef(env, headerArray);
	(*env)->DeleteLocalRef(env, zeroArray);
	(*env)->DeleteLocalRef(env, oneArray);
	if(repeatArray)
		(*env)->DeleteLocalRef(env, repeatArray);
	if(togglemask)
		(*env)->DeleteLocalRef(env, togglemask);

	//create IRLearn object
	if (!(irlearn = (*env)->NewObject(env, irLearn_class, irLearn_cons, ircode, ircodeinfo)))
	{
		(*env)->DeleteLocalRef(env, ircode);
		(*env)->DeleteLocalRef(env, ircodeinfo);
		return -1;
	}
	(*env)->DeleteLocalRef(env, ircode);
	(*env)->DeleteLocalRef(env, ircodeinfo);

	if (!(learnEv = (*env)->NewObject(env, learnEvent_class,
	    learnEvent_cons, obj, irlearn)))
	{
		(*env)->DeleteLocalRef(env, irlearn);
		return -1;
	}
	(*env)->DeleteLocalRef(env, irlearn);

	(*env)->CallVoidMethod(env, obj, fireLearn_mid, learnEv);

	(*env)->DeleteLocalRef(env, learnEv);

	return 0;
}

//RawData event
static int CCONV rawData_handler(CPhidgetIRHandle h, void *arg, int *data, int dataLength);
JNIEXPORT void JNICALL
Java_com_phidgets_IRPhidget_enableRawDataEvents(JNIEnv *env, jobject obj, jboolean b)
{
	jlong gr = updateGlobalRef(env, obj, nativeRawDataHandler_fid, b);
	CPhidgetIRHandle h = (CPhidgetIRHandle)(uintptr_t)(*env)->GetLongField(env, obj, handle_fid);
	CPhidgetIR_set_OnRawData_Handler(h, b ? rawData_handler : 0, (void *)(uintptr_t)gr);
}
static int CCONV
rawData_handler(CPhidgetIRHandle h, void *arg, int *data, int dataLength)
{
	JNIEnv *env;
	jobject obj;
	jobject rawDataEv;
	jintArray js;
	jint *datas;
	int i;

	if ((*ph_vm)->AttachCurrentThread(ph_vm, (JNIEnvPtr)&env, NULL))
		JNI_ABORT_STDERR("Couldn't AttachCurrentThread");

	obj = (jobject)arg;

	//create and fill in int array
	js = (*env)->NewIntArray(env, dataLength);
	if (!js)
		return -1;
	datas = (*env)->GetIntArrayElements(env, js, 0);
	if (!datas)
		return -1;
	for (i=0; i<dataLength; i++)
		datas[i] = (jint)data[i];
	(*env)->ReleaseIntArrayElements(env, js, datas, 0);

	if (!(rawDataEv = (*env)->NewObject(env, rawDataEvent_class, rawDataEvent_cons, obj, js)))
		return -1;
	(*env)->DeleteLocalRef(env, js);

	(*env)->CallVoidMethod(env, obj, fireRawData_mid, rawDataEv);

	(*env)->DeleteLocalRef(env, rawDataEv);

	return 0;
}

JNI_CREATE(IR)

JNIEXPORT jobject JNICALL
Java_com_phidgets_IRPhidget_getLastCode(JNIEnv *env, jobject obj)
{
	CPhidgetIRHandle h = (CPhidgetIRHandle)(uintptr_t)(*env)->GetLongField( env, obj, handle_fid);
	int error;
	unsigned char data[IR_MAX_CODE_DATA_LENGTH];
	int dataLength=IR_MAX_CODE_DATA_LENGTH, bitCount, i;
	jobject ircode;
	jshortArray js;
	jshort *datas;

	if ((error = CPhidgetIR_getLastCode(h, data, &dataLength, &bitCount)))
	{
		PH_THROW(error);
		return NULL;
	}

	//create and fill in short array
	js = (*env)->NewShortArray(env, dataLength);
	if (!js)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}
	datas = (*env)->GetShortArrayElements(env, js, 0);
	if (!datas)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}
	for (i=0; i<dataLength; i++)
		datas[i] = (jshort)data[i];
	(*env)->ReleaseShortArrayElements(env, js, datas, 0);

	//create and return IRCode object
	if (!(ircode = (*env)->NewObject(env, irCode_class, irCode_cons, js, bitCount)))
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}

	return ircode;
}

JNIEXPORT jint JNICALL
Java_com_phidgets_IRPhidget_readRaw(JNIEnv *env, jobject obj, jintArray intArray, jint offset, jint count)
{
	CPhidgetIRHandle h = (CPhidgetIRHandle)(uintptr_t)(*env)->GetLongField( env, obj, handle_fid);
	int error, dataLength = count;
	jint *datai;

	//create and fill in int array
	datai = (*env)->GetIntArrayElements(env, intArray, 0);
	if (!datai)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return -1;
	}
	if ((error = CPhidgetIR_getRawData(h, datai, &dataLength)))
	{
		(*env)->ReleaseIntArrayElements(env, intArray, datai, 0);
		PH_THROW(error);
		return -1;
	}
	(*env)->ReleaseIntArrayElements(env, intArray, datai, 0);

	return dataLength;
}

JNIEXPORT jobject JNICALL
Java_com_phidgets_IRPhidget_getLastLearnedCode(JNIEnv *env, jobject obj)
{
	CPhidgetIRHandle h = (CPhidgetIRHandle)(uintptr_t)(*env)->GetLongField( env, obj, handle_fid);
	int error;
	unsigned char data[IR_MAX_CODE_DATA_LENGTH];
	int dataLength=IR_MAX_CODE_DATA_LENGTH;
	CPhidgetIR_CodeInfo codeInfoReal;
	CPhidgetIR_CodeInfoHandle codeInfo = &codeInfoReal;
	jobject ircode, ircodeinfo, irlearn, togglemask;
	jshortArray codeData;
	jintArray headerArray, zeroArray, oneArray, repeatArray;
	jshort *datas;
	jint *datai;
	int i,j;

	if ((error = CPhidgetIR_getLastLearnedCode(h, data, &dataLength, codeInfo)))
	{
		PH_THROW(error);
		return NULL;
	}

	//create and fill in short array
	if(dataLength > 0){
	codeData = (*env)->NewShortArray(env, dataLength);
	if (!codeData)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}
	datas = (*env)->GetShortArrayElements(env, codeData, 0);
	if (!datas)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}
	for (i=0; i<dataLength; i++)
		datas[i] = (jshort)data[i];
	(*env)->ReleaseShortArrayElements(env, codeData, datas, 0);
	}
	else
		codeData=NULL;
	//create IRCode object
	if (!(ircode = (*env)->NewObject(env, irCode_class, irCode_cons, codeData, codeInfo->bitCount)))
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}

	//Header
	if(codeInfo->header[0])
	{
		headerArray = (*env)->NewIntArray(env, 2);
		if (!headerArray)
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return NULL;
		}
		datai = (*env)->GetIntArrayElements(env, headerArray, 0);
		if (!datai)
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return NULL;
		}
		for (i=0; i<2; i++)
			datai[i] = (jint)codeInfo->header[i];
		(*env)->ReleaseIntArrayElements(env, headerArray, datai, 0);
	}
	else
		headerArray=NULL;

	//Zero
	zeroArray = (*env)->NewIntArray(env, 2);
	if (!zeroArray)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}
	datai = (*env)->GetIntArrayElements(env, zeroArray, 0);
	if (!datai)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}
	for (i=0; i<2; i++)
		datai[i] = (jint)codeInfo->zero[i];
	(*env)->ReleaseIntArrayElements(env, zeroArray, datai, 0);

	//One
	oneArray = (*env)->NewIntArray(env, 2);
	if (!oneArray)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}
	datai = (*env)->GetIntArrayElements(env, oneArray, 0);
	if (!datai)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}
	for (i=0; i<2; i++)
		datai[i] = (jint)codeInfo->one[i];
	(*env)->ReleaseIntArrayElements(env, oneArray, datai, 0);

	//Repeat
	i=0;
	while(codeInfo->repeat[i])
		i++;
	if(i)
	{
		repeatArray = (*env)->NewIntArray(env, i);
		if (!repeatArray)
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return NULL;
		}
		datai = (*env)->GetIntArrayElements(env, repeatArray, 0);
		if (!datai)
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return NULL;
		}
		for (j=0; j<i; j++)
			datai[j] = (jint)codeInfo->repeat[j];
		(*env)->ReleaseIntArrayElements(env, repeatArray, datai, 0);
	}
	else{ 
		repeatArray=NULL;
	}

	//ToggleMask
	
	if(dataLength > 0){
		codeData = (*env)->NewShortArray(env, dataLength);
		if (!codeData)
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return NULL;
		}
		datas = (*env)->GetShortArrayElements(env, codeData, 0);
		if (!datas)
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return NULL;
		}
		for (i=0; i<dataLength; i++){

			datas[i] = (jshort)codeInfo->toggle_mask[i];
		}
		(*env)->ReleaseShortArrayElements(env, codeData, datas, 0);
		}
	else{ 
		codeData=NULL;
	}
	
	if (!(togglemask = (*env)->NewObject(env, irCode_class, irCode_cons, codeData, codeInfo->bitCount)))
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}

	//create IRCodeInfo object 
	if (!(ircodeinfo = (*env)->NewObject(env, irCodeInfo_class, irCodeInfo_cons, 
		codeInfo->encoding, codeInfo->bitCount, headerArray, zeroArray, oneArray,
		codeInfo->trail, codeInfo->gap, repeatArray, codeInfo->min_repeat, togglemask,   
		codeInfo->length, codeInfo->carrierFrequency, codeInfo->dutyCycle)))
	{
		(*env)->DeleteLocalRef(env, ircode);
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}

	//create IRLearn object
	if (!(irlearn = (*env)->NewObject(env, irLearn_class, irLearn_cons, ircode, ircodeinfo)))
	{
		(*env)->DeleteLocalRef(env, ircode);
		(*env)->DeleteLocalRef(env, ircodeinfo);
		PH_THROW(EPHIDGET_UNEXPECTED);
		return NULL;
	}

	return irlearn;
}

JNIEXPORT void JNICALL
Java_com_phidgets_IRPhidget_transmit(JNIEnv *env, jobject obj, jobject code, jobject irCodeInfo)
{
	CPhidgetIRHandle h = (CPhidgetIRHandle)(uintptr_t)(*env)->GetLongField( env, obj, handle_fid);
	jshortArray codeData;
	jintArray headerArray, zeroArray, oneArray, repeatArray;
	jobject togglemask;
	jshort *datas;
	jint *datai;
	unsigned char data[IR_MAX_CODE_DATA_LENGTH];
	jsize len;
	CPhidgetIR_CodeInfo codeInfoReal = {0};
	CPhidgetIR_CodeInfoHandle codeInfo = &codeInfoReal;
	int i;
	int error;

	//read in code
	if (!(codeData = (*env)->CallObjectMethod(env, code, irCode_getData)))
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return;
	}
	datas = (*env)->GetShortArrayElements(env, codeData, 0);
	if (!datas)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return;
	}
	len = (*env)->GetArrayLength(env, codeData);
	for(i=0;i<len;i++)
	{
		data[i] = (unsigned char)datas[i];
	}
	(*env)->ReleaseShortArrayElements(env, codeData, datas, 0);

	//read in codeInfo
	//ints
	codeInfo->encoding = (*env)->CallIntMethod(env, irCodeInfo, irCodeInfo_getEncoding);
	codeInfo->length = (*env)->CallIntMethod(env, irCodeInfo, irCodeInfo_getLength);
	codeInfo->bitCount = (*env)->CallIntMethod(env, irCodeInfo, irCodeInfo_getBitCount);
	codeInfo->gap = (*env)->CallIntMethod(env, irCodeInfo, irCodeInfo_getGap);
	codeInfo->trail = (*env)->CallIntMethod(env, irCodeInfo, irCodeInfo_getTrail);
	codeInfo->min_repeat = (*env)->CallIntMethod(env, irCodeInfo, irCodeInfo_getMinRepeat);
	codeInfo->carrierFrequency = (*env)->CallIntMethod(env, irCodeInfo, irCodeInfo_getCarrierFrequency);
	codeInfo->dutyCycle = (*env)->CallIntMethod(env, irCodeInfo, irCodeInfo_getDutyCycle);
	//arrays/objects
	//Header
	headerArray = (*env)->CallObjectMethod(env, irCodeInfo, irCodeInfo_getHeader);
	if(headerArray)
	{
		datai = (*env)->GetIntArrayElements(env, headerArray, 0);
		if (!datai)
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return;
		}
		for (i=0; i<2; i++)
			codeInfo->header[i] = datai[i];
		(*env)->ReleaseIntArrayElements(env, headerArray, datai, 0);
	}
	else
		headerArray=NULL;

	//Zero
	zeroArray = (*env)->CallObjectMethod(env, irCodeInfo, irCodeInfo_getZero);
	datai = (*env)->GetIntArrayElements(env, zeroArray, 0);
	if (!datai)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return;
	}
	for (i=0; i<2; i++)
		codeInfo->zero[i] = datai[i];
	(*env)->ReleaseIntArrayElements(env, zeroArray, datai, 0);

	//One
	oneArray = (*env)->CallObjectMethod(env, irCodeInfo, irCodeInfo_getOne);
	datai = (*env)->GetIntArrayElements(env, oneArray, 0);
	if (!datai)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return;
	}
	for (i=0; i<2; i++)
		codeInfo->one[i] = datai[i];
	(*env)->ReleaseIntArrayElements(env, oneArray, datai, 0);

	//Repeat
	repeatArray = (*env)->CallObjectMethod(env, irCodeInfo, irCodeInfo_getRepeat);
	if(repeatArray)
	{
		len = (*env)->GetArrayLength(env, repeatArray);
		if(len)
		{
			datai = (*env)->GetIntArrayElements(env, repeatArray, 0);
			if (!datai)
			{
				PH_THROW(EPHIDGET_UNEXPECTED);
				return;
			}
			for (i=0; i<len; i++)
				codeInfo->repeat[i] = datai[i];
			(*env)->ReleaseIntArrayElements(env, repeatArray, datai, 0);
		}
	}
	else
		repeatArray=NULL;

	//ToggleMask
	togglemask = (*env)->CallObjectMethod(env, irCodeInfo, irCodeInfo_getToggleMask);
	if(togglemask)
	{
		if (!(codeData = (*env)->CallObjectMethod(env, togglemask, irCode_getData)))
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return;
		}
		datas = (*env)->GetShortArrayElements(env, codeData, 0);
		if (!datas)
		{
			PH_THROW(EPHIDGET_UNEXPECTED);
			return;
		}
		len = (*env)->GetArrayLength(env, codeData);
		for(i=0;i<len;i++)
		{
			codeInfo->toggle_mask[i] = (unsigned char)datas[i];
		}
		(*env)->ReleaseShortArrayElements(env, codeData, datas, 0);
	}
	else
		togglemask=NULL;

	if ((error = CPhidgetIR_Transmit(h, data, codeInfo)))
		PH_THROW(error);
}

JNIEXPORT void JNICALL
Java_com_phidgets_IRPhidget_transmitRepeat(JNIEnv *env, jobject obj)
{
	CPhidgetIRHandle h = (CPhidgetIRHandle)(uintptr_t)(*env)->GetLongField( env, obj, handle_fid);
	int error;
	if ((error = CPhidgetIR_TransmitRepeat(h)))
		PH_THROW(error);
}

JNIEXPORT void JNICALL
Java_com_phidgets_IRPhidget_transmitRaw(JNIEnv *env, jobject obj, jintArray data, jint offset, jint count, jint gap, jint carrierFrequency, jint dutyCycle)
{
	CPhidgetIRHandle h = (CPhidgetIRHandle)(uintptr_t)(*env)->GetLongField( env, obj, handle_fid);
	int error;
	jint *datai;

	datai = (*env)->GetIntArrayElements(env, data, 0);
	if (!datai)
	{
		PH_THROW(EPHIDGET_UNEXPECTED);
		return;
	}

	if ((error = CPhidgetIR_TransmitRaw(h, datai+offset, count, carrierFrequency, dutyCycle, gap)))
		PH_THROW(error);

	(*env)->ReleaseIntArrayElements(env, data, datai, 0);
}
