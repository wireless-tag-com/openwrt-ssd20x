/*
* mhal_hwnat_entry.h- Sigmastar
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
// (“MStar Confidential Information”) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   Mhal_hwnat_entry.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MHAL_HWNAT_ENTRY_H
#define MHAL_HWNAT_ENTRY_H

#ifndef ETH_HLEN
#define ETH_HLEN (14)
#endif

#ifndef VLAN_LEN
#define VLAN_LEN       (4)
#endif


#define MAX_MCAST_ENTRY     16
#define MAX_MCAST_ENTRY16_63    48
#define MAX_MCAST_ENTRY_TOTOAL  64

typedef struct {
    uint32_t    mc_vid:12;
    uint32_t    mc_qos_qid54:2;
    uint32_t    valid:1;
    uint32_t    rev1:1;
    uint32_t    mc_px_en:4;
    uint32_t    mc_mpre_sel:2; //0=01:00, 2=33:33
    uint32_t    mc_vid_cmp:1;
    uint32_t    rev2:1;
    uint32_t    mc_px_qos_en:4;
    uint32_t    mc_qos_qid:4;
} ppe_mcast_h;

typedef struct {
    uint8_t mc_mac_addr[4]; //mc_mac_addr[31:0]
} ppe_mcast_l;


#define GET_PPE_MCAST_H(idx)        ((ppe_mcast_h *)(PPE_MCAST_H_0 + ((idx) * 8)))
#define GET_PPE_MCAST_L(idx)        ((ppe_mcast_l *)(PPE_MCAST_L_0 + ((idx) * 8)))
#define GET_PPE_MCAST_H10(idx)      ((ppe_mcast_h *)(PPE_MCAST_H_10 + ((idx) * 8)))
#define GET_PPE_MCAST_L10(idx)      ((ppe_mcast_l *)(PPE_MCAST_L_10 + ((idx) * 8)))



#define PPTP_TCP_PORT           1723
#define PPP_ADDRESS_CONTROL     0xff03
/* gre header structure: -------------------------------------------- */

#define PPTP_GRE_PROTO  0x880B
#define PPTP_GRE_VER    0x1

#define PPTP_GRE_FLAG_C 0x80
#define PPTP_GRE_FLAG_R 0x40
#define PPTP_GRE_FLAG_K 0x20
#define PPTP_GRE_FLAG_S 0x10
#define PPTP_GRE_FLAG_A 0x80

#define PPTP_GRE_IS_C(f) ((f)&PPTP_GRE_FLAG_C)
#define PPTP_GRE_IS_R(f) ((f)&PPTP_GRE_FLAG_R)
#define PPTP_GRE_IS_K(f) ((f)&PPTP_GRE_FLAG_K)
#define PPTP_GRE_IS_S(f) ((f)&PPTP_GRE_FLAG_S)
#define PPTP_GRE_IS_A(f) ((f)&PPTP_GRE_FLAG_A)

/* 16 bytes GRE header */
struct pptp_gre_header {
    u8 flags;             /* bitfield */
    u8 ver;                       /* should be PPTP_GRE_VER (enhanced GRE) */
    u16 protocol;         /* should be PPTP_GRE_PROTO (ppp-encaps) */
    u16 payload;      /* size of ppp payload, not inc. gre header */
    u16 cid;          /* peer's call_id for this session */
    u32 seq;              /* sequence number.  Present if S==1 */
    u32 ack;              /* seq number of highest packet recieved by */
    /*  sender in this session */
} __packed;


struct hnat_pptp
{
    uint32_t tx_seqno;
    uint32_t rx_seqno;
    uint32_t saddr;
    uint32_t daddr;
    uint16_t call_id;
    uint16_t call_id_udp;/*tcp udp with different ID*/
    uint16_t call_id_tcp;
    uint16_t tx_ip_id;
    uint8_t eth_header[ETH_HLEN];
    uint32_t key;/*MT7620:add key*/
};

/*L2TP*/
struct hnat_l2tp
{
    uint32_t daddr;                     /* DIP */
    uint32_t saddr;             /* SIP */
    uint16_t tid;                     /* Tunnel ID */
    uint16_t sid;                     /* Session ID */
    uint16_t source;                  /* UDP source port */
    uint16_t dest;                    /* UDP dest port */
    uint8_t eth_header[ETH_HLEN];
};

struct l2tp_add_hdr
{
    uint16_t source; /* UDP */
    uint16_t dest;
    uint16_t len;
    uint16_t checksum;
    uint16_t type; /* L2TP */
    uint16_t tid;
    uint16_t sid;
};


struct hnat_l2tp_parse
{
    uint16_t ver;                   /* Packets Type */
    uint16_t length;                /* Length (Optional)*/
    uint16_t tid;                   /* Tunnel ID */
    uint16_t sid;                   /* Session ID */
};

struct ppp_hdr {
    uint16_t addr_ctrl;
    uint16_t protocol;
};



enum L2RuleDir {
    OTHERS = 0,
    DMAC = 1,
    SMAC = 2,
    SDMAC = 3
};

enum L3RuleDir {
    IP_QOS = 0,
    DIP = 1,
    SIP = 2,
    SDIP = 3
};

enum L4RuleDir {
    DONT_CARE = 0,
    DPORT = 1,
    SPORT = 2,
    SDPORT = 3
};

