/*
* miu_bw.h- Sigmastar
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
#include <linux/device.h>

/*=============================================================*/
// Macro definition
/*=============================================================*/

#ifndef MIU_NUM
#define MIU_NUM         (2)
#endif

#define CPU_CLIENT_ID       (0x70)
#define MIU_GRP_CLIENT_NUM  (0x10)
#define MIU_GRP_NUM         (4)
#define MIU_CLIENT_NUM      (0x41)  // CPU + (16 clients per group, total 4 groups)

#define MIU_ARB_CLIENT_NUM  (0x40)  // MIU_CLIENT_NUM - CPU
#define MIU_ARB_CLIENT_ALL  (0)     // Client 0 means all clients

#define MIU_IDX(c)          (((c - '0') > MIU_NUM) ? 0 : c - '0')

// priority
#define MIU_ARB_PRIO_1ST    0
#define MIU_ARB_PRIO_2ND    1
#define MIU_ARB_PRIO_3RD    2
#define MIU_ARB_PRIO_4TH    3

// burst
#define MIU_ARB_BURST_NOLIM 4
#define MIU_ARB_BURST_OPT   5   // no limit + 4 different burst length option

// group flow control
#define MIU_ARB_CNT_PERIOD_MAX          (0xFF)
#define MIU_ARB_GET_CNT_EN(en)          (en&0xFF)
#define MIU_ARB_GET_CNT_ID0(id)         (id&0x0F)
#define MIU_ARB_GET_CNT_ID1(id)         ((id>>4)&0x0F)
#define MIU_ARB_GET_CNT_ID0_EN(en)      (en&0x0F)
#define MIU_ARB_GET_CNT_ID1_EN(en)      ((en>>4)&0x0F)
#define MIU_ARB_SET_CNT_ID0(id, i)      id = ((id&0xF0)|i)
#define MIU_ARB_SET_CNT_ID1(id, i)      id = ((id&0x0F)|(i<<4))
#define MIU_ARB_SET_CNT_ID0_EN(en, e)   en = ((en&0xF0)|e)
#define MIU_ARB_SET_CNT_ID1_EN(en, e)   en = ((en&0x0F)|(e<<4))

// dump
#define MIU_ARB_DUMP_TEXT   0
#define MIU_ARB_DUMP_REG    1
#define MIU_ARB_DUMP_MAX    2

// policy
#define MIU_ARB_POLICY_RR   0
#define MIU_ARB_POLICY_RT   1
#define MIU_ARB_POLICY_NUM  2

// log output
#define ASCII_COLOR_RED     "\033[1;31m"
#define ASCII_COLOR_WHITE   "\033[1;37m"
#define ASCII_COLOR_YELLOW  "\033[1;33m"
#define ASCII_COLOR_BLUE    "\033[1;36m"
#define ASCII_COLOR_GREEN   "\033[1;32m"
#define ASCII_COLOR_END     "\033[0m"

/*=============================================================*/
// Structure definition
/*=============================================================*/

struct miu_device {
    struct device dev;
    int index;
};

// common for all MIU
struct miu_client {
    const char *name;
    const short id;
    const short rsvd;
};

// dedicated for each MIU
struct miu_client_bw {
    short enabled;
    short dump_en;
    short filter_en;
    short max;
    short avg;
    short effi_avg;
    short effi_min;
};

struct arb_flowctrl {
    unsigned char cnt0_id;      // client ID, bit[3:0]: ID0, bit[7:4]: ID1
    unsigned char cnt0_period;  // mask period, (max. 255)
    unsigned char cnt0_enable;  // client enable, bit[3:0]: ID0, bit[7:4]: ID1
    unsigned char cnt1_id;      // client ID, bit[3:0]: ID0, bit[7:4]: ID1
    unsigned char cnt1_period;  // mask period, (max. 255)
    unsigned char cnt1_enable;  // client enable, bit[3:0]: ID0, bit[7:4]: ID1
};

struct miu_arb {
    char name[12];                          // device name (miu_arbx)
    char priority[MIU_ARB_CLIENT_NUM];      // client priority (0/1/2/3)
    char burst[MIU_ARB_CLIENT_NUM];         // client burst length (8/16/32/64), 0 => no limit
    char promote[MIU_ARB_CLIENT_NUM];       // client promote enable/disable
    struct arb_flowctrl fctrl[MIU_GRP_NUM]; // flow control, mask request for a period
    char dump;                              // dump mode: readable text, register table
    short client_selected;
};

/*=============================================================*/
// Export Functions
/*=============================================================*/

extern const char* miu_client_id_to_name(U16 id);
extern short miu_client_reserved(U16 id);
extern void create_miu_bw_node(struct bus_type *miu_subsys);

