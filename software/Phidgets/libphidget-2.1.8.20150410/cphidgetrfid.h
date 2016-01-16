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

#ifndef __CPHIDGETRFID
#define __CPHIDGETRFID
#include "cphidget.h"

/** \defgroup phidrfid Phidget RFID 
 * \ingroup phidgets
 * These calls are specific to the Phidget RFID object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(RFID)
CHDRSTANDARD(RFID)

#ifdef DEBUG
#define RFID_HITAGS_SUPPORT
#define RFID_RAWDATA_API_SUPPORT
#endif

/**
 * RFID encoding protocols supported by the PhidgetRFID Read-Write
 */
typedef enum {
	PHIDGET_RFID_PROTOCOL_EM4100 = 1,		/**< EM4100 (EM4102) 40-bit */
	PHIDGET_RFID_PROTOCOL_ISO11785_FDX_B,	/**< ISO11785 FDX-B encoding (Animal ID) */
	PHIDGET_RFID_PROTOCOL_PHIDGETS,			/**< PhidgetTAG Protocol 24 character ASCII */
#ifdef RFID_HITAGS_SUPPORT
	PHIDGET_RFID_PROTOCOL_HITAGS_UID		/**< HiTag S UID */
#endif
} CPhidgetRFID_Protocol;

/**
 * Gets the number of outputs supported by this board.
 * @param phid An attached phidget rfid handle.
 * @param count The output count.
 */
CHDRGET(RFID,OutputCount,int *count)
/**
 * Gets the state of an output.
 * @param phid An attached phidget rfid handle.
 * @param index The output index.
 * @param outputState The output state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(RFID,OutputState,int *outputState)
/**
 * Sets the state of an output.
 * @param phid An attached phidget rfid handle.
 * @param index The output index.
 * @param outputState The output state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSETINDEX(RFID,OutputState,int outputState)
/**
 * Set an output change handler. This is called when an output changes.
 * @param phid An attached phidget rfid handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(RFID,OutputChange,int outputState)

/**
 * Gets the state of the antenna.
 * @param phid An attached phidget rfid handle.
 * @param antennaState The antenna state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGET(RFID,AntennaOn,int *antennaState)
/**
 * Sets the state of the antenna. Note that the antenna must be enabled before tags will be read.
 * @param phid An attached phidget rfid handle.
 * @param antennaState The antenna state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSET(RFID,AntennaOn,int antennaState)
/**
 * Gets the state of the onboard LED.
 * @param phid An attached phidget rfid handle.
 * @param LEDState The LED state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGET(RFID,LEDOn,int *LEDState)
/**
 * Sets the state of the onboard LED.
 * @param phid An attached phidget rfid handle.
 * @param LEDState The LED state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSET(RFID,LEDOn,int LEDState)

/**
 * Gets the last tag read by the reader. This tag may or may not still be on the reader.
 * @param phid An attached phidget rfid handle.
 * @param tagString A pointer which will be set to point to a char array containing the tag string.
 * @param protocol The tag protocol.
 */
CHDRGET(RFID,LastTag2,char **tagString, CPhidgetRFID_Protocol *protocol)
/**
 * Gets the tag present status. This is whether or not a tag is being read by the reader.
 * @param phid An attached phidget rfid handle.
 * @param status The tag status. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGET(RFID,TagStatus,int *status)

/**
 * Program a tag. This requires a T5577 tag.
 * @param phid An attached phidget rfid handle.
 * @param tagString The tag data to write. 
 *		EM4100 (40-bit hex): "90fd32987b"
 *		FDX-B (15 digit decimal): "999000000003471"
 *		Phidgets (7-bit ASCII, up to 24 characters): "I'm a PHIDGET tag"
 * @param protocol The tag protocol to write
 * @param lock Lock the tag so that it cannot be written again.
 */
PHIDGET21_API int CCONV CPhidgetRFID_write(CPhidgetRFIDHandle phid, char *tagString, CPhidgetRFID_Protocol protocol, int lock);

