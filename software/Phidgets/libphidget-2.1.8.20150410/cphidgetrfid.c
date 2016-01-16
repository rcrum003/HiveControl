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
#include "cphidgetrfid.h"
#include "stdio.h"
#include "cusb.h"
#include "csocket.h"
#include "cthread.h"

// === Internal Functions === //
CThread_func_return_t tagTimerThreadFunction(CThread_func_arg_t userPtr);
static int analyze_data(CPhidgetRFIDHandle phid);
static int tagEvent_fromEM4100Data(CPhidgetRFIDHandle phid, unsigned char *data);
#ifdef RFID_HITAGS_SUPPORT
static int analyze_data_AC(CPhidgetRFIDHandle phid);
static int HitagS_UID_REQUEST(CPhidgetRFIDHandle phid);
#endif

//clearVars - sets all device variables to unknown state
CPHIDGETCLEARVARS(RFID)
	int i = 0;

	phid->antennaEchoState = PUNI_BOOL;
	phid->ledEchoState = PUNI_BOOL;
	phid->tagPresent = PUNI_BOOL;
	phid->lastTagValid = PUNI_BOOL;
	for (i = 0; i<RFID_MAXOUTPUTS; i++)
	{
		phid->outputEchoState[i] = PUNI_BOOL;
	}

	return EPHIDGET_OK;
}

//initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
CPHIDGETINIT(RFID)
	int i = 0;
	int result;
	TESTPTR(phid);
	
	//make sure the tagTimerThread isn't running
	if (phid->tagTimerThread.thread_status == PTRUE)
	{
		phid->tagTimerThread.thread_status = PFALSE;
		CThread_join(&phid->tagTimerThread);
	}
	phid->tagTimerThread.thread_status = PFALSE;

	//Make sure no old writes are still pending
	phid->outputPacketLen = 0;

	//setup anything device specific
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_OLD:
		case PHIDUID_RFID:
			phid->fullStateEcho = PFALSE;
			phid->antennaEchoState = PTRUE;
			break;
		case PHIDUID_RFID_2OUTPUT_NO_ECHO:
			phid->fullStateEcho = PFALSE;
			phid->antennaEchoState = PUNK_BOOL;
			break;
		case PHIDUID_RFID_2OUTPUT:
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
			phid->fullStateEcho = PTRUE;
			phid->antennaEchoState = PUNK_BOOL;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//set data arrays to unknown
	for (i = 0; i<phid->phid.attr.rfid.numOutputs; i++)
	{
		phid->outputEchoState[i] = PUNK_BOOL;
	}

	phid->ledEchoState = PUNK_BOOL;
	phid->tagPresent = PUNK_BOOL;
	ZEROMEM(&phid->lastTag, sizeof(CPhidgetRFID_Tag));
	ZEROMEM(&phid->pendingTag, sizeof(CPhidgetRFID_Tag));
	phid->tagEventPending = PFALSE;
	phid->lastTagValid = PFALSE;

	phid->pregapClocksEcho = PUNK_INT;
	phid->prepulseClocksEcho = PUNK_INT;
	phid->zeroClocksEcho = PUNK_INT;
	phid->oneClocksEcho = PUNK_INT;
	phid->spaceClocksEcho = PUNK_INT;
	phid->postgapClocksEcho = PUNK_INT;
	phid->eofpulseClocksEcho = PUNK_INT;
	phid->listenDuringEOFEcho = PUNK_BOOL;

	phid->_4097ConfEcho = PUNK_INT;

	phid->dataReadPtr = 0;
	phid->dataWritePtr = 0;
	phid->manReadPtr = 0;
	phid->manWritePtr = 0;
	phid->biphaseReadPtr = 0;
	phid->biphaseWritePtr = 0;

	phid->shortClocks = phid->longClocks = 0;
	
	phid->manLockedIn = PFALSE;
	phid->biphaseLockedIn = PFALSE;
	phid->manShortChange = PFALSE;
	phid->biphaseShortChange = PFALSE;
	
	CThread_reset_event(&phid->tagAvailableEvent);

#ifdef RFID_HITAGS_SUPPORT
	phid->dataReadACPtr = 0;
	phid->ACCodingOK = PFALSE;
	phid->polling = PFALSE;
	setTimeNow(&phid->lastDataTime);
	setTimeNow(&phid->hitagReqTime);
#endif

#ifdef RFID_RAWDATA_API_SUPPORT
	phid->userReadPtr = 0;
	phid->manEventReadPtr = 0;
	phid->lastManEventLong = PFALSE;
#endif

	//send out any initial pre-read packets
	switch(phid->phid.deviceUID) {
		case PHIDUID_RFID_OLD:
		{
			unsigned char buffer[8] = { 0 };
			ZEROMEM(buffer,8);
			LOG(PHIDGET_LOG_INFO,"Sending workaround startup packet");
			if ((result = CUSBSendPacket((CPhidgetHandle)phid, buffer)) != EPHIDGET_OK)
				return result;
			break;
		}
		default:
			break;
	}

	//issue a read for devices that return output data
	if(phid->fullStateEcho)
	{
		int readtries = 16; //should guarentee a packet with output data - even if a tag is present
		while(readtries-- > 0)
		{
			CPhidget_read((CPhidgetHandle)phid);
			if(phid->outputEchoState[0] != PUNK_BOOL)
					break;
		}
		//one more read guarantees that if there is a tag present, we will see it - output packets only happen every 255ms
		CPhidget_read((CPhidgetHandle)phid);
	}

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
			//Did we get some data? Wait for a tag
			if(phid->dataWritePtr != 0)
			{	
				int readtries = 30; //This should be enough data to guarantee detection of a tag.
				while(readtries-- > 0)
				{
					CPhidget_read((CPhidgetHandle)phid);
					if(phid->tagPresent != PUNK_BOOL)
							break;
				}
			}
			break;
		default:
			break;
	}

	//if the antenna is on, and tagPresent is unknown, then it is false
	if(phid->antennaEchoState == PTRUE && phid->tagPresent == PUNK_BOOL)
		phid->tagPresent = PFALSE;

	//So that we can get the tag in the attach handler
	if(phid->tagPresent == PTRUE)
	{
		phid->lastTag = phid->pendingTag;
		phid->lastTagValid = PTRUE;
	}

	//recover what we can - if anything isn't filled out, it's PUNK anyways
	for (i = 0; i<phid->phid.attr.rfid.numOutputs; i++)
	{
		phid->outputState[i] = phid->outputEchoState[i];
	}
	phid->antennaState = phid->antennaEchoState;
	phid->ledState = phid->ledEchoState;
	
	phid->pregapClocks = phid->pregapClocksEcho;
	phid->postgapClocks = phid->postgapClocksEcho;
	phid->zeroClocks = phid->zeroClocksEcho;
	phid->oneClocks = phid->oneClocksEcho;
	phid->spaceClocks = phid->spaceClocksEcho;
	phid->eofpulseClocks = phid->eofpulseClocksEcho;
	phid->prepulseClocks = phid->prepulseClocksEcho;
	phid->listenDuringEOF = phid->listenDuringEOFEcho;
	phid->_4097Conf = phid->_4097ConfEcho;

	return EPHIDGET_OK;
}

