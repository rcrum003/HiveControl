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
#include <string.h>
#include <assert.h>
#include "ptree.h"

//#define VERIFY_TREE

struct ptree_node {
	void *pn_value;
	struct ptree_node *pn_parent;
	struct ptree_node *pn_left;
	struct ptree_node *pn_right;
};

static int
_walk_to(void *v, 
		 struct ptree_node **startp, 
		 struct ptree_node ***pp,
		 int (*cmp)(const void *v1, const void *v2))
{
	struct ptree_node **nextp;
	int c = -1;

	nextp = startp;
	while (*nextp)
	{
		c = cmp(v, (*nextp)->pn_value);
		*startp = *nextp;
		if (c == 0)
			break;
		if (c < 0)
			nextp = &(*nextp)->pn_left;
		else
			nextp = &(*nextp)->pn_right;
		if (pp)
			*pp = nextp;
	}

	return c;
}


static ptree_walk_res_t
_visit(struct ptree_node *pn, 
	   int level, 
	   void *arg1, 
	   void *arg2)
{
	ptree_walk_res_t (*func)(const void *, int, void *, void *) = arg1;
	
	return func(pn->pn_value, level, arg2, pn);
}

static ptree_node_t *
_find_min(ptree_node_t *pn, 
		  int *levelp)
{
	while (pn->pn_left)
	{
		pn = pn->pn_left;
		if (levelp)
			(*levelp)++;
	}
	return pn;
}

static ptree_node_t *
_find_succ(ptree_node_t *pn, 
		   int *levelp)
{
	if (pn->pn_right)
	{
		pn = pn->pn_right;
		if (levelp)
			(*levelp)++;
		while (pn->pn_left)
		{
			pn = pn->pn_left;
			if (levelp)
				(*levelp)++;
		}
	}
	else
	{
		while (pn->pn_parent && pn->pn_parent->pn_right == pn)
		{
			pn = pn->pn_parent;
			if (levelp)
				(*levelp)--;
		}
		pn = pn->pn_parent;
		if (levelp)
			(*levelp)--;
	}
		
	return pn;
}

static int
_walk_int(struct ptree_node *pn, 
		  ptree_order_t order, 
		  int level,
		  ptree_walk_res_t (*func)(struct ptree_node *, int level, void *, void *),
		  void *arg1, 
		  void *arg2)
{
	ptree_walk_res_t res;
	ptree_node_t *next;

	if (!pn)
		return PTREE_WALK_CONTINUE;

	if (order == PTREE_INORDER)
	{
		int nlevel;

		pn = _find_min(pn, &level);
		while (pn) 
		{
			nlevel = level;
			next = _find_succ(pn, &nlevel);
			if ((res = func(pn, level, arg1, arg2)) != PTREE_WALK_CONTINUE)
				return res;
			level = nlevel;
			if (level < 0)
				level = 0;
			pn = next;
		}
		return PTREE_WALK_CONTINUE;
	}
	if (order == PTREE_PREORDER && (res = func(pn, level, arg1, arg2)) != PTREE_WALK_CONTINUE)
		return res;
	if ((res = _walk_int(pn->pn_left, order, level + 1, func, arg1, arg2)) != PTREE_WALK_CONTINUE)
		return res;
	if ((res = _walk_int(pn->pn_right, order, level + 1, func, arg1, arg2)) != PTREE_WALK_CONTINUE)
		return res;
	if (order == PTREE_POSTORDER && (res = func(pn, level, arg1, arg2)) != PTREE_WALK_CONTINUE)
		return res;
	return PTREE_WALK_CONTINUE;
}

#ifdef VERIFY_TREE
static ptree_walk_res_t
_count(struct ptree_node *pn, 
	   int level, 
	   void *arg1, 
	   void *arg2)
{
	int *counter = arg1;
	(*counter)++;
	return PTREE_WALK_CONTINUE;
}

//counts the number of nodes
static int
_count_nodes(ptree_node_t **rootp)
{
	int counter = 0;
	_walk_int(*rootp, PTREE_INORDER, 0, _count, &counter, 0);
	return counter;	
}

