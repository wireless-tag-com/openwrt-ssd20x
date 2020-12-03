///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2006 - 2007 Mstar Semiconductor, Inc.
// This program is free software.
// You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   devGMAC.c
/// @brief  GMAC Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/init.h>
#include <linux/autoconf.h>
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
#include <linux/cpumask.h>
#include <linux/smp.h>

#if defined(CONFIG_MIPS)
#include <asm/mips-boards/prom.h>
#include "mhal_chiptop_reg.h"
#elif defined(CONFIG_ARM)
#include <prom.h>
#include <asm/mach/map.h>
#elif defined(CONFIG_ARM64)
#include <asm/arm-boards/prom.h>
#include <asm/mach/map.h>
#endif
#include "mdrv_types.h"
#include "mst_platform.h"
#include "mdrv_system.h"
#include "chip_int.h"
#include "mhal_gmac.h"
#include "mdrv_gmac.h"
#include "chip_setup.h"

#ifdef CONFIG_GMAC_SUPPLY_RNG
#include <linux/input.h>
#include <random.h>
#include "mhal_rng_reg.h"
#endif
//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
#define GMAC_RX_TMR         (0)
#define GMAC_LINK_TMR       (1)

#define GMAC_CHECK_LINK_TIME        (HZ)

#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
#define GMAC_IER_FOR_INT_JULIAN_D  GMAC_INT_TUND|GMAC_INT_RTRY|GMAC_INT_TCOM|GMAC_INT_ROVR|GMAC_INT_RBNA
#else /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */
#define GMAC_IER_FOR_INT_JULIAN_D   (0x0000E435UL)
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */

#define GMAC_CHECK_CNT              (500000)

#define GMAC_TX_PTK_BASE            (GMAC_TX_SKB_BASE + GMAC_RAM_VA_PA_OFFSET)

#define GMAC_ALBANY_OUI_MSB              (0)
#define GMAC_RTL_8210                    (0x1CUL)

#define GMAC_RX_THROUGHPUT_TEST 0
#define GMAC_RX_THROUGHPUT_TEST_ON_RECEIVE 0
#define GMAC_TX_THROUGHPUT_TEST 0

#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
#define GMAC_TX_SW_QUEUE_IN_GENERAL_TX     0
#define GMAC_TX_SW_QUEUE_IN_IRQ            1
#define GMAC_TX_SW_QUEUE_IN_TIMER          2
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */
//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
u32 gmac_initstate= 0;
u8 gmac_txidx =0;
spinlock_t gmac_lock;
spinlock_t gmac_txlock;
#ifdef CONFIG_GMAC_ISR_BOTTOM_HALF
spinlock_t gmac_lock_rx;
spinlock_t gmac_lock_rst;
#endif /* CONFIG_GMAC_ISR_BOTTOM_HALF */
u32 ROVRcount = 0;
unsigned char gmac_phyaddr = 0;
static unsigned int gmac_debug = 0;
static unsigned int gmac_tx_debug = 0;
static unsigned int gmac_debug_napi = 0;
static unsigned int gmac_dump_skb = 0;
unsigned long received_num = 0;

#ifdef GMAC_RX_ZERO_COPY
static struct sk_buff *rx_skb[GMAC_MAX_RX_DESCR];
static struct sk_buff * rx_skb_dummy;
#ifdef CONFIG_ARM64
static u64 rx_abso_addr[GMAC_MAX_RX_DESCR];
static u64 rx_abso_addr_dummy;
#else
static u32 rx_abso_addr[GMAC_MAX_RX_DESCR];
static u32 rx_abso_addr_dummy;
#endif /* CONFIG_ARM64 */

#ifdef CONFIG_GMAC_RX_CMA
extern struct device *pci_cma_device;
extern struct sk_buff *__alloc_skb_from_cma(struct device *cma_dev, unsigned int size, gfp_t gfp_mask, int flags, int node);
#endif /* CONFIG_GMAC_RX_CMA */
#endif

#ifdef CONFIG_GMAC_ISR_BH_NAPI
static void MDev_GMAC_enable_INT_RX(void);
static void MDev_GMAC_disable_INT_RX(void);
static int MDev_GMAC_napi_poll(struct napi_struct *napi, int budget);
#endif /* CONFIG_GMAC_ISR_BH_NAPI */

#ifdef GMAC_NAPI
#ifdef NR_NAPI
int rx_napi_weight[NR_CPUS] = {4, 8, 8, 32};
module_param_array(rx_napi_weight, int, NULL, 0444);
MODULE_PARM_DESC(rx_napi_weight, "Per CPU NAPI WEIGHT parameter in GMAC.");
struct gmac_napi_wrapper {
	struct napi_struct napi_str;
	int available;
	int cpu;
} __cacheline_aligned_in_smp;
static struct gmac_napi_wrapper gmac_napi[NR_CPUS] __cacheline_aligned_in_smp;

//-------------------------------------------------------------------------------------------------
// Record each core status
//-------------------------------------------------------------------------------------------------
struct gmac_core_state {
	int baseline_cores;
	int active_cores;
	//volatile int active_cores;
	spinlock_t lock;
} ____cacheline_aligned_in_smp;

static struct gmac_core_state cpu_state __cacheline_aligned_in_smp;
#endif
#endif

// 0x78c9: link is down.
static u32 gmac_phy_status_register = 0x78c9UL;

struct sk_buff *gmac_pseudo_packet;

#if GMAC_TX_THROUGHPUT_TEST
static unsigned int gmac_tx_test = 0;
static unsigned int gmac_tx_thread = 10;
static struct task_struct *tx_tsk[20];
unsigned char gmac_packet_content[] = {
0xa4, 0xba, 0xdb, 0x95, 0x25, 0x29, 0x00, 0x30, 0x1b, 0xba, 0x02, 0xdb, 0x08, 0x00, 0x45, 0x00,
0x05, 0xda, 0x69, 0x0a, 0x40, 0x00, 0x40, 0x11, 0xbe, 0x94, 0xac, 0x10, 0x5a, 0xe3, 0xac, 0x10,
0x5a, 0x70, 0x92, 0x7f, 0x13, 0x89, 0x05, 0xc6, 0x0c, 0x5b, 0x00, 0x00, 0x03, 0x73, 0x00, 0x00,
0x00, 0x65, 0x00, 0x06, 0xe1, 0xef, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
0x13, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0xff, 0xff, 0xfc, 0x18, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39
};
#endif

unsigned char ipv6udp_csumok[] =
{
0x00, 0x00, 0x86, 0x05, 0x80, 0xda, 0x00, 0x30, 0x1b, 0xba, 0x02, 0xdb, 0x86, 0xdd, 0x60, 0x00,
0x00, 0x00, 0x00, 0x14, 0x11, 0x02, 0x3f, 0xfe, 0x05, 0x07, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00,
0x86, 0xff, 0xfe, 0x05, 0x80, 0xda, 0x3f, 0xfe, 0x05, 0x01, 0x04, 0x10, 0x00, 0x00, 0x02, 0xc0,
0xdf, 0xff, 0xfe, 0x47, 0x03, 0x3e, 0xa0, 0x75, 0x82, 0x9f, 0x00, 0x14, 0xed, 0xe2, 0x05, 0x02,
0x00, 0x00, 0xf9, 0xc8, 0xe7, 0x36, 0x85, 0x91, 0x09, 0x00
};

unsigned char ipv6udp_csumerr[] =
{
0x00, 0x00, 0x86, 0x05, 0x80, 0xda, 0x00, 0x30, 0x1b, 0xba, 0x02, 0xdb, 0x86, 0xdd, 0x60, 0x00,
0x00, 0x00, 0x00, 0x14, 0x11, 0x02, 0x3f, 0xfe, 0x05, 0x07, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00,
0x86, 0xff, 0xfe, 0x05, 0x80, 0xda, 0x3f, 0xfe, 0x05, 0x01, 0x04, 0x10, 0x00, 0x00, 0x02, 0xc0,
0xdf, 0xff, 0xfe, 0x47, 0x03, 0x3e, 0xa0, 0x75, 0x82, 0x9f, 0x00, 0x14, 0x12, 0x34, 0x05, 0x02,
0x00, 0x00, 0xf9, 0xc8, 0xe7, 0x36, 0x85, 0x91, 0x09, 0x00
};

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
static struct timer_list GMAC_timer, GMAC_Link_timer;
#if GMAC_RX_THROUGHPUT_TEST
#define RX_THROUGHPUT_TEST_INTERVAL 10
static struct timer_list GMAC_RX_timer;
#endif
static struct net_device *gmac_dev;
//-------------------------------------------------------------------------------------------------
//  GMAC Function
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_tx (struct sk_buff *skb, struct net_device *dev);
static void MDev_GMAC_timer_callback( unsigned long value );
static int MDev_GMAC_SwReset(struct net_device *dev);
static void MDev_GMAC_irq_onoff(int enable, const char *call_from);
//static void MDev_GMAC_Send_PausePkt(struct net_device* dev);
#ifdef GMAC_RX_ZERO_COPY
static int GMAC_rx_fill_ring(struct net_device *netdev);
static int GMAC_dequeue_rx_buffer(struct GMAC_private *p, struct sk_buff **pskb);
static int free_rx_skb(void);
static int free_rx_skb(void)
{
	int i = 0;

	for (i = 0; i < GMAC_MAX_RX_DESCR; i ++)
	{
		if (rx_skb[i])
			kfree_skb(rx_skb[i]);

	}

}
#endif

#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
static void _MDev_GMAC_tx_reset_TX_SW_QUEUE(struct net_device* netdev);
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */

static void dump_skb(struct sk_buff *skb)
{
	int i;
	unsigned char *data = skb->data;

    printk("=== pdata=0x%p, len=%d ===\n", data, skb->len);
	for (i = 0; i < skb->len; i++) {
        if ((u32)i%0x10UL ==0)
            printk("%lx: ", (long unsigned int)&data[i]);
        if (data[i] < 0x10UL)
            printk("0%x ", data[i]);
        else
            printk("%x ", data[i]);
        if ((u32)i%0x10UL == 0x0fUL)
			printk("\n");
	}
    printk("\n");
}

unsigned long gmac_oldTime;
static unsigned long getCurMs(void)
{
    struct timeval tv;
    unsigned long curMs;

    do_gettimeofday(&tv);
    curMs = tv.tv_usec/1000;
    curMs += tv.tv_sec * 1000;
    return curMs;
}

#if GMAC_RX_THROUGHPUT_TEST
unsigned long gmac_receive_bytes = 0;
static void RX_timer_callback( unsigned long value){
    int get_bytes = receive_bytes;
    int cur_speed;
    receive_bytes = 0;

    cur_speed = get_bytes*8/RX_THROUGHPUT_TEST_INTERVAL;
    printk(" %dkbps",cur_speed);
    GMAC_RX_timer.expires = jiffies + (RX_THROUGHPUT_TEST_INTERVAL*GMAC_CHECK_LINK_TIME);
    add_timer(&RX_timer);
}
#endif

#if GMAC_TX_THROUGHPUT_TEST
static struct timer_list GMAC_TX_timer;
int gmac_tx_bytes = 0;
u64 tx_bytes = 0;
u64 tx_count = 0;
static void TX_timer_callback( unsigned long value){
    u64 get_bytes = tx_bytes;
    u64 get_count = tx_count;
    u64 cur_speed;
    tx_bytes = 0;
	tx_count = 0;

    cur_speed = get_bytes*8/10;
    printk("\n TX %llu bps count= %llu\n",cur_speed, get_count);
    GMAC_TX_timer.expires = jiffies + 10*GMAC_CHECK_LINK_TIME;
    add_timer(&GMAC_TX_timer);
}

static void tx_sender(void* arg){
	int cpu;
	while (1) {
	if (gmac_tx_test) {
		while(1){
			MDev_GMAC_tx(gmac_pseudo_packet, gmac_dev);
			if (!gmac_tx_test) {
				break;
			}
		}
	} else {
		cpu=get_cpu(); put_cpu();
		printk("%s %d:cpu:%d\n",__func__,__LINE__,cpu);
		msleep(5000);
	}
	}
}
#endif

//-------------------------------------------------------------------------------------------------
// PHY MANAGEMENT
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Access the PHY to determine the current Link speed and Mode, and update the
// MAC accordingly.
// If no link or auto-negotiation is busy, then no changes are made.
// Returns:  0 : OK
//              -1 : No link
//              -2 : AutoNegotiation still in progress
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_update_linkspeed (struct net_device *dev)
{
    u32 bmsr, bmcr, physr;
    u32 speed, duplex, LocPtrA;

    if (gmac_phyaddr >= 32) return 0;
    /* Link status is latched, so read twice to get current value */
    MHal_GMAC_read_phy (gmac_phyaddr, MII_BMSR, &bmsr);
    MHal_GMAC_read_phy (gmac_phyaddr, MII_BMSR, &bmsr);

    /* No link */
    if (!(bmsr & BMSR_LSTATUS)){
        return -1;
    }
    MHal_GMAC_read_phy (gmac_phyaddr, MII_BMCR, &bmcr);

    /* AutoNegotiation is enabled */
    if (bmcr & BMCR_ANENABLE)
    {
        if (!(bmsr & BMSR_ANEGCOMPLETE))
        {
            GMAC_DBG("==> AutoNegotiation still in progress\n");
            return -2;
        }

#ifndef CONFIG_GMAC_ETHERNET_ALBANY

        MHal_GMAC_read_phy (gmac_phyaddr, MII_PHYSR, &physr);

        if(((physr & PHYSR_SPEED_MASK) & PHYSR_1000) &&
           ((physr & PHYSR_DUPLEX_MASK) & PHYSR_FULL_DUPLEX))
        {
            speed = SPEED_1000;
            duplex = DUPLEX_FULL;
        }
        else if(((physr & PHYSR_SPEED_MASK) & PHYSR_1000) &&
                !((physr & PHYSR_DUPLEX_MASK) & PHYSR_HALF_DUPLEX))
        {
            speed = SPEED_1000;
            duplex = DUPLEX_HALF;
        }
        else if(((physr & PHYSR_SPEED_MASK) & PHYSR_100) &&
                ((physr & PHYSR_DUPLEX_MASK) & PHYSR_FULL_DUPLEX))
        {
            speed = SPEED_100;
            duplex = DUPLEX_FULL;
        }
        else if(((physr & PHYSR_SPEED_MASK) & PHYSR_100) &&
                !((physr & PHYSR_DUPLEX_MASK) & PHYSR_HALF_DUPLEX))
        {
            speed = SPEED_100;
            duplex = DUPLEX_HALF;
        }
        else if(!((physr & PHYSR_SPEED_MASK) & PHYSR_10) &&
                ((physr & PHYSR_DUPLEX_MASK) & PHYSR_FULL_DUPLEX))
        {
            speed = SPEED_10;
            duplex = DUPLEX_FULL;
        }
        else
        {
            speed = SPEED_10;
            duplex = DUPLEX_HALF;
        }
#else
         MHal_GMAC_read_phy(gmac_phyaddr, MII_LPA, &LocPtrA);
        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_100HALF))
        {
            speed = SPEED_100;
        }
        else
        {
            speed = SPEED_10;
        }

        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_10FULL))
        {
            duplex = DUPLEX_FULL;
        }
        else
        {
            duplex = DUPLEX_HALF;
        }
#endif
    }
    else
    {
        speed = (bmcr & 0x0040) ? SPEED_1000 : ((bmcr & BMCR_SPEED100) ? SPEED_100 : SPEED_10);
        duplex = (bmcr & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
    }

    // Update the MAC //
    MHal_GMAC_update_speed_duplex(speed,duplex);
    return 0;
}

static int MDev_GMAC_get_info(struct net_device *dev)
{
    u32 bmsr, bmcr, LocPtrA;
    u32 uRegStatus =0;

    // Link status is latched, so read twice to get current value //
    MHal_GMAC_read_phy (gmac_phyaddr, MII_BMSR, &bmsr);
    MHal_GMAC_read_phy (gmac_phyaddr, MII_BMSR, &bmsr);
    if (!(bmsr & BMSR_LSTATUS)){
        uRegStatus &= ~GMAC_ETHERNET_TEST_RESET_STATE;
        uRegStatus |= GMAC_ETHERNET_TEST_NO_LINK; //no link //
    }
    MHal_GMAC_read_phy (gmac_phyaddr, MII_BMCR, &bmcr);

    if (bmcr & BMCR_ANENABLE)
    {
        //AutoNegotiation is enabled //
        if (!(bmsr & BMSR_ANEGCOMPLETE))
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_RESET_STATE;
            uRegStatus |= GMAC_ETHERNET_TEST_AUTO_NEGOTIATION; //AutoNegotiation //
        }
        else
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_RESET_STATE;
            uRegStatus |= GMAC_ETHERNET_TEST_LINK_SUCCESS; //link success //
        }

        MHal_GMAC_read_phy (gmac_phyaddr, MII_LPA, &LocPtrA);
        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_100HALF))
        {
            uRegStatus |= GMAC_ETHERNET_TEST_SPEED_100M; //SPEED_100//
        }
        else
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_SPEED_100M; //SPEED_10//
        }

        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_10FULL))
        {
            uRegStatus |= GMAC_ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_FULL//
        }
        else
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_HALF//
        }
    }
    else
    {
        if(bmcr & BMCR_SPEED100)
        {
            uRegStatus |= GMAC_ETHERNET_TEST_SPEED_100M; //SPEED_100//
        }
        else
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_SPEED_100M; //SPEED_10//
        }

        if(bmcr & BMCR_FULLDPLX)
        {
            uRegStatus |= GMAC_ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_FULL//
        }
        else
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_HALF//
        }
    }

    return uRegStatus;
}

