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
#include "com_phidgets_StepperPhidget.h"
#include "../cphidgetstepper.h"

EVENT_VARS(stepperPositionChange, StepperPositionChange)
EVENT_VARS(inputChange, InputChange)
EVENT_VARS(stepperVelocityChange, StepperVelocityChange)
EVENT_VARS(currentChange, CurrentChange)

JNI_LOAD(stepper, Stepper)
	EVENT_VAR_SETUP(stepper, stepperPositionChange, StepperPositionChange, IJ, V)
	EVENT_VAR_SETUP(stepper, inputChange, InputChange, IZ, V)
	EVENT_VAR_SETUP(stepper, stepperVelocityChange, StepperVelocityChange, ID, V)
	EVENT_VAR_SETUP(stepper, currentChange, CurrentChange, ID, V)
}

EVENT_HANDLER_INDEXED(Stepper, stepperPositionChange, StepperPositionChange, 
					  CPhidgetStepper_set_OnPositionChange_Handler, long long)
EVENT_HANDLER_INDEXED(Stepper, inputChange, InputChange, 
					  CPhidgetStepper_set_OnInputChange_Handler, int)
EVENT_HANDLER_INDEXED(Stepper, stepperVelocityChange, StepperVelocityChange, 
					  CPhidgetStepper_set_OnVelocityChange_Handler, double)
EVENT_HANDLER_INDEXED(Stepper, currentChange, CurrentChange, 
					  CPhidgetStepper_set_OnCurrentChange_Handler, double)

JNI_CREATE(Stepper)
JNI_INDEXED_GETFUNC(Stepper, Acceleration, Acceleration, jdouble)
JNI_INDEXED_SETFUNC(Stepper, Acceleration, Acceleration, jdouble)
JNI_INDEXED_GETFUNC(Stepper, AccelerationMin, AccelerationMin, jdouble)
JNI_INDEXED_GETFUNC(Stepper, AccelerationMax, AccelerationMax, jdouble)
JNI_INDEXED_GETFUNC(Stepper, Velocity, Velocity, jdouble)
JNI_INDEXED_SETFUNC(Stepper, VelocityLimit, VelocityLimit, jdouble)
JNI_INDEXED_GETFUNC(Stepper, VelocityLimit, VelocityLimit, jdouble)
JNI_INDEXED_GETFUNC(Stepper, VelocityMin, VelocityMin, jdouble)
JNI_INDEXED_GETFUNC(Stepper, VelocityMax, VelocityMax, jdouble)
JNI_INDEXED_GETFUNC(Stepper, TargetPosition, TargetPosition, jlong)
JNI_INDEXED_SETFUNC(Stepper, TargetPosition, TargetPosition, jlong)
JNI_INDEXED_GETFUNC(Stepper, CurrentPosition, CurrentPosition, jlong)
JNI_INDEXED_SETFUNC(Stepper, CurrentPosition, CurrentPosition, jlong)
JNI_INDEXED_GETFUNC(Stepper, PositionMin, PositionMin, jlong)
JNI_INDEXED_GETFUNC(Stepper, PositionMax, PositionMax, jlong)
JNI_INDEXED_GETFUNC(Stepper, Current, Current, jdouble)
JNI_INDEXED_SETFUNC(Stepper, CurrentLimit, CurrentLimit, jdouble)
JNI_INDEXED_GETFUNC(Stepper, CurrentLimit, CurrentLimit, jdouble)
JNI_INDEXED_GETFUNC(Stepper, CurrentMin, CurrentMin, jdouble)
JNI_INDEXED_GETFUNC(Stepper, CurrentMax, CurrentMax, jdouble)
JNI_INDEXED_GETFUNCBOOL(Stepper, Stopped, Stopped)
JNI_INDEXED_GETFUNCBOOL(Stepper, Engaged, Engaged)
JNI_INDEXED_SETFUNC(Stepper, Engaged, Engaged, jboolean)
JNI_INDEXED_GETFUNCBOOL(Stepper, InputState, InputState)
JNI_GETFUNC(Stepper, MotorCount, MotorCount, jint)
JNI_GETFUNC(Stepper, InputCount, InputCount, jint)
