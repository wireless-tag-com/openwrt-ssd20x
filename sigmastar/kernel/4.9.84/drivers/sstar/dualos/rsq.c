/*
* rsq.c- Sigmastar
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
 * rsq.c
 */
#include "rsq.h"
#include "string.h"
#include "cam_os_wrapper.h"

#define RQ_TAG          (0xfeeffe2f)

/*
 **NOTE**: RQ_ALIGN size should be larger or equal than sizeof(slot_t)!!
 */


/* rsq/int usage map,
 * each rsq with an interrupt (SGI/IPI) to send irq to NS.
 * bit is 0 for free, 1 is occupied.
 * Total number of SGI is 16 for CA7, linux occupied 0~7.
 * RSQ can use 8~15
 */
static int _rsqid_ = 0x00ffffff;
/*
 * init_rsq_sndr
 */
void *init_rsq_sndr(void* rqbuf, unsigned int size, unsigned int oobsize, char *name, int datatype)
{
    reservoir_t *rvr;
    rsq_t       *prsq;

    if (rqbuf == NULL || size == 0 || _rsqid_ == -1)
        return NULL;
    /* share memory needs locate at 4K boundary */
    rvr       = (reservoir_t*)ALIGN4K(rqbuf);
    rvr->size = size - ((u32)rvr - (u32)rqbuf);
    if (rvr->size < (SIZEOF_RESERVIOR + oobsize))
        return NULL;
    rvr->me   = rvr;
    rvr->slotno  = 0;
    rvr->headno  = 0;
    rvr->dropcnt = 0;
    rvr->dropfrms = 0;
    rvr->corrfrms = 0;
    rvr->sloterr  = 0;
    rvr->synccnt  = 0;
    rvr->resetcnt = 0;
    rvr->datatype = datatype;
    rvr->iid      = 8 - __builtin_clz(_rsqid_);
    _rsqid_      |= (1 << (24 + rvr->iid));
    memset(rvr->name, 0, sizeof(rvr->name));
    strncpy(rvr->name, name, sizeof(rvr->name) - 1);
    init_rlink(&(rvr->link));
    /* init ring buffer */
    prsq = &(rvr->rsq);
    prsq->woff = 0;
    prsq->sndr_sbuf = (unsigned char*)rvr + SIZEOF_RESERVIOR;
    prsq->sndr_end  = (unsigned char*)rvr + size;
    prsq->sndr_oob  = prsq->sndr_end - oobsize;
    /* init write and read pointer */
    prsq->sndr_wp = prsq->sndr_sbuf;
    prsq->sndr_rp = prsq->sndr_sbuf;

    prsq->wrno =
    prsq->rdno = rvr->slotno;
    prsq->head =
    prsq->tail = NULL;
    return rvr;
}

#define SNDR_2_RCVR_PTR(prsq, sp)   (typeof(sp))((sp) + (prsq)->woff)
#define RCVR_2_SNDR_PTR(prsq, rp)   (typeof(rp))((rp) - (prsq)->woff)

void init_rsq_rcvr(reservoir_t *rvr)
{
    rsq_t   *prsq;

    prsq = &(rvr->rsq);
    prsq->woff = (void*)rvr - (void*)rvr->me;

    prsq->rcvr_sbuf = SNDR_2_RCVR_PTR(prsq, prsq->sndr_sbuf);
    prsq->rcvr_wp = SNDR_2_RCVR_PTR(prsq, prsq->sndr_wp);
    prsq->rcvr_rp = SNDR_2_RCVR_PTR(prsq, prsq->sndr_rp);
    prsq->rcvr_oob = SNDR_2_RCVR_PTR(prsq, prsq->sndr_oob);
    prsq->rcvr_end = SNDR_2_RCVR_PTR(prsq, prsq->sndr_end);
}

void reset_rsq(reservoir_t *rvr)
{
    rsq_t   *prsq;
    slot_t  *slot;

    prsq = &(rvr->rsq);
    prsq->head =
    prsq->tail = NULL;

    /* init write and read pointer */
    prsq->sndr_wp = prsq->sndr_sbuf;
    prsq->sndr_rp = prsq->sndr_sbuf;
    prsq->wrno =
    prsq->rdno = rvr->slotno;

    /* init sequeuce slot queue */
    slot = (slot_t*)prsq->sndr_wp;
    slot->tag = 0;
    rvr->resetcnt++;
}

