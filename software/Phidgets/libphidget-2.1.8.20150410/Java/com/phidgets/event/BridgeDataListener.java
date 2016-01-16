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
/**
 * This interface represents a BridgeDataEvent. This event originates from the Phidget Bridge
 * 
 * @author Phidgets Inc.
 */
public interface BridgeDataListener
{
	/**
	 * This method is called with the event data when a new event arrives. The event is issued at the specified data rate, for 
	 * each enabled bridge.
	 * 
	 * @param ae the event data object containing event data
	 */
	public void bridgeData(BridgeDataEvent ae);
}
