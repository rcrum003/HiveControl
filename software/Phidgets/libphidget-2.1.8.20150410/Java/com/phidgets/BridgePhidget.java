
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
 * This class represents a Phidget Bridge. All methods
 * to manipulate the Phidget Bridge are implemented in this class.
 * <p>
 * 
 * @author Phidgets Inc.
 */
public final class BridgePhidget extends Phidget
{
	public BridgePhidget () throws PhidgetException
	{
		super (create ());
	}
	private static native long create () throws PhidgetException;
	/**
	 * 1 Gain. This is used with {@link #getGain(int) getGain} and {@link #setGain(int, int) setGain}
	 */
	public static final int PHIDGET_BRIDGE_GAIN_1 = 1;
	/**
	 * 8 Gain. This is used with {@link #getGain(int) getGain} and {@link #setGain(int, int) setGain}
	 */
	public static final int PHIDGET_BRIDGE_GAIN_8 = 2;
	/**
	 * 16 Gain. This is used with {@link #getGain(int) getGain} and {@link #setGain(int, int) setGain}
	 */
	public static final int PHIDGET_BRIDGE_GAIN_16 = 3;
	/**
	 * 32 Gain. This is used with {@link #getGain(int) getGain} and {@link #setGain(int, int) setGain}
	 */
	public static final int PHIDGET_BRIDGE_GAIN_32 = 4;
	/**
	 * 64 Gain. This is used with {@link #getGain(int) getGain} and {@link #setGain(int, int) setGain}
	 */
	public static final int PHIDGET_BRIDGE_GAIN_64 = 5;
	/**
	 * 128 Gain. This is used with {@link #getGain(int) getGain} and {@link #setGain(int, int) setGain}
	 */
	public static final int PHIDGET_BRIDGE_GAIN_128 = 6;
	/**
	 * Unknown Gain. This is used with {@link #getGain(int) getGain} and {@link #setGain(int, int) setGain}
	 */
	public static final int PHIDGET_BRIDGE_GAIN_UNKNOWN = 7;
	/**
	 * Returns the number of bridges.
	 * @return Number of bridges
	 */
	public native int getInputCount () throws PhidgetException;
	/**
	 * Returns the value of the selected input, in mV/V. If the input is not enabled, an EPHIDGET_UNKNOWNVAL exception will be thrown. If the bridge is saturated
	 *, this will be equal to BridgeMax or BridgeMin and an error event will be fired - in this case, gain should be reduced if possible.
	 * @param index Index of the bridge
	 * @return bridge value
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getBridgeValue (int index) throws PhidgetException;
		/**
	 * Returns the minimum value that the selected bridge can measure, in mV/V. This value will depend on the selected gain. At a gain of 1, BridgeMin == -1000 mV/V.
	 * @param index Index of the bridge
	 * @return minimum value 
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getBridgeMin (int index) throws PhidgetException;
	/**
	 * Returns the maximum value that the selected bridge can measure, in mV/V. This value will depend on the selected gain. At a gain of 1, BridgeMin == 1000 mV/V.
	 * @param index Index of the bridge
	 * @return maximum value 
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is out of range. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native double getBridgeMax (int index) throws PhidgetException;
	/**
	 * Returns the enabled state of the specified bridge. This applies power between +5v and Ground, and starts measuring the differential on the +/- pins. By default, all bridges are 
	 * disabled, and need to be explicitly enabled on startup.
	 * @param index Index of the bridge
	 * @return state of the specified bridge
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native boolean getEnabled (int index) throws PhidgetException;
	/**
	 * Sets the enabled state of the specified bridge. This applies power between +5v and Ground, and starts measuring the differential on the +/- pins. By default, all bridges are 
	 * disabled, and need to be explicitly enabled on startup.
	 * @param index Index of the bridge
	 * @param state new state of the specified bridge
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setEnabled (int index, boolean state) throws PhidgetException;
	/**
	 * Returns the gain for the selected bridge.  Note that increasing the gains will reduce the measurable
	 * voltage difference by the gain factor, with +-1000 mV/V being the maximum, with no gain.
	 * The possible values for gain are {@link #PHIDGET_BRIDGE_GAIN_1 PHIDGET_BRIDGE_GAIN_1},
	 * {@link #PHIDGET_BRIDGE_GAIN_8 PHIDGET_BRIDGE_GAIN_8}, {@link #PHIDGET_BRIDGE_GAIN_16 PHIDGET_BRIDGE_GAIN_16},
	 * {@link #PHIDGET_BRIDGE_GAIN_32 PHIDGET_BRIDGE_GAIN_32}, {@link #PHIDGET_BRIDGE_GAIN_64 PHIDGET_BRIDGE_GAIN_64},
	 * {@link #PHIDGET_BRIDGE_GAIN_128 PHIDGET_BRIDGE_GAIN_128}, {@link #PHIDGET_BRIDGE_GAIN_UNKNOWN PHIDGET_BRIDGE_GAIN_UNKNOWN}
	 * <p>
	 * @param index Index of the bridge
	 * @return gain
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getGain (int index) throws PhidgetException;
	/**
	 * Sets the gain for the selected bridge.  Note that increasing the gains will reduce the measurable
	 * voltage difference by the gain factor, with +-1000 mV/V being the maximum, with no gain.
	 * <p>
	 * The possible values for gain are {@link #PHIDGET_BRIDGE_GAIN_1 PHIDGET_BRIDGE_GAIN_1},
	 * {@link #PHIDGET_BRIDGE_GAIN_8 PHIDGET_BRIDGE_GAIN_8}, {@link #PHIDGET_BRIDGE_GAIN_16 PHIDGET_BRIDGE_GAIN_16},
	 * {@link #PHIDGET_BRIDGE_GAIN_32 PHIDGET_BRIDGE_GAIN_32}, {@link #PHIDGET_BRIDGE_GAIN_64 PHIDGET_BRIDGE_GAIN_64},
	 * {@link #PHIDGET_BRIDGE_GAIN_128 PHIDGET_BRIDGE_GAIN_128}, {@link #PHIDGET_BRIDGE_GAIN_UNKNOWN PHIDGET_BRIDGE_GAIN_UNKNOWN}
	 * <p>
	 * @param index Index of the bridge
	 * @param gain new gain
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setGain (int index, int gain) throws PhidgetException;
	/**
	 * Returns the data rate, in ms. Data rate applies to all 4 bridges simultaneously. Setting a slower data rate will reduce
	 * noise at the cost of sample time. Also note that each bridge is being sampled only 1/4 of the time - this is okay for very stable signals,
	 * but for changing signals, it is best to set a higher sampling rate and do averaging in the software. 
	 * <p>
	 * Data rate must be a multiple of 8ms. Trying to set something between multiplies of 8 will cause an EPHIDGET_INVALIDARG exception.
	 * <p>
	 * @return Data Rate
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the index is invalid. 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getDataRate () throws PhidgetException;
	/**
	 * Sets the data rate, in ms. Data rate applies to all 4 bridges simultaneously. Setting a slower data rate will reduce
	 * noise at the cost of sample time. Also note that each bridge is being sampled only 1/4 of the time - this is okay for very stable signals,
	 * but for changing signals, it is best to set a higher sampling rate and do averaging in the software.
	 * <p>
	 * Data rate must be a multiple of 8ms. Trying to set something between multiplies of 8 will cause an EPHIDGET_INVALIDARG exception.
	 * <p>
	 * @param rate new data rate
	 * @throws PhidgetException If this Phidget is not opened and attached, or if the data rate is out of range 
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native void setDataRate (int rate) throws PhidgetException;
	/**
	 * Gets the minimum supported data rate, in ms.
	 * @return minimum supported data rate
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getDataRateMin () throws PhidgetException;
	/**
	 * Gets the maximum supported data rate, in ms.
	 * @return maximum supported data rate
	 * @throws PhidgetException If this Phidget is not opened and attached.
	 * See {@link com.phidgets.Phidget#open(int) open} for information on determining if a device is attached.
	 */
	public native int getDataRateMax () throws PhidgetException;

