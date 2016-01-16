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

#include "stdafx.h"

#include "cphidget.h"
#include "cphidgetmanager.h"
#include "cphidgetdictionary.h"
#include "cphidgetaccelerometer.h"
#include "cphidgetadvancedservo.h"
#include "cphidgetanalog.h"
#include "cphidgetbridge.h"
#include "cphidgetencoder.h"
#include "cphidgetfrequencycounter.h"
#include "cphidgetgps.h"
#include "cphidgetinterfacekit.h"
#include "cphidgetir.h"
#include "cphidgetmanager.h"
#include "cphidgetled.h"
#include "cphidgetmotorcontrol.h"
#include "cphidgetphsensor.h"
#include "cphidgetrfid.h"
#include "cphidgetservo.h"
#include "cphidgetspatial.h"
#include "cphidgetstepper.h"
#include "cphidgettemperaturesensor.h"
#include "cphidgettextlcd.h"
#include "cphidgettextled.h"
#include "cphidgetweightsensor.h"

#include "csocket.h"

#if COMPILE_PHIDGETS_LABVIEW

#include "phidget_labview.h"

LV_CFHANDLE_0(, Attach, lvNothing)
LV_CFHANDLE_0(, Detach, lvNothing)
LV_CFHANDLE_0(, ServerConnect, lvNothing)
LV_CFHANDLE_0(, ServerDisconnect, lvNothing)
LV_CFHANDLE_BODY(, Error, lvError, void *userPtr, int val1, const char *val2)
	data->val1 = val1;
    data->val2=(LStrHandle)DSNewHandle(sizeof(int32)+255*sizeof(char));
	memset(LStrBuf(*data->val2),'\0',255);
	snprintf((char*)LStrBuf(*data->val2),255,"%s",val2);
	LStrLen(*data->val2)=strlen(val2);

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val2);
    DSDisposePtr(data);
	return EPHIDGET_OK;
}

LV_CFHANDLE_2(Accelerometer, AccelerationChange, lvIndexedDouble, int, double)

LV_CFHANDLE_2(AdvancedServo, PositionChange, lvIndexedDouble, int, double)
LV_CFHANDLE_2(AdvancedServo, VelocityChange, lvIndexedDouble, int, double)
LV_CFHANDLE_2(AdvancedServo, CurrentChange, lvIndexedDouble, int, double)

LV_CFHANDLE_2(Bridge, BridgeData, lvIndexedDouble, int, double)

LV_CFHANDLE_2(Encoder, InputChange, lvIndexedInt32, int, int)
LV_CFHANDLE_3(Encoder, PositionChange, lvIndexedInt32Int32, int, int, int)

LV_CFHANDLE_3(FrequencyCounter, Count, lvIndexedInt32Int32, int, int, int)

LV_CFHANDLE_3(GPS, PositionChange, lvDoubleDoubleDouble, double, double, double)
LV_CFHANDLE_1(GPS, PositionFixStatusChange, lvInt32, int)

LV_CFHANDLE_2(InterfaceKit, InputChange, lvIndexedInt32, int, int)
LV_CFHANDLE_2(InterfaceKit, OutputChange, lvIndexedInt32, int, int)
LV_CFHANDLE_2(InterfaceKit, SensorChange, lvIndexedInt32, int, int)

