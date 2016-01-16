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
#include "csocket.h"
#include "csocketevents.h"
#include "cphidgetlist.h"
#include "cphidgetmanager.h"
#include "cphidgetdictionary.h"
#include "cphidgetsbc.h"
#include "zeroconf.h"
#include "dns_sd.h"

#ifdef ZEROCONF_RUNTIME_LINKING
//function prototypes for run-time loaded library
typedef DNSServiceErrorType (DNSSD_API * DNSServiceRegisterType) 
(DNSServiceRef *,DNSServiceFlags,uint32_t,const char *,
 const char *,const char *,const char *,uint16_t,uint16_t,
 const void *,DNSServiceRegisterReply,void *);
typedef DNSServiceErrorType (DNSSD_API * DNSServiceProcessResultType) (DNSServiceRef);
typedef void (DNSSD_API * DNSServiceRefDeallocateType) (DNSServiceRef);
typedef DNSServiceErrorType (DNSSD_API * DNSServiceAddRecordType)
(DNSServiceRef, DNSRecordRef *, DNSServiceFlags, 
 uint16_t, uint16_t, const void *, uint32_t);
typedef DNSServiceErrorType (DNSSD_API * DNSServiceUpdateRecordType)
(DNSServiceRef, DNSRecordRef, DNSServiceFlags, 
 uint16_t, const void *, uint32_t);
typedef DNSServiceErrorType (DNSSD_API * DNSServiceRemoveRecordType)
(DNSServiceRef, DNSRecordRef, DNSServiceFlags);
typedef DNSServiceErrorType (DNSSD_API * DNSServiceBrowseType)
(DNSServiceRef *, DNSServiceFlags, uint32_t, const char *,
 const char *, DNSServiceBrowseReply, void *);
typedef DNSServiceErrorType (DNSSD_API * DNSServiceResolveType)
(DNSServiceRef *, DNSServiceFlags, uint32_t, const char *,
 const char *, const char *, DNSServiceResolveReply, 
 void *context);
typedef DNSServiceErrorType (DNSSD_API * DNSServiceQueryRecordType)
(DNSServiceRef *, DNSServiceFlags, uint32_t, const char *,
 uint16_t, uint16_t, DNSServiceQueryRecordReply, void *context);
typedef int (DNSSD_API * DNSServiceConstructFullNameType)
(char *, const char *, const char *, const char *);
typedef int (DNSSD_API * DNSServiceRefSockFDType) (DNSServiceRef sdRef);
#else
#define DNSServiceRegisterPtr DNSServiceRegister
#define DNSServiceProcessResultPtr DNSServiceProcessResult
#define DNSServiceRefDeallocatePtr DNSServiceRefDeallocate
#define DNSServiceAddRecordPtr DNSServiceAddRecord
#define DNSServiceUpdateRecordPtr DNSServiceUpdateRecord
#define DNSServiceRemoveRecordPtr DNSServiceRemoveRecord
#define DNSServiceBrowsePtr DNSServiceBrowse
#define DNSServiceResolvePtr DNSServiceResolve
#define DNSServiceQueryRecordPtr DNSServiceQueryRecord
#define DNSServiceConstructFullNamePtr DNSServiceConstructFullName
#define DNSServiceRefSockFDPtr DNSServiceRefSockFD
#endif

int Dns_sdInitialized = FALSE;
int Dns_sdBrowsing = FALSE;
int stopBrowsing = FALSE;
DNSServiceRef zeroconf_browse_ws_ref  = NULL;
DNSServiceRef zeroconf_browse_sbc_ref  = NULL;
DNSServiceRef zeroconf_browse_phidget_ref  = NULL;

pthread_t dns_thread = NULL;

#ifdef ZEROCONF_RUNTIME_LINKING

//DNS_SD functions
DNSServiceRegisterType DNSServiceRegisterPtr = NULL;
DNSServiceProcessResultType DNSServiceProcessResultPtr = NULL;
DNSServiceRefDeallocateType DNSServiceRefDeallocatePtr = NULL;
DNSServiceAddRecordType DNSServiceAddRecordPtr = NULL;
DNSServiceUpdateRecordType DNSServiceUpdateRecordPtr = NULL;
DNSServiceRemoveRecordType DNSServiceRemoveRecordPtr = NULL;
DNSServiceBrowseType DNSServiceBrowsePtr = NULL;
DNSServiceResolveType DNSServiceResolvePtr = NULL;
DNSServiceQueryRecordType DNSServiceQueryRecordPtr = NULL;
DNSServiceConstructFullNameType DNSServiceConstructFullNamePtr = NULL;
DNSServiceRefSockFDType DNSServiceRefSockFDPtr = NULL;

#ifdef _WINDOWS
HMODULE dllHandle = NULL;
#elif _LINUX
void *libHandle = NULL;
#endif
#endif


static uint8_t *InternalTXTRecordSearch
(
 uint16_t         txtLen,
 const void       *txtRecord,
 const char       *key,
 unsigned long    *keylen
 )
{
	uint8_t *p = (uint8_t*)txtRecord;
	uint8_t *e = p + txtLen;
	*keylen = (unsigned long) strlen(key);
	while (p<e)
	{
		uint8_t *x = p;
		p += 1 + p[0];
		if (p <= e && *keylen <= x[0] && !strncmp(key, (char*)x+1, *keylen))
			if (*keylen == x[0] || x[1+*keylen] == '=') return(x);
	}
	return(NULL);
}

const void * TXTRecordGetValuePtrPtr
(
 uint16_t         txtLen,
 const void       *txtRecord,
 const char       *key,
 uint8_t          *valueLen
 )
{
	unsigned long keylen;
	uint8_t *item = InternalTXTRecordSearch(txtLen, txtRecord, key, &keylen);
	if (!item || item[0] <= keylen) return(NULL);	// If key not found, or found with no value, return NULL
	*valueLen = (uint8_t)(item[0] - (keylen + 1));
	return (item + 1 + keylen + 1);
}

int dns_callback_thread(void *ptr)
{    
    int nfds;
    fd_set readfds;
    struct timeval tv;
    int result;
	
	int ws_fd = DNSServiceRefSockFDPtr(zeroconf_browse_ws_ref);
	int sbc_fd = DNSServiceRefSockFDPtr(zeroconf_browse_sbc_ref);
	int phidget_fd = DNSServiceRefSockFDPtr(zeroconf_browse_phidget_ref);
	
	nfds = ws_fd;
	if(sbc_fd>nfds)
		nfds = sbc_fd;
	if(phidget_fd > nfds)
		nfds = phidget_fd;
	nfds++;
	
	while (!stopBrowsing)
	{
		FD_ZERO(&readfds);
		FD_SET(ws_fd, &readfds);
		FD_SET(sbc_fd, &readfds);
		FD_SET(phidget_fd, &readfds);
		
		//100ms
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		
		result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
		if (result > 0)
		{
			DNSServiceErrorType err = kDNSServiceErr_NoError;
			if (FD_ISSET(ws_fd, &readfds))
			{
				if ((err = DNSServiceProcessResultPtr(zeroconf_browse_ws_ref)) != kDNSServiceErr_NoError)
					stopBrowsing = 1;
			}
			if (FD_ISSET(sbc_fd, &readfds))
			{
				if ((err = DNSServiceProcessResultPtr(zeroconf_browse_sbc_ref)) != kDNSServiceErr_NoError)
					stopBrowsing = 1;
			}
			if (FD_ISSET(phidget_fd, &readfds))
			{
				if ((err = DNSServiceProcessResultPtr(zeroconf_browse_phidget_ref)) != kDNSServiceErr_NoError)
					stopBrowsing = 1;
			}
		}
		else if(result == SOCKET_ERROR)
		{
			LOG(PHIDGET_LOG_DEBUG, "select(  ) returned %d errno %d %s\n", result, errno, strerror(errno));
			if (errno != EINTR)
				stopBrowsing = 1;
		}
		//result==0 means timeout, loop around again
	}
	if(fptrJavaDetachCurrentThread)
		fptrJavaDetachCurrentThread();
	return EPHIDGET_OK;
}

