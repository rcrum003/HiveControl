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
#include "cphidgetgps.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"
#include <math.h>

// === Internal Functions === //
static int checkcrc(char *data, int crc);
static int parse_NMEA_data(char *data, CPhidgetGPSInfo *phid);
static int parse_GPS_packets(CPhidgetGPSInfo *phid);

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(GPS)
	TESTPTR(phid);

	phid->fix = PUNI_BOOL;

	phid->heading = PUNK_DBL;
	phid->velocity = PUNK_DBL;
	phid->altitude = PUNK_DBL;
	phid->latitude = PUNK_DBL;
	phid->longitude = PUNK_DBL;

	phid->haveTime = PUNK_BOOL;
	phid->haveDate = PUNK_BOOL;

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(GPS)
	unsigned char buffer[8] = { 0 };
	int readtries;
	TESTPTR(phid);

	phid->sckbuf_read=0;
	phid->sckbuf_write=0;
	ZEROMEM(phid->sckbuf, 256);

	phid->lastFix = PUNK_BOOL;
	phid->lastLatitude = PUNK_DBL;
	phid->lastLongitude = PUNK_DBL;
	phid->lastAltitude = PUNK_DBL;
	
	phid->fix = PUNK_BOOL;
	phid->heading = PUNK_DBL;
	phid->velocity = PUNK_DBL;
	phid->altitude = PUNK_DBL;
	phid->latitude = PUNK_DBL;
	phid->longitude = PUNK_DBL;

	phid->haveTime = PUNK_BOOL;
	phid->haveDate = PUNK_BOOL;

	//clean out the NMEA sentences
	ZEROMEM(&phid->GPSData, sizeof(NMEAData));

	//Examples for sending data. buffer[0] is dataLength, other spots are data
	//  continue in multiple packets if needed. Wait for ACK between sends
	//set factory defaults and reboot
	/*buffer[0]=0x02;buffer[1]=0x04;buffer[2]=0x01;
	CUSBSendPacket((CPhidgetHandle)phid, buffer);*/
	//set 10Hz update rate
	/*buffer[0]=0x03;buffer[1]=0x0e;buffer[2]=0x0a;buffer[3]=0x00;
	CUSBSendPacket((CPhidgetHandle)phid, buffer);*/
	//Get Ephemeris
	/*buffer[0]=0x02;buffer[1]=0x30;buffer[2]=0x00;
	CUSBSendPacket((CPhidgetHandle)phid, buffer);*/
	//Enable WAAS
	buffer[0]=0x03;buffer[1]=0x37;buffer[2]=0x01;buffer[2]=0x01;
	CUSBSendPacket((CPhidgetHandle)phid, buffer);
	//Query WAAS
	/*buffer[0]=0x01;buffer[1]=0x38;
	CUSBSendPacket((CPhidgetHandle)phid, buffer);*/

	//read some initial data - rate is 10Hz so we shouldn't have to wait long
	//This ensures we have received at least one GGA and one RMC
	readtries = 30; //250ms
	while((phid->fix==PUNK_BOOL || phid->haveTime==PUNK_BOOL || phid->haveDate==PUNK_BOOL) && readtries)
	{
		CPhidget_read((CPhidgetHandle)phid);
		readtries--;
	}
	if(phid->fix==PUNK_BOOL)
		phid->fix=PFALSE;

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(GPS)
	int i=0;

	if (length < 0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	//LOG(PHIDGET_LOG_DEBUG, "GPS Packet size: %d", buffer[0]);

	/* stick it in a buffer */
	for (i=0; i<buffer[0]; i++)
	{
		phid->sckbuf[phid->sckbuf_write++] = buffer[i+1];
	}
	parse_GPS_packets(phid);

	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(GPS)
	if(phid->fix != PUNK_BOOL)
		FIRE(PositionFixStatusChange, phid->fix);
	if (phid->fix == PTRUE && (phid->latitude != PUNK_DBL || phid->longitude != PUNK_DBL || phid->altitude != PUNK_DBL))
		FIRE(PositionChange, phid->latitude, phid->longitude, phid->altitude);
	return EPHIDGET_OK;
}

