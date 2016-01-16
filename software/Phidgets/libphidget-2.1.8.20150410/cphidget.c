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
#include "cphidget.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"
#include "cphidgetlist.h"
#include "utils.h"

static int CPhidgetGPP_dataInput(CPhidgetHandle phid, unsigned char *buffer, int length);

int print_debug_messages = FALSE;

void(CCONV *fptrJavaDetachCurrentThread)(void) = NULL;

CPhidgetListHandle ActiveDevices = 0;
CPhidgetListHandle AttachedDevices = 0;

/* Protects ActiveDevices and AttachedDevices */
int phidgetLocksInitialized = PFALSE;
CThread_mutex_t activeDevicesLock;
CThread_mutex_t attachedDevicesLock;

char
translate_bool_to_ascii(char value)
{
	switch (value) {
	case PTRUE:
		return '1';
	case PFALSE:
		return '0';
	}
	return '?';
}

int CCONV phidget_type_to_id(const char *name)
{
	int i;
	for(i=0;i<PHIDGET_DEVICE_CLASS_COUNT;i++)
	{
		if(Phid_DeviceName[i] != NULL)
		{
			if(!strcmp(Phid_DeviceName[i],name))
				return i;
		}
	}
	return -1;
}

//returns ms
double timestampdiff(CPhidget_Timestamp time1, CPhidget_Timestamp time2)
{
	return ((time1.seconds * 1000) + (double)(time1.microseconds / 1000.0)) - ((time2.seconds * 1000) + (double)(time2.microseconds / 1000.0));
}

double timeSince(TIME *start)
{
#ifdef _WINDOWS
	TIME now;
	FILETIME nowft, oldft, resft;
	ULARGE_INTEGER nowul, oldul, resul;
	double duration;
	GetSystemTime(&now);
	SystemTimeToFileTime(&now, &nowft);
	SystemTimeToFileTime(start, &oldft);

	nowul.HighPart = nowft.dwHighDateTime;
	nowul.LowPart = nowft.dwLowDateTime;
	oldul.HighPart = oldft.dwHighDateTime;
	oldul.LowPart = oldft.dwLowDateTime;

	resul.HighPart = nowul.HighPart - oldul.HighPart;
	resul.LowPart = nowul.LowPart - oldul.LowPart;

	resft.dwHighDateTime = resul.HighPart;
	resft.dwLowDateTime = resul.LowPart;

	duration = (double)(resft.dwLowDateTime/10000000.0);
#else
	struct timeval now;
	gettimeofday(&now, NULL);
	double duration = (now.tv_sec - start->tv_sec) + (double)((now.tv_usec-start->tv_usec)/1000000.0);
#endif
	return duration;
}

void setTimeNow(TIME *now)
{
	#ifdef _WINDOWS
		GetSystemTime(now);
	#else
		gettimeofday(now,NULL);
	#endif
}

/* The is a void pointer that gets malloced only of some platforms */
void CPhidgetFHandle_free(void *arg)
{
	#if defined(_MACOSX) || defined(WINCE)
	return;
	#else
	free(arg); arg = NULL;
	return;
	#endif
}

//Two different Phidget Handles that refer to the same Physical Phidget
int CCONV CPhidget_areEqual(void *arg1, void *arg2)
{
	CPhidgetHandle phid1 = (CPhidgetHandle)arg1;
	CPhidgetHandle phid2 = (CPhidgetHandle)arg2;
	
	if(!phid1 || !phid2)
		return PFALSE;

	// If they are not the same device class
	if(phid2->deviceID != phid1->deviceID)
		return PFALSE;

	// If they both have a device id, but they are different
	if(phid1->deviceIDSpec && phid2->deviceIDSpec && (phid2->deviceIDSpec != phid1->deviceIDSpec))
		return PFALSE;

	// Neither device is marked as PHIDGETOPEN_ANY
	if(phid1->specificDevice && phid2->specificDevice)
	{
		// If one is open serial and the other is open label
		if((phid1->specificDevice == PHIDGETOPEN_SERIAL && phid2->specificDevice == PHIDGETOPEN_LABEL) ||
			(phid1->specificDevice == PHIDGETOPEN_LABEL && phid2->specificDevice == PHIDGETOPEN_SERIAL))
			return PFALSE;

		// If one is open serial but they have different serials
		if((phid1->specificDevice == PHIDGETOPEN_SERIAL || phid2->specificDevice == PHIDGETOPEN_SERIAL) &&
			phid2->serialNumber != phid1->serialNumber)
			return PFALSE;

		// If one is open label but they have different labels
		if((phid1->specificDevice == PHIDGETOPEN_LABEL || phid2->specificDevice == PHIDGETOPEN_LABEL) &&
			strncmp(phid1->label, phid2->label, MAX_LABEL_STORAGE-1))
			return PFALSE;
	}
		
	return PTRUE;

	//OLD
	/*if(
		(
		(phid1->specificDevice == 0 || phid2->specificDevice == 0)?1:(phid2->serialNumber == phid1->serialNumber))
		&&
		(
		(phid1->deviceIDSpec == 0 || phid2->deviceIDSpec == 0)?1:(phid2->deviceIDSpec == phid1->deviceIDSpec)
		) &&
		(phid2->deviceID == phid1->deviceID))
		return 1;
	return 0;*/
}

//Two different Phidget Handles that refer to the same Physical Phidget - more stringent
// Same serial, device class and device id
int CCONV CPhidget_areExtraEqual(void *arg1, void *arg2)
{
	CPhidgetHandle phid1 = (CPhidgetHandle)arg1;
	CPhidgetHandle phid2 = (CPhidgetHandle)arg2;
	
	if(!phid1 || !phid2)
		return PFALSE;
		
	if(
		(phid2->serialNumber == phid1->serialNumber) &&
		(phid2->deviceIDSpec == phid1->deviceIDSpec) &&
		(phid2->deviceID == phid1->deviceID)
	   )
		return PTRUE;
	return PFALSE;
}

//Two identical Phidget Handles (same pointer)
int CCONV CPhidgetHandle_areEqual(void *arg1, void *arg2)
{
	if(arg1 == arg2) return 1;
	return 0;
}

void CPhidgetErrorEvent_free(void *arg)
{
	free(((void **)arg)[0]);
	free(arg);
}

void CCONV CPhidget_free(void *arg)
{
	CPhidgetHandle phid = (CPhidgetHandle)arg;
	if (!phid)
		return;

	//Call device specific free function if it exists
	if(phid->fptrFree)
		phid->fptrFree(phid);

	//this is only malloc-ed on windows and linux, not wince or mac
#if (defined(_WINDOWS) && !defined(WINCE)) || defined(_LINUX) || defined(_ANDROID)
	if (phid->CPhidgetFHandle) {
		CPhidgetFHandle_free(phid->CPhidgetFHandle); phid->CPhidgetFHandle = NULL;
	}
#endif
	
	CThread_mutex_destroy(&phid->lock);
	CThread_mutex_destroy(&phid->openCloseLock);
	CThread_mutex_destroy(&phid->writelock);
	CThread_mutex_destroy(&phid->outputLock);
	CThread_destroy_event(&phid->writeAvailableEvent);
	CThread_destroy_event(&phid->writtenEvent);

	CList_emptyList((CListHandle *)&phid->errEventList, PTRUE, CPhidgetErrorEvent_free);

	free(phid); phid = NULL;
	return;
}

int CCONV CPhidget_create(CPhidgetHandle *phid)
{
	CPhidgetHandle temp_phid;

	TESTPTR(phid)
	
	if(!(temp_phid = malloc(sizeof(CPhidget))))
		return EPHIDGET_NOMEMORY;
	ZEROMEM(temp_phid, sizeof(CPhidget));

	CThread_mutex_init(&temp_phid->lock);
	CThread_mutex_init(&temp_phid->openCloseLock);
	CThread_mutex_init(&temp_phid->writelock);
	CThread_mutex_init(&temp_phid->outputLock);
	CThread_create_event(&temp_phid->writeAvailableEvent);
	CThread_create_event(&temp_phid->writtenEvent);

	CPhidget_clearStatusFlag(&temp_phid->status, PHIDGET_ATTACHED_FLAG, &temp_phid->lock);

	*phid = temp_phid;
	 
	 return EPHIDGET_OK;
}

const char *CPhidget_strerror(int error)
{
	if ((error < 0) || (error >= PHIDGET_ERROR_CODE_COUNT))
		return Phid_UnknownErrorDescription;

	return Phid_ErrorDescriptions[error];
}

int CPhidget_statusFlagIsSet(int status, int flag)
{
	if(status & flag) return PTRUE;
	return PFALSE;
}

