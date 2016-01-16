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
#include "cphidgetconstantsinternal.h"
#include "cphidgetconstants.h"

const char *Phid_ErrorDescriptions[PHIDGET_ERROR_CODE_COUNT] = {
"Function completed successfully.",
"A Phidget matching the type and or serial number could not be found.", 
"Memory could not be allocated.",
"Unexpected Error.  Contact Phidgets Inc. for support.", 
"Invalid argument passed to function.",
"Phidget not physically attached.", 
"Read/Write operation was interrupted.", 
"The Error Code is not defined.",
"Network Error.",
"Value is Unknown (State not yet received from device, or not yet set by user).",
"Authorization Failed.",
"Not Supported.",
"Duplicated request.",
"Given timeout has been exceeded.",
"Index out of Bounds.",
"A non-null error code was returned from an event handler.",
"A connection to the server does not exist.",
"Function is not applicable for this device.",
"Phidget handle was closed.",
"Webservice and Client protocol versions don't match. Update both to newest release."};

const char Phid_UnknownErrorDescription[] = "Unknown Error Code.";

/* enum starts a 1 so we need a blank for 0 
 * Don't  reorder this list */
const char *Phid_DeviceName[PHIDGET_DEVICE_CLASS_COUNT] = {
"ERROR",
"Uninitialized Phidget Handle",
"PhidgetAccelerometer",
"PhidgetAdvancedServo",
"PhidgetEncoder",
"PhidgetGPS",
"ERROR", //Old Gyro
"PhidgetInterfaceKit", 
"PhidgetLED",
"PhidgetMotorControl",
"PhidgetPHSensor",
"PhidgetRFID", 
"PhidgetServo",
"PhidgetStepper",
"PhidgetTemperatureSensor",
"PhidgetTextLCD",
"PhidgetTextLED",
"PhidgetWeightSensor",
"PhidgetGeneric",
"PhidgetIR",
"PhidgetSpatial",
"PhidgetFrequencyCounter",
"PhidgetAnalog",
"PhidgetBridge"};

#ifdef DEBUG
const char LibraryVersion[] = "Phidget21 Debug - Version 2.1.8 - Built " __DATE__
    " " __TIME__;
#else
const char LibraryVersion[] = "Phidget21 - Version 2.1.8 - Built " __DATE__
    " " __TIME__;
#endif

#ifndef _MSC_EXTENSIONS
#define UFINTS(name, initializer...) .name = { initializer }
#else
#define UFINTS(name, ...) { __VA_ARGS__ }
#endif

#define UNUSED_DEVICE(pid) PHIDCLASS_NOTHING, 0x6C2, pid, 0, { UFINTS(dummy, 0, 0, 0) }, ""

