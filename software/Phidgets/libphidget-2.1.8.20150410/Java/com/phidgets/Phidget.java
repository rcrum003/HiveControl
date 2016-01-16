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

import com.phidgets.event.*;

import java.util.Iterator;
import java.util.LinkedList;

/**
 * This is the base class from which all Phidget device classes derive.
 * Don't create phidget devices directly using this class. Use the specific class for the
 * device that you wish to access.
 * 
 * @author Phidgets Inc.
 * @version 2.1.7
 */
public class Phidget
{
	static
	{
		try
		{
		
			System.loadLibrary("phidget21");

		}
		catch(UnsatisfiedLinkError ex)
		{ 
			String os = System.getProperty("os.name");
			if(os.startsWith("Linux"))
			{
				throw new ExceptionInInitializerError(ex.getMessage()
				+"\nCould not locate the Phidget C library (libphidget21.so)."
				+"\nMake sure it is installed, and add it's path to LD_LIBRARY_PATH.");
			}
			else if(os.startsWith("Windows"))
			{
				throw new ExceptionInInitializerError(ex.getMessage()
				+"\nCould not locate the Phidget C library."
				+"\nThe Windows Phidget21 MSI must be installed.");
			}
			else if(os.startsWith("Mac"))
			{
				throw new ExceptionInInitializerError(ex.getMessage()
				+"\nCould not locate the Phidget C library."
				+"\nThe Mac Phidget21 DMG must be installed.");
			}
			else
			{
				throw new ExceptionInInitializerError(ex.getMessage()
				+"\nCould not locate the Phidget C library.");
			}
		}
		

	
	}

	private long handle = 0;
	boolean managerPhidget = false;

	/**
	 * Class constructor specifying a handle. This constructor is to be used only by subclasses, as the Phidget calss should never need to
	 * be instatiated directly by the user.
	 * 
	 * @param handle A native Handle points to the underlying phidget structure in the base phidget21 C library.
	 */
	public Phidget(long handle) {
		this.handle = handle;
	}

	private final long getHandle() {
		return handle;
	}

	/**
	 * Returns the library version. This is the library version of the underlying phidget21 C library and not the version of the 
	 * JNI wrapper implementation.
	 * <p>
	 * The version is retured at a string which contains the version number and build date.
	 * 
	 * @return Library Version
	 */
	public final static native String getLibraryVersion();

	/**
	 * Critial error messages. This is used with {@link #enableLogging enableLogging} and {@link #log log}
	 * <p>
	 * This is the lowest logging level. Errors at this level are generally non-recoverable and indicate either hardware problems,
	 * library bugs, or other serious issues.
	 * <P>
	 * If logging is enabled at this level, only {@link #PHIDGET_LOG_CRITICAL PHIDGET_LOG_CRITICAL} messages are logged.
	 */
	public static final int PHIDGET_LOG_CRITICAL = 1;
	/**
	 * Non-critical error messages. This is used with {@link #enableLogging enableLogging} and {@link #log log}
	 * <P>
	 * Errors at this level are generally automatically recoverable, but may help to track down issues.
	 * <p>
	 * If logging is enabled at this level, {@link #PHIDGET_LOG_ERROR PHIDGET_LOG_ERROR} as well as 
	 * {@link #PHIDGET_LOG_CRITICAL PHIDGET_LOG_CRITICAL} messages are logged.
	 */
	public static final int PHIDGET_LOG_ERROR = 2;
	/**
	 * Warning messages. This is used with {@link #enableLogging enableLogging} and {@link #log log}
	 * <P>
	 * Warnings are used to log behaviour that is not neccessarily in error, but is nevertheless odd or unexpected.
	 * <p>
	 * If logging is enabled at this level, {@link #PHIDGET_LOG_WARNING PHIDGET_LOG_WARNING} as well as 
	 * {@link #PHIDGET_LOG_ERROR PHIDGET_LOG_ERROR} and {@link #PHIDGET_LOG_CRITICAL PHIDGET_LOG_CRITICAL} messages are logged.
	 */
	public static final int PHIDGET_LOG_WARNING = 3;
	/**
	 * Debug messages. This is used with {@link #enableLogging enableLogging} and {@link #log log}
	 * <P>
	 * Debug messages are generally used for debugging at Phdigets Inc.
	 * <p>
	 * Note: {@link #PHIDGET_LOG_DEBUG PHIDGET_LOG_DEBUG} messages are only logged in the debug version of the library, regardless of logging level.
	 * Thus, these logs should never be seen outside of Phidgets Inc.
	 * <p>
	 * If logging is enabled at this level, {@link #PHIDGET_LOG_DEBUG PHIDGET_LOG_DEBUG} as well as 
	 * {@link #PHIDGET_LOG_WARNING PHIDGET_LOG_WARNING}, {@link #PHIDGET_LOG_ERROR PHIDGET_LOG_ERROR} 
	 * and {@link #PHIDGET_LOG_CRITICAL PHIDGET_LOG_CRITICAL} messages are logged.
	 */
	public static final int PHIDGET_LOG_DEBUG = 4;
	/**
	 * Informational messages. This is used with {@link #enableLogging enableLogging} and {@link #log log}
	 * <P>
	 * Informational messages track key happenings within phidget21 - mostly to do with threads starting and shutting down, and the
	 * internals of the USB code.
	 * <p>
	 * If logging is enabled at this level, {@link #PHIDGET_LOG_INFO PHIDGET_LOG_INFO} as well as 
	 * {@link #PHIDGET_LOG_DEBUG PHIDGET_LOG_DEBUG}, {@link #PHIDGET_LOG_WARNING PHIDGET_LOG_WARNING}, {@link #PHIDGET_LOG_ERROR PHIDGET_LOG_ERROR} 
	 * and {@link #PHIDGET_LOG_CRITICAL PHIDGET_LOG_CRITICAL} messages are logged.
	 */
	public static final int PHIDGET_LOG_INFO = 5;
	/**
	 * Verbose messages. This is used with {@link #enableLogging enableLogging} and {@link #log log}
	 * <P>
	 * This is the highest logging level. Verbose messages are informational messages that are expected to happen so
	 * frequently that they tend to drown out other log messages.
	 * <p>
	 * If logging is enabled at this level, {@link #PHIDGET_LOG_VERBOSE PHIDGET_LOG_VERBOSE} as well as 
	 * {@link #PHIDGET_LOG_INFO PHIDGET_LOG_INFO}, {@link #PHIDGET_LOG_DEBUG PHIDGET_LOG_DEBUG}, 
	 * {@link #PHIDGET_LOG_WARNING PHIDGET_LOG_WARNING}, {@link #PHIDGET_LOG_ERROR PHIDGET_LOG_ERROR} 
	 * and {@link #PHIDGET_LOG_CRITICAL PHIDGET_LOG_CRITICAL} messages are logged.
	 */
	public static final int PHIDGET_LOG_VERBOSE = 6;

