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
#include "cphidgetmanager.h"
#include "cphidget.h"
#include "cthread.h"
#include "cusb.h"
#include "cphidgetlist.h"
#include "csocket.h"

/* A list of local Phidget Managers */
CPhidgetManagerListHandle localPhidgetManagers = NULL;
int ActivePhidgetManagers = 0;

/* Protects localPhidgetManagers */
int managerLockInitialized = PFALSE;
CThread_mutex_t managerLock;

int CPhidgetManager_areEqual(void *arg1, void *arg2)
{
	if(arg1 == arg2) return 1;
	return 0;
}

void CPhidgetManager_free(void *arg)
{
	CPhidgetManagerHandle phidm = (CPhidgetManagerHandle)arg;
	if(!phidm) return;

	if(CPhidget_statusFlagIsSet(phidm->status, PHIDGET_REMOTE_FLAG))
	{
		CList_emptyList((CListHandle *)&phidm->AttachedPhidgets, PTRUE, CPhidget_free);
	}

	CThread_mutex_destroy(&phidm->lock);
	CThread_mutex_destroy(&phidm->openCloseLock);

	free(phidm); phidm = NULL;
	return;
}

int CCONV CPhidgetManager_set_OnError_Handler(CPhidgetManagerHandle phidm, 
	int(CCONV *fptr)(CPhidgetManagerHandle, void *, int, const char *), void *userPtr)
{
	TESTPTR(phidm)
	phidm->fptrError = fptr;
	phidm->fptrErrorptr = userPtr;
	return EPHIDGET_OK;
}

/* The internal event handlers for PhidgetManager - these should be called directly from the usb functions...
 * these run in the context of the central thread
 * if AddDevice returns EPHIDGET_DUPLICATE, the user callback is not run (good)
 */
int CPhidgetAttachEvent(CPhidgetHandle phid) {
	int result = 0;
	CPhidgetManagerList *trav2 = 0;
	TESTPTR(phid)
	result = CList_addToList((CListHandle *)&AttachedDevices, phid, CPhidget_areEqual);
	if(result == EPHIDGET_DUPLICATE)
	{
		return EPHIDGET_OK;
	}
	else if(result) 
	{
		return result;
	}
	
	for(trav2 = localPhidgetManagers; trav2; trav2 = trav2->next)
	{
		if (trav2->phidm->fptrAttachChange && trav2->phidm->state == PHIDGETMANAGER_ACTIVE)
		{
			//So we can access AttachedDevices from within the manager atach event
			CThread_mutex_unlock(&attachedDevicesLock);
			trav2->phidm->fptrAttachChange((CPhidgetHandle)phid, trav2->phidm->fptrAttachChangeptr);
			CThread_mutex_lock(&attachedDevicesLock);
		}
	}

	return findActiveDevice(phid);
}

