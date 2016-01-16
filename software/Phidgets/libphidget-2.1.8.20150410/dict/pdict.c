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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <regex.h>
#if defined(_WINDOWS) && !defined(_CYGWIN)
#include "wincompat.h"
#endif
#include "utils.h"
#include "plist.h"
#include "ptree.h"
#include "pdict-impl.h"

typedef struct pdict_ent pdict_ent_t;
typedef int (*pdict_walk_int_func_t)(pdict_ent_t *pde, void *arg);

static int _pdict_ent_add_persistent_change_listeners(pdict_t *pd,
    pdict_ent_t *pde);
static int _pdict_ent_add_persistent_change_listener(pdict_ent_t *pde,
    pdict_persistent_listener_t *pl);
static int _pdict_ent_remove_persistent_change_listener(pdict_ent_t *pde,
    pdict_persistent_listener_t *pl);
static void _pdict_ent_notify(pdict_ent_t *pde, int reason, const char *ov);
static int _pdict_ent_add_change_listener(pdict_ent_t *pde,
    pdl_notify_func_t notify, void *arg);
static int _pdict_walk_int(pdict_t *pd, pdict_walk_int_func_t w, void *arg);
static int _pdict_ent_remove_change_listener(pdict_ent_t *pde,
    pdl_notify_func_t notify, void *a);

pdict_t *
pdict_alloc(void)
{
	pdict_t *pd;

	if (!(pd = malloc(sizeof (*pd))))
		return 0;
	memset(pd, 0, sizeof (*pd));

	return pd;
}

static int
pdecmp(const void *sv, const void *dv)
{
	return strcmp(*(char **)sv, *(char **)dv);
}

static int
pdict_ent_remove_change_listeners_cb(const void *v, const void *v0, void *a)
{
	free((void *)v); v = NULL;
	return (1);
}

static int
pdict_ent_remove_change_listeners(pdict_ent_t *pde)
{
	plist_walk(pde->pde_listeners, pdict_ent_remove_change_listeners_cb, 0);
	plist_clear(&pde->pde_listeners);
	return 1;
}

static int
pdict_ent_listeners_copy_cb(const void *v, const void *v0, void *a)
{
	pdict_listener_t *pdl = (pdict_listener_t *)v;
	pdict_ent_t *pde_ent_copy = a;

	return _pdict_ent_add_change_listener(pde_ent_copy, pdl->pdl_notify, pdl->pdl_arg);
}

static void
_pdict_ent_listeners_copy(pdict_ent_t *pde, pdict_ent_t *pde_copy)
{
	plist_walk(pde->pde_listeners, pdict_ent_listeners_copy_cb, pde_copy);
}

/*
 * Sets or resets an entry to the dictionary, returning 0 on failure.
 */
int
pdict_add(pdict_t *pd, const char *k, const char *v, const char **ovp)
{
	pdict_ent_t *n;
	pdict_ent_t n_copy;
	const char *ov;

	if (!(k = strdup(k)))
		return 0;
	if (!(v = strdup(v))) {
		free((void *)k); k = NULL;
		return 0;
	}
	memset(&n_copy, 0, sizeof(pdict_ent_t));
	if (ptree_contains((void *)&k, pd->pd_ents, pdecmp, (void *)&n)) {
		free((void *)k); k = NULL;
		ov = n->pde_val;
		n->pde_val = v;

		if (ovp)
			*ovp = ov;
		else {
			free((void *)ov); ov = NULL;
		}
		
		//We copy n so that it's safe if it gets removed during a callback
		if(n->pde_listeners)
		{
			n_copy.pde_key = strdup(n->pde_key);
			n_copy.pde_val = strdup(n->pde_val);
			_pdict_ent_listeners_copy(n, &n_copy);
			_pdict_ent_notify(&n_copy, PDR_VALUE_CHANGED, ov);
			pdict_ent_remove_change_listeners(&n_copy);
			free((char *)n_copy.pde_key);
			free((char *)n_copy.pde_val);
		}
		return 1;
	}
	if (!(n = malloc(sizeof (*n)))) {
		free((void *)k); k = NULL;
		free((void *)v); v = NULL;
		return (0);
	}
	memset(n, 0, sizeof (*n));
	n->pde_key = k;
	n->pde_val = v;

	//Add dict listeners to the dict entry object
	if (!_pdict_ent_add_persistent_change_listeners(pd, n)) {
		free((void *)k); k = NULL;
		free((void *)v); v = NULL;
		free(n); n = NULL;
		return (0);
	}
	//Add the dict entry to the dict (replacing if there is a matching key).
	if (!ptree_replace(n, &pd->pd_ents, pdecmp, NULL)) {
		pdict_ent_remove_change_listeners(n);
		free((void *)k); k = NULL;
		free((void *)v); v = NULL;
		free(n); n = NULL;
		return (0);
	}
	//notify the listeners
	if(n->pde_listeners)
	{
		n_copy.pde_key = strdup(n->pde_key);
		n_copy.pde_val = strdup(n->pde_val);
		_pdict_ent_listeners_copy(n, &n_copy);
		_pdict_ent_notify(&n_copy, PDR_ENTRY_ADDED, n_copy.pde_val);
		pdict_ent_remove_change_listeners(&n_copy);
		free((char *)n_copy.pde_key);
		free((char *)n_copy.pde_val);
	}

	if (ovp)
		*ovp = NULL;

	return 1;
}

