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

#ifndef __PHIDGET21
#define __PHIDGET21
 
 /*! \mainpage Phidget21 C API Documentation
 *
 * Describes the Application Program Interface (API) for each Phidget device.  
 * The API can be used by a number of languages; 
 * this manual discusses use via C and the code examples reflect this.
 *
 * \section howto_sec How to use Phidgets
 Phidgets are an easy to use set of building blocks for low cost sensing and control from your PC.  
 Using the Universal Serial Bus (USB) as the basis for all Phidgets, the complexity is managed behind 
 this easy to use and robust Application Program Interface (API) library.
 
 The library was written originally for Windows, but has been ported to MacOS and Linux.  
 Although the library is written in C, the functions can be called from a number of languages including C#, C/C++, Java, Python, Visual Basic, and many more.  
 The full library source is not available for all platforms - however, the Linux distribution contains all platform independent code.
 
 See the <a class="el" href="modules.html">Modules</a> section for the API documentation.
 
 See \ref phidcommon for calls common to all phidgets.

 Refer to the User Guide for your device and the 
 <a class="el" href="http://www.phidgets.com/docs/General_Phidget_Programming" target="_blank">General Phidget Programming</a> page for more detailed, language inspecific API documentation and examples. 
 You can download a set of C/C++ example code from the Quick Downloads section of the <a class="el" href="http://www.phidgets.com/docs/Language_-_C/C%2B%2B#Quick_Downloads" target="_blank"> C/C++ Language Page</a>.

 \section general_sec General Usage
Phidgets in C are accessed/controlled via handles. Every Phidget has it's own Handle, as well as a generic Phidget handle. These handles are simply pointers
from the user's perspective - internally, they point to structures. Almost every function in the API takes a Phidget handle as the first argument.

phidget21.h contains the API calls for all Phidgets. It should be included in any programs which need to access the Phidget C Library from C/C++.

Note that this file is not distributed in the source, this file is generated from the source using the C preproccessor. It is installed with the libraries.

The first step is to create the handle. Every phidget has it's own _create function. This creates space for the structure internally and gives you a 
valid Handle for passing to all other Phidget functions.

The next step is to register event handlers. These need to be registered now, because they only throw events in the future, 
and if they are registered later, you will miss events. Every Phidget event takes a Phidget handle, a function callback pointer and
a user defined pointer, which is passed back from within the callback.

The next step is to open the Phidget. This is done with either \ref CPhidget_open, \ref CPhidget_openRemote or \ref CPhidget_openRemoteIP, depending on whether you want
to open the Phidget locally or over the network. Open returns right away (is asynchronous) so you have to either wait for the attach event, 
or call \ref CPhidget_waitForAttachment before using the handle further.

The next step it to actually use the Phidget. Once attached, all functions can be called. Device specific handles should be cast to \ref CPhidgetHandle for use
with the common API functions.

Once finished, you should call first \ref CPhidget_close and then \ref CPhidget_delete to clean up the connection to the Phidget and the Handle before exiting.

\section return_sec Return Codes
Every Phidget function returns an int. A return code of 0 means success, anything else means failure. The failure codes are documented here: \ref phidconst.

When a value is unknown, in addition to returning an error code, the pointer will be set to one of the unknonwn values documented here: \ref phidconst.
 *
 */

/** \defgroup phidgets Specific Phidgets
 */

#ifdef _WINDOWS
	#ifdef PHIDGET21_EXPORTS
		#ifndef PHIDGET21_API
		#define PHIDGET21_API
		#endif
	#else
		#ifndef PHIDGET21_API
		#define PHIDGET21_API __declspec(dllimport)
		#endif
	#endif
	#ifndef CCONV
	#define CCONV __stdcall
	#endif
	#ifndef DEPRECATE_WIN
	#define DEPRECATE_WIN(depmsg) __declspec(deprecated( depmsg ))
	#endif
	#ifndef DEPRECATE_GCC
	#define DEPRECATE_GCC
	#endif

#else
	#ifndef PHIDGET21_API
	#define PHIDGET21_API
	#endif
	#ifndef CCONV
	#define CCONV
	#endif
	#ifndef DEPRECATE_GCC
	#define DEPRECATE_GCC __attribute__ ((deprecated))
	#endif
	#ifndef DEPRECATE_WIN
	#define DEPRECATE_WIN(depmsg)
	#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "cphidget.h"
#include "cphidgetdictionary.h"
#include "csocket.h"
#include "clog.h"
#include "cphidgetaccelerometer.h"
#include "cphidgetadvancedservo.h"
#include "cphidgetanalog.h"
#include "cphidgetbridge.h"
#include "cphidgetencoder.h"
#include "cphidgetfrequencycounter.h"
#include "cphidgetgps.h"
#include "cphidgetinterfacekit.h"
#include "cphidgetir.h"
#include "cphidgetmanager.h"
#include "cphidgetled.h"
#include "cphidgetmotorcontrol.h"
#include "cphidgetphsensor.h"
#include "cphidgetrfid.h"
#include "cphidgetservo.h"
#include "cphidgetspatial.h"
#include "cphidgetstepper.h"
#include "cphidgettemperaturesensor.h"
#include "cphidgettextlcd.h"
#include "cphidgettextled.h"
#include "cphidgetweightsensor.h"
//Only include in the debug header
#if defined(DEBUG) || !defined(EXTERNALPROTO)
	#include "cphidgetgeneric.h"
	#include "cphidgetsbc.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