//status is a flags variable
int CPhidget_setStatusFlag(int *status, int flag, CThread_mutex_t *lock)
{
	TESTPTR(status)

	if(lock != NULL) CThread_mutex_lock(lock);
	*status |= flag;
	if(lock != NULL) CThread_mutex_unlock(lock);

	return EPHIDGET_OK;
}

int CPhidget_clearStatusFlag(int *status, int flag, CThread_mutex_t *lock)
{
	TESTPTR(status)

	if(lock != NULL) CThread_mutex_lock(lock);
	*status &= (~flag);
	if(lock != NULL) CThread_mutex_unlock(lock);

	return EPHIDGET_OK;
}

int CPhidget_read(CPhidgetHandle phid)
{
	int result = EPHIDGET_OK;

	TESTPTR(phid)

	if (CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG) || CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHING_FLAG))
	{
		if((result = CUSBReadPacket((CPhidgetHandle)phid, phid->lastReadPacket)) != EPHIDGET_OK)
			return result;
		if((phid->lastReadPacket[0] & PHID_USB_GENERAL_PACKET_FLAG) && deviceSupportsGeneralUSBProtocol(phid))
			result = CPhidgetGPP_dataInput(phid, phid->lastReadPacket, phid->inputReportByteLength);
		else if (phid->fptrData)
			result= phid->fptrData(phid, phid->lastReadPacket, phid->inputReportByteLength);
		return result;
	}
	return EPHIDGET_NOTATTACHED;
}


int CPhidget_write(CPhidgetHandle phid)
{
	unsigned char buffer[MAX_OUT_PACKET_SIZE];
	unsigned int len;
	int result = EPHIDGET_OK;
	
	TESTPTR(phid)
	
	ZEROMEM(buffer, sizeof(buffer));
	
	CThread_reset_event(&phid->writeAvailableEvent);
	
	len = MAX_OUT_PACKET_SIZE;
	if ((result = phid->fptrGetPacket((CPhidgetHandle)phid, buffer, &len))
		!= EPHIDGET_OK)
		goto fail;
	// XXX len ignored
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) !=
		EPHIDGET_OK)
		goto fail;
fail:
	/*
	 * under all circumstances (especially failure), signal
	 * waiting writers to loop around and check device status.
	 */
	CThread_set_event(&phid->writtenEvent);
	return result;
}
//Begin exported functions
int CCONV
CPhidget_open(CPhidgetHandle phid, int serialNumber)
{
	int result = 0;
	TESTPTR(phid)
	if (serialNumber < -1)
		return EPHIDGET_INVALIDARG;
	
#if defined(_IPHONE)
	return EPHIDGET_UNSUPPORTED;
#endif
	
#if defined(_ANDROID)
	if(!ANDROID_USB_GOOD)
		return EPHIDGET_UNSUPPORTED;
#endif
	
	CThread_mutex_lock(&phid->openCloseLock);
	if (CPhidget_statusFlagIsSet(phid->status, PHIDGET_OPENED_FLAG))
	{
		LOG(PHIDGET_LOG_WARNING, "Open was called on an already opened Phidget handle.");
		CThread_mutex_unlock(&phid->openCloseLock);
		return EPHIDGET_OK;
	}

	if (serialNumber == -1)
		phid->specificDevice = PHIDGETOPEN_ANY;
	else
		phid->specificDevice = PHIDGETOPEN_SERIAL;
	phid->serialNumber = serialNumber;

	result = RegisterLocalDevice(phid);
	
	CPhidget_setStatusFlag(&phid->status, PHIDGET_OPENED_FLAG, &phid->lock);
	CThread_mutex_unlock(&phid->openCloseLock);

	return result;
}

int CCONV
CPhidget_openLabel(CPhidgetHandle phid, const char *label)
{
	int result = 0;
	TESTPTR(phid)
	
#if defined(_IPHONE)
	return EPHIDGET_UNSUPPORTED;
#endif
	
#if defined(_ANDROID)
	if(!ANDROID_USB_GOOD)
		return EPHIDGET_UNSUPPORTED;
#endif

	if(label != NULL && ((result = encodeLabelString(label, NULL, NULL)) != EPHIDGET_OK))
		return result;

	CThread_mutex_lock(&phid->openCloseLock);
	if (CPhidget_statusFlagIsSet(phid->status, PHIDGET_OPENED_FLAG))
	{
		LOG(PHIDGET_LOG_WARNING, "Open was called on an already opened Phidget handle.");
		CThread_mutex_unlock(&phid->openCloseLock);
		return EPHIDGET_OK;
	}

	if (label == NULL)
		phid->specificDevice = PHIDGETOPEN_ANY;
	else
	{
		phid->specificDevice = PHIDGETOPEN_LABEL;
		memcpy(phid->label, label, strlen(label)+1);
	}

	result = RegisterLocalDevice(phid);
	
	CPhidget_setStatusFlag(&phid->status, PHIDGET_OPENED_FLAG, &phid->lock);
	CThread_mutex_unlock(&phid->openCloseLock);

	return result; 
}

int CCONV
CPhidget_close(CPhidgetHandle phid)
{ 
	int result = EPHIDGET_OK;

	char key[1024], val[6];

	TESTPTR(phid)
	
	CThread_mutex_lock(&phid->openCloseLock);
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_OPENED_FLAG))
	{
		//Could be .NET finalizer - always calls close even if already called.
		LOG(PHIDGET_LOG_INFO, "Close was called on an already closed Phidget handle.");
		CThread_mutex_unlock(&phid->openCloseLock);
		return EPHIDGET_OK;
	}

	//Call device specific close function if it exists
	if(phid->fptrClose)
		phid->fptrClose(phid);

	if(CPhidget_statusFlagIsSet(phid->status, PHIDGET_REMOTE_FLAG))
	{
		CThread_mutex_lock(&phid->lock);
		if(CPhidget_statusFlagIsSet(phid->status, PHIDGET_SERVER_CONNECTED_FLAG))
		{
			struct sockaddr_storage name;
			char addr[200], *a;
			socklen_t namelen = sizeof(name);
			int port, e;
			
			if(getsockname(phid->networkInfo->server->socket, (struct sockaddr *)&name, &namelen) != 0)
			{
#ifndef _WINDOWS
				LOG(PHIDGET_LOG_WARNING,"getsockname: %s", strerror(errno));
#else
				LOG(PHIDGET_LOG_WARNING,"getsockname: WSA Error %d", WSAGetLastError());
#endif
				goto netdone;
			}
			if((e = getnameinfo((struct sockaddr *)&name, namelen, addr, sizeof(addr), NULL, 0, NI_NUMERICHOST)) != 0)
			{
				LOG(PHIDGET_LOG_WARNING,"getnameinfo: %s", gai_strerror(e));
				goto netdone;
			}
			port = (int)((struct sockaddr_in *)&name)->sin_port;
			escape(addr, strlen(addr), &a);

			if(phid->specificDevice == PHIDGETOPEN_SERIAL)
				snprintf(key, sizeof(key), "/PCK/Client/%s/%d%05d/%s/%d", a, phid->networkInfo->uniqueConnectionID, port, Phid_DeviceName[phid->deviceID], phid->serialNumber);
			else if(phid->specificDevice == PHIDGETOPEN_LABEL)
			{
				char *l;
				escape(phid->label, strlen(phid->label), &l);
				snprintf(key, sizeof(key), "/PCK/Client/%s/%d%05d/%s/-1/%s", a, phid->networkInfo->uniqueConnectionID, port, Phid_DeviceName[phid->deviceID], l);
				free(l);
			}
			else
				snprintf(key, sizeof(key), "/PCK/Client/%s/%d%05d/%s", a, phid->networkInfo->uniqueConnectionID, port, Phid_DeviceName[phid->deviceID]);
			snprintf(val, sizeof(val), "Close");
			free(a);
			//don't care about errors on this, so we don't add the error handler - just close already!
			pdc_async_set(phid->networkInfo->server->pdcs, key, val, (int)strlen(val), PTRUE, NULL, NULL); 
		}
netdone:
		CThread_mutex_unlock(&phid->lock);

		result = unregisterRemotePhidget(phid);

		phid->keyCount = 0;
	}
	else
	{
		if(!phidgetLocksInitialized)
		{
			CThread_mutex_init(&activeDevicesLock);
			CThread_mutex_init(&attachedDevicesLock);
			phidgetLocksInitialized = PTRUE;
		}
		CThread_mutex_lock(&activeDevicesLock);
		CList_removeFromList((CListHandle *)&ActiveDevices, phid, CPhidget_areEqual, FALSE, NULL);
		CThread_mutex_unlock(&activeDevicesLock);
		if (CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)) {
			phid->writeStopFlag = PTRUE;
			CThread_join(&phid->writeThread); //join before closing because we want to wait for outstanding writes to complete
			result = CUSBCloseHandle(phid);
			CThread_join(&phid->readThread);
		}
		
		if(phid->specificDevice == PHIDGETOPEN_ANY_ATTACHED)
		{
			phid->specificDevice = PHIDGETOPEN_ANY;
			phid->serialNumber = -1;
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
	}
	CPhidget_clearStatusFlag(&phid->status, PHIDGET_OPENED_FLAG, &phid->lock);

	CThread_mutex_unlock(&phid->openCloseLock);
	return result;
}