unsigned int check_rsqslot(reservoir_t *rvr, unsigned int expect)
{
    rsq_t           *prsq;
    slot_t          *slot;
    unsigned int    nws;
    unsigned char   *rp;
    unsigned char   *ptr;

    if (!rvr)
        return 0;
    CamOsMemInvalidate((void *)rvr, sizeof(reservoir_t));
    prsq  = &(rvr->rsq);

    rp  = prsq->sndr_rp;
    ptr = prsq->sndr_wp;

    /* need total written size */
    nws = SIZEOF_SLOTHEADER + expect;
    slot = (slot_t*)rp;

    if ((ptr <= rp) && (slot->no != prsq->wrno) && ((rp - ptr) < nws))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned int gc_rsqslot(reservoir_t *rvr, unsigned int expect)
{
    rsq_t           *prsq;
    slot_t          *slot;
    unsigned int    nws;
    unsigned char   *rp;
    unsigned char   *ptr;
    unsigned char   *p;
    unsigned char   *pns;

    if (!rvr)
        return 0;
    CamOsMemInvalidate((void *)rvr, sizeof(reservoir_t));
    prsq  = &(rvr->rsq);

    rp  = prsq->sndr_rp;
    ptr = prsq->sndr_wp;

    /* need total written size */
    nws = SIZEOF_SLOTHEADER + expect;
    slot = (slot_t*)rp;

    if ((ptr <= rp) && (slot->no != prsq->wrno) && ((rp - ptr) < nws)) {
        /* |-------WsssssRss----O----------| */
        /* sbuf   ptr          oob           */
        /* FULL!! TODO: drop a group slots */
        unsigned int    df = 0;
        rvr->dropcnt++;
        do {
            slot_t  *s = (slot_t*)(prsq->sndr_rp);

            p = rp + SIZEOF_SLOTHEADER + slot->bs;
            if (p >= prsq->sndr_oob) {
                /* |--R---------------WdOddddp-----|  */
                /* data in oob, wrap to head of buffer*/
                pns = prsq->sndr_sbuf;
            } else {
                /* |--R--------Wddddp-R-O----------|  */
                pns = p;
            }

            rp = prsq->sndr_rp = pns;
            slot = (slot_t*)rp;
            /* for check only */
            if (slot->tag != RQ_TAG) {
                CamOsPrintf(KERN_ERR "DO DROP 0x%x mark %d to 0x%x %s\n",
                               (unsigned int)s, s->mark, (unsigned int)prsq->sndr_rp,
                                slot->tag == RQ_TAG? "OK" : "BAD");
            }

            df += (slot->no - s->no);
            prsq->rdno  =
            rvr->headno = slot->no;     /* oldest no. in rvr */

            if (ptr >= rp || (rp - ptr) >= nws) {
                rvr->dropfrms += df;
                break;
            }
            // CamOsPrintf(KERN_ERR "%s %d new - rp %x\n", __FUNCTION__, __LINE__, (unsigned int)prsq->rp);
        } while (1);
    }

    // flush to physical storage
    CamOsMemFlush((void *)rvr, sizeof(reservoir_t));

    return 1;
}

void *begin_rsqslot(reservoir_t *rvr, unsigned int expect)
{
    rsq_t           *prsq;
    slot_t          *slot;
    unsigned char   *ptr;

    if (!rvr)
        return NULL;
    CamOsMemInvalidate((void *)rvr, sizeof(reservoir_t));
    prsq  = &(rvr->rsq);

    ptr = prsq->sndr_wp;

    slot = (slot_t*)(prsq->sndr_wp);
    CamOsMemInvalidate((void *)slot, sizeof(slot_t));
    slot->rese = rvr;
    CamOsMemFlush((void *)slot, sizeof(slot_t));
    /* |-------Wssssssss-R--O----------|  */
    /* sbuf   ptr        ^ oob        end */

    /* |----R---Wsssssss----O----------|  */
    /* sbuf ^  ptr         oob        end */
    ptr += SIZEOF_SLOTHEADER;

    return ptr;
}

/* trim slot size after data in el */
unsigned int end_rsqslot(void* wp, unsigned int ws, unsigned int mark, unsigned int ts)
{
    reservoir_t     *rvr;
    rsq_t           *prsq;
    slot_t          *slot;
    unsigned char   *ptr;

    if (!wp) return 0;
    ptr  = wp;
    slot = (slot_t*)(ptr - SIZEOF_SLOTHEADER);

    CamOsMemInvalidate((void *)slot, sizeof(slot_t));
    rvr  = slot->rese;
    CamOsMemInvalidate((void *)rvr, sizeof(reservoir_t));
    prsq = &(rvr->rsq);

    /* for debug/verify only */
    if ((void*)slot != prsq->sndr_wp) {
        /* wp is invalid */
        CamOsPrintf(KERN_ERR "ERROR: %s %d\n", __FUNCTION__, __LINE__);
        return 0;
    }
    slot->tag  = RQ_TAG;            /* a tag for allocated */
    slot->no   = rvr->slotno;
    slot->dc   = ws;                /* real size of data */
    slot->bs   = ALIGN32(ws);       /* the slot allocated size */
    slot->sc   = SIZEOF_SLOTHEADER; /* slot header size */
    slot->next = NULL;
    slot->mark = mark;
    slot->ts   = ts;
    ptr += slot->bs;                            /* move write pointer for next writting */
    if (/* this is made sure by begin_rsqslot() */
        /* (prsq->sndr_wp < prsq->sndr_rp && ptr > prsq->sndr_rp) || */
        (ptr > prsq->sndr_end)) {
        /* |-----------WdddRddp-O----------|  */
        /* |----------------WdddOdddddddddddp */
        /* overwrite!! */
        /* TODO: expect in begin_rsqslot too small !! */
        CamOsPrintf(KERN_ERR "%s %d RSQ OVERWRITE!!\n", __FUNCTION__, __LINE__);
        return 0;
    }
    /* update write pointer */
    if (ptr >= prsq->sndr_oob) {
        /* |--R---------------WdOddddp-----|  */
        /* data in oob, wrap to head of buffer*/
        prsq->sndr_wp = prsq->sndr_sbuf;
    } else {
        /* |--R--------Wddddp-R-O----------|  */
        prsq->sndr_wp = ptr;
    }

    prsq->wrno = slot->no;
    rvr->slotno++;
    // flush to pyhsical storage
    CamOsMemFlush((void *)rvr, sizeof(reservoir_t));
    CamOsMemFlush((void *)slot, sizeof(slot_t));
    return ws;
}

/*
 * rvr should be a noncache port (device memory)
 */
unsigned int rcvr_de_rsqslot(reservoir_t *rvr, void *buf, unsigned int size, slot_t *out)
{
    rsq_t           *prsq;
    unsigned char   *rp;

    prsq = &(rvr->rsq);

    /* sync with sender */
    prsq->rcvr_rp = SNDR_2_RCVR_PTR(prsq, prsq->sndr_rp);

    if (prsq->rdno == prsq->wrno) {
        out->buf = prsq->rcvr_sbuf + out->sc;
        return 0;  //empty!!
    }

    memcpy(out, prsq->rcvr_rp, sizeof(slot_t));
    rp = prsq->rcvr_rp + out->sc;

    /* for debuging, can remove!! */
    if (out->tag != RQ_TAG) {
        // CamOsPrintf(KERN_ERR "Not Slot header 0x%08x\n", (int)prsq->rp);
        rvr->sloterr++;
        out->buf = NULL;
        return 0;
    }

    if (buf && size > out->dc) {
        /* enough to copy */
        memcpy(buf, rp, out->dc);
        rp += out->bs;
        prsq->rcvr_rp = (rp >= prsq->rcvr_oob) ? prsq->rcvr_sbuf : rp;

        /* sync with sender */
        prsq->sndr_rp = RCVR_2_SNDR_PTR(prsq, prsq->rcvr_rp);
    } else {
        CamOsPrintf(KERN_ERR "buffer size %x is less than data size %x\n", size, out->dc);
        return 0;
    }

    prsq->rdno = out->no + 1;

    return out->dc;
}

/*
 * rvr should be a noncache port (device memory)
 */
unsigned int sndr_de_rsqslot(reservoir_t *rvr, void *buf, unsigned int size, slot_t *out)
{
    rsq_t           *prsq;
    unsigned char   *rp;

    prsq = &(rvr->rsq);

    if (prsq->rdno == prsq->wrno) {
        out->buf = prsq->sndr_sbuf + out->sc;
        return 0;  //empty!!
    }

    memcpy(out, prsq->sndr_rp, sizeof(slot_t));
    rp = prsq->sndr_rp + out->sc;

    /* for debuging, can remove!! */
    if (out->tag != RQ_TAG) {
        // CamOsPrintf(KERN_ERR "Not Slot header 0x%08x\n", (int)prsq->rp);
        rvr->sloterr++;
        out->buf = NULL;
        return 0;
    }

    if (buf && size > out->dc) {
        /* enough to copy */
        memcpy(buf, rp, out->dc);
        rp += out->bs;
        prsq->sndr_rp = (rp >= prsq->sndr_oob) ? prsq->sndr_sbuf : rp;

        /* sync with receiver */
        prsq->rcvr_rp = RCVR_2_SNDR_PTR(prsq, prsq->sndr_rp);
    } else {
        CamOsPrintf(KERN_ERR "buffer size %x is less than data size %x\n", size, out->dc);
        return 0;
    }

    prsq->rdno = out->no + 1;

    return out->dc;
}

slot_t *get_rsqslot(void *databuf)
{
    slot_t*	slot;

    slot = (slot_t*)((unsigned char*)databuf - SIZEOF_SLOTHEADER);
    if (slot->tag == RQ_TAG)
        return slot;
    return NULL;
}
