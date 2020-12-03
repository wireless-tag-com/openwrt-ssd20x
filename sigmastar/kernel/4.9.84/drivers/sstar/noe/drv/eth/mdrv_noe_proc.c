/*
* mdrv_noe_proc.c- Sigmastar
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
/// @file   MDRV NOE PROC c
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

#include "mdrv_types.h"
#include "mhal_noe.h"
#include "mdrv_noe.h"
#include "mdrv_noe_proc.h"
#include "mdrv_noe_ethtool.h"
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define MAX_AGGR 64
#define MAX_DESC  8


//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
enum {
    E_NOE_PROC_RING0 = 0,
    E_NOE_PROC_RING1,
    E_NOE_PROC_RING2,
    E_NOE_PROC_RING3,
};
enum {
    E_NOE_PROC_TX = 1,
    E_NOE_PROC_RX = 2,
};

enum {
    E_NOE_PROC_GMAC1 = 0,
    E_NOE_PROC_GMAC2,
    E_NOE_PROC_SKB_FREE,
    E_NOE_PROC_TX_RING,
    E_NOE_PROC_RX_RING,
    E_NOE_PROC_NUM_OF_TXD,
    E_NOE_PROC_TSO_LEN,
    E_NOE_PROC_LRO_STATS,
    E_NOE_PROC_SNMP,
    E_NOE_PROC_ESW_CNT,
    E_NOE_PROC_ETH_CNT,
    E_NOE_PROC_SCHE,
    E_NOE_PROC_INT_DBG,
    E_NOE_PROC_SET_LAN_IP,
    E_NOE_PROC_PIN_MUX,
    E_NOE_PROC_LOG_CTRL,
    E_NOE_PROC_QSCH,
    E_NOE_PROC_MAX,
};

//--------------------------------------------------------------------------------------------------
//  Local Functions
//--------------------------------------------------------------------------------------------------
static ssize_t _MDrv_NOE_PROC_Write_Log_Ctrl(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static int _MDrv_NOE_PROC_Open_Log_Ctrl(struct inode *inode, struct file *file);

static ssize_t _MDrv_NOE_PROC_Write_Pin_Mux(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static int _MDrv_NOE_PROC_Open_Pin_Mux(struct inode *inode, struct file *file);
static int _MDrv_NOE_PROC_Open_Gmac(struct inode *inode, struct file *file);
static int _MDrv_NOE_PROC_Open_Gmac2(struct inode *inode, struct file *file);
static int _MDrv_NOE_PROC_Read_Regs(struct seq_file *seq, void *offset);
static void _MDrv_NOE_PROC_Dump_Reg(struct seq_file *s);
static ssize_t _MDrv_NOE_PROC_Set_Gmac2_Phyid(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static ssize_t _MDrv_NOE_PROC_Set_Gmac1_Phyid(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static int _MDrv_NOE_PROC_Open_Skb_Free(struct inode *inode, struct file *file);
static void *_MDrv_NOE_PROC_Start_Skb_Free(struct seq_file *seq, loff_t *pos);
static void *_MDrv_NOE_PROC_Get_Next_Skb_Free(struct seq_file *seq, void *offset, loff_t *pos);
static void _MDrv_NOE_PROC_Stop_Skb_Free(struct seq_file *seq, void *offset);
static int _MDrv_NOE_PROC_Show_Skb_Free(struct seq_file *seq, void *offset);
static int _MDrv_NOE_PROC_Open_Tx_Ring(struct inode *inode, struct file *file);
static int _MDrv_NOE_PROC_Open_Rx_Ring(struct inode *inode, struct file *file);
static int _MDrv_NOE_PROC_Open_Tso_Txd_Num(struct inode *inode, struct file *file);
static void *_MDrv_NOE_PROC_Start_Tso_Txd_Num(struct seq_file *seq, loff_t *pos);
static void *_MDrv_NOE_PROC_Get_Next_Tso_Txd_Num(struct seq_file *seq, void *offset, loff_t *pos);
static void _MDrv_NOE_PROC_Stop_Tso_Txd_Num(struct seq_file *seq, void *offset);
static int _MDrv_NOE_PROC_Show_Tso_Txd_Num(struct seq_file *seq, void *offset);
static int _MDrv_NOE_PROC_Open_Tso_Len(struct inode *inode, struct file *file);
static void *_MDrv_NOE_PROC_Get_Next_Tso_Len(struct seq_file *seq, void *offset, loff_t *pos);
static void _MDrv_NOE_PROC_Stop_Tso_Len(struct seq_file *seq, void *offset);
static int _MDrv_NOE_PROC_Show_Tso_Len(struct seq_file *seq, void *offset);
static void *_MDrv_NOE_PROC_Start_Tso_Len(struct seq_file *seq, loff_t *pos);
ssize_t _MDrv_NOE_PROC_Write_Lro_Stats(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static int _MDrv_NOE_PROC_Open_Lro_Stats(struct inode *inode, struct file *file);
int _MDrv_NOE_PROC_Show_Lro_Stats(struct seq_file *seq, void *offset);
ssize_t _MDrv_NOE_PROC_Write_Tso_Len(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static int _MDrv_NOE_PROC_Open_Snmp(struct inode *inode, struct file *file);
static int _MDrv_NOE_PROC_Show_Snmp(struct seq_file *seq, void *offset);
static int _MDrv_NOE_PROC_Open_Switch_Count(struct inode *inode, struct file *file);
int _MDrv_NOE_PROC_Show_Switch_Count(struct seq_file *seq, void *offset);
int _MDrv_NOE_PROC_Show_Eth_Count(struct seq_file *seq, void *offset);
static int _MDrv_NOE_PROC_Open_Eth_Count(struct inode *inode, struct file *file);
static int _MDrv_NOE_PROC_Open_Schedule(struct inode *inode, struct file *file);
static ssize_t _MDrv_NOE_PROC_Write_Schedule(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static int _MDrv_NOE_PROC_Read_Schedule(struct seq_file *seq, void *offset);
static int _MDrv_NOE_PROC_Read_Lan_Ip_Setting(struct seq_file *seq, void *offset);
static int _MDrv_NOE_PROC_Open_Lan_Ip_Setting(struct inode *inode, struct file *file);
static ssize_t _MDrv_NOE_PROC_Write_Lan_Ip_Setting(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static int _MDrv_NOE_PROC_Read_Intr_Dbg(struct seq_file *seq, void *offset);
static ssize_t _MDrv_NOE_PROC_Write_Intr_Dbg(struct file *file, const char __user *buffer, size_t count, loff_t *data);
static int _MDrv_NOE_PROC_Open_Intr_Dbg(struct inode *inode, struct file *file);
static ssize_t _MDrv_NOE_PROC_Write_Txd_Num(struct file *file, const char __user *buffer, size_t count, loff_t *data);
void _MDrv_NOE_PROC_Dump_Qdma_Cnt(void);
static int _MDrv_NOE_PROC_Get_Ring_Usage(int dma, int mode, int i);
static int _MDrv_NOE_PROC_Open_QoS(struct inode *inode, struct file *file);

//--------------------------------------------------------------------------------------------------
//  External Variable
//--------------------------------------------------------------------------------------------------

extern unsigned int M2Q_table[64];
extern struct QDMA_txdesc *free_head;
extern struct SFQ_table *sfq0;
extern struct SFQ_table *sfq1;
extern struct SFQ_table *sfq2;
extern struct SFQ_table *sfq3;
extern int init_schedule;
extern int working_schedule;

//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------
struct proc_dir_entry *proc_reg_dir = NULL;
static struct mdrv_noe_proc_intr mdrv_noe_proc_intr;
int tso_cnt[16];
int lro_stats_cnt[MAX_AGGR + 1];
int txd_cnt[MAX_SKB_FRAGS / 2 + 1];
int lro_stats_cnt[MAX_AGGR + 1];
int lro_flush_cnt[MAX_AGGR + 1];
int lro_len_cnt1[16];
/* int lro_len_cnt2[16]; */
int aggregated[MAX_DESC];
int lro_aggregated;
int lro_flushed;
int lro_nodesc;
int force_flush;
int tot_called1;
int tot_called2;
static struct net_device *_noe_proc_dev = NULL;
static struct proc_dir_entry *proc_entry[E_NOE_PROC_MAX] = {NULL};
static const struct seq_operations _MDrv_NOE_PROC_Set_Skb_free_Ops = {
    .start = _MDrv_NOE_PROC_Start_Skb_Free,
    .next = _MDrv_NOE_PROC_Get_Next_Skb_Free,
    .stop = _MDrv_NOE_PROC_Stop_Skb_Free,
    .show = _MDrv_NOE_PROC_Show_Skb_Free
};

