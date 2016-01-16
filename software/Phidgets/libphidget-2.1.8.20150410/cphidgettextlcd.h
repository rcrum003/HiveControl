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

#ifndef __CPHIDGETTEXTLCD
#define __CPHIDGETTEXTLCD
#include "cphidget.h"

/** \defgroup phidtextlcd Phidget TextLCD 
 * \ingroup phidgets
 * These calls are specific to the Phidget Text LCD object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(TextLCD)
CHDRSTANDARD(TextLCD)

/**
 * Gets the number of rows supported by this display.
 * @param phid An attached phidget text lcd handle.
 * @param count The row count.
 */
CHDRGET(TextLCD,RowCount,int *count)
/**
 * Gets the number of columns per supported by this display.
 * @param phid An attached phidget text lcd handle.
 * @param count The Column count.
 */
CHDRGET(TextLCD,ColumnCount,int *count)

/**
 * Gets the state of the backlight.
 * @param phid An attached phidget text lcd handle.
 * @param backlightState The backlight state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGET(TextLCD,Backlight,int *backlightState)
/**
 * Sets the state of the backlight.
 * @param phid An attached phidget text lcd handle.
 * @param backlightState The backlight state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSET(TextLCD,Backlight,int backlightState)
/**
 * Gets the brightness of the backlight. Not supported on all TextLCDs
 * @param phid An attached phidget text lcd handle.
 * @param brightness The backlight brightness (0-255).
 */
CHDRGET(TextLCD,Brightness,int *brightness)
/**
 * Sets the brightness of the backlight. Not supported on all TextLCDs
 * @param phid An attached phidget text lcd handle.
 * @param brightness The backlight brightness (0-255).
 */
CHDRSET(TextLCD,Brightness,int brightness)
/**
 * Gets the last set contrast value.
 * @param phid An attached phidget text lcd handle.
 * @param contrast The contrast (0-255).
 */
CHDRGET(TextLCD,Contrast,int *contrast)
/**
 * Sets the last set contrast value.
 * @param phid An attached phidget text lcd handle.
 * @param contrast The contrast (0-255).
 */
CHDRSET(TextLCD,Contrast,int contrast)
/**
 * Gets the cursor visible state.
 * @param phid An attached phidget text lcd handle.
 * @param cursorState The state of the cursor.
 */
CHDRGET(TextLCD,CursorOn,int *cursorState)
/**
 * Sets the cursor visible state.
 * @param phid An attached phidget text lcd handle.
 * @param cursorState The state of the cursor.
 */
CHDRSET(TextLCD,CursorOn,int cursorState)
/**
 * Gets the cursor blink state.
 * @param phid An attached phidget text lcd handle.
 * @param cursorBlinkState The cursor blink state.
 */
CHDRGET(TextLCD,CursorBlink,int *cursorBlinkState)
/**
 * Sets the cursor blink state.
 * @param phid An attached phidget text lcd handle.
 * @param cursorBlinkState The cursor blink state.
 */
CHDRSET(TextLCD,CursorBlink,int cursorBlinkState)
/**
 * Sets a custom character. See the product manual for more information.
 * @param phid An attached phidget text lcd handle.
 * @param index The custom character index (8-15).
 * @param var1 The first part of the custom character.
 * @param var2 The second part of the custom character.
 */
CHDRSETINDEX(TextLCD,CustomCharacter,int var1,int var2)
/**
 * Sets a single character on the display.
 * @param phid An attached phidget text lcd handle.
 * @param index The row index.
 * @param column The column index.
 * @param character The character to display.
 */
CHDRSETINDEX(TextLCD,DisplayCharacter,int column,unsigned char character)
/**
 * Sets a row on the display.
 * @param phid An attached phidget text lcd handle.
 * @param index The row index.
 * @param displayString The string to display. Make sure this is not longer then \ref CPhidgetTextLCD_getColumnCount.
 */
CHDRSETINDEX(TextLCD,DisplayString,char *displayString)

/**
 * The Phidget TextLCD Adapter supports these screen sizes
 */
typedef enum {
	PHIDGET_TEXTLCD_SCREEN_NONE = 1,/**< no screen attached */
	PHIDGET_TEXTLCD_SCREEN_1x8,		/**< 1 row, 8 column screen */
	PHIDGET_TEXTLCD_SCREEN_2x8,		/**< 2 row, 8 column screen */
	PHIDGET_TEXTLCD_SCREEN_1x16,	/**< 1 row, 16 column screen */
	PHIDGET_TEXTLCD_SCREEN_2x16,	/**< 2 row, 16 column screen */
	PHIDGET_TEXTLCD_SCREEN_4x16,	/**< 4 row, 16 column screen */
	PHIDGET_TEXTLCD_SCREEN_2x20,	/**< 2 row, 20 column screen */
	PHIDGET_TEXTLCD_SCREEN_4x20,	/**< 4 row, 20 column screen */
	PHIDGET_TEXTLCD_SCREEN_2x24,	/**< 2 row, 24 column screen */
	PHIDGET_TEXTLCD_SCREEN_1x40,	/**< 1 row, 40 column screen */
	PHIDGET_TEXTLCD_SCREEN_2x40,	/**< 2 row, 40 column screen */
	PHIDGET_TEXTLCD_SCREEN_4x40,	/**< 4 row, 40 column screen (special case, requires both screen connections) */
	PHIDGET_TEXTLCD_SCREEN_UNKNOWN
}  CPhidgetTextLCD_ScreenSize;