void PhidFromTXT(CPhidgetHandle phid, uint16_t txtLen, const char *txtRecord)
{
	int i = 0;
	short txtver;
	//char *label = NULL;
	char temp[128];
	//unsigned int labelLen = 0;
	
	uint8_t valLen = 0;
	const char *valPtr = NULL;
	
	//txt version
	if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "txtvers", &valLen))) return;
	memcpy(temp, valPtr, valLen);
	temp[valLen] = '\0';
	txtver = (short)strtol(temp, NULL, 10);
	
	//Serial Number
	if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "serial", &valLen))) return;
	memcpy(temp, valPtr, valLen);
	temp[valLen] = '\0';
	phid->serialNumber = strtol(temp, NULL, 10);
	phid->specificDevice = PHIDGETOPEN_SERIAL;
	
	//version
	if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "version", &valLen))) return;
	memcpy(temp, valPtr, valLen);
	temp[valLen] = '\0';
	phid->deviceVersion = strtol(temp, NULL, 10);
	
	//label
	if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "label", &valLen))) return;
	if(valLen >= MAX_LABEL_STORAGE) valLen = MAX_LABEL_STORAGE-1;
	memcpy(phid->label, valPtr, valLen);
	phid->label[valLen] = '\0';
	/*	memcpy(temp, valPtr, valLen);
	 temp[valLen] = '\0';
	 unescape(temp, &label, &labelLen);
	 if(labelLen > MAX_LABEL_STORAGE-1) labelLen = MAX_LABEL_STORAGE-1;
	 memcpy(phid->label, label, labelLen);
	 phid->label[labelLen] = '\0';
	 free(label);*/
	
	//server_id
	if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "server_id", &valLen))) return;
	free(phid->networkInfo->zeroconf_server_id);
	if(!(phid->networkInfo->zeroconf_server_id = malloc(valLen+1))) return;
	ZEROMEM(phid->networkInfo->zeroconf_server_id, valLen+1);
	memcpy(phid->networkInfo->zeroconf_server_id, valPtr, valLen);
	
	if(txtver >= 3)
	{
		//USB Product String
		if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "usbstr", &valLen))) return;
		memcpy(phid->usbProduct, valPtr, valLen);
	}

	// things added in version 2 of the txt
	if(txtver >= 2)
	{
		//Device ID
		if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "id", &valLen))) return;
		memcpy(temp, valPtr, valLen);
		temp[valLen] = '\0';
		phid->deviceIDSpec = strtol(temp, NULL, 10);
		
		for(i = 1;i<PHIDGET_DEVICE_COUNT;i++)
			if(phid->deviceIDSpec == Phid_Device_Def[i].pdd_sdid) break;
		phid->deviceDef = &Phid_Device_Def[i];
		phid->attr = Phid_Device_Def[i].pdd_attr;
		
		//Device Class
		if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "class", &valLen))) return;
		memcpy(temp, valPtr, valLen);
		temp[valLen] = '\0';
		phid->deviceID = strtol(temp, NULL, 10);
		phid->deviceType = Phid_DeviceName[phid->deviceID];
	}

	//Old version uses string searching, but some devices have the same name with different IDs
	if(txtver == 1)
	{
		char *name = NULL;
		char *type = NULL;
		
		//name
		if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "name", &valLen))) return;
		if(!(name = malloc(valLen+1))) return;
		ZEROMEM(name, valLen+1);
		memcpy(name, valPtr, valLen);
		for(i = 0;i<PHIDGET_DEVICE_COUNT;i++)
		{
			if(!strcmp(name, Phid_Device_Def[i].pdd_name))
			{
				phid->deviceIDSpec = Phid_Device_Def[i].pdd_sdid;
				phid->deviceDef = &Phid_Device_Def[i];
				phid->attr = Phid_Device_Def[i].pdd_attr;
				break;
			}
		}
		free(name);
		
		//type
		if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "type", &valLen))) return;
		if(!(type = malloc(valLen+1))) return;
		ZEROMEM(type, valLen+1);
		memcpy(type, valPtr, valLen);
		phid->deviceID = phidget_type_to_id(type);
		phid->deviceType = Phid_DeviceName[phid->deviceID];
		free(type);
	}
	
	phid->deviceUID = CPhidget_getUID(phid->deviceIDSpec, phid->deviceVersion);

	phid->networkInfo->mdns = PTRUE;
	
}

void DNSServiceQueryRecord_Phidget_CallBack
(
 DNSServiceRef                       DNSServiceRef,
 DNSServiceFlags                     flags,
 uint32_t                            interfaceIndex,
 DNSServiceErrorType                 errorCode,
 const char                          *fullname,
 uint16_t                            rrtype,
 uint16_t                            rrclass,
 uint16_t                            rdlen,
 const void                          *rdata,
 uint32_t                            ttl,
 void                                *context
 )
{
	if (errorCode != kDNSServiceErr_NoError)
		LOG(PHIDGET_LOG_ERROR, "DNSServiceQueryRecord_Phidget_CallBack returned error: %d\n", errorCode);
	else
	{
		CPhidgetHandle phid = (CPhidgetHandle)context;
		LOG(PHIDGET_LOG_INFO, "DNSServiceQueryRecord_Phidget_CallBack: %s (%d)",fullname,interfaceIndex);
		
		PhidFromTXT(phid, rdlen, rdata);
	}
}

void SBCFromTXT(CPhidgetSBCHandle sbc, uint16_t txtLen, const char *txtRecord)
{
	char *hversion = NULL, *txtver = NULL;
	
	uint8_t valLen = 0;
	const char *valPtr = NULL;
	
	//txt version
	if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "txtvers", &valLen))) return;
	if(!(txtver = malloc(valLen+1))) return;
	ZEROMEM(txtver, valLen+1);
	memcpy(txtver, valPtr, valLen);
	sbc->txtver = (short)strtol(txtver, NULL, 10);
	free(txtver);
	
	//Firmware version
	if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "fversion", &valLen))) return;
	if(valLen > 12) valLen = 12;
	memcpy(sbc->fversion, valPtr, valLen);
	sbc->fversion[valLen] = '\0';
	
	//Hardware version
	if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "hversion", &valLen))) return;
	if(!(hversion = malloc(valLen+1))) return;
	ZEROMEM(hversion, valLen+1);
	memcpy(hversion, valPtr, valLen);
	sbc->hversion = (short)strtol(hversion, NULL, 10);
	free(hversion);
	
	// things added in version 2 of the txt
	if(sbc->txtver >= 2)
	{
		//Hostname
		if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "hostname", &valLen))) return;
		if(valLen > 128) valLen = 128;
		memcpy(sbc->hostname, valPtr, valLen);
		sbc->hostname[valLen] = '\0';
	}
	// things added in version 3 of the txt
	if(sbc->txtver >= 3)
	{
		//Device Name
		if(!(valPtr = TXTRecordGetValuePtrPtr(txtLen, txtRecord, "name", &valLen))) return;
		if(valLen > 128) valLen = 128;
		memcpy(sbc->deviceName, valPtr, valLen);
		sbc->deviceName[valLen] = '\0';
	}
	else
	{
		sprintf(sbc->deviceName, "PhidgetSBC");
	}
}

