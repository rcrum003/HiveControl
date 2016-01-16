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
#include "cphidgettextlcd.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

/*
	Protocol Documentation

	buffer[7] = packet type and screen number
		Top 3 bits are screen number. [SSSCCCCC]
		0x01 - 0x07 = LCD command (data count)
		0x08 - 0x1F = Space for extra packet types:
		0x11 = Backlight, Brightness
			buffer[0] = 0x00 for off, 0x01 for on
			buffer[1] = 0x00 - 0xff (variable brightness where supported)
		0x12 = Contrast
			buffer[0] = 0x00 - 0xff
		0x13 = Init
			re-initializes a display - this can fix a display that's gone wonky, or bring up a display plugged in after power-up
		anything else is ignored

	HD44780-based Character-LCD
	Documentation -		http://home.iae.nl/users/pouweha/lcd/lcd0.shtml
						http://www.doc.ic.ac.uk/~ih/doc/lcd/index.html

	buffer[0-6] = LCD command / data
	So sending a packet with any arbitrary set of LCD commands in 0-6 and the command length in 7 is how it works.
	You can combine any number of commands / data in one packet - up to 7 bytes.
		LCD commands - special characters:
			0x00 = escape the next character (can escape 0x00, 0x01, 0x02)
			0x01 = following is commands (clears RS)
			0x02 = following is data (sets RS)

	Always leave in command mode (0x01) when you finish

	So, we can send any command / data, but we can not read back anything (busy flag, CGRAM, DDRAM)

	On our 2x20 display:
		Display Data (DDRAM): Row 0 address 0x00-0x13, Row 1 address 0x40-0x53
		Custom characters (CGRAM): 0x08-0x15 - don't use 0x00-0x07 because 0x00 will terminate displaystring early

*/


