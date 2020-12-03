/*
* mdrv_emac.c- Sigmastar
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
* mdrv_emac.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: richard.guo <richard.guo@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/init.h>
//#include <linux/autoconf.h>
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
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

#if defined(CONFIG_MIPS)
#include <asm/mips-boards/prom.h>
#include "mhal_chiptop_reg.h"
#elif defined(CONFIG_ARM)
#include <asm/prom.h>
#include <asm/mach/map.h>
#endif

#include "mdrv_types.h"
//#include "mst_platform.h"
//#include "mdrv_system.h"
//#include "chip_int.h"
#include "ms_msys.h"
#include "mhal_emac.h"
#include "mdrv_emac.h"
#include "ms_platform.h"
#include "registers.h"

#ifdef CONFIG_EMAC_SUPPLY_RNG
#include <linux/input.h>
#include <random.h>
#include "mhal_rng_reg.h"
#endif

// #include "mdrv_msys_io_st.h"
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/phy.h>

#include "gpio.h"
#ifdef CONFIG_MS_PADMUX
#include "mdrv_padmux.h"
#include "mdrv_puse.h"
#endif

#ifdef CONFIG_MS_GPIO
extern void MDrv_GPIO_Set_Low(U8 u8IndexGPIO);
extern void MDrv_GPIO_Set_High(U8 u8IndexGPIO);
extern void MDrv_GPIO_Pad_Set(U8 u8IndexGPIO);
#else
#define MDrv_GPIO_Set_Low(x)
#define MDrv_GPIO_Set_High(x)
#define MDrv_GPIO_Pad_Set(x)
#endif

/////////////////////////////////
// to be refined
/////////////////////////////////
#define TXD_NUM         0
// #define TXQ_NUM_SW      256
#define TXQ_NUM_SW      0

#define RX_DESC_API     0

#if EXT_PHY_PATCH
#ifdef CONFIG_EMAC_SPEED10_HALF_PATCH
#define IS_EXT_PHY(hemac)       (1)
#else
#define IS_EXT_PHY(hemac)       (0 == (hemac)->phyRIU)
#endif
#endif

//--------------------------------------------------------------------------------------------------
//  helper definition
//--------------------------------------------------------------------------------------------------
#define CLR_BITS(a, bits)       ((a) & (~(bits)))
#define SET_BITS(a, bits)       ((a) | (bits))

#define PA2BUS(a)               CLR_BITS(a, MIU0_BUS_BASE)
#define BUS2PA(a)               SET_BITS(a, MIU0_BUS_BASE)

#define BUS2VIRT(a)             phys_to_virt(BUS2PA((a)))
#define VIRT2BUS(a)             PA2BUS(virt_to_phys((a)))

#define VIRT2PA(a)              virt_to_phys((a))

#if RX_DESC_API
#define RX_DESC_MAKE(desc, bus, wrap) \
{ \
    if ((bus) & 0x3) \
        printk("[%s][%d] bad RX buffer address 0x%08x\n", __FUNCTION__, __LINE__, (bus)); \
    ((desc)->addr = (((bus) << 2) & 0xFFFFFFFC) | (wrap)); \
}
#define RX_ADDR_GET(desc)                       BUS2VIRT(((((desc)->addr) & 0xFFFFFFFC)>> 2))
#endif

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#if MSTAR_EMAC_NAPI
#define EMAC_NAPI_WEIGHT                    32
#endif

#define RX_DESC_NUM                             0x100
#define RX_DESC_SIZE                            (sizeof(struct rbf_t))
#define RX_DESC_QUEUE_SIZE                      (RX_DESC_NUM * RX_DESC_SIZE)

#define EMAC_PACKET_SIZE_MAX                    0x600

#if EMAC_SG
    #define FEATURES_EMAC_SG                    (NETIF_F_SG)
#else
    #define FEATURES_EMAC_SG                    (0)
#endif

#if (EMAC_GSO)
    #define FEATURES_EMAC_GSO                   ((NETIF_F_GSO) | (NETIF_F_GRO))
#else
    #define FEATURES_EMAC_GSO                   (0)
#endif

#if RX_CHECKSUM
    #define FEATURES_EMAC_CSUM_RX               (NETIF_F_RXCSUM)
#else
    #define FEATURES_EMAC_CSUM_RX               (0)
#endif

#if TX_CHECKSUM
    #define FEATURES_EMAC_CSUM_TX               (NETIF_F_HW_CSUM)
#else
    #define FEATURES_EMAC_CSUM_TX               (0)
#endif

#define EMAC_FEATURES                           (FEATURES_EMAC_SG | FEATURES_EMAC_GSO | FEATURES_EMAC_CSUM_RX | FEATURES_EMAC_CSUM_TX)

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
// #define EMAC_RX_TMR         (0)
// #define EMAC_LINK_TMR       (1)
// #define EMAC_FLOW_CTL_TMR   (2)

// #define TIMER_EMAC_DYNAMIC_RX   (1)
// #define TIMER_EMAC_FLOW_TX      (2)

#define EMAC_CHECK_LINK_TIME        (HZ)

#define IDX_CNT_INT_DONE                (0)
#define IDX_CNT_INT_RCOM                (1)
#define IDX_CNT_INT_RBNA                (2)
#define IDX_CNT_INT_TOVR                (3)
#define IDX_CNT_INT_TUND                (4)
#define IDX_CNT_INT_RTRY                (5)
#define IDX_CNT_INT_TBRE                (6)
#define IDX_CNT_INT_TCOM                (7)
#define IDX_CNT_INT_TIDLE               (8)
#define IDX_CNT_INT_LINK                (9)
#define IDX_CNT_INT_ROVR                (10)
#define IDX_CNT_INT_HRESP               (11)
#define IDX_CNT_JULIAN_D                (12)
#define IDX_CNT_INT_TXQUEUE_THRESHOLD   (24) // (EMAC)(I3E)
#define IDX_CNT_INT_TXQUEUE_EMPTY       (25) // (EMAC)(I3E)
#define IDX_CNT_INT_TXQUEUE_DROP        (26) // (EMAC)(I3E)

#if 0
u32 gu32CheckLinkTime = HZ;
u32 gu32CheckLinkTimeDis = 100;
u32 gu32intrEnable;
u32 irq_count[32]={0};
u32 gu32PhyResetCount1=0;
u32 gu32PhyResetCount2=0;
u32 gu32PhyResetCount3=0;
u32 gu32PhyResetCount4=0;
u32 gu32PhyResetCount=0;

static u32 skb_tx_send = 0;
static u32 skb_tx_free = 0;

static u64 data_done = 0;
static u32 txPkt = 0;
static u32 txInt = 0;
static struct timespec data_time_last = { 0 };
static DEFINE_SPINLOCK(emac_data_done_lock);
#endif

// #define RTL_8210                    (0x1CUL)

#define RX_THROUGHPUT_TEST 0
#define TX_THROUGHPUT_TEST 0

#ifdef CONFIG_MP_ETHERNET_MSTAR_ICMP_ENHANCE
#define PACKET_THRESHOLD 260
#define TXCOUNT_THRESHOLD 10
#endif

#if EMAC_FLOW_CONTROL_TX
#define MAC_CONTROL_TYPE              0x8808
#define MAC_CONTROL_OPCODE            0x0001
#define PAUSE_QUANTA_TIME_10M         ((1000000*10)/500)
#define PAUSE_QUANTA_TIME_100M        ((1000000*100)/500)
#define PAUSE_TIME_DIVISOR_10M        (PAUSE_QUANTA_TIME_10M/HZ)
#define PAUSE_TIME_DIVISOR_100M       (PAUSE_QUANTA_TIME_100M/HZ)
#endif // #if EMAC_FLOW_CONTROL_TX

//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
// u32 contiROVR = 0;
// u32 initstate= 0;
// u8 txidx =0;
// u32 txcount = 0;
// spinlock_t emac_lock;

// 0x78c9: link is down.
// static u32 phy_status_register = 0x78c9UL;

static dev_t gEthDev;
static u8 _u8Minor = MINOR_EMAC_NUM;
struct sk_buff *pseudo_packet;

#if TX_THROUGHPUT_TEST
unsigned char packet_content[] = {
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

static unsigned int max_rx_packet_count=0;
static unsigned int max_tx_packet_count=0;
static unsigned int min_tx_fifo_idle_count=0xffff;
static unsigned int tx_bytes_per_timerbak=0;
static unsigned int tx_bytes_per_timer=0;
// u32 RAM_ALLOC_SIZE=0;

// static unsigned int gu32GatingRxIrqTimes=0;


int rx_packet_cnt = 0;

static struct timespec rx_time_last = { 0 };
static int rx_duration_max = 0;

static int _phyReset = 0;

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
// static struct timer_list EMAC_timer, hemac->timer_link;
// static struct timer_list hemac->timer_link;
#if RX_THROUGHPUT_TEST
static struct timer_list RX_timer;
#endif

//-------------------------------------------------------------------------------------------------
//  EMAC Function
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_tx (struct sk_buff *skb, struct net_device *dev);
static int MDev_EMAC_SwReset(struct net_device *dev);
static void MDev_EMAC_dts(struct net_device*);
#if EMAC_FLOW_CONTROL_TX
static int _MDrv_EMAC_Pause_TX(struct net_device* emac_dev, struct sk_buff* skb, unsigned char* p_recv);
static void _MDev_EMAC_FlowTX_CB(unsigned long data);
#endif
#if REDUCE_CPU_FOR_RBNA
static void _MDev_EMAC_IntRX_CB(unsigned long data);
#endif // #if REDUCE_CPU_FOR_RBNA

// static void MDev_EMAC_timer_callback( unsigned long value );
// static void MDev_EMAC_timer_LinkStatus(unsigned long data);

static void free_rx_skb(struct emac_handle *hemac)
{
    rx_desc_queue_t* rxinfo = &(hemac->rx_desc_queue);
    int i = 0;
    // unsigned long flags;

    if (NULL == rxinfo->skb_arr)
        return;

    // spin_lock_irqsave(&hemac->mutexRXD, flags);
    for (i = 0; i < rxinfo->num_desc; i ++)
    {
        if (rxinfo->skb_arr[i])
            kfree_skb(rxinfo->skb_arr[i]);
    }
    // spin_unlock_irqrestore(&hemac->mutexRXD, flags);
}

// unsigned long oldTime;
// unsigned long PreLinkStatus;
#if MSTAR_EMAC_NAPI
static int MDev_EMAC_napi_poll(struct napi_struct *napi, int budget);
#endif

#ifdef CONFIG_MSTAR_EEE
static int MDev_EMAC_IS_TX_IDLE(void);
#endif //CONFIG_MSTAR_EEE


//!!!! PACKET_DUMP has not been tested as they are not used. 2016/07/18
#if defined(PACKET_DUMP)
extern struct file* msys_kfile_open(const char* path, int flags, int rights);
extern void msys_kfile_close(struct file* fp);
extern int msys_kfile_write(struct file* fp, unsigned long long offset, unsigned char* data, unsigned int size);

static int txDumpCtrl=0;
static int rxDumpCtrl=0;
static int txDumpFileLength=0;
static int rxDumpFileLength=0;
static char txDumpFileName[32]={0};
static char rxDumpFileName[32]={0};
static struct file* txDumpFile=NULL;
static struct file* rxDumpFile=NULL;

static ssize_t tx_dump_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    txDumpCtrl = simple_strtoul(buf, NULL, 10);
    if(1==txDumpCtrl)
    {
        strcpy(txDumpFileName,"/tmp/emac/tx_dump");
        txDumpFile=msys_kfile_open(rxDumpFileName, O_RDWR | O_CREAT, 0644);
        if(NULL!=txDumpFile)
        {
            txDumpFileLength=0;
            // printk(KERN_WARNING"success to open emac tx_dump file, '%s'...\n",txDumpFileName);
        }
        else
        {
            // printk(KERN_WARNING"failed to open emac tx_dump file, '%s'!!\n",txDumpFileName);
        }
    }
    else if(0==txDumpCtrl && txDumpFile!=NULL)
    {

        msys_kfile_close(txDumpFile);
        txDumpFile=NULL;
    }
    return count;
}
static ssize_t tx_dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{

    return sprintf(buf, "%d\n", txDumpCtrl);
}
DEVICE_ATTR(tx_dump, 0644, tx_dump_show, tx_dump_store);

static ssize_t rx_dump_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{

    rxDumpCtrl = simple_strtoul(buf, NULL, 10);
    if(1==rxDumpCtrl)
    {
        strcpy(rxDumpFileName,"/tmp/emac/rx_dump");
        rxDumpFile=msys_kfile_open(rxDumpFileName, O_RDWR | O_CREAT, 0644);
        if(NULL!=rxDumpFile)
        {
            rxDumpFileLength=0;
            // printk(KERN_WARNING"success to open emac rx_dump file, '%s'...\n",rxDumpFileName);
        }
        else
        {
            // printk(KERN_WARNING"failed to open emac rx_dump file, '%s'!!\n",rxDumpFileName);
        }
    }
    else if(0==rxDumpCtrl)
    {
        if(rxDumpFile!=NULL)
        {
            msys_kfile_close(rxDumpFile);
            rxDumpFile=NULL;
        }
    }
    return count;
}
static ssize_t rx_dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{

    return sprintf(buf, "%d\n", rxDumpCtrl);
}
DEVICE_ATTR(rx_dump, 0644, rx_dump_show, rx_dump_store);
#endif

static unsigned long getCurMs(void)
{
    struct timeval tv;
    unsigned long curMs;

    do_gettimeofday(&tv);
    curMs = tv.tv_usec/1000;
    curMs += tv.tv_sec * 1000;
    return curMs;
}

#if RX_THROUGHPUT_TEST
int receive_bytes = 0;
static void RX_timer_callback( unsigned long value){
    int get_bytes = receive_bytes;
    int cur_speed;
    receive_bytes = 0;

    cur_speed = get_bytes*8/20/1024;
    printk(" %dkbps",cur_speed);
    RX_timer.expires = jiffies + 20*EMAC_CHECK_LINK_TIME;
    add_timer(&RX_timer);
}
#endif

//-------------------------------------------------------------------------------------------------
// skb_queue implementation
//-------------------------------------------------------------------------------------------------
#define SKBQ_SANITY             0
static int skb_queue_create(skb_queue* skb_q, int size, int size1)
{
    if ((NULL == skb_q) || (0 == size))
        return 0;
    // skb_q->size = size + 1;
    skb_q->size[0] = size + 1;
    skb_q->size[1] = size1 + 1;
    if (NULL == (skb_q->skb_info_arr = kzalloc(skb_q->size[1]*sizeof(skb_info), GFP_KERNEL)))
        return 0;
    skb_q->read = skb_q->write = skb_q->rw = 0;
    return 1;
}

static int skb_queue_destroy(skb_queue* skb_q)
{
    int i;
    struct emac_handle *hemac = container_of(skb_q, struct emac_handle, skb_queue_tx);

    if (NULL == skb_q->skb_info_arr)
        return 0;
    for (i = 0; i < skb_q->size[1]; i++)
    {
        if (skb_q->skb_info_arr[i].skb)
        {
            dev_kfree_skb_any(skb_q->skb_info_arr[i].skb);
            hemac->skb_tx_free++;
        }
    }
    kfree(skb_q->skb_info_arr);
    skb_q->skb_info_arr = NULL;
    skb_q->size[0] = skb_q->size[1] = skb_q->read = skb_q->write = skb_q->rw = 0;
    return 1;
}

static int skb_queue_reset(skb_queue* skb_q)
{
    int i;
    struct emac_handle *hemac = container_of(skb_q, struct emac_handle, skb_queue_tx);

    if (NULL == skb_q->skb_info_arr)
        return 0;
    for (i = 0; i < skb_q->size[1]; i++)
    {
        if (skb_q->skb_info_arr[i].skb)
        {
            dev_kfree_skb_any(skb_q->skb_info_arr[i].skb);
            hemac->skb_tx_free++;
        }
    }
    memset(skb_q->skb_info_arr, 0, skb_q->size[1]*sizeof(skb_info));
    skb_q->read = skb_q->write = skb_q->rw = 0;
    return 1;
}

#define QUEUE_USED(size, read, write)           ((write) >= (read))?  ((write) - (read)) : ((size) - (read) + (write))
#define QUEUE_FREE(size, read, write)           ((write) >= (read))?  ((size) - (write) + (read) - 1) : ((read) - (write) - 1)
/*
static int skb_queue_used(skb_queue* skb_q, int idx, int idx_size)
{
#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
#endif
    if (2 == idx)
        return QUEUE_USED(skb_q->size[1], skb_q->rw, skb_q->write);
    return (0 == idx) ?
        QUEUE_USED(skb_q->size[idx_size], skb_q->read, skb_q->rw) :
        QUEUE_USED(skb_q->size[idx_size], skb_q->read, skb_q->write);
}
*/
inline static int skb_queue_used(skb_queue* skb_q, int idx)
{
#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
#endif
    if (2 == idx)
        return QUEUE_USED(skb_q->size[1], skb_q->rw, skb_q->write);
    return (0 == idx) ?
        QUEUE_USED(skb_q->size[1], skb_q->read, skb_q->rw) :
        QUEUE_USED(skb_q->size[1], skb_q->read, skb_q->write);
}

inline static int skb_queue_free(skb_queue* skb_q, int idx)
{
#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
#endif
    // return skb_q->size[idx] - skb_queue_used(skb_q, idx) - 1;
    if (2 == idx)
        return QUEUE_FREE(skb_q->size[1], skb_q->rw, skb_q->write);
    return (0 == idx) ?
        QUEUE_FREE(skb_q->size[1], skb_q->read, skb_q->rw) :
        QUEUE_FREE(skb_q->size[1], skb_q->read, skb_q->write);
}

inline static int skb_queue_remove(skb_queue* skb_q, struct sk_buff** pskb, dma_addr_t* pphys, int bSkbFree, int idx)
{
    skb_info* pskb_info;
    int read;
    int len;
    struct emac_handle *hemac = container_of(skb_q, struct emac_handle, skb_queue_tx);

#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
    if (0 == skb_queue_used(skb_q, idx))
    {
        printk("[%s][%d] why\n", __FUNCTION__, __LINE__);
        return 0;
    }
#endif
    read = skb_q->read;
    pskb_info = &(skb_q->skb_info_arr[read]);
#if SKBQ_SANITY
    if ((!pskb_info->skb) && (!pskb_info->skb_phys))
    {
        printk("[%s][%d] strange remove\n", __FUNCTION__, __LINE__);
        return -1;
    }
#endif
    len = pskb_info->skb_len;

    // printk("[%s][%d] (skb, addr, len) = (0x%08x, 0x%08x, %d)\n", __FUNCTION__, __LINE__, (int)pskb_info->skb, VIRT2BUS(pskb_info->skb->data), len);

    hemac->skb_tx_free++;
    if (bSkbFree)
    {
        if (pskb_info->skb)
        {
            if (0xFFFFFFFF == (int)pskb_info->skb)
            {
                void* p = BUS2VIRT(pskb_info->skb_phys);
                kfree(p);
            }
            else
            {
                dev_kfree_skb_any(pskb_info->skb);
            }
            pskb_info->skb = NULL;
            // hemac->skb_tx_free++;
        }
    }
    else
    {
        *pskb = pskb_info->skb;
        *pphys = pskb_info->skb_phys;
    }
    pskb_info->skb_phys = 0;
    pskb_info->skb_len = 0;

    skb_q->read++;
    if (skb_q->read >= skb_q->size[1])
        skb_q->read -= skb_q->size[1];
    return len;
}

inline static int skb_queue_insert(skb_queue* skb_q, struct sk_buff* skb, dma_addr_t phys, int skb_len, int idx)
{
    skb_info* pskb_info;
    int* pwrite = NULL;
    struct emac_handle *hemac = container_of(skb_q, struct emac_handle, skb_queue_tx);

#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
    if (0 == skb_queue_free(skb_q, idx))
    {
        printk("[%s][%d] why\n", __FUNCTION__, __LINE__);
        return 0;
    }
#endif
    pwrite = (0 == idx) ? &skb_q->rw : &skb_q->write;
    pskb_info = &(skb_q->skb_info_arr[*pwrite]);
    // if ((pskb_info->used) || (pskb_info->skb))
    // if (pskb_info->used)
#if SKBQ_SANITY
    if ((pskb_info->skb) || (pskb_info->skb_phys))
    {
        printk("[%s][%d] strange insert\n", __FUNCTION__, __LINE__);
        return -1;
    }
#endif
    // if (skb)
        hemac->skb_tx_send++;
    // pskb_info->used = 1;
    pskb_info->skb_phys = phys;
    pskb_info->skb_len = skb_len;
    pskb_info->skb = skb;
    (*pwrite)++;
    if (*pwrite >= skb_q->size[1])
        (*pwrite) -= skb_q->size[1];
    return 1;
}

inline static int skb_queue_head_inc(skb_queue* skb_q, struct sk_buff** skb, dma_addr_t* pphys, int* plen, int idx)
{
    skb_info* pskb_info;
    int* pwrite = NULL;
#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
    if (0 == skb_queue_free(skb_q, idx))
    {
        printk("[%s][%d] why\n", __FUNCTION__, __LINE__);
        return 0;
    }
#endif
    *skb = NULL;
    *pphys = 0;
    pwrite = (0 == idx) ? &skb_q->rw : &skb_q->write;
    pskb_info = &(skb_q->skb_info_arr[*pwrite]);
#if SKBQ_SANITY
    // if ((!pskb_info->skb) || (!pskb_info->skb_phys))
    if ((!pskb_info->skb_phys))
    {
        printk("[%s][%d] strange head inc\n", __FUNCTION__, __LINE__);
        return 0;
    }
#endif
    *skb = pskb_info->skb;
    *pphys = pskb_info->skb_phys;
    *plen = pskb_info->skb_len;
    (*pwrite)++;
    if (*pwrite >= skb_q->size[1])
        (*pwrite) -= skb_q->size[1];
    return 1;
}

/*
static int skb_queue_emtpy(skb_queue* skb_q)
{
    return (skb_queue_used(skb_q))? 0 : 1;
}
*/

inline static int skb_queue_full(skb_queue* skb_q, int idx)
{
    return (skb_queue_free(skb_q, idx))? 0 : 1;
}

/*
static int skb_queue_size(skb_queue* skb_q, int idx)
{
    return skb_q->size[idx] - 1;
}
*/

//-------------------------------------------------------------------------------------------------
// PHY MANAGEMENT
//-------------------------------------------------------------------------------------------------

#if 0
//-------------------------------------------------------------------------------------------------
// Access the PHY to determine the current Link speed and Mode, and update the
// MAC accordingly.
// If no link or auto-negotiation is busy, then no changes are made.
// Returns:  0 : OK
//              -1 : No link
//              -2 : AutoNegotiation still in progress
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_update_linkspeed (struct net_device *dev)
{
    u32 bmsr, bmcr, adv, lpa, neg;
    u32 speed, duplex;
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);

