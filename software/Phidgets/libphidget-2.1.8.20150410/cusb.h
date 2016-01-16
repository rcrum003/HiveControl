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

#ifndef __CUSB
#define __CUSB

#include "cphidget.h"
#include "cphidgetlist.h"

#ifndef EXTERNALPROTO
int CUSBBuildList(CPhidgetList **curList);
int CUSBOpenHandle(CPhidgetHandle phid);
int CUSBCloseHandle(CPhidgetHandle phid);
int CUSBSetLabel(CPhidgetHandle phid, char *buffer);
void CUSBCleanup();
int CUSBSetupNotifications();
int CUSBRefreshLabelString(CPhidgetHandle phid);
int CUSBGetString(CPhidgetHandle phid, int index, char *str);
#if defined(_LINUX) && !defined(_ANDROID)
void CUSBUninit();
#else
int CUSBGetDeviceCapabilities(CPhidgetHandle phid, HANDLE DeviceHandle);
#endif
#endif

#ifdef _IPHONE
int CPhidgetManager_setupNotifications(CFRunLoopRef runloop);
int CPhidgetManager_teardownNotifications();
int reenumerateDevice(CPhidgetHandle phid);
#endif

PHIDGET21_API int CCONV CUSBReadPacket(CPhidgetHandle phidA, unsigned char *buffer);
PHIDGET21_API int CCONV CUSBSendPacket(CPhidgetHandle phidA, unsigned char *buffer);

#if defined(_ANDROID)
#include "com_phidgets_usb_Manager.h"
#endif

#endif