static const struct seq_operations _MDrv_NOE_PROC_Set_Tso_Txd_Num_Ops = {
    .start = _MDrv_NOE_PROC_Start_Tso_Txd_Num,
    .next = _MDrv_NOE_PROC_Get_Next_Tso_Txd_Num,
    .stop = _MDrv_NOE_PROC_Stop_Tso_Txd_Num,
    .show = _MDrv_NOE_PROC_Show_Tso_Txd_Num
};

static const struct seq_operations _MDrv_NOE_PROC_Set_Tso_Len_Ops = {
    .start = _MDrv_NOE_PROC_Start_Tso_Len,
    .next = _MDrv_NOE_PROC_Get_Next_Tso_Len,
    .stop = _MDrv_NOE_PROC_Stop_Tso_Len,
    .show = _MDrv_NOE_PROC_Show_Tso_Len
};



static const char noe_proc_name[E_NOE_PROC_MAX][16] = {
    [E_NOE_PROC_GMAC1] = { NOE_PROC_GMAC },
    [E_NOE_PROC_GMAC2] = { NOE_PROC_GMAC2 },
    [E_NOE_PROC_SKB_FREE] = { NOE_PROC_SKBFREE },
    [E_NOE_PROC_TX_RING] = { NOE_PROC_TX_RING },
    [E_NOE_PROC_RX_RING] = { NOE_PROC_RX_RING },
    [E_NOE_PROC_NUM_OF_TXD] = { NOE_PROC_NUM_OF_TXD },
    [E_NOE_PROC_TSO_LEN] = { NOE_PROC_TSO_LEN },
    [E_NOE_PROC_LRO_STATS] = { NOE_PROC_LRO_STATS },
    [E_NOE_PROC_SNMP] = { NOE_PROC_SNMP },
    [E_NOE_PROC_ESW_CNT] = { NOE_PROC_ESW_CNT },
    [E_NOE_PROC_ETH_CNT] = { NOE_PROC_ETH_CNT },
    [E_NOE_PROC_SCHE] = { NOE_PROC_SCHE },
    [E_NOE_PROC_INT_DBG] = { NOE_PROC_INT_DBG },
    [E_NOE_PROC_SET_LAN_IP] = { NOE_PROC_SET_LAN_IP },
    [E_NOE_PROC_PIN_MUX] = { NOE_PROC_PIN_MUX },
    [E_NOE_PROC_LOG_CTRL] = { NOE_PROC_LOG_CTRL },
    [E_NOE_PROC_QSCH] = { NOE_PROC_QSCH },
};

static const struct file_operations noe_proc_fops[E_NOE_PROC_MAX] = {
    [E_NOE_PROC_GMAC1] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Gmac,
        .read = seq_read,
        .llseek = seq_lseek,
        .write = _MDrv_NOE_PROC_Set_Gmac1_Phyid,
        .release = single_release
    },
    [E_NOE_PROC_GMAC2] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Gmac2,
        .read = seq_read,
        .llseek = seq_lseek,
        .write = _MDrv_NOE_PROC_Set_Gmac2_Phyid
    },
    [E_NOE_PROC_SKB_FREE] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Skb_Free,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = seq_release
    },
    [E_NOE_PROC_TX_RING] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Tx_Ring,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release
    },
    [E_NOE_PROC_RX_RING] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Rx_Ring,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release
    },
    [E_NOE_PROC_NUM_OF_TXD] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Tso_Txd_Num,
        .read = seq_read,
        .llseek = seq_lseek,
        .write = _MDrv_NOE_PROC_Write_Txd_Num,
        .release = seq_release
    },
    [E_NOE_PROC_TSO_LEN] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Tso_Len,
        .read = seq_read,
        .llseek = seq_lseek,
        .write = _MDrv_NOE_PROC_Write_Tso_Len,
        .release = seq_release
    },
    [E_NOE_PROC_LRO_STATS] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Lro_Stats,
        .read = seq_read,
        .llseek = seq_lseek,
        .write = _MDrv_NOE_PROC_Write_Lro_Stats,
        .release = single_release
    },
    [E_NOE_PROC_SNMP] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Snmp,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release
    },
    [E_NOE_PROC_ESW_CNT] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Switch_Count,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release
    },
    [E_NOE_PROC_ETH_CNT] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Eth_Count,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release
    },
    [E_NOE_PROC_SCHE] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Schedule,
        .read = seq_read,
        .write = _MDrv_NOE_PROC_Write_Schedule,
        .llseek = seq_lseek,
        .release = single_release
    },
    [E_NOE_PROC_INT_DBG] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Intr_Dbg,
        .read = seq_read,
        .write = _MDrv_NOE_PROC_Write_Intr_Dbg
    },
    [E_NOE_PROC_SET_LAN_IP] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Lan_Ip_Setting,
        .read = seq_read,
        .write = _MDrv_NOE_PROC_Write_Lan_Ip_Setting
    },
    [E_NOE_PROC_PIN_MUX] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Pin_Mux,
        .read = seq_read,
        .write = _MDrv_NOE_PROC_Write_Pin_Mux,
    },
    [E_NOE_PROC_LOG_CTRL] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_Log_Ctrl,
        .read = seq_read,
        .write = _MDrv_NOE_PROC_Write_Log_Ctrl,

    },
    [E_NOE_PROC_QSCH] = {
        .owner = THIS_MODULE,
        .open = _MDrv_NOE_PROC_Open_QoS,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release
    },
};

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------

