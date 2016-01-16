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
#include <libusb-1.0/libusb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

int CUSBCloseHandle(CPhidgetHandle phid) {
	int ret = 0;
	int result = EPHIDGET_OK;
	
	if (!phid)
		return EPHIDGET_INVALIDARG;

	CPhidget_clearStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
	
	if (phid->deviceHandle == NULL)
		return EPHIDGET_NOTATTACHED;

	CThread_join(&phid->readThread);
		
	if((ret = libusb_release_interface((libusb_device_handle *) phid->deviceHandle, phid->deviceDef->pdd_iid)) != 0)
	{
		switch(ret)
		{
			case LIBUSB_ERROR_NO_DEVICE:
				//usb_release_interface called after the device was unplugged
				LOG(PHIDGET_LOG_WARNING, "libusb_release_interface called on unplugged device.");
				break;
			default:
				LOG(PHIDGET_LOG_ERROR, "libusb_release_interface failed with error code: %d", ret);
		}
	}
	
	//if we notice that PHIDGET_USB_ERROR_FLAG is set, then reset this device before closing
	//this gives us a better chance of getting it back if something has gone wrong.
	if(CPhidget_statusFlagIsSet(phid->status, PHIDGET_USB_ERROR_FLAG))
	{
		LOG(PHIDGET_LOG_WARNING,"PHIDGET_USB_ERROR_FLAG is set - resetting device.");
		if((ret = libusb_reset_device((libusb_device_handle *) phid->deviceHandle)) != 0)
		{
			LOG(PHIDGET_LOG_ERROR, "libusb_reset_device failed with error code: %d", ret);
			result = EPHIDGET_UNEXPECTED;
		}
	}
	
	libusb_close((libusb_device_handle *) phid->deviceHandle);
	
	phid->deviceHandle = NULL;
	
	return result;
}