//dataInput - parses device packets
CPHIDGETDATA(RFID)
	int i = 0, j = 0;
	unsigned char newStateData = PFALSE;
	unsigned char outputs[RFID_MAXOUTPUTS];
	unsigned char antennaState, ledState;

	if (length<0) return EPHIDGET_INVALIDARG;
	TESTPTR(phid);
	TESTPTR(buffer);

	//Parse device packets - store data locally
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_OLD:
		case PHIDUID_RFID:
			// Enything other then all 0's means a tag is detected
			if(memcmp("\0\0\0\0\0", buffer+1, 5))
				tagEvent_fromEM4100Data(phid, buffer+1);
			break;

		case PHIDUID_RFID_2OUTPUT_NO_ECHO:
		case PHIDUID_RFID_2OUTPUT:
			switch(buffer[0])
			{
				case RFID_PACKET_TAG:
					// Enything other then all 0's means a tag is detected
					if(memcmp("\0\0\0\0\0", buffer+1, 5))
						tagEvent_fromEM4100Data(phid, buffer+1);
					break;
				case RFID_PACKET_OUTPUT_ECHO:
					if(phid->fullStateEcho)
					{
						newStateData = PTRUE;

						for (i = 0, j = 0x01; i < phid->phid.attr.rfid.numOutputs; i++, j<<=1)
						{
							if (buffer[1] & j)
								outputs[i] = PTRUE;
							else
								outputs[i] = PFALSE;
						}

						if(buffer[1] & RFID_LED_FLAG)
							ledState = PTRUE;
						else
							ledState = PFALSE;

						if(buffer[1] & RFID_ANTENNA_FLAG)
							antennaState = PTRUE;
						else
							antennaState = PFALSE;
					}
					break;
				default:
					return EPHIDGET_UNEXPECTED;
			}
			break;

		// RFID with decoding in software and write support
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
		{
			int dataLength = 0;
			int dataOffset = 1;
			int data[RFID_MAX_DATA_PER_PACKET];
			switch(buffer[0] & 0x40)
			{
				case RFID_ECHO_IN_PACKET:
					dataOffset = 11;
					newStateData = PTRUE;
					
					//Don't bother - we don't use it.
					//phid->frequencyEcho = buffer[1] * 1000;

					phid->pregapClocksEcho = buffer[2];
					phid->prepulseClocksEcho = buffer[7];
					phid->zeroClocksEcho = buffer[4];
					phid->oneClocksEcho = buffer[5];
					phid->spaceClocksEcho = buffer[6];
					phid->postgapClocksEcho = buffer[3];
					phid->eofpulseClocksEcho = buffer[8];

					for (i = 0, j = 0x01; i < phid->phid.attr.rfid.numOutputs; i++, j<<=1)
					{
						if (buffer[9] & j)
							outputs[i] = PTRUE;
						else
							outputs[i] = PFALSE;
					}

					if(buffer[9] & RFID_LED_FLAG)
						ledState = PTRUE;
					else
						ledState = PFALSE;

					if(buffer[9] & RFID_ANTENNA_FLAG)
						antennaState = PTRUE;
					else
						antennaState = PFALSE;

					if(buffer[9] & RFID_LISTEN_DURING_EOF_FLAG)
						phid->listenDuringEOFEcho = PTRUE;
					else
						phid->listenDuringEOFEcho = PFALSE;

					phid->_4097ConfEcho = buffer[10];

					//NOTE: Fall Through
				case RFID_READ_DATA_IN_PACKET:
					
					//move RFID data into local storage
					dataLength = buffer[0] & 0x3F;
					for(i = 0; i < dataLength; i++)
					{
						data[i] = buffer[i+dataOffset] << 1;
						if((data[i] & 0xFE) == 0xFE)
						{
							data[i] = PUNK_INT;
							phid->dataBuffer[phid->dataWritePtr] = PUNK_INT;
						}
						else
						{
							// convert to data lengths that we expect to deal with internally
							int polarity = data[i] & 0x100;
							int clocks = data[i] & 0xff;

							if(clocks >= 10 && clocks <= 22)
								phid->dataBuffer[phid->dataWritePtr] = polarity | 16;
							else if(clocks >= 26 && clocks <= 40)
								phid->dataBuffer[phid->dataWritePtr] = polarity | 32;
							else if(clocks >= 42 && clocks <= 54)
								phid->dataBuffer[phid->dataWritePtr] = polarity | 48;
							else if(clocks >= 56 && clocks <= 72)
								phid->dataBuffer[phid->dataWritePtr] = polarity | 64;
							else if(clocks >= 120 && clocks <= 136)
								phid->dataBuffer[phid->dataWritePtr] = polarity | 128;
							else 
								phid->dataBuffer[phid->dataWritePtr] = PUNK_INT;
						}

						phid->dataWritePtr++;
						phid->dataWritePtr &= RFID_DATA_ARRAY_MASK;

						//if we run into data that hasn't been read... too bad, we overwrite it and adjust the read pointer
						if(phid->dataWritePtr == phid->dataReadPtr)
						{
							phid->dataReadPtr++;
							phid->dataReadPtr &= RFID_DATA_ARRAY_MASK;
						}
#ifdef RFID_HITAGS_SUPPORT
						if(phid->dataWritePtr == phid->dataReadACPtr)
						{
							phid->dataReadACPtr++;
							phid->dataReadACPtr &= RFID_DATA_ARRAY_MASK;
						}
#endif
					}

					break;
				default:
					return EPHIDGET_UNEXPECTED;
			}

			if(dataLength)
			{

#ifdef RFID_RAWDATA_API_SUPPORT
				FIRE(RawData, data, dataLength);
#endif

				//analyze data
				analyze_data(phid);

#ifdef RFID_HITAGS_SUPPORT
				setTimeNow(&phid->lastDataTime);
				if(phid->ACCodingOK)
					analyze_data_AC(phid);
				else
					phid->dataReadACPtr = phid->dataWritePtr;
#endif
			}
			break;
		}
		default:
			return EPHIDGET_UNEXPECTED;
	}

	//Make sure values are within defined range, and store to structure
	if(newStateData)
	{
		unsigned char lastOutputs[RFID_MAXOUTPUTS];

		for (i = 0; i < phid->phid.attr.rfid.numOutputs; i++)
		{
			lastOutputs[i] = phid->outputEchoState[i];
			phid->outputEchoState[i] = outputs[i];
		}

		phid->ledEchoState = ledState;
		phid->antennaEchoState = antennaState;

		//Events
		for (i = 0; i < phid->phid.attr.rfid.numOutputs; i++)
		{
			if(lastOutputs[i] != phid->outputEchoState[i])
				FIRE(OutputChange, i, phid->outputEchoState[i]);
		}
	}
	
	return EPHIDGET_OK;
}

//eventsAfterOpen - sends out an event for all valid data, used during attach initialization
CPHIDGETINITEVENTS(RFID)

	if(phid->fullStateEcho)
	{
		for (i = 0; i < phid->phid.attr.rfid.numOutputs; i++)
		{
			if(phid->outputEchoState[i] != PUNK_BOOL)
				FIRE(OutputChange, i, phid->outputEchoState[i]);
		}
	}

	//Initial non-remote tag events are sent from the tagTimerThread

	//Don't start the tag thread if this is a networked Phidget
	CThread_mutex_lock(&phid->phid.lock);
	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_ATTACHED_FLAG))
	{
		//For remote - if there is a tag present, send the tag event here
		if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		{
			CThread_mutex_unlock(&phid->phid.lock);
			if(phid->tagPresent == PTRUE && phid->lastTagValid == PTRUE)
			{
				if(phid->lastTag.protocol == PHIDGET_RFID_PROTOCOL_EM4100)
					FIRE(Tag, phid->lastTag.tagData);
				FIRE(Tag2, phid->lastTag.tagString, phid->lastTag.protocol);
			}
		}
		else
		{
			CThread_mutex_unlock(&phid->phid.lock);
			//Start the tagTimerThread - do it here because we are about to start the read thread, and that will keep it active
			phid->tagTimerThread.thread_status = PTRUE;
			if (CThread_create(&phid->tagTimerThread, tagTimerThreadFunction, phid))
			{
				phid->tagTimerThread.thread_status = PFALSE;
				return EPHIDGET_UNEXPECTED;
			}
		}
	}
	else
	{
		CThread_mutex_unlock(&phid->phid.lock);
		return EPHIDGET_NOTATTACHED;
	}

	return EPHIDGET_OK;
}

//Extra things to do during a close
//This is run before the other things that close does
int CPhidgetRFID_close(CPhidgetHandle phidG)
{
	CPhidgetRFIDHandle phid = (CPhidgetRFIDHandle)phidG;
	//make sure the tagTimerThread isn't running
	if (phid->tagTimerThread.thread_status == PTRUE)
	{
		phid->tagTimerThread.thread_status = PFALSE;
		CThread_join(&phid->tagTimerThread);
	}
	return EPHIDGET_OK;
}

//Extra things to do during a free
//This is run before the other things that free does
int CPhidgetRFID_free(CPhidgetHandle phidG)
{
	CPhidgetRFIDHandle phid = (CPhidgetRFIDHandle)phidG;
	CThread_mutex_destroy(&phid->tagthreadlock);
	CThread_destroy_event(&phid->tagAvailableEvent);
	return EPHIDGET_OK;
}

//getPacket - used by write thread to get the next packet to send to device
CGETPACKET_BUF(RFID)

//sendpacket - sends a packet to the device asynchronously, blocking if the 1-packet queue is full
CSENDPACKET_BUF(RFID)

//makePacket - constructs a packet using current device state
CMAKEPACKET(RFID)
	int i = 0, j = 0;

	TESTPTRS(phid, buffer);

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_2OUTPUT_NO_ECHO:
		case PHIDUID_RFID_2OUTPUT:
			//have to make sure that everything to be sent has some sort of default value if the user hasn't set a value
			for (i = 0; i < phid->phid.attr.rfid.numOutputs; i++)
			{
				if (phid->outputState[i] == PUNK_BOOL)
					phid->outputState[i] = PFALSE;
			}
			if(phid->antennaState == PUNK_BOOL)
				phid->antennaState = PFALSE;
			if(phid->ledState == PUNK_BOOL)
				phid->ledState = PFALSE;

			//construct the packet
			for (i = 0, j = 1; i < phid->phid.attr.rfid.numOutputs; i++, j<<=1)
			{
				if (phid->outputState[i])
					buffer[0] |= j;
			}
			if(phid->ledState == PTRUE)
				buffer[0] |= RFID_LED_FLAG;
			if(phid->antennaState == PTRUE)
				buffer[0] |= RFID_ANTENNA_FLAG;
			break;
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
			//have to make sure that everything to be sent has some sort of default value if the user hasn't set a value
			for (i = 0; i < phid->phid.attr.rfid.numOutputs; i++)
			{
				if (phid->outputState[i] == PUNK_BOOL)
					phid->outputState[i] = PFALSE;
			}
			if(phid->antennaState == PUNK_BOOL)
				phid->antennaState = PFALSE;
			if(phid->ledState == PUNK_BOOL)
				phid->ledState = PFALSE;
			
			// Default write timing to T5577
			if(phid->pregapClocks == PUNK_INT)
				phid->pregapClocks = RFID_T5577_StartGap;
			if(phid->prepulseClocks == PUNK_INT)
				phid->prepulseClocks = RFID_T5577_PrePulse;
			if(phid->zeroClocks == PUNK_INT)
				phid->zeroClocks = RFID_T5577_Zero;
			if(phid->oneClocks == PUNK_INT)
				phid->oneClocks = RFID_T5577_One;
			if(phid->spaceClocks == PUNK_INT)
				phid->spaceClocks = RFID_T5577_WriteGap;
			if(phid->postgapClocks == PUNK_INT)
				phid->postgapClocks = RFID_T5577_EndGap;
			if(phid->eofpulseClocks == PUNK_INT)
				phid->eofpulseClocks = RFID_T5577_EOF;
			if(phid->listenDuringEOF == PUNK_BOOL)
				phid->listenDuringEOF = PFALSE;

			//construct the packet
			for (i = 0, j = 1; i < phid->phid.attr.rfid.numOutputs; i++, j<<=1)
			{
				if (phid->outputState[i])
					buffer[0] |= j;
			}
			if(phid->ledState == PTRUE)
				buffer[0] |= RFID_LED_FLAG;
			if(phid->antennaState == PTRUE)
				buffer[0] |= RFID_ANTENNA_FLAG;

			if(phid->antennaState == PTRUE)
				phid->_4097Conf = RFID_4097_DefaultON;
			else
				phid->_4097Conf = RFID_4097_PowerDown;
			
			buffer[0] |= RFID_CONTROL_OUT_PACKET;
			buffer[1] = (phid->pregapClocks-1) & 0x3F;
			buffer[1] |= ((phid->postgapClocks-1) << 2) & 0xC0;
			buffer[2] = (phid->postgapClocks-1) & 0x0F;
			buffer[2] |= ((phid->spaceClocks-1) << 2) & 0xF0;
			buffer[3] = (phid->spaceClocks-1) & 0x03;
			buffer[3] |= ((phid->zeroClocks-1) << 1) & 0xFC;
			buffer[4] = (phid->zeroClocks-1) & 0x01;
			buffer[4] |= ((phid->oneClocks-1) << 1) & 0xFE;
			buffer[5] = phid->prepulseClocks;
			buffer[6] = phid->eofpulseClocks;
			buffer[7] = phid->_4097Conf;
			break;
		default:
			return EPHIDGET_UNEXPECTED;
	}
	return EPHIDGET_OK;
}