void DNSServiceQueryRecord_SBC_CallBack
(
 DNSServiceRef                       DNSServiceRef,
 DNSServiceFlags                     flags,
 uint32_t                            interfaceIndex,
 DNSServiceErrorType                 errorCode,
 const char                          *fullname,
 uint16_t                            rrtype,
 uint16_t                            rrclass,
 uint16_t                            rdlen,
 const void                          *rdata,
 uint32_t                            ttl,
 void                                *context
 )
{
	if (errorCode != kDNSServiceErr_NoError)
		LOG(PHIDGET_LOG_ERROR, "DNSServiceQueryRecord_sbc_CallBack returned error: %d\n", errorCode);
	else
	{
		CPhidgetSBCHandle sbc = (CPhidgetSBCHandle)context;
		LOG(PHIDGET_LOG_INFO, "DNSServiceQueryRecord_sbc_CallBack: %s (%d)",fullname,interfaceIndex);
		
		SBCFromTXT(sbc, rdlen, rdata);
	}
}

void DNSServiceBrowse_Phidget_CallBack(DNSServiceRef service,
									   DNSServiceFlags flags,
									   uint32_t interfaceIndex,
									   DNSServiceErrorType errorCode,
									   const char * name,
									   const char * type,
									   const char * domain,
									   void * context)
{
	if (errorCode != kDNSServiceErr_NoError)
		LOG(PHIDGET_LOG_ERROR, "Dns_sd_BrowseCallBack returned error: %d\n", errorCode);
	else
	{    
		DNSServiceErrorType error;
		DNSServiceRef  serviceRef;
		char fullname[kDNSServiceMaxDomainName];
		
		CPhidgetHandle phid;
		CPhidgetHandle found_phid;
		CPhidgetManagerList *trav;
		
		DNSServiceConstructFullNamePtr(fullname, name, type, domain);
		
		LOG(PHIDGET_LOG_INFO, "DNSServiceBrowse_Phidget_CallBack: %s (%s, %s, %d)",name,type,domain,interfaceIndex);
		
		if((CPhidget_create(&phid))) return;
		if((CPhidgetRemote_create(&phid->networkInfo))) return;
		
		phid->networkInfo->zeroconf_name = strdup(name);
		phid->networkInfo->zeroconf_type = strdup(type);
		phid->networkInfo->zeroconf_domain = strdup(domain);
		phid->networkInfo->zeroconf_interface = interfaceIndex;
		
		if(flags & kDNSServiceFlagsAdd)
		{
			error = DNSServiceQueryRecordPtr(&serviceRef, 0, interfaceIndex, fullname,
											 kDNSServiceType_TXT, kDNSServiceClass_IN, DNSServiceQueryRecord_Phidget_CallBack, phid);
			if (error == kDNSServiceErr_NoError)
			{
				DNSServiceProcessResultPtr(serviceRef);
				DNSServiceRefDeallocatePtr(serviceRef);
				CThread_mutex_lock(&zeroconfPhidgetsLock);
				CThread_mutex_lock(&activeRemoteManagersLock);
				
				CPhidget_setStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
				CPhidget_setStatusFlag(&phid->status, PHIDGET_REMOTE_FLAG, &phid->lock);
				
				if(CList_findInList((CListHandle)zeroconfPhidgets, phid, CPhidget_areExtraEqual, (void **)&found_phid) == EPHIDGET_OK)
				{
					//Only do a detach/attach cycle if something's different
					if(found_phid->serialNumber == phid->serialNumber
					   && found_phid->deviceVersion == phid->deviceVersion
					   && !strcmp(found_phid->label, phid->label)
					   && !strcmp(found_phid->networkInfo->zeroconf_server_id, phid->networkInfo->zeroconf_server_id))
					{
						//prefer local. domain
						if((strcmp(domain, found_phid->networkInfo->zeroconf_domain) != 0) && !(strcmp(domain, "local.")))
						{
							free(found_phid->networkInfo->zeroconf_domain);
							found_phid->networkInfo->zeroconf_domain = phid->networkInfo->zeroconf_domain;
							phid->networkInfo->zeroconf_domain = NULL;
						}
						CPhidgetRemote_free(phid->networkInfo);
						CPhidget_free(phid);
						CThread_mutex_unlock(&activeRemoteManagersLock);
						CThread_mutex_unlock(&zeroconfPhidgetsLock);
						return;
					}
					
					//set detaching status
					CPhidget_clearStatusFlag(&found_phid->status, PHIDGET_ATTACHED_FLAG, &found_phid->lock);
					CPhidget_setStatusFlag(&found_phid->status, PHIDGET_DETACHING_FLAG, &found_phid->lock);
					
					//Remove from list - don't free until after detach event
					CList_removeFromList((CListHandle *)&zeroconfPhidgets, found_phid, CPhidget_areExtraEqual, FALSE, NULL);
					
					for (trav=activeRemoteManagers; trav; trav = trav->next)
					{
						if(trav->phidm->networkInfo->requested_address==NULL
						   && (trav->phidm->networkInfo->requested_serverID == NULL || !strcmp(trav->phidm->networkInfo->requested_serverID,found_phid->networkInfo->zeroconf_server_id)))
						{
							CList_removeFromList((CListHandle *)&trav->phidm->AttachedPhidgets, found_phid, CPhidget_areExtraEqual, PFALSE, NULL);
							
							if (trav->phidm->fptrDetachChange && trav->phidm->state == PHIDGETMANAGER_ACTIVE)
								trav->phidm->fptrDetachChange((CPhidgetHandle)found_phid, trav->phidm->fptrDetachChangeptr);
						}
					}
					CPhidgetRemote_free(found_phid->networkInfo);
					CPhidget_free(found_phid);
				}
				
				CPhidget_setStatusFlag(&phid->status, PHIDGET_SERVER_CONNECTED_FLAG, &phid->lock);
				
				//now add it
				CList_addToList((CListHandle *)&zeroconfPhidgets, phid, CPhidget_areExtraEqual);
				//managers
				for (trav=activeRemoteManagers; trav; trav = trav->next)
				{
					if(trav->phidm->networkInfo->requested_address==NULL
					   && (trav->phidm->networkInfo->requested_serverID == NULL || !strcmp(trav->phidm->networkInfo->requested_serverID,phid->networkInfo->zeroconf_server_id)))
					{
						CList_addToList((CListHandle *)&trav->phidm->AttachedPhidgets, phid, CPhidget_areExtraEqual);
						
						if (trav->phidm->fptrAttachChange && trav->phidm->state == PHIDGETMANAGER_ACTIVE)
							trav->phidm->fptrAttachChange((CPhidgetHandle)phid, trav->phidm->fptrAttachChangeptr);
					}
				}
				
				CThread_mutex_unlock(&activeRemoteManagersLock);
				CThread_mutex_unlock(&zeroconfPhidgetsLock);
			}
			else
				LOG(PHIDGET_LOG_ERROR, "DNSServiceQueryRecordPtr returned error: %d\n", error);
		}
		else
		{
			//have to fill in phid manually from just the name
			int i;
			char *name_copy;

			//Look to see if this is a 'firmware upgrade' device.
			if(name[0] == '1')
			{
				char *realname = strchr(name, ' ');
				if(!realname)
					return;
				name_copy = strdup(&realname[1]);
			}
			else
				name_copy = strdup(name);
			for(i=0;i<(int)strlen(name_copy);i++)
				if(name_copy[i] == '(') break;
			if(i<=1) return;
			name_copy[strlen(name_copy)-1]='\0';
			name_copy[i-1] = '\0';
			phid->serialNumber = strtol(name_copy+i+1, NULL, 10);
			//we need to set this so it checks the serial numbers for a match
			phid->specificDevice = PHIDGETOPEN_SERIAL;
			for(i = 0;i<PHIDGET_DEVICE_COUNT;i++)
				if(!strcmp(name_copy, Phid_Device_Def[i].pdd_name)) break;
			phid->deviceDef = &Phid_Device_Def[i];
			phid->deviceIDSpec = 0; //Needs to be 0 because the name can be ambiguous (ie 1018/1200) - we will still match on serial/device class, which is enough for uniqueness
			phid->attr = Phid_Device_Def[i].pdd_attr;
			phid->deviceID = Phid_Device_Def[i].pdd_did;
			phid->deviceType = Phid_DeviceName[phid->deviceID];
			phid->networkInfo->mdns = PTRUE;
			
			CThread_mutex_lock(&zeroconfPhidgetsLock);
			CThread_mutex_lock(&activeRemoteManagersLock);
			
			CPhidget_clearStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
			CPhidget_setStatusFlag(&phid->status, PHIDGET_DETACHING_FLAG, &phid->lock);
			
			if(!CList_findInList((CListHandle)zeroconfPhidgets, phid, CPhidget_areEqual, (void **)&found_phid))
			{
				CPhidget_clearStatusFlag(&found_phid->status, PHIDGET_ATTACHED_FLAG, &found_phid->lock);
				CPhidget_setStatusFlag(&found_phid->status, PHIDGET_DETACHING_FLAG, &found_phid->lock);
				CPhidget_setStatusFlag(&found_phid->status, PHIDGET_REMOTE_FLAG, &found_phid->lock);
				CPhidget_clearStatusFlag(&found_phid->status, PHIDGET_SERVER_CONNECTED_FLAG, &found_phid->lock);
				
				CList_removeFromList((CListHandle *)&zeroconfPhidgets, found_phid, CPhidget_areExtraEqual, FALSE, NULL);
				//managers
				for (trav=activeRemoteManagers; trav; trav = trav->next)
				{
					if(trav->phidm->networkInfo->requested_address==NULL
					   && (trav->phidm->networkInfo->requested_serverID == NULL || !strcmp(trav->phidm->networkInfo->requested_serverID,found_phid->networkInfo->zeroconf_server_id)))
					{
						CList_removeFromList((CListHandle *)&trav->phidm->AttachedPhidgets, found_phid, CPhidget_areExtraEqual, PFALSE, NULL);
						
						if (trav->phidm->fptrDetachChange && trav->phidm->state == PHIDGETMANAGER_ACTIVE)
							trav->phidm->fptrDetachChange((CPhidgetHandle)found_phid, trav->phidm->fptrDetachChangeptr);
					}
				}
				CPhidget_clearStatusFlag(&found_phid->status, PHIDGET_DETACHING_FLAG, &found_phid->lock);
				CPhidgetRemote_free(found_phid->networkInfo);
				CPhidget_free(found_phid);
			}
			CPhidgetRemote_free(phid->networkInfo);
			CPhidget_free(phid);
			
			CThread_mutex_unlock(&activeRemoteManagersLock);
			CThread_mutex_unlock(&zeroconfPhidgetsLock);
			free(name_copy);
		}
	}
}