// We own Product IDs 0x30 - 0xAF (48-175)
// This needs to start with the device id = 1 invalid device and end in NULL
// This list could be re-ordered, but we leave it in product-id order.
const CPhidgetDeviceDef Phid_Device_Def[PHIDGET_DEVICE_COUNT+1] = {
{ PHIDID_NOTHING,						PHIDCLASS_NOTHING,			0x000, 0x00,	0, { UFINTS(dummy,				 0,  0,  0 ) },	"Uninitialized Phidget Handle"},		//1 start for list logic

/* Very old devices - we don't own these product IDs so don't allocate anymore!! We maintain support for these devices */
{ PHIDID_SERVO_1MOTOR_OLD,				PHIDCLASS_SERVO,			0x925, 0x8101,	0, { UFINTS(servo,				 1         ) },	"Phidget Servo Controller 1-motor"},	//Original 1000
{ PHIDID_SERVO_4MOTOR_OLD,				PHIDCLASS_SERVO,			0x925, 0x8104,	0, { UFINTS(servo,				 4         ) },	"Phidget Servo Controller 4-motor"},	//Original 1001
{ PHIDID_INTERFACEKIT_2_8_8,			PHIDCLASS_INTERFACEKIT,		0x925, 0x8200,	0, { UFINTS(ifkit,				 2,  8,  8 ) },	"Phidget InterfaceKit 2/8/8"},			//ORIGINAL Original Ifkit
{ PHIDID_INTERFACEKIT_4_8_8,			PHIDCLASS_INTERFACEKIT,		0x925, 0x8201,	0, { UFINTS(ifkit,				 4,  8,  8 ) },	"Phidget InterfaceKit 4/8/8"},			//Original Ifkit

//Pid 0x8500 (Vid 0x925): PhidgetPower

/* Valid product IDs */
{ PHIDID_RFID,							PHIDCLASS_RFID,				0x6C2, 0x30,	0, { UFINTS(rfid,				 0         ) },	"Phidget RFID"},
{ PHIDID_RFID_2OUTPUT,					PHIDCLASS_RFID,				0x6C2, 0x31,	0, { UFINTS(rfid,				 2         ) },	"Phidget RFID 2-output"},
{ PHIDID_TEMPERATURESENSOR_4,			PHIDCLASS_TEMPERATURESENSOR,0x6C2, 0x32,	0, { UFINTS(temperaturesensor,	 4         ) },	"Phidget Temperature Sensor 4-input"},
{ PHIDID_SPATIAL_ACCEL_GYRO_COMPASS,	PHIDCLASS_SPATIAL,			0x6C2, 0x33,	0, { UFINTS(spatial,			 3,  3,  3 ) },	"Phidget Spatial 3/3/3"},
{ PHIDID_RFID_2OUTPUT_READ_WRITE,			PHIDCLASS_RFID,				0x6C2, 0x34,	0, { UFINTS(rfid,				 2         ) },	"Phidget RFID Read-Write"},
{ PHIDID_FREQUENCYCOUNTER_2INPUT,		PHIDCLASS_FREQUENCYCOUNTER,	0x6C2, 0x35,	0, { UFINTS(frequencycounter,	 2         ) },	"Phidget Frequency Counter 2-input"},
{ PHIDID_INTERFACEKIT_2_2_2,			PHIDCLASS_INTERFACEKIT,		0x6C2, 0x36,	0, { UFINTS(ifkit,				 2,  2,  2 ) },	"Phidget InterfaceKit 2/2/2"},
{ PHIDID_ANALOG_4OUTPUT,				PHIDCLASS_ANALOG,			0x6C2, 0x37,	0, { UFINTS(analog,				 4         ) },	"Phidget Analog 4-output"},
{ PHIDID_SERVO_4MOTOR,					PHIDCLASS_SERVO,			0x6C2, 0x38,	0, { UFINTS(servo,				 4         ) },	"Phidget Servo Controller 4-motor"}, 
{ PHIDID_SERVO_1MOTOR,					PHIDCLASS_SERVO,			0x6C2, 0x39,	0, { UFINTS(servo,				 1         ) },	"Phidget Servo Controller 1-motor"},
{ PHIDID_ADVANCEDSERVO_8MOTOR,			PHIDCLASS_ADVANCEDSERVO,	0x6C2, 0x3A,	0, { UFINTS(advancedservo,		 8         ) },	"Phidget Advanced Servo Controller 8-motor"},
{ PHIDID_BRIDGE_4INPUT,					PHIDCLASS_BRIDGE,			0x6C2, 0x3B,	0, { UFINTS(bridge,				 4         ) },	"Phidget Bridge 4-input"},
{ PHIDID_TEMPERATURESENSOR_IR,			PHIDCLASS_TEMPERATURESENSOR,0x6C2, 0x3C,	0, { UFINTS(temperaturesensor,	 1         ) },	"Phidget Temperature Sensor IR"},
{ PHIDID_TEXTLCD_ADAPTER,				PHIDCLASS_TEXTLCD,			0x6C2, 0x3D,	0, { UFINTS(textlcd,			 0,  0,  2 ) },	"Phidget TextLCD Adapter"},
{ PHIDID_MOTORCONTROL_1MOTOR,			PHIDCLASS_MOTORCONTROL,		0x6C2, 0x3E,	0, { UFINTS(motorcontrol,		 1, 2, 1, 2) },	"Phidget Motor Controller 1-motor"},

{ PHIDID_INTERFACEKIT_0_0_4,			PHIDCLASS_INTERFACEKIT,		0x6C2, 0x40,	0, { UFINTS(ifkit,				 0,  0,  4 ) },	"Phidget InterfaceKit 0/0/4"},

{ PHIDID_INTERFACEKIT_0_16_16,			PHIDCLASS_INTERFACEKIT,		0x6C2, 0x44,	0, { UFINTS(ifkit,				 0, 16, 16 ) },	"Phidget InterfaceKit 0/16/16"},
{ PHIDID_INTERFACEKIT_8_8_8,			PHIDCLASS_INTERFACEKIT,		0x6C2, 0x45,	0, { UFINTS(ifkit,				 8,  8,  8 ) },	"Phidget InterfaceKit 8/8/8"},

{ PHIDID_TEXTLED_4x8,					PHIDCLASS_TEXTLED,			0x6C2, 0x48,	0, { UFINTS(textled,			 4,  8     ) },	"Phidget TextLED 4x8"},
{ PHIDID_TEXTLED_1x8,					PHIDCLASS_TEXTLED,			0x6C2, 0x49,	0, { UFINTS(textled,			 1,  8     ) },	"Phidget TextLED 1x8"},
{ PHIDID_LED_64,						PHIDCLASS_LED,				0x6C2, 0x4A,	0, { UFINTS(led,				64         ) },	"Phidget LED 64"},
{ PHIDID_ENCODER_1ENCODER_1INPUT,		PHIDCLASS_ENCODER,			0x6C2, 0x4B,	0, { UFINTS(encoder,			 1,  1     ) },	"Phidget Encoder 1-encoder 1-input"},
{ PHIDID_LED_64_ADV,					PHIDCLASS_LED,				0x6C2, 0x4C,	0, { UFINTS(led,				64         ) },	"Phidget LED 64 Advanced"},
{ PHIDID_IR,							PHIDCLASS_IR,				0x6C2, 0x4D,	0, { UFINTS(ir,					 0         ) },	"Phidget IR Receiver Transmitter"},

{ PHIDID_ENCODER_HS_4ENCODER_4INPUT,	PHIDCLASS_ENCODER,			0x6C2, 0x4F,	0, { UFINTS(encoder,			 4,  4     ) },	"Phidget High Speed Encoder 4-input"},

{ PHIDID_INTERFACEKIT_0_5_7,			PHIDCLASS_INTERFACEKIT,		0x6C2, 0x51,	0, { UFINTS(ifkit,				 0,  5,  7 ) },	"Phidget InterfaceKit 0/5/7"},			//with TextLCD - Spain
{ PHIDID_TEXTLCD_2x20_CUSTOM,			PHIDCLASS_TEXTLCD,			0x6C2, 0x51,	0, { UFINTS(textlcd,			 2, 20,  1 ) },	"Phidget TextLCD Custom"},				//with 0/5/7 - Spain
{ PHIDID_TEXTLCD_2x20,					PHIDCLASS_TEXTLCD,			0x6C2, 0x52,	0, { UFINTS(textlcd,			 2, 20,  1 ) },	"Phidget TextLCD"},						//no ifkit part
{ PHIDID_INTERFACEKIT_0_8_8_w_LCD,		PHIDCLASS_INTERFACEKIT,		0x6C2, 0x53,	0, { UFINTS(ifkit,				 0,  8,  8 ) },	"Phidget InterfaceKit 0/8/8"},			//with TextLCD
{ PHIDID_TEXTLCD_2x20_w_0_8_8,			PHIDCLASS_TEXTLCD,			0x6C2, 0x53,	0, { UFINTS(textlcd,			 2, 20,  1 ) },	"Phidget TextLCD"},						//with 0/8/8

{ PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT,	PHIDCLASS_MOTORCONTROL,		0x6C2, 0x58,	0, { UFINTS(motorcontrol,		 2, 4, 0, 0) },	"Phidget Low Voltage Motor Controller 2-motor 4-input"},
{ PHIDID_MOTORCONTROL_HC_2MOTOR,		PHIDCLASS_MOTORCONTROL,		0x6C2, 0x59,	0, { UFINTS(motorcontrol,		 2, 0, 0, 0) },	"Phidget High Current Motor Controller 2-motor"},

//Pid 0x60: PhidgetInterfaceKit 0/32/32

{ PHIDID_TEMPERATURESENSOR,				PHIDCLASS_TEMPERATURESENSOR,0x6C2, 0x70,	0, { UFINTS(temperaturesensor,	 1         ) },	"Phidget Temperature Sensor"},
{ PHIDID_ACCELEROMETER_2AXIS,			PHIDCLASS_ACCELEROMETER,	0x6C2, 0x71,	0, { UFINTS(accelerometer,		 2         ) },	"Phidget Accelerometer 2-axis"},
{ PHIDID_WEIGHTSENSOR,					PHIDCLASS_WEIGHTSENSOR,		0x6C2, 0x72,	0, { UFINTS(weightsensor,		 0         ) },	"Phidget Weight Sensor"},

{ PHIDID_PHSENSOR,						PHIDCLASS_PHSENSOR,			0x6C2, 0x74,	0, { UFINTS(phsensor,			 0         ) },	"Phidget PH Sensor"},

{ PHIDID_LINEAR_TOUCH,					PHIDCLASS_INTERFACEKIT,		0x6C2, 0x76,	0, { UFINTS(ifkit,				 1,  2     ) },	"Phidget Touch Slider"},
{ PHIDID_ROTARY_TOUCH,					PHIDCLASS_INTERFACEKIT,		0x6C2, 0x77,	0, { UFINTS(ifkit,				 1,  2     ) },	"Phidget Touch Rotation"},

{ PHIDID_GPS,							PHIDCLASS_GPS,				0x6C2, 0x79,	0, { UFINTS(gps,				 1         ) },	"Phidget GPS"},
{ PHIDID_UNIPOLAR_STEPPER_4MOTOR,		PHIDCLASS_STEPPER,			0x6C2, 0x7A,	0, { UFINTS(stepper,			 4         ) },	"Phidget Unipolar Stepper Controller 4-motor"},
{ PHIDID_BIPOLAR_STEPPER_1MOTOR,		PHIDCLASS_STEPPER,			0x6C2, 0x7B,	0, { UFINTS(stepper,			 1,  4     ) },	"Phidget Bipolar Stepper Controller 1-motor"},

{ PHIDID_INTERFACEKIT_8_8_8_w_LCD,		PHIDCLASS_INTERFACEKIT,		0x6C2, 0x7D,	0, { UFINTS(ifkit,				 8,  8,  8 ) },	"Phidget InterfaceKit 8/8/8"},			//with TextLCD
{ PHIDID_TEXTLCD_2x20_w_8_8_8,			PHIDCLASS_TEXTLCD,			0x6C2, 0x7D,	1, { UFINTS(textlcd,			 2, 20,  1 ) },	"Phidget TextLCD"},						//with 8/8/8
{ PHIDID_ACCELEROMETER_3AXIS,			PHIDCLASS_ACCELEROMETER,	0x6C2, 0x7E,	0, { UFINTS(accelerometer,		 3         ) },	"Phidget Accelerometer 3-axis"},
{ PHIDID_SPATIAL_ACCEL_3AXIS,			PHIDCLASS_SPATIAL,			0x6C2, 0x7F,	0, { UFINTS(spatial,			 3,  0,  0 ) },	"Phidget Spatial 0/0/3"},
{ PHIDID_ENCODER_HS_1ENCODER,			PHIDCLASS_ENCODER,			0x6C2, 0x80,	0, { UFINTS(encoder,			 1         ) },	"Phidget High Speed Encoder 1-encoder"},
{ PHIDID_INTERFACEKIT_0_0_8,			PHIDCLASS_INTERFACEKIT,		0x6C2, 0x81,	0, { UFINTS(ifkit,				 0,  0,  8 ) },	"Phidget InterfaceKit 0/0/8"},
{ PHIDID_ADVANCEDSERVO_1MOTOR,			PHIDCLASS_ADVANCEDSERVO,	0x6C2, 0x82,	0, { UFINTS(advancedservo,		 1         ) },	"Phidget Advanced Servo Controller 1-motor"},

{ PHIDID_FIRMWARE_UPGRADE,				PHIDCLASS_GENERIC,			0x6C2, 0x98,	0, { UFINTS(dummy,				 0,  0,  0 ) },	"Firmware Upgrade"},	
{ PHIDID_GENERIC,						PHIDCLASS_GENERIC,			0x6C2, 0x99,	0, { UFINTS(dummy,				 0,  0,  0 ) },	"Phidget Generic Device"},				//generic device - used for prototyping

{ 0 } //ending null
};

