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

#ifndef __CTHREAD
#define __CTHREAD

#ifndef EXTERNALPROTO

#ifdef _WINDOWS
typedef void *CThread_thread_t;
typedef DWORD CThread_func_return_t;
typedef LPVOID CThread_func_arg_t;
typedef CRITICAL_SECTION CThread_mutex_t;
typedef HANDLE EVENT;
typedef DWORD EVENT_TIME;
#else

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t condition;
	int ready_to_go;
} UNIX_EVENT;

typedef unsigned long EVENT_TIME;
typedef UNIX_EVENT EVENT;
typedef pthread_t CThread_thread_t;
typedef void *CThread_func_return_t;
typedef void *CThread_func_arg_t;
typedef pthread_mutex_t CThread_mutex_t;
#endif
typedef CThread_func_return_t(* CThread_func_t)(CThread_func_arg_t);

typedef struct {
	CThread_thread_t m_ThreadHandle;
	unsigned long m_ThreadIdentifier;
	unsigned char thread_status;
#ifdef _MACOSX
	CFRunLoopRef runLoop;
	unsigned char macInitDone;
#endif
} CThread;

int CThread_create(CThread *cp, CThread_func_t fp, CThread_func_arg_t arg);
int CThread_create_detached(CThread *cp, CThread_func_t fp, CThread_func_arg_t arg);
void CThread_join(CThread *cp);
//void CThread_kill(CThread *cp);
int CThread_is_my_thread(CThread cp);
int CThread_mutex_init(CThread_mutex_t *);
int CThread_mutex_destroy(CThread_mutex_t *mp);
void CThread_mutex_lock(CThread_mutex_t *mp);
void CThread_mutex_unlock(CThread_mutex_t *mp);

void CThread_set_event(EVENT *ev);
void CThread_reset_event(EVENT *ev);
int CThread_wait_on_event(EVENT *ev, EVENT_TIME time);
void CThread_create_event(EVENT *ev);
int CThread_destroy_event(EVENT *ev);

int RegisterLocalDevice(CPhidgetHandle phid);
int StartCentralThread();
int JoinCentralThread();

CThread_func_return_t ReadThreadFunction(CThread_func_arg_t arg);
CThread_func_return_t WriteThreadFunction(CThread_func_arg_t arg);

#endif
#endif
