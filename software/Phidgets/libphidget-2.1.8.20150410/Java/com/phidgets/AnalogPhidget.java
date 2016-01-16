
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
 * This class represents a Phidget Analog. All methods
 * to manipulate the Phidget Analog are implemented in this class.
 * 
 * @author Phidgets Inc.
 */
public final class AnalogPhidget extends Phidget
{
	public AnalogPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;

	/**
	 * Returns the number of analog outputs.
	 * @return Number of analog outputs
	 */
	public native int getOutputCount () throws PhidgetException;
	/**
	 * Returns the current voltage output setting for an analog output, in Volts. The range is between {@link #getVoltageMin(int) getVoltageMin} and {@link #getVoltageMax(int) getVoltageMax}.
	 * @param index index of the analog output
	 * @return current voltage for an analog output, in Volts
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVoltage (int index) throws PhidgetException;
	/**
	 * Sets the current voltage setting for an analog output, in Volts. The range is between {@link #getVoltageMin(int) getVoltageMin} and {@link #getVoltageMax(int) getVoltageMax}.
	 * @param index index of the analog output
	 * @param current new voltage for an analog output, in Volts 
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setVoltage (int index, double current) throws PhidgetException;
	/**
	 * Gets the minimum supported voltage for an output, in Volts.
	 * @param index index of the analog output
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVoltageMin (int index) throws PhidgetException;
	/**
	 * Gets the maximum supported voltage for an output, in Volts.	 
	 * @param index index of the analog output
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getVoltageMax (int index) throws PhidgetException;
	/**
	 * Returns the enabled state for an analog output. When enabled, the output drives at the set Voltage, up to 20mA. When disabled, the output is tied to ground via a 4K pull-down.
	 * @param index index of the analog output
	 * @return enabled state for an output
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getEnabled (int index) throws PhidgetException;
	/**
	 * Sets the enabled state for a an analog output. When enabled, the output drives at the set Voltage, up to 20mA. When disabled, the output is tied to ground via a 4K pull-down. 
	 * @param index index of the analog output
	 * @param state new state of the analog output
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setEnabled (int index, boolean state) throws PhidgetException;
	private final void enableDeviceSpecificEvents (boolean b)
	{
	}
}