/**
 * Set a tag handler. This is called when a tag is first detected by the reader.
 * @param phid An attached phidget rfid handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENT(RFID, Tag2, char *tagString, CPhidgetRFID_Protocol protocol)
/**
 * Set a tag lost handler. This is called when a tag is no longer detected by the reader.
 * @param phid An attached phidget rfid handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENT(RFID, TagLost2, char *tagString, CPhidgetRFID_Protocol protocol)

#ifndef REMOVE_DEPRECATED
DEP_CHDRGET("Deprecated - use CPhidgetRFID_getOutputCount",RFID,NumOutputs,int *)
DEP_CHDRGET("Deprecated - use CPhidgetRFID_getLastTag2",RFID,LastTag,unsigned char *tag)
DEP_CHDREVENT("Deprecated - use CPhidgetRFID_set_OnTag2_Handler",RFID,Tag,unsigned char *tag)
DEP_CHDREVENT("Deprecated - use CPhidgetRFID_set_OnTagLost2_Handler",RFID,TagLost,unsigned char *tag)
#endif

/* Keep these hidden - unofficial API, not supported. */
#if !defined(EXTERNALPROTO) || defined(RFID_HITAGS_SUPPORT)
/* poll for HiTag S tags */
CHDRSET(RFID,PollingOn,int pollingState)
#endif
#if !defined(EXTERNALPROTO) || defined(RFID_RAWDATA_API_SUPPORT)
/* Write raw data to a tag */
PHIDGET21_API int CCONV CPhidgetRFID_writeRaw(CPhidgetRFIDHandle phid, unsigned char *data, int bitlength, int pregap, int space, int postgap, int zero, int one, int prepulse, int eof, int listenDuringEOF);
/* Read in raw data from tags */
PHIDGET21_API int CCONV CPhidgetRFID_getRawData(CPhidgetRFIDHandle phid, int *data, int *dataLength);
/* Raw data event */
CHDREVENT(RFID, RawData, int *data, int dataLength)
/* Manchester data event */
CHDREVENT(RFID, ManchesterData, unsigned char *data, int dataLength)
#endif

#ifndef EXTERNALPROTO

#define RFID_PACKET_TAG 0
#define RFID_PACKET_OUTPUT_ECHO 1

#define RFID_LED_FLAG 0x04
#define RFID_ANTENNA_FLAG 0x08
#define RFID_LISTEN_DURING_EOF_FLAG 0x10

#define RFID_WRITE_DATA_OUT_PACKET	0x00
#define RFID_CONTROL_OUT_PACKET		0x40

#define RFID_READ_DATA_IN_PACKET	0x00
#define RFID_ECHO_IN_PACKET			0x40

#define RFID_MAX_DATA_PER_PACKET	63

#define RFID_DATA_ARRAY_SIZE		1024
#define RFID_DATA_ARRAY_MASK		0x3ff

#define RFID_MAXOUTPUTS 2


/* 4097 constants */
#define RFID_4097_AmpDemod		0x00	//Amplitude demodulation
#define RFID_4097_PhaseDemod	0x01	//Phase demodulation

#define RFID_4097_PowerDown		0x00
#define RFID_4097_Active		0x02

#define RFID_4097_DataOut		0x00	//DATA_OUT is data from the rfid card
#define RFID_4097_ClkOut		0x04	//DATA_OUT is the internal clock/32

#define	RFID_4097_IntPLL		0x00
#define RFID_4097_ExtClk		0x08

#define RFID_4097_FastStart		0x10

#define RFID_4097_Gain960		0x40
#define RFID_4097_Gain480		0x00
#define RFID_4097_Gain240		0x60
#define RFID_4097_Gain120		0x20

#define RFID_4097_TestMode		0x80

#define RFID_4097_DefaultON		(RFID_4097_AmpDemod | RFID_4097_Active | RFID_4097_DataOut | RFID_4097_IntPLL | RFID_4097_FastStart | RFID_4097_Gain960)


/* T5577 Write Timing Constants */
#define RFID_T5577_StartGap 30
#define RFID_T5577_WriteGap 15
#define RFID_T5577_EndGap 15
#define RFID_T5577_Zero 24
#define RFID_T5577_One 56
#define RFID_T5577_EOF 100
#define RFID_T5577_PrePulse (136 + RFID_T5577_Zero)

#ifdef RFID_HITAGS_SUPPORT
typedef enum {
	PHIDGET_RFID_TAG_READONLY = 1,
	PHIDGET_RFID_TAG_HITAGS
} CPhidgetRFID_TagType;
#endif

