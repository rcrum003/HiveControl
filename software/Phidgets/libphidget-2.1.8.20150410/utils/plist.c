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

#include "../stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "plist.h"

/*
 * A plist is a double-linked list of key-value pairs
 * It can be circular or not.
 */

/*
 * This is a node in a plist
 */
struct plist_node {
	void *pn_key;
	void *pn_value;
	struct plist_node *pn_next;
	struct plist_node *pn_prev;
};

/*
 * Adds a new entry into a plist.
 * if !*root it creates a node where prev and next point to itself
 * else, inserts itself in the list before *root
 * returns 1 on success, 0 on NOMEM failure.
 */
int
plist_add(void *k, void *v, plist_node_t **root)
{
	plist_node_t *n;

	if (!(n = malloc(sizeof (*n))))
		return 0;
	n->pn_key = k;
	n->pn_value = v;
	if (!*root) {
		n->pn_next = n;
		n->pn_prev = n;
		*root = n;
	} else {
		n->pn_prev = (*root)->pn_prev;
		n->pn_next = (*root);
		(*root)->pn_prev->pn_next = n;
		(*root)->pn_prev = n;
	}
	return 1;
}

/*
 * traverse list until cur == *root or cur == 0, freeing each node
 * (list could be either circular or not)
 * note that the key and value pointers are still valid.
 */
void
plist_clear(plist_node_t **root)
{
	plist_node_t *cur;
	plist_node_t *fr;

	cur = *root;
	while (cur) {
		fr = cur;
		cur = cur->pn_next;
		free(fr); fr = NULL;
		if (cur == *root) {
			*root = NULL;
			return;
		}
	}
}

/*
 * removes a node from a plist where the key pointer == k
 * if ov is not null, it will point to the old value from the removed node
 * This only removes the 1st occurance of k.
 * returns 1 on success, 0 if k was not in the list
 */
int
plist_remove(void *k, plist_node_t **rootp, void **ov)
{
	plist_node_t *cur;

	cur = *rootp;
	while (cur) {
		if (cur->pn_key == k) {
			if (ov)
				*ov = cur->pn_value;
			cur->pn_prev->pn_next = cur->pn_next;
			cur->pn_next->pn_prev = cur->pn_prev;
			if (cur->pn_next == cur)
				*rootp = NULL;
			else if (*rootp == cur)
				*rootp = cur->pn_next;
			free(cur); cur = NULL;
			return 1;
		}
		cur = cur->pn_next;
		if (cur == *rootp)
			return 0;
	}
	return 0;
}

/*
 * finds k in plist.
 * if ov != 0, it will be set to the value pointer
 * returns 1 when the key is found, 0 if not found.
 */
int
plist_contains(void *k, plist_node_t *root, void **ov)
{
	plist_node_t *cur;

	cur = root;
	while (cur) {
		if (cur->pn_key == k) {
			if (ov)
				*ov = cur->pn_value;
			return 1;
		}
		cur = cur->pn_next;
		if (cur == root)
			return 0;
	}
	return 0;
}

/*
 * traverses the plist from start, until we get back to start or next==0
 * runs func with key, val, arg.
 * if func returns 0, stops traversing and returns 0
 * else returns 1 for success
 */
int
plist_walk(plist_node_t *start, int (*func)(const void *k, const void *v,
    void *arg), void *arg)
{
	plist_node_t *cur;
	int res;

	cur = start;
	while (cur) {
		if (!(res = func(cur->pn_key, cur->pn_value, arg)))
			return res;
		cur = cur->pn_next;
		if (cur == start)
			return 1;
	}
	return 1;
}
