/*
* mdrv_noe_lro_proc.c- Sigmastar
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
/// @file   MDRV_NOE_LRO_PROC.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/init.h>
#include <generated/autoconf.h>
#include <linux/mii.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/pci.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/ctype.h>

#if defined(CONFIG_MIPS)
#include <asm/mips-boards/prom.h>
#include "mhal_chiptop_reg.h"
#elif defined(CONFIG_ARM)
#include <asm/prom.h>
#include <asm/mach/map.h>
#elif defined(CONFIG_ARM64)
#include <asm/arm-boards/prom.h>
#include <asm/mach/map.h>
#endif
#include <linux/proc_fs.h>

#include "mdrv_noe.h"
#include "mdrv_noe_proc.h"


//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------

#define HW_LRO_RING_NUM 3
#define MAX_HW_LRO_AGGR 64
#define HW_LRO_AGG_FLUSH        (1)
#define HW_LRO_AGE_FLUSH        (2)
#define HW_LRO_NOT_IN_SEQ_FLUSH (3)
#define HW_LRO_TIMESTAMP_FLUSH  (4)
#define HW_LRO_NON_RULE_FLUSH   (5)



//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------

typedef int (*LRO_PROC_OPEN) (struct inode *, struct file *);
typedef int (*LRO_PROC_SET) (int);

enum {
    E_LRO_PROC_RING1 = 0,
    E_LRO_PROC_RING2,
    E_LRO_PROC_RING3,
    E_LRO_PROC_STATS,
    E_LRO_PROC_ATTLB,
    E_LRO_PROC_MAX,
};


enum {
    E_AGG_CNT = 0,
    E_AGG_TIME,
    E_AGE_TIME,
    E_BW_THRESHOLD,
    E_RING_CONTROL,
};


//--------------------------------------------------------------------------------------------------
//  Local Functions
//--------------------------------------------------------------------------------------------------
static int _MDrv_NOE_LRO_PROC_Show_Rx_Ring1(struct seq_file *seq, void *offset);
static int _MDrv_NOE_LRO_PROC_Show_Rx_Ring2(struct seq_file *seq, void *offset);
static int _MDrv_NOE_LRO_PROC_Show_Rx_Ring3(struct seq_file *seq, void *offset);
static int _MDrv_NOE_LRO_PROC_Show_Auto_Tlb(struct seq_file *seq, void *offset);
static int _MDrv_NOE_LRO_PROC_Show_Stats(struct seq_file *seq, void *offset);
static int _MDrv_NOE_LRO_PROC_Open_Rx_Ring1(struct inode *inode, struct file *file);
static int _MDrv_NOE_LRO_PROC_Open_Rx_Ring2(struct inode *inode, struct file *file);
static int _MDrv_NOE_LRO_PROC_Open_Rx_Ring3(struct inode *inode, struct file *file);
static int _MDrv_NOE_LRO_PROC_Open_Auto_Tlb(struct inode *inode, struct file *file);
static int _MDrv_NOE_LRO_PROC_Open_Stats(struct inode *inode, struct file *file);
static ssize_t _MDrv_NOE_LRO_PROC_Write_Stats(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static ssize_t _MDrv_NOE_LRO_PROC_Write_Auto_Tlb(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static int _MDrv_NOE_LRO_PROC_Update_Len(unsigned int agg_size);
static int _MDrv_NOE_LRO_PROC_Set_Agg_Cnt(int par2);
static int _MDrv_NOE_LRO_PROC_Set_Agg_Time(int par2);
static int _MDrv_NOE_LRO_PROC_Set_Age_Time(int par2);
static int _MDrv_NOE_LRO_PROC_Set_Threshold(int par2);
static int _MDrv_NOE_LRO_PROC_Enable_Ring_Ctrl(int par2);

//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------
static struct net_device *lro_proc_dev;
static unsigned int lro_agg_num_cnt[HW_LRO_RING_NUM][MAX_HW_LRO_AGGR + 1];
static unsigned int lro_agg_size_cnt[HW_LRO_RING_NUM][16];
static unsigned int lro_tot_agg_cnt[HW_LRO_RING_NUM];
static unsigned int lro_flush_cnt_tot[HW_LRO_RING_NUM];
static unsigned int lro_flush_cnt_agg[HW_LRO_RING_NUM];
static unsigned int lro_flush_cnt_age[HW_LRO_RING_NUM];
static unsigned int lro_flush_cnt_unseq[HW_LRO_RING_NUM];
static unsigned int lro_flush_cnt_timestamp[HW_LRO_RING_NUM];
static unsigned int lro_flush_cnt_norule[HW_LRO_RING_NUM];
static const LRO_PROC_SET lro_proc_set[] = {
    [E_AGG_CNT] = _MDrv_NOE_LRO_PROC_Set_Agg_Cnt,
    [E_AGG_TIME] = _MDrv_NOE_LRO_PROC_Set_Agg_Time,
    [E_AGE_TIME] = _MDrv_NOE_LRO_PROC_Set_Age_Time,
    [E_BW_THRESHOLD] = _MDrv_NOE_LRO_PROC_Set_Threshold,
    [E_RING_CONTROL] = _MDrv_NOE_LRO_PROC_Enable_Ring_Ctrl
};

static struct proc_dir_entry *lro_proc_entry[E_LRO_PROC_MAX] = {NULL, NULL, NULL, NULL, NULL};

static const char lro_proc_name[E_LRO_PROC_MAX][16] = {
    NOE_PROC_LRO_RX_RING1,
    NOE_PROC_LRO_RX_RING2,
    NOE_PROC_LRO_RX_RING3,
    NOE_PROC_HW_LRO_STATS,
    NOE_PROC_HW_LRO_AUTO_TLB
};

static const struct file_operations lro_file_fops[E_LRO_PROC_MAX] = {
    [E_LRO_PROC_RING1] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_LRO_PROC_Open_Rx_Ring1,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release
    },
    [E_LRO_PROC_RING2] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_LRO_PROC_Open_Rx_Ring2,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release
    },
    [E_LRO_PROC_RING3] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_LRO_PROC_Open_Rx_Ring3,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release
    },
    [E_LRO_PROC_STATS] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_LRO_PROC_Open_Stats,
        .read = seq_read,
        .llseek = seq_lseek,
        .write = _MDrv_NOE_LRO_PROC_Write_Stats,
        .release = single_release
    },
    [E_LRO_PROC_ATTLB] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_LRO_PROC_Open_Auto_Tlb,
        .read = seq_read,
        .llseek = seq_lseek,
        .write = _MDrv_NOE_LRO_PROC_Write_Auto_Tlb,
        .release = single_release
    },
};


//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------

static int _MDrv_NOE_LRO_PROC_Set_Agg_Cnt(int par2)
{
    MHal_NOE_LRO_Control(E_NOE_LRO_CTRL_AGG_CNT, par2);
    return 0;
}

static int _MDrv_NOE_LRO_PROC_Set_Agg_Time(int par2)
{
    MHal_NOE_LRO_Control(E_NOE_LRO_CTRL_AGG_TIME, par2);
    return 0;
}

static int _MDrv_NOE_LRO_PROC_Set_Age_Time(int par2)
{
    MHal_NOE_LRO_Control(E_NOE_LRO_CTRL_AGE_TIME, par2);
    return 0;
}

static int _MDrv_NOE_LRO_PROC_Set_Threshold(int par2)
{
    MHal_NOE_LRO_Control(E_NOE_LRO_CTRL_BW_THRESHOLD, par2);
    return 0;
}

static int _MDrv_NOE_LRO_PROC_Enable_Ring_Ctrl(int par2)
{
    MHal_NOE_LRO_Control(E_NOE_LRO_CTRL_SWITCH, (!par2)?NOE_DISABLE:NOE_ENABLE);
    return 0;
}



static int _MDrv_NOE_LRO_PROC_Update_Len(unsigned int agg_size)
{
    int len_idx;

    if (agg_size > 65000)
        len_idx = 13;
    else if (agg_size > 60000)
        len_idx = 12;
    else if (agg_size > 55000)
        len_idx = 11;
    else if (agg_size > 50000)
        len_idx = 10;
    else if (agg_size > 45000)
        len_idx = 9;
    else if (agg_size > 40000)
        len_idx = 8;
    else if (agg_size > 35000)
        len_idx = 7;
    else if (agg_size > 30000)
        len_idx = 6;
    else if (agg_size > 25000)
        len_idx = 5;
    else if (agg_size > 20000)
        len_idx = 4;
    else if (agg_size > 15000)
        len_idx = 3;
    else if (agg_size > 10000)
        len_idx = 2;
    else if (agg_size > 5000)
        len_idx = 1;
    else
        len_idx = 0;

    return len_idx;
}

void MDrv_NOE_LRO_PROC_Update_Flush_Stats(unsigned int ring_num, struct PDMA_rxdesc *rx_ring)
{
    unsigned int flush_reason = rx_ring->rxd_info2.REV;

    if ((ring_num > 0) && (ring_num < 4)) {
        if ((flush_reason & 0x7) == HW_LRO_AGG_FLUSH)
            lro_flush_cnt_agg[ring_num - 1]++;
        else if ((flush_reason & 0x7) == HW_LRO_AGE_FLUSH)
            lro_flush_cnt_age[ring_num - 1]++;
        else if ((flush_reason & 0x7) == HW_LRO_NOT_IN_SEQ_FLUSH)
            lro_flush_cnt_unseq[ring_num - 1]++;
        else if ((flush_reason & 0x7) == HW_LRO_TIMESTAMP_FLUSH)
            lro_flush_cnt_timestamp[ring_num - 1]++;
        else if ((flush_reason & 0x7) == HW_LRO_NON_RULE_FLUSH)
            lro_flush_cnt_norule[ring_num - 1]++;
    }
}

void MDrv_NOE_LRO_PROC_Update_Stats(unsigned int ring_num, struct PDMA_rxdesc *rx_ring)
{
    unsigned int agg_cnt = rx_ring->rxd_info2.LRO_AGG_CNT;
    unsigned int agg_size = (rx_ring->rxd_info2.PLEN1 << 14) | rx_ring->rxd_info2.PLEN0;

    if ((ring_num > 0) && (ring_num < 4)) {
        lro_agg_size_cnt[ring_num - 1][_MDrv_NOE_LRO_PROC_Update_Len(agg_size)]++;
        lro_agg_num_cnt[ring_num - 1][agg_cnt]++;
        lro_flush_cnt_tot[ring_num - 1]++;
        lro_tot_agg_cnt[ring_num - 1] += agg_cnt;
    }
}

void _MDrv_NOE_LRO_PROC_Dump_Auto_Tlb(struct seq_file *seq, unsigned int index)
{
    struct lro_tbl info;
    struct PDMA_LRO_AUTO_TLB_INFO pdma_lro_auto_tlb;

    MHAL_NOE_LRO_Get_Table(index, &info);
    if (info.valid == FALSE) {
        return;
    }
    memcpy(&pdma_lro_auto_tlb, &(info.tlb_info), sizeof(struct PDMA_LRO_AUTO_TLB_INFO));

    if (index >= 4)
        seq_printf(seq, "\n===== TABLE Entry: %d (Act) =====\n", index);
    else
        seq_printf(seq, "\n===== TABLE Entry: %d (LRU) =====\n", index);
    if (pdma_lro_auto_tlb.auto_tlb_info8.IPV4) {
        seq_printf(seq, "SIP = 0x%x:0x%x:0x%x:0x%x (IPv4)\n",
               pdma_lro_auto_tlb.auto_tlb_info4.SIP3,
               pdma_lro_auto_tlb.auto_tlb_info3.SIP2,
               pdma_lro_auto_tlb.auto_tlb_info2.SIP1,
               pdma_lro_auto_tlb.auto_tlb_info1.SIP0);
    }
    else {
        seq_printf(seq, "SIP = 0x%x:0x%x:0x%x:0x%x (IPv6)\n",
               pdma_lro_auto_tlb.auto_tlb_info4.SIP3,
               pdma_lro_auto_tlb.auto_tlb_info3.SIP2,
               pdma_lro_auto_tlb.auto_tlb_info2.SIP1,
               pdma_lro_auto_tlb.auto_tlb_info1.SIP0);
    }
    seq_printf(seq, "DIP_ID = %d\n",
           pdma_lro_auto_tlb.auto_tlb_info8.DIP_ID);
    seq_printf(seq, "TCP SPORT = %d | TCP DPORT = %d\n",
           pdma_lro_auto_tlb.auto_tlb_info0.STP,
           pdma_lro_auto_tlb.auto_tlb_info0.DTP);
    seq_printf(seq, "VLAN_VID_VLD = %d\n",
           pdma_lro_auto_tlb.auto_tlb_info6.VLAN_VID_VLD);
    seq_printf(seq, "VLAN1 = %d | VLAN2 = %d | VLAN3 = %d | VLAN4 =%d\n",
           (pdma_lro_auto_tlb.auto_tlb_info5.VLAN_VID0 & 0xfff),
           ((pdma_lro_auto_tlb.auto_tlb_info5.VLAN_VID0 >> 12) & 0xfff),
           ((pdma_lro_auto_tlb.auto_tlb_info6.VLAN_VID1 << 8) |
           ((pdma_lro_auto_tlb.auto_tlb_info5.VLAN_VID0 >> 24)
             & 0xfff)),
           ((pdma_lro_auto_tlb.auto_tlb_info6.VLAN_VID1 >> 4) & 0xfff));
    seq_printf(seq, "TPUT = %d | FREQ = %d\n", pdma_lro_auto_tlb.auto_tlb_info7.DW_LEN, pdma_lro_auto_tlb.auto_tlb_info6.CNT);
    seq_printf(seq, "PRIORITY = %d\n", info.priority);

}

static ssize_t _MDrv_NOE_LRO_PROC_Write_Auto_Tlb(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    char buf[32];
    char *p_buf;
    int len = count;
    long x = 0, y = 0;
    char *p_token = NULL;
    char *p_delimiter = " \t";
    int ret;

    NOE_MSG_DUMP("[hw_lro_auto_tlb_write]write parameter len = %d\n\r", (int)len);
    if (len >= sizeof(buf)) {
        NOE_MSG_ERR("input handling fail!\n");
        len = sizeof(buf) - 1;
        return -1;
    }

    if (copy_from_user(buf, buffer, len))
        return -EFAULT;

    buf[len] = '\0';
    NOE_MSG_DUMP("[hw_lro_auto_tlb_write]write parameter data = %s\n\r", buf);

    p_buf = buf;
    p_token = strsep(&p_buf, p_delimiter);
    if (!p_token)
        x = 0;
    else
        ret = kstrtol(p_token, 10, &x);

    p_token = strsep(&p_buf, "\t\n ");
    if (p_token) {
        ret = kstrtol(p_token, 10, &y);
        NOE_MSG_DUMP("y = %ld\n\r", y);
    }

    if (lro_proc_set[x] &&
        (ARRAY_SIZE(lro_proc_set) > x)) {
        (*lro_proc_set[x]) (y);
    }

    return count;
}

static ssize_t _MDrv_NOE_LRO_PROC_Write_Stats(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    memset(lro_agg_num_cnt, 0, sizeof(lro_agg_num_cnt));
    memset(lro_agg_size_cnt, 0, sizeof(lro_agg_size_cnt));
    memset(lro_tot_agg_cnt, 0, sizeof(lro_tot_agg_cnt));
    memset(lro_flush_cnt_tot, 0, sizeof(lro_flush_cnt_tot));
    memset(lro_flush_cnt_agg, 0, sizeof(lro_flush_cnt_agg));
    memset(lro_flush_cnt_age, 0, sizeof(lro_flush_cnt_age));
    memset(lro_flush_cnt_unseq, 0, sizeof(lro_flush_cnt_unseq));
    memset(lro_flush_cnt_timestamp, 0, sizeof(lro_flush_cnt_timestamp));
    memset(lro_flush_cnt_norule, 0, sizeof(lro_flush_cnt_norule));
    NOE_MSG_DUMP("clear lro cnt table\n");
    return count;
}

static int _MDrv_NOE_LRO_PROC_Show_Auto_Tlb(struct seq_file *seq, void *offset)
{
    int i;
    struct lro_ctrl info;

    if (MDrv_NOE_LOG_Get_Level() >= E_MDRV_NOE_MSG_CTRL_DUMP) {
        seq_puts(seq, "Usage of /proc/" NOE_PROC_DIR  "/hw_lro_auto_tlb:\n");
        seq_puts(seq, "echo [function] [setting] > /proc/" NOE_PROC_DIR "/hw_lro_auto_tlb\n");
        seq_puts(seq, "Functions:\n");
        seq_puts(seq, "[0] = _MDrv_NOE_LRO_PROC_Set_Agg_Cnt\n");
        seq_puts(seq, "[1] = _MDrv_NOE_LRO_PROC_Set_Agg_Time\n");
        seq_puts(seq, "[2] = _MDrv_NOE_LRO_PROC_Set_Age_Time\n");
        seq_puts(seq, "[3] = _MDrv_NOE_LRO_PROC_Set_Threshold\n");
        seq_puts(seq, "[4] = _MDrv_NOE_LRO_PROC_Enable_Ring_Ctrl\n\n");
    }

    /* Read valid entries of the auto-learn table */
    seq_printf(seq, "HW LRO Auto-learn Table:\n");

    for (i = (MHAL_NOE_MAX_ENTRIES_IN_LRO_TABLE - 1); i >= 0; i--) {
            _MDrv_NOE_LRO_PROC_Dump_Auto_Tlb(seq, i);
    }

    /* Read the agg_time/age_time/agg_cnt of LRO rings */
    seq_puts(seq, "\nHW LRO Ring Settings\n");
    for (i = E_NOE_RING_NO1; i <= E_NOE_RING_NO3; i++) {
        MHAL_NOE_LRO_Get_Control(i, &info);
        seq_printf(seq, "Ring[%d]: MAX_AGG_CNT=%d, AGG_TIME=%d, AGE_TIME=%d, Threshold=%d\n", i, info.agg_cnt, info.agg_time, info.age_time, info.threshold);
    }

    seq_puts(seq, "\n");

    return 0;
}



