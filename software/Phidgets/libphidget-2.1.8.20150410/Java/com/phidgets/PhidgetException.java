/*
 * This file is part of libphidget21
 *
 * Copyright © 2006-2015 Phidgets Inc <patrick@phidgets.com>
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

package com.phidgets;

/**
 * This class represents Phidget related exceptions. All Phidget exceptions originate in the phidget21 C library.
 * These exceptions can be thrown by most function in the library and cover such things as trying to access
 * a Phidget before opening it, or before it is attached and ready to use, out of bounds Index and data values, 
 * trying to read data that isn't available, and other less common problems. EPHIDGET_(error) errors are thrown
 * by any API calls, EEPHIDGET_(error) errors are returned via the {@link com.phidgets.event.ErrorEvent Error} event.
 */
public class PhidgetException extends java.lang.Exception {
	private int errno;
	private String description;

	/**
	 * Constructor which takes in an error number and description. 
	 * These exceptions originate from and are filled in by the phidget 21 C library.
	 */
	public PhidgetException(int errno, String description) {
		this.description = description;
		this.errno = errno;
	}

	/**
	 * Phidget not found exception. "A Phidget matching the type and or serial number could not be found."
	 * <P>
	 * This exception is not currently used externally.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_NOTFOUND = 1;
	/**
	 * No memory exception. "Memory could not be allocated."
	 * <P>
	 * This exception is thrown when a memory allocation (malloc) call fails in the c library.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_NOMEMORY = 2;
	/**
	 * Unexpected exception. "Unexpected Error.  Contact Phidgets Inc. for support."
	 * <P>
	 * This exception is thrown when something unexpected happens (more enexpected then another exception). This generally 
	 * points to a bug or bad code in the C library, and hopefully won't even be seen.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_UNEXPECTED = 3;
	/**
	 * Invalid argument exception. "Invalid argument passed to function."
	 * <P>
	 * This exception is thrown whenever a function receives an unexpected null pointer, or a value that is out of range. ie setting a motor's speed to 101 when the maximum is 100.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_INVALIDARG = 4;
	/**
	 * Phidget not attached exception. "Phidget not physically attached."
	 * <P>
	 * This exception is thrown when a method is called on a device that is not attached, and the method requires the device to be attached. ie trying to read the serial number, or
	 * the state of an ouput.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_NOTATTACHED = 5;
	/**
	 * Interrupted exception. "Read/Write operation was interrupted."
	 * <P>
	 * This exception is not currently used externally.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_INTERRUPTED = 6;
	/**
	 * Invalid error exception. "The Error Code is not defined."
	 * <P>
	 * This exception is thrown when trying to get the string description of an undefined error code. This should not be seen in Java.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_INVALID = 7;
	/**
	 * Network exception. "Network Error."
	 * <P>
	 * This exception is thrown when a network related error occurs. The {@link #EEPHIDGET_NETWORK EEPHIDGET_NETWORK} code is more often used (in error events) 
	 * because most network exceptions come through asynchronously.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_NETWORK = 8;
	/**
	 * Value unknown exception. "Value is Unknown (State not yet received from device, or not yet set by user)."
	 * <P>
	 * This exception is thrown when a device that is set to unknow is read. ie trying to read the position of a servo before setting it's position.
	 * <p>
	 * Every effort is made in the library to fill in as much of a device's state before the attach event gets thrown, however, many there are some states
	 * that cannot be filled in automatically. ie older interface kits do not return their output states, and so these will be unknown until they are set.
	 * <p>
	 * This is a quite common exception for some devices, and so should always be caught
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_UNKNOWNVAL = 9;
	/**
	 * Authorization exception. "Authorization Failed."
	 * <P>
	 * This exception has been replaced by {@link #EEPHIDGET_BADPASSWORD EEPHIDGET_BADPASSWORD}.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_BADPASSWORD = 10;
	/**
	 * Unsupported exception. "Not Supported."
	 * <P>
	 * This exception is thrown when a method is called that is not supported, either by that device, or by the system. ie calling setRatiometric on an interfaceKit that does not have sensors.
	 * <p>
	 * This is also used for methods that are not yet complete, ie setLabel on Windows.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_UNSUPPORTED = 11;
	/**
	 * Duplicate request exception. "Duplicated request."
	 * <P>
	 * This exception is thrown when open is called twice on a device, without calling close in between. The second call to open is ignored.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_DUPLICATE = 12;
	/**
	 * Timeout exception. "Given timeout has been exceeded."
	 * <P>
	 * This exception is thrown by {@link com.phidgets.Phidget#waitForAttachment(int) waitForAttachment(int)} if the provided timeout expires before an attach happens.
	 * This may also be thrown by a device set request, if the set times out - though this should not happen, and would generally mean a problem with the device.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_TIMEOUT = 13;
	/**
	 * Out of bounds exception. "Index out of Bounds."
	 * <P>
	 * This exception is thrown anytime an indexed set or get method is called with an out of bounds index.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_OUTOFBOUNDS = 14;
	/**
	 * Event exception. "A non-null error code was returned from an event handler."
	 * <P>
	 * This exception is not currently used.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_EVENT = 15;
	/**
	 * Network not connected exception. "A connection to the server does not exist."
	 * <P>
	 * This exception is thrown when a network specific method is called on a device that was opened remotely, but there is no connection to a server. ie getServerID.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_NETWORK_NOTCONNECTED = 16;
	/**
	 * Wrong device exception. "Function is not applicable for this device."
	 * <P>
	 * This exception is thrown when a method from device is called by another device. ie casting an InterfaceKit to a Servo and calling setPosition.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_WRONGDEVICE = 17;
	/**
	 * Phidget closed exception. "Phidget handle was closed."
	 * <P>
	 * This exception is thrown when {@link com.phidgets.Phidget#waitForAttachment waitForAttachment} is called on a Phidget that has not been opened, or was closed.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_CLOSED = 18;
	/**
	 * Version mismatch exception. "Webservice and Client protocol versions don't match. Update to newest release."
	 * <P>
	 * This exception has been replaced by {@link #EEPHIDGET_BADVERSION EEPHIDGET_BADVERSION}.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EPHIDGET_BADVERSION = 19;


	//Error Event codes

	/**
	 * Network exception. "Network Error."
	 * <P>
	 * This exception is sent via the {@link com.phidgets.event.ErrorEvent Error} event. It will be accompanied by a specific Description of the network problem.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EEPHIDGET_NETWORK = 0x8001;
	/**
	 * Authorization exception. "Authorization Failed."
	 * <P>
	 * This exception is sent via the {@link com.phidgets.event.ErrorEvent Error} event. It means that a connection could not be authenticated because of a passwrod missmatch.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EEPHIDGET_BADPASSWORD = 0x8002;
	/**
	 * Version mismatch exception. "Webservice and Client protocol versions don't match. Update to newest release."
	 * <P>
	 * This exception is sent via the {@link com.phidgets.event.ErrorEvent Error} event when trying to connect to a remote phidget and the webservice version does not match your client library version.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EEPHIDGET_BADVERSION = 0x8003;
	/**
	 * Overrun exception. "A sampling overrun happend in firmware."
	 * <P>
	 * This exception sent via in the {@link com.phidgets.event.ErrorEvent Error} event. It is accompanied by a specific {@link #getDescription description}.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EEPHIDGET_OVERRUN = 0x9002;
	/**
	 * Packet lost exception. "One or more packets were lost."
	 * <P>
	 * This exception sent via in the {@link com.phidgets.event.ErrorEvent Error} event. It is accompanied by a specific {@link #getDescription description}.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EEPHIDGET_PACKETLOST = 0x9003;
	/**
	 * Wraparound exception. "A variable has wrapped around."
	 * <P>
	 * This exception sent via in the {@link com.phidgets.event.ErrorEvent Error} event. It is accompanied by a specific {@link #getDescription description}.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EEPHIDGET_WRAP = 0x9004;
	/**
	 * Overtemperature exception. "Overtemperature condition detected."
	 * <P>
	 * This exception sent via in the {@link com.phidgets.event.ErrorEvent Error} event. It is accompanied by a specific {@link #getDescription description}.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EEPHIDGET_OVERTEMP = 0x9005;
	/**
	 * Overcurrent exception. "Overcurrent condition detected."
	 * <P>
	 * This exception sent via in the {@link com.phidgets.event.ErrorEvent Error} event. It is accompanied by a specific {@link #getDescription description}.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EEPHIDGET_OVERCURRENT = 0x9006;
	/**
	 * Out of range exception. "Out of range condition detected."
	 * <P>
	 * This exception sent via in the {@link com.phidgets.event.ErrorEvent Error} event. It is accompanied by a specific {@link #getDescription description}.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EEPHIDGET_OUTOFRANGE = 0x9007;
	/**
	 * Bad power exception. "Power supply problem detected."
	 * <P>
	 * This exception sent via in the {@link com.phidgets.event.ErrorEvent Error} event. It is accompanied by a specific {@link #getDescription description}.
	 * <p>
	 * This is returned by {@link #getErrorNumber getErrorNumber}
	 */
	public static final int EEPHIDGET_BADPOWER = 0x9008;