//getPacket - not used for GPS
CGETPACKET(GPS)
	return EPHIDGET_UNEXPECTED;
}

/* checks a CRC */
static int checkcrc(char *data, int crc) {
	unsigned int i=0;
	unsigned char check=0;
	for(i=1;i<strlen(data);i++)
		check^=data[i];
	if(check == crc)
		return 0;
	return 1;
}

/* this parses a full NMEA sentence */
static int parse_NMEA_data(char *data, CPhidgetGPSInfo *phid) {
	char *dataarray[50];
	int numfields = 0;
	int i,j,crc=0;
	double dintpart, decpart;
	int intpart;
	double tempD;

	//LOG(PHIDGET_LOG_DEBUG,"%s\n",data);

	/* fist check CRC if there is one */
	j = (int)strlen(data);
	for(i=0;i<j;i++) {
		if(data[i] == '*') {
			crc = strtol(data+i+1,NULL,16);
			data[i] = '\0';
			if(checkcrc(data, crc))
			{
				/* This will usually be caused by blocking in an event handler, which causes some packets to be dropped. 
					Safe to ignore. */
				LOG(PHIDGET_LOG_WARNING,"CRC Error parsing NMEA sentence.");
				return 1;
			}
			break;
		}
	}

	/* seperate out by commas */
	dataarray[0] = data;
	j = (int)strlen(data);
	for(i=0;i<j;i++) {
		if(data[i] == ',') {
			numfields++;
			dataarray[numfields] = data+i+1;
			data[i] = '\0';
		}
	}

	if(strlen(dataarray[0]) != 6)
	{
		LOG(PHIDGET_LOG_WARNING,"Bad sentence type.");
		return 1;
	}

	/* find the type of sentence */
	if(!strncmp("GGA",dataarray[0]+3,3)) {

		//time: HHMMSS.milliseconds
		if(strlen(dataarray[1]) >= 6) {
			decpart = modf(strtod(dataarray[1], NULL), &dintpart);
			intpart = (int)dintpart;
			phid->GPSData.GGA.time.tm_hour = (short)(intpart/10000);
			phid->GPSData.GGA.time.tm_min = (short)(intpart/100%100);
			phid->GPSData.GGA.time.tm_sec = (short)(intpart%100);
			phid->GPSData.GGA.time.tm_ms = (short)round(decpart*1000);
			phid->haveTime = PTRUE;
		}
		else
			phid->haveTime = PFALSE;

		/* convert lat/long to signed decimal degree format */
		if(strlen(dataarray[2])) {
			tempD = (int)(strtol(dataarray[2], NULL, 10) / 100) + (strtod((dataarray[2]+2),NULL) / 60);
			if(dataarray[3][0] == 'S')
				phid->GPSData.GGA.latitude = -tempD;
			else
				phid->GPSData.GGA.latitude = tempD;
		}
		else
			phid->GPSData.GGA.latitude = 0;

		if(strlen(dataarray[4])) {
			tempD = (int)(strtol(dataarray[4], NULL, 10) / 100) + (strtod((dataarray[4]+3),NULL) / 60);
			if(dataarray[5][0] == 'W')
				phid->GPSData.GGA.longitude = -tempD;
			else
				phid->GPSData.GGA.longitude = tempD;
		}
		else
			phid->GPSData.GGA.longitude = 0;

		phid->GPSData.GGA.fixQuality = (short)strtol(dataarray[6],NULL,10);
		phid->GPSData.GGA.numSatellites = (short)strtol(dataarray[7],NULL,10);
		phid->GPSData.GGA.horizontalDilution = strtod(dataarray[8],NULL);

		phid->GPSData.GGA.altitude = strtod(dataarray[9],NULL);
		phid->GPSData.GGA.heightOfGeoid = strtod(dataarray[11],NULL);

		//Set local variables for getters/events
		phid->fix = (phid->GPSData.GGA.fixQuality == 0) ? PFALSE : PTRUE;
		if(phid->fix)
		{
			phid->altitude = phid->GPSData.GGA.altitude;
			phid->latitude = phid->GPSData.GGA.latitude;
			phid->longitude = phid->GPSData.GGA.longitude;
		}
		else
		{
			phid->altitude = PUNK_DBL;
			phid->latitude = PUNK_DBL;
			phid->longitude = PUNK_DBL;
		}

		//Fix status changed
		if(phid->fix != phid->lastFix)
		{
			FIRE(PositionFixStatusChange, phid->fix);
			phid->lastFix = phid->fix;
		}

		/* only sends event if the fix is valid, and position has changed a bit */
		if (phid->fix == PTRUE
			&& (phid->latitude != phid->lastLatitude || phid->longitude != phid->lastLongitude || phid->altitude != phid->lastAltitude)
			)
		{
			FIRE(PositionChange, phid->latitude, phid->longitude, phid->altitude);
			phid->lastLatitude = phid->latitude;
			phid->lastLongitude = phid->longitude;
			phid->lastAltitude = phid->altitude;
		}
	}
	else if(!strncmp("GSA",dataarray[0]+3,3)) {
		phid->GPSData.GSA.mode = dataarray[1][0];
		phid->GPSData.GSA.fixType = (short)strtol(dataarray[2],NULL,10);
		for(i=0;i<12;i++)
			phid->GPSData.GSA.satUsed[i] = (short)strtol(dataarray[i+3],NULL,10);
		phid->GPSData.GSA.posnDilution = strtod(dataarray[15],NULL);
		phid->GPSData.GSA.horizDilution = strtod(dataarray[16],NULL);
		phid->GPSData.GSA.vertDilution = strtod(dataarray[17],NULL);
	}
	else if(!strncmp("GSV",dataarray[0]+3,3)) {
		int numSentences, sentenceNumber, numSats;

		numSentences = strtol(dataarray[1],NULL,10);
		sentenceNumber = strtol(dataarray[2],NULL,10);
		numSats = strtol(dataarray[3],NULL,10);

		phid->GPSData.GSV.satsInView = (short)numSats;
		for(i=0;i<(numSentences==sentenceNumber?numSats-(4*(numSentences-1)):4);i++) {
			phid->GPSData.GSV.satInfo[i+((sentenceNumber-1)*4)].ID = (short)strtol(dataarray[4+(i*4)],NULL,10);
			phid->GPSData.GSV.satInfo[i+((sentenceNumber-1)*4)].elevation = (short)strtol(dataarray[5+(i*4)],NULL,10);
			phid->GPSData.GSV.satInfo[i+((sentenceNumber-1)*4)].azimuth = strtol(dataarray[6+(i*4)],NULL,10);
			phid->GPSData.GSV.satInfo[i+((sentenceNumber-1)*4)].SNR = (short)strtol(dataarray[7+(i*4)],NULL,10);
		}
	}
	else if(!strncmp("RMC",dataarray[0]+3,3)) {
		if(strlen(dataarray[1])>=6) {
			decpart = modf(strtod(dataarray[1], NULL), &dintpart);
			intpart = (int)dintpart;
			phid->GPSData.RMC.time.tm_hour = (short)(intpart/10000);
			phid->GPSData.RMC.time.tm_min = (short)(intpart/100%100);
			phid->GPSData.RMC.time.tm_sec = (short)(intpart%100);
			phid->GPSData.RMC.time.tm_ms = (short)round(decpart*1000);
		}

		phid->GPSData.RMC.status = dataarray[2][0];

		/* convert lat/long to signed decimal degree format */
		if(strlen(dataarray[3])) {
			tempD = (int)(strtol(dataarray[3], NULL, 10) / 100) + (strtod((dataarray[3]+2),NULL) / 60);
			if(dataarray[4][0] == 'S')
				phid->GPSData.RMC.latitude = -tempD;
			else
				phid->GPSData.RMC.latitude = tempD;
		}
		else
			phid->GPSData.RMC.latitude = 0;

		if(strlen(dataarray[5])) {
			tempD = (int)(strtol(dataarray[5], NULL, 10) / 100) + (strtod((dataarray[5]+3),NULL) / 60);
			if(dataarray[6][0] == 'W')
				phid->GPSData.RMC.longitude = -tempD;
			else
				phid->GPSData.RMC.longitude = tempD;
		}
		else
			phid->GPSData.RMC.longitude = 0;

		phid->GPSData.RMC.speedKnots = strtod(dataarray[7],NULL);
		phid->GPSData.RMC.heading = strtod(dataarray[8],NULL);

		if(strlen(dataarray[9])>=6) {
			intpart = strtol(dataarray[9], NULL, 10);
			phid->GPSData.RMC.date.tm_mday = (short)(intpart/10000);
			phid->GPSData.RMC.date.tm_mon = (short)(intpart/100%100);
			phid->GPSData.RMC.date.tm_year = (short)(intpart%100) + 2000; //2-digit year, add 2000 years
			phid->haveDate = PTRUE;
		}
		else
			phid->haveDate = PFALSE;

		tempD = strtod(dataarray[10],NULL);
		if(dataarray[11][0] == 'W')
			phid->GPSData.RMC.magneticVariation = -tempD;
		else
			phid->GPSData.RMC.magneticVariation = tempD;

		phid->GPSData.RMC.mode = dataarray[12][0];

		if(phid->GPSData.RMC.status == 'A')
		{
			phid->velocity = phid->GPSData.RMC.speedKnots * 1.852; //convert to km/h
			phid->heading = phid->GPSData.RMC.heading;
		}
		else
		{
			phid->velocity = PUNK_DBL;
			phid->heading = PUNK_DBL;
		}
	}
	else if(!strncmp("VTG",dataarray[0]+3,3)) {
		phid->GPSData.VTG.trueHeading = strtod(dataarray[1],NULL);
		phid->GPSData.VTG.magneticHeading = strtod(dataarray[3],NULL);
		phid->GPSData.VTG.speedKnots = strtod(dataarray[5],NULL);
		phid->GPSData.VTG.speed = strtod(dataarray[7],NULL);
		phid->GPSData.VTG.mode = dataarray[9][0];
	}
	else {
		LOG(PHIDGET_LOG_INFO,"Unrecognized sentence type: %s", dataarray[0]+3);
	}

	return 0;
}