//TODO: maybe this should take (CPhidgetHandle *) so that we can nulify the pointer
// (and handle multiple calls to CPhidget_delete)
int CCONV
CPhidget_delete(CPhidgetHandle phid)
{
	CPhidget_free(phid);
	return EPHIDGET_OK;
}

int CCONV
CPhidget_set_OnDetach_Handler(CPhidgetHandle phid,
    int(CCONV *fptr)(CPhidgetHandle, void *), void *userPtr)
{
	TESTPTR(phid)
	phid->fptrDetach = fptr;
	phid->fptrDetachptr = userPtr;
	return EPHIDGET_OK;
}

int CCONV
CPhidget_set_OnAttach_Handler(CPhidgetHandle phid,
    int(CCONV *fptr)(CPhidgetHandle, void *), void *userPtr)
{
	TESTPTR(phid)
	phid->fptrAttach = fptr;
	phid->fptrAttachptr = userPtr;
	return EPHIDGET_OK;
}

int CCONV
CPhidget_set_OnError_Handler(CPhidgetHandle phid,
    int (CCONV *fptr) (CPhidgetHandle, void *, int, const char *), void *userPtr)
{
	TESTPTR(phid)
	phid->fptrError = fptr;
	phid->fptrErrorptr = userPtr;
	return EPHIDGET_OK;
}
int CCONV CPhidget_set_OnServerConnect_Handler(CPhidgetHandle phid, int (CCONV *fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr)
{
	TESTPTR(phid)
	phid->fptrServerConnect = fptr; 
	phid->fptrServerConnectptr = userPtr; 
	return EPHIDGET_OK; 
}
int CCONV CPhidget_set_OnServerDisconnect_Handler(CPhidgetHandle phid, int (CCONV *fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr)
{
	TESTPTR(phid)
	phid->fptrServerDisconnect = fptr; 
	phid->fptrServerDisconnectptr = userPtr; 
	return EPHIDGET_OK; 
}

int CCONV
CPhidget_getDeviceName(CPhidgetHandle phid, const char **buffer)
{
	TESTPTRS(phid, buffer)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
		&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_DETACHING_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(phid->deviceIDSpec == PHIDID_FIRMWARE_UPGRADE)
	{
		if(!phid->firmwareUpgradeName[0])
			snprintf(phid->firmwareUpgradeName, 30, "%s %s", phid->usbProduct, phid->deviceDef->pdd_name);
		*buffer = phid->firmwareUpgradeName;
	}
	else
		*buffer = (char *)phid->deviceDef->pdd_name;
	return EPHIDGET_OK;
}

int CCONV
CPhidget_getSerialNumber(CPhidgetHandle phid, int *serialNumber)
{
	TESTPTRS(phid, serialNumber)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
		&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_DETACHING_FLAG))
		return EPHIDGET_NOTATTACHED;

	*serialNumber = phid->serialNumber;
	return EPHIDGET_OK;
}

int CCONV
CPhidget_getDeviceVersion(CPhidgetHandle phid, int *devVer)
{
	TESTPTRS(phid, devVer)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
		&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_DETACHING_FLAG))
		return EPHIDGET_NOTATTACHED;

	*devVer = phid->deviceVersion;
	return EPHIDGET_OK;
}

/* for now this just returns the attached bit of the status variable - this function should probably be renamed*/
// This CAN be called on closed devices, this should NOT be called internally as it is confusing
int CCONV
CPhidget_getDeviceStatus(CPhidgetHandle phid, int *status)
{
	TESTPTRS(phid, status)

	*status = CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG);
	return EPHIDGET_OK;
}

/* for now this just returns the attached bit of the status variable */
int CCONV
CPhidget_getServerStatus(CPhidgetHandle phid, int *status)
{
	TESTPTRS(phid, status)

	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_REMOTE_FLAG))
		return EPHIDGET_UNSUPPORTED;

	CThread_mutex_lock(&phid->lock);
	if(CPhidget_statusFlagIsSet(phid->status, PHIDGET_SERVER_CONNECTED_FLAG))
		if(phid->networkInfo->server)
			*status = CPhidget_statusFlagIsSet(phid->networkInfo->server->status, PHIDGETSOCKET_CONNECTED_FLAG);
		else
			*status = PFALSE;
	else
		*status = PFALSE;
	CThread_mutex_unlock(&phid->lock);

	return EPHIDGET_OK;
}


int CCONV
CPhidget_getLibraryVersion(const char **buffer)
{
	TESTPTR(buffer)
	*buffer = LibraryVersion;
	return EPHIDGET_OK;
}

int CCONV
CPhidget_getDeviceType(CPhidgetHandle phid, const char **buffer)
{
	TESTPTRS(phid, buffer)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
		&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_DETACHING_FLAG))
		return EPHIDGET_NOTATTACHED;

	*buffer = (char *)Phid_DeviceName[phid->deviceID];
	return EPHIDGET_OK;
}

int CCONV
CPhidget_getDeviceID(CPhidgetHandle phid, CPhidget_DeviceID *deviceID)
{
	TESTPTRS(phid, deviceID)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
		&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_DETACHING_FLAG))
		return EPHIDGET_NOTATTACHED;

	*deviceID = phid->deviceIDSpec;
	return EPHIDGET_OK;
}

int CCONV
CPhidget_getDeviceClass(CPhidgetHandle phid, CPhidget_DeviceClass *deviceClass)
{
	TESTPTRS(phid, deviceClass)
	*deviceClass = phid->deviceID;
	return EPHIDGET_OK;
}

int CCONV
CPhidget_getUSBProductString(CPhidgetHandle phid, const char **buffer)
{
	TESTPTRS(phid, buffer)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
		&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_DETACHING_FLAG))
		return EPHIDGET_NOTATTACHED;

	*buffer = (char *)phid->usbProduct;
	return EPHIDGET_OK;
}

int CCONV
CPhidget_getDeviceLabel(CPhidgetHandle phid, const char **buffer)
{
	TESTPTRS(phid, buffer)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
		&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_DETACHING_FLAG))
		return EPHIDGET_NOTATTACHED;

	*buffer = (char *)phid->label;
	return EPHIDGET_OK;
}