static int _MDrv_NOE_LRO_PROC_Open_Auto_Tlb(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_LRO_PROC_Show_Auto_Tlb, NULL);
}


static int _MDrv_NOE_LRO_PROC_Show_Stats(struct seq_file *seq, void *offset)
{
    int i;
    struct END_DEVICE *ei_local = netdev_priv(lro_proc_dev);

    seq_puts(seq, "HW LRO statistic dump:\n");

    /* Agg number count */
    seq_puts(seq, "Cnt:   RING1 | RING2 | RING3 | Total\n");
    for (i = 0; i <= MAX_HW_LRO_AGGR; i++) {
        seq_printf(seq, " %d :      %d        %d        %d        %d\n",
               i, lro_agg_num_cnt[0][i],
               lro_agg_num_cnt[1][i], lro_agg_num_cnt[2][i],
               lro_agg_num_cnt[0][i] + lro_agg_num_cnt[1][i] +
               lro_agg_num_cnt[2][i]);
    }

    /* Total agg count */
    seq_puts(seq, "Total agg:   RING1 | RING2 | RING3 | Total\n");
    seq_printf(seq, "                %d      %d      %d      %d\n",
           lro_tot_agg_cnt[0], lro_tot_agg_cnt[1],
           lro_tot_agg_cnt[2],
           lro_tot_agg_cnt[0] + lro_tot_agg_cnt[1] +
           lro_tot_agg_cnt[2]);

    /* Total flush count */
    seq_puts(seq, "Total flush:   RING1 | RING2 | RING3 | Total\n");
    seq_printf(seq, "                %d      %d      %d      %d\n",
           lro_flush_cnt_tot[0], lro_flush_cnt_tot[1],
           lro_flush_cnt_tot[2],
           lro_flush_cnt_tot[0] + lro_flush_cnt_tot[1] +
           lro_flush_cnt_tot[2]);

    /* Avg agg count */
    seq_puts(seq, "Avg agg:   RING1 | RING2 | RING3 | Total\n");
    seq_printf(seq, "                %d      %d      %d      %d\n",
           (lro_flush_cnt_tot[0]) ? lro_tot_agg_cnt[0] / lro_flush_cnt_tot[0] : 0,
           (lro_flush_cnt_tot[1]) ? lro_tot_agg_cnt[1] / lro_flush_cnt_tot[1] : 0,
           (lro_flush_cnt_tot[2]) ? lro_tot_agg_cnt[2] / lro_flush_cnt_tot[2] : 0,
           (lro_flush_cnt_tot[0] + lro_flush_cnt_tot[1] + lro_flush_cnt_tot[2]) ? ((lro_tot_agg_cnt[0] + lro_tot_agg_cnt[1] + lro_tot_agg_cnt[2]) /
                        (lro_flush_cnt_tot[0] + lro_flush_cnt_tot[1] + lro_flush_cnt_tot[2])) : 0);

    /*  Statistics of aggregation size counts */
    seq_puts(seq, "HW LRO flush pkt len:\n");
    seq_puts(seq, " Length  | RING1  | RING2  | RING3  | Total\n");
    for (i = 0; i < 15; i++) {
        seq_printf(seq, "%d~%d: %d      %d      %d      %d\n", i * 5000,
               (i + 1) * 5000, lro_agg_size_cnt[0][i],
               lro_agg_size_cnt[1][i], lro_agg_size_cnt[2][i],
               lro_agg_size_cnt[0][i] +
               lro_agg_size_cnt[1][i] +
               lro_agg_size_cnt[2][i]);
    }

    /* CONFIG_NOE_HW_LRO_REASON_DBG */
    if (ei_local->features & FE_HW_LRO_DBG) {
        seq_puts(seq, "Flush reason:   RING1 | RING2 | RING3 | Total\n");
        seq_printf(seq, "AGG timeout:      %d      %d      %d      %d\n",
               lro_flush_cnt_agg[0], lro_flush_cnt_agg[1],
               lro_flush_cnt_agg[2],
               (lro_flush_cnt_agg[0] + lro_flush_cnt_agg[1] +
                lro_flush_cnt_agg[2])
            );
        seq_printf(seq, "AGE timeout:      %d      %d      %d      %d\n",
               lro_flush_cnt_age[0], lro_flush_cnt_age[1],
               lro_flush_cnt_age[2],
               (lro_flush_cnt_age[0] + lro_flush_cnt_age[1] +
                lro_flush_cnt_age[2])
            );
        seq_printf(seq, "Not in-sequence:  %d      %d      %d      %d\n",
               lro_flush_cnt_unseq[0], lro_flush_cnt_unseq[1],
               lro_flush_cnt_unseq[2],
               (lro_flush_cnt_unseq[0] + lro_flush_cnt_unseq[1] +
                lro_flush_cnt_unseq[2])
            );
        seq_printf(seq, "Timestamp:        %d      %d      %d      %d\n",
               lro_flush_cnt_timestamp[0],
               lro_flush_cnt_timestamp[1],
               lro_flush_cnt_timestamp[2],
               (lro_flush_cnt_timestamp[0] +
                lro_flush_cnt_timestamp[1] +
                lro_flush_cnt_timestamp[2])
            );
        seq_printf(seq, "No LRO rule:      %d      %d      %d      %d\n",
               lro_flush_cnt_norule[0],
               lro_flush_cnt_norule[1],
               lro_flush_cnt_norule[2],
               (lro_flush_cnt_norule[0] +
                lro_flush_cnt_norule[1] +
                lro_flush_cnt_norule[2])
            );
    }

    return 0;
}