// === Internal Functions === //

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(TextLCD)
	int i;

	for (i = 0; i<TEXTLCD_MAXSCREENS; i++)
	{
		//set data arrays to unknown
		phid->cursorBlink[i] = PUNK_BOOL;
		phid->cursorOn[i] = PUNK_BOOL;
		phid->contrast[i] = PUNK_INT;
		phid->backlight[i] = PUNK_BOOL;
		phid->brightness[i] = PUNK_INT;
		phid->contrastEcho[i] = PUNI_INT;
		phid->backlightEcho[i] = PUNI_BOOL;
		phid->brightnessEcho[i] = PUNI_INT;
		phid->init[i] = 0;
		phid->screenSize[i] = -1;
		phid->rowCount[i] = PUNI_INT;
		phid->columnCount[i] = PUNI_INT;
		phid->cursorLocation[i] = 0;
		phid->cursorColumn[i] = 0;
	}

	phid->currentScreen = 0;
	phid->lastScreen = 0;

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(TextLCD)
	int i;
	unsigned char buffer[8] = {0};
	int ret = EPHIDGET_OK;

	TESTPTR(phid);

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEXTLCD_2x20:
		case PHIDID_TEXTLCD_2x20_CUSTOM:
		case PHIDID_TEXTLCD_2x20_w_0_8_8:
		case PHIDID_TEXTLCD_2x20_w_8_8_8:
			for (i = 0; i<phid->phid.attr.textlcd.numScreens; i++)
			{
				//set data arrays to unknown
				phid->contrastEcho[i] = PUNK_INT;
				phid->backlightEcho[i] = PUNK_BOOL;
				phid->brightnessEcho[i] = PUNK_INT;
				phid->screenSize[i] = PHIDGET_TEXTLCD_SCREEN_2x20;
				phid->rowCount[i] = phid->phid.attr.textlcd.numRows;
				phid->columnCount[i] = phid->phid.attr.textlcd.numColumns;
				phid->cursorLocation[i] = 0;
				phid->cursorColumn[i] = 0;
			}
			break;
		case PHIDID_TEXTLCD_ADAPTER:
			for (i = 0; i<phid->phid.attr.textlcd.numScreens; i++)
			{
				//set data arrays to unknown
				phid->contrastEcho[i] = PUNK_INT;
				phid->backlightEcho[i] = PUNK_BOOL;
				phid->brightnessEcho[i] = PUNK_INT;
				phid->screenSize[i] = PHIDGET_TEXTLCD_SCREEN_UNKNOWN;
				phid->rowCount[i] = 0;
				phid->columnCount[i] = 0;
				phid->cursorLocation[i] = 0;
				phid->cursorColumn[i] = 0;
			}
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//Make sure no old writes are still pending
	phid->outputPacketLen = 0;

	phid->currentScreen = 0;
	phid->lastScreen = 0;

	phid->fullStateEcho = PFALSE;

	//Device specific stuff
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEXTLCD_ADAPTER:
			phid->fullStateEcho = PTRUE;
			break;
		case PHIDID_TEXTLCD_2x20:
		case PHIDID_TEXTLCD_2x20_CUSTOM:
		case PHIDID_TEXTLCD_2x20_w_0_8_8:
		case PHIDID_TEXTLCD_2x20_w_8_8_8:
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//Make sure the TextLCD is in command mode! Also turn off cursor and cursor blink explicitly
	for (i = 0; i<phid->phid.attr.textlcd.numScreens; i++)
	{
		buffer[0] = TEXTLCD_COMMAND_MODE;
		buffer[1] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON;
		buffer[7] = 2 | TEXTLCD_SCREEN(i);
		if ((ret = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
			return ret;

		phid->cursorBlink[i] = 0;
		phid->cursorOn[i] = 0;
	}

	//Issue a read if device supports state echo
	if(phid->fullStateEcho)
		CPhidget_read((CPhidgetHandle)phid);

	//set everything to it's echo
	for (i = 0; i<phid->phid.attr.textlcd.numScreens; i++)
	{
		phid->contrast[i] = phid->contrastEcho[i];
		phid->backlight[i] = phid->backlightEcho[i];
		phid->brightness[i] = phid->brightnessEcho[i];
	}

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(TextLCD)
	int i;

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEXTLCD_ADAPTER:
			{
				for (i = 0; i<phid->phid.attr.textlcd.numScreens; i++)
				{
					phid->backlightEcho[i] = buffer[i];
					phid->brightnessEcho[i] = buffer[i+2];
					phid->contrastEcho[i] = buffer[i+4];
				}
			}
			break;
		case PHIDID_TEXTLCD_2x20_w_8_8_8:
		case PHIDID_TEXTLCD_2x20:
		case PHIDID_TEXTLCD_2x20_CUSTOM:
		case PHIDID_TEXTLCD_2x20_w_0_8_8:
		default:
			return EPHIDGET_UNEXPECTED;
	}
	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization - not used
CPHIDGETINITEVENTS(TextLCD)
	phid=0;
	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET_BUF(TextLCD)

//sendpacket - sends a packet to the device asynchronously, blocking if the 1-packet queue is full
CSENDPACKET_BUF(TextLCD)

//makePacket - constructs a packet using current device state
CMAKEPACKETINDEXED(TextLCD)
	int screen = phid->currentScreen;

	TESTPTRS(phid, buffer)

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEXTLCD_2x20:
		case PHIDID_TEXTLCD_2x20_CUSTOM:
		case PHIDID_TEXTLCD_2x20_w_0_8_8:
		case PHIDID_TEXTLCD_2x20_w_8_8_8:
		case PHIDID_TEXTLCD_ADAPTER:
			switch(Index)
			{
				case TEXTLCD_BACKLIGHT_PACKET: //backlight

					//if unknown then this must have been called from setBrightness - they still need to enable the backlight
					if (phid->backlight[screen] == PUNK_BOOL)
					{
						phid->backlight[screen] = PFALSE;
						if(!phid->fullStateEcho)
							phid->backlightEcho[screen] = phid->backlight[screen];
					}

					buffer[0] = phid->backlight[screen];

					//Brightness where supported
					switch(phid->phid.deviceIDSpec)
					{
						case PHIDID_TEXTLCD_2x20_w_8_8_8:
							if (phid->phid.deviceVersion < 200)
								break;
						case PHIDID_TEXTLCD_ADAPTER:
							//default the brightness to full
							if (phid->brightness[screen] == PUNK_INT)
							{
								phid->brightness[screen] = 255;
								if(!phid->fullStateEcho)
									phid->brightnessEcho[screen] = phid->brightness[screen];
							}
							buffer[1] = phid->brightness[screen];
						default:
							break;
					}

					buffer[7] = TEXTLCD_BACKLIGHT_PACKET | TEXTLCD_SCREEN(screen);

					break;
				case TEXTLCD_CONTRAST_PACKET: //contrast
					buffer[0] = (unsigned char)phid->contrast[screen];
					buffer[7] = TEXTLCD_CONTRAST_PACKET | TEXTLCD_SCREEN(screen);

					break;
				case TEXTLCD_INIT_PACKET: //re-init a screen
					if(phid->screenSize[screen] == PHIDGET_TEXTLCD_SCREEN_4x40 && screen == 0)
					{
						buffer[7] = TEXTLCD_INIT_PACKET | TEXTLCD_SCREEN(2); //screen '2' represents both screens 0 and 1, but ONLY for initialization!!
						phid->cursorOn[1] = PFALSE;
						phid->cursorBlink[1] = PFALSE;
						phid->cursorColumn[1] = 0;
						phid->cursorLocation[1] = 0;
					}
					else
						buffer[7] = TEXTLCD_INIT_PACKET | TEXTLCD_SCREEN(screen);

					phid->lastScreen = 0;
					phid->cursorOn[screen] = PFALSE;
					phid->cursorBlink[screen] = PFALSE;
					phid->cursorColumn[screen] = 0;
					phid->cursorLocation[screen] = 0;

					break;
				case TEXTLCD_CURSOR_PACKET: //LCD commands - Cursor

					if (phid->cursorOn[screen] == PUNK_BOOL)
					{
						phid->cursorOn[screen] = PFALSE;
					}

					if (phid->cursorBlink[screen] == PUNK_BOOL)
					{
						phid->cursorBlink[screen] = PFALSE;
					}

					buffer[0] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON;
					//only actually turn on if the cursor is on-display
					if((phid->screenSize[0] == PHIDGET_TEXTLCD_SCREEN_4x40 && screen == 0 && phid->cursorColumn[phid->lastScreen] < phid->columnCount[screen])
						|| (phid->cursorColumn[screen] < phid->columnCount[screen]))
					{
						if (phid->cursorOn[screen])
							buffer[0] |= HD44780_CURSOR_ON;
						if (phid->cursorBlink[screen])
							buffer[0] |= HD44780_CURSOR_BLINK_ON;
					}

					//Special case - 4x40, rows 2 and 3
					if(phid->screenSize[0] == PHIDGET_TEXTLCD_SCREEN_4x40 && screen == 0)
						buffer[7] = 1 | TEXTLCD_SCREEN(phid->lastScreen);
					else
						buffer[7] = 1 | TEXTLCD_SCREEN(screen);

					break;
				default:
					return EPHIDGET_UNEXPECTED;
			}
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	return EPHIDGET_OK;
}

