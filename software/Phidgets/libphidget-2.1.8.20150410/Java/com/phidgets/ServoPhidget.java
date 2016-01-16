
/*
 * This file is part of libphidget21
 *
 * Copyright Â© 2006-2015 Phidgets Inc <patrick@phidgets.com>
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
import java.util.Iterator;
import java.util.LinkedList;
import com.phidgets.event.*;
/**
 * This class represents a Phidget servo Controller. All methods
 * to control a Servo Controller are implemented in this class.
 * <p>
 * The Phidget Sevo controller simply outputs varying widths of PWM, which is what 
 * most servo motors take as an input driving signal.
 * 
 * @author Phidgets Inc.
 */
public final class ServoPhidget extends Phidget
{
	public ServoPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;

	/**
	 * Default - This is what the servo API been historically used, originally based on the Futaba FP-S148. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_DEFAULT = 1;
	/**
	 * Raw us mode - all position, velocity, acceleration functions are specified in microseconds rather then degrees. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_RAW_us_MODE = 2;
	/**
	 * HiTec HS-322HD Standard Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_HITEC_HS322HD = 3;
	/**
	 * HiTec HS-5245MG Digital Mini Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_HITEC_HS5245MG = 4;
	/**
	 * HiTec HS-805BB Mega Quarter Scale Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_HITEC_805BB = 5;
	/**
	 * HiTec HS-422 Standard Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_HITEC_HS422 = 6;
	/**
	 * Tower Pro MG90 Micro Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_TOWERPRO_MG90 = 7;
	/**
	 * HiTec HSR-1425CR Continuous Rotation Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_HITEC_HSR1425CR = 8;
	/**
	 * HiTec HS-785HB Sail Winch Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_HITEC_HS785HB = 9;
	/**
	 * HiTec HS-485HB Deluxe Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_HITEC_HS485HB = 10;
	/**
	 * HiTec HS-645MG Ultra Torque Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_HITEC_HS645MG = 11;
	/**
	 * HiTec HS-815BB Mega Sail Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_HITEC_815BB = 12;
	/**
	 * Firgelli L12 Linear Actuator 30mm 50:1. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_FIRGELLI_L12_30_50_06_R = 13;
	/**
	 * Firgelli L12 Linear Actuator 50mm 100:1. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_FIRGELLI_L12_50_100_06_R = 14;
	/**
	 * Firgelli L12 Linear Actuator 50mm 210:1. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_FIRGELLI_L12_50_210_06_R = 15;
	/**
	 * Firgelli L12 Linear Actuator 100mm 50:1. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_FIRGELLI_L12_100_50_06_R = 16;
	/**
	 * Firgelli L12 Linear Actuator 100mm 100:1. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_FIRGELLI_L12_100_100_06_R = 17;
	/**
	 * SpringRC SM-S2313M Micro Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_SPRINGRC_SM_S2313M = 18;
	/**
	 * SpringRC SM-S3317M Small Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_SPRINGRC_SM_S3317M = 19;
	/**
	 * SpringRC SM-S3317SR Small Continuous Rotation Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_SPRINGRC_SM_S3317SR = 20;
	/**
	 * SpringRC SM-S4303R Standard Continuous Rotation Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_SPRINGRC_SM_S4303R = 21;
	/**
	 * SpringRC SM-S4315M High Torque Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_SPRINGRC_SM_S4315M = 22;
	/**
	 * SpringRC SM-S4315R High Torque Continuous Rotation Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_SPRINGRC_SM_S4315R = 23;
	/**
	 * SpringRC SM-S4505B Standard Servo. This is used with {@link #getServoType getServoType} and {@link #setServoType setServoType}
	 */
	public static final int PHIDGET_SERVO_SPRINGRC_SM_S4505B = 24;
	/**
	 * User Defined servo parameters. This is used with {@link #getServoType getServoType}
	 */
	public static final int PHIDGET_SERVO_USER_DEFINED = 25;

