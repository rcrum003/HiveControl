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

import java.util.Iterator;
import java.util.LinkedList;

import com.phidgets.event.*;

/**
 * This class represents a Phidget Interface Kit. All methods
 * to read and write data to and from an Interface Kit are implemented in this class.
 * <p>
 * There are many types of Interface Kits, but each is simply a collection of 0 or more 
 * digital inputs, digital outpus and analog sensor inputs. The inputs can be read and outputs can be set,
 * and event handlers can be set for each of these.
 * <p>
 * See your device's User Guide for more specific API details, 
 * technical information, and revision details. The User Guide, along with other resources, can be found on 
 * the product page for your device.
 * 
 * @author Phidgets Inc.
 */
public final class InterfaceKitPhidget extends Phidget
{
	/**
	 * Class Constructor. Calling this opens a connection to the phidget21 C library creates
	 * an internal handle for this Phidget, ready to call open on.
	 * 
	 * @throws PhidgetException If there was a problem connecting to phidget21 or creating 
	 * the internal handle.
	 */
	public InterfaceKitPhidget() throws PhidgetException {
		super(create());
	}

	private static native long create() throws PhidgetException;

	/**
	 * Returns the number of digital outputs on this Interface Kit. Not all interface kits
	 * have the same number of digital outputs, and some don't have any digital outputs at all.
	 * 
	 * @return Number of digital outputs
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getOutputCount() throws PhidgetException;

	/**
	 * Returns the number of ditigal inputs on this Interface Kit. Not all interface kits
	 * have the same number of digital inputs, and some don't have any digital inputs at all.
	 * 
	 * @return Number of digital inputs
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getInputCount() throws PhidgetException;

	/**
	 * Returns the number of analog inputs on the Interface Kit. Not all interface kits
	 * have the same number of analog inputs, and some don't have any analog inputs at all.
	 * 
	 * @return Number of analog inputs
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getSensorCount() throws PhidgetException;

	/**
	 * Returns the minimum data rate for a sensor, in milliseconds.
	 * 
	 * @param index Index of the input
	 * @return minimum data rate
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getDataRateMin(int index) throws PhidgetException;

	/**
	 * Returns the maximum data rate for a sensor, in milliseconds.
	 * 
	 * @param index Index of the input
	 * @return maximum data rate
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getDataRateMax(int index) throws PhidgetException;

	/**
	 * Returns the state of a digital input. Digital inputs read True where they are activated and false
	 * when they are in their default state.
	 * <p>
	 * Be sure to check {@link #getInputCount() getInputCount} first if you are unsure as to the number of inputs, so as not to
	 * set an Index that is out of range.
	 * 
	 * @param index Index of the input
	 * @return State of the input
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getInputState(int index) throws PhidgetException;

	/**
	 * Returns the state of a digital output. Depending on the Phidget, this value may be either the value that you last
	 * wrote out to the Phidget, or the value that the Phidget last returned. This is because some Phidgets return their output state
	 * and others do not. This means that with some devices, reading the output
	 * state of a pin directly after setting it, may not return the value that you just set.
	 * <p>
	 * Be sure to check {@link #getOutputCount() getOutputCount} first if you are unsure as to the number of outputs, so as not to
	 * attempt to get an Index that is out of range.
	 * 
	 * @param index Index of the output
	 * @return State of the output
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getOutputState(int index) throws PhidgetException;

	/**
	 * Returns the value of a analog input.
	 * The analog inputs are where analog sensors are attached on the InterfaceKit 8/8/8. 
	 * On the Linear and Circular touch sensor Phidgets, analog input 0 represents position
	 * on the slider.
	 * <p>
	 * The valid range is 0-1000. In the case of a sensor, this value can be converted to an actual
	 * sensor value using the formulas provided here: http://www.phidgets.com/documentation/Sensors.pdf
	 * 
	 * @param index Index of the sensor
	 * @return Sensor value
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getSensorValue(int index) throws PhidgetException;

	/**
	 * Returns the raw value of a analog input. This is a more accurate version of {@link #getSensorValue(int) getSensorValue}.
	 * The valid range is 0-4095. Note however that the analog outputs on the Interface Kit 8/8/8 are only 10-bit values
	 * and this value represents an oversampling to 12-bit.
	 * 
	 * @param index Index of the sensor
	 * @return Sensor value
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getSensorRawValue(int index) throws PhidgetException;

	/**
	 * Returns the change trigger for an analog input. This is the ammount that an inputs must change
	 * between successive SensorChangeEvents. This is based on the 0-1000 range provided by getSensorValue.
	 * This value is by default set to 10 for most Interface Kits with analog inputs.
	 * 
	 * @param index Index of the sensor
	 * @return Trigger value
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getSensorChangeTrigger(int index)
	  throws PhidgetException;

	/**
	 * Returns the data rate for a sensor, in milliseconds.
	 * 
	 * @param index Index of the sensor
	 * @return data rate
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getDataRate(int index)
	  throws PhidgetException;

	/**
	 * Gets the ratiometric state for the analog sensors
	 * 
	 * @return state State
	 * @throws PhidgetException If this Phidget is not opened and attached, or if this phidget does not support ratiometric.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getRatiometric()
	  throws PhidgetException;

	/**
	 * Sets the state of a digital output. Setting this to true will activate the output, False is the default state.
	 * 
	 * @param index Index of the output
	 * @param newVal State to set the output to
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setOutputState(int index, boolean newVal)
	  throws PhidgetException;

	/**
	 * Sets the change trigger for an analog input. This is the ammount that an inputs must change
	 * between successive SensorChangeEvents. This is based on the 0-1000 range provided by getSensorValue.
	 * This value is by default set to 10 for most Interface Kits with analog inputs.
	 * 
	 * @param index Input
	 * @param newVal Value
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setSensorChangeTrigger(int index, int newVal)
	  throws PhidgetException;

	/**
	 * Sets the data rate of a sensor, in milliseconds.
	 * 
	 * @param index Input
	 * @param newVal data rate
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setDataRate(int index, int newVal)
	  throws PhidgetException;

	/**
	 * Sets the ratiometric state for the analog inputs. The default is for ratiometric to be set on and this
	 * is appropriate for most sensors.
	 * 
	 * @param state State
	 * @throws PhidgetException If this Phidget is not opened and attached, or if this Phidget does not support ratiometric
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setRatiometric(boolean state)
	  throws PhidgetException;

	private final void enableDeviceSpecificEvents(boolean b) {
		enableInputChangeEvents(b && inputChangeListeners.size() > 0);
		enableOutputChangeEvents(b && outputChangeListeners.size() > 0);
		enableSensorChangeEvents(b && sensorChangeListeners.size() > 0);
	}

	private LinkedList inputChangeListeners = new LinkedList();
	private long nativeInputChangeHandler = 0;

	/**
	 * Adds an input change listener. The input change handler is a method that will be called when an input on this
	 * Interface Kit has changed.
	 * <p>
	 * There is no limit on the number of input change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.InputChangeListener InputChangeListener} interface
	 */
	public final void addInputChangeListener(InputChangeListener l) {
		synchronized (inputChangeListeners) {
			inputChangeListeners.add(l);
			enableInputChangeEvents(true);
		}
	}