/* deals with moving the cursor/blink between the 'screens' on a 4x40 setup 
 * make sure this is called from within a write lock
 */
//static int dealWithCursor(CPhidgetTextLCDHandle phid, int screen)
//{
//	unsigned char buffer[8] = {0};
//	int ret = EPHIDGET_OK;
//
//	//Switching between screens for special 4x40 case
//	if(phid->screenSize[0] == PHIDGET_TEXTLCD_SCREEN_4x40)
//	{
//		if(phid->cursorOn[0] == PTRUE || phid->cursorBlink[0] == PTRUE)
//		{
//			if(phid->lastScreen != screen)
//			{
//				buffer[0] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON;	
//				buffer[7] = 1 | TEXTLCD_SCREEN(phid->lastScreen);
//
//				if ((ret = CPhidgetTextLCD_sendpacket(phid, buffer)) != EPHIDGET_OK)
//					return ret;
//
//				buffer[0] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON;
//				if (phid->cursorOn[0])
//					buffer[0] |= HD44780_CURSOR_ON;
//				if (phid->cursorBlink[0])
//					buffer[0] |= HD44780_CURSOR_BLINK_ON;
//				buffer[7] = 1 | TEXTLCD_SCREEN(screen);
//
//				if ((ret = CPhidgetTextLCD_sendpacket(phid, buffer)) != EPHIDGET_OK)
//					return ret;
//			}
//		}
//	}
//
//	phid->lastScreen = screen;
//	return ret;
//}

/*
 * Returns DDRAM position for the requested row and col
 * May change screen number for the special 4x40 case
 */
static int getScreenAndPos(CPhidgetTextLCDHandle phid, int *screen, int row, int col)
{
	int cols = phid->columnCount[*screen];
	int pos = 0;

	switch(row)
	{
		case 0:
			pos = 0x00;
			break;
		case 1:
			pos = 0x40;
			break;
		case 2:
			if(cols == 16)
			{
				pos = 0x10;
			}
			else if(cols == 40)
			{
				pos = 0x00;
				*screen = 1;
			}
			else	
			{
				pos = 0x14;
			}
			break;
		case 3:
			if(cols == 16)
			{
				pos = 0x50;
			}
			else if(cols == 40)
			{
				pos = 0x40;
				*screen = 1;
			}
			else	
			{
				pos = 0x54;
			}
			break;
	}

	return pos+col;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(TextLCD, PHIDCLASS_TEXTLCD)

CGET(TextLCD,RowCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED

	MASGN(rowCount[phid->currentScreen])
}

CGET(TextLCD,ColumnCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED

	MASGN(columnCount[phid->currentScreen])
}

CGET(TextLCD,ScreenCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED

	MASGN(phid.attr.textlcd.numScreens)
}

CGET(TextLCD,Backlight,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	TESTMASGN(backlightEcho[phid->currentScreen], PUNK_BOOL)

	MASGN(backlightEcho[phid->currentScreen])
}
CSET(TextLCD,Backlight,int)
	int Index;
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	TESTRANGE(PFALSE, PTRUE)

	Index = phid->currentScreen;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Backlight, "%d", backlight);
	else
	{
		SENDPACKETINDEXED(TextLCD, backlight[Index], TEXTLCD_BACKLIGHT_PACKET);

		if(!phid->fullStateEcho)
			phid->backlightEcho[Index] = phid->backlight[Index];
	}

	return EPHIDGET_OK;
}