int CCONV
CPhidget_setDeviceLabel(CPhidgetHandle phid, const char *buffer)
{
	int ret = EPHIDGET_OK;
	TESTPTRS(phid, buffer)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(CPhidget_statusFlagIsSet(phid->status, PHIDGET_REMOTE_FLAG))
	{
		char key[1024];
		
		if((ret = encodeLabelString(buffer, NULL, NULL)) != EPHIDGET_OK)
			return ret;

		snprintf(key, sizeof(key), "/PCK/%s/%d/Label", phid->deviceType, phid->serialNumber);
		CThread_mutex_lock(&phid->lock);
		if(!CPhidget_statusFlagIsSet(phid->status, PHIDGET_SERVER_CONNECTED_FLAG))
		{
			CThread_mutex_unlock(&phid->lock);
			return EPHIDGET_NETWORK_NOTCONNECTED;
		}
		pdc_async_set(phid->networkInfo->server->pdcs, key, buffer, (int)strlen(buffer), PFALSE, internal_async_network_error_handler, phid);
		CThread_mutex_unlock(&phid->lock);
		return EPHIDGET_OK;
	}
	else
	{
		int len;
		char buffer2[(MAX_LABEL_SIZE * 2) + 2];
		ZEROMEM(buffer2, (MAX_LABEL_SIZE * 2) + 2);
		
		len = (MAX_LABEL_SIZE * 2);
		if((ret = encodeLabelString(buffer, &buffer2[2], &len)) == EPHIDGET_OK)
		{
			//length of descriptor
			buffer2[0] = len+2;
			//type of descriptor (string)
			buffer2[1] = 3;
			
			//make sure we're not trying to set a label that will match the wrap-around bug when read back
			if(labelHasWrapError(phid->serialNumber, buffer2) == PTRUE)
			{
				LOG(PHIDGET_LOG_WARNING, "Can't set a label that would match the wraparound bug.");
				return EPHIDGET_INVALIDARG;
			}
				
			if ((ret = CUSBSetLabel(phid, buffer2)) == EPHIDGET_OK)
			{
				int triedUTF8 = PFALSE;
				
			refresh:
				//read back the label and compare it
				if ((ret = CUSBRefreshLabelString(phid)) == EPHIDGET_OK)
				{
					CPhidgetHandle foundPhidget;
					
					//label read back didn't match
					if(strcmp(buffer, phid->label))
					{
						//label descriptor is longer then 16 bytes and the first 7 bytes back match;
						// almost certainly this is a problem with the wrap around bug.
						if(buffer2[0] > 16 && !strncmp(buffer, phid->label, 7) && triedUTF8 == PFALSE)
						{
							//try setting the label as UTF-8 with 0xFFFF header - we can encode up to 12 bytes
							if(strlen(buffer) <= 12)
							{
								LOG(PHIDGET_LOG_INFO, "Trying to setLabel as UTF-8 because of wrap around bug.");
								
								//only try this once
								triedUTF8 = PTRUE;
								
								strcpy(&buffer2[4], buffer);
								buffer2[0] = strlen(buffer) + 4;
								buffer2[2] = 0xFF;
								buffer2[3] = 0xFF;
								
								if ((ret = CUSBSetLabel(phid, buffer2)) == EPHIDGET_OK)
								{
									//go check it
									goto refresh;
								}
								else //setLabel failed
								{
									LOG(PHIDGET_LOG_ERROR, "Something unexpected happened trying to set the label (UTF-8). Try again.");
									goto clearlabel; 
								}
							}
							else //label is too long to be stored, but we have tried to write the label, so we need to clear out anything stored
							{
								ret = EPHIDGET_INVALIDARG;
								LOG(PHIDGET_LOG_ERROR, "This device supports 12-bytes UTF-8 labels. Try again with a shorter string, or pure ASCII.");
								goto clearlabel;
							}
						}
						else //label doesn't match and it doesn't look like the wrap around error
						{
							ret = EPHIDGET_UNEXPECTED;
							LOG(PHIDGET_LOG_ERROR, "set label doesn't match read back label: \"%s\" vs. \"%s\"", buffer, phid->label);
							goto clearlabel;
						}
					}
					else //label matches, we're good
					{
						//update label in PhidgetManager
						CThread_mutex_lock(&attachedDevicesLock);
						if(CList_findInList((CListHandle)AttachedDevices, phid, CPhidget_areEqual, (void **)&foundPhidget) == EPHIDGET_OK)
						{
							strcpy(foundPhidget->label, buffer);
						}
						CThread_mutex_unlock(&attachedDevicesLock);
					}
				}
				else 
				{
					LOG(PHIDGET_LOG_ERROR, "Was unable to read back the label after setting.");
					goto clearlabel;
				}
			}
			else 
			{
				LOG(PHIDGET_LOG_ERROR, "Something unexpected happened trying to set the label. Try again.");
				return ret;
			}
		}
		else 
		{
			LOG(PHIDGET_LOG_ERROR, "Error encoding label string, not setting label.");
			return ret;
		}
		
		//Success!
		return EPHIDGET_OK;

		//if a setLabel succeeded, but then we got an error verifying, then we should just clear the label so there's nothing funky in there.
	clearlabel:
		LOG(PHIDGET_LOG_INFO, "Clearing label because of an error during set.");
		ZEROMEM(buffer2, (MAX_LABEL_SIZE * 2) + 2);
		buffer2[0] = 2;
		buffer2[1] = 3;
		CUSBSetLabel(phid, buffer2);
		return ret;
	}
}

int CCONV
CPhidget_getServerID(CPhidgetHandle phid, const char **buffer)
{
	TESTPTRS(phid, buffer)

#ifdef USE_ZEROCONF
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_REMOTE_FLAG))
		return EPHIDGET_UNSUPPORTED;

	CThread_mutex_lock(&phid->lock);

	if(!phid->networkInfo->mdns) //not mDNS - not yet supported
	{
		CThread_mutex_unlock(&phid->lock);
		return EPHIDGET_UNSUPPORTED;
	}

	//refresh ONLY if connected - otherwise it might block
	if(CPhidget_statusFlagIsSet(phid->status, PHIDGET_SERVER_CONNECTED_FLAG))
	{
		if(refreshZeroconfPhidget(phid))
		{
			CThread_mutex_unlock(&phid->lock);
			return EPHIDGET_NETWORK;
		}
	}
	if(phid->networkInfo->zeroconf_server_id)
	{
		*buffer = (char *)phid->networkInfo->zeroconf_server_id;
	}
	else if(phid->networkInfo->requested_serverID)
	{
		*buffer = (char *)phid->networkInfo->requested_serverID;
	}
	else
	{
		CThread_mutex_unlock(&phid->lock);
		return EPHIDGET_UNEXPECTED;
	}

	CThread_mutex_unlock(&phid->lock);
	return EPHIDGET_OK;
#else
	return EPHIDGET_UNSUPPORTED;
#endif
}

int CCONV
CPhidget_getServerAddress(CPhidgetHandle phid, const char **ipAddr, int *port)
{
	TESTPTRS(phid, ipAddr)
	TESTPTR(port)

	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_REMOTE_FLAG))
		return EPHIDGET_UNSUPPORTED;

	CThread_mutex_lock(&phid->lock);
#ifdef USE_ZEROCONF
	if(phid->networkInfo->mdns)
	{
		//Look it up again new EVERY time!
		if(getZeroconfHostPort(phid->networkInfo))
		{
			CThread_mutex_unlock(&phid->lock);
			return EPHIDGET_NETWORK;
		}
		if(!phid->networkInfo->zeroconf_host || !phid->networkInfo->zeroconf_port)
		{
			CThread_mutex_unlock(&phid->lock);
			return EPHIDGET_UNEXPECTED;
		}
		*ipAddr = (char *)phid->networkInfo->zeroconf_host;
		*port = strtol(phid->networkInfo->zeroconf_port, NULL, 10);
	}
	else
#endif
	{
		if(CPhidget_statusFlagIsSet(phid->status, PHIDGET_SERVER_CONNECTED_FLAG))
		{
			if(!phid->networkInfo->server->address || !phid->networkInfo->server->port)
			{
				CThread_mutex_unlock(&phid->lock);
				return EPHIDGET_UNEXPECTED;
			}
			*ipAddr = (char *)phid->networkInfo->server->address;
			*port = strtol(phid->networkInfo->server->port, NULL, 10);
		}
		else
		{
			*ipAddr = (char *)phid->networkInfo->requested_address;
			*port = strtol(phid->networkInfo->requested_port, NULL, 10);
		}
	}
	CThread_mutex_unlock(&phid->lock);
	return EPHIDGET_OK;
}

int CCONV CPhidget_getErrorDescription(int ErrorCode, const char **buf)
{
	TESTPTR(buf)
		if ((ErrorCode < 0) || (ErrorCode >= PHIDGET_ERROR_CODE_COUNT)) {
			*buf = CPhidget_strerror(EPHIDGET_INVALID);
		return EPHIDGET_INVALID;
		}
	*buf = CPhidget_strerror(ErrorCode);
	return EPHIDGET_OK;
}

//expect 6 bytes of data
int CCONV CPhidget_calibrate(CPhidgetHandle phid, unsigned char Offset, unsigned char *data)
{
	int result;
	unsigned char buffer[8];
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	ZEROMEM(buffer, sizeof(buffer));
	
	buffer[0] = 0x74;
	buffer[1] = Offset;
	buffer[2] = data[0];
	buffer[3] = data[1];
	buffer[4] = data[2];
	buffer[5] = data[3];
	buffer[6] = data[4];
	buffer[7] = data[5];

	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	return EPHIDGET_OK;
}

int CCONV CPhidget_calibrate_gain2offset(CPhidgetHandle phid, int Index, unsigned short offset, unsigned long gain1, unsigned long gain2)
{
	int result;
	unsigned char buffer[8];
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	ZEROMEM(buffer, sizeof(buffer));
	
	buffer[0] = 0x73;
	
	buffer[1] = 0 + Index;
	buffer[2] = (unsigned char)(offset >> 8);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 1 + Index;
	buffer[2] = (unsigned char)(offset & 0xff);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 2 + Index;
	buffer[2] = (unsigned char)(gain1 >> 16);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 3 + Index;
	buffer[2] = (unsigned char)(gain1 >> 8);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 4 + Index;
	buffer[2] = (unsigned char)(gain1);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 5 + Index;
	buffer[2] = (unsigned char)(gain2 >> 16);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 6 + Index;
	buffer[2] = (unsigned char)(gain2 >> 8);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 7 + Index;
	buffer[2] = (unsigned char)(gain2);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	return EPHIDGET_OK;
}

