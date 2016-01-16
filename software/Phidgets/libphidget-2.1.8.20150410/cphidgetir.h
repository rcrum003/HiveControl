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

#ifndef __CPHIDGETIR
#define __CPHIDGETIR
#include "cphidget.h"

/** \defgroup phidIR Phidget IR 
 * \ingroup phidgets
 * These calls are specific to the Phidget IR object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(IR)
CHDRSTANDARD(IR)

//This needs to be evenly divisible by 8
#define IR_MAX_CODE_BIT_COUNT		128							/**< Maximum bit count for sent / received data */
#define IR_MAX_CODE_DATA_LENGTH		(IR_MAX_CODE_BIT_COUNT / 8)	/**< Maximum array size needed to hold the longest code */
#define IR_MAX_REPEAT_LENGTH		26							/**< Maximum array size for a repeat code */

/**
 * The PhidgetIR supports these data encodings.
 */
typedef enum {
	PHIDGET_IR_ENCODING_UNKNOWN = 1,	/**< Unknown - the default value */
	PHIDGET_IR_ENCODING_SPACE,			/**< Space encoding, or Pulse Distance Modulation */
	PHIDGET_IR_ENCODING_PULSE,			/**< Pulse encoding, or Pulse Width Modulation */
	PHIDGET_IR_ENCODING_BIPHASE,		/**< Bi-Phase, or Manchester encoding */
	PHIDGET_IR_ENCODING_RC5,			/**< RC5 - a type of Bi-Phase encoding */
	PHIDGET_IR_ENCODING_RC6				/**< RC6 - a type of Bi-Phase encoding */
} CPhidgetIR_Encoding;

/**
 * The PhidgetIR supports these encoding lengths
 */
typedef enum {
	PHIDGET_IR_LENGTH_UNKNOWN = 1,	/**< Unknown - the default value */
	PHIDGET_IR_LENGTH_CONSTANT,		/**< Constant - the bitstream + gap length is constant */
	PHIDGET_IR_LENGTH_VARIABLE		/**< Variable - the bitstream has a variable length with a constant gap */
} CPhidgetIR_Length;

//If you modify this, it's NEEDS to be modified in .NET and Flash!!!
/**
 * The PhidgetIR CodeInfo structure contains all information needed to transmit a code, apart from the actual code data.
 * Some values can be set to null to select defaults. See the product manual for more information.
 */
typedef struct _CPhidgetIR_CodeInfo
{
	int						bitCount;	/**< Number of bits in the code */
	CPhidgetIR_Encoding		encoding;	/**< Encoding used to encode the data */
	CPhidgetIR_Length		length;		/**< Constan or Variable length encoding */
	int			gap;		/**< Gap time in us */
	int			trail;		/**< Trail time in us - can be 0 for none */
	int			header[2];	/**< Header pulse and space - can be 0 for none */
	int			one[2];		/**< Pulse and Space times to represent a '1' bit, in us */
	int			zero[2];	/**< Pulse and Space times to represent a '0' bit, in us */
	int			repeat[IR_MAX_REPEAT_LENGTH];	/**< A series or pulse and space times to represent the repeat code. Start and end with pulses and null terminate. Set to 0 for none. */
	int						min_repeat;	/**< Minium number of times to repeat a code on transmit */
	unsigned char			toggle_mask[IR_MAX_CODE_DATA_LENGTH];	/**< Bit toggles, which are applied to the code after each transmit */
	int						carrierFrequency;	/**< Carrier frequency in Hz - defaults to 38kHz */
	int						dutyCycle;	/**< Duty Cycle in percent (10-50). Defaults to 33 */
} CPhidgetIR_CodeInfo, *CPhidgetIR_CodeInfoHandle;

/**
 * Transmits a code according to the settings in a CodeInto structure
 * @param phid An attached phidget ir handle.
 * @param data The code to send. Data is transmitted MSBit first. MSByte is in array index 0. LSBit is right justified, so MSBit may be in bit positions 0-7 in array index 0 depending on the bit count.
 * @param codeInfo The CodeInfo structure specifying to to send the code. Anything left as null to select default is filled in for the user.
 */
PHIDGET21_API int CCONV CPhidgetIR_Transmit(CPhidgetIRHandle phid, unsigned char *data, CPhidgetIR_CodeInfoHandle codeInfo);
/**
 * Transmits a repeat of the last transmited code. Depending of the CodeInfo structure, this may be a retransmission of the code itself,
 * or there may be a special repeat code.
 * @param phid An attached phidget ir handle.
 */
PHIDGET21_API int CCONV CPhidgetIR_TransmitRepeat(CPhidgetIRHandle phid);
/**
 * Transmits RAW data as a series of pulses and spaces.
 * @param phid An attached phidget ir handle.
 * @param data The data to send. The array must start and end with a pulse and each element is a positive time in us.
 * @param length The length of the data array. Maximum length is 200, but streams should be kept much shorter, ie. < 100ms between gaps.
 * @param carrierFrequency The Carrier Frequency in Hz. leave as 0 for default.
 * @param dutyCycle The Duty Cycle (10-50). Leave as 0 for default.
 * @param gap The gap time in us. This guarantees a gap time (no transmitting) after the data is sent, but can be set to 0.
 */