int
pdict_ent_remove(pdict_t *pd, const char *k, char **ovp)
{
	pdict_ent_t *n;

	pu_log(PUL_VERB, 0, "Removing in key pdict_ent_remove: %s", k);
	
	if (!ptree_remove((void *)&k, &pd->pd_ents, pdecmp, (void *)&n))
	{
		//pu_log(PUL_INFO, 0, "Failed to remove key in pdict_ent_remove: %s", k);
		return 0;
	}
	
	_pdict_ent_notify(n, PDR_ENTRY_REMOVING, n->pde_val);
	
	if (ovp)
		*ovp = (char *)n->pde_val;
	else {
		free((void *)n->pde_val);
	}
	free((void *)n->pde_key);
	pdict_ent_remove_change_listeners(n);
	free(n);
	return 1;
}

static int
pdict_ent_remove_persistent_change_listener_cb(const void *pl, const void *v,
    void *a)
{
	_pdict_ent_remove_persistent_change_listener((pdict_ent_t *)a,
	    (pdict_persistent_listener_t *)pl);
	return 1;
}

static int
pdict_ent_add_persistent_change_listener_cb(const void *lid, const void *pl,
    void *a)
{
	return _pdict_ent_add_persistent_change_listener((pdict_ent_t *)a,
	    (pdict_persistent_listener_t *)pl);
}

static int
_pdict_ent_add_persistent_change_listeners(pdict_t *pd, pdict_ent_t *pde)
{
	if (!plist_walk(pd->pd_persistent_listeners, pdict_ent_add_persistent_change_listener_cb, pde)) {
		plist_walk(pd->pd_persistent_listeners, pdict_ent_remove_persistent_change_listener_cb, pde);
		pu_log(PUL_WARN, 0, "Failed to add persistent change listener in _pdict_ent_add_persistent_change_listeners.");
		return 0;
	}
	return 1;
}

static int
pdict_ent_remove_persistent_change_listener_dcb(pdict_ent_t *pde, void *pl)
{
	return _pdict_ent_remove_persistent_change_listener(pde,
	    (pdict_persistent_listener_t *)pl);
}

static int
pdict_ent_add_persistent_change_listener_dcb(pdict_ent_t *pde, void *pl)
{
	return _pdict_ent_add_persistent_change_listener(pde,
	    (pdict_persistent_listener_t *)pl);
}

static int
_pdict_ent_add_persistent_change_listener(pdict_ent_t *pde,
    pdict_persistent_listener_t *pdpl)
{
	int res;

	if ((res = regexec(&pdpl->pdpl_regex, pde->pde_key, 0, NULL, 0)) != 0)
	{
		return res == REG_NOMATCH;
	}
	if (!_pdict_ent_add_change_listener(pde, pdpl->pdpl_l.pdl_notify, pdpl->pdpl_l.pdl_arg))
	{
		pu_log(PUL_WARN, 0, "Failed to add persistent change listener in _pdict_ent_add_persistent_change_listener.");
		return 0;
	}
	if (pdpl->pdpl_new)
		pdpl->pdpl_l.pdl_notify(pde->pde_key, pde->pde_val, PDR_CURRENT_VALUE, NULL, pdpl->pdpl_l.pdl_arg);
	return 1;
}