CGET(TextLCD,Brightness,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEXTLCD_2x20_w_8_8_8:
			if (phid->phid.deviceVersion < 200)
				return EPHIDGET_UNSUPPORTED;
			break;
		case PHIDID_TEXTLCD_ADAPTER:
			break;
		case PHIDID_TEXTLCD_2x20:
		case PHIDID_TEXTLCD_2x20_CUSTOM:
		case PHIDID_TEXTLCD_2x20_w_0_8_8:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	TESTMASGN(brightnessEcho[phid->currentScreen], PUNK_INT)

	MASGN(brightnessEcho[phid->currentScreen])
}
CSET(TextLCD,Brightness,int)
	int Index;
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED

	Index = phid->currentScreen;
	
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEXTLCD_2x20_w_8_8_8:
			if (phid->phid.deviceVersion < 200)
				return EPHIDGET_UNSUPPORTED;
			break;
		case PHIDID_TEXTLCD_ADAPTER:
			break;
		case PHIDID_TEXTLCD_2x20:
		case PHIDID_TEXTLCD_2x20_CUSTOM:
		case PHIDID_TEXTLCD_2x20_w_0_8_8:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	TESTRANGE(0, 255)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Brightness, "%d", brightness);
	else
	{
		SENDPACKETINDEXED(TextLCD, brightness[Index], TEXTLCD_BACKLIGHT_PACKET);

		if(!phid->fullStateEcho)
			phid->brightnessEcho[Index] = phid->brightness[Index];
	}

	return EPHIDGET_OK;
}

CGET(TextLCD,Contrast,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	TESTMASGN(contrastEcho[phid->currentScreen], PUNK_INT)

	MASGN(contrastEcho[phid->currentScreen])
}
CSET(TextLCD,Contrast,int)
	int Index;
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	TESTRANGE(0, 255)

	Index = phid->currentScreen;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Contrast, "%d", contrast);
	else
	{
		SENDPACKETINDEXED(TextLCD, contrast[Index], TEXTLCD_CONTRAST_PACKET);

		if(!phid->fullStateEcho)
			phid->contrastEcho[Index] = phid->contrast[Index];
	}

	return EPHIDGET_OK;
}

CGET(TextLCD,CursorOn,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	TESTMASGN(cursorOn[phid->currentScreen], PUNK_BOOL)

	MASGN(cursorOn[phid->currentScreen])
}
CSET(TextLCD,CursorOn,int)
	int Index;
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	TESTRANGE(PFALSE, PTRUE)

	Index = phid->currentScreen;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(CursorOn, "%d", cursorOn);
	else
		SENDPACKETINDEXED(TextLCD, cursorOn[Index], TEXTLCD_CURSOR_PACKET);

	return EPHIDGET_OK;
}

CGET(TextLCD,CursorBlink,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	TESTMASGN(cursorBlink[phid->currentScreen], PUNK_BOOL)

	MASGN(cursorBlink[phid->currentScreen])
}
CSET(TextLCD,CursorBlink,int)
	int Index;
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	TESTRANGE(PFALSE, PTRUE)

	Index = phid->currentScreen;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(CursorBlink, "%d", cursorBlink);
	else
		SENDPACKETINDEXED(TextLCD, cursorBlink[Index], TEXTLCD_CURSOR_PACKET);

	return EPHIDGET_OK;
}

CGET(TextLCD,Screen,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED

	MASGN(currentScreen)
}
CSET(TextLCD,Screen,int)
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	TESTRANGE(0, phid->phid.attr.textlcd.numScreens)

	//This prevents a screen size change during a multi-packet write
	CThread_mutex_lock(&phid->phid.writelock);
	phid->currentScreen = newVal;
	CThread_mutex_unlock(&phid->phid.writelock);

	return EPHIDGET_OK;
}