//-------------------------------------------------------------------------------------------------
//Program the hardware MAC address from dev->dev_addr.
//-------------------------------------------------------------------------------------------------
void MDev_GMAC_update_mac_address (struct net_device *dev)
{
    u32 value;
    value = (dev->dev_addr[3] << 24) | (dev->dev_addr[2] << 16) | (dev->dev_addr[1] << 8) |(dev->dev_addr[0]);
    MHal_GMAC_Write_SA1L(value);
    value = (dev->dev_addr[5] << 8) | (dev->dev_addr[4]);
    MHal_GMAC_Write_SA1H(value);
}

//-------------------------------------------------------------------------------------------------
// ADDRESS MANAGEMENT
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Set the ethernet MAC address in dev->dev_addr
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_get_mac_address (struct net_device *dev)
{
    char addr[6];
    u32 HiAddr, LoAddr;

    // Check if bootloader set address in Specific-Address 1 //
    HiAddr = MHal_GMAC_get_SA1H_addr();
    LoAddr = MHal_GMAC_get_SA1L_addr();

    addr[0] = (LoAddr & 0xffUL);
    addr[1] = (LoAddr & 0xff00UL) >> 8;
    addr[2] = (LoAddr & 0xff0000UL) >> 16;
    addr[3] = (LoAddr & 0xff000000UL) >> 24;
    addr[4] = (HiAddr & 0xffUL);
    addr[5] = (HiAddr & 0xff00UL) >> 8;

    if (is_valid_ether_addr (addr))
    {
        memcpy (dev->dev_addr, &addr, 6);
        return;
    }
    // Check if bootloader set address in Specific-Address 2 //
    HiAddr = MHal_GMAC_get_SA2H_addr();
    LoAddr = MHal_GMAC_get_SA2L_addr();
    addr[0] = (LoAddr & 0xffUL);
    addr[1] = (LoAddr & 0xff00UL) >> 8;
    addr[2] = (LoAddr & 0xff0000UL) >> 16;
    addr[3] = (LoAddr & 0xff000000UL) >> 24;
    addr[4] = (HiAddr & 0xffUL);
    addr[5] = (HiAddr & 0xff00UL) >> 8;

    if (is_valid_ether_addr (addr))
    {
        memcpy (dev->dev_addr, &addr, 6);
        return;
    }
}

#ifdef GMAC_URANUS_ETHER_ADDR_CONFIGURABLE
//-------------------------------------------------------------------------------------------------
// Store the new hardware address in dev->dev_addr, and update the MAC.
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_set_mac_address (struct net_device *dev, void *addr)
{
    struct sockaddr *address = addr;
    if (!is_valid_ether_addr (address->sa_data))
        return -EADDRNOTAVAIL;

    memcpy (dev->dev_addr, address->sa_data, dev->addr_len);
    MDev_GMAC_update_mac_address (dev);
    return 0;
}
#endif

#if 0
//-------------------------------------------------------------------------------------------------
// Add multicast addresses to the internal multicast-hash table.
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_sethashtable (struct net_device *dev)
{
   struct dev_mc_list *curr;
   u32 mc_filter[2], i, bitnr;

   mc_filter[0] = mc_filter[1] = 0;

   curr = dev->mc_list;
   for (i = 0; i < dev->mc_count; i++, curr = curr->next)
   {
       if (!curr)
           break;          // unexpected end of list //

       bitnr = ether_crc (ETH_ALEN, curr->dmi_addr) >> 26;
       mc_filter[bitnr >> 5] |= 1 << (bitnr & 31);
   }

   MHal_GMAC_update_HSH(mc_filter[0],mc_filter[1]);
}
#endif

//-------------------------------------------------------------------------------------------------
//Enable/Disable promiscuous and multicast modes.
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_set_rx_mode (struct net_device *dev)
{
    u32 uRegVal;
    uRegVal  = MHal_GMAC_Read_CFG();

    if (dev->flags & IFF_PROMISC)
    {   // Enable promiscuous mode //
        uRegVal |= GMAC_CAF;
    }
    else if (dev->flags & (~IFF_PROMISC))
    {   // Disable promiscuous mode //
        uRegVal &= ~GMAC_CAF;
    }
    MHal_GMAC_Write_CFG(uRegVal);

    if (dev->flags & IFF_ALLMULTI)
    {   // Enable all multicast mode //
        MHal_GMAC_update_HSH(-1,-1);
        uRegVal |= GMAC_MTI;
    }
    else if (dev->flags & IFF_MULTICAST)
    {   // Enable specific multicasts//
        //MDev_GMAC_sethashtable (dev);
        MHal_GMAC_update_HSH(-1,-1);
        uRegVal |= GMAC_MTI;
    }
    else if (dev->flags & ~(IFF_ALLMULTI | IFF_MULTICAST))
    {   // Disable all multicast mode//
        MHal_GMAC_update_HSH(0,0);
        uRegVal &= ~GMAC_MTI;
    }

    MHal_GMAC_Write_CFG(uRegVal);
}
//-------------------------------------------------------------------------------------------------
// IOCTL
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enable/Disable MDIO
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_mdio_read (struct net_device *dev, int phy_id, int location)
{
    u32 value;
    MHal_GMAC_read_phy (phy_id, location, &value);
    return value;
}

static void MDev_GMAC_mdio_write (struct net_device *dev, int phy_id, int location, int value)
{
    MHal_GMAC_write_phy (phy_id, location, value);
}

//-------------------------------------------------------------------------------------------------
//ethtool support.
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_ethtool_ioctl (struct net_device *dev, void *useraddr)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 ethcmd;
    int res = 0;

    if (copy_from_user (&ethcmd, useraddr, sizeof (ethcmd)))
        return -EFAULT;

    spin_lock_irq (LocPtr->lock);

    switch (ethcmd)
    {
        case ETHTOOL_GSET:
        {
            struct ethtool_cmd ecmd = { ETHTOOL_GSET };
            res = mii_ethtool_gset (&LocPtr->mii, &ecmd);
            if (LocPtr->phy_media == PORT_FIBRE)
            {   //override media type since mii.c doesn't know //
                ecmd.supported = SUPPORTED_FIBRE;
                ecmd.port = PORT_FIBRE;
            }
            if (copy_to_user (useraddr, &ecmd, sizeof (ecmd)))
                res = -EFAULT;
            break;
        }
        case ETHTOOL_SSET:
        {
            struct ethtool_cmd ecmd;
            if (copy_from_user (&ecmd, useraddr, sizeof (ecmd)))
                res = -EFAULT;
            else
                res = mii_ethtool_sset (&LocPtr->mii, &ecmd);
            break;
        }
        case ETHTOOL_NWAY_RST:
        {
            res = mii_nway_restart (&LocPtr->mii);
            break;
        }
        case ETHTOOL_GLINK:
        {
            struct ethtool_value edata = { ETHTOOL_GLINK };
            edata.data = mii_link_ok (&LocPtr->mii);
            if (copy_to_user (useraddr, &edata, sizeof (edata)))
                res = -EFAULT;
            break;
        }
        default:
            res = -EOPNOTSUPP;
    }
    spin_unlock_irq (LocPtr->lock);
    return res;
}

//-------------------------------------------------------------------------------------------------
// User-space ioctl interface.
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    struct mii_ioctl_data *data = if_mii(rq);

    if (!netif_running(dev))
    {
        rq->ifr_metric = GMAC_ETHERNET_TEST_INIT_FAIL;
    }

    switch (cmd)
    {
        case SIOCGMIIPHY:
            data->phy_id = (gmac_phyaddr & 0x1F);
            return 0;

        case SIOCDEVPRIVATE:
            rq->ifr_metric = (MDev_GMAC_get_info(gmac_dev)|gmac_initstate);
            return 0;

        case SIOCDEVON:
            MHal_GMAC_Power_On_Clk();
            return 0;

        case SIOCDEVOFF:
            MHal_GMAC_Power_Off_Clk();
            return 0;

        case SIOCGMIIREG:
            // check PHY's register 1.
            if((data->reg_num & 0x1fUL) == 0x1)
            {
                // PHY's register 1 value is set by timer callback function.
                spin_lock_irq(LocPtr->lock);
                data->val_out = gmac_phy_status_register;
                spin_unlock_irq(LocPtr->lock);
            }
            else
            {
                MHal_GMAC_read_phy((gmac_phyaddr & 0x1FUL), (data->reg_num & 0x1fUL), (u32 *)&(data->val_out));
            }
            return 0;

        case SIOCSMIIREG:
            if (!capable(CAP_NET_ADMIN))
                return -EPERM;
            MHal_GMAC_write_phy((gmac_phyaddr & 0x1FUL), (data->reg_num & 0x1fUL), data->val_in);
            return 0;

        case SIOCETHTOOL:
            return MDev_GMAC_ethtool_ioctl (dev, (void *) rq->ifr_data);

        default:
            return -EOPNOTSUPP;
    }
}
//-------------------------------------------------------------------------------------------------
// MAC
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//Initialize and start the Receiver and Transmit subsystems
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_start (struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    struct gmac_recv_desc_bufs *dlist, *dlist_phys;
#ifndef GMAC_SOFTWARE_DESCRIPTOR
    int i;
#endif
#ifdef GMAC_RX_ZERO_COPY
    u32 RBQP_rx_skb_addr = 0;
#endif
    u32 uRegVal;

    dlist = LocPtr->dlist;
    dlist_phys = LocPtr->dlist_phys;
#ifdef GMAC_SOFTWARE_DESCRIPTOR
    dlist->descriptors[GMAC_MAX_RX_DESCR - 1].addr |= GMAC_DESC_WRAP;
#else
    for(i = 0; i < GMAC_MAX_RX_DESCR; i++)
    {
        dlist->descriptors[i].addr = 0;
        dlist->descriptors[i].size = 0;
    }
    // Set the Wrap bit on the last descriptor //
    dlist->descriptors[GMAC_MAX_RX_DESCR - 1].addr = GMAC_DESC_WRAP;
#endif //#ifndef SOFTWARE_DESCRIPTOR

    // Program address of descriptor list in Rx Buffer Queue register //
    uRegVal = ((GMAC_REG) & dlist_phys->descriptors)- GMAC_RAM_VA_PA_OFFSET - GMAC_MIU0_BUS_BASE;
    MHal_GMAC_Write_RBQP(uRegVal);

    //Reset buffer index//
    LocPtr->rxBuffIndex = 0;

    // Enable Receive and Transmit //
    uRegVal = MHal_GMAC_Read_CTL();
    uRegVal |= (GMAC_RE | GMAC_TE);
    MHal_GMAC_Write_CTL(uRegVal);
}

#ifdef NR_NAPI
//-------------------------------------------------------------------------------------------------
// Trigger the first NAPI to receive packet
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Schedule a NAPI to receive packet
//-------------------------------------------------------------------------------------------------
static void gmac_enable_napi(void)
{
	int cpu = smp_processor_id();

	napi_schedule(&gmac_napi[cpu].napi_str);
}

//-------------------------------------------------------------------------------------------------
// Trigger one avaliable CPU to receive packet
//-------------------------------------------------------------------------------------------------
static void gmac_enable_one_cpu(char *call_from)
{
	int cpu, ret = 0;
	unsigned long flags;
	spin_lock_irqsave(&cpu_state.lock, flags);
	/* if the CPU is available, trigger the NAPI polling for the CPU. */
	for_each_online_cpu(cpu) {
		if (gmac_napi[cpu].available > 0) {
			gmac_napi[cpu].available--;
			cpu_state.active_cores++;
			if (cpu == smp_processor_id()) {
				BUG_ON(cpu_state.active_cores > cpu_state.baseline_cores);
				gmac_enable_napi();
			}
			else {
				ret = smp_call_function_single(cpu, gmac_enable_napi, NULL, 0);
				if (unlikely(gmac_debug_napi))
					printk("%s %d: cpu:%d active_cores:%d napi[%d].available:%d from:%s ret:%d\n",__func__,__LINE__,cpu,cpu_state.active_cores,cpu,gmac_napi[cpu].available, call_from, ret);
			}
			if (ret)
				panic("Can't enable NAPI on CPU:%d.", cpu);
				spin_unlock_irqrestore(&cpu_state.lock, flags);
			goto out;
		}
	}
	spin_unlock_irqrestore(&cpu_state.lock, flags);
out:
	return;
}

//-------------------------------------------------------------------------------------------------
// Trigger one CPU to receive packet
//-------------------------------------------------------------------------------------------------
static void gmac_no_more_work(struct napi_struct *napi)
{
	int cur_active;
	unsigned long flags;
	struct gmac_napi_wrapper *gn = container_of(napi, struct gmac_napi_wrapper, napi_str);

	spin_lock_irqsave(&cpu_state.lock, flags);

	cpu_state.active_cores--;
	cur_active = cpu_state.active_cores;
	gn->available++;
	BUG_ON(gn->available != 1);


	if (!cur_active) {
	/*
	 * No more CPUs doing receive packet, enable interrupt so we
	 * can start to processing again.
	 */
	if (unlikely(gmac_debug_napi))
		printk("%s %d: active core:%d cpu:%d ENABLE IRQ\n",__func__,__LINE__,cur_active, gn->cpu);
		// Enable MAC interrupts //
		MDev_GMAC_irq_onoff(1, __func__);
	}
	spin_unlock_irqrestore(&cpu_state.lock, flags);

}
#endif

//-------------------------------------------------------------------------------------------------
// Enable/Disable IRQ
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_irq_onoff(int enable, const char *call_from)
{
    u32 uRegVal;

	if (enable) {
		// Enable MAC interrupts //
#ifndef GMAC_INT_JULIAN_D
        uRegVal = GMAC_INT_RCOM | GMAC_IER_FOR_INT_JULIAN_D;
        MHal_GMAC_Write_IER(uRegVal);
#else
        uRegVal = MHal_GMAC_Read_JULIAN_0104();
        uRegVal |= 0x00000080UL;
        MHal_GMAC_Write_JULIAN_0104(uRegVal);
        MHal_GMAC_Write_IER(GMAC_IER_FOR_INT_JULIAN_D);
#endif
	} else {
#ifndef GMAC_INT_JULIAN_D
		//Disable MAC interrupts //
		uRegVal = GMAC_INT_RCOM | GMAC_IER_FOR_INT_JULIAN_D;
		MHal_GMAC_Write_IDR(uRegVal);
#else
        uRegVal = MHal_GMAC_Read_JULIAN_0104();
        uRegVal &= ~(0x00000080UL);
        MHal_GMAC_Write_JULIAN_0104(uRegVal);
		MHal_GMAC_Write_IDR(GMAC_IER_FOR_INT_JULIAN_D);
#endif
	}
#if 0
            //Disable MAC interrupts //
            uRegVal = GMAC_INT_RCOM | GMAC_IER_FOR_INT_JULIAN_D;
            MHal_GMAC_Write_IDR(uRegVal);
            uRegVal = MHal_GMAC_Read_JULIAN_0104();
            uRegVal &= ~(0x00000080UL);
            MHal_GMAC_Write_JULIAN_0104(uRegVal);
#endif

}

