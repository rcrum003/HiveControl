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

#include "avahi-client/client.h"
#include "avahi-client/lookup.h"

#include <avahi-common/simple-watch.h>
#include "avahi-common/malloc.h"
#include "avahi-common/error.h"
#include "avahi-common/domain.h"

static int UninitializeZeroconf1(int lock);

#ifdef ZEROCONF_RUNTIME_LINKING
typedef AvahiClient * (* avahi_client_new_type) (
    const AvahiPoll *poll_api /**< The abstract event loop API to use */,
    AvahiClientFlags flags /**< Some flags to modify the behaviour of  the client library */,
    AvahiClientCallback callback /**< A callback that is called whenever the state of the client changes. This may be NULL */,
    void *userdata /**< Some arbitrary user data pointer that will be passed to the callback function */,
    int *error /**< If creation of the client fails, this integer will contain the error cause. May be NULL if you aren't interested in the reason why avahi_client_new() failed. */);
typedef void (* avahi_client_free_type)(AvahiClient *client);
typedef const char * (* avahi_client_get_host_name_type) (AvahiClient *);
typedef AvahiServiceBrowser * (* avahi_service_browser_new_type) (
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *type,
    const char *domain,
    AvahiLookupFlags flags,
    AvahiServiceBrowserCallback callback,
    void *userdata);
typedef int (* avahi_service_browser_free_type)(AvahiServiceBrowser *s);
typedef AvahiServiceResolver * (* avahi_service_resolver_new_type)(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    const char *type,
    const char *domain,
    AvahiProtocol aprotocol,
    AvahiLookupFlags flags,
    AvahiServiceResolverCallback callback,
    void *userdata);
typedef int (* avahi_service_resolver_free_type)(AvahiServiceResolver *r);
typedef AvahiRecordBrowser * (* avahi_record_browser_new_type)(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    uint16_t clazz,
    uint16_t type,
    AvahiLookupFlags flags,
    AvahiRecordBrowserCallback callback,
    void *userdata);
typedef int (* avahi_record_browser_free_type)(AvahiRecordBrowser *);
typedef int (* avahi_service_name_join_type)(char *p, size_t size, const char *name, const char *type, const char *domain);
typedef const char *(* avahi_strerror_type)(int error);
typedef int (* avahi_client_errno_type) (AvahiClient*);
typedef AvahiSimplePoll *(* avahi_simple_poll_new_type)(void);
typedef const AvahiPoll* (* avahi_simple_poll_get_type)(AvahiSimplePoll *p);
typedef int (* avahi_simple_poll_iterate_type)(AvahiSimplePoll *s, int sleep_time);
typedef void (*avahi_simple_poll_free_type)(AvahiSimplePoll *s);
typedef void (*avahi_simple_poll_quit_type)(AvahiSimplePoll *s);
typedef const char *(* avahi_client_get_version_string_type)(AvahiClient *c);
typedef void (* avahi_free_type) (void *p);
typedef AvahiStringList *(* avahi_string_list_get_next_type) (AvahiStringList *l);
typedef int (* avahi_string_list_get_pair_type) (AvahiStringList *l, char **key, char **value, size_t *size);


avahi_service_browser_new_type avahi_service_browser_new_ptr = NULL;
avahi_service_browser_free_type avahi_service_browser_free_ptr = NULL;
avahi_service_resolver_new_type avahi_service_resolver_new_ptr = NULL;
avahi_service_resolver_free_type avahi_service_resolver_free_ptr = NULL;
avahi_record_browser_new_type avahi_record_browser_new_ptr = NULL;
avahi_record_browser_free_type avahi_record_browser_free_ptr = NULL;
avahi_service_name_join_type avahi_service_name_join_ptr = NULL;
avahi_client_new_type avahi_client_new_ptr = NULL;
avahi_client_free_type avahi_client_free_ptr = NULL;
avahi_strerror_type avahi_strerror_ptr = NULL;
avahi_client_errno_type avahi_client_errno_ptr = NULL;
avahi_simple_poll_new_type avahi_simple_poll_new_ptr = NULL;
avahi_simple_poll_get_type avahi_simple_poll_get_ptr = NULL;
avahi_simple_poll_iterate_type avahi_simple_poll_iterate_ptr = NULL;
avahi_simple_poll_free_type avahi_simple_poll_free_ptr = NULL;
avahi_simple_poll_quit_type avahi_simple_poll_quit_ptr = NULL;
avahi_client_get_version_string_type avahi_client_get_version_string_ptr = NULL;
avahi_free_type avahi_free_ptr = NULL;
avahi_string_list_get_next_type avahi_string_list_get_next_ptr = NULL;
avahi_string_list_get_pair_type avahi_string_list_get_pair_ptr = NULL;
#else
#define avahi_service_browser_new_ptr avahi_service_browser_new
#define avahi_service_browser_free_ptr avahi_service_browser_free
#define avahi_service_resolver_new_ptr avahi_service_resolver_new
#define avahi_service_resolver_free_ptr avahi_service_resolver_free
#define avahi_record_browser_new_ptr avahi_record_browser_new
#define avahi_record_browser_free_ptr avahi_record_browser_free
#define avahi_service_name_join_ptr avahi_service_name_join
#define avahi_client_new_ptr avahi_client_new
#define avahi_client_free_ptr avahi_client_free
#define avahi_strerror_ptr avahi_strerror
#define avahi_client_errno_ptr avahi_client_errno
#define avahi_simple_poll_new_ptr avahi_simple_poll_new
#define avahi_simple_poll_get_ptr avahi_simple_poll_get
#define avahi_simple_poll_iterate_ptr avahi_simple_poll_iterate
#define avahi_simple_poll_free_ptr avahi_simple_poll_free
#define avahi_simple_poll_quit_ptr avahi_simple_poll_quit
#define avahi_client_get_version_string_ptr avahi_client_get_version_string
#define avahi_free_ptr avahi_free
#define avahi_string_list_get_next_ptr avahi_string_list_get_next
#define avahi_string_list_get_pair_ptr avahi_string_list_get_pair
#endif