PHIDGET21_API int CCONV CPhidgetIR_TransmitRaw(CPhidgetIRHandle phid, int *data, int length, int carrierFrequency, int dutyCycle, int gap);
/**
 * Read any available raw data. This should be polled continuously (every 20ms) to avoid missing data. Read data always starts with a space and ends with a pulse.
 * @param phid An attached phidget ir handle.
 * @param data A user array for raw data to be written into.
 * @param dataLength The maximum ammount of data to read. This is set to the actual ammount of data read.
 */
PHIDGET21_API int CCONV CPhidgetIR_getRawData(CPhidgetIRHandle phid, int *data, int *dataLength);
/**
 * Gets the last code that was received.
 * @param phid An attached phidget ir handle.
 * @param data A user array to store the code data in.
 * @param dataLength Length of the user array - should be at least IR_MAX_CODE_DATA_LENGTH. This is set to the ammount of data actually written to the array.
 * @param bitCount set to the bit count of the code.
 */
PHIDGET21_API int CCONV CPhidgetIR_getLastCode(CPhidgetIRHandle phid, unsigned char *data, int *dataLength, int *bitCount);
/**
 * Gets the last code that was learned.
 * @param phid An attached phidget ir handle.
 * @param data A user array to store the code data in.
 * @param dataLength Length of the user array - should be at least IR_MAX_CODE_DATA_LENGTH. This is set to the ammount of data actually written to the array.
 * @param codeInfo The CodeInfo structure for the learned code.
 */
PHIDGET21_API int CCONV CPhidgetIR_getLastLearnedCode(CPhidgetIRHandle phid, unsigned char *data, int *dataLength, CPhidgetIR_CodeInfo *codeInfo);
/**
 * Set a Code handler. This is called when a code has been received that could be automatically decoded.
 * Data is return as an array with MSB in index 0. Bit count and a repeat flag are also returned.
 * Repeats are detected as either the same code repeated in < 100ms or as a special repeat code.
 * @param phid An attached phidget ir handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENT(IR, Code, unsigned char *data, int dataLength, int bitCount, int repeat)
/**
 * Set a Learn handler. This is called when a code has been received for long enough to be learned. 
 * The returned CodeInfo structure can be used to retransmit the same code.
 * @param phid An attached phidget ir handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENT(IR, Learn, unsigned char *data, int dataLength, CPhidgetIR_CodeInfoHandle codeInfo)
/**
 * Set a Raw Data handler. This is called when raw data has been read from the device. Raw data always starts with a space and ends with a pulse.
 * @param phid An attached phidget ir handle.
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENT(IR, RawData, int *data, int dataLength)

#ifndef EXTERNALPROTO
PHIDGET21_API int CCONV codeInfoToString(CPhidgetIR_CodeInfoHandle codeInfo, char *string);
PHIDGET21_API int CCONV stringToCodeInfo(char *string, CPhidgetIR_CodeInfoHandle codeInfo);

#define IR_MAX_DATA_PER_PACKET	31

#define IR_DATA_ARRAY_SIZE		2048
#define IR_DATA_ARRAY_MASK		0x7ff

//for transmitting / receiving raw data
#define IR_MAX_DATA_us			327670

//this is just actual gap, not the gap that includes data
#define IR_MAX_GAP_LENGTH		100000 //us
#define IR_MIN_GAP_LENGTH		20000 //us

#define IR_DEFINEDATA_PACKET	0

#define IR_STOP_RX_WHILE_TX_FLAG	0x01

#define IR_RAW_DATA_WS_KEYS_MAX		100

struct _CPhidgetIR {
	CPhidget phid;

	int (CCONV *fptrCode)(CPhidgetIRHandle, void *, unsigned char *, int, int, int);	//data, length, bitCount, repeat
	int (CCONV *fptrRawData)(CPhidgetIRHandle, void *, int *, int);
	int (CCONV *fptrLearn)(CPhidgetIRHandle, void *, unsigned char *, int, CPhidgetIR_CodeInfoHandle); //needs > 1 sec. of continuous data - for learning

	void *fptrCodeptr;
	void *fptrRawDataptr;
	void *fptrLearnptr;

	int dataBuffer[IR_DATA_ARRAY_SIZE];
	int dataBufferNormalized[IR_DATA_ARRAY_SIZE];
	int dataReadPtr, dataWritePtr;
	int userReadPtr; //for the getRawData function
	int learnReadPtr; //for the learning function

	unsigned char polarity;

	unsigned char lastCodeKnown;
	unsigned char lastCode[IR_MAX_CODE_BIT_COUNT/8];
	CPhidgetIR_CodeInfo lastCodeInfo;
	unsigned char lastRepeat;
	int lastGap;
	
	unsigned char lastLearnedCodeKnown;
	unsigned char lastLearnedCode[IR_MAX_CODE_BIT_COUNT/8];
	CPhidgetIR_CodeInfo lastLearnedCodeInfo;

	unsigned char lastSentCode[IR_MAX_CODE_BIT_COUNT/8];
	CPhidgetIR_CodeInfo lastSentCodeInfo;

	TIME	lastDataTime;

	unsigned char delayCode;

	char *tempString;
	int flip;

	int rawDataSendWSCounter, rawDataSendWSKeys[IR_RAW_DATA_WS_KEYS_MAX];

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;
} typedef CPhidgetIRInfo;
#endif

/** @} */

#endif