//-------------------------------------------------------------------------------------------------
// Open the ethernet interface
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_open (struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 uRegVal;
    int ret;
#if defined(GMAC_TX_THROUGHPUT_TEST) || defined(NR_NAPI)
	int i;
#endif

#ifdef GMAC_NAPI
#ifdef NR_NAPI
	for_each_possible_cpu(i) {
		if (!gmac_napi[i].available) {
			napi_enable(&gmac_napi[i].napi_str);
			gmac_napi[i].available = 1;
			if (unlikely(gmac_debug))
				printk("%s %d:enable napi%d available:%dn",__func__,__LINE__,i,gmac_napi[i].available);
		}
	}
#else
	if (!netif_running(dev))
		napi_enable(&LocPtr->napi_str);
#endif
#endif

#ifdef CONFIG_GMAC_ISR_BH_NAPI
    napi_enable(&LocPtr->napi);
#endif /* CONFIG_GMAC_ISR_BH_NAPI */

    spin_lock_irq (LocPtr->lock);
    ret = MDev_GMAC_update_linkspeed(dev);
    spin_unlock_irq (LocPtr->lock);

    if (!is_valid_ether_addr (dev->dev_addr))
       return -EADDRNOTAVAIL;

#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
    _MDev_GMAC_tx_reset_TX_SW_QUEUE(dev);
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */

    //ato  GMAC_SYS->PMC_PCER = 1 << GMAC_ID_GMAC;   //Re-enable Peripheral clock //
    MHal_GMAC_Power_On_Clk();
    uRegVal = MHal_GMAC_Read_CTL();
    uRegVal |= GMAC_CSR;
    MHal_GMAC_Write_CTL(uRegVal);
    // Enable PHY interrupt //
    MHal_GMAC_enable_phyirq ();

    // Enable MAC interrupts //
	MDev_GMAC_irq_onoff(1, __func__);

    LocPtr->ep_flag |= GMAC_EP_FLAG_OPEND;

    MDev_GMAC_start (dev);
    netif_start_queue (dev);

    init_timer( &GMAC_Link_timer );
    GMAC_Link_timer.data = GMAC_LINK_TMR;
    GMAC_Link_timer.function = MDev_GMAC_timer_callback;
    GMAC_Link_timer.expires = jiffies + GMAC_CHECK_LINK_TIME;
    add_timer(&GMAC_Link_timer);

    /* check if network linked */
    if (-1 == ret)
    {
        netif_carrier_off(dev);
        GmacThisBCE.connected = 0;
    }
    else if(0 == ret)
    {
        netif_carrier_on(dev);
        GmacThisBCE.connected = 1;
    }
#if GMAC_TX_THROUGHPUT_TEST
	for (i = 0; i < gmac_tx_thread; i++)
	{
		tx_tsk[i] = kthread_create(tx_sender, NULL, "tx_sender");
		kthread_bind(tx_tsk[i], (i%CONFIG_NR_CPUS));
		if (IS_ERR(tx_tsk[i])) {
			printk("Can't create kthread of TX sender %d failed\n");
		} else {
			wake_up_process(tx_tsk[i]);
		}
	}
#endif
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Close the interface
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_close (struct net_device *dev)
{
    u32 uRegVal;
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);

#ifdef GMAC_NAPI
#ifdef NR_NAPI
	int i;
	for_each_possible_cpu(i) {
		//netif_napi_del(&gmac_napi[i].napi_str);
		napi_disable(&gmac_napi[i].napi_str);
		gmac_napi[i].available = 0;
	}
	cpu_state.active_cores = 0;
#else
	if (netif_running(dev))
		napi_disable(&LocPtr->napi_str);
#endif
#endif

#ifdef CONFIG_GMAC_ISR_BH_NAPI
    napi_disable(&LocPtr->napi);
#endif /* CONFIG_GMAC_ISR_BH_NAPI */

    //Disable Receiver and Transmitter //
    uRegVal = MHal_GMAC_Read_CTL();
    uRegVal &= ~(GMAC_TE | GMAC_RE);
    MHal_GMAC_Write_CTL(uRegVal);
    // Disable PHY interrupt //
    MHal_GMAC_disable_phyirq ();
    //Disable MAC interrupts //
	MDev_GMAC_irq_onoff(0, __func__);
    netif_stop_queue (dev);
    netif_carrier_off(dev);
    del_timer(&GMAC_Link_timer);
    //MHal_GMAC_Power_Off_Clk();
    GmacThisBCE.connected = 0;
    LocPtr->ep_flag &= (~GMAC_EP_FLAG_OPEND);

#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
    _MDev_GMAC_tx_reset_TX_SW_QUEUE(dev);
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Update the current statistics from the internal statistics registers.
//-------------------------------------------------------------------------------------------------
static struct net_device_stats * MDev_GMAC_stats (struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    int ale, lenerr, seqe, lcol, ecol;
    if (netif_running (dev))
    {
		unsigned long flags;
		spin_lock_irqsave(LocPtr->lock, flags);
		LocPtr->stats.rx_packets = received_num;
		spin_unlock_irqrestore(LocPtr->lock, flags);
        //LocPtr->stats.rx_packets += MHal_GMAC_Read_OK();            /* Good frames received */
        ale = MHal_GMAC_Read_ALE();
        LocPtr->stats.rx_frame_errors += ale;                       /* Alignment errors */
        lenerr = MHal_GMAC_Read_ELR();
        LocPtr->stats.rx_length_errors += lenerr;                   /* Excessive Length or Undersize Frame error */
        seqe = MHal_GMAC_Read_SEQE();
        LocPtr->stats.rx_crc_errors += seqe;                        /* CRC error */
        LocPtr->stats.rx_fifo_errors += MHal_GMAC_Read_ROVR();
        LocPtr->stats.rx_errors += ale + lenerr + seqe + MHal_GMAC_Read_SE() + MHal_GMAC_Read_RJB();
        LocPtr->stats.tx_packets += MHal_GMAC_Read_FRA();           /* Frames successfully transmitted */
        LocPtr->stats.tx_fifo_errors += MHal_GMAC_Read_TUE();       /* Transmit FIFO underruns */
        LocPtr->stats.tx_carrier_errors += MHal_GMAC_Read_CSE();    /* Carrier Sense errors */
        LocPtr->stats.tx_heartbeat_errors += MHal_GMAC_Read_SQEE(); /* Heartbeat error */
        lcol = MHal_GMAC_Read_LCOL();
        ecol = MHal_GMAC_Read_ECOL();
        LocPtr->stats.tx_window_errors += lcol;                     /* Late collisions */
        LocPtr->stats.tx_aborted_errors += ecol;                    /* 16 collisions */
        LocPtr->stats.collisions += MHal_GMAC_Read_SCOL() + MHal_GMAC_Read_MCOL() + lcol + ecol;
    }
    return &LocPtr->stats;
}

static int MDev_GMAC_TxRxReset(void)
{
    u32 val = MHal_GMAC_Read_CTL() & 0x00000000UL;

	val |= (GMAC_TE|GMAC_RE|GMAC_MPE);
    MHal_GMAC_Write_CTL(val);
    return 0;
}

static int MDev_GMAC_TxReset(void)
{
    u32 val = MHal_GMAC_Read_CTL() & 0x000001FFUL;

    MHal_GMAC_Write_CTL((val & ~GMAC_TE));
    MHal_GMAC_Write_TCR(0);
    MHal_GMAC_Write_CTL((MHal_GMAC_Read_CTL() | GMAC_TE));
    return 0;
}

static int MDev_GMAC_CheckTSR(void)
{
    u32 check;
    u32 tsrval = 0;

    #ifdef GMAC_TX_QUEUE_4
    u8  avlfifo[8] = {0};
    u8  avlfifoidx;
    u8  avlfifoval = 0;

    for (check = 0; check < GMAC_CHECK_CNT; check++)
    {
        tsrval = MHal_GMAC_Read_TSR();

        avlfifo[0] = ((tsrval & GMAC_IDLETSR) != 0)? 1 : 0;
        avlfifo[1] = ((tsrval & GMAC_BNQ)!= 0)? 1 : 0;
        avlfifo[2] = ((tsrval & GMAC_TBNQ) != 0)? 1 : 0;
        avlfifo[3] = ((tsrval & GMAC_FBNQ) != 0)? 1 : 0;
        avlfifo[4] = ((tsrval & GMAC_FIFO1IDLE) !=0)? 1 : 0;
        avlfifo[5] = ((tsrval & GMAC_FIFO2IDLE) != 0)? 1 : 0;
        avlfifo[6] = ((tsrval & GMAC_FIFO3IDLE) != 0)? 1 : 0;
        avlfifo[7] = ((tsrval & GMAC_FIFO4IDLE) != 0)? 1 : 0;

        avlfifoval = 0;

        for(avlfifoidx = 0; avlfifoidx < 8; avlfifoidx++)
        {
            avlfifoval += avlfifo[avlfifoidx];
        }

        if (avlfifoval > 4)
            return NETDEV_TX_OK;
    }
    #else
    for (check = 0; check < GMAC_CHECK_CNT; check++)
    {
        tsrval = MHal_GMAC_Read_TSR();

        // check GMAC_FIFO1IDLE is ok for gmac one queue
        if ((tsrval & GMAC_IDLETSR) && (tsrval & GMAC_FIFO1IDLE))
            return NETDEV_TX_OK;
    }
    #endif

    GMAC_DBG("Err CheckTSR:0x%x\n", tsrval);
    MDev_GMAC_TxReset();

    return NETDEV_TX_BUSY;
}

#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
int MDev_GMAC_GetTXFIFOIdle(void)
{
    u32 tsrval = 0;
    u8  avlfifo[8] = {0};
    u8  avlfifoidx;
    u8  avlfifoval = 0;

    tsrval = MHal_GMAC_Read_TSR();
    avlfifo[0] = ((tsrval & GMAC_IDLETSR) != 0)? 1 : 0;
    avlfifo[1] = ((tsrval & GMAC_BNQ)!= 0)? 1 : 0;
    avlfifo[2] = ((tsrval & GMAC_TBNQ) != 0)? 1 : 0;
    avlfifo[3] = ((tsrval & GMAC_FBNQ) != 0)? 1 : 0;
    avlfifo[4] = ((tsrval & GMAC_FIFO1IDLE) !=0)? 1 : 0;
    avlfifo[5] = ((tsrval & GMAC_FIFO2IDLE) != 0)? 1 : 0;
    avlfifo[6] = ((tsrval & GMAC_FIFO3IDLE) != 0)? 1 : 0;
    avlfifo[7] = ((tsrval & GMAC_FIFO4IDLE) != 0)? 1 : 0;

    avlfifoval = 0;
    for(avlfifoidx = 0; avlfifoidx < 8; avlfifoidx++)
    {
        avlfifoval += avlfifo[avlfifoidx];
    }

    if (avlfifoval > 4)
    {
        return avlfifoval - 4;
    }

    return 0;
}
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */

#if 0
static u8 pause_pkt[] =
{
    //DA - multicast
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x01,
    //SA
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //Len-Type
    0x88, 0x08,
    //Ctrl code
    0x00, 0x01,
    //Ctrl para 8192
    0x20, 0x00
};
#endif

#ifdef CONFIG_GMAC_NEW_TX_QUEUE
static dma_addr_t get_tx_addr(void)
{
    dma_addr_t addr;
    static int gmac_tx_idx = 0;

    addr = GMAC_TX_PTK_BASE + (GMAC_SOFTWARE_DESC_LEN * gmac_tx_idx);
    gmac_tx_idx++;
    gmac_tx_idx = gmac_tx_idx % CONFIG_GMAC_NEW_TX_QUEUE_THRESHOLD;

    return addr;
}
#else
static dma_addr_t get_tx_addr(void)
{
    dma_addr_t addr;

    addr = GMAC_TX_PTK_BASE + GMAC_SOFTWARE_DESC_LEN * gmac_txidx;
    gmac_txidx ++;
    gmac_txidx = gmac_txidx % GMAC_TX_RING_SIZE;
    return addr;
}
#endif /* CONFIG_GMAC_NEW_TX_QUEUE */

void MDrv_GMAC_DumpMem(phys_addr_t addr, u32 len)
{
    u8 *ptr = (u8 *)addr;
    u32 i;

    printk("\n ===== Dump %lx, len=%d =====\n", (long unsigned int)ptr, len);
    for (i=0; i<len; i++)
    {
        if ((u32)i%0x10UL ==0)
            printk("%lx: ", (long unsigned int)ptr);
        if (*ptr < 0x10UL)
            printk("0%x ", *ptr);
        else
            printk("%x ", *ptr);
        if ((u32)i%0x10UL == 0x0fUL)
            printk("\n");
    ptr++;
    }
    printk("\n");
}

#if 0
//Background send
// remove it due to directly MDev_GMAC_SwReset when RX overrun in GMAC due to h/w bug
static int MDev_GMAC_BGsend(struct net_device* dev, phys_addr_t addr, int len )
{
    dma_addr_t skb_addr;
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);

    if (NETDEV_TX_OK != MDev_GMAC_CheckTSR())
        return NETDEV_TX_BUSY;

    skb_addr = get_tx_addr();
    memcpy((void*)skb_addr,(void *)addr, len);

    LocPtr->stats.tx_bytes += len;

#ifdef CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
    if((unsigned int)skb_addr < 0xC0000000UL)
    {
        Chip_Flush_Memory_Range((unsigned int)skb_addr&0x0FFFFFFFUL, len);
    }
    else
    {
        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    }
    #elif defined(CONFIG_ARM)
        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    #else
        #ERROR
    #endif
#endif

    //Set address of the data in the Transmit Address register //
    MHal_GMAC_Write_TAR(skb_addr - GMAC_RAM_VA_PA_OFFSET - GMAC_MIU0_BUS_BASE);

    // Set length of the packet in the Transmit Control register //
    MHal_GMAC_Write_TCR(len);

    return NETDEV_TX_OK;
}

static void MDev_GMAC_Send_PausePkt(struct net_device* dev)
{
    u32 val = MHal_GMAC_Read_CTL() & 0x000001FFUL;

    //Disable Rx
    MHal_GMAC_Write_CTL((val & ~GMAC_RE));
    memcpy(&pause_pkt[6], dev->dev_addr, 6);
    MDev_GMAC_BGsend(dev, (u32)pause_pkt, sizeof(pause_pkt));
    //Enable Rx
    MHal_GMAC_Write_CTL((MHal_GMAC_Read_CTL() | GMAC_RE));
}
#endif

static void MDev_GMAC_Check_TXRX(int enable)
{
    u32 uRegVal = 0;
	u32 rRegVal = 0;
	rRegVal |= (GMAC_TE|GMAC_RE|GMAC_MPE);

	if (enable) {
    //Check if Receiver and Transmitter are enabled or not//
again:
		uRegVal = MHal_GMAC_Read_CTL();
		if (unlikely(gmac_debug || gmac_debug_napi))
			printk("%s %d: uRegVal:%lx rRegVal:%x\n",__func__,__LINE__,uRegVal,rRegVal);
		if (uRegVal != rRegVal)
		{
			if (unlikely(gmac_debug || gmac_debug_napi))
				printk("%s %d: TE:%lx RE:%lx\n",__func__,__LINE__,uRegVal&GMAC_TE,uRegVal&GMAC_RE);
			//MDev_GMAC_TxReset();
			MDev_GMAC_TxRxReset();
			goto again;
		}
	} else {
    //Check if Receiver and Transmitter are enabled or not//

	}
}

//-------------------------------------------------------------------------------------------------
//Transmit packet.
//-------------------------------------------------------------------------------------------------
#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
//  read skb from TX_SW_QUEUE to HW,
//  !!!! NO SPIN LOCK INSIDE !!!!
static void _MDev_GMAC_tx_read_TX_SW_QUEUE(int txIdleCount,struct net_device *dev,int intr)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);

    while(txIdleCount > 0){
        struct gmac_tx_ring *txq = &(LocPtr->tx_swq[LocPtr->tx_rdidx]);
        if(txq->used == TX_DESC_WROTE)
        {
            //Chip_Flush_Cache_Range((size_t)txq->skb->data,txq->skb->len);
            Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);

            MHal_GMAC_Write_TAR(txq->skb_physaddr  - MIU0_BUS_BASE );
            MHal_GMAC_Write_TCR(txq->skb->len);
            txq->used=TX_DESC_READ;
            LocPtr->tx_rdidx ++;
            if(TX_SW_QUEUE_SIZE==LocPtr->tx_rdidx)
            {
                LocPtr->tx_rdidx =0;
            }
        }else{
            break;
        }
        txIdleCount--;
    }
}

//  clear skb from TX_SW_QUEUE
//  !!!! NO SPIN LOCK INSIDE !!!!
static void _MDev_GMAC_tx_clear_TX_SW_QUEUE(int txIdleCount,struct net_device *dev,int intr)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    int clearcnt = 0;
    int fifoCount = 0;
    if(0 == txIdleCount) return;

    fifoCount = LocPtr->tx_rdidx - LocPtr->tx_clidx;
    if(fifoCount < 0) fifoCount = LocPtr->tx_rdidx + (TX_SW_QUEUE_SIZE - LocPtr->tx_clidx);

    /*
     * "fifoCount" is the count of the packets that has been sent to the GMAC HW.
     * "(TX_FIFO_SIZE-txIdleCount)" is the packet count that has not yet been sent
     * out completely by GMAC HW.
     */
    clearcnt = fifoCount - (TX_FIFO_SIZE - txIdleCount);
    if((clearcnt > TX_FIFO_SIZE) || (clearcnt < 0)){
        //printk(KERN_ERR "fifoCount in _MDev_GMAC_tx_clear_TX_SW_QUEUE() ERROR!! \
                 fifoCount=%d intr=%d, %d, %d, %d, %d\n", fifoCount, intr, \
                 LocPtr->tx_rdidx, LocPtr->tx_clidx, txIdleCount, TX_FIFO_SIZE);
    }

    while(clearcnt > 0)
    {
        struct gmac_tx_ring *txq=&(LocPtr->tx_swq[LocPtr->tx_clidx]);
        if(TX_DESC_READ==txq->used)
        {
            dma_unmap_single(&dev->dev, txq->skb_physaddr, txq->skb->len, DMA_TO_DEVICE);
            LocPtr->stats.tx_bytes += txq->skb->len;

            dev_kfree_skb_any(txq->skb);

            txq->used = TX_DESC_CLEARED;
            txq->skb=NULL;
            LocPtr->tx_clidx++;
            if(TX_SW_QUEUE_SIZE==LocPtr->tx_clidx)
            {
                LocPtr->tx_clidx =0;
            }
        } else {
            break;
        }
        clearcnt--;
    }
}

static void _MDev_GMAC_tx_reset_TX_SW_QUEUE(struct net_device* netdev)
{
    struct GMAC_private *LocPtr;
    u32 i=0;

    LocPtr = (struct GMAC_private*) netdev_priv(netdev);
    for (i=0;i<TX_SW_QUEUE_SIZE;i++)
    {
        if(LocPtr->tx_swq[i].skb != NULL)
        {
            dma_unmap_single(&netdev->dev, LocPtr->tx_swq[i].skb_physaddr
                    , LocPtr->tx_swq[i].skb->len, DMA_TO_DEVICE);
            dev_kfree_skb_any(LocPtr->tx_swq[i].skb);
        }
        LocPtr->tx_swq[i].skb = NULL;
        LocPtr->tx_swq[i].used = TX_DESC_CLEARED;
        LocPtr->tx_swq[i].skb_physaddr = 0;
    }
    LocPtr->tx_clidx = 0;
    LocPtr->tx_wridx = 0;
    LocPtr->tx_rdidx = 0;
}