	/**
	 * Returns the error number of this exception. This error number will match one of the EPHIDGET_(error) or EEPHIDGET_(error) constants defined in this class.
	 * These numbers are defined in the phidget21 c library and are consistent across all APIs.
	 * <p>
	 * There are 19 error codes defined, as follows:
	 * <br>
	 * {@link #EPHIDGET_NOTFOUND EPHIDGET_NOTFOUND},
	 * {@link #EPHIDGET_NOMEMORY EPHIDGET_NOMEMORY},
	 * {@link #EPHIDGET_UNEXPECTED EPHIDGET_UNEXPECTED},
	 * {@link #EPHIDGET_INVALIDARG EPHIDGET_INVALIDARG},
	 * {@link #EPHIDGET_NOTATTACHED EPHIDGET_NOTATTACHED},
	 * {@link #EPHIDGET_INTERRUPTED EPHIDGET_INTERRUPTED},
	 * {@link #EPHIDGET_INVALID EPHIDGET_INVALID},
	 * {@link #EPHIDGET_NETWORK EPHIDGET_NETWORK},
	 * {@link #EPHIDGET_UNKNOWNVAL EPHIDGET_UNKNOWNVAL},
	 * {@link #EPHIDGET_BADPASSWORD EPHIDGET_BADPASSWORD},
	 * {@link #EPHIDGET_UNSUPPORTED EPHIDGET_UNSUPPORTED},
	 * {@link #EPHIDGET_DUPLICATE EPHIDGET_DUPLICATE},
	 * {@link #EPHIDGET_TIMEOUT EPHIDGET_TIMEOUT},
	 * {@link #EPHIDGET_OUTOFBOUNDS EPHIDGET_OUTOFBOUNDS},
	 * {@link #EPHIDGET_EVENT EPHIDGET_EVENT},
	 * {@link #EPHIDGET_NETWORK_NOTCONNECTED EPHIDGET_NETWORK_NOTCONNECTED},
	 * {@link #EPHIDGET_WRONGDEVICE	EPHIDGET_WRONGDEVICE},
	 * {@link #EPHIDGET_CLOSED EPHIDGET_CLOSED} and
	 * {@link #EPHIDGET_BADVERSION EPHIDGET_BADVERSION}</pre>
	 * <p>
	 * There are also 9 error event codes defined, as follows:
	 * <br>
	 * {@link #EEPHIDGET_NETWORK EEPHIDGET_NETWORK},
	 * {@link #EEPHIDGET_BADPASSWORD EEPHIDGET_BADPASSWORD},
	 * {@link #EEPHIDGET_BADVERSION EEPHIDGET_BADVERSION},
	 * {@link #EEPHIDGET_OVERRUN EEPHIDGET_OVERRUN},
	 * {@link #EEPHIDGET_PACKETLOST EEPHIDGET_PACKETLOST},
	 * {@link #EEPHIDGET_WRAP EEPHIDGET_WRAP},
	 * {@link #EEPHIDGET_OVERTEMP EEPHIDGET_OVERTEMP},
	 * {@link #EEPHIDGET_OVERCURRENT EEPHIDGET_OVERCURRENT},
	 * {@link #EEPHIDGET_OUTOFRANGE EEPHIDGET_OUTOFRANGE}</pre>
	 * 
	 * @return The error code
	 */
	public int getErrorNumber() {
		return errno;
	}

	/**
	 * Returns a description of this exception. This is an english phrase that describes the exception that
	 * occured. These strings originate in the base phidget21 C library, and should help to diagnose problems.
	 * 
	 * @return The error description
	 */
	public String getDescription() {
		return description;
	}

	/**
	 * Returns a string containing the error number and exception description.
	 * 
	 * @return A string describing the error
	 */
	public String toString() {
		return "PhidgetException " + errno + " (" + description + ")";
	}
}
