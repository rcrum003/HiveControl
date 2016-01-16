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
#include "cphidgetsbc.h"
#include "cphidget.h"
#include "cthread.h"
#include "cphidgetlist.h"
#include "csocket.h"
#include "zeroconf.h"

//PhidgetSBCManager

//Private
void CPhidgetSBCManager_free(void *arg)
{
	CPhidgetSBCManagerHandle sbcm = (CPhidgetSBCManagerHandle)arg;
	
	if(!sbcm) return;
	free(sbcm); sbcm = NULL;
	return;
}

//Public
int CCONV CPhidgetSBCManager_create(CPhidgetSBCManagerHandle *sbcm)
{
	CPhidgetSBCManagerHandle sbcmtemp = 0;
	
	TESTPTR(sbcm)
	if(!(sbcmtemp = (CPhidgetSBCManagerHandle)malloc(sizeof(CPhidgetSBCManager))))
		return EPHIDGET_NOMEMORY;
	ZEROMEM(sbcmtemp, sizeof(CPhidgetSBCManager));

	*sbcm = sbcmtemp;
	return EPHIDGET_OK;
}

int CCONV CPhidgetSBCManager_stop(CPhidgetSBCManagerHandle sbcm)
{
	TESTPTR(sbcm)

	if(sbcm->state == PHIDGETMANAGER_ACTIVE)
	{
		sbcm->state = PHIDGETMANAGER_INACTIVE;
		unregisterSBCManager(sbcm);
	}

	return EPHIDGET_OK;
}

int CCONV CPhidgetSBCManager_delete(CPhidgetSBCManagerHandle sbcm)
{
	CPhidgetSBCManager_free(sbcm);
	return EPHIDGET_OK;
}

int CCONV CPhidgetSBCManager_set_OnAttach_Handler(CPhidgetSBCManagerHandle sbcm, int (CCONV *fptr)(CPhidgetSBCHandle sbc, void *userPtr), void *userPtr)
{
	TESTPTR(sbcm)
	sbcm->fptrAttachChange = fptr; 
	sbcm->fptrAttachChangeptr = userPtr; 
	return EPHIDGET_OK; 
}

int CCONV CPhidgetSBCManager_set_OnDetach_Handler(CPhidgetSBCManagerHandle sbcm, int (CCONV *fptr)(CPhidgetSBCHandle sbc, void *userPtr), void *userPtr)
{
	TESTPTR(sbcm)
	sbcm->fptrDetachChange = fptr; 
	sbcm->fptrDetachChangeptr = userPtr; 
	return EPHIDGET_OK; 
}

int CCONV CPhidgetSBCManager_set_OnError_Handler(CPhidgetSBCManagerHandle sbcm, 
	int(CCONV *fptr)(CPhidgetSBCManagerHandle, void *, int, const char *), void *userPtr)
{
	TESTPTR(sbcm)
	sbcm->fptrError = fptr;
	sbcm->fptrErrorptr = userPtr;
	return EPHIDGET_OK;
}

int CCONV CPhidgetSBCManager_getAttachedSBCs(CPhidgetSBCManagerHandle sbcm, CPhidgetSBCHandle *sbcArray[], int *count)
{
	TESTPTRS(sbcArray, count)
	TESTPTR(sbcm)

	return EPHIDGET_UNSUPPORTED; 
}


//PhidgetSBC

//Private
int CCONV CPhidgetSBC_areExtraEqual(void *arg1, void *arg2)
{
	CPhidgetSBCHandle sbc1 = (CPhidgetSBCHandle)arg1;
	CPhidgetSBCHandle sbc2 = (CPhidgetSBCHandle)arg2;
	
	TESTPTRS(sbc1, sbc2)
	
	if(!strcmp(sbc1->mac, sbc2->mac) 
	   && !strcmp(sbc1->fversion, sbc2->fversion) 
	   && sbc1->hversion == sbc2->hversion
	   && !strcmp(sbc1->hostname?sbc1->hostname:"", sbc2->hostname?sbc2->hostname:""))
		return PTRUE;
	return PFALSE;
}

int CCONV CPhidgetSBC_areEqual(void *arg1, void *arg2)
{
	CPhidgetSBCHandle sbc1 = (CPhidgetSBCHandle)arg1;
	CPhidgetSBCHandle sbc2 = (CPhidgetSBCHandle)arg2;
	
	if(!sbc1 || !sbc2)
		return PFALSE;
		
	if(!strcmp(sbc1->mac, sbc2->mac))
		return PTRUE;
	return PFALSE;
}