void DNSServiceBrowse_sbc_CallBack(DNSServiceRef service,
								   DNSServiceFlags flags,
								   uint32_t interfaceIndex,
								   DNSServiceErrorType errorCode,
								   const char * name,
								   const char * type,
								   const char * domain,
								   void * context)
{
	if (errorCode != kDNSServiceErr_NoError)
		LOG(PHIDGET_LOG_ERROR, "Dns_sbc_BrowseCallBack returned error: %d\n", errorCode);
	else
	{    
		DNSServiceErrorType error;
		DNSServiceRef  serviceRef;
		char fullname[kDNSServiceMaxDomainName];
		
		CPhidgetSBCHandle sbc;
		CPhidgetSBCHandle found_sbc;
		CPhidgetSBCManagerList *trav;
		
		DNSServiceConstructFullNamePtr(fullname, name, type, domain);
		
		
		if((CPhidgetSBC_create(&sbc))) return;
		if((CPhidgetRemote_create(&sbc->networkInfo))) return;
		
		sbc->networkInfo->zeroconf_name = strdup(name);
		sbc->networkInfo->zeroconf_type = strdup(type);
		sbc->networkInfo->zeroconf_domain = strdup(domain);
		sbc->networkInfo->zeroconf_interface = interfaceIndex;
		sbc->networkInfo->mdns = PTRUE;
		
		strncpy(sbc->mac, name+12, 18); //name == 'PhidgetSBC (??:??:??:??:??:??)'
		sbc->mac[17] = '\0';
		
		if(flags & kDNSServiceFlagsAdd)
		{
			LOG(PHIDGET_LOG_INFO, "DNSServiceBrowse_sbc_CallBack (Add): %s",name);
			error = DNSServiceQueryRecordPtr(&serviceRef, 0, interfaceIndex, fullname,
											 kDNSServiceType_TXT, kDNSServiceClass_IN, DNSServiceQueryRecord_SBC_CallBack, sbc);
			if (error == kDNSServiceErr_NoError)
			{
				DNSServiceProcessResultPtr(serviceRef);
				DNSServiceRefDeallocatePtr(serviceRef);
				
				CThread_mutex_lock(&zeroconfSBCsLock);
				CThread_mutex_lock(&activeSBCManagersLock);
				
				//Check if it's in the list and if it's different, remove it to make way for the new one
				// (Sometimes, we don't get a proper detach notification)
				if(CList_findInList((CListHandle)zeroconfSBCs, sbc, CPhidgetSBC_areEqual, (void **)&found_sbc) == EPHIDGET_OK)
				{
					if(CPhidgetSBC_areExtraEqual(found_sbc, sbc) != PTRUE) //A version number has changed
					{
						//make sure zeroconf_ref is not pending
						if(found_sbc->networkInfo)
						{
							CThread_mutex_lock(&found_sbc->networkInfo->zeroconf_ref_lock);
							if(found_sbc->networkInfo->zeroconf_ref)
							{
								DNSServiceRefDeallocatePtr((DNSServiceRef)found_sbc->networkInfo->zeroconf_ref);
								found_sbc->networkInfo->zeroconf_ref = NULL;
							}
							CThread_mutex_unlock(&found_sbc->networkInfo->zeroconf_ref_lock);
						}
						
						//Remove from list - don't free until after detach event
						CList_removeFromList((CListHandle *)&zeroconfSBCs, found_sbc, CPhidgetSBC_areEqual, PFALSE, NULL);
						
						for (trav=activeSBCManagers; trav; trav = trav->next)
						{
							if (trav->sbcm->fptrDetachChange && trav->sbcm->state == PHIDGETMANAGER_ACTIVE)
								trav->sbcm->fptrDetachChange((CPhidgetSBCHandle)found_sbc, trav->sbcm->fptrDetachChangeptr);
						}
						
						CPhidgetSBC_free(found_sbc);
						
						//now we fall through and add back to new one
					}
					else //Nothing has changed, we didn't remove, don't add
					{
						//prefer local. domain
						if((strcmp(domain, found_sbc->networkInfo->zeroconf_domain) != 0) && !(strcmp(domain, "local.")))
						{
							free(found_sbc->networkInfo->zeroconf_domain);
							found_sbc->networkInfo->zeroconf_domain = sbc->networkInfo->zeroconf_domain;
							sbc->networkInfo->zeroconf_domain = NULL;
						}
						CPhidgetSBC_free(sbc);
						goto dontadd;
					}
				}
				
				//now add it
				CList_addToList((CListHandle *)&zeroconfSBCs, sbc, CPhidgetSBC_areEqual);
				
				//send out events
				for (trav=activeSBCManagers; trav; trav = trav->next)
				{
					if (trav->sbcm->fptrAttachChange && trav->sbcm->state == PHIDGETMANAGER_ACTIVE)
						trav->sbcm->fptrAttachChange((CPhidgetSBCHandle)sbc, trav->sbcm->fptrAttachChangeptr);
					
				}
			dontadd:
				
				CThread_mutex_unlock(&activeSBCManagersLock);
				CThread_mutex_unlock(&zeroconfSBCsLock);
			}
			else
				LOG(PHIDGET_LOG_ERROR, "DNSServiceQueryRecordPtr returned error: %d\n", error);
		}
		else
		{
			
			LOG(PHIDGET_LOG_INFO, "DNSServiceBrowse_sbc_CallBack (Remove): %s",name);
			CThread_mutex_lock(&zeroconfSBCsLock);
			CThread_mutex_lock(&activeSBCManagersLock);
			
			//make sure zeroconf_ref is not pending
			if(sbc->networkInfo)
			{
				CThread_mutex_lock(&sbc->networkInfo->zeroconf_ref_lock);
				if(sbc->networkInfo->zeroconf_ref)
				{
					DNSServiceRefDeallocatePtr((DNSServiceRef)sbc->networkInfo->zeroconf_ref);
					sbc->networkInfo->zeroconf_ref = NULL;
				}
				CThread_mutex_unlock(&sbc->networkInfo->zeroconf_ref_lock);
			}
			
			if(CList_findInList((CListHandle)zeroconfSBCs, sbc, CPhidgetSBC_areEqual, (void **)&found_sbc) == EPHIDGET_OK)
			{
				CList_removeFromList((CListHandle *)&zeroconfSBCs, found_sbc, CPhidgetSBC_areEqual, PFALSE, NULL);
				//managers
				for (trav=activeSBCManagers; trav; trav = trav->next)
				{
					if (trav->sbcm->fptrDetachChange && trav->sbcm->state == PHIDGETMANAGER_ACTIVE)
						trav->sbcm->fptrDetachChange((CPhidgetSBCHandle)found_sbc, trav->sbcm->fptrDetachChangeptr);
				}
				CPhidgetSBC_free(found_sbc);
			}
			
			CThread_mutex_unlock(&activeSBCManagersLock);
			CThread_mutex_unlock(&zeroconfSBCsLock);
			
			CPhidgetSBC_free(sbc);
		}
	}
}