/**
 * Gets the number of Display supported by this TextLCD
 * @param phid An attached phidget text lcd handle.
 * @param count The Screen count.
 */
CHDRGET(TextLCD,ScreenCount,int *count)
/**
 * Gets the active screen.
 * @param phid An attached phidget text lcd handle.
 * @param screenIndex The active screen.
 */
CHDRGET(TextLCD,Screen,int *screenIndex)
/**
 * Sets the active screen. This is the screen that all subsequent API calls will apply to.
 * @param phid An attached phidget text lcd handle.
 * @param screenIndex The active screen.
 */
CHDRSET(TextLCD,Screen,int screenIndex)
/**
 * Gets the screen size.
 * @param phid An attached phidget text lcd handle.
 * @param screenSize The screen size.
 */
CHDRGET(TextLCD,ScreenSize,CPhidgetTextLCD_ScreenSize *screenSize)
/**
 * Sets the active screen size. Only supported on the TextLCD Adapter.
 * @param phid An attached phidget text lcd handle.
 * @param screenSize The screen size.
 */
CHDRSET(TextLCD,ScreenSize,CPhidgetTextLCD_ScreenSize screenSize)
/**
 * Initializes the active screen. Only supported on the TextLCD adapter.
 * This should be called if a screen is attached after power up, or to clear the screen after
 * setting the size.
 * @param phid An attached phidget text lcd handle.
 */
PHIDGET21_API int CCONV CPhidgetTextLCD_initialize(CPhidgetTextLCDHandle phid); 

#ifndef REMOVE_DEPRECATED
DEP_CHDRGET("Deprecated - use CPhidgetTextLCD_getRowCount",TextLCD,NumRows,int *)
DEP_CHDRGET("Deprecated - use CPhidgetTextLCD_getColumnCount",TextLCD,NumColumns,int *)
#endif

#ifndef EXTERNALPROTO
#define TEXTLCD_MAXROWS 2
#define TEXTLCD_MAXCOLS 40
#define TEXTLCD_MAXSCREENS 2

#define TEXTLCD_CURSOR_PACKET		0x00
#define TEXTLCD_BACKLIGHT_PACKET	0x11
#define TEXTLCD_CONTRAST_PACKET		0x12
#define TEXTLCD_INIT_PACKET			0x13

#define TEXTLCD_SCREEN(x)			(x << 5)
#define TEXTLCD_CGRAM_ADDR(x)		(x << 3)	//each custom character takes 8 bytes of CGRAM storage

#define TEXTLCD_ESCAPE_CHAR			0x00
#define TEXTLCD_COMMAND_MODE		0x01
#define TEXTLCD_DATA_MODE			0x02

//HD44780 commands
#define HD44780_CLEAR_DISPLAY	0x01
#define HD44780_CURSOR_HOME		0x02

//These are ORed together
#define HD44780_DISPLAY_CNTRL	0x08
#define HD44780_DISPLAY_ON		0x04
#define HD44780_CURSOR_ON		0x02
#define HD44780_CURSOR_BLINK_ON	0x01

#define HD44780_SET_CGRAM_ADDR	0x40
#define HD44780_SET_DDRAM_ADDR	0x80

struct _CPhidgetTextLCD {
	CPhidget phid;

	int currentScreen;
	int lastScreen;

	unsigned char cursorOn[TEXTLCD_MAXSCREENS], cursorBlink[TEXTLCD_MAXSCREENS], backlight[TEXTLCD_MAXSCREENS];
	int contrast[TEXTLCD_MAXSCREENS], brightness[TEXTLCD_MAXSCREENS];
	
	int cursorLocation[TEXTLCD_MAXSCREENS], cursorColumn[TEXTLCD_MAXSCREENS];

	unsigned char backlightEcho[TEXTLCD_MAXSCREENS];
	int contrastEcho[TEXTLCD_MAXSCREENS], brightnessEcho[TEXTLCD_MAXSCREENS];

	unsigned char fullStateEcho;

	//used for network sets
	char *customs[TEXTLCD_MAXSCREENS][16];
	char chars[TEXTLCD_MAXSCREENS][TEXTLCD_MAXROWS * TEXTLCD_MAXCOLS];
	char *strings[TEXTLCD_MAXSCREENS][TEXTLCD_MAXROWS];

	//For TextLCD Adapter
	int rowCount[TEXTLCD_MAXSCREENS], columnCount[TEXTLCD_MAXSCREENS];
	CPhidgetTextLCD_ScreenSize screenSize[TEXTLCD_MAXSCREENS];

	unsigned char init[TEXTLCD_MAXSCREENS];

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;
} typedef CPhidgetTextLCDInfo;
#endif

/** @} */

#endif