void CCONV CPhidgetSBC_free(void *arg)
{
	CPhidgetSBCHandle sbc = (CPhidgetSBCHandle)arg;
	if (!sbc)
		return;

	CPhidgetRemote_free(sbc->networkInfo);

	free(sbc); sbc = NULL;
	return;
}

//Public
int CCONV CPhidgetSBC_create(CPhidgetSBCHandle *sbc)
{
	CPhidgetSBCHandle sbctemp = 0;
	
	TESTPTR(sbc)
	if(!(sbctemp = (CPhidgetSBCHandle)malloc(sizeof(CPhidgetSBC))))
		return EPHIDGET_NOMEMORY;
	ZEROMEM(sbctemp, sizeof(CPhidgetSBC));
	
	// Version 1 doesn't support hostname variable
	sbctemp->txtver = 1;

	*sbc = sbctemp;
	return EPHIDGET_OK;
}

int CCONV CPhidgetSBC_delete(CPhidgetSBCHandle sbc)
{
	CPhidgetSBC_free(sbc); sbc = NULL;
	return EPHIDGET_OK;
}

int CCONV CPhidgetSBC_getFirmwareVersion(CPhidgetSBCHandle sbc, const char **version)
{
	TESTPTRS(sbc, version)

	*version = (char *)sbc->fversion;
	return EPHIDGET_OK;
}

int CCONV CPhidgetSBC_getHardwareVersion(CPhidgetSBCHandle sbc, int *version)
{
	TESTPTRS(sbc, version)

	*version = sbc->hversion;
	return EPHIDGET_OK;
}

int CCONV CPhidgetSBC_getMacAddress(CPhidgetSBCHandle sbc, const char **mac)
{
	TESTPTRS(sbc, mac)

	*mac = (char *)sbc->mac;
	return EPHIDGET_OK;
}

int CCONV CPhidgetSBC_getDeviceName(CPhidgetSBCHandle sbc, const char **name)
{
	TESTPTRS(sbc, name)

	*name = (char *)sbc->deviceName;
	return EPHIDGET_OK;
}

int CCONV CPhidgetSBC_getHostname(CPhidgetSBCHandle sbc, const char **hostname)
{
	TESTPTRS(sbc, hostname)

#ifdef USE_ZEROCONF
	if(sbc->txtver >= 2)
	{
		refreshZeroconfSBC(sbc);
		*hostname = (char *)sbc->hostname;
		return EPHIDGET_OK;
	}
	else
#endif
	{
		*hostname = NULL;
		return EPHIDGET_UNSUPPORTED;
	}
}

int CCONV CPhidgetSBC_getAddress(CPhidgetSBCHandle sbc, const char **ipAddr)
{
	TESTPTRS(sbc, ipAddr)
	if (!sbc->networkInfo)
		return EPHIDGET_NETWORK_NOTCONNECTED;
	if(!sbc->networkInfo->mdns) //not mdns
	{
		return EPHIDGET_UNEXPECTED;
	}
#ifdef USE_ZEROCONF
	if(getZeroconfHostPort(sbc->networkInfo))
		return EPHIDGET_NETWORK;
	if(!sbc->networkInfo->zeroconf_host)
	{
		return EPHIDGET_NETWORK;
	}
	*ipAddr = (char *)sbc->networkInfo->zeroconf_host;
	return EPHIDGET_OK;
#else
	return EPHIDGET_UNEXPECTED;
#endif
}

int CCONV CPhidgetSBC_getIPAddressList(CPhidgetSBCHandle sbc, long *list, unsigned int *size)
{
	const char *addr;
	struct hostent *addr_lookup;
	CPhidgetSBC_getAddress(sbc, &addr);
	/* this will resolve to an IP address, including .local hostnames (for SBC, because it can't resolve .local hostnames on its own) */
#ifdef ZEROCONF_LOOKUP
	addr_lookup = mdns_gethostbyname(addr);
#else
	addr_lookup = gethostbyname(addr);
#endif
	if (addr_lookup == NULL)
	{
	   return EPHIDGET_UNKNOWNVAL;
	}
	else
	{
	   unsigned int i = 0;
	   while ( addr_lookup -> h_addr_list[i] != NULL && i<*size) {
		   struct in_addr inaddr = *(( struct in_addr*)( addr_lookup -> h_addr_list[i]));
		   list[i] = inaddr.s_addr;
		   i++;
	   }
	   *size = i;
	}

	return EPHIDGET_OK;
}