void DNSServiceBrowse_ws_CallBack(DNSServiceRef service,
								  DNSServiceFlags flags,
								  uint32_t interfaceIndex,
								  DNSServiceErrorType errorCode,
								  const char * name,
								  const char * type,
								  const char * domain,
								  void * context)
{
	if (errorCode != kDNSServiceErr_NoError)
		LOG(PHIDGET_LOG_ERROR, "DNSServiceBrowse_ws_CallBack returned error: %d\n", errorCode);
	else
	{
		char fullname[kDNSServiceMaxDomainName];
		
		CPhidgetRemoteHandle networkInfo;
		if((CPhidgetRemote_create(&networkInfo))) return;
		
		networkInfo->zeroconf_name = strdup(name);
		networkInfo->zeroconf_server_id = strdup(name);
		networkInfo->zeroconf_type = strdup(type);
		networkInfo->zeroconf_domain = strdup(domain);
		networkInfo->zeroconf_interface = interfaceIndex;
		
		DNSServiceConstructFullNamePtr(fullname, name, type, domain);
		
		LOG(PHIDGET_LOG_INFO, "DNSServiceBrowse_ws_CallBack: %s (%s, %s, %d)",name,type,domain,interfaceIndex);
		
		CThread_mutex_lock(&zeroconfServersLock);
		if(flags & kDNSServiceFlagsAdd)
		{
			if(CList_addToList((CListHandle *)&zeroconfServers, networkInfo, CPhidgetRemote_areEqual) != EPHIDGET_OK)
				CPhidgetRemote_free(networkInfo);
		}
		else
		{
			CList_removeFromList((CListHandle *)&zeroconfServers, networkInfo, CPhidgetRemote_areEqual, TRUE, CPhidgetRemote_free);
		}
		CThread_mutex_unlock(&zeroconfServersLock);
	}
}

int cancelPendingZeroconfLookups(CPhidgetRemoteHandle networkInfo)
{	
	if(networkInfo->zeroconf_ref != NULL)
	{
		DNSServiceRefDeallocatePtr((DNSServiceRef)networkInfo->zeroconf_ref);
		networkInfo->zeroconf_ref = NULL;
	}
	return EPHIDGET_OK;
}


void DNSServiceQueryRecord_CallBack
(
 DNSServiceRef                       DNSServiceRef,
 DNSServiceFlags                     flags,
 uint32_t                            interfaceIndex,
 DNSServiceErrorType                 errorCode,
 const char                          *fullname,
 uint16_t                            rrtype,
 uint16_t                            rrclass,
 uint16_t                            rdlen,
 const void                          *rdata,
 uint32_t                            ttl,
 void                                *context
 )
{
	
	CPhidgetRemoteHandle networkInfo = (CPhidgetRemoteHandle)context;
	
	if (errorCode != kDNSServiceErr_NoError)
	{
		LOG(PHIDGET_LOG_ERROR, "DNSServiceQueryRecord_CallBack returned error: %d\n", errorCode);
		networkInfo->zeroconf_ipaddr = NULL;
	}
	else if(networkInfo->zeroconf_ipaddr == NULL)
	{
		if(rrtype == kDNSServiceType_A && rdlen == 4)
		{
			struct in_addr ip;
			memcpy(&ip, rdata, 4);
			
			networkInfo->zeroconf_ipaddr = strdup(inet_ntoa(ip));
		}
		//NOTE: Windows doesn't have inet_ntop
		/*
		else if(rrtype == kDNSServiceType_AAAA && rdlen == 16)
		{
			struct in6_addr ip;
			memcpy(&ip, rdata, 16);
			
			if((networkInfo->zeroconf_ipaddr = malloc(200)) != NULL)
			{
				networkInfo->zeroconf_ipaddr = strdup(inet_ntop(AF_INET6, &ip, networkInfo->zeroconf_ipaddr, 200));
			}
		}
		*/

		LOG(PHIDGET_LOG_INFO, "DNSServiceQueryRecord_CallBack: %s (%s %d)",fullname, networkInfo->zeroconf_ipaddr, interfaceIndex);
	}
	else
		LOG(PHIDGET_LOG_INFO, "DNSServiceQueryRecord_CallBack (extra, not used): %s (%d)",fullname, interfaceIndex);
}