static int _MDrv_NOE_LRO_PROC_Read_Ring(struct seq_file *seq, void *offset, struct PDMA_rxdesc *rx_ring_p)
{
    int i = 0;

    for (i = 0; i < NUM_LRO_RX_DESC; i++) {
        seq_printf(seq, "%4d: %08x %08x %08x %08x\n", i,
               *(int *)&rx_ring_p[i].rxd_info1,
               *(int *)&rx_ring_p[i].rxd_info2,
               *(int *)&rx_ring_p[i].rxd_info3,
               *(int *)&rx_ring_p[i].rxd_info4);
    }

    return 0;
}

static int _MDrv_NOE_LRO_PROC_Open_Stats(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_LRO_PROC_Show_Stats, NULL);
}

static int _MDrv_NOE_LRO_PROC_Show_Rx_Ring1(struct seq_file *seq, void *offset)
{
    struct END_DEVICE *ei_local = netdev_priv(lro_proc_dev);
    _MDrv_NOE_LRO_PROC_Read_Ring(seq, offset, ei_local->rx_ring[1]);
    return 0;
}

static int _MDrv_NOE_LRO_PROC_Show_Rx_Ring2(struct seq_file *seq, void *offset)
{
    struct END_DEVICE *ei_local = netdev_priv(lro_proc_dev);
    _MDrv_NOE_LRO_PROC_Read_Ring(seq, offset, ei_local->rx_ring[2]);
    return 0;
}

