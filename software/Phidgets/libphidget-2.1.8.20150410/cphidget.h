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

#ifndef __CPHIDGET
#define __CPHIDGET

/** \defgroup phidcommon Phidget Common
 * These calls are common to all Phidgets objects. See the <a class="el" href="http://www.phidgets.com/docs/General_Phidget_Programming" target="_blank">General Phidget Programming</a> page for more in-depth usage instructions and examples.
 * @{
 */
 
/**
 * A phidget handle.
 */
typedef struct _CPhidget *CPhidgetHandle;

/**
 * Timestamp structure - usually initialized to 0.
 */
typedef struct _CPhidget_Timestamp {
	int seconds;		/**< Number of seconds since timing began */
	int microseconds;	/**< Number of microseconds since last second passed - range is 0 - 999999 */
} CPhidget_Timestamp, *CPhidget_TimestampHandle;

#include "cphidgetattr.h"
#ifndef EXTERNALPROTO

#ifndef _WINDOWS
/**
 * Platform independent 64-bit integer.
 */
typedef long long __int64;
typedef unsigned long long __uint64;
#endif

#include "cphidgetconstantsinternal.h"
#include "cphidgetconstants.h"
#include "cthread.h"

typedef enum  {EVENTMODE_DATARATE=1, EVENTMODE_CHANGETRIGGER} CPhidget_EventMode;

typedef struct _CPhidget CPhidget;
//This is for a socket - one of these can be referenced by several phidgets!
typedef struct __CPhidgetSocketClient {
	int socket;
	char *port;
	char *address;
	void *pdcs;
	int status;
	CThread_mutex_t lock; /* protects server status */
	CThread_mutex_t pdc_lock; /* protects synchronous pdc functions */
	CThread auth_thread;
	CThread auth_error_thread;
	/* Heartbeat Vars */
	TIME lastHeartbeatTime;
	unsigned char waitingForHeartbeat;
	int heartbeatCount;
	double avgHeartbeatTime;
	int avgHeartbeatTimeCount;
	int heartbeat_listen_id;
	int runningEvent;
} CPhidgetSocketClient, *CPhidgetSocketClientHandle;

typedef struct __CPhidgetSocketClientList
{
	struct __CPhidgetSocketClientList *next;
	CPhidgetSocketClientHandle client;
} CPhidgetSocketClientList, *CPhidgetSocketClientListHandle;

typedef struct __CPhidgetRemote {
	CPhidgetSocketClientHandle server;	//can be NULL if not yet connected to a server
	char *requested_port;		//only !NULL if not zeroconf
	char *requested_address;	//only !NULL if not zeroconf
	char *requested_serverID;	//only !NULL if zeroconf
	char *password;				//can be NULL - will be used if needed for connecting to remote Phidgets
	int listen_id;
	int mdns;					//true if mdns, false if not
	char *zeroconf_name;		//for use before a connection is active
	char *zeroconf_domain;		//for use before a connection is active
	char *zeroconf_type;		//for use before a connection is active
	char *zeroconf_host;
	char *zeroconf_port;
	char *zeroconf_ipaddr;
	char *zeroconf_server_id;	//for use before a connection is active
	int zeroconf_interface;
	int zeroconf_auth;			//for use before a connection is active
	void *zeroconf_ref;	//service ref for mdns host/port lookups
	int cancelSocket;
	CThread_mutex_t zeroconf_ref_lock; /* protects zeroconf_ref */
	int uniqueConnectionID;
} CPhidgetRemote, *CPhidgetRemoteHandle;

typedef struct __CPhidgetRemoteList
{
	struct __CPhidgetRemoteList *next;
	CPhidgetRemoteHandle networkInfo;
} CPhidgetRemoteList, *CPhidgetRemoteListHandle;

typedef struct {
	CPhidget_DeviceID pdd_sdid;
	CPhidget_DeviceClass pdd_did;
	int pdd_vid;
	int pdd_pid;
	int pdd_iid;
	CPhidgetAttr pdd_attr;
	const char *pdd_name;
} CPhidgetDeviceDef;

typedef struct {
	CPhidget_DeviceUID pdd_uid;
	CPhidget_DeviceID pdd_id;
	int pdd_vlow;
	int pdd_vhigh;
} CPhidgetUniqueDeviceDef;

