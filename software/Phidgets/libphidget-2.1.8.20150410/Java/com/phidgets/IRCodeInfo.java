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
 * This class represents IR Code Properties needed for transmit.
 *
 * @author Phidget Inc.
 */
public final class IRCodeInfo
{
	/**
	* Unknown encoding.  This is used with {@link #getEncoding getEncoding}
	*/
	public static final int ENCODING_UNKNOWN = 1;
	/**
	* Space encoding.  This is used with {@link #getEncoding getEncoding}
	*/
	public static final int ENCODING_SPACE = 2;
	/**
	* Pulse encoding.  This is used with {@link #getEncoding getEncoding}
	*/
	public static final int ENCODING_PULSE = 3;
	/**
	* BiPhase encoding.  This is used with {@link #getEncoding getEncoding}
	*/
	public static final int ENCODING_BIPHASE = 4;
	/**
	* RC5 encoding.  This is used with {@link #getEncoding getEncoding}
	*/
	public static final int ENCODING_RC5 = 5;
	/**
	* RC6 encoding.  This is used with {@link #getEncoding getEncoding}
	*/
	public static final int ENCODING_RC6 = 6;
	
	/**
	* Unknown length.  This is used with {@link #getLength getLength}
	*/
	public static final int LENGTH_UNKNOWN = 1;
	/**
	* Constant length.  This is used with {@link #getLength getLength}
	*/
	public static final int LENGTH_CONSTANT = 2;
	/**
	* Variable length.  This is used with {@link #getLength getLength}
	*/
	public static final int LENGTH_VARIABLE = 3;
	
	private int encoding=ENCODING_UNKNOWN, length=LENGTH_UNKNOWN, bitCount=0, gap=0, trail=0, minRepeat=1, carrierFrequency=38000, dutyCycle=33;
	private int[] header=null, one=new int[] {0,0}, zero=new int[] {0,0}, repeat=null;
	private IRCode toggleMask=null;
	
	/**
	* Creates a new IR Code Info object.
	* @param bitCount the bit count
	*/
	public IRCodeInfo(int bitCount) throws PhidgetException
	{
		this.bitCount = bitCount;
	}
	
	/**
	* Creates a new IR Code Info object.
	* @param encoding the encoding - one of {@link #ENCODING_SPACE ENCODING_SPACE}, 
	* {@link #ENCODING_PULSE ENCODING_PULSE}, {@link #ENCODING_BIPHASE ENCODING_BIPHASE}, 
	* {@link #ENCODING_RC5 ENCODING_RC5} or {@link #ENCODING_RC6 ENCODING_RC6}
	* @param bitCount the bit count
	*/
	public IRCodeInfo(int encoding, int bitCount) throws PhidgetException
	{
		this.encoding = encoding;
		this.bitCount = bitCount;
	}
	
	/**
	* Creates a new IR Code Info object.
	* @param encoding the encoding - one of {@link #ENCODING_SPACE ENCODING_SPACE}, 
	* {@link #ENCODING_PULSE ENCODING_PULSE}, {@link #ENCODING_BIPHASE ENCODING_BIPHASE}, 
	* {@link #ENCODING_RC5 ENCODING_RC5} or {@link #ENCODING_RC6 ENCODING_RC6}
	* @param bitCount the bit count
	* @param header the header (pulse,space)
	* @param zero the zero (pulse, space)
	* @param one the one (pulse, space)
	* @param trail the trailing pulse
	* @param gap the trailing gap
	*/
	public IRCodeInfo(int encoding, int bitCount, int[] header, int[] zero, int[] one, int trail, int gap) throws PhidgetException
	{
		this.encoding = encoding;
		this.bitCount = bitCount;
		
		if(header != null)
		{
			if(header.length != 2)
				throw new PhidgetException(PhidgetException.EPHIDGET_INVALIDARG, "header must have 2 elements");
			this.header = new int[] { header[0], header[1] };
		}
		
		if(zero==null || zero.length != 2)
			throw new PhidgetException(PhidgetException.EPHIDGET_INVALIDARG, "zero must have 2 elements");
		this.zero = new int[] { zero[0], zero[1] };
		
		if(one==null || one.length != 2)
			throw new PhidgetException(PhidgetException.EPHIDGET_INVALIDARG, "one must have 2 elements");
		this.one = new int[] { one[0], one[1] };
		
		this.trail = trail;
		this.gap = gap;
	}
	
	/**
	* Creates a new IR Code Info object.
	* @param encoding the encoding - one of {@link #ENCODING_SPACE ENCODING_SPACE}, 
	* {@link #ENCODING_PULSE ENCODING_PULSE}, {@link #ENCODING_BIPHASE ENCODING_BIPHASE}, 
	* {@link #ENCODING_RC5 ENCODING_RC5} or {@link #ENCODING_RC6 ENCODING_RC6}
	* @param bitCount the bit count
	* @param header the header (pulse,space)
	* @param zero the zero (pulse, space)
	* @param one the one (pulse, space)
	* @param trail the trailing pulse
	* @param gap the trailing gap
	* @param repeat the special repeat code
	*/
	public IRCodeInfo(int encoding, int bitCount, int[] header, int[] zero, int[] one, int trail, int gap, int[] repeat) throws PhidgetException
	{
		this.encoding = encoding;
		this.bitCount = bitCount;
		
		if(header != null)
		{
			if(header.length != 2)
				throw new PhidgetException(PhidgetException.EPHIDGET_INVALIDARG, "header must have 2 elements");
			this.header = new int[] { header[0], header[1] };
		}
		
		if(zero==null || zero.length != 2)
			throw new PhidgetException(PhidgetException.EPHIDGET_INVALIDARG, "zero must have 2 elements");
		this.zero = new int[] { zero[0], zero[1] };
		
		if(one==null || one.length != 2)
			throw new PhidgetException(PhidgetException.EPHIDGET_INVALIDARG, "one must have 2 elements");
		this.one = new int[] { one[0], one[1] };
		
		this.trail = trail;
		this.gap = gap;
		
		if(repeat != null)
		{
			this.repeat = new int[repeat.length];
			for(int i=0; i<repeat.length; i++)
				this.repeat[i] = repeat[i];
		}
	}
	