CGET(TextLCD,ScreenSize,CPhidgetTextLCD_ScreenSize)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED
	TESTMASGN(screenSize[phid->currentScreen], PHIDGET_TEXTLCD_SCREEN_UNKNOWN)

	MASGN(screenSize[phid->currentScreen])
}
CSET(TextLCD,ScreenSize,CPhidgetTextLCD_ScreenSize)
	int otherScreen;
	int screen;

	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED

	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEXTLCD_ADAPTER:
			break;
		case PHIDID_TEXTLCD_2x20_w_8_8_8:
		case PHIDID_TEXTLCD_2x20:
		case PHIDID_TEXTLCD_2x20_CUSTOM:
		case PHIDID_TEXTLCD_2x20_w_0_8_8:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	TESTRANGE(PHIDGET_TEXTLCD_SCREEN_NONE, PHIDGET_TEXTLCD_SCREEN_4x40)

	screen = phid->currentScreen;

	otherScreen = 1;
	if(screen == 1)
		otherScreen = 0;

	switch(newVal)
	{
		case PHIDGET_TEXTLCD_SCREEN_NONE:
			phid->rowCount[screen] = 0;
			phid->columnCount[screen] = 0;
			break;
		case PHIDGET_TEXTLCD_SCREEN_1x8:
			phid->rowCount[screen] = 1;
			phid->columnCount[screen] = 8;
			break;
		case PHIDGET_TEXTLCD_SCREEN_2x8:
			phid->rowCount[screen] = 2;
			phid->columnCount[screen] = 8;
			break;
		case PHIDGET_TEXTLCD_SCREEN_1x16:
			phid->rowCount[screen] = 1;
			phid->columnCount[screen] = 16;
			break;
		case PHIDGET_TEXTLCD_SCREEN_2x16:
			phid->rowCount[screen] = 2;
			phid->columnCount[screen] = 16;
			break;
		case PHIDGET_TEXTLCD_SCREEN_4x16:
			phid->rowCount[screen] = 4;
			phid->columnCount[screen] = 16;
			break;
		case PHIDGET_TEXTLCD_SCREEN_2x20:
			phid->rowCount[screen] = 2;
			phid->columnCount[screen] = 20;
			break;
		case PHIDGET_TEXTLCD_SCREEN_4x20:
			phid->rowCount[screen] = 4;
			phid->columnCount[screen] = 20;
			break;
		case PHIDGET_TEXTLCD_SCREEN_2x24:
			phid->rowCount[screen] = 2;
			phid->columnCount[screen] = 24;
			break;
		case PHIDGET_TEXTLCD_SCREEN_1x40:
			phid->rowCount[screen] = 1;
			phid->columnCount[screen] = 40;
			break;
		case PHIDGET_TEXTLCD_SCREEN_2x40:
			phid->rowCount[screen] = 2;
			phid->columnCount[screen] = 40;
			break;
		case PHIDGET_TEXTLCD_SCREEN_4x40:
			//Only supported on screen 0
			if(screen != 0)
				return EPHIDGET_UNSUPPORTED;

			phid->rowCount[screen] = 4;
			phid->columnCount[screen] = 40;

			//sets other screen to none
			phid->rowCount[otherScreen] = 0;
			phid->columnCount[otherScreen] = 0;
			//set screen size
			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
			{
				int newVal = PHIDGET_TEXTLCD_SCREEN_NONE;
				int Index = otherScreen;
				ADDNETWORKKEYINDEXED(ScreenSize, "%d", screenSize);
			}
			else
			{
				phid->screenSize[otherScreen] = PHIDGET_TEXTLCD_SCREEN_NONE;
			}
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}
	
	phid->lastScreen = 0;

	//can't have a 4x40 with anything other then NONE
	if(phid->screenSize[otherScreen] == PHIDGET_TEXTLCD_SCREEN_4x40 && newVal != PHIDGET_TEXTLCD_SCREEN_NONE)
	{
		//sets other screen to none
		phid->rowCount[otherScreen] = 0;
		phid->columnCount[otherScreen] = 0;
		//set screen size
		if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		{
			int newVal = PHIDGET_TEXTLCD_SCREEN_NONE;
			int Index = otherScreen;
			ADDNETWORKKEYINDEXED(ScreenSize, "%d", screenSize);
		}
		else
		{
			phid->screenSize[otherScreen] = PHIDGET_TEXTLCD_SCREEN_NONE;
		}
	}
	
	//set screen size
	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		int Index = screen;
		ADDNETWORKKEYINDEXED(ScreenSize, "%d", screenSize);
	}
	else
	{
		phid->screenSize[screen] = newVal;
	}
	
	return EPHIDGET_OK;
}


