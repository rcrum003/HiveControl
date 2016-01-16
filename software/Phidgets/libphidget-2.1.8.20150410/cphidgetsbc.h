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

#ifndef __CPHIDGETSBC
#define __CPHIDGETSBC
#include "cphidget.h"
#include "cphidgetmanager.h"

DPHANDLE(SBC)
DPHANDLE(SBCManager)

#ifndef EXTERNALPROTO

typedef struct _CPhidgetSBC
{
	CPhidgetRemoteHandle networkInfo;
	int txtver;
	char fversion[13];
	short hversion;
	char mac[18];
	char hostname[129];
	char deviceName[255];
} CPhidgetSBC;

typedef struct _CPhidgetSBCList
{
	struct _CPhidgetSBCList *next;
	CPhidgetSBC *sbc;
} CPhidgetSBCList, *CPhidgetSBCListHandle;

typedef struct _CPhidgetSBCManager
{
	int mdns;					//true if mdns, false if not
	int(CCONV *fptrError)(CPhidgetSBCManagerHandle , void *, int, const char *);
	void *fptrErrorptr;
	CPhidgetManagerState state;
	int (CCONV *fptrAttachChange)(CPhidgetSBCHandle sbc, void *userPtr);
	void *fptrAttachChangeptr;
	int (CCONV *fptrDetachChange)(CPhidgetSBCHandle sbc, void *userPtr);
	void *fptrDetachChangeptr;
} CPhidgetSBCManager;

typedef struct _CPhidgetSBCManagerList
{
	struct _CPhidgetSBCManagerList *next;
	CPhidgetSBCManager *sbcm;
} CPhidgetSBCManagerList, *CPhidgetSBCManagerListHandle;

int CCONV CPhidgetSBC_areEqual(void *arg1, void *arg2);
int CCONV CPhidgetSBC_areExtraEqual(void *arg1, void *arg2);
void CCONV CPhidgetSBC_free(void *arg);
int CCONV CPhidgetSBC_create(CPhidgetSBCHandle *sbc);
int CCONV CPhidgetSBC_delete(CPhidgetSBCHandle sbc);

#endif

PHIDGET21_API int CCONV CPhidgetSBCManager_create(CPhidgetSBCManagerHandle *sbcm);
PHIDGET21_API int CCONV CPhidgetSBCManager_start(CPhidgetSBCManagerHandle sbcm);
PHIDGET21_API int CCONV CPhidgetSBCManager_stop(CPhidgetSBCManagerHandle sbcm);
PHIDGET21_API int CCONV CPhidgetSBCManager_delete(CPhidgetSBCManagerHandle sbcm);
PHIDGET21_API int CCONV CPhidgetSBCManager_set_OnAttach_Handler(CPhidgetSBCManagerHandle sbcm, int (CCONV *fptr)(CPhidgetSBCHandle sbc, void *userPtr), void *userPtr);
PHIDGET21_API int CCONV CPhidgetSBCManager_set_OnDetach_Handler(CPhidgetSBCManagerHandle sbcm, int (CCONV *fptr)(CPhidgetSBCHandle sbc, void *userPtr), void *userPtr);
PHIDGET21_API int CCONV CPhidgetSBCManager_getAttachedSBCs(CPhidgetSBCManagerHandle sbcm, CPhidgetSBCHandle *sbcArray[], int *count);
PHIDGET21_API int CCONV CPhidgetSBCManager_set_OnError_Handler(CPhidgetSBCManagerHandle sbcm, int(CCONV *fptr)(CPhidgetSBCManagerHandle sbcm, void *userPtr, int errorCode, const char *errorString), void *userPtr);

PHIDGET21_API int CCONV CPhidgetSBC_getFirmwareVersion(CPhidgetSBCHandle sbc, const char **firmwareVersion);
PHIDGET21_API int CCONV CPhidgetSBC_getHardwareVersion(CPhidgetSBCHandle sbc, int *hardwareVersion);
PHIDGET21_API int CCONV CPhidgetSBC_getMacAddress(CPhidgetSBCHandle sbc, const char **macAddress);
PHIDGET21_API int CCONV CPhidgetSBC_getAddress(CPhidgetSBCHandle sbc, const char **ipAddr);
PHIDGET21_API int CCONV CPhidgetSBC_getHostname(CPhidgetSBCHandle sbc, const char **hostname);
PHIDGET21_API int CCONV CPhidgetSBC_getDeviceName(CPhidgetSBCHandle sbc, const char **name);

#endif