struct _CPhidget {
	CPhidgetRemoteHandle networkInfo; //NULL if local, !NULL if remote
	int(CCONV *fptrError)(CPhidgetHandle , void *, int, const char *);
	void *fptrErrorptr;
	int(CCONV *fptrServerConnect)(CPhidgetHandle , void *);
	void *fptrServerConnectptr; 
	int(CCONV *fptrServerDisconnect)(CPhidgetHandle , void *);
	void *fptrServerDisconnectptr;
	CThread_mutex_t lock; /* protects status */
	int status;
	CThread_mutex_t openCloseLock; /* protects status */
	int keyCount; //counts key during network open
	int initKeys; //number of initial keys during network open
	CThread_mutex_t writelock; /* protects write - exclusive */
	CThread readThread;
	CThread writeThread;
	HANDLE deviceHandle;
#ifdef _WINDOWS
	OVERLAPPED asyncRead;
	BOOL readPending;
	EVENT closeReadEvent;
	OVERLAPPED asyncWrite;
	unsigned char inbuf[MAX_IN_PACKET_SIZE+1];
#endif
	int specificDevice;
	CPhidget_DeviceClass deviceID;
	CPhidget_DeviceID deviceIDSpec;
	CPhidget_DeviceUID deviceUID;
	const CPhidgetDeviceDef *deviceDef;
	//int Phid_Device_Def_index;
	int deviceVersion;
	unsigned short ProductID;
	unsigned short VendorID;
	int serialNumber;
	const char *deviceType;
	unsigned short outputReportByteLength;
	unsigned short inputReportByteLength;
	char label[MAX_LABEL_STORAGE];
	char *escapedLabel; //for webservice
	char usbProduct[64];
	char firmwareUpgradeName[30];
	unsigned char GPPResponse;
	int(CCONV *fptrInit)(CPhidgetHandle);
	int(CCONV *fptrClear)(CPhidgetHandle);
	int(CCONV *fptrEvents)(CPhidgetHandle);
	int(CCONV *fptrClose)(CPhidgetHandle);
	int(CCONV *fptrFree)(CPhidgetHandle);
	int(CCONV *fptrData)(CPhidgetHandle, unsigned char *buffer, int length);
	int(CCONV *fptrGetPacket)(CPhidgetHandle, unsigned char *buffer,
	  unsigned int *length);
	unsigned char lastReadPacket[MAX_IN_PACKET_SIZE];
	unsigned char awdc_enabled;
	void *dnsServiceRef;
	void *errEventList;
#if defined(_MACOSX) && !defined(_IPHONE)
	io_object_t CPhidgetFHandle;
#else
	void *CPhidgetFHandle;
#endif
#ifdef _LINUX
	int tryAgainCounter;
#endif
#if !defined(_WINDOWS) || defined(WINCE)
	unsigned char interruptOutEndpoint;
#endif
	CThread_mutex_t outputLock; /* device-specific code responsible */
	EVENT writeAvailableEvent; /* device-specific code sets w/o OLL held */
	EVENT writtenEvent; /* device-specific code clears w/OLL held */
	int writeStopFlag; /* set when closing */
	int(CCONV *fptrAttach)(CPhidgetHandle , void *);
	void *fptrAttachptr; 
	int(CCONV *fptrDetach)(CPhidgetHandle , void *);
	void *fptrDetachptr;
	CPhidgetAttr attr;
};

struct _CPhidgetList
{
	struct _CPhidgetList *next;
	CPhidgetHandle phid;
} typedef CPhidgetList, *CPhidgetListHandle;

extern const char LibraryVersion[];
extern const char *Phid_DeviceName[PHIDGET_DEVICE_CLASS_COUNT];
extern const char Phid_UnknownErrorDescription[];
extern const char *Phid_ErrorDescriptions[PHIDGET_ERROR_CODE_COUNT];

extern CPhidgetList *ActiveDevices;
extern CPhidgetList *AttachedDevices;
extern int phidgetLocksInitialized;
extern CThread_mutex_t activeDevicesLock, attachedDevicesLock;

extern const CPhidgetDeviceDef Phid_Device_Def[];
extern const CPhidgetUniqueDeviceDef Phid_Unique_Device_Def[];