//if the time since last tag read > 200ms, fire tagLost event
//NOTE: blocking in data events for too long will cause tagLost events
CThread_func_return_t tagTimerThreadFunction(CThread_func_arg_t userPtr)
{
	CPhidgetRFIDHandle phid = (CPhidgetRFIDHandle)userPtr;

	if(!phid) return (CThread_func_return_t)EPHIDGET_INVALIDARG;

	LOG(PHIDGET_LOG_INFO,"tagTimerThread running");

	while (CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_ATTACHED_FLAG) && phid->tagTimerThread.thread_status == PTRUE)
	{
		//sleeps for up to 50ms, but can be signalled externally to return immediately
		CThread_wait_on_event(&phid->tagAvailableEvent, 50);
		CThread_reset_event(&phid->tagAvailableEvent);

		//Tag events
		CThread_mutex_lock(&phid->tagthreadlock);
		if(phid->tagEventPending)
		{
			CThread_mutex_unlock(&phid->tagthreadlock);
			if(phid->pendingTag.protocol == PHIDGET_RFID_PROTOCOL_EM4100)
				FIRE(Tag, phid->pendingTag.tagData);
			FIRE(Tag2, phid->pendingTag.tagString, phid->pendingTag.protocol);

			//Fill in lastTag after the event so that LAST tag (not current tag) is avialable in the event
			CThread_mutex_lock(&phid->tagthreadlock);
			phid->lastTag = phid->pendingTag;
			phid->lastTagValid = PTRUE;
			phid->tagEventPending = PFALSE;
		}
		CThread_mutex_unlock(&phid->tagthreadlock);

		//TAG Lost events
		CThread_mutex_lock(&phid->tagthreadlock);
		if(phid->tagPresent != PFALSE)
		{
			/* check for tag lost */
#ifdef RFID_HITAGS_SUPPORT
			if((timeSince(&phid->lastTagTime) > 0.2 && phid->lastTag.protocol != PHIDGET_RFID_PROTOCOL_HITAGS_UID)
				|| (timeSince(&phid->lastTagTime) > 0.5 && phid->lastTag.protocol == PHIDGET_RFID_PROTOCOL_HITAGS_UID))
#else
			if(timeSince(&phid->lastTagTime) > 0.2)
#endif
			{
				if (phid->tagPresent == PTRUE) {
					phid->tagPresent = PFALSE;
					CThread_mutex_unlock(&phid->tagthreadlock);

					if(phid->pendingTag.protocol == PHIDGET_RFID_PROTOCOL_EM4100)
						FIRE(TagLost, phid->lastTag.tagData);
					FIRE(TagLost2, phid->lastTag.tagString, phid->lastTag.protocol);

					CThread_mutex_lock(&phid->tagthreadlock);
				}
				else if(phid->antennaEchoState == PTRUE) //could be PUNK_BOOL - don't send event, just set to PFALSE (but only if the antenna is on)
					phid->tagPresent = PFALSE;
			}
		}
		CThread_mutex_unlock(&phid->tagthreadlock);

#ifdef RFID_HITAGS_SUPPORT
		//Actively look for tags if we haven't gotten data for a while (Hitag)
		CThread_mutex_lock(&phid->tagthreadlock);
		if(phid->antennaEchoState == PTRUE && phid->polling == PTRUE)
		{
			switch(phid->phid.deviceUID)
			{
				case PHIDUID_RFID_2OUTPUT_READ_WRITE:
					if(timeSince(&phid->hitagReqTime) > 0.1 && timeSince(&phid->lastDataTime) > 0.1) //100ms
					{
						HitagS_UID_REQUEST(phid);
					}
					break;

				//Cannot send data
				default:
					break;
			}
		}
		CThread_mutex_unlock(&phid->tagthreadlock);
#endif
	}

	if(fptrJavaDetachCurrentThread)
		fptrJavaDetachCurrentThread();
	LOG(PHIDGET_LOG_INFO,"tagTimerThread exiting normally");
	phid->tagTimerThread.thread_status = FALSE;
	return (CThread_func_return_t)EPHIDGET_OK;
}

static int tagEvent(CPhidgetRFIDHandle phid, CPhidgetRFID_TagHandle tagPtr)
{
again:
	CThread_mutex_lock(&phid->tagthreadlock);

	//update time
	setTimeNow(&phid->lastTagTime);

	//See if there is a current tag, and if it matches this tag
	if(phid->tagPresent != PTRUE || 
		(phid->lastTagValid && strcmp(phid->lastTag.tagString, tagPtr->tagString) && phid->lastTag.protocol == tagPtr->protocol))
	{
		//Wait for tagEventPending to be false if it's true
		if(phid->tagEventPending == PTRUE)
		{
			CThread_mutex_unlock(&phid->tagthreadlock);
			SLEEP(10);
			goto again;
		}

		phid->pendingTag = *tagPtr;
		phid->tagEventPending = PTRUE;
		CThread_set_event(&phid->tagAvailableEvent);
	}

	phid->tagPresent = PTRUE;

	CThread_mutex_unlock(&phid->tagthreadlock);

	return EPHIDGET_OK;
}

static int tagEvent_fromEM4100Data(CPhidgetRFIDHandle phid, unsigned char *data)
{	
	CPhidgetRFID_Tag tag;
	ZEROMEM(&tag, sizeof(CPhidgetRFID_Tag));
	snprintf(tag.tagString, RFID_MAX_TAG_STRING_LEN, "%02x%02x%02x%02x%02x",data[0],data[1],data[2],data[3],data[4]);
	tag.protocol = PHIDGET_RFID_PROTOCOL_EM4100;
	memcpy(tag.tagData, data, 5);
	return tagEvent(phid, &tag);
}

static int waitForTag(CPhidgetRFIDHandle phid, char *tagString, int timeout)
{
	//Wait for this tag to show up
	while(timeout > 0)
	{
		CThread_mutex_lock(&phid->tagthreadlock);
		if(!strncmp(phid->pendingTag.tagString, tagString, RFID_MAX_TAG_STRING_LEN))
		{
			CThread_mutex_unlock(&phid->tagthreadlock);
			return EPHIDGET_OK;
		}
		CThread_mutex_unlock(&phid->tagthreadlock);
		SLEEP(50);
		timeout-=50;
	}

	return EPHIDGET_TIMEOUT;
}

static int sendRAWData(CPhidgetRFIDHandle phid, unsigned char *data, int bitlength)
{
	unsigned char buffer[10];
	int i, j, result, bits;

	int length = bitlength/8 + (bitlength%8 ? 1 : 0);

	if(length > 0xff)
		return EPHIDGET_INVALIDARG;

	bits = 0;
    for (i = 0, j = 1; i < length; i++, j++)
    {
        buffer[j] = data[i];
		if(bitlength < 8)
		{
			bits += bitlength;
			bitlength = 0;
		}
		else
		{
			bits += 8;
			bitlength -= 8;
		}
        if (j == 7 || i == (length-1))
        {
			buffer[0] = RFID_WRITE_DATA_OUT_PACKET | bits;
			if ((result = CPhidgetRFID_sendpacket(phid, buffer)) != EPHIDGET_OK)
			{
				return result;
			}
			j = 0;
			bits = 0;
        }
    }

	return EPHIDGET_OK;
}

static void resetValuesFromSpace(CPhidgetRFIDHandle phid)
{
	phid->shortClocks = phid->longClocks = 0;
	phid->manLockedIn = 0;
	phid->manReadPtr = 0;
	phid->manWritePtr = 0;
	phid->biphaseReadPtr = 0;
	phid->biphaseWritePtr = 0;
	phid->manShortChange = 0;
	phid->biphaseShortChange = 0;
	phid->biphaseLockedIn = 0;

#ifdef RFID_RAWDATA_API_SUPPORT
	phid->manEventReadPtr = 0;

	//Manchester event with space
	if(phid->lastManEventLong == PFALSE)
	{
		unsigned char manEventData[1];
		phid->lastManEventLong = PTRUE;
		manEventData[0] = PUNK_BOOL;
		FIRE(ManchesterData, manEventData, 1);
	}
#endif
}

//ISO11785 CRC
static void CRC_16_CCITT_update(unsigned short *crc, unsigned char x)
{
	unsigned short crc_new = (unsigned char)((*crc) >> 8) | ((*crc) << 8);
	crc_new ^= x;
	crc_new ^= (unsigned char)(crc_new & 0xff) >> 4;
	crc_new ^= crc_new << 12;
	crc_new ^= (crc_new & 0xff) << 5;
	(*crc) = crc_new;
}

//Reverse all bits
static __uint64
reverse(__uint64 x)
{
	x = (((x & 0xaaaaaaaaaaaaaaaaLL) >> 1) | ((x & 0x5555555555555555LL) << 1));
	x = (((x & 0xccccccccccccccccLL) >> 2) | ((x & 0x3333333333333333LL) << 2));
	x = (((x & 0xf0f0f0f0f0f0f0f0LL) >> 4) | ((x & 0x0f0f0f0f0f0f0f0fLL) << 4));
	x = (((x & 0xff00ff00ff00ff00LL) >> 8) | ((x & 0x00ff00ff00ff00ffLL) << 8));
	x = (((x & 0xffff0000ffff0000LL) >> 16) | ((x & 0x0000ffff0000ffffLL) << 16));
	return((x >> 32) | (x << 32));
}

/* Takes the tagString in 10-digit hex, and produces data for programming.
 *  blockData are 32-bit blocks.
 *  blockDataLen will be 2 when we return, as EM4100 takes 64-bits.
 */
