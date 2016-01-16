
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
 * This class represents a Phidget Frequency Counter. All methods
 * to read manipulate the Phidget Frequency Counter are implemented in this class.
 * <p>
 * 
 * @author Phidgets Inc.
 */
public final class FrequencyCounterPhidget extends Phidget
{
	public FrequencyCounterPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;
	/**
	 * Zero-Crossing Filter Type. This is used with {@link #getFilter(int) getFilter} and {@link #setFilter(int, int) setFilter}
	 */
	public static final int PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING = 1;
	/**
	 * Logic-Level Filter Type. This is used with {@link #getFilter(int) getFilter} and {@link #setFilter(int, int) setFilter}
	 */
	public static final int PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL = 2;
	/**
	 * Unknown Filter Type. This is used with {@link #getFilter(int) getFilter} and {@link #setFilter(int, int) setFilter}
	 */
	public static final int PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_UNKNOWN = 3;
	/**
	 * Returns the number of channels.
	 * @return Number of inputs
	 */
	public native int getFrequencyInputCount () throws PhidgetException;
	/**
	 * Returns the last calcualted frequency on the specified channel, in Hz. This function will return 0 if the {@link #getTimeout(int) getTimeout} value elapses without detecting a signal.
	 * Frequency is recalculated up to 31.25 times a second, depending on the pulse rate.  
	 * @param index Index of the channel
	 * @return frequency of the channel
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getFrequency (int index) throws PhidgetException;
	/**
	 * Returns the total number of pulses detected on the specified channel since the Phidget was opened, or since the last reset.
	 * @param index Index of the channel
	 * @return total number of pulses
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native long getTotalCount (int index) throws PhidgetException;
	/**
	 * Returns the total elapsed tiem since Phidget was opened, or since the last reset, in microseconds. This time corresponds to the {@link #getTotalCount(int) getTotalCount} property.
	 * @param index Index of the channel
	 * @return total time, in microseconds
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native long getTotalTime (int index) throws PhidgetException;
	/**
	 * Returns the timeout value, in microseconds. This value is used to set the time to wait without detecting a signal before reporting 0 Hz. The valid range is 0.1 - 100 seconds(100,000 - 100,000,000 microseconds).
	 * 0.1 timeout represents the lowest frequency that will be measurable.
	 * @param index Index of the channel
	 * @return timeout value
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getTimeout (int index) throws PhidgetException;
	/**
	* Sets the timeout value, in microseconds. This value is used to set the time to wait without detecting a signal before reporting 0 Hz. The valid range is 0.1 - 100 seconds(100,000 - 100,000,000 microseconds).
	 * 0.1 timeout represents the lowest frequency that will be measurable.
	 * @param index Index of the channel
	 * @param timeout new timeout value
	 * @throws PhidgetException If this Phidget is not opened and attached, the index is out of range or the the timeout value is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setTimeout (int index, int timeout) throws PhidgetException;
	/**
	 * Returns the enabled state on the specified channel. When the channel is disabled, it will no longer register counts. TotalTime and TotalCount properties will not be incremented until
	 * the channel is re-enabled.
	 * @param index Index of the channel
	 * @return state
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getEnabled (int index) throws PhidgetException;
	/**
	 * Gets the enabled state on the specified channel. When the channel is disabled, it will no longer register counts. TotalTime and TotalCount properties will not be incremented until
	 * the channel is re-enabled.
	 * @param index Index of the channel
	 * @param state new enabled state
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setEnabled (int index, boolean state) throws PhidgetException;
	/**
	 * Gets the channel filter mode. This controls the type of signal that the Frequency Counter will respond to - either a zero-centered or a logic-level signal.
	 * <p>
	 * The possible values for type are {@link #PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING},
	 * {@link #PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL}, {@link #PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_UNKNOWN PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_UNKNOWN}
	 * <p>
	 * @param index Index of the channel
	 * @return filter type
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getFilter (int index) throws PhidgetException;
	/**
	 * Sets the channel filter mode. This controls the type of signal that the Frequency Counter will respond to - either a zero-centered or a logic-level signal.
	 * <p>
	 * The possible values for type are {@link #PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_ZERO_CROSSING},
	 * {@link #PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_LOGIC_LEVEL}, {@link #PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_UNKNOWN PHIDGET_FREQUENCYCOUNTER_FILTERTYPE_UNKNOWN}
	 * <p>
	 * @param index Index of the channel
	 * @param type new filter type
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setFilter (int index, int type) throws PhidgetException;
	/**
	 * Resets the {@link #getTotalCount(int) getTotalCount} and {@link #getTotalTime(int) getTotalTime} counters to 0 for the specified channel. For best performance, this should be called when the channel is disabled.
	 * @param index Index of the channel
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void reset (int index) throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableFrequencyCounterCountEvents (b && frequencyCounterCountListeners.size () > 0);
	}
	/**
	 * Adds a count listener. The count handler is a method that will be called whenever some counts have been detected.
	 * This event will fire up to 31.25 times a second, depending on the pulse rate. The time is in microseconds and represents the amount
	 * of time in which the number of counts occured. This event can be used to calculate frequency independently of the phidget21 library frequency implementation.
	 * This event will fire with a count of 0 once, after the Timeout time has elapsed with no counts for a channel, to indicate 0 Hz.
	 * <p>
	 * There is no limit on the number of count handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.FrequencyCounterCountListener FrequencyCounterCountListener} interface
	 */
	public final void addFrequencyCounterCountListener (FrequencyCounterCountListener l)
	{
		synchronized (frequencyCounterCountListeners)
		{
			frequencyCounterCountListeners.add (l);
			enableFrequencyCounterCountEvents (true);
	}} private LinkedList frequencyCounterCountListeners = new LinkedList ();
	private long nativeFrequencyCounterCountHandler = 0;
	public final void removeFrequencyCounterCountListener (FrequencyCounterCountListener l)
	{
		synchronized (frequencyCounterCountListeners)
		{
			frequencyCounterCountListeners.remove (l);
			enableFrequencyCounterCountEvents (frequencyCounterCountListeners.size () > 0);
	}} private void fireFrequencyCounterCount (FrequencyCounterCountEvent e)
	{
		synchronized (frequencyCounterCountListeners)
		{
			for (Iterator it = frequencyCounterCountListeners.iterator (); it.hasNext ();)
				((FrequencyCounterCountListener) it.next ()).frequencyCounterCounted (e);
		}
	}
	private native void enableFrequencyCounterCountEvents (boolean b);
}