/* this parses a full sentence */
static int parse_SkyTraq_response(unsigned char *data, CPhidgetGPSInfo *phid) {
	int msgLength = data[3];
	int crc = data[msgLength + 4];
	int i, crccheck=0;
	char buffer[256];
	
	for(i=0;i<msgLength;i++)
	{
		crccheck^=data[i+4];
	}
	if(crc != crccheck)
	{
		LOG(PHIDGET_LOG_WARNING,"CRC Error parsing SkyTraq response.");
		return -1;
	}

	switch(data[4])
	{
		case 0x83: //ACK
			LOG(PHIDGET_LOG_INFO, "SkyTraq ACK: 0x%02x", data[5]);
			break;
		case 0x84: //NACK
			LOG(PHIDGET_LOG_INFO, "SkyTraq NACK: 0x%02x", data[5]);
			break;
		default:
			LOG(PHIDGET_LOG_INFO, "Got a SkyTraq message: 0x%02x", data[4]);
			buffer[0]=0;
			for(i=0;i<msgLength-1;i++)
			{
				if(i%8==0 && i!=0)
				{
					LOG(PHIDGET_LOG_INFO, "%s", buffer);
					buffer[0]=0;
				}
				sprintf(buffer+strlen(buffer)," 0x%02x", data[i+5]);
			}
			if(buffer[0])
				LOG(PHIDGET_LOG_INFO, "%s", buffer);
	}

	return 0;
}

