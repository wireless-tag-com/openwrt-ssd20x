/*
* mdrv_hwnat_foe.h- Sigmastar
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
/// @file   MDRV_HWNAT_FDB.h
/// @brief  HWNAT Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef _MDRV_HWNAT_FOE_
#define _MDRV_HWNAT_FOE_

#include <net/ip.h>
#include "mdrv_hwnat_ioctl.h"

extern struct foe_entry *ppe_foe_base;
extern struct ps_entry *ppe_ps_base;

/* DEFINITIONS AND MACROS*/
#define FOE_ENTRY_LIFE_TIME 5
#define FOE_THRESHOLD       1000
#define FOE_HASH_MASK       0x00001FFF
#define FOE_HASH_WAY        2
#define FOE_1K_SIZ_MASK     0x000001FF
#define FOE_2K_SIZ_MASK     0x000003FF
#define FOE_4K_SIZ_MASK     0x000007FF
#define FOE_8K_SIZ_MASK     0x00000FFF
#define FOE_16K_SIZ_MASK    0x00001FFF

#if defined(CONFIG_RA_HW_NAT_TBL_1K)
#define FOE_4TB_SIZ     1024
#define FOE_4TB_BIT     10
#elif defined(CONFIG_RA_HW_NAT_TBL_2K)
#define FOE_4TB_SIZ     2048
#define FOE_4TB_BIT     11
#elif defined(CONFIG_RA_HW_NAT_TBL_4K)
#define FOE_4TB_SIZ     4096
#define FOE_4TB_BIT     12
#elif defined(CONFIG_RA_HW_NAT_TBL_8K)
#define FOE_4TB_SIZ     8192
#define FOE_4TB_BIT     13
#elif defined(CONFIG_RA_HW_NAT_TBL_16K)
#define FOE_4TB_SIZ     16384
#define FOE_4TB_BIT     14
#endif

#define FOE_ENTRY_SIZ       128 /* for ipv6 backward compatible */

#define IP_FORMAT3(addr) (((unsigned char *)&addr)[3])
#define IP_FORMAT2(addr) (((unsigned char *)&addr)[2])
#define IP_FORMAT1(addr) (((unsigned char *)&addr)[1])
#define IP_FORMAT0(addr) (((unsigned char *)&addr)[0])

struct pkt_parse_result {
    /*layer2 header */
    u8 dmac[6];
    u8 smac[6];

    /*vlan header */
    u16 vlan_tag;
    u16 vlan1_gap;
    u16 vlan1;
    u16 vlan2_gap;
    u16 vlan2;
    u16 vlan_layer;

    /*pppoe header */
    u32 pppoe_gap;
    u16 ppp_tag;
    u16 pppoe_sid;

    /*layer3 header */
    u16 eth_type;
    struct iphdr iph;
    struct ipv6hdr ip6h;

    /*layer4 header */
    struct tcphdr th;
    struct udphdr uh;

    u32 pkt_type;
    u8 is_mcast;

};

struct pkt_rx_parse_result {
    /*layer2 header */
    u8 dmac[6];
    u8 smac[6];

    /*vlan header */
    u16 vlan_tag;
    u16 vlan1_gap;
    u16 vlan1;
    u16 vlan2_gap;
    u16 vlan2;
    u16 vlan_layer;

    /*pppoe header */
    u32 pppoe_gap;
    u16 ppp_tag;
    u16 pppoe_sid;

    /*layer3 header */
    u16 eth_type;
    struct iphdr iph;
    struct ipv6hdr ip6h;

    /*layer4 header */
    struct tcphdr th;
    struct udphdr uh;

    u32 pkt_type;
    u8 is_mcast;

};

 /* TYPEDEFS AND STRUCTURES*/
enum FOE_TBL_SIZE {
    FOE_TBL_SIZE_1K,
    FOE_TBL_SIZE_2K,
    FOE_TBL_SIZE_4K,
    FOE_TBL_SIZE_8K,
    FOE_TBL_SIZE_16K
};

enum VLAN_ACTION {
    NO_ACT = 0,
    MODIFY = 1,
    INSERT = 2,
    DELETE = 3
};

enum FOE_ENTRY_STATE {
    INVALID = 0,
    UNBIND = 1,
    BIND = 2,
    FIN = 3
};

enum FOE_TBL_TCP_UDP {
    TCP = 0,
    UDP = 1,
    ANY = 2
};

enum FOE_TBL_EE {
    NOT_ENTRY_END = 0,
    ENTRY_END_FP = 1,
    ENTRY_END_FOE = 2
};

