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

#ifndef _CPHIDGET_MACROS
#define _CPHIDGET_MACROS

#define PHANDLE(pname) CPhidget##pname##Handle 

#define DPHANDLE(pname) typedef struct _CPhidget##pname *CPhidget##pname##Handle; /**< A Phidget pname handle*/

//Regular Versions
#define CHDRSTANDARD(pname) \
/** Creates a Phidget pname handle. @param phid A pointer to an unallocated Phidget pname handle. */ \
PHIDGET21_API int CCONV CPhidget##pname##_create(CPhidget##pname##Handle *phid);

#define CHDREVENTINDEX(pname,param,...) PHIDGET21_API int CCONV CPhidget##pname##_set_On##param##_Handler(PHANDLE(pname) phid, int (CCONV *fptr)(PHANDLE(pname) phid, void *userPtr, int index, __VA_ARGS__), void *userPtr);
#define CHDREVENT(pname,param,...) PHIDGET21_API int CCONV CPhidget##pname##_set_On##param##_Handler(PHANDLE(pname) phid, int (CCONV *fptr)(PHANDLE(pname) phid, void *userPtr, __VA_ARGS__), void *userPtr);

#define CHDRGET(pname,propname,...) PHIDGET21_API int CCONV CPhidget##pname##_get##propname(PHANDLE(pname) phid, __VA_ARGS__); 
#define CHDRSET(pname,propname,...) PHIDGET21_API int CCONV CPhidget##pname##_set##propname(PHANDLE(pname) phid, __VA_ARGS__); 

#define CHDRGETINDEX(pname,propname,...) PHIDGET21_API int CCONV CPhidget##pname##_get##propname(PHANDLE(pname) phid, int index, __VA_ARGS__); 
#define CHDRSETINDEX(pname,propname,...) PHIDGET21_API int CCONV CPhidget##pname##_set##propname(PHANDLE(pname) phid, int index, __VA_ARGS__); 

//Versions for Deprecation

#define DEP_CHDREVENTINDEX(depmsg,pname,param,...) DEPRECATE_WIN(depmsg) PHIDGET21_API int CCONV CPhidget##pname##_set_On##param##_Handler(PHANDLE(pname), int (CCONV *fptr)(PHANDLE(pname), void *userPtr, int index, __VA_ARGS__), void *userPtr) DEPRECATE_GCC ;
#define DEP_CHDREVENT(depmsg,pname,param,...) DEPRECATE_WIN(depmsg) PHIDGET21_API int CCONV CPhidget##pname##_set_On##param##_Handler(PHANDLE(pname), int (CCONV *fptr)(PHANDLE(pname), void *userPtr, __VA_ARGS__), void *userPtr) DEPRECATE_GCC ;

#define DEP_CHDRGET(depmsg,pname,propname,...) DEPRECATE_WIN(depmsg) PHIDGET21_API int CCONV CPhidget##pname##_get##propname(PHANDLE(pname),__VA_ARGS__) DEPRECATE_GCC ;
#define DEP_CHDRSET(depmsg,pname,propname,...) DEPRECATE_WIN(depmsg) PHIDGET21_API int CCONV CPhidget##pname##_set##propname(PHANDLE(pname), __VA_ARGS__) DEPRECATE_GCC ; 

#define DEP_CHDRGETINDEX(depmsg,pname,propname,...) DEPRECATE_WIN(depmsg) PHIDGET21_API int CCONV CPhidget##pname##_get##propname(PHANDLE(pname), int index, __VA_ARGS__) DEPRECATE_GCC ; 
#define DEP_CHDRSETINDEX(depmsg,pname,propname,...) DEPRECATE_WIN(depmsg) PHIDGET21_API int CCONV CPhidget##pname##_set##propname(PHANDLE(pname), int index, __VA_ARGS__) DEPRECATE_GCC ; 


#define TESTPTR(phidname) if (!phidname) return EPHIDGET_INVALIDARG;
#define TESTPTRS(vname1,vname2) if ((!vname1) || (!vname2)) return EPHIDGET_INVALIDARG;
#define TESTINDEX(vname1) if (Index >= phid->vname1 || Index < 0) return EPHIDGET_OUTOFBOUNDS;
#define TESTMASGN(vname1, check) if(phid->vname1 == check) { *pVal = check; return EPHIDGET_UNKNOWNVAL; }
#define TESTRANGE(min, max) if(newVal < (min) || newVal > (max)) return EPHIDGET_INVALIDARG;
#define TESTDEVICETYPE(def) if (phid->phid.deviceID != def)	return EPHIDGET_WRONGDEVICE;
#define TESTATTACHED if (!CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_ATTACHED_FLAG)) return EPHIDGET_NOTATTACHED;
#define MASGN(vname1) { *pVal = phid->vname1; return EPHIDGET_OK; }

