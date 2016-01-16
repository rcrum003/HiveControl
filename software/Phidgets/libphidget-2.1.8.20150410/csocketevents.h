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

#ifndef __CSOCKETEVENTS
#define __CSOCKETEVENTS

#include "cphidget.h"
#include "regex.h"
#include "utils/utils.h"
#include "cphidgetlist.h"
#include "pdictclient.h"
#ifdef USE_ZEROCONF
#include "dns_sd.h"
#endif

extern regex_t phidgetsetex;
extern regex_t managerex;
extern regex_t managervalex;

void network_phidget_event_handler(const char *key, const char *val, unsigned int len, pdict_reason_t reason, void *ptr);
void network_manager_event_handler(const char *key, const char *val, unsigned int len, pdict_reason_t, void *ptr);
void network_heartbeat_event_handler(const char *key, const char *val, unsigned int len, pdict_reason_t reason, void *ptr);

#endif