static ptree_walk_res_t
_verify_node(struct ptree_node *pn, 
	   int level, 
	   void *arg1, 
	   void *arg2)
{
	int (*cmp)(const void *v1, const void *v2) = arg1;
	struct ptree_node **prev_pn_p = (struct ptree_node **)arg2;
	struct ptree_node *prev_pn = *prev_pn_p;
	
	if(pn->pn_left != NULL && pn->pn_left->pn_parent != pn)
   		return PTREE_WALK_STOP;
	if(pn->pn_right != NULL && pn->pn_right->pn_parent != pn)
   		return PTREE_WALK_STOP;

	if(prev_pn != NULL)
	{
		//this value should always be greater then the last value
		if(cmp(pn->pn_value, prev_pn->pn_value) <= 0)
   			return PTREE_WALK_STOP;
	}
	*prev_pn_p = pn;
		   
   return PTREE_WALK_CONTINUE;
}

//static int
//pdecmp(const void *sv, const void *dv)
//{
//	int res;
//	
//	char *str1 = *(char **)sv;
//	char *str2 = *(char **)dv;
//	
//	res = strcmp(str1, str2);
//	
//	if(res <= 0)
//		return res;
//	
//	return res;
//}

//Verifies that the tree is a valid Binary search tree
static int
_verify_tree(ptree_node_t **rootp, int (*cmp)(const void *v1, const void *v2))
{
	struct ptree_node *prev_pn = NULL;
	if(_walk_int(*rootp, PTREE_INORDER, 0, _verify_node, cmp, &prev_pn) == PTREE_WALK_STOP)
		return 0;
	return 1;
}
#endif

static void
_remove_node(ptree_node_t **rootp, 
			 ptree_node_t *pn, 
			 void **oldval)
{
	ptree_node_t **pp;
	ptree_node_t **predp;
	ptree_node_t *pred;
	
#ifdef VERIFY_TREE
	int countStart = _count_nodes(rootp), countEnd=0;
#endif

	//Find pn in the tree
	if (!pn->pn_parent)
	{
		assert(rootp && pn == *rootp);
		pp = rootp;
	}
	else
	{
		if (pn->pn_parent->pn_left == pn)
			pp = &pn->pn_parent->pn_left;
		else
			pp = &pn->pn_parent->pn_right;
	}
	
	//pp should now point at the location where pn is stored in the tree
	assert(pp);
	
	//Best case - only one child, on no children, just remove as in a list
	if (!pn->pn_left)
	{
		*pp = pn->pn_right;
		if (pn->pn_right)
			pn->pn_right->pn_parent = pn->pn_parent;
	}
	else if (!pn->pn_right)
	{
		*pp = pn->pn_left;
		if (pn->pn_left)
			pn->pn_left->pn_parent = pn->pn_parent;
	}

	//Worst case: pn contains both left and right children
	else 
	{
		for (predp = &pn->pn_left; (*predp)->pn_right; )
			predp = &(*predp)->pn_right;
		pred = *predp;

		//pred either has a left child or no child
		if (pred->pn_parent->pn_left == pred)
			pred->pn_parent->pn_left = pred->pn_left;
		else
			pred->pn_parent->pn_right = pred->pn_left;
		if(pred->pn_left)
			pred->pn_left->pn_parent = pred->pn_parent;

		*pp = pred;

		pred->pn_parent = pn->pn_parent;
		pred->pn_left = pn->pn_left;
		pred->pn_right = pn->pn_right;

		if (pn->pn_left)
			pn->pn_left->pn_parent = pred;
		pn->pn_right->pn_parent = pred;
	}

	//Return the value of the removed node
	if (oldval)
		*oldval = pn->pn_value;

	free(pn);
	
#ifdef VERIFY_TREE
	countEnd = _count_nodes(rootp);
	countStart--;
	assert(countEnd == countStart);
#endif
}