/* this parses out the packets */
static int parse_GPS_packets(CPhidgetGPSInfo *phid) {

	unsigned char current_queuesize, msgsize, temp;
	int result, i=0;

	do {
		result = 0;
		/* Not known if packetsize is valid yet... */

		/* advance read ptr to '$' or 0xa0 */
		i=0;
		while ((i < 255) && (phid->sckbuf_read != phid->sckbuf_write))
		{
			if(phid->sckbuf[phid->sckbuf_read] == '$' || phid->sckbuf[phid->sckbuf_read] == 0xa0)
				break;
			i++;
			phid->sckbuf_read++;
		}
		
		current_queuesize = phid->sckbuf_write - phid->sckbuf_read;

		//response msg from skytraq - size is in posn 3
		if(phid->sckbuf[phid->sckbuf_read] == 0xa0)
		{
			unsigned char tempbuffer[256];
			if(current_queuesize < 4)
				break;
			msgsize = 7 + phid->sckbuf[(unsigned char)(phid->sckbuf_read + 3)];
			if(current_queuesize < msgsize)
				break;

			for (i = 0; i<msgsize; i++)
				tempbuffer[i] = phid->sckbuf[phid->sckbuf_read++];

			/* We know that we have at least a full sentence here... look for another */
			result=1;

			/* here we'll actually parse this sentence */
			if(parse_SkyTraq_response(tempbuffer, phid))
			{
				LOG(PHIDGET_LOG_WARNING,"Error parsing SkyTraq response.");
			}
		}
		else //NMEA/Other
		{
			char tempbuffer[256];
			/* find the end of the sentence */
			temp = phid->sckbuf_read;
			msgsize = 0;
			for (i=0; i < current_queuesize; i++,temp++)
			{
				if(phid->sckbuf[temp] == '\n') {
					msgsize = i;
					break;
				}
			}
			if(!msgsize) break; //couldn't find it

			for (i = 0; i<msgsize; i++)
				tempbuffer[i] = phid->sckbuf[phid->sckbuf_read++];
			tempbuffer[i] = 0;

			/* We know that we have at least a full sentence here... look for another */
			result=1;

			//NMEA - always starts with '$GP'
			if (current_queuesize >= 6 && tempbuffer[1] == 'G' && tempbuffer[2] == 'P')
			{

				/* here we'll actually parse this sentence */
				if(parse_NMEA_data(tempbuffer, phid))
				{
					LOG(PHIDGET_LOG_WARNING,"Error parsing NMEA sentence.");
				}
			}
			else //Something else that starts with a '$'
			{
				LOG(PHIDGET_LOG_INFO, "GPS Message: %s", tempbuffer);
			}
		}
	}
	while(result);

	return 0;
}