#ifdef CONFIG_EMAC_PHY_RESTART_AN
        u32 hcd_link_st_ok, an_100t_link_st = 0;
        static unsigned int phy_restart_cnt = 0;
        u32 an_state = 0;
        u32 an_state2 = 0;
        u32 an_state3 = 0;
#endif /* CONFIG_EMAC_PHY_RESTART_AN */

    // Link status is latched, so read twice to get current value //
    MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, MII_BMSR, &bmsr);
    MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, MII_BMSR, &bmsr);
    if (!(bmsr & BMSR_LSTATUS)){
    #ifdef CONFIG_MSTAR_EEE
        MHal_EMAC_Disable_EEE();

        if (hemac->PreLinkStatus == 1)
        {
            MHal_EMAC_Reset_EEE();
        }
    #endif

        hemac->PreLinkStatus = 0;

        return -1;          //no link //
    }

    MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, MII_BMCR, &bmcr);

    if (bmcr & BMCR_ANENABLE)
    {               //AutoNegotiation is enabled //
        if (!(bmsr & BMSR_ANEGCOMPLETE))
        {
            //EMAC_DBG("==> AutoNegotiation still in progress\n");
            return -2;
        }

        /* Get Link partner and advertisement from the PHY not from the MAC */
        MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, MII_ADVERTISE, &adv);
        MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, MII_LPA, &lpa);

        /* For Link Parterner adopts force mode and EPHY used,
         * EPHY LPA reveals all zero value.
         * EPHY would be forced to Full-Duplex mode.
         */
        if (!lpa)
        {
            /* 100Mbps Full-Duplex */
            if (bmcr & BMCR_SPEED100)
                lpa |= LPA_100FULL;
            else /* 10Mbps Full-Duplex */
                lpa |= LPA_10FULL;
        }

        neg = adv & lpa;

        if (neg & LPA_100FULL)
        {
            speed = SPEED_100;
            duplex = DUPLEX_FULL;
        }
        else if (neg & LPA_100HALF)
        {
            speed = SPEED_100;
            duplex = DUPLEX_HALF;
        }
        else if (neg & LPA_10FULL)
        {
            speed = SPEED_10;
            duplex = DUPLEX_FULL;
        }
        else if (neg & LPA_10HALF)
        {
            speed = SPEED_10;
            duplex = DUPLEX_HALF;
        }
        else
        {
            speed = SPEED_10;
            duplex = DUPLEX_HALF;
            EMAC_DBG("%s: No speed and mode found (LPA=0x%x, ADV=0x%x)\n", __FUNCTION__, lpa, adv);
        }

    }
    else
    {
        speed = (bmcr & BMCR_SPEED100) ? SPEED_100 : SPEED_10;
        duplex = (bmcr & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
    }

    // Update the MAC //
    MHal_EMAC_update_speed_duplex(hemac->hal, speed,duplex);

#ifdef CONFIG_MSTAR_EEE
    /*TX idle, enable EEE*/
    if((MDev_EMAC_IS_TX_IDLE()) && (speed == SPEED_100) && (duplex == DUPLEX_FULL))
    {
        if (hemac->PreLinkStatus == 0)
        {
            MHal_EMAC_Enable_EEE(300);
        }
        else
        {
            MHal_EMAC_Enable_EEE(0);
        }
    }
#endif

    hemac->PreLinkStatus = 1;
#ifdef CONFIG_EMAC_PHY_RESTART_AN
        if (speed == SPEED_100) {
            MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, 0x21, &hcd_link_st_ok);
            MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, 0x22, &an_100t_link_st);
            if ( (!(hcd_link_st_ok & 0x100) && ((an_100t_link_st & 0x300) == 0x200))) {
                phy_restart_cnt++;
                hemac->gu32PhyResetCount1++;
                EMAC_ERR("hcd_link_st_ok:0x%x, an_100t_link_st:0x%x\n", hcd_link_st_ok, an_100t_link_st);
                if (phy_restart_cnt > 10) {
                    EMAC_DBG("MDev_EMAC_update_linkspeed: restart AN process\n");
                    MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1200UL);
                    hemac->gu32PhyResetCount++;
                    phy_restart_cnt = 0;
                    return 0;
                }
            }
            else if (((hcd_link_st_ok & 0x100) && !(an_100t_link_st & 0x300)) ) {
                phy_restart_cnt++;
                hemac->gu32PhyResetCount2++;
                EMAC_ERR("hcd_link_st_ok:0x%x, an_100t_link_st:0x%x\n", hcd_link_st_ok, an_100t_link_st);
                if (phy_restart_cnt > 10) {
                    EMAC_DBG("MDev_EMAC_update_linkspeed: restart AN process\n");
                    MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1200UL);
                    hemac->gu32PhyResetCount++;
                    phy_restart_cnt = 0;
                    return 0;
                }
            }

            /* Monitor AN state*/
            MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, 0x2e, &an_state);
            MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, 0x2e, &an_state2);
            MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, 0x2e, &an_state3);
            if ((an_state != an_state2) || (an_state != an_state3))
            {
                EMAC_ERR("an_state 1:0x%x, 2:0x%x, 3:0x%x\n", an_state, an_state2, an_state3);
                return 0;
            }

            if ((an_state & 0xf000) == 0x3000)
            {
                EMAC_ERR("an_state=0x%x\n", an_state);
                phy_restart_cnt++;
                hemac->gu32PhyResetCount3++;
                if (phy_restart_cnt > 10){
                    EMAC_DBG("PHY_AN_monitor_timer_callback: restart AN process\n");
                    MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1200UL);
                    hemac->gu32PhyResetCount++;
                    phy_restart_cnt = 0;
                    return 0;
                }
            }
            else if ((an_state & 0xf000) == 0x2000)
            {
                EMAC_ERR("an_state=0x%x\n", an_state);
                phy_restart_cnt++;
                hemac->gu32PhyResetCount4++;
                if (phy_restart_cnt > 10){
                    EMAC_DBG("PHY_AN_monitor_timer_callback: restart AN process\n");
                    MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1200UL);
                    hemac->gu32PhyResetCount++;
                    phy_restart_cnt = 0;
                    return 0;
                }
            }
            // else{
            //     phy_restart_cnt = 0;
            // }
        }
#endif /* CONFIG_EMAC_PHY_RESTART_AN */

    return 0;
}
#endif

#if 0
static int MDev_EMAC_get_info(struct net_device *dev)
{
    u32 bmsr, bmcr, LocPtrA;
    u32 uRegStatus =0;
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);

    // Link status is latched, so read twice to get current value //
    MHal_EMAC_read_phy (hemac->hal, hemac->phyaddr, MII_BMSR, &bmsr);
    MHal_EMAC_read_phy (hemac->hal, hemac->phyaddr, MII_BMSR, &bmsr);
    if (!(bmsr & BMSR_LSTATUS)){
        uRegStatus &= ~ETHERNET_TEST_RESET_STATE;
        uRegStatus |= ETHERNET_TEST_NO_LINK; //no link //
    }
    MHal_EMAC_read_phy (hemac->hal, hemac->phyaddr, MII_BMCR, &bmcr);

    if (bmcr & BMCR_ANENABLE)
    {
        //AutoNegotiation is enabled //
        if (!(bmsr & BMSR_ANEGCOMPLETE))
        {
            uRegStatus &= ~ETHERNET_TEST_RESET_STATE;
            uRegStatus |= ETHERNET_TEST_AUTO_NEGOTIATION; //AutoNegotiation //
        }
        else
        {
            uRegStatus &= ~ETHERNET_TEST_RESET_STATE;
            uRegStatus |= ETHERNET_TEST_LINK_SUCCESS; //link success //
        }

        MHal_EMAC_read_phy (hemac->hal, hemac->phyaddr, MII_LPA, &LocPtrA);
        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_100HALF))
        {
            uRegStatus |= ETHERNET_TEST_SPEED_100M; //SPEED_100//
        }
        else
        {
            uRegStatus &= ~ETHERNET_TEST_SPEED_100M; //SPEED_10//
        }

        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_10FULL))
        {
            uRegStatus |= ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_FULL//
        }
        else
        {
            uRegStatus &= ~ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_HALF//
        }
    }
    else
    {
        if(bmcr & BMCR_SPEED100)
        {
            uRegStatus |= ETHERNET_TEST_SPEED_100M; //SPEED_100//
        }
        else
        {
            uRegStatus &= ~ETHERNET_TEST_SPEED_100M; //SPEED_10//
        }

        if(bmcr & BMCR_FULLDPLX)
        {
            uRegStatus |= ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_FULL//
        }
        else
        {
            uRegStatus &= ~ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_HALF//
        }
    }

    return uRegStatus;
}
#endif

//-------------------------------------------------------------------------------------------------
//Program the hardware MAC address from dev->dev_addr.
//-------------------------------------------------------------------------------------------------
void MDev_EMAC_update_mac_address (struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 value;
    value = (dev->dev_addr[3] << 24) | (dev->dev_addr[2] << 16) | (dev->dev_addr[1] << 8) |(dev->dev_addr[0]);
    MHal_EMAC_Write_SA1L(hemac->hal, value);
    value = (dev->dev_addr[5] << 8) | (dev->dev_addr[4]);
    MHal_EMAC_Write_SA1H(hemac->hal, value);
}

//-------------------------------------------------------------------------------------------------
// ADDRESS MANAGEMENT
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Set the ethernet MAC address in dev->dev_addr
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_get_mac_address (struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    char addr[6];
    u32 HiAddr, LoAddr;

    // Check if bootloader set address in Specific-Address 1 //
    HiAddr = MHal_EMAC_get_SA1H_addr(hemac->hal);
    LoAddr = MHal_EMAC_get_SA1L_addr(hemac->hal);

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
    HiAddr = MHal_EMAC_get_SA2H_addr(hemac->hal);
    LoAddr = MHal_EMAC_get_SA2L_addr(hemac->hal);
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

#ifdef URANUS_ETHER_ADDR_CONFIGURABLE
//-------------------------------------------------------------------------------------------------
// Store the new hardware address in dev->dev_addr, and update the MAC.
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_set_mac_address (struct net_device *dev, void *addr)
{
    struct sockaddr *address = addr;
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);

    if (!is_valid_ether_addr (address->sa_data))
        return -EADDRNOTAVAIL;
    spin_lock(&hemac->mutexPhy);
    memcpy (dev->dev_addr, address->sa_data, dev->addr_len);
    MDev_EMAC_update_mac_address (dev);
    spin_unlock(&hemac->mutexPhy);
    return 0;
}
#endif

//-------------------------------------------------------------------------------------------------
// Mstar Multicast hash rule
//-------------------------------------------------------------------------------------------------
//Hash_index[5] = da[5] ^ da[11] ^ da[17] ^ da[23] ^ da[29] ^ da[35] ^ da[41] ^ da[47]
//Hash_index[4] = da[4] ^ da[10] ^ da[16] ^ da[22] ^ da[28] ^ da[34] ^ da[40] ^ da[46]
//Hash_index[3] = da[3] ^ da[09] ^ da[15] ^ da[21] ^ da[27] ^ da[33] ^ da[39] ^ da[45]
//Hash_index[2] = da[2] ^ da[08] ^ da[14] ^ da[20] ^ da[26] ^ da[32] ^ da[38] ^ da[44]
//Hash_index[1] = da[1] ^ da[07] ^ da[13] ^ da[19] ^ da[25] ^ da[31] ^ da[37] ^ da[43]
//Hash_index[0] = da[0] ^ da[06] ^ da[12] ^ da[18] ^ da[24] ^ da[30] ^ da[36] ^ da[42]
//-------------------------------------------------------------------------------------------------

static void MDev_EMAC_sethashtable(struct net_device *dev, unsigned char *addr)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 mc_filter[2];
    u32 uHashIdxBit;
    u32 uHashValue;
    u32 i;
    u32 tmpcrc;
    u32 uSubIdx;
    u64 macaddr;
    u64 mac[6];

    uHashValue = 0;
    macaddr = 0;

    // Restore mac //
    for(i = 0; i < 6;  i++)
    {
        mac[i] =(u64)addr[i];
    }

    // Truncate mac to u64 container //
    macaddr |=  mac[0] | (mac[1] << 8) | (mac[2] << 16);
    macaddr |=  (mac[3] << 24) | (mac[4] << 32) | (mac[5] << 40);

    // Caculate the hash value //
    for(uHashIdxBit = 0; uHashIdxBit < 6;  uHashIdxBit++)
    {
        tmpcrc = (macaddr & (0x1UL << uHashIdxBit)) >> uHashIdxBit;
        for(i = 1; i < 8;  i++)
        {
            uSubIdx = uHashIdxBit + (i * 6);
            tmpcrc = tmpcrc ^ ((macaddr >> uSubIdx) & 0x1);
        }
        uHashValue |= (tmpcrc << uHashIdxBit);
    }

    mc_filter[0] = MHal_EMAC_ReadReg32(hemac->hal, REG_ETH_HSL);
    mc_filter[1] = MHal_EMAC_ReadReg32(hemac->hal, REG_ETH_HSH);

    // Set the corrsponding bit according to the hash value //
    if(uHashValue < 32)
    {
        mc_filter[0] |= (0x1UL <<  uHashValue);
        MHal_EMAC_WritReg32(hemac->hal, REG_ETH_HSL, mc_filter[0] );
    }
    else
    {
        mc_filter[1] |= (0x1UL <<  (uHashValue - 32));
        MHal_EMAC_WritReg32(hemac->hal, REG_ETH_HSH, mc_filter[1] );
    }
}

//-------------------------------------------------------------------------------------------------
//Enable/Disable promiscuous and multicast modes.
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_set_rx_mode (struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 uRegVal;
    struct netdev_hw_addr *ha;

    uRegVal = MHal_EMAC_Read_CFG(hemac->hal);

    if (dev->flags & IFF_PROMISC)
    {
        // Enable promiscuous mode //
        uRegVal |= EMAC_CAF;
    }
    else if (dev->flags & (~IFF_PROMISC))
    {
        // Disable promiscuous mode //
        uRegVal &= ~EMAC_CAF;
    }
    MHal_EMAC_Write_CFG(hemac->hal, uRegVal);

    if (dev->flags & IFF_ALLMULTI)
    {
        // Enable all multicast mode //
        MHal_EMAC_update_HSH(hemac->hal, -1,-1);
        uRegVal |= EMAC_MTI;
    }
    else if (dev->flags & IFF_MULTICAST)
    {
        // Enable specific multicasts//
        MHal_EMAC_update_HSH(hemac->hal, 0,0);
        netdev_for_each_mc_addr(ha, dev)
        {
            MDev_EMAC_sethashtable(dev, ha->addr);
        }
        uRegVal |= EMAC_MTI;
    }
    else if (dev->flags & ~(IFF_ALLMULTI | IFF_MULTICAST))
    {
        // Disable all multicast mode//
        MHal_EMAC_update_HSH(hemac->hal, 0,0);
        uRegVal &= ~EMAC_MTI;
    }

    MHal_EMAC_Write_CFG(hemac->hal, uRegVal);
}
//-------------------------------------------------------------------------------------------------
// IOCTL
//-------------------------------------------------------------------------------------------------
#if 0
//-------------------------------------------------------------------------------------------------
// Enable/Disable MDIO
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_mdio_read (struct net_device *dev, int phy_id, int location)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 value;
    MHal_EMAC_read_phy (hemac->hal, phy_id, location, &value);
    return value;
}

static void MDev_EMAC_mdio_write (struct net_device *dev, int phy_id, int location, int value)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    MHal_EMAC_write_phy (hemac->hal, phy_id, location, value);
}
#endif

//-------------------------------------------------------------------------------------------------
//ethtool support.
//-------------------------------------------------------------------------------------------------
#if 0
static int MDev_EMAC_ethtool_ioctl (struct net_device *dev, void *useraddr)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 ethcmd;
    int res = 0;

    if (copy_from_user (&ethcmd, useraddr, sizeof (ethcmd)))
        return -EFAULT;

    switch (ethcmd)
    {
        case ETHTOOL_GSET:
        {
            struct ethtool_cmd ecmd = { ETHTOOL_GSET };
            res = mii_ethtool_gset (&hemac->mii, &ecmd);
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
                res = mii_ethtool_sset (&hemac->mii, &ecmd);
            break;
        }
        case ETHTOOL_NWAY_RST:
        {
            res = mii_nway_restart (&hemac->mii);
            break;
        }
        case ETHTOOL_GLINK:
        {
            struct ethtool_value edata = { ETHTOOL_GLINK };
            edata.data = mii_link_ok (&hemac->mii);
            if (copy_to_user (useraddr, &edata, sizeof (edata)))
                res = -EFAULT;
            break;
        }
        default:
            res = -EOPNOTSUPP;
    }
    return res;
}
#endif

//-------------------------------------------------------------------------------------------------
// User-space ioctl interface.
//-------------------------------------------------------------------------------------------------
#if 0
static int MDev_EMAC_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    struct mii_ioctl_data *data = if_mii(rq);
    u32 value;
    if (!netif_running(dev))
    {
        rq->ifr_metric = ETHERNET_TEST_INIT_FAIL;
    }

    switch (cmd)
    {
        case SIOCGMIIPHY:
            data->phy_id = (hemac->phyaddr & 0x1FUL);
            return 0;

        case SIOCDEVPRIVATE:
            rq->ifr_metric = (MDev_EMAC_get_info(dev)|hemac->initstate);
            return 0;
/*
        case SIOCDEVON:
            MHal_EMAC_Power_On_Clk();
            return 0;

        case SIOCDEVOFF:
            MHal_EMAC_Power_Off_Clk();
            return 0;
*/
        case SIOCGMIIREG:
            // check PHY's register 1.
            if((data->reg_num & 0x1fUL) == 0x1UL)
            {
                // PHY's register 1 value is set by timer callback function.
                spin_lock(&hemac->mutexPhy);
                data->val_out = hemac->phy_status_register;
                spin_unlock(&hemac->mutexPhy);
            }
            else
            {
                MHal_EMAC_read_phy(hemac->hal, (hemac->phyaddr & 0x1FUL), (data->reg_num & 0x1fUL), (u32 *)&(value));
                data->val_out = value;
            }
            return 0;

        case SIOCSMIIREG:
            if (!capable(CAP_NET_ADMIN))
                return -EPERM;
            MHal_EMAC_write_phy(hemac->hal, (hemac->phyaddr & 0x1FUL), (data->reg_num & 0x1fUL), data->val_in);
            return 0;

        case SIOCETHTOOL:
            return MDev_EMAC_ethtool_ioctl (dev, (void *) rq->ifr_data);

        default:
            return -EOPNOTSUPP;
    }
}
#else
static int MDev_EMAC_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
    switch (cmd)
    {
    case SIOCGMIIPHY:
    case SIOCGMIIREG:
    case SIOCSMIIREG:
        return phy_mii_ioctl(dev->phydev, rq, cmd);
    default:
        break;
    }
    return -EOPNOTSUPP;
}
#endif

//-------------------------------------------------------------------------------------------------
// MAC
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//Initialize and start the Receiver and Transmit subsystems
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_start (struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 uRegVal;

    // Enable Receive and Transmit //
    uRegVal = MHal_EMAC_Read_CTL(hemac->hal);
    uRegVal |= (EMAC_RE | EMAC_TE);
    MHal_EMAC_Write_CTL(hemac->hal, uRegVal);
}

//-------------------------------------------------------------------------------------------------
// Open the ethernet interface
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_open (struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 uRegVal;
    // unsigned long flags;

#if MSTAR_EMAC_NAPI
    napi_enable(&hemac->napi);
#endif

    spin_lock(&hemac->mutexPhy);
    if (!is_valid_ether_addr (dev->dev_addr))
    {
       spin_unlock(&hemac->mutexPhy);
       return -EADDRNOTAVAIL;
    }
    spin_unlock(&hemac->mutexPhy);

#ifdef TX_SW_QUEUE
    _MDev_EMAC_tx_reset_TX_SW_QUEUE(dev);
#endif
    //ato  EMAC_SYS->PMC_PCER = 1 << EMAC_ID_EMAC;   //Re-enable Peripheral clock //
    //MHal_EMAC_Power_On_Clk(dev->dev);
    uRegVal = MHal_EMAC_Read_CTL(hemac->hal);
    uRegVal |= EMAC_CSR;
    MHal_EMAC_Write_CTL(hemac->hal, uRegVal);
    // Enable PHY interrupt //
    MHal_EMAC_enable_phyirq(hemac->hal);

    MHal_EMAC_IntEnable(hemac->hal, 0xFFFFFFFF, 0);
    hemac->gu32intrEnable = EMAC_INT_RBNA|EMAC_INT_TUND|EMAC_INT_RTRY|EMAC_INT_ROVR|EMAC_INT_HRESP;
#if !DYNAMIC_INT_TX
    hemac->gu32intrEnable |= EMAC_INT_TCOM;
#endif
    hemac->gu32intrEnable |= EMAC_INT_RCOM;
    MHal_EMAC_IntEnable(hemac->hal, hemac->gu32intrEnable, 1);

    hemac->ep_flag |= EP_FLAG_OPEND;

    MDev_EMAC_start(dev);
    phy_start(dev->phydev);
    netif_start_queue (dev);

    // init_timer( &hemac->timer_link );

#if 0
    hemac->timer_link.data = (unsigned long)dev;
    hemac->timer_link.function = MDev_EMAC_timer_LinkStatus;

    hemac->timer_link.expires = jiffies + EMAC_CHECK_LINK_TIME;
    add_timer(&hemac->timer_link);
#endif

#if 0
#ifdef CONFIG_EMAC_PHY_RESTART_AN
    // MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1200UL);
    // MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1000UL);
    //MHal_EMAC_write_phy(hemac->phyaddr, MII_BMCR, 0x1200UL);
    MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART);
#else
    MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x9000UL);
    MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1000UL);
#endif /* CONFIG_EMAC_PHY_RESTART_AN */
#endif

#ifdef CONFIG_EMAC_PHY_RESTART_AN
    if(hemac->phy_mode != PHY_INTERFACE_MODE_RMII)
        MHal_EMAC_Phy_Restart_An(hemac->hal);