	private final void enableDeviceSpecificEvents (boolean b)
	{
		enableBridgeDataEvents (b && bridgeDataListeners.size () > 0);
	}
	/**
	 * Adds a bridge data listener. The bridge data handler is a method that will be called at the specified data rate, for each enabled bridge. Value is the bridgeValue, in mV/V.
	 * <p>
	 * There is no limit on the number of bridge data handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.BridgeDataListener BridgeDataListener} interface
	 */
	public final void addBridgeDataListener (BridgeDataListener l)
	{
		synchronized (bridgeDataListeners)
		{
			bridgeDataListeners.add (l);
			enableBridgeDataEvents (true);
	}} private LinkedList bridgeDataListeners = new LinkedList ();
	private long nativeBridgeDataHandler = 0;
	public final void removeBridgeDataListener (BridgeDataListener l)
	{
		synchronized (bridgeDataListeners)
		{
			bridgeDataListeners.remove (l);
			enableBridgeDataEvents (bridgeDataListeners.size () > 0);
	}} private void fireBridgeData (BridgeDataEvent e)
	{
		synchronized (bridgeDataListeners)
		{
			for (Iterator it = bridgeDataListeners.iterator (); it.hasNext ();)
				((BridgeDataListener) it.next ()).bridgeData (e);
		}
	}
	private native void enableBridgeDataEvents (boolean b);
}