static int MDev_GMAC_tx (struct sk_buff *skb, struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    unsigned long flags;
    dma_addr_t skb_addr;

    spin_lock_irqsave(LocPtr->txlock, flags);

    if (skb->len > GMAC_MTU)
    {
        GMAC_DBG("Wrong Tx len:%u\n", skb->len);
        spin_unlock_irqrestore(LocPtr->txlock, flags);
        return NETDEV_TX_BUSY;
    }
    {
        int txIdleCount=0;
        //FIFO full, loop until HW empty then try again
        //This is an abnormal condition as the upper network
        //tx_queue should already been stopped by "netif_stop_queue(dev)" in code below
        if( LocPtr->tx_swq[LocPtr->tx_wridx].used > TX_DESC_CLEARED)
        {
            //printk(KERN_ERR"ABNORMAL !! %d, %d, %d, %d\n",
            //       LocPtr->tx_wridx,LocPtr->tx_rdidx,
            //       LocPtr->tx_clidx, LocPtr->tx_swq[LocPtr->tx_wridx].used );
            netif_stop_queue(dev);
            //goto fifo_full;
            goto out_unlock;
            //BUG();
        }

#ifdef CONFIG_MSTAR_KANO
        /* workaround: data bytes at the end are wrong, after dma to GMAC */
        if ((skb->len > 1375) && (skb->len < 1392)) {
            skb->len = 1408;
        } else if ((skb->len > 863) && (skb->len < 880)) {
            skb->len = 896;
        } else if ((skb->len > 351) && (skb->len < 368)) {
            skb->len = 384;
        }
#endif /* CONFIG_MSTAR_KANO */

        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);

        //map skbuffer for DMA
        skb_addr = dma_map_single(&dev->dev, skb->data, skb->len, DMA_TO_DEVICE);

        if (dma_mapping_error(&dev->dev, skb_addr))
        {
            dev_kfree_skb_any(skb);
            printk(KERN_ERR"ERROR!![%s]%d\n",__FUNCTION__,__LINE__);
            dev->stats.tx_dropped++;

            goto out_unlock;
        }

        LocPtr->tx_swq[LocPtr->tx_wridx].skb = skb;
        LocPtr->tx_swq[LocPtr->tx_wridx].skb_physaddr= skb_addr;
        LocPtr->tx_swq[LocPtr->tx_wridx].used = TX_DESC_WROTE;
        LocPtr->tx_wridx ++;
        if(TX_SW_QUEUE_SIZE==LocPtr->tx_wridx)
        {
            LocPtr->tx_wridx=0;
        }

        //if FIFO is full, netif_stop_queue
        if( LocPtr->tx_swq[LocPtr->tx_wridx].used > TX_DESC_CLEARED)
        {
            netif_stop_queue(dev);
        }

fifo_full:
        // clear & read to HW FIFO
        txIdleCount=MDev_GMAC_GetTXFIFOIdle();

        _MDev_GMAC_tx_clear_TX_SW_QUEUE(txIdleCount,dev,GMAC_TX_SW_QUEUE_IN_GENERAL_TX);
        _MDev_GMAC_tx_read_TX_SW_QUEUE(txIdleCount,dev,GMAC_TX_SW_QUEUE_IN_GENERAL_TX);
    }

out_unlock:
    spin_unlock_irqrestore(LocPtr->txlock, flags);

    return NETDEV_TX_OK;
}

#else /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */

#ifdef CONFIG_GMAC_NEW_TX_QUEUE

static int MDev_GMAC_tx(struct sk_buff *skb, struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*)netdev_priv(dev);
    unsigned long flags;
    //static int tx_busy_cnt = 0;
    dma_addr_t skb_addr;

    spin_lock_irqsave(LocPtr->txlock, flags);
    if (skb->len > GMAC_MTU)
    {
        GMAC_DBG("Wrong Tx len:%u\n", skb->len);
        spin_unlock_irqrestore(LocPtr->txlock, flags);

        return NETDEV_TX_BUSY;
    }

    if (MHal_GMAC_New_TX_QUEUE_OVRN_Get() == 1)
    {
        //tx_busy_cnt++;
        //if (tx_busy_cnt > 100) {
        //    GMAC_DBG("TX queues are full, waited to xmit for %d times\n", tx_busy_cnt);
        //    tx_busy_cnt = 0;
        //}
        spin_unlock_irqrestore(LocPtr->txlock, flags);

        return NETDEV_TX_BUSY;
    }

    skb_addr = get_tx_addr();

    if (!skb_addr)
    {
        GMAC_DBG("Can not get memory from GMAC area\n");
        spin_unlock_irqrestore(LocPtr->txlock, flags);

        return -ENOMEM;
    }

    memcpy((void*)skb_addr, skb->data, skb->len);

    LocPtr->stats.tx_bytes += skb->len;

#if defined(CONFIG_MIPS)
    if((unsigned int)skb_addr < 0xC0000000UL)
    {
        Chip_Flush_Memory_Range((unsigned int)skb_addr&0x0FFFFFFFUL, skb->len);
    }
    else
    {
        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    }
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
#else
#ERROR "No Any ARCH Definition"
#endif

    MHal_GMAC_Write_TAR(skb_addr - GMAC_RAM_VA_PA_OFFSET - GMAC_MIU0_BUS_BASE);

    MHal_GMAC_Write_TCR(skb->len);

    dev->trans_start = jiffies;
    dev_kfree_skb_irq(skb);
    spin_unlock_irqrestore(LocPtr->txlock, flags);

    return NETDEV_TX_OK;
}

#else /* CONFIG_GMAC_NEW_TX_QUEUE */
static int busy_count;
static int MDev_GMAC_tx (struct sk_buff *skb, struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    unsigned long flags;
    dma_addr_t skb_addr;

    spin_lock_irqsave(LocPtr->txlock, flags);
    if (skb->len > GMAC_MTU)
    {
        GMAC_DBG("Wrong Tx len:%u\n", skb->len);
        spin_unlock_irqrestore(LocPtr->txlock, flags);
        return NETDEV_TX_BUSY;
    }

    if (NETDEV_TX_OK != MDev_GMAC_CheckTSR())
    {
#if 0
		busy_count ++;
		if (busy_count >= 3) {
			spin_lock_irqsave(LocPtr->lock, flags);
			MDev_GMAC_SwReset(dev);
			spin_unlock_irqrestore(LocPtr->lock, flags);
		} else {
			busy_count = 0;
		}
#endif
        spin_unlock_irqrestore(LocPtr->txlock, flags);
        return NETDEV_TX_BUSY; //check
    }

#ifndef GMAC_TX_SKB_PTR
    #ifndef GMAC_TX_QUEUE_4
        skb_addr = get_tx_addr();

        #ifdef GMAC_K3_SW_PATCH
        if(_OFFSET_CONDITION(skb->len))
            skb_addr+=0x40UL;
        #endif

        memcpy((void*)skb_addr, skb->data, skb->len);
    #else
        skb_addr = get_tx_addr();

        #ifdef GMAC_K3_SW_PATCH
        if(_OFFSET_CONDITION(skb->len))
            skb_addr+=0x40UL;
        #endif

        memcpy((void*)skb_addr, skb->data, skb->len);
    #endif
#else
    LocPtr->txpkt = dma_map_single(NULL, skb->data, skb->len,DMA_TO_DEVICE);
#endif

    if (!skb_addr)
    {
        dev_err(NULL,
                "dma map 2 failed (%p, %i). Dropping packet\n",
                skb->data, skb->len);
        spin_unlock_irqrestore(LocPtr->txlock, flags);
        return -ENOMEM;
    }

    // Store packet information (to free when Tx completed) //
    LocPtr->stats.tx_bytes += skb->len;

#ifdef GMAC_CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
    if((unsigned int)skb_addr < 0xC0000000UL)
    {
        Chip_Flush_Memory_Range((unsigned int)skb_addr&0x0FFFFFFFUL, skb->len);
    }
    else
    {
        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    }
    #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    #else
        #ERROR
    #endif
#endif
    //Moniter TX packet
    //MDrv_GMAC_DumpMem(skb_addr, skb->len);

    //Set address of the data in the Transmit Address register //
    MHal_GMAC_Write_TAR(skb_addr - GMAC_RAM_VA_PA_OFFSET - GMAC_MIU0_BUS_BASE);

    // Set length of the packet in the Transmit Control register //
    MHal_GMAC_Write_TCR(skb->len);

    //netif_stop_queue (dev);
    dev->trans_start = jiffies;
#if GMAC_TX_THROUGHPUT_TEST
	if (gmac_tx_test) {
		tx_bytes += skb->len;
		tx_count ++;
	}
	else {
		dev_kfree_skb_irq(skb);
	}
#else
    dev_kfree_skb_irq(skb);
#endif
    spin_unlock_irqrestore(LocPtr->txlock, flags);
    return NETDEV_TX_OK;
}

#endif /* CONFIG_GMAC_NEW_TX_QUEUE */
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */

#ifdef CONFIG_GMAC_ISR_BOTTOM_HALF
//-------------------------------------------------------------------------------------------------
// Extract received frame from buffer descriptors and sent to upper layers.
// (Called from interrupt context)
// (Disable RX software discriptor)
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_rx (struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    struct gmac_recv_desc_bufs *dlist;
    unsigned char *p_recv;
    u32 pktlen;
    u32 retval=0;
    u32 received=0;
    struct sk_buff *skb;
    u32 RBQP_offset;
    u32 RBQP_rx_skb_addr = 0;

    dlist = LocPtr->dlist ;

    do
    {
        if (LocPtr->rx_current_fill < 5)
        {
            GMAC_rx_fill_ring(gmac_dev);
        }

#if defined(CONFIG_MIPS)
        Chip_Read_Memory_Range((unsigned int)(&(dlist->descriptors[LocPtr->rxBuffIndex])) & 0x0FFFFFFFUL,
                               sizeof(dlist->descriptors[LocPtr->rxBuffIndex]));
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        //Chip_Inv_Cache_Range_VA_PA((unsigned long)(&(dlist->descriptors[LocPtr->rxBuffIndex])),
        //        (unsigned long)(&(dlist->descriptors[LocPtr->rxBuffIndex])) - GMAC_RAM_VA_PA_OFFSET,
        //        sizeof(dlist->descriptors[LocPtr->rxBuffIndex]));
        Chip_Flush_Cache_Range_VA_PA((unsigned long)(&(dlist->descriptors[LocPtr->rxBuffIndex])),
                (unsigned long)(&(dlist->descriptors[LocPtr->rxBuffIndex])) - GMAC_RAM_VA_PA_OFFSET,
                sizeof(dlist->descriptors[LocPtr->rxBuffIndex]));
#else
#ERROR "No Any ARCH Definition"
#endif /* CONFIG_MIPS CONFIG_ARM CONFIG_ARM64 */

        if (!((dlist->descriptors[LocPtr->rxBuffIndex].addr) & GMAC_DESC_DONE))
        {
            if (unlikely(gmac_debug))
                GMAC_DBG("CPU:%d rxBuffIndex:%d GMAC_DESC_DONE break\n",smp_processor_id(),LocPtr->rxBuffIndex);
            break;
        }

        p_recv = (char *)((((dlist->descriptors[LocPtr->rxBuffIndex].addr) & 0xFFFFFFFFUL) + \
                           GMAC_RAM_VA_PA_OFFSET + GMAC_MIU0_BUS_BASE) & \
                          ~(GMAC_DESC_DONE | GMAC_DESC_WRAP));
        /* Length of frame including FCS */
        pktlen = ((dlist->descriptors[LocPtr->rxBuffIndex].high_tag & 0x7) << 11) | \
                 (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & 0x7ffUL);

        if (pktlen < 64)
        {
            /* the pktlen */
            GMAC_DBG("Receive invalid packet with length=%d\n", pktlen);
            goto no_receive;
        }

        /* skip 4 CRC bytes at the end of data */
        pktlen -= 4;

	if (pktlen > GMAC_SOFTWARE_DESC_LEN)
	{
            GMAC_DBG("Receive invalid packet with length=%d\n", pktlen);
            GMAC_DBG("pktlen:%d > GMAC_SOFTWARE_DESC_LEN:%d\n",pktlen,GMAC_SOFTWARE_DESC_LEN);
            goto no_receive;
	}

        skb_put(rx_skb[LocPtr->rxBuffIndex], pktlen);
        rx_skb[LocPtr->rxBuffIndex]->dev = dev;
        rx_skb[LocPtr->rxBuffIndex]->protocol = eth_type_trans(rx_skb[LocPtr->rxBuffIndex], dev);
        dev->last_rx = jiffies;
        LocPtr->stats.rx_bytes += pktlen;

        if(((dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_TCP ) || \
            (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_UDP )) && \
           (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_IP_CSUM) && \
           (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_TCP_UDP_CSUM))
        {
            rx_skb[LocPtr->rxBuffIndex]->ip_summed = CHECKSUM_UNNECESSARY;
        }
        else
        {
            rx_skb[LocPtr->rxBuffIndex]->ip_summed = CHECKSUM_NONE;
        }

#ifdef CONFIG_GMAC_ISR_BH_NAPI
#ifdef CONFIG_GMAC_NAPI_GRO
        retval = napi_gro_receive(&LocPtr->napi, rx_skb[LocPtr->rxBuffIndex]);
#else
        retval = netif_receive_skb(rx_skb[LocPtr->rxBuffIndex]);
#endif /* CONFIG_GMAC_NAPI_GRO */
#else
        retval = netif_rx_ni(rx_skb[LocPtr->rxBuffIndex]);
#endif /* CONFIG_GMAC_ISR_BH_NAPI */

        received++;
        received_num += received;

no_receive:

        GMAC_dequeue_rx_buffer(LocPtr, &skb);
        if (!skb)
        {
            GMAC_DBG("%s[%d]: rx_next:%d rx_next_fill:%d rx_current_fill:%d\n",
                     __func__, __LINE__, LocPtr->rx_next, LocPtr->rx_next_fill, LocPtr->rx_current_fill);
            panic("Can't dequeue skb from buffer.");
        }
        rx_skb[LocPtr->rxBuffIndex] = skb;
        rx_abso_addr[LocPtr->rxBuffIndex] = rx_skb[LocPtr->rxBuffIndex]->data;

        RBQP_offset = LocPtr->rxBuffIndex * 16;
        if(LocPtr->rxBuffIndex < (GMAC_MAX_RX_DESCR-1))
        {
            RBQP_rx_skb_addr = __virt_to_phys(rx_abso_addr[LocPtr->rxBuffIndex]) - GMAC_MIU0_BUS_BASE;
            RBQP_rx_skb_addr |= GMAC_DESC_DONE;
            MHal_GMAC_WritRam32(GMAC_RAM_VA_PA_OFFSET, GMAC_RBQP_BASE + RBQP_offset, RBQP_rx_skb_addr);
        }
        else
        {
            RBQP_rx_skb_addr = __virt_to_phys(rx_abso_addr[LocPtr->rxBuffIndex]) - \
                               GMAC_MIU0_BUS_BASE + GMAC_DESC_WRAP;
            RBQP_rx_skb_addr |= GMAC_DESC_DONE;
            MHal_GMAC_WritRam32(GMAC_RAM_VA_PA_OFFSET, GMAC_RBQP_BASE + RBQP_offset, RBQP_rx_skb_addr);
        }

        if (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_MULTICAST)
        {
            LocPtr->stats.multicast++;
        }

        /* reset ownership bit */
        dlist->descriptors[LocPtr->rxBuffIndex].addr &= ~GMAC_DESC_DONE;

#if defined(CONFIG_MIPS)
        Chip_Flush_Memory_Range((unsigned int)(&(dlist->descriptors[LocPtr->rxBuffIndex].addr)) & 0x0FFFFFFFUL,
                                sizeof(dlist->descriptors[LocPtr->rxBuffIndex].addr));
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        Chip_Flush_Cache_Range_VA_PA((unsigned long)(&(dlist->descriptors[LocPtr->rxBuffIndex])),
                (unsigned long)(&(dlist->descriptors[LocPtr->rxBuffIndex])) - GMAC_RAM_VA_PA_OFFSET,
                sizeof(dlist->descriptors[LocPtr->rxBuffIndex]));
#else
#ERROR "No Any ARCH Definition"
#endif

        LocPtr->rxBuffIndex++;
        if (LocPtr->rxBuffIndex == GMAC_MAX_RX_DESCR)
        {
            LocPtr->rxBuffIndex = 0;
        }

#ifdef CONFIG_GMAC_ISR_BH_NAPI
        if(received >= CONFIG_GMAC_NAPI_WEIGHT)
        {
            break;
        }
#endif /* CONFIG_GMAC_ISR_BH_NAPI */

    } while (1);

    GmacxReceiveNum=0;
    GmacThisUVE.flagRBNA=0;

    GMAC_rx_fill_ring(gmac_dev);

    return received;
}

#else /* CONFIG_GMAC_ISR_BOTTOM_HALF */

static int MDev_GMAC_rx (struct net_device *dev, int budget)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    struct gmac_recv_desc_bufs *dlist;
    unsigned char *p_recv;
    u32 pktlen;
    u32 retval=0;
    u32 received=0;
    struct sk_buff *skb;
#ifdef GMAC_RX_ZERO_COPY
    u32 uRegVal = 0;
    u32 RBQP_offset;
    u32 RBQP_rx_skb_addr = 0;
#endif
#ifndef GMAC_INT_JULIAN_D
    u32 uRegVal=0;
    int count = 0;
#endif
    unsigned long flags;