static int _MDrv_NOE_PROC_Get_Ring_Usage(int dma, int mode, int ring_no)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);
    unsigned long tx_ctx_idx, tx_dtx_idx, tx_usage;
    unsigned long rx_calc_idx, rx_drx_idx, rx_usage;
    struct PDMA_rxdesc *rxring;
    struct PDMA_txdesc *txring;
    struct noe_dma_info ring_info;

    if (mode == E_NOE_PROC_RX) {
        /* cpu point to the next descriptor of rx dma ring */
        MHal_NOE_LRO_Get_Ring_Info(dma, E_NOE_DIR_RX, E_NOE_RING_NO0, &ring_info);
        rx_calc_idx = ring_info.ring_st.cpu_idx;
        rx_drx_idx = ring_info.ring_st.dma_idx;
        rxring = ei_local->rx_ring[0];

        rx_usage = (rx_drx_idx - rx_calc_idx - 1 + NUM_RX_DESC) % NUM_RX_DESC;
        if (rx_calc_idx == rx_drx_idx) {
            if (rxring[rx_drx_idx].rxd_info2.DDONE_bit == 1)
                tx_usage = NUM_RX_DESC;
            else
                tx_usage = 0;
        }
        return rx_usage;
    }

    switch (ring_no) {
        case E_NOE_PROC_RING0:
            MHal_NOE_LRO_Get_Ring_Info(dma, E_NOE_DIR_TX, E_NOE_RING_NO0, &ring_info);
            tx_ctx_idx = ring_info.ring_st.cpu_idx;
            tx_dtx_idx = ring_info.ring_st.dma_idx;
            txring = ei_local->tx_ring0;
            break;
        default:
            NOE_MSG_ERR("get_tx_idx failed %d %d\n", mode, ring_no);
            return 0;
    };

    tx_usage = (tx_ctx_idx - tx_dtx_idx + NUM_TX_DESC) % NUM_TX_DESC;
    if (tx_ctx_idx == tx_dtx_idx) {
        if (txring[tx_ctx_idx].txd_info2.DDONE_bit == 1)
            tx_usage = 0;
        else
            tx_usage = NUM_TX_DESC;
    }
    return tx_usage;
}

static ssize_t _MDrv_NOE_PROC_Write_Txd_Num(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_TSO) {
        memset(txd_cnt, 0, sizeof(txd_cnt));
        NOE_MSG_DUMP("clear txd cnt table\n");
        return count;
    } else {
        return 0;
    }
}

static int _MDrv_NOE_PROC_Read_Lan_Ip_Setting(struct seq_file *seq, void *v)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    seq_printf(seq, "ei_local->lan_ip4_addr = %s\n", ei_local->lan_ip4_addr);

    return 0;
}

static int _MDrv_NOE_PROC_Open_Lan_Ip_Setting(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Read_Lan_Ip_Setting, NULL);
}

static ssize_t _MDrv_NOE_PROC_Write_Lan_Ip_Setting(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);
    unsigned int lan_ip;

    if (count > IP4_ADDR_LEN)
        return -EFAULT;

    memset(&ei_local->lan_ip4_addr[0], 0, IP4_ADDR_LEN);
    if (copy_from_user(ei_local->lan_ip4_addr, buffer, count))
        return -EFAULT;

    NOE_MSG_DUMP("LAN IP = %s\n", ei_local->lan_ip4_addr);

    if (ei_local->features & FE_SW_LRO) {
        MDrv_NOE_Set_Lro_Ip(ei_local->lan_ip4_addr);
    }

    if (ei_local->features & FE_HW_LRO) {
        MDrv_NOE_UTIL_Str_To_Ip(&lan_ip, ei_local->lan_ip4_addr);
        MHal_NOE_LRO_Set_Ip(lan_ip);
    }

    return count;
}


static int _MDrv_NOE_PROC_Read_Intr_Dbg(struct seq_file *seq, void *v)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_NOE_INT_DBG) {
        seq_puts(seq, "Raether Interrupt Statistics\n");
        seq_printf(seq, "RX_COHERENT = %d\n", mdrv_noe_proc_intr.RX_COHERENT_CNT);
        seq_printf(seq, "RX_DLY_INT = %d\n", mdrv_noe_proc_intr.RX_DLY_INT_CNT);
        seq_printf(seq, "TX_COHERENT = %d\n", mdrv_noe_proc_intr.TX_COHERENT_CNT);
        seq_printf(seq, "TX_DLY_INT = %d\n", mdrv_noe_proc_intr.TX_DLY_INT_CNT);
        seq_printf(seq, "RING3_RX_DLY_INT = %d\n", mdrv_noe_proc_intr.RING3_RX_DLY_INT_CNT);
        seq_printf(seq, "RING2_RX_DLY_INT = %d\n", mdrv_noe_proc_intr.RING2_RX_DLY_INT_CNT);
        seq_printf(seq, "RING1_RX_DLY_INT = %d\n", mdrv_noe_proc_intr.RING1_RX_DLY_INT_CNT);
        seq_printf(seq, "RXD_ERROR = %d\n", mdrv_noe_proc_intr.RXD_ERROR_CNT);
        seq_printf(seq, "ALT_RPLC_INT3 = %d\n", mdrv_noe_proc_intr.ALT_RPLC_INT3_CNT);
        seq_printf(seq, "ALT_RPLC_INT2 = %d\n", mdrv_noe_proc_intr.ALT_RPLC_INT2_CNT);
        seq_printf(seq, "ALT_RPLC_INT1 = %d\n", mdrv_noe_proc_intr.ALT_RPLC_INT1_CNT);
        seq_printf(seq, "RX_DONE_INT3 = %d\n", mdrv_noe_proc_intr.RX_DONE_INT3_CNT);
        seq_printf(seq, "RX_DONE_INT2 = %d\n", mdrv_noe_proc_intr.RX_DONE_INT2_CNT);
        seq_printf(seq, "RX_DONE_INT1 = %d\n", mdrv_noe_proc_intr.RX_DONE_INT1_CNT);
        seq_printf(seq, "RX_DONE_INT0 = %d\n", mdrv_noe_proc_intr.RX_DONE_INT0_CNT);
        seq_printf(seq, "TX_DONE_INT3 = %d\n", mdrv_noe_proc_intr.TX_DONE_INT3_CNT);
        seq_printf(seq, "TX_DONE_INT2 = %d\n", mdrv_noe_proc_intr.TX_DONE_INT2_CNT);
        seq_printf(seq, "TX_DONE_INT1 = %d\n", mdrv_noe_proc_intr.TX_DONE_INT1_CNT);
        seq_printf(seq, "TX_DONE_INT0 = %d\n", mdrv_noe_proc_intr.TX_DONE_INT0_CNT);

        memset(&mdrv_noe_proc_intr, 0, sizeof(mdrv_noe_proc_intr));
    }
    return 0;
}

static int _MDrv_NOE_PROC_Open_Intr_Dbg(struct inode *inode, struct file *file)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_NOE_INT_DBG) {
        /* memset(&mdrv_noe_proc_intr, 0, sizeof(mdrv_noe_proc_intr)); */
        return single_open(file, _MDrv_NOE_PROC_Read_Intr_Dbg, NULL);
    } else {
        return 0;
    }
}

static ssize_t _MDrv_NOE_PROC_Write_Intr_Dbg(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    return 0;
}



static int _MDrv_NOE_PROC_Read_Schedule(struct seq_file *seq, void *v)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & TASKLET_WORKQUEUE_SW) {
        if (init_schedule == 1)
            seq_printf(seq,
                   "Initialize Raeth with workqueque<%d>\n",
                   init_schedule);
        else
            seq_printf(seq,
                   "Initialize Raeth with tasklet<%d>\n",
                   init_schedule);
        if (working_schedule == 1)
            seq_printf(seq,
                   "Raeth is running at workqueque<%d>\n",
                   working_schedule);
        else
            seq_printf(seq,
                   "Raeth is running at tasklet<%d>\n",
                   working_schedule);
    }

    return 0;
}