int CCONV CPhidget_calibrate_gainoffset(CPhidgetHandle phid, int Index, unsigned short offset, unsigned long gain)
{
	//BL:Not sure what this does
	int result;
	unsigned char buffer[8];
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	ZEROMEM(buffer, sizeof(buffer));
	
	buffer[0] = 0x72;
	
	//BL: Index can overflow byte
	buffer[1] = 0 + Index;
	buffer[2] = (unsigned char)(offset >> 8);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 1 + Index;
	buffer[2] = (unsigned char)(offset & 0xff);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 2 + Index;
	buffer[2] = (unsigned char)(gain >> 16);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 3 + Index;
	buffer[2] = (unsigned char)(gain >> 8);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	buffer[1] = 4 + Index;
	buffer[2] = (unsigned char)(gain);
	if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		return result;

	return EPHIDGET_OK;
}

//0 ms = infinite timeout - note that it only checks the attach variable every 10ms
int CCONV CPhidget_waitForAttachment(CPhidgetHandle phid, int milliseconds)
{
	//BL: Should this use constants for infinite timeout rather than just 0?
	long duration = 0;
	TIME	start;

#ifdef _WINDOWS
	TIME now;
	FILETIME nowft, oldft, resft;
	ULARGE_INTEGER nowul, oldul, resul;
#else
	struct timeval now;
#endif

	TESTPTR(phid)
	if(milliseconds)
	{
#ifdef _WINDOWS
		GetSystemTime(&start);
#else
		gettimeofday(&start,NULL);
#endif
	}

	while(!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
	{
		if(!CPhidget_statusFlagIsSet(phid->status, PHIDGET_OPENED_FLAG))
			return EPHIDGET_CLOSED;
		if(milliseconds)
		{
#ifdef _WINDOWS
			GetSystemTime(&now);
			SystemTimeToFileTime(&now, &nowft);
			SystemTimeToFileTime(&start, &oldft);

			nowul.HighPart = nowft.dwHighDateTime;
			nowul.LowPart = nowft.dwLowDateTime;
			oldul.HighPart = oldft.dwHighDateTime;
			oldul.LowPart = oldft.dwLowDateTime;

			resul.HighPart = nowul.HighPart - oldul.HighPart;
			resul.LowPart = nowul.LowPart - oldul.LowPart;

			resft.dwHighDateTime = resul.HighPart;
			resft.dwLowDateTime = resul.LowPart;

			duration = (resft.dwLowDateTime/10000);
#else
			gettimeofday(&now, NULL);
			duration = (now.tv_sec - start.tv_sec)*1000 + ((now.tv_usec-start.tv_usec)/1000);
#endif
			if(duration > milliseconds)
				return EPHIDGET_TIMEOUT;
		}
		SLEEP(10);
	}
	return EPHIDGET_OK;
}

void throw_error_event(CPhidgetHandle phid, const char *error, int errcode)
{
	//BL:didn't add checking here since don't want to mess with error propagation. Should?
	if(phid && phid->fptrError)
	{
		phid->fptrError(phid, phid->fptrErrorptr, errcode, error);
		return;
	}
	LOG(PHIDGET_LOG_WARNING,"Got an async error: %d: %s\n\tTip: Set up an error handler to catch this properly.", errcode, error);
	//abort();
}

int findActiveDevice(CPhidgetHandle attachedDevice)
{
	int result;
	CPhidgetList *activeDevice;
	CPhidgetList *matches = 0;

	CThread_mutex_lock(&activeDevicesLock);

	/* First search for an active device that matches this specific attached device
	 * (specific serial number of specific label
	 */
	for (activeDevice=ActiveDevices; activeDevice; activeDevice = activeDevice->next)
	{
		/* Don't bother with devices that are already associated with an attached Phidget */
		if(!CPhidget_statusFlagIsSet(activeDevice->phid->status, PHIDGET_ATTACHED_FLAG))
		{
			/* Check device ID (Phidget class) */
			if(activeDevice->phid->deviceID == attachedDevice->deviceID)
			{
				/* Check for either openLabel with matching label, or openSerial with matching serial */
				if((
						activeDevice->phid->specificDevice == PHIDGETOPEN_SERIAL 
						&& (activeDevice->phid->serialNumber == attachedDevice->serialNumber)
					) || (
						activeDevice->phid->specificDevice == PHIDGETOPEN_LABEL 
						&& !strncmp(activeDevice->phid->label, attachedDevice->label, MAX_LABEL_STORAGE-1)
					))
				{
					/* add to match list */
					CList_addToList((CListHandle *)&matches, activeDevice->phid, CPhidgetHandle_areEqual);
				}
			}
		}
	}

	/* If we didn't find a specific match, then check is we have any 'openany' active devices
	 * that match the device type
	 */
	for (activeDevice=ActiveDevices; activeDevice; activeDevice = activeDevice->next)
	{
		/* Don't bother with devices that are already associated with an attached Phidget */
		if(!CPhidget_statusFlagIsSet(activeDevice->phid->status, PHIDGET_ATTACHED_FLAG))
		{
			/* Check device ID (Phidget class) */
			if(activeDevice->phid->deviceID == attachedDevice->deviceID)
			{
				/* Make sure this is an openAny device */
				if(activeDevice->phid->specificDevice == PHIDGETOPEN_ANY)
				{
					/* add to match list */
					CList_addToList((CListHandle *)&matches, activeDevice->phid, CPhidgetHandle_areEqual);
				}
			}
		}
	}

	CThread_mutex_unlock(&activeDevicesLock);

	/* we create a list of possible matches and try to open them till we get success
	 * this is so we can unlock activeDevicesLock before calling attachActiveDevice.
	 */

	/* Found a match, try to associate this Phidget with the actual Phidget and open it.
	 * This should send the attach event, set the PHIDGET_ATTACHED_FLAG flag to true, 
	 * star the read and write threads and send out initial events
	 */
	result = EPHIDGET_NOTFOUND;
	for (activeDevice=matches; activeDevice; activeDevice = activeDevice->next)
	{
		/* Prevent close from being called during attachActiveDevice */
		CThread_mutex_lock(&activeDevice->phid->openCloseLock);
		if(attachActiveDevice(activeDevice->phid, attachedDevice) == EPHIDGET_OK)
			result = EPHIDGET_OK;
		CThread_mutex_unlock(&activeDevice->phid->openCloseLock);
		
		/* if successfull, stop looking. */
		if(result == EPHIDGET_OK)
			break;
	}
	//free list
	CList_emptyList((CListHandle *)&matches, PFALSE, NULL);

	return result;
}

int findActiveDevices()
{
	CPhidgetList *attachedDevice;
	int result = 0;

	CThread_mutex_lock(&attachedDevicesLock);

	for (attachedDevice=AttachedDevices; attachedDevice; attachedDevice = attachedDevice->next)
	{
		result = findActiveDevice(attachedDevice->phid);
	}

	CThread_mutex_unlock(&attachedDevicesLock);

	return result;
}

//active device is in the list of devices waiting for connections, attachedDevice is the device that was just plugged in
int attachActiveDevice(CPhidgetHandle activeDevice, CPhidgetHandle attachedDevice)
{
	int result = 0;
	TESTPTRS(activeDevice, attachedDevice)

	if(!CPhidget_statusFlagIsSet(activeDevice->status, PHIDGET_OPENED_FLAG))
		return EPHIDGET_UNEXPECTED;

#ifdef _WINDOWS
#ifndef WINCE
	//open uses this so it doesn't have to enumerate all devices - but ONLY ON WINDOWS!
	activeDevice->CPhidgetFHandle = malloc(wcslen(attachedDevice->CPhidgetFHandle)*sizeof(WCHAR)+10);
	wcsncpy((WCHAR *)activeDevice->CPhidgetFHandle, attachedDevice->CPhidgetFHandle, wcslen(attachedDevice->CPhidgetFHandle)+1);
	activeDevice->deviceIDSpec = attachedDevice->deviceIDSpec;
	activeDevice->deviceUID = attachedDevice->deviceUID;
	activeDevice->deviceDef = attachedDevice->deviceDef;
#endif
#endif
	
#ifdef _ANDROID
	//Android also uses the file handle to open the device
	activeDevice->CPhidgetFHandle = strdup(attachedDevice->CPhidgetFHandle);
	activeDevice->deviceIDSpec = attachedDevice->deviceIDSpec;
	activeDevice->deviceUID = attachedDevice->deviceUID;
	activeDevice->deviceDef = attachedDevice->deviceDef;
#endif

	if(activeDevice->specificDevice == PHIDGETOPEN_ANY)
	{
		activeDevice->specificDevice = PHIDGETOPEN_ANY_ATTACHED;
	}
	activeDevice->serialNumber = attachedDevice->serialNumber;

	if ((result = CUSBOpenHandle(activeDevice)) !=
		EPHIDGET_OK) {
		LOG(PHIDGET_LOG_WARNING,"unable to open active device: %d", result);
		if(activeDevice->specificDevice == PHIDGETOPEN_ANY_ATTACHED)
		{
			activeDevice->specificDevice = PHIDGETOPEN_ANY;
			activeDevice->serialNumber = -1;
		}
		activeDevice->deviceIDSpec = 0;
		activeDevice->deviceUID = 0;
		return result;
	}
	
	CThread_mutex_lock(&activeDevice->lock);
	CPhidget_setStatusFlag(&activeDevice->status, PHIDGET_ATTACHING_FLAG, NULL);
	if((result = activeDevice->fptrInit((CPhidgetHandle)activeDevice)))
	{
		CPhidget_clearStatusFlag(&activeDevice->status, PHIDGET_ATTACHING_FLAG, NULL);
		CThread_mutex_unlock(&activeDevice->lock);
		if(activeDevice->specificDevice == PHIDGETOPEN_ANY_ATTACHED)
		{
			activeDevice->specificDevice = PHIDGETOPEN_ANY;
			activeDevice->serialNumber = -1;
		}
		LOG(PHIDGET_LOG_ERROR, "Device Initialization functions failed: %d", result);
		if(result == EPHIDGET_BADVERSION)
		{
			if (activeDevice->fptrError)
				activeDevice->fptrError((CPhidgetHandle)activeDevice, activeDevice->fptrErrorptr, EEPHIDGET_BADVERSION, "This Phidget requires a newer library - please upgrade.");
		}
		CUSBCloseHandle(activeDevice);
		return result;
	}

	//make sure the write events are in a good state
	activeDevice->writeStopFlag = FALSE;
	CThread_reset_event(&activeDevice->writtenEvent);
	CThread_reset_event(&activeDevice->writeAvailableEvent);

	//set phidget as attached
	CPhidget_clearStatusFlag(&activeDevice->status, PHIDGET_ATTACHING_FLAG, NULL);
	CPhidget_setStatusFlag(&activeDevice->status, PHIDGET_ATTACHED_FLAG, NULL);

	//Start write thread
	if (CThread_create(&activeDevice->writeThread,
		WriteThreadFunction, activeDevice)) {
		LOG(PHIDGET_LOG_WARNING,"unable to create write thread");
		CPhidget_clearStatusFlag(&activeDevice->status, PHIDGET_ATTACHED_FLAG, NULL);
		CThread_mutex_unlock(&activeDevice->lock);
		CUSBCloseHandle(activeDevice);
		if(activeDevice->specificDevice == PHIDGETOPEN_ANY_ATTACHED)
		{
			activeDevice->specificDevice = PHIDGETOPEN_ANY;
			activeDevice->serialNumber = -1;
		}
		return EPHIDGET_UNEXPECTED;
	}
	activeDevice->writeThread.thread_status = TRUE;
	
	CThread_mutex_unlock(&activeDevice->lock);
	
	//Attach event
	if (activeDevice->fptrAttach)
	{	
		//printf("Throwing read thread\n");
		activeDevice->fptrAttach(activeDevice,
			activeDevice->fptrAttachptr);
	}
	
	activeDevice->fptrEvents((CPhidgetHandle)activeDevice);

	//Start read thread
	//We start this after the attach event returns so that we can guarantee no data events
	CThread_mutex_lock(&activeDevice->lock);
	if (CThread_create(&activeDevice->readThread,
		ReadThreadFunction, activeDevice)) {
		LOG(PHIDGET_LOG_WARNING,"unable to create read thread");
		CPhidget_clearStatusFlag(&activeDevice->status, PHIDGET_ATTACHED_FLAG, NULL);
		CThread_mutex_unlock(&activeDevice->lock);
		activeDevice->writeStopFlag = PTRUE;
		CThread_join(&activeDevice->writeThread); //join before closing because we want to wait for outstanding writes to complete
		CUSBCloseHandle(activeDevice);
		if(activeDevice->specificDevice == PHIDGETOPEN_ANY_ATTACHED)
		{
			activeDevice->specificDevice = PHIDGETOPEN_ANY;
			activeDevice->serialNumber = -1;
		}
		return EPHIDGET_UNEXPECTED;
	}
	activeDevice->readThread.thread_status = TRUE;
	CThread_mutex_unlock(&activeDevice->lock);

	return EPHIDGET_OK;
}

double round_double(double x, int decimals)
{
	return (double)((double)round(x * (double)(pow(10, decimals))) / (double)(pow(10, decimals)));
}

//Verifies that the UTF-8 label will fit in 20 UTF-16 bytes
//Also gives you a handle to the output label - since we had to make it anyways...
int encodeLabelString(const char *buffer, char *out, int *outLen)
{
	int len;
	char buffer2[(MAX_LABEL_SIZE * 2)];
	
#ifdef USE_INTERNAL_UNICONV
	const UTF8 *utf8label = (const UTF8 *)buffer;
	const UTF8 *utf8labelEnd = utf8label + strlen(buffer);
	UTF16 *utf16label = (UTF16 *)buffer2;
	UTF16 *utf16labelEnd = utf16label + (MAX_LABEL_SIZE);
	ConversionResult resp;
	
	resp = ConvertUTF8toUTF16(&utf8label, utf8labelEnd, &utf16label, utf16labelEnd, strictConversion);
	
	if(resp != conversionOK)
	{
		switch(resp)
		{
			case sourceExhausted:
				LOG (PHIDGET_LOG_WARNING, "source exhausted error.");
				break;
			case targetExhausted:
				LOG (PHIDGET_LOG_WARNING, "target exhausted error.");
				break;
			default:
				LOG (PHIDGET_LOG_WARNING, "unexpected error.");
				return EPHIDGET_UNEXPECTED;
		}
		return EPHIDGET_INVALIDARG;
	}
	len = (size_t)utf16label - (size_t)buffer2;

#else
	
 #ifndef _WINDOWS
	//Mac and Linux compatible UTF-8 to UTF-16LE conversion
	char *utf8label = (char *)buffer;
	char *utf16label = buffer2;
	size_t inBytes = strlen(buffer); // Up to MAX_LABEL_STORAGE bytes read
	size_t outBytes = (MAX_LABEL_SIZE * 2); //UTF-16 characters are two bytes each.
	iconv_t conv;
	size_t resp;
	
	conv= iconv_open("UTF-16LE", "UTF-8");
	if (conv == (iconv_t)(-1))
		return EPHIDGET_UNEXPECTED;
	
	resp = iconv(conv, &utf8label, &inBytes, (char **)&utf16label, &outBytes);
	
	iconv_close(conv);
	
	if (resp == (size_t) -1) {
		switch (errno) {
			case EILSEQ:
			case EINVAL:
				LOG (PHIDGET_LOG_WARNING, "Malformed UTF8 string used for label.");
				break;
			case E2BIG:
				LOG (PHIDGET_LOG_WARNING, "Label string is too long.");
				break;
			default:
				LOG (PHIDGET_LOG_ERROR, "Unexpected error in parsing label string: %s.", strerror (errno));
				return EPHIDGET_UNEXPECTED;
		}
		return EPHIDGET_INVALIDARG;
	}
	//length of descriptor = string length in bytes plus header (buffer[0] and buffer[1])
	len = ((MAX_LABEL_SIZE * 2)-outBytes);
 #else
	//Windows compatible UTF-8 to UTF-16LE conversion
	int ret = EPHIDGET_OK;
	int charsWritten=0;
	wchar_t *outLabel = (wchar_t *)buffer2;
	//don't try to convert a nothing string
	if(strlen(buffer) > 0)
	{
		charsWritten = MultiByteToWideChar(CP_UTF8, 0, buffer, strlen(buffer), outLabel, MAX_LABEL_SIZE);

		//Error
		if(!charsWritten)
		{
			switch (GetLastError())
			{
				case ERROR_INSUFFICIENT_BUFFER:
					LOG(PHIDGET_LOG_WARNING, "Label string is too long.");
					break;
				case ERROR_NO_UNICODE_TRANSLATION:
					LOG (PHIDGET_LOG_WARNING, "Malformed UTF8 string used for label.");
					break;
				case ERROR_INVALID_PARAMETER:
				default:
					LOG (PHIDGET_LOG_ERROR, "Unexpected error in parsing label string.");
					return EPHIDGET_UNEXPECTED;
			}
			return EPHIDGET_INVALIDARG;
		}
	}

	len = charsWritten*2;
 #endif
	
#endif
	
	if(out && outLen)
	{
		if(len <= *outLen)
			*outLen = len;
		memcpy(out, buffer2, *outLen);
	}
	return EPHIDGET_OK;
}

//detect if this label descriptor exhibits the wraparound error
//ie bytes 16-21 will match bytes 0-5 of the serial number string descriptor
int labelHasWrapError(int serialNumber, char *labelBuf)
{
	char errorBytes[6];
	char serialString[8];
	int serialLen;
	int compareSize;
	int i;
	
	//only applies when the label descriptor is > 16 bytes
	if(labelBuf[0] <= 16)
		return PFALSE;
	
	//only applies when the first 7 digits are ascii (set by old label functions)
	for(i=3; i<16; i+=2)
		if(labelBuf[i] != 0x00)
			return PFALSE;
	
	ZEROMEM(errorBytes, 6);
	
	//construct the 1st 6 bytes of the serial number descriptor
	snprintf(serialString, 8, "%d", serialNumber);
	serialLen = strlen(serialString);
	
	errorBytes[0] = serialLen * 2 + 2; //length
	errorBytes[1] = 0x03;	//type
	
	//serial number 1st digit
	if(serialLen>=1)
		errorBytes[2] = serialString[0];
	else
		errorBytes[2] = 0x00;
	errorBytes[3] = 0x00;
	
	//serial number 2nd digit
	if(serialLen>=2)
		errorBytes[4] = serialString[1];
	else
		errorBytes[4] = 0x00;
	errorBytes[5] = 0x00;
	
	//compare the end of the label buffer with the string descriptor
	compareSize = labelBuf[0] - 16;
	if(!strncmp(&labelBuf[16], errorBytes, compareSize))
		return PTRUE;
	
	return PFALSE;
}

int UTF16toUTF8(char *in, int inLen, char *out)
{
#ifdef USE_INTERNAL_UNICONV
		UTF8 *utf8string = (UTF8 *)out;
		UTF8 *utf8stringEnd = utf8string + MAX_LABEL_STORAGE;
		const UTF16 *utf16string = (const UTF16 *)in;
		const UTF16 *utf16stringEnd = utf16string + (inLen / 2);
		ConversionResult resp;
		resp = ConvertUTF16toUTF8(&utf16string, utf16stringEnd, &utf8string, utf8stringEnd, strictConversion);
		
		if(resp != conversionOK)
		{
			switch(resp)
			{
				case sourceExhausted:
					LOG (PHIDGET_LOG_WARNING, "source exhausted error.");
					break;
				case targetExhausted:
					LOG (PHIDGET_LOG_WARNING, "target exhausted error.");
					break;
				default:
					LOG (PHIDGET_LOG_WARNING, "unexpected error.");
					return EPHIDGET_UNEXPECTED;
			}
			return EPHIDGET_INVALIDARG;
		}
#else
 #ifndef _WINDOWS
		char *utf16string = in;
		char *utf8string = (char *)out;
		size_t inBytes = inLen; // Up to MAX_LABEL_STORAGE bytes read
		size_t outBytes = (MAX_LABEL_STORAGE); //UTF-16 characters are two bytes each.
		iconv_t conv;
		size_t resp;
		conv= iconv_open("UTF-8", "UTF-16LE");
		if (conv == (iconv_t)(-1))
			return EPHIDGET_UNEXPECTED;
		
		resp = iconv(conv, &utf16string, &inBytes, &utf8string, &outBytes);
		
		iconv_close(conv);
		
		if (resp == (size_t) -1) {
			switch (errno) {
				case EILSEQ:
				case EINVAL:
				case E2BIG:
				default:
					LOG (PHIDGET_LOG_ERROR, "Unexpected error converting string to UTF-8: %s.", strerror (errno));
					return EPHIDGET_UNEXPECTED;
			}
		}
 #else
		//stringData in NULL terminated
		int bytesWritten = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)in, -1, out, MAX_LABEL_STORAGE+1, NULL, NULL);

		//Error
		if(!bytesWritten)
		{
			LOG(PHIDGET_LOG_ERROR, "Unable to convert string to UTF-8!");
			return EPHIDGET_UNEXPECTED;
		}
 #endif