#ifdef NR_NAPI
	if (received == 0) {
		u32 packets_in_hw;
		int in_use = cpu_state.active_cores;
		//In the first time, we check received packet of HW, then decide how many core we must trigger for this moment.
        packets_in_hw = MHal_GMAC_Read_OK();/* Good frames received */
		if (packets_in_hw > (budget * in_use)) {
			if (unlikely(gmac_debug_napi))
				GMAC_DBG("cpu:%d packets in hw:%u. (budget * in_use)=%d. We call one more core\n",smp_processor_id(),packets_in_hw,(budget * in_use));
			gmac_enable_one_cpu(__func__);
		}

	}
#endif

    dlist = LocPtr->dlist ;
    // If any Ownership bit is 1, frame received.
    //while ( (dlist->descriptors[LocPtr->rxBuffIndex].addr )& GMAC_DESC_DONE)
    do
    {
    spin_lock_irqsave(LocPtr->lock, flags);
#ifdef GMAC_CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
        Chip_Read_Memory_Range((unsigned int)(&(dlist->descriptors[LocPtr->rxBuffIndex])) & 0x0FFFFFFFUL, sizeof(dlist->descriptors[LocPtr->rxBuffIndex]));
    #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        Chip_Inv_Cache_Range_VA_PA((unsigned long)(&(dlist->descriptors[LocPtr->rxBuffIndex])),
            (unsigned long)(&(dlist->descriptors[LocPtr->rxBuffIndex])) - GMAC_RAM_VA_PA_OFFSET ,sizeof(dlist->descriptors[LocPtr->rxBuffIndex]));
    #else
        #ERROR
    #endif
#endif
        if(!((dlist->descriptors[LocPtr->rxBuffIndex].addr) & GMAC_DESC_DONE))
        {
			if (unlikely(gmac_debug))
				GMAC_DBG("CPU:%d rxBuffIndex:%d GMAC_DESC_DONE break\n",smp_processor_id(),LocPtr->rxBuffIndex);
			spin_unlock_irqrestore(LocPtr->lock, flags);
                break;
        }

		p_recv = (char *) ((((dlist->descriptors[LocPtr->rxBuffIndex].addr) & 0xFFFFFFFFUL) + GMAC_RAM_VA_PA_OFFSET + GMAC_MIU0_BUS_BASE) & ~(GMAC_DESC_DONE | GMAC_DESC_WRAP));
        pktlen = ((dlist->descriptors[LocPtr->rxBuffIndex].high_tag & 0x7) << 11) | (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & 0x7ffUL);    /* Length of frame including FCS */

    #if GMAC_RX_THROUGHPUT_TEST
        gmac_receive_bytes += pktlen;
	#if GMAC_RX_THROUGHPUT_TEST_ON_RECEIVE
		kfree_skb(skb);
		goto no_receive;
	#endif
	#endif

#ifdef GMAC_RX_ZERO_COPY
	if (pktlen > GMAC_SOFTWARE_DESC_LEN)
	{
		if (unlikely(gmac_debug))
			GMAC_DBG("pktlen:%d > GMAC_SOFTWARE_DESC_LEN:%d\n",pktlen,GMAC_SOFTWARE_DESC_LEN);
#endif
        skb = alloc_skb (pktlen + 6, GFP_ATOMIC);

        if (skb != NULL)
        {
            skb_reserve (skb, 2);
    #ifdef GMAC_CHIP_FLUSH_READ
        #if defined(CONFIG_MIPS)
            if((unsigned int)p_recv < 0xC0000000UL)
            {
                Chip_Read_Memory_Range((unsigned int)(p_recv) & 0x0FFFFFFFUL, pktlen);
            }
            else
            {
                Chip_Read_Memory_Range(0, 0xFFFFFFFFUL);
            }
        #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        Chip_Inv_Cache_Range_VA_PA((size_t)p_recv,(size_t)p_recv - GMAC_RAM_VA_PA_OFFSET ,pktlen);
        #else
            #ERROR
        #endif
    #endif
            memcpy(skb_put(skb, pktlen), p_recv, pktlen);
            skb->dev = dev;
            skb->protocol = eth_type_trans (skb, dev);
            dev->last_rx = jiffies;
            LocPtr->stats.rx_bytes += pktlen;

        #ifdef GMAC_RX_CHECKSUM
            if(((dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_TCP ) || (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_UDP )) && \
               (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_IP_CSUM) && \
               (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_TCP_UDP_CSUM) )
            {
                skb->ip_summed = CHECKSUM_UNNECESSARY;
            }
            else
            {
                skb->ip_summed = CHECKSUM_NONE;
            }
        #endif

        }
        else
        {
            LocPtr->stats.rx_dropped += 1;
        }
#ifdef GMAC_RX_ZERO_COPY
	}
	else
	{
        skb_put(rx_skb[LocPtr->rxBuffIndex], pktlen);
        // update consumer pointer//
        rx_skb[LocPtr->rxBuffIndex]->dev = dev;
        rx_skb[LocPtr->rxBuffIndex]->protocol = eth_type_trans (rx_skb[LocPtr->rxBuffIndex], dev);
        dev->last_rx = jiffies;
        LocPtr->stats.rx_bytes += pktlen;
    #ifdef GMAC_RX_CHECKSUM
        if(((dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_TCP ) || (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_UDP )) \
            && (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_IP_CSUM) \
            && (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_DESC_TCP_UDP_CSUM) )
        {
            rx_skb[LocPtr->rxBuffIndex]->ip_summed = CHECKSUM_UNNECESSARY;
        }
        else
        {
            rx_skb[LocPtr->rxBuffIndex]->ip_summed = CHECKSUM_NONE;
        }
    #endif
	}
#endif /*GMAC_RX_ZERO_COPY*/

		#ifdef GMAC_NAPI
		#ifdef GMAC_RX_ZERO_COPY
			retval = netif_receive_skb(rx_skb[LocPtr->rxBuffIndex]);
			if (gmac_dump_skb)
				dump_skb(rx_skb[LocPtr->rxBuffIndex]);
		#else
            retval = netif_receive_skb(skb);
			if (gmac_dump_skb)
				dump_skb(skb);
		#endif
        #else
		#ifdef GMAC_RX_ZERO_COPY
			retval = netif_rx (rx_skb[LocPtr->rxBuffIndex]);
		#else
            retval = netif_rx(skb);
		#endif
        #endif

        received++;
		received_num += received;
no_receive:

#ifdef GMAC_RX_ZERO_COPY
		GMAC_dequeue_rx_buffer(LocPtr, &skb);
		if (!skb) {
			GMAC_DBG("%d: rx_next:%d rx_next_fill:%d rx_current_fill:%d\n",__LINE__,LocPtr->rx_next, LocPtr->rx_next_fill, LocPtr->rx_current_fill);
			panic("Can't dequeue skb from buffer.");
		}
        rx_skb[LocPtr->rxBuffIndex] = skb;
        rx_abso_addr[LocPtr->rxBuffIndex] = rx_skb[LocPtr->rxBuffIndex]->data;

		RBQP_offset = LocPtr->rxBuffIndex * 16;
		if(LocPtr->rxBuffIndex<(GMAC_MAX_RX_DESCR-1))
        {
			RBQP_rx_skb_addr = __virt_to_phys(rx_abso_addr[LocPtr->rxBuffIndex]) - GMAC_MIU0_BUS_BASE;
			RBQP_rx_skb_addr |= GMAC_DESC_DONE;
            MHal_GMAC_WritRam32(GMAC_RAM_VA_PA_OFFSET, GMAC_RBQP_BASE + RBQP_offset, RBQP_rx_skb_addr);
        }
        else
        {
			RBQP_rx_skb_addr = __virt_to_phys(rx_abso_addr[LocPtr->rxBuffIndex]) - GMAC_MIU0_BUS_BASE + GMAC_DESC_WRAP;
			RBQP_rx_skb_addr |= GMAC_DESC_DONE;
            MHal_GMAC_WritRam32(GMAC_RAM_VA_PA_OFFSET, GMAC_RBQP_BASE + RBQP_offset, RBQP_rx_skb_addr);
        }
#endif

        if (dlist->descriptors[LocPtr->rxBuffIndex].low_tag & GMAC_MULTICAST)
        {
            LocPtr->stats.multicast++;
        }
        dlist->descriptors[LocPtr->rxBuffIndex].addr  &= ~GMAC_DESC_DONE;  /* reset ownership bit */
#ifdef GMAC_CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
        Chip_Flush_Memory_Range((unsigned int)(&(dlist->descriptors[LocPtr->rxBuffIndex].addr)) & 0x0FFFFFFFUL, sizeof(dlist->descriptors[LocPtr->rxBuffIndex].addr));
    #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        Chip_Flush_Cache_Range_VA_PA((unsigned long)(&(dlist->descriptors[LocPtr->rxBuffIndex])),
        (unsigned long)(&(dlist->descriptors[LocPtr->rxBuffIndex])) - GMAC_RAM_VA_PA_OFFSET ,sizeof(dlist->descriptors[LocPtr->rxBuffIndex]));
    #else
        #ERROR
    #endif
#endif

        //wrap after last buffer //
        LocPtr->rxBuffIndex++;
        if (LocPtr->rxBuffIndex == GMAC_MAX_RX_DESCR)
        {
            LocPtr->rxBuffIndex = 0;
        }
		spin_unlock_irqrestore(LocPtr->lock, flags);

    #ifdef CONFIG_GMAC_SUPPLY_RNG
        {
            static unsigned long u32LastInputRNGJiff=0;
            unsigned long u32Jiff=jiffies;

            if ( time_after(u32Jiff, u32LastInputRNGJiff+InputRNGJiffThreshold) )
            {
                unsigned int u32Temp;
                unsigned short u16Temp;

                u32LastInputRNGJiff = u32Jiff;
                u16Temp = MIPS_REG(REG_RNG_OUT);
                memcpy((unsigned char *)&u32Temp+0, &u16Temp, 2);
                u16Temp = MIPS_REG(REG_RNG_OUT);
                memcpy((unsigned char *)&u32Temp+2, &u16Temp, 2);
                add_input_randomness(EV_MSC, MSC_SCAN, u32Temp);
            }
        }
    #endif
#ifdef GMAC_NAPI
    }while(received < budget);
#else
    }while(1);
#endif
#ifdef GMAC_INT_JULIAN_D
    GmacxReceiveNum=0;
    GmacThisUVE.flagRBNA=0;
#endif
#ifdef GMAC_RX_ZERO_COPY
    spin_lock_irqsave(LocPtr->lock, flags);
	GMAC_rx_fill_ring(gmac_dev);
    spin_unlock_irqrestore(LocPtr->lock, flags);
#endif

	return received;
}

#endif /* CONFIG_GMAC_ISR_BOTTOM_HALF */

#ifdef CONFIG_GMAC_ISR_BOTTOM_HALF
#ifdef CONFIG_GMAC_ISR_BH_TASKLET
void MDev_GMAC_bottom_rx_task(unsigned long data)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(gmac_dev);
    unsigned long flags;
    u32 uRegVal;

    spin_lock_irqsave(LocPtr->lock_rx, flags);
    uRegVal = MHal_GMAC_Read_JULIAN_0104();
    uRegVal &= ~(0x00000080UL);
    MHal_GMAC_Write_JULIAN_0104(uRegVal);
    MDev_GMAC_rx(gmac_dev);
    uRegVal = MHal_GMAC_Read_JULIAN_0104();
    uRegVal |= 0x00000080UL;
    MHal_GMAC_Write_JULIAN_0104(uRegVal);
    spin_unlock_irqrestore(LocPtr->lock_rx, flags);
}

void MDev_GMAC_bottom_rst_task(unsigned long data)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(gmac_dev);
    unsigned long flags;

    spin_lock_irqsave(LocPtr->lock_rx, flags);
    spin_lock_irqsave(LocPtr->txlock, flags);
    LocPtr->stats.rx_over_errors++;
    MHal_GMAC_Write_RSR(GMAC_RSROVR);
    MDev_GMAC_SwReset(gmac_dev);
    spin_unlock_irqrestore(LocPtr->txlock, flags);
    spin_unlock_irqrestore(LocPtr->lock_rx, flags);
}

#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
void MDev_GMAC_bottom_tx_task(unsigned long data)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(gmac_dev);
    int txIdleCount = 0;
    unsigned long flags;
    u32 uRegVal;

    spin_lock_irqsave(LocPtr->txlock, flags);
    uRegVal = GMAC_INT_TCOM;
    MHal_GMAC_Write_IDR(uRegVal);
    txIdleCount=MDev_GMAC_GetTXFIFOIdle();

    while(txIdleCount>0 && (LocPtr->tx_rdidx != LocPtr->tx_wridx))
    {
        _MDev_GMAC_tx_clear_TX_SW_QUEUE(txIdleCount,gmac_dev,GMAC_TX_SW_QUEUE_IN_IRQ);
        _MDev_GMAC_tx_read_TX_SW_QUEUE(txIdleCount,gmac_dev,GMAC_TX_SW_QUEUE_IN_IRQ);
        txIdleCount=MDev_GMAC_GetTXFIFOIdle();
    }
    uRegVal = GMAC_INT_TCOM | GMAC_IER_FOR_INT_JULIAN_D;
    MHal_GMAC_Write_IER(uRegVal);
    txIdleCount=MDev_GMAC_GetTXFIFOIdle();

    if (txIdleCount > 0) {
        if (netif_queue_stopped(gmac_dev))
            netif_wake_queue(gmac_dev);
    }
    spin_unlock_irqrestore(LocPtr->txlock, flags);
}
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */
#endif /* CONFIG_GMAC_ISR_BH_TASKLET */
#endif /* CONFIG_GMAC_ISR_BOTTOM_HALF */

#ifdef GMAC_INT_JULIAN_D

#ifdef CONFIG_GMAC_ISR_BOTTOM_HALF
irqreturn_t MDev_GMAC_interrupt(int irq,void *dev_id)
{
    struct net_device *dev = (struct net_device *) dev_id;
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 intstatus=0;
    u32 xReceiveFlag=0;
    unsigned long flags;
#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
    int txIdleCount = 0;
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */

    spin_lock_irqsave(LocPtr->lock, flags);
    /* MAC Interrupt Status register indicates what interrupts are pending. */
    /* It is automatically cleared once read. */
    GmacxoffsetValue = MHal_GMAC_Read_JULIAN_0108() & 0x0000FFFFUL;

    if(GmacxoffsetValue&0x8000UL)
    {
        xReceiveFlag = 1;
    }
    intstatus = MHal_GMAC_Read_ISR() & ~MHal_GMAC_Read_IMR() & GMAC_INT_MASK;
    {
        if (intstatus & GMAC_INT_RBNA)
        {
            LocPtr->stats.rx_dropped ++;
            xReceiveFlag = 1;
            /* write 1 clear */
            MHal_GMAC_Write_RSR(GMAC_BNA);
        }

        /* RX Overrun */
        if(intstatus & GMAC_INT_ROVR)
        {
/* Reset should do rapidly, if ROVR. So, do not push it to BH. */
#if 0//def CONFIG_GMAC_ISR_BH_TASKLET
            tasklet_schedule(&LocPtr->rst_task);
#else
            LocPtr->stats.rx_dropped++;

            /* write 1 clear RX overrun */
            MHal_GMAC_Write_RSR(GMAC_RSROVR);
            MDev_GMAC_SwReset(dev);
#endif /* CONFIG_GMAC_ISR_BH_TASKLET */
        }

        /* Receive complete */
        if(xReceiveFlag == 1)
        {
            xReceiveFlag = 0;
#ifdef CONFIG_GMAC_ISR_BH_TASKLET
            tasklet_hi_schedule(&LocPtr->rx_task);
#elif CONFIG_GMAC_ISR_BH_NAPI
            if (napi_schedule_prep(&LocPtr->napi))
            {
                MDev_GMAC_disable_INT_RX();
                __napi_schedule(&LocPtr->napi);
            }
#else
            spin_unlock_irqrestore(LocPtr->lock, flags);
            MDev_GMAC_rx(dev);
            spin_lock_irqsave(LocPtr->lock, flags);
#endif /* CONFIG_GMAC_ISR_BH_TASKLET */
        }

#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
        // Transmit complete //
        if (intstatus & GMAC_INT_TCOM)
        {
#ifdef CONFIG_GMAC_ISR_BH_TASKLET
            tasklet_schedule(&LocPtr->tx_task);
#else
            spin_lock_irqsave(LocPtr->txlock, flags);
            if( ((LocPtr->ep_flag&GMAC_EP_FLAG_SUSPENDING)==0) && netif_queue_stopped (dev))
            {
                netif_wake_queue(dev);
            }

            txIdleCount = MDev_GMAC_GetTXFIFOIdle();
            while(txIdleCount > 0 && (LocPtr->tx_rdidx != LocPtr->tx_wridx))
            {
                _MDev_GMAC_tx_clear_TX_SW_QUEUE(txIdleCount,dev,GMAC_TX_SW_QUEUE_IN_IRQ);
                _MDev_GMAC_tx_read_TX_SW_QUEUE(txIdleCount,dev,GMAC_TX_SW_QUEUE_IN_IRQ);
                txIdleCount = MDev_GMAC_GetTXFIFOIdle();
            }
            spin_unlock_irqrestore(LocPtr->txlock, flags);
#endif /* CONFIG_GMAC_ISR_BH_TASKLET */
        }
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */
    }
    spin_unlock_irqrestore(LocPtr->lock, flags);

    return IRQ_HANDLED;
}

#else /* CONFIG_GMAC_ISR_BOTTOM_HALF */

