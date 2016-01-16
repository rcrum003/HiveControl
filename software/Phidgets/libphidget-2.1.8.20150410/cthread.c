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

#include <assert.h>
#include "stdafx.h"
#include "cphidget.h"
#include "cphidgetmanager.h"
#include "cusb.h"
#include "cphidgetlist.h"

#if defined(_MACOSX) && !defined(_IPHONE)
#include "macusb.h"
#endif

CThread_func_return_t CentralThreadFunction(CThread_func_arg_t arg);

static CThread CentralThread;
static int checkForDevicesEventInitialized = PFALSE;
static EVENT checkForDevicesEvent;

/* used by OSX to pause traffic during sleep */
int pause_usb_traffic = PFALSE;
int usb_write_paused = PFALSE;
int usb_read_paused = PFALSE;

#ifdef _MACOSX
void macPeriodicTimerFunction(CFRunLoopTimerRef timer, void *Handle);
void macFindActiveDevicesSource(void *nothing);
CFRunLoopTimerRef timer = NULL;
CFRunLoopSourceRef findActiveDevicesSource = NULL;
#endif

#ifdef _WINDOWS

#if defined(_WINDOWS) && !defined(WINCE)
extern void initializeThreadSecurityAttributes();
#else
#define initializeThreadSecurityAttributes()
#endif
extern PSECURITY_ATTRIBUTES pSA;

#endif

int
StartCentralThread()
{

#ifdef _WINDOWS
	if (CentralThread.m_ThreadHandle) {
		int threadStatus = 0;
		int result = 0;
		result = GetExitCodeThread(CentralThread.m_ThreadHandle,
		    (LPDWORD)&threadStatus);
		if (result) {
			if (threadStatus != STILL_ACTIVE) {
				CloseHandle(CentralThread.m_ThreadHandle);
				CentralThread.m_ThreadHandle = 0;
			}
		}
	}
#endif

	if(checkForDevicesEventInitialized == PFALSE)
	{
		checkForDevicesEventInitialized = PTRUE;
		CThread_create_event(&checkForDevicesEvent);
	}
	
#ifdef _MACOSX
	if(findActiveDevicesSource == NULL)
	{
		CFRunLoopSourceContext sourceContext = {0};
		sourceContext.perform = macFindActiveDevicesSource;
		findActiveDevicesSource = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &sourceContext);
	}
	if(timer == NULL)
		timer = CFRunLoopTimerCreate(kCFAllocatorDefault, 0, 0.250, 0, 0, macPeriodicTimerFunction, NULL);
#endif

	//start the thread
	if (!CentralThread.m_ThreadHandle || CentralThread.thread_status == FALSE)
	{
#ifdef _MACOSX
		CentralThread.macInitDone = PFALSE;
#endif
		CThread_reset_event(&checkForDevicesEvent);

		if (CThread_create(&CentralThread, CentralThreadFunction, 0))
			return EPHIDGET_UNEXPECTED;
		CentralThread.thread_status = TRUE;
	}
	//if it was already running, signal it to check for device matches NOW
	else {
#ifdef _MACOSX
		//make sure mac thread stuff is initialized
		while(!CentralThread.macInitDone)
			SLEEP(10);
		//run findActiveDevices in the context of the central thread
		CFRunLoopSourceSignal(findActiveDevicesSource);
		CFRunLoopWakeUp(CentralThread.runLoop);
#else
		//signal thread to poll and findActiveDevices
		CThread_set_event(&checkForDevicesEvent);
#endif
	}
	return EPHIDGET_OK;
}

int
JoinCentralThread()
{
	if(CentralThread.m_ThreadHandle && !CThread_is_my_thread(CentralThread))
	{
#ifdef _MACOSX
		while(!CentralThread.macInitDone)
			SLEEP(10);
		CPhidgetManager_teardownNotifications();
		
		CFRunLoopRemoveTimer(CentralThread.runLoop, timer, kCFRunLoopDefaultMode);
		CFRunLoopRemoveSource(CentralThread.runLoop, findActiveDevicesSource, kCFRunLoopDefaultMode);
		
		CFRunLoopStop(CentralThread.runLoop);
		
		CentralThread.macInitDone = PFALSE;
#endif
		CThread_join(&CentralThread);
		CentralThread.m_ThreadHandle = 0;
	}
	return EPHIDGET_OK;
}