#endif

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Close the interface
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_close (struct net_device *dev)
{
    u32 uRegVal;
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    unsigned long flags;

#if MSTAR_EMAC_NAPI
    napi_disable(&hemac->napi);
#endif

    spin_lock(&hemac->mutexPhy);
    //Disable Receiver and Transmitter //
    uRegVal = MHal_EMAC_Read_CTL(hemac->hal);
    uRegVal &= ~(EMAC_TE | EMAC_RE);
    MHal_EMAC_Write_CTL(hemac->hal, uRegVal);
    // Disable PHY interrupt //
    MHal_EMAC_disable_phyirq(hemac->hal);
    spin_unlock(&hemac->mutexPhy);

    MHal_EMAC_IntEnable(hemac->hal, 0xFFFFFFFF, 0);
    netif_stop_queue (dev);
    netif_carrier_off(dev);
    phy_stop(dev->phydev);
    // del_timer(&hemac->timer_link);
    //MHal_EMAC_Power_Off_Clk(dev->dev);
    // hemac->ThisBCE.connected = 0;
    hemac->ep_flag &= (~EP_FLAG_OPEND);
    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    skb_queue_reset(&(hemac->skb_queue_tx));
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Update the current statistics from the internal statistics registers.
//-------------------------------------------------------------------------------------------------
static struct net_device_stats * MDev_EMAC_stats (struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    int ale, lenerr, seqe, lcol, ecol;

    // spin_lock_irq (hemac->lock);

    if (netif_running (dev))
    {
        hemac->stats.rx_packets += MHal_EMAC_Read_OK(hemac->hal);            /* Good frames received */
        ale = MHal_EMAC_Read_ALE(hemac->hal);
        hemac->stats.rx_frame_errors += ale;                       /* Alignment errors */
        lenerr = MHal_EMAC_Read_ELR(hemac->hal);
        hemac->stats.rx_length_errors += lenerr;                   /* Excessive Length or Undersize Frame error */
        seqe = MHal_EMAC_Read_SEQE(hemac->hal);
        hemac->stats.rx_crc_errors += seqe;                        /* CRC error */
        hemac->stats.rx_fifo_errors += MHal_EMAC_Read_ROVR(hemac->hal);
        hemac->stats.rx_errors += ale + lenerr + seqe + MHal_EMAC_Read_SE(hemac->hal) + MHal_EMAC_Read_RJB(hemac->hal);
        hemac->stats.tx_packets += MHal_EMAC_Read_FRA(hemac->hal);           /* Frames successfully transmitted */
        hemac->stats.tx_fifo_errors += MHal_EMAC_Read_TUE(hemac->hal);       /* Transmit FIFO underruns */
        hemac->stats.tx_carrier_errors += MHal_EMAC_Read_CSE(hemac->hal);    /* Carrier Sense errors */
        hemac->stats.tx_heartbeat_errors += MHal_EMAC_Read_SQEE(hemac->hal); /* Heartbeat error */
        lcol = MHal_EMAC_Read_LCOL(hemac->hal);
        ecol = MHal_EMAC_Read_ECOL(hemac->hal);
        hemac->stats.tx_window_errors += lcol;                     /* Late collisions */
        hemac->stats.tx_aborted_errors += ecol;                    /* 16 collisions */
        hemac->stats.collisions += MHal_EMAC_Read_SCOL(hemac->hal) + MHal_EMAC_Read_MCOL(hemac->hal) + lcol + ecol;
    }

    // spin_unlock_irq (hemac->lock);

    return &hemac->stats;
}

static int MDev_EMAC_TxReset(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 val = MHal_EMAC_Read_CTL(hemac->hal) & 0x000001FFUL;

    MHal_EMAC_Write_CTL(hemac->hal, (val & ~EMAC_TE));

    val = MHal_EMAC_Read_CTL(hemac->hal) & 0x000001FFUL;
    EMAC_ERR ("MAC0_CTL:0x%08x\n", val);
    //MHal_EMAC_Write_TCR(0);
    mdelay(1);
    MHal_EMAC_Write_CTL(hemac->hal, (MHal_EMAC_Read_CTL(hemac->hal) | EMAC_TE));
    val = MHal_EMAC_Read_CTL(hemac->hal) & 0x000001FFUL;
    EMAC_ERR ("MAC0_CTL:0x%08x\n", val);
    return 0;
}

#ifdef CONFIG_MSTAR_EEE
static int MDev_EMAC_IS_TX_IDLE(void)
{
    u32 check;
    u32 tsrval = 0;

    u8  avlfifo[8] = {0};
    u8  avlfifoidx;
    u8  avlfifoval = 0;

#ifdef TX_QUEUE_4
    for (check = 0; check < EMAC_CHECK_CNT; check++)
    {
        tsrval = MHal_EMAC_Read_TSR();

        avlfifo[0] = ((tsrval & EMAC_IDLETSR) != 0)? 1 : 0;
        avlfifo[1] = ((tsrval & EMAC_BNQ)!= 0)? 1 : 0;
        avlfifo[2] = ((tsrval & EMAC_TBNQ) != 0)? 1 : 0;
        avlfifo[3] = ((tsrval & EMAC_FBNQ) != 0)? 1 : 0;
        avlfifo[4] = ((tsrval & EMAC_FIFO1IDLE) !=0)? 1 : 0;
        avlfifo[5] = ((tsrval & EMAC_FIFO2IDLE) != 0)? 1 : 0;
        avlfifo[6] = ((tsrval & EMAC_FIFO3IDLE) != 0)? 1 : 0;
        avlfifo[7] = ((tsrval & EMAC_FIFO4IDLE) != 0)? 1 : 0;

        avlfifoval = 0;

        for(avlfifoidx = 0; avlfifoidx < 8; avlfifoidx++)
        {
            avlfifoval += avlfifo[avlfifoidx];
        }

        if (avlfifoval == 8)
            return 1;
    }
#endif

    return 0;
}
#endif //CONFIG_MSTAR_EEE


#if 0
void MDrv_EMAC_DumpMem(phys_addr_t addr, u32 len)
{
    u8 *ptr = (u8 *)addr;
    u32 i;

    printk("\n ===== Dump %lx =====\n", (long unsigned int)ptr);
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
#endif

#if EMAC_FLOW_CONTROL_RX
static void _MDrv_EMAC_PausePkt_Send(struct net_device* dev)
{
    unsigned long flags;
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 val = MHal_EMAC_Read_CTL(hemac->hal) & 0x000001FFUL;

    //Disable Rx
    MHal_EMAC_Write_CTL(hemac->hal, (val & ~EMAC_RE)); // why RX
    memcpy(&hemac->pu8PausePkt[6], dev->dev_addr, 6);
    Chip_Flush_Cache_Range((size_t)hemac->pu8PausePkt, hemac->u8PausePktSize);
    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    hemac->isPausePkt = 1;
    // skb_queue_insert(&(hemac->skb_queue_tx), NULL, VIRT2BUS(hemac->pu8PausePkt), hemac->u8PausePktSize, 1);
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
    MHal_EMAC_Write_CTL(hemac->hal, (MHal_EMAC_Read_CTL(hemac->hal) | EMAC_RE)); // why RX
}
#endif // #if EMAC_FLOW_CONTROL_RX

//-------------------------------------------------------------------------------------------------
//Patch for losing small-size packet when running SMARTBIT
//-------------------------------------------------------------------------------------------------
#ifdef CONFIG_MP_ETHERNET_MSTAR_ICMP_ENHANCE
static void MDev_EMAC_Period_Retry(struct sk_buff *skb, struct net_device* dev)
{
    u32 xval;
    u32 uRegVal;

    xval = MHal_EMAC_ReadReg32(hemac->hal,REG_ETH_CFG);

    if((skb->len <= PACKET_THRESHOLD) && !(xval & EMAC_SPD) && !(xval & EMAC_FD))
    {
        txcount++;
    }
    else
    {
        txcount = 0;
    }

    if(txcount > TXCOUNT_THRESHOLD)
    {
        uRegVal  = MHal_EMAC_Read_CFG(hemac->hal);
        uRegVal  |= 0x00001000UL;
        MHal_EMAC_Write_CFG(hemac->hal, uRegVal);
    }
    else
    {
        uRegVal = MHal_EMAC_Read_CFG(hemac->hal);
        uRegVal &= ~(0x00001000UL);
        MHal_EMAC_Write_CFG(hemac->hal, uRegVal);
    }
}
#endif

#if 0
static int _MDev_EMAC_tx_free(struct emac_handle *hemac)
{
    // int pkt_num = 0;
    // int byte_num = 0;
    int txUsedCnt;
    int txUsedCntSW;
    int i;
    unsigned long flags;
    int len;
    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    txUsedCnt = MHal_EMAC_TXQ_Used(hemac->hal);
    txUsedCntSW = skb_queue_used(&hemac->skb_queue_tx, 0);
    if (txUsedCntSW < txUsedCnt)
    {
        spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
        return 0;
    }
    for (i = txUsedCnt; i < txUsedCntSW; i++)
    {
        len = skb_queue_remove(&hemac->skb_queue_tx, NULL, NULL, 1, 0);
        // if (len < 0)
            // return 0;
        hemac->stats.tx_bytes += len;
        // pkt_num++;
        // byte_num+= len;
        tx_bytes_per_timer += len;
        // skb_tx_free++;
    }
/*
    if (pkt_num)
    {
        // netdev_completed_queue(dev, pkt_num, byte_num);
    }
*/
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
    return 1;
}
#else
static int _MDev_EMAC_tx_pump(struct emac_handle *hemac, int bFree, int bPump)
{
    int txUsedCnt;
    int txUsedCntSW;
    int i;
    unsigned long flags;
    unsigned long flags1;
    int len;
    struct sk_buff* skb = NULL;
    dma_addr_t skb_addr;
    int nPkt;
    int txFreeCnt;
    int txPendCnt;
    int ret = 0;

    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    if (bFree)
    {
        txUsedCnt = MHal_EMAC_TXQ_Used(hemac->hal);
        txUsedCntSW = skb_queue_used(&hemac->skb_queue_tx, 0);
        ret = txUsedCntSW - txUsedCnt;
        for (i = txUsedCnt; i < txUsedCntSW; i++)
        {
            // MHal_EMAC_TXQ_Remove();
            len = skb_queue_remove(&hemac->skb_queue_tx, NULL, NULL, 1, 0);
            spin_lock_irqsave(&hemac->emac_data_done_lock, flags1);
            hemac->data_done += len;
            spin_unlock_irqrestore(&hemac->emac_data_done_lock, flags1);

            hemac->stats.tx_bytes += len;
            tx_bytes_per_timer += len;
        }
    }

    if (bPump)
    {
        int skb_len;
        txFreeCnt = skb_queue_free(&hemac->skb_queue_tx, 0);
        txPendCnt = skb_queue_used(&hemac->skb_queue_tx, 2);
        nPkt = (txFreeCnt < txPendCnt) ? txFreeCnt : txPendCnt;
        for (i = 0; i < nPkt; i++)
        {
            skb_queue_head_inc(&hemac->skb_queue_tx, &skb, &skb_addr, &skb_len, 0);
            if (skb_addr)
            {
                MHal_EMAC_TXQ_Insert(hemac->hal, skb_addr, skb_len);
            }
        }
    }
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
    return ret;
}
#endif

static int MDev_EMAC_tx(struct sk_buff *skb, struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    dma_addr_t skb_addr;
    // int txIdleCount=0;
    // int txIdleCntSW = 0;
    unsigned long flags;
    unsigned long flag1;
    int ret = NETDEV_TX_OK;

    spin_lock_irqsave(&hemac->mutexNetIf, flag1);

    if(hemac->led_orange!=-1 && hemac->led_green!=-1)
    {
        if(hemac->led_count++ > hemac->led_flick_speed){
            MDrv_GPIO_Set_Low(hemac->led_orange);
            hemac->led_count=0;
        }
    }

#ifdef CONFIG_MSTAR_EEE
    MHal_EMAC_Disable_EEE_TX();
#endif

    if (netif_queue_stopped(dev)){
        EMAC_ERR("netif_queue_stopped\n");
        ret = NETDEV_TX_BUSY;
        goto out_unlock;
    }
    if (!netif_carrier_ok(dev)){
        // EMAC_ERR("netif_carrier_off\n");
        ret = NETDEV_TX_BUSY;
        goto out_unlock;
    }
    if (skb->len > EMAC_MTU)
    {
        // EMAC_ERR("Something wrong (mtu, tx_len) = (%d, %d)\n", dev->mtu, skb->len);
        // ret = NETDEV_TX_BUSY;
        dev_kfree_skb_any(skb);
        dev->stats.tx_dropped++;
        goto out_unlock;
    }
#if defined(PACKET_DUMP)
    if(1==txDumpCtrl && NULL!=txDumpFile)
    {
        txDumpFileLength+=msys_kfile_write(txDumpFile,txDumpFileLength,skb->data,skb->len);
    }
    else if(2==txDumpCtrl && NULL!=txDumpFile)
    {
        msys_kfile_close(txDumpFile);
        txDumpFile=NULL;
        printk(KERN_WARNING"close emac tx_dump file '%s', len=0x%08X...\n",txDumpFileName,txDumpFileLength);
    }
#endif
    //if buffer remains one space, notice upperr layer to block transmition.
    // if (MHal_EMAC_TXQ_Full() || skb_queue_full(&(hemac->skb_queue_tx)))
    // if (MHal_EMAC_TXQ_Full())
    if (skb_queue_full(&hemac->skb_queue_tx, 1))
    {
        netif_stop_queue(dev);
#if DYNAMIC_INT_TX
        MHal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 1);
#endif
        ret = NETDEV_TX_BUSY;
        goto out_unlock;
    }

#if EMAC_FLOW_CONTROL_RX
    if (hemac->isPausePkt)
    {
        unsigned long flags;
        spin_lock_irqsave(&hemac->mutexTXQ, flags);
        skb_queue_insert(&(hemac->skb_queue_tx), NULL, VIRT2BUS(hemac->pu8PausePkt), hemac->u8PausePktSize, 1);
        hemac->isPausePkt = 0;
        spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
        ret = NETDEV_TX_BUSY;
        goto out_unlock;
    }
#endif // #if EMAC_FLOW_CONTROL_RX

#if EMAC_SG
    {
        int i;
        int nr_frags = skb_shinfo(skb)->nr_frags;
        int len;

        // dma_unmap_single(NULL, VIRT2PA(start), EMAC_MTU, DMA_TO_DEVICE);
        if (nr_frags)
        {
            char* start = kmalloc(EMAC_MTU, GFP_ATOMIC);
            char* p = start;

            if (!start)
            {
                ret = NETDEV_TX_BUSY;
                goto out_unlock;
            }

            memcpy(p, skb->data, skb_headlen(skb));
            p += skb_headlen(skb);
            len = skb_headlen(skb);
            for (i = 0; i < nr_frags; i++)
            {
                struct skb_frag_struct *frag = &skb_shinfo(skb)->frags[i];
#if EMAC_SG_BDMA
                {
                    MSYS_DMA_COPY stDmaCopyCfg;

                    Chip_Flush_Cache_Range((size_t)skb_frag_address(frag), skb_frag_size(frag));
                    stDmaCopyCfg.phyaddr_src = (unsigned long long)VIRT2BUS(skb_frag_address(frag));
                    stDmaCopyCfg.phyaddr_dst = (unsigned long long)VIRT2BUS(p);
                    stDmaCopyCfg.length = skb_frag_size(frag);
                    xx_msys_dma_copy(&stDmaCopyCfg);
                }
#else
                memcpy(p, skb_frag_address(frag), skb_frag_size(frag));
#endif
                p += skb_frag_size(frag);
                len += skb_frag_size(frag);
            }
/*
            if (len != skb->len)
                printk("[%s][%d] strange ??? (len, skb->len) = (%d, %d)\n", __FUNCTION__, __LINE__, len, skb->len);
*/
            if (EMAC_SG_BUF_CACHE)
                Chip_Flush_Cache_Range((size_t)start, skb->len);
            skb_addr = VIRT2BUS(start);
            spin_lock_irqsave(&hemac->mutexTXQ, flags);
            skb_queue_insert(&(hemac->skb_queue_tx), (struct sk_buff*)0xFFFFFFFF, (dma_addr_t)skb_addr, skb->len, 1);
            spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
            // Chip_Flush_Cache_Range((size_t)start, skb->len);
            dev_kfree_skb_any(skb);
        }
        else
        {
            {
                struct sk_buff* skb_tmp = skb_clone(skb, GFP_ATOMIC);
                if (!skb_tmp)
                {
                    printk("[%s][%d] skb_clone fail\n", __FUNCTION__, __LINE__);
                    ret = NETDEV_TX_BUSY;
                    goto out_unlock;
                }
                dev_kfree_skb_any(skb);
                skb = skb_tmp;
            }
            skb_addr = VIRT2BUS(skb->data);
            spin_lock_irqsave(&hemac->mutexTXQ, flags);
            skb_queue_insert(&(hemac->skb_queue_tx), skb, skb_addr, skb->len, 1);
            spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
            Chip_Flush_Cache_Range((size_t)skb->data, skb->len);
        }
	if (nr_frags >= hemac->maxSG)
	    hemac->maxSG = nr_frags + 1;
    }
#else // #if #if EMAC_SG

#if DYNAMIC_INT_TX_TIMER
    if ((DYNAMIC_INT_TX) && (0 == hemac->timerTxWdtPeriod))
#else
    if (DYNAMIC_INT_TX)
#endif
    {
        struct sk_buff* skb_tmp = skb_clone(skb, GFP_ATOMIC);
        if (!skb_tmp)
        {
            printk("[%s][%d] skb_clone fail\n", __FUNCTION__, __LINE__);
            ret = NETDEV_TX_BUSY;
            goto out_unlock;
        }
        dev_kfree_skb_any(skb);
        // kfree_skb(skb);
        skb = skb_tmp;
    }
    skb_addr = VIRT2BUS(skb->data);
    if (unlikely(0 == skb_addr))
    {
        dev_kfree_skb_any(skb);
        // kfree_skb(skb);
        printk(KERN_ERR"ERROR!![%s]%d\n",__FUNCTION__,__LINE__);
        dev->stats.tx_dropped++;
        goto out_unlock;
    }
    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    skb_queue_insert(&(hemac->skb_queue_tx), skb, skb_addr, skb->len, 1);
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);

    //set DMA address and trigger DMA
    Chip_Flush_Cache_Range((size_t)skb->data, skb->len);
    // Chip_Flush_MIU_Pipe();
#endif // #if #if EMAC_SG

    // txIdleCount = skb_queue_free(&hemac->skb_queue_tx, 0);
    // if(min_tx_fifo_idle_count>txIdleCount) min_tx_fifo_idle_count=txIdleCount;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
    netif_trans_update(dev);
#else
    dev->trans_start = jiffies;
#endif
out_unlock:
    {
/*
        int bSkbFree = ((hemac->skb_tx_send & 0x0f) == 0) ? 1 : 0;
        _MDev_EMAC_tx_pump(hemac, bSkbFree, 1);
*/
        _MDev_EMAC_tx_pump(hemac, 1, 1);
    }

/*
#if DYNAMIC_INT_TX
    // if ((hemac->skb_tx_send - hemac->skb_tx_free) > 32)
    // if ((hemac->skb_tx_send & 0x0f) == 0)
    // if ((hemac->skb_tx_send - hemac->skb_tx_free) >= DYNAMIC_INT_TX_TH)
    if ((hemac->skb_tx_send - hemac->skb_tx_free) >= 64)
    {
        MHal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 1);
    }
#endif
*/
    spin_unlock_irqrestore(&hemac->mutexNetIf, flag1);
    return ret;
}

#if 0 //ajtest
static int ajtest_recv_count=0;
static ssize_t ajtest_recv_count_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{

    int val = simple_strtoul(buf, NULL, 10);
    if(0==val)
    {

        ajtest_recv_count=0;
    }
    return count;
}
static ssize_t ajtest_recv_count_show(struct device *dev, struct device_attribute *attr, char *buf)
{

    return sprintf(buf, "%d\n", ajtest_recv_count);
}
DEVICE_ATTR(ajtest_recv_count, 0644, ajtest_recv_count_show, ajtest_recv_count_store);


unsigned int aj_checksum( unsigned char * buffer, long length )
{

     long index;
     unsigned int checksum;

     for( index = 0L, checksum = 0; index < length; index++)
     {
         checksum += (int)buffer[index];
         checksum &= 0x0FFFFFFF;
//         if(index<10)printf("%d %d\n",buffer[index],checksum);

     }
     return checksum;

}

static int aj_check(char *pData, int pktlen, int flag)
{
    int res=0;
    if(pktlen>32)
    {
        char *dbuf=(pData+0x36);

        if(0x51==dbuf[0] && 0x58==dbuf[1] && 0x91==dbuf[2] && 0x58==dbuf[3])
        {
            int dlen=0;
            int pktid=0;
            int pktcs=0;//((buf[ret-1])<<24) + (buf[ret-2]<<16)+(buf[ret-3]<<8) + (buf[ret-4]);
            int cs=0;
            unsigned char *buf=(unsigned char *)(dbuf+8);

            dlen=(dbuf[7]<<24) + (dbuf[6]<<16)+(dbuf[5]<<8) + (dbuf[4]);
            pktid=(dbuf[11]<<24) + (dbuf[10]<<16)+(dbuf[9]<<8) + (dbuf[8]);

//                                printf("pktCount: %08d\n",pktCount);
            pktcs=((buf[dlen-1])<<24) + (buf[dlen-2]<<16)+(buf[dlen-3]<<8) + (buf[dlen-4]);
            cs=aj_checksum(buf,dlen-4);
            if(pktcs!=cs)
            {
//                    int j=0;
//                    unsigned int lc=0;
                printk(KERN_WARNING"<[!! AJ_ERR %d, %d: 0x%08X, 0x%08X, %d ]>\n\n",ajtest_recv_count,pktid,cs,pktcs,flag);
                res=-1;
//                    for(j=0;j<ret;j++)
//                    {
//                        if(0==(j%16)){printf("0x%08X: ",lc++);}
//                        printf("%02X ",buf[j]);
//                        if(15==(j%16))printf("\n");
//                    }
//                    printf("cs2=0x%08X\n",checksum(buf,ret-4));
//                    printf("\n");
//                    while(1);
            }
//          else
//          {
//              printk(KERN_WARNING"<[AJOK %d:%d: 0x%08X, 0x%08X, %d ]>\n",ajss_recv_count,pktid,cs,pktcs,flag);
//          }

            ajtest_recv_count++;
        }
    }
    return res;
}
#endif