#endif
	return EPHIDGET_OK;
}

//takes the label string buffer from the USB device and outputs a UTF-8 version
int decodeLabelString(char *labelBuf, char *out, int serialNumber)
{
	//out NEEDS to be zeroed out, or we'll end up with a UTF-8 string with no terminating NULL
	ZEROMEM(out, MAX_LABEL_STORAGE);
	
	//this returns true only if our descriptor is > 16 bytes and has the error, so we truncate
	if (labelHasWrapError(serialNumber, labelBuf))
	{
		int i;
		for(i=16;i<labelBuf[0];i++)
			labelBuf[i] = 0x00;
		labelBuf[0] = 16;
		LOG(PHIDGET_LOG_WARNING, "Detected getLabel error - label is being truncated to first 7 characters. Please setLabel again to correct this.");
	}
	
	//check if the label is stored as UTF-8 directly
	if(labelBuf[0] > 4 && labelBuf[2] == (char)0xFF && labelBuf[3] == (char)0xFF)
	{
		LOG(PHIDGET_LOG_DEBUG, "Found a wrap-around bug style label.");
		memcpy(out, &labelBuf[4], labelBuf[0]-4);
		out[labelBuf[0]-4] = '\0';
	}
	//otherwise it's stored as UTF-16LE
	else
	{
		return UTF16toUTF8(&labelBuf[2], labelBuf[0]-2, out);
	}
	
	return EPHIDGET_OK;
}

