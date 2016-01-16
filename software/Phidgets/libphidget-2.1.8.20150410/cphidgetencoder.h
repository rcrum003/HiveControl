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

#ifndef __CPHIDGETENCODER
#define __CPHIDGETENCODER
#include "cphidget.h"

/** \defgroup phidenc Phidget Encoder 
 * \ingroup phidgets
 * These calls are specific to the Phidget Encoder object. See your device's User Guide for more specific API details, technical information, and revision details. The User Guide, along with other resources, can be found on the product page for your device.
 * @{
 */

DPHANDLE(Encoder)
CHDRSTANDARD(Encoder)

/**
 * Gets the number of digital inputs supported by this board.
 * @param phid An attached phidget encoder handle
 * @param count The input count.
 */
CHDRGET(Encoder,InputCount,int *count)
/**
 * Gets the state of a digital input.
 * @param phid An attached phidget encoder handle
 * @param index The input index.
 * @param inputState The input state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(Encoder,InputState,int *inputState)
/**
 * Sets an input change handler. This is called when a digital input changes.
 * @param phid An attached phidget encoder handle
 * @param fptr Callback function pointer.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(Encoder,InputChange,int inputState)
/**
 * Gets the number of encoder inputs supported by this board.
 * @param phid An attached phidget encoder handle
 * @param count The encoder input count.
 */
CHDRGET(Encoder,EncoderCount,int *count)
/**
 * Gets the current position of an encoder.
 * @param phid An attached phidget encoder handle
 * @param index The encoder input index.
 * @param position The current position
 */
CHDRGETINDEX(Encoder,Position,int *position)
/**
 * Sets the current position of an encoder.
 * @param phid An attached phidget encoder handle
 * @param index The encoder input index.
 * @param position The new position
 */
CHDRSETINDEX(Encoder,Position,int position)
/**
 * Sets an encoder position change handler. This is called when an encoder position changes.
 * @param phid An attached phidget encoder handle
 * @param fptr Callback function pointer. Note that positionChange is a relative not absolute change and time is the time
 *	in ms since the last position change event.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(Encoder,PositionChange,int time,int positionChange)
/**
 * Sets an encoder index handler. This is called when there is a pulse on the index pin.
 * @param phid An attached phidget encoder handle
 * @param fptr Callback function pointer. This returns the encoder position at which the index pulse occured.
 * @param userPtr A pointer for use by the user - this value is passed back into the callback function.
 */
CHDREVENTINDEX(Encoder,Index,int indexPosition)
/**
 * Gets the position of the last index pulse, as referenced to \ref CPhidgetEncoder_getPosition.
 * This will return EPHIDGET_UNKNOWN if there hasn't been an index event, or if the encoder doesn't support index.
 * @param phid An attached phidget encoder handle
 * @param index The encoder index.
 * @param position The index position.
 */
CHDRGETINDEX(Encoder,IndexPosition,int *position)
/**
 * Gets the enabled state of an encoder. This is whether the encoder is powered or not.
 * @param phid An attached phidget encoder handle
 * @param index The encoder index.
 * @param enabledState The enabled state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRGETINDEX(Encoder,Enabled,int *enabledState)
/**
 * Sets the enabled state of an encoder. This is whether the encoder is powered or not.
 * @param phid An attached phidget encoder handle
 * @param index The encoder index.
 * @param enabledState The enabled state. Possible values are \ref PTRUE and \ref PFALSE.
 */
CHDRSETINDEX(Encoder,Enabled,int enabledState)

#ifndef REMOVE_DEPRECATED
DEP_CHDRGETINDEX("Deprecated - use CPhidgetEncoder_getPosition",Encoder,EncoderPosition,int *)
DEP_CHDRSETINDEX("Deprecated - use CPhidgetEncoder_setPosition",Encoder,EncoderPosition,int)
DEP_CHDRGET("Deprecated - use CPhidgetEncoder_getInputCount",Encoder,NumInputs,int *)
DEP_CHDRGET("Deprecated - use CPhidgetEncoder_getEncoderCount",Encoder,NumEncoders,int *)
#endif

#ifndef EXTERNALPROTO
#define ENCODER_MAXENCODERS 4
#define ENCODER_MAXINPUTS 4
struct _CPhidgetEncoder {
	CPhidget phid;

	int (CCONV *fptrInputChange)        (CPhidgetEncoderHandle, void *, int, int); 
	void *fptrInputChangeptr;
	int (CCONV *fptrPositionChange)        (CPhidgetEncoderHandle, void *, int, int, int); 
	void *fptrPositionChangeptr;
	int (CCONV *fptrIndex)        (CPhidgetEncoderHandle, void *, int, int); 
	void *fptrIndexptr;

	unsigned char inputState[ENCODER_MAXINPUTS];

	int encoderPosition[ENCODER_MAXENCODERS];
	int encoderTimeStamp[ENCODER_MAXENCODERS];

	unsigned char enableState[ENCODER_MAXENCODERS];
	unsigned char enableStateEcho[ENCODER_MAXENCODERS];

	int indexPosition[ENCODER_MAXENCODERS];

	unsigned char outputPacket[MAX_OUT_PACKET_SIZE];
	unsigned int outputPacketLen;

} typedef CPhidgetEncoderInfo;
#endif

/** @} */

#endif