//-------------------------------------------------------------------------------------------------
// Extract received frame from buffer descriptors and sent to upper layers.
// (Called from interrupt context)
// (Disable RX software discriptor)
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_rx(struct net_device *dev, int budget)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    unsigned char *p_recv;
    u32 pktlen;
    u32 received=0;
    struct sk_buff *skb;
    struct sk_buff *clean_skb;
    rx_desc_queue_t* rxinfo = &(hemac->rx_desc_queue);
    // unsigned long flags;

    if (0 == budget)
        return 0;

    if(hemac->led_orange!=-1 && hemac->led_green!=-1)
    {
        if(hemac->led_count++ > hemac->led_flick_speed){
            MDrv_GPIO_Set_Low(hemac->led_orange);
            hemac->led_count=0;
        }
    }

    // spin_lock_irqsave(&hemac->mutexRXD, flags);
    // If any Ownership bit is 1, frame received.
    do
    {
        char* pData;
        struct rbf_t* desc = &(rxinfo->desc[rxinfo->idx]);

        // if(!((dlist->descriptors[hemac->rxBuffIndex].addr) & EMAC_DESC_DONE))
        if (!(desc->addr & EMAC_DESC_DONE))
        {
            break;
        }
        // p_recv = (char *) ((((dlist->descriptors[hemac->rxBuffIndex].addr) & 0xFFFFFFFFUL) + RAM_VA_PA_OFFSET + MIU0_BUS_BASE) &~(EMAC_DESC_DONE | EMAC_DESC_WRAP));

#if RX_DESC_API
        p_recv = RX_ADDR_GET(desc);
#else
        p_recv = BUS2VIRT(CLR_BITS(desc->addr, EMAC_DESC_DONE | EMAC_DESC_WRAP));
#endif
        pktlen = desc->size & 0x7ffUL;    /* Length of frame including FCS */

    #if RX_THROUGHPUT_TEST
        receive_bytes += pktlen;
    #endif

        if (unlikely(((pktlen > EMAC_MTU) || (pktlen < 64))))
        {
            EMAC_ERR("drop packet!!(pktlen = %d)", pktlen);
            desc->addr = CLR_BITS(desc->addr, EMAC_DESC_DONE);
            Chip_Flush_MIU_Pipe();
            rxinfo->idx++;
            if (rxinfo->idx >= rxinfo->num_desc)
                rxinfo->idx = 0;
            hemac->stats.rx_length_errors++;
            hemac->stats.rx_errors++;
            hemac->stats.rx_dropped++;
            continue;
        }

        if (unlikely(!(clean_skb = alloc_skb (EMAC_PACKET_SIZE_MAX, GFP_ATOMIC))))
        {
            // printk(KERN_ERR"Can't alloc skb.[%s]%d\n",__FUNCTION__,__LINE__);;
            goto jmp_rx_exit;
            // return -ENOMEM;
        }
        skb = rxinfo->skb_arr[rxinfo->idx];
    #if EXT_PHY_PATCH
        if (IS_EXT_PHY(hemac))
        {
            dma_unmap_single(NULL, VIRT2PA(p_recv), pktlen, DMA_FROM_DEVICE);
            memcpy(skb->data, p_recv, pktlen);
            pktlen -= 4; /* Remove FCS */
            pData = skb_put(skb, pktlen);
        }
        else
    #endif
        {
            pktlen -= 4; /* Remove FCS */
            pData = skb_put(skb, pktlen);
            dma_unmap_single(NULL, VIRT2PA(pData), pktlen, DMA_FROM_DEVICE);
        }

        //ajtest
        /* below code is used to find the offset of ajtest header in incoming packet
        for(cidx=0;cidx<pktlen;cidx++)
        {
          if((cidx+4)>=pktlen)
          {
              break;
          }
          if(0x51==pData[cidx] && 0x58==pData[cidx+1] && 0x91==pData[cidx+2] && 0x58==pData[cidx+3])
          {
              printk(KERN_WARNING"cidx: 0x%08X\n",cidx);
              break;
          }
        }
        */
#if defined(PACKET_DUMP)
        if(1==rxDumpCtrl && NULL!=rxDumpFile)
        {
            rxDumpFileLength+=msys_kfile_write(rxDumpFile,rxDumpFileLength,pData,pktlen);
        }
        else if(2==rxDumpCtrl && NULL!=rxDumpFile)
        {
            msys_kfile_close(rxDumpFile);
            rxDumpFile=NULL;
            printk(KERN_WARNING"close emac rx_dump file '%s', len=0x%08X...\n",rxDumpFileName,rxDumpFileLength);
        }
#endif

        skb->dev = dev;
        skb->protocol = eth_type_trans (skb, dev);
        //skb->len = pktlen;
        dev->last_rx = jiffies;
        hemac->stats.rx_bytes += pktlen;
#if EMAC_FLOW_CONTROL_TX
        if (0 == MHal_EMAC_FlowControl_TX(hemac))
        {
            _MDrv_EMAC_Pause_TX(dev, skb, p_recv);
        }
#endif // #if EMAC_FLOW_CONTROL_TX
    #if RX_THROUGHPUT_TEST
        kfree_skb(skb);
    #else

    #if RX_CHECKSUM
        if (((desc->size & EMAC_DESC_TCP) || (desc->size & EMAC_DESC_UDP)) &&
             (desc->size & EMAC_DESC_IP_CSUM) &&
             (desc->size & EMAC_DESC_TCP_UDP_CSUM))
        {
            skb->ip_summed = CHECKSUM_UNNECESSARY;
        }
        else
        {
            skb->ip_summed = CHECKSUM_NONE;
        }
    #endif

    #ifdef ISR_BOTTOM_HALF
        netif_rx_ni(skb);
    #elif MSTAR_EMAC_NAPI
        #if (EMAC_GSO)
        napi_gro_receive(&hemac->napi, skb);
        #else
        netif_receive_skb(skb);
        #endif
    #else
        netif_rx (skb);
    #endif

        received++;
        #endif/*RX_THROUGHPUT_TEST*/

        // if (dlist->descriptors[hemac->rxBuffIndex].size & EMAC_MULTICAST)
        if (desc->size & EMAC_MULTICAST)
        {
            hemac->stats.multicast++;
        }

        //fill clean_skb into RX descriptor
        rxinfo->skb_arr[rxinfo->idx] = clean_skb;

#if EXT_PHY_PATCH
        if (IS_EXT_PHY(hemac))
        {
            if (rxinfo->idx == (rxinfo->num_desc-1))
                desc->addr = VIRT2BUS(p_recv) | EMAC_DESC_DONE | EMAC_DESC_WRAP;
            else
                desc->addr = VIRT2BUS(p_recv) | EMAC_DESC_DONE;
            desc->addr = CLR_BITS(desc->addr, EMAC_DESC_DONE);
            dma_map_single(NULL, p_recv, EMAC_PACKET_SIZE_MAX, DMA_FROM_DEVICE);
        }
        else
#endif
        {
#if RX_DESC_API
            RX_DESC_MAKE(desc, VIRT2BUS(skb->data), (rxinfo->idx == (rxinfo->num_desc-1)) ? EMAC_DESC_WRAP : 0);
#else
            if (rxinfo->idx == (rxinfo->num_desc-1))
                desc->addr = VIRT2BUS(clean_skb->data) | EMAC_DESC_DONE | EMAC_DESC_WRAP;
            else
                desc->addr = VIRT2BUS(clean_skb->data) | EMAC_DESC_DONE;
            desc->addr = CLR_BITS(desc->addr, EMAC_DESC_DONE);
#endif
            dma_map_single(NULL, clean_skb->data, EMAC_PACKET_SIZE_MAX, DMA_FROM_DEVICE);
        }
        Chip_Flush_MIU_Pipe();

        rxinfo->idx++;
        if (rxinfo->idx >= rxinfo->num_desc)
            rxinfo->idx = 0;

#if MSTAR_EMAC_NAPI
        // if(received >= EMAC_NAPI_WEIGHT) {
        if(received >= budget) {
            break;
        }
#endif

    } while(1);

jmp_rx_exit:
    // spin_unlock_irqrestore(&hemac->mutexRXD, flags);
    if(received>max_rx_packet_count)max_rx_packet_count=received;
    rx_packet_cnt += received;
    return received;
}



//-------------------------------------------------------------------------------------------------
//MAC interrupt handler
//(Interrupt delay enable)
//-------------------------------------------------------------------------------------------------
static int RBNA_detailed=0;

static u32 _MDrv_EMAC_ISR_RBNA(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = NULL;
    int empty=0;
    int idx;
    rx_desc_queue_t* rxinfo;

    if (0 == (intstatus & EMAC_INT_RBNA))
        return 0;

    hemac = (struct emac_handle*) netdev_priv(dev);
    rxinfo = &(hemac->rx_desc_queue);

    hemac->stats.rx_missed_errors++;

    //write 1 clear
    MHal_EMAC_Write_RSR(hemac->hal, EMAC_BNA);
    if(RBNA_detailed>0)
    {
        // u32 u32RBQP_Addr = MHal_EMAC_Read_RBQP(hemac->hal)- VIRT2BUS(rxinfo->desc);
        for(idx=0;idx<rxinfo->num_desc;idx++)
        {
            // if(!((dlist->descriptors[idx].addr) & EMAC_DESC_DONE))
            if (!(rxinfo->desc[idx].addr & EMAC_DESC_DONE))
            {
                empty++;
            }
            else
            {
                printk(KERN_ERR"RBNA: [0x%X]\n",idx);
            }

        }
        // printk(KERN_ERR"RBNA: empty=0x%X, rxBuffIndex=0x%X, rx_missed_errors=%ld RBQP_offset=0x%x\n",empty, rxinfo->idx, hemac->stats.rx_missed_errors,u32RBQP_Addr);
    }
#if 0
    if (1)
    {
        struct rbf_t* desc = NULL;
        unsigned long flags;

        spin_lock_irqsave(&hemac->mutexRXD, flags);
        for (idx=0;idx<rxinfo->num_desc;idx++)
        {
            desc = &(rxinfo->desc[idx]);
            if (idx == (rxinfo->num_desc-1))
                desc->addr = CLR_BITS(desc->addr, EMAC_DESC_DONE) | EMAC_DESC_WRAP;
            else
                desc->addr = CLR_BITS(desc->addr, EMAC_DESC_DONE);
	    desc->size = 0;
        }
        spin_unlock_irqrestore(&hemac->mutexRXD, flags);
        Chip_Flush_MIU_Pipe();
    }
#endif

    hemac->irq_count[IDX_CNT_INT_RBNA]++;
    //printk("RBNA\n");
#if REDUCE_CPU_FOR_RBNA
    {
        unsigned long flags;
        spin_lock_irqsave(&hemac->mutexIntRX, flags);
        if (0 == timer_pending(&hemac->timerIntRX))
        {
            MHal_EMAC_RX_ParamSet(hemac->hal, 0xff, 0xff);
            hemac->timerIntRX.expires = jiffies + HZ;
            add_timer(&hemac->timerIntRX);
        }
        spin_unlock_irqrestore(&hemac->mutexIntRX, flags);
    }
#endif // #if REDUCE_CPU_FOR_RBNA
    // gu32GatingRxIrqTimes = 1;
    return 1;
}

static u32 _MDrv_EMAC_ISR_TCOM(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = NULL;

    if (0 == (intstatus & EMAC_INT_TCOM))
        return 0;

    hemac = (struct emac_handle*) netdev_priv(dev);

    if(hemac->led_orange!=-1 && hemac->led_green!=-1)
    {
        if(hemac->led_count++ > hemac->led_flick_speed){
            MDrv_GPIO_Set_High(hemac->led_orange);
            hemac->led_count=0;
        }
    }
    hemac->tx_irqcnt++;

    // The TCOM bit is set even if the transmission failed. //
    if (intstatus & (EMAC_INT_TUND | EMAC_INT_RTRY))
    {
        hemac->stats.tx_errors += 1;
        if(intstatus & EMAC_INT_TUND)
        {
            //write 1 clear
            // MHal_EMAC_Write_TSR(EMAC_UND);

            //Reset TX engine
            MDev_EMAC_TxReset(dev);
            EMAC_ERR ("Transmit TUND error, TX reset\n");
            hemac->irq_count[IDX_CNT_INT_TUND]++;
        }
        hemac->irq_count[IDX_CNT_INT_RTRY]++;
    }

    #if TX_THROUGHPUT_TEST
    MDev_EMAC_tx(pseudo_packet, dev);
    #endif

#if 0
    // _MDev_EMAC_tx_free(hemac);
    _MDev_EMAC_tx_pump(hemac, 1, 0);

    // if (netif_queue_stopped (dev) && skb_queue_free(&hemac->skb_queue_tx, 1)) // ??
    if (netif_queue_stopped (dev) && skb_queue_free(&hemac->skb_queue_tx, 0)) // 35%
    // if (netif_queue_stopped (dev) && (skb_queue_free(&hemac->skb_queue_tx, 0)> 80)) // 27.5%
    // if (netif_queue_stopped (dev) && (skb_queue_free(&hemac->skb_queue_tx, 0) > (skb_queue_size(&hemac->skb_queue_tx, 0) >> 1))) // 29.7%
    // if (((skb_queue_size(&hemac->skb_queue_tx, 1)>>1) < skb_queue_free(&hemac->skb_queue_tx, 1)) && netif_queue_stopped (dev)) // 45%
    // if (netif_queue_stopped (dev) && (skb_queue_free(&hemac->skb_queue_tx, 0) > (skb_queue_size(&hemac->skb_queue_tx, 0)/3))) // 28.3%
    {
#if DYNAMIC_INT_TX
        MHal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 0);
#endif
#if EMAC_FLOW_CONTROL_TX
        // if (0 == hemac->isPauseTX)
        if (0 == timer_pending(&hemac->timerFlowTX))
            netif_wake_queue(dev);
#else // #if EMAC_FLOW_CONTROL_TX
        netif_wake_queue(dev);
#endif
    }
#endif
    hemac->irq_count[IDX_CNT_INT_TCOM]++;
    return 1;
}

static u32 _MDrv_EMAC_ISR_DONE(struct net_device *dev, u32 intstatus)
{
    struct emac_handle* hemac = (struct emac_handle*) netdev_priv(dev);

    if (0 == (intstatus&EMAC_INT_DONE))
        return 0;
    hemac->irq_count[IDX_CNT_INT_DONE]++;
    return 1;
}

static u32 _MDrv_EMAC_ISR_ROVR(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);

#if EMAC_FLOW_CONTROL_RX
#if EMAC_FLOW_CONTROL_RX_TEST
    {
        static int cnt = 0;
        cnt++;

        if (0 == (cnt & 0xF))
            _MDrv_EMAC_PausePkt_Send(dev);
        return 0;
    }
#endif
#endif

    if (0 == (intstatus & EMAC_INT_ROVR))
    {
        hemac->contiROVR = 0;
        return 0;
    }

    hemac->stats.rx_over_errors++;
    hemac->contiROVR++;
    //write 1 clear
    MHal_EMAC_Write_RSR(hemac->hal, EMAC_RSROVR);
#if EMAC_FLOW_CONTROL_RX
    if (hemac->contiROVR < 3)
    {
        _MDrv_EMAC_PausePkt_Send(dev);
    }
    else
    {
        MDev_EMAC_SwReset(dev);
    }
#endif
    hemac->irq_count[IDX_CNT_INT_ROVR]++;
    return 0;
}

static u32 _MDrv_EMAC_ISR_RCOM(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = NULL;

    hemac = (struct emac_handle*) netdev_priv(dev);

    if (intstatus & EMAC_INT_RCOM_DELAY)
    {
        hemac->irq_count[IDX_CNT_JULIAN_D]++;
    }
    else if (intstatus & EMAC_INT_RCOM)
    {
        intstatus |= EMAC_INT_RCOM_DELAY;
        hemac->irq_count[IDX_CNT_INT_RCOM]++;
    }

    // Receive complete //
    if (intstatus & EMAC_INT_RCOM_DELAY)
    {
        if ((0 == rx_time_last.tv_sec) && (0 == rx_time_last.tv_nsec))
        {
            getnstimeofday(&rx_time_last);
        }
        else
        {
            struct timespec ct;
            int duration;

            getnstimeofday(&ct);
            duration = (ct.tv_sec - rx_time_last.tv_sec)*1000 + (ct.tv_nsec - rx_time_last.tv_nsec)/1000000;
            rx_duration_max = (rx_duration_max < duration) ? duration : rx_duration_max;
            rx_time_last = ct;
        }

        if(hemac->led_orange!=-1 && hemac->led_green!=-1)
        {
            if(hemac->led_count++ > hemac->led_flick_speed){
                MDrv_GPIO_Set_High(hemac->led_orange);
                hemac->led_count=0;
            }
        }
    #if MSTAR_EMAC_NAPI
        /* Receive packets are processed by poll routine. If not running start it now. */
        if (napi_schedule_prep(&hemac->napi))
        {
            // MDEV_EMAC_DISABLE_RX_REG();
            MHal_EMAC_IntEnable(hemac->hal, EMAC_INT_RCOM, 0);
            __napi_schedule(&hemac->napi);
        }
        else
        {
            // printk("[%s][%d] NAPI RX cannot be scheduled\n", __FUNCTION__, __LINE__);
        }
    #elif defined ISR_BOTTOM_HALF
        /*Triger rx_task*/
        schedule_work(&hemac->rx_task);
    #else
        {
            unsigned long flags;
            spin_lock_irqsave(&hemac->mutexRXInt, flags);
            MDev_EMAC_rx(dev, 0x0FFFFFFF);
            spin_unlock_irqrestore(&hemac->mutexRXInt, flags);
        }
    #endif
    }
    return intstatus;
}

irqreturn_t MDev_EMAC_interrupt(int irq,void *dev_id)
{
    struct net_device *dev = (struct net_device *) dev_id;
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 intstatus=0;

    hemac->irqcnt++;
    hemac->oldTime = getCurMs();
    _MDev_EMAC_tx_pump(hemac, 1, 0);
    while ((intstatus = MHal_EMAC_IntStatus(hemac->hal)))
    {
        _MDrv_EMAC_ISR_RBNA(dev, intstatus);
        _MDrv_EMAC_ISR_TCOM(dev, intstatus);
        _MDrv_EMAC_ISR_DONE(dev, intstatus);
        _MDrv_EMAC_ISR_ROVR(dev, intstatus);
        _MDrv_EMAC_ISR_RCOM(dev, intstatus);
    }

    if (netif_queue_stopped (dev) && skb_queue_free(&hemac->skb_queue_tx, 0))
    {
#if DYNAMIC_INT_TX
        MHal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 0);
#endif
#if EMAC_FLOW_CONTROL_TX
        if (0 == timer_pending(&hemac->timerFlowTX))
            netif_wake_queue(dev);
#else // #if EMAC_FLOW_CONTROL_TX
        netif_wake_queue(dev);
#endif
    }

    return IRQ_HANDLED;
}

#if DYNAMIC_INT_RX
#define emac_time_elapse(start)                                                                 \
({                                                                                              \
    unsigned long delta;                                                                        \
    struct timespec ct;                                                                         \
    getnstimeofday(&ct);                                                                        \
    delta = (ct.tv_sec - (start).tv_sec)*1000 + (ct.tv_nsec - (start).tv_nsec)/1000000;         \
    (delta);                                                                                    \
})
#endif // #if DYNAMIC_INT_RX

#if MSTAR_EMAC_NAPI
static int MDev_EMAC_napi_poll(struct napi_struct *napi, int budget)
{
    struct emac_handle  *hemac = container_of(napi, struct emac_handle,napi);
    struct net_device *dev = hemac->netdev;
    int work_done = 0;
    int budget_rmn = budget;
#if DYNAMIC_INT_RX
    unsigned long elapse = 0;
    unsigned long packets = 0;
#endif // #if DYNAMIC_INT_RX

// rx_poll_again:
    work_done = MDev_EMAC_rx(dev, budget_rmn);

    if (work_done)
    {
        // budget_rmn -= work_done;
        // goto rx_poll_again;
    }
#if DYNAMIC_INT_RX
    if (hemac->rx_stats_enable)
    {
        if (0xFFFFFFFF == hemac->rx_stats_packet)
        {
            getnstimeofday(&hemac->rx_stats_time);
            hemac->rx_stats_packet = 0;
        }
        hemac->rx_stats_packet += work_done;
        if ((elapse = emac_time_elapse(hemac->rx_stats_time))>= 1000)
        {
            packets = hemac->rx_stats_packet;
            packets *= 1000;
            packets /= elapse;

#if REDUCE_CPU_FOR_RBNA
        {
            unsigned long flags;
            spin_lock_irqsave(&hemac->mutexIntRX, flags);
            if (0 == timer_pending(&hemac->timerIntRX))
            {
#endif // #if REDUCE_CPU_FOR_RBNA
                // printk("[%s][%d] packet for delay number (elapse, current, packet) = (%d, %d, %d)\n", __FUNCTION__, __LINE__, (int)elapse, (int)hemac->rx_stats_packet, (int)packets);
                // MHal_EMAC_RX_ParamSet(hemac->hal, packets/200 + 2, 0xFFFFFFFF);
                MHal_EMAC_RX_ParamSet(hemac->hal, packets/200 + 1, 0xFFFFFFFF);
#if REDUCE_CPU_FOR_RBNA
            }
            spin_unlock_irqrestore(&hemac->mutexIntRX, flags);
        }
#endif // #if REDUCE_CPU_FOR_RBNA
            hemac->rx_stats_packet = 0;
            getnstimeofday(&hemac->rx_stats_time);
        }
    }
#endif

/*
    if (work_done == budget_rmn)
        return budget;
*/

    napi_gro_flush(napi, false);

#if 1
    /* If budget not fully consumed, exit the polling mode */
    if (work_done < budget) {
        napi_complete(napi);
        MHal_EMAC_IntEnable(hemac->hal, EMAC_INT_RCOM, 1);
    }
    return work_done;
#else
    napi_complete(napi);
    MHal_EMAC_IntEnable(hemac->hal, EMAC_INT_RCOM, 1);
    return budget + work_done - budget_rmn;
#endif
}
#endif

#ifdef LAN_ESD_CARRIER_INTERRUPT
irqreturn_t MDev_EMAC_interrupt_cable_unplug(int irq,void *dev_instance)
{
    struct net_device* dev = (struct net_device*)dev_instance;
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);

    if (netif_carrier_ok(dev))
        netif_carrier_off(dev);
    if (!netif_queue_stopped(dev))
        netif_stop_queue(dev);
    hemac->ThisBCE.connected = 0;

    #ifdef TX_SW_QUEUE
    _MDev_EMAC_tx_reset_TX_SW_QUEUE(dev);
    #endif
    skb_queue_reset(&(hemac->skb_queue_tx));

    if(hemac->led_orange!=-1 && hemac->led_green!=-1)
    {
        MDrv_GPIO_Set_Low(hemac->led_orange);
        MDrv_GPIO_Set_Low(hemac->led_green);
    }

    return IRQ_HANDLED;
}
#endif

