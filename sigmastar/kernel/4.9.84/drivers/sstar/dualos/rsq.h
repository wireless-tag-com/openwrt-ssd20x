/*
* rsq.h- Sigmastar
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
 * rq.h
 * ring queue header
 */
#ifndef	__RSQ_H__
#define	__RSQ_H__

#include "rlink.h"

typedef struct _slot_t {
	unsigned int	tag;	/* for check */
	unsigned int	no;
	unsigned int	sc;		/* slot size */
	unsigned int	dc;		/* data size */
	unsigned int	bs;		/* size of buffer allocated */
	unsigned int	mark;	/* for groupping */
	unsigned int	ts;		/* time stamp */
	void			*rese;
	void			*buf;	/* data address */
	unsigned int	wrap;	/* wrapping */
	struct _slot_t	*next;	/* link */
} slot_t;

typedef struct ghost_slot_t {
	slot_t	copy;
	slot_t	*ghost;
} gslot_t;

typedef struct _rsq_t {
	unsigned char	*sndr_sbuf;
	unsigned char	*sndr_oob;	/* out of bound */
	unsigned char	*sndr_end;	/* bottom of ring buffer */
	unsigned char	*sndr_wp;
	unsigned char	*sndr_rp;

	unsigned char	*rcvr_sbuf;
	unsigned char	*rcvr_oob;	/* out of bound */
	unsigned char	*rcvr_end;	/* bottom of ring buffer */
	unsigned char	*rcvr_wp;
	unsigned char	*rcvr_rp;

	unsigned int	woff;
	unsigned int	reset_id;
	unsigned int	wrno;	/* newest slot no */
	unsigned int	rdno;	/* read slot no */
	slot_t			*head;	/* oldest slot */
	slot_t			*tail;	/* newest slot */
} rsq_t;

#define	RVR_NAME_SIZE	(16)
#define	DATATYPE_VIDEO	0
#define	DATATYPE_STILL	1
#define	DATATYPE_AUDIO	2
#define	DATATYPE_LOG	3
typedef struct _reservoir_ {
	struct rlink_head	link;	// must be first
	char			name[RVR_NAME_SIZE];
	rsq_t			rsq;
	void			*me;		// for recognition.
	int				iid;
	unsigned int	size;
	unsigned int	slotno;		// current written no.
	unsigned int	headno;		// oldest no. after dropping.
	unsigned int	dropcnt;
	unsigned int	dropfrms;
	unsigned int	sloterr;
	unsigned int	corrfrms;	// corrupted frames
	unsigned int	synccnt;
	unsigned int	resetcnt;
	unsigned int	timestamp;	// last update
	int				datatype;	// video stream or still data
	void			*ext;
} reservoir_t;

#define	RQ_ALIGN(s)		((((u32)(a) + 15) >> 4) << 4)
#define	ALIGN32(s)		((((u32)(s) + 31) >> 5) << 5)
#define	ALIGN4K(s)		((((u32)(s) + 4095) >> 12) << 12)
#define	SIZEOF_RESERVIOR	(((sizeof(reservoir_t) + 4095) >> 12) << 12)
#define	SIZEOF_SLOTHEADER	ALIGN32(sizeof(slot_t))
#define	SIZEOF_RVR_HEADER	(SIZEOF_RESERVIOR + SIZEOF_SLOTHEADER)
#define	SIZEOF_RSQ		(1024)
#define	RSQ_VIDEO0		0
#define	RSQ_AUDIO0		80

/* */
void *get_rvr(int HANDLE);
void set_rvrint(int en);
int get_rvrint(void);
void corrupted_frame(void *p);
void cnt_rsqerr(void *p);

/* rtk functions */
void add_rsqstream(void*, unsigned int size);	/* should be struct rlink* */
/* rsq functions */
void *init_rsq_sndr(void *rqbuf, unsigned int size, unsigned int oobsize, char *name, int datatype);
void init_rsq_rcvr(reservoir_t *rvr);
/* slot/oob functions */
unsigned int check_rsqslot(reservoir_t *rvr, unsigned int expect);
unsigned int gc_rsqslot(reservoir_t *rvr, unsigned int expect);
void *begin_rsqslot(reservoir_t *rvr, unsigned int expect);
unsigned int end_rsqslot(void* wp, unsigned int ws, unsigned int mark, unsigned int ts);
unsigned int rcvr_de_rsqslot(reservoir_t *rvr, void *buf, unsigned int size, slot_t *out);
unsigned int sndr_de_rsqslot(reservoir_t *rvr, void *buf, unsigned int size, slot_t *out);
slot_t *get_rsqslot(void *databuf);

#define	IPI_LINUX_USED	8
static inline int sgi_rsqslot(void *p)
{
	reservoir_t	*rvr = (reservoir_t*)p;
	return rvr->iid + IPI_LINUX_USED;
}

#endif	// __RSQ_H__