static int _MDrv_NOE_LRO_PROC_Show_Rx_Ring3(struct seq_file *seq, void *offset)
{
    struct END_DEVICE *ei_local = netdev_priv(lro_proc_dev);
    _MDrv_NOE_LRO_PROC_Read_Ring(seq, offset, ei_local->rx_ring[3]);
    return 0;
}


static int _MDrv_NOE_LRO_PROC_Open_Rx_Ring1(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_LRO_PROC_Show_Rx_Ring1, NULL);
}

static int _MDrv_NOE_LRO_PROC_Open_Rx_Ring2(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_LRO_PROC_Show_Rx_Ring2, NULL);
}

static int _MDrv_NOE_LRO_PROC_Open_Rx_Ring3(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_LRO_PROC_Show_Rx_Ring3, NULL);
}

int MDrv_NOE_LRO_PROC_Init(struct proc_dir_entry *proc_reg_dir, struct net_device *dev)
{
    unsigned char i = 0;
    lro_proc_dev = dev;
    for (i = 0; i < E_LRO_PROC_MAX; i++){
        lro_proc_entry[i] = proc_create(lro_proc_name[i], 0, proc_reg_dir, &lro_file_fops[i]);
        if (!lro_proc_entry[i])
            NOE_MSG_ERR("!! FAIL to create %s PROC !!\n", lro_proc_name[i]);
    }
    return 0;
}

void MDrv_NOE_LRO_PROC_Exit(struct proc_dir_entry *proc_reg_dir)
{
    unsigned char i = 0;
    for (i = 0; i < E_LRO_PROC_MAX; i++){
        if (lro_proc_entry[i])
            remove_proc_entry(lro_proc_name[i], proc_reg_dir);
        lro_proc_entry[i] = NULL;
    }
}