/*
 * registers a device to receive events and be polled by the central
 * thread This needs to start the central thread if it's not yet
 * running.
*/
int
RegisterLocalDevice(CPhidgetHandle phid)
{
	int result;

	TESTPTR(phid)
	
	if(!phidgetLocksInitialized)
	{
		CThread_mutex_init(&activeDevicesLock);
		CThread_mutex_init(&attachedDevicesLock);
		phidgetLocksInitialized = PTRUE;
	}
	CThread_mutex_lock(&activeDevicesLock);
	
	if(phid->specificDevice == PHIDGETOPEN_SERIAL || phid->specificDevice == PHIDGETOPEN_LABEL)
		result = CList_addToList((CListHandle *)&ActiveDevices, phid, CPhidget_areEqual);
	else
		result = CList_addToList((CListHandle *)&ActiveDevices, phid, CPhidgetHandle_areEqual);

	if (result)
	{
		CThread_mutex_unlock(&activeDevicesLock);
		return result;
	}
	CThread_mutex_unlock(&activeDevicesLock);

	result = StartCentralThread();
	return result;
}

#ifdef _MACOSX
//this is run every 250ms from the CentralThread runLoop
void macPeriodicTimerFunction(CFRunLoopTimerRef timer, void *Handle) {
	CPhidgetList *trav = 0;
	
	//looks for any devices that have set PHIDGET_USB_ERROR_FLAG and reenumerate them
	CThread_mutex_lock(&activeDevicesLock);
	for (trav=ActiveDevices; trav; trav = trav->next)
	{	
		if(CPhidget_statusFlagIsSet(trav->phid->status, PHIDGET_ATTACHED_FLAG))
		{
			if(CPhidget_statusFlagIsSet(trav->phid->status, PHIDGET_USB_ERROR_FLAG))
			{
				LOG(PHIDGET_LOG_WARNING,"PHIDGET_USB_ERROR_FLAG is set - cycling device through a reenumeration.");
				reenumerateDevice(trav->phid);
			}
		}
	}
	CThread_mutex_unlock(&activeDevicesLock);
	
	//need to always keep checking because a device could be opened by another app
	//and we want to notice when it becomes accessible.
	if(ActiveDevices) {
		findActiveDevices(); //this looks for attached active devices and opens them
	}
	
	return;
}

//This is run when a new Phidget is registered when the CentralThread is already running
void macFindActiveDevicesSource(void *nothing) {
	if(ActiveDevices) {
		findActiveDevices(); //this looks for attached active devices and opens them
	}
	return;
}
#endif

//The central thread should stop itself when there are no more active devices...? 
//Or we can stop it in unregisterlocaldevice

CThread_func_return_t CentralThreadFunction(CThread_func_arg_t lpdwParam)
{
#ifdef _MACOSX
	CentralThread.runLoop = CFRunLoopGetCurrent();

	CFRunLoopAddTimer(CentralThread.runLoop, timer, kCFRunLoopDefaultMode);
	CFRunLoopAddSource(CentralThread.runLoop, findActiveDevicesSource, kCFRunLoopDefaultMode);
	
	CentralThread.macInitDone = PTRUE;
	
	//setup notifications of Phidget attach/detach
	CPhidgetManager_setupNotifications(CentralThread.runLoop);
	
	//start run loop - note that this blocks until JoinCentralThread() is called.
	CFRunLoopRun();
#else
	//loop as long as there are active devices, or a phidget manager
	while(ActiveDevices || ActivePhidgetManagers) {
		CPhidgetManager_poll(); //this will update the list, as well as sending out attach and detach events
		findActiveDevices(); //this looks for attached active devices and opens them

		//Wait for signal of newly registered device, or 250ms
		//we don't really care about the reason so we don't check the return value
		CThread_wait_on_event(&checkForDevicesEvent, 250);
		CThread_reset_event(&checkForDevicesEvent);
	}
#endif

	//if we actually get here, it means there are no active devices, and no phidgetmanagers, so free up the
	//last of the memory we are using, and exit the thread...
	CThread_mutex_lock(&attachedDevicesLock);
	CList_emptyList((CListHandle *)&AttachedDevices, TRUE, CPhidget_free);
	CThread_mutex_unlock(&attachedDevicesLock);
	
	LOG(PHIDGET_LOG_INFO,"Central Thread exiting");
	
	if(fptrJavaDetachCurrentThread)
		fptrJavaDetachCurrentThread();
	CentralThread.thread_status = FALSE;
	return EPHIDGET_OK;
}