	private final static native void nativeEnableLogging(int level, String file);

	/**
	 * Turns on logging in the native C Library. This is mostly usefull for debugging purposes - when an issue needs to be resolved by
	 * Phidgets Inc. The output is mostly low-level library information, that won't be usefull for most users.
	 * <p>
	 * Logging may be usefull for users trying to debug their own problems, as logs can be inserted by the user using {@link #log log}.
	 * <p>
	 * The level can be one of: {@link #PHIDGET_LOG_VERBOSE PHIDGET_LOG_VERBOSE},
	 * {@link #PHIDGET_LOG_INFO PHIDGET_LOG_INFO}, {@link #PHIDGET_LOG_DEBUG PHIDGET_LOG_DEBUG}, 
	 * {@link #PHIDGET_LOG_WARNING PHIDGET_LOG_WARNING}, {@link #PHIDGET_LOG_ERROR PHIDGET_LOG_ERROR} 
	 * or {@link #PHIDGET_LOG_CRITICAL PHIDGET_LOG_CRITICAL}
	 * 
	 * @param level highest level of logging that will be output
	 * @param file file to output log to. specify 'null' to output to the console
	 */
	public final static void enableLogging(int level, String file)
	{
		nativeEnableLogging(level, file);
	}

	/**
	 * Turns off logging in the native C Library. This only needs to be called if enableLogging was called to turn logging on.
	 * This will turn logging back off.
	 */
	public final static native void disableLogging();

	private final static native void nativeLog(int level, String id, String log);

	/**
	 * Adds a log entry into the phidget log. This log is enabled by calling {@link #enableLogging(int level, String file) enableLogging}
	 * and this allows the entry of user logs in amongst the phidget library logs.
	 * <p>
	 * The level can be one of: {@link #PHIDGET_LOG_VERBOSE PHIDGET_LOG_VERBOSE},
	 * {@link #PHIDGET_LOG_INFO PHIDGET_LOG_INFO}, {@link #PHIDGET_LOG_DEBUG PHIDGET_LOG_DEBUG}, 
	 * {@link #PHIDGET_LOG_WARNING PHIDGET_LOG_WARNING}, {@link #PHIDGET_LOG_ERROR PHIDGET_LOG_ERROR} 
	 * or {@link #PHIDGET_LOG_CRITICAL PHIDGET_LOG_CRITICAL}
	 * <p>
	 * Note: {@link #PHIDGET_LOG_DEBUG PHIDGET_LOG_DEBUG} should not be used, as these logs are only printed when using the debug library,
	 * which is not generally available.
	 * 
	 * @param level level to enter the log at
	 * @param id an arbitrary identifier for this log. This can be NULL. The C library uses this field for source filename and line number
	 * @param log the message to log
	 */
	public final static void log(int level, String id, String log)
	{
		nativeLog(level, id, log);
	}
	
	public static final int PHIDID_NOTHING							= 1;