irqreturn_t MDev_GMAC_interrupt(int irq,void *dev_id)
{
    struct net_device *dev = (struct net_device *) dev_id;
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 intstatus=0;
    u32 xReceiveFlag=0;
    unsigned long flags;
    u32 uRegVal;

#ifndef GMAC_RX_ZERO_COPY
    u32 wp = 0;
#endif

    spin_lock_irqsave(LocPtr->lock, flags);
    //MAC Interrupt Status register indicates what interrupts are pending.
    //It is automatically cleared once read.
    GmacxoffsetValue = MHal_GMAC_Read_JULIAN_0108() & 0x0000FFFFUL;

#ifndef GMAC_RX_ZERO_COPY
    wp = MHal_GMAC_Read_JULIAN_0100() & 0x00100000UL;
    if(wp)
    {
        GMAC_DBG("GMAC HW write invalid address");
    }
#endif

    if(GmacxoffsetValue&0x8000UL)
    {
        xReceiveFlag = 1;
    }

	if (unlikely(gmac_debug || gmac_debug_napi))
		GMAC_DBG("xReceiveFlag:%d\n",xReceiveFlag);

    GmacThisUVE.flagRBNA = 0;


    gmac_oldTime = getCurMs();
    while((xReceiveFlag == 1) || (intstatus = (MHal_GMAC_Read_ISR() & ~MHal_GMAC_Read_IMR() & GMAC_INT_MASK )) )
    {
		if (unlikely(gmac_debug || gmac_debug_napi))
			GMAC_DBG("intstatus:%x\n",intstatus);
        if (((LocPtr->ep_flag&GMAC_EP_FLAG_SUSPENDING)==0) && netif_queue_stopped (dev))
        {
            netif_wake_queue(dev);
        }

        if (intstatus & GMAC_INT_RBNA)
        {
            LocPtr->stats.rx_dropped ++;
            GmacThisUVE.flagRBNA = 1;
            xReceiveFlag = 1;
            //write 1 clear
            MHal_GMAC_Write_RSR(GMAC_BNA);
			if (unlikely(gmac_debug || gmac_debug_napi))
				GMAC_DBG("GMAC_INT_RBNA xReceiveFlag:%d\n",xReceiveFlag);
        }

        // The TCOM bit is set even if the transmission failed. //
        if (intstatus & (GMAC_INT_TUND | GMAC_INT_RTRY))
        {
            LocPtr->stats.tx_errors += 1;
			if (unlikely(gmac_debug || gmac_debug_napi))
				GMAC_DBG("intstatus & GMAC_INT_TUND | GMAC_INT_RTRY\n");

            if(intstatus & GMAC_INT_TUND)
            {
                //write 1 clear
                MHal_GMAC_Write_TSR(GMAC_UND);

                //Reset TX engine
                MDev_GMAC_TxReset();
                GMAC_DBG ("Transmit TUND error, TX reset\n");
            }
        }

        if(intstatus&GMAC_INT_DONE)
        {
            GmacThisUVE.flagISR_INT_DONE = 0x01UL;
        }

        //RX Overrun //
        if(intstatus & GMAC_INT_ROVR)
        {
            LocPtr->stats.rx_dropped++;
            ROVRcount++;
			if (unlikely(gmac_debug || gmac_debug_napi))
				GMAC_DBG("intstatus & GMAC_INT_ROVR ROVRcount:%d\n",ROVRcount);

            //write 1 clear RX overrun
            MHal_GMAC_Write_RSR(GMAC_RSROVR);

            if (ROVRcount >= 1)
            {	//If ROVRcount happens 6 times, we had better do SwReset
                MDev_GMAC_SwReset(dev);
				if (unlikely(gmac_debug || gmac_debug_napi))
					GMAC_DBG("intstatus & GMAC_INT_ROVR ROVRcount:%d SWReset\n",ROVRcount);
            }
        }
        else
        {
            ROVRcount = 0;
        }

        // Receive complete //
        if(xReceiveFlag == 1)
        {
            xReceiveFlag = 0;
        #ifdef GMAC_NAPI
#if 0
			MDev_GMAC_irq_onoff(0, __func__);
#else
            //Disable MAC interrupts //
            uRegVal = GMAC_INT_RCOM | GMAC_IER_FOR_INT_JULIAN_D;
            MHal_GMAC_Write_IDR(uRegVal);
            uRegVal = MHal_GMAC_Read_JULIAN_0104();
            uRegVal &= ~(0x00000080UL);
            MHal_GMAC_Write_JULIAN_0104(uRegVal);
#endif

            /* Receive packets are processed by poll routine. If not running start it now. */
			#ifdef NR_NAPI
			gmac_enable_one_cpu(__func__);
			//gmac_enable_napi();
			#else
            napi_schedule(&LocPtr->napi_str);
			#endif
        #else
			spin_unlock_irqrestore(LocPtr->lock, flags);
            MDev_GMAC_rx(dev, GMAC_NAPI_WEIGHT);
			spin_lock_irqsave(LocPtr->lock, flags);
        #endif
        }
    }
    spin_unlock_irqrestore(LocPtr->lock, flags);
    return IRQ_HANDLED;
}
#endif /* CONFIG_GMAC_ISR_BOTTOM_HALF */
#endif //#ifdef GMAC_INT_JULIAN_D

#ifdef CONFIG_GMAC_ISR_BOTTOM_HALF
#ifdef CONFIG_GMAC_ISR_BH_NAPI
static void MDev_GMAC_enable_INT_RX(void) {
    u32 uRegVal;

    /* enable delay interrupt */
    uRegVal = MHal_GMAC_Read_JULIAN_0104();
    uRegVal |= 0x00000080UL;
    MHal_GMAC_Write_JULIAN_0104(uRegVal);
}

static void MDev_GMAC_disable_INT_RX(void) {
    u32 uRegVal;

    /* disable delay interrupt */
    uRegVal = MHal_GMAC_Read_JULIAN_0104();
    uRegVal &= ~(0x00000080UL);
    MHal_GMAC_Write_JULIAN_0104(uRegVal);
}

static int MDev_GMAC_napi_poll(struct napi_struct *napi, int budget)
{
    struct GMAC_private *LocPtr = container_of(napi, struct GMAC_private, napi);
    struct net_device *dev = LocPtr->dev;
    unsigned long flags = 0;
    int work_done = 0;

    work_done = MDev_GMAC_rx(dev);

    /* If budget not fully consumed, exit the polling mode */
    if (work_done < budget) {
        napi_complete(napi);
        // enable MAC interrupt
        spin_lock_irqsave(LocPtr->lock, flags);
        MDev_GMAC_enable_INT_RX();
        spin_unlock_irqrestore(LocPtr->lock, flags);
    }

    return work_done;
}
#endif /* CONFIG_GMAC_ISR_BH_NAPI */
#else /* CONFIG_GMAC_ISR_BOTTOM_HALF */

static int MDev_GMAC_napi_poll(struct napi_struct *napi, int budget)
{
#ifdef NR_NAPI
    struct net_device *dev = napi->dev;
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
#else
    struct GMAC_private *LocPtr = container_of(napi, struct GMAC_private, napi_str);
    struct net_device *dev = LocPtr->dev;
#endif
    unsigned long flags;
    int work_done = 0;
    u32 uRegVal;

    work_done = MDev_GMAC_rx(dev, budget);

	//printk("work_done = %d, budget = %d\n",work_done, budget);
    if (work_done < budget)
    {
        /*
        * Order is important since data can get interrupted
        * again when we think we are done.
        */
        __napi_complete(napi);

#ifdef NR_NAPI
		gmac_no_more_work(napi);
#else
        // Enable MAC interrupts //
		MDev_GMAC_irq_onoff(1, __func__);
#endif
    }

    return work_done;
}

#endif /* CONFIG_GMAC_ISR_BOTTOM_HALF */

//-------------------------------------------------------------------------------------------------
// GMAC Hardware register set
//-------------------------------------------------------------------------------------------------
void MDev_GMAC_HW_init(void)
{
    u32 word_ETH_CTL = 0x00000000UL;
    u32 word_ETH_CFG = 0x00000800UL;
    u32 uJulian104Value = 0;
    u32 uNegPhyVal = 0;
#ifdef GMAC_SOFTWARE_DESCRIPTOR
    u32 idxRBQP = 0;
    u32 RBQP_offset = 0;
#endif
#ifdef GMAC_RX_ZERO_COPY
    struct sk_buff *skb = NULL;
    u32 RBQP_rx_skb_addr = 0;
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(gmac_dev);
#endif
    // (20071026_CHARLES) Disable TX, RX and MDIO:   (If RX still enabled, the RX buffer will be overwrited)
    MHal_GMAC_Write_CTL(word_ETH_CTL);

    // Initialize "Receive Buffer Queue Pointer"
    MHal_GMAC_Write_RBQP(GMAC_RBQP_BASE -GMAC_MIU0_BUS_BASE);

    // Initialize Receive Buffer Descriptors
    memset((u8*)GMAC_RAM_VA_PA_OFFSET + GMAC_RBQP_BASE, 0x00UL, GMAC_RBQP_SIZE);        // Clear for max(8*1024)bytes (max:1024 descriptors)
    MHal_GMAC_WritRam32(GMAC_RAM_VA_PA_OFFSET, (GMAC_RBQP_BASE + GMAC_RBQP_SIZE - 0x10UL), 0x00000002UL);             // (n-1) : Wrap = 1
#ifdef GMAC_INT_JULIAN_D
    //Reg_rx_frame_cyc[15:8] -0xFF range 1~255
    //Reg_rx_frame_num[7:0]  -0x05 receive frames per INT.
    //0x80 Enable interrupt delay mode.
    //register 0x104 receive counter need to modify smaller for ping
    //Modify bigger(need small than 8) for throughput
    uJulian104Value = GMAC_JULIAN_104_VAL;//0xFF050080;
    MHal_GMAC_Write_JULIAN_0104(uJulian104Value);
#else
    // Enable Interrupts ----------------------------------------------------
    uJulian104Value = 0x00000000UL;
    MHal_GMAC_Write_JULIAN_0104(uJulian104Value);
#endif
    // Set MAC address ------------------------------------------------------
    MHal_GMAC_Write_SA1_MAC_Address(GmacThisBCE.sa1[0], GmacThisBCE.sa1[1], GmacThisBCE.sa1[2], GmacThisBCE.sa1[3], GmacThisBCE.sa1[4], GmacThisBCE.sa1[5]);
    MHal_GMAC_Write_SA2_MAC_Address(GmacThisBCE.sa2[0], GmacThisBCE.sa2[1], GmacThisBCE.sa2[2], GmacThisBCE.sa2[3], GmacThisBCE.sa2[4], GmacThisBCE.sa2[5]);
    MHal_GMAC_Write_SA3_MAC_Address(GmacThisBCE.sa3[0], GmacThisBCE.sa3[1], GmacThisBCE.sa3[2], GmacThisBCE.sa3[3], GmacThisBCE.sa3[4], GmacThisBCE.sa3[5]);
    MHal_GMAC_Write_SA4_MAC_Address(GmacThisBCE.sa4[0], GmacThisBCE.sa4[1], GmacThisBCE.sa4[2], GmacThisBCE.sa4[3], GmacThisBCE.sa4[4], GmacThisBCE.sa4[5]);

#ifdef GMAC_SOFTWARE_DESCRIPTOR
    #ifdef GMAC_RX_CHECKSUM
    uJulian104Value=uJulian104Value | (GMAC_RX_CHECKSUM_ENABLE | GMAC_SOFTWARE_DESCRIPTOR_ENABLE);
    #else
    uJulian104Value=uJulian104Value | GMAC_SOFTWARE_DESCRIPTOR_ENABLE;
    #endif

    MHal_GMAC_Write_JULIAN_0104(uJulian104Value);

    for(idxRBQP = 0; idxRBQP < GMAC_RBQP_LENG; idxRBQP++)
    {
		#ifdef GMAC_RX_ZERO_COPY
		#if 1
		GMAC_dequeue_rx_buffer(LocPtr, &skb);
		if (!skb) {
#ifdef CONFIG_GMAC_RX_CMA
            skb = __alloc_skb_from_cma(pci_cma_device, GMAC_SOFTWARE_DESC_LEN, GFP_ATOMIC, 0, -1);
#else
			skb = alloc_skb(GMAC_SOFTWARE_DESC_LEN, GFP_ATOMIC);
#endif /* CONFIG_GMAC_RX_CMA */
		}
        rx_skb[idxRBQP] = skb;
		#else
        rx_skb[idxRBQP] = alloc_skb(GMAC_SOFTWARE_DESC_LEN, GFP_ATOMIC);
		#endif
	//	skb_reserve(rx_skb[idxRBQP], 2);
        rx_abso_addr[idxRBQP] = rx_skb[idxRBQP]->data;
        RBQP_offset = idxRBQP * 16;
        if(idxRBQP < (GMAC_RBQP_LENG - 1))
        {
			RBQP_rx_skb_addr = __virt_to_phys(rx_abso_addr[idxRBQP]) - GMAC_MIU0_BUS_BASE;
            MHal_GMAC_WritRam32(GMAC_RAM_VA_PA_OFFSET, GMAC_RBQP_BASE + RBQP_offset, RBQP_rx_skb_addr);
        }
        else
        {
			RBQP_rx_skb_addr = __virt_to_phys(rx_abso_addr[idxRBQP]) - GMAC_MIU0_BUS_BASE + GMAC_DESC_WRAP;
            MHal_GMAC_WritRam32(GMAC_RAM_VA_PA_OFFSET, GMAC_RBQP_BASE + RBQP_offset, RBQP_rx_skb_addr);
        }
		#else
        RBQP_offset = idxRBQP * 16;

        if(idxRBQP < (GMAC_RBQP_LENG - 1))
        {
            MHal_GMAC_WritRam32(GMAC_RAM_VA_PA_OFFSET, GMAC_RBQP_BASE + RBQP_offset, (GMAC_RX_BUFFER_BASE - GMAC_MIU0_BUS_BASE + GMAC_SOFTWARE_DESC_LEN * idxRBQP));
        }
        else
        {
            MHal_GMAC_WritRam32(GMAC_RAM_VA_PA_OFFSET, GMAC_RBQP_BASE + RBQP_offset, (GMAC_RX_BUFFER_BASE - GMAC_MIU0_BUS_BASE + GMAC_SOFTWARE_DESC_LEN * idxRBQP + GMAC_DESC_WRAP));
        }
		#endif /*GMAC_RX_ZERO_COPY*/
    }
#endif //#ifdef GMAC_SOFTWARE_DESCRIPTOR

    if (!GmacThisUVE.initedGMAC)
    {
        MHal_GMAC_write_phy(gmac_phyaddr, MII_BMCR, 0x9000UL);
        MHal_GMAC_write_phy(gmac_phyaddr, MII_BMCR, 0x1000UL);
        // IMPORTANT: Run NegotiationPHY() before writing REG_ETH_CFG.
        uNegPhyVal = MHal_GMAC_NegotiationPHY();
        if(uNegPhyVal == 0x01UL)
        {
            GmacThisUVE.flagMacTxPermit = 0x01UL;
            GmacThisBCE.duplex = 1;

        }
        else if(uNegPhyVal == 0x02UL)
        {
            GmacThisUVE.flagMacTxPermit = 0x01UL;
            GmacThisBCE.duplex = 2;
        }

        // ETH_CFG Register -----------------------------------------------------
        word_ETH_CFG = 0x00000800UL;        // Init: CLK = 0x2
        // (20070808) IMPORTANT: REG_ETH_CFG:bit1(FD), 1:TX will halt running RX frame, 0:TX will not halt running RX frame.
        // If always set FD=0, no CRC error will occur. But throughput maybe need re-evaluate.
        // IMPORTANT: (20070809) NO_MANUAL_SET_DUPLEX : The real duplex is returned by "negotiation"
        if(GmacThisBCE.speed     == GMAC_SPEED_100) word_ETH_CFG |= 0x00000001UL;
        if(GmacThisBCE.duplex    == 2)              word_ETH_CFG |= 0x00000002UL;
        if(GmacThisBCE.cam       == 1)              word_ETH_CFG |= 0x00000200UL;
        if(GmacThisBCE.rcv_bcast == 0)              word_ETH_CFG |= 0x00000020UL;
        if(GmacThisBCE.rlf       == 1)              word_ETH_CFG |= 0x00000100UL;

        MHal_GMAC_Write_CFG(word_ETH_CFG);
        // ETH_CTL Register -----------------------------------------------------
        //word_ETH_CTL = 0x0000000CUL;                          // Enable transmit and receive : TE + RE = 0x0C (Disable MDIO)
        if(GmacThisBCE.wes == 1) word_ETH_CTL |= 0x00000080UL;
        MHal_GMAC_Write_CTL(word_ETH_CTL);
#if 0 //FIXME latter
        MHal_GMAC_Write_JULIAN_0100(GMAC_JULIAN_100_VAL);
#else
        MHal_GMAC_Write_JULIAN_0100(0x0000F001UL);
#endif

    #ifdef CONFIG_GMAC_ETHERNET_ALBANY
        MHal_GMAC_Write_JULIAN_0100(0x0000F001UL);
    #endif

        GmacThisUVE.flagPowerOn = 1;
        GmacThisUVE.initedGMAC  = 1;
    }

#ifdef CONFIG_GMAC_NEW_TX_QUEUE
    MHal_GMAC_New_TX_QUEUE_Enable();
    MHal_GMAC_New_TX_QUEUE_Threshold_Set(CONFIG_GMAC_NEW_TX_QUEUE_THRESHOLD);
#endif /* CONFIG_GMAC_NEW_TX_QUEUE */

    MHal_GMAC_HW_init();

}