int
ptree_inorder_walk_remove(ptree_node_t **rootp, 
						  void **oldval, 
						  void *pn,
						  int (*cmp)(const void *v1, const void *v2))
{
	assert(pn);
	if (!pn)
		return 0;
#ifdef VERIFY_TREE
	assert(_verify_tree(rootp, cmp));
#endif
	_remove_node(rootp, pn, oldval);
#ifdef VERIFY_TREE
	assert(_verify_tree(rootp, cmp));
#endif
	return 1;
}

ptree_walk_res_t
ptree_walk(ptree_node_t *start, 
		   ptree_order_t order,
		   ptree_walk_res_t (*func)(const void *v1, int level, void *arg, void *ptree_inorder_walking_remove_arg), 
		   int (*cmp)(const void *v1, const void *v2),
		   void *arg)
{
#ifdef VERIFY_TREE
	ptree_node_t *start_p = start;
	if(cmp != NULL)
		assert(_verify_tree(&start_p, cmp));
#endif

	return _walk_int((struct ptree_node *)start, order, 0, _visit, (void *)func, arg);
}

ptree_walk_res_t
ptree_clear_func(struct ptree_node *pn, 
				 int level, 
				 void *arg1, 
				 void *arg2)
{
	free(pn); pn = NULL;

	return PTREE_WALK_CONTINUE;
}

void
ptree_clear(ptree_node_t **rootp)
{
	_walk_int(*rootp, PTREE_POSTORDER, 0, ptree_clear_func, 0, 0);
	*rootp = 0;
}

int
ptree_remove(void *v, 
			 ptree_node_t **rootp, 
			 int (*cmp)(const void *v1, const void *v2), 
			 void **oldval)
{
	struct ptree_node *cur;
	
#ifdef VERIFY_TREE
	assert(_verify_tree(rootp, cmp));
#endif

	cur = *rootp;
	if (_walk_to(v, &cur, NULL, cmp) != 0)
		return 0;
	_remove_node(rootp, cur, oldval);
	
#ifdef VERIFY_TREE
	assert(_verify_tree(rootp, cmp));
#endif

	return 1;
}

int
ptree_replace(void *v, 
			  ptree_node_t **rootp, 
			  int (*cmp)(const void *v1, const void *v2), 
			  void **oldval)
{
	struct ptree_node **parentpp;
	struct ptree_node *parentp;
	struct ptree_node *pn;
	int c;
	
#ifdef VERIFY_TREE
	int countStart = _count_nodes(rootp), countEnd=0;
#endif
	
	parentp = *rootp;
	parentpp = rootp;
	
#ifdef VERIFY_TREE
	assert(_verify_tree(rootp, cmp));
#endif

	//if we find v in the tree, replace it
	if ((c = _walk_to(v, &parentp, &parentpp, cmp)) == 0)
	{
		if (oldval)
			*oldval = parentp->pn_value;
		parentp->pn_value = v;
	
#ifdef VERIFY_TREE
	assert(_verify_tree(rootp, cmp));
#endif
	
#ifdef VERIFY_TREE
	countEnd = _count_nodes(rootp);
	assert(countEnd == countStart);
#endif
		
		return 1;
	}
	
	//otherwise, add it to the tree
	if (!(pn = malloc(sizeof (*pn))))
		return 0;
	
	memset(pn, 0, sizeof (*pn));
	pn->pn_value = v;
	pn->pn_parent = parentp;
	*parentpp = pn;
	if (oldval)
		*oldval = 0;
	
#ifdef VERIFY_TREE
	assert(_verify_tree(rootp, cmp));
#endif
	
#ifdef VERIFY_TREE
	countEnd = _count_nodes(rootp);
	countStart++;
	assert(countEnd == countStart);
#endif

	return 1;
}

int
ptree_contains(void *v, 
			   ptree_node_t *parentp, 
			   int (*cmp)(const void *v1, const void *v2), 
			   void **nodeval)
{
	int c;

	if ((c = _walk_to(v, &parentp, NULL, cmp)) == 0)
	{
		if (nodeval)
			*nodeval = parentp->pn_value;
		return 1;
	}
	if (nodeval)
		*nodeval = 0;
	return 0;
}