	/* These are all current devices */
	public static final int PHIDID_ACCELEROMETER_3AXIS				= 0x07E;
	public static final int PHIDID_ADVANCEDSERVO_1MOTOR				= 0x082;
	public static final int PHIDID_ADVANCEDSERVO_8MOTOR				= 0x03A;
	public static final int PHIDID_ANALOG_4OUTPUT					= 0x037;
	public static final int PHIDID_BIPOLAR_STEPPER_1MOTOR			= 0x07B;
	public static final int PHIDID_BRIDGE_4INPUT	                = 0x03B;
	public static final int PHIDID_ENCODER_1ENCODER_1INPUT			= 0x04B;
	public static final int PHIDID_ENCODER_HS_1ENCODER				= 0x080;
	public static final int PHIDID_ENCODER_HS_4ENCODER_4INPUT		= 0x04F;
	public static final int PHIDID_FREQUENCYCOUNTER_2INPUT			= 0x035;
	public static final int PHIDID_GPS								= 0x079;
	public static final int PHIDID_INTERFACEKIT_0_0_4				= 0x040;
	public static final int PHIDID_INTERFACEKIT_0_0_8				= 0x081;
	public static final int PHIDID_INTERFACEKIT_0_16_16				= 0x044;
	public static final int PHIDID_INTERFACEKIT_2_2_2				= 0x036;
	public static final int PHIDID_INTERFACEKIT_8_8_8				= 0x045;
	public static final int PHIDID_INTERFACEKIT_8_8_8_w_LCD			= 0x07D;
	public static final int PHIDID_IR								= 0x04D;
	public static final int PHIDID_LED_64_ADV						= 0x04C;
	public static final int PHIDID_LINEAR_TOUCH						= 0x076;
	public static final int PHIDID_MOTORCONTROL_1MOTOR				= 0x03E;
	public static final int PHIDID_MOTORCONTROL_HC_2MOTOR			= 0x059;
	public static final int PHIDID_RFID_2OUTPUT						= 0x031;
	public static final int PHIDID_RFID_2OUTPUT_READ_WRITE			= 0x034;
	public static final int PHIDID_ROTARY_TOUCH						= 0x077;
	public static final int PHIDID_SPATIAL_ACCEL_3AXIS				= 0x07F;
	public static final int PHIDID_SPATIAL_ACCEL_GYRO_COMPASS		= 0x033;
	public static final int PHIDID_TEMPERATURESENSOR				= 0x070;
	public static final int PHIDID_TEMPERATURESENSOR_4				= 0x032;
	public static final int PHIDID_TEMPERATURESENSOR_IR				= 0x03C;
	public static final int PHIDID_TEXTLCD_2x20_w_8_8_8				= 0x17D;
	public static final int PHIDID_TEXTLCD_ADAPTER                  = 0x03D;
	public static final int PHIDID_UNIPOLAR_STEPPER_4MOTOR			= 0x07A;
	
	/* These are all past devices (no longer sold) */
	public static final int PHIDID_ACCELEROMETER_2AXIS				= 0x071;
	public static final int PHIDID_INTERFACEKIT_0_8_8_w_LCD			= 0x053;
	public static final int PHIDID_INTERFACEKIT_4_8_8				= 4;
	public static final int PHIDID_LED_64							= 0x04A;
	public static final int PHIDID_MOTORCONTROL_LV_2MOTOR_4INPUT	= 0x058;
	public static final int PHIDID_PHSENSOR							= 0x074;
	public static final int PHIDID_RFID								= 0x030;
	public static final int PHIDID_SERVO_1MOTOR						= 0x039;
	public static final int PHIDID_SERVO_1MOTOR_OLD					= 2;
	public static final int PHIDID_SERVO_4MOTOR						= 0x038;
	public static final int PHIDID_SERVO_4MOTOR_OLD					= 3;
	public static final int PHIDID_TEXTLCD_2x20						= 0x052;
	public static final int PHIDID_TEXTLCD_2x20_w_0_8_8				= 0x153;
	public static final int PHIDID_TEXTLED_1x8						= 0x049;
	public static final int PHIDID_TEXTLED_4x8						= 0x048;
	public static final int PHIDID_WEIGHTSENSOR						= 0x072;


	/**
	 * Returns the Device ID of this Phidget. This is one of the PHIDID_(...) constants defined above. This uniquely identifies a specific type of Phidget.
	 * 
	 * @return Device ID
	 * @throws PhidgetException If this Phidget is not opened and attached. See {@link #open(int) open} for information on determining if a device is attached.
	 */
	public final native int getDeviceID() throws PhidgetException;

	public static final int PHIDCLASS_NOTHING = 1;
	public static final int PHIDCLASS_ACCELEROMETER = 2;
	public static final int PHIDCLASS_ADVANCEDSERVO = 3;
	public static final int PHIDCLASS_ANALOG = 22;
	public static final int PHIDCLASS_BRIDGE = 23;
	public static final int PHIDCLASS_ENCODER = 4;
	public static final int PHIDCLASS_FREQUENCYCOUNTER = 21;
	public static final int PHIDCLASS_GPS = 5;		
	public static final int PHIDCLASS_INTERFACEKIT = 7;
	public static final int PHIDCLASS_LED = 8;
	public static final int PHIDCLASS_MOTORCONTROL = 9;
	public static final int PHIDCLASS_PHSENSOR = 10;
	public static final int PHIDCLASS_RFID = 11;
	public static final int PHIDCLASS_SERVO = 12;
	public static final int PHIDCLASS_STEPPER = 13;
	public static final int PHIDCLASS_TEMPERATURESENSOR = 14;
	public static final int PHIDCLASS_TEXTLCD = 15;
	public static final int PHIDCLASS_TEXTLED = 16;
	public static final int PHIDCLASS_WEIGHTSENSOR = 17;

	/**
	 * Returns the Device Class of this Phidget. This is one of the PHIDCLASS_(...) constants defined above. This represents a class of Phidgets - ie Interface Kit.
	 * 
	 * @return Device ID
	 * @throws PhidgetException If this Phidget is not opened and attached. See {@link #open(int) open} for information on determining if a device is attached.
	 */
	public final native int getDeviceClass() throws PhidgetException;

	/**
	 * Returns the unique serial number of this Phidget. This number is set during manufacturing, and is unique across all Phidgets.
	 * This number can be used in calls to {@link #open(int) open} to specify this specific Phidget to be opened.
	 * 
	 * @return Serial Number
	 * @throws PhidgetException If this Phidget is not opened and attached. See {@link #open(int) open} for information on determining if a device is attached.
	 */
	public final native int getSerialNumber() throws PhidgetException;

	/**
	 * Returns the device version of this Phidget. This number is simply a way of distinguishing between different revisions
	 * of a specific type of Phidget, and is only really of use if you need to troubleshoot device problems with Phidgets Inc.
	 * 
	 * @return Version
	 * @throws PhidgetException If this Phidget is not opened and attached. See {@link #open(int) open} for information on determining if a device is attached.
	 */
	public final native int getDeviceVersion() throws PhidgetException;