	/**
	* Creates a new IR Code Info object.
	* @param encoding the encoding - one of {@link #ENCODING_SPACE ENCODING_SPACE}, 
	* {@link #ENCODING_PULSE ENCODING_PULSE}, {@link #ENCODING_BIPHASE ENCODING_BIPHASE}, 
	* {@link #ENCODING_RC5 ENCODING_RC5} or {@link #ENCODING_RC6 ENCODING_RC6}
	* @param bitCount the bit count
	* @param header the header (pulse,space)
	* @param zero the zero (pulse, space)
	* @param one the one (pulse, space)
	* @param trail the trailing pulse
	* @param gap the trailing gap
	* @param repeat the special repeat code
	* @param minRepeat the minimum number of times to repeat the code
	* @param toggleMask the bits to toggle when minRepeat is > 1
	* @param length the length style - one of {@link #LENGTH_CONSTANT LENGTH_CONSTANT} or {@link #LENGTH_VARIABLE LENGTH_VARIABLE}
	* @param carrierFrequency the carrier frequency in kHz
	* @param dutyCycle the duty cycle in percent
	*/
	public IRCodeInfo(
		int encoding, 
		int bitCount, 
		int[] header, 
		int[] zero, 
		int[] one, 
		int trail, 
		int gap, 
		int[] repeat, 
		int minRepeat, 
		IRCode toggleMask, 
		int length, 
		int carrierFrequency, 
		int dutyCycle) throws PhidgetException
	{
		this.encoding = encoding;
		this.bitCount = bitCount;
		
		if(header != null)
		{
			if(header.length != 2)
				throw new PhidgetException(PhidgetException.EPHIDGET_INVALIDARG, "header must have 2 elements");
			this.header = new int[] { header[0], header[1] };
		}
		
		if(zero==null || zero.length != 2)
			throw new PhidgetException(PhidgetException.EPHIDGET_INVALIDARG, "zero must have 2 elements");
		this.zero = new int[] { zero[0], zero[1] };
		
		if(one==null || one.length != 2)
			throw new PhidgetException(PhidgetException.EPHIDGET_INVALIDARG, "one must have 2 elements");
		this.one = new int[] { one[0], one[1] };
		
		this.trail = trail;
		this.gap = gap;
		
		if(repeat != null)
		{
			this.repeat = new int[repeat.length];
			for(int i=0; i<repeat.length; i++)
				this.repeat[i] = repeat[i];
		}
		
		this.minRepeat = minRepeat;
		
		if(toggleMask != null)
			toggleMask = new IRCode(toggleMask.getData(), toggleMask.getBitCount());
			
		this.length = length;
		this.carrierFrequency = carrierFrequency;
		this.dutyCycle = dutyCycle;
	}
	
	/**
	 * Returns the encoding.
	 * @return encoding
	 */
	public int getEncoding()
	{
		return encoding;
	}
	/**
	 * Returns the length style.
	 * @return length style
	 */
	public int getLength()
	{
		return length;
	}
	/**
	 * Returns the bit count.
	 * @return bit count
	 */
	public int getBitCount()
	{
		return bitCount;
	}
	/**
	 * Returns the gap.
	 * @return gap
	 */
	public int getGap()
	{
		return gap;
	}
	/**
	 * Returns the trail.
	 * @return trail
	 */
	public int getTrail()
	{
		return trail;
	}
	/**
	 * Returns the min repeat.
	 * @return min repeat
	 */
	public int getMinRepeat()
	{
		return minRepeat;
	}
	/**
	 * Returns the carrier frequency.
	 * @return carrier frequency
	 */
	public int getCarrierFrequency()
	{
		return carrierFrequency;
	}
	/**
	 * Returns the duty cycle.
	 * @return duty cycle
	 */
	public int getDutyCycle()
	{
		return dutyCycle;
	}
	/**
	 * Returns the header.
	 * @return header
	 */
	public int[] getHeader()
	{
		return header;
	}
	/**
	 * Returns the zero.
	 * @return zero
	 */
	public int[] getZero()
	{
		return zero;
	}
	/**
	 * Returns the one.
	 * @return one
	 */
	public int[] getOne()
	{
		return one;
	}
	/**
	 * Returns the repeat code.
	 * @return repeat code
	 */
	public int[] getRepeat()
	{
		return repeat;
	}
	/**
	 * Returns the toggle mask.
	 * @return toggle mask
	 */
	public IRCode getToggleMask()
	{
		return toggleMask;
	}
	
	
	public String toString()
	{
		String out = "";
		out = out + "  Encoding: " + encoding + "\n";
		out = out + "  Zero: " + zero[0] + ", " + zero[1] + "\n";
		out = out + "  One: " + one[0] + ", " + one[1] + "\n";
		if (header != null)
		{
			if (header[0] != 0)
				out = out + "  Header: " + header[0] + ", " + header[1] + "\n";
		}
		
		if (trail != 0)
			out = out + "  Trail: " + trail + "\n";
		if (gap != 0)
			out = out + "  Gap: " + gap + "\n";
		if (repeat != null)
		{
			out = out + "  Repeat: ";
			for(int i=0;i<repeat.length;i++)
				out = out + repeat[i] + ", ";
			out = out + "\n";
		}
		return out;
	}
}