/* 
 * TXT record version - this should be 1 for a long time
 *  - only need to change if we really change the TXT record format
 */
const char *dnssd_txt_ver = "1";

int Dns_sdInitialized = FALSE;
int stopBrowsing = FALSE;

static AvahiSimplePoll *simple_poll = NULL;
static AvahiClient *client = NULL;

static AvahiServiceBrowser *zeroconf_browse_sbc_ref  = NULL;
static AvahiServiceBrowser *zeroconf_browse_ws_ref  = NULL;
static AvahiServiceBrowser *zeroconf_browse_phidget_ref  = NULL;

pthread_t dns_thread = (pthread_t)NULL;

void *avahiLibHandle = NULL;

/* Main Avahi thread - source of callbacks */
int dns_callback_thread(void *ptr)
{    
	while (!stopBrowsing)
	{
    	/* Run the main loop */
    	if(avahi_simple_poll_iterate_ptr(simple_poll, 100)) //100 ms
		{
			LOG(PHIDGET_LOG_VERBOSE, "avahi_simple_poll_iterate returned non-zero - breaking out of loop");
			break;
		}
	}
	
	LOG(PHIDGET_LOG_VERBOSE, "dns_thread exiting");
	if(fptrJavaDetachCurrentThread)
		fptrJavaDetachCurrentThread();
	return EPHIDGET_OK;
}

static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
    assert(c);

    /* Called whenever the client or server state changes */

    switch (state) {
        case AVAHI_CLIENT_S_RUNNING:
        
            /* The server has startup successfully and registered its host
             * name on the network */
			Dns_sdInitialized = TRUE;
            break;

        case AVAHI_CLIENT_FAILURE:
            
            LOG(PHIDGET_LOG_ERROR, "Client failure: %s", avahi_strerror_ptr(avahi_client_errno_ptr(c)));
			         
            break;

        case AVAHI_CLIENT_S_COLLISION:
        
            /* Let's drop our registered services. When the server is back
             * in AVAHI_SERVER_RUNNING state we will register them
             * again with the new host name. */
            
        case AVAHI_CLIENT_S_REGISTERING:

            /* The server records are now being established. This
             * might be caused by a host name change. We need to wait
             * for our own records to register until the host name is
             * properly esatblished. */
            
            break;

        case AVAHI_CLIENT_CONNECTING:
            break;
    }
}