void CPhidgetFHandle_free(void *arg);
int CPhidget_read(CPhidgetHandle phid);
int CPhidget_write(CPhidgetHandle phid);
int CPhidget_statusFlagIsSet(int status, int flag);
int CPhidget_setStatusFlag(int *status, int flag, CThread_mutex_t *lock);
int CPhidget_clearStatusFlag(int *status, int flag, CThread_mutex_t *lock);
char translate_bool_to_ascii(char value);
const char *CPhidget_strerror(int error);
void throw_error_event(CPhidgetHandle phid, const char *error, int errcode);
int findActiveDevice(CPhidgetHandle attachedDevice);
int findActiveDevices();
int attachActiveDevice(CPhidgetHandle activeDevice, CPhidgetHandle attachedDevice);
double timestampdiff(CPhidget_Timestamp time1, CPhidget_Timestamp time2);
double timeSince(TIME *start);
void setTimeNow(TIME *now);
int encodeLabelString(const char *buffer, char *out, int *outLen);
int decodeLabelString(char *labelBuf, char *out, int serialNumber);
int UTF16toUTF8(char *in, int inBytes, char *out);
int labelHasWrapError(int serialNumber, char *labelBuf);
void CPhidgetErrorEvent_free(void *arg);
CPhidget_DeviceUID CPhidget_getUID(CPhidget_DeviceID id, int version);
int deviceSupportsGeneralUSBProtocol(CPhidgetHandle phid);

PHIDGET21_API int CCONV CPhidget_areEqual(void *arg1, void *arg2);
PHIDGET21_API int CCONV CPhidget_areExtraEqual(void *arg1, void *arg2);
PHIDGET21_API int CCONV CPhidgetHandle_areEqual(void *arg1, void *arg2);
PHIDGET21_API void CCONV CPhidget_free(void *arg);
PHIDGET21_API int CCONV phidget_type_to_id(const char *Type);
PHIDGET21_API int CCONV CPhidget_create(CPhidgetHandle *phid);

PHIDGET21_API int CCONV CPhidget_calibrate_gainoffset(CPhidgetHandle phid, int index, unsigned short offset, unsigned long gain);

//Some constants and function for new M3 Phidgets (General Packet Protocol)

//Bit 7 (MSB)
#define PHID_USB_GENERAL_PACKET_FLAG 0x80

//Bit 6 (used for return value on IN stream)
#define PHID_USB_GENERAL_PACKET_SUCCESS	0x00
#define PHID_USB_GENERAL_PACKET_FAIL	0x40

//Bits 0-5 (up to 31 types)
#define PHID_USB_GENERAL_PACKET_IGNORE 0x00
#define PHID_USB_GENERAL_PACKET_REBOOT_FIRMWARE_UPGRADE	0x01
#define PHID_USB_GENERAL_PACKET_REBOOT_ISP 0x02
//This is more data for when we need to send more then will fit in a single packet.
#define PHID_USB_GENERAL_PACKET_CONTINUATION 0x03
#define PHID_USB_GENERAL_PACKET_ZERO_CONFIG 0x04
#define PHID_USB_GENERAL_PACKET_WRITE_FLASH 0x05
#define PHID_USB_GENERAL_PACKET_FIRMWARE_UPGRADE_WRITE_SECTOR 0x06
#define PHID_USB_GENERAL_PACKET_SET_DS_TABLE 0x07
#define PHID_USB_GENERAL_PACKET_SET_DW_TABLE 0x08
#define PHID_USB_GENERAL_PACKET_FIRMWARE_UPGRADE_ERASE 0x09
#define PHID_USB_GENERAL_PACKET_ERASE_CONFIG 0x0A

#endif