static ssize_t _MDrv_NOE_PROC_Write_Schedule(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & TASKLET_WORKQUEUE_SW) {
        char buf[2];
        int old;

        if (copy_from_user(buf, buffer, count))
            return -EFAULT;
        old = init_schedule;
        init_schedule = kstrtol(buf, 10, NULL);
        NOE_MSG_DUMP("ChangeRaethInitScheduleFrom <%d> to <%d>\n", old, init_schedule);
        NOE_MSG_DUMP("Not running schedule at present !\n");

        return count;
    } else {
        return 0;
    }
}

static int _MDrv_NOE_PROC_Open_Schedule(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Read_Schedule, NULL);
}



int _MDrv_NOE_PROC_Show_Eth_Count(struct seq_file *seq, void *v)
{
    u8 i;
    struct noe_pse_info pse;
    struct noe_qdma_cnt info;
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);
    MHal_NOE_Get_Pse_Info(&pse);
    seq_printf(seq, "       <<PSE DROP CNT>>\n");
    seq_printf(seq, "| FQ_PCNT_MIN : %010u |\n", pse.min_free_cnt);
    seq_printf(seq, "| FQ_PCNT     : %010u |\n", pse.free_cnt);
    seq_printf(seq, "| FE_DROP_FQ  : %010u |\n", pse.fq_drop_cnt);
    seq_printf(seq, "| FE_DROP_FC  : %010u |\n", pse.fc_drop_cnt);
    seq_printf(seq, "| FE_DROP_PPE : %010u |\n", pse.ppe_drop_cnt);
    seq_printf(seq, "\n       <<QDMA PKT/DROP CNT>>\n");

    if (ei_local->features & FE_QDMA) {
        for (i = 0; i < NUM_PQ; i++) {
            info.pq_no = i;
            MHal_NOE_Get_Qdma_Info(E_NOE_QDMA_INFO_CNT, &info);
            seq_printf(seq, "QDMA Q%02d PKT CNT: %010u, DROP CNT: %010u\n", i, info.pkt_cnt , info.drop_cnt);
        }
    }
    seq_puts(seq, "\n");

    return 0;
}

static int _MDrv_NOE_PROC_Open_Eth_Count(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Show_Eth_Count, NULL);
}

int _MDrv_NOE_PROC_Show_Switch_Count(struct seq_file *seq, void *v)
{
    int i =0;
    struct noe_mac_info info;
    for (i = 0; i < E_NOE_GE_MAC_MAX; i++)
    {
        memset(&info, 0, sizeof(struct noe_mac_info));
        MHal_NOE_Get_Mac_Info(i, &info);
        seq_printf(seq, " GDMA%d:                                \n", i+1);
        seq_printf(seq, "      RX_GBCNT  : %010u (Rx Good Bytes)    \n", info.rx.good_cnt);
        seq_printf(seq, "      RX_GPCNT  : %010u (Rx Good Pkts) \n", info.rx.good_pkt );
        seq_printf(seq, "      RX_OERCNT : %010u (overflow error)   \n", info.rx.overflow_err);
        seq_printf(seq, "      RX_FERCNT : %010u (FCS error)    \n", info.rx.fcs_err);
        seq_printf(seq, "      RX_SERCNT : %010u (too short)    \n", info.rx.ser_cnt);
        seq_printf(seq, "      RX_LERCNT : %010u (too long) \n", info.rx.ler_pkt);
        seq_printf(seq, "      RX_CERCNT : %010u (checksum error)   \n", info.rx.chk_err);
        seq_printf(seq, "      RX_FCCNT  : %010u (flow control) \n", info.rx.flow_ctrl);
        seq_printf(seq, "      TX_SKIPCNT: %010u (skip count)   \n", info.tx.skip_cnt);
        seq_printf(seq, "      TX_COLCNT : %010u (collision count)  \n", info.tx.collision_cnt);
        seq_printf(seq, "      TX_GBCNT  : %010u (Tx Good Bytes)    \n", info.tx.good_cnt);
        seq_printf(seq, "      TX_GPCNT  : %010u (Tx Good Pkts) \n", info.tx.good_pkt);
        seq_printf(seq, "      ST_MCTRL  : %010x (Control)  \n", info.stat.control);
        seq_printf(seq, "      ST_STATUS : %010x (Status)   \n", info.stat.status);

    }


    seq_puts(seq, "\n");

    return 0;
}

static int _MDrv_NOE_PROC_Open_Switch_Count(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Show_Switch_Count, NULL);
}


static int _MDrv_NOE_PROC_Show_Snmp(struct seq_file *seq, void *v)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & USER_SNMPD) {

    }

    return 0;
}

static int _MDrv_NOE_PROC_Open_Snmp(struct inode *inode, struct file *file)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & USER_SNMPD)
        return single_open(file, _MDrv_NOE_PROC_Show_Snmp, NULL);
    else
        return 0;
}




ssize_t _MDrv_NOE_PROC_Write_Lro_Stats(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_SW_LRO) {
        memset(lro_stats_cnt, 0, sizeof(lro_stats_cnt));
        memset(lro_flush_cnt, 0, sizeof(lro_flush_cnt));
        memset(lro_len_cnt1, 0, sizeof(lro_len_cnt1));
        /* memset(lro_len_cnt2, 0, sizeof(lro_len_cnt2)); */
        memset(aggregated, 0, sizeof(aggregated));
        lro_aggregated = 0;
        lro_flushed = 0;
        lro_nodesc = 0;
        force_flush = 0;
        tot_called1 = 0;
        tot_called2 = 0;
        NOE_MSG_DBG("clear lro  cnt table\n");

        return count;
    } else {
        return 0;
    }
}

int _MDrv_NOE_PROC_Show_Lro_Stats(struct seq_file *seq, void *v)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_SW_LRO) {
        int i;
        int tot_cnt = 0;
        int tot_aggr = 0;
        int ave_aggr = 0;

        seq_puts(seq, "LRO statistic dump:\n");
        seq_puts(seq, "Cnt:   Kernel | Driver\n");
        for (i = 0; i <= MAX_AGGR; i++) {
            tot_cnt = tot_cnt + lro_stats_cnt[i] + lro_flush_cnt[i];
            seq_printf(seq, " %d :      %d        %d\n", i,
                   lro_stats_cnt[i], lro_flush_cnt[i]);
            tot_aggr =
                tot_aggr + i * (lro_stats_cnt[i] +
                        lro_flush_cnt[i]);
        }
        ave_aggr = lro_aggregated / lro_flushed;
        seq_printf(seq, "Total aggregated pkt: %d\n", lro_aggregated);
        seq_printf(seq, "Flushed pkt: %d  %d\n", lro_flushed,
               force_flush);
        seq_printf(seq, "Average flush cnt:  %d\n", ave_aggr);
        seq_printf(seq, "No descriptor pkt: %d\n\n\n", lro_nodesc);

        seq_puts(seq, "Driver flush pkt len:\n");
        seq_puts(seq, " Length  | Count\n");
        for (i = 0; i < 15; i++) {
            seq_printf(seq, "%d~%d: %d\n", i * 5000,
                   (i + 1) * 5000, lro_len_cnt1[i]);
        }
        seq_printf(seq, "Kernel flush: %d;  Driver flush: %d\n",
               tot_called2, tot_called1);
        return 0;
    } else {
        return 0;
    }
}