static int encodeEM4100(char *tagString, unsigned int *blockData, int *blockDataLen)
{
	__int64 tagData, mask;
	__uint64 encodedTagData = 0;
	int i,j,row,col;
	TESTPTRS(tagString, blockData)
	TESTPTR(blockDataLen)
	if(*blockDataLen < 2)
		return EPHIDGET_INVALIDARG;
	if(strlen(tagString) != 10 )
		if(!(strlen(tagString) == 12 && tagString[0] == '0' && tagString[1] == 'x'))
			return EPHIDGET_INVALIDARG;

	tagData = strtoll(tagString, NULL, 16);

	//9 leading 1's
	encodedTagData = 0xFF80000000000000LL;
	//Data
	for(i=0, mask=0x0078000000000000LL, j=15; i<10; i++, j--, mask>>=5)
	{
		encodedTagData |= ((tagData << j) & mask);
	}
	//Parity
	for(i=0, row=1, col=1; i<40; i++, col++)
	{
		if(col>4) { col=1; row++; }
		
		//column parity
		encodedTagData ^= ((tagData & 0x1) << col);
		//row parity
		encodedTagData ^= ((tagData & 0x1) << (row*5));
		
		tagData>>=1;
	}

	blockData[0] = (int)(encodedTagData >> 32);
	blockData[1] = (int)encodedTagData;
	*blockDataLen = 2;

	return EPHIDGET_OK;
}

/* Takes the tagString in 15-digit decimal, and produces data for programming.
 *  blockData are 32-bit blocks.
 *  blockDataLen will be 4 when we return, as FDX-B takes 128-bits.
 */
static int encodeISO11785_FDX_B(char *tagString, unsigned int *blockData, int *blockDataLen)
{
	__uint64 tagData, encodedTagData[2] = {0,0}, mask, uniqueID;
	unsigned short crc = 0x0000;
	unsigned long countryCode;
	char countryCodeStr[4];
	int i,j;
	TESTPTRS(tagString, blockData)
	TESTPTR(blockDataLen)
	if(*blockDataLen < 4)
		return EPHIDGET_INVALIDARG;
	if(strlen(tagString) != 15 )
		return EPHIDGET_INVALIDARG;

	//Get uniqueID
	uniqueID = (__uint64)strtoll(tagString+3, NULL, 10);
	//must be 38-bit or less
	if(uniqueID > 0x3FFFFFFFFFLL)
		return EPHIDGET_INVALIDARG;

	//Get Country Code
	memcpy(countryCodeStr, tagString, 3);
	countryCodeStr[3] = '\0';
	countryCode = strtoul(countryCodeStr, NULL, 10);

	//Create ISO11784 64-bit data
	tagData = (((__uint64)countryCode) << 38) | uniqueID;
	//Add the animal bit
	tagData |= 0x8000000000000000LL;
	//Reverse because order is LSB 1st.
	tagData = reverse(tagData);
	//Calculate CRC
	for(i=0,j=7*8;i<8;i++,j-=8)
		CRC_16_CCITT_update(&crc, (unsigned char)((tagData>>j) & 0xFF));

	//Put it into the FDX-B Format

	//Header and control bits
	encodedTagData[0] = 0x0020100804020100LL;
	encodedTagData[1] = 0x8040201008040201LL;
	//Data
	for(i=0, mask = 0x001FE00000000000LL, j=11; i<6; i++, mask >>= 9, j++)
		encodedTagData[0] |= ((tagData >> j) & mask);
	for(i=0, mask = 0x7F80000000000000LL, j=47; i<2; i++, mask >>= 9, j--)
		encodedTagData[1] |= ((tagData << j) & mask);
	//CRC
	encodedTagData[1] |= ((((__uint64)crc) << 29) & 0x00001FE000000000LL);
	encodedTagData[1] |= ((((__uint64)crc) << 28) & 0x0000000FF0000000LL);

	blockData[0] = (int)(encodedTagData[0] >> 32);
	blockData[1] = (int)encodedTagData[0];
	blockData[2] = (int)(encodedTagData[1] >> 32);
	blockData[3] = (int)encodedTagData[1];
	*blockDataLen = 4;

	return EPHIDGET_OK;
}

/* Takes the tagString in ASCII up to 24 characters, and produces data for programming.
 *  blockData are 32-bit blocks.
 *  blockDataLen will be 7 when we return, as PHIDGETS_TAG takes 224-bits.
 */
static int encodePHIDGETS_TAG(char *tagString, unsigned int *blockData, int *blockDataLen)
{
	char tagData[24];
	int len, i, j;
	unsigned short crc;
	TESTPTRS(tagString, blockData)
	TESTPTR(blockDataLen)
	if(*blockDataLen < 7)
		return EPHIDGET_INVALIDARG;
	if(strlen(tagString) > 24 )
		return EPHIDGET_INVALIDARG;

	//copy of string.
	len = strlen(tagString);
	ZEROMEM(tagData, 24);
	memcpy(tagData, tagString, len);

	//Calculate CRC
	crc=0;
	for(i=0;i<24;i++)
		CRC_16_CCITT_update(&crc, (unsigned char)tagData[i]);

	//Header, control bits and CRC
	blockData[0] = 0x00040201;
	blockData[0] |= (((unsigned int)crc) << 2 & 0x0003FC00);
	blockData[0] |= (((unsigned int)crc) << 1 & 0x000001FE);
	//Control bits
	for(i=1;i<7;i++)
		blockData[i] = 0x01010101;
	//Data
	for(i=0,j=184;i<24;i++,j-=8)
	{
		int block = i/4+1;
		int shift = (j%32)+1;
		blockData[block] |= (tagData[i] << shift);
	}

	*blockDataLen = 7;

	return EPHIDGET_OK;
}

static int decodeEM4100(CPhidgetRFIDHandle phid, unsigned char *data, int *startPtr, int *endPtr, CPhidgetRFID_TagHandle tag)
{
	int i, foundStart, k, j;
	int myReadPtr = *startPtr;
	int em4103data[64];
	unsigned char decodedData[5];
	int bytesInQueue;
	//Look for the starting pattern of 9 Ones
start:
	bytesInQueue = *endPtr - myReadPtr;
	if(myReadPtr > *endPtr)
		bytesInQueue += RFID_DATA_ARRAY_SIZE;

	while(myReadPtr != *endPtr)
	{
		if(bytesInQueue < 64)
			return EPHIDGET_NOTFOUND;
		foundStart = 1;

		for(i=0;i<9;i++)
		{
			if(data[(myReadPtr + i) & RFID_DATA_ARRAY_MASK] != 1)
			{
				foundStart = 0;
				break;
			}
		}
		if(foundStart)
			break;

		myReadPtr++;
		myReadPtr &= RFID_DATA_ARRAY_MASK;

		bytesInQueue--;
	}

	//Got here? - We found the start pattern
	//Now decode the EM4100 data
	for(i=0;i<64;i++)
	{
		em4103data[i] = data[(myReadPtr + i) & RFID_DATA_ARRAY_MASK];
	}

	//last bit should be zero (stop bit)
	if(em4103data[63] != 0)
		goto tryagain;

	//row Parity
	for(i=0;i<10;i++)
	{
		int rowParity = 0;
		for(k=0;k<4;k++)
			rowParity ^= em4103data[9+i*5+k];
		if(rowParity != em4103data[9+i*5+k])
			goto tryagain;
	}
	//column parity
	for(i=0;i<4;i++)
	{
		int colParity = 0;
		for(k=0;k<10;k++)
			colParity ^= em4103data[9+i+k*5];
		if(colParity != em4103data[9+i+k*5])
			goto tryagain;
	}

	//We're good! Strip out data
	memset(decodedData, 0, 5);
	for(i=0,j=9;i<5;i++)
	{
		for(k=7;k>=4;k--,j++)
			decodedData[i] |= em4103data[j] << k;
		j++; //skip row parity bit
		for(k=3;k>=0;k--,j++)
			decodedData[i] |= em4103data[j] << k;
		j++; //skip row parity bit
	}
	

	//Update the tag struct for the tag2 event
	snprintf(tag->tagString, RFID_MAX_TAG_STRING_LEN, "%02x%02x%02x%02x%02x",decodedData[0],decodedData[1],decodedData[2],decodedData[3],decodedData[4]);
	tag->protocol = PHIDGET_RFID_PROTOCOL_EM4100;
	memcpy(tag->tagData, decodedData, 5);

	//update master read pointer
	(*startPtr)+=64;
	(*startPtr) &= RFID_DATA_ARRAY_MASK;
	return EPHIDGET_OK;

tryagain:
	myReadPtr++;
	myReadPtr &= RFID_DATA_ARRAY_MASK;
	goto start;
}