//-------------------------------------------------------------------------------------------------
// EMAC Hardware register set
//-------------------------------------------------------------------------------------------------
void MDev_EMAC_HW_init(struct net_device* dev)
{
    struct emac_handle *hemac = (struct emac_handle*) netdev_priv(dev);
    u32 word_ETH_CTL = 0x00000000UL;
    // u32 word_ETH_CFG = 0x00000800UL;
    // u32 uNegPhyVal = 0;
    u32 idxRBQP = 0;
    // u32 RBQP_offset = 0;
    struct sk_buff *skb = NULL;
    // u32 RBQP_rx_skb_addr = 0;
    // unsigned long flags;

    // (20071026_CHARLES) Disable TX, RX and MDIO:   (If RX still enabled, the RX buffer will be overwrited)
    MHal_EMAC_Write_CTL(hemac->hal, word_ETH_CTL);
    MHal_EMAC_Write_BUFF(hemac->hal, 0x00000000UL);
    // Set MAC address ------------------------------------------------------
    MHal_EMAC_Write_SA1_MAC_Address(hemac->hal, hemac->sa[0][0], hemac->sa[0][1], hemac->sa[0][2], hemac->sa[0][3], hemac->sa[0][4], hemac->sa[0][5]);
    MHal_EMAC_Write_SA2_MAC_Address(hemac->hal, hemac->sa[1][0], hemac->sa[1][1], hemac->sa[1][2], hemac->sa[1][3], hemac->sa[1][4], hemac->sa[1][5]);
    MHal_EMAC_Write_SA3_MAC_Address(hemac->hal, hemac->sa[2][0], hemac->sa[2][1], hemac->sa[2][2], hemac->sa[2][3], hemac->sa[2][4], hemac->sa[2][5]);
    MHal_EMAC_Write_SA4_MAC_Address(hemac->hal, hemac->sa[3][0], hemac->sa[3][1], hemac->sa[3][2], hemac->sa[3][3], hemac->sa[3][4], hemac->sa[3][5]);

    // spin_lock_irqsave(&hemac->mutexRXD, flags);
    {
        struct emac_handle *hemac = (struct emac_handle *) netdev_priv(dev);
        rx_desc_queue_t* rxinfo = &(hemac->rx_desc_queue);
#if EXT_PHY_PATCH
        char* p = hemac->pu8RXBuf;
#endif

        // Initialize Receive Buffer Descriptors
        memset(rxinfo->desc, 0x00, rxinfo->size_desc_queue);

        for(idxRBQP = 0; idxRBQP < rxinfo->num_desc; idxRBQP++)
        {
            struct rbf_t* desc = &(rxinfo->desc[idxRBQP]);

            if (!(skb = alloc_skb (EMAC_PACKET_SIZE_MAX, GFP_ATOMIC))) {
                // printk("%s %d: alloc skb failed!\n",__func__, __LINE__);
                panic("can't alloc skb");
            }
            rxinfo->skb_arr[idxRBQP] = skb;
#if RX_DESC_API
            RX_DESC_MAKE(desc, VIRT2BUS(skb->data), (idxRBQP < (rxinfo->num_desc- 1)) ? 0 : EMAC_DESC_WRAP);
#else
    #if EXT_PHY_PATCH
            if (IS_EXT_PHY(hemac))
            {
                if(idxRBQP < (rxinfo->num_desc- 1))
                    desc->addr = VIRT2BUS(p);
                else
                    desc->addr = VIRT2BUS(p) | EMAC_DESC_WRAP;
                p += EMAC_PACKET_SIZE_MAX;
            }
            else
    #endif
            {
                if(idxRBQP < (rxinfo->num_desc- 1))
                    desc->addr = VIRT2BUS(skb->data);
                else
                    desc->addr = VIRT2BUS(skb->data) | EMAC_DESC_WRAP;
            }
#endif
        }
        Chip_Flush_MIU_Pipe();
        // Initialize "Receive Buffer Queue Pointer"
        MHal_EMAC_Write_RBQP(hemac->hal, VIRT2BUS(rxinfo->desc));
    }
    // spin_unlock_irqrestore(&hemac->mutexRXD, flags);

#if 0
    if (!hemac->ThisUVE.initedEMAC)
    {
#if 0 // should move to phy driver
#if (EMAC_FLOW_CONTROL_TX == EMAC_FLOW_CONTROL_TX_SW)
        MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_ADVERTISE, 0xDE1UL); //BIT0|BIT5~8|BIT10|BIT11 //pause disable transmit, enable receive
#endif
#endif
        // MHal_EMAC_Write_JULIAN_0100(hemac->hal, JULIAN_100_VAL);
        MHal_EMAC_Write_JULIAN_0100(hemac->hal, 0);

        hemac->ThisUVE.flagPowerOn = 1;
        hemac->ThisUVE.initedEMAC  = 1;
    }
#endif
    MHal_EMAC_HW_init(hemac->hal);
}

static void* MDev_EMAC_RX_Desc_Init(struct emac_handle *hemac, void* p)
{
    rx_desc_queue_t* rxinfo = &(hemac->rx_desc_queue);

    rxinfo->num_desc = RX_DESC_NUM;
    rxinfo->size_desc_queue = RX_DESC_QUEUE_SIZE;
#if 0
    mem_info.length = rxinfo->size_desc_queue;
    strcpy(mem_info.name, "EMAC_BUFF");
    if((ret=msys_request_dmem(&mem_info)))
    {
        panic("unable to locate DMEM for EMAC alloRAM!! %d\n", rxinfo->size_desc_queue);
    }
    // rxinfo->descPhys = (dma_addr_t)mem_info.phys;
#endif
    rxinfo->desc = (struct rbf_t*)p;
    
    // EMAC_DBG("alloRAM_VA_BASE=0x%zx alloRAM_PA_BASE=0x%zx\n  alloRAM_SIZE=0x%zx\n", (size_t) rxinfo->desc,(size_t) rxinfo->descPhys,(size_t)rxinfo->size_desc_queue);
    BUG_ON(!rxinfo->desc);

    rxinfo->idx = 0;
    memset(rxinfo->desc, 0x00, rxinfo->size_desc_queue);
    Chip_Flush_MIU_Pipe();

    rxinfo->skb_arr = kzalloc(rxinfo->num_desc*sizeof(struct sk_buff*), GFP_KERNEL);
    BUG_ON(!rxinfo->skb_arr);

    EMAC_DBG("RAM_VA_BASE=0x%08x\n", rxinfo->desc);
    EMAC_DBG("RAM_PA_BASE=0x%08x\n", VIRT2PA(rxinfo->desc));
    EMAC_DBG("RAM_VA_PA_OFFSET=0x%08x\n", rxinfo->off_va_pa);
    EMAC_DBG("RBQP_BASE=0x%08x size=0x%x\n", VIRT2BUS(rxinfo->desc), rxinfo->size_desc_queue);
    return (void*)(((int)p)+RX_DESC_QUEUE_SIZE);
}

static void MDev_EMAC_RX_Desc_Free(struct emac_handle *hemac)
{
    rx_desc_queue_t* rxinfo = &(hemac->rx_desc_queue);

    if (rxinfo->skb_arr)
    {
        kfree(rxinfo->skb_arr);
        rxinfo->skb_arr = NULL;
    }

    rxinfo->desc = NULL;
    rxinfo->idx = 0;
    rxinfo->num_desc = 0;
    rxinfo->size_desc_queue = 0;
}

static void MDev_EMAC_MemFree(struct emac_handle *hemac)
{
    if (hemac->mem_info.length)
    {
        msys_release_dmem(&hemac->mem_info);
        memset(&hemac->mem_info, 0, sizeof(hemac->mem_info));
    }
}

static void* MDev_EMAC_MemAlloc(struct emac_handle *hemac, u32 size)
{
    int ret;

    hemac->mem_info.length = size;
    // strcpy(hemac->mem_info.name, "EMAC_BUFF");
    sprintf(hemac->mem_info.name, "%s_buff", hemac->name);
    if((ret=msys_request_dmem(&hemac->mem_info)))
    {
        memset(&hemac->mem_info, 0, sizeof(hemac->mem_info));
        panic("unable to locate DMEM for EMAC alloRAM!! %d\n", size);
        return NULL;
    }
    return (void*)((size_t)hemac->mem_info.kvirt);
}

//-------------------------------------------------------------------------------------------------
// EMAC init Variable
//-------------------------------------------------------------------------------------------------
// extern phys_addr_t memblock_start_of_DRAM(void);
// extern phys_addr_t memblock_size_of_first_region(void);

static void* MDev_EMAC_VarInit(struct emac_handle *hemac)
{
    char addr[6];
    u32 HiAddr, LoAddr;
    int txd_len;
    void* p = NULL;
#if EMAC_FLOW_CONTROL_RX
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
    int pausePktSize = sizeof(pause_pkt);
    static u8 ETH_PAUSE_FRAME_DA_MAC[6] = { 0x01UL, 0x80UL, 0xC2UL, 0x00UL, 0x00UL, 0x01UL };
#else
    int pausePktSize = 0;
#endif // #if EMAC_FLOW_CONTROL_RX

/*
#if EMAC_SG
    int txBufSize = MHal_EMAC_TXQ_Size(hemac->hal) * EMAC_MTU;
#else
    int txBufSize = 0;
#endif
*/

    // TXD init
    txd_len = MHal_EMAC_TXD_Cfg(hemac->hal, hemac->txd_num);
#if EXT_PHY_PATCH
    if (IS_EXT_PHY(hemac))
    {
        #if (EMAC_SG && EMAC_SG_BUF_CACHE)
        p = MDev_EMAC_MemAlloc(hemac, RX_DESC_QUEUE_SIZE + (RX_DESC_NUM * EMAC_PACKET_SIZE_MAX) + txd_len + pausePktSize);
        #else
        p = MDev_EMAC_MemAlloc(hemac, RX_DESC_QUEUE_SIZE + (RX_DESC_NUM * EMAC_PACKET_SIZE_MAX) + txd_len + pausePktSize + txBufSize);
        #endif
    }
    else
#endif // #if EXT_PHY_PATCH
    {
        #if (EMAC_SG && EMAC_SG_BUF_CACHE)
        p = MDev_EMAC_MemAlloc(hemac, RX_DESC_QUEUE_SIZE + txd_len + pausePktSize);
        #else
        p = MDev_EMAC_MemAlloc(hemac, RX_DESC_QUEUE_SIZE + txd_len + pausePktSize + txBufSize);
        #endif
    }
    if (NULL == p)
    {
        printk("[%s][%d] alloc memory fail %d\n", __FUNCTION__, __LINE__, RX_DESC_QUEUE_SIZE + txd_len);
        return NULL;
    }
/*
#if (EMAC_SG && EMAC_SG_BUF_CACHE)
    if (NULL == (hemac->pTxBuf = kmalloc(txBufSize, GFP_KERNEL)))
    {
        printk("[%s][%d] kmalloc fail %d\n", __FUNCTION__, __LINE__, txBufSize);
        MDev_EMAC_MemFree(hemac);
        return NULL;
    }
#endif
*/
    if (txd_len)
    {
        MHal_EMAC_TXD_Buf(hemac->hal, p, VIRT2BUS(p), txd_len);
        p = (void*)(((size_t)p) + txd_len);
    }

#if EXT_PHY_PATCH
    {
        int start, end;
        start = (int) p;
        p = MDev_EMAC_RX_Desc_Init(hemac, p);
        if (IS_EXT_PHY(hemac))
        {
            hemac->pu8RXBuf = (char*)p;
            p = (void*) (hemac->pu8RXBuf + (RX_DESC_NUM * EMAC_PACKET_SIZE_MAX));
            end = (int) p;
            MHal_EMAC_MIU_Protect_RX(hemac->hal, VIRT2BUS((void*)start), VIRT2BUS((void*)end));
        }
        else
        {
            hemac->pu8RXBuf = NULL;
        }
    }
#else
    p = MDev_EMAC_RX_Desc_Init(hemac, p);
#endif

#if EMAC_FLOW_CONTROL_RX
    hemac->isPausePkt = 0;
    hemac->u8PausePktSize = pausePktSize;
    hemac->pu8PausePkt = (pausePktSize) ? p : NULL;
    if (pausePktSize)
    {
        memcpy(hemac->pu8PausePkt, pause_pkt, pausePktSize);
    }
    p = (void*)(((char*)p) + pausePktSize);
#endif // #if EMAC_FLOW_CONTROL_RX

#if EMAC_SG
/*
    if (NULL == hemac->pTxBuf)
        hemac->pTxBuf = (txBufSize) ? (char*)p : NULL;
    hemac->TxBufIdx = 0;
*/
    hemac->maxSG = 0;
#endif // #if EMAC_SG

    memset(hemac->sa, 0, sizeof(hemac->sa));

    // Check if bootloader set address in Specific-Address 1 //
    HiAddr = MHal_EMAC_get_SA1H_addr(hemac->hal);
    LoAddr = MHal_EMAC_get_SA1L_addr(hemac->hal);

    addr[0] = (LoAddr & 0xffUL);
    addr[1] = (LoAddr & 0xff00UL) >> 8;
    addr[2] = (LoAddr & 0xff0000UL) >> 16;
    addr[3] = (LoAddr & 0xff000000UL) >> 24;
    addr[4] = (HiAddr & 0xffUL);
    addr[5] = (HiAddr & 0xff00UL) >> 8;

    if (is_valid_ether_addr (addr))
    {
        memcpy(&hemac->sa[0][0], &addr, 6);
    }
    else
    {
        // Check if bootloader set address in Specific-Address 2 //
        HiAddr = MHal_EMAC_get_SA2H_addr(hemac->hal);
        LoAddr = MHal_EMAC_get_SA2L_addr(hemac->hal);
        addr[0] = (LoAddr & 0xffUL);
        addr[1] = (LoAddr & 0xff00UL) >> 8;
        addr[2] = (LoAddr & 0xff0000UL) >> 16;
        addr[3] = (LoAddr & 0xff000000UL) >> 24;
        addr[4] = (HiAddr & 0xffUL);
        addr[5] = (HiAddr & 0xff00UL) >> 8;

        if (is_valid_ether_addr (addr))
        {
            memcpy(&hemac->sa[0][0], &addr, 6);
        }
        else
        {
            memcpy(&hemac->sa[0][0], MY_MAC, 6);
        }
    }
#if EMAC_FLOW_CONTROL_RX
    memcpy(&hemac->sa[1][0], ETH_PAUSE_FRAME_DA_MAC, 6);
#endif
    return p;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void MDev_EMAC_netpoll(struct net_device *dev)
{
    unsigned long flags;

    local_irq_save(flags);
    MDev_EMAC_interrupt(dev->irq, dev);
    local_irq_restore(flags);
}
#endif

#if KERNEL_PHY
static void emac_phy_link_adjust(struct net_device *dev)
{
    int cam = 0; // 0:No CAM, 1:Yes
    int rcv_bcast = 1; // 0:No, 1:Yes
    int rlf = 0;
    u32 word_ETH_CFG = 0x00000800UL;
    struct emac_handle* hemac =(struct emac_handle*) netdev_priv(dev);
    unsigned long flag1;

    spin_lock_irqsave(&hemac->mutexNetIf, flag1);
    if (!hemac->bEthCfg)
    {
        // ETH_CFG Register -----------------------------------------------------
        // (20070808) IMPORTANT: REG_ETH_CFG:bit1(FD), 1:TX will halt running RX frame, 0:TX will not halt running RX frame.
        // If always set FD=0, no CRC error will occur. But throughput maybe need re-evaluate.
        // IMPORTANT: (20070809) NO_MANUAL_SET_DUPLEX : The real duplex is returned by "negotiation"
        word_ETH_CFG = 0x00000800UL;        // Init: CLK = 0x2
        if (SPEED_100 == dev->phydev->speed)
            word_ETH_CFG |= 0x00000001UL;
        if (DUPLEX_FULL == dev->phydev->duplex)
            word_ETH_CFG |= 0x00000002UL;
        if (cam)
            word_ETH_CFG |= 0x00000200UL;
        if (0 == rcv_bcast)
            word_ETH_CFG |= 0x00000020UL;
        if (1 == rlf)
            word_ETH_CFG |= 0x00000100UL;

        MHal_EMAC_Write_CFG(hemac->hal, word_ETH_CFG);
        hemac->bEthCfg = 1;
    }

    if (dev->phydev->link)
    {
        MHal_EMAC_update_speed_duplex(hemac->hal, dev->phydev->speed, dev->phydev->duplex);
        netif_carrier_on(dev);
        netif_start_queue(dev);
    }
    else
    {
        // unsigned long flags;

        if (!netif_queue_stopped(dev))
            netif_stop_queue(dev);
        if (netif_carrier_ok(dev))
            netif_carrier_off(dev);
        // spin_lock_irqsave(&hemac->mutexTXQ, flags);
        // skb_queue_reset(&(hemac->skb_queue_tx));
        // spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
    }
    spin_unlock_irqrestore(&hemac->mutexNetIf, flag1);

#if 0
    printk("[%s][%d] adjust phy (link, speed, duplex) = (%d, %d, %d, %d)\n", __FUNCTION__, __LINE__,
        dev->phydev->link, dev->phydev->speed, dev->phydev->duplex, dev->phydev->autoneg);
#endif
}

static int emac_phy_connect(struct net_device* netdev)
{
    struct emac_handle* hemac =(struct emac_handle*) netdev_priv(netdev);
    struct device_node* np = NULL;
    struct phy_device *phydev;

    np = of_parse_phandle(netdev->dev.of_node, "phy-handle", 0);
    if (!np && of_phy_is_fixed_link(netdev->dev.of_node))
         if (!of_phy_register_fixed_link(netdev->dev.of_node))
             np = of_node_get(netdev->dev.of_node);
    if (!np)
    {
        printk("[%s][%d] can not find phy-handle in dts\n", __FUNCTION__, __LINE__);
        return -ENODEV;
    }

#if 0
    if (0 > (phy_mode = of_get_phy_mode(np)))
    {
        printk("[%s][%d] incorrect phy-mode %d\n", __FUNCTION__, __LINE__, phy_mode);
        goto jmp_err_connect;
    }
#endif
#if 0
    printk("[%s][%d] phy_mode = %d\n", __FUNCTION__, __LINE__, hemac->phy_mode);
    printk("[%s][%d] of_phy_connect (netdev, np, emac_phy_link_adjust) = (0x%08x, 0x%08x, 0x%08x)\n",
        __FUNCTION__, __LINE__,
        (int)netdev, (int)np, (int)emac_phy_link_adjust);
#endif
    if (!(phydev = of_phy_connect(netdev, np, emac_phy_link_adjust, 0, hemac->phy_mode)))
    {
        printk("[%s][%d] could not connect to PHY\n", __FUNCTION__, __LINE__);
        goto jmp_err_connect;
    }

    phy_init_hw(phydev);

    printk("[%s][%d] connected mac %s to PHY at %s [uid=%08x, driver=%s]\n", __FUNCTION__, __LINE__,
        hemac->name, phydev_name(phydev), phydev->phy_id, phydev->drv->name);

    netdev->phydev->autoneg = AUTONEG_ENABLE;
    netdev->phydev->speed = 0;
    netdev->phydev->duplex = 0;
#if 0
    if (of_phy_is_fixed_link(netdev->dev.of_node))
        dev->phydev->supported |=
		SUPPORTED_Pause | SUPPORTED_Asym_Pause;
#endif
#if 0
	dev->phydev->supported &= PHY_GBIT_FEATURES | SUPPORTED_Pause |
				   SUPPORTED_Asym_Pause;
#endif

    // the speed has to be limited to 10 MBits/sec in FPGA
    {
        u32 max_speed = 0;
        if (!of_property_read_u32(netdev->dev.of_node, "max-speed", &max_speed))
        {
            switch (max_speed)
            {
            case 10:
                phy_set_max_speed(phydev, SPEED_10);
                break;
            case 100:
                phy_set_max_speed(phydev, SPEED_100);
                break;
            default:
                break;
            }
        }
    }
#if (EMAC_FLOW_CONTROL_TX || EMAC_FLOW_CONTROL_RX)
    netdev->phydev->supported &= (PHY_BASIC_FEATURES | SUPPORTED_Pause | SUPPORTED_Asym_Pause);
#else
    netdev->phydev->supported &= PHY_BASIC_FEATURES;
#endif
    netdev->phydev->advertising = netdev->phydev->supported | ADVERTISED_Autoneg;
    if (0 > phy_start_aneg(netdev->phydev))
    {
        printk("[%s][%d] phy_start_aneg fail\n", __FUNCTION__, __LINE__);
    }
    of_node_put(np);
    return 0;

jmp_err_connect:
    if (of_phy_is_fixed_link(netdev->dev.of_node))
	of_phy_deregister_fixed_link(netdev->dev.of_node);
    of_node_put(np);
    printk("[%s][%d]: invalid phy\n", __FUNCTION__, __LINE__);
    return -EINVAL;
}
#endif

static int __init MDev_EMAC_ndo_init(struct net_device *dev)
{
#if KERNEL_PHY
    emac_phy_connect(dev);
#endif
    return 0;
}

static void MDev_EMAC_ndo_uninit(struct net_device *dev)
{
#if KERNEL_PHY
    phy_disconnect(dev->phydev);
#endif
}

/*
static void MDev_EMAC_ndo_tx_timeout(struct net_device *dev)
{
}
*/

int MDev_EMAC_ndo_change_mtu(struct net_device *dev, int new_mtu)
{
    if ((new_mtu < 68) || (new_mtu > EMAC_MTU))
    {
        printk("[%s][%d] not support mtu size %d\n", __FUNCTION__, __LINE__, new_mtu);
        return -EINVAL;
    }
    printk("[%s][%d] change mtu size from %d to %d\n", __FUNCTION__, __LINE__, dev->mtu, new_mtu);
    dev->mtu = new_mtu;
    return 0;
}

static int sstar_emac_get_link_ksettings(struct net_device *ndev,
				  struct ethtool_link_ksettings *cmd)
{
    return phy_ethtool_ksettings_get(ndev->phydev, cmd);
}

static int sstar_emac_set_link_ksettings(struct net_device *ndev,
				  const struct ethtool_link_ksettings *cmd)
{
    return phy_ethtool_ksettings_set(ndev->phydev, cmd);
}

static int sstar_emac_nway_reset(struct net_device *dev)
{
    return genphy_restart_aneg(dev->phydev);
}

static u32 sstar_emac_get_link(struct net_device *dev)
{
    int err;

    err = genphy_update_link(dev->phydev);
    if (err)
        return ethtool_op_get_link(dev);
    return dev->phydev->link;
}

static int MDev_EMAC_set_features(struct net_device *netdev,
				netdev_features_t features)
{
/*
printk("[%s][%d] ##################################\n", __FUNCTION__, __LINE__);
        printk("[%s][%d] set features = 0x%08x\n", __FUNCTION__, __LINE__, (int)features);
printk("[%s][%d] ##################################\n", __FUNCTION__, __LINE__);
*/
	netdev->features = features;
	return 0;
}

static netdev_features_t MDev_EMAC_fix_features(
		struct net_device *netdev, netdev_features_t features)
{
/*
printk("[%s][%d] ##################################\n", __FUNCTION__, __LINE__);
    printk("[%s][%d] fix features = 0x%08x 0x%08x \n", __FUNCTION__, __LINE__, (int)features, (int)(features & (EMAC_FEATURES)));
printk("[%s][%d] ##################################\n", __FUNCTION__, __LINE__);
*/
    return (features & EMAC_FEATURES);
}

//-------------------------------------------------------------------------------------------------
// Initialize the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static const struct net_device_ops mstar_lan_netdev_ops = {
        .ndo_init               = MDev_EMAC_ndo_init,
        .ndo_uninit             = MDev_EMAC_ndo_uninit,
        // .ndo_tx_timeout         = MDev_EMAC_ndo_tx_timeout,
        .ndo_change_mtu         = MDev_EMAC_ndo_change_mtu,
        .ndo_open               = MDev_EMAC_open,
        .ndo_stop               = MDev_EMAC_close,
        .ndo_start_xmit         = MDev_EMAC_tx,
        .ndo_set_mac_address    = MDev_EMAC_set_mac_address,
        .ndo_set_rx_mode        = MDev_EMAC_set_rx_mode,
        .ndo_do_ioctl           = MDev_EMAC_ioctl,
        .ndo_get_stats          = MDev_EMAC_stats,
	.ndo_set_features = MDev_EMAC_set_features,
	.ndo_fix_features = MDev_EMAC_fix_features,
#ifdef CONFIG_NET_POLL_CONTROLLER
        .ndo_poll_controller    = MDev_EMAC_netpoll,
#endif

};