PHIDGET21_API int CCONV CPhidgetTextLCD_setDisplayCharacter (CPhidgetTextLCDHandle phid, int Row, int Column, unsigned char Character)
{
	unsigned char buffer[8] = {0};
	int ret = EPHIDGET_OK;
	int screen, screenEnable, pos, buf_ptr;

	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED

	screen = screenEnable = phid->currentScreen;

	if (Row >= phid->rowCount[screen] || Row < 0) return EPHIDGET_OUTOFBOUNDS;
	if (Column >= phid->columnCount[screen] || Column < 0) return EPHIDGET_OUTOFBOUNDS;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		char newVal = Character;
		//int Index = (Row + 1) * (Column + 1);
		int Index = (Column << 16) + (Row << 8) + screen;
		ADDNETWORKKEYINDEXED(DisplayCharacter, "%c", chars[screen]);
	}
	else
	{
		switch(phid->phid.deviceIDSpec)
		{
			case PHIDID_TEXTLCD_2x20:
			case PHIDID_TEXTLCD_2x20_CUSTOM:
			case PHIDID_TEXTLCD_2x20_w_0_8_8:
			case PHIDID_TEXTLCD_2x20_w_8_8_8:
			case PHIDID_TEXTLCD_ADAPTER:

				CThread_mutex_lock(&phid->phid.writelock);

				pos = getScreenAndPos(phid, &screenEnable, Row, Column);
				phid->cursorLocation[screenEnable] = pos + 1;
				phid->cursorColumn[screenEnable] = Column + 1;
				buf_ptr = 0;
				//turn off cursor if it's on
				if(phid->cursorOn[screen] == PTRUE || phid->cursorBlink[screen] == PTRUE)
				{
					if(phid->screenSize[screen] == PHIDGET_TEXTLCD_SCREEN_4x40 && screen == 0 && screenEnable != phid->lastScreen)
					{
						//4x40 and the cursor is on on the other screen, need to turn it off in it's own packet
						buffer[0] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON;	
						buffer[7] = 1 | TEXTLCD_SCREEN(phid->lastScreen);

						if ((ret = CPhidgetTextLCD_sendpacket(phid, buffer)) != EPHIDGET_OK)
							goto done;
					}
					else
						buffer[buf_ptr++] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON; /* disable cursors */
				}

				buffer[buf_ptr++] = pos | HD44780_SET_DDRAM_ADDR; /* Address */
				buffer[buf_ptr++] = TEXTLCD_DATA_MODE;
				//escape if needed
				if(Character == 0x00 || Character == 0x01 || Character == 0x02)
					buffer[buf_ptr++] = TEXTLCD_ESCAPE_CHAR;
				buffer[buf_ptr++] = Character;
				buffer[buf_ptr++] = TEXTLCD_COMMAND_MODE;	//always leave in command mode
				//turn cursor back on if we turned it off
				if(phid->cursorOn[screen] == PTRUE || phid->cursorBlink[screen] == PTRUE)
				{
					if(phid->cursorColumn[screenEnable] < phid->columnCount[screen])
					{
						buffer[buf_ptr] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON; /* enable cursors */
						if(phid->screenSize[0] == PHIDGET_TEXTLCD_SCREEN_4x40)
						{
							if (phid->cursorOn[0])
								buffer[buf_ptr] |= HD44780_CURSOR_ON;
							if (phid->cursorBlink[0])
								buffer[buf_ptr] |= HD44780_CURSOR_BLINK_ON;
						}
						else
						{
							if (phid->cursorOn[screenEnable])
								buffer[buf_ptr] |= HD44780_CURSOR_ON;
							if (phid->cursorBlink[screenEnable])
								buffer[buf_ptr] |= HD44780_CURSOR_BLINK_ON;
						}
						buf_ptr++;
					}
				}
				//don't need to set DDRAM position it's already right
				//at most, we're sending 7 bytes, so it's safe for one packet
				buffer[7] = buf_ptr | TEXTLCD_SCREEN(screenEnable);

				if((ret = CPhidgetTextLCD_sendpacket(phid, buffer)) != EPHIDGET_OK)
					goto done;

				//so we know what screen we last wrote to
				phid->lastScreen = screenEnable;
done:
				CThread_mutex_unlock(&phid->phid.writelock);
				break;
			default:
				return EPHIDGET_UNEXPECTED;
		}
	}
	return ret;
}