//The read thread
CThread_func_return_t ReadThreadFunction(CThread_func_arg_t lpdwParam)
{
	CPhidgetHandle phid = (CPhidgetHandle)lpdwParam;
	int result = EPHIDGET_OK;
	LOG(PHIDGET_LOG_INFO,"ReadThread running");
	
	if (!phid)
	{
		LOG(PHIDGET_LOG_ERROR,"ReadThread exiting - Invalid device handle");
		return (CThread_func_return_t)EPHIDGET_INVALIDARG;
	}
	
	//quit read thread if it's not needed
	switch(phid->deviceID)
	{
		case PHIDCLASS_SERVO:
			if(phid->deviceVersion < 313)
				goto exit_not_needed;
			break;
		case PHIDCLASS_INTERFACEKIT:
			if(phid->deviceIDSpec == PHIDID_INTERFACEKIT_0_0_4 && phid->deviceVersion < 704)
				goto exit_not_needed;
			break;
		case PHIDCLASS_LED:
			if(phid->deviceIDSpec == PHIDID_LED_64)
				goto exit_not_needed;
			break;
		case PHIDCLASS_TEXTLCD:
			if(phid->deviceIDSpec != PHIDID_TEXTLCD_ADAPTER)
				goto exit_not_needed;
			break;
		case PHIDCLASS_TEXTLED:
exit_not_needed:
			LOG(PHIDGET_LOG_INFO,"ReadThread exiting normally (Not Needed for this device)");
			goto exit;
		default:
			break;
	}

	while (CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
	{
		if(pause_usb_traffic)
		{
			usb_read_paused = PTRUE;
			SLEEP(20);
			continue;
		}
		else
			usb_read_paused = PFALSE;
		
		result=CPhidget_read(phid);

		switch(result)
		{
			case EPHIDGET_OK:
			case EPHIDGET_TRYAGAIN:
				break;
			case EPHIDGET_NOTATTACHED:
				LOG(PHIDGET_LOG_INFO,"ReadThread exiting normally (Phidget detach detected in CPhidget_read)");
				goto exit;
			case EPHIDGET_INTERRUPTED:
				LOG(PHIDGET_LOG_INFO,"ReadThread exiting normally (signaled by CPhidget_close)");
				goto exit;
			case EPHIDGET_TIMEOUT:
				//Set the error flag for devices that should never have a USB timeout, then exit
				//continue for devices where it is ok
				switch(phid->deviceID)
				{
				case PHIDCLASS_INTERFACEKIT:
					switch(phid->deviceIDSpec)
					{
					case PHIDID_INTERFACEKIT_0_16_16:
						if(phid->deviceVersion >= 601)
							goto timeout_not_ok;
						else
							goto timeout_ok;
					case PHIDID_INTERFACEKIT_0_8_8_w_LCD:
						goto timeout_ok;
					default:
						goto timeout_not_ok;
					}
				case PHIDCLASS_RFID:
					if(phid->deviceVersion >= 201)
						goto timeout_not_ok;
					else
						goto timeout_ok;
				case PHIDCLASS_ENCODER:
					if(phid->deviceIDSpec == PHIDID_ENCODER_HS_4ENCODER_4INPUT)
						goto timeout_not_ok;
					else
						goto timeout_ok;
				case PHIDCLASS_GENERIC:
					goto timeout_ok;
				default:
					goto timeout_not_ok;
				}
			timeout_not_ok:
				CPhidget_setStatusFlag(&phid->status, PHIDGET_USB_ERROR_FLAG, &phid->lock);
				LOG(PHIDGET_LOG_ERROR,"ReadThread exiting - unexpected timeout (could be an ESD event)");
				goto exit;
			timeout_ok:
				LOG(PHIDGET_LOG_VERBOSE,"CUSBReadPacket expected time out"); //verbose because it could happen a LOT
				break;
			case EPHIDGET_UNEXPECTED:
			default:
				LOG(PHIDGET_LOG_ERROR,"ReadThread exiting - CPhidget_read returned : %d",result);
				CPhidget_setStatusFlag(&phid->status, PHIDGET_USB_ERROR_FLAG, &phid->lock);
				goto exit;
		}
	}

	LOG(PHIDGET_LOG_INFO,"ReadThread exiting normally (Phidget detached)");
exit:
	if(fptrJavaDetachCurrentThread)
		fptrJavaDetachCurrentThread();
	phid->readThread.thread_status = FALSE;
	return (CThread_func_return_t)(size_t)result;
}

//The write thread
CThread_func_return_t WriteThreadFunction(CThread_func_arg_t lpdwParam)
{
	CPhidgetHandle phid = (CPhidgetHandle)lpdwParam;
	int result = EPHIDGET_OK, wait_return = 0;
	LOG(PHIDGET_LOG_INFO,"WriteThread running");

	if (!phid)
	{
		LOG(PHIDGET_LOG_ERROR,"WriteThread exiting - Invalid device handle");
		return (CThread_func_return_t)EPHIDGET_INVALIDARG;
	}

	//quit write thread if it's not needed
	switch(phid->deviceID)
	{
	case PHIDCLASS_INTERFACEKIT:
		if(phid->deviceIDSpec == PHIDID_LINEAR_TOUCH
			|| phid->deviceIDSpec == PHIDID_ROTARY_TOUCH)
			goto exit_not_needed;
		break;
	case PHIDCLASS_RFID:
		if(phid->deviceIDSpec == PHIDID_RFID)
			goto exit_not_needed;
		break;
	case PHIDCLASS_ENCODER:
		if(phid->deviceIDSpec == PHIDID_ENCODER_1ENCODER_1INPUT
			|| phid->deviceIDSpec == PHIDID_ENCODER_HS_1ENCODER)
			goto exit_not_needed;
		break;
	case PHIDCLASS_ACCELEROMETER:
	case PHIDCLASS_TEMPERATURESENSOR:
	case PHIDCLASS_PHSENSOR:
	case PHIDCLASS_WEIGHTSENSOR:
exit_not_needed:
		LOG(PHIDGET_LOG_INFO,"WriteThread exiting normally (Not Needed for this device)");
		goto exit;
	default:
		break;
	}

	while (CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG))
	{
		//if awdc_enabled is true, then we timeout in 200ms and do a write, otherwise no timeout
		wait_return = CThread_wait_on_event(&phid->writeAvailableEvent, 200);
		switch (wait_return) {
		case WAIT_TIMEOUT:
			//putting this in the timeout so actual writes are not missed.
			if(phid->writeStopFlag) 
			{
				LOG(PHIDGET_LOG_INFO,"WriteThread exiting normally (signaled by writeStopFlag)");
				goto exit;
			}
			if(!phid->awdc_enabled) //only fall through to writting out if awdc is set
				break;
		case WAIT_OBJECT_0: //writeAvailable
			if(pause_usb_traffic)
			{
				usb_write_paused = PTRUE;
				break;
			}
			else
				usb_write_paused = PFALSE;
					
			if((result = CPhidget_write(phid)))
			{
				switch(result)
				{
				case EPHIDGET_NOTATTACHED:
					LOG(PHIDGET_LOG_INFO,"WriteThread exiting normally (Phidget detach detected in CPhidget_write)");
					break;
				case EPHIDGET_INTERRUPTED:
					LOG(PHIDGET_LOG_INFO,"WriteThread exiting normally (signaled by CPhidget_close)");
					break;
				case EPHIDGET_TIMEOUT:
					LOG(PHIDGET_LOG_ERROR,"WriteThread exiting - unexpected timeout (could be an ESD event)");
					CPhidget_setStatusFlag(&phid->status, PHIDGET_USB_ERROR_FLAG, &phid->lock);
					break;
				case EPHIDGET_UNEXPECTED:
				default:
					LOG(PHIDGET_LOG_ERROR,"WriteThread exiting - CPhidget_write returned : %d",result);
					CPhidget_setStatusFlag(&phid->status, PHIDGET_USB_ERROR_FLAG, &phid->lock);
					break;
				}
				goto exit;
			}
			break;
		default:
			LOG(PHIDGET_LOG_ERROR,"WriteThread exiting - wait on phid->writeAvailableEvent failed");
			CPhidget_setStatusFlag(&phid->status, PHIDGET_USB_ERROR_FLAG, &phid->lock);
			result = EPHIDGET_UNEXPECTED;
			goto exit;
		}
	}
	LOG(PHIDGET_LOG_INFO,"WriteThread exiting normally (Phidget detached)");

exit:
	if(fptrJavaDetachCurrentThread)
		fptrJavaDetachCurrentThread();
	phid->writeStopFlag = FALSE;
	phid->writeThread.thread_status = FALSE;
	return (CThread_func_return_t)(size_t)result;
}

