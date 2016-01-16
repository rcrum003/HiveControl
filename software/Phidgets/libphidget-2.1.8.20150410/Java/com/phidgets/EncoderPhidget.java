
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
 * This class represents a Phidget Encoder. All methods
 * to read encoder data from an encoder are implemented in this class.
 <p>
 Phidget Encoder boards generally support 1 or more encoders with 0 or more digital inputs.
 Both high speed optical and low speed mechanical encoders are supported with this API.
 * 
 * @author Phidgets Inc.
 */
public final class EncoderPhidget extends Phidget
{
	public EncoderPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;
	/**
	 * Returns number of encoders. All current encoder boards support one encoder.
	 * @return number of encoders
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getEncoderCount () throws PhidgetException;
	/**
	 * Returns number of digital inputs. On the mechanical encoder this refers to the pushbutton. The
	 high speed encoder does not have any digital inputs.
	 * @return number of inputs
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getInputCount () throws PhidgetException;
	/**
	 * Returns the position of an encoder. This is an absolute position as calcutated since the encoder was plugged in.
	 This value can be reset to anything using {@link #setEncoderPosition(int, int) setEncoderPosition}.
	 * @param index index of the encoder
	 * @return position of the encoder
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getPosition (int index) throws PhidgetException;
	/**
	 * Returns the last index position of an encoder. This is the position of the last index pulse. Not all encoders support index.
	 * @param index index of the encoder
	 * @return last index position of the encoder
	 * @throws PhidgetException If this Phidget is not opened and attached, if the index is out of range, of if index is not supported. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getIndexPosition (int index) throws PhidgetException;
	/**
	 * @deprecated  Replaced by
	 *              {@link #getPosition}
	 */
	public native int getEncoderPosition (int index) throws PhidgetException;
	/**
	 * Sets the position of a specific encoder. This resets the internal position count for an encoder. This call in no way actually 
	 sends information to the device, as an absolute position is maintained only in the library. After this call, position changes
	 from the encoder will use the new value to calculate absolute position as reported by {@link #getEncoderPosition}.
	 * @param index index of the encoder
	 * @param position new position for this encoder.
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setPosition (int index, int position) throws PhidgetException;
	/**
	 * @deprecated  Replaced by
	 *              {@link #setPosition}
	 */
	public native void setEncoderPosition (int index, int position) throws PhidgetException;
	/**
	 * Returns the state of a digital input. On the mechanical encoder this refers to the pushbutton. The
	 high speed encoder does not have any digital inputs. A value of true means that the input is active(the button is pushed).
	 * @param index index of the input
	 * @return state of the input
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getInputState (int index) throws PhidgetException;
	/**
	 * Returns the enabled state of an encoder. True indicated activated, False deactivated, which is the default.
	 * @param index index of the encoder
	 * @return state of the encoder
	 * @throws PhidgetException If this Phidget is not opened and attached, the index is out of range, or enable is not supported. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getEnabled (int index) throws PhidgetException;
	/**
	 * Sets the enabled state of an encoder. True indicated activated, False deactivated, which is the default.
	 * @param index index of the encoder
	 * @param state desired state
	 * @throws PhidgetException If this Phidget is not opened and attached, the index is out of range, or enable is not supported. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setEnabled (int index, boolean state) throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableEncoderPositionChangeEvents (b && encoderPositionChangeListeners.size () > 0);
		enableInputChangeEvents (b && inputChangeListeners.size () > 0);
	}
	/**
	 * Adds a position change listener. The position change handler is a method that will be called when the position of an 
	 encoder changes. The position change event provides data about how many ticks have occured, and how much time has
	 passed since the last position change event, but does not contain an absolute position. This can be obtained from
	 {@link #getEncoderPosition}.
	 * <p>
	 * There is no limit on the number of position change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.EncoderPositionChangeListener EncoderPositionChangeListener} interface
	 */
	public final void addEncoderPositionChangeListener (EncoderPositionChangeListener l)
	{
		synchronized (encoderPositionChangeListeners)
		{
			encoderPositionChangeListeners.add (l);
			enableEncoderPositionChangeEvents (true);
	}} private LinkedList encoderPositionChangeListeners = new LinkedList ();
	private long nativeEncoderPositionChangeHandler = 0;
	public final void removeEncoderPositionChangeListener (EncoderPositionChangeListener l)
	{
		synchronized (encoderPositionChangeListeners)
		{
			encoderPositionChangeListeners.remove (l);
			enableEncoderPositionChangeEvents (encoderPositionChangeListeners.size () > 0);
	}} private void fireEncoderPositionChange (EncoderPositionChangeEvent e)
	{
		synchronized (encoderPositionChangeListeners)
		{
			for (Iterator it = encoderPositionChangeListeners.iterator (); it.hasNext ();)
				((EncoderPositionChangeListener) it.next ()).encoderPositionChanged (e);
		}
	}
	private native void enableEncoderPositionChangeEvents (boolean b);
	/**
	 * Adds an input change listener. The input change handler is a method that will be called when an input on this
	 * Encoder board has changed.
	 * <p>
	 * There is no limit on the number of input change handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.InputChangeListener InputChangeListener} interface
	 */
	public final void addInputChangeListener (InputChangeListener l)
	{
		synchronized (inputChangeListeners)
		{
			inputChangeListeners.add (l);
			enableInputChangeEvents (true);
	}} private LinkedList inputChangeListeners = new LinkedList ();
	private long nativeInputChangeHandler = 0;
	public final void removeInputChangeListener (InputChangeListener l)
	{
		synchronized (inputChangeListeners)
		{
			inputChangeListeners.remove (l);
			enableInputChangeEvents (inputChangeListeners.size () > 0);
	}} private void fireInputChange (InputChangeEvent e)
	{
		synchronized (inputChangeListeners)
		{
			for (Iterator it = inputChangeListeners.iterator (); it.hasNext ();)
				((InputChangeListener) it.next ()).inputChanged (e);
		}
	}
	private native void enableInputChangeEvents (boolean b);
}