enum FOE_LINK_TYPE {
    LINK_TO_FOE = 0,
    LINK_TO_FP = 1
};

enum FOE_IP_ACT {
    IPV4_HNAPT = 0,
    IPV4_HNAT = 1,
    IPV6_1T_ROUTE = 2,
    IPV4_DSLITE = 3,
    IPV6_3T_ROUTE = 4,
    IPV6_5T_ROUTE = 5,
    IPV6_6RD = 7,
};

enum FOE_ENTRY_FMT {
    IPV4_NAPT=0,
    IPV4_NAT=1,
    IPV6_ROUTING=5
};

#define IS_IPV4_HNAPT(x)    (((x)->bfib1.pkt_type == IPV4_HNAPT) ? 1 : 0)
#define IS_IPV4_HNAT(x)     (((x)->bfib1.pkt_type == IPV4_HNAT) ? 1 : 0)
#define IS_IPV6_1T_ROUTE(x) (((x)->bfib1.pkt_type == IPV6_1T_ROUTE) ? 1 : 0)
#define IS_IPV4_DSLITE(x)   (((x)->bfib1.pkt_type == IPV4_DSLITE) ? 1 : 0)
#define IS_IPV6_3T_ROUTE(x) (((x)->bfib1.pkt_type == IPV6_3T_ROUTE) ? 1 : 0)
#define IS_IPV6_5T_ROUTE(x) (((x)->bfib1.pkt_type == IPV6_5T_ROUTE) ? 1 : 0)
#define IS_IPV6_6RD(x)      (((x)->bfib1.pkt_type == IPV6_6RD) ? 1 : 0)
#define IS_IPV4_GRP(x)      (IS_IPV4_HNAPT(x) | IS_IPV4_HNAT(x))
#define IS_IPV6_GRP(x)      (IS_IPV6_1T_ROUTE(x) | IS_IPV6_3T_ROUTE(x) | IS_IPV6_5T_ROUTE(x) | IS_IPV6_6RD(x) |  IS_IPV4_DSLITE(x))


/* state = unbind & dynamic */
struct ud_info_blk1 {
    uint32_t time_stamp:8;
    uint32_t pcnt:16;   /* packet count */
    uint32_t preb:1;
    uint32_t pkt_type:3;
    uint32_t state:2;
    uint32_t udp:1;
    uint32_t sta:1;     /* static entry */
};

/* state = bind & fin */
struct bf_info_blk1 {
    uint32_t time_stamp:15;
    uint32_t ka:1;      /* keep alive */
    uint32_t vlan_layer:3;
    uint32_t psn:1;     /* egress packet has PPPoE session */
    uint32_t vpm:1;     /* 0:ethertype remark, 1:0x8100(CR default) */
    uint32_t ps:1;      /* packet sampling */
    uint32_t cah:1;     /* cacheable flag */
    uint32_t rmt:1;     /* remove tunnel ip header (6rd/dslite only) */
    uint32_t ttl:1;
    uint32_t pkt_type:3;
    uint32_t state:2;
    uint32_t udp:1;
    uint32_t sta:1;     /* static entry */
};

struct _info_blk2 {
    uint32_t qid:4;     /* QID in Qos Port */
    uint32_t fqos:1;    /* force to PSE QoS port */
    uint32_t dp:3;      /* force to PSE port x *//*0:PSE,1:GSW, 2:GMAC,4:PPE,5:QDMA,7=DROP */
    uint32_t mcast:1;   /* multicast this packet to CPU */
    uint32_t pcpl:1;    /* OSBN */
    uint32_t mibf:1;
    uint32_t alen:1;
    uint32_t qid1:2;
    uint32_t noused:2;
    uint32_t wdmaid:1;
    uint32_t winfo:1;
    uint32_t acnt:6;
    uint32_t dscp:8;    /* DSCP value */
};

