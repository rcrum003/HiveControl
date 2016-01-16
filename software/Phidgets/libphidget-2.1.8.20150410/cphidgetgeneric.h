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

#ifndef __CPHIDGETGENERIC
#define __CPHIDGETGENERIC
#include "cphidget.h"

DPHANDLE(Generic)
CHDRSTANDARD(Generic)

CHDRGET(Generic,INPacketLength,int *length)
CHDRGET(Generic,OUTPacketLength,int *length)

CHDRGET(Generic,LastPacket,const unsigned char **packet, int *length)
CHDRSET(Generic,Packet,unsigned char *packet, int length)
CHDREVENT(Generic,Packet,const unsigned char *packet, int length)

#ifndef EXTERNALPROTO
struct _CPhidgetGeneric {
	CPhidget phid;

	int (CCONV *fptrPacket)(CPhidgetGenericHandle, void *, const unsigned char *, int);           
	void *fptrPacketptr;

	unsigned char lastPacket[MAX_IN_PACKET_SIZE];

	unsigned char buffer[MAX_OUT_PACKET_SIZE];

	int in, out;

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;
} typedef CPhidgetGenericInfo;
#endif

/** @} */

#endif