//-------------------------------------------------------------------------------------------------
// GMAC init Variable
//-------------------------------------------------------------------------------------------------
extern phys_addr_t memblock_start_of_DRAM(void);
extern phys_addr_t memblock_size_of_first_region(void);

static phys_addr_t MDev_GMAC_VarInit(void)
{
    phys_addr_t alloRAM_PA_BASE;
    phys_addr_t alloRAM_SIZE;
    char addr[6];
    u32 HiAddr, LoAddr;
    phys_addr_t *alloRAM_VA_BASE;

    get_boot_mem_info(GMAC_MEM, &alloRAM_PA_BASE, &alloRAM_SIZE);
#if defined (CONFIG_ARM64)
    // get gmac addr only from mboot
    //alloRAM_PA_BASE = memblock_start_of_DRAM() + memblock_size_of_first_region();
#endif

    alloRAM_VA_BASE = (phys_addr_t *)ioremap(alloRAM_PA_BASE, alloRAM_SIZE); //map buncing buffer from PA to VA

    GMAC_DBG("alloRAM_VA_BASE = 0x%zx alloRAM_PA_BASE= 0x%zx  alloRAM_SIZE= 0x%zx\n", (size_t)alloRAM_VA_BASE, (size_t)alloRAM_PA_BASE, alloRAM_SIZE);
    BUG_ON(!alloRAM_VA_BASE);
#ifndef GMAC_RX_ZERO_COPY
    //Add Write Protect
    MHal_GMAC_Write_Protect(alloRAM_PA_BASE & 0x0fffffffUL, alloRAM_SIZE);
#endif
    memset((phys_addr_t *)alloRAM_VA_BASE,0x00UL,alloRAM_SIZE);
    GMAC_RAM_VA_BASE       = ((phys_addr_t)alloRAM_VA_BASE + sizeof(struct GMAC_private) + 0x3FFFUL) & ~0x3FFFUL;   // IMPORTANT: Let lowest 14 bits as zero.
    GMAC_RAM_PA_BASE       = ((phys_addr_t)alloRAM_PA_BASE + sizeof(struct GMAC_private) + 0x3FFFUL) & ~0x3FFFUL;   // IMPORTANT: Let lowest 14 bits as zero.
    GMAC_RX_BUFFER_BASE    = GMAC_RAM_PA_BASE + GMAC_RBQP_SIZE;
    GMAC_RBQP_BASE         = GMAC_RAM_PA_BASE;
#ifdef GMAC_RX_ZERO_COPY
    GMAC_TX_BUFFER_BASE    = GMAC_RAM_PA_BASE + GMAC_RBQP_SIZE;
#else
    GMAC_TX_BUFFER_BASE    = GMAC_RAM_PA_BASE + (GMAC_RX_BUFFER_SIZE + GMAC_RBQP_SIZE);
#endif
    GMAC_RAM_VA_PA_OFFSET  = GMAC_RAM_VA_BASE - GMAC_RAM_PA_BASE;  // IMPORTANT_TRICK_20070512
    GMAC_TX_SKB_BASE       = GMAC_TX_BUFFER_BASE + GMAC_MAX_RX_DESCR * sizeof(struct gmac_rbf_t);

    memset(&GmacThisBCE,0x00UL,sizeof(BasicConfigGMAC));
    memset(&GmacThisUVE,0x00UL,sizeof(UtilityVarsGMAC));

    GmacThisBCE.wes         = 0;                    // 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
    GmacThisBCE.duplex      = 2;                    // 1:Half-duplex, 2:Full-duplex
    GmacThisBCE.cam         = 0;                    // 0:No CAM, 1:Yes
    GmacThisBCE.rcv_bcast   = 0;                    // 0:No, 1:Yes
    GmacThisBCE.rlf         = 0;                    // 0:No, 1:Yes receive long frame(1522)
    GmacThisBCE.rcv_bcast   = 1;
    GmacThisBCE.speed       = GMAC_SPEED_100;

    // Check if bootloader set address in Specific-Address 1 //
    HiAddr = MHal_GMAC_get_SA1H_addr();
    LoAddr = MHal_GMAC_get_SA1L_addr();

    addr[0] = (LoAddr & 0xffUL);
    addr[1] = (LoAddr & 0xff00UL) >> 8;
    addr[2] = (LoAddr & 0xff0000UL) >> 16;
    addr[3] = (LoAddr & 0xff000000UL) >> 24;
    addr[4] = (HiAddr & 0xffUL);
    addr[5] = (HiAddr & 0xff00UL) >> 8;

    if (is_valid_ether_addr (addr))
    {
        memcpy (GmacThisBCE.sa1, &addr, 6);
    }
    else
    {
        // Check if bootloader set address in Specific-Address 2 //
        HiAddr = MHal_GMAC_get_SA2H_addr();
        LoAddr = MHal_GMAC_get_SA2L_addr();
        addr[0] = (LoAddr & 0xffUL);
        addr[1] = (LoAddr & 0xff00UL) >> 8;
        addr[2] = (LoAddr & 0xff0000UL) >> 16;
        addr[3] = (LoAddr & 0xff000000UL) >> 24;
        addr[4] = (HiAddr & 0xffUL);
        addr[5] = (HiAddr & 0xff00UL) >> 8;

        if (is_valid_ether_addr (addr))
        {
            memcpy (GmacThisBCE.sa1, &addr, 6);
        }
        else
        {
            GmacThisBCE.sa1[0]      = GMAC_MY_MAC[0];
            GmacThisBCE.sa1[1]      = GMAC_MY_MAC[1];
            GmacThisBCE.sa1[2]      = GMAC_MY_MAC[2];
            GmacThisBCE.sa1[3]      = GMAC_MY_MAC[3];
            GmacThisBCE.sa1[4]      = GMAC_MY_MAC[4];
            GmacThisBCE.sa1[5]      = GMAC_MY_MAC[5];
        }
    }
    GmacThisBCE.connected = 0;
    return (phys_addr_t)alloRAM_VA_BASE;
}

//-------------------------------------------------------------------------------------------------
// Initialize the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static const struct net_device_ops mstar_lan_netdev_ops = {
        .ndo_open               = MDev_GMAC_open,
        .ndo_stop               = MDev_GMAC_close,
        .ndo_start_xmit =  MDev_GMAC_tx,
        .ndo_set_mac_address    = MDev_GMAC_set_mac_address,
        .ndo_set_rx_mode        = MDev_GMAC_set_rx_mode,
        .ndo_do_ioctl           = MDev_GMAC_ioctl,
        .ndo_get_stats          = MDev_GMAC_stats,
};
#endif
#ifdef GMAC_TASKLET_SUPPORT
static void MDrv_GMAC_TaskletBH(unsigned long data)
{
    struct GMAC_private *LocPtr = (struct GMAC_private *) data;
	struct net_device *dev = LocPtr->dev;

	MDev_GMAC_rx(dev, GMAC_NAPI_WEIGHT);

}
#endif

static int MDev_GMAC_setup (struct net_device *dev, unsigned long phy_type)
{
    struct GMAC_private *LocPtr;

    static int already_initialized = 0;
    dma_addr_t dmaaddr;
    u32 val;
    phys_addr_t RetAddr;
#ifdef CONFIG_MSTAR_GMAC_HW_TX_CHECKSUM
    u32 retval;
#endif
#ifdef CONFIG_MSTAR_GMAC_JUMBO_PACKET
    u32 retval;
#endif
    if (already_initialized)
        return FALSE;

        LocPtr = (struct GMAC_private *) netdev_priv(dev);

#ifdef CONFIG_GMAC_ISR_BOTTOM_HALF
#ifdef CONFIG_GMAC_ISR_BH_TASKLET
    tasklet_init(&LocPtr->rx_task, MDev_GMAC_bottom_rx_task, (unsigned long)LocPtr);
    tasklet_init(&LocPtr->rst_task, MDev_GMAC_bottom_rst_task, (unsigned long)LocPtr);
#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
    tasklet_init(&LocPtr->tx_task, MDev_GMAC_bottom_tx_task, (unsigned long)LocPtr);
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */
#endif /* CONFIG_GMAC_ISR_BH_TASKLET */
#endif /* CONFIG_GMAC_ISR_BOTTOM_HALF */

        LocPtr->dev = dev;
        RetAddr = MDev_GMAC_VarInit();

        if(!RetAddr)
        {
            GMAC_DBG("Var init fail!!\n");
            return FALSE;
        }

        if (LocPtr == NULL)
        {
            free_irq (dev->irq, dev);
            GMAC_DBG("LocPtr fail\n");
            return -ENOMEM;
        }

        dev->base_addr = (long) GMAC_REG_ADDR_BASE;
        MDev_GMAC_HW_init();
        dev->irq =  E_IRQEXPH_SECEMAC;

        // Allocate memory for DMA Receive descriptors //
        LocPtr->dlist_phys = LocPtr->dlist = (struct gmac_recv_desc_bufs *) (GMAC_RBQP_BASE + GMAC_RAM_VA_PA_OFFSET);

        if (LocPtr->dlist == NULL)
        {
            dma_free_noncoherent((void *)LocPtr, GMAC_ABSO_MEM_SIZE,&dmaaddr,0);//kfree (dev->priv);
            free_irq (dev->irq, dev);
            return -ENOMEM;
        }

        LocPtr->lock = &gmac_lock;
        LocPtr->txlock = &gmac_txlock;
        spin_lock_init (LocPtr->lock);
        spin_lock_init (LocPtr->txlock);
#ifdef CONFIG_GMAC_ISR_BOTTOM_HALF
    LocPtr->lock_rx = &gmac_lock_rx;
    spin_lock_init(LocPtr->lock_rx);
    LocPtr->lock_rst = &gmac_lock_rst;
    spin_lock_init(LocPtr->lock_rst);
#endif /* CONFIG_GMAC_ISR_BOTTOM_HALF */
        ether_setup (dev);
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,28)
        dev->open = MDev_GMAC_open;
        dev->stop = MDev_GMAC_close;
        dev->hard_start_xmit = MDev_GMAC_tx;
        dev->get_stats = MDev_GMAC_stats;
        dev->set_multicast_list = MDev_GMAC_set_rx_mode;
        dev->do_ioctl = MDev_GMAC_ioctl;
        dev->set_mac_address = MDev_GMAC_set_mac_address;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
        dev->netdev_ops = &mstar_lan_netdev_ops;
#endif
        dev->tx_queue_len = GMAC_MAX_TX_QUEUE;

        MDev_GMAC_get_mac_address (dev);    // Get ethernet address and store it in dev->dev_addr //
        MDev_GMAC_update_mac_address (dev); // Program ethernet address into MAC //
        spin_lock_irq (LocPtr->lock);
        MHal_GMAC_enable_mdi ();
        MHal_GMAC_read_phy (gmac_phyaddr, GMAC_MII_USCR_REG, &val);
        if ((val & (1 << 10)) == 0)   // DSCR bit 10 is 0 -- fiber mode //
            LocPtr->phy_media = PORT_FIBRE;

        spin_unlock_irq (LocPtr->lock);

        //Support for ethtool //
        LocPtr->mii.dev = dev;
        LocPtr->mii.mdio_read = MDev_GMAC_mdio_read;
        LocPtr->mii.mdio_write = MDev_GMAC_mdio_write;
        already_initialized = 1;
#ifdef CONFIG_MSTAR_GMAC_HW_TX_CHECKSUM
        retval = MHal_GMAC_Read_JULIAN_0104() | GMAC_TX_CHECKSUM_ENABLE;
        MHal_GMAC_Write_JULIAN_0104(retval);
        dev->features |= NETIF_F_IP_CSUM;

        retval = MHal_GMAC_Read_JULIAN_0414() | GMAC_TX_V6_CHECKSUM_ENABLE;
        MHal_GMAC_Write_JULIAN_0414(retval);
        dev->features |= NETIF_F_IPV6_CSUM;
#endif

#ifdef CONFIG_MSTAR_GMAC_JUMBO_PACKET
        retval = MHal_GMAC_Read_JULIAN_0418() | GMAC_TX_CHECKSUM_ENABLE;
        retval &= 0xffff0000UL;
        retval |= GMAC_TX_JUMBO_FRAME_ENABLE;
        MHal_GMAC_Write_JULIAN_0418(retval);
#endif

#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
    /* clear interrupt status,  to avoid interrupt was set in uboot*/
    MHal_GMAC_Read_ISR();
    MHal_GMAC_Write_IDR(0xFFFF);
    MHal_GMAC_Read_JULIAN_0108();
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */

        //Install the interrupt handler //
        //Notes: Modify linux/kernel/irq/manage.c  /* interrupt.h */
    if (request_irq(dev->irq, MDev_GMAC_interrupt, SA_INTERRUPT | IRQF_ONESHOT, dev->name, dev))
            return -EBUSY;
#if defined(CONFIG_MP_PLATFORM_GIC_SET_MULTIPLE_CPUS) && defined(CONFIG_MP_PLATFORM_INT_1_to_1_SPI)
		irq_set_affinity_hint(dev->irq, cpu_online_mask);
    irq_set_affinity(dev->irq, cpu_online_mask);
#endif

#ifdef GMAC_TASKLET_SUPPORT
		tasklet_init(&LocPtr->rx_tasklet, MDrv_GMAC_TaskletBH, (unsigned long)LocPtr);
#endif

        //Determine current link speed //
        spin_lock_irq (LocPtr->lock);
        (void) MDev_GMAC_update_linkspeed (dev);
    spin_unlock_irq (LocPtr->lock);

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Restar the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_SwReset(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private *) netdev_priv(dev);
    u32 oldCFG, oldCTL;
    u32 retval;

    MDev_GMAC_get_mac_address (dev);
    oldCFG = MHal_GMAC_Read_CFG();
    oldCTL = MHal_GMAC_Read_CTL() & ~(GMAC_TE | GMAC_RE);

    //free tx skb
    if (LocPtr->retx_count)
    {
        if (LocPtr->skb)
        {
            dev_kfree_skb_irq(LocPtr->skb );
            LocPtr->skb = NULL;
        }
        if (netif_queue_stopped (dev))
            netif_wake_queue (dev);
    }
#ifdef GMAC_RX_ZERO_COPY
	free_rx_skb();
#endif

    netif_stop_queue (dev);

    retval = MHal_GMAC_Read_JULIAN_0100();
    MHal_GMAC_Write_JULIAN_0100(retval & 0x00000FFFUL);
    MHal_GMAC_Write_JULIAN_0100(retval);

    MDev_GMAC_HW_init();
    MHal_GMAC_Write_CFG(oldCFG);
    MHal_GMAC_Write_CTL(oldCTL);
    MHal_GMAC_enable_mdi ();
    MDev_GMAC_update_mac_address (dev); // Program ethernet address into MAC //
    MDev_GMAC_update_linkspeed (dev);
    MHal_GMAC_Write_IER(GMAC_IER_FOR_INT_JULIAN_D);
    MDev_GMAC_start (dev);
    MDev_GMAC_set_rx_mode(dev);
    netif_start_queue (dev);
    LocPtr->retx_count = 0;
    ROVRcount = 0;
#ifdef CONFIG_MSTAR_GMAC_HW_TX_CHECKSUM
    retval = MHal_GMAC_Read_JULIAN_0104() | GMAC_TX_CHECKSUM_ENABLE;
    MHal_GMAC_Write_JULIAN_0104(retval);
    dev->features |= NETIF_F_IP_CSUM;
#endif
    //GMAC_DBG("=> Take %lu ms to reset GMAC!\n", (getCurMs() - gmac_oldTime));
    return 0;
}

#if defined (CONFIG_ARM64)
static struct of_device_id mstargmac_of_device_ids[] = {
    {.compatible = "mstar-gmac"},
    {},
};
#endif
//-------------------------------------------------------------------------------------------------
// Detect MAC and PHY and perform initialization
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_probe (struct net_device *dev)
{
    int detected = -1;

    /* Read the PHY ID registers - try all addresses */
    detected = MDev_GMAC_setup(dev, GMAC_MII_URANUS_ID);
    return detected;
}

//-------------------------------------------------------------------------------------------------
// GMAC Timer set for Receive function
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_timer_callback(unsigned long value)
{
    int ret = 0;
    struct GMAC_private *LocPtr = (struct GMAC_private *) netdev_priv(gmac_dev);
    static u32 bmsr, time_count = 0;
#ifndef GMAC_INT_JULIAN_D
    if (GMAC_RX_TMR == value)
    {
        MHal_GMAC_timer_callback(value);
        return;
    }
#endif
    spin_lock_irq (LocPtr->lock);
    ret = MDev_GMAC_update_linkspeed(gmac_dev);
    spin_unlock_irq (LocPtr->lock);
    if (0 == ret)
    {
        if (!GmacThisBCE.connected)
        {
            GmacThisBCE.connected = 1;
            netif_carrier_on(gmac_dev);
        }

        // Link status is latched, so read twice to get current value //
        MHal_GMAC_read_phy (gmac_phyaddr, MII_BMSR, &bmsr);
        MHal_GMAC_read_phy (gmac_phyaddr, MII_BMSR, &bmsr);
        time_count = 0;
        spin_lock_irq (LocPtr->lock);
        gmac_phy_status_register = bmsr;
        spin_unlock_irq (LocPtr->lock);
        // Normally, time out sets 1 Sec.
        GMAC_Link_timer.expires = jiffies + GMAC_CHECK_LINK_TIME;
    }
    else    //no link
    {
        if(GmacThisBCE.connected) {
            GmacThisBCE.connected = 0;
        }
        // If disconnected is over 3 Sec, the real value of PHY's status register will report to application.
        if(time_count > 30) {
            // Link status is latched, so read twice to get current value //
            MHal_GMAC_read_phy (gmac_phyaddr, MII_BMSR, &bmsr);
            MHal_GMAC_read_phy (gmac_phyaddr, MII_BMSR, &bmsr);
            spin_lock_irq (LocPtr->lock);
            gmac_phy_status_register = bmsr;
            spin_unlock_irq (LocPtr->lock);
            // Report to kernel.
            netif_carrier_off(gmac_dev);
            GmacThisBCE.connected = 0;
            // Normally, time out is set 1 Sec.
            GMAC_Link_timer.expires = jiffies + GMAC_CHECK_LINK_TIME;
        }
        else if(time_count <= 30){
            time_count++;
            // Time out is set 100ms. Quickly checks next phy status.
            GMAC_Link_timer.expires = jiffies + (GMAC_CHECK_LINK_TIME / 10);
        }
    }
	GMAC_Link_timer.expires = jiffies + GMAC_CHECK_LINK_TIME;

	if (GmacThisBCE.connected)
	{
		MDev_GMAC_Check_TXRX(1);
	}
    add_timer(&GMAC_Link_timer);
}