/* Foe Entry (64B) */
/*      IPV4:                IPV6: */
/*  +-----------------------+    +-----------------------+ */
/*  |  Information Block 1  |    |  Information Block 1  | */
/*  +-----------------------+    +-----------------------+ */
/*  |   SIP(4B)     |    |     IPv6_DIP0(4B)     | */
/*  +-----------------------+    +-----------------------+ */
/*  |   DIP(4B)     |    |     IPv6_DIP1(4B)     | */
/*  +-----------------------+    +-----------------------+ */
/*  | SPORT(2B) | DPORT(2B) |    |        Rev(4B)        | */
/*  +-----------+-----------+    +-----------------------+ */
/*  | Information Block 2   |    |  Information Block 2  | */
/*  +-----------------------+    +-----------------------+ */
/*  |      New SIP(4B)  |    |     IPv6_DIP2(4B)     | */
/*  +-----------------------+    +-----------------------+ */
/*  |      New DIP(4B)  |    |     IPv6_DIP3(4B)     | */
/*  +-----------------------+    +-----------------------+ */
/*  | New SPORT | New DPORT |    |         Rev(4B)       | */
/*  +-----------+-----------+    +-----------------------+ */
/*  | VLAN1(2B) |DMAC[47:32]|    | VLAN1(2B) |DMAC[47:32]| */
/*  +-----------|-----------+    +-----------|-----------+ */
/*  |   DMAC[31:0]      |    |       DMAC[31:0]      | */
/*  +-----------------------+    +-----------------------+ */
/*  | PPPoE_ID  |SMAC[47:32]|    | PPPoE_ID  |SMAC[47:32]| */
/*  +-----------+-----------+    +-----------+-----------+ */
/*  |       SMAC[31:0]      |    |       SMAC[31:0]      | */
/*  +-----------------------+    +-----------------------+ */
/*  | Rev |  SNAP_Ctrl(3B)  |    | Rev |  SNAP_Ctrl(3B)  | */
/*  +-----------------------+    +-----------------------+ */
/*  |    Rev    | VLAN2(2B) |    |   Rev     | VLAN2(2B) | */
/*  +-----------------------+    +-----------------------+ */
/*  |     Rev(4B)           |    |       Rev(4B)         | */
/*  +-----------------------+    +-----------------------+ */
/*  |     tmp_buf(4B)       |    |       tmp_buf(4B)     | */
/*  +-----------------------+    +-----------------------+ */
/* Foe Entry (80) */
/* */
/*      IPV4 HNAPT:              IPV4: */
/*  +-----------------------+    +-----------------------+ */
/*  |  Information Block 1  |    |  Information Block 1  | */
/*  +-----------------------+    +-----------------------+ */
/*  |   SIP(4B)     |    |      SIP(4B)      | */
/*  +-----------------------+    +-----------------------+ */
/*  |   DIP(4B)     |    |      DIP(4B)      | */
/*  +-----------------------+    +-----------------------+ */
/*  | SPORT(2B) | DPORT(2B) |    |        Rev(4B)        | */
/*  +-----------+-----------+    +-----------------------+ */
/*  | EG DSCP| Info Block 2 |    |  Information Block 2  | */
/*  +-----------------------+    +-----------------------+ */
/*  |      New SIP(4B)  |    |     New SIP (4B)      | */
/*  +-----------------------+    +-----------------------+ */
/*  |      New DIP(4B)  |    |     New DIP (4B)      | */
/*  +-----------------------+    +-----------------------+ */
/*  | New SPORT | New DPORT |    | New SPORT | New DPORT | */
/*  +-----------+-----------+    +-----------------------+ */
/*  |          REV          |    |      REV      | */
/*  +-----------------------+    +-----------------------+ */
/*  |Act_dp|   REV          |    |Act_dp|   REV      | */
/*  +-----------------------+    +-----------------------+ */
/*  |      tmp_buf(4B)      |    |       temp_buf(4B)    | */
/*  +-----------------------+    +-----------|-----------+ */
/*  | ETYPE     | VLAN1 ID  |    | ETYPE     |  VLAN1    | */
/*  +-----------+-----------+    +-----------+-----------+ */
/*  |       DMAC[47:16]     |    |       SMAC[47:16]     | */
/*  +-----------------------+    +-----------------------+ */
/*  | DMAC[15:0]| VLAN2 ID  |    | DMAC[15:0]|  VLAN2    | */
/*  +-----------------------+    +-----------------------+ */
/*  |       SMAC[47:16]     |    |       SMAC[47:16]     | */
/*  +-----------------------+    +-----------------------+ */
/*  | SMAC[15:0]| PPPOE ID  |    | SMAC[15:0]| PPPOE ID  | */
/*  +-----------------------+    +-----------------------+ */
/*                               */

