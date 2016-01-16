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

#ifndef __CPHIDGETMANAGER
#define __CPHIDGETMANAGER
#include "cphidget.h"

/** \defgroup phidmanager Phidget Manager 
 * These calls are specific to the Phidget Manager. The Phidget Manager allows enumeration of all connected devices and notification of device
 * attach and detach events.
 * @{
 */

/**
 * A Phidget Manager handle.
 */
DPHANDLE(Manager)

#ifndef EXTERNALPROTO

typedef enum
{
	PHIDGETMANAGER_INACTIVE = 1,
	PHIDGETMANAGER_ACTIVE,
	PHIDGETMANAGER_ACTIVATING
} CPhidgetManagerState;

typedef struct _CPhidgetManager
{
	CPhidgetRemoteHandle networkInfo;
	int(CCONV *fptrError)(CPhidgetManagerHandle , void *, int, const char *);
	void *fptrErrorptr;
	int(CCONV *fptrServerConnect)(CPhidgetManagerHandle , void *);
	void *fptrServerConnectptr; 
	int(CCONV *fptrServerDisconnect)(CPhidgetManagerHandle , void *);
	void *fptrServerDisconnectptr;
	CThread_mutex_t lock; /* protects status */
	int status;
	CThread_mutex_t openCloseLock; /* protects open / close */
	CPhidgetManagerState state;
	CPhidgetList *AttachedPhidgets;
	int (CCONV *fptrAttachChange)(CPhidgetHandle phid, void *userPtr);
	void *fptrAttachChangeptr;
	int (CCONV *fptrDetachChange)(CPhidgetHandle phid, void *userPtr);
	void *fptrDetachChangeptr;
} CPhidgetManager;

typedef struct _CPhidgetManagerList
{
	struct _CPhidgetManagerList *next;
	CPhidgetManager *phidm;
} CPhidgetManagerList, *CPhidgetManagerListHandle;

extern CPhidgetManagerListHandle localPhidgetManagers;
extern int ActivePhidgetManagers;

int CPhidgetManager_poll();
int CPhidgetAttachEvent(CPhidgetHandle phid);
int CPhidgetDetachEvent(CPhidgetHandle phid);

int CPhidgetManager_areEqual(void *arg1, void *arg2);
void CPhidgetManager_free(void *arg);
#endif

/**
 * Creates a Phidget Manager handle.
 * @param phidm A pointer to an empty phidget manager handle.
 */
PHIDGET21_API int CCONV CPhidgetManager_create(CPhidgetManagerHandle *phidm);
/**
 * Opens a Phidget Manager.
 * @param phidm A phidget manager handle.
 */
PHIDGET21_API int CCONV CPhidgetManager_open(CPhidgetManagerHandle phidm);
/**
 * Closes a Phidget Manager.
 * @param phidm An opened phidget manager handle.
 */
PHIDGET21_API int CCONV CPhidgetManager_close(CPhidgetManagerHandle phidm);
/**
 * Frees a Phidget Manager handle.
 * @param phidm A closed phidget manager handle.
 */
PHIDGET21_API int CCONV CPhidgetManager_delete(CPhidgetManagerHandle phidm);
/**
 * Sets an attach handler callback function. This is called when a Phidget is plugged into the system.
 * @param phidm A phidget manager handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidgetManager_set_OnAttach_Handler(CPhidgetManagerHandle phidm, int (CCONV *fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr);
/**
 * Sets a detach handler callback function. This is called when a Phidget is unplugged from the system.
 * @param phidm A phidget manager handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidgetManager_set_OnDetach_Handler(CPhidgetManagerHandle phidm, int (CCONV *fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr);
/**
 * Gets a list of all currently attached Phidgets. When you are finished with the list, free it with CPhidgetManager_freeAttachedDevicesArray.
 * @param phidm An opened phidget manager handle.
 * @param phidArray An empty pointer for returning the list of Phidgets. Note that this list is created internally, you don't need to pass in a array.
 * @param count An int pointer for returning the list size
 */
PHIDGET21_API int CCONV CPhidgetManager_getAttachedDevices(CPhidgetManagerHandle phidm, CPhidgetHandle *phidArray[], int *count);
/**
 * Frees the array that is allocated when CPhidgetManager_getAttachedDevices is called. Since the array is malloced internally to the library, it
 * should also be freed internally to the library.
 * @param phidArray An array of CPhidgetHandles.
 */
PHIDGET21_API int CCONV CPhidgetManager_freeAttachedDevicesArray(CPhidgetHandle phidArray[]);
/**
 * Sets the error handler callback function. This is called when an asynchronous error occurs.
 * @param phidm A phidget manager handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidgetManager_set_OnError_Handler(CPhidgetManagerHandle phidm, int(CCONV *fptr)(CPhidgetManagerHandle phidm, void *userPtr, int errorCode, const char *errorString), void *userPtr);
/**
 * Sets a server connect handler callback function. This is used for opening Phidget Managers remotely, and is called when a connection to the sever has been made.
 * @param phidm A phidget manager handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidgetManager_set_OnServerConnect_Handler(CPhidgetManagerHandle phidm, int (CCONV *fptr)(CPhidgetManagerHandle phidm, void *userPtr), void *userPtr);
/**
 * Sets a server disconnect handler callback function. This is used for opening Phidget Managers remotely, and is called when a connection to the server has been lost.
 * @param phidm A phidget manager handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidgetManager_set_OnServerDisconnect_Handler(CPhidgetManagerHandle phidm, int (CCONV *fptr)(CPhidgetManagerHandle phidm, void *userPtr), void *userPtr);
/**
 * Gets the server ID of a remotely opened Phidget Manager. This will fail if the manager was opened locally.
 * @param phidm A connected phidget manager handle.
 * @param serverID A pointer which will be set to a char array containing the server ID string.
 */
PHIDGET21_API int CCONV CPhidgetManager_getServerID(CPhidgetManagerHandle phidm, const char **serverID);
/**
 * Gets the address and port of a remotely opened Phidget Manager. This will fail if the manager was opened locally.
 * @param phidm A connected phidget manager handle.
 * @param address A pointer which will be set to a char array containing the address string.
 * @param port An int pointer for returning the port number.
 */
PHIDGET21_API int CCONV CPhidgetManager_getServerAddress(CPhidgetManagerHandle phidm, const char **address, int *port);
/**
 * Gets the connected to server status of a remotely opened Phidget Manager. This will fail if the manager was opened locally.
 * @param phidm An opened phidget manager handle.
 * @param serverStatus An int pointer for returning the server status. Possible codes are \ref PHIDGET_ATTACHED and \ref PHIDGET_NOTATTACHED.
 */
PHIDGET21_API int CCONV CPhidgetManager_getServerStatus(CPhidgetManagerHandle phidm, int *serverStatus);

/** @} */

#endif

