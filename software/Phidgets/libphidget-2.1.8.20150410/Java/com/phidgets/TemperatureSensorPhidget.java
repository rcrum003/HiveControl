
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
 * This class represents a Phidget temperature Sensor. All methods
 * to read temperaure data from the sensor are implemented in this class.
 * <p>
 * The Temperature Phidget consists of a thermocouple interface, and a temperature
 * sensing IC, which is used to measure the temperature of the thermocouple cold junction
 * and calibrate the thermocouple sensed temperature.
 * <p>
 * Both the thermocouple and temperature IC temperatures can be read. Value are returned in degrees celcius.
 * 
 * @author Phidgets Inc.
 */
public final class TemperatureSensorPhidget extends Phidget
{
	public TemperatureSensorPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;

	/**
	 * K-Type Thermocouple. This is used with {@link #getThermocoupleType getThermocoupleType} and {@link #setThermocoupleType setThermocoupleType}
	 */
	public static final int PHIDGET_TEMPERATURE_SENSOR_K_TYPE = 1;
	/**
	 * J-Type Thermocouple. This is used with {@link #getThermocoupleType getThermocoupleType} and {@link #setThermocoupleType setThermocoupleType}
	 */
	public static final int PHIDGET_TEMPERATURE_SENSOR_J_TYPE = 2;
	/**
	 * E-Type Thermocouple. This is used with {@link #getThermocoupleType getThermocoupleType} and {@link #setThermocoupleType setThermocoupleType}
	 */
	public static final int PHIDGET_TEMPERATURE_SENSOR_E_TYPE = 3;
	/**
	 * T-Type Thermocouple. This is used with {@link #getThermocoupleType getThermocoupleType} and {@link #setThermocoupleType setThermocoupleType}
	 */
	public static final int PHIDGET_TEMPERATURE_SENSOR_T_TYPE = 4;

