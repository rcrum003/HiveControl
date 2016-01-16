
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
 * This class represents a Phidget IR. All methods
 * to send and receive IR data are implemented in this class.
 * <p>
 * The Phidget IR Receiver-Transmitter can send and receive Consumer-IR signals. Ability to learn and re-transmit codes, 
 * as well as low-level access to raw data, is provided.
 * 
 * @author Phidgets Inc.
 */
public final class IRPhidget extends Phidget
{
	public IRPhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;

	/**
	 * Represents a long space (greater then 327,670 microseconds) in raw data. 
	 * This can be considered a period of no IR activity. This is used with {@link #readRaw readRaw}
	 */
	public static final int RAWDATA_LONGSPACE = 0x7fffffff;

	/**
	 * Transmits a code.
	 * @param code the code to transmit
	 * @param codeInfo the code specification
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void transmit (IRCode code, IRCodeInfo codeInfo) throws PhidgetException;
	/**
	 * Transmits a repeat. This needs to be called within the gap time of a transmit to be meaningful.
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void transmitRepeat () throws PhidgetException;
	/**
	 * Transmits raw data.
	 * @param data data in microseconds, must start and end with a pulse
	 * @param offset offset in the data array to start transmitting
	 * @param count number of elements of data to transmit
	 * @param gap gap size in microseconds
	 * @param carrierFrequency carrier frequency in kHz
	 * @param dutyCycle duty cycle in percent
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void transmitRaw (int[]data, int offset, int count, int gap, int carrierFrequency, int dutyCycle) throws PhidgetException;
	/**
	 * Transmits raw data.
	 * @param data data in microseconds, must start and end with a pulse
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public final void transmitRaw (int[]data) throws PhidgetException
	{
		transmitRaw (data, 0, data.length, 0, 0, 0);
	}
	/**
	 * Transmits raw data.
	 * @param data data in microseconds, must start and end with a pulse
	 * @param gap gap size in microseconds
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public final void transmitRaw (int[]data, int gap) throws PhidgetException
	{
		transmitRaw (data, 0, data.length, gap, 0, 0);
	}
	/**
	 * Transmits raw data.
	 * @param data data in microseconds, must start and end with a pulse
	 * @param offset offset in the data array to start transmitting
	 * @param count number of elements of data to transmit
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public final void transmitRaw (int[]data, int offset, int count) throws PhidgetException
	{
		transmitRaw (data, offset, count, 0, 0, 0);
	}
	/**
	 * Transmits raw data.
	 * @param data data in microseconds, must start and end with a pulse
	 * @param offset offset in the data array to start transmitting
	 * @param count number of elements of data to transmit
	 * @param gap gap size in microseconds
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public final void transmitRaw (int[]data, int offset, int count, int gap) throws PhidgetException
	{
		transmitRaw (data, offset, count, gap, 0, 0);
	}
	/**
	 * Reads raw data. Use {@link #RAWDATA_LONGSPACE RAWDATA_LONGSPACE} to detect gaps in IR data.
	 * @param buffer array into which data will be read.
	 * @param offset offset in data to start writing
	 * @param count maximum ammount of data to read
	 * @return ammount of data read
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int readRaw (int[]buffer, int offset, int count) throws PhidgetException;
	/**
	 * Reads raw data. Use {@link #RAWDATA_LONGSPACE RAWDATA_LONGSPACE} to detect gaps in IR data.
	 * @param buffer array into which data will be read.
	 * @return ammount of data read
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public final int readRaw (int[]buffer) throws PhidgetException
	{
		return readRaw (buffer, 0, buffer.length);
	}
	/**
	 * Returns the last received code. This is updated right after the code event returns.
	 * @return last code
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native IRCode getLastCode () throws PhidgetException;
	/**
	 * Returns the last learned code. This is updated right after the learn event returns.
	 * @return last learned code
	 * @throws PhidgetException If this Phidget is not opened and attached. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native IRLearnedCode getLastLearnedCode () throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableCodeEvents (b && codeListeners.size () > 0);
		enableLearnEvents (b && learnListeners.size () > 0);
		enableRawDataEvents (b && rawDataListeners.size () > 0);
	}
	/**
	 * Adds a code listener. The code handler is a method that will be called when a new code is
	 seen by the reader. The event is fired on each code, including repetitions.
	 * <p>
	 * There is no limit on the number of code handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.CodeListener CodeListener} interface
	 */
	public final void addCodeListener (CodeListener l)
	{
		synchronized (codeListeners)
		{
			codeListeners.add (l);
			enableCodeEvents (true);
	}} private LinkedList codeListeners = new LinkedList ();
	private long nativeCodeHandler = 0;
	public final void removeCodeListener (CodeListener l)
	{
		synchronized (codeListeners)
		{
			codeListeners.remove (l);
			enableCodeEvents (codeListeners.size () > 0);
	}} private void fireCode (CodeEvent e)
	{
		synchronized (codeListeners)
		{
			for (Iterator it = codeListeners.iterator (); it.hasNext ();)
				((CodeListener) it.next ()).code (e);
		}
	}
	private native void enableCodeEvents (boolean b);
	/**
	 * Adds a code learn listener. The learn handler is a method that will be called when a new code is
	 learned by the reader. This requires that the code be repeated several times.
	 * <p>
	 * There is no limit on the number of learn handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.LearnListener LearnListener} interface
	 */
	public final void addLearnListener (LearnListener l)
	{
		synchronized (learnListeners)
		{
			learnListeners.add (l);
			enableLearnEvents (true);
	}} private LinkedList learnListeners = new LinkedList ();
	private long nativeLearnHandler = 0;
	public final void removeLearnListener (LearnListener l)
	{
		synchronized (learnListeners)
		{
			learnListeners.remove (l);
			enableLearnEvents (learnListeners.size () > 0);
	}} private void fireLearn (LearnEvent e)
	{
		synchronized (learnListeners)
		{
			for (Iterator it = learnListeners.iterator (); it.hasNext ();)
				((LearnListener) it.next ()).learn (e);
		}
	}
	private native void enableLearnEvents (boolean b);
	/**
	 * Adds a rawData listener. The rawData handler is a method that will be called when a raw IR data is received.
	 * <p>
	 * There is no limit on the number of rawData handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.RawDataListener RawDataListener} interface
	 */
	public final void addRawDataListener (RawDataListener l)
	{
		synchronized (rawDataListeners)
		{
			rawDataListeners.add (l);
			enableRawDataEvents (true);
	}} private LinkedList rawDataListeners = new LinkedList ();
	private long nativeRawDataHandler = 0;
	public final void removeRawDataListener (RawDataListener l)
	{
		synchronized (rawDataListeners)
		{
			rawDataListeners.remove (l);
			enableRawDataEvents (rawDataListeners.size () > 0);
	}} private void fireRawData (RawDataEvent e)
	{
		synchronized (rawDataListeners)
		{
			for (Iterator it = rawDataListeners.iterator (); it.hasNext ();)
				((RawDataListener) it.next ()).rawData (e);
		}
	}
	private native void enableRawDataEvents (boolean b);
}
