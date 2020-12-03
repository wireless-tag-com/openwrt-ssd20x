/*
* ceva_linkdrv.h- Sigmastar
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
 * ceva_linkdrv.h
 *
 *  Created on: Nov 12, 2013
 *  Author: Ido Reis <ido.reis@tandemg.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef CEVA_LINKDRV_H_
#define CEVA_LINKDRV_H_

#include <linux/list.h>
#include <linux/types.h>
#include <linux/kfifo.h>

#include "ceva_linkdrv-generic.h"

struct ceva_pcidev;

/*!
 * type for CEVA event (32 bit)
 */
typedef u32 ceva_event_t;

/*!
 * logical module of ceva_linkdrv's
 */
struct ceva_linkdrv {
	int online;                          /*! indicates driver state         */
	struct xm6_dev_data *owner;          /*!< physical driver owner         */
	spinlock_t waiting_lock;             /*!< spin for critical sections    */
	struct list_head pfd_list;           /*!< private descriptors list      */
	wait_queue_head_t wq;                /*!< waitqueue for synchronization */
	struct mutex mux;                    /*!< internal mutex object         */
#if defined(CEVA_LINKDRV_DEBUG)
	union ceva_linkdrv_debug_info debug; /*!< debug statistics              */
#endif
};

#if defined(CEVA_LINKDRV_DEBUG)
#define DEBUG_INFO_INC(link, name) \
	do { (link)->debug.data.name++; } while( 0 )
#define DEBUG_INFO_DEC(link, name) \
	do { (link)->debug.data.name--; } while( 0 )
#define DEBUG_INFO_ADD(link, name, val) \
	do { (link)->debug.data.name += val; } while( 0 )
#define DEBUG_INFO_SUB(link, name, val) \
	do { (link)->debug.data.name -= val; } while( 0 )
#else
#define DEBUG_INFO_INC(link, val)
#define DEBUG_INFO_DEC(link, val)
#define DEBUG_INFO_ADD(link, name, val)
#define DEBUG_INFO_SUB(link, name, val)
#endif

extern int ceva_linkdrv_broadcast_events(struct ceva_linkdrv* link,
		ceva_event_t* events, int sz);
extern int ceva_linkdrv_open_cb(struct ceva_linkdrv *link);
extern int ceva_linkdrv_release_cb(struct ceva_linkdrv *link);
extern long ceva_linkdrv_ioctl(struct ceva_linkdrv *link, unsigned int cmd,
		unsigned long arg);
extern int ceva_linkdrv_init(struct ceva_linkdrv* link,
		struct xm6_dev_data *owner);
extern void ceva_linkdrv_deinit(struct ceva_linkdrv* link);

#endif /* CEVA_LINKDRV_H_ */