	/**
	 * Returns the number of thermocouples.
	 * @return Number of inputs
	 */
	public native int getTemperatureInputCount () throws PhidgetException;
	/**
	 * @deprecated  Replaced by
	 *              {@link #getTemperatureInputCount}
	 */
	public native int getSensorCount () throws PhidgetException;
	/**
	 * Returns the temperature of a thermocouple. This value is returned in degrees celcius but can easily be converted into other units.
	 * This value will always be between {@link #getTemperatureMin getTemperatureMin} and {@link #getTemperatureMax getTemperatureMax}.
	 * The accuracy depends on the thermocouple used. The board is calibrated during manufacture.
	 * @param index Index of the thermocouple
	 * @return Temperature in derees celcius
	 * @throws PhidgetException If this Phidget is not opened and attached, or if a thermocouple sensor is not plugged into the board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getTemperature (int index) throws PhidgetException;
	/**
	 * Returns the maximum temperature that will be returned by a thermocouple input. This value depends on the thermocouple type.
	 * @param index Index of the thermocouple
	 * @return Maximum temperature in derees celcius
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getTemperatureMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum temperature that will be returned by a thermocouple input. This value depends on the thermocouple type.
	 * @param index Index of the thermocouple
	 * @return Minimum temperature in derees celcius
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getTemperatureMin (int index) throws PhidgetException;
	/**
	 * Returns the temperature of the ambient sensor. This value is returned in degrees celcius but can easily be converted into other units.
	 * This value will always be between {@link #getAmbientTemperatureMin getAmbientTemperatureMin} and {@link #getAmbientTemperatureMax getAmbientTemperatureMax}.
	 * This is the temperature of the board at the thermocouple cold junction.
	 * @return Temperature in derees celcius
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAmbientTemperature () throws PhidgetException;
	/**
	 * Returns the maximum temperature that will be returned by the ambient sensor.
	 * @return Maximum ambient temperature in derees celcius
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAmbientTemperatureMax () throws PhidgetException;
	/**
	 * Returns the minimum temperature that will be returned by the ambient sensor.
	 * @return Minimum ambient temperature in derees celcius
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAmbientTemperatureMin () throws PhidgetException;
	/**
	 * Returns the potential of a thermocouple input. This value is returned in millivolts.
	 * This value will always be between {@link #getPotentialMin getPotentialMin} and {@link #getPotentialMax getPotentialMax}.
	 * This is very accurate, as it is a raw value from the A/D. This is the value that is internally used to calculate temperature in the library.
	 * @param index Index of the thermocouple
	 * @return Potential in millivolts
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPotential (int index) throws PhidgetException;
	/**
	 * Returns the maximum potential that will be returned by a thermocouple input.
	 * @param index Index of the thermocouple
	 * @return Maximum potential in millivolts
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPotentialMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum potential that will be returned by a thermocouple input.
	 * @param index Index of the thermocouple
	 * @return Minimum potential in millivolts
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPotentialMin (int index) throws PhidgetException;
	/**
	 * Sets the change trigger for an input. This is the ammount by which the sensed temperature must
	 * change between TemperatureChangeEvents. By default this is set to 0.5.
	 * @param index Input
	 * @param newVal Trigger
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setTemperatureChangeTrigger (int index, double newVal) throws PhidgetException;
	/**
	 * Returns the change trigger for an input. This is the ammount by which the sensed temperature must
	 * change between TemperatureChangeEvents. By default this is set to 0.5.
	 * @param index Input
	 * @return Trigger
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getTemperatureChangeTrigger (int index) throws PhidgetException;
	/**
	 * Returns the thermocouple type for an input.
	 * The possible values for type are {@link #PHIDGET_TEMPERATURE_SENSOR_K_TYPE PHIDGET_TEMPERATURE_SENSOR_K_TYPE},
	 * {@link #PHIDGET_TEMPERATURE_SENSOR_J_TYPE PHIDGET_TEMPERATURE_SENSOR_J_TYPE}, {@link #PHIDGET_TEMPERATURE_SENSOR_E_TYPE PHIDGET_TEMPERATURE_SENSOR_E_TYPE} and
	 * {@link #PHIDGET_TEMPERATURE_SENSOR_T_TYPE PHIDGET_TEMPERATURE_SENSOR_T_TYPE}
	 * @param index Input
	 * @return Thermocouple Type
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getThermocoupleType (int index) throws PhidgetException;
	/**
	 * Sets the thermocouple type for an input. The Phidget Temperature Sensor board can be used with K, E, J and T-Type Thermocouples.
	 * Support for other thermocouple types, and voltage sources other then thermocouples in the valid range (between {@link #getPotentialMin getPotentialMin} and {@link #getPotentialMax getPotentialMax}) can
	 * be achieved using {@link #getPotential getPotential}.
	 * <p>
	 * The possible values for type are {@link #PHIDGET_TEMPERATURE_SENSOR_K_TYPE PHIDGET_TEMPERATURE_SENSOR_K_TYPE},
	 * {@link #PHIDGET_TEMPERATURE_SENSOR_J_TYPE PHIDGET_TEMPERATURE_SENSOR_J_TYPE}, {@link #PHIDGET_TEMPERATURE_SENSOR_E_TYPE PHIDGET_TEMPERATURE_SENSOR_E_TYPE} and
	 * {@link #PHIDGET_TEMPERATURE_SENSOR_T_TYPE PHIDGET_TEMPERATURE_SENSOR_T_TYPE}
	 * <p>
	 * By default, type is set to {@link #PHIDGET_TEMPERATURE_SENSOR_K_TYPE PHIDGET_TEMPERATURE_SENSOR_K_TYPE}.
	 * @param index Input
	 * @param type Thermocouple Type
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setThermocoupleType (int index, int type) throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableTemperatureChangeEvents (b && temperatureChangeListeners.size () > 0);
	}
	/**
	 * Adds a temperature change listener. The temperature change handler is a method that will be called when the temperature 
	 * has changed by at least the {@link #getTemperatureChangeTrigger(int) Trigger} that has been set.
	 * <p>
	 * There is no limit on the number of temperature change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.TemperatureChangeListener TemperatureChangeListener} interface
	 */
	public final void addTemperatureChangeListener (TemperatureChangeListener l)
	{
		synchronized (temperatureChangeListeners)
		{
			temperatureChangeListeners.add (l);
			enableTemperatureChangeEvents (true);
	}} private LinkedList temperatureChangeListeners = new LinkedList ();
	private long nativeTemperatureChangeHandler = 0;
	public final void removeTemperatureChangeListener (TemperatureChangeListener l)
	{
		synchronized (temperatureChangeListeners)
		{
			temperatureChangeListeners.remove (l);
			enableTemperatureChangeEvents (temperatureChangeListeners.size () > 0);
	}} private void fireTemperatureChange (TemperatureChangeEvent e)
	{
		synchronized (temperatureChangeListeners)
		{
			for (Iterator it = temperatureChangeListeners.iterator (); it.hasNext ();)
				((TemperatureChangeListener) it.next ()).temperatureChanged (e);
		}
	}
	private native void enableTemperatureChangeEvents (boolean b);
}