static int decodeISO11785_FDX_B(CPhidgetRFIDHandle phid, unsigned char *data, int *startPtr, int *endPtr, CPhidgetRFID_TagHandle tag)
{
	int i, foundStart, k;
	int myReadPtr = *startPtr;
	unsigned char iso11785data[8];
	unsigned char iso11785dataReversed[8];
	unsigned short iso11785checksum;
	int bytesInQueue;
	unsigned short crc = 0x0000;
	//Look for the starting pattern of 10 zeroes and 1 one
start:
	bytesInQueue = *endPtr - myReadPtr;
	if(myReadPtr > *endPtr)
		bytesInQueue += RFID_DATA_ARRAY_SIZE;

	while(myReadPtr != *endPtr)
	{
		//full sequence is 128 bits
		if(bytesInQueue < 128)
			return EPHIDGET_NOTFOUND;
		foundStart = 1;

		for(i=0;i<10;i++)
		{
			if(data[(myReadPtr + i) & RFID_DATA_ARRAY_MASK] != 0)
			{
				foundStart = 0;
				break;
			}
		}
		if(data[(myReadPtr + 10) & RFID_DATA_ARRAY_MASK] != 1)
		{
			foundStart = 0;
		}
		if(foundStart)
			break;

		myReadPtr++;
		myReadPtr &= RFID_DATA_ARRAY_MASK;

		bytesInQueue--;
	}

	//advance past header
	myReadPtr += 11;
	myReadPtr &= RFID_DATA_ARRAY_MASK;

	//Got here? - We found the start pattern
	//Now decode the ISO11785 data
	//every block of 8 is followed by a '1'
	memset(iso11785data, 0, 8);
	memset(iso11785dataReversed, 0, 8);
	for(i=0,k=0;i<64;i++,k++)
	{
		if(i>0 && i%8 == 0)
		{
			if(data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] != 1) goto tryagain;
			k++;
		}
		iso11785data[i/8] |= data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] << (7-(i%8));
		iso11785dataReversed[7-(i/8)] |= data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] << (i%8);
	}
	if(data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] != 1) goto tryagain;
	k++;

	//Now the checksum
	iso11785checksum = 0;
	for(i=0;i<16;i++,k++)
	{
		if(i>0 && i%8 == 0)
		{
			if(data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] != 1) goto tryagain;
			k++;
		}
		iso11785checksum |= data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] << (15-i);
	}	
	if(data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] != 1) goto tryagain;
	k++;

	//TODO: there is also a 24 bit trailer which can contain extra info

	//Checksum
	crc = 0x0000;
	for(i=0;i<8;i++)
	{
		CRC_16_CCITT_update(&crc, iso11785data[i]);
	}

	if(crc != iso11785checksum)
	{
		//FAIL
		goto tryagain;
	}

	//We're good!
	//Parse out the different sections
	{
		//These are not used for now
		//unsigned char animal = iso11785dataReversed[0] & 0x80 ? PTRUE : PFALSE; //1 bit - bit 0
		//unsigned char extraData = iso11785dataReversed[1] & 0x01 ? PTRUE : PFALSE; //1 bit - bit 15
		unsigned short countryCode = (iso11785dataReversed[2] << 2 | iso11785dataReversed[3] >> 6) & 0x3ff; //10 bit - bits 16-26
		unsigned long long UID = (
			(((__uint64)iso11785dataReversed[3]) << 32) +
			(((__uint64)iso11785dataReversed[4]) << 24) +
			(((__uint64)iso11785dataReversed[5]) << 16) +
			(((__uint64)iso11785dataReversed[6]) << 8) +
			((__uint64)iso11785dataReversed[7])) & 0x3FFFFFFFFFll;// 38 bit - bits 27-63

		snprintf(tag->tagString, RFID_MAX_TAG_STRING_LEN, "%03d%012lld",countryCode, UID);
		tag->protocol = PHIDGET_RFID_PROTOCOL_ISO11785_FDX_B;
	}

	//update master read pointer
	(*startPtr) += 128;
	(*startPtr) &= RFID_DATA_ARRAY_MASK;
	return EPHIDGET_OK;

tryagain:
	myReadPtr++;
	myReadPtr &= RFID_DATA_ARRAY_MASK;
	goto start;
}

static int decodePHIDGETS_TAG(CPhidgetRFIDHandle phid, unsigned char *data, int *startPtr, int *endPtr, CPhidgetRFID_TagHandle tag)
{
	int i, foundStart, k;
	int myReadPtr = *startPtr;
	char tagData[24];
	int bytesInQueue;
	unsigned short crcCalc = 0x0000, crcRead = 0x0000;
	//Look for the starting pattern of 13 zeroes and 1 one
start:
	bytesInQueue = *endPtr - myReadPtr;
	if(myReadPtr > *endPtr)
		bytesInQueue += RFID_DATA_ARRAY_SIZE;

	while(myReadPtr != *endPtr)
	{
		//full sequence is 224 bits
		if(bytesInQueue < 224)
			return EPHIDGET_NOTFOUND;
		foundStart = 1;

		for(i=0;i<13;i++)
		{
			if(data[(myReadPtr + i) & RFID_DATA_ARRAY_MASK] != 0)
			{
				foundStart = 0;
				break;
			}
		}
		if(data[(myReadPtr + 13) & RFID_DATA_ARRAY_MASK] != 1)
		{
			foundStart = 0;
		}
		if(foundStart)
			break;

		myReadPtr++;
		myReadPtr &= RFID_DATA_ARRAY_MASK;

		bytesInQueue--;
	}

	//advance past header
	myReadPtr += 14;
	myReadPtr &= RFID_DATA_ARRAY_MASK;

	//Got here? - We found the start pattern

	//Pull out the CRC
	for(k=0,i=0;i<16;i++,k++)
	{
		if(i>0 && i%8 == 0)
		{
			if(data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] != 1) goto tryagain;
			k++;
		}
		crcRead |= data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] << (15-i);
	}	
	if(data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] != 1) goto tryagain;
	k++;

	//Now decode the PHIDGETS_TAG data
	//every block of 7 is followed by a '1'
	memset(tagData, 0, 24);
	for(i=0;i<192;i++,k++)
	{
		//Check for control bit
		if((i+1)%8 == 0)
		{
			if(data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] != 1)
				goto tryagain;
		}
		//pull out data
		else
		{
			tagData[i/8] |= data[(myReadPtr + k) & RFID_DATA_ARRAY_MASK] << (6-(i%8));
		}
	}

	//Calculate CRC
	crcCalc = 0;
	for(i=0;i<24;i++)
		CRC_16_CCITT_update(&crcCalc, (unsigned char)tagData[i]);

	if(crcCalc != crcRead)
	{
		//FAIL
		goto tryagain;
	}

	//We're good!
	memcpy(tag->tagString, tagData, 24);
	tag->tagString[24] = '\0';
	tag->protocol = PHIDGET_RFID_PROTOCOL_PHIDGETS;

	//update master read pointer
	(*startPtr) += 224;
	(*startPtr) &= RFID_DATA_ARRAY_MASK;
	return EPHIDGET_OK;

tryagain:
	myReadPtr++;
	myReadPtr &= RFID_DATA_ARRAY_MASK;
	goto start;
}

static int add_biphase_data(CPhidgetRFIDHandle phid, int readToPtr)
{
	int myReadPtr = phid->dataReadPtr;
	while(myReadPtr != readToPtr)
	{
		int clocks = phid->dataBuffer[myReadPtr] & 0xFF;

		if (clocks == phid->longClocks) {

			phid->biphaseBuffer[phid->biphaseWritePtr] = 1;

			phid->biphaseWritePtr++;
			phid->biphaseWritePtr &= RFID_DATA_ARRAY_MASK;

			if(phid->biphaseWritePtr == phid->biphaseReadPtr)
			{
				phid->biphaseReadPtr++;
				phid->biphaseReadPtr &= RFID_DATA_ARRAY_MASK;
			}
			
			phid->biphaseLockedIn = 1;
			phid->biphaseShortChange = 0;
		} 
		else if (clocks == phid->shortClocks) {
			if (phid->biphaseLockedIn && phid->biphaseShortChange) {
				phid->biphaseBuffer[phid->biphaseWritePtr] = 0;

				phid->biphaseWritePtr++;
				phid->biphaseWritePtr &= RFID_DATA_ARRAY_MASK;

				if(phid->biphaseWritePtr == phid->biphaseReadPtr)
				{
					phid->biphaseReadPtr++;
					phid->biphaseReadPtr &= RFID_DATA_ARRAY_MASK;
				}

				phid->biphaseShortChange=0;
			}
			else	
				phid->biphaseShortChange=1;
		}
		else {
			phid->biphaseLockedIn = 0;
			//invalid
			phid->biphaseReadPtr = phid->biphaseWritePtr;
			//This is not BiPhase encoded data
			return EPHIDGET_NOTFOUND;
		}

		myReadPtr++;
		myReadPtr &= RFID_DATA_ARRAY_MASK;
	}
	return EPHIDGET_OK;
}

static int add_manchester_data(CPhidgetRFIDHandle phid, int readToPtr)
{
	int myReadPtr = phid->dataReadPtr;
	while(myReadPtr != readToPtr)
	{
		int clocks = phid->dataBuffer[myReadPtr] & 0xFF;
		int polarity = (phid->dataBuffer[myReadPtr] & 0x100) ? 1 : 0;

		if (clocks == phid->longClocks)
		{
			//We're out of sync - re-sync
			if(phid->manShortChange)
			{
				phid->manReadPtr = phid->manWritePtr;
				phid->manReadPtr &= RFID_DATA_ARRAY_MASK;
#ifdef RFID_RAWDATA_API_SUPPORT
				phid->manEventReadPtr = phid->manWritePtr;
				phid->manEventReadPtr &= RFID_DATA_ARRAY_MASK;
#endif
			}

			if (polarity)
				phid->manBuffer[phid->manWritePtr] = 1;
			else
				phid->manBuffer[phid->manWritePtr] = 0;

			phid->manWritePtr++;
			phid->manWritePtr &= RFID_DATA_ARRAY_MASK;

			//TODO: is there a danger of these actually happening??
			if(phid->manWritePtr == phid->manReadPtr)
			{
				phid->manReadPtr++;
				phid->manReadPtr &= RFID_DATA_ARRAY_MASK;
			}

#ifdef RFID_RAWDATA_API_SUPPORT
			if(phid->manWritePtr == phid->manEventReadPtr)
			{
				phid->manEventReadPtr++;
				phid->manEventReadPtr &= RFID_DATA_ARRAY_MASK;
			}
#endif
			
			phid->manLockedIn = 1;
			phid->manShortChange = 0;
		}
		// Short clocks
		else
		{
			if(!phid->manLockedIn)
			{
				if(polarity)
				{
					phid->manShortChange=1;
					phid->manLockedIn = 1;
				}
			}
			else
			{
				if(phid->manShortChange)
				{
					if (polarity)
						phid->manBuffer[phid->manWritePtr] = 1;
					else
						phid->manBuffer[phid->manWritePtr] = 0;

					phid->manWritePtr++;
					phid->manWritePtr &= RFID_DATA_ARRAY_MASK;

					if(phid->manWritePtr == phid->manReadPtr)
					{
						phid->manReadPtr++;
						phid->manReadPtr &= RFID_DATA_ARRAY_MASK;
					}

#ifdef RFID_RAWDATA_API_SUPPORT
					if(phid->manWritePtr == phid->manEventReadPtr)
					{
						phid->manEventReadPtr++;
						phid->manEventReadPtr &= RFID_DATA_ARRAY_MASK;
					}
#endif

					phid->manShortChange=0;
				}
				else
					phid->manShortChange=1;
			}
		}

		myReadPtr++;
		myReadPtr &= RFID_DATA_ARRAY_MASK;
	}
	return EPHIDGET_OK;
}