typedef enum {
	PHIDGET_RFID_ENCODING_MANCHESTER = 1,
	PHIDGET_RFID_ENCODING_BIPHASE,
#ifdef RFID_HITAGS_SUPPORT
	PHIDGET_RFID_ENCODING_AC
#endif
} CPhidgetRFID_Encoding;

#define RFID_MAX_TAG_STRING_LEN 25
typedef struct _CPhidgetRFID_Tag
{
	CPhidgetRFID_Protocol protocol;
	char tagString[RFID_MAX_TAG_STRING_LEN];
	unsigned char tagData[10]; //used for old EM4100 events
#ifdef RFID_HITAGS_SUPPORT
	CPhidgetRFID_TagType tagType;
#endif
} CPhidgetRFID_Tag, *CPhidgetRFID_TagHandle;

struct _CPhidgetRFID {
	CPhidget phid;

	int (CCONV *fptrOutputChange)(CPhidgetRFIDHandle, void *, int, int);
	int (CCONV *fptrTag)(CPhidgetRFIDHandle, void *, unsigned char *);
	int (CCONV *fptrTagLost)(CPhidgetRFIDHandle, void *, unsigned char *);
	int (CCONV *fptrTag2)(CPhidgetRFIDHandle, void *, char *, CPhidgetRFID_Protocol);
	int (CCONV *fptrTagLost2)(CPhidgetRFIDHandle, void *, char *, CPhidgetRFID_Protocol);

	void *fptrOutputChangeptr;
	void *fptrTagptr;
	void *fptrTagLostptr;
	void *fptrTag2ptr;
	void *fptrTagLost2ptr;
	
#ifdef RFID_RAWDATA_API_SUPPORT
	int (CCONV *fptrRawData)(CPhidgetRFIDHandle, void *, int *, int);
	int (CCONV *fptrManchesterData)(CPhidgetRFIDHandle, void *, unsigned char *, int);
	void *fptrRawDataptr;
	void *fptrManchesterDataptr;
#endif

	/* State */
	unsigned char outputEchoState[RFID_MAXOUTPUTS];
	unsigned char antennaEchoState;
	unsigned char ledEchoState;
	int spaceClocks, pregapClocks, postgapClocks, oneClocks, zeroClocks, prepulseClocks, eofpulseClocks;
	unsigned char listenDuringEOF;
	int _4097Conf;

	/* State Echo */
	unsigned char outputState[RFID_MAXOUTPUTS];
	unsigned char antennaState;
	unsigned char ledState;
	int spaceClocksEcho, pregapClocksEcho, postgapClocksEcho, oneClocksEcho, zeroClocksEcho, prepulseClocksEcho, eofpulseClocksEcho;
	unsigned char listenDuringEOFEcho;
	int _4097ConfEcho;

	unsigned char fullStateEcho;

	CThread tagTimerThread;
	CThread_mutex_t tagthreadlock; /* protects tag thread access to things */
	EVENT tagAvailableEvent;

	/* Tag event */
	CPhidgetRFID_Tag lastTag;
	unsigned char lastTagValid;
	TIME lastTagTime;
	unsigned char tagPresent;
	CPhidgetRFID_Tag pendingTag;
	unsigned char tagEventPending;

	/* Raw data buffer */
	int dataBuffer[RFID_DATA_ARRAY_SIZE];
	unsigned int dataReadPtr, dataWritePtr;
	
	int shortClocks, longClocks;

	/* Manchester decoder */
	unsigned char manBuffer[RFID_DATA_ARRAY_SIZE];
	int manReadPtr, manWritePtr;
	unsigned char manLockedIn;
	unsigned char manShortChange;

	/* BiPhase Decoder */
	unsigned char biphaseBuffer[RFID_DATA_ARRAY_SIZE];
	int biphaseReadPtr, biphaseWritePtr;
	unsigned char biphaseLockedIn;
	unsigned char biphaseShortChange;

#ifdef RFID_HITAGS_SUPPORT
	TIME lastDataTime;
	unsigned int dataReadACPtr;
	TIME hitagReqTime;
	unsigned char polling;
	unsigned char ACCodingOK;
#endif
#ifdef RFID_RAWDATA_API_SUPPORT
	unsigned int userReadPtr;
	int manEventReadPtr;
	unsigned char lastManEventLong;
#endif

	//for remote write
	char *remoteWrite;

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;

} typedef CPhidgetRFIDInfo;
#endif

/** @} */

#endif