	/**
	 * Return the device type of this Phidget. This is a string that describes the device as a class of devices. For example,
	 * all PhidgetInterfaceKit Phidgets will returns the String "PhidgetInterfaceKit".
	 * <p>
	 * This function is mostly usefull if you are using the {@link com.phidgets.Manager Phidget Manager} because it's attach and detach handlers
	 * return {link com.phidgets.phidget phidget} objects, and you can use this method to determine their proper type.
	 * 
	 * @return Device Type
	 * @throws PhidgetException If this Phidget is not opened and attached. See {@link #open(int) open} for information on determining if a device is attached.
	 */
	public final native String getDeviceType() throws PhidgetException;

	/**
	 * Return the name of this Phidget. This is a string that describes the device. For example, a PhidgetInterfaceKit could be described
	 * as "Phidget InterfaceKit 8/8/8", or "Phidget InterfaceKit 0/0/4", among others, depending on the specific device.
	 * <p>
	 * This lets you determine the specific type of a Phidget, within the broader classes of Phidgets, such as PhidgetInterfaceKit, or PhidgetServo
	 * 
	 * @return Name
	 * @throws PhidgetException If this Phidget is not opened and attached. See {@link #open(int) open} for information on determining if a device is attached.
	 */
	public final native String getDeviceName() throws PhidgetException;

	/**
	 * Returns the attached status of this Phidget. This method returns True or False, depending on whether the Phidget is phisically plugged
	 * into the computer, initialized, and ready to use - or not. If a Phidget is not attached, many functions calls will fail with a PhidgetException,
	 * so either checking this function, or using the Attach and Detach events, is recommended, if a device is likely to be attached or
	 * detached during use.
	 * 
	 * @return Status
	 * @throws PhidgetException If this Phidget is not opened.
	 */
	public final native boolean isAttached() throws PhidgetException;

	/**
	 * Returns the network attached status for remotely opened Phidgets. This method returns True or False, depending on whether a connection to the
	 * Phidget WebService is open - or not. If this is false for a remote Phidget then the connection is not active - either because a connection
	 * has not yet been established, or because the connection was terminated.
	 * 
	 * @return Status
	 * @throws PhidgetException If this Phidget is not opened.
	 */
	public final native boolean isAttachedToServer() throws PhidgetException;

	/**
	 * Returns the label associated with this Phidget. This label is a String - up to ten digits - that is stored in the Flash memory
	 * of newer Phidgets. This label can be set programatically (see {@link #setDeviceLabel(String) setDeviceLabel}), and is non-volatile - so it is remembered
	 * even if the Phidget is unplugged.
	 * 
	 * @return Label
	 * @throws PhidgetException If this Phidget is not opened and attached, or if this Phidget does not support labels. 
	 * See {@link #open(int) open} for determining if a device is attached.
	 */
	public final native String getDeviceLabel() throws PhidgetException;

	/**
	 * Sets the Label associated with this Phidget. Labels are user defined Strings that persist in a Phidget's Flash. 
	 * They are only supported in the newer Phidgets. The maximum length is 10 characters.
	 * <p>
	 * Labels can not currently be set from Windows because of driver incompatibility. Labels can be set from MacOS, Linux and Windows CE.
	 * 
	 * @param label Label
	 * @throws PhidgetException If this Phidget is not opened and attached, or if this Phidget does not support labels, or if this is
	 * called from Windows. See {@link #open(int) open} for determining if a device is attached.
	 */
	public final native void setDeviceLabel(String label) throws PhidgetException;

	/**
	 * Returns the Address of a Phidget Webservice when this Phidget was opened as remote. This may be an IP Address or a hostname.
	 * 
	 * @return Server Address
	 * @throws PhidgetException if this Phidget was open opened as a remote Phidget.
	 */
	public final native String getServerAddress() throws PhidgetException;
	/**
	 * Returns the Server ID of a Phidget Webservice when this Phidget was opened as remote. This is an arbitrary server identifier,
	 * independant of IP address and Port.
	 * 
	 * @return Server ID
	 * @throws PhidgetException if this Phidget was open opened as a remote Phidget.
	 */
	public final native String getServerID() throws PhidgetException;
	/**
	 * Returns the Port of a Phidget Webservice when this Phidget was opened as remote.
	 * 
	 * @return Server Port
	 * @throws PhidgetException if this Phidget was open opened as a remote Phidget.
	 */
	public final native int getServerPort() throws PhidgetException;

	private final native void nativeClose() throws PhidgetException;
	private final native void nativeDelete() throws PhidgetException;
	private final native void nativeOpen(int serial) throws PhidgetException;
	private final native void nativeOpenRemote(int serial, String serverID, String pass) throws PhidgetException;
	private final native void nativeOpenRemoteIP(int serial, String ipAddress, int port, String pass) throws PhidgetException;
	private final native void nativeOpenLabel(String label) throws PhidgetException;
	private final native void nativeOpenLabelRemote(String label, String serverID, String pass) throws PhidgetException;
	private final native void nativeOpenLabelRemoteIP(String label, String ipAddress, int port, String pass) throws PhidgetException;