LV_CFHANDLE_BODY(IR, Code, lvIRCode, void *userPtr, unsigned char *val1, int val2, int val3, int val4)
    data->val1=(lvArrUInt8DH)DSNewHandle(sizeof(int32)+val2*sizeof(uInt8));
    (*(data->val1))->length = val2;
	memcpy((*(data->val1))->data, val1, val2 * sizeof(uInt8));
	data->val2 = val3;
	data->val3 = val4;

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val1);
    DSDisposePtr(data);
	return EPHIDGET_OK;
}
LV_CFHANDLE_BODY(IR, RawData, lvInt32Array, void *userPtr, int *val1, int val2)
    data->val1=(lvArrInt32DH)DSNewHandle(sizeof(int32)+val2*sizeof(int32));
    (*(data->val1))->length = val2;
	memcpy((*(data->val1))->data, val1, val2 * sizeof(int32));

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val1);
    DSDisposePtr(data);
	return EPHIDGET_OK;
}
LV_CFHANDLE_BODY(IR, Learn, lvIRLearn, void *userPtr, unsigned char *val1, int val2, CPhidgetIR_CodeInfoHandle val3)
    data->val1=(lvArrUInt8DH)DSNewHandle(sizeof(int32)+val2*sizeof(uInt8));
    (*(data->val1))->length = val2;
	memcpy((*(data->val1))->data, val1, val2 * sizeof(uInt8));
	data->val2 = *val3;

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val1);
    DSDisposePtr(data);
	return EPHIDGET_OK;
}

LV_CFHANDLE_2(MotorControl, InputChange, lvIndexedInt32, int, int)
LV_CFHANDLE_2(MotorControl, VelocityChange, lvIndexedDouble, int, double)
LV_CFHANDLE_2(MotorControl, CurrentChange, lvIndexedDouble, int, double)
LV_CFHANDLE_3(MotorControl, EncoderPositionChange, lvIndexedInt32Int32, int, int, int)
LV_CFHANDLE_2(MotorControl, EncoderPositionUpdate, lvIndexedInt32, int, int)
LV_CFHANDLE_2(MotorControl, BackEMFUpdate, lvIndexedDouble, int, double)
LV_CFHANDLE_2(MotorControl, SensorUpdate, lvIndexedInt32, int, int)
LV_CFHANDLE_2(MotorControl, CurrentUpdate, lvIndexedDouble, int, double)

LV_CFHANDLE_1(PHSensor, PHChange, lvDouble, double)

LV_CFHANDLE_2(RFID, OutputChange, lvIndexedInt32, int, int)
LV_CFHANDLE_BODY(RFID, Tag, lvUInt8Array, void *userPtr, unsigned char *val1)
    data->val1=(lvArrUInt8DH)DSNewHandle(sizeof(int32)+5*sizeof(uInt8));
    (*(data->val1))->length = 5;
	memcpy((*(data->val1))->data, val1, 5 * sizeof(uInt8));

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val1);
    DSDisposePtr(data);
	return EPHIDGET_OK;
}
LV_CFHANDLE_BODY(RFID, TagLost, lvUInt8Array, void *userPtr, unsigned char *val1)
    data->val1=(lvArrUInt8DH)DSNewHandle(sizeof(int32)+5*sizeof(uInt8));
    (*(data->val1))->length = 5;
	memcpy((*(data->val1))->data, val1, 5 * sizeof(uInt8));

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val1);
    DSDisposePtr(data);
	return EPHIDGET_OK;
}
LV_CFHANDLE_BODY(RFID, Tag2, lvRFIDTag2, void *userPtr, char *val1, CPhidgetRFID_Protocol val2)
    data->val1=(LStrHandle)DSNewHandle(sizeof(int32)+255*sizeof(char));
	memset(LStrBuf(*data->val1),'\0',255);
	snprintf((char*)LStrBuf(*data->val1),255,"%s",val1);
	LStrLen(*data->val1)=strlen(val1);
	data->val2 = val2;

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val1);
    DSDisposePtr(data);
	return EPHIDGET_OK;
}
LV_CFHANDLE_BODY(RFID, TagLost2, lvRFIDTag2, void *userPtr, char *val1, CPhidgetRFID_Protocol val2)
    data->val1=(LStrHandle)DSNewHandle(sizeof(int32)+255*sizeof(char));
	memset(LStrBuf(*data->val1),'\0',255);
	snprintf((char*)LStrBuf(*data->val1),255,"%s",val1);
	LStrLen(*data->val1)=strlen(val1);
	data->val2 = val2;

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val1);
    DSDisposePtr(data);
	return EPHIDGET_OK;
}

