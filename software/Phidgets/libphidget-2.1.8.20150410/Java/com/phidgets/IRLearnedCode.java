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

package com.phidgets;
/**
 * This class represents an IR Learned Code. This is used in the Learn event.
 *
 * @author Phidget Inc.
 */
public final class IRLearnedCode
{
	private IRCode code;
	private IRCodeInfo codeInfo;
	
	/**
	* Creates a new IR Learned Code. This is used in the Learn event.
	* @param code the IR code
	* @param codeInfo the code parameters
	*/
	public IRLearnedCode(IRCode code, IRCodeInfo codeInfo)
	{
		this.code=code;
		this.codeInfo=codeInfo;
	}
	
	/**
	 * Returns the code.
	 * @return code
	 */
	public IRCode getCode()
	{
		return code;
	}

	/**
	 * Returns the code parameters.
	 * @return code parameters
	 */
	public IRCodeInfo getCodeInfo()
	{
		return codeInfo;
	}
	
	public String toString()
	{
		return code.toString() + " (" + code.getBitCount() + "-bit)\n" + codeInfo.toString();
	}
}

