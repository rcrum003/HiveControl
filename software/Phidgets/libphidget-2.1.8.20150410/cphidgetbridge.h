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

#ifndef __CPHIDGETBRIDGE
#define __CPHIDGETBRIDGE
#include "cphidget.h"

/** \defgroup phidbridge Phidget Bridge 
 * \ingroup phidgets
 * These calls are specific to the Phidget Bridge object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

/**
 * List of gains supported by the PhidgetBridge.
 */
typedef enum {
	PHIDGET_BRIDGE_GAIN_1 = 1,	/**< Gain of 1. */
	PHIDGET_BRIDGE_GAIN_8,		/**< Gain of 8. */
	PHIDGET_BRIDGE_GAIN_16,		/**< Gain of 16. */
	PHIDGET_BRIDGE_GAIN_32,		/**< Gain of 32. */
	PHIDGET_BRIDGE_GAIN_64,		/**< Gain of 64. */
	PHIDGET_BRIDGE_GAIN_128,	/**< Gain of 128. */
	PHIDGET_BRIDGE_GAIN_UNKNOWN	/**< Unknown Gain. */
}  CPhidgetBridge_Gain;

DPHANDLE(Bridge)
CHDRSTANDARD(Bridge)

/**
 * Gets the number of inputs supported by this phidget bridge.
 * @param phid An attached phidget bridge handle.
 * @param count The input count.
 */
CHDRGET(Bridge,InputCount,int *count)
/**
 * Gets the current value of a bridge input, in mV/V.
 * @param phid An attached phidget bridge handle.
 * @param index The input index.
 * @param value The value.
 */
CHDRGETINDEX(Bridge,BridgeValue,double *value)
/**
 * Gets the maximum value supported by a bridge input, in mV/V. This is affected by Gain.
 * @param phid An attached phidget bridge handle.
 * @param index The input index.
 * @param max The max value.
 */
CHDRGETINDEX(Bridge,BridgeMax,double *max)
/**
 * Gets the minimum value supported by a bridge input, in mV/V. This is affected by Gain.
 * @param phid An attached phidget bridge handle.
 * @param index The input index.
 * @param min The min value.
 */
CHDRGETINDEX(Bridge,BridgeMin,double *min)
/**
 * Gets the enabled state for an input.
 * @param phid An attached phidget bridge handle.
 * @param index The input index.
 * @param enabledState The enabled state.
 */
CHDRSETINDEX(Bridge,Enabled,int enabledState)
/**
 * Sets the enabled state for an input.
 * @param phid An attached phidget bridge handle.
 * @param index The input index.
 * @param enabledState The enabled state.
 */
CHDRGETINDEX(Bridge,Enabled,int *enabledState)
/**
 * Gets the the Gain for an input.
 * @param phid An attached phidget bridge handle.
 * @param index The input index.
 * @param gain The gain.
 */
CHDRGETINDEX(Bridge,Gain, CPhidgetBridge_Gain *gain)
/**
 * Sets the the Gain for an input.
 * @param phid An attached phidget bridge handle.
 * @param index The input index.
 * @param gain The gain.
 */
CHDRSETINDEX(Bridge,Gain, CPhidgetBridge_Gain gain)
/**
 * Gets the the data rate for the Phidget Bridge, in milliseconds.
 * @param phid An attached phidget bridge handle.
 * @param milliseconds The data rate.
 */
CHDRGET(Bridge,DataRate, int *milliseconds)
/**
 * Sets the the data rate for the Phidget Bridge, in milliseconds.
 * @param phid An attached phidget bridge handle.
 * @param milliseconds The data rate.
 */
CHDRSET(Bridge,DataRate, int milliseconds)
/**
 * Gets the the maximum data rate for the Phidget Bridge, in milliseconds.
 * @param phid An attached phidget bridge handle.
 * @param max The max data rate.
 */
CHDRGET(Bridge,DataRateMax, int *max)
/**
 * Gets the the minimum data rate for the Phidget Bridge, in milliseconds.
 * @param phid An attached phidget bridge handle.
 * @param min The min data rate.
 */
CHDRGET(Bridge,DataRateMin, int *min)
/**
 * Sets a bridge data event handler. This is called at a set rate as defined by data rate.
 * @param phid A phidget bridge handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(Bridge,BridgeData,double value)

#ifndef EXTERNALPROTO
#define BRIDGE_MAXINPUTS 4
struct _CPhidgetBridge {
	CPhidget phid;
	int (CCONV *fptrBridgeData)(CPhidgetBridgeHandle, void *, int, double);           
	void *fptrBridgeDataptr;

	unsigned char enabled[BRIDGE_MAXINPUTS];
	CPhidgetBridge_Gain gain[BRIDGE_MAXINPUTS];
	int dataRate;

	double bridgeValue[BRIDGE_MAXINPUTS];
	unsigned char enabledEcho[BRIDGE_MAXINPUTS];
	CPhidgetBridge_Gain gainEcho[BRIDGE_MAXINPUTS];
	int dataRateEcho;

	int dataRateMin, dataRateMax;
	double bridgeMin[BRIDGE_MAXINPUTS], bridgeMax[BRIDGE_MAXINPUTS];

	unsigned char outOfRange[BRIDGE_MAXINPUTS], lastOutOfRange[BRIDGE_MAXINPUTS];
	
	//for PWS
	double lastBridgeMin[BRIDGE_MAXINPUTS], lastBridgeMax[BRIDGE_MAXINPUTS];

	//Firmware bug handling
	unsigned char chEnabledBugNotValid[BRIDGE_MAXINPUTS];
	unsigned char ch0EnableOverride;

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;
} typedef CPhidgetBridgeInfo;
#endif

/** @} */

#endif
