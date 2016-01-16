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
#include "cphidgetdictionary.h"
#include "csocket.h"
#include "csocketevents.h"
#include "cphidgetlist.h"
#include "cphidgetmanager.h"

int CPhidgetDictionary_areEqual(void *arg1, void *arg2)
{
	if(arg1 == arg2) return 1;
	return 0;
}

void CPhidgetDictionary_free(void *arg)
{
	CPhidgetDictionaryHandle dict = arg;
	
	if(!dict)
		return;
	
	CThread_mutex_lock(&dict->listenersLock);
	CList_emptyList((CListHandle *)&dict->listeners, PTRUE, CPhidgetDictionaryListener_free);
	CThread_mutex_unlock(&dict->listenersLock);

	CThread_mutex_destroy(&dict->lock);
	CThread_mutex_destroy(&dict->listenersLock);
	CThread_mutex_destroy(&dict->openCloseLock);

	free(dict); dict = NULL;
}

int CPhidgetDictionaryListener_areEqual(void *arg1, void *arg2)
{
	if(arg1 == arg2) return 1;
	return 0;
}

void CPhidgetDictionaryListener_free(void *arg)
{
	CPhidgetDictionaryListenerHandle dict = arg;
	
	if(!dict)
		return;
	free(dict); dict = NULL;
}

int CCONV CPhidgetDictionary_create(CPhidgetDictionaryHandle *dict)
{
	CPhidgetDictionaryHandle dicttemp = 0;
	
	TESTPTR(dict)

	if(!(dicttemp = (CPhidgetDictionaryHandle)malloc(sizeof(CPhidgetDictionary))))
		return EPHIDGET_NOMEMORY;
	ZEROMEM(dicttemp, sizeof(CPhidgetDictionary));

	CThread_mutex_init(&dicttemp->lock);
	CThread_mutex_init(&dicttemp->listenersLock);
	CThread_mutex_init(&dicttemp->openCloseLock);

	*dict = dicttemp;
	return EPHIDGET_OK;
}	

int CCONV CPhidgetDictionary_close(CPhidgetDictionaryHandle dict)
{
	int result = EPHIDGET_OK;
	TESTPTR(dict)

	CThread_mutex_lock(&dict->openCloseLock);
	if (!CPhidget_statusFlagIsSet(dict->status, PHIDGET_OPENED_FLAG))
	{
		LOG(PHIDGET_LOG_WARNING, "Close was called on an already closed Dictionary handle.");
		CThread_mutex_unlock(&dict->openCloseLock);
		return EPHIDGET_OK;
	}

	if((result = unregisterRemoteDictionary(dict)) != EPHIDGET_OK)
	{
		CThread_mutex_unlock(&dict->openCloseLock);
		return result;
	}

	CPhidget_clearStatusFlag(&dict->status, PHIDGET_OPENED_FLAG, &dict->lock);
	CThread_mutex_unlock(&dict->openCloseLock);
	return EPHIDGET_OK;
}

int CCONV CPhidgetDictionary_delete(CPhidgetDictionaryHandle dict)
{
	CPhidgetDictionary_free(dict);
	return EPHIDGET_OK;
}

int CCONV CPhidgetDictionary_getServerID(CPhidgetDictionaryHandle dict, const char **serverID)
{
	return  CPhidget_getServerID((CPhidgetHandle)dict, serverID);
}
int CCONV CPhidgetDictionary_getServerAddress(CPhidgetDictionaryHandle dict, const char **address, int *port)
{
	return  CPhidget_getServerAddress((CPhidgetHandle)dict, address, port);
}
int CCONV CPhidgetDictionary_getServerStatus(CPhidgetDictionaryHandle dict, int *status)
{
	return CPhidget_getServerStatus((CPhidgetHandle)dict, status);
}

int CCONV CPhidgetDictionary_set_OnError_Handler(CPhidgetDictionaryHandle dict,
    int(CCONV *fptr)(CPhidgetDictionaryHandle, void *, int, const char *), void *userPtr)
{
	TESTPTR(dict)
	dict->fptrError = fptr;
	dict->fptrErrorptr = userPtr;
	return EPHIDGET_OK;
}

