/*
 * This file is part of libphidget21
 *
 * Copyright 2006-2015 Phidgets Inc <patrick@phidgets.com>
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

#include "stdafx.h"
#include "cusb.h"
#include <usb.h>

int CUSBGetDeviceCapabilities(CPhidgetHandle phid, struct usb_device *dev, struct usb_dev_handle *udev);

void CUSBUninit()
{}

int CUSBCloseHandle(CPhidgetHandle phid) {
	int ret = 0;
	int result = EPHIDGET_OK;
	

	if (!phid)
		return EPHIDGET_INVALIDARG;

	CPhidget_clearStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
	
	if (phid->deviceHandle == NULL)
		return EPHIDGET_NOTATTACHED;

	CThread_join(&phid->readThread);
		
	if((ret = usb_release_interface((usb_dev_handle *) phid->deviceHandle, phid->deviceDef->pdd_iid)) < 0)
	{
		switch(ret)
		{
		case -ENODEV:
			//usb_release_interface called after the device was unplugged
			LOG(PHIDGET_LOG_WARNING, "usb_release_interface called on unplugged device.");
			break;
		default:
			LOG(PHIDGET_LOG_ERROR, "usb_release_interface failed with error code: %d \"%s\"", ret, strerror(-ret));
		}
	}
	
	//if we notice that PHIDGET_USB_ERROR_FLAG is set, then reset this device before closing
	//this gives us a better chance of getting it back if something has gone wrong.
	if(CPhidget_statusFlagIsSet(phid->status, PHIDGET_USB_ERROR_FLAG))
	{
		LOG(PHIDGET_LOG_WARNING,"PHIDGET_USB_ERROR_FLAG is set - resetting device.");
		if((ret = usb_reset((usb_dev_handle *) phid->deviceHandle)) < 0)
		{
			LOG(PHIDGET_LOG_ERROR, "usb_reset failed with error code: %d \"%s\"", ret, strerror(-ret));
			result = EPHIDGET_UNEXPECTED;
		}
	}
	
	if((ret = usb_close((usb_dev_handle *) phid->deviceHandle)) < 0)
	{
		LOG(PHIDGET_LOG_ERROR, "usb_close failed with error code: %d \"%s\"", ret, strerror(-ret));
		result = EPHIDGET_UNEXPECTED;
	}
	
	phid->deviceHandle = NULL;
	
	return result;
}

int CUSBSendPacket(CPhidgetHandle phid, unsigned char *buffer) {
	int BytesWritten = 0;

	if (!phid)
		return EPHIDGET_INVALIDARG;
		
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
		&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHING_FLAG))
		return EPHIDGET_NOTATTACHED;
		
	if (phid->deviceHandle == NULL)
	{
		LOG(PHIDGET_LOG_WARNING,"Handle for writing is not valid");
		return EPHIDGET_UNEXPECTED;
	}
		
	if(phid->interruptOutEndpoint)
	{
		BytesWritten = usb_interrupt_write((usb_dev_handle *)phid->deviceHandle, 
										   phid->deviceDef->pdd_iid+1, 
										   (char *)buffer, 
										   phid->outputReportByteLength, /* size */
										   500); /* FIXME? timeout */
	}
	else
	{
		BytesWritten = usb_control_msg((usb_dev_handle *)phid->deviceHandle, 
						USB_ENDPOINT_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
						USB_REQ_SET_CONFIGURATION,
						0x0200, /* value */
						phid->deviceDef->pdd_iid, /* index*/
						(char *)buffer, 
						phid->outputReportByteLength, /* size */
						500); /* FIXME? timeout */
	}
					
	if(BytesWritten < 0)
	{
		switch(BytesWritten)
		{
		case -ETIMEDOUT: //important case?
				return EPHIDGET_TIMEOUT;
		case -ENODEV:
			//device is gone - unplugged.
			LOG(PHIDGET_LOG_INFO, "Device was unplugged - detach.");
			return EPHIDGET_NOTATTACHED;
		default:
			LOG(PHIDGET_LOG_ERROR, "usb_control_msg failed with error code: %d \"%s\"", BytesWritten, strerror(-BytesWritten));
			return EPHIDGET_UNEXPECTED;
		}
	}

	if (BytesWritten != phid->outputReportByteLength)
	{
		LOG(PHIDGET_LOG_WARNING,"Failure in CUSBSendPacket - Report Length"
			": %d, bytes written: %d",
		    (int)phid->outputReportByteLength, (int)BytesWritten);
		return EPHIDGET_UNEXPECTED;
	}
	
	return EPHIDGET_OK;
}