int CUSBSendPacket(CPhidgetHandle phid, unsigned char *buffer) {
	int BytesWritten = 0, ret;

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
		ret = libusb_interrupt_transfer((libusb_device_handle *)phid->deviceHandle, 
										   LIBUSB_ENDPOINT_OUT | phid->deviceDef->pdd_iid+1, 
										   (char *)buffer, 
										   phid->outputReportByteLength, /* size */
										   &BytesWritten,
										   500); /* FIXME? timeout */
	}
	else
	{
		BytesWritten = libusb_control_transfer((libusb_device_handle *)phid->deviceHandle, 
						LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
						LIBUSB_REQUEST_SET_CONFIGURATION,
						0x0200, /* value */
						phid->deviceDef->pdd_iid, /* index*/
						(char *)buffer, 
						phid->outputReportByteLength, /* size */
						500); /* FIXME? timeout */
		ret = BytesWritten;
	}
					
	if(ret < 0)
	{
		switch(ret)
		{
			case LIBUSB_ERROR_TIMEOUT: //important case?
					return EPHIDGET_TIMEOUT;
			case LIBUSB_ERROR_NO_DEVICE:
				//device is gone - unplugged.
				LOG(PHIDGET_LOG_INFO, "Device was unplugged - detach.");
				return EPHIDGET_NOTATTACHED;
			default:
				if(phid->interruptOutEndpoint)
					LOG(PHIDGET_LOG_ERROR, "libusb_interrupt_transfer failed in CUSBSendPacket with error code: %d", ret);
				else
					LOG(PHIDGET_LOG_ERROR, "libusb_control_msg failed in CUSBSendPacket with error code: %d", ret);
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
			
		BytesWritten = libusb_control_transfer((libusb_device_handle *)phid->deviceHandle, 
						LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
						LIBUSB_REQUEST_SET_DESCRIPTOR,
						0x0304, /* value */
						0x0409, /* index*/
						(char *)buffer, 
						size, /* size */
						500); /* FIXME? timeout */
						
		if(BytesWritten < 0)
		{
			switch(BytesWritten)
			{
			case LIBUSB_ERROR_TIMEOUT: //important case?	
			default:
				LOG(PHIDGET_LOG_INFO, "usb_control_msg failed in CUSBSetLabel with error code: %d", BytesWritten);
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
	int BytesRead = 0, ret;
	
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
	
	ret = libusb_interrupt_transfer((libusb_device_handle *)phid->deviceHandle, 
				LIBUSB_ENDPOINT_IN | phid->deviceDef->pdd_iid+1, 
				(char *)buffer, 
				phid->inputReportByteLength,
				&BytesRead,
				500);
				
	if (ret != 0)
	{
		switch(ret)
		{
			// A timeout occured, but we'll just try again
			case LIBUSB_ERROR_TIMEOUT:
				LOG(PHIDGET_LOG_VERBOSE, "libusb_interrupt_transfer timeout in CUSBReadPacket");
				return EPHIDGET_TIMEOUT;
			case LIBUSB_ERROR_BUSY:
				//This happens when someone else calls claim_interface on this interface (a manager for ex.) - basically just wait until they release it.
				//This will happen if an open occurs in another app which (for some reason) can steal the interface from this one.
				LOG(PHIDGET_LOG_INFO, "Device is busy on Read - try again.");
				return EPHIDGET_TRYAGAIN;
			case LIBUSB_ERROR_NO_DEVICE:
				//device is gone - unplugged.
				LOG(PHIDGET_LOG_INFO, "Device was unplugged - detach.");
				return EPHIDGET_NOTATTACHED;
			case LIBUSB_ERROR_PIPE:
			case LIBUSB_ERROR_OVERFLOW:
			default:
				LOG(PHIDGET_LOG_ERROR, "libusb_interrupt_transfer in CUSBReadPacket returned: %d", ret);
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
		LOG(PHIDGET_LOG_ERROR, "CUSBReadPacket returned EPHIDGET_TRYAGAIN too many times in a row - reset device.");
		phid->tryAgainCounter = 0;
		return EPHIDGET_UNEXPECTED;
	}
	return EPHIDGET_TRYAGAIN;
}

static int getLabelString(CPhidgetHandle phid, struct libusb_device_handle *handle)
{	
	int len = 0, ret;
	char labelBuf[22];
	memset(labelBuf, 0, sizeof(labelBuf));
	libusb_device *device = libusb_get_device(handle);
	struct libusb_device_descriptor	desc;
	
	if((ret = libusb_get_device_descriptor(device, &desc)) != 0)
	{
		LOG(PHIDGET_LOG_ERROR, "libusb_get_device_descriptor failed with error code: %d", ret);
		return EPHIDGET_UNEXPECTED;
	}
	
	if(desc.iSerialNumber == 3)
	{
		
		//Note that this returns the whole descriptor, including the length and type bytes
		len = libusb_get_string_descriptor(handle, 4, 0, (char *)labelBuf, 22);
		
		if(len < 0)
		{
			switch(len)
			{
				case LIBUSB_ERROR_TIMEOUT: //important case?
				default:
					LOG(PHIDGET_LOG_INFO, "libusb_get_string_descriptor failed in CUSBGetDeviceCapabilities with error code: %d while reading label"\
						" - this probably just means the device doesn't support labels, so this is fine.", len);
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
	return getLabelString(phid, (struct libusb_device_handle *)phid->deviceHandle);
}

int CUSBGetDeviceCapabilities(CPhidgetHandle phid, struct libusb_device *dev, struct libusb_device_handle *udev) {
	unsigned char buf[255];
	int len = 0, i = 0, j, ret;
	const struct libusb_interface_descriptor *interfaceDesc;
	struct libusb_config_descriptor *configDesc = NULL;
	
	memset(buf, 0, sizeof(buf));
	
	//Get config descriptor
	if((ret = libusb_get_active_config_descriptor(dev, &configDesc)) == 0)
	{
		interfaceDesc = NULL;
		
		//Find the interface Descriptor
		for(i=0; i<configDesc->bNumInterfaces; i++)
		{
			for(j=0; j<configDesc->interface[i].num_altsetting; j++)
			{
				if(configDesc->interface[i].altsetting[j].bInterfaceNumber == phid->deviceDef->pdd_iid)
				{
					interfaceDesc = &configDesc->interface[i].altsetting[j];
					break;
				}
			}
		}
		
		if(interfaceDesc == NULL)
		{
			LOG(PHIDGET_LOG_ERROR, "Couldn't find interface descriptor!");
			ret = EPHIDGET_UNEXPECTED;
			goto done;
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
	else
	{
		LOG(PHIDGET_LOG_ERROR, "libusb_get_active_config_descriptor failed in CUSBGetDeviceCapabilities with error code: %d", ret);
		ret = EPHIDGET_UNEXPECTED;
		goto done;
	}
	
	//Get a HID descriptor
	len = libusb_control_transfer(udev, LIBUSB_ENDPOINT_IN+1, 
						LIBUSB_REQUEST_GET_DESCRIPTOR,
						(LIBUSB_DT_REPORT << 8) + 0, phid->deviceDef->pdd_iid, (char*)buf, 
						sizeof(buf), 500 /* ms timeout */);
						
	if(len < 0)
	{
		switch(len)
		{
		case LIBUSB_ERROR_TIMEOUT: //important case?
		default:
				LOG(PHIDGET_LOG_ERROR, "usb_control_msg failed in CUSBGetDeviceCapabilities with error code: %d", len);
				ret = EPHIDGET_UNEXPECTED;
				goto done;
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
		ret = EPHIDGET_UNEXPECTED;
		goto done;
	}
	
	ret = getLabelString(phid, udev);
	
done:
	if(configDesc)
		libusb_free_config_descriptor(configDesc);
	return ret;
}

libusb_context * libusbContext = NULL;
void CUSBUninit()
{
	if(libusbContext)
	{
		LOG(PHIDGET_LOG_INFO, "Deinitializing libusb");
		libusb_exit(libusbContext);
		libusbContext = NULL;
	}
}

int CUSBBuildList(CPhidgetList **curList) {
	int MemberIndex, i, j, ret = EPHIDGET_OK, found;
	unsigned long Length;
	CPhidgetList *traverse;
	Length = 0;
	MemberIndex = 0;
	CPhidgetHandle phid;
	char unique_name[20];
	libusb_device **list = NULL;
	
	TESTPTR(curList)
	
	if(!libusbContext)
	{
		LOG(PHIDGET_LOG_INFO, "Initializing libusb");
		if((ret = libusb_init(&libusbContext)) != 0)
		{
			LOG(PHIDGET_LOG_ERROR, "libusb_init failed with error code: %d", ret);
			libusbContext = NULL;
			goto done;
		}
	}
	
	ssize_t cnt = libusb_get_device_list(libusbContext, &list);
	if(cnt < 0)
	{
		LOG(PHIDGET_LOG_ERROR, "libusb_get_device_list failed with error code: %d", cnt);
		goto done;
	}
	
	//search through all USB devices
	for (j = 0; j < cnt; j++) 
	{
		libusb_device *device = list[j];
		
		snprintf(unique_name,20,"%d/%d",libusb_get_bus_number(device), libusb_get_device_address(device));
		
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
		
		struct libusb_device_descriptor	desc;
		if((ret = libusb_get_device_descriptor(device, &desc)) != 0)
		{
			LOG(PHIDGET_LOG_ERROR, "libusb_get_device_descriptor failed with error code: %d", ret);
			goto next;
		}
		
		//LOG(PHIDGET_LOG_DEBUG, "Device %d: %04x %04x", j, desc.idVendor, desc.idProduct);
	
		for (i = 1; i<PHIDGET_DEVICE_COUNT; i++) {	 
			if ((desc.idVendor == Phid_Device_Def[i].pdd_vid) && 
				(desc.idProduct == Phid_Device_Def[i].pdd_pid))
			{
				if (!(phid = (CPhidgetHandle)malloc(sizeof (*phid))))
				{
					ret = EPHIDGET_NOMEMORY;
					goto done;
				}
				ZEROMEM(phid, sizeof(*phid));
				
				//LOG(PHIDGET_LOG_DEBUG,"New Device: %s",(char *)Phid_DeviceName[Phid_Device_Def[i].pdd_did]);
				
				libusb_device_handle *handle = NULL;
				if ((ret = libusb_open(device, &handle)) == 0)
				{
					if (desc.bcdDevice < 0x100) 
						phid->deviceVersion = desc.bcdDevice * 100;
					else
						phid->deviceVersion = ((desc.bcdDevice >> 8) * 100) + ((desc.bcdDevice & 0xff));
					phid->deviceType = (char *)Phid_DeviceName[Phid_Device_Def[i].pdd_did];

					CPhidget_setStatusFlag(&phid->status, PHIDGET_ATTACHED_FLAG, &phid->lock);
					phid->deviceIDSpec = Phid_Device_Def[i].pdd_sdid;
					phid->deviceUID = CPhidget_getUID(phid->deviceIDSpec, phid->deviceVersion);
					phid->deviceDef = &Phid_Device_Def[i];
					phid->deviceID = Phid_Device_Def[i].pdd_did;
					phid->ProductID = desc.idProduct;
					phid->VendorID = desc.idVendor;
					
					if (desc.iSerialNumber) {
						char string[256];
						memset(string, 0, 256);
						if((ret = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, string, sizeof(string))) < 0)
						{
							LOG(PHIDGET_LOG_ERROR, "libusb_get_string_descriptor_ascii failed with error code: %d", ret);
							libusb_close(handle);
							free(phid);
							goto next;
						}
						else
						{
							phid->serialNumber = atol(string);
							getLabelString(phid, handle);
						}
					}
					if (desc.iProduct) {
						if((ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, phid->usbProduct, sizeof(phid->usbProduct))) < 0)
						{
							LOG(PHIDGET_LOG_ERROR, "libusb_get_string_descriptor_ascii failed with error code: %d", ret);
							libusb_close(handle);
							free(phid);
							goto next;
						}
					}
					phid->specificDevice = TRUE;
					phid->attr = Phid_Device_Def[i].pdd_attr;
					
					if(!(phid->CPhidgetFHandle = strdup(unique_name)))
					{
						ret = EPHIDGET_NOMEMORY;
						goto done;
					}
					
					LOG(PHIDGET_LOG_INFO, "New device in CUSBBuildList: %s", (char *)phid->CPhidgetFHandle);

					libusb_close(handle);
					CList_addToList((CListHandle *)curList, phid, CPhidget_areEqual);
				} //if(udev)
				else
				{
					LOG(PHIDGET_LOG_WARNING, "libusb_open failed in CUSBBuildList with error code: %d", ret);
					LOG(PHIDGET_LOG_INFO, "This usually means you need to run as root, or install the udev rules.");
					free(phid);
					libusb_close(handle);
				}
			} //vendor, product ids match
		} /* iterate over phidget device table */
next: ;
	} /* iterate over USB devices */
	
done:
	if(list)	
		libusb_free_device_list(list, 1);
    return ret;
}

/*
 * Got this from libusb-0.1 because 1.0 doesn't expose driver name!
 */
#define USB_MAXDRIVERNAME 255
struct usb_getdriver {
	unsigned int interface;
	char driver[USB_MAXDRIVERNAME + 1];
};
struct linux_device_handle_priv {
	int fd;
};
struct list_head {
	struct list_head *prev, *next;
};
struct libusb_device_handle_internal {
	pthread_mutex_t lock;
	unsigned long claimed_interfaces;
	struct list_head list;
	void *dev;
	unsigned char os_priv[0];
};
static struct linux_device_handle_priv *_device_handle_priv(struct libusb_device_handle_internal *handle)
{
	return (struct linux_device_handle_priv *) handle->os_priv;
}
#define IOCTL_USB_GETDRIVER	_IOW('U', 8, struct usb_getdriver)
static int libusb_get_driver_name(libusb_device_handle *handle, int interface, char *name, unsigned int namelen)
{
  int fd = _device_handle_priv((struct libusb_device_handle_internal *)handle)->fd;
  struct usb_getdriver getdrv;
  int ret;

  getdrv.interface = interface;
  ret = ioctl(fd, IOCTL_USB_GETDRIVER, &getdrv);
  if (ret)
	LOG(PHIDGET_LOG_ERROR, "could not get bound driver: %d", errno);

  strncpy(name, getdrv.driver, namelen - 1);
  name[namelen - 1] = 0;

  return 0;
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
	int i, j, ret, retVal = EPHIDGET_NOTFOUND;
	libusb_device **list = NULL;
	
	ssize_t cnt = libusb_get_device_list(libusbContext, &list);
	
	if(cnt < 0)
	{
		LOG(PHIDGET_LOG_ERROR, "libusb_get_device_list failed with error code: %d", cnt);
		goto done;
	}
	
	//search through all USB devices
	for (j = 0; j < cnt; j++) 
	{
		libusb_device *device = list[j];
		
		struct libusb_device_descriptor	desc;
		if((ret = libusb_get_device_descriptor(device, &desc)) != 0)
		{
			LOG(PHIDGET_LOG_ERROR, "libusb_get_device_descriptor failed with error code: %d", ret);
			goto next;
		}
		
		for (i = 1; i<PHIDGET_DEVICE_COUNT; i++) {
			if (Phid_Device_Def[i].pdd_did == phid->deviceID) {
				idVendor = Phid_Device_Def[i].pdd_vid;
				idProduct = Phid_Device_Def[i].pdd_pid;
				if ((desc.idVendor == idVendor) && (desc.idProduct == idProduct)) {
					
					/* the vend/prod matches! */
					libusb_device_handle *handle = NULL;
					if ((ret = libusb_open(device, &handle)) == 0)
					{
						serial = -1;
						if (desc.iSerialNumber) {
							char string[256];
							if((ret = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, string, sizeof(string))) < 0)
							{
								LOG(PHIDGET_LOG_WARNING, "libusb_get_string_descriptor_ascii failed with error code: %d", ret);
								libusb_close(handle);
								goto next;
							}
							else
							{
								serial = atol(string);
							}
						}
						if (desc.iProduct) {
							if((ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, phid->usbProduct, sizeof(phid->usbProduct))) < 0)
							{
								LOG(PHIDGET_LOG_ERROR, "libusb_get_string_descriptor_ascii failed with error code: %d", ret);
								libusb_close(handle);
								free(phid);
								goto next;
							}
						}
						if (serial == phid->serialNumber) {
							//Detach any Kernel Drivers
							if((ret = libusb_kernel_driver_active(handle, Phid_Device_Def[i].pdd_iid)) < 0)
							{
								LOG(PHIDGET_LOG_WARNING, "libusb_kernel_driver_active failed with error code: %d", ret);
							}
							else if(ret == 1)
							{
								char name[64];
								if((ret = libusb_get_driver_name(handle, Phid_Device_Def[i].pdd_iid, name, 32)) < 0)
								{
									LOG(PHIDGET_LOG_WARNING, "libusb_get_driver_name failed with error code: %d", ret);
								}
								else
								{
									LOG(PHIDGET_LOG_INFO, "Kernel driver name: %s", name);
									if(strncmp(name, "usbfs", 5)) //not usbfs
									{
										LOG(PHIDGET_LOG_INFO, "Kernel driver is active - will attempt a detach");
										if((ret = libusb_detach_kernel_driver(handle, Phid_Device_Def[i].pdd_iid)) != 0)
										{
											LOG(PHIDGET_LOG_WARNING, "libusb_detach_kernel_driver failed with error code: %d", ret);
										}
										else
										{
											LOG(PHIDGET_LOG_INFO, "Successfully detached kernel driver");
										}
									}
									else
										LOG(PHIDGET_LOG_INFO, "Not detaching kernel driver - already using usbfs.");
								}
							}
							
							if((ret = libusb_claim_interface(handle, Phid_Device_Def[i].pdd_iid)) != 0)
							{
								if(ret == LIBUSB_ERROR_BUSY)
									LOG(PHIDGET_LOG_WARNING, "libusb_claim_interface failed with BUSY - probably the device is opened by another program.");
								else
									LOG(PHIDGET_LOG_WARNING, "libusb_claim_interface failed with error code: %d", ret);
								libusb_close(handle);
							}
							else
							{
								/* the serialnum is okay */
						
								phid->deviceHandle = (HANDLE)handle;
								phid->deviceIDSpec = Phid_Device_Def[i].pdd_sdid;
								phid->deviceDef = &Phid_Device_Def[i];
								phid->deviceType = (char *)Phid_DeviceName[Phid_Device_Def[i].pdd_did];
		
								phid->ProductID = idProduct;
								phid->VendorID = idVendor;
								if (desc.bcdDevice < 0x100)
									phid->deviceVersion = desc.bcdDevice * 100;
								else
									phid->deviceVersion = ((desc.bcdDevice >> 8) * 100) + ((desc.bcdDevice & 0xff));
									
								phid->deviceUID = CPhidget_getUID(phid->deviceIDSpec, phid->deviceVersion);
								phid->serialNumber = serial;
		
								if((ret = CUSBGetDeviceCapabilities(phid, device, handle)))
								{
									LOG(PHIDGET_LOG_ERROR, "CUSBGetDeviceCapabilities returned nonzero code: %d", ret);
								}

								phid->attr = Phid_Device_Def[i].pdd_attr;
		
								retVal = EPHIDGET_OK;
								goto done;
							} /* usb_claim_interface */
						} /* serial matches */
						else
						{
							libusb_close(handle);
						}
					} /* udev open */
					else
					{
						LOG(PHIDGET_LOG_WARNING, "libusb_open failed in CUSBOpenHandle with error code: %d", ret);
						LOG(PHIDGET_LOG_INFO, "This usually means you need to run as root, or install the udev rules.");
						libusb_close(handle);
					}
				} /* vendor/product match */
			} /* deviceID matches in table */
		} /* iterate over phidget device table */
next: ;
	} /* iterate over USB devices */
done:
	if(list)	
		libusb_free_device_list(list, 1);
	
  	return retVal;
}