LV_CFHANDLE_2(Servo, PositionChange, lvIndexedDouble, int, double)

LV_CFHANDLE_BODY(Spatial,SpatialData,lvSpatialData,void *userPtr,CPhidgetSpatial_SpatialEventDataHandle *val1,int val2)
	DSDisposePtr(data);
	{
		int i;
		for(i=0;i<val2;i++)
		{
			data = (lvSpatialData *)DSNewPtr(sizeof(lvSpatialData));
			data->nothing = (int32)phid;
#if 0
			data->acceleration0 = val1[i]->acceleration[0];
			data->acceleration1 = val1[i]->acceleration[1];
			data->acceleration2 = val1[i]->acceleration[2];
			data->angularRate0 = val1[i]->angularRate[0];
			data->angularRate1 = val1[i]->angularRate[1];
			data->angularRate2 = val1[i]->angularRate[2];
			data->magneticField0 = val1[i]->magneticField[0];
			data->magneticField1 = val1[i]->magneticField[1];
			data->magneticField2 = val1[i]->magneticField[2];
			data->microseconds = val1[i]->timestamp.microseconds;
			data->seconds = val1[i]->timestamp.seconds;
#else
			data->val1 = *val1[i];
#endif
			ret = PostLVUserEvent(ev, data);
			DSDisposePtr(data);
		}
	}
	return EPHIDGET_OK;
}

LV_CFHANDLE_2(Stepper, InputChange, lvIndexedInt32, int, int)
LV_CFHANDLE_2(Stepper, PositionChange, lvIndexedInt64, int, __int64)
LV_CFHANDLE_2(Stepper, PositionChange32, lvIndexedInt32, int, int)
LV_CFHANDLE_2(Stepper, VelocityChange, lvIndexedDouble, int, double)
LV_CFHANDLE_2(Stepper, CurrentChange, lvIndexedDouble, int, double)

LV_CFHANDLE_2(TemperatureSensor, TemperatureChange, lvIndexedDouble, int, double)

LV_CFHANDLE_1(WeightSensor, WeightChange, lvDouble, double)

//Manager

int CCONV CPhidgetManager_OnAttach_LaviewHandler(CPhidgetHandle phid, void *userPtr);
LABVIEW_EXPORT int CCONV CPhidgetManager_set_OnAttach_LabviewHandler(CPhidgetManagerHandle phidA, LVUserEventRef *lvEventRef)
{
	CPhidgetManagerHandle phid = (CPhidgetManagerHandle)phidA;
	TESTPTR(phid);
	if(!lvEventRef || !*lvEventRef)
	{
		phid->fptrAttachChange = NULL;
		phid->fptrAttachChange = NULL;
	}
	else
	{
		phid->fptrAttachChangeptr = CPhidgetManager_OnAttach_LaviewHandler;
		phid->fptrAttachChangeptr = lvEventRef;
	}
	return EPHIDGET_OK;
}
int CCONV CPhidgetManager_OnAttach_LaviewHandler(CPhidgetHandle phid, void *userPtr)
{
	LVUserEventRef ev = *(LVUserEventRef *)userPtr;
	MgErr ret = 0;
	lvManager *data;
	data = (lvManager *)DSNewPtr(sizeof(lvManager));
	data->val1 = (size_t)phid;
	LV_CFHANDLE_END

int CCONV CPhidgetManager_OnDetach_LaviewHandler(CPhidgetHandle phid, void *userPtr);
LABVIEW_EXPORT int CCONV CPhidgetManager_set_OnDetach_LabviewHandler(CPhidgetManagerHandle phidA, LVUserEventRef *lvEventRef)
{
	CPhidgetManagerHandle phid = (CPhidgetManagerHandle)phidA;
	TESTPTR(phid);
	if(!lvEventRef || !*lvEventRef)
	{
		phid->fptrDetachChange = NULL;
		phid->fptrDetachChange = NULL;
	}
	else
	{
		phid->fptrDetachChangeptr = CPhidgetManager_OnDetach_LaviewHandler;
		phid->fptrDetachChangeptr = lvEventRef;
	}
	return EPHIDGET_OK;
}
int CCONV CPhidgetManager_OnDetach_LaviewHandler(CPhidgetHandle phid, void *userPtr)
{
	LVUserEventRef ev = *(LVUserEventRef *)userPtr;
	MgErr ret = 0;
	lvManager *data;
	data = (lvManager *)DSNewPtr(sizeof(lvManager));
	data->val1 = (size_t)phid;
	LV_CFHANDLE_END

LV_CFHANDLE_0(Manager, ServerConnect, lvNothing)
LV_CFHANDLE_0(Manager, ServerDisconnect, lvNothing)
LV_CFHANDLE_BODY(Manager, Error, lvError, void *userPtr, int val1, const char *val2)
	data->val1 = val1;
    data->val2=(LStrHandle)DSNewHandle(sizeof(int32)+255*sizeof(char));
	memset(LStrBuf(*data->val2),'\0',255);
	snprintf((char*)LStrBuf(*data->val2),255,"%s",val2);
	LStrLen(*data->val2)=strlen(val2);

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val2);
    DSDisposePtr(data);
	return EPHIDGET_OK;
}