	/**
	 * Open this Phidget with a specific serial number. 
	 * <p>
	 * Open is pervasive. What this means is that you can call open on a device before it is plugged in, 
	 * and keep the device opened across device dis- and re-connections.
	 * <p>
	 * Open is Asynchronous. What this means is that open will return immediately --
	 * before the device being opened is actually available, so you need to use either the {@link #addAttachListener(AttachListener l) attach event} 
	 * or the {@link #waitForAttachment() waitForAttachment} method to determine if a device 
	 * is available before using it.
	 * <p>
	 * This version of open specifies a serial number - The serial number is a unique number assigned to each Phidget
	 * during production and can be used to uniquely identify specific phidgets.
	 * use {@link #openAny() openAny} to open a device without specifying the serial number.
	 * 
	 * @param serial Serial Number
	 */
	public final void open(int serial) throws PhidgetException {
		enableEvents(true);
		nativeOpen(serial);
	}

	/**
	 * Open this Phidget remotely using an IP Address, and a specific serial number. 	 
	 * <p>
	 * This version of open is network based.
	 * <p>
	 * Open is pervasive. What this means is that you can call open on a device before it is plugged in,
	 * and on a server before it is running, and keep the device opened across device, and server dis- and re-connections.
	 * <p>
	 * Open is Asynchronous. What this means is that open will return immediately -- 
	 * before the device being opened is actually available, so you need to use either the {@link #addAttachListener(AttachListener l) attach event} 
	 * or the {@link #waitForAttachment() waitForAttachment} method to determine if a device 
	 * is available before using it.
	 * <p>
	 * This version of open specifies a serial number - The serial number is a unique number assigned to each Phidget
	 * during production and can be used to uniquely identify specific phidgets.
	 * use {@link #openAny(String ipAddress, int port) openAny(String ipAddress, int port)} to open a device without specifying the serial number.
	 * 
	 * @param serial Serial Number
	 * @param ipAddress IP Address or hostname of the Phidget Webservice
	 * @param port Port of the Phidget Webservice
	 * @throws PhidgetException if the Phidget Webservice cannot be contacted
	 */
	public final void open(int serial, String ipAddress, int port) throws PhidgetException
	{
		open(serial, ipAddress, port, "");
	}
	/**
	 * Open this Phidget remotely and securely, using an IP Address, and a specific serial number. 
	 * <p>
	 * This method is the same as {@link #open(int, String, int) open(int serial, String ipAddress, int port)}, except that it specifies a password. 
	 * This password can be set as a parameter when starting the Phidget Webservice.
	 */
	public final void open(int serial, String ipAddress, int port, String password) throws PhidgetException
	{
		enableEvents(true);
		nativeOpenRemoteIP(serial, ipAddress, port, password);
	}

	/**
	 * Open this Phidget remotely using a Server ID, and a specific serial number. 	 
	 * <p>
	 * This version of open is network based.
	 * <p>
	 * Open is pervasive. What this means is that you can call open on a device before it is plugged in,
	 * and on a server before it is running, and keep the device opened across device, and server dis- and re-connections.
	 * <p>
	 * Open is Asynchronous. What this means is that open will return immediately -- 
	 * before the device being opened is actually available, so you need to use either the {@link #addAttachListener(AttachListener l) attach event} 
	 * or the {@link #waitForAttachment() waitForAttachment} method to determine if a device 
	 * is available before using it.
	 * <p>
	 * This version of open specifies a serial number - The serial number is a unique number assigned to each Phidget
	 * during production and can be used to uniquely identify specific phidgets.
	 * use {@link #openAny(String) openAny(String serverID)} to open a device without specifying the serial number.
	 * <p>
	 * ServerID can be set to null, if the ServerID does not matter. In this case, the specified Phidget (by serial number) will be opened
	 * whenever it is seen on the network, regardless of which server it appears on. This also applies when not specifying a serial number.
	 * <p>
	 * Note: All open methods that specify a ServerID rather then an IP Addresss and Port require that both the client and host sides of the
	 * connection be running an implementation of zeroconf:
	 * <ul><li>On Windows, this means installing Apple's Bonjour - available <a href="http://www.apple.com/support/downloads/bonjourforwindows.html">here</a>.</li>
	 * <li>On Linux, this means Avahi, which is usually either installed by default or available as a package install.</li>
	 * <li>On MacOSX, Bonjour is already integrated into the operationg system.</li></ul>
	 * 
	 * @param serial Serial Number
	 * @param serverID ServerID of the Phidget Webservice
	 */
	public final void open(int serial, String serverID) throws PhidgetException
	{
		open(serial, serverID, "");
	}

	/**
	 * Open this Phidget remotely and securely, using a Server ID, and a specific serial number. 
	 * <p>
	 * This method is the same as {@link #open(int, String) open(int serial, String serverID)}, except that it specifies a password. 
	 * This password can be set as a parameter when starting the Phidget Webservice.
	 */
	public final void open(int serial, String serverID, String password) throws PhidgetException
	{
		enableEvents(true);
		nativeOpenRemote(serial, serverID, password);
	}

	/**
	 * Open a this Phidget without a serial number. This method is the same as {@link #open(int) open}, except that it specifies no serial number. Therefore, the first available
	 * Phidget will be opened. If there are two Phidgets of the same type attached to the system, you should specify a serial number, as
	 * there is no guarantee which Phidget will be selected by the call to openAny().
	 */
	public final void openAny() throws PhidgetException {
		open(-1);
	}

	/**
	 * Open this Phidget remotely using an IP Address, without a serial number. 
	 * This method is the same as {@link #open(int, String, int) open(int serial, String ipAddress, int port)}, 
	 * except that it specifies no serial number. Therefore, the first available
	 * Phidget will be opened. If there are two Phidgets of the same type attached to the system, you should specify a serial number, as
	 * there is no guarantee which Phidget will be selected by the call to openAny().
	 */
	public final void openAny(String ipAddress, int port) throws PhidgetException
	{
		open(-1, ipAddress, port, "");
	}


