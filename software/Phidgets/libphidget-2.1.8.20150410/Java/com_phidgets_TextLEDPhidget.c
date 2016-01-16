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
#include "com_phidgets_TextLEDPhidget.h"
#include "../cphidgettextled.h"

JNI_LOAD(led, TextLED)
}

JNI_CREATE(TextLED)
JNI_GETFUNC(TextLED, Brightness, Brightness, jint)
JNI_SETFUNC(TextLED, Brightness, Brightness, jint)
JNI_GETFUNC(TextLED, RowCount, RowCount, jint)
JNI_GETFUNC(TextLED, ColumnCount, ColumnCount, jint)

JNIEXPORT void JNICALL
Java_com_phidgets_TextLEDPhidget_setDisplayString(JNIEnv *env, jobject obj, jint index, jstring v)
{
	CPhidgetTextLEDHandle h = (CPhidgetTextLEDHandle)(uintptr_t)
	    (*env)->GetLongField(env, obj, handle_fid);
	int error;
    jboolean iscopy;
    const char *textString = (*env)->GetStringUTFChars(
                env, v, &iscopy);

	if ((error = CPhidgetTextLED_setDisplayString(h, index, (char *)textString)))
		PH_THROW(error);

	(*env)->ReleaseStringUTFChars(env, v, textString);
}
