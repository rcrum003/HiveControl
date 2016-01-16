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

import java.util.Iterator;
import java.util.LinkedList;
import java.util.Vector;

/**
 * This class represents the Phidget Dictionary.
 * <p>
 * The Phidget Dictionary is a service provided by the Phidget Webservice. 
 * The Webservice maintains a centralized dictionary of key-value pairs that can be accessed and changed from any number of clients.
 * <p>
 * Note that the Webservice uses this dictionary to control access to Phidgets through the openRemote and openRemoteIP interfaces, 
 * and as such, you should never add or modify a key that starts with /PSK/ or /PCK/, unless you want to explicitly modify Phidget 
 * specific data -- and this is highly discouraged, as it's very easy to break things. Listening to these keys is fine if so desired.
 * <p>
 * The intended use for the dictionary is as a central repository for communication and persistent storage of data between several 
 * client applications. As an example - a higher level interface exposed by one application -- which controls the Phidgets, for 
 * others to access -- rather then every client talking directly to the Phidgets themselves.
 * <p>
 * The dictionary makes use of extended regular expressions for key matching.
 * 
 * @author Phidgets Inc.
 */
public class Dictionary
{
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
					+"\nCould not locate the Phidget C library (libphidget21.so)."
					+"\nMake sure it is installed, and add it's path to LD_LIBRARY_PATH.");
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
	
	public long handle = 0;
	/**
	 * The default constructor. Creating a Phidget Dictionary object will initialize the
	 * dictionary handle internally, but {@link #open(String, int) open} still needs to be called
	 * to actually work with it.
	 */
	public Dictionary() throws PhidgetException
	{
		handle = create();
	}
	private final native long create() throws PhidgetException;

	private final native void nativeClose() throws PhidgetException;
	private final native void nativeDelete() throws PhidgetException;
	private final native void nativeOpenRemote(String serverID, String pass) throws PhidgetException;
	private final native void nativeOpenRemoteIP(String ipAddress, int port, String pass) throws PhidgetException;
	private final native void nativeAddKey(String key, String val, int persistent) throws PhidgetException;
	private final native void nativeRemoveKey(String keyPattern) throws PhidgetException;
	private final native String nativeGetKey(String key) throws PhidgetException;


	/**
	 * Returns the Address of a Phidget Webservice when this Dictionary was opened as remote. This may be an IP Address or a hostname.
	 * 
	 * @return Server Address
	 * @throws PhidgetException if this Dictionary was not opened.
	 */
	public final native String getServerAddress() throws PhidgetException;
	/**
	 * Returns the Server ID of a Phidget Webservice when this Dictionary was opened as remote. This is an arbitrary server identifier,
	 * independant of IP address and Port.
	 * 
	 * @return Server ID
	 * @throws PhidgetException if this Dictionary was not opened.
	 */
	public final native String getServerID() throws PhidgetException;
	/**
	 * Returns the Port of a Phidget Webservice when this Dictionary was opened as remote.
	 * 
	 * @return Server Port
	 * @throws PhidgetException if this Dictionary was not opened.
	 */
	public final native int getServerPort() throws PhidgetException;
	/**
	 * Returns the attached status of this Dictionary.
	 * 
	 * @return Status
	 * @throws PhidgetException If this Dictionary is not opened.
	 */
	public final native boolean isAttached() throws PhidgetException;

	/**
	 * Returns the network attached status for remotely opened Phidgets. This method returns True or False, depending on whether a connection to the
	 * Phidget WebService is open - or not. If this is false for a remote Phidget then the connection is not active - either because a connection
	 * has not yet been established, or because the connection was terminated.
	 * 
	 * @return Status
	 * @throws PhidgetException If this Phidget is not opened.
	 */
	public final native boolean isAttachedToServer() throws PhidgetException;

	/**
	 * Adds a new key to the Dictionary, or modifies the value of an existing key.
	 * <p>
	 * The key can only contain numbers, letters, "/", ".", "-", "_", and must begin with a letter, "_" or "/".
	 * <p>
	 * The value can contain any value.
	 * <p>
	 * The key will stay in the dictionary until explicitly removed.
	 * 
	 * 
	 */
	public final void add(String key, String val) throws PhidgetException
	{
		nativeAddKey(key, val, 1);
	}

	/**
	 * Adds a new key to the Dictionary, or modifies the value of an existing key.
	 * <p>
	 * The key can only contain numbers, letters, "/", ".", "-", "_", and must begin with a letter, "_" or "/".
	 * <p>
	 * The value can contain any value.
	 * <p>
	 * The persistent value controls whether a key will stay in the dictionary after the client that created it disconnects. 
	 * If persistent == 0, the key is removed when the connection closes. Otherwise the key remains in the dictionary until 
	 * it is explicitly removed. 
	 * 
	 */
	public final void add(String key, String val, boolean persistent) throws PhidgetException
	{
		if (persistent) nativeAddKey(key, val, 1);
		else nativeAddKey(key, val, 0);
	}
	/**
	 * Removes a key, or set of keys, from the Dictionary.
	 * <p>
	 * The key name is a regular expressions pattern, and so care must be taken to only have it match the specific keys you want to remove. 
	 */
	public final void remove(String pattern) throws PhidgetException
	{
		nativeRemoveKey(pattern);
	}

	/**
	 * Gets the value for a key.
	 */
	public final String get(String key) throws PhidgetException
	{
		return nativeGetKey(key);
	}

	/**
	 * Open this Dictionary remotely and securely, using an IP Address. 
	 * <p>
	 * This method is the same as {@link #open(String, int) open(String ipAddress, int port)}, except that it specifies a password. 
	 * This password can be set as a parameter when starting the Phidget Webservice.
	 */
	public final void open(String ipAddress, int port, String password) throws PhidgetException
	{
		enableEvents(true);
		nativeOpenRemoteIP(ipAddress, port, password);
	}
	/**
	 * Open this Dictionary remotely using an IP Address. 	 
	 * <p>
	 * This version of open is network based, and relies on the Phidget Webservice running at the specified Address and Port
	 * before open is called.
	 * <p>
	 * @param ipAddress IP Address or hostname of the Phidget Webservice
	 * @param port Port of the Phidget Webservice
	 * @throws PhidgetException if the Phidget Webservice cannot be contacted
	 */
	public final void open(String ipAddress, int port) throws PhidgetException
	{
		enableEvents(true);
		nativeOpenRemoteIP(ipAddress, port, "");
	}
	/**
	 * Open this Dictionary remotely and securely, using a Server ID. 
	 * <p>
	 * This method is the same as {@link #open(String) open(String serverID)}, except that it specifies a password. 
	 * This password can be set as a parameter when starting the Phidget Webservice.
	 */
	public final void open(String serverID, String password) throws PhidgetException
	{
		enableEvents(true);
		nativeOpenRemote(serverID, password);
	}
	/**
	 * Open this Dictionary remotely using a Server ID. 	 
	 * <p>
	 * This version of open is network based.
	 * 
	 * @param serverID ServerID of the Phidget Webservice
	 */
	public final void open(String serverID) throws PhidgetException
	{
		enableEvents(true);
		nativeOpenRemote(serverID, "");
	}
	/**
	 * Closes this Dictionary.
	 * This will shut down all threads dealing with this Dictionary and you won't receive any more events.
	 * 
	 * @throws PhidgetException If this Dictionary is not opened. 
	 */
	public final void close() throws PhidgetException
	{
		enableEvents(false);
		nativeClose();
	}

	private void enableEvents(boolean b)
	{
		enableServerConnectEvents(b && serverConnectListeners.size() > 0);
		enableServerDisconnectEvents(b && serverDisconnectListeners.size() > 0);
	}

	private LinkedList serverConnectListeners = new LinkedList();
	private long nativeServerConnectHandler = 0;

	/**
	 * Adds an serverConnect listener. The serverConnect handler is a method that will be called when a connection to a server is made
	 * <p>
	 * There is no limit on the number of serverConnect handlers that can be registered for a particular Manager.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.ServerConnectListener ServerConnectListener} interface
	 */
	public final void addServerConnectListener(ServerConnectListener l)
	{
		synchronized (serverConnectListeners)
		{
			serverConnectListeners.add(l);
			enableServerConnectEvents(true);
		}
	}

	/**
	 * Removes an serverConnect listener. This will remove a previously added serverConnect listener.
	 */
	public final void removeServerConnectListener(ServerConnectListener l)
	{
		synchronized (serverConnectListeners)
		{
			serverConnectListeners.remove(l);
			enableServerConnectEvents(serverConnectListeners.size() > 0);
		}
	}
	private void fireServerConnect(ServerConnectEvent e)
	{
		synchronized (serverConnectListeners)
		{
			for (Iterator it = serverConnectListeners.iterator();
			  it.hasNext(); )
				((ServerConnectListener)it.next()).serverConnected(e);
		}
	}
	private native void enableServerConnectEvents(boolean b);


	private LinkedList serverDisconnectListeners = new LinkedList();
	private long nativeServerDisconnectHandler = 0;

	/**
	 * Adds an serverDisconnect listener. The serverDisconnect handler is a method that will be called when a connection to a server is terminated.
	 * <p>
	 * There is no limit on the number of serverDisconnect handlers that can be registered for a particular Manager.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.ServerDisconnectListener ServerDisconnectListener} interface
	 */
	public final void addServerDisconnectListener(ServerDisconnectListener l)
	{
		synchronized (serverDisconnectListeners)
		{
			serverDisconnectListeners.add(l);
			enableServerDisconnectEvents(true);
		}
	}

	/**
	 * Removes an serverDisconnect listener. This will remove a previously added serverDisconnect listener.
	 */
	public final void removeServerDisconnectListener(ServerDisconnectListener l)
	{
		synchronized (serverDisconnectListeners)
		{
			serverDisconnectListeners.remove(l);
			enableServerDisconnectEvents(serverDisconnectListeners.size() > 0);
		}
	}
	private void fireServerDisconnect(ServerDisconnectEvent e)
	{
		synchronized (serverDisconnectListeners)
		{
			for (Iterator it = serverDisconnectListeners.iterator();
			  it.hasNext(); )
				((ServerDisconnectListener)it.next()).serverDisconnected(e);
		}
	}
	private native void enableServerDisconnectEvents(boolean b);

	/**
	 * Return a Sring describing this dictionary.
	 */
	public String toString()
	{
		return "PhidgetDictionary: ";
	}

	protected void finalize() {
		try
		{
			close();
			nativeDelete();
			handle = 0;
		} catch (Exception e) {
			;
		}
	}
}