struct _ipv4_hnapt {
    union {
        struct ud_info_blk1 udib1;
        struct bf_info_blk1 bfib1;
        u32 info_blk1;
    };
    u32 sip;
    u32 dip;
    u16 dport;
    u16 sport;
    union {
        struct _info_blk2 iblk2;
        u32 info_blk2;
    };
    u32 new_sip;
    u32 new_dip;
    u16 new_dport;
    u16 new_sport;
    u32 resv1;
    u32 resv2;
    uint32_t resv3:26;
    uint32_t act_dp:6;  /* UDF */
    u16 vlan1;
    u16 etype;
    u8 dmac_hi[4];
    union {
        u16 vlan2_winfo;
        u16 vlan2;
    };
    u8 dmac_lo[2];
    u8 smac_hi[4];
    u16 pppoe_id;
    u8 smac_lo[2];
};

struct _ipv4_dslite {
    union {
        struct ud_info_blk1 udib1;
        struct bf_info_blk1 bfib1;
        u32 info_blk1;
    };
    u32 sip;
    u32 dip;
    u16 dport;
    u16 sport;

    u32 tunnel_sipv6_0;
    u32 tunnel_sipv6_1;
    u32 tunnel_sipv6_2;
    u32 tunnel_sipv6_3;

    u32 tunnel_dipv6_0;
    u32 tunnel_dipv6_1;
    u32 tunnel_dipv6_2;
    u32 tunnel_dipv6_3;

    u8 flow_lbl[3]; /* in order to consist with Linux kernel (should be 20bits) */
    uint16_t priority:4;    /* in order to consist with Linux kernel (should be 8bits) */
    uint16_t resv1:4;
    uint32_t hop_limit:8;
    uint32_t resv2:18;
    uint32_t act_dp:6;  /* UDF */

    union {
        struct _info_blk2 iblk2;
        u32 info_blk2;
    };

    u16 vlan1;
    u16 etype;
    u8 dmac_hi[4];
    union {
        u16 vlan2_winfo;
        u16 vlan2;
    };
    u8 dmac_lo[2];
    u8 smac_hi[4];
    u16 pppoe_id;
    u8 smac_lo[2];
};

struct _ipv6_1t_route {
    union {
        struct ud_info_blk1 udib1;
        struct bf_info_blk1 bfib1;
        u32 info_blk1;
    };
    u32 ipv6_dip0;
    u32 ipv6_dip1;
    u32 resv;

    union {
        struct _info_blk2 iblk2;
        u32 info_blk2;
    };

    u32 ipv6_dip2;
    u32 ipv6_dip3;
    u32 resv1;

    uint32_t act_dp:6;  /* UDF */
    u16 vlan1;
    u16 etype;
    u8 dmac_hi[4];
    union {
        u16 vlan2_winfo;
        u16 vlan2;
    };
    u8 dmac_lo[2];
    u8 smac_hi[4];
    u16 pppoe_id;
    u8 smac_lo[2];
};

struct _ipv6_3t_route {
    union {
        struct ud_info_blk1 udib1;
        struct bf_info_blk1 bfib1;
        u32 info_blk1;
    };
    u32 ipv6_sip0;
    u32 ipv6_sip1;
    u32 ipv6_sip2;
    u32 ipv6_sip3;
    u32 ipv6_dip0;
    u32 ipv6_dip1;
    u32 ipv6_dip2;
    u32 ipv6_dip3;
    uint32_t prot:8;
    uint32_t resv:24;

    u32 resv1;
    u32 resv2;
    u32 resv3;
    uint32_t resv4:26;
    uint32_t act_dp:6;  /* UDF */

    union {
        struct _info_blk2 iblk2;
        u32 info_blk2;
    };
    u16 vlan1;
    u16 etype;
    u8 dmac_hi[4];
    union {
        u16 vlan2_winfo;
        u16 vlan2;
    };
    u8 dmac_lo[2];
    u8 smac_hi[4];
    u16 pppoe_id;
    u8 smac_lo[2];
};

struct _ipv6_5t_route {
    union {
        struct ud_info_blk1 udib1;
        struct bf_info_blk1 bfib1;
        u32 info_blk1;
    };
    u32 ipv6_sip0;
    u32 ipv6_sip1;
    u32 ipv6_sip2;
    u32 ipv6_sip3;
    u32 ipv6_dip0;
    u32 ipv6_dip1;
    u32 ipv6_dip2;
    u32 ipv6_dip3;
    u16 dport;
    u16 sport;

    u32 resv1;
    u32 resv2;
    u32 resv3;
    uint32_t resv4:26;
    uint32_t act_dp:6;  /* UDF */

    union {
        struct _info_blk2 iblk2;
        u32 info_blk2;
    };

    u16 vlan1;
    u16 etype;
    u8 dmac_hi[4];
    union {
        u16 vlan2_winfo;
        u16 vlan2;
    };
    u8 dmac_lo[2];
    u8 smac_hi[4];
    u16 pppoe_id;
    u8 smac_lo[2];
};