// === Exported Functions === //

//create and initialize a device structure
CCREATE(GPS, PHIDCLASS_GPS)

//event setup functions
CFHANDLE(GPS, PositionChange, double, double, double)
CFHANDLE(GPS, PositionFixStatusChange, int)

CGET(GPS,Latitude,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_GPS)
	TESTATTACHED
	TESTMASGN(latitude, PUNK_DBL)

	MASGN(latitude)
}

CGET(GPS,Longitude,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_GPS)
	TESTATTACHED
	TESTMASGN(longitude, PUNK_DBL)

	MASGN(longitude)
}

CGET(GPS,Altitude,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_GPS)
	TESTATTACHED
	TESTMASGN(altitude, PUNK_DBL)

	MASGN(altitude)
}

CGET(GPS,Time,GPSTime)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_GPS)
	TESTATTACHED
	if(phid->haveTime == PUNK_BOOL)
		return EPHIDGET_UNKNOWNVAL;

	MASGN(GPSData.GGA.time)
}

CGET(GPS,Date,GPSDate)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_GPS)
	TESTATTACHED
	if(phid->haveDate == PUNK_BOOL)
		return EPHIDGET_UNKNOWNVAL;

	MASGN(GPSData.RMC.date)
}

CGET(GPS,NMEAData,NMEAData)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_GPS)
	TESTATTACHED

	//Not uspported over webservice
	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		return EPHIDGET_UNSUPPORTED;

	MASGN(GPSData)
}

CGET(GPS,Heading,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_GPS)
	TESTATTACHED
	TESTMASGN(heading, PUNK_DBL)

	MASGN(heading)
}

CGET(GPS,Velocity,double)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_GPS)
	TESTATTACHED
	TESTMASGN(velocity, PUNK_DBL)

	MASGN(velocity)
}

CGET(GPS,PositionFixStatus,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_GPS)
	TESTATTACHED
	TESTMASGN(fix, PUNK_BOOL)

	MASGN(fix)
}