static int _MDrv_NOE_PROC_Open_Lro_Stats(struct inode *inode, struct file *file)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_SW_LRO)
        return single_open(file, _MDrv_NOE_PROC_Show_Lro_Stats, NULL);
    else
        return 0;
}




static void *_MDrv_NOE_PROC_Start_Tso_Len(struct seq_file *seq, loff_t *pos)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_TSO) {
        seq_puts(seq, " Length  | Count\n");
        if (*pos < 15)
            return pos;
    }
    return NULL;
}

static void *_MDrv_NOE_PROC_Get_Next_Tso_Len(struct seq_file *seq, void *v, loff_t *pos)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_TSO) {
        (*pos)++;
        if (*pos >= 15)
            return NULL;
        return pos;
    } else {
        return NULL;
    }
}

static void _MDrv_NOE_PROC_Stop_Tso_Len(struct seq_file *seq, void *v)
{
    /* Nothing to do */
}

static int _MDrv_NOE_PROC_Show_Tso_Len(struct seq_file *seq, void *v)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_TSO) {
        int i = *(loff_t *)v;
        seq_printf(seq, "%8d ~ %8d: %8d\n", i * 5000, (i + 1) * 5000, tso_cnt[i]);
    }
    return 0;
}


static int _MDrv_NOE_PROC_Open_Tso_Len(struct inode *inode, struct file *file)
{
    return seq_open(file, &_MDrv_NOE_PROC_Set_Tso_Len_Ops);
}

ssize_t _MDrv_NOE_PROC_Write_Tso_Len(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_TSO) {
        memset(tso_cnt, 0, sizeof(tso_cnt));
        NOE_MSG_DBG("clear tso cnt table\n");
        return count;
    }
    else {
        return 0;
    }
}


static void *_MDrv_NOE_PROC_Start_Tso_Txd_Num(struct seq_file *seq, loff_t *pos)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_TSO) {
        seq_puts(seq, "TXD | Count\n");
        if (*pos < (MAX_SKB_FRAGS / 2 + 1))
            return pos;
    }
    return NULL;
}

static void *_MDrv_NOE_PROC_Get_Next_Tso_Txd_Num(struct seq_file *seq, void *v, loff_t *pos)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_TSO) {
        (*pos)++;
        if (*pos >= (MAX_SKB_FRAGS / 2 + 1))
            return NULL;
        return pos;
    } else {
        return NULL;
    }
}

static void _MDrv_NOE_PROC_Stop_Tso_Txd_Num(struct seq_file *seq, void *v)
{
    /* Nothing to do */
}

static int _MDrv_NOE_PROC_Show_Tso_Txd_Num(struct seq_file *seq, void *v)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_TSO) {
        int i = *(loff_t *)v;

        seq_printf(seq, "%8d: %8d\n", i, txd_cnt[i]);
    }
    return 0;
}


static int _MDrv_NOE_PROC_Open_Tso_Txd_Num(struct inode *inode, struct file *file)
{
    return seq_open(file, &_MDrv_NOE_PROC_Set_Tso_Txd_Num_Ops);
}

int _MDrv_NOE_PROC_Read_Rx_Ring(struct seq_file *seq, void *v)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);
    int i = 0, j = 0;

    seq_printf(seq, "        0x%08x , 0x%p\n", (unsigned int)ei_local->phy_rx_ring[0], (void *)ei_local->rx_ring[0]);

    for (i = 0; i < NUM_RX_DESC; i++) {
        seq_printf(seq, "%8d: %08x %08x %08x %08x\n", i,
              *(int *)&ei_local->rx_ring[j][i].rxd_info1,
              *(int *)&ei_local->rx_ring[j][i].rxd_info2,
              *(int *)&ei_local->rx_ring[j][i].rxd_info3,
              *(int *)&ei_local->rx_ring[j][i].rxd_info4);
    }
    return 0;
}

static int _MDrv_NOE_PROC_Open_Rx_Ring(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Read_Rx_Ring, NULL);
}




int _MDrv_NOE_PROC_Show_Tx_Ring(struct seq_file *seq, void *v)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);
    struct PDMA_txdesc *tx_ring;
    int i = 0;


    if (ei_local->features & FE_QDMA_TX) {
        seq_printf(seq, "%8p,%8p \n", ei_local, ei_local->txd_pool);
        seq_printf(seq, "cpu:%4d/rls:%4d \n",ei_local->tx_cpu_idx, ei_local->rls_cpu_idx);
        for (i = 0; i < TOTAL_TXQ_NUM; i++) {
            seq_printf(seq, "%4d: %4d/%4d(%4d)/%4d(%4d)/%d\n",
                    i, atomic_read(&ei_local->free_txd_num[i]),
                    ei_local->free_txd_head[i], ei_local->txd_pool_info[ei_local->free_txd_head[i]],
                    ei_local->free_txd_tail[i], ei_local->txd_pool_info[ei_local->free_txd_tail[i]],
                    ei_local->stats.min_free_txd[i]);
        }

        for (i = 0; i < (GMAC1_TXQ_TXD_NUM + GMAC2_TXQ_TXD_NUM); i++) {
            seq_printf(seq, "%4d:[%08x] %08x %08x %08x %08x \n",
                    i,
                    (ei_local->phy_txd_pool + i * QTXD_LEN),
                    *(int *)&ei_local->txd_pool[i].txd_info1,
                    *(int *)&ei_local->txd_pool[i].txd_info2,
                    *(int *)&ei_local->txd_pool[i].txd_info3,
                    *(int *)&ei_local->txd_pool[i].txd_info4);
        }
    }
    else {
        for (i = 0; i < NUM_TX_DESC; i++) {
            tx_ring = &ei_local->tx_ring0[i];
            seq_printf(seq, "%4d: %08x %08x %08x %08x\n", i,
                   *(int *)&tx_ring->txd_info1,
                   *(int *)&tx_ring->txd_info2,
                   *(int *)&tx_ring->txd_info3,
                   *(int *)&tx_ring->txd_info4);
        }


    }

    return 0;
}

static int _MDrv_NOE_PROC_Open_Tx_Ring(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Show_Tx_Ring, NULL);
}


static ssize_t _MDrv_NOE_PROC_Set_Gmac1_Phyid(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    int val = 0;
    char if_name[64];
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->netdev == NULL)
        return 0;
    strcpy(if_name, ei_local->netdev->name);

    if (ei_local->features & (FE_ETHTOOL)) {
        char buf[32];
        struct net_device *cur_dev_p;
        struct END_DEVICE *ei_local;
        unsigned int phy_id;

        if (count > 32)
            count = 32;
        memset(buf, 0, 32);
        if (copy_from_user(buf, buffer, count))
            return -EFAULT;

        /* determine interface name */
        if (isalpha(buf[0])) {
            val = sscanf(buf, "%4s %1d", if_name, &phy_id);
            if (val == -1)
                return -EFAULT;
        }
        else {
            phy_id = kstrtol(buf, 10, NULL);
        }
        cur_dev_p = dev_get_by_name(&init_net, if_name);

        if (!cur_dev_p)
            return -EFAULT;

        ei_local = netdev_priv(cur_dev_p);

        ei_local->mii_info.phy_id = (unsigned char)phy_id;
        return count;
    } else {
        return 0;
    }
}