/* Async add - errors returned to a registered error handler */
int CCONV CPhidgetDictionary_addKey(CPhidgetDictionaryHandle dict, const char *key, const char *val, int persistent)
{
	TESTPTR(dict)
	TESTPTRS(key, val)

	CThread_mutex_lock(&dict->lock);
	if(!CPhidget_statusFlagIsSet(dict->status, PHIDGET_SERVER_CONNECTED_FLAG))
	{
		CThread_mutex_unlock(&dict->lock);
		return EPHIDGET_NETWORK_NOTCONNECTED;
	}

	pdc_async_set(dict->networkInfo->server->pdcs, key, val, (int)strlen(val), persistent?0:1, internal_async_network_error_handler, dict);

	CThread_mutex_unlock(&dict->lock);

	return EPHIDGET_OK;
}

int CCONV CPhidgetDictionary_removeKey(CPhidgetDictionaryHandle dict, const char *pattern)
{
//	int result;
//	char err[1024];
	TESTPTRS(dict, pattern)

	CThread_mutex_lock(&dict->lock);
	if(!CPhidget_statusFlagIsSet(dict->status, PHIDGET_SERVER_CONNECTED_FLAG))
	{
		CThread_mutex_unlock(&dict->lock);
		return EPHIDGET_NETWORK_NOTCONNECTED;
	}

//	CThread_mutex_lock(&dict->networkInfo->server->pdc_lock);
//	result = pdc_remove(dict->networkInfo->server->pdcs, pattern, err, sizeof(err));
//	CThread_mutex_unlock(&dict->networkInfo->server->pdc_lock);
	
	pdc_async_remove(dict->networkInfo->server->pdcs, pattern, internal_async_network_error_handler, dict);

	CThread_mutex_unlock(&dict->lock);

//	if(result == 0) return EPHIDGET_UNEXPECTED;
	return EPHIDGET_OK;
}

int CCONV CPhidgetDictionary_getKey(CPhidgetDictionaryHandle dict, const char *key, char *val, int vallen)
{
	int ret;

	TESTPTRS(dict, key)
	TESTPTR(val)

	CThread_mutex_lock(&dict->lock);
	if(!CPhidget_statusFlagIsSet(dict->status, PHIDGET_SERVER_CONNECTED_FLAG))
	{
		ret = EPHIDGET_NETWORK_NOTCONNECTED;
	}
	else
	{
		int result, size;
		char err[1024], *keywrap;

		//The get command returns a list of keys - since we want just a single key, lets wrap in ^ and $
		//other reg exp tags are allowed and will be honoured
		size = (int)strlen(key);
		keywrap = (char *)malloc(size+3);
		snprintf(keywrap, size+3, "^%s$",key);

		CThread_mutex_lock(&dict->networkInfo->server->pdc_lock);
		if(!dict->networkInfo->server->pdcs)
			ret = EPHIDGET_NETWORK_NOTCONNECTED;
		else
		{
			result = pdc_get(dict->networkInfo->server->pdcs, keywrap, val, vallen, err, sizeof(err));
			switch(result)
			{
				case 1:
					ret = EPHIDGET_OK;
					break;
				case 2:
					ret = EPHIDGET_NOTFOUND;
					break;
				case 0:
				default:
					ret = EPHIDGET_UNEXPECTED;
					break;
			}
		}
		CThread_mutex_unlock(&dict->networkInfo->server->pdc_lock);

		free(keywrap);
	}

	CThread_mutex_unlock(&dict->lock);

	return ret;
}

void dict_event_handler(const char *key, const char *val, unsigned int len, pdict_reason_t reason, void *ptr)
{
	CPhidgetDictionaryListenerHandle listener = ptr;
	
	if(!listener) return;
	
	if(listener->fptr)
		listener->fptr(listener->dict, listener->userPtr, key, val, (CPhidgetDictionary_keyChangeReason)reason);
	return;
}

int CCONV CPhidgetDictionary_set_OnServerConnect_Handler(CPhidgetDictionaryHandle dict, int (CCONV *fptr)(CPhidgetDictionaryHandle dict, void *userPtr), void *userPtr)
{
	TESTPTR(dict)
	dict->fptrServerConnect = fptr; 
	dict->fptrServerConnectptr = userPtr; 
	return EPHIDGET_OK; 
}
int CCONV CPhidgetDictionary_set_OnServerDisconnect_Handler(CPhidgetDictionaryHandle dict, int (CCONV *fptr)(CPhidgetDictionaryHandle dict, void *userPtr), void *userPtr)
{
	TESTPTR(dict)
	dict->fptrServerDisconnect = fptr; 
	dict->fptrServerDisconnectptr = userPtr; 
	return EPHIDGET_OK; 
}

