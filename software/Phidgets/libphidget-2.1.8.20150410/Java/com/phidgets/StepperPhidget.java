
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
 * This class represents a Phidget Stepper Controller. All methods
 * to to control a stepper controller and read back stepper data are implemented in this class.
 * <p>
 * The Phidget Stepper is able to control 1 or more Stepper motors. Motor Acceleration and Velocity are
 * controllable, and micro-stepping is used. The type and number of motors that can be controlled
 * depend on the Stepper Controller. Digital inputs are available on select Phidget Stepper Controllers.
 * 
 * @author Phidgets Inc.
 */
public final class StepperPhidget extends Phidget
{
	public StepperPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;
	/**
	 * Returns the number of stepper motors supported by this Phidget. This does not neccesarily correspond
	 to the number of motors actually attached to the board.
	 * @return number of supported motors
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getMotorCount () throws PhidgetException;
	/**
	 * Returns the number of digital inputs. Not all Stepper Controllers have digital inputs.
	 * @return number of digital inputs
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getInputCount () throws PhidgetException;
	/**
	 * Returns the state of a digital input. True means that the input is activated, and False indicated the default state.
	 * @param index index of the input
	 * @return state of the input
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getInputState (int index) throws PhidgetException;
	/**
	 * Returns a motor's acceleration. The valid range is between {@link #getAccelerationMin getAccelerationMin} 
	 * and {@link #getAccelerationMax getAccelerationMax}, and refers to how fast the Stepper Controller will change the speed of a motor.
	 * <p>This value is in (micro)steps per second squared. The step unit will depend on the Stepper Controller. For example, the Bipolar
	 * Stepper controller has an accuracy of 16th steps, so this value would be in 16th steps per second squared.
	 * @param index index of motor
	 * @return acceleration of motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if the acceleration is unknown.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAcceleration (int index) throws PhidgetException;
	/**
	 * Returns the maximum acceleration that a motor will accept, or return.
	 * <p>This value uses the same units as {@link #setAcceleration setAcceleration}/{@link #getAcceleration getAcceleration}.
	 * @param index Index of the motor
	 * @return Maximum acceleration
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAccelerationMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum acceleration that a motor will accept, or return.
	 * <p>This value uses the same units as {@link #setAcceleration setAcceleration}/{@link #getAcceleration getAcceleration}.
	 * @param index Index of the motor
	 * @return Minimum acceleration
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAccelerationMin (int index) throws PhidgetException;
	/**
	 * Sets a motor's acceleration. 
	 * The valid range is between {@link #getAccelerationMin getAccelerationMin} and {@link #getAccelerationMax getAccelerationMax}. 
	 * This controls how fast the motor changes speed.
	 * <p>This value is in (micro)steps per second squared. The step unit will depend on the Stepper Controller. For example, the Bipolar
	 * Stepper controller has an accuracy of 16th steps, so this value would be in 16th steps per second squared.
	 * @param index index of the motor
	 * @param acceleration requested acceleration for that motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or acceleration are invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setAcceleration (int index, double acceleration) throws PhidgetException;
	/**
	 * Returns a motor's current velocity. The valid range is between {@link #getVelocityMin getVelocityMin} and {@link #getVelocityMax getVelocityMax}, 
	 * with 0 being stopped.
	 * <p>This value is in (micro)steps per second. The step unit will depend on the Stepper Controller. For example, the Bipolar
	 * Stepper controller has an accuracy of 16th steps, so this value would be in 16th steps per second.
	 * @param index index of the motor
	 * @return current speed of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if the velocity in unknown. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVelocity (int index) throws PhidgetException;
	/**
	 * Returns the maximum velocity that a stepper motor will accept, or return.
	 * <p>This value uses the same units as {@link #setVelocityLimit setVelocityLimit}/{@link #getVelocityLimit getVelocityLimit} and {@link #getVelocity getVelocity}.
	 * @param index Index of the motor
	 * @return Maximum velocity
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVelocityMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum velocity that a stepper motor will accept, or return.
	 * <p>This value uses the same units as {@link #setVelocityLimit setVelocityLimit}/{@link #getVelocityLimit getVelocityLimit} and {@link #getVelocity getVelocity}.
	 * @param index Index of the motor
	 * @return Minimum velocity
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVelocityMin (int index) throws PhidgetException;
	/**
	 * Sets a motor's velocity limit. This is the maximum velocity that the motor will turn at.
	 * The valid range is between {@link #getVelocityMin getVelocityMin} and {@link #getVelocityMax getVelocityMax}, 
	 * with 0 being stopped.
	 * <p>This value is in (micro)steps per second. The step unit will depend on the Stepper Controller. For example, the Bipolar
	 * Stepper controller has an accuracy of 16th steps, so this value would be in 16th steps per second.
	 * @param index index of the motor
	 * @param velocity requested velocity for the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or velocity are invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setVelocityLimit (int index, double velocity) throws PhidgetException;
	/**
	 * Returns a motor's velocity limit. This is the maximum velocity that the motor will turn at. 
	 * The valid range is between {@link #getVelocityMin getVelocityMin} and {@link #getVelocityMax getVelocityMax}, 
	 * with 0 being stopped.
	 * <p>This value is in (micro)steps per second. The step unit will depend on the Stepper Controller. For example, the Bipolar
	 * Stepper controller has an accuracy of 16th steps, so this value would be in 16th steps per second.
	 * @param index index of the motor
	 * @return current speed of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if the velocity in unknown. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVelocityLimit (int index) throws PhidgetException;
	/**
	 * Returns the maximum position that a stepper motor will accept, or return.
	 * <p>This value uses the same units as 
	 * {@link #setTargetPosition setTargetPosition}/{@link #getTargetPosition getTargetPosition} and {@link #setCurrentPosition setCurrentPosition}/{@link #getCurrentPosition getCurrentPosition}.
	 * @param index Index of the motor
	 * @return Maximum position
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native long getPositionMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum position that a stepper motor will accept, or return.
	 * <p>This value uses the same units as 
	 * {@link #setTargetPosition setTargetPosition}/{@link #getTargetPosition getTargetPosition} and {@link #setCurrentPosition setCurrentPosition}/{@link #getCurrentPosition getCurrentPosition}.
	 * @param index Index of the motor
	 * @return Minimum position
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native long getPositionMin (int index) throws PhidgetException;
	/**
	 * Sets a motor's current position. Use this is (re)set the current physical position of the motor to a specific position value.
	 * This does not move the motor, and if the motor is moving, calling this will cause it to stop moving. Use {@link #setTargetPosition setTargetPosition}
	 * to move the motor to a position.
	 * The valid range is between {@link #getPositionMin getPositionMin} and {@link #getPositionMax getPositionMax}.
	 * <p>This value is in (micro)steps. The step unit will depend on the Stepper Controller. For example, the Bipolar
	 * Stepper controller has an accuracy of 16th steps, so this value would be in 16th steps.
	 * @param index index of the motor
	 * @param position current position of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or position are invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setCurrentPosition (int index, long position) throws PhidgetException;
	/**
	 * Returns a motor's current position. This is the actual step position that the motor is at right now.
	 * The valid range is between {@link #getPositionMin getPositionMin} and {@link #getPositionMax getPositionMax}.
	 * <p>This value is in (micro)steps. The step unit will depend on the Stepper Controller. For example, the Bipolar
	 * Stepper controller has an accuracy of 16th steps, so this value would be in 16th steps.
	 * @param index index of the motor
	 * @return current position of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if the position in unknown. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native long getCurrentPosition (int index) throws PhidgetException;
	/**
	 * Sets a motor's target position. Use this is set the target position for the stepper. If the stepper is {@link #setEngaged engaged} it will start moving towards
	 * this target position.
	 * The valid range is between {@link #getPositionMin getPositionMin} and {@link #getPositionMax getPositionMax}.
	 * <p>This value is in (micro)steps. The step unit will depend on the Stepper Controller. For example, the Bipolar
	 * Stepper controller has an accuracy of 16th steps, so this value would be in 16th steps.
	 * @param index index of the motor
	 * @param position target position of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or position are invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setTargetPosition (int index, long position) throws PhidgetException;
	/**
	 * Returns a motor's target position. This is the position that the motor wants to be at. If the motor is not moving,
	 * it probably has reached the target position, and this will match {@link #getCurrentPosition current position}.
	 * The valid range is between {@link #getPositionMin getPositionMin} and {@link #getPositionMax getPositionMax}.
	 * <p>This value is in (micro)steps. The step unit will depend on the Stepper Controller. For example, the Bipolar
	 * Stepper controller has an accuracy of 16th steps, so this value would be in 16th steps.
	 * @param index index of the motor
	 * @return target position of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if the position in unknown. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native long getTargetPosition (int index) throws PhidgetException;
	/**
	 * Returns a motor's current usage. The valid range is between {@link #getCurrentMin getCurrentMin} and {@link #getCurrentMax getCurrentMax}.
	 * This value is in Amps.
	 * <p>Note that this is not supported on all stepper controllers.
	 * @param index index of the motor
	 * @return current usage of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, if the value is unknown, or if this is not supported. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getCurrent (int index) throws PhidgetException;
	/**
	 * Returns the current limit.
	 * <p>This value is in Amps.
	 * @param index Index of the motor
	 * @return Current Limit
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, if the value is unknown, or if this is not supported.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getCurrentLimit (int index) throws PhidgetException;
	/**
	 * Sets a motor's current usage limit. The valid range is between {@link #getCurrentMin getCurrentMin} and {@link #getCurrentMax getCurrentMax}.
	 * This sets the maximum current that a motor will be allowed to draw. Use this with the Bipolar stepper controller to get smooth micro stepping - 
	 * see the product manual for more details. This value is in Amps.
	 * <p>Note that this is not supported on all stepper controllers.
	 * @param index index of the motor
	 * @param current current limit for the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, if the value is unknown, or if this is not supported. 
	 * 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setCurrentLimit (int index, double current) throws PhidgetException;
	/**
	 * Returns the maximum current that a stepper motor will accept, or return.
	 * <p>This value is in Amps.
	 * @param index Index of the motor
	 * @return Maximum current
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if this is not supported.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getCurrentMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum current that a stepper motor will accept, or return.
	 * <p>This value is in Amps.
	 * @param index Index of the motor
	 * @return Minimum current
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is invalid, or if this is not supported.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getCurrentMin (int index) throws PhidgetException;
	/**
	 * Engage or disengage a motor.
	 * <p>
	 * This engages or disengages the stepper motor. The motors are by default disengaged when the stepper controller is plugged in.
	 * When the stepper is disengaged, position, velocity, etc. can all be set, but the motor will not start moving until it is engaged.
	 * If position is read when a motor is disengaged, it will throw an exception.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setEngaged (int index, boolean state) throws PhidgetException;
	/**
	 * Returns the engaged state of a motor.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getEngaged (int index) throws PhidgetException;
	/**
	 * Returns the stopped state of a motor. Use this to determine if the motor is moving and/or up to date with the latest commands you have sent.
	 * If this is true, the motor is guaranteed to be stopped and to have processed every command issued. Generally, this would
	 * be polled after a target position is set to wait until that position is reached.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getStopped (int index) throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableStepperPositionChangeEvents (b && stepperPositionChangeListeners.size () > 0);
		enableStepperVelocityChangeEvents (b && stepperVelocityChangeListeners.size () > 0);
		enableCurrentChangeEvents (b && currentChangeListeners.size () > 0);
		enableInputChangeEvents (b && inputChangeListeners.size () > 0);
	}
	/**
	 * Adds a position change listener. The position change handler is a method that will be called when the stepper
	 * position has changed.
	 * <p>
	 * There is no limit on the number of position change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.StepperPositionChangeListener StepperPositionChangeListener} interface
	 */
	public final void addStepperPositionChangeListener (StepperPositionChangeListener l)
	{
		synchronized (stepperPositionChangeListeners)
		{
			stepperPositionChangeListeners.add (l);
			enableStepperPositionChangeEvents (true);
	}} private LinkedList stepperPositionChangeListeners = new LinkedList ();
	private long nativeStepperPositionChangeHandler = 0;
	public final void removeStepperPositionChangeListener (StepperPositionChangeListener l)
	{
		synchronized (stepperPositionChangeListeners)
		{
			stepperPositionChangeListeners.remove (l);
			enableStepperPositionChangeEvents (stepperPositionChangeListeners.size () > 0);
	}} private void fireStepperPositionChange (StepperPositionChangeEvent e)
	{
		synchronized (stepperPositionChangeListeners)
		{
			for (Iterator it = stepperPositionChangeListeners.iterator (); it.hasNext ();)
				((StepperPositionChangeListener) it.next ()).stepperPositionChanged (e);
		}
	}
	private native void enableStepperPositionChangeEvents (boolean b);
	/**
	 * Adds a velocity change listener. The velocity change handler is a method that will be called when the stepper
	 * velocity has changed.
	 * <p>
	 * There is no limit on the number of velocity change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.StepperVelocityChangeListener StepperVelocityChangeListener} interface
	 */
	public final void addStepperVelocityChangeListener (StepperVelocityChangeListener l)
	{
		synchronized (stepperVelocityChangeListeners)
		{
			stepperVelocityChangeListeners.add (l);
			enableStepperVelocityChangeEvents (true);
	}} private LinkedList stepperVelocityChangeListeners = new LinkedList ();
	private long nativeStepperVelocityChangeHandler = 0;
	public final void removeStepperVelocityChangeListener (StepperVelocityChangeListener l)
	{
		synchronized (stepperVelocityChangeListeners)
		{
			stepperVelocityChangeListeners.remove (l);
			enableStepperVelocityChangeEvents (stepperVelocityChangeListeners.size () > 0);
	}} private void fireStepperVelocityChange (StepperVelocityChangeEvent e)
	{
		synchronized (stepperVelocityChangeListeners)
		{
			for (Iterator it = stepperVelocityChangeListeners.iterator (); it.hasNext ();)
				((StepperVelocityChangeListener) it.next ()).stepperVelocityChanged (e);
		}
	}
	private native void enableStepperVelocityChangeEvents (boolean b);
	/**
	 * Adds a current change listener. The current change handler is a method that will be called when the stepper
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
	/**
	 * Adds an input change listener. The input change handler is a method that will be called when an input on this
	 * Stepper Controller board has changed.
	 * <p>
	 * There is no limit on the number of input change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.InputChangeListener InputChangeListener} interface
	 */
	public final void addInputChangeListener (InputChangeListener l)
	{
		synchronized (inputChangeListeners)
		{
			inputChangeListeners.add (l);
			enableInputChangeEvents (true);
	}} private LinkedList inputChangeListeners = new LinkedList ();
	private long nativeInputChangeHandler = 0;
	public final void removeInputChangeListener (InputChangeListener l)
	{
		synchronized (inputChangeListeners)
		{
			inputChangeListeners.remove (l);
			enableInputChangeEvents (inputChangeListeners.size () > 0);
	}} private void fireInputChange (InputChangeEvent e)
	{
		synchronized (inputChangeListeners)
		{
			for (Iterator it = inputChangeListeners.iterator (); it.hasNext ();)
				((InputChangeListener) it.next ()).inputChanged (e);
		}
	}
	private native void enableInputChangeEvents (boolean b);
}
