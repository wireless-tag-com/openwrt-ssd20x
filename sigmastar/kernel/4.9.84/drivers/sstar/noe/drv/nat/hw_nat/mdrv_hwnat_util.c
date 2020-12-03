/*
* mdrv_hwnat_util.c- Sigmastar
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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (“MStar Confidential Information”) by the recipien
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   MDRV_HWNAT_UTIL.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ctype.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/ipv6.h>
#include "mdrv_hwnat_foe.h"
#include "mdrv_hwnat_util.h"

void MacReverse(uint8_t * Mac)
{
    uint8_t tmp;
    uint8_t i;

    for (i = 5; i > 2; i--) {
        tmp = Mac[i];
        Mac[i] = Mac[5 - i];
        Mac[5 - i] = tmp;
    }
}

static int _GetNext(char *src, int separator, char *dest)
{
    char *c;
    int len = 0;

    if ((src == NULL) || (dest == NULL)) {
        return -1;
    }

    c = strchr(src, separator);
    if (c == NULL) {
        strncpy(dest, src, len);
        return -1;
    }
    len = c - src;
    strncpy(dest, src, len);
    dest[len] = '\0';
    return len + 1;
}

static inline int atoi(char *s)
{
    int i = 0;
    while (isdigit(*s)) {
        i = i * 10 + *(s++) - '0';
    }
    return i;
}



unsigned int MDrv_HWNAT_Str2Ip(char *str)
{
    int len;
    char *ptr = str;
    char buf[128];
    unsigned char c[4];
    int i;
    for (i = 0; i < 3; ++i) {
        if ((len = _GetNext(ptr, '.', buf)) == -1) {
            return 1;   /* parsing error */
        }
        c[i] = atoi(buf);
        ptr += len;
    }
    c[3] = atoi(ptr);
    return ((c[0] << 24) + (c[1] << 16) + (c[2] << 8) + c[3]);
}

void reg_modify_bits(unsigned int *Addr, uint32_t Data, uint32_t Offset, uint32_t Len)
{
#if 0
    unsigned int mask = 0;
    unsigned int value;
    unsigned int i;

    for (i = 0; i < len; i++) {
        mask |= 1 << (offset + i);
    }

    value = sysregread(addr);
    value &= ~mask;
    value |= (data << offset) & mask;;
    sysregwrite(addr, value);
#endif
}
static inline uint16_t CsumPart(uint32_t o, uint32_t n, uint16_t old)
{
    uint32_t d[] = { o, n };
    return csum_fold(csum_partial((char *)d, sizeof(d), old ^ 0xFFFF));
}

/*
 * KeepAlive with new header mode will pass the modified packet to cpu.
 * We must change to original packet to refresh NAT table.
 */

/*
 * Recover TCP Src/Dst Port and recalculate tcp checksum
 */
void foe_to_org_tcphdr(struct foe_entry *entry, struct iphdr *iph,
            struct tcphdr *th)
{
    /* TODO: how to recovery 6rd/dslite packet */
    th->check =
        CsumPart((th->source) ^ 0xffff,
             htons(entry->ipv4_hnapt.sport), th->check);
    th->check =
        CsumPart((th->dest) ^ 0xffff,
             htons(entry->ipv4_hnapt.dport), th->check);
    th->check =
        CsumPart(~(iph->saddr), htonl(entry->ipv4_hnapt.sip),
             th->check);
    th->check =
        CsumPart(~(iph->daddr), htonl(entry->ipv4_hnapt.dip),
             th->check);
    th->source = htons(entry->ipv4_hnapt.sport);
    th->dest = htons(entry->ipv4_hnapt.dport);
}

/*
 * Recover UDP Src/Dst Port and recalculate udp checksum
 */
void
foe_to_org_udphdr(struct foe_entry *entry, struct iphdr *iph,
            struct udphdr *uh)
{
    /* TODO: how to recovery 6rd/dslite packet */

    uh->check =
        CsumPart((uh->source) ^ 0xffff,
             htons(entry->ipv4_hnapt.sport), uh->check);
    uh->check =
        CsumPart((uh->dest) ^ 0xffff,
             htons(entry->ipv4_hnapt.dport), uh->check);
    uh->check =
        CsumPart(~(iph->saddr), htonl(entry->ipv4_hnapt.sip),
             uh->check);
    uh->check =
        CsumPart(~(iph->daddr), htonl(entry->ipv4_hnapt.dip),
             uh->check);
    uh->source = htons(entry->ipv4_hnapt.sport);
    uh->dest = htons(entry->ipv4_hnapt.dport);
}