int
CThread_create(CThread *cp, CThread_func_t fp, CThread_func_arg_t arg)
{
#ifdef _WINDOWS
	initializeThreadSecurityAttributes();
	cp->m_ThreadHandle = CreateThread(pSA, 0, (LPTHREAD_START_ROUTINE)fp, arg, 0, &cp->m_ThreadIdentifier);
	if(cp->m_ThreadHandle) return EPHIDGET_OK;
	else return GetLastError();
#else
	return pthread_create(&cp->m_ThreadHandle, NULL, fp, arg);
#endif
}

int
CThread_create_detached(CThread *cp, CThread_func_t fp, CThread_func_arg_t arg)
{
#ifdef _WINDOWS
	cp->m_ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fp, arg, 0, &cp->m_ThreadIdentifier);
	if(cp->m_ThreadHandle) return EPHIDGET_OK;
	else return GetLastError();
#else
	pthread_attr_t attr;
	int err;
	if((err = pthread_attr_init(&attr)) == 0)
	{
		if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) == 0)
		{
			return pthread_create(&cp->m_ThreadHandle, &attr, fp, arg);
		}
		else
		{
			LOG(PHIDGET_LOG_ERROR, "pthread_attr_setdetachstate failed with error: %d",err);
			return err;
		}
	}
	else
	{
		LOG(PHIDGET_LOG_ERROR, "pthread_attr_init failed with error: %d",err);
		return err;
	}