static ssize_t _MDrv_NOE_PROC_Set_Gmac2_Phyid(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    int val = 0;
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & (FE_ETHTOOL | FE_GE2_SUPPORT)) {
        char buf[32];
        struct net_device *cur_dev_p;
        struct PSEUDO_ADAPTER *p_pseudo_ad;
        char if_name[64];
        unsigned int phy_id;

        if (count > 32)
            count = 32;
        memset(buf, 0, 32);
        if (copy_from_user(buf, buffer, count))
            return -EFAULT;
        /* determine interface name */
        strcpy(if_name, ei_local->pseudo_dev->name);
        if (isalpha(buf[0])) {
            val = sscanf(buf, "%4s %1d", if_name, &phy_id);
            if (val == -1)
                return -EFAULT;
        } else {
            phy_id = kstrtol(buf, 10, NULL);
        }
        cur_dev_p = dev_get_by_name(&init_net, if_name);

        if (!cur_dev_p)
            return -EFAULT;
        p_pseudo_ad = netdev_priv(cur_dev_p);
        p_pseudo_ad->mii_info.phy_id = (unsigned char)(phy_id & 0xFF);
        return count;
    } else {
        return 0;
    }
}

static void *_MDrv_NOE_PROC_Start_Skb_Free(struct seq_file *seq, loff_t *pos)
{
    if (*pos < NUM_TX_DESC)
        return pos;
    return NULL;
}

static void *_MDrv_NOE_PROC_Get_Next_Skb_Free(struct seq_file *seq, void *v, loff_t *pos)
{
    (*pos)++;
    if (*pos >= NUM_TX_DESC)
        return NULL;
    return pos;
}

static void _MDrv_NOE_PROC_Stop_Skb_Free(struct seq_file *seq, void *offset)
{
    /* Nothing to do */
}

static int _MDrv_NOE_PROC_Show_Skb_Free(struct seq_file *seq, void *offset)
{
    int i = *(loff_t *)offset;
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    seq_printf(seq, "%8d: %08x\n", i, *(int *)&ei_local->skb_free[i]);

    return 0;
}



static int _MDrv_NOE_PROC_Open_Skb_Free(struct inode *inode, struct file *file)
{
    return seq_open(file, &_MDrv_NOE_PROC_Set_Skb_free_Ops);
}

static void _MDrv_NOE_PROC_Dump_Reg(struct seq_file *s)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);
    struct noe_dma_info tx_ring[E_NOE_RING_MAX];
    struct noe_dma_info rx_ring;
    struct noe_intr_info intr;
    struct noe_pdma_dbg dbg_info;
    int rx_usage;
    int tx_usage = 0;
    int i;

    seq_printf(s, "FEATURES         : 0x%08x\n", ei_local->features);
    seq_printf(s, "ARCH             : 0x%08x\n", ei_local->architecture);
    MHal_NOE_Get_Intr_Info(&intr);
    rx_usage = _MDrv_NOE_PROC_Get_Ring_Usage(E_NOE_DMA_PACKET, E_NOE_PROC_RX, E_NOE_PROC_RING0);

    if (!(ei_local->features & FE_QDMA)) {
        tx_usage = _MDrv_NOE_PROC_Get_Ring_Usage(E_NOE_DMA_PACKET, E_NOE_PROC_TX, E_NOE_PROC_RING0);
        for (i = 0; i < E_NOE_RING_MAX; i++) {
            MHal_NOE_LRO_Get_Ring_Info(E_NOE_DMA_PACKET, E_NOE_DIR_TX, E_NOE_RING_NO0, &tx_ring[i]);
        }
    }
    else {
        MHal_NOE_LRO_Get_Ring_Info(E_NOE_DMA_QUEUE, E_NOE_DIR_TX, E_NOE_RING_NO0, &tx_ring[0]);
    }

    seq_printf(s, "P_INT_ENABLE     : 0x%08x\n", intr.fe_intr_enable);
    seq_printf(s, "P_INT_MASK       : 0x%08x\n", intr.fe_intr_mask);
    seq_printf(s, "P_INT_STATUS     : 0x%08x\n", intr.fe_intr_status);
    seq_printf(s, "Q_INT_ENABLE     : 0x%08x\n", intr.qfe_intr_enable);
    seq_printf(s, "Q_INT_MASK       : 0x%08x\n", intr.qfe_intr_mask);
    seq_printf(s, "Q_INT_STATUS     : 0x%08x\n", intr.qfe_intr_status);

    if (!(ei_local->features & FE_QDMA))
        seq_printf(s, "TxRing PktCnt  : %d/%d\n", tx_usage, NUM_TX_DESC);

    seq_printf(s, "RxRing PktCnt: %d/%d\n\n", rx_usage, NUM_RX_DESC);
    seq_printf(s, "DLY_INT_CFG     : 0x%08x\n\n", intr.delay_intr_cfg);

    if (!(ei_local->features & FE_QDMA)) {
        for (i = 0; i < 4; i++) {
            seq_printf(s, "TX_BASE_PTR%02d   : 0x%08x\n", i, (unsigned int)tx_ring[i].ring_st.base_adr);
            seq_printf(s, "TX_MAX_CNT%02d    : 0x%08x\n", i, tx_ring[i].ring_st.max_cnt);
            seq_printf(s, "TX_CTX_IDX%02d   : 0x%08x\n", i, tx_ring[i].ring_st.cpu_idx);
            seq_printf(s, "TX_DTX_IDX%02d   : 0x%08x\n\n", i, tx_ring[i].ring_st.dma_idx);
        }
    }
    else {
        i = 0;
        seq_printf(s, "CTX_PTR%02d       : 0x%08x\n", i, (unsigned int)tx_ring[i].adr_st.ctx_adr);
        seq_printf(s, "DTX_PTR%02d       : 0x%08x\n", i, (unsigned int)tx_ring[i].adr_st.dtx_adr);
        seq_printf(s, "CRX_PTR%02d       : 0x%08x\n", i, (unsigned int)tx_ring[i].adr_st.crx_adr);
        seq_printf(s, "DRX_PTR%02d       : 0x%08x\n\n", i, (unsigned int)tx_ring[i].adr_st.drx_adr);
    }

    for (i = 0; i < 4; i++) {
        if ((i >= 1) && (!(ei_local->features & FE_HW_LRO))) {
            break;
        }
        MHal_NOE_LRO_Get_Ring_Info(E_NOE_DMA_PACKET, E_NOE_DIR_RX, i, &rx_ring);
        seq_printf(s, "RX_BASE_PTR%02d    : 0x%08x\n", i, (unsigned int)rx_ring.ring_st.base_adr);
        seq_printf(s, "RX_MAX_CNT%02d     : 0x%08x\n", i, rx_ring.ring_st.max_cnt);
        seq_printf(s, "RX_CALC_IDX%02d    : 0x%08x\n", i, rx_ring.ring_st.cpu_idx);
        seq_printf(s, "RX_DRX_IDX%02d     : 0x%08x\n\n", i, rx_ring.ring_st.dma_idx);
    }

    MHal_NOE_Get_Pdma_Info(E_NOE_PDMA_INFO_DBG, &dbg_info);
    seq_printf(s, "RX_DBG0          : 0x%08x\n", dbg_info.rx[0]);
    seq_printf(s, "RX_DBG1          : 0x%08x\n", dbg_info.rx[1]);

    if (ei_local->features & FE_ETHTOOL)
        seq_printf(s, "The current PHY address selected by ethtool is %d\n", MDrv_NOE_ETHTOOL_Get_Phy_Address());
}

