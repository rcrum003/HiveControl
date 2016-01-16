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

#ifndef __PHLABVIEW
#define __PHLABVIEW
#include "cphidget.h"

#ifndef EXTERNALPROTO

#ifdef _WINDOWS
#include "windows/extcode.h"
#elif _LINUX
#include "extcode.h"
#elif _MACOSX
#include "macos/2010/extcode.h"
#endif

#else
typedef unsigned int LVUserEventRef;
#endif

#ifdef _WINDOWS

#define LABVIEW_STRUCT_PACK 1
//#define LABVIEW_EXPORT __declspec(dllexport)
#define LABVIEW_EXPORT

#pragma pack(push)
#pragma pack(show)

#else

#define LABVIEW_STRUCT_PACK 4
#define LABVIEW_EXPORT

#endif

#define LV_CFHANDLE_BODY(pname,param,structtype, ...) \
int CCONV CPhidget##pname##_On##param##_LaviewHandler(CPhidget##pname##Handle phid, __VA_ARGS__ ); \
LABVIEW_EXPORT int CCONV CPhidget##pname##_set_On##param##_LabviewHandler(CPhidget##pname##Handle phidA, LVUserEventRef *lvEventRef) \
{ \
	CPhidget##pname##Handle phid = (CPhidget##pname##Handle)phidA; \
	TESTPTR(phid); \
	if(!lvEventRef || !*lvEventRef) \
	{ \
		phid->fptr##param = NULL; \
		phid->fptr##param##ptr = NULL; \
	} \
	else \
	{ \
		phid->fptr##param = CPhidget##pname##_On##param##_LaviewHandler; \
		phid->fptr##param##ptr = lvEventRef; \
	} \
	return EPHIDGET_OK; \
} \
int CCONV CPhidget##pname##_On##param##_LaviewHandler(CPhidget##pname##Handle phid, __VA_ARGS__ ) \
{ \
	LVUserEventRef ev = *(LVUserEventRef *)userPtr; \
	MgErr ret = 0; \
	structtype *data; \
	data = (structtype *)DSNewPtr(sizeof(structtype)); \
	data->nothing = (int32)phid; \

#define LV_CFHANDLE_END \
    ret = PostLVUserEvent(ev, data); \
    DSDisposePtr(data); \
	return EPHIDGET_OK; \
}

#define LV_CFHANDLE_0(pname,param,structtype) \
	LV_CFHANDLE_BODY(pname,param,structtype,void *userPtr) \
    LV_CFHANDLE_END

#define LV_CFHANDLE_1(pname,param,structtype,datatype1) \
	LV_CFHANDLE_BODY(pname,param,structtype,void *userPtr,datatype1 val1) \
	data->val1 = val1; \
    LV_CFHANDLE_END

#define LV_CFHANDLE_2(pname,param,structtype,datatype1,datatype2) \
	LV_CFHANDLE_BODY(pname,param,structtype,void *userPtr,datatype1 val1,datatype2 val2) \
	data->val1 = val1; \
	data->val2 = val2; \
    LV_CFHANDLE_END

#define LV_CFHANDLE_3(pname,param,structtype,datatype1,datatype2,datatype3) \
	LV_CFHANDLE_BODY(pname,param,structtype,void *userPtr,datatype1 val1,datatype2 val2,datatype3 val3) \
	data->val1 = val1; \
	data->val2 = val2; \
	data->val3 = val3; \
    LV_CFHANDLE_END

#define LV_CFHANDLE_4(pname,param,structtype,datatype1,datatype2,datatype3,datatype4) \
	LV_CFHANDLE_BODY(pname,param,structtype,void *userPtr,datatype1 val1,datatype2 val2,datatype3 val3,datatype4 val4) \
	data->val1 = val1; \
	data->val2 = val2; \
	data->val3 = val3; \
	data->val4 = val4; \
    LV_CFHANDLE_END

#define LV_CHDREVENT(pname,param) LABVIEW_EXPORT int CCONV CPhidget##pname##_set_On##param##_LabviewHandler(CPhidget##pname##Handle phid, LVUserEventRef *lvEventRef);

/** \defgroup phidlabview Phidget Labview Functions 
 * Calls specific to the Phidget Labview Support.
 * @{
 */

/**
 * Labview 1D int32 Array
 */