#endif
}

int
CThread_is_my_thread(CThread cp)
{
#ifdef _WINDOWS
	return (cp.m_ThreadIdentifier == GetCurrentThreadId());
#else
	return pthread_equal(cp.m_ThreadHandle, pthread_self());
#endif
}

void
CThread_join(CThread *cp)
{
#ifdef _WINDOWS
	DWORD ec;

	while (GetExitCodeThread(cp->m_ThreadHandle, &ec) && ec == STILL_ACTIVE)
		SLEEP(10);
	CloseHandle(cp->m_ThreadHandle);
	cp->m_ThreadHandle = NULL;

#else
	if (cp->thread_status == TRUE)
		pthread_join(cp->m_ThreadHandle, 0);
#endif
}

/*void
CThread_kill(CThread *cp)
{
#ifdef _WINDOWS
	TerminateThread(cp->m_ThreadHandle, 0);
#else
	pthread_cancel(cp->m_ThreadHandle);
#endif
}*/

int
CThread_mutex_init(CThread_mutex_t *mp)
{
#ifdef _WINDOWS
	InitializeCriticalSection(mp);
	return 1;
#else
	return pthread_mutex_init(mp, NULL) == 0;
#endif
}

int
CThread_mutex_destroy(CThread_mutex_t *mp)
{
#ifdef _WINDOWS
	DeleteCriticalSection(mp);
	return 1;
#else
	return pthread_mutex_destroy(mp) == 0;
#endif
}

