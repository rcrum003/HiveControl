
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
import java.util.Calendar;
	/**
 * This class represents a Phidget GPS. All methods
 * to manipulate the Phidget GPS are implemented in this class.
 * <p>
 * 
 * @author Phidgets Inc.
 */
public final class GPSPhidget extends Phidget
{
	public GPSPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;

	/**
	 * Returns the current latitude, in signed degrees format. 
	 * @return Current latitude,  in signed degrees format.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the GPS is not plugged into the board. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 * If the latitude is not available, an EPHIDGET_UNKNOWNVAL exception is thrown.
	 */
	public native double getLatitude () throws PhidgetException;
	/**
	 * Returns the current longitude, in signed degrees format. 
	 * @return Current longitude,  in signed degrees format.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the GPS is not plugged into the board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 * If the longitude is not available, an EPHIDGET_UNKNOWNVAL exception is thrown.
	 */
	public native double getLongitude () throws PhidgetException;
	/**
	 * Returns the current altitude, in meters. 
	 * @return Current altitude, in meters.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the GPS is not plugged into the board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 * If the altitude is not available, an EPHIDGET_UNKNOWNVAL exception is thrown.
	 */
	public native double getAltitude () throws PhidgetException;
	/**
	 * Returns the current heading, in degrees - compass bearing format. Heading is only accurate if the GPS is moving, and it represents 
	 * a heading over time, and not the actual direction the PhidgetGPS is pointing.
     * exception is thrown.
	 * @return Heading in degrees(compass bearing format).
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the GPS is not plugged into the board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 * If the heading is not available, an EPHIDGET_UNKNOWNVAL.
	 */
	public native double getHeading () throws PhidgetException;
	/**
	 * Returns the current velocity, in km/h. Velocity is only accurate if the PhidgetGPS is moving. 
	 * @return Velocity in km/h.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the GPS is not plugged into the board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 * If the velocity is not available, an EPHIDGET_UNKNOWNVAL exception is thrown.
	 */
	public native double getVelocity () throws PhidgetException;
	/**
	 * Returns the current GPS date and time, in UTC. The time is updated 10 times a second and is accurate to within at least 500ms when {@link #getPositionFixStatus getPositionFixStatus} is true.
	 * @return Current GPS date and time.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the GPS is not plugged into the board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 * If the date or time are not available, an EPHIDGET_UNKNOWNVAL exception is thrown. 
	 */
	public native Calendar getDateAndTime () throws PhidgetException;
	/**
	 * Returns the current position fix status. If true, all of the above properties will be available. Time and date may or may not be available, but they can only be trusted
	 * as accurate when the PositionFixStatus is true. 
	 * @return Current position fix status.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the GPS is not plugged into the board.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 * If false, the {@link #getHeading getHeading} / {@link #getVelocity getVelocity} will throw an EPHIDGET_UNKNOWN exception. 
	 */
	public native boolean getPositionFixStatus () throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableGPSPositionFixStatusChangeEvents (b && gpsPositionFixStatusChangeListeners.size () > 0);
		enableGPSPositionChangeEvents (b && gpsPositionChangeListeners.size () > 0);
	}
	/**
	 * Adds a position fix status change listener. The position fix status change handler is a method that will be called when the position fix status changes.
	 * <p>
	 * There is no limit on the number of position fix status change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.GPSPositionFixStatusChangeListener GPSPositionFixStatusChangeListener} interface
	 */
	public final void addGPSPositionFixStatusChangeListener (GPSPositionFixStatusChangeListener l)
	{
		synchronized (gpsPositionFixStatusChangeListeners)
		{
			gpsPositionFixStatusChangeListeners.add (l);
			enableGPSPositionFixStatusChangeEvents (true);
	}} private LinkedList gpsPositionFixStatusChangeListeners = new LinkedList ();
	private long nativeGPSPositionFixStatusChangeHandler = 0;
	public final void removeGPSPositionFixStatusChangeListener (GPSPositionFixStatusChangeListener l)
	{
		synchronized (gpsPositionFixStatusChangeListeners)
		{
			gpsPositionFixStatusChangeListeners.remove (l);
			enableGPSPositionFixStatusChangeEvents (gpsPositionFixStatusChangeListeners.size () > 0);
	}} private void fireGPSPositionFixStatusChange (GPSPositionFixStatusChangeEvent e)
	{
		synchronized (gpsPositionFixStatusChangeListeners)
		{
			for (Iterator it = gpsPositionFixStatusChangeListeners.iterator (); it.hasNext ();)
				((GPSPositionFixStatusChangeListener) it.next ()).gpsPositionFixStatusChanged (e);
		}
	}
	private native void enableGPSPositionFixStatusChangeEvents (boolean b);
	/**
	 * Adds a position change listener. The position change handler is a method that will be called when the position changes.
	 * <p>
	 * There is no limit on the number of position change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.GPSPositionChangeListener GPSPositionChangeListener} interface
	 */
	public final void addGPSPositionChangeListener (GPSPositionChangeListener l)
	{
		synchronized (gpsPositionChangeListeners)
		{
			gpsPositionChangeListeners.add (l);
			enableGPSPositionChangeEvents (true);
	}} private LinkedList gpsPositionChangeListeners = new LinkedList ();
	private long nativeGPSPositionChangeHandler = 0;
	public final void removeGPSPositionChangeListener (GPSPositionChangeListener l)
	{
		synchronized (gpsPositionChangeListeners)
		{
			gpsPositionChangeListeners.remove (l);
			enableGPSPositionChangeEvents (gpsPositionChangeListeners.size () > 0);
	}} private void fireGPSPositionChange (GPSPositionChangeEvent e)
	{
		synchronized (gpsPositionChangeListeners)
		{
			for (Iterator it = gpsPositionChangeListeners.iterator (); it.hasNext ();)
				((GPSPositionChangeListener) it.next ()).gpsPositionChanged (e);
		}
	}
	private native void enableGPSPositionChangeEvents (boolean b);
}