typedef struct {
    int32    length;	// Length of array
    int32    data[1];	// Array
} lvArrInt32, *lvArrInt32H, **lvArrInt32DH;

/**
 * Labview 1D uInt8 Array
 */
typedef struct {
    int32    length;	// Length of array
    uInt8    data[1];	// Array
} lvArrUInt8, *lvArrUInt8H, **lvArrUInt8DH;


/* pack all of the following structures on Windows
 * Note: Labview 64-bit seems to want double to be 8-byte aligned,
 * but 32-bit wants them packed to presumably 4-byte
 * ints, etc. need to be packed in close.
 * mac/linux don't need anything special.
 * http://zone.ni.com/reference/en-XX/help/371361G-01/lvconcepts/how_labview_stores_data_in_memory/
 */
#ifdef _WINDOWS
#pragma pack(LABVIEW_STRUCT_PACK)
#pragma pack(show)
#endif

/**
 * Used for Labview events where only the event sender is returned
 */
typedef struct _lvNothing {
	int32 nothing;			/**< Not Used */
} lvNothing;

/**
 * Used for Labview events that return an error code/string
 */
typedef struct _lvError {
	int32 nothing;			/**< Not Used */
	int32 val1;				/**< Error Code */
	LStrHandle val2;		/**< Error String */
} lvError;

/**
 * Used for Labview events that return a double
 */
typedef struct _lvDouble {
//Labview 64-bit windows wants doubles on 8-byte boundaries
#ifdef _WINDOWS
	size_t nothing;			/**< Not Used */
#else
	int32 nothing;			/**< Not Used */
#endif
	double val1;			/**< Value */
} lvDouble;

/**
 * Used for Labview events that return 3 doubles
 */
typedef struct _lvDoubleDoubleDouble {
//Labview 64-bit windows wants doubles on 8-byte boundaries
#ifdef _WINDOWS
	size_t nothing;			/**< Not Used */
#else
	int32 nothing;			/**< Not Used */
#endif
	double val1;			/**< Value 1 */
	double val2;			/**< Value 2 */
	double val3;			/**< Value 3 */
} lvDoubleDoubleDouble;

/**
 * Used for Labview events that return an int32
 */
typedef struct _lvInt32 {
	int32 nothing;			/**< Not Used */
	int32 val1;				/**< Value */
} lvInt32;

/**
 * Used for Labview events that return an indexed double
 */
typedef struct _lvIndexedDouble {
	int32 nothing;			/**< Not Used */
	int32 val1;				/**< Index */
	double val2;			/**< Value */
} lvIndexedDouble;

/**
 * Used for Labview events that return an indexed int32
 */
typedef struct _lvIndexedInt32 {
	int32 nothing;			/**< Not Used */
	int32 val1;				/**< Index */
	int32 val2;				/**< Value */
} lvIndexedInt32;

/**
 * Used for Labview events that return two indexed int32s
 */
typedef struct _lvIndexedInt32Int32 {
	int32 nothing;			/**< Not Used */
	int32 val1;				/**< Index */
	int32 val2;				/**< Value 1 */
	int32 val3;				/**< Value 2 */
} lvIndexedInt32Int32;

/**
 * Used for Labview events that return an indexed int64
 */
typedef struct _lvIndexedInt64 {
	int32 nothing;			/**< Not Used */
	int32 val1;				/**< Index */
	int64 val2;				/**< Value */
} lvIndexedInt64;

/**
 * Used for Labview events that return an int32 array
 */
typedef struct _lvInt32Array {
	lvArrInt32DH val1;		/**< Data */
	int32 nothing;			/**< Not Used */
} lvInt32Array;

/**
 * Used for Labview events that return an uInt8 array
 */
typedef struct _lvUInt8Array {
	lvArrUInt8DH val1;		/**< Data */
	int32 nothing;			/**< Not Used */
} lvUInt8Array;

/**
 * Used for Labview events that return an IR Code
 */
typedef struct _lvIRCode {
	lvArrUInt8DH val1;		/**< Code data */
	int32 val2;				/**< bit count */
	int32 val3;				/**< repeat */
	int32 nothing;			/**< Not Used */
} lvIRCode;

/**
 * Used for Labview events that return IR Learn data
 */