CPhidget_DeviceUID CPhidget_getUID(CPhidget_DeviceID id, int version)
{
	const CPhidgetUniqueDeviceDef *uidList = Phid_Unique_Device_Def;
	int i = 0;

	while (uidList->pdd_uid)
	{
		if(uidList->pdd_id == id && version >= uidList->pdd_vlow && version < uidList->pdd_vhigh)\
			return uidList->pdd_uid;
		i++;
		uidList++;
	}

	//Should never get here!
	LOG(PHIDGET_LOG_DEBUG, "We have a Phidgets that doesn't match and Device UID!");
	return PHIDUID_NOTHING;
}



/**
 * General Packet Protocol
 *
 * These are devices on the new M3. They all support this protocol.
 *  MSB is set is buf[0] for incoming and outgoing packets.
 */

int deviceSupportsGeneralUSBProtocol(CPhidgetHandle phid)
{
	switch(phid->deviceUID)
	{
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1041:
		case PHIDUID_SPATIAL_ACCEL_3AXIS_1043:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1042:
		case PHIDUID_SPATIAL_ACCEL_GYRO_COMPASS_1044:
		case PHIDUID_LED_64_ADV_M3:
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
		case PHIDUID_STEPPER_BIPOLAR_1MOTOR_M3:
			return PTRUE;

		case PHIDUID_FIRMWARE_UPGRADE:
			return PTRUE;

		case PHIDUID_GENERIC:
			return PTRUE;

		default:
			return PFALSE;
	}
}

static int CPhidgetGPP_dataInput(CPhidgetHandle phid, unsigned char *buffer, int length)
{
	int result = EPHIDGET_OK;

	//if response bits are set (0x00 is ignore), then store response
	if(buffer[0] & 0x3f)
		phid->GPPResponse = buffer[0];

	return result;
}

int GPP_getResponse(CPhidgetHandle phid, int packetType, int timeout)
{
	while((phid->GPPResponse & 0x3f) != packetType && timeout > 0)
	{
		SLEEP(20);
		timeout -= 20;
	}

	//Didn't get the response!
	if((phid->GPPResponse & 0x3f) != packetType)
	{
		return EPHIDGET_TIMEOUT;
	}

	if(phid->GPPResponse & PHID_USB_GENERAL_PACKET_FAIL)
		return EPHIDGET_UNEXPECTED;

	return EPHIDGET_OK;
}

