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

#ifndef __CPHIDGETFREQUENCYCOUNTER
#define __CPHIDGETFREQUENCYCOUNTER
#include "cphidget.h"

/** \defgroup phidfreq Phidget Frequency Counter
 * \ingroup phidgets
 * These calls are specific to the Phidget Frequency Counter object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 *
 * @{
 */

/**
 * Filter Types supported by the frequency counter.
 */
typedef enum {
	PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING = 1,	/**< Zero crossing signal filter. */
	PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL,		/**< Logic level signal filter. */
	PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_UNKNOWN				/**< Filter type unknown. */
}  CPhidgetFrequencyCounter_FilterType;

DPHANDLE(FrequencyCounter)
CHDRSTANDARD(FrequencyCounter)

/**
 * Gets the number of inputs supported by this phidget frequency counter.
 * @param phid An attached phidget frequency counter handle.
 * @param count The input count.
 */
CHDRGET(FrequencyCounter,FrequencyInputCount,int *count)
/**
 * Gets the measured frequency of an input, in Hz.
 * @param phid An attached phidget frequency counter handle.
 * @param index The input index.
 * @param frequency The frequency.
 */
CHDRGETINDEX(FrequencyCounter,Frequency,double *frequency)
/**
 * Gets the total time that has passed since the last reset on this input, in microseconds.
 * @param phid An attached phidget frequency counter handle.
 * @param index The input index.
 * @param time The time.
 */
CHDRGETINDEX(FrequencyCounter,TotalTime,__int64 *time)
/**
 * Gets the total number of ticks that have happened since the last reset on this input.
 * @param phid An attached phidget frequency counter handle.
 * @param index The input index.
 * @param count The tick count.
 */
CHDRGETINDEX(FrequencyCounter,TotalCount,__int64 *count)
/**
 * Gets the timeout value for an input, in microseconds. This controls the lowest measurable frequency.
 * @param phid An attached phidget frequency counter handle.
 * @param index The input index.
 * @param timeout The timeout.
 */
CHDRSETINDEX(FrequencyCounter,Timeout,int timeout)
/**
 * Sets the timeout value for an input, in microseconds.
 * @param phid An attached phidget frequency counter handle.
 * @param index The input index.
 * @param timeout The timeout.
 */
CHDRGETINDEX(FrequencyCounter,Timeout,int *timeout)
/**
 * Gets the enabled state for an input.
 * @param phid An attached phidget frequency counter handle.
 * @param index The input index.
 * @param enabledState The enabled state.
 */
CHDRSETINDEX(FrequencyCounter,Enabled,int enabledState)
/**
 * Sets the enabled state for an input.
 * @param phid An attached phidget frequency counter handle.
 * @param index The input index.
 * @param enabledState The enabled state.
 */
CHDRGETINDEX(FrequencyCounter,Enabled,int *enabledState)
/**
 * Gets the filter type for an input.
 * @param phid An attached phidget frequency counter handle.
 * @param index The input index.
 * @param filter The filter type.
 */
CHDRSETINDEX(FrequencyCounter,Filter,CPhidgetFrequencyCounter_FilterType filter)
/**
 * Sets the filter type for an input.
 * @param phid An attached phidget frequency counter handle.
 * @param index The input index.
 * @param filter The filter type.
 */
CHDRGETINDEX(FrequencyCounter,Filter,CPhidgetFrequencyCounter_FilterType *filter)
/**
 * Resets total count and total time for an input.
 * @param phid An attached phidget frequency counter handle.
 * @param index The input index.
 */
PHIDGET21_API int CCONV CPhidgetFrequencyCounter_reset(CPhidgetFrequencyCounterHandle phid, int index);
/**
 * Sets a count event handler. This is called when ticks have been counted on an input, or when the timeout has passed.
 * @param phid A phidget frequency counter handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(FrequencyCounter,Count,int time,int counts)

#ifndef EXTERNALPROTO
CHDRGETINDEX(FrequencyCounter,TotalTime32,int *time)
CHDRGETINDEX(FrequencyCounter,TotalCount32,int *count)
#define FREQCOUNTER_MAXINPUTS 2

#define FREQCOUNTER_TICKS_PER_SEC	100000
#define FREQCOUNTER_MICROSECONDS_PER_TICK	(1000000 / FREQCOUNTER_TICKS_PER_SEC)

//OUT packet flags
#define FREQCOUNTER_FLAG_CH1_LOGIC 0x01
#define FREQCOUNTER_FLAG_CH0_LOGIC 0x02
#define FREQCOUNTER_FLAG_CH1_ENABLE 0x04
#define FREQCOUNTER_FLAG_CH0_ENABLE 0x08

struct _CPhidgetFrequencyCounter {
	CPhidget phid;

	int (CCONV *fptrCount)(CPhidgetFrequencyCounterHandle, void *, int, int, int);
	void *fptrCountptr;

	int timeout[FREQCOUNTER_MAXINPUTS]; //microseconds
	CPhidgetFrequencyCounter_FilterType filter[FREQCOUNTER_MAXINPUTS];
	unsigned char enabled[FREQCOUNTER_MAXINPUTS];

	CPhidgetFrequencyCounter_FilterType filterEcho[FREQCOUNTER_MAXINPUTS];
	unsigned char enabledEcho[FREQCOUNTER_MAXINPUTS];

	double frequency[FREQCOUNTER_MAXINPUTS]; //Hz
	int totalTicksSinceLastCount[FREQCOUNTER_MAXINPUTS]; //ticks

	__int64 totalCount[FREQCOUNTER_MAXINPUTS];
	__int64 totalTime[FREQCOUNTER_MAXINPUTS]; //microseconds

	int flip[FREQCOUNTER_MAXINPUTS];
	int lastPacketCount;
	
	CThread_mutex_t resetlock; /* protects reset */

	int countsGood[FREQCOUNTER_MAXINPUTS]; //for webservice

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;
} typedef CPhidgetFrequencyCounterInfo;
#endif

/** @} */

#endif
