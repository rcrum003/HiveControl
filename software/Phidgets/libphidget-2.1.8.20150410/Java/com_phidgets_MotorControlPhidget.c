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
#include "com_phidgets_MotorControlPhidget.h"
#include "../cphidgetmotorcontrol.h"

EVENT_VARS(inputChange, InputChange)
EVENT_VARS(motorVelocityChange, MotorVelocityChange)
EVENT_VARS(currentChange, CurrentChange)
EVENT_VARS(currentUpdate, CurrentUpdate)
EVENT_VARS(encoderPositionChange, EncoderPositionChange)
EVENT_VARS(encoderPositionUpdate, EncoderPositionUpdate)
EVENT_VARS(backEMFUpdate, BackEMFUpdate)
EVENT_VARS(sensorUpdate, SensorUpdate)

JNI_LOAD(motor, MotorControl)
	EVENT_VAR_SETUP(motor, inputChange, InputChange, IZ, V)
	EVENT_VAR_SETUP(motor, motorVelocityChange, MotorVelocityChange, ID, V)
	EVENT_VAR_SETUP(motor, currentChange, CurrentChange, ID, V)
	EVENT_VAR_SETUP(motor, currentUpdate, CurrentUpdate, ID, V)
	EVENT_VAR_SETUP(motor, encoderPositionChange, EncoderPositionChange, III, V)
	EVENT_VAR_SETUP(motor, encoderPositionUpdate, EncoderPositionUpdate, II, V)
	EVENT_VAR_SETUP(motor, backEMFUpdate, BackEMFUpdate, ID, V)
	EVENT_VAR_SETUP(motor, sensorUpdate, SensorUpdate, II, V)

}

EVENT_HANDLER_INDEXED(MotorControl, inputChange, InputChange, 
					  CPhidgetMotorControl_set_OnInputChange_Handler, int)
EVENT_HANDLER_INDEXED(MotorControl, motorVelocityChange, MotorVelocityChange, 
					  CPhidgetMotorControl_set_OnVelocityChange_Handler, double)
EVENT_HANDLER_INDEXED(MotorControl, currentChange, CurrentChange, 
					  CPhidgetMotorControl_set_OnCurrentChange_Handler, double)
EVENT_HANDLER_INDEXED(MotorControl, currentUpdate, CurrentUpdate, 
					  CPhidgetMotorControl_set_OnCurrentUpdate_Handler, double)
EVENT_HANDLER_INDEXED2(MotorControl, encoderPositionChange, EncoderPositionChange, 
					  CPhidgetMotorControl_set_OnEncoderPositionChange_Handler, int, int)
EVENT_HANDLER_INDEXED(MotorControl, encoderPositionUpdate, EncoderPositionUpdate, 
					  CPhidgetMotorControl_set_OnEncoderPositionUpdate_Handler, int)
EVENT_HANDLER_INDEXED(MotorControl, backEMFUpdate, BackEMFUpdate, 
					  CPhidgetMotorControl_set_OnBackEMFUpdate_Handler, double)
EVENT_HANDLER_INDEXED(MotorControl, sensorUpdate, SensorUpdate, 
					  CPhidgetMotorControl_set_OnSensorUpdate_Handler, int)


JNI_CREATE(MotorControl)
JNI_INDEXED_GETFUNC(MotorControl, Acceleration, Acceleration, jdouble)
JNI_INDEXED_SETFUNC(MotorControl, Acceleration, Acceleration, jdouble)
JNI_INDEXED_GETFUNC(MotorControl, AccelerationMin, AccelerationMin, jdouble)
JNI_INDEXED_GETFUNC(MotorControl, AccelerationMax, AccelerationMax, jdouble)
JNI_INDEXED_GETFUNC(MotorControl, Velocity, Velocity, jdouble)
JNI_INDEXED_SETFUNC(MotorControl, Velocity, Velocity, jdouble)
JNI_INDEXED_GETFUNC(MotorControl, Current, Current, jdouble)
JNI_INDEXED_GETFUNCBOOL(MotorControl, InputState, InputState)

JNI_INDEXED_GETFUNC(MotorControl, EncoderPosition, EncoderPosition, jint)
JNI_INDEXED_SETFUNC(MotorControl, EncoderPosition, EncoderPosition, jint)
JNI_INDEXED_GETFUNCBOOL(MotorControl, BackEMFSensingState, BackEMFSensingState)
JNI_INDEXED_SETFUNC(MotorControl, BackEMFSensingState, BackEMFSensingState, jboolean)
JNI_INDEXED_GETFUNC(MotorControl, BackEMF, BackEMF, jdouble)
JNI_GETFUNC(MotorControl, SupplyVoltage, SupplyVoltage, jdouble)
JNI_INDEXED_GETFUNC(MotorControl, Braking, Braking, jdouble)
JNI_INDEXED_SETFUNC(MotorControl, Braking, Braking, jdouble)
JNI_INDEXED_GETFUNC(MotorControl, SensorValue, SensorValue, jint)
JNI_INDEXED_GETFUNC(MotorControl, SensorRawValue, SensorRawValue, jint)
JNI_GETFUNCBOOL(MotorControl, Ratiometric, Ratiometric)
JNI_SETFUNC(MotorControl, Ratiometric, Ratiometric, jboolean)

JNI_GETFUNC(MotorControl, MotorCount, MotorCount, jint)
JNI_GETFUNC(MotorControl, InputCount, InputCount, jint)
JNI_GETFUNC(MotorControl, EncoderCount, EncoderCount, jint)
JNI_GETFUNC(MotorControl, SensorCount, SensorCount, jint)

//Deprecated
JNI_INDEXED_GETFUNC(MotorControl, Speed, Velocity, jdouble)
JNI_INDEXED_SETFUNC(MotorControl, Speed, Velocity, jdouble)
