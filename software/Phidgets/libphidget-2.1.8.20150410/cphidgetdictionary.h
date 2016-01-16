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

#ifndef __CPHIDGETDICTIONARY
#define __CPHIDGETDICTIONARY
#include "cphidget.h"

/** \defgroup phiddict Phidget Dictionary 
 * These calls are specific to the Phidget Dictionary.
 * @{
 */

/**
 * Possible reasons for a key event.
 */
typedef enum {
	PHIDGET_DICTIONARY_VALUE_CHANGED = 1,	/**< The value of an existing key/value pair changed. */
	PHIDGET_DICTIONARY_ENTRY_ADDED,			/**< A new key/value pair was added. */
	PHIDGET_DICTIONARY_ENTRY_REMOVING,		/**< A key is being removed. */
	PHIDGET_DICTIONARY_CURRENT_VALUE		/**< Initial state received once a handler was added. */
}  CPhidgetDictionary_keyChangeReason;

/**
 * A Phidget Dictionary handle.
 */
DPHANDLE(Dictionary)
/**
 * A Dictionary key listener handle.
 */
DPHANDLE(DictionaryListener)

#ifndef EXTERNALPROTO

typedef struct _CPhidgetDictionaryListener
{
	CPhidgetDictionaryHandle dict;
	int(CCONV *fptr)(CPhidgetDictionaryHandle dict, void *userPtr, const char *key, const char *val, CPhidgetDictionary_keyChangeReason reason);
	void *userPtr;
	int listen_id;
} CPhidgetDictionaryListener;

typedef struct _CPhidgetDictionaryListenerList
{
	struct _CPhidgetDictionaryListenerList *next;
	CPhidgetDictionaryListenerHandle listener;
} CPhidgetDictionaryListenerList, *CPhidgetDictionaryListenerListHandle;

typedef struct _CPhidgetDictionary
{
	CPhidgetRemoteHandle networkInfo;
	int(CCONV *fptrError)(CPhidgetDictionaryHandle , void *, int, const char *);
	void *fptrErrorptr;
	int(CCONV *fptrServerConnect)(CPhidgetDictionaryHandle , void *);
	void *fptrServerConnectptr; 
	int(CCONV *fptrServerDisconnect)(CPhidgetDictionaryHandle , void *);
	void *fptrServerDisconnectptr;
	CThread_mutex_t lock; /* protects status */
	int status;
	CThread_mutex_t openCloseLock; /* protects open / close */
	CPhidgetDictionaryListenerListHandle listeners;
	CThread_mutex_t listenersLock; /* protects listeners */
} CPhidgetDictionary;

typedef struct _CPhidgetDictionaryList
{
	struct _CPhidgetDictionaryList *next;
	CPhidgetDictionaryHandle dict;
} CPhidgetDictionaryList, *CPhidgetDictionaryListHandle;

int CPhidgetDictionary_areEqual(void *arg1, void *arg2);
void CPhidgetDictionary_free(void *arg);
int CPhidgetDictionaryListener_areEqual(void *arg1, void *arg2);
void CPhidgetDictionaryListener_free(void *arg);