int CPhidgetDetachEvent(CPhidgetHandle phid) {
	int result = 0;
	CPhidgetList *trav = 0;
	CPhidgetManagerList *trav2 = 0;
	CPhidgetHandle travPhid = 0;
	TESTPTR(phid)
	CPhidget_clearStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, NULL);
	CPhidget_setStatusFlag(&phid->status, PHIDGET_DETACHING_FLAG, NULL);

	CList_removeFromList((CListHandle *)&AttachedDevices, phid, CPhidget_areExtraEqual, FALSE, NULL);
	for(trav2 = localPhidgetManagers; trav2; trav2 = trav2->next)
	{
		if (trav2->phidm->fptrDetachChange && trav2->phidm->state == PHIDGETMANAGER_ACTIVE)
		{
			//So we can access AttachedDevices from within the manager atach event
			CThread_mutex_unlock(&attachedDevicesLock);
			trav2->phidm->fptrDetachChange((CPhidgetHandle)phid, trav2->phidm->fptrDetachChangeptr);
			CThread_mutex_lock(&attachedDevicesLock);
		}
	}
	CPhidget_clearStatusFlag(&phid->status, PHIDGET_DETACHING_FLAG, NULL);

	CThread_mutex_lock(&activeDevicesLock);
	for (trav=ActiveDevices; trav; trav = trav->next)
	{	
		if((CPhidget_areExtraEqual(phid, trav->phid) && CPhidget_statusFlagIsSet(trav->phid->status, PHIDGET_ATTACHED_FLAG)) 
			|| CPhidgetHandle_areEqual(phid, trav->phid))
		{
			CPhidget_setStatusFlag(&trav->phid->status, PHIDGET_DETACHING_FLAG, &trav->phid->lock);
			if(trav->phid->specificDevice == PHIDGETOPEN_ANY_ATTACHED)
				trav->phid->specificDevice = PHIDGETOPEN_ANY;

			trav->phid->writeStopFlag = PTRUE;
			CThread_set_event(&trav->phid->writeAvailableEvent); //if it's waiting on this event - signal NOW

			result = CUSBCloseHandle(trav->phid);
			CThread_join(&trav->phid->writeThread);
			CThread_join(&trav->phid->readThread);

			//because trav can be freed during the detach call, don't use it in or after the call
			travPhid = trav->phid;
			CThread_mutex_unlock(&activeDevicesLock);
			if (travPhid->fptrDetach)
				travPhid->fptrDetach((CPhidgetHandle)travPhid, travPhid->fptrDetachptr);

			travPhid->deviceIDSpec = 0;
			travPhid->deviceUID = 0;
			
#if !defined(_MACOSX) && !defined(WINCE)
			CPhidgetFHandle_free(travPhid->CPhidgetFHandle);
			travPhid->CPhidgetFHandle = NULL;
#endif
			
			CPhidget_clearStatusFlag(&travPhid->status, PHIDGET_DETACHING_FLAG, &travPhid->lock);
			CPhidget_clearStatusFlag(&travPhid->status, PHIDGET_USB_ERROR_FLAG, &travPhid->lock);
			goto found_to_detach;
		}

	}
	CThread_mutex_unlock(&activeDevicesLock);

found_to_detach:
	CPhidget_free(phid);

	return result;
}

// sends out any initial attach events for new managers
static int sendInitialEvents()
{
	CPhidgetManagerList *managerList = 0;
	CPhidgetList *phidList = 0;
	
	for(managerList = localPhidgetManagers; managerList; managerList = managerList->next)
	{
		if (managerList->phidm->state == PHIDGETMANAGER_ACTIVATING)
		{
			managerList->phidm->state = PHIDGETMANAGER_ACTIVE;
			
			if (managerList->phidm->fptrAttachChange)
			{
				for (phidList=AttachedDevices; phidList; phidList = phidList->next) 
				{
					//So we can access AttachedDevices from within the manager atach event
					CThread_mutex_unlock(&attachedDevicesLock);
					managerList->phidm->fptrAttachChange((CPhidgetHandle)(phidList->phid), managerList->phidm->fptrAttachChangeptr);
					CThread_mutex_lock(&attachedDevicesLock);
				}
			}
		}
	}

	return EPHIDGET_OK;
}

