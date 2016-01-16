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

#ifndef __CPHIDGETTEXTLED
#define __CPHIDGETTEXTLED
#include "cphidget.h"

/** \defgroup phidtextled Phidget TextLED 
 * \ingroup phidgets
 * These calls are specific to the Phidget Text LED object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(TextLED)
CHDRSTANDARD(TextLED)

/**
 * Gets the number of rows supported by this display.
 * @param phid An attached phidget text led handle.
 * @param count The row count.
 */
CHDRGET(TextLED,RowCount,int *count)
/**
 * Gets the number of columns per supported by this display.
 * @param phid An attached phidget text led handle.
 * @param count The Column count.
 */
CHDRGET(TextLED,ColumnCount,int *count)

/**
 * Gets the last set brightness value.
 * @param phid An attached phidget text led handle.
 * @param brightness The brightness (0-100).
 */
CHDRGET(TextLED,Brightness,int *brightness)
/**
 * Sets the last set brightness value.
 * @param phid An attached phidget text led handle.
 * @param brightness The brightness (0-100).
 */
CHDRSET(TextLED,Brightness,int brightness)

/**
 * Sets a row on the display.
 * @param phid An attached phidget text led handle.
 * @param index The row index.
 * @param displayString The string to display. Make sure this is not longer then \ref CPhidgetTextLED_getColumnCount.
 */
CHDRSETINDEX(TextLED,DisplayString,char *displayString)

#ifndef REMOVE_DEPRECATED
DEP_CHDRGET("Deprecated - use CPhidgetTextLED_getRowCount",TextLED,NumRows,int *)
DEP_CHDRGET("Deprecated - use CPhidgetTextLED_getColumnCount",TextLED,NumColumns,int *)
#endif

#ifndef EXTERNALPROTO
#define TEXTLED_MAXROWS 4
#define TEXTLED_MAXCOLS 8

#define TEXTLED_BRIGHTNESS_PACKET -1
#define TEXTLED_DISPLAYSTRING_PACKET 0

struct _CPhidgetTextLED {
	CPhidget phid;

	char *displayStringPtr[TEXTLED_MAXROWS];
	int brightness;

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;

	char *strings[TEXTLED_MAXROWS];
} typedef CPhidgetTextLEDInfo;
#endif

/** @} */

#endif
