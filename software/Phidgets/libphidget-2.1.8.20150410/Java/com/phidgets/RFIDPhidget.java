
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
 * This class represents a Phidget RFID Reader. All methods
 * to read tags and set outputs on the RFID reader are implemented in this class.
 * <p>
 * The Phidget RFID reader can read one tag at a time. Both tag and tagloss event handlers are provided,
 * as well as control over the antenna so that multiple readers can exists in close proximity without interference.
 * 
 * @author Phidgets Inc.
 */
public final class RFIDPhidget extends Phidget
{
	public RFIDPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;

	/**
	 * EM4100 (EM4102) 40-bit. This is used with {@link #getLastTagProtocol() getSgetLastTagProtocolcreenSize} and {@link #write(String, int, boolean) write}
	 */
	public static final int PHIDGET_RFID_PROTOCOL_EM4100 = 1;
	/**
	 * ISO11785 FDX-B encoding (Animal ID). This is used with {@link #getLastTagProtocol() getSgetLastTagProtocolcreenSize} and {@link #write(String, int, boolean) write}
	 */
	public static final int PHIDGET_RFID_PROTOCOL_ISO11785_FDX_B = 2;
	/**
	 * PhidgetsTAG Protocol 24 character ASCII. This is used with {@link #getLastTagProtocol() getSgetLastTagProtocolcreenSize} and {@link #write(String, int, boolean) write}
	 */
	public static final int PHIDGET_RFID_PROTOCOL_PHIDGETS = 3;

	/**
	 * Returns the number of outputs. These are the outputs provided by the terminal block.
	 * Older RFID readers do not have these outputs, and this method will return 0.
	 * @return number of outputs
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getOutputCount () throws PhidgetException;
	/**
	 * Returns the state of an output. True indicated activated, False deactivated, which is the default.
	 * @param index index of the output
	 * @return state of the output
	 * @throws PhidgetException If this Phidget is not opened and attached, or the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getOutputState (int index) throws PhidgetException;
	/**
	 * Sets the state of a digital output. True indicated activated, False deactivated, which is the default.
	 * @param index index of the output
	 * @param state desired state
	 * @throws PhidgetException If this Phidget is not opened and attached, or the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setOutputState (int index, boolean state) throws PhidgetException;
	/**
	 * Returns the state of the antenna. True indicated that the antenna is active, False indicated inactive.
	 * @return state of the antenna
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getAntennaOn () throws PhidgetException;
	/**
	 * Sets the state of the antenna. True turns the antenna on, False turns it off. The antenna if by default
	 * turned off, and needs to be explicitely activated before tags can be read. Control over the antenna
	 * allows multiple readers to be used in close proximity, as multiple readers will interfere with each other if
	 * their antenna's are activated simultaneously.
	 * @param state new state for the antenna
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setAntennaOn (boolean state) throws PhidgetException;
	/**
	 * Returns the state of the onboard LED. This LED is by default turned off.
	 * @return state of the LED
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getLEDOn () throws PhidgetException;
	/**
	 * Sets the state of the onboard LED. True turns the LED on, False turns it off. The LED is by default turned off.
	 * @param state new state for the LED
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setLEDOn (boolean state) throws PhidgetException;
	/**
	 * Returns the last tag read. This method will only return a valid tag after a tag has been seen.
	 * This method can be used even after a tag has been removed from the reader
	 * @return tag
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native String getLastTag () throws PhidgetException;
	/**
	 * Returns the protocol of the last tag read. This method will only return a valid protocol after a tag has been seen.
	 * This method can be used even after a tag has been removed from the reader
	 * @return protocol
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getLastTagProtocol () throws PhidgetException;
	/**
	 * Returns the state of whether or not a tag is being read by the reader.
	 * True indicated that a tag is on (or near) the reader, false indicates that one is not.
	 * @return tag read state
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getTagStatus () throws PhidgetException;
	/**
	 * Writes a tag.
	 * @param tag tag string
	 * @param protocol tag protocol
	 * @param lock lock tag from futher writes
	 * @throws PhidgetException If this Phidget is not opened and attached, if the string is too malformed, or the protocol is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void write (String tag, int protocol, boolean lock) throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableTagGainEvents (b && tagGainListeners.size () > 0);
		enableTagLossEvents (b && tagLossListeners.size () > 0);
		enableOutputChangeEvents (b && outputChangeListeners.size () > 0);
	}
	/**
	 * Adds a tag gained listener. The tag gaiend handler is a method that will be called when a new tag is
	 seen by the reader. The event is only fired one time for a new tag, so the tag has to be removed and then replaced before
	 another tag gained event will fire.
	 * <p>
	 * There is no limit on the number of tag gained change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.TagGainListener TagGainListener} interface
	 */
	public final void addTagGainListener (TagGainListener l)
	{
		synchronized (tagGainListeners)
		{
			tagGainListeners.add (l);
			enableTagGainEvents (true);
	}} private LinkedList tagGainListeners = new LinkedList ();
	private long nativeTagGainHandler = 0;
	public final void removeTagGainListener (TagGainListener l)
	{
		synchronized (tagGainListeners)
		{
			tagGainListeners.remove (l);
			enableTagGainEvents (tagGainListeners.size () > 0);
	}} private void fireTagGain (TagGainEvent e)
	{
		synchronized (tagGainListeners)
		{
			for (Iterator it = tagGainListeners.iterator (); it.hasNext ();)
				((TagGainListener) it.next ()).tagGained (e);
		}
	}
	private native void enableTagGainEvents (boolean b);
	/**
	 * Adds a tag lost listener. The tag lost handler is a method that will be called when a tag is
	 removed from the reader.
	 * <p>
	 * There is no limit on the number of tag lost change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.TagLossListener TagLossListener} interface
	 */
	public final void addTagLossListener (TagLossListener l)
	{
		synchronized (tagLossListeners)
		{
			tagLossListeners.add (l);
			enableTagLossEvents (true);
	}} private LinkedList tagLossListeners = new LinkedList ();
	private long nativeTagLossHandler = 0;
	public final void removeTagLossListener (TagLossListener l)
	{
		synchronized (tagLossListeners)
		{
			tagLossListeners.remove (l);
			enableTagLossEvents (tagLossListeners.size () > 0);
	}} private void fireTagLoss (TagLossEvent e)
	{
		synchronized (tagLossListeners)
		{
			for (Iterator it = tagLossListeners.iterator (); it.hasNext ();)
				((TagLossListener) it.next ()).tagLost (e);
		}
	}
	private native void enableTagLossEvents (boolean b);
	/**
	 * Adds an output change listener. The output change handler is a method that will be called when an output has changed.
	 * <p>
	 * There is no limit on the number of output change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.OutputChangeListener OutputChangeListener} interface
	 */
	public final void addOutputChangeListener (OutputChangeListener l)
	{
		synchronized (outputChangeListeners)
		{
			outputChangeListeners.add (l);
			enableOutputChangeEvents (true);
	}} private LinkedList outputChangeListeners = new LinkedList ();
	private long nativeOutputChangeHandler = 0;
	public final void removeOutputChangeListener (OutputChangeListener l)
	{
		synchronized (outputChangeListeners)
		{
			outputChangeListeners.remove (l);
			enableOutputChangeEvents (outputChangeListeners.size () > 0);
	}} private void fireOutputChange (OutputChangeEvent e)
	{
		synchronized (outputChangeListeners)
		{
			for (Iterator it = outputChangeListeners.iterator (); it.hasNext ();)
				((OutputChangeListener) it.next ()).outputChanged (e);
		}
	}
	private native void enableOutputChangeEvents (boolean b);
}
