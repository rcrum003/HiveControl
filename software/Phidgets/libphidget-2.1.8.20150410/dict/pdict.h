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

#ifndef _PDICT_H_
#define _PDICT_H_

typedef enum {
	PDR_VALUE_CHANGED = 1,
	PDR_ENTRY_ADDED,
	PDR_ENTRY_REMOVING,
	PDR_CURRENT_VALUE
}  pdict_reason_t;

/* The _t stands for typedef? */
typedef struct pdict pdict_t;

typedef void (*pdl_notify_func_t)(const char *k, 
								  const char *v,
								  pdict_reason_t r, 
								  const char *pde_oldval, 
								  void *arg);

typedef int (*pdict_walk_func_t)(const char *k, 
								 const char *v, 
								 void *arg);

pdict_t *pdict_alloc(void);

int pdict_add(pdict_t *pd, 
			  const char *k, 
			  const char *v, 
			  const char **ovp);

int pdict_add_persistent_change_listener(pdict_t *pd, 
										 const char *kpat,
										 pdl_notify_func_t, 
										 void *);

int pdict_walk(pdict_t *pd, 
			   pdict_walk_func_t, 
			   void *arg);

/*
 * Return whether a given key is in the dictionary.  If v is set, it
 * will be set to point to a copy of the value, which must be freed by
 * the caller.
 */
int pdict_ent_lookup(pdict_t *pd, 
					 const char *k, 
					 const char **v);

int pdict_ent_add_change_listener(pdict_t *pd, 
								  const char *k,
								  pdl_notify_func_t, 
								  void *);

int pdict_ent_remove_change_listener(pdict_t *pd, 
									 const char *k,
									 pdl_notify_func_t, 
									 void *);

int pdict_remove_persistent_change_listener(pdict_t *pd, 
											int id);

int pdict_ent_remove(pdict_t *pd, 
					 const char *k, 
					 char **ovp);

const char *pdict_reason_str(pdict_reason_t);

pdict_reason_t pdict_reason_from_str(const char *);

#endif