int CPhidgetManager_poll()
{
	CPhidgetList *curList = 0, *detachList = 0;
	CPhidgetList *trav = 0, *trav2 = 0;
	CPhidgetHandle foundPhidget;
			
	if(!managerLockInitialized)
	{
		CThread_mutex_init(&managerLock);
		managerLockInitialized = PTRUE;
	}
	
	CThread_mutex_lock(&managerLock);
	CThread_mutex_lock(&attachedDevicesLock);

	sendInitialEvents();

	CUSBBuildList(&curList);


	for (trav=AttachedDevices; trav; trav = trav->next)
	{		
		if(CList_findInList((CListHandle)curList, trav->phid, CPhidget_areExtraEqual, NULL) == EPHIDGET_NOTFOUND)
		{
			CList_addToList((CListHandle *)&detachList, trav->phid, CPhidget_areEqual);
		}
	}
	for (trav=curList; trav; trav = trav->next)
	{
		if(CList_findInList((CListHandle)AttachedDevices, trav->phid, CPhidget_areExtraEqual, NULL) == EPHIDGET_NOTFOUND)
		{
			CPhidgetAttachEvent(trav->phid);
		}
		
		//if PHIDGET_USB_ERROR_FLAG is set, cycle device through a detach
		//if it's ok, it will re-attach
		
		CThread_mutex_lock(&activeDevicesLock);
		if(CList_findInList((CListHandle)ActiveDevices, trav->phid, CPhidget_areEqual, (void **)&foundPhidget) == EPHIDGET_OK)
		{		
			if(CPhidget_statusFlagIsSet(foundPhidget->status, PHIDGET_ATTACHED_FLAG)
				&& CPhidget_statusFlagIsSet(foundPhidget->status, PHIDGET_USB_ERROR_FLAG))
			{
				LOG(PHIDGET_LOG_WARNING,"PHIDGET_USB_ERROR_FLAG is set - cycling device through a detach");
				CList_addToList((CListHandle *)&detachList, trav->phid, CPhidget_areEqual);
				
				//if this is a composite device, we must find it's pair and detach that as well.
				//same serial, but different interface num
				for (trav2=curList; trav2; trav2 = trav2->next)
				{
					if(trav->phid->serialNumber == trav2->phid->serialNumber
					&& trav->phid->deviceDef->pdd_iid != trav2->phid->deviceDef->pdd_iid)
					{
						LOG(PHIDGET_LOG_WARNING,"PHIDGET_USB_ERROR_FLAG is set - cycling composite device 2nd interface through a detach");
						CList_addToList((CListHandle *)&detachList, trav2->phid, CPhidget_areEqual);
					}
				}
			}
		}
		
		
		CThread_mutex_unlock(&activeDevicesLock);
	}
	for (trav=detachList; trav; trav = trav->next)
	{
		CPhidgetDetachEvent(trav->phid);
	}
	CList_emptyList((CListHandle *)&detachList, FALSE, NULL);
	CList_emptyList((CListHandle *)&curList, FALSE, NULL);

	CThread_mutex_unlock(&attachedDevicesLock);
	CThread_mutex_unlock(&managerLock);
	return EPHIDGET_OK;
}

int CCONV CPhidgetManager_create(CPhidgetManagerHandle *phidm)
{
	CPhidgetManagerHandle phidmtemp = 0;
	
	TESTPTR(phidm)
	if(!(phidmtemp = (CPhidgetManagerHandle)malloc(sizeof(CPhidgetManager))))
		return EPHIDGET_NOMEMORY;
	ZEROMEM(phidmtemp, sizeof(CPhidgetManager));
	
	phidmtemp->state = PHIDGETMANAGER_INACTIVE;
	
	if(!managerLockInitialized)
	{
		CThread_mutex_init(&managerLock);
		managerLockInitialized = PTRUE;
	}

	CThread_mutex_init(&phidmtemp->lock);
	CThread_mutex_init(&phidmtemp->openCloseLock);
	
	*phidm = phidmtemp;
	return EPHIDGET_OK;
}