int getZeroconfIPAddr(CPhidgetRemoteHandle networkInfo)
{
	DNSServiceErrorType error;
	
	LOG(PHIDGET_LOG_VERBOSE, "Resolving IP Address for: %s, %s, %s, %s, %d",
		networkInfo->zeroconf_name, //name
		networkInfo->zeroconf_type, // service type
		networkInfo->zeroconf_domain, //domain
		networkInfo->zeroconf_host,
		networkInfo->zeroconf_interface);
	
	//already a lookup pending? cancel it and start a new one...
	CThread_mutex_lock(&networkInfo->zeroconf_ref_lock);
	if(networkInfo->zeroconf_ref != NULL)
	{
		DNSServiceRefDeallocatePtr((DNSServiceRef)networkInfo->zeroconf_ref);
		networkInfo->zeroconf_ref = NULL;
	}
	
	free(networkInfo->zeroconf_ipaddr);
	networkInfo->zeroconf_ipaddr = NULL;
	
	error = DNSServiceQueryRecordPtr((DNSServiceRef *)&networkInfo->zeroconf_ref,
									 0,  // no flags
									 networkInfo->zeroconf_interface,
									 networkInfo->zeroconf_host, //hostname
									 kDNSServiceType_A, // IPv4 address for now..
									 kDNSServiceClass_IN, // service class
									 DNSServiceQueryRecord_CallBack,
									 networkInfo);
	if (error != kDNSServiceErr_NoError)
	{
		LOG(PHIDGET_LOG_ERROR, "DNSServiceQueryRecordPtr returned error: %d\n", error);
		networkInfo->zeroconf_ref = NULL;
		CThread_mutex_unlock(&networkInfo->zeroconf_ref_lock);
		return EPHIDGET_NETWORK;
	}
	else
	{
		DNSServiceProcessResultPtr((DNSServiceRef)networkInfo->zeroconf_ref);
		DNSServiceRefDeallocatePtr((DNSServiceRef)networkInfo->zeroconf_ref);
		networkInfo->zeroconf_ref = NULL;
		CThread_mutex_unlock(&networkInfo->zeroconf_ref_lock);
		LOG(PHIDGET_LOG_VERBOSE, "Successfull lookup: %s",networkInfo->zeroconf_host);
		return EPHIDGET_OK;
	}
	
}

void DNSServiceResolve_CallBack(
								DNSServiceRef						sdRef,
								DNSServiceFlags                     flags,
								uint32_t                            interfaceIndex,
								DNSServiceErrorType                 errorCode,
								const char                          *fullname,
								const char                          *hosttarget,
								uint16_t                            port,
								uint16_t                            txtLen,
								const unsigned char					*txtRecord,
								void                                *context)
{	
	CPhidgetRemoteHandle networkInfo = (CPhidgetRemoteHandle)context;
	
	
	if (errorCode != kDNSServiceErr_NoError)
	{
		LOG(PHIDGET_LOG_ERROR, "DNSServiceResolve_CallBack returned error: %d\n", errorCode);
		networkInfo->zeroconf_host = NULL;
		networkInfo->zeroconf_port = NULL;
	}
	else if(!networkInfo->zeroconf_host && !networkInfo->zeroconf_port)
	{
		
		LOG(PHIDGET_LOG_INFO, "DNSServiceResolve_CallBack: %s (%s:%d %d)",fullname, hosttarget, ntohs(port), interfaceIndex);
		
		networkInfo->zeroconf_host = strdup(hosttarget);
		networkInfo->zeroconf_port = malloc(10);
		snprintf(networkInfo->zeroconf_port, 10, "%d", ntohs(port));
	}
	else
		LOG(PHIDGET_LOG_INFO, "DNSServiceResolve_CallBack (extra, not used): %s (%s:%d %d)",fullname, hosttarget, ntohs(port), interfaceIndex);
}

int getZeroconfHostPort(CPhidgetRemoteHandle networkInfo)
{
	DNSServiceErrorType error;
	
	LOG(PHIDGET_LOG_VERBOSE, "Resolving host:port for: %s, %s, %s",
		networkInfo->zeroconf_name, //name
		networkInfo->zeroconf_type, // service type
		networkInfo->zeroconf_domain //domain
		);
	
	//already a lookup pending? cancel it and start a new one...
	CThread_mutex_lock(&networkInfo->zeroconf_ref_lock);
	if(networkInfo->zeroconf_ref != NULL)
	{
		DNSServiceRefDeallocatePtr((DNSServiceRef)networkInfo->zeroconf_ref);
		networkInfo->zeroconf_ref = NULL;
	}
	
	free(networkInfo->zeroconf_host);
	networkInfo->zeroconf_host = NULL;
	free(networkInfo->zeroconf_port);
	networkInfo->zeroconf_port = NULL;
	
	error = DNSServiceResolvePtr((DNSServiceRef *)&networkInfo->zeroconf_ref,
								 0,  // no flags
								 0,  // all interfaces
								 //networkInfo->zeroconf_interface,  // interface this service was discovered on
								 networkInfo->zeroconf_name, //name
								 networkInfo->zeroconf_type, // service type
								 networkInfo->zeroconf_domain, //domain
								 DNSServiceResolve_CallBack,
								 networkInfo);     // no context
	if (error != kDNSServiceErr_NoError)
	{
		LOG(PHIDGET_LOG_ERROR, "getZeroconfHostPort returned error: %d\n", error);
		networkInfo->zeroconf_ref = NULL;
		CThread_mutex_unlock(&networkInfo->zeroconf_ref_lock);
		return EPHIDGET_NETWORK;
	}
	else
	{
		DNSServiceProcessResultPtr((DNSServiceRef)networkInfo->zeroconf_ref);
		DNSServiceRefDeallocatePtr((DNSServiceRef)networkInfo->zeroconf_ref);
		networkInfo->zeroconf_ref = NULL;
		LOG(PHIDGET_LOG_VERBOSE, "Successfull host:port lookup: %s:%s",networkInfo->zeroconf_host,networkInfo->zeroconf_port);
		CThread_mutex_unlock(&networkInfo->zeroconf_ref_lock);
		//Don't bother - we aren't going to use it anyways
		//This is just another way to lookup the IP address - from the DNS record
		//getZeroconfIPAddr(networkInfo);
		return EPHIDGET_OK;
	}
}