static const struct ethtool_ops sstar_emac_ethtool_ops = {
	.get_link_ksettings	= sstar_emac_get_link_ksettings,
	.set_link_ksettings	= sstar_emac_set_link_ksettings,
	// .get_drvinfo		= sstar_emac_get_drvinfo,
	// .get_msglevel		= sstar_emac_get_msglevel,
	// .set_msglevel		= sstar_emac_set_msglevel,
	.nway_reset		= sstar_emac_nway_reset,
	.get_link		= sstar_emac_get_link,
	// .get_strings		= sstar_emac_get_strings,
	// .get_sset_count		= sstar_emac_get_sset_count,
	// .get_ethtool_stats	= sstar_emac_get_ethtool_stats,
	// .get_rxnfc		= sstar_emac_get_rxnfc,
	// .set_rxnfc              = sstar_emac_set_rxnfc,
};

#if 0
static int MDev_EMAC_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    struct emac_handle *hemac =(struct emac_handle *) netdev_priv(dev);

    mii_ethtool_gset (&hemac->mii, cmd);

    return 0;
}

static int MDev_EMAC_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    struct emac_handle *hemac =(struct emac_handle *) netdev_priv(dev);

    mii_ethtool_sset (&hemac->mii, cmd);

    return 0;
}

static int MDev_EMAC_nway_reset(struct net_device *dev)
{
    struct emac_handle *hemac =(struct emac_handle *) netdev_priv(dev);

    mii_nway_restart (&hemac->mii);

    return 0;
}

static u32 MDev_EMAC_get_link(struct net_device *dev)
{
    u32    u32data;
    struct emac_handle *hemac =(struct emac_handle *) netdev_priv(dev);

    u32data = mii_link_ok (&hemac->mii);

    return u32data;
}

static const struct ethtool_ops ethtool_ops = {
    .get_settings = MDev_EMAC_get_settings,
    .set_settings = MDev_EMAC_set_settings,
    .nway_reset   = MDev_EMAC_nway_reset,
    .get_link     = MDev_EMAC_get_link,
};
#endif

#endif

static ssize_t dlist_info_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    u32 input;
    int idx=0;

    input = simple_strtoul(buf, NULL, 10);

    if(0==input)
    {
        RBNA_detailed=0;
    }
    else if(1==input)
    {
        RBNA_detailed=1;
    }
    else if(2==input)
    {
        max_rx_packet_count=0;
        max_tx_packet_count=0;
        min_tx_fifo_idle_count=0xffff;
    }
    else if(3==input)
    {
        for(idx=0; idx<sizeof(hemac->irq_count)/sizeof(u32); idx++)
            hemac->irq_count[idx]=0;
    }

    return count;
}

static ssize_t dlist_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int idx=0;
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    rx_desc_queue_t* rxinfo = &(hemac->rx_desc_queue);
    int empty=0;
    int max= rxinfo->num_desc;
    u32 u32RBQP_Addr=0;
    char descriptor_maps[RX_DESC_NUM];

    for(idx=0;idx<rxinfo->num_desc;idx++)
    {
        // if(!((dlist->descriptors[idx].addr) & EMAC_DESC_DONE))
        if(!((rxinfo->desc[idx].addr) & EMAC_DESC_DONE))
        {
            empty++;
            descriptor_maps[idx]=1;
        }
        else
        {
            descriptor_maps[idx]=0;
        }
    }
    // u32RBQP_Addr =( MHal_EMAC_Read_RBQP()-(RBQP_BASE - MIU0_BUS_BASE))/RBQP_HW_BYTES;
    u32RBQP_Addr = BUS2PA(MHal_EMAC_Read_RBQP(hemac->hal))/RX_DESC_SIZE;
    str += scnprintf(str, end - str, "%s=0x%x\n", "RBQP_size", max);
    str += scnprintf(str, end - str, "empty=0x%x, hemac->rxBuffIndex=0x%x, u32RBQP_Addr=0x%x\n",
                        // empty, hemac->rxBuffIndex, u32RBQP_Addr);
                        empty, rxinfo->idx, u32RBQP_Addr);
    #define CHANGE_LINE_LENG 0x20
    for(idx=0;idx<rxinfo->num_desc;idx++)
    {
        if(idx%CHANGE_LINE_LENG==0x0)
            str += scnprintf(str, end - str, "0x%03x: ", idx);

        str += scnprintf(str, end - str, "%d", descriptor_maps[idx]);

        if(idx%0x10==(0xF))
            str += scnprintf(str, end - str, " ");

        if(idx%CHANGE_LINE_LENG==(CHANGE_LINE_LENG-1))
            str += scnprintf(str, end - str, "\n");
    }

    str += scnprintf(str, end - str, "%s=%d\n", "max_rx_packet_count", max_rx_packet_count);

    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_DONE", hemac->irq_count[IDX_CNT_INT_DONE]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_RCOM", hemac->irq_count[IDX_CNT_INT_RCOM]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_RBNA", hemac->irq_count[IDX_CNT_INT_RBNA]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_TOVR", hemac->irq_count[IDX_CNT_INT_TOVR]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_TUND", hemac->irq_count[IDX_CNT_INT_TUND]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_RTRY", hemac->irq_count[IDX_CNT_INT_RTRY]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_TCOM", hemac->irq_count[IDX_CNT_INT_TCOM]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_ROVR", hemac->irq_count[IDX_CNT_INT_ROVR]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_JULIAN_D", hemac->irq_count[IDX_CNT_JULIAN_D]);
/*
    str += scnprintf(str, end - str, "%s=%d\n", "MonitorAnStates1", hemac->gu32PhyResetCount1);
    str += scnprintf(str, end - str, "%s=%d\n", "MonitorAnStates2", hemac->gu32PhyResetCount2);
    str += scnprintf(str, end - str, "%s=%d\n", "MonitorAnStates3", hemac->gu32PhyResetCount3);
    str += scnprintf(str, end - str, "%s=%d\n", "MonitorAnStates4", hemac->gu32PhyResetCount4);
    str += scnprintf(str, end - str, "%s=%d\n", "RESET count", hemac->gu32PhyResetCount);
*/
    str += scnprintf(str, end - str, "%s=%d\n", "skb_tx_send", hemac->skb_tx_send);
    str += scnprintf(str, end - str, "%s=%d\n", "skb_tx_free", hemac->skb_tx_free);
    str += scnprintf(str, end - str, "%s=%d\n", "rx_duration_max", rx_duration_max);
    str += scnprintf(str, end - str, "%s=%d\n", "rx_packet_cnt", rx_packet_cnt);

/*
    str += scnprintf(str, end - str, "%s=%d\n", "tx_used_hw", MHal_EMAC_TXQ_Used(hemac->hal));
    str += scnprintf(str, end - str, "%s=%d\n", "tx_free_hw", MHal_EMAC_TXQ_Free());

    str += scnprintf(str, end - str, "%s=%d\n", "tx_used_0", skb_queue_used(&hemac->skb_queue_tx, 0));
    str += scnprintf(str, end - str, "%s=%d\n", "tx_free_0", skb_queue_free(&hemac->skb_queue_tx, 0));

    str += scnprintf(str, end - str, "%s=%d\n", "tx_used_1", skb_queue_used(&hemac->skb_queue_tx, 1));
    str += scnprintf(str, end - str, "%s=%d\n", "tx_free_1", skb_queue_free(&hemac->skb_queue_tx, 1));

    str += scnprintf(str, end - str, "%s=%d\n", "tx_used_2", skb_queue_used(&hemac->skb_queue_tx, 2));

    str += scnprintf(str, end - str, "(size0, size1) = (%d, %d)\n", hemac->skb_queue_tx.size[0], hemac->skb_queue_tx.size[1]);
    str += scnprintf(str, end - str, "read =           %d\n", hemac->skb_queue_tx.read);
    str += scnprintf(str, end - str, "rw =             %d\n", hemac->skb_queue_tx.rw);
    str += scnprintf(str, end - str, "write =          %d\n", hemac->skb_queue_tx.write);
*/
    {
        struct timespec ct;
        int duration;
        u64 data_done_ct;
        unsigned long flags;
	u32 txPkt_ct = hemac->skb_tx_send;
	u32 txInt_ct = hemac->irq_count[IDX_CNT_INT_TCOM];

        getnstimeofday(&ct);
        duration = (ct.tv_sec - hemac->data_time_last.tv_sec)*1000 + (ct.tv_nsec - hemac->data_time_last.tv_nsec)/1000000;
        spin_lock_irqsave(&hemac->emac_data_done_lock, flags);
        data_done_ct = hemac->data_done;
        hemac->data_done = 0;
        spin_unlock_irqrestore(&hemac->emac_data_done_lock, flags);

        // tx_duration_max = (tx_duration_max < duration) ? duration : tx_duration_max;
        hemac->data_time_last = ct;
        str += scnprintf(str, end - str, "%s=%lld\n", "data_done", data_done_ct);
        str += scnprintf(str, end - str, "%s=%d\n", "data_duration", duration);
        do_div(data_done_ct, duration);
        str += scnprintf(str, end - str, "%s=%lld\n", "data_average", data_done_ct);
        str += scnprintf(str, end - str, "%s=%d\n", "tx_pkt (duration)", txPkt_ct - hemac->txPkt);
        str += scnprintf(str, end - str, "%s=%d\n", "tx_int (duration)", txInt_ct - hemac->txInt);
	hemac->txPkt = txPkt_ct;
	hemac->txInt = txInt_ct;
    }
    str += scnprintf(str, end - str, "%s=%d\n", "MHal_EMAC_TXQ_Mode", MHal_EMAC_TXQ_Mode(hemac->hal));
#if EMAC_SG
    str += scnprintf(str, end - str, "%s=%d\n", "maxSG", hemac->maxSG);
#endif // #if #if EMAC_SG
    return (str - buf);
}
DEVICE_ATTR(dlist_info, 0644, dlist_info_show, dlist_info_store);

static ssize_t info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    rx_desc_queue_t* rxinfo = &(hemac->rx_desc_queue);

    // str += scnprintf(str, end - str, "%s %s\n", __DATE__, __TIME__);
    str += scnprintf(str, end - str, "RAM_ALLOC_SIZE=0x%08x\n", rxinfo->size_desc_queue);
    str += scnprintf(str, end - str, "RAM_VA_BASE=0x%08x\n", (int)rxinfo->desc);
    str += scnprintf(str, end - str, "RAM_PA_BASE=0x%08x\n", (int)VIRT2PA(rxinfo->desc));

#if MSTAR_EMAC_NAPI
    str += scnprintf(str, end - str, "NAPI enabled, NAPI_weight=%d\n", EMAC_NAPI_WEIGHT);
#endif
    str += scnprintf(str, end - str, "ZERO_COPY enabled\n");
#ifdef NEW_TX_QUEUE_INTERRUPT_THRESHOLD
    str += scnprintf(str, end - str, "NEW_TX_QUEUE_INTERRUPT_THRESHOLD enabled\n");
#endif

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}
DEVICE_ATTR(info, 0444, info_show, NULL);


//struct timeval proc_read_time;
static ssize_t tx_sw_queue_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int idleCount=0;

    idleCount= MHal_EMAC_TXQ_Free(hemac->hal);
    str += scnprintf(str, end - str,
    "netif_queue_stopped=%d \n idleCount=%d \n irqcnt=%d, tx_irqcnt=%d \n tx_bytes_per_timerbak=%d \n min_tx_fifo_idle_count=%d \n",
            netif_queue_stopped(netdev),
            idleCount, hemac->irqcnt, hemac->tx_irqcnt,
            tx_bytes_per_timerbak,
            min_tx_fifo_idle_count);

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

DEVICE_ATTR(tx_sw_queue_info, 0444, tx_sw_queue_info_show, NULL);


static ssize_t reverse_led_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    u32 input;

    input = simple_strtoul(buf, NULL, 10);
    MHal_EMAC_Set_Reverse_LED(hemac->hal, input);
    return count;
}
static ssize_t reverse_led_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    u8 u8reg=0;
    u8reg = MHal_EMAC_Get_Reverse_LED(hemac->hal);
    return sprintf(buf, "%d\n", u8reg);
}
DEVICE_ATTR(reverse_led, 0644, reverse_led_show, reverse_led_store);


static ssize_t check_link_time_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    u32 input;

    input = simple_strtoul(buf, NULL, 10);
    hemac->gu32CheckLinkTime = input;
    return count;
}
static ssize_t check_link_time_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);

    return sprintf(buf, "%d\n", hemac->gu32CheckLinkTime);
}
DEVICE_ATTR(check_link_time, 0644, check_link_time_show, check_link_time_store);

static ssize_t check_link_timedis_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    u32 input;

    input = simple_strtoul(buf, NULL, 10);
    hemac->gu32CheckLinkTimeDis = input;
    return count;
}
static ssize_t check_link_timedis_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    return sprintf(buf, "%d\n", hemac->gu32CheckLinkTimeDis);
}
DEVICE_ATTR(check_link_timedis, 0644, check_link_timedis_show, check_link_timedis_store);

static ssize_t sw_led_flick_speed_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    u32 input;

    input = simple_strtoul(buf, NULL, 10);
    hemac->led_flick_speed = input;
    return count;
}
static ssize_t sw_led_flick_speed_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);

    return sprintf(buf, "LED flick speed, the smaller the faster\n%d\n", hemac->led_flick_speed);
}
DEVICE_ATTR(sw_led_flick_speed, 0644, sw_led_flick_speed_show, sw_led_flick_speed_store);

#if DYNAMIC_INT_TX_TIMER
#if DYNAMIC_INT_TX_TIMER_HR
static enum hrtimer_restart _MDev_EMAC_TxWdt_CB(struct hrtimer *timer)
{
    struct emac_handle *hemac = container_of(timer, struct emac_handle, timerTxWdt);

    if (!hemac->timerTxWdtPeriod)
	return HRTIMER_NORESTART;

    MHal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 1);
    hrtimer_forward_now(&hemac->timerTxWdt, ns_to_ktime(hemac->timerTxWdtPeriod*1000));
    return HRTIMER_RESTART;
}
#else
static void _MDev_EMAC_TxWdt_CB(unsigned long data)
{
    struct emac_handle *hemac = (struct emac_handle*)data;

    if (!hemac->timerTxWdtPeriod)
        return;

    MHal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 1);
    hemac->timerTxWdt.expires = jiffies + ((hemac->timerTxWdtPeriod)*HZ)/1000000;
    add_timer(&hemac->timerTxWdt);
}
#endif
#endif

extern void MHal_EMAC_phy_trunMax(void*);
// extern void MHal_EMAC_trim_phy(void*);
static ssize_t turndrv_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    // u32 input;
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);

#if 0
    if(!strncmp(buf, "0",strlen("0")))
    {
        MHal_EMAC_trim_phy(hemac->hal);
        return count;
    }
#endif

    if(!strncmp(buf, "max",strlen("max")))
    {
        MHal_EMAC_phy_trunMax(hemac->hal);
        return count;
    }

    if(!strncmp(buf, "f10t",strlen("10t")))
    {
/*
        //force to set 10M on FPGA
        MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_ADVERTISE, 0x0061UL);
        mdelay(10);
        MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1200UL);
*/
        phy_set_max_speed(netdev->phydev, SPEED_10);
        return count;
    }
    if(!strncmp(buf, "an",strlen("an")))
    {
/*
        //force to set 10M on FPGA
        MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_ADVERTISE, 0x01e1UL);
        mdelay(10);
        MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1200UL);
*/
        netdev->phydev->advertising = netdev->phydev->supported | ADVERTISED_Autoneg;
        phy_start_aneg(netdev->phydev);
        return count;
    }
#if DYNAMIC_INT_RX
    if(!strncmp(buf, "dir_on", strlen("dir_on")))
    {
        hemac->rx_stats_enable = 1;
    }
    if(!strncmp(buf, "dir_off", strlen("dir_off")))
    {
        hemac->rx_stats_enable = 0;
    }
#endif

#if DYNAMIC_INT_TX_TIMER
    {
        unsigned char cmd[16];
        int param;
        if (2 == sscanf(buf, "%s %d", cmd, &param))
        {
            if (0 == strcasecmp(cmd, "tx_int"))
            {
#if DYNAMIC_INT_TX_TIMER_HR
                hemac->timerTxWdtPeriod = param;
                hrtimer_try_to_cancel(&hemac->timerTxWdt);
                hrtimer_start(&hemac->timerTxWdt, ns_to_ktime(hemac->timerTxWdtPeriod*1000), HRTIMER_MODE_REL);
#else
                int prev = hemac->timerTxWdtPeriod;
                unsigned long expires;

                hemac->timerTxWdtPeriod = param;
                expires = jiffies + ((hemac->timerTxWdtPeriod)*HZ)/1000000;
                if (hemac->timerTxWdtPeriod)
                {
                    if (!prev)
                    {
                        hemac->timerTxWdt.expires = expires;
                        add_timer(&hemac->timerTxWdt);
                    }
                }
#endif
                // printk("[%s][%d] timer %d\n", __FUNCTION__, __LINE__, hemac->timerTxWdtPeriod);
            }
        }
    }
#endif

    {
        unsigned char cmd[16];
        int param;

        if (2 == sscanf(buf, "%s %d", cmd, &param))
        {
            if (!strncmp(cmd, "swing_100",strlen("swing_100")))
            {
                u32 val;
                int tmp;
                MHal_EMAC_read_phy(hemac->hal, 0, 0x142, &val);
                tmp = ((val >> 8) & 0x1f);
                
#if 0
                if (tmp & 0x10)
                {
                    param = -param;
                }
                tmp = ((tmp + param) & 0x1f) << 8;
#else
                if (tmp & 0x10)
                {
                    tmp = tmp & 0xf;
                    tmp ++;
                    tmp = -tmp;
                }
                tmp += param;
                if (tmp > 15)
                    tmp = 15;
                if (tmp < -16)
                    tmp = -16;
                if (tmp < 0)
                {
                    tmp = -tmp;
                    tmp --;
                    tmp = tmp | 0x10;
                }
                tmp = tmp << 8;
#endif
                val = (val & ~0x1f00) | tmp;
                MHal_EMAC_write_phy(hemac->hal, 0, 0x142, val);
            }
        }
    }

    // input = simple_strtoul(buf, NULL, 10);
    return count;
}

static ssize_t turndrv_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return 0;
}

DEVICE_ATTR(turndrv, 0644, turndrv_show, turndrv_store);

static int MDev_EMAC_setup (struct net_device *dev)
{
    struct emac_handle *hemac;
    // dma_addr_t dmaaddr;
    void* RetAddr;
    unsigned long flags;
#ifdef CONFIG_MSTAR_HW_TX_CHECKSUM
    u32 retval;
#endif

    hemac = (struct emac_handle *) netdev_priv(dev);

    if (hemac->bInit)
    // if (already_initialized)
    {
        printk("[%s][%d] %s has been initiated\n", __FUNCTION__, __LINE__, hemac->name);
        return FALSE;
    }
    if (hemac == NULL)
    {
        free_irq (dev->irq, dev);
        EMAC_ERR("hemac fail\n");
        return -ENOMEM;
    }

#if defined ISR_BOTTOM_HALF
    /*Init tx and rx tasks*/
    INIT_WORK(&hemac->rx_task, MDev_EMAC_bottom_rx_task);
#endif
#ifdef TX_SOFTWARE_QUEUE
    INIT_WORK(&hemac->tx_task, MDev_EMAC_bottom_tx_task);
#endif

    hemac->netdev = dev;

    // skb_queue_create(&(hemac->skb_queue_tx), MHal_EMAC_TXQ_Size(), 256);
    // skb_queue_create(&(hemac->skb_queue_tx), MHal_EMAC_TXQ_Size(), MHal_EMAC_TXQ_Size());
    // RetAddr = MDev_EMAC_VarInit();
    RetAddr = MDev_EMAC_VarInit(hemac);
    if(!RetAddr)
    {
        EMAC_ERR("Var init fail!!\n");
        return FALSE;
    }
    skb_queue_create(&(hemac->skb_queue_tx), MHal_EMAC_TXQ_Size(hemac->hal), MHal_EMAC_TXQ_Size(hemac->hal)+ hemac->txq_num_sw);

    // dev->base_addr = (long) (EMAC_RIU_REG_BASE+REG_BANK_EMAC0*0x200); // seems useless

    // spin_lock_init(&hemac->mutexRXD);
    MDev_EMAC_HW_init(dev);

    // hemac->lock = &emac_lock;
    spin_lock_init(&hemac->mutexNetIf);
    spin_lock_init(&hemac->mutexPhy);
    spin_lock_init(&hemac->mutexTXQ);
#if (0 == MSTAR_EMAC_NAPI)
    spin_lock_init(&hemac->mutexRXInt);
#endif

#if EMAC_FLOW_CONTROL_TX
    spin_lock_init(&hemac->mutexFlowTX);
    init_timer(&hemac->timerFlowTX);
    hemac->timerFlowTX.data = (unsigned long)hemac;
    hemac->timerFlowTX.expires = jiffies;
    hemac->timerFlowTX.function = _MDev_EMAC_FlowTX_CB;
    // hemac->isPauseTX = 0;
#endif

#if REDUCE_CPU_FOR_RBNA
    spin_lock_init(&hemac->mutexIntRX);
    init_timer(&hemac->timerIntRX);
    hemac->timerIntRX.data = (unsigned long)hemac;
    hemac->timerIntRX.expires = jiffies;
    hemac->timerIntRX.function = _MDev_EMAC_IntRX_CB;
#endif

#if DYNAMIC_INT_RX
    // getnstimeofday(&hemac->rx_stats_time);
    hemac->rx_stats_packet = 0xFFFFFFFF;
    hemac->rx_stats_enable = 1;
#endif

    ether_setup (dev);
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,28)
    dev->open = MDev_EMAC_open;
    dev->stop = MDev_EMAC_close;
    dev->hard_start_xmit = MDev_EMAC_tx;
    dev->get_stats = MDev_EMAC_stats;
    dev->set_multicast_list = MDev_EMAC_set_rx_mode;
    dev->do_ioctl = MDev_EMAC_ioctl;
    dev->set_mac_address = MDev_EMAC_set_mac_address;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    dev->netdev_ops = &mstar_lan_netdev_ops;