	/**
	 * Open this Phidget remotely and securely, using an IP Address, without a serial number. 
	 * This method is the same as {@link #open(int, String, int, String) open(int serial, String ipAddress, int port, String password)}, 
	 * except that it specifies no serial number. Therefore, the first available
	 * Phidget will be opened. If there are two Phidgets of the same type attached to the system, you should specify a serial number, as
	 * there is no guarantee which Phidget will be selected by the call to openAny().
	 */
	public final void openAny(String ipAddress, int port, String password) throws PhidgetException
	{
		open(-1, ipAddress, port, password);
	}


	/**
	 * Open this Phidget remotely using a Server ID, without a serial number. 
	 * This method is the same as {@link #open(int, String) open(int serial, String serverID)}, 
	 * except that it specifies no serial number. Therefore, the first available
	 * Phidget will be opened. If there are two Phidgets of the same type attached to the system, you should specify a serial number, as
	 * there is no guarantee which Phidget will be selected by the call to openAny().
	 */
	public final void openAny(String serverID) throws PhidgetException
	{
		open(-1, serverID, "");
	}

	/**
	 * Open this Phidget remotely and securely, using a Server ID, without a serial number. 
	 * This method is the same as {@link #open(int, String, String) open(int serial, String serverID, String password)}, 
	 * except that it specifies no serial number. Therefore, the first available
	 * Phidget will be opened. If there are two Phidgets of the same type attached to the system, you should specify a serial number, as
	 * there is no guarantee which Phidget will be selected by the call to openAny().
	 */
	public final void openAny(String serverID, String password) throws PhidgetException
	{
		open(-1, serverID, password);
	}
	
	/**
	 * Open this Phidget with a specific label. 
	 * <p>
	 * Open is pervasive. What this means is that you can call open on a device before it is plugged in, 
	 * and keep the device opened across device dis- and re-connections.
	 * <p>
	 * Open is Asynchronous. What this means is that open will return immediately --
	 * before the device being opened is actually available, so you need to use either the {@link #addAttachListener(AttachListener l) attach event} 
	 * or the {@link #waitForAttachment() waitForAttachment} method to determine if a device 
	 * is available before using it.
	 * <p>
	 * This version of open specifies a label - The label can be set and changed by using the setLabel method (setLabel is unsupported on Windows).
	 * use {@link #openAny() openAny} to open a device without specifying the label.
	 * 
	 * @param label The Label
	 */
	public final void openLabel(String label) throws PhidgetException {
		enableEvents(true);
		nativeOpenLabel(label);
	}

	/**
	 * Open this Phidget remotely using an IP Address, and a specific label. 	 
	 * <p>
	 * This version of open is network based.
	 * <p>
	 * Open is pervasive. What this means is that you can call open on a device before it is plugged in,
	 * and on a server before it is running, and keep the device opened across device, and server dis- and re-connections.
	 * <p>
	 * Open is Asynchronous. What this means is that open will return immediately -- 
	 * before the device being opened is actually available, so you need to use either the {@link #addAttachListener(AttachListener l) attach event} 
	 * or the {@link #waitForAttachment() waitForAttachment} method to determine if a device 
	 * is available before using it.
	 * <p>
	 * This version of open specifies a label - The label can be set and changed by using the setLabel method (setLabel is unsupported on Windows).
	 * use {@link #openAny(String ipAddress, int port) openAny(String ipAddress, int port)} to open a device without specifying the label.
	 * 
	 * @param label The Label
	 * @param ipAddress IP Address or hostname of the Phidget Webservice
	 * @param port Port of the Phidget Webservice
	 * @throws PhidgetException if the Phidget Webservice cannot be contacted
	 */
	public final void openLabel(String label, String ipAddress, int port) throws PhidgetException
	{
		openLabel(label, ipAddress, port, "");
	}
	/**
	 * Open this Phidget remotely and securely, using an IP Address, and a specific label. 
	 * <p>
	 * This method is the same as {@link #openLabel(String, String, int) openLabel(String label, String ipAddress, int port)}, except that it specifies a password. 
	 * This password can be set as a parameter when starting the Phidget Webservice.
	 */
	public final void openLabel(String label, String ipAddress, int port, String password) throws PhidgetException
	{
		enableEvents(true);
		nativeOpenLabelRemoteIP(label, ipAddress, port, password);
	}

	/**
	 * Open this Phidget remotely using a Server ID, and a specific label. 	 
	 * <p>
	 * This version of open is network based.
	 * <p>
	 * Open is pervasive. What this means is that you can call open on a device before it is plugged in,
	 * and on a server before it is running, and keep the device opened across device, and server dis- and re-connections.
	 * <p>
	 * Open is Asynchronous. What this means is that open will return immediately -- 
	 * before the device being opened is actually available, so you need to use either the {@link #addAttachListener(AttachListener l) attach event} 
	 * or the {@link #waitForAttachment() waitForAttachment} method to determine if a device 
	 * is available before using it.
	 * <p>
	 * This version of open specifies a label - The label can be set and changed by using the setLabel method (setLabel is unsupported on Windows).
	 * use {@link #openAny(String) openAny(String serverID)} to open a device without specifying the label.
	 * <p>
	 * ServerID can be set to null, if the ServerID does not matter. In this case, the specified Phidget (by label) will be opened
	 * whenever it is seen on the network, regardless of which server it appears on. This also applies when not specifying a label.
	 * <p>
	 * Note: All open methods that specify a ServerID rather then an IP Addresss and Port require that both the client and host sides of the
	 * connection be running an implementation of zeroconf:
	 * <ul><li>On Windows, this means installing Apple's Bonjour - available <a href="http://www.apple.com/support/downloads/bonjourforwindows.html">here</a>.</li>
	 * <li>On Linux, this means Avahi, which is usually either installed by default or available as a package install.</li>
	 * <li>On MacOSX, Bonjour is already integrated into the operationg system.</li></ul>
	 * 
	 * @param label The Label
	 * @param serverID ServerID of the Phidget Webservice
	 */
	public final void openLabel(String label, String serverID) throws PhidgetException
	{
		openLabel(label, serverID, "");
	}