int CCONV CPhidgetManager_close(CPhidgetManagerHandle phidm)
{
	TESTPTR(phidm)

	CThread_mutex_lock(&phidm->openCloseLock);
	if (!CPhidget_statusFlagIsSet(phidm->status, PHIDGET_OPENED_FLAG))
	{
		LOG(PHIDGET_LOG_WARNING, "Close was called on an already closed Manager handle.");
		CThread_mutex_unlock(&phidm->openCloseLock);
		return EPHIDGET_OK;
	}

	if(phidm->state == PHIDGETMANAGER_ACTIVE || phidm->state == PHIDGETMANAGER_ACTIVATING)
	{
		phidm->state = PHIDGETMANAGER_INACTIVE;
		CPhidget_clearStatusFlag(&phidm->status, PHIDGET_ATTACHED_FLAG, &phidm->lock);
		if(CPhidget_statusFlagIsSet(phidm->status, PHIDGET_REMOTE_FLAG))
		{
			//Only free the list phidgets if this is an openRemoteIP - not for openRemote
			int freeList = phidm->networkInfo->mdns ? PFALSE : PTRUE;
			unregisterRemoteManager(phidm);

			CList_emptyList((CListHandle *)&phidm->AttachedPhidgets, freeList, CPhidget_free);
		}
		else
		{
			CThread_mutex_lock(&managerLock);
			ActivePhidgetManagers--;
			CList_removeFromList((CListHandle *)&localPhidgetManagers, phidm, CPhidgetManager_areEqual, PFALSE, NULL);
			CThread_mutex_unlock(&managerLock);
		}
	}
	
	//if there are no more active phidgets or managers, wait for the central thread to exit
	if(!ActiveDevices && !ActivePhidgetManagers)
	{
		JoinCentralThread();
		//Shut down USB
#if defined(_LINUX) && !defined(_ANDROID)
		CUSBUninit();
#endif
	}

	CPhidget_clearStatusFlag(&phidm->status, PHIDGET_OPENED_FLAG, &phidm->lock);
	CThread_mutex_unlock(&phidm->openCloseLock);
	return EPHIDGET_OK;
}

int CCONV CPhidgetManager_delete(CPhidgetManagerHandle phidm)
{
	CPhidgetManager_free(phidm);
	return EPHIDGET_OK;
}

int CCONV CPhidgetManager_getServerID(CPhidgetManagerHandle phidm, const char **serverID)
{
	return  CPhidget_getServerID((CPhidgetHandle)phidm, serverID);
}
int CCONV CPhidgetManager_getServerAddress(CPhidgetManagerHandle phidm, const char **address, int *port)
{
	return  CPhidget_getServerAddress((CPhidgetHandle)phidm, address, port);
}
int CCONV CPhidgetManager_getServerStatus(CPhidgetManagerHandle phidm, int *status)
{
	return CPhidget_getServerStatus((CPhidgetHandle)phidm, status);
}

int CCONV CPhidgetManager_open(CPhidgetManagerHandle phidm)
{
	int result = EPHIDGET_OK;
	
	TESTPTR(phidm)
	
#if defined(_IPHONE)
	return EPHIDGET_UNSUPPORTED;
#endif
	
#if defined(_ANDROID)
	if(!ANDROID_USB_GOOD)
		return EPHIDGET_UNSUPPORTED;
#endif
	
	CThread_mutex_lock(&phidm->openCloseLock);
	if (CPhidget_statusFlagIsSet(phidm->status, PHIDGET_OPENED_FLAG))
	{
		LOG(PHIDGET_LOG_WARNING, "Open was called on an already opened Manager handle.");
		CThread_mutex_unlock(&phidm->openCloseLock);
		return EPHIDGET_OK;
	}

	if(!phidgetLocksInitialized)
	{
		CThread_mutex_init(&activeDevicesLock);
		CThread_mutex_init(&attachedDevicesLock);
		phidgetLocksInitialized = PTRUE;
	}

	if(phidm->state == PHIDGETMANAGER_INACTIVE)
	{
		CThread_mutex_lock(&managerLock);
	
		CList_addToList((CListHandle *)&localPhidgetManagers, phidm, CPhidgetManager_areEqual);
		
#ifdef _MACOSX
		phidm->state = PHIDGETMANAGER_ACTIVE;
#else
		phidm->state = PHIDGETMANAGER_ACTIVATING;
#endif
		
		CPhidget_setStatusFlag(&phidm->status, PHIDGET_ATTACHED_FLAG, &phidm->lock);

		ActivePhidgetManagers++;

		CThread_mutex_unlock(&managerLock);

		result = StartCentralThread();
	}

	CPhidget_setStatusFlag(&phidm->status, PHIDGET_OPENED_FLAG, &phidm->lock);
	CThread_mutex_unlock(&phidm->openCloseLock);
	return result;
}