static int _MDrv_NOE_PROC_Read_Regs(struct seq_file *seq, void *offset)
{
    _MDrv_NOE_PROC_Dump_Reg(seq);
    return 0;
}


static int _MDrv_NOE_PROC_Open_Gmac(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Read_Regs, NULL);
}

static int _MDrv_NOE_PROC_Read_Gmac2(struct seq_file *seq, void *offset)
{
    struct net_device *cur_dev_p;
    struct PSEUDO_ADAPTER *p_pseudo_ad;
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_GE2_SUPPORT) {
        cur_dev_p = dev_get_by_name(&init_net, ei_local->pseudo_dev->name);

        if (!cur_dev_p)
            return -EFAULT;
        p_pseudo_ad = netdev_priv(cur_dev_p);

        seq_printf(seq, "%d\n", p_pseudo_ad->mii_info.phy_id);
        return 0;
    }
    return -EFAULT;
}

static int _MDrv_NOE_PROC_Open_Gmac2(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Read_Gmac2, NULL);
}


static int _MDrv_NOE_PROC_Read_Pin_Mux(struct seq_file *seq, void *offset)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);
    seq_printf(seq, "Current Setting : \"%d\" \n", ei_local->pin_mux);
    seq_printf(seq, "%d:  MAC1 TO ChipTop\n", E_NOE_SEL_PIN_MUX_GE1_TO_CHIPTOP);
    seq_printf(seq, "%d:  MAC2 TO ChipTop\n", E_NOE_SEL_PIN_MUX_GE2_TO_CHIPTOP);
    seq_printf(seq, "%d:  MAC1 TO ChipTop, MAC2 TO CHIPTOP\n", E_NOE_SEL_PIN_MUX_GE1_TO_CHIPTOP_GE2_TO_CHIPTOP);
    return 0;
}


static int _MDrv_NOE_PROC_Open_Pin_Mux(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Read_Pin_Mux, NULL);
}

static ssize_t _MDrv_NOE_PROC_Write_Pin_Mux(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);
    char buf[4];
    int mux = NOE_CFG_PIN_MUX_SEL;

    if (count > 2)
        return -EFAULT;

    if (copy_from_user(buf, buffer, count))
        return -EFAULT;

    if ((buf[0] <='0') || (buf[0] >= '9'))
        return -EFAULT;

    mux = buf[0] - '0';
    NOE_MSG_DUMP("Pin Mux = %d, %s, %c, %zu\n", mux, buf, buf[0], count);

    if ((mux <= E_NOE_SEL_PIN_MUX_INVALID) || (mux >= E_NOE_SEL_PIN_MUX_MAX))
        return -EFAULT;

    ei_local->pin_mux = mux;
    MHal_NOE_Set_Pin_Mux(ei_local->pin_mux);

    return 0;
}

static ssize_t _MDrv_NOE_PROC_Write_Log_Ctrl(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
#define NOE_LOG_MAX_CNT (2)
    char buf[4];
    unsigned char level = 0xFF;

    if (count > (NOE_LOG_MAX_CNT + 1))
        return -EFAULT;

    if (copy_from_user(buf, buffer, count))
        return -EFAULT;

    if ((buf[0] >='0') && (buf[0] <= '9'))
        level = buf[0] - '0';
    else if ((buf[0] >='A') && (buf[0] <= 'F'))
        level = buf[0] - 'A' + 10;
    else if ((buf[0] >='a') && (buf[0] <= 'f'))
        level = buf[0] - 'a' + 10;

    if (strlen(buf) >= NOE_LOG_MAX_CNT) {
        if ((buf[1] >='0') && (buf[1] <= '9'))
            level = (buf[1] - '0')+ (level << 4);
        else if ((buf[1] >='A') && (buf[1] <= 'F'))
            level = (buf[1] - 'A' + 10) + (level << 4);
        else if ((buf[1] >='a') && (buf[0] <= 'f'))
            level = (buf[1] - 'a' + 10) + (level << 4);
    }

    if (level != 0xFF) {
        MDrv_NOE_LOG_Set_Level(level);
        return 0;
    }

    return -EFAULT;
}

static int _MDrv_NOE_PROC_Read_Log_Ctrl(struct seq_file *seq, void *offset)
{
    seq_printf(seq, "0x%X\n", MDrv_NOE_LOG_Get_Level());
    return 0;
}


static int _MDrv_NOE_PROC_Open_Log_Ctrl(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Read_Log_Ctrl, NULL);
}