#if !defined(EXTERNALPROTO) || defined(DEBUG)
PHIDGET21_API int CCONV CPhidgetGPP_reboot_firmwareUpgrade(CPhidgetHandle phid);
PHIDGET21_API int CCONV CPhidgetGPP_reboot_ISP(CPhidgetHandle phid);
PHIDGET21_API int CCONV CPhidgetGPP_setLabel(CPhidgetHandle phid, const char *buffer);
PHIDGET21_API int CCONV CPhidgetGPP_setDeviceSpecificConfigTable(CPhidgetHandle phid, unsigned char *data, int length, int index);
PHIDGET21_API int CCONV CPhidgetGPP_setDeviceWideConfigTable(CPhidgetHandle phid, unsigned char *data, int length, int index);
PHIDGET21_API int CCONV CPhidgetGPP_upgradeFirmware(CPhidgetHandle phid, unsigned char *data, int length);
PHIDGET21_API int CCONV CPhidgetGPP_eraseFirmware(CPhidgetHandle phid);
PHIDGET21_API int CCONV CPhidgetGPP_eraseConfig(CPhidgetHandle phid);
PHIDGET21_API int CCONV CPhidgetGPP_writeFlash(CPhidgetHandle phid);
#endif

#include "cphidgetmacros.h"

/**
 * Opens a Phidget.
 * @param phid A phidget handle.
 * @param serialNumber Serial number. Specify -1 to open any.
 */	
PHIDGET21_API int CCONV CPhidget_open(CPhidgetHandle phid, int serialNumber);
/**
 * Opens a Phidget by label.
 * @param phid A phidget handle.
 * @param label Label string. Labels can be up to 10 characters (UTF-8 encoding). Specify NULL to open any.
 */	
PHIDGET21_API int CCONV CPhidget_openLabel(CPhidgetHandle phid, const char *label);
/**
 * Closes a Phidget.
 * @param phid An opened phidget handle.
 */
PHIDGET21_API int CCONV CPhidget_close(CPhidgetHandle phid);
/**
 * Frees a Phidget handle.
 * @param phid A closed phidget handle.
 */
PHIDGET21_API int CCONV CPhidget_delete(CPhidgetHandle phid);
/**
 * Sets a detach handler callback function. This is called when this Phidget is unplugged from the system.
 * @param phid A phidget handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidget_set_OnDetach_Handler(CPhidgetHandle phid, int(CCONV *fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr);
/**
 * Sets an attach handler callback function. This is called when this Phidget is plugged into the system, and is ready for use.
 * @param phid A phidget handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidget_set_OnAttach_Handler(CPhidgetHandle phid, int(CCONV *fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr);
/**
 * Sets a server connect handler callback function. This is used for opening Phidgets remotely, and is called when a connection to the sever has been made.
 * @param phid A phidget handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidget_set_OnServerConnect_Handler(CPhidgetHandle phid, int (CCONV *fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr);
/**
 * Sets a server disconnect handler callback function. This is used for opening Phidgets remotely, and is called when a connection to the server has been lost.
 * @param phid A phidget handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidget_set_OnServerDisconnect_Handler(CPhidgetHandle phid, int (CCONV *fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr);
/**
 * Sets the error handler callback function. This is called when an asynchronous error occurs.
 * @param phid A phidget handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidget_set_OnError_Handler(CPhidgetHandle phid, int(CCONV *fptr)(CPhidgetHandle phid, void *userPtr, int errorCode, const char *errorString), void *userPtr);
/**
 * Gets the specific name of a Phidget.
 * @param phid An attached phidget handle.
 * @param deviceName A pointer which will be set to point to a char array containing the device name.
 */
PHIDGET21_API int CCONV CPhidget_getDeviceName(CPhidgetHandle phid, const char **deviceName);
/**
 * Gets the serial number of a Phidget.
 * @param phid An attached phidget handle.
 * @param serialNumber An int pointer for returning the serial number.
 */
PHIDGET21_API int CCONV CPhidget_getSerialNumber(CPhidgetHandle phid, int *serialNumber);
/**
 * Gets the firmware version of a Phidget.
 * @param phid An attached phidget handle.
 * @param deviceVersion An int pointer for returning the device version.
 */
PHIDGET21_API int CCONV CPhidget_getDeviceVersion(CPhidgetHandle phid, int *deviceVersion);
/**
 * Gets the attached status of a Phidget.
 * @param phid A phidget handle.
 * @param deviceStatus An int pointer for returning the device status. Possible codes are \ref PHIDGET_ATTACHED and \ref PHIDGET_NOTATTACHED.
 */