int CCONV CPhidgetDictionary_set_OnKeyChange_Handler(CPhidgetDictionaryHandle dict, CPhidgetDictionaryListenerHandle *dictlistener, const char *pattern, 
	int(CCONV *fptr)(CPhidgetDictionaryHandle dict, void *userPtr, const char *key, const char *val, CPhidgetDictionary_keyChangeReason reason),
	void *userPtr)
{
	CPhidgetDictionaryListenerHandle dict_listener;
	char errdesc[1024];
	int result;

	TESTPTRS(dict, pattern)
	TESTPTR(dictlistener)

	CThread_mutex_lock(&dict->lock);
	if(!CPhidget_statusFlagIsSet(dict->status, PHIDGET_SERVER_CONNECTED_FLAG))
	{
		CThread_mutex_unlock(&dict->lock);
		return EPHIDGET_NETWORK_NOTCONNECTED;
	}
		
	if(!(dict_listener = malloc(sizeof(CPhidgetDictionaryListener))))
	{
		CThread_mutex_unlock(&dict->lock);
		return EPHIDGET_NOMEMORY;
	}
	ZEROMEM(dict_listener, sizeof(CPhidgetDictionaryListener));
	
	dict_listener->dict = dict;
	dict_listener->fptr = fptr;
	dict_listener->userPtr = userPtr;
	
	CThread_mutex_lock(&dict->networkInfo->server->pdc_lock);
	if (!(dict_listener->listen_id = pdc_listen(dict->networkInfo->server->pdcs, pattern, dict_event_handler, dict_listener, errdesc, sizeof (errdesc))))
	{
		LOG(PHIDGET_LOG_DEBUG,"pdc_listen: %s", errdesc);
		free(dict_listener);
		CThread_mutex_unlock(&dict->networkInfo->server->pdc_lock);
		CThread_mutex_unlock(&dict->lock);
		return EPHIDGET_UNEXPECTED;
	}
	CThread_mutex_unlock(&dict->networkInfo->server->pdc_lock);
	
	CThread_mutex_lock(&dict->listenersLock);
	if((result = CList_addToList((CListHandle *)&dict->listeners, dict_listener, CPhidgetDictionaryListener_areEqual)))
	{
		CThread_mutex_unlock(&dict->listenersLock);
		CThread_mutex_lock(&dict->networkInfo->server->pdc_lock);
		pdc_ignore(dict->networkInfo->server->pdcs, dict_listener->listen_id, NULL, 0);
		CThread_mutex_unlock(&dict->networkInfo->server->pdc_lock);
		free(dict_listener);
		CThread_mutex_unlock(&dict->lock);
		return result;
	}
	CThread_mutex_unlock(&dict->listenersLock);
	CThread_mutex_unlock(&dict->lock);

	*dictlistener = dict_listener;
	
	return EPHIDGET_OK;
}

//This can be called even when not connected to a server
int CCONV CPhidgetDictionary_remove_OnKeyChange_Handler(CPhidgetDictionaryListenerHandle keylistener)
{
	int result = 0;
	char errdesc[1024];
	CPhidgetDictionaryHandle dict;

	TESTPTR(keylistener)
	dict = keylistener->dict;

	CThread_mutex_lock(&dict->lock);

	if(CPhidget_statusFlagIsSet(dict->status, PHIDGET_SERVER_CONNECTED_FLAG))
	{
		CThread_mutex_lock(&dict->networkInfo->server->pdc_lock);
		if(!(result = pdc_ignore(dict->networkInfo->server->pdcs, keylistener->listen_id, errdesc, sizeof (errdesc))))
		{
			LOG(PHIDGET_LOG_WARNING,"pdc_ignore: %s",errdesc);
			CThread_mutex_unlock(&dict->networkInfo->server->pdc_lock);
			CThread_mutex_unlock(&dict->lock);
			return EPHIDGET_UNEXPECTED;
		}
		CThread_mutex_unlock(&dict->networkInfo->server->pdc_lock);
	}

	CThread_mutex_lock(&dict->listenersLock);
	if((result = CList_removeFromList((CListHandle *)&dict->listeners, keylistener, 
		CPhidgetDictionaryListener_areEqual, PTRUE, CPhidgetDictionaryListener_free)))
	{
		CThread_mutex_unlock(&dict->listenersLock);
		CThread_mutex_unlock(&dict->lock);
		return result;
	}
	CThread_mutex_unlock(&dict->listenersLock);
	CThread_mutex_unlock(&dict->lock);

	return EPHIDGET_OK;
}
