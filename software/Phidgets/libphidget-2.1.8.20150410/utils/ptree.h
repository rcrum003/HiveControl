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

#ifndef _PTREE_H_
#define _PTREE_H_

typedef struct ptree_node ptree_node_t;

typedef enum {
	PTREE_PREORDER = 1,
	PTREE_INORDER = 2,
	PTREE_POSTORDER = 3
}  ptree_order_t;

typedef enum {
	PTREE_WALK_STOP = 0,
	PTREE_WALK_CONTINUE = 1
} ptree_walk_res_t;

int ptree_contains(void *v, ptree_node_t *root, int(*)(const void *sv,
    const void *tv), void **nodeval);
int ptree_remove(void *v, ptree_node_t **root, int(*)(const void *sv,
    const void *tv), void **oltval);
int ptree_replace(void *v, ptree_node_t **root, int(*)(const void *sv,
    const void *tv), void **oltval);
void ptree_clear(ptree_node_t **root);
ptree_walk_res_t
ptree_walk(ptree_node_t *start, 
		   ptree_order_t order,
		   ptree_walk_res_t (*func)(const void *v1, int level, void *arg, void *ptree_inorder_walking_remove_arg), 
		   int (*cmp)(const void *v1, const void *v2),
		   void *arg);
int ptree_inorder_walk_remove(ptree_node_t **rootp, void **oldval, void *piwra,
						  int (*cmp)(const void *v1, const void *v2));

#endif