void PhidFromTXT(CPhidgetHandle phid, AvahiStringList *txt)
{
	AvahiStringList *curTxt = txt;
	short txtver;
	int i;
	
	do
	{
		char *key, *value;
		size_t size;
		
		avahi_string_list_get_pair_ptr(curTxt, &key, &value, &size);
		
		if(!strcmp(key, "txtvers"))
		{
			txtver = (short)strtol(value, NULL, 10);
		}
		else if(!strcmp(key, "serial"))
		{
			phid->serialNumber = strtol(value, NULL, 10);
			phid->specificDevice = PTRUE;
		}
		else if(!strcmp(key, "version"))
		{
			phid->deviceVersion = strtol(value, NULL, 10);
		}
		else if(!strcmp(key, "label"))
		{
			strncpy(phid->label, value, MAX_LABEL_STORAGE);
		}
		else if(!strcmp(key, "server_id"))
		{
			free(phid->networkInfo->zeroconf_server_id);
			phid->networkInfo->zeroconf_server_id = strdup(value);
		}
		else if(!strcmp(key, "usbstr"))
		{
			strncpy(phid->usbProduct, value, 64);
		}
		else if(!strcmp(key, "id"))
		{
			phid->deviceIDSpec = strtol(value, NULL, 10);
			
			for(i = 1;i<PHIDGET_DEVICE_COUNT;i++)
				if(phid->deviceIDSpec == Phid_Device_Def[i].pdd_sdid) break;
			phid->deviceDef = &Phid_Device_Def[i];
			phid->attr = Phid_Device_Def[i].pdd_attr;
		}
		else if(!strcmp(key, "class"))
		{
			phid->deviceID = strtol(value, NULL, 10);
			phid->deviceType = Phid_DeviceName[phid->deviceID];
		}
		else if(!strcmp(key, "name"))
		{
			for(i = 0;i<PHIDGET_DEVICE_COUNT;i++)
			{
				if(!strcmp(value, Phid_Device_Def[i].pdd_name))
				{
					phid->deviceIDSpec = Phid_Device_Def[i].pdd_sdid;
					phid->deviceDef = &Phid_Device_Def[i];
					phid->attr = Phid_Device_Def[i].pdd_attr;
					break;
				}
			}
		}
		else if(!strcmp(key, "type"))
		{
			phid->deviceID = phidget_type_to_id(value);
			phid->deviceType = Phid_DeviceName[phid->deviceID];
		}
				
		avahi_free_ptr(key);
		avahi_free_ptr(value);
	}
	while((curTxt = avahi_string_list_get_next_ptr(curTxt)) != NULL);

	phid->deviceUID = CPhidget_getUID(phid->deviceIDSpec, phid->deviceVersion);
	phid->networkInfo->mdns = PTRUE;
}

void SBCFromTXT(CPhidgetSBCHandle sbc, AvahiStringList *txt)
{
	AvahiStringList *curTxt = txt;
	
	do
	{
		char *key, *value;
		size_t size;
		
		avahi_string_list_get_pair_ptr(curTxt, &key, &value, &size);
		
		if(!strcmp(key, "txtvers"))
		{
			sbc->txtver = (short)strtol(value, NULL, 10);
		}
		else if(!strcmp(key, "fversion"))
		{
			strncpy(sbc->fversion, value, 12);
		}
		else if(!strcmp(key, "hversion"))
		{
			sbc->hversion = (short)strtol(value, NULL, 10);
		}
		else if(!strcmp(key, "hostname"))
		{
			strncpy(sbc->hostname, value, 128);
		}
		else if(!strcmp(key, "name"))
		{
			strncpy(sbc->deviceName, value, 128);
		}
		
		avahi_free_ptr(key);
		avahi_free_ptr(value);
	}
	while((curTxt = avahi_string_list_get_next_ptr(curTxt)) != NULL);
	
	if(sbc->txtver < 3)
	{
		strncpy(sbc->deviceName, "PhidgetSBC", 128);
	}
}

void DNSServiceResolve_CallBack(
    AvahiServiceResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *address,
    uint16_t port,
    AvahiStringList *txt,
    AvahiLookupResultFlags flags,
	void* userdata)
{    

	CPhidgetRemoteHandle networkInfo = (CPhidgetRemoteHandle)userdata;
	switch (event) {
        case AVAHI_RESOLVER_FAILURE:
            LOG(PHIDGET_LOG_ERROR, "(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s", name, type, domain, avahi_strerror_ptr(avahi_client_errno_ptr(client)));
			networkInfo->zeroconf_host = strdup("err");
            break;
        case AVAHI_RESOLVER_FOUND: 
		{
			LOG(PHIDGET_LOG_INFO, "DNSServiceResolve_CallBack: %s",name);
			networkInfo->zeroconf_host = strdup(host_name);
			networkInfo->zeroconf_port = malloc(10);
			snprintf(networkInfo->zeroconf_port, 9, "%d", port);
		}
    }

    avahi_service_resolver_free_ptr(r);
}