	/**
	 * Open this Phidget remotely and securely, using a Server ID, and a specific label. 
	 * <p>
	 * This method is the same as {@link #openLabel(String, String) openLabel(String label, String serverID)}, except that it specifies a password. 
	 * This password can be set as a parameter when starting the Phidget Webservice.
	 */
	public final void openLabel(String label, String serverID, String password) throws PhidgetException
	{
		enableEvents(true);
		nativeOpenLabelRemote(label, serverID, password);
	}

	/**
	 * Closes this Phidget.
	 * This will shut down all threads dealing with this Phidget and you won't receive any more events.
	 * 
	 * @throws PhidgetException If this Phidget is not opened. 
	 */
	public final void close() throws PhidgetException {
		if(!managerPhidget)
		{
			enableEvents(false);
			nativeClose();
		}
	}

	private void enableEvents(boolean b) {
		enableAttachEvents(b && attachListeners.size() > 0);
		enableDetachEvents(b && detachListeners.size() > 0);
		enableErrorEvents(b && errorListeners.size() > 0);
		enableServerConnectEvents(b && serverConnectListeners.size() > 0);
		enableServerDisconnectEvents(b && serverDisconnectListeners.size() > 0);
		enableDeviceSpecificEvents(b);
	}

	/**
	 * Waits for this Phidget to become available. This method can be called after open has been called
	 * to wait for thid Phidget to become available. This is usefull because open is asynchronous (and thus returns immediately),
	 * and most methods will throw a PhidgetException is they are called before a device is actually ready. This method is
	 * synonymous with polling the isAttached method until it returns True, or using the Attach event.
	 * <p>
	 * This method blocks for up to the timeout, at which point it will throw a PhidgetException. Otherwise, it returns when
	 * the phidget is attached and initialized.
	 * <p>
	 * A timeout of 0 is infinite.
	 * 
	 * @param timeout Timeout in milliseconds
	 * @throws PhidgetException If this Phidget is not opened. 
	 */
	public final native void waitForAttachment(int timeout) throws PhidgetException;
	/**
	 * Waits for this Phidget to become available. This method can be called after open has been called
	 * to wait for thid Phidget to become available. This is usefull because open is asynchronous (and thus returns immediately),
	 * and most methods will throw a PhidgetException is they are called before a device is actually ready. This method is
	 * synonymous with polling the isAttached method until it returns True, or using the Attach event.
	 * <p>
	 * This method blocks indefinitely until the Phidget becomes available. This can be quite some time
	 * (forever), if the Phidget is never plugged in.
	 * 
	 * @throws PhidgetException If this Phidget is not opened. 
	 */
	public final void waitForAttachment() throws PhidgetException
	{
		waitForAttachment(0);
	}

	private void enableDeviceSpecificEvents(boolean b) { }

	private LinkedList attachListeners = new LinkedList();
	private long nativeAttachHandler = 0;

	/**
	 * Adds an attach listener. The attach handler is a method that will be called when this Phidget is phisically attached
	 * to the system, and has gone through its initalization, and so is ready to be used.
	 * <p>
	 * There is no limit on the number of attach handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.AttachListener AttachListener} interface
	 */
	public final void addAttachListener(AttachListener l) {
		synchronized (attachListeners) {
			attachListeners.add(l);
			enableAttachEvents(true);
		}
	}

	/**
	 * Removes an attach listener. This will remove a previously added attach listener.
	 */
	public final void removeAttachListener(AttachListener l) {
		synchronized (attachListeners) {
			attachListeners.remove(l);
			enableAttachEvents(attachListeners.size() > 0);
		}
	}
	private void fireAttach(AttachEvent e) {
		synchronized (attachListeners) {
			for (Iterator it = attachListeners.iterator();
			  it.hasNext(); )
				((AttachListener)it.next()).attached(e);
		}
	}
	private native void enableAttachEvents(boolean b);

	private LinkedList errorListeners = new LinkedList();
	private long nativeErrorHandler = 0;

	/**
	 * Adds an error listener. The error handler is a method that will be called when an asynchronous error occurs.
	 * Error events are not currently used, but will be in the future to report any problems that happen out of context from
	 * a direct function call.
	 * <p>
	 * There is no limit on the number of error handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.ErrorListener ErrorListener} interface
	 */
	public final void addErrorListener(ErrorListener l) {
		synchronized (errorListeners) {
			errorListeners.add(l);
			enableErrorEvents(true);
		}
	}

	/**
	 * Removes an error listener. This will remove a previously added error listener.
	 */
	public final void removeErrorListener(ErrorListener l) {
		synchronized (errorListeners) {
			errorListeners.remove(l);
			enableErrorEvents(errorListeners.size() > 0);
		}
	}
	private void fireError(ErrorEvent e) {
		synchronized (errorListeners) {
			for (Iterator it = errorListeners.iterator();
			  it.hasNext(); )
			((ErrorListener)it.next()).error(e);
		}
	}
	private native void enableErrorEvents(boolean b);

