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

#ifndef _PDICT_IMPL_H_
#define _PDICT_IMPL_H_

#include "pdict.h"

struct pdict_listener;

/*
 * Represents a dictionary entry.
 * key and value strings, with a plist of listeners
 */
struct pdict_ent {
	const char *pde_key;
	const char *pde_val;
	plist_node_t *pde_listeners;
};

typedef struct pdict_listener {
	pdl_notify_func_t pdl_notify;
	void *pdl_arg;
} pdict_listener_t;

typedef struct pdict_persistent_listener {
	pdict_listener_t pdpl_l;
	regex_t pdpl_regex;
	int pdpl_new;
} pdict_persistent_listener_t;

/*
 * A Phidget Dictionary
 * contains a ptree of entries and a plist of persistent listeners
 */
struct pdict {
	ptree_node_t *pd_ents;
	plist_node_t *pd_persistent_listeners;
};

#endif