int CUSBSetLabel(CPhidgetHandle phid, char *buffer) {
	if(deviceSupportsGeneralUSBProtocol(phid))
	{
		return CPhidgetGPP_setLabel(phid, buffer);
	}
	else
	{
		int BytesWritten = 0;
		int size = buffer[0];
		
		if(size>22) return EPHIDGET_INVALID;
		
		if (!phid)
			return EPHIDGET_INVALIDARG;
			
		if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
			&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHING_FLAG))
			return EPHIDGET_NOTATTACHED;
			
		if (phid->deviceHandle == NULL)
		{
			LOG(PHIDGET_LOG_WARNING,"Handle for writing is not valid");
			return EPHIDGET_UNEXPECTED;
		}
			
		BytesWritten = usb_control_msg(phid->deviceHandle, 
						USB_ENDPOINT_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE,
						USB_REQ_SET_DESCRIPTOR,
						0x0304, /* value */
						0x0409, /* index*/
						(char *)buffer, 
						size, /* size */
						500); /* FIXME? timeout */
						
		if(BytesWritten < 0)
		{
			switch(BytesWritten)
			{
			case -ETIMEDOUT: //important case?	
			default:
				LOG(PHIDGET_LOG_INFO, "usb_control_msg failed with error code: %d \"%s\"", BytesWritten, strerror(-BytesWritten));
				return EPHIDGET_UNSUPPORTED;
			}
		}

		if (BytesWritten != size)
		{
			LOG(PHIDGET_LOG_WARNING,"Failure in CUSBSetLabel - Report Length"
				": %d, bytes written: %d",
				size, (int)BytesWritten);
			return EPHIDGET_UNEXPECTED;
		}
		
		return EPHIDGET_OK;
	}
}

/* Buffer should be at least 8 bytes long */
int CUSBReadPacket(CPhidgetHandle phid, unsigned char *buffer) {
	int BytesRead = 0;
	
	if (!phid)
		return EPHIDGET_INVALIDARG;
		
	if (!CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHED_FLAG)
		&& !CPhidget_statusFlagIsSet(phid->status, PHIDGET_ATTACHING_FLAG))
		return EPHIDGET_NOTATTACHED;
		
	if (phid->deviceHandle == NULL)
	{
		LOG(PHIDGET_LOG_WARNING,"Handle for writing is not valid");
		return EPHIDGET_UNEXPECTED;
	}
	
	BytesRead = usb_interrupt_read((usb_dev_handle *)phid->deviceHandle, 
				phid->deviceDef->pdd_iid+1, 
				(char *)buffer, 
				phid->inputReportByteLength, 
				500);
				
	if (BytesRead < 0)
	{
		switch(BytesRead)
		{
			// A timeout occured, but we'll just try again
			case -ETIMEDOUT:
				LOG(PHIDGET_LOG_VERBOSE, "usb_interrupt_read timeout: %d \"%s\"", BytesRead, strerror(-BytesRead));
				return EPHIDGET_TIMEOUT;
			case -EBUSY:
				//This happens when someone else calls claim_interface on this interface (a manager for ex.) - basically just wait until they release it.
				//This will happen if an open occurs in another app which (for some reason) can steal the interface from this one.
				LOG(PHIDGET_LOG_INFO, "Device is busy on Read - try again.");
				return EPHIDGET_TRYAGAIN;
			case -ENOSPC:
				//This happens when too many Interrupt devices are plugged in (Phidgets, HID devices, etc.
				LOG(PHIDGET_LOG_ERROR, "USB Interrupt bandwidth exceeded. Try distributing devices more evenly. Reads will continue, but data is being lost.");
				goto tryagain;
			case -ENODEV:
				//device is gone - unplugged.
				LOG(PHIDGET_LOG_INFO, "Device was unplugged - detach.");
				return EPHIDGET_NOTATTACHED;
			default:
				LOG(PHIDGET_LOG_ERROR, "usb_interrupt_read returned: %d \"%s\"", BytesRead, strerror(-BytesRead));
				goto tryagain;
		}
	}

	if (BytesRead != phid->inputReportByteLength)
	{
		//Generally means the device was unplugged, but can mean that there is not enough Interrupt bandwidth
		//We keep trying and we'll get data, just not all data
		LOG(PHIDGET_LOG_WARNING,"Failure in CUSBReadPacket - Report Length"
			": %d, bytes read: %d. Probably trying to use too many Phidgets at once, and some data is being lost.",
		    (int)phid->inputReportByteLength, (int)BytesRead);
		goto tryagain;
	}
	
	phid->tryAgainCounter = 0;
	return EPHIDGET_OK;
	
	//if we see too many tryagains in a row, then we assume something has actually gone wrong and reset the device