//Analyses streaming data in Manchester or Biphase coding
static int analyze_data(CPhidgetRFIDHandle phid)
{
	int bytesToRead = 0, bytesRead = 0;
	int temp, clocks;
	int myReadPtr;
	CPhidgetRFID_Tag tag;

	//read till we have real data
start:
	while(phid->dataReadPtr != phid->dataWritePtr)
	{
		if(phid->dataBuffer[phid->dataReadPtr] == PUNK_INT)
		{
			//LOG(PHIDGET_LOG_DEBUG, "LONG");
			phid->dataReadPtr++;
			phid->dataReadPtr &= RFID_DATA_ARRAY_MASK;
			resetValuesFromSpace(phid);
		}
		else
			break;
	}

	//Make sure we have enough data to do something useful with..
	bytesToRead = phid->dataWritePtr - phid->dataReadPtr;
	if(phid->dataReadPtr > phid->dataWritePtr)
		bytesToRead += RFID_DATA_ARRAY_SIZE;

	//then read till we have a space or run out of data - figure out data rate
	bytesRead = 0;
	myReadPtr = phid->dataReadPtr;
	while(myReadPtr != phid->dataWritePtr)
	{
		if(phid->dataBuffer[myReadPtr] == PUNK_INT)
			break;

		clocks = (phid->dataBuffer[myReadPtr] & 0xFF);

		if(!phid->shortClocks)
		{
			phid->shortClocks = clocks;
		}
		else if(clocks != phid->shortClocks)
		{
			if(!phid->longClocks)
			{
				if(phid->shortClocks * 2 == clocks || phid->shortClocks / 2 == clocks)
					phid->longClocks = clocks;
				else
					//found a values that doesn't work - error
					goto update_readPtr_restart;
			}
			else if(clocks != phid->longClocks)
				//found a values that doesn't work - error
				goto update_readPtr_restart;
		}

		myReadPtr++;
		myReadPtr &= RFID_DATA_ARRAY_MASK;
		bytesRead++;
	}

	//We haven't seen a one and a two!!
	if(phid->shortClocks == 0 || phid->longClocks == 0)
	{
		//got a gap? move on to more data
		if(bytesRead != bytesToRead)
			goto check_done;

		//otherwise, wait for more data
		return EPHIDGET_OK;
	}

	//Order them by size
	if(phid->longClocks < phid->shortClocks)
	{
		temp = phid->longClocks;
		phid->longClocks = phid->shortClocks;
		phid->shortClocks = temp;
	}

	ZEROMEM(&tag, sizeof(CPhidgetRFID_Tag));

	//Shift data into Manchester and Biphase decoders, update read ptr
	if(!add_manchester_data(phid, myReadPtr))
	{
#ifdef RFID_RAWDATA_API_SUPPORT
		unsigned char manEventData[RFID_DATA_ARRAY_SIZE];
		int manEventDataWritePtr = 0;
#endif

		if(!decodeEM4100(phid, phid->manBuffer, &phid->manReadPtr, &phid->manWritePtr, &tag))
			tagEvent(phid, &tag);

#ifdef RFID_RAWDATA_API_SUPPORT
		//Manchester data event
		while(phid->manEventReadPtr != phid->manWritePtr)
		{
			manEventData[manEventDataWritePtr++] = phid->manBuffer[phid->manEventReadPtr];
			phid->manEventReadPtr++;
			phid->manEventReadPtr&=RFID_DATA_ARRAY_MASK;
		}
		if(manEventDataWritePtr)
		{
			phid->lastManEventLong = PFALSE;
			FIRE(ManchesterData, manEventData, manEventDataWritePtr);
		}
	}
	//Manchester event with space
	else if(phid->lastManEventLong == PFALSE)
	{
		unsigned char manEventData[1];
		phid->lastManEventLong = PTRUE;
		manEventData[0] = PUNK_BOOL;
		FIRE(ManchesterData, manEventData, 1);
#endif
	}

	if(!add_biphase_data(phid, myReadPtr))
	{
		if(!decodePHIDGETS_TAG(phid, phid->biphaseBuffer, &phid->biphaseReadPtr, &phid->biphaseWritePtr, &tag))
			tagEvent(phid, &tag);
		if(!decodeISO11785_FDX_B(phid, phid->biphaseBuffer, &phid->biphaseReadPtr, &phid->biphaseWritePtr, &tag))
			tagEvent(phid, &tag);
	}

check_done:
	//update read pointer
	phid->dataReadPtr = myReadPtr;

	//If there is more data, loop around
	if(phid->dataReadPtr != phid->dataWritePtr)
		goto start;

	return EPHIDGET_OK;

	//ran into a bad pulse length or a gap - reset stuff
update_readPtr_restart:
	phid->dataReadPtr++;
	phid->dataReadPtr &= RFID_DATA_ARRAY_MASK;
	resetValuesFromSpace(phid);

	goto start;
}

static int T5577_WriteBlock(CPhidgetRFIDHandle phid, int page, int block, unsigned int data, int lockpage)
{
	int res = EPHIDGET_OK;
	unsigned char byteData[5];

    byteData[0] = ((0x02 | (page & 0x01)) << 6); //OpCode
    byteData[0] |= ((lockpage ? 0x01 : 0x00) << 5); //Lock
    byteData[0] |= ((data >> 27) & 0x1F); //Data 31:27
    byteData[1] = ((data >> 19) & 0xFF); //Data 26:19
    byteData[2] = ((data >> 11) & 0xFF); //Data 18:11
    byteData[3] = ((data >> 3) & 0xFF); //Data 10:3
    byteData[4] = ((data << 5) & 0xE0); //Data 2:0
    byteData[4] |= ((block & 0x07) << 2); //Block (0-7)

	res = CPhidgetRFID_writeRaw(phid, byteData, 38, 
		RFID_T5577_StartGap, 
		RFID_T5577_WriteGap, 
		RFID_T5577_EndGap, 
		RFID_T5577_Zero, 
		RFID_T5577_One, 
		RFID_T5577_PrePulse, 
		RFID_T5577_EOF, 
		PFALSE);

	return res;
}

static int T5577_Reset(CPhidgetRFIDHandle phid)
{
	int res = EPHIDGET_OK;
	unsigned char byteData[1];

    byteData[0] = 0x00; //OpCode

	res = CPhidgetRFID_writeRaw(phid, byteData, 2, 
		RFID_T5577_StartGap, 
		RFID_T5577_WriteGap, 
		RFID_T5577_EndGap, 
		RFID_T5577_Zero, 
		RFID_T5577_One, 
		RFID_T5577_PrePulse, 
		RFID_T5577_EOF, 
		PFALSE);

	return res;
}

/* Programs a T5577 using the specified protocol and data
 *  data: Tag string.
 *  lock: lock the T5577 so it cannot be reprogrammed.
 */
static int T5577_WriteTag(CPhidgetRFIDHandle phid, CPhidgetRFID_Protocol protocol, char *tagString, int lock)
{
	int ret = EPHIDGET_OK, i;
	unsigned int data[7];
	int dataLen = 7;
	unsigned int configBlock;

	switch(protocol)
	{
		case PHIDGET_RFID_PROTOCOL_EM4100:
			if((ret = encodeEM4100(tagString, data, &dataLen)) != EPHIDGET_OK)
				return ret;
			configBlock = 0x00148040; // RF/63, manchester, Maxblock = 2
			break;
		case PHIDGET_RFID_PROTOCOL_ISO11785_FDX_B:
			if((ret = encodeISO11785_FDX_B(tagString, data, &dataLen)) != EPHIDGET_OK)
				return ret;
			configBlock = 0x603F8080; // RF/32, differential bi-phase, Maxblock = 4
			break;
		case PHIDGET_RFID_PROTOCOL_PHIDGETS:
			if((ret = encodePHIDGETS_TAG(tagString, data, &dataLen)) != EPHIDGET_OK)
				return ret;
			configBlock = 0x603F80E0; // RF/32, differential bi-phase, Maxblock = 7
			break;
		default:
			return EPHIDGET_INVALIDARG;
	}

	//Write Data
	for(i=0;i<dataLen;i++)
	{
		if((ret = T5577_WriteBlock(phid, 0, i+1, data[i], lock)) != EPHIDGET_OK)
			return ret;
		SLEEP(50); //some time beetween writes!
	}
	//Write config
	if((ret = T5577_WriteBlock(phid, 0, 0, configBlock, lock)) != EPHIDGET_OK)
		return ret;
		
	SLEEP(50); //some time beetween writes!
	//Reset Chip
	if((ret = T5577_Reset(phid)) != EPHIDGET_OK)
		return ret;

	return ret;
}

#ifdef RFID_HITAGS_SUPPORT
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define pdiff(a, b) ( ABS((a) - (b)) / (double)( ((a) + (b)) / 2.0 ) )

static void resetHitagACBuffer(CPhidgetRFIDHandle phid)
{
	phid->dataReadACPtr = phid->dataWritePtr;
	phid->ACCodingOK = PTRUE;
}

//Hitag CRC
#define CRC_PRESET 0xFF
#define CRC_POLYNOM 0x1D
static void calc_crc_hitag(unsigned char * crc,
			  unsigned char data,
			  unsigned char Bitcount)
{ 
	*crc ^= data; //crc = crc (exor) data
	 do
	 {
		 if( *crc & 0x80 ) // if (MSB-CRC == 1)
		 {
			 *crc<<=1; // CRC = CRC Bit-shift left
			 *crc ^= CRC_POLYNOM; // CRC = CRC (exor) CRC_POLYNOM
		 }
		 else
			 *crc<<=1; // CRC = CRC Bit-shift left
	 } while(--Bitcount);
}
static unsigned char hitagCRC8(unsigned char *data, int dataBits)
{
	unsigned char crc;
	int i;
	int dataLength = dataBits / 8;
	if(dataBits%8 != 0)
		dataLength++;
	crc = CRC_PRESET; /* initialize crc algorithm */

	for(i=0; i<dataLength; i++)
	{
		calc_crc_hitag(&crc, data[i], ((dataBits > 8) ? 8 : dataBits));
		dataBits -= 8;
	}

	return crc;
}

