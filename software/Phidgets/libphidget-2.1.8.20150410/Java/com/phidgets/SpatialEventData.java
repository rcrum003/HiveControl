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
 * This class represents an a set of spatial data. It's used in the SpatialData event.
 *
 * @author Phidget Inc.
 */
public final class SpatialEventData
{
	private double[] acceleration;
	private double[] angularRate;
	private double[] magneticField;
	private int timeSeconds, timeMicroSeconds;
	
	/**
	* Creates a new SpatialEventData object.
	* @param acceleration the acceleration data
	* @param angularRate the gyro data
	* @param magneticField the compass data
	* @param timeSeconds the timestamp in seconds
	* @param timeMicroSeconds the time since the last second in microseconds
	*/
	public SpatialEventData(double[] acceleration, double[] angularRate, double[] magneticField, int timeSeconds, int timeMicroSeconds)
	{
		this.acceleration = new double[acceleration.length];
		this.angularRate = new double[angularRate.length];
		this.magneticField = new double[magneticField.length];
		for(int i=0;i<acceleration.length;i++)
			this.acceleration[i] = acceleration[i];
		for(int i=0;i<angularRate.length;i++)
			this.angularRate[i] = angularRate[i];
		for(int i=0;i<magneticField.length;i++)
			this.magneticField[i] = magneticField[i];
		this.timeSeconds = timeSeconds;
		this.timeMicroSeconds = timeMicroSeconds;
	}
	
	/**
	 * Returns the acceleration data.
	 * @return acceleration data
	 */
	public double[] getAcceleration()
	{
		return acceleration;
	}
	
	/**
	 * Returns the angularRate data.
	 * @return angularRate data
	 */
	public double[] getAngularRate()
	{
		return angularRate;
	}
	
	/**
	 * Returns the magneticField data.
	 * @return magneticField data
	 */
	public double[] getMagneticField()
	{
		return magneticField;
	}
	
	/**
	 * Returns the seconds since attach timestamp.
	 * @return whole seconds
	 */
	public int getTimeSeconds()
	{
		return timeSeconds;
	}
	
	/**
	 * Returns the microseconds since last second.
	 * @return microseconds
	 */
	public int getTimeMicroSeconds()
	{
		return timeMicroSeconds;
	}
	
	/**
	 * Returns time since attach in seconds
	 * @return seconds
	 */
	public double getTime()
	{
		return (timeMicroSeconds/1000000.0 + timeSeconds);
	}
	
	public String toString()
	{
		return "Spatial Data";
	}
}

