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

import com.phidgets.Dictionary;

/**
 * This class represents the data for a KeyChangeEvent.
 * 
 * @author Dictionarys Inc.
 */
public class KeyChangeEvent
{
	Dictionary source;
	String value;
	String key;

	/**
	 * Class constructor. This is called internally by the Dictionary library when creating this event.
	 * 
	 * @param source the Dictionary object from which this event originated
	 */
	public KeyChangeEvent(Dictionary source, String key, String value)
	{
		this.source = source;
		this.value = value;
		this.key = key;
	}

	/**
	 * Returns the source Dictionary of this event. This is a reference to the Dictionary object from which this
	 * event was called. This object can be cast into a specific type of Dictionary object to call specific
	 * device calls on it.
	 * 
	 * @return the event caller
	 */
	public Dictionary getSource() {
		return source;
	}

	public String getKey()
	{
		return key;
	}

	public String getValue()
	{
		return value;
	}

	/**
	 * Returns a string containing information about the event.
	 * 
	 * @return an informative event string
	 */
	public String toString()
	{
		return source.toString() + " Key changed: "
		  + key + ":" + value;
	}
}
