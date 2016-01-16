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

#ifndef _PLIST_H_
#define _PLIST_H_

typedef struct plist_node plist_node_t;

int plist_contains(void *k, plist_node_t *root, void **nodeval);
int plist_remove(void *k, plist_node_t **root, void **ov);
int plist_add(void *k, void *v, plist_node_t **root);
void plist_clear(plist_node_t **root);
int plist_walk(plist_node_t *start, int(*func)(const void *k, const void *v,
    void *arg), void *arg);

#endif