//Dictionary

int CCONV CPhidgetDictionary_OnKeyChange_LaviewHandler(CPhidgetDictionaryHandle dict, void *userPtr, const char *key, const char *value, CPhidgetDictionary_keyChangeReason reason);
LABVIEW_EXPORT int CCONV CPhidgetDictionary_set_OnKeyChange_LabviewHandler(CPhidgetDictionaryHandle dict, CPhidgetDictionaryListenerHandle *listen, const char *pattern, LVUserEventRef *lvEventRef)
{
	TESTPTR(dict);
	if(lvEventRef && *lvEventRef)
		CPhidgetDictionary_set_OnKeyChange_Handler(dict, listen, pattern, CPhidgetDictionary_OnKeyChange_LaviewHandler, lvEventRef);
	else
		return EPHIDGET_INVALIDARG;
	return EPHIDGET_OK;
}
int CCONV CPhidgetDictionary_OnKeyChange_LaviewHandler(CPhidgetDictionaryHandle dict, void *userPtr, const char *key, const char *value, CPhidgetDictionary_keyChangeReason reason)
{
	LVUserEventRef ev = *(LVUserEventRef *)userPtr;
	MgErr ret = 0;
	lvDictionary *data;

	data = (lvDictionary *)DSNewPtr(sizeof(lvDictionary));

	//key
    data->val1=(LStrHandle)DSNewHandle(sizeof(int32)+(strlen(key)+1)*sizeof(char));
	memset(LStrBuf(*data->val1),'\0',(strlen(key)+1)*sizeof(char));
	snprintf((char*)LStrBuf(*data->val1),(strlen(key)+1)*sizeof(char),"%s",key);
	LStrLen(*data->val1)=strlen(key);
	
	//value
    data->val2=(LStrHandle)DSNewHandle(sizeof(int32)+(strlen(value)+1)*sizeof(char));
	memset(LStrBuf(*data->val2),'\0',(strlen(value)+1)*sizeof(char));
	snprintf((char*)LStrBuf(*data->val2),(strlen(value)+1)*sizeof(char),"%s",value);
	LStrLen(*data->val2)=strlen(value);

	data->val3 = reason;

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val1);
	DSDisposeHandle(data->val2);
    DSDisposePtr(data);

	return EPHIDGET_OK;
}
LV_CFHANDLE_0(Dictionary, ServerConnect, lvNothing)
LV_CFHANDLE_0(Dictionary, ServerDisconnect, lvNothing)
LV_CFHANDLE_BODY(Dictionary, Error, lvError, void *userPtr, int val1, const char *val2)
	data->val1 = val1;
    data->val2=(LStrHandle)DSNewHandle(sizeof(int32)+255*sizeof(char));
	memset(LStrBuf(*data->val2),'\0',255);
	snprintf((char*)LStrBuf(*data->val2),255,"%s",val2);
	LStrLen(*data->val2)=strlen(val2);

    ret = PostLVUserEvent(ev, data);

	DSDisposeHandle(data->val2);
    DSDisposePtr(data);
	return EPHIDGET_OK;
}