void
CThread_mutex_lock(CThread_mutex_t *mp)
{
#ifdef _WINDOWS
	EnterCriticalSection(mp);
#else
	pthread_mutex_lock(mp);
#endif
}

void
CThread_mutex_unlock(CThread_mutex_t *mp)
{
#ifdef _WINDOWS
	LeaveCriticalSection(mp);
#else
	pthread_mutex_unlock(mp);
#endif
}

void CThread_create_event(EVENT *ev)
{
#ifdef _WINDOWS
	*ev = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    pthread_mutex_init(&ev->mutex, NULL);
    pthread_cond_init(&ev->condition, NULL);
	ev->ready_to_go = PFALSE;
#endif
}

int CThread_destroy_event(EVENT *ev)
{
#ifdef _WINDOWS
	return CloseHandle(*ev);
#else
    if(pthread_mutex_destroy(&ev->mutex)) return 0;
    if(pthread_cond_destroy(&ev->condition)) return 0;
	return 1;
#endif
}

int CThread_wait_on_event(EVENT *ev, EVENT_TIME time)
{
#ifdef _WINDOWS
	return WaitForSingleObject(*ev, time);
#else
	int retval;
	struct timespec timeout;
	struct timeval now;
	
    // Lock the mutex.
    pthread_mutex_lock(&ev->mutex);
 
    // If the predicate is already set, then the while loop is bypassed;
    // otherwise, the thread sleeps until the predicate is set.
    if(ev->ready_to_go == PFALSE)
    {
		if(time == INFINITE)
			retval = pthread_cond_wait(&ev->condition, &ev->mutex);
		else {
			gettimeofday(&now,0);
			timeout.tv_sec = now.tv_sec + time/1000;
			timeout.tv_nsec = now.tv_usec*1000 + (time%1000 * 1000000);
			if(timeout.tv_nsec >= 1000000000)
			{
				timeout.tv_sec++;
				timeout.tv_nsec -= 1000000000;
			}
			retval = pthread_cond_timedwait(&ev->condition, &ev->mutex, &timeout);
		}
		
		switch(retval)
		{
			case ETIMEDOUT:
				pthread_mutex_unlock(&ev->mutex); 
				return WAIT_TIMEOUT;
			case 0:
				pthread_mutex_unlock(&ev->mutex); 
				return WAIT_OBJECT_0;
			case EINVAL:
				pthread_mutex_unlock(&ev->mutex); 
				return WAIT_FAILED;
			default:
				pthread_mutex_unlock(&ev->mutex); 
				return WAIT_FAILED;
		}
    }
	pthread_mutex_unlock(&ev->mutex); 
	return WAIT_OBJECT_0;
#endif
}

void CThread_reset_event(EVENT *ev)
{
#ifdef _WINDOWS
	ResetEvent(*ev);
#else
    // Reset the predicate and release the mutex.
    pthread_mutex_lock(&ev->mutex);
    ev->ready_to_go = PFALSE;
    pthread_mutex_unlock(&ev->mutex); 
#endif
}

void CThread_set_event(EVENT *ev)
{
#ifdef _WINDOWS
	SetEvent(*ev);
#else
    // At this point, there should be work for the other thread to do.
    pthread_mutex_lock(&ev->mutex);
    ev->ready_to_go = PTRUE;
    // Signal the other thread to begin work.
    pthread_cond_signal(&ev->condition);
    pthread_mutex_unlock(&ev->mutex);
 
#endif
}