/*
 * Recover Src/Dst IP and recalculate ip checksum
 */
void foe_to_org_iphdr(struct foe_entry *entry, struct iphdr *iph)
{
    /* TODO: how to recovery 6rd/dslite packet */
    iph->saddr = htonl(entry->ipv4_hnapt.sip);
    iph->daddr = htonl(entry->ipv4_hnapt.dip);
    iph->check = 0;
    iph->check = ip_fast_csum((unsigned char *)(iph), iph->ihl);
}

void hwnat_memcpy(void *dest, void *src, u32 n)
{
    memcpy(dest, src, n);
}










/* Convert IP address from Hex to string */
uint8_t *MDrv_HWNAT_Util_Ip_To_Str(uint32_t Ip)
{
    static uint8_t Buf[32];
    uint8_t *ptr = (char *)&Ip;
    uint8_t c[4];

    c[0] = *(ptr);
    c[1] = *(ptr + 1);
    c[2] = *(ptr + 2);
    c[3] = *(ptr + 3);
    sprintf(Buf, "%d.%d.%d.%d", c[3], c[2], c[1], c[0]);
    return Buf;
}


unsigned int MDrv_HWNAT_Util_Str_To_Ip(char *str)
{
    int len;
    char *ptr = str;
    char buf[128];
    unsigned char c[4];
    int i;
    for (i = 0; i < 3; ++i) {
        if ((len = _GetNext(ptr, '.', buf)) == -1) {
            return 1;   /* parsing error */
        }
        c[i] = atoi(buf);
        ptr += len;
    }
    c[3] = atoi(ptr);
    return ((c[0] << 24) + (c[1] << 16) + (c[2] << 8) + c[3]);
}

void MDrv_HWNAT_Util_Calc_Tcphdr(struct foe_entry *entry, struct iphdr *iph, struct tcphdr *th)
{
    /* TODO: how to recovery 6rd/dslite packet */
    th->check = CsumPart((th->source) ^ 0xffff, htons(entry->ipv4_hnapt.sport), th->check);
    th->check = CsumPart((th->dest) ^ 0xffff, htons(entry->ipv4_hnapt.dport), th->check);
    th->check = CsumPart(~(iph->saddr), htonl(entry->ipv4_hnapt.sip), th->check);
    th->check = CsumPart(~(iph->daddr), htonl(entry->ipv4_hnapt.dip), th->check);
    th->source = htons(entry->ipv4_hnapt.sport);
    th->dest = htons(entry->ipv4_hnapt.dport);

}

void MDrv_HWNAT_Util_Calc_Udphdr(struct foe_entry *entry, struct iphdr *iph, struct udphdr *uh)
{
    /* TODO: how to recovery 6rd/dslite packet */
    uh->check = CsumPart((uh->source) ^ 0xffff, htons(entry->ipv4_hnapt.sport), uh->check);
    uh->check = CsumPart((uh->dest) ^ 0xffff, htons(entry->ipv4_hnapt.dport), uh->check);
    uh->check = CsumPart(~(iph->saddr), htonl(entry->ipv4_hnapt.sip), uh->check);
    uh->check = CsumPart(~(iph->daddr), htonl(entry->ipv4_hnapt.dip), uh->check);
    uh->source = htons(entry->ipv4_hnapt.sport);
    uh->dest = htons(entry->ipv4_hnapt.dport);
}

void MDrv_HWNAT_Util_Calc_Iphdr(struct foe_entry *entry, struct iphdr *iph)
{
    /* TODO: how to recovery 6rd/dslite packet */
    iph->saddr = htonl(entry->ipv4_hnapt.sip);
    iph->daddr = htonl(entry->ipv4_hnapt.dip);
    iph->check = 0;
    iph->check = ip_fast_csum((unsigned char *)(iph), iph->ihl);

}


void MDrv_HWNAT_Util_Memcpy(void *dest, void *src, u32 n)
{
#if 0
    ether_addr_copy(dest, src);
#else
    memcpy(dest, src, n);
#endif
}