static int _MDrv_NOE_PROC_Show_QoS(struct seq_file *seq, void *v)
{
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);
    struct noe_qdma_fq qdma_fq;
    struct noe_qdma_sch qdma_sch;
    struct noe_qdma_fsm qdma_fsm;
    struct noe_qdma_fc qdma_fc;
    struct noe_qdma_vq qdma_vq;
    struct noe_qdma_pq qdma_pq;
    unsigned int i, queue;

    if (ei_local->features & FE_QDMA) {
        seq_puts(seq, "==== General Information ====\n");
        MHal_NOE_Get_Qdma_Info(E_NOE_QDMA_INFO_FQ, &qdma_fq);
        seq_printf(seq, "SW TXD: %d/%d; HW TXD: %d/%d\n", qdma_fq.sw_fq, NUM_TX_DESC, qdma_fq.hw_fq, NUM_QDMA_PAGE);
        seq_printf(seq, "SW TXD virtual start address: 0x%p\n", ei_local->txd_pool);
        seq_printf(seq, "HW TXD virtual start address: 0x%p\n\n", free_head);

        seq_puts(seq, "==== Scheduler Information ====\n");
        MHal_NOE_Get_Qdma_Info(E_NOE_QDMA_INFO_SCH, &qdma_sch);
        seq_printf(seq, "SCH1 rate control:%d. Rate is %dKbps.\n", qdma_sch.sch[0].max_en,qdma_sch.sch[0].max_rate);
        seq_printf(seq, "SCH2 rate control:%d. Rate is %dKbps.\n\n", qdma_sch.sch[1].max_en, qdma_sch.sch[1].max_rate);

        seq_puts(seq, "==== Physical Queue Information ====\n");
        for (queue = 0; queue < NUM_PQ; queue++) {
            qdma_pq.queue = queue;
            MHal_NOE_Get_Qdma_Info(E_NOE_QDMA_INFO_PQ, &qdma_pq);
            seq_printf(seq, "Queue#%d Information:\n", queue);
            seq_printf(seq, "%d packets in the queue; head address is 0x%08x, tail address is 0x%08x.\n", qdma_pq.txd_cnt, qdma_pq.queue_head, qdma_pq.queue_tail);
            seq_printf(seq, "HW_RESV: %d; SW_RESV: %d; SCH: %d; Weighting: %d\n", qdma_pq.hw_resv, qdma_pq.sw_resv, qdma_pq.sch, qdma_pq.weight);
            seq_printf(seq, "Min_Rate_En is %d, Min_Rate is %dKbps; Max_Rate_En is %d, Max_Rate is %dKbps.\n\n", qdma_pq.min_en, qdma_pq.min_rate, qdma_pq.max_en, qdma_pq.max_rate);
        }
        if (ei_local->features & FE_HW_SFQ) {
            seq_puts(seq, "==== Virtual Queue Information ====\n");
            seq_printf(seq, "VQTX_TB_BASE_0:0x%p;VQTX_TB_BASE_1:0x%p;VQTX_TB_BASE_2:0x%p;VQTX_TB_BASE_3:0x%p\n", sfq0, sfq1, sfq2, sfq3);
            MHal_NOE_Get_Qdma_Info(E_NOE_QDMA_INFO_VQ, &qdma_vq);
            seq_printf(seq, "VQTX_NUM_0:0x%01x;VQTX_NUM_1:0x%01x;VQTX_NUM_2:0x%01x;VQTX_NUM_3:0x%01x\n\n",
                qdma_vq.vq_num[E_NOE_VQ_NO0], qdma_vq.vq_num[E_NOE_VQ_NO1], qdma_vq.vq_num[E_NOE_VQ_NO2], qdma_vq.vq_num[E_NOE_VQ_NO3] );
        }

        seq_puts(seq, "==== Flow Control Information ====\n");
        MHal_NOE_Get_Qdma_Info(E_NOE_QDMA_INFO_FC, &qdma_fc);
        seq_printf(seq, "SW_DROP_EN:%x; SW_DROP_FFA:%d; SW_DROP_MODE:%d\n", qdma_fc.sw.en, qdma_fc.sw.ffa, qdma_fc.sw.mode);
        seq_printf(seq, "HW_DROP_EN:%x; HW_DROP_FFA:%d; HW_DROP_MODE:%d\n", qdma_fc.hw.en, qdma_fc.hw.ffa, qdma_fc.hw.mode);
        seq_printf(seq, "SW_DROP_FSTVQ_MODE:%d;SW_DROP_FSTVQ:%d\n", qdma_fc.sw.fst_vq_mode, qdma_fc.sw.fst_vq_en);
        seq_printf(seq, "HW_DROP_FSTVQ_MODE:%d;HW_DROP_FSTVQ:%d\n", qdma_fc.hw.fst_vq_mode, qdma_fc.hw.fst_vq_en);

        seq_puts(seq, "\n==== FSM Information\n");
        MHal_NOE_Get_Qdma_Info(E_NOE_QDMA_INFO_FSM, &qdma_fsm);
        seq_printf(seq, "VQTB_FSM:0x%01x\n", qdma_fsm.vqtb);
        seq_printf(seq, "FQ_FSM:0x%01x\n", qdma_fsm.fq);
        seq_printf(seq, "TX_FSM:0x%01x\n", qdma_fsm.tx);
        seq_printf(seq, "RX_FSM:0x%01x\n", qdma_fsm.rx);
        seq_printf(seq, "RLS_FSM:0x%01x\n", qdma_fsm.rls);
        seq_printf(seq, "FWD_FSM:0x%01x\n", qdma_fsm.fwd);

        seq_puts(seq, "==== M2Q Information ====\n");
        for (i = 0; i < NUM_PQ; i += 8) {
            seq_printf(seq, "(%2d,%2d)(%2d,%2d)(%2d,%2d)(%2d,%2d)(%2d,%2d)(%2d,%2d)(%2d,%2d)(%2d,%2d)\n",
                   i, M2Q_table[i], i + 1, M2Q_table[i + 1],
                   i + 2, M2Q_table[i + 2], i + 3,
                   M2Q_table[i + 3], i + 4, M2Q_table[i + 4],
                   i + 5, M2Q_table[i + 5], i + 6,
                   M2Q_table[i + 6], i + 7, M2Q_table[i + 7]);
        }

        return 0;
    }
    else {
        return 0;
    }
}

static int _MDrv_NOE_PROC_Open_QoS(struct inode *inode, struct file *file)
{
    return single_open(file, _MDrv_NOE_PROC_Show_QoS, NULL);
}



int MDrv_NOE_Update_Tso_Len(int tso_len)
{

    if(tso_len > 70000) {
        tso_cnt[14]++;
    }else if(tso_len >  65000) {
        tso_cnt[13]++;
    }else if(tso_len >  60000) {
        tso_cnt[12]++;
    }else if(tso_len >  55000) {
        tso_cnt[11]++;
    }else if(tso_len >  50000) {
        tso_cnt[10]++;
    }else if(tso_len >  45000) {
        tso_cnt[9]++;
    }else if(tso_len > 40000) {
        tso_cnt[8]++;
    }else if(tso_len > 35000) {
        tso_cnt[7]++;
    }else if(tso_len > 30000) {
        tso_cnt[6]++;
    }else if(tso_len > 25000) {
        tso_cnt[5]++;
    }else if(tso_len > 20000) {
        tso_cnt[4]++;
    }else if(tso_len > 15000) {
        tso_cnt[3]++;
    }else if(tso_len > 10000) {
        tso_cnt[2]++;
    }else if(tso_len > 5000) {
        tso_cnt[1]++;
    }else {
        tso_cnt[0]++;
    }

    return 0;
}


int MDrv_NOE_PROC_Init(struct net_device *dev)
{
    struct END_DEVICE *ei_local = NULL;
    int i = 0;
    unsigned char enable = 0;
    _noe_proc_dev = dev;
    ei_local = netdev_priv(_noe_proc_dev);

    if (!proc_reg_dir)
        proc_reg_dir = proc_mkdir(NOE_PROC_DIR, NULL);

    if (ei_local->features & FE_HW_LRO)
        MDrv_NOE_LRO_PROC_Init(proc_reg_dir, dev);

    for (i = 0; i < E_NOE_PROC_MAX; i++) {
        enable = 0;
        switch(i) {
            case E_NOE_PROC_GMAC2:
                if (ei_local->features & (FE_ETHTOOL | FE_GE2_SUPPORT))
                    enable = 1;
                break;
            case E_NOE_PROC_NUM_OF_TXD:
            case E_NOE_PROC_TSO_LEN:
                if (ei_local->features & FE_TSO)
                    enable = 1;
                break;
            case E_NOE_PROC_LRO_STATS:
                if (ei_local->features & FE_SW_LRO)
                    enable = 1;
                break;
            case E_NOE_PROC_SNMP:
                if (ei_local->features & USER_SNMPD)
                    enable = 1;
                break;
            case E_NOE_PROC_SCHE:
                if (ei_local->features & TASKLET_WORKQUEUE_SW)
                    enable = 1;
                break;
            case E_NOE_PROC_INT_DBG:
                if (ei_local->features & FE_NOE_INT_DBG)
                    enable = 1;
                break;
            default:
                enable = 1;
                break;
        }

        if (enable == 1) {
            proc_entry[i] = proc_create(noe_proc_name[i], 0, proc_reg_dir, &noe_proc_fops[i]);
            if (!proc_entry[i])
                NOE_MSG_ERR("!! FAIL to create %s PROC !!\n", noe_proc_name[i]);
        }
    }
    return 0;
}



void MDrv_NOE_PROC_Exit(void)
{
    int i = 0;
    struct END_DEVICE *ei_local = netdev_priv(_noe_proc_dev);

    if (ei_local->features & FE_HW_LRO)
        MDrv_NOE_LRO_PROC_Exit(proc_reg_dir);

    for (i = 0; i < E_NOE_PROC_MAX; i++) {
        if (!proc_entry[i]) {
            remove_proc_entry(noe_proc_name[i], proc_reg_dir);
            proc_entry[i] = NULL;
        }
    }
}


EXPORT_SYMBOL(proc_reg_dir);