tryagain:
	phid->tryAgainCounter++;
	if(phid->tryAgainCounter > 30) //this will be hit in < 1 second for all devices
	{
		LOG(PHIDGET_LOG_ERROR, "CUSBReadPacket returned EPHIDGET_TRYAGAIN too many times in a row - reset device.", BytesRead, strerror(-BytesRead));
		phid->tryAgainCounter = 0;
		return EPHIDGET_UNEXPECTED;
	}
	return EPHIDGET_TRYAGAIN;
}

static int getLabelString(CPhidgetHandle phid, struct usb_dev_handle *udev)
{	
	int len = 0;
	char labelBuf[22];
	struct usb_device *dev;
	memset(labelBuf, 0, sizeof(labelBuf));
	
	dev = usb_device(udev);
	
	if(dev->descriptor.iSerialNumber == 3)
	{
		//Note that this returns the whole descriptor, including the length and type bytes
		len = usb_get_string(udev, 4, 0, (char *)labelBuf, 22);
		
		if(len < 0)
		{
			switch(len)
			{
				case -ETIMEDOUT: //important case?
				default:
					LOG(PHIDGET_LOG_INFO, "usb_get_string_simple failed in CUSBGetDeviceCapabilities with error code: %d \"%s\" while reading label - this probably just means the device doesn't support labels, so this is fine.", len, strerror(-len));
			}
			phid->label[0]='\0';
			return EPHIDGET_OK;
		}
		else
			return decodeLabelString(labelBuf, phid->label, phid->serialNumber);
	}
	phid->label[0]='\0';
	return EPHIDGET_OK;
}

int CUSBRefreshLabelString(CPhidgetHandle phid)
{
	return getLabelString(phid, (struct usb_dev_handle *)phid->deviceHandle);
}

