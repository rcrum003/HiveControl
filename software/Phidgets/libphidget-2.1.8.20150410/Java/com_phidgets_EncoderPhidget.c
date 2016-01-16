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

#include "../stdafx.h"
#include "phidget_jni.h"
#include "com_phidgets_EncoderPhidget.h"
#include "../cphidgetencoder.h"

EVENT_VARS(inputChange, InputChange)
EVENT_VARS(encoderPositionChange, EncoderPositionChange)

JNI_LOAD(enc, Encoder)
	EVENT_VAR_SETUP(enc, inputChange, InputChange, IZ, V)
	EVENT_VAR_SETUP(enc, encoderPositionChange, EncoderPositionChange, III, V)
}

EVENT_HANDLER_INDEXED(Encoder, inputChange, InputChange, 
					  CPhidgetEncoder_set_OnInputChange_Handler, int)
EVENT_HANDLER_INDEXED2(Encoder, encoderPositionChange, EncoderPositionChange, 
					  CPhidgetEncoder_set_OnPositionChange_Handler, int, int)

JNI_CREATE(Encoder)
JNI_INDEXED_GETFUNC(Encoder, Position, Position, jint)
JNI_INDEXED_SETFUNC(Encoder, Position, Position, jint)
JNI_INDEXED_GETFUNCBOOL(Encoder, InputState, InputState)
JNI_GETFUNC(Encoder, EncoderCount, EncoderCount, jint)
JNI_GETFUNC(Encoder, InputCount, InputCount, jint)
JNI_INDEXED_GETFUNC(Encoder, IndexPosition, IndexPosition, jint)
JNI_INDEXED_GETFUNCBOOL(Encoder, Enabled, Enabled)
JNI_INDEXED_SETFUNC(Encoder, Enabled, Enabled, jboolean)

//Deprecated
JNI_INDEXED_GETFUNC(Encoder, EncoderPosition, Position, jint)
JNI_INDEXED_SETFUNC(Encoder, EncoderPosition, Position, jint)