	private LinkedList detachListeners = new LinkedList();
	private long nativeDetachHandler = 0;

	/**
	 * Adds a detach listener. The detach handler is a method that will be called when this Phidget is phisically detached
	 * from the system, and is no longer available. This is particularly usefull for applications when a phisical detach would be expected.
	 * <p>
	 * Remember that many of the methods, if called on an unattached device, will throw a PhidgetException. This Exception can be checked to see if it
	 * was caused by a device being unattached, but a better method would be to regiter the detach handler, which could notify the main program logic that
	 * the device is no longer available, disable GUI controls, etc.
	 * <p>
	 * There is no limit on the number of detach handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.DetachListener DetachListener} interface
	 */
	public final void addDetachListener(DetachListener l) {
		synchronized (detachListeners) {
			detachListeners.add(l);
			enableDetachEvents(true);
		}
	}	
	/**
	 * Removes a detach listener. This will remove a previously added detach listener.
	 */
	public final void removeDetachListener(DetachListener l) {
		synchronized (detachListeners) {
			detachListeners.remove(l);
			enableDetachEvents(detachListeners.size() > 0);
		}
	}
	private void fireDetach(DetachEvent e) {
		synchronized (detachListeners) {
			for (Iterator it = detachListeners.iterator();
			  it.hasNext(); )
			((DetachListener)it.next()).detached(e);
		}
	}
	private native void enableDetachEvents(boolean b);


	private LinkedList serverConnectListeners = new LinkedList();
	private long nativeServerConnectHandler = 0;

	/**
	 * Adds an serverConnect listener. The serverConnect handler is a method that will be called when a connection to a server is made.
	 * This is only usefull for Phidgets opened remotely.
	 * <p>
	 * There is no limit on the number of serverConnect handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.ServerConnectListener ServerConnectListener} interface
	 */
	public final void addServerConnectListener(ServerConnectListener l)
	{
		synchronized (serverConnectListeners)
		{
			serverConnectListeners.add(l);
			enableServerConnectEvents(true);
		}
	}

	/**
	 * Removes an serverConnect listener. This will remove a previously added serverConnect listener.
	 */
	public final void removeServerConnectListener(ServerConnectListener l)
	{
		synchronized (serverConnectListeners)
		{
			serverConnectListeners.remove(l);
			enableServerConnectEvents(serverConnectListeners.size() > 0);
		}
	}
	private void fireServerConnect(ServerConnectEvent e)
	{
		synchronized (serverConnectListeners)
		{
			for (Iterator it = serverConnectListeners.iterator();
			  it.hasNext(); )
				((ServerConnectListener)it.next()).serverConnected(e);
		}
	}
	private native void enableServerConnectEvents(boolean b);


	private LinkedList serverDisconnectListeners = new LinkedList();
	private long nativeServerDisconnectHandler = 0;

	/**
	 * Adds an serverDisconnect listener. The serverDisconnect handler is a method that will be called when a connection to a server is terminated.
	 * This is only usefull for Phidgets opened remotely.
	 * <p>
	 * There is no limit on the number of serverDisconnect handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.ServerDisconnectListener ServerDisconnectListener} interface
	 */
	public final void addServerDisconnectListener(ServerDisconnectListener l)
	{
		synchronized (serverDisconnectListeners)
		{
			serverDisconnectListeners.add(l);
			enableServerDisconnectEvents(true);
		}
	}

	/**
	 * Removes an serverDisconnect listener. This will remove a previously added serverDisconnect listener.
	 */
	public final void removeServerDisconnectListener(ServerDisconnectListener l)
	{
		synchronized (serverDisconnectListeners)
		{
			serverDisconnectListeners.remove(l);
			enableServerDisconnectEvents(serverDisconnectListeners.size() > 0);
		}
	}
	private void fireServerDisconnect(ServerDisconnectEvent e)
	{
		synchronized (serverDisconnectListeners)
		{
			for (Iterator it = serverDisconnectListeners.iterator();
			  it.hasNext(); )
				((ServerDisconnectListener)it.next()).serverDisconnected(e);
		}
	}
	private native void enableServerDisconnectEvents(boolean b);

	/**
	 * Return a Sring describing this Phidget.
	 */
	public String toString() {
		int dv = -1;
		int sn = -1;
		String dt = null;
		String at = "";

		try {
			dv = getDeviceVersion();
			sn = getSerialNumber();
			dt = getDeviceType();
			at = isAttached() ? " (attached)" : " (unattached)";
		} catch (PhidgetException e) {
			;
		} finally {
			if (dt == null)
				dt = getClass().getName().replaceFirst(
				  ".*\\.", "");
		}
		return dt + " v" + dv + " #" + sn + at;
	}

	protected void finalize() throws Throwable
	{
		try
		{
			if(!managerPhidget)
			{
				try
				{
					close();
				}
				catch (Exception e)
				{
					;
				}
				try
				{
					nativeDelete();
				}
				catch (Exception e)
				{
					;
				}
			}
		}
		finally
		{
			handle = 0;
			super.finalize();
		}
	}

	/**
	 * Compares two Phidgets. This method compares two Phidgets using serial number, device type and version.
	 */
	public boolean equals(Object comp)
	{
		Phidget temp = (Phidget)comp;
		try
		{
			if (temp.getSerialNumber() == getSerialNumber() && temp.getDeviceType().equals(getDeviceType()) &&
			 temp.getDeviceVersion() == getDeviceVersion())
				return true;
		}
		catch (Exception e) { }
		return false;
	}
}