static int
_pdict_ent_remove_persistent_change_listener(pdict_ent_t *pde,
    pdict_persistent_listener_t *pl)
{
	_pdict_ent_remove_change_listener(pde, pl->pdpl_l.pdl_notify,
	    pl->pdpl_l.pdl_arg);
	return 1;
}

int
pdict_remove_persistent_change_listener(pdict_t *pd, int id)
{
	pdict_persistent_listener_t *pdpl;

	if (!plist_remove((void *)(size_t)id, &pd->pd_persistent_listeners, (void **)&pdpl) || !pdpl)
	{
		pu_log(PUL_WARN, 0, "Failed plist_remove in pdict_remove_persistent_change_listener.");
		return 0;
	}
	if (!_pdict_walk_int(pd, pdict_ent_remove_persistent_change_listener_dcb, pdpl))
	{
		pu_log(PUL_WARN, 0, "Failed _pdict_walk_int in pdict_remove_persistent_change_listener.");
		return 0;
	}
	regfree(&pdpl->pdpl_regex);
	free(pdpl); pdpl = NULL;
	return 1;
}

int
pdict_add_persistent_change_listener(pdict_t *pd, const char *kpat,
    pdl_notify_func_t notify, void *arg)
{
	pdict_persistent_listener_t *pl;
	static int lid = 1;

	if (!(pl = malloc(sizeof (*pl))))
		return 0;
	memset(pl, 0, sizeof (*pl));
	pl->pdpl_l.pdl_notify = notify;
	pl->pdpl_l.pdl_arg = arg;
	if (regcomp(&pl->pdpl_regex, kpat, REG_EXTENDED | REG_NOSUB) != 0) {
		// XXX todo: communicate error context is not libc
		free(pl); pl = NULL;
		pu_log(PUL_WARN, 0, "Failed regcomp in pdict_add_persistent_change_listener.");
		return 0;
	}

	plist_add((void *)(size_t)lid, pl, &pd->pd_persistent_listeners);

	pl->pdpl_new = 1;
	if (!_pdict_walk_int(pd,
	    pdict_ent_add_persistent_change_listener_dcb, pl)) {
		_pdict_walk_int(pd,
		    pdict_ent_remove_persistent_change_listener_dcb, pl);
		plist_remove((void *)(size_t)lid, &pd->pd_persistent_listeners, NULL);
		regfree(&pl->pdpl_regex);
		free(pl); pl = NULL;
		pu_log(PUL_WARN, 0, "Failed _pdict_walk_int in pdict_add_persistent_change_listener.");
		return 0;
	}
	pl->pdpl_new = 0;
	return lid++;
}

/*
 * Return whether a given key is in the dictionary.  If the given
 * entry pointer is non-NULL, set it to the entry.
 */
static int
_pdict_ent_lookup(pdict_t *pd, const char *k, pdict_ent_t **e)
{
	return ptree_contains((void *)&k, pd->pd_ents, pdecmp, (void **)e);
}

int
pdict_ent_lookup(pdict_t *pd, const char *k, const char **v)
{
	pdict_ent_t *pde;

	if (_pdict_ent_lookup(pd, k, &pde)) {
		if (v)
			*v = strdup(pde->pde_val);
		return 1;
	}
	return 0;
}

static int
pdict_ent_remove_change_listener_cb(const void *k, const void *v, void *a)
{
	pdict_listener_t *l = (pdict_listener_t *)k;
	void **arg = (void **)a;

	if (l->pdl_notify == (pdl_notify_func_t)arg[0] && l->pdl_arg == arg[1]) {
		arg[2] = l;
		return 0;
	}
	return 1;
}

static int
_pdict_ent_remove_change_listener(pdict_ent_t *pde, pdl_notify_func_t notify,
    void *a)
{
	void *arg[3];

	arg[0] = (void *)notify;
	arg[1] = a;
	arg[2] = NULL;
	plist_walk(pde->pde_listeners, pdict_ent_remove_change_listener_cb, arg);
	if (arg[2]) {
		plist_remove(arg[2], &pde->pde_listeners, NULL);
		free(arg[2]); arg[2] = NULL;
		return 1;
	}
	return 0;
}

