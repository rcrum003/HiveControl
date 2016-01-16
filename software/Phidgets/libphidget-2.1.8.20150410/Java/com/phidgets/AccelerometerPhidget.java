
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
 * This class represents a Phidget Accelerometer. All methods to read
 * acceleration data from an Accelerometer are implemented in this class.
 <p>
 The Phidget Accelerometer provides 2 or 3 axes of acceleration data, at anywhere from 2g to 10g
 sensitivity, depending on the specific revision. See your hardware documetation for more information.
 They can measure both static (gravity) and dynamic acceleration.
 
 * @author Phidgets Inc.
 */
public final class AccelerometerPhidget extends Phidget
{
	public AccelerometerPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;

	/**
	 * Returns the number of accelerometer axes. Currently all accelerometers provide two or three axes of acceleration - x, y, (and z).
	 * @return number of axes of acceleration
	 */
	public native int getAxisCount () throws PhidgetException;
	/**
	 * Returns the acceleration of a particular axis. This value is returned in g's, where one g of acceleration is
	 * equal to gravity. This means that at a standstill each axis will measure between -1.0 and 1.0 g's depending on 
	 * orientation.
	 * <P>
	 * This value will always be between {@link #getAccelerationMin getAccelerationMin} and {@link #getAccelerationMax getAccelerationMax}.
	 * <p>
	 * Index 0 is the x-axis, 1 is the y-axis, and 2 is the z-axis (where available).
	 * 
	 * @param index index of the axis
	 * @return acceleration of the selected axis
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAcceleration (int index) throws PhidgetException;
	/**
	 * Returns the maximum acceleration value that this axis will report. This will be set to just higher then the maximum
	 * acceleration that this axis can measure. If the acceleration is equal to this maximum, assume that that axis is saturated
	 * beyond what it can measure.
	 * @return maximum acceleration
	 */
	public native double getAccelerationMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum acceleration value that this axis will report. This will be set to just lower then the minimum
	 * acceleration that this axis can measure. If the acceleration is equal to this minimum, assume that that axis is saturated
	 * beyond what it can measure.
	 * @return minimum acceleration
	 */
	public native double getAccelerationMin (int index) throws PhidgetException;
	/**
	 * Sets the change trigger for an Axis. This value is in g's and is by default set to 0.
	 * This is the difference in acceleration that must appear between succesive calls to the
	 * OnAccelerationChange event handler.
	 * @param index index of the axis
	 * @param newVal the new change trigger for this axis
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setAccelerationChangeTrigger (int index, double newVal) throws PhidgetException;
	/**
	 * Returns the change trigger for an Axis. This value is in g's and is by default set to 0.
	 * @param index index of the axis
	 * @return change trigger of the selected axis
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAccelerationChangeTrigger (int index) throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableAccelerationChangeEvents (b && accelerationChangeListeners.size () > 0);
	}
	/**
	 * Adds an acceleration change listener. The acceleration change handler is a method that will be called when the acceleration 
	 * of an axis has changed by at least the {@link #getAccelerationChangeTrigger(int) Trigger} that has been set for that axis.
	 * <p>
	 * There is no limit on the number of acceleration change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.AccelerationChangeListener AccelerationChangeListener} interface
	 */
	public final void addAccelerationChangeListener (AccelerationChangeListener l)
	{
		synchronized (accelerationChangeListeners)
		{
			accelerationChangeListeners.add (l);
			enableAccelerationChangeEvents (true);
	}} private LinkedList accelerationChangeListeners = new LinkedList ();
	private long nativeAccelerationChangeHandler = 0;
	public final void removeAccelerationChangeListener (AccelerationChangeListener l)
	{
		synchronized (accelerationChangeListeners)
		{
			accelerationChangeListeners.remove (l);
			enableAccelerationChangeEvents (accelerationChangeListeners.size () > 0);
	}} private void fireAccelerationChange (AccelerationChangeEvent e)
	{
		synchronized (accelerationChangeListeners)
		{
			for (Iterator it = accelerationChangeListeners.iterator (); it.hasNext ();)
				((AccelerationChangeListener) it.next ()).accelerationChanged (e);
		}
	}
	private native void enableAccelerationChangeEvents (boolean b);
}
