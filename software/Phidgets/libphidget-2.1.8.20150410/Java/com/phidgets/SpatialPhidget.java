
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
 * This class represents a Phidget Spatial. All methods for a PhidgetSpatial are implemented in this class.
 <p>
 The Phidget Spatial may provide up to 3 axes of acceleration data, 3 axes of angular rate data and 3 axes of magnetic field data.
 
 * @author Phidgets Inc.
 */
public final class SpatialPhidget extends Phidget
{
	public SpatialPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;

	/**
	 * Returns the number of accelerometer axes.
	 * @return number of axes of acceleration
	 */
	public native int getAccelerationAxisCount () throws PhidgetException;
	/**
	 * Returns the acceleration of a particular axis.
	 * 
	 * @param index index of the axis
	 * @return acceleration of the selected axis
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAcceleration (int index) throws PhidgetException;
	/**
	 * Returns the maximum acceleration value that this axis will report.
	 * @return maximum acceleration
	 */
	public native double getAccelerationMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum acceleration value that this axis will report.
	 * @return minimum acceleration
	 */
	public native double getAccelerationMin (int index) throws PhidgetException;

	/**
	 * Returns the number of gyro axes.
	 * @return number of axes of angularRate
	 */
	public native int getGyroAxisCount () throws PhidgetException;
	/**
	 * Returns the angularRate of a particular axis.
	 * 
	 * @param index index of the axis
	 * @return angularRate of the selected axis
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getAngularRate (int index) throws PhidgetException;
	/**
	 * Returns the maximum angularRate value that this axis will report.
	 * @return maximum angularRate
	 */
	public native double getAngularRateMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum angularRate value that this axis will report.
	 * @return minimum angularRate
	 */
	public native double getAngularRateMin (int index) throws PhidgetException;

	/**
	 * Returns the number of compass axes.
	 * @return number of axes of magneticField
	 */
	public native int getCompassAxisCount () throws PhidgetException;
	/**
	 * Returns the magneticField of a particular axis.
	 * 
	 * @param index index of the axis
	 * @return magneticField of the selected axis
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getMagneticField (int index) throws PhidgetException;
	/**
	 * Returns the maximum magneticField value that this axis will report.
	 * @return maximum magneticField
	 */
	public native double getMagneticFieldMax (int index) throws PhidgetException;
	/**
	 * Returns the minimum magneticField value that this axis will report.
	 * @return minimum magneticField
	 */
	public native double getMagneticFieldMin (int index) throws PhidgetException;

	/**
	 * Returns the data rate, in milliseconds.
	 * 
	 * @return data rate
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getDataRate () throws PhidgetException;
	/**
	 * Sets the data rate, in milliseconds.
	 * 
	 * @param rate data rate
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setDataRate (int rate) throws PhidgetException;
	/**
	 * Returns the minimum data rate, in milliseconds.
	 * 
	 * @return minimum data rate
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getDataRateMin () throws PhidgetException;
	/**
	 * Returns the maximum data rate, in milliseconds.
	 * 
	 * @return maximum data rate
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getDataRateMax () throws PhidgetException;

	/**
	 * Zeroes the gyroscope. This takes about two seconds and the gyro zxes will report 0 during the process.
	 * This should only be called when the board is not moving.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void zeroGyro () throws PhidgetException;
	/**
	 * Sets the compass correction factors. This can be used to correcting any sensor errors, including hard and soft iron offsets and sensor error factors.
	 *
	 * @param magField Local magnetic field strength.
	 * @param offset0 Axis 0 offset correction.
	 * @param offset1 Axis 1 offset correction.
	 * @param offset2 Axis 2 offset correction.
	 * @param gain0 Axis 0 gain correction.
	 * @param gain1 Axis 1 gain correction.
	 * @param gain2 Axis 2 gain correction.
	 * @param T0 Non-orthogonality correction factor 0.
	 * @param T1 Non-orthogonality correction factor 1.
	 * @param T2 Non-orthogonality correction factor 2.
	 * @param T3 Non-orthogonality correction factor 3.
	 * @param T4 Non-orthogonality correction factor 4.
	 * @param T5 Non-orthogonality correction factor 5.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setCompassCorrectionParameters (double magField, double offset0, double offset1, double offset2, double gain0, double gain1, double gain2, double T0, double T1, double T2,
							   double T3, double T4, double T5) throws PhidgetException;
	/**
	 * Resets the compass correction factors. Magnetic field data will be presented directly as reported by the sensor.
	 *
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void resetCompassCorrectionParameters () throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableSpatialDataEvents (b && spatialDataListeners.size () > 0);
	}
	/**
	 * Adds a spatial data listener. The spatial data handler is a method that will be called at a rate of
	 * {@link #getDataRate() DataRate} that has been set for the device. This event contains data for all axes of all sensors.
	 * <p>
	 * There is no limit on the number of spatial data handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.SpatialDataListener SpatialDataListener} interface
	 */
	public final void addSpatialDataListener (SpatialDataListener l)
	{
		synchronized (spatialDataListeners)
		{
			spatialDataListeners.add (l);
			enableSpatialDataEvents (true);
	}} private LinkedList spatialDataListeners = new LinkedList ();
	private long nativeSpatialDataHandler = 0;
	public final void removeSpatialDataListener (SpatialDataListener l)
	{
		synchronized (spatialDataListeners)
		{
			spatialDataListeners.remove (l);
			enableSpatialDataEvents (spatialDataListeners.size () > 0);
	}} private void fireSpatialData (SpatialDataEvent e)
	{
		synchronized (spatialDataListeners)
		{
			for (Iterator it = spatialDataListeners.iterator (); it.hasNext ();)
				((SpatialDataListener) it.next ()).data (e);
		}
	}
	private native void enableSpatialDataEvents (boolean b);
}
