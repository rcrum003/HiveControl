
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
 * This class represents a Phidget LED. All methods
 * to control a Phidget LED are implemented in this class.
 <p>
 The Phidget LED is a board that is meant for driving LEDs. Currently, the
 only available version drives 64 LEDs, but other versions may become available
 so this number is not absolute.
 <p>
 LEDs can be controlled individually, at brightness levels from 0-100.
 * 
 * @author Phidgets Inc.
 */
public final class LEDPhidget extends Phidget
{
	public LEDPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;

	/**
	 * 20mA current limit. This is used with {@link #getCurrentLimit getCurrentLimit} and {@link #setCurrentLimit setCurrentLimit}
	 */
	public static final int PHIDGET_LED_CURRENT_LIMIT_20mA = 1;
	/**
	 * 40mA current limit. This is used with {@link #getCurrentLimit getCurrentLimit} and {@link #setCurrentLimit setCurrentLimit}
	 */
	public static final int PHIDGET_LED_CURRENT_LIMIT_40mA = 2;
	/**
	 * 60mA current limit. This is used with {@link #getCurrentLimit getCurrentLimit} and {@link #setCurrentLimit setCurrentLimit}
	 */
	public static final int PHIDGET_LED_CURRENT_LIMIT_60mA = 3;
	/**
	 * 80mA current limit. This is used with {@link #getCurrentLimit getCurrentLimit} and {@link #setCurrentLimit setCurrentLimit}
	 */
	public static final int PHIDGET_LED_CURRENT_LIMIT_80mA = 4;

	/**
	 * 1.7V Voltage output. This is used with {@link #getVoltage getVoltage} and {@link #setVoltage setVoltage}
	 */
	public static final int PHIDGET_LED_VOLTAGE_1_7V = 1;
	/**
	 * 2.75V Voltage output. This is used with {@link #getVoltage getVoltage} and {@link #setVoltage setVoltage}
	 */
	public static final int PHIDGET_LED_VOLTAGE_2_75V = 2;
	/**
	 * 3.9V Voltage output. This is used with {@link #getVoltage getVoltage} and {@link #setVoltage setVoltage}
	 */
	public static final int PHIDGET_LED_VOLTAGE_3_9V = 3;
	/**
	 * 5.0V Voltage output. This is used with {@link #getVoltage getVoltage} and {@link #setVoltage setVoltage}
	 */
	public static final int PHIDGET_LED_VOLTAGE_5_0V = 4;

	/**
	 * Returns the current limit for the all outputs. This is only supported by some PhidgetLEDs - see the product manual.
	 * The possible values for type are {@link #PHIDGET_LED_CURRENT_LIMIT_20mA PHIDGET_LED_CURRENT_LIMIT_20mA},
	 * {@link #PHIDGET_LED_CURRENT_LIMIT_40mA PHIDGET_LED_CURRENT_LIMIT_40mA}, {@link #PHIDGET_LED_CURRENT_LIMIT_60mA PHIDGET_LED_CURRENT_LIMIT_60mA} and
	 * {@link #PHIDGET_LED_CURRENT_LIMIT_80mA PHIDGET_LED_CURRENT_LIMIT_80mA}
	 * @return Current Limit
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is out of range, or if unsupported by this board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getCurrentLimit () throws PhidgetException;
	/**
	 * Sets the current limit for all outputs. This is only supported by some PhidgetLEDs - see the product manual.
	 * <p>
	 * The possible values for type are {@link #PHIDGET_LED_CURRENT_LIMIT_20mA PHIDGET_LED_CURRENT_LIMIT_20mA},
	 * {@link #PHIDGET_LED_CURRENT_LIMIT_40mA PHIDGET_LED_CURRENT_LIMIT_40mA}, {@link #PHIDGET_LED_CURRENT_LIMIT_60mA PHIDGET_LED_CURRENT_LIMIT_60mA} and
	 * {@link #PHIDGET_LED_CURRENT_LIMIT_80mA PHIDGET_LED_CURRENT_LIMIT_80mA}
	 * <p>
	 * By default, currentLimit is set to {@link #PHIDGET_LED_CURRENT_LIMIT_20mA PHIDGET_LED_CURRENT_LIMIT_20mA}.
	 * @param currentLimit Current Limit
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is out of range, or if unsupported by this board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setCurrentLimit (int currentLimit) throws PhidgetException;

	/**
	 * Returns the voltage output for the all outputs. This is only supported by some PhidgetLEDs - see the product manual.
	 * The possible values for type are {@link #PHIDGET_LED_VOLTAGE_1_7V PHIDGET_LED_VOLTAGE_1_7V},
	 * {@link #PHIDGET_LED_VOLTAGE_2_75V PHIDGET_LED_VOLTAGE_2_75V}, {@link #PHIDGET_LED_VOLTAGE_3_9V PHIDGET_LED_VOLTAGE_3_9V} and
	 * {@link #PHIDGET_LED_VOLTAGE_5_0V PHIDGET_LED_VOLTAGE_5_0V}
	 * @return Voltage
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is out of range, or if unsupported by this board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getVoltage () throws PhidgetException;
	/**
	 * Sets the voltage output for all outputs. This is only supported by some PhidgetLEDs - see the product manual.
	 * <p>
	 * The possible values for type are {@link #PHIDGET_LED_VOLTAGE_1_7V PHIDGET_LED_VOLTAGE_1_7V},
	 * {@link #PHIDGET_LED_VOLTAGE_2_75V PHIDGET_LED_VOLTAGE_2_75V}, {@link #PHIDGET_LED_VOLTAGE_3_9V PHIDGET_LED_VOLTAGE_3_9V} and
	 * {@link #PHIDGET_LED_VOLTAGE_5_0V PHIDGET_LED_VOLTAGE_5_0V}
	 * <p>
	 * By default, voltage is set to {@link #PHIDGET_LED_VOLTAGE_2_75V PHIDGET_LED_VOLTAGE_2_75V}.
	 * @param voltage Voltage
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is out of range, or if unsupported by this board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setVoltage (int voltage) throws PhidgetException;

	/**
	 * Returns the number of LEDs that this board can drive. This may not correspond to the actual number
	 of LEDs attached.
	 * @return number of LEDs
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getLEDCount () throws PhidgetException;
	/**
	 * Deprecated. Use {@link #getBrightness getBrightness}
	 */
	public native int getDiscreteLED (int index) throws PhidgetException;
	/**
	 * Deprecated. Use {@link #setBrightness setBrightness}
	 */
	public native void setDiscreteLED (int index, int brightness) throws PhidgetException;
	/**
	 * Returns the brightness value of an LED. This value ranges from 0-100.
	 * @param index LED
	 * @return brightness
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getBrightness (int index) throws PhidgetException;
	/**
	 * Sets the brightness of an LED.
	 * Valid values are 0-100, with 0 being off and 100 being the brightest.
	 * @param index index of the LED
	 * @param brightness desired brightness of this LED
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or brightness value are out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setBrightness (int index, double brightness) throws PhidgetException;
	/**
	 * Returns the current limit value of an LED. This value ranges from 0-80 mA.
	 * @param index LED
	 * @return current limit
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getCurrentLimit (int index) throws PhidgetException;
	/**
	 * Sets the current limit of an LED.
	 * Valid values are 0-80 mA.
	 * @param index index of the LED
	 * @param limit desired current limit of this LED
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index or limit value are out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setCurrentLimit (int index, double limit) throws PhidgetException;
	private final void enableDeviceSpecificEvents (boolean b)
	{
	}
}