	/**
	 * Removes an input change listener. This will remove a previously added input change listener.
	 */
	public final void removeInputChangeListener(InputChangeListener l) {
		synchronized (inputChangeListeners) {
			inputChangeListeners.remove(l);
			enableInputChangeEvents(inputChangeListeners.size() >
			  0);
		}
	}
	private void fireInputChange(InputChangeEvent e) {
		synchronized (inputChangeListeners) {
			for (Iterator it = inputChangeListeners.iterator();
			  it.hasNext(); )
				((InputChangeListener)it.next()).inputChanged(
				  e);
		}
	}
	private native void enableInputChangeEvents(boolean b);

	private LinkedList outputChangeListeners = new LinkedList();
	private long nativeOutputChangeHandler = 0;

	/**
	 * Adds an output change listener. The output change handler is a method that will be called when an output on this
	 * Interface Kit has changed.
	 * <p>
	 * There is no limit on the number of output change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.OutputChangeListener OutputChangeListener} interface
	 */
	public final void addOutputChangeListener(OutputChangeListener l) {
		synchronized (outputChangeListeners) {
			outputChangeListeners.add(l);
			enableOutputChangeEvents(true);
		}
	}

	/**
	 * Removes an output change listener. This will remove a previously added output change listener.
	 */
	public final void removeOutputChangeListener(OutputChangeListener l) {
		synchronized (outputChangeListeners) {
			outputChangeListeners.remove(l);
			enableOutputChangeEvents(outputChangeListeners.size()
			  > 0);
		}
	}
	private void fireOutputChange(OutputChangeEvent e) {
		synchronized (outputChangeListeners) {
			for (Iterator it = outputChangeListeners.iterator();
			  it.hasNext(); )
				((OutputChangeListener)it.next())
				  .outputChanged(e);
		}
	}
	private native void enableOutputChangeEvents(boolean b);

	private LinkedList sensorChangeListeners = new LinkedList();
	private long nativeSensorChangeHandler = 0;

	/**
	 * Adds a sensor change listener. The sensor change handler is a method that will be called when a sensor on this
	 * Interface Kit has changed by at least the {@link #getSensorChangeTrigger(int) Trigger} that has been set for this sensor.
	 * <p>
	 * There is no limit on the number of sensor change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.SensorChangeListener SensorChangeListener} interface
	 */
	public final void addSensorChangeListener(SensorChangeListener l) {
		synchronized (sensorChangeListeners) {
			sensorChangeListeners.add(l);
			enableSensorChangeEvents(true);
		}
	}

	/**
	 * Removes a sensor change listener. This will remove a previously added sensor change listener.
	 */
	public final void removeSensorChangeListener(SensorChangeListener l) {
		synchronized (sensorChangeListeners) {
			sensorChangeListeners.remove(l);
			enableSensorChangeEvents(sensorChangeListeners.size()
			  > 0);
		}
	}
	private void fireSensorChange(SensorChangeEvent e) {
		synchronized (sensorChangeListeners) {
			for (Iterator it = sensorChangeListeners.iterator();
			  it.hasNext(); )
				((SensorChangeListener)it.next())
				  .sensorChanged(e);
		}
	}
	private native void enableSensorChangeEvents(boolean b);
}