int CUSBGetDeviceCapabilities(CPhidgetHandle phid, struct usb_device *dev, struct usb_dev_handle *udev) {
	unsigned char buf[255];
	int len = 0, i = 0;
	struct usb_interface_descriptor *interfaceDesc;
	struct usb_config_descriptor configDesc;
	
	memset(buf, 0, sizeof(buf));
	
//	if(usb_device->config->interface->bNumEndpoints == 2)
//	{
//		LOG(PHIDGET_LOG_INFO, "Using Interrupt OUT endpoing to Host->Device communication.");
//		phid->interruptOutEndpoint = PTRUE;
//	}
//	else
//	{
//		LOG(PHIDGET_LOG_INFO, "Using Control endpoing to Host->Device communication.");
//		phid->interruptOutEndpoint = PFALSE;
//	}
	
	//Get config descriptor
	len = usb_get_descriptor(udev, USB_DT_CONFIG, 0, (void *)&configDesc, USB_DT_CONFIG_SIZE);
	
	if(len == USB_DT_CONFIG_SIZE)
	{
		//Get the rest
		len = usb_get_descriptor(udev, USB_DT_CONFIG, 0, (void *)buf, configDesc.wTotalLength);
		
		if(len == configDesc.wTotalLength)
		{
			int i;
			interfaceDesc = NULL;
			struct usb_descriptor_header *currentDesc;
			
			//Find the interface Descriptor
			for(i=0;i<configDesc.wTotalLength-2;i+=currentDesc->bLength)
			{
				currentDesc = (struct usb_descriptor_header *)&buf[i];
				if (currentDesc->bDescriptorType == USB_DT_INTERFACE)
				{
					interfaceDesc = (struct usb_interface_descriptor *)&buf[i];
					if(interfaceDesc->bInterfaceNumber == phid->deviceDef->pdd_iid)
						break;
					else
						interfaceDesc = NULL;
				}
			}
			
			if(interfaceDesc == NULL)
			{
				LOG(PHIDGET_LOG_ERROR, "Couldn't find interface descriptor!");
				return EPHIDGET_UNEXPECTED;
			}
			
			if(interfaceDesc->bNumEndpoints == 2)
			{
				LOG(PHIDGET_LOG_INFO, "Using Interrupt OUT Endpoint for Host->Device communication.");
				phid->interruptOutEndpoint = PTRUE;
			}
			else
			{
				LOG(PHIDGET_LOG_INFO, "Using Control Endpoint for Host->Device communication.");
				phid->interruptOutEndpoint = PFALSE;
			}
		}
		else if (len < 0)
		{
			LOG(PHIDGET_LOG_ERROR, "usb_get_descriptor failed in CUSBGetDeviceCapabilities with error code: %d \"%s\"", len, strerror(-len));
			return EPHIDGET_UNEXPECTED;
		}
		else
		{
			LOG(PHIDGET_LOG_ERROR, "Couldn't get interface descriptor in CUSBGetDeviceCapabilities");
			return EPHIDGET_UNEXPECTED;
		}
	}
	else if (len < 0)
	{
		LOG(PHIDGET_LOG_ERROR, "usb_get_descriptor failed in CUSBGetDeviceCapabilities with error code: %d \"%s\"", len, strerror(-len));
		return EPHIDGET_UNEXPECTED;
	}
	else
	{
		LOG(PHIDGET_LOG_ERROR, "Couldn't get interface descriptor in CUSBGetDeviceCapabilities");
		return EPHIDGET_UNEXPECTED;
	}
	
	//Get a HID descriptor
	len = usb_control_msg(udev, USB_ENDPOINT_IN+1, 
						USB_REQ_GET_DESCRIPTOR,
						(USB_DT_REPORT << 8) + 0, phid->deviceDef->pdd_iid, (char*)buf, 
						sizeof(buf), 500 /* ms timeout */);
						
	if(len < 0)
	{
		switch(len)
		{
		case -ETIMEDOUT: //important case?
		default:
			LOG(PHIDGET_LOG_ERROR, "usb_control_msg failed in CUSBGetDeviceCapabilities with error code: %d \"%s\"", len, strerror(-len));
			return EPHIDGET_UNEXPECTED;
		}
	}
	
	if(len >= 10)
	{	
		for(i=10;i<len;i++) {
			if(buf[i]==0x81 && buf[i-2]==0x95)
				phid->inputReportByteLength=buf[i-1];
			else if(buf[i]==0x81 && buf[i-4]==0x95)
				phid->inputReportByteLength=buf[i-3];
			if(buf[i]==0x91 && buf[i-2]==0x95)
				phid->outputReportByteLength=buf[i-1];
			else if(buf[i]==0x91 && buf[i-4]==0x95)
				phid->outputReportByteLength=buf[i-3];
		}
	}
	else
	{
		LOG(PHIDGET_LOG_ERROR, "Couldn't get report lengths in CUSBGetDeviceCapabilities");
		return EPHIDGET_UNEXPECTED;
	}
	
	return getLabelString(phid, udev);
}