#else
//On WindowsCE, we need stubs because they are in the exports.def file

#define LV_NULL_FUNC(pname,param) int CCONV CPhidget##pname##_set_On##param##_LabviewHandler(CPhidget##pname##Handle phidA, void *lvEventRef) { return EPHIDGET_UNSUPPORTED; }

LV_NULL_FUNC(, Attach)
LV_NULL_FUNC(, Detach)
LV_NULL_FUNC(, ServerConnect)
LV_NULL_FUNC(, ServerDisconnect)
LV_NULL_FUNC(, Error)

LV_NULL_FUNC(Accelerometer, AccelerationChange)

LV_NULL_FUNC(AdvancedServo, PositionChange)
LV_NULL_FUNC(AdvancedServo, VelocityChange)
LV_NULL_FUNC(AdvancedServo, CurrentChange)

LV_NULL_FUNC(Bridge, BridgeData)

LV_NULL_FUNC(Encoder, InputChange)
LV_NULL_FUNC(Encoder, PositionChange)

LV_NULL_FUNC(FrequencyCounter, Count)

LV_NULL_FUNC(GPS, PositionChange)
LV_NULL_FUNC(GPS, PositionFixStatusChange)

LV_NULL_FUNC(InterfaceKit, InputChange)
LV_NULL_FUNC(InterfaceKit, OutputChange)
LV_NULL_FUNC(InterfaceKit, SensorChange)

LV_NULL_FUNC(IR, Code)
LV_NULL_FUNC(IR, RawData)
LV_NULL_FUNC(IR, Learn)

LV_NULL_FUNC(MotorControl, InputChange)
LV_NULL_FUNC(MotorControl, VelocityChange)
LV_NULL_FUNC(MotorControl, CurrentChange)
LV_NULL_FUNC(MotorControl, EncoderPositionChange)
LV_NULL_FUNC(MotorControl, EncoderPositionUpdate)
LV_NULL_FUNC(MotorControl, BackEMFUpdate)
LV_NULL_FUNC(MotorControl, SensorUpdate)
LV_NULL_FUNC(MotorControl, CurrentUpdate)

LV_NULL_FUNC(PHSensor, PHChange)

LV_NULL_FUNC(RFID, OutputChange)
LV_NULL_FUNC(RFID, Tag)
LV_NULL_FUNC(RFID, TagLost)
LV_NULL_FUNC(RFID, Tag2)
LV_NULL_FUNC(RFID, TagLost2)

LV_NULL_FUNC(Servo, PositionChange)

LV_NULL_FUNC(Spatial,SpatialData)

LV_NULL_FUNC(Stepper, InputChange)
LV_NULL_FUNC(Stepper, PositionChange)
LV_NULL_FUNC(Stepper, PositionChange32)
LV_NULL_FUNC(Stepper, VelocityChange)
LV_NULL_FUNC(Stepper, CurrentChange)

LV_NULL_FUNC(TemperatureSensor, TemperatureChange)

LV_NULL_FUNC(WeightSensor, WeightChange)

LV_NULL_FUNC(Manager, Attach)
LV_NULL_FUNC(Manager, Detach)
LV_NULL_FUNC(Manager, ServerConnect)
LV_NULL_FUNC(Manager, ServerDisconnect)
LV_NULL_FUNC(Manager, Error)

LV_NULL_FUNC(Dictionary, KeyChange)
LV_NULL_FUNC(Dictionary, ServerConnect)
LV_NULL_FUNC(Dictionary, ServerDisconnect)
LV_NULL_FUNC(Dictionary, Error)

#endif