#endif
    dev->tx_queue_len = EMAC_MAX_TX_QUEUE;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    ////SET_ETHTOOL_OPS(dev, &ethtool_ops);
    //EMAC_TODO("set Ethtool_ops\n");
    // netdev_set_default_ethtool_ops(dev, &ethtool_ops);
#endif
    spin_lock_irqsave(&hemac->mutexPhy, flags);
    MDev_EMAC_get_mac_address (dev);    // Get ethernet address and store it in dev->dev_addr //
    MDev_EMAC_update_mac_address (dev); // Program ethernet address into MAC //
    MHal_EMAC_enable_mdi(hemac->hal);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);

#if 0
    //Support for ethtool //
    hemac->mii.dev = dev;
    hemac->mii.mdio_read = MDev_EMAC_mdio_read;
    hemac->mii.mdio_write = MDev_EMAC_mdio_write;
#endif
    // already_initialized = 1;
    // hemac->bInit = 1;
#ifdef CONFIG_MSTAR_HW_TX_CHECKSUM
    dev->features |= NETIF_F_IP_CSUM;
#endif

    dev->features |= EMAC_FEATURES;
    dev->vlan_features |= EMAC_FEATURES;


    hemac->irqcnt=0;
    hemac->tx_irqcnt=0;

    // printk("[%s][%d] (irq_emac, irq_lan) = (%d, %d)\n", __FUNCTION__, __LINE__, hemac->irq_emac, hemac->irq_lan);
    // printk("[%s][%d] %d\n", __FUNCTION__, __LINE__, dev->irq);
    dev->irq = hemac->irq_emac;
    // dev->irq = irq_of_parse_and_map(dev->dev.of_node, 0);
    if (!dev->irq)
    {
        EMAC_ERR("Get irq number0 error from DTS\n");
        return -EPROBE_DEFER;
    }

    // printk("[%s][%d] request irq for %d %d\n", __FUNCTION__, __LINE__, dev->irq, hemac->irq_emac);
    //Install the interrupt handler //
    //Notes: Modify linux/kernel/irq/manage.c  /* interrupt.h */
    if (request_irq(dev->irq, MDev_EMAC_interrupt, 0/*SA_INTERRUPT*/, dev->name, dev))
        return -EBUSY;

#ifdef LAN_ESD_CARRIER_INTERRUPT
    // val = irq_of_parse_and_map(dev->dev.of_node, 1);
    val = hemac->irq_lan;
    if (!val)
    {
        EMAC_ERR("Get irq number0 error from DTS\n");
        return -EPROBE_DEFER;
    }
    if (request_irq(val/*INT_FIQ_LAN_ESD+32*/, MDev_EMAC_interrupt_cable_unplug, 0/*SA_INTERRUPT*/, dev->name, dev))
        return -EBUSY;
#endif
    //Determine current link speed //
    spin_lock_irqsave(&hemac->mutexPhy, flags);
    // MDev_EMAC_update_linkspeed (dev);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);

#if DYNAMIC_INT_TX_TIMER

#if DYNAMIC_INT_TX_TIMER_HR
    hrtimer_init(&hemac->timerTxWdt, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hemac->timerTxWdt.function = _MDev_EMAC_TxWdt_CB;
#else
    init_timer(&hemac->timerTxWdt);
    hemac->timerTxWdt.data = (unsigned long)hemac;
    // hemac->timerDummy.expires = jiffies + HZ/10;
    hemac->timerTxWdt.function = _MDev_EMAC_TxWdt_CB;
    hemac->timerTxWdtPeriod = 0;
    // add_timer(&hemac->timerDummy);
#endif

#endif
    alloc_chrdev_region(&gEthDev, 0, MINOR_EMAC_NUM, "ETH");
    hemac->mstar_class_emac_device = device_create(msys_get_sysfs_class(), NULL, MKDEV(MAJOR(gEthDev), hemac->u8Minor), NULL, hemac->name);
    dev_set_drvdata(hemac->mstar_class_emac_device, (void*)dev);
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_tx_sw_queue_info);
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_dlist_info);
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_reverse_led);
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_check_link_time);
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_check_link_timedis);
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_info);
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_sw_led_flick_speed);
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_turndrv);
#if 0//ajtest
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_ajtest_recv_count);
#endif
#if defined(PACKET_DUMP)
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_tx_dump);
    device_create_file(hemac->mstar_class_emac_device, &dev_attr_rx_dump);
#endif
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Restar the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------
static int MDev_EMAC_SwReset(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(dev);
    u32 oldCFG, oldCTL;
    // u32 retval;
    unsigned long flags;

    spin_lock_irqsave(&hemac->mutexPhy, flags);
    MDev_EMAC_get_mac_address (dev);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);
    oldCFG = MHal_EMAC_Read_CFG(hemac->hal);
    oldCTL = MHal_EMAC_Read_CTL(hemac->hal) & ~(EMAC_TE | EMAC_RE);

    skb_queue_reset(&(hemac->skb_queue_tx));
    free_rx_skb(hemac); // @FIXME : how about RX descriptor
    netif_stop_queue (dev);

    /*
    retval = MHal_EMAC_Read_JULIAN_0100(hemac->hal);
    MHal_EMAC_Write_JULIAN_0100(hemac->hal, retval & 0x00000FFFUL);
    MHal_EMAC_Write_JULIAN_0100(hemac->hal, retval);
    */
    MHal_EMAC_Write_JULIAN_0100(hemac->hal, 1);

    MDev_EMAC_HW_init(dev);
    MHal_EMAC_Write_CFG(hemac->hal, oldCFG);
    MHal_EMAC_Write_CTL(hemac->hal, oldCTL);

    spin_lock_irqsave(&hemac->mutexPhy, flags);
    MHal_EMAC_enable_mdi(hemac->hal);
    MDev_EMAC_update_mac_address (dev); // Program ethernet address into MAC //
    // (void)MDev_EMAC_update_linkspeed (dev);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);

    MHal_EMAC_IntEnable(hemac->hal, 0xFFFFFFFF, 0);
    MHal_EMAC_IntEnable(hemac->hal, hemac->gu32intrEnable, 1);

    MDev_EMAC_start(dev);
    MDev_EMAC_set_rx_mode(dev);
    netif_start_queue (dev);
    hemac->contiROVR = 0;
    EMAC_ERR("=> Take %lu ms to reset EMAC!\n", (getCurMs() - hemac->oldTime));
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Detect MAC and PHY and perform initialization
//-------------------------------------------------------------------------------------------------
#if defined (CONFIG_OF)
static struct of_device_id mstaremac_of_device_ids[] = {
         {.compatible = "sstar-emac"},
         {},
};
#endif

static int MDev_EMAC_probe (struct net_device *dev)
{
    int detected;
    /* Read the PHY ID registers - try all addresses */
    detected = MDev_EMAC_setup(dev);
    return detected;
}

//-------------------------------------------------------------------------------------------------
// EMAC Timer to detect cable pluged/unplugged
//-------------------------------------------------------------------------------------------------
#if 0
static void MDev_EMAC_timer_callback(unsigned long value)
{
    int ret = 0;
/*
    struct net_device* netdev = (struct net_device*) dev_get_drvdata(dev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
*/
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(emac_dev);

    static u32 bmsr, time_count = 0;
    unsigned long flags;

#if (EMAC_FLOW_CONTROL_TX == EMAC_FLOW_CONTROL_TX_SW)
    if (EMAC_FLOW_CTL_TMR == value)
    {
        netif_wake_queue((struct net_device *)emac_dev);
        spin_lock_irq(&emac_flow_ctl_lock);
        eth_pause_cmd_enable = 0;
        spin_unlock_irq(&emac_flow_ctl_lock);
        return;
    }
#endif

    if(gu32GatingRxIrqTimes)
    {
        MHal_EMAC_RX_Param(0x01, 0x01);
        gu32GatingRxIrqTimes--;
    }
    spin_lock_irqsave(&hemac->mutexPhy, flags);
    ret = MDev_EMAC_update_linkspeed(emac_dev);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);

    tx_bytes_per_timerbak = tx_bytes_per_timer;
    tx_bytes_per_timer = 0;

    if (0 == ret)
    {
        if (!hemac->ThisBCE.connected)
        {
            hemac->ThisBCE.connected = 1;
            netif_carrier_on(emac_dev);
            netif_start_queue(emac_dev);
            //EMAC_ERR("connected\n");
        }

        if(hemac->led_orange!=-1 && hemac->led_green!=-1)
        {
            MDrv_GPIO_Set_High(hemac->led_orange);
            MDrv_GPIO_Set_High(hemac->led_green);
        }

        // Link status is latched, so read twice to get current value //
        MHal_EMAC_read_phy (hemac->phyaddr, MII_BMSR, &bmsr);
        MHal_EMAC_read_phy (hemac->phyaddr, MII_BMSR, &bmsr);
        time_count = 0;
        spin_lock_irqsave(&hemac->mutexPhy, flags);
        phy_status_register = bmsr;
        spin_unlock_irqrestore(&hemac->mutexPhy, flags);
        // Normally, time out sets 1 Sec.
        hemac->timer_link.expires = jiffies + gu32CheckLinkTime;
    }
    else    //no link
    {
        if(hemac->ThisBCE.connected) {
            hemac->ThisBCE.connected = 0;
        }

        if(hemac->led_orange!=-1 && hemac->led_green!=-1)
        {
            MDrv_GPIO_Set_Low(hemac->led_orange);
            MDrv_GPIO_Set_Low(hemac->led_green);
        }

        // If disconnected is over 3 Sec, the real value of PHY's status register will report to application.
        if(time_count > CONFIG_DISCONNECT_DELAY_S*(HZ/gu32CheckLinkTimeDis)) {
            // Link status is latched, so read twice to get current value //
            MHal_EMAC_read_phy (hemac->phyaddr, MII_BMSR, &bmsr);
            MHal_EMAC_read_phy (hemac->phyaddr, MII_BMSR, &bmsr);

            // Report to kernel.
            if (netif_carrier_ok(emac_dev))
                netif_carrier_off(emac_dev);
            if (!netif_queue_stopped(emac_dev))
                netif_stop_queue(emac_dev);

            spin_lock_irqsave(&hemac->mutexPhy, flags);
            phy_status_register = bmsr;
#ifdef TX_SW_QUEUE
            _MDev_EMAC_tx_reset_TX_SW_QUEUE(emac_dev);
#endif
            spin_unlock_irqrestore(&hemac->mutexPhy, flags);
            hemac->ThisBCE.connected = 0;
            // Normally, time out is set 1 Sec.
            hemac->timer_link.expires = jiffies + gu32CheckLinkTime;
        }
        else// if(time_count <= CONFIG_DISCONNECT_DELAY_S*10)
        {
            time_count++;
            // Time out is set 100ms. Quickly checks next phy status.
            hemac->timer_link.expires = jiffies + gu32CheckLinkTimeDis;
            //EMAC_ERR("disconnect\n");
        }
    }
    add_timer(&hemac->timer_link);
}
#endif

#if 0
static void MDev_EMAC_timer_LinkStatus(unsigned long data)
{
    int ret = 0;
    struct net_device* netdev = (struct net_device*)data; 
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    static u32 bmsr, time_count = 0;
    unsigned long flags;

    // printk("[%s][%d] aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n", __FUNCTION__, __LINE__);
    spin_lock_irqsave(&hemac->mutexPhy, flags);
    ret = MDev_EMAC_update_linkspeed(netdev);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);

    tx_bytes_per_timerbak = tx_bytes_per_timer;
    tx_bytes_per_timer = 0;

    if (0 == ret)
    {
        if (!hemac->ThisBCE.connected)
        {
            hemac->ThisBCE.connected = 1;
            netif_carrier_on(netdev);
            netif_start_queue(netdev);
            //EMAC_ERR("connected\n");
        }

        if(hemac->led_orange!=-1 && hemac->led_green!=-1)
        {
            MDrv_GPIO_Set_High(hemac->led_orange);
            MDrv_GPIO_Set_High(hemac->led_green);
        }

        // Link status is latched, so read twice to get current value //
        MHal_EMAC_read_phy (hemac->hal, hemac->phyaddr, MII_BMSR, &bmsr);
        MHal_EMAC_read_phy (hemac->hal, hemac->phyaddr, MII_BMSR, &bmsr);
        time_count = 0;
        spin_lock_irqsave(&hemac->mutexPhy, flags);
        hemac->phy_status_register = bmsr;
        spin_unlock_irqrestore(&hemac->mutexPhy, flags);
        // Normally, time out sets 1 Sec.
        hemac->timer_link.expires = jiffies + hemac->gu32CheckLinkTime;
    }
    else    //no link
    {
        if(hemac->ThisBCE.connected) {
            hemac->ThisBCE.connected = 0;
        }

        if(hemac->led_orange!=-1 && hemac->led_green!=-1)
        {
            MDrv_GPIO_Set_Low(hemac->led_orange);
            MDrv_GPIO_Set_Low(hemac->led_green);
        }

        // If disconnected is over 3 Sec, the real value of PHY's status register will report to application.
        if(time_count > CONFIG_DISCONNECT_DELAY_S*(HZ/hemac->gu32CheckLinkTimeDis)) {
            // Link status is latched, so read twice to get current value //
            MHal_EMAC_read_phy (hemac->hal, hemac->phyaddr, MII_BMSR, &bmsr);
            MHal_EMAC_read_phy (hemac->hal, hemac->phyaddr, MII_BMSR, &bmsr);

            // Report to kernel.
            if (netif_carrier_ok(netdev))
                netif_carrier_off(netdev);
            if (!netif_queue_stopped(netdev))
                netif_stop_queue(netdev);

            spin_lock_irqsave(&hemac->mutexPhy, flags);
            hemac->phy_status_register = bmsr;

#ifdef TX_SW_QUEUE
            _MDev_EMAC_tx_reset_TX_SW_QUEUE(netdev);
#endif
            skb_queue_reset(&(hemac->skb_queue_tx));

            spin_unlock_irqrestore(&hemac->mutexPhy, flags);
            hemac->ThisBCE.connected = 0;
            // Normally, time out is set 1 Sec.
            hemac->timer_link.expires = jiffies + hemac->gu32CheckLinkTime;
        }
        else// if(time_count <= CONFIG_DISCONNECT_DELAY_S*10)
        {
            time_count++;
            // Time out is set 100ms. Quickly checks next phy status.
            hemac->timer_link.expires = jiffies + hemac->gu32CheckLinkTimeDis;
            //EMAC_ERR("disconnect\n");
        }
    }
    add_timer(&hemac->timer_link);
}
#endif

//-------------------------------------------------------------------------------------------------
// EMAC MACADDR Setup
//-------------------------------------------------------------------------------------------------

#ifndef MODULE

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

        MY_MAC[0] = (u8)simple_strtoul(&(addrs[0]),  NULL, 16);
        MY_MAC[1] = (u8)simple_strtoul(&(addrs[3]),  NULL, 16);
        MY_MAC[2] = (u8)simple_strtoul(&(addrs[6]),  NULL, 16);
        MY_MAC[3] = (u8)simple_strtoul(&(addrs[9]),  NULL, 16);
        MY_MAC[4] = (u8)simple_strtoul(&(addrs[12]), NULL, 16);
        MY_MAC[5] = (u8)simple_strtoul(&(addrs[15]), NULL, 16);

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
#endif

//-------------------------------------------------------------------------------------------------
// EMAC init module
//-------------------------------------------------------------------------------------------------
#if 0
static int MDev_EMAC_ScanPhyAddr(struct net_device* netdev)
{
    unsigned char addr = 0;
    u32 value = 0;
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);

    MHal_EMAC_Write_JULIAN_0100(hemac->hal, JULIAN_100_VAL);

    MHal_EMAC_enable_mdi(hemac->hal);
    do
    {
        MHal_EMAC_read_phy(hemac->hal, addr, MII_BMSR, &value);
        if (0 != value && 0x0000FFFFUL != value)
        {

            EMAC_DBG("[ PHY Addr:%d ] ==> :%x\n", addr, value);
            break;
        }
    }while(++addr && addr < 32);
    MHal_EMAC_disable_mdi(hemac->hal);
    hemac->phyaddr = addr;

    if (hemac->phyaddr >= 32)
    {
        EMAC_ERR("Wrong PHY Addr and reset to 0\n");
        // hemac->phyaddr = 0;
        hemac->phyaddr = 0xff;
        return -1;
    }
    return 0;
}

static void Rtl_Patch(struct net_device* netdev)
{
    u32 val;
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);

    MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, 25, &val);
    MHal_EMAC_write_phy(hemac->hal, hemac->phyaddr, 25, 0x400UL);
    MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, 25, &val);
}

static void MDev_EMAC_Patch_PHY(struct net_device* netdev)
{
    u32 val;
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);

    MHal_EMAC_read_phy(hemac->hal, hemac->phyaddr, 2, &val);
    if (RTL_8210 == val)
        Rtl_Patch(netdev);
}
#endif

#if KERNEL_PHY
static int MDev_EMAC_mii_write(struct mii_bus *bus, int phy_addr, int phy_reg, u16 val)
{
    struct emac_handle *hemac = (struct emac_handle *) bus->priv;
    int ret;

    ret = MHal_EMAC_write_phy(hemac->hal, phy_addr, phy_reg, (u32)val);
    return ret;
}

static int MDev_EMAC_mii_read(struct mii_bus *bus, int phy_addr, int phy_reg)
{
    u32 val;
    struct emac_handle *hemac = (struct emac_handle *) bus->priv;
    int ret;

    ret = MHal_EMAC_read_phy(hemac->hal, phy_addr, phy_reg, &val);
    return (int)val;
}

static int MDev_EMAC_mii_init(struct net_device* emac_dev)
{
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(emac_dev);
    struct device_node *mii_np = NULL;
    int ret = 0;

    // the force internal mdio bus in FPGA
    if (1)
    {
        u32 mdio_path = 0;
        if (!of_property_read_u32(emac_dev->dev.of_node, "mdio_path", &mdio_path))
        {
            MHal_EMAC_mdio_path(hemac->hal, mdio_path);
        }
    }

    if (!(mii_np = of_get_child_by_name(emac_dev->dev.of_node, "mdio-bus")))
    {
        // printk("[%s][%d] no child node of mdio-bus is found\n", __FUNCTION__, __LINE__);
        return -1;
    }
    if (!of_device_is_available(mii_np))
    {
        // printk("[%s][%d] mii_np is unavailable\n", __FUNCTION__, __LINE__);
        ret = -2;
        goto jmp_err_put_node;
    }
    if (!(hemac->mii_bus = devm_mdiobus_alloc(hemac->dev)))
    {
        // printk("[%s][%d] devm_mdiobus_alloc fail\n", __FUNCTION__, __LINE__);
        ret = -3;
        goto jmp_err_put_node;
    }

    hemac->mii_bus->name = "mdio";
    hemac->mii_bus->read = MDev_EMAC_mii_read;
    hemac->mii_bus->write = MDev_EMAC_mii_write;
    hemac->mii_bus->priv = hemac;
    hemac->mii_bus->parent = hemac->dev;

    snprintf(hemac->mii_bus->id, MII_BUS_ID_SIZE, "%s@%s", mii_np->name, hemac->name);
    ret = of_mdiobus_register(hemac->mii_bus, mii_np);
jmp_err_put_node:
    of_node_put(mii_np);
    return ret;
}

static void MDev_EMAC_mii_uninit(struct net_device* emac_dev)
{
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(emac_dev);
    if (!hemac->mii_bus)
        return;
    mdiobus_unregister(hemac->mii_bus);
    devm_mdiobus_free(hemac->dev, hemac->mii_bus);    
}

#endif

