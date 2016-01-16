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

import com.phidgets.event.*;

import com.phidgets.Dictionary;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * This class represents a key listener.
 * <p>
 * This key listener is used, along with the Dictionary object, to set up listener for specific keys, or groups of keys.
 * Events are available for key add or change, and for key removal.
 */
public class DictionaryKeyListener
{
	private String pattern;
	Dictionary dict;

	static
	{
		try
		{
			System.loadLibrary("phidget21");
		}
		catch(UnsatisfiedLinkError ex)
		{
			String os = System.getProperty("os.name");
			if(os.startsWith("Linux"))
			{
				throw new ExceptionInInitializerError(ex.getMessage()
				+"\nCould not locate the Phidget C library."
				+"\nMake sure to compile with 'make jni' and install with 'make install'.");
			}
			else if(os.startsWith("Windows"))
			{
				throw new ExceptionInInitializerError(ex.getMessage()
				+"\nCould not locate the Phidget C library."
				+"\nThe Windows Phidget21 MSI must be installed.");
			}
			else if(os.startsWith("Mac"))
			{
				throw new ExceptionInInitializerError(ex.getMessage()
				+"\nCould not locate the Phidget C library."
				+"\nThe Mac Phidget21 DMG must be installed.");
			}
			else
			{
				throw new ExceptionInInitializerError(ex.getMessage()
				+"\nCould not locate the Phidget C library.");
			}
		}
	}

	private long handle = 0;
	private long nativeHandler = 0;
	public long listenerhandle = 0;

	/**
	 * Start this key listener. This method should not be called until the coresponding dictionary is connected.
	 */
	public void start() throws PhidgetException
	{
		this.handle = dict.handle;
		if (handle == 0)
			throw (new PhidgetException(5, "Dictionary is not attached - call open first"));
		listenerhandle = enableDictionaryKeyEvents((keyChangeListeners.size() > 0) || (keyRemovalListeners.size() > 0), pattern);
	}

	/**
	 * Stop this key listener.
	 */
	public void stop()
	{
		listenerhandle = enableDictionaryKeyEvents(false, pattern);
	}

	/**
	 * Returns the Dictionary object that this listener is listening on.
	 */
	public Dictionary getDictionary()
	{
		return dict;
	}

	/**
	 * Creates a new key listener, for a specific pattern, on a specific dictionary object.
	 * The pattern is a regular expression.
	 */
	public DictionaryKeyListener(Dictionary dict, String pattern)
	{
		this.pattern = pattern;
		this.dict = dict;
		this.handle = dict.handle;
	}

	private LinkedList keyChangeListeners = new LinkedList();
	private long nativeKeyChangeHandler = 0;

	/**
	 * Add a new listener for key change events. This also applies for key add events..
	 */
	public final void addKeyChangeListener(KeyChangeListener l)
	{
		synchronized (keyChangeListeners)
		{
			keyChangeListeners.add(l);
			//enableDictionaryKeyEvents(true, pattern);
		}
	}
	/**
	 * Removes a key change listener.
	 */
	public final void removeKeyChangeListener(KeyChangeListener l)
	{
		synchronized (keyChangeListeners)
		{
			keyChangeListeners.remove(l);
			//enableDictionaryKeyEvents(keyChangeListeners.size() > 0, pattern);
		}
	}
	private void fireKeyChange(KeyChangeEvent e)
	{
		synchronized (keyChangeListeners)
		{
			for (Iterator it = keyChangeListeners.iterator();
			  it.hasNext(); )
				((KeyChangeListener)it.next()).keyChanged(e);
		}
	}
	private native long enableDictionaryKeyEvents(boolean b, String pattern);

	private LinkedList keyRemovalListeners = new LinkedList();
	private long nativeKeyRemovalHandler = 0;

	/**
	 * Add a new listener for key removal events.
	 */
	public final void addKeyRemovalListener(KeyRemovalListener l)
	{
		synchronized (keyRemovalListeners)
		{
			keyRemovalListeners.add(l);
			//enableDictionaryKeyEvents(true, pattern);
		}
	}
	/**
	 * removes a key removal listener.
	 */
	public final void removeKeyRemovalListener(KeyRemovalListener l)
	{
		synchronized (keyRemovalListeners)
		{
			keyRemovalListeners.remove(l);
			//enableDictionaryKeyEvents(keyRemovalListeners.size() > 0, pattern);
		}
	}
	private void fireKeyRemoval(KeyRemovalEvent e)
	{
		synchronized (keyRemovalListeners)
		{
			for (Iterator it = keyRemovalListeners.iterator();
			  it.hasNext(); )
				((KeyRemovalListener)it.next()).keyRemoved(e);
		}
	}

	/**
	 * Return a Sring describing this dictionary key listener.
	 */
	public String toString()
	{
		return dict.toString() + "Dictionary Key Listener ("+pattern+"): ";
	}

	protected void finalize()
	{
		listenerhandle = enableDictionaryKeyEvents(false, pattern);
	}
}