typedef struct _lvIRLearn {
	lvArrUInt8DH val1;			/**< Code data */
	CPhidgetIR_CodeInfo val2;	/**< code parameters */
	int32 nothing;			/**< Not Used */
} lvIRLearn;

/**
 * Used for Labview events that return an rfid tag (v2)
 */
typedef struct _lvRFIDTag2 {
	LStrHandle val1;			/**< Tag String */
	CPhidgetRFID_Protocol val2;	/**< tag protocol */
	int32 nothing;			/**< Not Used */
} lvRFIDTag2;

/**
 * Used for Labview events that return spatial data
 */
typedef struct _lvSpatialData {
//Labview 64-bit windows wants doubles on 8-byte boundaries
#ifdef _WINDOWS
	size_t nothing;			/**< Not Used */
#else
	int32 nothing;			/**< Not Used */
#endif
	CPhidgetSpatial_SpatialEventData val1;			/**< Spatial Data */
} lvSpatialData;


/**
 * Used for Labview manager attach and detach events
 */
typedef struct _lvManager {
	__int64 val1;			/**< Handle to a phidget */
} lvManager;

/**
 * Used for Labview dictionary key change events
 */
typedef struct _lvDictionary {
	LStrHandle val1;		/**< key String */
	LStrHandle val2;		/**< value String */
	CPhidgetDictionary_keyChangeReason	val3;	/**< key change reason */
} lvDictionary;


#ifdef _WINDOWS
//Restore default packing
#pragma pack(pop)
#pragma pack(show)
#endif

