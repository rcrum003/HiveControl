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

#ifndef __CPHIDGETLED
#define __CPHIDGETLED
#include "cphidget.h"

/** \defgroup phidled Phidget LED 
 * \ingroup phidgets
 * These calls are specific to the Phidget LED object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(LED)
CHDRSTANDARD(LED)

/**
 * The Phidget LED supports these current limits
 */
typedef enum {
	PHIDGET_LED_CURRENT_LIMIT_20mA = 1,	/**< 20mA */
	PHIDGET_LED_CURRENT_LIMIT_40mA,		/**< 40mA */
	PHIDGET_LED_CURRENT_LIMIT_60mA,		/**< 60mA */
	PHIDGET_LED_CURRENT_LIMIT_80mA		/**< 80mA */
}  CPhidgetLED_CurrentLimit;
/**
 * The Phidget LED supports these voltages
 */
typedef enum {
	PHIDGET_LED_VOLTAGE_1_7V = 1,	/**< 1.7V */
	PHIDGET_LED_VOLTAGE_2_75V,		/**< 2.75V */
	PHIDGET_LED_VOLTAGE_3_9V,		/**< 3.9V */
	PHIDGET_LED_VOLTAGE_5_0V		/**< 5.0V */
}  CPhidgetLED_Voltage;

/**
 * Gets the number of LEDs supported by this board.
 * @param phid An attached phidget LED handle.
 * @param count The led count.
 */
CHDRGET(LED,LEDCount,int *count)

/**
 * Gets the current limit. This is for all ouputs.
 * @param phid An attached phidget LED handle.
 * @param currentLimit The Current Limit.
 */
CHDRGET(LED,CurrentLimit,CPhidgetLED_CurrentLimit *currentLimit)
/**
 * Sets the current limit. This is for all ouputs.
 * @param phid An attached phidget LED handle.
 * @param currentLimit The Current Limit.
 */
CHDRSET(LED,CurrentLimit,CPhidgetLED_CurrentLimit currentLimit)
/**
 * Gets the output voltate. This is for all ouputs.
 * @param phid An attached phidget LED handle.
 * @param voltage The Output Voltage.
 */
CHDRGET(LED,Voltage,CPhidgetLED_Voltage *voltage)
/**
 * Sets the output voltage. This is for all ouputs.
 * @param phid An attached phidget LED handle.
 * @param voltage The Output Voltage.
 */
CHDRSET(LED,Voltage,CPhidgetLED_Voltage voltage)
/**
 * Gets the brightness of an LED.
 * @param phid An attached phidget LED handle.
 * @param index The LED index.
 * @param brightness The LED brightness (0-100).
 */
CHDRGETINDEX(LED,Brightness,double *brightness)
/**
 * Sets the brightness of an LED.
 * @param phid An attached phidget LED handle.
 * @param index The LED index.
 * @param brightness The LED brightness (0-100).
 */
CHDRSETINDEX(LED,Brightness,double brightness)
/**
 * Gets the current limit of an LED.
 * @param phid An attached phidget LED handle.
 * @param index The LED index.
 * @param limit The LED current limit (0-80 mA).
 */
CHDRGETINDEX(LED,CurrentLimitIndexed,double *limit)
/**
 * Sets the current limit of an LED.
 * @param phid An attached phidget LED handle.
 * @param index The LED index.
 * @param limit The LED current limit (0-80 mA).
 */
CHDRSETINDEX(LED,CurrentLimitIndexed,double limit)

#ifndef REMOVE_DEPRECATED
DEP_CHDRGET("Deprecated - use CPhidgetLED_getLEDCount",LED,NumLEDs,int *)
DEP_CHDRGETINDEX("Deprecated - use CPhidgetLED_getBrightness",LED,DiscreteLED,int *brightness)
DEP_CHDRSETINDEX("Deprecated - use CPhidgetLED_getBrightness",LED,DiscreteLED,int brightness)
#endif
	
#ifndef EXTERNALPROTO

#define LED_MAXLEDS 64

//OUT Packet Types
#define LED64_NORMAL_PACKET 0x00
#define LED64_CONTROL_PACKET 0x40
#define LED64_OUTLOW_PACKET 0x80
#define LED64_OUTHIGH_PACKET 0xc0

#define LED64_M3_OUT_LOW_PACKET 0x00
#define LED64_M3_OUT_HIGH_PACKET 0x20
#define LED64_M3_CONTROL_PACKET 0x40

//IN Packet Types
#define LED64_IN_LOW_PACKET 0x00
#define LED64_IN_HIGH_PACKET 0x80

#define LED64_M3_IN_LOW_PACKET 0x00
#define LED64_M3_IN_HIGH_PACKET 0x20
#define LED64_M3_IN_MISC_PACKET 0x40

//Flags
#define LED64_PGOOD_FLAG 0x01
#define LED64_CURSELA_FLAG 0x02
#define LED64_CURSELB_FLAG 0x04
#define LED64_PWRSELA_FLAG 0x08
#define LED64_PWRSELB_FLAG 0x10
#define LED64_FAULT_FLAG 0x20
#define LED64_OE_FLAG 0x40

//M3 LED64
#define LED64_M3_CURRENTLIMIT	80 //80 mA max

struct _CPhidgetLED 
{
	CPhidget phid;

	double LED_Power[LED_MAXLEDS];
	double LED_CurrentLimit[LED_MAXLEDS];
	CPhidgetLED_Voltage voltage;
	CPhidgetLED_CurrentLimit currentLimit;

	double nextLED_Power[LED_MAXLEDS];
	double lastLED_Power[LED_MAXLEDS];
	unsigned char changedLED_Power[LED_MAXLEDS];
	unsigned char changeRequests;

	double LED_PowerEcho[LED_MAXLEDS];
	double LED_CurrentLimitEcho[LED_MAXLEDS];
	unsigned char outputEnabledEcho[LED_MAXLEDS];
	unsigned char ledOpenDetectEcho[LED_MAXLEDS];
	unsigned char powerGoodEcho;
	unsigned char outputEnableEcho;
	unsigned char faultEcho;
	CPhidgetLED_Voltage voltageEcho;
	CPhidgetLED_CurrentLimit currentLimitEcho;
	
	unsigned char TSDCount[4], TSDClearCount[4], TWarnCount[4], TWarnClearCount[4], PGoodErrState;
	
	unsigned char controlPacketWaiting;
	unsigned char lastOutputPacket;

} typedef CPhidgetLEDInfo;

#endif

/** @} */
#endif
