/*
 * This file is part of libphidget21
 *
 * Copyright © 2006-2015 Phidgets Inc <patrick@phidgets.com>
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

package com.phidgets.event;

import com.phidgets.Phidget;

/**
 * This class represents the data for a FrequencyCounterCountEvent.
 * 
 * @author Phidgets Inc.
 */
public class FrequencyCounterCountEvent
{
	Phidget source;
	int index;
	int time;
	int count;

	/**
	 * Class constructor. This is called internally by the phidget library when creating this event.
	 * 
	 * @param source the Phidget object from which this event originated
	 */
	public FrequencyCounterCountEvent(Phidget source, int index, int time, int count) {
		this.source = source;
		this.index = index;
		this.time = time;
		this.count = count;
	}

	/**
	 * Returns the source Phidget of this event. This is a reference to the Phidget object from which this
	 * event was called. This object can be cast into a specific type of Phidget object to call specific
	 * device calls on it.
	 * 
	 * @return the event caller
	 */
	public Phidget getSource() {
		return source;
	}

	/**
	 * Returns the index of the channel. 
	 * 
	 * @return the index of the channel
	 */
	public int getIndex() {
		return index;
	}

	/**
	 * Returns the amount of time in which the number of counts occured, in microseconds.
	 * 
	 * @return the amount of time in which the number of counts occured
	 */
	public int getTime() {
		return time;
	}

	/**
	 * Returns the number of counts detected. 
	 * 
	 * @return the number of counts detected
	 */
	public int getCount() {
		return count;
	}
	
	/**
	 * Returns a string containing information about the event.
	 * 
	 * @return an informative event string
	 */
	public String toString() {  

		return source.toString() + " Channel " + index + ": " + count + " pulses in "
		  + time + " miroseconds";
	}
}