/**
 * Sets up a Labview event callback for the Attach event.
 * Event callback returns an \ref _lvNothing.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(,Attach)
/**
 * Sets up a Labview event callback for the Detach event.
 * Event callback returns an \ref _lvNothing.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(,Detach)
/**
 * Sets up a Labview event callback for the ServerConnect event.
 * Event callback returns an \ref _lvNothing.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(,ServerConnect)
/**
 * Sets up a Labview event callback for the ServerDisconnect event.
 * Event callback returns an \ref _lvNothing.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(,ServerDisconnect)
/**
 * Sets up a Labview event callback for the Error event.
 * Event callback returns an \ref _lvError.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(,Error)

/**
 * Sets up a Labview event callback for the AccelerationChange event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(Accelerometer,AccelerationChange)

/**
 * Sets up a Labview event callback for the PositionChange event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(AdvancedServo, PositionChange)
/**
 * Sets up a Labview event callback for the VelocityChange event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(AdvancedServo, VelocityChange)
/**
 * Sets up a Labview event callback for the CurrentChange event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(AdvancedServo, CurrentChange)

/**
 * Sets up a Labview event callback for the BridgeData event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(Bridge, BridgeData)

/**
 * Sets up a Labview event callback for the InputChange event.
 * Event callback returns an \ref _lvIndexedInt32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(Encoder, InputChange)
/**
 * Sets up a Labview event callback for the PositionChange event.
 * Event callback returns an \ref _lvIndexedInt32Int32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(Encoder, PositionChange)

/**
 * Sets up a Labview event callback for the Count event.
 * Event callback returns an \ref _lvIndexedInt32Int32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(FrequencyCounter, Count)

/**
 * Sets up a Labview event callback for the PositionChange event.
 * Event callback returns an \ref _lvDoubleDoubleDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(GPS, PositionChange)
/**
 * Sets up a Labview event callback for the PositionFixStatusChange event.
 * Event callback returns an \ref _lvInt32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(GPS, PositionFixStatusChange)

/**
 * Sets up a Labview event callback for the InputChange event.
 * Event callback returns an \ref _lvIndexedInt32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(InterfaceKit, InputChange)
/**
 * Sets up a Labview event callback for the OutputChange event.
 * Event callback returns an \ref _lvIndexedInt32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(InterfaceKit, OutputChange)
/**
 * Sets up a Labview event callback for the SensorChange event.
 * Event callback returns an \ref _lvIndexedInt32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(InterfaceKit, SensorChange)

/**
 * Sets up a Labview event callback for the Code event.
 * Event callback returns an \ref _lvIRCode.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(IR, Code)
/**
 * Sets up a Labview event callback for the RawData event.
 * Event callback returns an \ref _lvIRRawData.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(IR, RawData)
/**
 * Sets up a Labview event callback for the Learn event.
 * Event callback returns an \ref _lvIRLearn.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(IR, Learn)

/**
 * Sets up a Labview event callback for the InputChange event.
 * Event callback returns an \ref _lvIndexedInt32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(MotorControl, InputChange)
/**
 * Sets up a Labview event callback for the VelocityChange event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(MotorControl, VelocityChange)
/**
 * Sets up a Labview event callback for the CurrentChange event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(MotorControl, CurrentChange)
/**
 * Sets up a Labview event callback for the EncoderPositionChange event.
 * Event callback returns an \ref _lvIndexedInt32Int32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(MotorControl, EncoderPositionChange)
/**
 * Sets up a Labview event callback for the EncoderPositionUpdate event.
 * Event callback returns an \ref _lvIndexedInt32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(MotorControl, EncoderPositionUpdate)
/**
 * Sets up a Labview event callback for the BackEMFUpdate event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(MotorControl, BackEMFUpdate)
/**
 * Sets up a Labview event callback for the SensorUpdate event.
 * Event callback returns an \ref _lvIndexedInt32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(MotorControl, SensorUpdate)
/**
 * Sets up a Labview event callback for the CurrentUpdate event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(MotorControl, CurrentUpdate)

/**
 * Sets up a Labview event callback for the PHChange event.
 * Event callback returns an \ref _lvDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(PHSensor, PHChange)

/**
 * Sets up a Labview event callback for the OutputChange event.
 * Event callback returns an \ref _lvIndexedInt32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(RFID, OutputChange)
/**
 * Sets up a Labview event callback for the Tag event.
 * Event callback returns an \ref _lvRFIDTag.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(RFID, Tag)
/**
 * Sets up a Labview event callback for the TagLost event.
 * Event callback returns an \ref _lvRFIDTag.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(RFID, TagLost)
/**
 * Sets up a Labview event callback for the RawData event.
 * Event callback returns an \ref _lvRFIDRawData.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(RFID, RawData)
/**
 * Sets up a Labview event callback for the Tag2 event.
 * Event callback returns an \ref _lvRFIDTag2.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(RFID, Tag2)
/**
 * Sets up a Labview event callback for the TagLost2 event.
 * Event callback returns an \ref _lvRFIDTag2.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(RFID, TagLost2)

/**
 * Sets up a Labview event callback for the PositionChange event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(Servo, PositionChange)

/**
 * Sets up a Labview event callback for the SpatialData event.
 * Event callback returns an \ref _lvSpatialData.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(Spatial, SpatialData)

/**
 * Sets up a Labview event callback for the InputChange event.
 * Event callback returns an \ref _lvIndexedInt32.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(Stepper, InputChange)
/**
 * Sets up a Labview event callback for the PositionChange event.
 * Event callback returns an \ref _lvIndexedInt64.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(Stepper, PositionChange)
LV_CHDREVENT(Stepper, PositionChange32)
/**
 * Sets up a Labview event callback for the VelocityChange event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(Stepper, VelocityChange)
/**
 * Sets up a Labview event callback for the CurrentChange event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(Stepper, CurrentChange)

/**
 * Sets up a Labview event callback for the TemperatureChange event.
 * Event callback returns an \ref _lvIndexedDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(TemperatureSensor, TemperatureChange)

/**
 * Sets up a Labview event callback for the WeightChange event.
 * Event callback returns an \ref _lvDouble.
 * @param phid An attached phidget handle.
 * @param lvEventRef Lavbiew user event ref
 */
LV_CHDREVENT(WeightSensor, WeightChange)

LV_CHDREVENT(Manager,Attach)
LV_CHDREVENT(Manager,Detach)
LV_CHDREVENT(Manager,ServerConnect)
LV_CHDREVENT(Manager,ServerDisconnect)
LV_CHDREVENT(Manager,Error)

LABVIEW_EXPORT int CCONV CPhidgetDictionary_set_OnKeyChange_LabviewHandler(CPhidgetDictionaryHandle dict, CPhidgetDictionaryListenerHandle *listen, const char *pattern, LVUserEventRef *lvEventRef);
LV_CHDREVENT(Dictionary,ServerConnect)
LV_CHDREVENT(Dictionary,ServerDisconnect)
LV_CHDREVENT(Dictionary,Error)

/** @} */

#endif
