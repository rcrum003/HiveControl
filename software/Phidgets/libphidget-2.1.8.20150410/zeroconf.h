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

#ifndef _ZEROCONF_H_
#define _ZEROCONF_H_

int getZeroconfHostPort(CPhidgetRemoteHandle networkInfo);
int cancelPendingZeroconfLookups(CPhidgetRemoteHandle networkInfo);
int refreshZeroconfSBC(CPhidgetSBCHandle sbc);
int refreshZeroconfPhidget(CPhidgetHandle phid);
int InitializeZeroconf();
int UninitializeZeroconf();

/* Internal version of .local lookups for SBC */
#ifdef ZEROCONF_LOOKUP
struct hostent *
mdns_gethostbyname (const char *name);
struct hostent *
mdns_gethostbyname2 (const char *name, int af);
#endif

#endif