PHIDGET21_API int CCONV CPhidgetTextLCD_setCustomCharacter (CPhidgetTextLCDHandle phid, int Index, int Val1, int Val2)
{
	unsigned char buffer[8] = {0};
	int ret = EPHIDGET_OK;
	char newVal[50];
	int screen;

	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED

	screen = phid->currentScreen;

	if ((Index < 8) || (Index > 15)) return EPHIDGET_INVALIDARG;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		Index = (Index << 8) + screen;
		sprintf(newVal, "%d,%d", Val1, Val2);
		ADDNETWORKKEYINDEXED(CustomCharacter, "%s", customs[screen]);
	}
	else
	{
		switch(phid->phid.deviceIDSpec)
		{
			case PHIDID_TEXTLCD_2x20:
			case PHIDID_TEXTLCD_2x20_CUSTOM:
			case PHIDID_TEXTLCD_2x20_w_0_8_8:
			case PHIDID_TEXTLCD_2x20_w_8_8_8:
			case PHIDID_TEXTLCD_ADAPTER:

				//actual index is 0-7
				Index -= 8;

				CThread_mutex_lock(&phid->phid.writelock);
again:
				//buffer[0] = TEXTLCD_COMMAND_MODE;

				buffer[0] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON; /* disable cursors */
				buffer[1] = HD44780_SET_CGRAM_ADDR | TEXTLCD_CGRAM_ADDR(Index); /* set CGRAM address */
				buffer[2] = TEXTLCD_DATA_MODE;
				buffer[3] = (Val1 & 0x1F) | 0x80;
				buffer[4] = ((Val1 >> 5) & 0x1F) | 0x80;
				buffer[5] = ((Val1 >> 10) & 0x1F) | 0x80;
				buffer[6] = ((Val1 >> 15) & 0x1F) | 0x80;
				buffer[7] = 7 | TEXTLCD_SCREEN(screen);

				if ((ret = CPhidgetTextLCD_sendpacket(phid, buffer)) != EPHIDGET_OK)
					goto done;

				buffer[0] = (Val2 & 0x1F) | 0x80;
				buffer[1] = ((Val2 >> 5) & 0x1F) | 0x80;
				buffer[2] = ((Val2 >> 10) & 0x1F) | 0x80;
				buffer[3] = ((Val2 >> 15) & 0x1F) | 0x80;
				buffer[4] = TEXTLCD_COMMAND_MODE;
				buffer[5] = phid->cursorLocation[screen] | HD44780_SET_DDRAM_ADDR; /* reset DDRAM address for cursors */
				buffer[6] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON; /* enable cursors */
				if(phid->screenSize[0] == PHIDGET_TEXTLCD_SCREEN_4x40 && phid->cursorColumn[screen] < phid->columnCount[0])
				{
					// for 4x40, only re-enable a cursor if we're on the last written to screen
					if(screen == phid->lastScreen)
					{
						if (phid->cursorOn[0])
							buffer[6] |= HD44780_CURSOR_ON;
						if (phid->cursorBlink[0])
							buffer[6] |= HD44780_CURSOR_BLINK_ON;
					}
				}
				else if(phid->cursorColumn[screen] < phid->columnCount[screen])
				{
					if (phid->cursorOn[screen])
						buffer[6] |= HD44780_CURSOR_ON;
					if (phid->cursorBlink[screen])
						buffer[6] |= HD44780_CURSOR_BLINK_ON;
				}
				buffer[7] = 7 | TEXTLCD_SCREEN(screen);

				if ((ret = CPhidgetTextLCD_sendpacket(phid, buffer)) != EPHIDGET_OK)
					goto done;

				/* for 4x40, we need to set these characters on screen 1 as well */
				if(phid->screenSize[0] == PHIDGET_TEXTLCD_SCREEN_4x40 && screen==0)
				{
					screen = 1;
					goto again;
				}
done:
				CThread_mutex_unlock(&phid->phid.writelock);
				break;
			default:
				return EPHIDGET_UNEXPECTED;
		}
	}

	return ret;
}

//0x00 is the escape character
//escape 0x01 and 0x02 because these are interpreted as special commands by the firmware
//we don't have to worry about 0x00 in the input string because that would end the string.
static void escapeLcdString(char *string, char *buffer, int *buf_ptr)
{
	unsigned int ui;
	size_t len = strlen(string);

	//Escape 0x01, 0x02
	for (ui = 0; ui<len; ui++)
	{
		if ((string[ui] == 0x01) || (string[ui] == 0x02))
			buffer[(*buf_ptr)++] = TEXTLCD_ESCAPE_CHAR;
		buffer[(*buf_ptr)++] = string[ui];
	}
}