/*
   This needs to maintain a list of devices - it's used by the phidget manager for
   keeping track of attach and detach events. (On Mac this is handled with 
   notifications, so this is not needed.) - PM
*/
int CUSBBuildList(CPhidgetList **curList) {
	int MemberIndex, i, ret, found;
	unsigned long Length;
	CPhidgetList *traverse;
	Length = 0;
	MemberIndex = 0;
	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *udev;
	CPhidgetHandle phid;
	char unique_name[1024];

	TESTPTR(curList)
	
	usb_init();
	if((ret = usb_find_busses()) < 0)
	{
		LOG(PHIDGET_LOG_ERROR, "usb_find_busses failed with error code: %d \"%s\"", ret, strerror(-ret));
	}
	if((ret = usb_find_devices()) < 0)
	{
		LOG(PHIDGET_LOG_ERROR, "usb_find_devices failed with error code: %d \"%s\"", ret, strerror(-ret));
	}
	
	//search through all USB devices
	for (bus = usb_busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			
			snprintf(unique_name,1024,"%s%s",bus->dirname, dev->filename);
			
			//LOG(PHIDGET_LOG_VERBOSE,"New Device: %s", unique_name);
		
			found = PFALSE;
			if (AttachedDevices) {
				// we need to loop all the way through because composite devices will appear twice in the list with the same 'unique' name
				for (traverse = AttachedDevices; traverse; traverse=traverse->next) {
					if (!strcmp((char *)traverse->phid->CPhidgetFHandle, unique_name)) {
						CList_addToList((CListHandle *)curList, traverse->phid, CPhidget_areEqual);
						found = PTRUE;
					}
				}
				if(found) goto next;
			}
			
		
			for (i = 1; i<PHIDGET_DEVICE_COUNT; i++) {	 
				if ((dev->descriptor.idVendor == Phid_Device_Def[i].pdd_vid) && 
					(dev->descriptor.idProduct == Phid_Device_Def[i].pdd_pid))
				{
					if (!(phid = (CPhidgetHandle)malloc(sizeof (*phid))))
						return EPHIDGET_NOMEMORY;
					ZEROMEM(phid, sizeof(*phid));
					
					//LOG(PHIDGET_LOG_DEBUG,"New Device: %s",(char *)Phid_DeviceName[Phid_Device_Def[i].pdd_did]);
					
					udev = usb_open(dev);
					if(udev) {
						if (dev->descriptor.bcdDevice < 0x100) 
							phid->deviceVersion = dev->descriptor.bcdDevice * 100;
						else
							phid->deviceVersion = ((dev->descriptor.bcdDevice >> 8) * 100) + ((dev->descriptor.bcdDevice & 0xff));
						phid->deviceType = (char *)Phid_DeviceName[Phid_Device_Def[i].pdd_did];

						CPhidget_setStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
						phid->deviceIDSpec = Phid_Device_Def[i].pdd_sdid;
						phid->deviceUID = CPhidget_getUID(phid->deviceIDSpec, phid->deviceVersion);
						phid->deviceDef = &Phid_Device_Def[i];
						phid->deviceID = Phid_Device_Def[i].pdd_did;
						phid->ProductID = dev->descriptor.idProduct;
						phid->VendorID = dev->descriptor.idVendor;
						
						if (dev->descriptor.iSerialNumber) {
							char string[256];
							memset(string, 0, 256);
							if((ret = usb_get_string_simple(udev, dev->descriptor.iSerialNumber, string, sizeof(string))) < 0)
							{
								LOG(PHIDGET_LOG_ERROR, "usb_get_string_simple failed with error code: %d \"%s\"", ret, strerror(-ret));
								LOG(PHIDGET_LOG_INFO, "This usually means you need to run as root");
								if((ret = usb_close(udev)) < 0)
								{
									LOG(PHIDGET_LOG_ERROR, "usb_close failed with error code: %d \"%s\"", ret, strerror(-ret));
								}
								free(phid);
								goto next;
							}
							else
							{
								phid->serialNumber = atol(string);
								getLabelString(phid, udev);
							}
						}
						if (dev->descriptor.iProduct) {
							if((ret = usb_get_string_simple(udev, dev->descriptor.iProduct, phid->usbProduct, sizeof(phid->usbProduct))) < 0)
							{
								LOG(PHIDGET_LOG_ERROR, "usb_get_string_simple failed with error code: %d \"%s\"", ret, strerror(-ret));
								LOG(PHIDGET_LOG_INFO, "This usually means you need to run as root");
								if((ret = usb_close(udev)) < 0)
								{
									LOG(PHIDGET_LOG_ERROR, "usb_close failed with error code: %d \"%s\"", ret, strerror(-ret));
								}
								free(phid);
								goto next;
							}
						}
						phid->specificDevice = TRUE;
						phid->attr = Phid_Device_Def[i].pdd_attr;
						
						if(!(phid->CPhidgetFHandle = strdup(unique_name)))
							return EPHIDGET_NOMEMORY;
						
						LOG(PHIDGET_LOG_INFO, "New device in CUSBBuildList: %s", (char *)phid->CPhidgetFHandle);

						if((ret = usb_close(udev)) < 0)
						{
							LOG(PHIDGET_LOG_ERROR, "usb_close failed with error code: %d \"%s\"", ret, strerror(-ret));
						}
						CList_addToList((CListHandle *)curList, phid, CPhidget_areEqual);
					} //if(udev)
					else
					{
						free(phid);
						if((ret = usb_close(udev)) < 0)
						{
							LOG(PHIDGET_LOG_ERROR, "usb_close failed with error code: %d \"%s\"", ret, strerror(-ret));
						}
					}
				} //vendor, product ids match
			} /* iterate over phidget device table */
next: ;
    	} /* iterate over USB devices */
  	} /* iterate over USB busses */
    return EPHIDGET_OK;
}

