
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
 * This class represents a Phidget PH Sensor. All methods
 * to read pH data from the PH Sensor are implemented in this class.
 * <p>
 * The Phidget PH Sensor provides one standard pH sensor input.
 * 
 * @author Phidgets Inc.
 */
public final class PHSensorPhidget extends Phidget
{
	public PHSensorPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;
	/**
	 * Returns the measured pH. This value can range from between {@link #getPHMin getPHMin} and {@link #getPHMax getPHMax}, but
	 * some of this range is likely outside of the valid range of most pH sensors. For example, when there is no pH sensor
	 * attached, the board will often report an (invalid) pH of 15, which while technically within a valid pH range, is unlikely to be seen.
	 * @return the current pH
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPH () throws PhidgetException;
	/**
	 * Returns the maximum pH that will be returned by the pH sensor input.
	 * @return Maximum pH
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPHMax () throws PhidgetException;
	/**
	 * Returns the minimum pH that will be returned by the pH sensor input.
	 * @return Minimum pH
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPHMin () throws PhidgetException;
	/**
	 * Returns the Potential, in millivolts. This returns the actual voltage potential measured by the A/D.
	 * This value will always be between {@link #getPotentialMin getPotentialMin} and {@link #getPotentialMax getPotentialMax}.
	 * This is the value that is internally used to calculate pH in the library.
	 * @return the current potential
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPotential () throws PhidgetException;
	/**
	 * Returns the maximum potential that will be returned by the pH sensor input.
	 * @return Maximum potential in millivolts
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPotentialMax () throws PhidgetException;
	/**
	 * Returns the minimum potential that will be returned by the pH sensor input.
	 * @return Minimum potential in millivolts
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPotentialMin () throws PhidgetException;
	/**
	 * Sets the probe temperature in degrees celcius.
	 * This value is used while calculating the pH. The default value in the libary is 20 degrees celcius. If the temperature
	 * of the liquid being measured is not 20 degrees, then it should be measued and set for maximum accuracy.
	 * <p>
	 * Note: All that this does is set a value in the library that is used for calculating ph. This does not set anything
	 * in the hardware itself.
	 *
	 * @param newVal temperature
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setTemperature (double newVal) throws PhidgetException;
	/**
	 * Sets the change trigger.
	 * This is how much the pH much change between successive PHChangeEvents. By default this value is set to 0.05
	 * @param newVal Trigger
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setPHChangeTrigger (double newVal) throws PhidgetException;
	/**
	 * Returns the change trigger.
	 * This is how much the pH much change between successive PHChangeEvents. By default this value is set to 0.05
	 * @return Trigger
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getPHChangeTrigger () throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enablePHChangeEvents (b && phChangeListeners.size () > 0);
	}
	/**
	 * Adds a pH change listener. The pH change handler is a method that will be called when the pH 
	 * has changed by at least the {@link #getPHChangeTrigger() Trigger} that has been set.
	 * <p>
	 * There is no limit on the number of pH change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.PHChangeListener PHChangeListener} interface
	 */
	public final void addPHChangeListener (PHChangeListener l)
	{
		synchronized (phChangeListeners)
		{
			phChangeListeners.add (l);
			enablePHChangeEvents (true);
	}} private LinkedList phChangeListeners = new LinkedList ();
	private long nativePHChangeHandler = 0;
	public final void removePHChangeListener (PHChangeListener l)
	{
		synchronized (phChangeListeners)
		{
			phChangeListeners.remove (l);
			enablePHChangeEvents (phChangeListeners.size () > 0);
	}} private void firePHChange (PHChangeEvent e)
	{
		synchronized (phChangeListeners)
		{
			for (Iterator it = phChangeListeners.iterator (); it.hasNext ();)
				((PHChangeListener) it.next ()).phChanged (e);
		}
	}
	private native void enablePHChangeEvents (boolean b);
}