int CCONV CPhidgetManager_set_OnAttach_Handler(CPhidgetManagerHandle phidm, int (CCONV *fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr)
{
	TESTPTR(phidm)
	phidm->fptrAttachChange = fptr; 
	phidm->fptrAttachChangeptr = userPtr; 
	return EPHIDGET_OK; 
}
int CCONV CPhidgetManager_set_OnDetach_Handler(CPhidgetManagerHandle phidm, int (CCONV *fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr)
{
	TESTPTR(phidm)
	phidm->fptrDetachChange = fptr; 
	phidm->fptrDetachChangeptr = userPtr; 
	return EPHIDGET_OK; 
}
int CCONV CPhidgetManager_set_OnServerConnect_Handler(CPhidgetManagerHandle phidm, int (CCONV *fptr)(CPhidgetManagerHandle phidm, void *userPtr), void *userPtr)
{
	TESTPTR(phidm)
	phidm->fptrServerConnect = fptr; 
	phidm->fptrServerConnectptr = userPtr; 
	return EPHIDGET_OK; 
}
int CCONV CPhidgetManager_set_OnServerDisconnect_Handler(CPhidgetManagerHandle phidm, int (CCONV *fptr)(CPhidgetManagerHandle phidm, void *userPtr), void *userPtr)
{
	TESTPTR(phidm)
	phidm->fptrServerDisconnect = fptr; 
	phidm->fptrServerDisconnectptr = userPtr; 
	return EPHIDGET_OK; 
}

int CCONV CPhidgetManager_freeAttachedDevicesArray(CPhidgetHandle phidArray[])
{
	if(!phidArray) return EPHIDGET_OK;
	free(phidArray); phidArray = NULL;
	return EPHIDGET_OK;
}

int CCONV CPhidgetManager_getAttachedDevices(CPhidgetManagerHandle phidm, CPhidgetHandle *phidArray[], int *count)
{
	CPhidgetList *trav = 0;
	int i = 0;

	TESTPTRS(phidArray, count)
	TESTPTR(phidm)

	*count = 0;
	if(CPhidget_statusFlagIsSet(phidm->status, PHIDGET_REMOTE_FLAG))
	{		
		for (trav=phidm->AttachedPhidgets; trav; trav = trav->next)
		{
			if (CPhidget_statusFlagIsSet(trav->phid->status, PHIDGET_ATTACHED_FLAG))
				(*count)++;
		}
		
		if(*count==0)
		{
			*phidArray = NULL;
		}
		else
		{
			*phidArray = (CPhidgetHandle *)malloc(sizeof(**phidArray) * *count);
			if (!*phidArray)
				return EPHIDGET_NOMEMORY;
			ZEROMEM(*phidArray, sizeof(**phidArray) * *count);

			for (trav=phidm->AttachedPhidgets, i=0; trav; trav = trav->next)
			{
				if (CPhidget_statusFlagIsSet(trav->phid->status, PHIDGET_ATTACHED_FLAG))
				{
					(*phidArray)[i] = trav->phid;
					i++;
				}
			}
		}
	}
	else
	{
		CThread_mutex_lock(&attachedDevicesLock);
		for (trav=AttachedDevices; trav; trav = trav->next)
		{
			if (CPhidget_statusFlagIsSet(trav->phid->status, PHIDGET_ATTACHED_FLAG))
				(*count)++;
		}
		if(*count==0)
		{
			*phidArray = NULL;
		}
		else
		{
			*phidArray = (CPhidgetHandle *)malloc(sizeof(**phidArray) * *count);
			if (!*phidArray)
			{
				CThread_mutex_unlock(&attachedDevicesLock);
				return EPHIDGET_NOMEMORY;
			}
			ZEROMEM(*phidArray, sizeof(**phidArray) * *count);

			for (trav=AttachedDevices, i=0; trav; trav = trav->next)
			{
				if (CPhidget_statusFlagIsSet(trav->phid->status, PHIDGET_ATTACHED_FLAG))
				{
					(*phidArray)[i] = trav->phid;
					i++;
				}
			}
		}

		CThread_mutex_unlock(&attachedDevicesLock);
	}
	return EPHIDGET_OK; 
}