int refreshZeroconf(CPhidgetRemoteHandle networkInfo, DNSServiceQueryRecordReply callBack, void *userPtr)
{
	DNSServiceErrorType error;
	char fullname[kDNSServiceMaxDomainName];
	
	LOG(PHIDGET_LOG_VERBOSE, "Refreshing zeroconf on: 0x%x",networkInfo);
	
	//already a lookup pending? cancel it and start a new one...
	CThread_mutex_lock(&networkInfo->zeroconf_ref_lock);
	
	DNSServiceConstructFullNamePtr(fullname, networkInfo->zeroconf_name, networkInfo->zeroconf_type, networkInfo->zeroconf_domain);
	
	if(networkInfo->zeroconf_ref != NULL)
	{
		DNSServiceRefDeallocatePtr((DNSServiceRef)networkInfo->zeroconf_ref);
		networkInfo->zeroconf_ref = NULL;
	}
	error = DNSServiceQueryRecordPtr((DNSServiceRef *)&networkInfo->zeroconf_ref,
									 0,  // no flags
									 networkInfo->zeroconf_interface,
									 fullname, //name
									 kDNSServiceType_TXT, // service type
									 kDNSServiceClass_IN, // service class
									 callBack,
									 userPtr);
	if (error != kDNSServiceErr_NoError)
	{
		LOG(PHIDGET_LOG_ERROR, "DNSServiceQueryRecordPtr returned error: %d\n", error);
		networkInfo->zeroconf_ref = NULL;
		CThread_mutex_unlock(&networkInfo->zeroconf_ref_lock);
		return EPHIDGET_NETWORK;
	}
	else
	{
		DNSServiceProcessResultPtr((DNSServiceRef)networkInfo->zeroconf_ref);
		DNSServiceRefDeallocatePtr((DNSServiceRef)networkInfo->zeroconf_ref);
		networkInfo->zeroconf_ref = NULL;
		CThread_mutex_unlock(&networkInfo->zeroconf_ref_lock);
		LOG(PHIDGET_LOG_VERBOSE, "Successfull refresh.");
		return EPHIDGET_OK;
	}
}

int refreshZeroconfSBC(CPhidgetSBCHandle sbc)
{
	return refreshZeroconf(sbc->networkInfo, DNSServiceQueryRecord_SBC_CallBack, sbc);
}

int refreshZeroconfPhidget(CPhidgetHandle phid)
{
	return refreshZeroconf(phid->networkInfo, DNSServiceQueryRecord_Phidget_CallBack, phid);
}