/*
 * List Every unique device - in terms of packet protocol
 * 0,9999 for version means any version
 * matching is >= lower version, < higher version
 */
const CPhidgetUniqueDeviceDef Phid_Unique_Device_Def[] = {
	{PHIDUID_NOTHING,									PHIDUID_NOTHING,						  0,	  0},	// Invalid
	
	{PHIDUID_ACCELEROMETER_2AXIS_2G,					PHIDID_ACCELEROMETER_2AXIS,				  0,	200},	// <200
	{PHIDUID_ACCELEROMETER_2AXIS_10G,					PHIDID_ACCELEROMETER_2AXIS,				200,	300},	// 200 - <300
	{PHIDUID_ACCELEROMETER_2AXIS_5G,					PHIDID_ACCELEROMETER_2AXIS,				300,	400},	// 300 - <400
	{PHIDUID_ACCELEROMETER_3AXIS_3G,					PHIDID_ACCELEROMETER_3AXIS,				400,	500},	// 400 - <500

	{PHIDUID_ADVANCEDSERVO_1MOTOR,						PHIDID_ADVANCEDSERVO_1MOTOR,			100,	200},	// 100 - <200
	{PHIDUID_ADVANCEDSERVO_8MOTOR,						PHIDID_ADVANCEDSERVO_8MOTOR,			100,	200},	// 100 - <200
	{PHIDUID_ADVANCEDSERVO_8MOTOR_PGOOD_FLAG,			PHIDID_ADVANCEDSERVO_8MOTOR,			200,	300},	// 200 - <300
	{PHIDUID_ADVANCEDSERVO_8MOTOR_CURSENSE_FIX,			PHIDID_ADVANCEDSERVO_8MOTOR,			300,	400},	// 300 - <400

	{PHIDUID_ANALOG_4OUTPUT,							PHIDID_ANALOG_4OUTPUT,					100,	200},	// 100 - <200

	{PHIDUID_BRIDGE_4INPUT_GAINBUG,						PHIDID_BRIDGE_4INPUT,					100,	102},	// 100 - <102
	{PHIDUID_BRIDGE_4INPUT,								PHIDID_BRIDGE_4INPUT,					102,	200},	// 102 - <200

	{PHIDUID_ENCODER_1ENCODER_1INPUT_OLD,				PHIDID_ENCODER_1ENCODER_1INPUT,			  0,	101},	// <=100
	{PHIDUID_ENCODER_1ENCODER_1INPUT_v1,				PHIDID_ENCODER_1ENCODER_1INPUT,			101,	110},	// 101 - <110
	{PHIDUID_ENCODER_1ENCODER_1INPUT_v2,				PHIDID_ENCODER_1ENCODER_1INPUT,			110,	300},	// 110 - <300
	{PHIDUID_ENCODER_HS_1ENCODER,						PHIDID_ENCODER_HS_1ENCODER,				300,	400},	// 300 - <400
	{PHIDUID_ENCODER_HS_4ENCODER_4INPUT,				PHIDID_ENCODER_HS_4ENCODER_4INPUT,		100,	200},	// 100 - <200

	{PHIDUID_FREQUENCYCOUNTER_2INPUT,					PHIDID_FREQUENCYCOUNTER_2INPUT,			  0,	200},	// < 200

	{PHIDUID_GPS,										PHIDID_GPS,								  0,   9999},	// ANY

	{PHIDUID_INTERFACEKIT_0_0_4_NO_ECHO,				PHIDID_INTERFACEKIT_0_0_4,				  0,	704},	// <704
	{PHIDUID_INTERFACEKIT_0_0_4,						PHIDID_INTERFACEKIT_0_0_4,				704,	800},	// 704 - <800
	{PHIDUID_INTERFACEKIT_0_0_8,						PHIDID_INTERFACEKIT_0_0_8,				  0,   9999},	// ANY
	{PHIDUID_INTERFACEKIT_0_5_7,						PHIDID_INTERFACEKIT_0_5_7,				  0,   9999},	// ANY
	{PHIDUID_INTERFACEKIT_0_8_8_w_LCD,					PHIDID_INTERFACEKIT_0_8_8_w_LCD,		  0,   9999},	// ANY
	{PHIDUID_INTERFACEKIT_0_16_16_NO_ECHO,				PHIDID_INTERFACEKIT_0_16_16,			  0,	601},	// <=600
	{PHIDUID_INTERFACEKIT_0_16_16_BITBUG,				PHIDID_INTERFACEKIT_0_16_16,			601,	602},	// 601
	{PHIDUID_INTERFACEKIT_0_16_16,						PHIDID_INTERFACEKIT_0_16_16,			602,	700},	// 602 - <700
	{PHIDUID_INTERFACEKIT_2_2_2,						PHIDID_INTERFACEKIT_2_2_2,				  0,	200},	// <200
	{PHIDUID_INTERFACEKIT_2_8_8,						PHIDID_INTERFACEKIT_2_8_8,				  0,   9999},	// ANY
	{PHIDUID_INTERFACEKIT_4_8_8,						PHIDID_INTERFACEKIT_4_8_8,				  0,	101},	// <=100
	{PHIDUID_INTERFACEKIT_8_8_8_NO_ECHO,				PHIDID_INTERFACEKIT_8_8_8,				  0,	821},	// <=820
	{PHIDUID_INTERFACEKIT_8_8_8,						PHIDID_INTERFACEKIT_8_8_8,				821,	900},	// 821 - <900
	{PHIDUID_INTERFACEKIT_8_8_8_FAST,					PHIDID_INTERFACEKIT_8_8_8,				900,   1000},	// 900 - <1000
	{PHIDUID_INTERFACEKIT_8_8_8_w_LCD_NO_ECHO,			PHIDID_INTERFACEKIT_8_8_8_w_LCD,		  0,	120},	// <120
	{PHIDUID_INTERFACEKIT_8_8_8_w_LCD,					PHIDID_INTERFACEKIT_8_8_8_w_LCD,		120,	200},	// 120 - <200
	{PHIDUID_INTERFACEKIT_8_8_8_w_LCD_FAST,				PHIDID_INTERFACEKIT_8_8_8_w_LCD,		200,	300},	// 200 - <300
	{PHIDUID_INTERFACEKIT_TOUCH_SLIDER,					PHIDID_LINEAR_TOUCH,					  0,   9999},	// ANY
	{PHIDUID_INTERFACEKIT_TOUCH_ROTARY,					PHIDID_ROTARY_TOUCH,					  0,   9999},	// ANY

	{PHIDUID_IR,										PHIDID_IR,								100,	200},	// 100 - <200

	{PHIDUID_LED_64,									PHIDID_LED_64,							100,	300},	// 100 - <300
	{PHIDUID_LED_64_ADV,								PHIDID_LED_64_ADV,						100,	200},	// 100 - <200
	{PHIDUID_LED_64_ADV_M3,								PHIDID_LED_64_ADV,						200,	300},	// 200 - <300

	{PHIDUID_MOTORCONTROL_1MOTOR,						PHIDID_MOTORCONTROL_1MOTOR,				100, 	200},	// 100 - <200
	{PHIDUID_MOTORCONTROL_HC_2MOTOR,					PHIDID_MOTORCONTROL_HC_2MOTOR,			100,	200},	// 100 - <200
	{PHIDUID_MOTORCONTROL_LV_2MOTOR_4INPUT,				PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT,	100,	200},	// 100 - <200

	{PHIDUID_PHSENSOR,									PHIDID_PHSENSOR,						100,	200},	// 100 - <200

	{PHIDUID_RFID_OLD,									PHIDID_RFID,							  0,	104},	// <=103
	{PHIDUID_RFID,										PHIDID_RFID,							104,	200},	// 104 - <200
	{PHIDUID_RFID_2OUTPUT_NO_ECHO,						PHIDID_RFID_2OUTPUT,					200,	201},	// 200
	{PHIDUID_RFID_2OUTPUT,								PHIDID_RFID_2OUTPUT,					201,	300},	// 201 - <300
	{PHIDUID_RFID_2OUTPUT_READ_WRITE,					PHIDID_RFID_2OUTPUT_READ_WRITE,			100,	200},	// 100 - <200

	{PHIDUID_SERVO_1MOTOR_OLD,							PHIDID_SERVO_1MOTOR_OLD,				200,	201},	// 200
	{PHIDUID_SERVO_1MOTOR_OLD,							PHIDID_SERVO_1MOTOR,					200,	201},	// 200
	{PHIDUID_SERVO_4MOTOR_OLD,							PHIDID_SERVO_4MOTOR_OLD,				200,	201},	// 200
	{PHIDUID_SERVO_4MOTOR_OLD,							PHIDID_SERVO_4MOTOR,					200,	201},	// 200
	{PHIDUID_SERVO_1MOTOR_NO_ECHO,						PHIDID_SERVO_1MOTOR,					300,	313},	// 300 - 312
	{PHIDUID_SERVO_1MOTOR,								PHIDID_SERVO_1MOTOR,					313,	400},	// 313 - <400
	{PHIDUID_SERVO_4MOTOR_NO_ECHO,						PHIDID_SERVO_4MOTOR,					300,	313},	// 300 - 312
	{PHIDUID_SERVO_4MOTOR,								PHIDID_SERVO_4MOTOR,					313,	400},	// 313 - <400

	{PHIDUID_SPATIAL_ACCEL_3AXIS_1049,					PHIDID_SPATIAL_ACCEL_3AXIS,				  0,	200},	// <200
	{PHIDUID_SPATIAL_ACCEL_3AXIS_1041,					PHIDID_SPATIAL_ACCEL_3AXIS,				200,	300},	// 200 - <300
	{PHIDUID_SPATIAL_ACCEL_3AXIS_1043,					PHIDID_SPATIAL_ACCEL_3AXIS,				300,	400},	// 300 - <400
	{PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056,			PHIDID_SPATIAL_ACCEL_GYRO_COMPASS,		  0,	200},	// <200
	{PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1056_NEG_GAIN,	PHIDID_SPATIAL_ACCEL_GYRO_COMPASS,		200,	300},	// 200 - <300
	{PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042,			PHIDID_SPATIAL_ACCEL_GYRO_COMPASS,		300,	400},	// 300 - <400
	{PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044,			PHIDID_SPATIAL_ACCEL_GYRO_COMPASS,		400,	500},	// 400 - <500

	{PHIDUID_STEPPER_BIPOLAR_1MOTOR,					PHIDID_BIPOLAR_STEPPER_1MOTOR,			100,	200},	// 100 - <200
	{PHIDUID_STEPPER_BIPOLAR_1MOTOR_M3,					PHIDID_BIPOLAR_STEPPER_1MOTOR,			200,	300},	// 200 - <300
	{PHIDUID_STEPPER_UNIPOLAR_4MOTOR,					PHIDID_UNIPOLAR_STEPPER_4MOTOR,			100,	200},	// 100 - <200

	{PHIDUID_TEMPERATURESENSOR_OLD,						PHIDID_TEMPERATURESENSOR,				  0,	200},	// <200
	{PHIDUID_TEMPERATURESENSOR,							PHIDID_TEMPERATURESENSOR,				200,	300},	// 200 - <300
	{PHIDUID_TEMPERATURESENSOR_AD22100,					PHIDID_TEMPERATURESENSOR,				300,	400},	// 300 - <400
	{PHIDUID_TEMPERATURESENSOR_TERMINAL_BLOCKS,			PHIDID_TEMPERATURESENSOR,				400,	500},	// 400 - <500
	{PHIDUID_TEMPERATURESENSOR_4,						PHIDID_TEMPERATURESENSOR_4,				100,	200},	// 100 - <200
	{PHIDUID_TEMPERATURESENSOR_IR,						PHIDID_TEMPERATURESENSOR_IR,			100,	200},	// 100 - <200

	{PHIDUID_TEXTLCD_2x20,								PHIDID_TEXTLCD_2x20,					  0,   9999},	// ANY
	{PHIDUID_TEXTLCD_2x20,								PHIDID_TEXTLCD_2x20_CUSTOM,				  0,   9999},	// ANY
	{PHIDUID_TEXTLCD_2x20,								PHIDID_TEXTLCD_2x20_w_0_8_8,			  0,   9999},	// ANY
	{PHIDUID_TEXTLCD_2x20_w_8_8_8,						PHIDID_TEXTLCD_2x20_w_8_8_8,			  0,	200},	// <200
	{PHIDUID_TEXTLCD_2x20_w_8_8_8_BRIGHTNESS,			PHIDID_TEXTLCD_2x20_w_8_8_8,			200,   9999},	// 200+
	{PHIDUID_TEXTLCD_ADAPTER,							PHIDID_TEXTLCD_ADAPTER,					  0,   9999},	// ANY

	{PHIDUID_TEXTLED_1x8,								PHIDID_TEXTLED_1x8,						111,	114},	// 111 - 113
	{PHIDUID_TEXTLED_4x8,								PHIDID_TEXTLED_4x8,						200,	300},	// 200 - <300

	{PHIDUID_WEIGHTSENSOR,								PHIDID_WEIGHTSENSOR,					  0,   9999},	// ANY

	{PHIDUID_FIRMWARE_UPGRADE,							PHIDID_FIRMWARE_UPGRADE,				  0,   9999},	// ANY
	{PHIDUID_GENERIC,									PHIDID_GENERIC,							  0,   9999},	// ANY
	{ 0 } //ending null
};