struct _ipv6_6rd {
    union {
        struct ud_info_blk1 udib1;
        struct bf_info_blk1 bfib1;
        u32 info_blk1;
    };
    u32 ipv6_sip0;
    u32 ipv6_sip1;
    u32 ipv6_sip2;
    u32 ipv6_sip3;
    u32 ipv6_dip0;
    u32 ipv6_dip1;
    u32 ipv6_dip2;
    u32 ipv6_dip3;
    u16 dport;
    u16 sport;

    u32 tunnel_sipv4;
    u32 tunnel_dipv4;
    uint32_t hdr_chksum:16;
    uint32_t dscp:8;
    uint32_t ttl:8;
    uint32_t flag:3;
    uint32_t resv1:13;
    uint32_t per_flow_6rd_id:1;
    uint32_t resv2:9;
    uint32_t act_dp:6;  /* UDF */

    union {
        struct _info_blk2 iblk2;
        u32 info_blk2;
    };

    u16 vlan1;
    u16 etype;
    u8 dmac_hi[4];
    union {
        u16 vlan2_winfo;
        u16 vlan2;
    };
    u8 dmac_lo[2];
    u8 smac_hi[4];
    u16 pppoe_id;
    u8 smac_lo[2];

};

struct foe_entry {
    union {
        struct ud_info_blk1 udib1;
        struct bf_info_blk1 bfib1;  /* common header */
        struct _ipv4_hnapt ipv4_hnapt;  /* nat & napt share same data structure */
        struct _ipv4_dslite ipv4_dslite;
        struct _ipv6_1t_route ipv6_1t_route;
        struct _ipv6_3t_route ipv6_3t_route;
        struct _ipv6_5t_route ipv6_5t_route;
        struct _ipv6_6rd ipv6_6rd;
    };
};

struct ps_entry {
    u8 en;
    u8 acl;
    u16 pkt_len;
    u16 pkt_cnt;
    u8 time_period;
    u8 resv0;
    u32 resv1;
    u16 hw_pkt_cnt;
    u16 hw_time;

};

struct mib_entry {
    u32 byt_cnt_l;
    u16 byt_cnt_h;
    u32 pkt_cnt_l;
    u8 pkt_cnt_h;
    u8 resv0;
    u32 resv1;
} __packed;

struct foe_pri_key {
    /* TODO: add new primary key to support dslite, 6rd */

    /* Ipv4 */
    struct {
        uint32_t sip;
        uint32_t dip;
        uint16_t sport;
        uint16_t dport;
        uint32_t is_udp:1;
    } ipv4_hnapt;

    struct {
        uint32_t sip;
        uint32_t dip;
        /* TODO */
    } ipv4_hnat;

    struct {
        uint32_t sip;
        uint32_t dip;
        /* TODO */
    } ipv4_dslite;

    /* IPv6 */
    struct {
        uint32_t sip0;
        uint32_t sip1;
        uint32_t sip2;
        uint32_t sip3;
        uint32_t dip0;
        uint32_t dip1;
        uint32_t dip2;
        uint32_t dip3;
        uint16_t sport;
        uint16_t dport;
        uint32_t is_udp:1;
    } ipv6_routing;

    struct {
        /* TODO */
    } ipv6_6rd;

    uint32_t pkt_type;   /* entry format */
};

void MDrv_HWNAT_Foe_Dump_Entry(uint32_t index);
int MDrv_HWNAT_Foe_Get_All_Entries(struct hwnat_args *opt);
int MDrv_HWNAT_Foe_Set_Bind(struct hwnat_args *opt);
int MDrv_HWNAT_Foe_Set_Unbind(struct hwnat_args *opt);
int MDrv_HWNAT_Foe_Drop_Entry(struct hwnat_args *opt);
int MDrv_HWNAT_Foe_Delete_Entry(uint32_t entry_num);
void MDrv_HWNAT_Foe_Clean_All_Entries(void);
void MDrv_HWNAT_Foe_Set_High_Mac(u8 *dst, uint8_t *src);
void MDrv_HWNAT_Foe_Set_Low_Mac(u8 *dst, uint8_t *src);
int MDrv_HWNAT_Add_Foe_Entry(struct hwnat_tuple *opt);
int MDrv_HWNAT_Delete_Foe_Entry(struct hwnat_tuple *opt);

#endif  /* _MDRV_HWNAT_FOE_ */
