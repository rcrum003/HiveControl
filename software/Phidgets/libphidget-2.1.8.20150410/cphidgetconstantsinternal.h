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

#ifndef CPHIDGET_CONSTANTS_INTERNAL
#define CPHIDGET_CONSTANTS_INTERNAL

//internal status representation via flags - for Phidget, Manager, Dictionary
/*
 * PHIDGET_ATTACHED_FLAG	
 *	true:	Phidget is attached to computer, and you can call all commands on this handle
 *	false:	Phidget is not ready to handle calls yet - might be not connected, 
 *			in proccess of connecting, just disconnected, not initialized, etc.
 *	This flag is returend externally to users.
 */
#define PHIDGET_ATTACHED_FLAG			0x01
/*
 * PHIDGET_DETACHING_FLAG	
 *	true:	Phidget has just been detached, but the detach code is still running
 *	false:	Any other time
 *	There are two types of funtions - those that communicate with devices and those that
 *	return cached info - the second class should succedd if this is true and PHIDGET_ATTACHED_FLAG is false,
 *	so we can get device info in the detach handler.
 */
#define PHIDGET_DETACHING_FLAG			0x02
/*
 * PHIDGET_USB_ERROR_FLAG	
 *	true:	Phidget has encountered a USB error bad enough to require a detach
 *	false:	Any other time
 *	The base thread will notice this error code and detach the device. If it is ok, it will
 *  be picked up next iteration and an attach will occur.
 */
#define PHIDGET_USB_ERROR_FLAG			0x04
/*
 * PHIDGET_ATTACHING_FLAG	
 *	true:	Phidget is attached to computer, but initialization is still in progress
 *	false:	any other time
 *	This is used internally only
 */
#define PHIDGET_ATTACHING_FLAG			0x08
/*
 * PHIDGET_OPENED_FLAG	
 *	true:	open has been called on the handle
 *	false:	open has not yet been called, or close has been called.
 *	This is used internally only
 */
#define PHIDGET_OPENED_FLAG				0x10
/*
 * PHIDGET_SERVER_CONNECTED_FLAG	
 *	true:	networkInfo->server is valid and connected
 *	false:	networkInfo should not be considered valid
 *	This is used internally only
 */
#define PHIDGET_SERVER_CONNECTED_FLAG	0x20
/*
 * PHIDGET_REMOTE_FLAG
 *	true:	This was opened with _remote and networkInfo is valid
 *	false:	networkInfo is not valid - not opened or opened non-remote
 *	This is used internally only
 */
#define PHIDGET_REMOTE_FLAG				0x40
/*
 * PHIDGET_INERROREVENT_FLAG	
 *	true:	an error event is being run
 *	false:	not
 *	This is used internally only
 */
#define PHIDGET_INERROREVENT_FLAG		0x80


//internal status representation via flags - for socket
/*
 * PHIDGETSOCKET_CONNECTED_FLAG	
 *	true:	Socket is connected, authorization succeeded and listeners and reports are set up.
 *	false:	Socket is not ready
 *	This flag is returned externally to users.
 */
#define PHIDGETSOCKET_CONNECTED_FLAG		0x01
/*
 * PHIDGETSOCKET_CONNECTING_FLAG	
 *	true:	connecting is being set up
 *	false:	not
 *	This leads to either PHIDGETSOCKET_CONNECTED_FLAG, PHIDGETSOCKET_AUTHERROR_FLAG, or PHIDGETSOCKET_CONNECTIONERROR_FLAG
 *	This is used internally only
 */
#define PHIDGETSOCKET_CONNECTING_FLAG		0x02
/*
 * PHIDGETSOCKET_CONNECTIONERROR_FLAG	
 *	true:	a connection (socket) error occured
 *	false:	a socket error did not occur
 *	This could happen any time, generally, we got a bad response from a socket, or a timeout
 *	This is used internally only
 */
#define PHIDGETSOCKET_CONNECTIONERROR_FLAG	0x04
/*
 * PHIDGETSOCKET_AUTHERROR_FLAG	
 *	true:	authorization with server failed
 *	false:	authorization didn't fail (yet)
 *	This is a password error
 *	This is used internally only
 */
#define PHIDGETSOCKET_AUTHERROR_FLAG		0x08


/* Internal error codes - start at 0x8000 */
#define EPHIDGET_TRYAGAIN	0x8000

/* 10 characters UTF-8, so up to 4 bytes per character */
#define MAX_LABEL_SIZE					10
#define MAX_LABEL_STORAGE				MAX_LABEL_SIZE*4+1

#define MAX_IN_PACKET_SIZE				64
#define MAX_OUT_PACKET_SIZE				64

#define PHIDGETOPEN_ANY				0x00
#define PHIDGETOPEN_SERIAL			0x01
#define PHIDGETOPEN_ANY_ATTACHED	0x02
#define PHIDGETOPEN_LABEL			0x04

/* Uninitialized values - for webservice */
#define PUNI_BOOL	0x03
#define PUNI_INT	0x7FFFFFFE
#define PUNI_INT64	0x7FFFFFFFFFFFFFFELL
#define PUNI_DBL	1e250
#define PUNI_FLT	1e25


// this needs to reflect the size of Phid_DeviceName
#define PHIDGET_DEVICE_CLASS_COUNT		24
// this needs to reflect the size of Phid_Device_Def (not including the null end)
#define PHIDGET_DEVICE_COUNT			55

#endif