int CCONV CPhidgetGPP_upgradeFirmware(CPhidgetHandle phid, unsigned char *data, int length)
{
	int result, i, j, index, indexEnd;
	unsigned char *buffer;
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(!deviceSupportsGeneralUSBProtocol(phid))
		return EPHIDGET_UNSUPPORTED;

	buffer = (unsigned char *) malloc(phid->outputReportByteLength);
	ZEROMEM(buffer, phid->outputReportByteLength);
	
	CThread_mutex_lock(&phid->writelock);

	phid->GPPResponse = 0;

	index = ((length & 0xf000) >> 12) + 1;
	indexEnd = length & 0xfff;
	j = 0;
	while (index)
	{
		int secLength = length - ((index - 1) * 0x1000);
		if(secLength > 0x1000) secLength = 0x1000;

		buffer[0] = PHID_USB_GENERAL_PACKET_FLAG | PHID_USB_GENERAL_PACKET_FIRMWARE_UPGRADE_WRITE_SECTOR;
		buffer[1] = index;
		buffer[2] = secLength;
		buffer[3] = secLength >> 8;

		for(i=4;i<phid->outputReportByteLength && j<indexEnd;i++,j++)
			buffer[i] = data[j];

		if((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
			goto done;

		while(j<indexEnd && result == EPHIDGET_OK)
		{
			buffer[0] = PHID_USB_GENERAL_PACKET_FLAG | PHID_USB_GENERAL_PACKET_CONTINUATION;
			for(i=1;i<phid->outputReportByteLength && j<indexEnd;i++,j++)
				buffer[i] = data[j];

			if((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
				goto done;
		}
		index--;
		indexEnd+=0x1000;
	}

done:

	result = GPP_getResponse(phid, PHID_USB_GENERAL_PACKET_FIRMWARE_UPGRADE_WRITE_SECTOR, 200);

	CThread_mutex_unlock(&phid->writelock);

	free(buffer);

	return result;
}

int CCONV CPhidgetGPP_eraseFirmware(CPhidgetHandle phid)
{
	int result;
	unsigned char *buffer;
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(!deviceSupportsGeneralUSBProtocol(phid))
		return EPHIDGET_UNSUPPORTED;

	buffer = (unsigned char *) malloc(phid->outputReportByteLength);
	ZEROMEM(buffer, phid->outputReportByteLength);
	
	buffer[0] = PHID_USB_GENERAL_PACKET_FLAG | PHID_USB_GENERAL_PACKET_FIRMWARE_UPGRADE_ERASE;
	
	CThread_mutex_lock(&phid->writelock);

	phid->GPPResponse = 0;
	if((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) == EPHIDGET_OK)
		result = GPP_getResponse(phid, PHID_USB_GENERAL_PACKET_FIRMWARE_UPGRADE_ERASE, 200);

	CThread_mutex_unlock(&phid->writelock);
	free(buffer);

	return result;
}

int CCONV CPhidgetGPP_eraseConfig(CPhidgetHandle phid)
{
	int result;
	unsigned char *buffer;
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(!deviceSupportsGeneralUSBProtocol(phid))
		return EPHIDGET_UNSUPPORTED;

	buffer = (unsigned char *) malloc(phid->outputReportByteLength);
	ZEROMEM(buffer, phid->outputReportByteLength);
	
	buffer[0] = PHID_USB_GENERAL_PACKET_FLAG | PHID_USB_GENERAL_PACKET_ERASE_CONFIG;
	
	CThread_mutex_lock(&phid->writelock);

	phid->GPPResponse = 0;
	if((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) == EPHIDGET_OK)
		result = GPP_getResponse(phid, PHID_USB_GENERAL_PACKET_ERASE_CONFIG, 200);

	CThread_mutex_unlock(&phid->writelock);
	free(buffer);

	return result;
}

int CCONV CPhidgetGPP_reboot_firmwareUpgrade(CPhidgetHandle phid)
{
	int result;
	unsigned char *buffer;
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(!deviceSupportsGeneralUSBProtocol(phid))
		return EPHIDGET_UNSUPPORTED;

	buffer = (unsigned char *) malloc(phid->outputReportByteLength);
	ZEROMEM(buffer, phid->outputReportByteLength);
	
	buffer[0] = PHID_USB_GENERAL_PACKET_FLAG | PHID_USB_GENERAL_PACKET_REBOOT_FIRMWARE_UPGRADE;
	
	//Stop the read/write threads first	
	phid->writeStopFlag = PTRUE;
	CThread_join(&phid->writeThread);
	CPhidget_clearStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
	CThread_join(&phid->readThread);
	CPhidget_setStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
	
	//Then send the command
	result = CUSBSendPacket((CPhidgetHandle)phid, buffer);

	free(buffer);

	return result;
}

int CCONV CPhidgetGPP_reboot_ISP(CPhidgetHandle phid)
{
	int result;
	unsigned char *buffer;
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(!deviceSupportsGeneralUSBProtocol(phid))
		return EPHIDGET_UNSUPPORTED;

	buffer = (unsigned char *) malloc(phid->outputReportByteLength);
	ZEROMEM(buffer, phid->outputReportByteLength);
	
	buffer[0] = PHID_USB_GENERAL_PACKET_FLAG | PHID_USB_GENERAL_PACKET_REBOOT_ISP;
	
	result = CUSBSendPacket((CPhidgetHandle)phid, buffer);

	free(buffer);

	return result;
}

int CCONV CPhidgetGPP_writeFlash(CPhidgetHandle phid)
{
	int result;
	unsigned char *buffer;
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(!deviceSupportsGeneralUSBProtocol(phid))
		return EPHIDGET_UNSUPPORTED;

	buffer = (unsigned char *) malloc(phid->outputReportByteLength);
	ZEROMEM(buffer, phid->outputReportByteLength);
	
	buffer[0] = PHID_USB_GENERAL_PACKET_FLAG | PHID_USB_GENERAL_PACKET_WRITE_FLASH;
	
	result = CUSBSendPacket((CPhidgetHandle)phid, buffer);

	free(buffer);

	return result;
}

int CCONV CPhidgetGPP_zeroConfig(CPhidgetHandle phid)
{
	int result;
	unsigned char *buffer;
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(!deviceSupportsGeneralUSBProtocol(phid))
		return EPHIDGET_UNSUPPORTED;

	buffer = (unsigned char *) malloc(phid->outputReportByteLength);
	ZEROMEM(buffer, phid->outputReportByteLength);
	
	buffer[0] = PHID_USB_GENERAL_PACKET_FLAG | PHID_USB_GENERAL_PACKET_ZERO_CONFIG;
	
	result = CUSBSendPacket((CPhidgetHandle)phid, buffer);

	free(buffer);

	return result;
}

int CCONV CPhidgetGPP_setLabel(CPhidgetHandle phid, const char *label)
{
	unsigned char buffer[26] = {0};
	int result;
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(!deviceSupportsGeneralUSBProtocol(phid))
		return EPHIDGET_UNSUPPORTED;

	//Label Table Header is: 0x0010001A
    buffer[3] = 0x00; //header high byte
    buffer[2] = 0x10;
    buffer[1] = 0x00;
    buffer[0] = 0x1a; //header low byte

	memcpy(buffer+4, label, label[0]);

	//Label Table index is: 0
	if((result=CPhidgetGPP_setDeviceWideConfigTable(phid, buffer, 26, 0))==EPHIDGET_OK)
		return CPhidgetGPP_writeFlash(phid);
	return result;
}

static int CPhidgetGPP_setConfigTable(CPhidgetHandle phid, unsigned char *data, int length, int index, int packetType)
{
	int result, i, j;
	unsigned char *buffer;
	TESTPTR(phid)
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
		return EPHIDGET_NOTATTACHED;

	if(!deviceSupportsGeneralUSBProtocol(phid))
		return EPHIDGET_UNSUPPORTED;

	buffer = (unsigned char *) malloc(phid->outputReportByteLength);
	ZEROMEM(buffer, phid->outputReportByteLength);
	
	buffer[0] = PHID_USB_GENERAL_PACKET_FLAG | packetType;
	buffer[1] = index;
	for(i=2,j=0;i<phid->outputReportByteLength && j<length;i++,j++)
		buffer[i] = data[j];

	CThread_mutex_lock(&phid->writelock);

	if((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
		goto done;

	while(j<length && result == EPHIDGET_OK)
	{
		buffer[0] = PHID_USB_GENERAL_PACKET_FLAG | PHID_USB_GENERAL_PACKET_CONTINUATION;
		for(i=1;i<phid->outputReportByteLength && j<length;i++,j++)
			buffer[i] = data[j];
		if((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
			goto done;
	}

done:
	CThread_mutex_unlock(&phid->writelock);

	free(buffer);

	return result;
}

int CCONV CPhidgetGPP_setDeviceSpecificConfigTable(CPhidgetHandle phid, unsigned char *data, int length, int index)
{
	return CPhidgetGPP_setConfigTable(phid, data, length, index, PHID_USB_GENERAL_PACKET_SET_DS_TABLE);
}

int CCONV CPhidgetGPP_setDeviceWideConfigTable(CPhidgetHandle phid, unsigned char *data, int length, int index)
{
	return CPhidgetGPP_setConfigTable(phid, data, length, index, PHID_USB_GENERAL_PACKET_SET_DW_TABLE);
}