//-------------------------------------------------------------------------------------------------
// GMAC MACADDR Setup
//-------------------------------------------------------------------------------------------------

#define MACADDR_FORMAT "XX:XX:XX:XX:XX:XX"

static int __init macaddr_auto_config_setup(char *addrs)
{
    if (strlen(addrs) == strlen(MACADDR_FORMAT)
        && ':' == addrs[2]
        && ':' == addrs[5]
        && ':' == addrs[8]
        && ':' == addrs[11]
        && ':' == addrs[14]
       )
    {
        addrs[2]  = '\0';
        addrs[5]  = '\0';
        addrs[8]  = '\0';
        addrs[11] = '\0';
        addrs[14] = '\0';

        GMAC_MY_MAC[0] = (u8)simple_strtoul(&(addrs[0]),  NULL, 16);
        GMAC_MY_MAC[1] = (u8)simple_strtoul(&(addrs[3]),  NULL, 16);
        GMAC_MY_MAC[2] = (u8)simple_strtoul(&(addrs[6]),  NULL, 16);
        GMAC_MY_MAC[3] = (u8)simple_strtoul(&(addrs[9]),  NULL, 16);
        GMAC_MY_MAC[4] = (u8)simple_strtoul(&(addrs[12]), NULL, 16);
        GMAC_MY_MAC[5] = (u8)simple_strtoul(&(addrs[15]), NULL, 16);

        /* set back to ':' or the environment variable would be destoried */ // REVIEW: this coding style is dangerous
        addrs[2]  = ':';
        addrs[5]  = ':';
        addrs[8]  = ':';
        addrs[11] = ':';
        addrs[14] = ':';
    }

    return 1;
}

__setup("macaddr=", macaddr_auto_config_setup);

//-------------------------------------------------------------------------------------------------
// GMAC init module
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_ScanPhyAddr(void)
{
    unsigned char addr = 1; // because address 0 = broadcast, RTL8211E will reply to broadcast addr
    u32 value = 0;

#ifdef CONFIG_GMAC_ETHERNET_ALBANY
    MHal_GMAC_Write_JULIAN_0100(0x0000F001UL);
#else
    MHal_GMAC_Write_JULIAN_0100(0x0000F007UL);
#endif

    MHal_GMAC_enable_mdi();
    do
    {
        value = 0;
        MHal_GMAC_read_phy(addr, MII_BMSR, &value);
        if (0 != value && 0x0000FFFFUL != value)
        {
            GMAC_DBG("[ PHY Addr ] ==> :%u BMSR = %08x\n", addr, value);
            break;
        }
    }while(++addr && addr < 32);

    gmac_phyaddr = addr;

    if (gmac_phyaddr >= 32)
    {
        addr = 0;
        MHal_GMAC_read_phy(addr, MII_BMSR, &value);
        if (0 != value && 0x0000FFFFUL != value)
        {
            GMAC_DBG("[ PHY Addr ] ==> :%u BMSR = %08x\n", addr, value);
            gmac_phyaddr = 0;
        }
        else
        {
        GMAC_DBG("Wrong PHY Addr, maybe MoCA?\n");
            gmac_phyaddr = 32;
    }
    }

    MHal_GMAC_disable_mdi();
    return 0;
}

static void Rtl_Patch(void)
{
    u32 val;

    MHal_GMAC_read_phy(gmac_phyaddr, 25, &val);
    MHal_GMAC_write_phy(gmac_phyaddr, 25, 0x400UL);
    MHal_GMAC_read_phy(gmac_phyaddr, 25, &val);
}

static void MDev_GMAC_Patch_PHY(void)
{
    u32 val;

    MHal_GMAC_read_phy(gmac_phyaddr, 2, &val);
    if (GMAC_RTL_8210 == val)
        Rtl_Patch();
}

#ifdef GMAC_RX_ZERO_COPY
static int GMAC_dequeue_rx_buffer(struct GMAC_private *p, struct sk_buff **pskb)
{
	p->rx_next = (p->rx_next + 1) % GMAC_RX_RING_SIZE;
	p->rx_current_fill--;
	*pskb = __skb_dequeue(&p->rx_list);

	if (gmac_debug)
		GMAC_DBG("rx_next:%d rx_next_fill:%d rx_current_fill:%d\n",p->rx_next, p->rx_next_fill, p->rx_current_fill);
	return p->rx_current_fill;
}

static int GMAC_rx_fill_ring(struct net_device *netdev)
{
    struct GMAC_private *p = netdev_priv(netdev);

	while (p->rx_current_fill < GMAC_RX_RING_SIZE)
	{
		struct sk_buff *skb;
#ifdef CONFIG_GMAC_RX_CMA
        if (!(skb = __alloc_skb_from_cma(pci_cma_device, GMAC_SOFTWARE_DESC_LEN, GFP_ATOMIC, 0, -1))) {
#else
        if (!(skb = alloc_skb (GMAC_SOFTWARE_DESC_LEN, GFP_ATOMIC))) {
#endif /* CONFIG_GMAC_RX_CMA */
			GMAC_DBG("%s %d: alloc skb failed! RX current fill:%d\n",__func__,__LINE__, p->rx_current_fill);
			break;
		}

//		skb_reserve(skb, NET_IP_ALIGN);
		__skb_queue_tail(&p->rx_list, skb);
		p->rx_current_fill++;
		p->rx_next_fill = (p->rx_next_fill + 1) % GMAC_RX_RING_SIZE;
	}

	if (unlikely(gmac_debug))
		GMAC_DBG("%s Current fill:%d. rx next fill:%d\n",__func__, p->rx_current_fill, p->rx_next_fill);
	return p->rx_current_fill;
}
#endif /*GMAC_RX_ZERO_COPY*/

static int MDev_GMAC_init(void)
{
    struct GMAC_private *LocPtr;
#ifdef NR_NAPI
	int i;
#endif

    if(gmac_dev)
        return -1;

    gmac_dev = alloc_etherdev(sizeof(*LocPtr));
    LocPtr = netdev_priv(gmac_dev);
    if (!gmac_dev)
    {
        GMAC_DBG( KERN_ERR "No GMAC dev mem!\n" );
        return -ENOMEM;
    }
#ifdef GMAC_RX_ZERO_COPY
	skb_queue_head_init(&LocPtr->tx_list);
	skb_queue_head_init(&LocPtr->rx_list);
	LocPtr->rx_next = 0;
	LocPtr->rx_next_fill = 0;
	LocPtr->rx_current_fill = 0;

	GMAC_rx_fill_ring(gmac_dev);
#endif

#if GMAC_TX_THROUGHPUT_TEST
    printk("==========TX_THROUGHPUT_TEST===============");
    gmac_pseudo_packet = alloc_skb(GMAC_SOFTWARE_DESC_LEN, GFP_ATOMIC);
    memcpy(gmac_pseudo_packet->data, (void *)gmac_packet_content, sizeof(gmac_packet_content));
    gmac_pseudo_packet->len = sizeof(gmac_packet_content);
    init_timer(&GMAC_TX_timer);
    GMAC_TX_timer.data = GMAC_RX_TMR;
    GMAC_TX_timer.function = TX_timer_callback;
    GMAC_TX_timer.expires = jiffies + 10*GMAC_CHECK_LINK_TIME;
    add_timer(&GMAC_TX_timer);
#endif

#if GMAC_RX_THROUGHPUT_TEST
    printk("==========RX_THROUGHPUT_TEST===============");
    init_timer(&GMAC_RX_timer);

    GMAC_RX_timer.data = GMAC_RX_TMR;
    GMAC_RX_timer.function = RX_timer_callback;
    GMAC_RX_timer.expires = jiffies + (RX_THROUGHPUT_TEST_INTERVAL*GMAC_CHECK_LINK_TIME);
    add_timer(&GMAC_RX_timer);
#endif
#ifdef GMAC_NAPI
#ifdef NR_NAPI
	spin_lock_init(&cpu_state.lock);
	for_each_possible_cpu(i) {
		netif_napi_add(gmac_dev, &gmac_napi[i].napi_str, MDev_GMAC_napi_poll, rx_napi_weight[i]);
		gmac_napi[i].available = 1;
		gmac_napi[i].cpu = i;
		napi_enable(&gmac_napi[i].napi_str);
	}
	cpu_state.baseline_cores = num_online_cpus();
	cpu_state.active_cores = 0;
#else
    netif_napi_add(gmac_dev, &LocPtr->napi_str, MDev_GMAC_napi_poll, GMAC_NAPI_WEIGHT);
    napi_enable(&LocPtr->napi_str);
#endif
#endif

#ifdef CONFIG_GMAC_ISR_BH_NAPI
    netif_napi_add(gmac_dev, &LocPtr->napi, MDev_GMAC_napi_poll, CONFIG_GMAC_NAPI_WEIGHT);
#endif /* CONFIG_GMAC_ISR_BH_NAPI */

    MHal_GMAC_Power_On_Clk();

    init_timer(&GMAC_timer);
    init_timer(&GMAC_Link_timer);

    GMAC_timer.data = GMAC_RX_TMR;
    GMAC_timer.function = MDev_GMAC_timer_callback;
    GMAC_timer.expires = jiffies;


    MHal_GMAC_Write_JULIAN_0100(GMAC_JULIAN_100_VAL);

    if (0 > MDev_GMAC_ScanPhyAddr())
        goto end;

    MDev_GMAC_Patch_PHY();
    if (!MDev_GMAC_probe (gmac_dev))
        return register_netdev (gmac_dev);

end:
    free_netdev(gmac_dev);
    gmac_dev = 0;
    gmac_initstate = GMAC_ETHERNET_TEST_INIT_FAIL;
    GMAC_DBG( KERN_ERR "Init GMAC error!\n" );
    return -1;
}
//-------------------------------------------------------------------------------------------------
// GMAC exit module
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_exit(void)
{
    if (gmac_dev)
    {
    #ifndef GMAC_INT_JULIAN_D
        del_timer(&GMAC_timer);
    #endif
        unregister_netdev(gmac_dev);
        free_netdev(gmac_dev);
    }
}

static int mstar_gmac_drv_suspend(struct platform_device *dev, pm_message_t state)
{
    struct net_device *netdev=(struct net_device*)dev->dev.platform_data;
    struct GMAC_private *LocPtr;
    u32 uRegVal;
    printk(KERN_INFO "mstar_gmac_drv_suspend\n");
    if(!netdev)
    {
        return -1;
    }

    LocPtr = (struct GMAC_private*) netdev_priv(netdev);

    LocPtr->ep_flag |= GMAC_EP_FLAG_SUSPENDING;
    netif_stop_queue (netdev);

    disable_irq(netdev->irq);
    del_timer(&GMAC_Link_timer);

    MHal_GMAC_Power_On_Clk();

    //Disable Receiver and Transmitter //
    uRegVal = MHal_GMAC_Read_CTL();
    uRegVal &= ~(GMAC_TE | GMAC_RE);
    MHal_GMAC_Write_CTL(uRegVal);

    // Disable PHY interrupt //
    MHal_GMAC_disable_phyirq ();

    //Disable MAC interrupts //
	MDev_GMAC_irq_onoff(0, __func__);
    MHal_GMAC_Power_Off_Clk();
    MDev_GMAC_close(netdev);
    return 0;
}
static int mstar_gmac_drv_resume(struct platform_device *dev)
{
    struct net_device *netdev=(struct net_device*)dev->dev.platform_data;
    struct GMAC_private *LocPtr;
    phys_addr_t alloRAM_PA_BASE;
    phys_addr_t alloRAM_SIZE;
    u32 retval;
    printk(KERN_INFO "mstar_gmac_drv_resume\n");
    if(!netdev)
    {
        return -1;
    }
    LocPtr = (struct GMAC_private*) netdev_priv(netdev);;
    LocPtr->ep_flag &= ~GMAC_EP_FLAG_SUSPENDING;

    MHal_GMAC_Power_On_Clk();

    MHal_GMAC_Write_JULIAN_0100(GMAC_JULIAN_100_VAL);

    if (0 > MDev_GMAC_ScanPhyAddr())
        return -1;

    MDev_GMAC_Patch_PHY();

    get_boot_mem_info(EMAC_MEM, &alloRAM_PA_BASE, &alloRAM_SIZE);
#if defined(CONFIG_ARM64)
    alloRAM_PA_BASE = memblock_start_of_DRAM() + memblock_size_of_first_region();
#endif
#ifndef GMAC_RX_ZERO_COPY
    //Add Write Protect
    //MHal_GMAC_Write_Protect(alloRAM_PA_BASE, alloRAM_SIZE);
#endif

    GmacThisUVE.initedGMAC = 0;
    MDev_GMAC_HW_init();

    MDev_GMAC_update_mac_address (netdev); // Program ethernet address into MAC //
    spin_lock_irq (LocPtr->lock);
    MHal_GMAC_enable_mdi ();
    MHal_GMAC_read_phy (gmac_phyaddr, GMAC_MII_USCR_REG, &retval);
    if ((retval & (1 << 10)) == 0)   // DSCR bit 10 is 0 -- fiber mode //
        LocPtr->phy_media = PORT_FIBRE;

    spin_unlock_irq (LocPtr->lock);

#ifdef CONFIG_MSTAR_GMAC_HW_TX_CHECKSUM
    retval = MHal_GMAC_Read_JULIAN_0104() | GMAC_TX_CHECKSUM_ENABLE;
    MHal_GMAC_Write_JULIAN_0104(retval);
#endif

    enable_irq(netdev->irq);
    if(0>MDev_GMAC_open(netdev))
    {
        printk(KERN_WARNING "Driver GMAC: open failed after resume\n");
    }
    return 0;
}

static int mstar_gmac_drv_probe(struct platform_device *pdev)
{
    int retval=0;
    if( !(pdev->name) || strcmp(pdev->name,"Mstar-gmac")
        || pdev->id!=0)
    {
        retval = -ENXIO;
    }
#ifdef CONFIG_GMAC_NAPI_GRO
    GMAC_DBG("Start GMAC V2 GRO\n");
#else
    GMAC_DBG("Start GMAC V2\n");
#endif

    retval = MDev_GMAC_init();
    if(!retval)
    {
        pdev->dev.platform_data=gmac_dev;
    }
    return retval;
}

static int mstar_gmac_drv_remove(struct platform_device *pdev)
{
    if( !(pdev->name) || strcmp(pdev->name,"Mstar-gmac")
        || pdev->id!=0)
    {
        return -1;
    }
    MDev_GMAC_exit();
    pdev->dev.platform_data=NULL;
    return 0;
}



static struct platform_driver Mstar_gmac_driver = {
    .probe      = mstar_gmac_drv_probe,
    .remove     = mstar_gmac_drv_remove,
    .suspend    = mstar_gmac_drv_suspend,
    .resume     = mstar_gmac_drv_resume,

    .driver = {
        .name   = "Mstar-gmac",
#if defined(CONFIG_ARM64)
        .of_match_table = mstargmac_of_device_ids,
#endif
        .owner  = THIS_MODULE,
    }
};

static int __init mstar_gmac_drv_init_module(void)
{
    int retval=0;

    gmac_dev=NULL;
    retval = platform_driver_register(&Mstar_gmac_driver);
    return retval;
}

static void __exit mstar_gmac_drv_exit_module(void)
{
    platform_driver_unregister(&Mstar_gmac_driver);
    gmac_dev=NULL;
}

module_param(gmac_tx_debug, uint, 0644);
MODULE_PARM_DESC(gmac_tx_debug, "Debug info for GMAC TX");
module_param(gmac_debug, uint, 0644);
MODULE_PARM_DESC(gmac_debug, "Debug info for GMAC");
module_param(gmac_debug_napi, uint, 0644);
MODULE_PARM_DESC(gmac_debug_napi, "Debug info for GMAC");
module_param(gmac_dump_skb, uint, 0644);
MODULE_PARM_DESC(gmac_dump_skb, "Dump SKB for GMAC");
#if GMAC_TX_THROUGHPUT_TEST
module_param(gmac_tx_test, uint, 0644);
MODULE_PARM_DESC(gmac_tx_test, "TX test for GMAC");
module_param(gmac_tx_thread, uint, 0644);
MODULE_PARM_DESC(gmac_tx_thread, "TX thread sender for GMAC");
#endif
#if GMAC_RX_THROUGHPUT_TEST
#endif

module_init(mstar_gmac_drv_init_module);
module_exit(mstar_gmac_drv_exit_module);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("GMAC Ethernet driver");
MODULE_LICENSE("GPL");
