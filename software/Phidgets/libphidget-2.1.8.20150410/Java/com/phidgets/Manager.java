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
 * <p>
 * This class represents a Phidget Manager.
 * The Phidget manager is a way to keep track of attached phidgets, it will send
 * Attach and Detach events as Phidgets are added and removed fromt the system.
 * <p>
 * The Phidget manager deals in base Phidget objects. These objects are not actually connected to
 * opened Phidgets but can be used to get serial number, name, version, etc.
 * 
 * @author Phidgets Inc.
 */
public class Manager
{
	private Vector phidgets;

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

	private long handle = 0;
	/**
	 * The default constructor. Creating a Phidget Manager object will initialize the
	 * attach and detach handlers internally, but {@link #open() open} still needs to be called
	 * to actually receive event notifications.
	 */
	public Manager() throws PhidgetException
	{
		handle = create();
		phidgets = new Vector();
		addAttachListener(new AttachListener()
		{
			public void attached(AttachEvent ae)
			{
				phidgets.add(ae.getSource());
			}
		});
		addDetachListener(new DetachListener()
		{
			public void detached(DetachEvent ae)
			{
				phidgets.remove(ae.getSource());
			}
		});
	}
	private final native long create() throws PhidgetException;

	/**
	 * Returns the Address of a Phidget Webservice when this Manager was opened as remote. This may be an IP Address or a hostname.
	 * 
	 * @return Server Address
	 * @throws PhidgetException if this Manager was not opened.
	 */
	public final native String getServerAddress() throws PhidgetException;
	/**
	 * Returns the Server ID of a Phidget Webservice when this Manager was opened as remote. This is an arbitrary server identifier,
	 * independant of IP address and Port.
	 * 
	 * @return Server ID
	 * @throws PhidgetException if this Manager was not opened.
	 */
	public final native String getServerID() throws PhidgetException;
	/**
	 * Returns the Port of a Phidget Webservice when this Manager was opened as remote.
	 * 
	 * @return Server Port
	 * @throws PhidgetException if this Manager was not opened.
	 */
	public final native int getServerPort() throws PhidgetException;
	/**
	 * Returns the attached status of this Manager.
	 * 
	 * @return Status
	 * @throws PhidgetException If this Manager is not opened.
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

	private final native void nativeClose() throws PhidgetException;
	private final native void nativeDelete() throws PhidgetException;
	private final native void nativeOpen() throws PhidgetException;
	private final native void nativeOpenRemote(String serverID, String pass) throws PhidgetException;
	private final native void nativeOpenRemoteIP(String ipAddress, int port, String pass) throws PhidgetException;

	/**
	 * Starts the PhidgetManager. This method starts the phidget manager running in the base Phidget21 C library.
	 * If attach and detach listeners are to be used, they should be registered before start is called so
	 * that no events are missed. Once start is called, the Phidget Manager will be active until {@link #close() close} is called.
	 */
	public final void open() throws PhidgetException {
		enableEvents(true);
		nativeOpen();
	}
	/**
	 * Open this Manager remotely and securely, using an IP Address. 
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
	 * Open this Manager remotely using an IP Address. 	 
	 * <p>
	 * This version of open is network based.
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
	 * Open this Manager remotely and securely, using a Server ID. 
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
	 * Open this Manager remotely using a Server ID. 	 
	 * <p>
	 * This version of open is network based.
	 * <p>
	 * ServerID can be NULL to get a listing of all Phidgets on all Servers
	 * 
	 * @param serverID ServerID of the Phidget Webservice
	 */
	public final void open(String serverID) throws PhidgetException
	{
		enableEvents(true);
		nativeOpenRemote(serverID, "");
	}

	/**
	 * Shuts down the Phidget Manager. This method should be called to close down the Phidget Manager.
	 * Events will no longer be received. This method gets called automatically when the class is
	 * destroyed so calling it is not required.
	 */
	public final void close() throws PhidgetException {
		enableEvents(false);
		nativeClose();
	}

	private void enableEvents(boolean b)
	{
		enableManagerAttachEvents(b && attachListeners.size() > 0);
		enableManagerDetachEvents(b && detachListeners.size() > 0);
		enableServerConnectEvents(b && serverConnectListeners.size() > 0);
		enableServerDisconnectEvents(b && serverDisconnectListeners.size() > 0);
	}

	private LinkedList attachListeners = new LinkedList();
	private long nativeAttachHandler = 0;

	/**
	 * Adds an attach listener. The attach handler is a method that will be called when a Phidget is phisically attached
	 * to the system, and has gone through its initalization, and so is ready to be used.
	 * <p>
	 * There is no limit on the number of attach handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.AttachListener AttachListener} interface
	 */
	public final void addAttachListener(AttachListener l) {
		synchronized (attachListeners) {
			attachListeners.add(l);
			enableManagerAttachEvents(true);
		}
	}

	/**
	 * Removes an attach listener. This will remove a previously added attach listener.
	 */
	public final void removeAttachListener(AttachListener l) {
		synchronized (attachListeners) {
			attachListeners.remove(l);
			enableManagerAttachEvents(attachListeners.size() > 0);
		}
	}
	private void fireAttach(AttachEvent e) {
		synchronized (attachListeners) {
			for (Iterator it = attachListeners.iterator();
			  it.hasNext(); )
				((AttachListener)it.next()).attached(e);
		}
	}
	private native void enableManagerAttachEvents(boolean b);

	private LinkedList detachListeners = new LinkedList();
	private long nativeDetachHandler = 0;

	/**
	 * Adds a detach listener. The detach handler is a method that will be called when a Phidget is phisically detached
	 * from the system, and is no longer available.
	 * <p>
	 * There is no limit on the number of detach handlers that can be registered for a particular Phidget.
	 * 
	 * @param l An implemetation of the {@link com.phidgets.event.DetachListener DetachListener} interface
	 */
	public final void addDetachListener(DetachListener l) {
		synchronized (detachListeners) {
			detachListeners.add(l);
			enableManagerDetachEvents(true);
		}
	}

	/**
	 * Removes a detach listener. This will remove a previously added detach listener.
	 */
	public final void removeDetachListener(DetachListener l) {
		synchronized (detachListeners) {
			detachListeners.remove(l);
			enableManagerDetachEvents(detachListeners.size() > 0);
		}
	}
	private void fireDetach(DetachEvent e) {
		synchronized (detachListeners) {
			for (Iterator it = detachListeners.iterator();
			  it.hasNext(); )
			((DetachListener)it.next()).detached(e);
		}
	}
	private native void enableManagerDetachEvents(boolean b);

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

	protected void finalize() throws Throwable
	{
		try
		{
			close();
		} catch (Exception e) {
			;
		}
		try
		{
			nativeDelete();
		}
		catch (Exception e)
		{
			;
		}
		finally{
			handle = 0;
			super.finalize();
		}
	}

	/**
	 * Return a Sring describing this manager.
	 */
	public String toString()
	{
		return "PhidgetManager: ";
	}

	/**
	 * Returns a list of Phidgets attached to the host computer. This list is updated right before the attach and detach
	 * events, and so will be up to date within these events.
	 * @return list of attached Phidgets
	 */
	public Vector getPhidgets()
	{
		return phidgets;
	}
}