void CUSBCleanup(void)
{
	;
}
/* 
	CUSBOpenHandle takes a CPhidgetInfo structure, with 
	ProductID/VendorID/SerialNumber filled in.

	Serial number is always filled in.
*/
int CUSBOpenHandle(CPhidgetHandle phid)
{
	int idVendor;
	int idProduct;
	int serial = 0;
	int i,ret;
	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *udev;
	
	usb_init();
	if((ret = usb_find_busses()) < 0)
	{
		LOG(PHIDGET_LOG_ERROR, "usb_find_busses failed with error code: %d \"%s\"", ret, strerror(-ret));
	}
	if((ret = usb_find_devices()) < 0)
	{
		LOG(PHIDGET_LOG_ERROR, "usb_find_devices failed with error code: %d \"%s\"", ret, strerror(-ret));
	}

	for (bus = usb_busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			for (i = 1; i<PHIDGET_DEVICE_COUNT; i++) {
				if (Phid_Device_Def[i].pdd_did == phid->deviceID) {
					idVendor = Phid_Device_Def[i].pdd_vid;
					idProduct = Phid_Device_Def[i].pdd_pid;
					if ((dev->descriptor.idVendor == idVendor) && (dev->descriptor.idProduct == idProduct)) {
						/* the vend/prod matches! */
						udev = usb_open(dev);
						if (udev) {
							serial = -1;
							if (dev->descriptor.iSerialNumber) {
								char string[256];
								if((ret = usb_get_string_simple(udev, dev->descriptor.iSerialNumber, string, sizeof(string))) < 0)
								{
									LOG(PHIDGET_LOG_WARNING, "usb_get_string_simple failed with error code: %d \"%s\"", ret, strerror(-ret));
									LOG(PHIDGET_LOG_INFO, "This usually means you need to run as root");
									if((ret = usb_close(udev)) < 0)
									{
										LOG(PHIDGET_LOG_ERROR, "usb_close failed with error code: %d \"%s\"", ret, strerror(-ret));
									}
									goto next;
								}
								else
								{
									serial = atol(string);
								}
							}
							if (dev->descriptor.iProduct) {
								if((ret = usb_get_string_simple(udev, dev->descriptor.iProduct, phid->usbProduct, sizeof(phid->usbProduct))) < 0)
								{
									LOG(PHIDGET_LOG_ERROR, "usb_get_string_simple failed with error code: %d \"%s\"", ret, strerror(-ret));
									LOG(PHIDGET_LOG_INFO, "This usually means you need to run as root");
									if((ret = usb_close(udev)) < 0)
									{
										LOG(PHIDGET_LOG_ERROR, "usb_close failed with error code: %d \"%s\"", ret, strerror(-ret));
									}
									free(phid);
									goto next;
								}
							}
							if (serial == phid->serialNumber) {
								/*	On Linux, the HID driver likes to claim Phidgets - we can disconnect it here.
									Maybe the user has installed the kernel drivers for the interface kit or servo - disconnect them too (does this work)
									Note this is Linux Only - PM
									Note that we don't need to claim it if usbfs has it - this is what libusb uses. */	
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
								char name[32];
								if((ret = usb_get_driver_np(udev, Phid_Device_Def[i].pdd_iid, name, 32)) < 0)
								{
									LOG(PHIDGET_LOG_WARNING, "usb_get_driver_np failed with error code: %d \"%s\"", ret, strerror(-ret));
								}
								else
								{
									LOG(PHIDGET_LOG_INFO, "Kernel driver name: %s", name);
									if(strncmp(name, "usbfs", 5)) //not usbfs
									{
										if((ret = usb_detach_kernel_driver_np(udev, Phid_Device_Def[i].pdd_iid)) < 0)
										{
											LOG(PHIDGET_LOG_WARNING, "usb_detach_kernel_driver_np failed with error code: %d \"%s\"", ret, strerror(-ret));
										}
										else
										{
											LOG(PHIDGET_LOG_INFO, "Successfully detached kernel driver: %s", name);
										}
									}
								}
#endif
								if((ret = usb_claim_interface(udev, Phid_Device_Def[i].pdd_iid)) < 0)
								{
									LOG(PHIDGET_LOG_WARNING, "usb_claim_interface failed with error code: %d \"%s\"", ret, strerror(-ret));
									if((ret = usb_close(udev)) < 0)
									{
										LOG(PHIDGET_LOG_ERROR, "usb_close failed with error code: %d \"%s\"", ret, strerror(-ret));
									}
								}
								else
								{
									/* the serialnum is okay */
							
									phid->deviceHandle = (HANDLE)udev;
									phid->deviceIDSpec = Phid_Device_Def[i].pdd_sdid;
									phid->deviceDef = &Phid_Device_Def[i];
									phid->deviceType = (char *)Phid_DeviceName[Phid_Device_Def[i].pdd_did];
			
									phid->ProductID = idProduct;
									phid->VendorID = idVendor;
									if (dev->descriptor.bcdDevice < 0x100)
										phid->deviceVersion = dev->descriptor.bcdDevice * 100;
									else
										phid->deviceVersion = ((dev->descriptor.bcdDevice >> 8) * 100) + ((dev->descriptor.bcdDevice & 0xff));
										
									phid->deviceUID = CPhidget_getUID(phid->deviceIDSpec, phid->deviceVersion);
									phid->serialNumber = serial;
			
									if((ret = CUSBGetDeviceCapabilities(phid, dev, udev)))
									{
										LOG(PHIDGET_LOG_ERROR, "CUSBGetDeviceCapabilities returned nonzero code: %d", ret);
									}
	
									phid->attr = Phid_Device_Def[i].pdd_attr;
			
									return EPHIDGET_OK;
								} /* usb_claim_interface */
	      					} /* serial matches */
	      					else
							{
								if((ret = usb_close(udev)) < 0)
								{
									LOG(PHIDGET_LOG_ERROR, "usb_close failed with error code: %d \"%s\"", ret, strerror(-ret));
								}
							}
	    				} /* udev open */
						else
						{
							if((ret = usb_close(udev)) < 0)
							{
								LOG(PHIDGET_LOG_ERROR, "usb_close failed with error code: %d \"%s\"", ret, strerror(-ret));
							}
							LOG(PHIDGET_LOG_WARNING, "usb_open failed - bad permission or what?");
						}
	  				} /* vendor/product match */
				} /* deviceID matches in table */
      		} /* iterate over phidget device table */
next: ;
    	} /* iterate over USB devices */
  	} /* iterate over USB busses */
  	return EPHIDGET_NOTFOUND;
}