PHIDGET21_API int CCONV CPhidget_getDeviceStatus(CPhidgetHandle phid, int *deviceStatus);
/**
 * Gets the library version. This contains a version number and a build date.
 * @param libraryVersion A pointer which will be set to point to a char array containing the library version string.
 */
PHIDGET21_API int CCONV CPhidget_getLibraryVersion(const char **libraryVersion);
/**
 * Gets the type (class) of a Phidget.
 * @param phid An attached phidget handle.
 * @param deviceType A pointer which will be set to a char array containing the device type string.
 */
PHIDGET21_API int CCONV CPhidget_getDeviceType(CPhidgetHandle phid, const char **deviceType);
/**
 * Gets the label of a Phidget.
 * @param phid An attached phidget handle.
 * @param deviceLabel A pointer which will be set to a char array containing the device label string.
 */
PHIDGET21_API int CCONV CPhidget_getDeviceLabel(CPhidgetHandle phid, const char **deviceLabel);
/**
 * Sets the label of a Phidget. Note that this is nut supported on very old Phidgets, and not yet supported in Windows.
 * @param phid An attached phidget handle.
 * @param deviceLabel A string containing the label to be set.
 */
PHIDGET21_API int CCONV CPhidget_setDeviceLabel(CPhidgetHandle phid, const char *deviceLabel);
/**
 * Gets the description for an error code.
 * @param errorCode The error code to get the description of.
 * @param errorString A pointer which will be set to a char array containing the error description string.
 */
PHIDGET21_API int CCONV CPhidget_getErrorDescription(int errorCode, const char **errorString);
/**
 * Waits for attachment to happen. This can be called wirght after calling \ref CPhidget_open, as an alternative to using the attach handler.
 * @param phid An opened phidget handle.
 * @param milliseconds Time to wait for the attachment. Specify 0 to wait forever.
 */
PHIDGET21_API int CCONV CPhidget_waitForAttachment(CPhidgetHandle phid, int milliseconds);
/**
 * Gets the server ID of a remotely opened Phidget. This will fail if the Phidget was opened locally.
 * @param phid A connected phidget handle.
 * @param serverID A pointer which will be set to a char array containing the server ID string.
 */
PHIDGET21_API int CCONV CPhidget_getServerID(CPhidgetHandle phid, const char **serverID);
/**
 * Gets the address and port of a remotely opened Phidget. This will fail if the Phidget was opened locally.
 * @param phid A connected phidget handle.
 * @param address A pointer which will be set to a char array containing the address string.
 * @param port An int pointer for returning the port number.
 */
PHIDGET21_API int CCONV CPhidget_getServerAddress(CPhidgetHandle phid, const char **address, int *port);
/**
 * Gets the connected to server status of a remotely opened Phidget. This will fail if the Phidget was opened locally.
 * @param phid An opened phidget handle.
 * @param serverStatus An int pointer for returning the server status. Possible codes are \ref PHIDGET_ATTACHED and \ref PHIDGET_NOTATTACHED.
 */
PHIDGET21_API int CCONV CPhidget_getServerStatus(CPhidgetHandle phid, int *serverStatus);
/**
 * Gets the device ID of a Phidget.
 * @param phid An attached phidget handle.
 * @param deviceID The device ID constant.
 */
PHIDGET21_API int CCONV CPhidget_getDeviceID(CPhidgetHandle phid, CPhidget_DeviceID *deviceID);
/**
 * Gets the class of a Phidget Handle.
 * @param phid A phidget handle.
 * @param deviceClass The device class constant.
 */
PHIDGET21_API int CCONV CPhidget_getDeviceClass(CPhidgetHandle phid, CPhidget_DeviceClass *deviceClass);

#ifdef _MACOSX
/**
 * Sets the phidgets sleep handler callback function. This is called when the system is going to sleep, right before 
 * all Phidgets become inaccessible.
 * @param phid A phidget handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidget_set_OnWillSleep_Handler(int(CCONV *fptr)(void *userPtr), void *userPtr);
/**
 * Sets the phidgets wakeup handler callback function. This is called when the system wakes up from sleep, after the Phidgets are
 * accessible once again.
 * @param phid A phidget handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
PHIDGET21_API int CCONV CPhidget_set_OnWakeup_Handler(int(CCONV *fptr)(void *userPtr), void *userPtr);
#endif

/** @} */

#endif