static int MDev_EMAC_init(struct platform_device *pdev)
{
    struct emac_handle *hemac;
    int ret;
    struct net_device* emac_dev = NULL;

    emac_dev = alloc_etherdev(sizeof(*hemac));
    hemac = netdev_priv(emac_dev);
    // printk("[%s][%d] alloc netdev = 0x%08x\n", __FUNCTION__, __LINE__, (int)emac_dev);

    if (!emac_dev)
    {
        EMAC_ERR( KERN_ERR "No EMAC dev mem!\n" );
        return -ENOMEM;
    }

    // led gpio
    hemac->led_orange = -1;
    hemac->led_green = -1;
    hemac->led_count = 0;
    hemac->led_flick_speed = 30;

    // phy address
    // hemac->phyaddr = 0xff;

////////////////////////////////////////////////////////////////////////////
    hemac->gu32CheckLinkTime = HZ;
    hemac->gu32CheckLinkTimeDis = 100;
    hemac->gu32intrEnable = 0;
    memset(hemac->irq_count, 0, sizeof(hemac->irq_count));

/*
    hemac->gu32PhyResetCount1 = 0;
    hemac->gu32PhyResetCount2 = 0;
    hemac->gu32PhyResetCount3 = 0;
    hemac->gu32PhyResetCount4 = 0;
    hemac->gu32PhyResetCount = 0;
*/
    hemac->skb_tx_send = 0;
    hemac->skb_tx_free = 0;
    hemac->data_done = 0;
    memset(&hemac->data_time_last, 0, sizeof(hemac->data_time_last));
    spin_lock_init(&hemac->emac_data_done_lock);

    hemac->txPkt = 0;
    hemac->txInt = 0;

    hemac->phy_status_register = 0x78c9UL;

    hemac->initstate= 0;
    hemac->contiROVR = 0;

    hemac->oldTime = 0;
    hemac->PreLinkStatus = 0;

    // hemac->phy_type = 0;
    hemac->irq_emac = 0;
    hemac->irq_lan = 0;

    hemac->name = NULL;
    hemac->bInit = 0;
    hemac->bEthCfg = 0;

    hemac->u8Minor = _u8Minor;
    _u8Minor++;
#if KERNEL_PHY
    hemac->dev = &pdev->dev;
#endif

////////////////////////////////////////////////////////////////////////////

    emac_dev->dev.of_node = pdev->dev.of_node; //pass of_node to MDev_EMAC_setup()

    SET_NETDEV_DEV(emac_dev, hemac->dev);
    // emac_dev->ethtool_ops = &sstar_emac_ethtool_ops;
    netdev_set_default_ethtool_ops(emac_dev, &sstar_emac_ethtool_ops);

    MDev_EMAC_dts(emac_dev);
    hemac->hal = MHal_EMAC_Alloc(hemac->emacRIU, hemac->emacX32, hemac->phyRIU);
    MHal_EMAC_Pad(hemac->hal, hemac->pad_reg, hemac->pad_msk, hemac->pad_val);
    MHal_EMAC_PadLed(hemac->hal, hemac->pad_led_reg, hemac->pad_led_msk, hemac->pad_led_val);
    MHal_EMAC_PhyMode(hemac->hal, hemac->phy_mode);

    if (0 == _phyReset)
    {
        #if defined(CONFIG_MS_PADMUX)
        if (mdrv_padmux_active())
        {
            int gpio_no;
            if (PAD_UNKNOWN != (gpio_no = mdrv_padmux_getpad(MDRV_PUSE_EMAC0_PHY_RESET)))
            {
                MDrv_GPIO_Set_High(gpio_no);
            }
            if (PAD_UNKNOWN != (gpio_no = mdrv_padmux_getpad(MDRV_PUSE_EMAC1_PHY_RESET)))
            {
                MDrv_GPIO_Set_High(gpio_no);
            }
        }
        #endif
        _phyReset = 1;
    }

#if TX_THROUGHPUT_TEST
    printk("==========TX_THROUGHPUT_TEST===============");
    pseudo_packet = alloc_skb(EMAC_PACKET_SIZE_MAX, GFP_ATOMIC);
    memcpy(pseudo_packet->data, (void *)packet_content, sizeof(packet_content));
    pseudo_packet->len = sizeof(packet_content);
#endif

#if RX_THROUGHPUT_TEST
    printk("==========RX_THROUGHPUT_TEST===============");
    init_timer(&RX_timer);

    RX_timer.data = EMAC_RX_TMR;
    RX_timer.function = RX_timer_callback;
    RX_timer.expires = jiffies + 20*EMAC_CHECK_LINK_TIME;
    add_timer(&RX_timer);
#endif

    MHal_EMAC_Power_On_Clk(hemac->hal, &pdev->dev);

    // init_timer(&EMAC_timer);
    // init_timer(&hemac->timer_link);

/*
    if (0 > MDev_EMAC_ScanPhyAddr(emac_dev))
        goto end;

    MDev_EMAC_Patch_PHY(emac_dev);
*/

#if MSTAR_EMAC_NAPI
    netif_napi_add(emac_dev, &hemac->napi, MDev_EMAC_napi_poll, EMAC_NAPI_WEIGHT);
#endif

    emac_dev->netdev_ops = &mstar_lan_netdev_ops;
    if (MDev_EMAC_probe (emac_dev))
        goto end;

#if KERNEL_PHY
    MDev_EMAC_mii_init(emac_dev);
#endif

    if ((ret = register_netdev (emac_dev)))
        goto end;

    // printk( KERN_ERR "[EMAC]Init EMAC success! (add delay in reset)\n" );
    platform_set_drvdata(pdev, (void*)emac_dev);
    hemac->bInit = 1;
    return 0;

end:
    skb_queue_destroy(&(hemac->skb_queue_tx));
    free_netdev(emac_dev);
    emac_dev = NULL;
    hemac->initstate = ETHERNET_TEST_INIT_FAIL;
    EMAC_ERR( KERN_ERR "Init EMAC error!\n" );
    return -1;
}

static void MDev_EMAC_dts(struct net_device* netdev)
{
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(netdev);
    // const char* tmp_str = NULL;
    unsigned int led_data;
    // struct resource *res;
    struct resource res;
    u32 val[3];

    hemac->irq_emac = irq_of_parse_and_map(netdev->dev.of_node, 0);
    hemac->irq_lan = irq_of_parse_and_map(netdev->dev.of_node, 1);

    // printk("[%s][%d] (irq_emac, irq_lan) = (%d, %d)\n", __FUNCTION__, __LINE__, hemac->irq_emac, hemac->irq_lan);

#if 0
    if (!of_property_read_string(netdev->dev.of_node, "emac_phy", &tmp_str) && tmp_str)
    {
        hemac->phy_type = 0;
        // printk("[%s][%d] emac dts phy name = [%s]\n", __FUNCTION__, __LINE__, tmp_str);
        if (!strcmp("ALBANY", tmp_str))
            hemac->phy_type = 0; // for internal phy, ie. ALBANY
        else if (!strcmp("EMAC_PHY_FPGA", tmp_str))
            hemac->phy_type = 1; // for FPGA
    }
    else
    {
        // printk("[%s][%d] emac default phy name = [%s]\n", __FUNCTION__, __LINE__, "ALBANY");
        hemac->phy_type = 0;
    }
#endif


    if (of_property_read_u32(netdev->dev.of_node, "txd_num", &hemac->txd_num))
    {
        hemac->txd_num = TXD_NUM; // default value
    }
    hemac->txd_num = (hemac->txd_num + 0xFF) & 0x100; // 256 alignment
    if (of_property_read_u32(netdev->dev.of_node, "txq_num_sw", &hemac->txq_num_sw))
    {
        hemac->txq_num_sw = TXQ_NUM_SW; // default value
    }

    if(!of_property_read_u32(netdev->dev.of_node, "led-orange", &led_data))
    {
        hemac->led_orange = (unsigned char)led_data;
        // printk(KERN_ERR "[EMAC]Set emac_led_orange=%d\n",led_data);
    }

    if(!of_property_read_u32(netdev->dev.of_node, "led-green", &led_data))
    {
        hemac->led_green = (unsigned char)led_data;
        // printk(KERN_ERR "[EMAC]Set emac_led_green=%d\n",led_data);
    }

    if (!of_address_to_resource(netdev->dev.of_node, 0, &res))
    {
        hemac->emacRIU = IO_ADDRESS(res.start);
        // printk("[%s][%d] (emacRIU, start) = (0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, hemac->emacRIU, res.start);
    }
    else
    {
        hemac->emacRIU = 0x1F2A2000;
        hemac->emacRIU = IO_ADDRESS(hemac->emacRIU);
        // printk("[%s][%d] (emacRIU) = (0x%08x)\n", __FUNCTION__, __LINE__, hemac->emacRIU);
    }
    if (!of_address_to_resource(netdev->dev.of_node, 1, &res))
    {
        hemac->emacX32 = IO_ADDRESS(res.start);
        // printk("[%s][%d] (emacX32, start) = (0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, hemac->emacX32, res.start);
    }
    else
    {
        hemac->emacX32 = 0x1F343C00;
        hemac->emacX32 = IO_ADDRESS(hemac->emacX32);
        // printk("[%s][%d] (emacX32) = (0x%08x)\n", __FUNCTION__, __LINE__, hemac->emacX32);
    }
    if (!of_address_to_resource(netdev->dev.of_node, 2, &res))
    {
        hemac->phyRIU = (res.start) ?  IO_ADDRESS(res.start) : 0;
        // printk("[%s][%d] (phyRIU, start) = (0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, hemac->phyRIU, res.start);
    }
    else
    {
        hemac->phyRIU = 0x1F006200;
        hemac->phyRIU = IO_ADDRESS(hemac->phyRIU);
        // printk("[%s][%d] (phyRIU) = (0x%08x)\n", __FUNCTION__, __LINE__, hemac->phyRIU);
    }

    val[0] = val[1] = val[2] = 0;
    if (of_property_read_u32_array(netdev->dev.of_node, "pad", val, 3))
    {
        // printk("[%s][%d] parse pad mux fail\n", __FUNCTION__, __LINE__);
        val[0] = 0x1F203C3C;
        val[1] = 0x0004;
        val[2] = 0x0004;
    }
    hemac->pad_reg = IO_ADDRESS(val[0]);
    hemac->pad_msk = val[1];
    hemac->pad_val = val[2];

    val[0] = val[1] = val[2] = 0;
    if (of_property_read_u32_array(netdev->dev.of_node, "pad_led", val, 3))
    {
        val[0] = 0x1F001CA0;
        val[1] = 0x0030;
        val[2] = 0x0010;
    }
    hemac->pad_led_reg = (val[0]) ?  IO_ADDRESS(val[0]) : 0;
    hemac->pad_led_msk = val[1];
    hemac->pad_led_val = val[2];

    // printk("[%s][%d] pad (reg, msk, val) = (0x%08x 0x%08x 0x%08x)\n", __FUNCTION__, __LINE__, hemac->pad_reg, hemac->pad_msk, hemac->pad_val);

    {
        // struct device_node* np = NULL;
        int phy_mode;

        hemac->phy_mode = PHY_INTERFACE_MODE_RMII;
#if 0
        np = of_parse_phandle(netdev->dev.of_node, "phy-handle", 0);
        if (!np && of_phy_is_fixed_link(netdev->dev.of_node))
            if (!of_phy_register_fixed_link(netdev->dev.of_node))
                np = of_node_get(netdev->dev.of_node);
        if (!np)
        {
            // printk("[%s][%d] can not find phy-handle in dts\n", __FUNCTION__, __LINE__);
        }
        else
        {
            if (0 > (phy_mode = of_get_phy_mode(np)))
            {
                // printk("[%s][%d] incorrect phy-mode %d\n", __FUNCTION__, __LINE__, phy_mode);
            }
            else
            {
                hemac->phy_mode = phy_mode;
            }
        }
#else
        if (0 > (phy_mode = of_get_phy_mode(netdev->dev.of_node)))
        {
            struct device_node* np = NULL;
            np = of_parse_phandle(netdev->dev.of_node, "phy-handle", 0);
            if ((np) && (0 <= (phy_mode = of_get_phy_mode(np))))
            {
                hemac->phy_mode = phy_mode;
            }
	    if (np)
	        of_node_put(np);
        }
        else
        {
            hemac->phy_mode = phy_mode;
        }
#endif
    }
    hemac->name = netdev->dev.of_node->name;
}

#if REDUCE_CPU_FOR_RBNA
static void _MDev_EMAC_IntRX_CB(unsigned long data)
{
    struct emac_handle *hemac = (struct emac_handle *) data;
    unsigned long flags;

    spin_lock_irqsave(&hemac->mutexIntRX, flags);
    // MHal_EMAC_RX_Param(hemac->hal, 0x01, 0x01);
    MHal_EMAC_RX_ParamRestore(hemac->hal);
    spin_unlock_irqrestore(&hemac->mutexIntRX, flags);
}
#endif // #if REDUCE_CPU_FOR_RBNA

#if EMAC_FLOW_CONTROL_TX
static void _MDev_EMAC_FlowTX_CB(unsigned long data)
{
    struct emac_handle *hemac = (struct emac_handle *) data;
    unsigned long flags;

    spin_lock_irqsave(&hemac->mutexFlowTX, flags);
#if 0
    if (1 == hemac->isPauseTX)
    {
        hemac->isPauseTX = 0;
        netif_wake_queue(hemac->netdev);
        // printk("[%s][%d] release\n", __FUNCTION__, __LINE__);
    }
#else
    netif_wake_queue(hemac->netdev);
#endif
    spin_unlock_irqrestore(&hemac->mutexFlowTX, flags);
}

static int _MDrv_EMAC_is_PausePkt(struct sk_buff* skb, unsigned char* p_recv)
{
    unsigned int mac_ctl_opcode = 0;

    if ((MAC_CONTROL_TYPE&0xFF) != ((skb->protocol>>8)&0xFF))
        return 0;
    if (((MAC_CONTROL_TYPE>>8)&0xFF) != (skb->protocol&0xFF))
        return 0;
    mac_ctl_opcode = (((*(p_recv+14))<<8)&0xFF00) + ((*(p_recv+15))&0xFF);
    return (MAC_CONTROL_OPCODE == mac_ctl_opcode) ? 1 : 0;
}

static int _MDrv_EMAC_Pause_TX(struct net_device* dev, struct sk_buff* skb, unsigned char* p_recv)
{
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(dev);
    int pause_time = 0;
    int pause_time_to_jiffies = 0;
    unsigned long flags;
    unsigned long expires;

#if EMAC_FLOW_CONTROL_TX_TEST
{
    static unsigned int cnt = 0;
    cnt++;
    if ((cnt & 0xF) == 0)
    {
        // printk("[%s][%d] pseudo pause packet\n", __FUNCTION__, __LINE__);
        pause_time = EMAC_FLOW_CONTROL_TX_TEST_TIME;
        goto jump_pause_tx_test;
    }
}
#endif // #if EMAC_FLOW_CONTROL_TX_TEST

    if (0 == _MDrv_EMAC_is_PausePkt(skb, p_recv))
        return 0;

    pause_time = (((*(p_recv+16))<<8)&0xFF00) + ((*(p_recv+17))&0xFF);

#if EMAC_FLOW_CONTROL_TX_TEST
jump_pause_tx_test:
#endif // #if EMAC_FLOW_CONTROL_TX_TEST

    if (SPEED_100 == dev->phydev->speed)
    {
        pause_time_to_jiffies = (pause_time/PAUSE_TIME_DIVISOR_100M)+((0==(pause_time%PAUSE_TIME_DIVISOR_100M))?0:1);
        expires = jiffies + pause_time_to_jiffies;
    }
    else if (SPEED_10 == dev->phydev->speed)
    {
        pause_time_to_jiffies = (pause_time/PAUSE_TIME_DIVISOR_10M)+((0==(pause_time%PAUSE_TIME_DIVISOR_10M))?0:1);
        expires = jiffies + pause_time_to_jiffies;
    }
    else
    {
        printk("[%s][%d] Get emac speed error : %d\n", __FUNCTION__, __LINE__, (int)dev->phydev->speed);
        return 0;
    }
    spin_lock_irqsave(&hemac->mutexFlowTX, flags);

    // if (0 == hemac->isPauseTX)
    if (0 == timer_pending(&hemac->timerFlowTX))
    {
        netif_stop_queue (dev);
        hemac->timerFlowTX.expires = expires;
        // hemac->isPauseTX = 1;
        add_timer(&hemac->timerFlowTX);
    }
    else
    {
        mod_timer(&hemac->timerFlowTX, expires);
    }
    spin_unlock_irqrestore(&hemac->mutexFlowTX, flags);
    return 1;
}
#endif // #if EMAC_FLOW_CONTROL_TX

//-------------------------------------------------------------------------------------------------
// EMAC exit module
//-------------------------------------------------------------------------------------------------
static void MDev_EMAC_exit(struct platform_device *pdev)
{
    struct net_device* emac_dev =(struct net_device*) platform_get_drvdata(pdev);

    if (emac_dev)
    {
        struct emac_handle *hemac = (struct emac_handle *) netdev_priv(emac_dev);

#if EMAC_FLOW_CONTROL_TX
        // hemac->isPauseTX = 0;
        del_timer(&hemac->timerFlowTX);
#endif // #if EMAC_FLOW_CONTROL_TX

#if REDUCE_CPU_FOR_RBNA
        del_timer(&hemac->timerIntRX);
#endif // #if REDUCE_CPU_FOR_RBNA

#if KERNEL_PHY
        MDev_EMAC_mii_uninit(emac_dev);
#endif

        skb_queue_destroy(&(hemac->skb_queue_tx));

	// printk("[%s][%d] free RX memory\n", __FUNCTION__, __LINE__);
        // mem_info.length = RBQP_SIZE;
        // strcpy(mem_info.name, "EMAC_BUFF");
        // msys_release_dmem(&mem_info);
        free_rx_skb(hemac);
        MDev_EMAC_RX_Desc_Free(hemac);
        MDev_EMAC_MemFree(hemac);
#if (EMAC_SG && EMAC_SG_BUF_CACHE)
/*
        if (hemac->pTxBuf)
        {
            kfree(hemac->pTxBuf);
            hemac->pTxBuf = NULL;
        }
*/
#endif
	MHal_EMAC_Free(hemac->hal);
        unregister_netdev(emac_dev);
        free_netdev(emac_dev);
    }
}

static int mstar_emac_drv_suspend(struct platform_device *dev, pm_message_t state)
{
    // struct net_device *netdev=(struct net_device*)dev->dev.platform_data;
    struct net_device *netdev=(struct net_device*) platform_get_drvdata(dev);
    struct emac_handle *hemac;
    u32 uRegVal;

    // printk(KERN_INFO "mstar_emac_drv_suspend\n");
    if(!netdev)
    {
        return -1;
    }

    hemac = (struct emac_handle*) netdev_priv(netdev);
    hemac->ep_flag |= EP_FLAG_SUSPENDING;
    //netif_stop_queue (netdev);


    disable_irq(netdev->irq);
    // del_timer(&hemac->timer_link);

    //MHal_EMAC_Power_On_Clk(dev->dev);

    //corresponds with resume call MDev_EMAC_open
#if MSTAR_EMAC_NAPI
    napi_disable(&hemac->napi);
#endif

    //Disable Receiver and Transmitter //
    uRegVal = MHal_EMAC_Read_CTL(hemac->hal);
    uRegVal &= ~(EMAC_TE | EMAC_RE);
    MHal_EMAC_Write_CTL(hemac->hal, uRegVal);

#ifdef TX_SW_QUEUE
    //make sure that TX HW FIFO is empty
    while(TX_FIFO_SIZE!= MHal_EMAC_TXQ_Free(hemac->hal));
#endif

    // Disable PHY interrupt //
    MHal_EMAC_disable_phyirq(hemac->hal);

    // MHal_EMAC_Write_IDR(0xFFFFFFFF);
    MHal_EMAC_IntEnable(hemac->hal, 0xFFFFFFFF, 0);

    MDev_EMAC_SwReset(netdev);
    MHal_EMAC_Power_Off_Clk(hemac->hal, &dev->dev);
#ifdef TX_SW_QUEUE
    _MDev_EMAC_tx_reset_TX_SW_QUEUE(netdev);
#endif
    skb_queue_reset(&(hemac->skb_queue_tx));

    disable_irq(netdev->irq);

    return 0;
}

static int mstar_emac_drv_resume(struct platform_device *dev)
{
    // struct net_device *netdev=(struct net_device*)dev->dev.platform_data;
    struct net_device *netdev=(struct net_device*) platform_get_drvdata(dev);
    struct emac_handle *hemac;
    unsigned long flags;

    // printk(KERN_INFO "mstar_emac_drv_resume\n");
    if(!netdev)
    {
        return -1;
    }
    hemac = (struct emac_handle*) netdev_priv(netdev);;
    hemac->ep_flag &= ~EP_FLAG_SUSPENDING;

    MHal_EMAC_Power_On_Clk(hemac->hal, &dev->dev);

    // MHal_EMAC_Write_JULIAN_0100(hemac->hal, JULIAN_100_VAL);
    MHal_EMAC_Write_JULIAN_0100(hemac->hal, 0);

    spin_lock_irqsave(&hemac->mutexPhy, flags);
/*
    if (0 > MDev_EMAC_ScanPhyAddr(netdev))
        return -1;

    MDev_EMAC_Patch_PHY(netdev);
*/
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);

    // hemac->ThisUVE.initedEMAC = 0;
    MDev_EMAC_HW_init(netdev);

    spin_lock_irqsave(&hemac->mutexPhy, flags);
    MDev_EMAC_update_mac_address (netdev); // Program ethernet address into MAC //
    MHal_EMAC_enable_mdi(hemac->hal);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);
    enable_irq(netdev->irq);
    if(hemac->ep_flag & EP_FLAG_OPEND)
    {
        if(0>MDev_EMAC_open(netdev))
        {
            // printk(KERN_WARNING "Driver Emac: open failed after resume\n");
        }
    }

    return 0;
}

static int mstar_emac_drv_probe(struct platform_device *pdev)
{
    int retval=0;
    struct net_device* netdev;
    struct emac_handle *hemac;

    if (!(pdev->name) || strcmp(pdev->name,"Sstar-emac")
        || pdev->id!=0)
    {
        retval = -ENXIO;
    }

    if ((retval = MDev_EMAC_init(pdev)))
        return retval;

    netdev=(struct net_device*) platform_get_drvdata(pdev);
    hemac = (struct emac_handle*) netdev_priv(netdev);;
/*
    if(!of_property_read_u32(pdev->dev.of_node, "led-orange", &led_data))
    {
        hemac->led_orange = (unsigned char)led_data;
        printk(KERN_ERR "[EMAC]Set emac_led_orange=%d\n",led_data);
    }

    if(!of_property_read_u32(pdev->dev.of_node, "led-green", &led_data))
    {
         hemac->led_green = (unsigned char)led_data;
         printk(KERN_ERR "[EMAC]Set emac_led_green=%d\n",led_data);
    }
*/
    if(hemac->led_orange!=-1)
    {
        MDrv_GPIO_Pad_Set(hemac->led_orange);
    }
    if(hemac->led_green!=-1)
    {
        MDrv_GPIO_Pad_Set(hemac->led_green);
    }
    return retval;
}

static int mstar_emac_drv_remove(struct platform_device *pdev)
{
    struct net_device* emac_dev =(struct net_device*) platform_get_drvdata(pdev);
    struct emac_handle *hemac = (struct emac_handle *) netdev_priv(emac_dev);

    if( !(pdev->name) || strcmp(pdev->name,"Sstar-emac")
        || pdev->id!=0)
    {
        return -1;
    }
    MDev_EMAC_exit(pdev);
    MHal_EMAC_Power_Off_Clk(hemac->hal, &pdev->dev);
#if 0
    netif_napi_del(&eth->tx_napi);
    netif_napi_del(&eth->rx_napi);

        netif_napi_add(emac_dev, &hemac->napi, MDev_EMAC_napi_poll, EMAC_NAPI_WEIGHT);


    struct net_device *netdev=(struct net_device*)dev->dev.platform_data;
    struct emac_handle *hemac;

    printk(KERN_INFO "mstar_emac_drv_resume\n");
    if(!netdev)
    {
        return -1;
    }
    hemac = (struct emac_handle*) netdev_priv(netdev);;
    hemac->ep_flag &= ~EP_FLAG_SUSPENDING;
#endif
    platform_set_drvdata(pdev, NULL);
    return 0;
}



static struct platform_driver Mstar_emac_driver = {
    .probe      = mstar_emac_drv_probe,
    .remove     = mstar_emac_drv_remove,
    .suspend    = mstar_emac_drv_suspend,
    .resume     = mstar_emac_drv_resume,

    .driver = {
        .name    = "Sstar-emac",
#if defined(CONFIG_OF)
        .of_match_table = mstaremac_of_device_ids,
#endif
        .owner  = THIS_MODULE,
    }
};

static int __init mstar_emac_drv_init_module(void)
{
    int retval=0;

    retval = platform_driver_register(&Mstar_emac_driver);
    if (retval)
    {
        // printk(KERN_INFO"Mstar_emac_driver register failed...\n");
        return retval;
    }

    return retval;
}

static void __exit mstar_emac_drv_exit_module(void)
{
    platform_driver_unregister(&Mstar_emac_driver);
    // emac_dev=NULL;
}

module_init(mstar_emac_drv_init_module);
module_exit(mstar_emac_drv_exit_module);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("EMAC Ethernet driver");
MODULE_LICENSE("GPL");
