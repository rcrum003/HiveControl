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
#include "cphidgetlist.h"

/* Adds an element to a list - Duplicates are not allowed.
 * Return:	EPHIDGET_OK on success
 *			EPHIDGET_DUPLICATE if the element already exists in the list
 */
int CList_addToList(CListHandle *list, void *element, 
	int (*compare_fptr)(void *element1, void *element2))
{
	int result = 0;
	CListHandle trav = 0, newentry = 0;

	TESTPTRS(list, element)

	/* The very first thing we do is make sure none of these already exist in the list */
	result = CList_findInList(*list, element, compare_fptr, NULL);
	switch (result) {
		case EPHIDGET_OK:
			return EPHIDGET_DUPLICATE;
		case EPHIDGET_NOTFOUND:
			break;
		default:
			return result;
	}

	newentry = (CListHandle)malloc(sizeof(CList));
	if (!newentry) return EPHIDGET_NOMEMORY;
	ZEROMEM(newentry, sizeof(CList));

	newentry->next = 0;
	newentry->element = element;

	if (!*list)
		*list = newentry;
	else
	{
		for (trav = *list; trav->next; trav = trav->next);
		trav->next = newentry;
	}
	return EPHIDGET_OK;
}

/* this returns success even if the element is not found in the list
 * this also removes duplicates.
 *
 * but! if we find in the list the exact element, rather then just a match, we need to free it after! 
 * interating, or the compare function will croak!
 */
int CList_removeFromList(CListHandle *list, void *element, 
	int (*compare_fptr)(void *element1, void *element2),
	int freeDevice, void (*free_fptr)(void *element))
{
	CListHandle traverse = 0, last = 0;
	int freeElement = PFALSE;

	TESTPTRS(list, element)

	for (traverse=*list; traverse; traverse = traverse->next) {
		if(compare_fptr(element, traverse->element)) {
			if(traverse == *list) {
				*list = traverse->next;
				if(freeDevice && traverse->element)
				{
					// don't free element in the middle of iterating!
					if(traverse->element == element)
						freeElement=PTRUE;
					else
						free_fptr(traverse->element);
					traverse->element = 0;
				}
				free(traverse); traverse = NULL;
				traverse=*list;
				break;
			}
			else {
				last->next=traverse->next;
				if(freeDevice && traverse->element)
				{
					// don't free element in the middle of iterating!
					if(traverse->element == element)
						freeElement=PTRUE;
					else
						free_fptr(traverse->element);
					traverse->element = 0;
				}
				free(traverse); traverse = NULL;
				traverse=last;
			}
		}
		last = traverse;
	}
	
	/* element itself was found in the list, and we want it freed - free it here */
	if(freeElement)
		free_fptr(element);

	return EPHIDGET_OK;
}

/* this takes any list, and frees all of the list element, 
	and can also free the elements that they point to */
int CList_emptyList(CListHandle *list, int freeDevices, void (*free_fptr)(void *element))
{
	CListHandle last = 0, traverse = 0;

	TESTPTR(list)

	last = 0;
	for(traverse = *list; traverse; last = traverse, traverse = traverse->next)
	{
		if(traverse->element && freeDevices)
		{
			free_fptr(traverse->element);
			traverse->element = 0;
		}
		if(last)
		{
			free(last); last = NULL;
		}
	}
	if(last)
	{
		free(last); last = NULL;
	}

	*list = 0;

	return EPHIDGET_OK;
}

/* returns EPHIDGET_OK if the device is in the list */
int CList_findInList(CListHandle list, void *element, 
	int (*compare_fptr)(void *element1, void *element2), void **found_element)
{
	CListHandle trav = 0;

	TESTPTR(element)

	for (trav=list; trav; trav = trav->next) {
		if(compare_fptr(element, trav->element)) {
			if(found_element) *found_element = trav->element;
			return EPHIDGET_OK;
		}
	}
	return EPHIDGET_NOTFOUND;
}