static int HitagS_UID_REQUEST(CPhidgetRFIDHandle phid)
{
	int res;
	//Send a Hitag S UID Request Command
	unsigned char buf[] = { 0xC0 };

	//make sure it's been at least 50ms since last hitag request
	while(timeSince(&phid->hitagReqTime) < 0.05) //50ms
		SLEEP(10);

	resetHitagACBuffer(phid);
	res = CPhidgetRFID_writeRaw(phid, buf, 5, 7, 7, 7, 13, 21, 0, 64, PTRUE);

	//Don't send it again for at least 100ms
	setTimeNow(&phid->hitagReqTime);

	return res;
}

static int decodeHitagUID(CPhidgetRFIDHandle phid, unsigned char *data, int bytesInQueue, CPhidgetRFID_TagHandle tag, int *collisionPos)
{
	int i, k;
	int myReadPtr = 0;
	unsigned int HitagUID;
	int sofBits = 3;
	int expectedBytes = 0;

	expectedBytes = 35;

	*collisionPos = -1;

	//UID is 32 bits, plus SOF == '111'
	if(bytesInQueue != expectedBytes)
		return EPHIDGET_NOTFOUND;

	//verify SOF
	for(i=0;i<sofBits;i++)
	{
		if(data[(myReadPtr + i) & RFID_DATA_ARRAY_MASK] != 1)
			return EPHIDGET_NOTFOUND;
	}

	//advance past SOF
	myReadPtr += sofBits;
	myReadPtr &= RFID_DATA_ARRAY_MASK;

	HitagUID = 0;

	for(i=0,k=0;i<(expectedBytes-sofBits);k++,i++)
	{
		//check for a collision
		if(data[(myReadPtr + i) & RFID_DATA_ARRAY_MASK] == PUNK_BOOL)
		{
			*collisionPos = k;
			return EPHIDGET_NOTFOUND;
		}
		HitagUID |= data[(myReadPtr + i) & RFID_DATA_ARRAY_MASK] << (31-k);
	}

	//We're good!
	snprintf(tag->tagString, RFID_MAX_TAG_STRING_LEN, "%08x",HitagUID);
	tag->protocol = PHIDGET_RFID_PROTOCOL_HITAGS_UID;

	return EPHIDGET_OK;
}

//Anti-Collision coding for Hitag S
static int decodeACdata(CPhidgetRFIDHandle phid, unsigned char *acBuffer, int *acBufferSize, int readToPtr, int shortClocks, int longClocks)
{
	int myReadPtr = phid->dataReadACPtr;
	int clocks1 = 0;
	int polarity1 = 0;
	int clocks2 = 0;
	int clocks = 0;
	int polarity2 = 0;
	int i;
	int bytesToRead;
	int lastIndex = 0;
	int acWritePtr = 0;
	unsigned char acBitLocation = 0;

	lastIndex = 34;

	if(*acBufferSize < lastIndex+1)
		goto fail;

	//if the first pulse is low, we need to add a high pulse before it
	if(!(phid->dataBuffer[phid->dataReadACPtr] & 0x100))
	{
		myReadPtr--;
		myReadPtr &= RFID_DATA_ARRAY_MASK;
		phid->dataBuffer[myReadPtr] = (shortClocks/2) | 0x100;
	}
	
	bytesToRead = readToPtr - myReadPtr;
	if(readToPtr < myReadPtr)
		bytesToRead += RFID_DATA_ARRAY_SIZE;

	bytesToRead &= 0xFFFE; //make LSB == 0

	//if(bytesToRead > lastIndex * 2)
	//	goto fail;

	for(i=0;i<bytesToRead;i+=2)
	{
		clocks1 = phid->dataBuffer[myReadPtr] & 0xFF;
		polarity1 = (phid->dataBuffer[myReadPtr] & 0x100) ? 1 : 0;
		clocks2 = phid->dataBuffer[(myReadPtr+1) & RFID_DATA_ARRAY_MASK] & 0xFF;
		polarity2 = (phid->dataBuffer[(myReadPtr+1) & RFID_DATA_ARRAY_MASK] & 0x100) ? 1 : 0;

		clocks = clocks1 + clocks2;

		if(polarity1 != 1 || polarity2 != 0)
			goto fail;

		//the first pulse can be long
		if (clocks == shortClocks || acWritePtr == 0)
		{			
			if(acBitLocation == 1)
			{
				acBuffer[acWritePtr] = PTRUE;

				acWritePtr++;
			}
			acBitLocation ^= 1;
		}
		else if(clocks == longClocks)
		{
			if(acBitLocation == 1)
			{
				//Sometimes we get a too short ending pulse
				if(acWritePtr == lastIndex && myReadPtr == ((readToPtr-1) & RFID_DATA_ARRAY_MASK))
				{
					acBuffer[acWritePtr] = PTRUE;
					acWritePtr++;
				}
				else
					goto fail;
			}
			else
			{
				if(pdiff(clocks1, clocks2) > 0.5)
					acBuffer[acWritePtr] = PUNK_BOOL;
				else
					acBuffer[acWritePtr] = PFALSE;

				acWritePtr++;

				acBitLocation = 0;
			}
		}
		else
			goto fail;

		if(acWritePtr > lastIndex)
			goto fail;
		
		myReadPtr+=2;
		myReadPtr &= RFID_DATA_ARRAY_MASK;
	}

	//last low pulse won't be seen because we idle low
	if(acWritePtr == lastIndex && myReadPtr == ((readToPtr-1) & RFID_DATA_ARRAY_MASK))
	{
		clocks = phid->dataBuffer[myReadPtr] & 0xFF;
		polarity1 = (phid->dataBuffer[myReadPtr] & 0x100) ? 1 : 0;

		if(polarity1 != 1)
			goto fail;
		
		if(clocks == shortClocks/2)
		{
			if(acBitLocation==1)
			{
				acBuffer[acWritePtr] = PTRUE;
				acWritePtr++;
			}
			//there should still be more data
			else
				acBitLocation ^= 1;
		}
		else if(clocks == shortClocks)
		{
			if(acBitLocation==1)
				goto fail;

			acBuffer[acWritePtr] = PFALSE;
			acWritePtr++;
		}
		else if(clocks == shortClocks * 1.5)
		{
			if(acBitLocation==1)
				goto fail;

			acBuffer[acWritePtr] = PUNK_BOOL;
			acWritePtr++;
		}
		else 
			goto fail;
	}
	else 
		goto fail;

	*acBufferSize = acWritePtr;
	return EPHIDGET_OK;

fail:
	//printf("beep2\n");
	
	acBitLocation = 0;

	//This is not AC encoded data
	return EPHIDGET_NOTFOUND;
}

//Analyses data for Hitag AC coding
static int analyze_data_AC(CPhidgetRFIDHandle phid)
{
	int bytesToRead = 0;
	unsigned int myReadPtr;
	CPhidgetRFID_Tag tag;
	unsigned char acBuffer[64];
	int acBufferSize = 64;

	ZEROMEM(&tag, sizeof(CPhidgetRFID_Tag));

	//read till we have real data
	while(phid->dataReadACPtr != phid->dataWritePtr)
	{
		if(phid->dataBuffer[phid->dataReadACPtr] == PUNK_INT)
		{
			phid->dataReadACPtr++;
			phid->dataReadACPtr &= RFID_DATA_ARRAY_MASK;
		}
		else
			break;
	}
	myReadPtr = phid->dataReadACPtr;

	if(phid->dataReadACPtr == phid->dataWritePtr)
		return EPHIDGET_OK;

	//read till we find the next gap
	while(myReadPtr != phid->dataWritePtr)
	{
		if(phid->dataBuffer[myReadPtr] != PUNK_INT)
		{
			myReadPtr++;
			myReadPtr &= RFID_DATA_ARRAY_MASK;
		}
		else
			break;
	}

	if(myReadPtr == phid->dataWritePtr)
		return EPHIDGET_OK;

	//We should now have a set of data between two gaps

	//Make sure we have enough data to do something useful with..
	bytesToRead = myReadPtr - phid->dataReadACPtr;
	if(phid->dataReadACPtr > myReadPtr)
		bytesToRead += RFID_DATA_ARRAY_SIZE;

	if(!decodeACdata(phid, acBuffer, &acBufferSize, myReadPtr, 32, 64))
	{
		int collision = -1;

		phid->ACCodingOK = PFALSE;

		if(!decodeHitagUID(phid, acBuffer, acBufferSize, &tag, &collision))
		{
			tagEvent(phid, &tag);
		}
	}

	//update read pointer
	phid->dataReadACPtr = myReadPtr;

	return EPHIDGET_OK;
}
#endif

// === Exported Functions === //

//create and initialize a device structure
CCREATE_EXTRA(RFID, PHIDCLASS_RFID)
	CThread_mutex_init(&phid->tagthreadlock);
	CThread_create_event(&phid->tagAvailableEvent);
	phid->phid.fptrClose = CPhidgetRFID_close;
	phid->phid.fptrFree = CPhidgetRFID_free;
	return EPHIDGET_OK;
}

//event setup functions
CFHANDLE(RFID, OutputChange, int, int)
CFHANDLE(RFID, Tag2, char *tagString, CPhidgetRFID_Protocol protocol)
CFHANDLE(RFID, TagLost2, char *tagString, CPhidgetRFID_Protocol protocol)
#ifdef RFID_RAWDATA_API_SUPPORT
CFHANDLE(RFID, ManchesterData, unsigned char *data, int dataLength)
CFHANDLE(RFID, RawData, int *data, int dataLength)
#else
PHIDGET21_API int CCONV CPhidgetRFID_set_OnManchesterData_Handler(CPhidgetRFIDHandle phidA, int (CCONV *fptr)(CPhidgetRFIDHandle, void *userPtr, unsigned char *data, int dataLength), void *userPtr)
{return EPHIDGET_UNSUPPORTED;}
PHIDGET21_API int CCONV CPhidgetRFID_set_OnRawData_Handler(CPhidgetRFIDHandle phidA, int (CCONV *fptr)(CPhidgetRFIDHandle, void *userPtr, int *data, int dataLength), void *userPtr)
{return EPHIDGET_UNSUPPORTED;}
#endif