static int
_pdict_ent_add_change_listener(pdict_ent_t *pde, pdl_notify_func_t notify,
    void *arg)
{
	pdict_listener_t *l;

	if (!(l = malloc(sizeof (*l))))
		return 0;
	memset(l, 0, sizeof (*l));
	l->pdl_notify = notify;
	l->pdl_arg = arg;
	if (!plist_add(l, 0, &pde->pde_listeners)) {
		free(l); l = NULL;
		pu_log(PUL_WARN, 0, "Failed plist_add in _pdict_ent_add_change_listener.");
		return 0;
	}

	return 1;
}

int
pdict_ent_add_change_listener(pdict_t *pd, const char *k,
    pdl_notify_func_t notify, void *arg)
{
	pdict_ent_t *pde;

	if (!_pdict_ent_lookup(pd, k, &pde))
		return 0;
	return _pdict_ent_add_change_listener(pde, notify, arg);
}

typedef struct {
	pdict_ent_t *penca_pde;
	pdict_reason_t penca_reason;
	const char *penca_ov;
} pdict_ent_notify_cb_args_t;

static int
pdict_ent_notify_cb(const void *v, const void *v0, void *a)
{
	pdict_listener_t *pdl = (pdict_listener_t *)v;
	pdict_ent_notify_cb_args_t *penca = a;

	pdl->pdl_notify(penca->penca_pde->pde_key, penca->penca_pde->pde_val,
	    penca->penca_reason, penca->penca_ov, pdl->pdl_arg);
	return 1;
}

static void
_pdict_ent_notify(pdict_ent_t *pde, int reason, const char *ov)
{
	pdict_ent_notify_cb_args_t penca = { pde, reason, ov };

	plist_walk(pde->pde_listeners, pdict_ent_notify_cb, &penca);
}

static ptree_walk_res_t
pdict_walk_int_cb(const void *v, int level, void *a, void *pwra)
{
	void **args = a;

	return ((pdict_walk_int_func_t)args[0])((pdict_ent_t *)v, args[1]);
}

static ptree_walk_res_t
pdict_walk_cb(const void *v, int level, void *a, void *pwra)
{
	pdict_ent_t *pde = (pdict_ent_t *)v;
	void **args = a;

	return ((pdict_walk_func_t)args[0])(pde->pde_key, pde->pde_val,
	    args[1]);
}

static int
_pdict_walk_int(pdict_t *pd, pdict_walk_int_func_t w, void *arg)
{
	void *args[2] = { (void *)w, arg };

	return ptree_walk(pd->pd_ents, PTREE_INORDER, pdict_walk_int_cb, pdecmp, args);
}

int
pdict_walk(pdict_t *pd, pdict_walk_func_t w, void *arg)
{
	void *args[2] = { (void *)w, arg };

	return ptree_walk(pd->pd_ents, PTREE_INORDER, pdict_walk_cb, NULL, args);
}

int
pdict_ent_remove_change_listener(pdict_t *pd, const char *k,
    pdl_notify_func_t nf, void *arg)
{
	pdict_ent_t *e;
	int res;

	if (!(res = _pdict_ent_lookup(pd, k, &e)))
		return 0;
	return _pdict_ent_remove_change_listener(e, nf, arg);
}

const char *
pdict_reason_str(pdict_reason_t r)
{
	switch (r) {
	case PDR_VALUE_CHANGED:
		return "changed";
	case PDR_ENTRY_ADDED:
		return "added";
	case PDR_ENTRY_REMOVING:
		return "removing";
	case PDR_CURRENT_VALUE:
		return "current";
	default:
		return "?";
	}
}

pdict_reason_t
pdict_reason_from_str(const char *s)
{
	if (strcmp(s, "changed") == 0)
		return PDR_VALUE_CHANGED;
	if (strcmp(s, "current") == 0)
		return PDR_CURRENT_VALUE;
	if (strcmp(s, "added") == 0)
		return PDR_ENTRY_ADDED;
	if (strcmp(s, "removing") == 0)
		return PDR_ENTRY_REMOVING;
	return 0;
}
