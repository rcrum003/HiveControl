
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
 * This class represents a Phidget Advanced Servo Controller. All methods
 * to to control a motor controller and read back motor data are implemented in this class.
 * <p>
 * The Phidget Advanced Servo Controller is able to control 1 or more servo motors. Motos acceleration
 * and Velocity are controllable. The number of servos that can be controlled depends on the
 * version of the controller.
 * 
 * @author Phidgets Inc.
 */
public final class AdvancedServoPhidget extends Phidget
{
	public AdvancedServoPhidget () throws PhidgetException
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
	 * Returns the number of servo motors supported by this Phidget. This does not neccesarily correspond
	 * to the number of motors actually attached to the board.
	 * @return number of supported motors
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getMotorCount () throws PhidgetException;
	/**
	 * Returns the maximum acceleration that a motor will accept, or return.
	 * <p>This value is in degrees per second squared.
	 * @param index Index of the motor
	 * @return Maximum acceleration
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAccelerationMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum acceleration that a motor will accept, or return.
	 * <p>This value is in degrees per second squared.
	 * @param index Index of the motor
	 * @return Minimum acceleration
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAccelerationMin (int index) throws PhidgetException;
	/**
	 * Returns the maximum velocity that a servo motor will accept, or return.
	 * <p>This value is in degrees per second.
	 * @param index Index of the motor
	 * @return Maximum velocity
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVelocityMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum velocity that a servo motor will accept, or return.
	 * <p>This value is in degrees per second.
	 * @param index Index of the motor
	 * @return Minimum velocity
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVelocityMin (int index) throws PhidgetException;
	/**
	 * Returns the maximum position that a servo motor will accept, or return.
	 * <p>This value is in degrees.
	 * @param index Index of the motor
	 * @return Maximum position
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPositionMax (int index) throws PhidgetException;
	/**
	 * Sets the maximum position that a servo motor will accept, or return. This is for limiting the range of motion of the servo
	 * controller. The Maximum cannot be extended beyond it's original value.
	 * <p>This value is in degrees.
	 * @param index Index of the motor
	 * @param position Maximum position
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setPositionMax (int index, double position) throws PhidgetException;
	/**
	 * Returns the minimum position that a servo motor will accept, or return.
	 * <p>This value uses the same units as 
	 * <p>This value is in degrees.
	 * @param index Index of the motor
	 * @return Minimum position
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPositionMin (int index) throws PhidgetException;
	/**
	 * Sets the minimum position that a servo motor will accept, or return. This is for limiting the range of motion of the servo
	 * controller. The Minimum cannot be extended beyond it's original value.
	 * <p>This value is in degrees.
	 * @param index Index of the motor
	 * @param position Minimum position
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setPositionMin (int index, double position) throws PhidgetException;
	/**
	 * Returns a motor's acceleration. The valid range is between {@link #getAccelerationMin getAccelerationMin} 
	 * and {@link #getAccelerationMax getAccelerationMax}, and refers to how fast the Servo Controller will change the speed of a motor.
	 * <p>This value is in degrees per second squared.
	 * @param index index of motor
	 * @return acceleration of motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if the acceleration is unknown.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAcceleration (int index) throws PhidgetException;
	/**
	 * Sets a motor's acceleration. 
	 * The valid range is between {@link #getAccelerationMin getAccelerationMin} and {@link #getAccelerationMax getAccelerationMax}. 
	 * This controls how fast the motor changes speed.
	 * <p>This value is in degrees per second squared.
	 * @param index index of the motor
	 * @param acceleration requested acceleration for that motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or acceleration are invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setAcceleration (int index, double acceleration) throws PhidgetException;
	/**
	 * Returns a motor's velocity limit. This is the maximum velocity that the motor will turn at. 
	 * The valid range is between {@link #getVelocityMin getVelocityMin} and {@link #getVelocityMax getVelocityMax}, 
	 * with 0 being stopped.
	 * <p>This value is in degrees per second.
	 * @param index index of the motor
	 * @return current speed of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if the velocity in unknown. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVelocityLimit (int index) throws PhidgetException;
	/**
	 * Sets a motor's velocity limit. This is the maximum velocity that the motor will turn at.
	 * The valid range is between {@link #getVelocityMin getVelocityMin} and {@link #getVelocityMax getVelocityMax}, 
	 * with 0 being stopped.
	 * <p>This value is in degrees per second.
	 * @param index index of the motor
	 * @param velocity requested velocity for the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or velocity are invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setVelocityLimit (int index, double velocity) throws PhidgetException;
	/**
	 * Returns a motor's current velocity. The valid range is between {@link #getVelocityMin getVelocityMin} and {@link #getVelocityMax getVelocityMax}, 
	 * with 0 being stopped.
	 * <p>This value is in degrees per second.
	 * @param index index of the motor
	 * @return current speed of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if the velocity in unknown. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVelocity (int index) throws PhidgetException;
	/**
	 * Returns a motor's current position. This is the actual position that the motor is at right now.
	 * The valid range is between {@link #getPositionMin getPositionMin} and {@link #getPositionMax getPositionMax}.
	 * <p>This value is in degrees.
	 * @param index index of the motor
	 * @return current position of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if the position in unknown. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPosition (int index) throws PhidgetException;
	/**
	 * Sets a motor's target position. Use this is set the target position for the servo. 
	 * If the servo is {@link #setEngaged engaged} it will start moving towards this target position.
	 * The valid range is between {@link #getPositionMin getPositionMin} and {@link #getPositionMax getPositionMax}.
	 * <p>This value is in degrees.
	 * @param index index of the motor
	 * @param position target position of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or position are invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setPosition (int index, double position) throws PhidgetException;
	/**
	 * Returns the engaged state of a motor.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getEngaged (int index) throws PhidgetException;
	/**
	 * Engage or disengage a motor.
	 * <p>
	 * This engages or disengages the servo motor. The motors are by default disengaged when the servo controller is plugged in.
	 * When the servo is disengaged, position, velocity, etc. can all be set, but the motor will not start moving until it is engaged.
	 * If position is read when a motor is disengaged, it will throw an exception.
	 * <p>
	 * This corresponds to a PCM of 0 being sent to the servo.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setEngaged (int index, boolean state) throws PhidgetException;
	/**
	 * Returns the speed ramping state of a motor.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getSpeedRampingOn (int index) throws PhidgetException;
	/**
	 * Sets the speed ramping state.
	 * <p>
	 * Disable speed ramping to disable velocity and acceleration control. With speed ramping disabled, the servo will be sent to
	 * the desired position immediately upon receiving the command. This is how the regular Phidget Servo Controller works.
	 * <p>
	 * This is turned on by default.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setSpeedRampingOn (int index, boolean state) throws PhidgetException;
	/**
	 * Returns a motor's current usage. The valid range depends on the servo controller.
	 * This value is in Amps.
	 * @param index index of the motor
	 * @return current usage of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if the value is unknown. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getCurrent (int index) throws PhidgetException;
	/**
	 * Returns the stopped state of a motor. Use this to determine if the motor is moving and/or up to date with the latest commands you have sent.
	 * If this is true, the motor is guaranteed to be stopped and to have processed every command issued. Generally, this would
	 * be polled after a target position is set to wait until that position is reached.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getStopped (int index) throws PhidgetException;
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
	 * Velocity is also constrained to the maximum speed attainable by the servo.
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
	 * Velocity is also constrained to the maximum speed attainable by the servo.
	 * <p>
	 * @param index Servo Index
	 * @param minUs Minimum PCM supported in microseconds
	 * @param maxUs Maximum PCM supported in microseconds
	 * @param degrees Degrees of rotation represented by the given PCM range
	 * @param velocityMax Maximum supported velocity of the servo in degrees/second
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setServoParameters (int index, double minUs, double maxUs, double degrees, double velocityMax) throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableServoPositionChangeEvents (b && servoPositionChangeListeners.size () > 0);
		enableServoVelocityChangeEvents (b && servoVelocityChangeListeners.size () > 0);
		enableCurrentChangeEvents (b && currentChangeListeners.size () > 0);
	}
	/**
	 * Adds a position change listener. The position change handler is a method that will be called when the servo
	 * position has changed.
	 * <p>
	 * There is no limit on the number of position change handlers that can be registered for a particular Phidget.
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
	/**
	 * Adds a velocity change listener. The velocity change handler is a method that will be called when the servo
	 * velocity has changed.
	 * <p>
	 * There is no limit on the number of velocity change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.ServoVelocityChangeListener ServoVelocityChangeListener} interface
	 */
	public final void addServoVelocityChangeListener (ServoVelocityChangeListener l)
	{
		synchronized (servoVelocityChangeListeners)
		{
			servoVelocityChangeListeners.add (l);
			enableServoVelocityChangeEvents (true);
	}} private LinkedList servoVelocityChangeListeners = new LinkedList ();
	private long nativeServoVelocityChangeHandler = 0;
	public final void removeServoVelocityChangeListener (ServoVelocityChangeListener l)
	{
		synchronized (servoVelocityChangeListeners)
		{
			servoVelocityChangeListeners.remove (l);
			enableServoVelocityChangeEvents (servoVelocityChangeListeners.size () > 0);
	}} private void fireServoVelocityChange (ServoVelocityChangeEvent e)
	{
		synchronized (servoVelocityChangeListeners)
		{
			for (Iterator it = servoVelocityChangeListeners.iterator (); it.hasNext ();)
				((ServoVelocityChangeListener) it.next ()).servoVelocityChanged (e);
		}
	}
	private native void enableServoVelocityChangeEvents (boolean b);
	/**
	 * Adds a current change listener. The current change handler is a method that will be called when the servo
	 * current has changed.
	 * <p>
	 * There is no limit on the number of current change handlers that can be registered for a particular Phidget.
	 * <p>
	 * Note that not all stepper controllers support current sensing.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.CurrentChangeListener CurrentChangeListener} interface
	 */
	public final void addCurrentChangeListener (CurrentChangeListener l)
	{
		synchronized (currentChangeListeners)
		{
			currentChangeListeners.add (l);
			enableCurrentChangeEvents (true);
	}} private LinkedList currentChangeListeners = new LinkedList ();
	private long nativeCurrentChangeHandler = 0;
	public final void removeCurrentChangeListener (CurrentChangeListener l)
	{
		synchronized (currentChangeListeners)
		{
			currentChangeListeners.remove (l);
			enableCurrentChangeEvents (currentChangeListeners.size () > 0);
	}} private void fireCurrentChange (CurrentChangeEvent e)
	{
		synchronized (currentChangeListeners)
		{
			for (Iterator it = currentChangeListeners.iterator (); it.hasNext ();)
				((CurrentChangeListener) it.next ()).currentChanged (e);
		}
	}
	private native void enableCurrentChangeEvents (boolean b);
}