int InitializeZeroconf()
{
	DNSServiceErrorType error;
	CThread_mutex_lock(&zeroconfInitLock);
	if(!Dns_sdInitialized)
	{
#ifdef ZEROCONF_RUNTIME_LINKING
		
#ifdef _WINDOWS
		if(!(dllHandle = LoadLibrary(L"dnssd.dll")))
		{
			DWORD error = GetLastError();
			switch(error)
			{
				case ERROR_MOD_NOT_FOUND:
					LOG(PHIDGET_LOG_DEBUG,"LoadLibrary failed - module could not be found");
					break;
				default:
					LOG(PHIDGET_LOG_DEBUG,"LoadLibrary failed with error code: %d", error);
			}
			CThread_mutex_unlock(&zeroconfInitLock);
			return EPHIDGET_UNEXPECTED;
		}
		
		// If the handle is valid, try to get the function address. 
		if (NULL != dllHandle) 
		{ 
			//Get pointers to our functions using GetProcAddress:
#ifdef WINCE
			DNSServiceRegisterPtr = (DNSServiceRegisterType)GetProcAddress(dllHandle, L"DNSServiceRegister");
			DNSServiceProcessResultPtr = (DNSServiceProcessResultType)GetProcAddress(dllHandle, L"DNSServiceProcessResult");
			DNSServiceRefDeallocatePtr = (DNSServiceRefDeallocateType)GetProcAddress(dllHandle, L"DNSServiceRefDeallocate");
			DNSServiceAddRecordPtr = (DNSServiceAddRecordType)GetProcAddress(dllHandle, L"DNSServiceAddRecord");
			DNSServiceUpdateRecordPtr = (DNSServiceUpdateRecordType)GetProcAddress(dllHandle, L"DNSServiceUpdateRecord");
			DNSServiceRemoveRecordPtr = (DNSServiceRemoveRecordType)GetProcAddress(dllHandle, L"DNSServiceRemoveRecord");
			DNSServiceBrowsePtr = (DNSServiceBrowseType)GetProcAddress(dllHandle, L"DNSServiceBrowse");
			DNSServiceResolvePtr = (DNSServiceResolveType)GetProcAddress(dllHandle, L"DNSServiceResolve");
			DNSServiceQueryRecordPtr = (DNSServiceQueryRecordType)GetProcAddress(dllHandle, L"DNSServiceQueryRecord");
			DNSServiceConstructFullNamePtr = (DNSServiceConstructFullNameType)GetProcAddress(dllHandle, L"DNSServiceConstructFullName");
			DNSServiceRefSockFDPtr = (DNSServiceRefSockFDType)GetProcAddress(dllHandle, L"DNSServiceRefSockFD");
#else
			DNSServiceRegisterPtr = (DNSServiceRegisterType)GetProcAddress(dllHandle, "DNSServiceRegister");
			DNSServiceProcessResultPtr = (DNSServiceProcessResultType)GetProcAddress(dllHandle, "DNSServiceProcessResult");
			DNSServiceRefDeallocatePtr = (DNSServiceRefDeallocateType)GetProcAddress(dllHandle, "DNSServiceRefDeallocate");
			DNSServiceAddRecordPtr = (DNSServiceAddRecordType)GetProcAddress(dllHandle, "DNSServiceAddRecord");
			DNSServiceUpdateRecordPtr = (DNSServiceUpdateRecordType)GetProcAddress(dllHandle, "DNSServiceUpdateRecord");
			DNSServiceRemoveRecordPtr = (DNSServiceRemoveRecordType)GetProcAddress(dllHandle, "DNSServiceRemoveRecord");
			DNSServiceBrowsePtr = (DNSServiceBrowseType)GetProcAddress(dllHandle, "DNSServiceBrowse");
			DNSServiceResolvePtr = (DNSServiceResolveType)GetProcAddress(dllHandle, "DNSServiceResolve");
			DNSServiceQueryRecordPtr = (DNSServiceQueryRecordType)GetProcAddress(dllHandle, "DNSServiceQueryRecord");
			DNSServiceConstructFullNamePtr = (DNSServiceConstructFullNameType)GetProcAddress(dllHandle, "DNSServiceConstructFullName");
			DNSServiceRefSockFDPtr = (DNSServiceRefSockFDType)GetProcAddress(dllHandle, "DNSServiceRefSockFD");
#endif
			
			Dns_sdInitialized = (
								 NULL != DNSServiceRegisterPtr && 
								 NULL != DNSServiceProcessResultPtr &&
								 NULL != DNSServiceRefDeallocatePtr &&
								 NULL != DNSServiceAddRecordPtr &&
								 NULL != DNSServiceUpdateRecordPtr &&
								 NULL != DNSServiceRemoveRecordPtr &&
								 NULL != DNSServiceQueryRecordPtr &&
								 NULL != DNSServiceConstructFullNamePtr &&
								 NULL != DNSServiceRefSockFDPtr);
		}
		
		if(!Dns_sdInitialized)
		{
			LOG(PHIDGET_LOG_DEBUG,"InitializeZeroconf failed somehow...");
			CThread_mutex_unlock(&zeroconfInitLock);
			return EPHIDGET_UNSUPPORTED;
		}
		
#elif _LINUX
		libHandle = dlopen("libdns_sd.so",RTLD_LAZY);
		if(!libHandle)
		{
			LOG(PHIDGET_LOG_WARNING, "dlopen failed with error: %s", dlerror());
			LOG(PHIDGET_LOG_WARNING, "Assuming that zeroconf is not supported on this machine.");
			CThread_mutex_unlock(&zeroconfInitLock);
			return EPHIDGET_UNSUPPORTED;
		}
		
		//Get pointers to our functions using dlsym:
		if(!(DNSServiceRegisterPtr = (DNSServiceRegisterType)dlsym(libHandle, "DNSServiceRegister"))) goto dlsym_err;
		if(!(DNSServiceProcessResultPtr = (DNSServiceProcessResultType)dlsym(libHandle, "DNSServiceProcessResult"))) goto dlsym_err;
		if(!(DNSServiceRefDeallocatePtr = (DNSServiceRefDeallocateType)dlsym(libHandle, "DNSServiceRefDeallocate"))) goto dlsym_err;
		if(!(DNSServiceAddRecordPtr = (DNSServiceAddRecordType)dlsym(libHandle, "DNSServiceAddRecord"))) goto dlsym_err;
		if(!(DNSServiceUpdateRecordPtr = (DNSServiceUpdateRecordType)dlsym(libHandle, "DNSServiceUpdateRecord"))) goto dlsym_err;
		if(!(DNSServiceRemoveRecordPtr = (DNSServiceRemoveRecordType)dlsym(libHandle, "DNSServiceRemoveRecord"))) goto dlsym_err;
		if(!(DNSServiceBrowsePtr = (DNSServiceBrowseType)dlsym(libHandle, "DNSServiceBrowse"))) goto dlsym_err;
		if(!(DNSServiceResolvePtr = (DNSServiceResolveType)dlsym(libHandle, "DNSServiceResolve"))) goto dlsym_err;
		if(!(DNSServiceQueryRecordPtr = (DNSServiceQueryRecordType)dlsym(libHandle, "DNSServiceQueryRecord"))) goto dlsym_err;
		if(!(DNSServiceConstructFullNamePtr = (DNSServiceConstructFullNameType)dlsym(libHandle, "DNSServiceConstructFullName"))) goto dlsym_err;
		if(!(DNSServiceRefSockFDPtr = (DNSServiceRefSockFDType)dlsym(libHandle, "DNSServiceRefSockFD"))) goto dlsym_err;
		
		goto dlsym_good;
		
	dlsym_err:
		LOG(PHIDGET_LOG_WARNING, "dlsym failed with error: %s", dlerror());
		LOG(PHIDGET_LOG_WARNING, "Assuming that zeroconf is not supported on this machine.");
		CThread_mutex_unlock(&zeroconfInitLock);
		return EPHIDGET_UNSUPPORTED;
		
	dlsym_good:
		Dns_sdInitialized = TRUE;
#endif
		
#else
		Dns_sdInitialized = TRUE;
#endif
		LOG(PHIDGET_LOG_DEBUG,"InitializeZeroconf - System supports zeroconf.");
	}
	if(!Dns_sdBrowsing)
	{
		error = DNSServiceBrowsePtr(&zeroconf_browse_ws_ref,
									0,                // no flags
									0,                // all network interfaces
									"_phidget_ws._tcp",     // service type
									"",               // default domains
									DNSServiceBrowse_ws_CallBack, // call back function
									NULL);            // no context
		if (error != kDNSServiceErr_NoError)
		{
			LOG(PHIDGET_LOG_ERROR,"DNSServiceBrowse on _phidget_ws._tcp failed: %d", error);
			CThread_mutex_unlock(&zeroconfInitLock);
			return EPHIDGET_TRYAGAIN;
		}
		
		error = DNSServiceBrowsePtr(&zeroconf_browse_sbc_ref,
									0,                // no flags
									0,                // all network interfaces
									"_phidget_sbc._tcp",     // service type
									"",               // default domains
									DNSServiceBrowse_sbc_CallBack, // call back function
									NULL);            // no context
		if (error != kDNSServiceErr_NoError)
		{
			LOG(PHIDGET_LOG_ERROR,"DNSServiceBrowse on _phidget_sbc._tcp failed: %d", error);
			CThread_mutex_unlock(&zeroconfInitLock);
			return EPHIDGET_TRYAGAIN;
		}
		
		error = DNSServiceBrowsePtr(&zeroconf_browse_phidget_ref,
									0,                // no flags
									0,                // all network interfaces
									"_phidget._tcp",     // service type
									"",               // default domains
									DNSServiceBrowse_Phidget_CallBack, // call back function
									NULL);            // no context
		if (error != kDNSServiceErr_NoError)
		{
			LOG(PHIDGET_LOG_ERROR,"DNSServiceBrowse on _phidget._tcp failed: %d", error);
			CThread_mutex_unlock(&zeroconfInitLock);
			return EPHIDGET_TRYAGAIN;
		}
		
		stopBrowsing = FALSE;
		pthread_create(&dns_thread, NULL, (void *(*)(void *))dns_callback_thread,NULL);
		
		Dns_sdBrowsing = PTRUE;
		LOG(PHIDGET_LOG_DEBUG,"InitializeZeroconf - Zeroconf browsing active.");
	}
	CThread_mutex_unlock(&zeroconfInitLock);
	return EPHIDGET_OK;
}

int UninitializeZeroconf()
{
	void *status;
	CThread_mutex_lock(&zeroconfInitLock);
	if(Dns_sdBrowsing)
	{
		stopBrowsing = TRUE;
		if(dns_thread)
		{
			pthread_join(dns_thread, &status);
			dns_thread = NULL;
		}
		
		if(zeroconf_browse_ws_ref)
		{
			DNSServiceRefDeallocatePtr(zeroconf_browse_ws_ref);
			zeroconf_browse_ws_ref = NULL;
		}
		if(zeroconf_browse_sbc_ref)
		{
			DNSServiceRefDeallocatePtr(zeroconf_browse_sbc_ref);
			zeroconf_browse_sbc_ref = NULL;
		}
		if(zeroconf_browse_phidget_ref)
		{
			DNSServiceRefDeallocatePtr(zeroconf_browse_phidget_ref);
			zeroconf_browse_phidget_ref = NULL;
		}
		
		CThread_mutex_lock(&zeroconfPhidgetsLock);
		CList_emptyList((CListHandle *)&zeroconfPhidgets, FALSE, NULL);
		CThread_mutex_unlock(&zeroconfPhidgetsLock);
		
		CThread_mutex_lock(&zeroconfSBCsLock);
		CList_emptyList((CListHandle *)&zeroconfSBCs, FALSE, NULL);
		CThread_mutex_unlock(&zeroconfSBCsLock);
		
		CThread_mutex_lock(&zeroconfServersLock);
		CList_emptyList((CListHandle *)&zeroconfServers, FALSE, NULL);
		CThread_mutex_unlock(&zeroconfServersLock);
		
		Dns_sdBrowsing = FALSE;
	}
	CThread_mutex_unlock(&zeroconfInitLock);
	return EPHIDGET_OK;
}