enum RuleType {
    L2_RULE = 0,
    L3_RULE = 1,
    L4_RULE = 2,
    PT_RULE = 3
};

enum PortNum {
    PN_CPU = 0,
    PN_GE1 = 1,
    PN_DONT_CARE = 7
};

enum OpCode {
    AND = 0,
    OR = 1
};

enum TcpFlag {
    TCP_FIN = 1,
    TCP_SYN = 2,
    TCP_RESET = 4,
    TCP_PUSH = 8,
    TCP_ACK = 16,
    TCP_URGENT = 32
};

enum TcpFlagOp {
    EQUAL = 0,
    NOT_EQUAL = 1,
    IN_SET = 2,
    NOT_IN_SET = 3
};

enum L4Type {
    FLT_IP_PROT = 0,
    FLT_UDP = 1,
    FLT_TCP = 2,
    FLT_TCP_UDP = 3
};

enum FpnType {
    FPN_CPU = 0,
    FPN_GE1 = 1,
    FPN_GE2 = 2,
    FPN_FRC_PRI_ONLY = 5,
    FPN_ALLOW = 4,
    FPN_DROP = 7
};

struct common_field {

    union {
        struct {
            uint8_t ee:1;   /* entry end */
            uint8_t resv:6;
            uint8_t logic:1;    /* logic operation with next rule (AND or OR) */
        } ee_0;     /* entry end =0 */

        struct {
            uint8_t ee:1;   /* entry end */
            uint8_t dop:1;  /* drop out profile */
            uint8_t mg:6;   /* meter group */
        } mtr;      /* meter */

        struct {
            uint8_t ee:1;   /* entry end */
            uint8_t rsv:1;
            uint8_t ag:6;   /* accounting group */
        } ac;       /* account */

        struct {
            uint8_t ee:1;   /* entry end */
            uint8_t fpp:1;  /* Force Destination Port */
            uint8_t fpn:3;  /* Force Port Number */
            uint8_t up:3;   /* User Perority */
        } fpp;      /* force destination port & force new user priority */

        struct {
            uint8_t ee:1;   /* entry end */
            uint8_t foe:1;  /* is Foe entry */
            uint8_t foe_tb:6;   /* FoE Table Entry */
        } foe;
    };

    uint8_t match:1;    /* 0: non-equal, 1: equal */
    uint8_t pn:3;       /* port number, Pre: ingress port/Post: egress port */
    uint8_t rt:2;       /* Rule type */
    uint8_t dir:2;      /* Direction */
};

struct l2_rule {

    struct common_field com;

    union {
        uint8_t mac[6];
        struct {
            uint16_t v:1;   /* compare VIDX */
            uint16_t s:1;   /* compare special tag */
            uint16_t e:1;   /* compare ethernet type */
            uint16_t p:1;   /* compare pppoe session id */
            uint16_t vid:12;    /* vlan id */
            uint16_t etyp_pprot;    /* ethernet type(p=0) or pppoe protocol(p=1) */
            uint16_t pppoe_id;
        } others;
    };
} __attribute__ ((packed));


struct l3_rule {

    struct common_field com;

    union {

        /* ip boundary = ip ~ ip + (ip_rng_m << ip_rng_e) */
        struct {
            uint32_t ip;
            uint16_t ip_rng_m:8;    /* ip range mantissa part */
            uint16_t ip_rng_e:5;    /* ip range exponent part */
            uint16_t v4:1;  /* ipv4/ipv6 */
            uint16_t ip_seg:2;  /* segment for ipv */
        } ip;

        struct {
            uint32_t tos_s:8;   /* start of ipv4 tos range */
            uint32_t tos_e:8;   /* end of ipv4 tos range */
            uint32_t resv:16;
            uint16_t resv1:4;
            uint16_t foz:1; /* IPv4 fragment offset zero */
            uint16_t mf:1;  /* Ipv4 more fragments flag */
            uint16_t fov:1; /* IPv4 fragment offset valid */
            uint16_t mfv:1; /*IPv4 more fragments flag valid */
            uint16_t rsv:5;
            uint16_t v4:1;  /* this rule is for ipv4 or ipv6 */
            uint16_t rsv1:2;
        } qos;
    };
} __attribute__ ((packed));

struct l4_rule {

    struct common_field com;

    uint16_t p_start;   /* start of port range */
    uint16_t p_end;     /* end of port range */

    union {
        struct {
            uint16_t tcp_fop:2; /* TCP flag operations */
            uint16_t tcp_f:6;   /* Expected value of TCP flags (U/A/P/R/S/F) */
            uint16_t tcp_fm:6;  /* Mask of TCP flags */
            uint16_t tu:2;  /* 11:tcp/udp, 10:tcp, 01:udp, 00:ip_proto */
        } tcp;

        struct {
            uint16_t resv:14;
            uint16_t tu:2;  /* 11:tcp/udp, 10:tcp, 01:udp, 00:ip_proto */
        } udp;

        struct {
            uint16_t prot:8;    /* ip protocol field */
            uint16_t resv:6;
            uint16_t tu:2;  /* 11:tcp/udp, 10:tcp, 01:udp, 00:ip_proto */
        } ip;
    };
} __attribute__ ((packed));



#endif /* MHAL_HWNAT_ENTRY_H */