	/**
	 * Returns the number of motors this Phidget can support. Note that there is no way of programatically
	 * determining how many motors are actaully attached to the board.
	 * @return number of motors
	 */
	public native int getMotorCount () throws PhidgetException;
	/**
	 * Returns the position of a servo motor. Note that since servo motors do not offer any feedback in their interface,
	 * this value is simply whatever the servo was last set to. There is no way of determining the position of a servo
	 * that has been plugged in, until it's position has been set. Therefore, if an initial position is important, it
	 * should be set as part of initialization.
	 * <p>
	 * If the servo is not engaged, the position is unknown and calling this function will throw an exception.
	 * <p> 
	 * The range here is between {@link #getPositionMin getPositionMin} and {@link #getPositionMax getPositionMax}, 
	 * and corresponds aproximately to an angle in degrees. Note that most servos will not be able to operate
	 * accross this entire range.
	 * @param index index of the motor
	 * @return current position of the selected motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPosition (int index) throws PhidgetException;
	/**
	 * Returns the maximum position that a servo will accept, or return.
	 * @param index Index of the servo
	 * @return Maximum position in degrees
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPositionMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum position that a servo will accept, or return.
	 * @param index Index of the servo
	 * @return Minimum position in degrees
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPositionMin (int index) throws PhidgetException;
	/**
	 * Sets the position of a servo motor.
	 * <p>
	 * The range here is between {@link #getPositionMin getPositionMin} and {@link #getPositionMax getPositionMax}, 
	 * and corresponds aproximately to an angle in degrees. Note that most servos will not be able to operate
	 * accross this entire range. Typically, the range might be 25 - 180 degrees, but this depends on the servo
	 * @param index index of the motor
	 * @param position desired position
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or position is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setPosition (int index, double position) throws PhidgetException;
	/**
	 * Engage or disengage a servo motor.
	 * <p>
	 * This engages or disengages the servo. The motor is engaged whenever you set a position, 
	 * use this function to disengage, and reengage without setting a position.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setEngaged (int index, boolean state) throws PhidgetException;
	/**
	 * @deprecated  Replaced by
	 *              {@link #setEngaged}
	 */
	public native void setMotorOn (int index, boolean state) throws PhidgetException;
	/**
	 * Returns the engaged state or a servo.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getEngaged (int index) throws PhidgetException;
	/**
	 * @deprecated  Replaced by
	 *              {@link #getEngaged}
	 */
	public native boolean getMotorOn (int index) throws PhidgetException;
	/**
	 * Returns the servo type for an index.
	 * The possible values for type are {@link #PHIDGET_SERVO_DEFAULT PHIDGET_SERVO_DEFAULT},
	 * {@link #PHIDGET_SERVO_RAW_us_MODE PHIDGET_SERVO_RAW_us_MODE}, {@link #PHIDGET_SERVO_HITEC_HS322HD PHIDGET_SERVO_HITEC_HS322HD},
	 * {@link #PHIDGET_SERVO_HITEC_HS5245MG PHIDGET_SERVO_HITEC_HS5245MG}, {@link #PHIDGET_SERVO_HITEC_805BB PHIDGET_SERVO_HITEC_805BB},
	 * {@link #PHIDGET_SERVO_HITEC_HS422 PHIDGET_SERVO_HITEC_HS422}, etc.
	 * @param index Input
	 * @return Servo Type
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getServoType (int index) throws PhidgetException;
	/**
	 * Sets the servo type for an index.
	 * <p>
	 * This constrains the PCM to the valid range of the servo, and sets the degrees-to-PCM ratio to match the actual servo rotation.
	 * <p>
	 * The possible values for type are {@link #PHIDGET_SERVO_DEFAULT PHIDGET_SERVO_DEFAULT},
	 * {@link #PHIDGET_SERVO_RAW_us_MODE PHIDGET_SERVO_RAW_us_MODE}, {@link #PHIDGET_SERVO_HITEC_HS322HD PHIDGET_SERVO_HITEC_HS322HD},
	 * {@link #PHIDGET_SERVO_HITEC_HS5245MG PHIDGET_SERVO_HITEC_HS5245MG}, {@link #PHIDGET_SERVO_HITEC_805BB PHIDGET_SERVO_HITEC_805BB},
	 * {@link #PHIDGET_SERVO_HITEC_HS422 PHIDGET_SERVO_HITEC_HS422}, etc.
	 * <p>
	 * Support for other servo types can be achieved by using {@link #setServoParameters setServoParameters}.
	 * <p>
	 * The default setting is {@link #PHIDGET_SERVO_DEFAULT PHIDGET_SERVO_DEFAULT}, which is used for historical (backwards compatibility) reasons.
	 * @param index Input
	 * @param type Servo Type
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setServoType (int index, int type) throws PhidgetException;
	/**
	 * Sets the servo parameters for a custom servo definition.
	 * <p>
	 * This constrains the PCM to the valid range of the servo, and sets the degrees-to-PCM ratio to match the actual servo rotation.
	 * <p>
	 * @param index Servo Index
	 * @param minUs Minimum PCM supported in microseconds
	 * @param maxUs Maximum PCM supported in microseconds
	 * @param degrees Degrees of rotation represented by the given PCM range
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setServoParameters (int index, double minUs, double maxUs, double degrees) throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableServoPositionChangeEvents (b && servoPositionChangeListeners.size () > 0);
	}
	/**
	 * Adds a servo position change listener. The servo position change handler is a method that will be called when the servo position 
	 * has changed. The event will get fired after every call to {@link #setPosition(int, double) setPosition}.
	 * <p>
	 * There is no limit on the number of servo position change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.ServoPositionChangeListener ServoPositionChangeListener} interface
	 */
	public final void addServoPositionChangeListener (ServoPositionChangeListener l)
	{
		synchronized (servoPositionChangeListeners)
		{
			servoPositionChangeListeners.add (l);
			enableServoPositionChangeEvents (true);
	}} private LinkedList servoPositionChangeListeners = new LinkedList ();
	private long nativeServoPositionChangeHandler = 0;
	public final void removeServoPositionChangeListener (ServoPositionChangeListener l)
	{
		synchronized (servoPositionChangeListeners)
		{
			servoPositionChangeListeners.remove (l);
			enableServoPositionChangeEvents (servoPositionChangeListeners.size () > 0);
	}} private void fireServoPositionChange (ServoPositionChangeEvent e)
	{
		synchronized (servoPositionChangeListeners)
		{
			for (Iterator it = servoPositionChangeListeners.iterator (); it.hasNext ();)
				((ServoPositionChangeListener) it.next ()).servoPositionChanged (e);
		}
	}
	private native void enableServoPositionChangeEvents (boolean b);
}
