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
#include "com_phidgets_AnalogPhidget.h"
#include "../cphidgetanalog.h"

JNI_LOAD(analog, Analog)
}

JNI_CREATE(Analog)

JNI_GETFUNC(Analog, OutputCount, OutputCount, jint)
JNI_INDEXED_GETFUNC(Analog, Voltage, Voltage, jdouble)
JNI_INDEXED_SETFUNC(Analog, Voltage, Voltage, jdouble)
JNI_INDEXED_GETFUNC(Analog, VoltageMax, VoltageMax, jdouble)
JNI_INDEXED_GETFUNC(Analog, VoltageMin, VoltageMin, jdouble)
JNI_INDEXED_GETFUNCBOOL(Analog, Enabled, Enabled)
JNI_INDEXED_SETFUNC(Analog, Enabled, Enabled, jboolean)