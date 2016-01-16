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

#ifndef __CSOCKET
#define __CSOCKET

#include "cphidget.h"
#include "cphidgetmanager.h"
#include "cphidgetdictionary.h"

#ifndef EXTERNALPROTO

#include "cphidgetsbc.h"

extern CThread_mutex_t serverLock;
extern CThread_mutex_t zeroconfPhidgetsLock;
extern CPhidgetListHandle zeroconfPhidgets;
extern CPhidgetRemoteListHandle zeroconfServers;
extern CPhidgetListHandle zeroconfPhidgets;
extern CPhidgetSBCListHandle zeroconfSBCs;
extern CThread_mutex_t zeroconfServersLock;
extern CThread_mutex_t zeroconfPhidgetsLock;
extern CThread_mutex_t zeroconfSBCsLock;
extern CPhidgetListHandle activeRemotePhidgets;
extern CPhidgetManagerListHandle activeRemoteManagers;
extern CPhidgetDictionaryListHandle activeRemoteDictionaries;
extern CPhidgetSBCManagerListHandle activeSBCManagers;
extern CThread_mutex_t activeRemotePhidgetsLock;
extern CThread_mutex_t activeRemoteManagersLock;
extern CThread_mutex_t activeRemoteDictionariesLock;
extern CThread_mutex_t activeSBCManagersLock;
extern CThread_mutex_t zeroconfInitLock;

void internal_async_network_error_handler(const char *error, void *phid);
int unregisterRemotePhidget(CPhidgetHandle phid);
int unregisterRemoteDictionary(CPhidgetDictionaryHandle dict);
int unregisterRemoteManager(CPhidgetManagerHandle phidm);
int unregisterSBCManager(CPhidgetSBCManagerHandle sbcm);
CThread_func_return_t DisconnectPhidgetThreadFunction(CThread_func_arg_t lpdwParam);
int getZeroconfHostPort(CPhidgetRemoteHandle networkInfo);
int refreshZeroconfPhidget(CPhidgetHandle phid);
int refreshZeroconfSBC(CPhidgetSBCHandle sbc);
int CCONV CPhidgetRemote_areEqual(void *arg1, void *arg2);
void CCONV CPhidgetRemote_free(void *arg);
int CCONV CPhidgetRemote_create(CPhidgetRemoteHandle *arg);

PHIDGET21_API void CCONV CPhidgetSocketClient_free(void *arg);
PHIDGET21_API int CCONV CPhidgetSocketClient_areEqual(void *arg1, void *arg2);
PHIDGET21_API int CCONV CPhidgetSocketClient_create(CPhidgetSocketClientHandle *arg);

#define SET_RUNNING_EVENT(phid) \
	CThread_mutex_lock(&phid->lock); \
	if(phid->networkInfo && phid->networkInfo->server) \
		phid->networkInfo->server->runningEvent = PTRUE; \
	CThread_mutex_unlock(&phid->lock);

#define CLEAR_RUNNING_EVENT(phid) \
	CThread_mutex_lock(&phid->lock); \
	if(phid->networkInfo && phid->networkInfo->server) \
	{ \
		setTimeNow(&phid->networkInfo->server->lastHeartbeatTime); \
		phid->networkInfo->server->runningEvent = PFALSE; \
	} \
	CThread_mutex_unlock(&phid->lock);

#endif

/** \addtogroup phidcommon
 * @{
 */
/**
 * Opens a Phidget remotely by ServerID. Note that this requires Bonjour (mDNS) to be running on both the host and the server.
 * @param phid A phidget handle.
 * @param serial Serial number. Specify -1 to open any.
 * @param serverID Server ID. Specify NULL to open any.
 * @param password Password. Can be NULL if the server is running unsecured.
 */
PHIDGET21_API int CCONV CPhidget_openRemote(CPhidgetHandle phid, int serial, const char *serverID, const char *password);
/**
 * Opens a Phidget remotely by ServerID. Note that this requires Bonjour (mDNS) to be running on both the host and the server.
 * @param phid A phidget handle.
 * @param label Label string. Labels can be up to 10 characters (UTF-8 encoding). Specify NULL to open any.
 * @param serverID Server ID. Specify NULL to open any.
 * @param password Password. Can be NULL if the server is running unsecured.
 */
PHIDGET21_API int CCONV CPhidget_openLabelRemote(CPhidgetHandle phid, const char *label, const char *serverID, const char *password);
/**
 * Opens a Phidget remotely by address and port, with optional serial number.
 * @param phid A phidget handle.
 * @param serial Serial number. Specify -1 to open any.
 * @param address Address. This can be a hostname or IP address.
 * @param port Port number. Default is 5001.
 * @param password Password. Can be NULL if the server is running unsecured.
 */
PHIDGET21_API int CCONV CPhidget_openRemoteIP(CPhidgetHandle phid, int serial, const char *address, int port, const char *password);
/**
 * Opens a Phidget remotely by address and port, with optional label.
 * @param phid A phidget handle.
 * @param label Label string. Labels can be up to 10 characters (UTF-8 encoding). Specify NULL to open any.
 * @param address Address. This can be a hostname or IP address.
 * @param port Port number. Default is 5001.
 * @param password Password. Can be NULL if the server is running unsecured.
 */
PHIDGET21_API int CCONV CPhidget_openLabelRemoteIP(CPhidgetHandle phid, const char *label, const char *address, int port, const char *password);
/** @} */

/** \addtogroup phidmanager
 * @{
 */
/**
 * Opens a Phidget manager remotely by ServerID. Note that this requires Bonjour (mDNS) to be running on both the host and the server.
 * @param phidm A phidget manager handle.
 * @param serverID Server ID. Specify NULL to open any.
 * @param password Password. Can be NULL if the server is running unsecured.
 */
PHIDGET21_API int CCONV CPhidgetManager_openRemote(CPhidgetManagerHandle phidm, const char *serverID, const char *password);
/**
 * Opens a Phidget manager remotely by address and port.
 * @param phidm A phidget manager handle.
 * @param address Address. This can be a hostname or IP address.
 * @param port Port number. Default is 5001.
 * @param password Password. Can be NULL if the server is running unsecured.
 */
PHIDGET21_API int CCONV CPhidgetManager_openRemoteIP(CPhidgetManagerHandle phidm, const char *address, int port, const char *password);
/** @} */

/** \addtogroup phiddict
 * @{
 */
/**
 * Opens a Phidget dictionary by ServerID. Note that this requires Bonjour (mDNS) to be running on both the host and the server.
 * @param dict A phidget dictionary handle.
 * @param serverID Server ID. Specify NULL to open any.
 * @param password Password. Can be NULL if the server is running unsecured.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_openRemote(CPhidgetDictionaryHandle dict, const char *serverID, const char *password);
/**
 * Opens a Phidget dictionary by address and port.
 * @param dict A phidget dictionary handle.
 * @param address Address. This can be a hostname or IP address.
 * @param port Port number. Default is 5001.
 * @param password Password. Can be NULL if the server is running unsecured.
 */
PHIDGET21_API int CCONV CPhidgetDictionary_openRemoteIP(CPhidgetDictionaryHandle dict, const char *address, int port, const char *password);
/** @} */

#endif
