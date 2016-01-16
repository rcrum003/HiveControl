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

#ifndef __CPHIDGETLIST
#define __CPHIDGETLIST

typedef struct _CList
{
	struct _CList *next;
	void *element;
} CList, *CListHandle;

int CList_addToList(CListHandle *list, void *element, 
	int (*compare_fptr)(void *element1, void *element2));
int CList_removeFromList(CListHandle *list, void *element, 
	int (*compare_fptr)(void *element1, void *element2),
	int freeDevice, void (*free_fptr)(void *element));
int CList_emptyList(CListHandle *list, int freeDevices, void (*free_fptr)(void *element));
int CList_findInList(CListHandle list, void *element, 
	int (*compare_fptr)(void *element1, void *element2), void **found_element);

#endif