#define CFHANDLE(pname,param, ...) \
PHIDGET21_API int CCONV CPhidget##pname##_set_On##param##_Handler(CPhidget##pname##Handle phidA, int (CCONV *fptr)(CPhidget##pname##Handle, void *userPtr, __VA_ARGS__ ), void *userPtr) \
{ \
	CPhidget##pname##Info *phid = (CPhidget##pname##Info *)phidA; \
	TESTPTR(phid); \
	phid->fptr##param = fptr; \
	phid->fptr##param##ptr = userPtr; \
	return EPHIDGET_OK; \
}

#define CPHIDGETDATA(pname) static int CPhidget##pname##_dataInput(CPhidgetHandle phidG, unsigned char *buffer, int length) \
{ \
	CPhidget##pname##Handle phid = (CPhidget##pname##Handle)phidG; 

#define FIRE(ename, ...) \
if (phid->fptr##ename) \
	if(CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_ATTACHED_FLAG)) \
		phid->fptr##ename(phid, phid->fptr##ename##ptr, __VA_ARGS__)

#define QUEUE_ERROR_EVENT(ecode, buf) \
else \
{ \
	void **args = malloc(sizeof(void *)*2); \
	if(args) \
	{ \
		args[0] = strdup(buf); \
		args[1] = (void *)(size_t)ecode; \
		if(args[0]) \
		{ \
			if(CList_addToList((CListHandle *)&((CPhidgetHandle)phid)->errEventList, args, CPhidgetHandle_areEqual)!=EPHIDGET_OK) \
			{ \
				free(args[0]); \
				free(args); \
			} \
		} \
		else \
			free(args); \
	} \
}

#define FIRE_ERROR_BUF_INT(ecode, buf) \
if(CPhidget_statusFlagIsSet(((CPhidgetHandle)phid)->status, PHIDGET_ATTACHED_FLAG)) \
{ \
	if (((CPhidgetHandle)phid)->fptrError) \
		((CPhidgetHandle)phid)->fptrError((CPhidgetHandle)phid, ((CPhidgetHandle)phid)->fptrErrorptr, ecode, buf); \
} \

#define FIRE_ERROR_BUF(ecode, buf) \
FIRE_ERROR_BUF_INT(ecode, buf) \
QUEUE_ERROR_EVENT(ecode, buf)

#define FIRE_ERROR(ecode, ...) \
snprintf(error_buffer,sizeof(error_buffer), __VA_ARGS__); \
FIRE_ERROR_BUF_INT(ecode, error_buffer) \
QUEUE_ERROR_EVENT(ecode, error_buffer)

#define FIRE_ERROR_NOQUEUE(ecode, ...) \
snprintf(error_buffer,sizeof(error_buffer), __VA_ARGS__); \
FIRE_ERROR_BUF_INT(ecode, error_buffer)

#define CGETINDEX(pname,propname,proptype) \
PHIDGET21_API int CCONV CPhidget##pname##_get##propname(CPhidget##pname##Handle phid, int Index, proptype *pVal) { 

#define CGET(pname,propname,proptype) \
PHIDGET21_API int CCONV CPhidget##pname##_get##propname(CPhidget##pname##Handle phid, proptype *pVal) {

#define CSETINDEX(pname,propname,proptype) \
PHIDGET21_API int CCONV CPhidget##pname##_set##propname(CPhidget##pname##Handle phid, int Index, proptype newVal) { 

#define CSET(pname,propname,proptype) \
PHIDGET21_API int CCONV CPhidget##pname##_set##propname(CPhidget##pname##Handle phid, proptype newVal) { 

#define CPHIDGETCLEARVARS(pname) \
static int CPhidget##pname##_clearVars(CPhidgetHandle phidG) \
{ \
	CPhidget##pname##Handle phid = (CPhidget##pname##Handle)phidG; 

#define CPHIDGETINIT(pname) \
static int CPhidget##pname##_initAfterOpen(CPhidgetHandle phidG) \
{ \
	CPhidget##pname##Handle phid = (CPhidget##pname##Handle)phidG; 

#define CPHIDGETINITEVENTS(pname) \
static int CPhidget##pname##_eventsAfterOpen(CPhidgetHandle phidG) \
{ \
	CPhidget##pname##Handle phid = (CPhidget##pname##Handle)phidG; \
	int i = 0; \
	CListHandle traverse = phidG->errEventList; \
	for (; traverse; traverse = traverse->next) { \
		if (phidG->fptrError) \
			phidG->fptrError(phidG, phidG->fptrErrorptr, (int)*(int *)(&((void **)traverse->element)[1]), (char *)((void **)traverse->element)[0]); \
	} \
	CList_emptyList((CListHandle *)&phidG->errEventList, PTRUE, CPhidgetErrorEvent_free); \
	i = 0;

#define CSENDPACKET_BUF(pname) \
static int CPhidget##pname##_sendpacket(CPhidget##pname##Handle phid, \
    const unsigned char *buf) \
{ \
	int waitReturn; \
	TESTPTRS(phid, buf) \
again: \
	if (!CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_ATTACHED_FLAG)) \
		return EPHIDGET_NOTATTACHED; \
	CThread_mutex_lock(&phid->phid.outputLock); \
	if (phid->outputPacketLen) { \
		CThread_mutex_unlock(&phid->phid.outputLock); \
		waitReturn = CThread_wait_on_event(&phid->phid.writtenEvent, 1500); \
		switch(waitReturn) \
		{ \
		case WAIT_OBJECT_0: \
			break; \
		case WAIT_ABANDONED: \
			return EPHIDGET_UNEXPECTED; \
		case WAIT_TIMEOUT: \
			return EPHIDGET_TIMEOUT; \
		} \
		goto again; \
	} \
	memcpy(phid->outputPacket, buf, phid->phid.outputReportByteLength); \
	phid->outputPacketLen = phid->phid.outputReportByteLength; \
	CThread_reset_event(&phid->phid.writtenEvent); \
	CThread_mutex_unlock(&phid->phid.outputLock); \
	CThread_set_event(&phid->phid.writeAvailableEvent); \
	return 0; \
}

#define CMAKEPACKET(pname) \
static int CPhidget##pname##_makePacket(CPhidget##pname##Handle phid, \
    unsigned char *buffer) \
{

#define SENDPACKET(pname, propname) \
	do{ \
		unsigned char *buffer; \
		int ret = 0; \
		if(!(buffer = malloc(phid->phid.outputReportByteLength))) return EPHIDGET_NOMEMORY; \
		ZEROMEM(buffer, phid->phid.outputReportByteLength); \
		CThread_mutex_lock(&phid->phid.writelock); \
		phid->propname = newVal; \
		if((ret = CPhidget##pname##_makePacket(phid, buffer))) goto done; \
		if((ret = CPhidget##pname##_sendpacket(phid, buffer))) goto done; \
done: \
		CThread_mutex_unlock(&phid->phid.writelock); \
		free(buffer); \
		if(ret) return ret; \
	}while(0)

#define CMAKEPACKETINDEXED(pname) \
static int CPhidget##pname##_makePacket(CPhidget##pname##Handle phid, \
    unsigned char *buffer, int Index) \
{

#define SENDPACKETINDEXED(pname, propname, index) \
	do{ \
		unsigned char *buffer; \
		int ret = 0; \
		if(!(buffer = malloc(phid->phid.outputReportByteLength))) return EPHIDGET_NOMEMORY; \
		ZEROMEM(buffer, phid->phid.outputReportByteLength); \
		CThread_mutex_lock(&phid->phid.writelock); \
		phid->propname = newVal; \
		if((ret = CPhidget##pname##_makePacket(phid, buffer, index))) goto done; \
		if((ret = CPhidget##pname##_sendpacket(phid, buffer))) goto done; \
done: \
		CThread_mutex_unlock(&phid->phid.writelock); \
		free(buffer); \
		if(ret) return ret; \
	}while(0)


#define CGETPACKET(pname) \
static int CPhidget##pname##_getPacket(CPhidgetHandle phidG, \
    unsigned char *buf, unsigned int *lenp) \
{ 

#define CGETPACKET_BUF(pname) \
static int CPhidget##pname##_getPacket(CPhidgetHandle phidG, \
    unsigned char *buf, unsigned int *lenp) \
{ \
	CPhidget##pname##Handle phid = (CPhidget##pname##Handle)phidG; \
	TESTPTRS(phid, buf) \
	TESTPTR(lenp) \
	CThread_mutex_lock(&phid->phid.outputLock); \
	if (*lenp < phid->outputPacketLen) { \
		CThread_mutex_unlock(&phid->phid.outputLock); \
		return EPHIDGET_INVALIDARG; \
	} \
	memcpy(buf, phid->outputPacket, *lenp = phid->outputPacketLen); \
	phid->outputPacketLen = 0; \
	CThread_mutex_unlock(&phid->phid.outputLock); \
	return EPHIDGET_OK; \
}

#define CCREATE_INT(pname,pdef) \
	CPhidget##pname##Handle phid; \
	TESTPTR(phidp) \
	if (!(phid = (CPhidget##pname##Handle)malloc(sizeof (*phid)))) \
		return EPHIDGET_NOMEMORY; \
	ZEROMEM(phid, sizeof(*phid)); \
	phid->phid.deviceID = pdef; \
	phid->phid.fptrInit = CPhidget##pname##_initAfterOpen; \
	phid->phid.fptrClear = CPhidget##pname##_clearVars; \
	phid->phid.fptrEvents = CPhidget##pname##_eventsAfterOpen; \
	phid->phid.fptrData = CPhidget##pname##_dataInput; \
	phid->phid.fptrGetPacket = CPhidget##pname##_getPacket; \
	CThread_mutex_init(&phid->phid.lock); \
	CThread_mutex_init(&phid->phid.openCloseLock); \
	CThread_mutex_init(&phid->phid.writelock); \
	CThread_mutex_init(&phid->phid.outputLock); \
	CPhidget_clearStatusFlag(&phid->phid.status, PHIDGET_ATTACHED_FLAG, &(&phid->phid)->lock); \
	CThread_create_event(&phid->phid.writeAvailableEvent); \
	CThread_create_event(&phid->phid.writtenEvent); \
	*phidp = phid;

#define CCREATE(pname,pdef) int CCONV CPhidget##pname##_create(CPhidget##pname##Handle *phidp) \
{ \
	CCREATE_INT(pname,pdef) \
	return EPHIDGET_OK; \
}

#define CCREATE_EXTRA(pname,pdef) int CCONV CPhidget##pname##_create(CPhidget##pname##Handle *phidp) \
{ \
	CCREATE_INT(pname,pdef)

#define ADDNETWORKKEYINDEXED(kname, valType, propname) \
do{ \
	char key[1024], val[1024]; \
	CThread_mutex_lock(&phid->phid.lock); \
	phid->propname[Index] = newVal; \
	if(!CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_SERVER_CONNECTED_FLAG)) { \
		CThread_mutex_unlock(&phid->phid.lock); \
		return EPHIDGET_NETWORK_NOTCONNECTED; } \
	snprintf(key, (unsigned int)sizeof(key), "/PCK/%s/%d/" #kname "/%d", phid->phid.deviceType, phid->phid.serialNumber, Index); \
	snprintf(val, (int)sizeof(val), valType, newVal); \
	pdc_async_set(phid->phid.networkInfo->server->pdcs, key, val, (int)strlen(val), PFALSE, internal_async_network_error_handler, &phid->phid); \
	CThread_mutex_unlock(&phid->phid.lock); \
}while(0)

#define ADDNETWORKKEY(kname, valType, propname) \
do{ \
	char key[1024], val[1024]; \
	CThread_mutex_lock(&phid->phid.lock); \
	phid->propname = newVal; \
	if(!CPhidget_statusFlagIsSet(phid->phid.status, PHIDGET_SERVER_CONNECTED_FLAG)) { \
		CThread_mutex_unlock(&phid->phid.lock); \
		return EPHIDGET_NETWORK_NOTCONNECTED; } \
	snprintf(key, sizeof(key), "/PCK/%s/%d/" #kname , phid->phid.deviceType, phid->phid.serialNumber); \
	snprintf(val, sizeof(val), valType, newVal); \
	pdc_async_set(phid->phid.networkInfo->server->pdcs, key, val, (int)strlen(val), PFALSE, internal_async_network_error_handler, &phid->phid); \
	CThread_mutex_unlock(&phid->phid.lock); \
}while(0)

/* used in csocketevents.c */
#define PWC_SETKEYS(pname) \
int phidget##pname##_set(CPhidgetHandle arg, const char *setThing, int index, const char *state, pdict_reason_t reason) \
{ \
	int ret = EPHIDGET_OK; \
	CPhidget##pname##Handle phid = (CPhidget##pname##Handle)arg;

#define GET_DOUBLE_VAL double value = strtod(state, NULL)
#define GET_INT_VAL int value = strtol(state, NULL, 10)
#define GET_INT64_VAL __int64 value = strtoll(state, NULL, 10)

#define INC_KEYCOUNT(var, unktype) if(phid->var == unktype) phid->phid.keyCount++;
#define KEYNAME(name) !strncmp(setThing, name, sizeof(name))
#define CHKINDEX(atr, def) (index < phid->phid.attr.atr?phid->phid.attr.atr:def)

#define PWC_BAD_SETTYPE(pname) \
	do { \
		ret = EPHIDGET_INVALIDARG; \
		LOG(PHIDGET_LOG_DEBUG,"Bad setType for " #pname ": %s", setThing); \
	}while(0)

#endif
