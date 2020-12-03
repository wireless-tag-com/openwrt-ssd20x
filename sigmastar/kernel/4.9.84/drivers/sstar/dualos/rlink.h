/*
* rlink.h- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
/*
 * rlink.h
 */
#ifndef	__RLINK_H__
#define	__RLINK_H__

#include "drv_dualos.h"

static inline void init_rlink(struct rlink_head *head)
{
	head->next = head;
	head->prev = head;
}

static inline int rlink_empty(const struct rlink_head *head)
{
	return head->next == head;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal rlink manipulation where we know
 * the prev/next entries already!
 */
static inline void __rlink_add(struct rlink_head *_new,
			      struct rlink_head *prev,
			      struct rlink_head *next,
				  unsigned int nsize)
{
	next->prev = _new;
	_new->next = next;
	_new->prev = prev;
	prev->next = _new;
	prev->nsize = nsize;
}

/**
 * rlink_add_tail - add a new entry
 * @new: new entry to be added
 * @head: rlink head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void rlink_add_tail(struct rlink_head *_new, struct rlink_head *head, unsigned int nsize)
{
	__rlink_add(_new, head->prev, head, nsize);
}

#endif	// __RLINK_H__