CSETINDEX(TextLCD, DisplayString, char *)
	int i, j, buf_ptr, ret = EPHIDGET_OK;
	size_t len;
	unsigned char form_buffer[250] = {0};
	unsigned char buffer[8] = {0};
	int screen, screenEnable, pos;

	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED

	len = strlen(newVal);
	screen = screenEnable = phid->currentScreen;

	TESTINDEX(rowCount[screen])

	if (len > (size_t)phid->columnCount[screen]) return EPHIDGET_INVALIDARG;

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		Index = (Index << 8) + screen;
		ADDNETWORKKEYINDEXED(DisplayString, "%s", strings[screen]);
	}
	else
	{
		switch(phid->phid.deviceIDSpec)
		{
			case PHIDID_TEXTLCD_2x20:
			case PHIDID_TEXTLCD_2x20_CUSTOM:
			case PHIDID_TEXTLCD_2x20_w_0_8_8:
			case PHIDID_TEXTLCD_2x20_w_8_8_8:
			case PHIDID_TEXTLCD_ADAPTER:
				CThread_mutex_lock(&phid->phid.writelock);

				//DDRAM address of start of selected row
				pos = getScreenAndPos(phid, &screenEnable, Index, 0);
				phid->cursorLocation[screenEnable] = pos + len;
				phid->cursorColumn[screenEnable] = len;
			
				buf_ptr = 0;
				form_buffer[buf_ptr++] = TEXTLCD_COMMAND_MODE; /* Command Mode */

				//turn off cursor if it's on
				if(phid->cursorOn[screen] == PTRUE || phid->cursorBlink[screen] == PTRUE)
				{
					if(phid->screenSize[screen] == PHIDGET_TEXTLCD_SCREEN_4x40 && screen == 0 && screenEnable != phid->lastScreen)
					{
						//4x40 and the cursor is on on the other screen, need to turn it off in it's own packet
						buffer[0] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON;	
						buffer[7] = 1 | TEXTLCD_SCREEN(phid->lastScreen);

						if ((ret = CPhidgetTextLCD_sendpacket(phid, buffer)) != EPHIDGET_OK)
							goto done;
					}
					else
						form_buffer[buf_ptr++] = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON; /* disable cursors */
				}

				form_buffer[buf_ptr++] = pos | HD44780_SET_DDRAM_ADDR; /* Set DDRAM Address */
				form_buffer[buf_ptr++] = TEXTLCD_DATA_MODE; /* Data Mode */

				/* get escaped string */
				escapeLcdString(newVal, (char *)form_buffer, &buf_ptr);

				/* fill rest of the row with spaces */
				for (i = 0; i < (phid->columnCount[screen] - (int)len); i++)
					form_buffer[buf_ptr++] = ' ';

				form_buffer[buf_ptr++] = TEXTLCD_COMMAND_MODE; /* Command Mode */
				form_buffer[buf_ptr++] = phid->cursorLocation[screenEnable] | HD44780_SET_DDRAM_ADDR;  /* Set DDRAM Address so cursor appears in the right place */

				/* re-enable the cursors if we need to */
				if((phid->cursorOn[screen] == PTRUE || phid->cursorBlink[screen] == PTRUE) 
					&& phid->cursorColumn[screenEnable] < phid->columnCount[screen])
				{
					unsigned char cursorData = HD44780_DISPLAY_CNTRL | HD44780_DISPLAY_ON;
					if (phid->cursorOn[screen])
						cursorData |= HD44780_CURSOR_ON;
					if (phid->cursorBlink[screen])
						cursorData |= HD44780_CURSOR_BLINK_ON;
					form_buffer[buf_ptr++] = cursorData;
				}

				/* we have to send the whole form_buffer, 7 bytes at a time */
				for (i = 0; i<buf_ptr; i+=7)
				{
					/* zero out packet */
					ZEROMEM(buffer, 8);

					/* data length for this packet */
					len = ( ((buf_ptr - i) > 7) ? 7 : (buf_ptr - i) );

					/* fill in data */
					for (j = 0; j < (int)len; j++)
						buffer[j] = form_buffer[i + j];

					/* choose screen and data length */
					buffer[7] = (unsigned char)len | TEXTLCD_SCREEN(screenEnable);
					
					/* send to device */
					if ((ret = CPhidgetTextLCD_sendpacket(phid, buffer)) != EPHIDGET_OK)
						goto done;
				}
				
				//so we know what screen we last wrote to
				phid->lastScreen = screenEnable;

done:
				CThread_mutex_unlock(&phid->phid.writelock);
				break;
			default:
				return EPHIDGET_UNEXPECTED;
		}
	}
	return ret;
}

PHIDGET21_API int CCONV CPhidgetTextLCD_initialize(CPhidgetTextLCDHandle phid)
{
	int Index, newVal=1;
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_TEXTLCD)
	TESTATTACHED

	Index = phid->currentScreen;
	
	switch(phid->phid.deviceIDSpec)
	{
		case PHIDID_TEXTLCD_ADAPTER:
			break;
		case PHIDID_TEXTLCD_2x20:
		case PHIDID_TEXTLCD_2x20_CUSTOM:
		case PHIDID_TEXTLCD_2x20_w_0_8_8:
		case PHIDID_TEXTLCD_2x20_w_8_8_8:
			return EPHIDGET_UNSUPPORTED;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
	{
		int newVal = phid->init[Index]^1;
		ADDNETWORKKEYINDEXED(Init, "%d", init);
	}
	else
		SENDPACKETINDEXED(TextLCD, init[Index], TEXTLCD_INIT_PACKET);

	return EPHIDGET_OK;
}

// === Deprecated Functions === //

CGET(TextLCD,NumRows,int)
	return CPhidgetTextLCD_getRowCount(phid, pVal);
}
CGET(TextLCD,NumColumns,int)
	return CPhidgetTextLCD_getColumnCount(phid, pVal);
}
