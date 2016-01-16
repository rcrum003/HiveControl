
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
 * This class represents a Phidget Motor Controller. All Methods
 * to to control a motor controller and read back motor data are implemented in this class.
 <p>
 The Motor Control Phidget is able to control 1 or more DC motors. 
 Both speed and acceleration are controllable. Speed is controlled via PWM. The size of the motors
 that can be driven depends on the motor controller. See your hardware documentation for more information.
 <p>
 The motor Controller boards also has 0 or more digital inputs.
 * 
 * @author Phidgets Inc.
 */
public final class MotorControlPhidget extends Phidget
{
	public MotorControlPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;

	/**
	 * Returns the number of motors supported by this Phidget. This does not neccesarily correspond
	 to the number of motors actually attached to the board.
	 * @return number of supported motors
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getMotorCount () throws PhidgetException;
	/**
	 * Returns the number of digital inputs. Not all Motor Controllers have digital inputs.
	 * @return number of digital inputs
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getInputCount () throws PhidgetException;
	/**
	 * Returns the number of encoders. Not all Motor Controllers have encoders.
	 * @return number of encoders
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getEncoderCount () throws PhidgetException;
	/**
	 * Returns the number of analog inputs. Not all Motor Controllers have analog inputs.
	 * @return number of analog inputs
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getSensorCount () throws PhidgetException;
	/**
	 * Returns the state of a digital input. True means that the input is activated, and False indicates the default state.
	 * @param index index of the input
	 * @return state of the input
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getInputState (int index) throws PhidgetException;
	/**
	 * Returns the position of an encoder. This is an absolute position as calcutated since the encoder was plugged in.
	 This value can be reset to anything using {@link #setEncoderPosition(int, int) setEncoderPosition}.  Not all Motor Controllers have encoders.

	 * @param index index of the encoder
	 * @return position of the encoder
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getEncoderPosition (int index) throws PhidgetException;
	/**
	 * Sets the position of a specific encoder. This resets the internal position count for an encoder. This call in no way actually 
	 sends information to the device, as an absolute position is maintained only in the library. After this call, position changes
	 from the encoder will use the new value to calculate absolute position as reported by {@link #getEncoderPosition}. Not all Motor Controllers have encoders.
	 * @param index index of the encoder
	 * @param position new position for a encoder.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setEncoderPosition (int index, int position) throws PhidgetException;
	/**
	 * Returns the value of a sensor. Not all Motor Controllers have analog sensors.
	 * @param index index of the sensor
	 * @return sensor value of the sensor. Range is 0-1000.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getSensorValue (int index) throws PhidgetException;
	/**
	 * Returns the raw value of a sensor(12-bit). Not all Motor Controllers have analog sensors.
	 * @param index index of the sensor
	 * @return sensor raw value of the sensor. Range is 0-4096.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getSensorRawValue (int index) throws PhidgetException;
	/**
	 * Returns the ratiometric state of a sensor. Not all Motor Controllers have analog sensors.
	 * @return ratiometric state of sensors
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getRatiometric () throws PhidgetException;
	/**
	 * Sets the ratiometric state. This controls the voltage reference used for sampling the analog sensors. 
	 * @param ratiometric new ratiometric state of the sensors
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setRatiometric (boolean ratiometric) throws PhidgetException;
	/**
	 * Returns a motor's acceleration. The valid range is between {@link #getAccelerationMin getAccelerationMin} and {@link #getAccelerationMax getAccelerationMax}, and refers to how fast the Motor Controller
	 will change the speed of a motor.
	 * @param index Index of motor
	 * @return acceleration of motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAcceleration (int index) throws PhidgetException;
	/**
	 * Sets a motor's acceleration. 
	 * The valid range is between {@link #getAccelerationMin getAccelerationMin} and {@link #getAccelerationMax getAccelerationMax}. This controls how fast the motor changes speed.
	 * @param index index of the motor
	 * @param acceleration requested acceleration for that motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or acceleration value are invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setAcceleration (int index, double acceleration) throws PhidgetException;
	/**
	 * Returns the maximum acceleration that a motor will accept, or return.
	 * @param index Index of the motor
	 * @return Maximum acceleration
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAccelerationMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum acceleration that a motor will accept, or return.
	 * @param index Index of the motor
	 * @return Minimum acceleration
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAccelerationMin (int index) throws PhidgetException;
	/**
	 * Returns a motor's velocity. The valid range is -100 - 100, with 0 being stopped.
	 * @param index index of the motor
	 * @return current velocity of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVelocity (int index) throws PhidgetException;
	/**
	 * @deprecated  Replaced by
	 *              {@link #getVelocity}
	 */
	public native double getSpeed (int index) throws PhidgetException;
	/**
	 * Sets a motor's velocity.
	 * The valid range is from -100 to 100, with 0 being stopped. -100 and 100 both correspond to full voltage,
	 with the value in between corresponding to different widths of PWM.
	 * @param index index of the motor
	 * @param velocity requested velocity for the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or speed value are invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setVelocity (int index, double velocity) throws PhidgetException;
	/**
	 * @deprecated  Replaced by
	 *              {@link #setVelocity}
	 */
	public native void setSpeed (int index, double speed) throws PhidgetException;
	/**
	 * Returns a motor's current usage. The valid range is 0 - 255. Note that this is not supported on all motor controllers.
	 * @param index index of the motor
	 * @return current usage of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getCurrent (int index) throws PhidgetException;
	/**
	 * Returns the Back EMF sensing state for a specific motor. Note that this is not supported on all motor controllers.
	 * @param index Index of the motor
	 * @return Back EMF Sensing state for the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getBackEMFSensingState (int index) throws PhidgetException;
	/**
	 * Sets the Back EMF sensing state for a specific motor. Note that this is not supported on all motor controllers.
	 * @param index Index of the motor
	 * @param bEMFState new Back EMF Sensing State
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setBackEMFSensingState (int index, boolean bEMFState) throws PhidgetException;
	/**
	 * Sets the Back EMF voltage for a specific motor. Note that this is not supported on all motor controllers.
	 * @param index Index of the motor
	 * @return Back EMF voltage of the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getBackEMF (int index) throws PhidgetException;
	/**
	 * Sets the supply voltage for the motors. This could be  higher than the actual voltage. Note that this is not supported on all motor controllers.
	 * @return Supply voltage of the motor, in volts
	 * @throws PhidgetException If this Phidget is not opened and attached, the index is invalid, or if {@link #setBackEMFSensingState setBackEMFSensingState} is not set.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getSupplyVoltage () throws PhidgetException;
	/**
	 * Returns the braking value for a specific motor. Note that this is not supported on all motor controllers.
	 * @param index index of the motor
	 * @return Braking value for the motor
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getBraking (int index) throws PhidgetException;
	/**
	 * Sets the braking value for a specific motor. This is applied when velocity is 0. Default is 0%. Note that this is not supported on all motor controllers. Range is 0-100.
	 * @param index index of the motor
	 * @param braking new braking value for a specific motor
	 * @throws PhidgetException If this Phidget is not opened and attached, the index is invalid, or the brake value is invalid 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setBraking (int index, double braking) throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableMotorVelocityChangeEvents (b && motorVelocityChangeListeners.size () > 0);
		enableCurrentChangeEvents (b && currentChangeListeners.size () > 0);
		enableCurrentUpdateEvents (b && currentUpdateListeners.size () > 0);
		enableInputChangeEvents (b && inputChangeListeners.size () > 0);
		enableEncoderPositionChangeEvents (b && encoderPositionChangeListeners.size () > 0);
		enableEncoderPositionUpdateEvents (b && encoderPositionUpdateListeners.size () > 0);
		enableBackEMFUpdateEvents (b && backEMFUpdateListeners.size () > 0);
		enableSensorUpdateEvents (b && sensorUpdateListeners.size () > 0);
	}
	/**
	 * Adds a velocity change listener. The velocity change handler is a method that will be called when the velocity
	 of a motor changes. These velocity changes are reported back from the Motor Controller and so correspond to actual motor speeds
	 over time.
	 * <p>
	 * There is no limit on the number of velocity change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.MotorVelocityChangeListener MotorVelocityChangeListener} interface
	 */
	public final void addMotorVelocityChangeListener (MotorVelocityChangeListener l)
	{
		synchronized (motorVelocityChangeListeners)
		{
			motorVelocityChangeListeners.add (l);
			enableMotorVelocityChangeEvents (true);
	}} private LinkedList motorVelocityChangeListeners = new LinkedList ();
	private long nativeMotorVelocityChangeHandler = 0;
	public final void removeMotorVelocityChangeListener (MotorVelocityChangeListener l)
	{
		synchronized (motorVelocityChangeListeners)
		{
			motorVelocityChangeListeners.remove (l);
			enableMotorVelocityChangeEvents (motorVelocityChangeListeners.size () > 0);
	}} private void fireMotorVelocityChange (MotorVelocityChangeEvent e)
	{
		synchronized (motorVelocityChangeListeners)
		{
			for (Iterator it = motorVelocityChangeListeners.iterator (); it.hasNext ();)
				((MotorVelocityChangeListener) it.next ()).motorVelocityChanged (e);
		}
	}
	private native void enableMotorVelocityChangeEvents (boolean b);
	/**
	 * Adds a current change listener. The current change handler is a method that will be called when the current
	 consumed by a motor changes. Note that this event is not supported with the current motor controller, but
	 will be supported in the future
	 * <p>
	 * There is no limit on the number of current change handlers that can be registered for a particular Phidget.
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
	 * Adds a current update listener. The current change handler is a method that will be called every 8ms, regardless whether the current
	 consumed by a motor changes. Note that this event is not supported with all motor controllers
	 * <p>
	 * There is no limit on the number of current update handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.CurrentUpdateListener CurrentUpdateListener} interface
	 */
	public final void addCurrentUpdateListener (CurrentUpdateListener l)
	{
		synchronized (currentUpdateListeners)
		{
			currentUpdateListeners.add (l);
			enableCurrentUpdateEvents (true);
	}} private LinkedList currentUpdateListeners = new LinkedList ();
	private long nativeCurrentUpdateHandler = 0;
	public final void removeCurrentUpdateListener (CurrentUpdateListener l)
	{
		synchronized (currentUpdateListeners)
		{
			currentUpdateListeners.remove (l);
			enableCurrentUpdateEvents (currentUpdateListeners.size () > 0);
	}} private void fireCurrentUpdate (CurrentUpdateEvent e)
	{
		synchronized (currentUpdateListeners)
		{
			for (Iterator it = currentUpdateListeners.iterator (); it.hasNext ();)
				((CurrentUpdateListener) it.next ()).currentUpdated (e);
		}
	}
	private native void enableCurrentUpdateEvents (boolean b);
	/**
	 * Adds an input change listener. The input change handler is a method that will be called when an input on this
	 * Motor Controller board has changed.
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
	/**
	 * Adds a position change listener. The position change handler is a method that will be called when the position of an 
	 encoder changes. The position change event provides data about how many ticks have occured, and how much time has
	 passed since the last position change event, but does not contain an absolute position. This can be obtained from
	 {@link #getEncoderPosition}.
	 * <p>
	 * There is no limit on the number of encoder position change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.EncoderPositionChangeListener EncoderPositionChangeListener} interface
	 */
	public final void addEncoderPositionChangeListener (EncoderPositionChangeListener l)
	{
		synchronized (encoderPositionChangeListeners)
		{
			encoderPositionChangeListeners.add (l);
			enableEncoderPositionChangeEvents (true);
	}} private LinkedList encoderPositionChangeListeners = new LinkedList ();
	private long nativeEncoderPositionChangeHandler = 0;
	public final void removeEncoderPositionChangeListener (EncoderPositionChangeListener l)
	{
		synchronized (encoderPositionChangeListeners)
		{
			encoderPositionChangeListeners.remove (l);
			enableEncoderPositionChangeEvents (encoderPositionChangeListeners.size () > 0);
	}} private void fireEncoderPositionChange (EncoderPositionChangeEvent e)
	{
		synchronized (encoderPositionChangeListeners)
		{
			for (Iterator it = encoderPositionChangeListeners.iterator (); it.hasNext ();)
				((EncoderPositionChangeListener) it.next ()).encoderPositionChanged (e);
		}
	}
	private native void enableEncoderPositionChangeEvents (boolean b);
	/**
	 * Adds a position update listener. The position update handler is a method that will be called at a constant rate; every 8ms, whether the encoder position has changed or not. 
	 * The position update event provides data about how many ticks have occured since the last position update event, but does not contain an absolute position. This can be obtained from
	 *{@link #getEncoderPosition}.
	 * <p>
	 * There is no limit on the number of encoder position update handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.EncoderPositionUpdateListener EncoderPositionUpdateListener} interface
	 */
	public final void addEncoderPositionUpdateListener (EncoderPositionUpdateListener l)
	{
		synchronized (encoderPositionUpdateListeners)
		{
			encoderPositionUpdateListeners.add (l);
			enableEncoderPositionUpdateEvents (true);
	}} private LinkedList encoderPositionUpdateListeners = new LinkedList ();
	private long nativeEncoderPositionUpdateHandler = 0;
	public final void removeEncoderPositionUpdateListener (EncoderPositionUpdateListener l)
	{
		synchronized (encoderPositionUpdateListeners)
		{
			encoderPositionUpdateListeners.remove (l);
			enableEncoderPositionUpdateEvents (encoderPositionUpdateListeners.size () > 0);
	}} private void fireEncoderPositionUpdate (EncoderPositionUpdateEvent e)
	{
		synchronized (encoderPositionUpdateListeners)
		{
			for (Iterator it = encoderPositionUpdateListeners.iterator (); it.hasNext ();)
				((EncoderPositionUpdateListener) it.next ()).encoderPositionUpdated (e);
		}
	}
	private native void enableEncoderPositionUpdateEvents (boolean b);
	/**
	 * Adds a Back EMF update listener. The Back EMF update handler is a method that will be called at a constant rate; every 8ms, whether the Back EMF value has changed or not. 
	 * <p>
	 * There is no limit on the number of Back EMF update handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.BackEMFUpdateListener BackEMFUpdateListener} interface
	 */
	public final void addBackEMFUpdateListener (BackEMFUpdateListener l)
	{
		synchronized (backEMFUpdateListeners)
		{
			backEMFUpdateListeners.add (l);
			enableBackEMFUpdateEvents (true);
	}} private LinkedList backEMFUpdateListeners = new LinkedList ();
	private long nativeBackEMFUpdateHandler = 0;
	public final void removeBackEMFUpdateListener (BackEMFUpdateListener l)
	{
		synchronized (backEMFUpdateListeners)
		{
			backEMFUpdateListeners.remove (l);
			enableBackEMFUpdateEvents (backEMFUpdateListeners.size () > 0);
	}} private void fireBackEMFUpdate (BackEMFUpdateEvent e)
	{
		synchronized (backEMFUpdateListeners)
		{
			for (Iterator it = backEMFUpdateListeners.iterator (); it.hasNext ();)
				((BackEMFUpdateListener) it.next ()).backEMFUpdated (e);
		}
	}
	private native void enableBackEMFUpdateEvents (boolean b);
	/**
	 * Adds a sensor update listener. The sensor update handler is a method that will be called at a constant rate; every 8ms, whether the sensor value has changed or not. 
	 * <p>
	 * There is no limit on the number of sensor update handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.SensorUpdateListener SensorUpdateListener} interface
	 */
	public final void addSensorUpdateListener (SensorUpdateListener l)
	{
		synchronized (sensorUpdateListeners)
		{
			sensorUpdateListeners.add (l);
			enableSensorUpdateEvents (true);
	}} private LinkedList sensorUpdateListeners = new LinkedList ();
	private long nativeSensorUpdateHandler = 0;
	public final void removeSensorUpdateListener (SensorUpdateListener l)
	{
		synchronized (sensorUpdateListeners)
		{
			sensorUpdateListeners.remove (l);
			enableSensorUpdateEvents (sensorUpdateListeners.size () > 0);
	}} private void fireSensorUpdate (SensorUpdateEvent e)
	{
		synchronized (sensorUpdateListeners)
		{
			for (Iterator it = sensorUpdateListeners.iterator (); it.hasNext ();)
				((SensorUpdateListener) it.next ()).sensorUpdated (e);
		}
	}
	private native void enableSensorUpdateEvents (boolean b);
}