CGET(RFID,OutputCount,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

	MASGN(phid.attr.rfid.numOutputs)
}

CGETINDEX(RFID,OutputState,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED
	TESTINDEX(phid.attr.rfid.numOutputs)
	TESTMASGN(outputEchoState[Index], PUNK_BOOL)

	MASGN(outputEchoState[Index])
}
CSETINDEX(RFID,OutputState,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED
	TESTRANGE(PFALSE, PTRUE)
	TESTINDEX(phid.attr.rfid.numOutputs)

	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
		ADDNETWORKKEYINDEXED(Output, "%d", outputState);
	else
	{
		SENDPACKET(RFID, outputState[Index]);
		//echo back output state if the device doesn't
		//do it here because this is after the packet has been sent off - so blocking in this event will not delay the output
		if (!(phid->fullStateEcho))
		{
			if (phid->outputEchoState[Index] == PUNK_BOOL || phid->outputEchoState[Index] != newVal)
			{
				phid->outputEchoState[Index] = newVal;
				FIRE(OutputChange, Index, newVal);
			}
		}
	}

	return EPHIDGET_OK;
}

CGET(RFID,AntennaOn,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED
	TESTMASGN(antennaEchoState, PUNK_BOOL)

	MASGN(antennaEchoState)
}
CSET(RFID,AntennaOn,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_2OUTPUT_NO_ECHO:
		case PHIDUID_RFID_2OUTPUT:
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
			TESTRANGE(PFALSE, PTRUE)

			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
				ADDNETWORKKEY(AntennaOn, "%d", antennaState);
			else
			{
				SENDPACKET(RFID, antennaState);
				//echo back state if the device doesn't
				if (phid->fullStateEcho == PFALSE)
					phid->antennaEchoState = newVal;
			}
			return EPHIDGET_OK;

		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CGET(RFID,LEDOn,int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_2OUTPUT_NO_ECHO:
		case PHIDUID_RFID_2OUTPUT:
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
			TESTMASGN(ledEchoState, PUNK_BOOL)
			MASGN(ledEchoState)
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}
CSET(RFID,LEDOn,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_2OUTPUT_NO_ECHO:
		case PHIDUID_RFID_2OUTPUT:
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
			TESTRANGE(PFALSE, PTRUE)

			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
				ADDNETWORKKEY(LEDOn, "%d", ledState);
			else
			{
				SENDPACKET(RFID, ledState);
				//echo back state if the device doesn't
				if (!(phid->fullStateEcho))
					phid->ledEchoState = newVal;
			}
			return EPHIDGET_OK;
		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

CSET(RFID,PollingOn,int)
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

#ifdef RFID_HITAGS_SUPPORT
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
				return EPHIDGET_UNSUPPORTED; //TODO? maybe just don't enable over webservice.
			else
			{
				phid->polling = newVal;
				return EPHIDGET_OK;
			}
		default:
			return EPHIDGET_UNSUPPORTED;
	}
#else
	return EPHIDGET_UNSUPPORTED;
#endif
}

PHIDGET21_API int CCONV CPhidgetRFID_getLastTag2(CPhidgetRFIDHandle phid, char **tagString, CPhidgetRFID_Protocol *protocol)
{
	TESTPTRS(phid,tagString)
	TESTPTR(protocol)
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

	CThread_mutex_lock(&phid->tagthreadlock);
	if(phid->lastTagValid == PTRUE)
	{
		*tagString = phid->lastTag.tagString;
		*protocol = phid->lastTag.protocol;
		CThread_mutex_unlock(&phid->tagthreadlock);
		return EPHIDGET_OK;
	}
	CThread_mutex_unlock(&phid->tagthreadlock);
	return EPHIDGET_UNKNOWNVAL;
}

CGET(RFID, TagStatus, int)
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

	*pVal = phid->tagPresent;

	if(*pVal == PUNK_BOOL) 
		return EPHIDGET_UNKNOWNVAL;

	return EPHIDGET_OK;
}

PHIDGET21_API int CCONV CPhidgetRFID_writeRaw(CPhidgetRFIDHandle phid, unsigned char *data, int bitlength, 
											  int pregap, int space, int postgap, int zero, int one, int prepulse, int eof, int listenDuringEOF)
{
	int retval;
	TESTPTR(phid)
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
			//limit spaces to 64 clocks
			if(pregap < (2) || pregap > (64)) return EPHIDGET_INVALIDARG;
			if(space < (2) || space > (64)) return EPHIDGET_INVALIDARG;
			if(postgap < (2) || postgap > (64)) return EPHIDGET_INVALIDARG;
			//pulses can be up to 256 clocks
			if(zero < (4) || zero > (128)) return EPHIDGET_INVALIDARG;
			if(one < (4) || one > (128)) return EPHIDGET_INVALIDARG;
			if(prepulse < (0) || prepulse > (255)) return EPHIDGET_INVALIDARG;
			if(eof < (0) || eof > (255)) return EPHIDGET_INVALIDARG;
			if(listenDuringEOF < (PFALSE) || listenDuringEOF > (PTRUE)) return EPHIDGET_INVALIDARG;

			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
				return EPHIDGET_UNSUPPORTED;//TODO
			else
			{
				unsigned char *buffer;
				int ret = 0;
				if(!(buffer = malloc(phid->phid.outputReportByteLength))) return EPHIDGET_NOMEMORY;
				ZEROMEM(buffer, phid->phid.outputReportByteLength);
				CThread_mutex_lock(&phid->phid.writelock);

				phid->pregapClocks = pregap;
				phid->postgapClocks = postgap;
				phid->spaceClocks = space;
				phid->zeroClocks = zero;
				phid->oneClocks = one;
				phid->prepulseClocks = prepulse;
				phid->eofpulseClocks = eof;
				phid->listenDuringEOF = listenDuringEOF;

				//Send timing
				if((ret = CPhidgetRFID_makePacket(phid, buffer))) goto done2;
				if((ret = CPhidgetRFID_sendpacket(phid, buffer))) goto done2;

				//send data
				if((retval = sendRAWData(phid, data, bitlength))) goto done2;

			done2:
				CThread_mutex_unlock(&phid->phid.writelock);
				free(buffer);
				if(ret) return ret;
			}

			return EPHIDGET_OK;

		default:
			return EPHIDGET_UNSUPPORTED;
	}
}

PHIDGET21_API int CCONV CPhidgetRFID_getRawData(CPhidgetRFIDHandle phid, int *data, int *dataLength)
{
	TESTPTR(phid) 
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

#ifdef RFID_RAWDATA_API_SUPPORT
	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
		{
			int i;
			//make sure length is even so we only send out data with starting space and ending pulse
			if((*dataLength % 2) == 1)
				(*dataLength)--;

			for(i=0;i<*dataLength;i++)
			{
				if(phid->userReadPtr == phid->dataWritePtr)
					break;

				data[i] = phid->dataBuffer[phid->userReadPtr];
				phid->userReadPtr = (phid->userReadPtr + 1) & RFID_DATA_ARRAY_MASK;
			}

			//make sure i is even so that we don't end with a pulse
			if((i % 2) == 1)
			{
				//negate the pulse if we added it
				i--;
				phid->userReadPtr = (phid->userReadPtr - 1) & RFID_DATA_ARRAY_MASK;
			}

			*dataLength = i;

			return EPHIDGET_OK;
		}

		default:
			return EPHIDGET_UNSUPPORTED;
	}
#else
	return EPHIDGET_UNSUPPORTED;
#endif
}

PHIDGET21_API int CCONV CPhidgetRFID_write(CPhidgetRFIDHandle phid, char *tagString, CPhidgetRFID_Protocol protocol, int lock)
{
	int ret = EPHIDGET_OK;
	char newVal[50];
	TESTPTRS(phid,tagString)
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

	switch(phid->phid.deviceUID)
	{
		case PHIDUID_RFID_2OUTPUT_READ_WRITE:
			if(protocol < (PHIDGET_RFID_PROTOCOL_EM4100) || protocol > (PHIDGET_RFID_PROTOCOL_PHIDGETS))
				return EPHIDGET_INVALIDARG;
			if(lock < (PFALSE) || lock > (PTRUE))
				return EPHIDGET_INVALIDARG;

			if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_REMOTE_FLAG))
			{
				sprintf(newVal, "%d/%d/%s", protocol, lock, tagString);
				ADDNETWORKKEY(WriteTag, "%s", remoteWrite);
			}
			else
			{
				CThread_mutex_lock(&phid->tagthreadlock);
				ret = T5577_WriteTag(phid, protocol, tagString, lock);
				CThread_mutex_unlock(&phid->tagthreadlock);
				if(ret != EPHIDGET_OK)
					return ret;
				return waitForTag(phid, tagString, 500);
			}

			break;

		default:
			return EPHIDGET_UNSUPPORTED;
	}

	return EPHIDGET_OK;
}

// === Deprecated Functions === //

CFHANDLE(RFID, Tag, unsigned char *)
CFHANDLE(RFID, TagLost, unsigned char *)
CGET(RFID,NumOutputs,int)
	return CPhidgetRFID_getOutputCount(phid, pVal);
}
CGET(RFID, LastTag, unsigned char)
	int ret = EPHIDGET_OK;
	TESTPTRS(phid,pVal) 
	TESTDEVICETYPE(PHIDCLASS_RFID)
	TESTATTACHED

	//This function only work if the last tag was EM4100
	CThread_mutex_lock(&phid->tagthreadlock);
	if(phid->lastTagValid)
	{
		if(phid->lastTag.protocol == PHIDGET_RFID_PROTOCOL_EM4100)
			memcpy(pVal, phid->lastTag.tagData, 5);
		else
			ret = EPHIDGET_UNSUPPORTED; //unsupported for the protocol of the last tag
	}
	else
		ret = EPHIDGET_UNKNOWNVAL;
	CThread_mutex_unlock(&phid->tagthreadlock);

	return ret;
}