void DNSServiceResolve_SBC_CallBack(
    AvahiServiceResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *address,
    uint16_t port,
    AvahiStringList *txt,
    AvahiLookupResultFlags flags,
	void* userdata)
{    
	
	switch (event) {
        case AVAHI_RESOLVER_FAILURE:
            LOG(PHIDGET_LOG_ERROR, "(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s", name, type, domain, avahi_strerror_ptr(avahi_client_errno_ptr(client)));
            break;
			
        case AVAHI_RESOLVER_FOUND: {
			CPhidgetSBCHandle sbc = (CPhidgetSBCHandle)userdata, found_sbc;
			CPhidgetSBCManagerList *trav;
			
			LOG(PHIDGET_LOG_INFO, "DNSServiceResolve_SBC_CallBack: %s",name);
			
			SBCFromTXT(sbc, txt);
			
			sbc->networkInfo->zeroconf_host = strdup(host_name);
			sbc->networkInfo->zeroconf_port = malloc(10);
			snprintf(sbc->networkInfo->zeroconf_port, 9, "%d", port);
			
			CThread_mutex_lock(&zeroconfSBCsLock);
			CThread_mutex_lock(&activeSBCManagersLock);
			
			//Check if it's in the list and if it's different, remove it to make way for the new one
			// (Sometimes, we don't get a proper detach notification)
			if(CList_findInList((CListHandle)zeroconfSBCs, sbc, CPhidgetSBC_areEqual, (void **)&found_sbc) == EPHIDGET_OK)
			{
				if(CPhidgetSBC_areExtraEqual(found_sbc, sbc) != PTRUE) //A version number has changed
				{
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
    }

    avahi_service_resolver_free_ptr(r);
}

void DNSServiceResolve_Phidget_CallBack(
    AvahiServiceResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *address,
    uint16_t port,
    AvahiStringList *txt,
    AvahiLookupResultFlags flags,
	void* userdata)
{    
	
	switch (event) {
        case AVAHI_RESOLVER_FAILURE:
            LOG(PHIDGET_LOG_ERROR, "(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s", name, type, domain, avahi_strerror_ptr(avahi_client_errno_ptr(client)));
            break;
			
        case AVAHI_RESOLVER_FOUND: {
			CPhidgetHandle phid = (CPhidgetHandle)userdata;
			CPhidgetManagerList *trav;
			
			LOG(PHIDGET_LOG_INFO, "DNSServiceResolve_SBC_CallBack: %s",name);
			
			PhidFromTXT(phid, txt);
			
			phid->networkInfo->zeroconf_host = strdup(host_name);
			phid->networkInfo->zeroconf_port = malloc(10);
			snprintf(phid->networkInfo->zeroconf_port, 9, "%d", port);
			
			LOG(PHIDGET_LOG_INFO, "DNSServiceQueryRecord_Phidget_CallBack: %s",name);
			CThread_mutex_lock(&zeroconfPhidgetsLock);
			CThread_mutex_lock(&activeRemoteManagersLock);
			
			CPhidget_setStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
			CPhidget_setStatusFlag(&phid->status, PHIDGET_REMOTE_FLAG, &phid->lock);
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
    }

    avahi_service_resolver_free_ptr(r);
}

void DNSServiceBrowse_Phidget_CallBack(
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) 
{

	CPhidgetHandle phid;
	CPhidgetManagerList *trav;
	
	int ret;
	
    switch (event) {

        case AVAHI_BROWSER_FAILURE:
            
            LOG(PHIDGET_LOG_WARNING, "(Browser) %s", avahi_strerror_ptr(avahi_client_errno_ptr(client)));
            avahi_simple_poll_quit_ptr(simple_poll);
            return;

        case AVAHI_BROWSER_NEW:
		{
			if((CPhidget_create(&phid))) return;
			if((CPhidgetRemote_create(&phid->networkInfo))) return;

			phid->networkInfo->zeroconf_name = strdup(name);
			phid->networkInfo->zeroconf_type = strdup(type);
			phid->networkInfo->zeroconf_domain = strdup(domain);
			
            LOG(PHIDGET_LOG_INFO, "(Browser) NEW: service '%s' of type '%s' in domain '%s'", name, type, domain);
				
			if (!(avahi_service_resolver_new_ptr(client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, 0, DNSServiceResolve_Phidget_CallBack, phid)))
				LOG(PHIDGET_LOG_ERROR, "avahi_service_resolver_new failed on service '%s': %s", name, avahi_strerror_ptr(avahi_client_errno_ptr(client)));
				
            break;
		}

        case AVAHI_BROWSER_REMOVE:
		{
			if((CPhidget_create(&phid))) return;
			if((CPhidgetRemote_create(&phid->networkInfo))) return;

			phid->networkInfo->zeroconf_name = strdup(name);
			phid->networkInfo->zeroconf_type = strdup(type);
			phid->networkInfo->zeroconf_domain = strdup(domain);
			
            LOG(PHIDGET_LOG_INFO, "(Browser) REMOVE: service '%s' of type '%s' in domain '%s'", name, type, domain);

			//have to fill in phid manually from just the name
			int i;
			CPhidgetHandle found_phid;
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
			for(i=0;i<strlen(name_copy);i++)
				if(name_copy[i] == '(') break;
			if(i<=1) return;
			name_copy[strlen(name_copy)-1]='\0';
			name_copy[i-1] = '\0';
			phid->serialNumber = strtol(name_copy+i+1, NULL, 10);
			phid->specificDevice = PTRUE;
			for(i = 0;i<PHIDGET_DEVICE_COUNT;i++)
				if(!strcmp(name_copy, Phid_Device_Def[i].pdd_name)) break;
			phid->deviceIDSpec = 0;
			phid->deviceDef = &Phid_Device_Def[i];
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
            break;

        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            LOG(PHIDGET_LOG_INFO, "(Browser) %s", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
            break;
    }
}

void DNSServiceBrowse_SBC_CallBack(
									   AvahiServiceBrowser *b,
									   AvahiIfIndex interface,
									   AvahiProtocol protocol,
									   AvahiBrowserEvent event,
									   const char *name,
									   const char *type,
									   const char *domain,
									   AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
									   void* userdata) 
{
	
	CPhidgetSBCHandle sbc, found_sbc;
	CPhidgetSBCManagerList *trav;
	int ret;
	
    switch (event) {
			
        case AVAHI_BROWSER_FAILURE:
            
            LOG(PHIDGET_LOG_WARNING, "(Browser) %s", avahi_strerror_ptr(avahi_client_errno_ptr(client)));
            avahi_simple_poll_quit_ptr(simple_poll);
            return;
			
        case AVAHI_BROWSER_NEW:
		{
			if((CPhidgetSBC_create(&sbc))) return;
			if((CPhidgetRemote_create(&sbc->networkInfo))) return;
			
			sbc->networkInfo->zeroconf_name = strdup(name);
			sbc->networkInfo->zeroconf_type = strdup(type);
			sbc->networkInfo->zeroconf_domain = strdup(domain);
			sbc->networkInfo->mdns = PTRUE;
			
			strncpy(sbc->mac, name+12, 18); //name == 'PhidgetSBC (??:??:??:??:??:??)'
			sbc->mac[17] = '\0';
			
            LOG(PHIDGET_LOG_INFO, "(Browser) NEW: service '%s' of type '%s' in domain '%s'", name, type, domain);
			
			if (!(avahi_service_resolver_new_ptr(client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, 0, DNSServiceResolve_SBC_CallBack, sbc)))
				LOG(PHIDGET_LOG_ERROR, "avahi_service_resolver_new failed on service '%s': %s", name, avahi_strerror_ptr(avahi_client_errno_ptr(client)));
            break;
		}
			
        case AVAHI_BROWSER_REMOVE:
		{
			if((CPhidgetSBC_create(&sbc))) return;
			if((CPhidgetRemote_create(&sbc->networkInfo))) return;
			
			sbc->networkInfo->zeroconf_name = strdup(name);
			sbc->networkInfo->zeroconf_type = strdup(type);
			sbc->networkInfo->zeroconf_domain = strdup(domain);
			sbc->networkInfo->mdns = PTRUE;
			
			strncpy(sbc->mac, name+12, 18); //name == 'PhidgetSBC (??:??:??:??:??:??)'
			sbc->mac[17] = '\0';
			
            LOG(PHIDGET_LOG_INFO, "(Browser) REMOVE: service '%s' of type '%s' in domain '%s'", name, type, domain);
			
			
			CThread_mutex_lock(&zeroconfSBCsLock);
			CThread_mutex_lock(&activeSBCManagersLock);
			
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
            break;
			
        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            LOG(PHIDGET_LOG_INFO, "(Browser) %s", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
            break;
    }
}

void DNSServiceBrowse_ws_CallBack(
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) 
{

    switch (event) {

        case AVAHI_BROWSER_FAILURE:
            
            LOG(PHIDGET_LOG_ERROR, "(Browser) %s", avahi_strerror_ptr(avahi_client_errno_ptr(client)));
            return;

        case AVAHI_BROWSER_NEW:
		{
			CPhidgetRemoteHandle networkInfo;
			if((CPhidgetRemote_create(&networkInfo))) return;

			networkInfo->zeroconf_name = strdup(name);
			networkInfo->zeroconf_server_id = strdup(name);
			networkInfo->zeroconf_type = strdup(type);
			networkInfo->zeroconf_domain = strdup(domain);
			
            LOG(PHIDGET_LOG_INFO, "(Browser) NEW: service '%s' of type '%s' in domain '%s'", name, type, domain);

			CThread_mutex_lock(&zeroconfServersLock);
			CList_addToList((CListHandle *)&zeroconfServers, networkInfo, CPhidgetRemote_areEqual);
			CThread_mutex_unlock(&zeroconfServersLock);
		}
            break;

        case AVAHI_BROWSER_REMOVE:
		{
			CPhidgetRemoteHandle networkInfo;
			if((CPhidgetRemote_create(&networkInfo))) return;

			networkInfo->zeroconf_name = strdup(name);
			networkInfo->zeroconf_server_id = strdup(name);
			networkInfo->zeroconf_type = strdup(type);
			networkInfo->zeroconf_domain = strdup(domain);
            LOG(PHIDGET_LOG_INFO, "(Browser) REMOVE: service '%s' of type '%s' in domain '%s'", name, type, domain);

			CThread_mutex_lock(&zeroconfServersLock);
			CList_removeFromList((CListHandle *)&zeroconfServers, networkInfo, CPhidgetRemote_areEqual, TRUE, CPhidgetRemote_free);
			CThread_mutex_unlock(&zeroconfServersLock);
		}
            break;

        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            LOG(PHIDGET_LOG_INFO, "(Browser) %s", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
            break;
    }
}

//Does nothing in Avahi
int cancelPendingZeroconfLookups(CPhidgetRemoteHandle networkInfo)
{	
	return EPHIDGET_OK;
}

int getZeroconfHostPort(CPhidgetRemoteHandle networkInfo)
{
	//Don't look up if we already have it.
	if(networkInfo->zeroconf_host && networkInfo->zeroconf_port)
		return EPHIDGET_OK;
	
	int timeout = 200; //2000ms
	
	if(networkInfo->zeroconf_host) free(networkInfo->zeroconf_host);
	networkInfo->zeroconf_host = NULL;
	if(networkInfo->zeroconf_port) free(networkInfo->zeroconf_port);
	networkInfo->zeroconf_port = NULL;
	
	if (!(avahi_service_resolver_new_ptr(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 
					   networkInfo->zeroconf_name, //name
					   networkInfo->zeroconf_type, // service type
					   networkInfo->zeroconf_domain, //domain
					   AVAHI_PROTO_UNSPEC, 0, DNSServiceResolve_CallBack, networkInfo)))
	{
		LOG(PHIDGET_LOG_ERROR, "Failed to resolve service '%s': %s", networkInfo->zeroconf_name, avahi_strerror_ptr(avahi_client_errno_ptr(client)));
		return EPHIDGET_UNEXPECTED;
	}

	while(!networkInfo->zeroconf_host)
	{
		usleep(10000);
		timeout--;
		if(!timeout)
		{
			LOG(PHIDGET_LOG_ERROR, "getZeroconfHostPort didn't work (timeout)");
			return EPHIDGET_UNEXPECTED;
		}
	}
	
	if(!strcmp(networkInfo->zeroconf_host, "err"))
	{
		LOG(PHIDGET_LOG_ERROR, "getZeroconfHostPort didn't work (error)");
		free(networkInfo->zeroconf_host);
		networkInfo->zeroconf_host = NULL;
		return EPHIDGET_UNEXPECTED;
	}
	
	return EPHIDGET_OK;
}

int refreshZeroconfSBC(CPhidgetSBCHandle sbc)
{
	return EPHIDGET_OK;
}

int refreshZeroconfPhidget(CPhidgetHandle phid)
{
	return EPHIDGET_OK;
}

int InitializeZeroconf()
{
    int error;
	int timeout = 50; //500ms
	const char *avahiVersion;
	
	CThread_mutex_lock(&zeroconfInitLock);
	if(Dns_sdInitialized) 
	{
		CThread_mutex_unlock(&zeroconfInitLock);
		return EPHIDGET_OK;
	}
	
#ifdef ZEROCONF_RUNTIME_LINKING

	avahiLibHandle = dlopen("libavahi-client.so",RTLD_LAZY);
	if(!avahiLibHandle)
	{
		avahiLibHandle = dlopen("libavahi-client.so.3",RTLD_LAZY);
	}
	if(!avahiLibHandle)
	{
		LOG(PHIDGET_LOG_WARNING, "dlopen failed with error: %s", dlerror());
		LOG(PHIDGET_LOG_WARNING, "Assuming that zeroconf is not supported on this machine.");
		CThread_mutex_unlock(&zeroconfInitLock);
		return EPHIDGET_UNSUPPORTED;
	}

	//These are always in Avahi
	if(!(avahi_client_get_version_string_ptr = (avahi_client_get_version_string_type)dlsym(avahiLibHandle, "avahi_client_get_version_string"))) goto dlsym_err;
	if(!(avahi_service_browser_new_ptr = (avahi_service_browser_new_type)dlsym(avahiLibHandle, "avahi_service_browser_new"))) goto dlsym_err;
	if(!(avahi_service_browser_free_ptr = (avahi_service_browser_free_type)dlsym(avahiLibHandle, "avahi_service_browser_free"))) goto dlsym_err;
	if(!(avahi_service_resolver_new_ptr = (avahi_service_resolver_new_type)dlsym(avahiLibHandle, "avahi_service_resolver_new"))) goto dlsym_err;
	if(!(avahi_service_resolver_free_ptr = (avahi_service_resolver_free_type)dlsym(avahiLibHandle, "avahi_service_resolver_free"))) goto dlsym_err;
	if(!(avahi_record_browser_new_ptr = (avahi_record_browser_new_type)dlsym(avahiLibHandle, "avahi_record_browser_new"))) goto dlsym_err;
	if(!(avahi_record_browser_free_ptr = (avahi_record_browser_free_type)dlsym(avahiLibHandle, "avahi_record_browser_free"))) goto dlsym_err;
	if(!(avahi_service_name_join_ptr = (avahi_service_name_join_type)dlsym(avahiLibHandle, "avahi_service_name_join"))) goto dlsym_err;
	if(!(avahi_client_new_ptr = (avahi_client_new_type)dlsym(avahiLibHandle, "avahi_client_new"))) goto dlsym_err;
	if(!(avahi_client_free_ptr = (avahi_client_free_type)dlsym(avahiLibHandle, "avahi_client_free"))) goto dlsym_err;
	if(!(avahi_strerror_ptr = (avahi_strerror_type)dlsym(avahiLibHandle, "avahi_strerror"))) goto dlsym_err;
	if(!(avahi_client_errno_ptr = (avahi_client_errno_type)dlsym(avahiLibHandle, "avahi_client_errno"))) goto dlsym_err;
	if(!(avahi_simple_poll_new_ptr = (avahi_simple_poll_new_type)dlsym(avahiLibHandle, "avahi_simple_poll_new"))) goto dlsym_err2;
	if(!(avahi_simple_poll_get_ptr = (avahi_simple_poll_get_type)dlsym(avahiLibHandle, "avahi_simple_poll_get"))) goto dlsym_err2;
	if(!(avahi_simple_poll_iterate_ptr = (avahi_simple_poll_iterate_type)dlsym(avahiLibHandle, "avahi_simple_poll_iterate"))) goto dlsym_err2;
	if(!(avahi_simple_poll_free_ptr = (avahi_simple_poll_free_type)dlsym(avahiLibHandle, "avahi_simple_poll_free"))) goto dlsym_err2;
	if(!(avahi_simple_poll_quit_ptr = (avahi_simple_poll_quit_type)dlsym(avahiLibHandle, "avahi_simple_poll_quit"))) goto dlsym_err2;
	
	//These are new ones I'm using, not sure when they were added...
	if(!(avahi_free_ptr = (avahi_free_type)dlsym(avahiLibHandle, "avahi_free"))) goto dlsym_err3;
	if(!(avahi_string_list_get_next_ptr = (avahi_string_list_get_next_type)dlsym(avahiLibHandle, "avahi_string_list_get_next"))) goto dlsym_err3;
	if(!(avahi_string_list_get_pair_ptr = (avahi_string_list_get_pair_type)dlsym(avahiLibHandle, "avahi_string_list_get_pair"))) goto dlsym_err3;
	
	
	goto dlsym_good;
	
dlsym_err:
	LOG(PHIDGET_LOG_WARNING, "dlsym failed with error: %s", dlerror());
	LOG(PHIDGET_LOG_WARNING, "Assuming that zeroconf is not supported on this machine.");
	CThread_mutex_unlock(&zeroconfInitLock);
	return EPHIDGET_UNSUPPORTED;
	
	//Old avahi didn't have the thread functions
dlsym_err2:
	LOG(PHIDGET_LOG_WARNING, "dlsym failed with error: %s", dlerror());
	LOG(PHIDGET_LOG_WARNING, "Avahi is too old, upgrade to at least version 0.6.4.");
	LOG(PHIDGET_LOG_WARNING, "Zeroconf will not be used on this machine.");
	CThread_mutex_unlock(&zeroconfInitLock);
	return EPHIDGET_UNSUPPORTED;
	
dlsym_err3:
	LOG(PHIDGET_LOG_WARNING, "dlsym failed with error: %s", dlerror());
	LOG(PHIDGET_LOG_WARNING, "Avahi is too old, upgrade to a newer version.");
	LOG(PHIDGET_LOG_WARNING, "Zeroconf will not be used on this machine.");
	CThread_mutex_unlock(&zeroconfInitLock);
	return EPHIDGET_UNSUPPORTED;
	
dlsym_good:
		
#endif
	
    /* Allocate main loop object */
    if (!(simple_poll = avahi_simple_poll_new_ptr())) {
        LOG(PHIDGET_LOG_ERROR, "Failed to create threaded poll object.");
		CThread_mutex_unlock(&zeroconfInitLock);
        return EPHIDGET_UNEXPECTED;
    }
	
    /* Allocate a new client */
    client = avahi_client_new_ptr(avahi_simple_poll_get_ptr(simple_poll), 0, client_callback, NULL, &error);

    /* Check wether creating the client object succeeded */
    if (!client) {
        LOG(PHIDGET_LOG_ERROR, "Failed to create client: %s", avahi_strerror_ptr(error));
		CThread_mutex_unlock(&zeroconfInitLock);
        return EPHIDGET_UNEXPECTED;
    }
	
	//get version
	avahiVersion = avahi_client_get_version_string_ptr(client);
	
	/* Create the service browsers */
    if (!(zeroconf_browse_ws_ref = avahi_service_browser_new_ptr(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_phidget_ws._tcp", NULL, 0, DNSServiceBrowse_ws_CallBack, client))) {
        LOG(PHIDGET_LOG_ERROR, "Failed to create service browser: %s", avahi_strerror_ptr(avahi_client_errno_ptr(client)));
		CThread_mutex_unlock(&zeroconfInitLock);
        return EPHIDGET_UNEXPECTED;
    }
    if (!(zeroconf_browse_phidget_ref = avahi_service_browser_new_ptr(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_phidget._tcp", NULL, 0, DNSServiceBrowse_Phidget_CallBack, client))) {
        LOG(PHIDGET_LOG_ERROR, "Failed to create service browser: %s", avahi_strerror_ptr(avahi_client_errno_ptr(client)));
		CThread_mutex_unlock(&zeroconfInitLock);
        return EPHIDGET_UNEXPECTED;
    }
    if (!(zeroconf_browse_sbc_ref = avahi_service_browser_new_ptr(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_phidget_sbc._tcp", NULL, 0, DNSServiceBrowse_SBC_CallBack, client))) {
        LOG(PHIDGET_LOG_ERROR, "Failed to create service browser: %s", avahi_strerror_ptr(avahi_client_errno_ptr(client)));
		CThread_mutex_unlock(&zeroconfInitLock);
        return EPHIDGET_UNEXPECTED;
    }
	
	//Create a new thread to run the browser/callbacks
	stopBrowsing = FALSE;
	if(pthread_create(&dns_thread, NULL, (void *(*)(void *))dns_callback_thread,NULL))
	{
		LOG(PHIDGET_LOG_ERROR, "pthread_create failed");
		CThread_mutex_unlock(&zeroconfInitLock);
        return EPHIDGET_UNEXPECTED;
	}
	
	while(!Dns_sdInitialized)
	{
		usleep(10000);
		timeout--;
		if(!timeout)
		{
			UninitializeZeroconf1(PFALSE);
			LOG(PHIDGET_LOG_ERROR, "InitializeZeroconf Seems bad... Dns_sdInitialized wasn't set to true.");
			CThread_mutex_unlock(&zeroconfInitLock);
			return EPHIDGET_UNEXPECTED;
		}
	}
	
	LOG(PHIDGET_LOG_INFO, "InitializeZeroconf Seems good... (%s)",avahiVersion);
	
	CThread_mutex_unlock(&zeroconfInitLock);
	return EPHIDGET_OK;
	
}


static int UninitializeZeroconf1(int lock)
{
	int ret;
    /* Cleanup things */
	if(lock)
		CThread_mutex_lock(&zeroconfInitLock);
	if(Dns_sdInitialized)
	{
		void *status;
		
		//Stop the poll and join the thread
		stopBrowsing = TRUE;
		if(dns_thread)
		{
			pthread_join(dns_thread, &status);
			dns_thread = (pthread_t)NULL;
		}
		
		//dealocate objects
    	if (zeroconf_browse_ws_ref)
        	avahi_service_browser_free_ptr(zeroconf_browse_ws_ref);
		zeroconf_browse_ws_ref = NULL;
		
    	if (zeroconf_browse_phidget_ref)
        	avahi_service_browser_free_ptr(zeroconf_browse_phidget_ref);
		zeroconf_browse_phidget_ref = NULL;
		
    	if (zeroconf_browse_sbc_ref)
        	avahi_service_browser_free_ptr(zeroconf_browse_sbc_ref);
		zeroconf_browse_sbc_ref = NULL;
		
		if(client)
			avahi_client_free_ptr(client);
		client = NULL;
		
		if (simple_poll)
			avahi_simple_poll_free_ptr(simple_poll);
		simple_poll = NULL;
	}
	
	Dns_sdInitialized = FALSE;
	if(lock)
		CThread_mutex_unlock(&zeroconfInitLock);
	return EPHIDGET_OK;
}

int UninitializeZeroconf()
{
	return UninitializeZeroconf1(PTRUE);
}