#endif
/**
 * Creates a Phidget Dictionary handle.
 * @param dict A pointer to an unallocated phidget dictionary handle.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_create(CPhidgetDictionaryHandle *dict);
/**
 * Closes the connection to a Phidget Dictionary.
 * @param dict An opened phidget dictionary handle.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_close(CPhidgetDictionaryHandle dict);
/**
 * Frees a Phidget Dictionary handle.
 * @param dict A closed dictionary handle.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_delete(CPhidgetDictionaryHandle dict);
/**
 * Sets the error handler callback function. This is called when an asynchronous error occurs.
 * @param dict A phidget dictionary handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_set_OnError_Handler(CPhidgetDictionaryHandle dict,
    int(CCONV *fptr)(CPhidgetDictionaryHandle, void *userPtr, int errorCode, const char *errorString), void *userPtr);
/**
 * Adds a key/value pair to the dictionary. Or, changes an existing key's value.
 * @param dict A connected dictionary handle.
 * @param key The key value.
 * @param value The value value.
 * @param persistent Whether the key stays in the dictionary after disconnection.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_addKey(CPhidgetDictionaryHandle dict, const char *key, const char *value, int persistent);
/**
 * Removes a set of keys from the dictionary.
 * @param dict A connected dictionary handle.
 * @param pattern A regular expression representing th eset of keys to remove.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_removeKey(CPhidgetDictionaryHandle dict, const char *pattern);
/**
 * Callback function for KeyChange events.
 * @param dict Dictionary from which this event originated.
 * @param userPtr User defined data.
 * @param key Key value.
 * @param value Value value.
 * @param reason Reason for KeyChange event.
 */
typedef int(CCONV *CPhidgetDictionary_OnKeyChange_Function)(CPhidgetDictionaryHandle dict, void *userPtr, 
	const char *key, const char *value, CPhidgetDictionary_keyChangeReason reason);
/**
 * Adds a key listener to an opened dictionary. Note that this should only be called after the connection to the
 * dictionary has been made - unlike all other events.
 * @param dict A connected dictionary handle.
 * @param dictlistener A pointer to an unallocated dictionary key listener handle.
 * @param pattern A regular expression representing the set of keys to monitor.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_set_OnKeyChange_Handler(CPhidgetDictionaryHandle dict, CPhidgetDictionaryListenerHandle *dictlistener, const char *pattern, 
	CPhidgetDictionary_OnKeyChange_Function fptr, void *userPtr);
/**
 * Removes a key listener.
 * @param dictlistener The dictionary key listener created by \ref CPhidgetDictionary_set_OnKeyChange_Handler
 */
PHIDGET21_API int CCONV CPhidgetDictionary_remove_OnKeyChange_Handler(CPhidgetDictionaryListenerHandle dictlistener);
/**
 * Gets a key value. If more then one key matches, only the first value is returned.
 * @param dict A phidget dictionary handle.
 * @param key A key value to look up.
 * @param value A user array for the value to be stored in. Set to NULL if the key does not exist.
 * @param valuelen Length of the value array.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_getKey(CPhidgetDictionaryHandle dict, const char *key, char *value, int valuelen);
/**
 * Sets a server connect handler callback function. This is called when a connection to the sever has been made.
 * @param dict A phidget dictionary handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_set_OnServerConnect_Handler(CPhidgetDictionaryHandle dict, int (CCONV *fptr)(CPhidgetDictionaryHandle dict, void *userPtr), void *userPtr);
/**
 * Sets a server disconnect handler callback function. This is called when a connection to the server has been lost.
 * @param dict A phidget dictionary handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_set_OnServerDisconnect_Handler(CPhidgetDictionaryHandle dict, int (CCONV *fptr)(CPhidgetDictionaryHandle dict, void *userPtr), void *userPtr);
/**
 * Gets the server ID.
 * @param dict A connected dictionary handle.
 * @param serverID A pointer which will be set to a char array containing the server ID string.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_getServerID(CPhidgetDictionaryHandle dict, const char **serverID);
/**
 * Gets the address and port.
 * @param dict A connected dictionary handle.
 * @param address A pointer which will be set to a char array containing the address string.
 * @param port An int pointer for returning the port number.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_getServerAddress(CPhidgetDictionaryHandle dict, const char **address, int *port);
/**
 * Gets the connected to server status.
 * @param dict An opened dictionary handle.
 * @param serverStatus An int pointer for returning the server status. Possible codes are \ref PHIDGET_ATTACHED and \ref PHIDGET_NOTATTACHED.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_getServerStatus(CPhidgetDictionaryHandle dict, int *serverStatus);
/** @} */
#endif
