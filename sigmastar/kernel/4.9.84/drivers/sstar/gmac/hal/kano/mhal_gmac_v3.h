/*
* mhal_gmac_v3.h- Sigmastar
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
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2006 - 2007 MStar Semiconductor, Inc.
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
/// @file   GMAC.h
/// @brief  GMAC Driver Interface
///////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// Linux GMAC.h define start
// -----------------------------------------------------------------------------
#ifndef __DRV_GMAC__
#define __DRV_GMAC__

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <mstar/mstar_chip.h>
//-------------------------------------------------------------------------------------------------
//  Define Enable or Compiler Switches
//-------------------------------------------------------------------------------------------------
#define GMAC_LINK_LED_CONTROL
#define GMAC_CHIP_FLUSH_READ
#define GMAC_ROVR_SW_RESET
//#define GMAC_MEMORY_PROTECT
#define RX_DESC_MODE
#ifdef CONFIG_GMAC_RX_DMA
#define RX_ZERO_COPY
#endif
#define GMAC_RX_CHECKSUM
#ifdef CONFIG_GMAC_RX_NAPI
#define RX_NAPI
#endif
#ifdef CONFIG_GMAC_RX_GRO
#define RX_GRO
#endif
#define GMAC_TX_QUEUE_4
#ifdef CONFIG_GMAC_RX_DELAY_INTERRUPT
#define RX_DELAY_INTERRUPT
#endif
//#define TX_COM_ENABLE
//#define TX_SOFTWARE_QUEUE
//#define TX_NAPI
// Compiler Switches
#define GMAC_URANUS_ETHER_ADDR_CONFIGURABLE /* MAC address can be changed? */
//--------------------------------------------------------------------------------------------------
//  PadMux
//--------------------------------------------------------------------------------------------------
#define GMAC1_EPHY 1
#define GMAC1_GPHY 2
#define GMAC0_GPHY 3
#define GMAC0_GPHY_PHY_ADDR          CONFIG_KANO_GMAC0_GPHY_PHY_ADDR
#define GMAC1_EPHY_PHY_ADDR          CONFIG_KANO_GMAC1_EPHY_PHY_ADDR
#define GMAC1_GPHY_PHY_ADDR          CONFIG_KANO_GMAC1_GPHY_PHY_ADDR
#define GMAC_EPHY  0
#define GMAC_GPHY  1
#ifdef CONFIG_KANO_GMAC1_EPHY
#define DEFAULT_PADMUX GMAC1_EPHY
#endif
#ifdef CONFIG_KANO_GMAC1_GPHY
#define DEFAULT_PADMUX GMAC1_GPHY
#endif
#ifdef CONFIG_KANO_GMAC0_GPHY
#define DEFAULT_PADMUX GMAC0_GPHY
#endif
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define TRUE                            1
#define FALSE                           0

#define GMAC_SOFTWARE_DESCRIPTOR_ENABLE 0x0001
#define GMAC_RX_CHECKSUM_ENABLE         0x000E
#define GMAC_SPEED_100                  100
#define GMAC_MAX_INT_COUNTER            100
#define GMAC_TX_CHECKSUM_ENABLE         (0x00000470)
#define GMAC_TX_V6_CHECKSUM_ENABLE      (0x000C0000)
#define GMAC_TX_JUMBO_FRAME_ENABLE      (0x0000ffff)
#define GMAC_MAX_TX_QUEUE               30
// Interrupt:
#define MAC_GMAC0_IRQ                   E_IRQ_GMAC  //aliasing for interrupt enum (GMAC0)
#define MAC_GMAC1_IRQ                   E_IRQHYPH_SECGMAC //aliasing for interrupt enum (GMAC1 & Internal)
#define GMAC_INT_MASK                   (0xdff)
#define GMAC_INT_ALL                    (0xffff)
#ifdef TX_COM_ENABLE
#define GMAC_INT_ENABLE                 GMAC_INT_DONE|GMAC_INT_RBNA|GMAC_INT_TUND|GMAC_INT_RTRY|GMAC_INT_ROVR|GMAC_INT_TCOM|(0x0000E000UL)
#else
#define GMAC_INT_ENABLE                 GMAC_INT_DONE|GMAC_INT_RBNA|GMAC_INT_TUND|GMAC_INT_RTRY|GMAC_INT_ROVR|(0x0000E000UL)
#endif
// Base address here:
#if defined(CONFIG_ARM)
#define  mstar_pm_base                  0xFD000000
#elif defined(CONFIG_MIPS)
#define  mstar_pm_base                  0xBF000000
#endif
#define RIU_REG_BASE                    mstar_pm_base
#define REG_ALBANY0_BANK                0x0031UL
#define REG_GMAC0_BANK                  0x121BUL
#define REG_GMAC1_BANK                  0x1220UL
#define INTERNEL_PHY_REG_BASE           RIU_REG_BASE + ((REG_ALBANY0_BANK*0x100UL)<<1)
#define GMAC0_REG_ADDR_BASE             RIU_REG_BASE + ((REG_GMAC0_BANK*0x100UL)<<1)
#define GMAC1_REG_ADDR_BASE             RIU_REG_BASE + ((REG_GMAC1_BANK*0x100UL)<<1)
#define GMAC_MIU0_BUS_BASE              MSTAR_MIU0_BUS_BASE
// Config Registers Value
#define WRITE_PROTECT_ENABLE            GMAC_MIU_WRITE_PROTECT|GMAC_MIU_WP_INT_EN
#define SOFTWARE_RESET                  GMAC_SW_RESET_MIU|GMAC_SW_RS_EMAC_TO_MIU|GMAC_SW_RESET_APB|GMAC_SW_RESET_AHB
#define EXTERNAL_PHY                    GMAC_RMII|GMAC_RMII_12
#ifdef GMAC_MEMORY_PROTECT
#define CONFIG2_VAL                     GMAC_POWER_UP|SOFTWARE_RESET|WRITE_PROTECT_ENABLE
#else
#define CONFIG2_VAL                     GMAC_POWER_UP|SOFTWARE_RESET
#endif
// Delay Interrupt mode:
//#define DELAY_NUMBER                    0x40
//#define DELAY_TIME                      0x04
#define DELAY_NUMBER                    CONFIG_GMAC_DELAY_INTERRUPT_NUMBER
#define DELAY_TIME                      CONFIG_GMAC_DELAY_INTERRUPT_TIMEOUT
#define DELAY_INTERRUPT_CONFIG          GMAC_INT_DELAY_MODE_EN|(DELAY_NUMBER<<GMAC_INT_DELAY_NUMBER_SHIFT)|(DELAY_TIME<<GMAC_INT_DELAY_TIME_SHIFT)
// RX Descriptor here:
#define GMAC_CLEAR_BUFF                 0x0
#define RX_DESC_SIZE                    0x0010UL                                //4words = 16bytes = 128bits
#ifdef RX_ZERO_COPY
#define RX_DESC_NUMBER                  0x0400UL                                //32 descriptor
#else
#define RX_DESC_NUMBER                  0x0180UL
#endif
#define RX_DESC_TABLE_SIZE              (RX_DESC_NUMBER*RX_DESC_SIZE)
#define RX_DESC_REFILL_NUMBER           RX_DESC_NUMBER/2
// RX Buffer here:
#define RX_BUFF_ENTRY_SIZE              0x800UL                                 //2048bytes
#define RX_BUFF_SIZE                    (RX_DESC_NUMBER*RX_BUFF_ENTRY_SIZE)
// TX Buffer here:
#define TX_BUFF_ENTRY_SIZE              0x800UL
#define TX_BUFF_ENTRY_NUMBER            0x08UL
//PHY Register
#define GMAC_PHY_REG_BASIC              (0)
#define GMAC_PHY_REG_STATUS             (1)
//PHY setting
#define GMAC_PHY_REGADDR_OFFSET         (18)
#define GMAC_PHY_ADDR_OFFSET            (23)

#ifdef RX_NAPI
//#define GMAC_RX_NAPI_WEIGHT             0x80
#define GMAC_RX_NAPI_WEIGHT             CONFIG_GMAC_RX_NAPI_WEIGHT
#endif

#ifdef TX_NAPI
#define GMAC_TX_NAPI_WEIGHT             0x40
#endif

#ifdef TX_SOFTWARE_QUEUE
#define TX_SW_QUEUE_SIZE                (1024)  //effected size = TX_RING_SIZE - 1
#define TX_DESC_CLEARED                 0
#define TX_DESC_WROTE                   1
#define TX_DESC_READ                    2
#define TX_FIFO_SIZE                    4 //HW FIFO size

#define TX_SW_QUEUE_IN_GENERAL_TX       0
#define TX_SW_QUEUE_IN_IRQ              1
#define TX_SW_QUEUE_IN_TIMER            2
#endif

//-------------------------------------------------------------------------------------------------
//  Bit Define
//-------------------------------------------------------------------------------------------------
#define GMAC_MIU_RW       ( 0x3 << 10)   //GMAC power on clk

// -------- GMAC_CTL : (GMAC Offset: 0x0)  --------
#define GMAC_LB           ( 0x1 <<  0) // (GMAC) Loopback. Optional. When set, loopback signal is at high level.
#define GMAC_LBL          ( 0x1 <<  1) // (GMAC) Loopback local.
#define GMAC_RE           ( 0x1 <<  2) // (GMAC) Receive enable.
#define GMAC_TE           ( 0x1 <<  3) // (GMAC) Transmit enable.
#define GMAC_MPE          ( 0x1 <<  4) // (GMAC) Management port enable.
#define GMAC_CSR          ( 0x1 <<  5) // (GMAC) Clear statistics registers.
#define GMAC_ISR          ( 0x1 <<  6) // (GMAC) Increment statistics registers.
#define GMAC_WES          ( 0x1 <<  7) // (GMAC) Write enable for statistics registers.
#define GMAC_BP           ( 0x1 <<  8) // (GMAC) Back pressure.
// -------- GMAC_CFG : (GMAC Offset: 0x4) Network Configuration Register --------
#define GMAC_SPD          ( 0x1 <<  0) // (GMAC) Speed.
#define GMAC_FD           ( 0x1 <<  1) // (GMAC) Full duplex.
#define GMAC_BR           ( 0x1 <<  2) // (GMAC) Bit rate.
#define GMAC_CAF          ( 0x1 <<  4) // (GMAC) Copy all frames.
#define GMAC_NBC          ( 0x1 <<  5) // (GMAC) No broadcast.
#define GMAC_MTI          ( 0x1 <<  6) // (GMAC) Multicast hash enable
#define GMAC_UNI          ( 0x1 <<  7) // (GMAC) Unicast hash enable.
#define GMAC_RLF          ( 0x1 <<  8) // (GMAC) Receive Long Frame.
#define GMAC_EAE          ( 0x1 <<  9) // (GMAC) External address match enable.
#define GMAC_CLK          ( 0x3 << 10) // (GMAC)
#define GMAC_CLK_HCLK_8   ( 0x0 << 10) // (GMAC) HCLK divided by 8
#define GMAC_CLK_HCLK_16  ( 0x1 << 10) // (GMAC) HCLK divided by 16
#define GMAC_CLK_HCLK_32  ( 0x2 << 10) // (GMAC) HCLK divided by 32
#define GMAC_CLK_HCLK_64  ( 0x3 << 10) // (GMAC) HCLK divided by 64
#define GMAC_RT           ( 0x1 << 12) // (GMAC) Retry test
#define GMAC_CAMMEG       ( 0x1 << 13) // (GMAC)
// -------- GMAC_SR : (GMAC Offset: 0x8) Network Status Register --------
#define GMAC_MDIO         ( 0x1 <<  1) // (GMAC)
#define GMAC_IDLE         ( 0x1 <<  2) // (GMAC)
// -------- GMAC_TCR : (GMAC Offset: 0x10) Transmit Control Register --------
#define GMAC_LEN          ( 0x7FF <<  0) // (GMAC)
#define GMAC_NCRC         ( 0x1 << 15) // (GMAC)
// -------- GMAC_TSR : (GMAC Offset: 0x14) Transmit Control Register --------
#define GMAC_OVR          ( 0x1 <<  0) // (GMAC)
#define GMAC_COL          ( 0x1 <<  1) // (GMAC)
#define GMAC_RLE          ( 0x1 <<  2) // (GMAC)
//#define GMAC_TXIDLE     ( 0x1 <<  3) // (GMAC)
#define GMAC_IDLETSR      ( 0x1 <<  3) // (GMAC)
#define GMAC_BNQ          ( 0x1 <<  4) // (GMAC)
#define GMAC_COMP         ( 0x1 <<  5) // (GMAC)
#define GMAC_UND          ( 0x1 <<  6) // (GMAC)
#define GMAC_TBNQ         ( 0x1 <<  7) // (GMAC)
#define GMAC_FBNQ         ( 0x1 <<  8) // (GMAC)
#define GMAC_FIFO1IDLE    ( 0x1 <<  9) // (GMAC)
#define GMAC_FIFO2IDLE    ( 0x1 <<  10) // (GMAC)
#define GMAC_FIFO3IDLE    ( 0x1 <<  11) // (GMAC)
#define GMAC_FIFO4IDLE    ( 0x1 <<  12) // (GMAC)
// -------- GMAC_RSR : (GMAC Offset: 0x20) Receive Status Register --------
#define GMAC_DNA          ( 0x1 <<  0) // (GMAC)
#define GMAC_REC          ( 0x1 <<  1) // (GMAC)
#define GMAC_RSROVR       ( 0x1 <<  2) // (GMAC)
#define GMAC_BNA          ( 0x1 <<  3) // (GMAC)
// -------- GMAC_ISR : (GMAC Offset: 0x24) Interrupt Status Register --------
#define GMAC_INT_DONE     ( 0x1 <<  0) // (GMAC)
#define GMAC_INT_RCOM     ( 0x1 <<  1) // (GMAC)
#define GMAC_INT_RBNA     ( 0x1 <<  2) // (GMAC)
#define GMAC_INT_TOVR     ( 0x1 <<  3) // (GMAC)
#define GMAC_INT_TUND     ( 0x1 <<  4) // (GMAC)
#define GMAC_INT_RTRY     ( 0x1 <<  5) // (GMAC)
#define GMAC_INT_TBRE     ( 0x1 <<  6) // (GMAC)
#define GMAC_INT_TCOM     ( 0x1 <<  7) // (GMAC)
#define GMAC_INT_TIDLE    ( 0x1 <<  8) // (GMAC)
#define GMAC_INT_LINK     ( 0x1 <<  9) // (GMAC)
#define GMAC_INT_ROVR     ( 0x1 << 10) // (GMAC)
#define GMAC_INT_HRESP    ( 0x1 << 11) // (GMAC)
// -------- GMAC_IER : (GMAC Offset: 0x28) Interrupt Enable Register --------
// -------- GMAC_IDR : (GMAC Offset: 0x2c) Interrupt Disable Register --------
// -------- GMAC_IMR : (GMAC Offset: 0x30) Interrupt Mask Register --------
// -------- GMAC_MAN : (GMAC Offset: 0x34) PHY Maintenance Register --------
#define GMAC_DATA         ( 0xFFFF <<  0) // (GMAC)
#define GMAC_CODE         ( 0x3 << 16) // (GMAC)
#define GMAC_CODE_802_3   ( 0x2 << 16) // (GMAC) Write Operation
#define GMAC_REGA         ( 0x1F << 18) // (GMAC)
#define GMAC_PHYA         ( 0x1F << 23) // (GMAC)
#define GMAC_RW           ( 0x3 << 28) // (GMAC)
#define GMAC_RW_R         ( 0x2 << 28) // (GMAC) Read Operation
#define GMAC_RW_W         ( 0x1 << 28) // (GMAC) Write Operation
#define GMAC_HIGH         ( 0x1 << 30) // (GMAC)
#define GMAC_LOW          ( 0x1 << 31) // (GMAC)
// -------- GMAC_RBRP: (GMAC Offset: 0x38) Receive Buffer First full pointer--------
#define GMAC_WRAP_R       ( 0x1 << 14) // Wrap bit
// -------- GMAC_RBWP: (GMAC Offset: 0x3C) Receive Buffer Current pointer--------
#define GMAC_WRAP_W       ( 0x1 << 14) // Wrap bit
// -------- EMAC_CFG2: (EMAC Offset: 0x100) Network Configuration Register2 --------
#define GMAC_POWER_UP           ( 0x1UL <<  0)      // (EMAC)
#define GMAC_RMII               ( 0x1UL <<  1)      // (EMAC)
#define GMAC_RMII_12            ( 0x1UL <<  2)      // (EMAC)
#define GMAC_MIU_WRITE_PROTECT  ( 0x1UL <<  6)      // (EMAC)
#define GMAC_MIU_WP_INT_EN      ( 0x1UL <<  7)      // (EMAC)
#define GMAC_SW_RESET_MIU       ( 0x1UL << 12)      // (EMAC)
#define GMAC_SW_RS_EMAC_TO_MIU  ( 0x1UL << 13)      // (EMAC)
#define GMAC_SW_RESET_APB       ( 0x1UL << 14)      // (EMAC)
#define GMAC_SW_RESET_AHB       ( 0x1UL << 15)      // (EMAC)
// -------- EMAC_CFG3: (EMAC Offset: 0x102) Network Configuration Register3 --------
#define GMAC_MASK_ALL_INT       ( 0x1UL <<  0)      // (EMAC)
#define GMAC_FORCE_INT          ( 0x1UL <<  1)      // (EMAC)
#define GMAC_MIU0_2048MB        ( 0x00UL << 6)      // (EMAC)
#define GMAC_MIU0_1024MB        ( 0x01UL << 6)      // (EMAC)
#define GMAC_MIU0_512MB         ( 0x10UL << 6)      // (EMAC)
#define GMAC_MIU0_256MB         ( 0x11UL << 6)      // (EMAC)
// -------- EMAC_CFG3: (EMAC Offset: 0x104) Network Configuration Register3 --------
#define GMAC_INT_DELAY_MODE_EN  ( 0x1UL <<  7)      // (EMAC)
#define GMAC_INT_DELAY_NUMBER_SHIFT 16              // (EMAC)
#define GMAC_INT_DELAY_NUMBER_MASK  0xFF00FFFFUL    // (EMAC)
#define GMAC_INT_DELAY_TIME_SHIFT   24              // (EMAC)
#define GMAC_INT_DELAY_TIME_MASK    0x00FFFFFFUL    // (EMAC)
// -------- EMAC_RX_DELAY_MODE: (EMAC Offset: 0x108) Network Configuration Register --------
/* ........................................................................ */
#define GMAC_DESC_DONE                  0x00000001UL  /* bit for if DMA is done */
#define GMAC_DESC_WRAP                  0x00000002UL  /* bit for wrap */
#define GMAC_BROADCAST                  0x80000000UL  /* broadcast address */
#define GMAC_MULTICAST                  0x40000000UL  /* multicast address */
#define GMAC_UNICAST                    0x20000000UL  /* unicast address */

#define GMAC_DESC_IPV4                  (0x1UL << 11)
#define GMAC_DESC_UDP                   (0x1UL << 18)
#define GMAC_DESC_TCP                   (0x1UL << 19)
#define GMAC_DESC_IP_CSUM               (0x1UL << 20)
#define GMAC_DESC_TCP_UDP_CSUM          (0x1UL << 21)
// Constant: ----------------------------------------------------------------
// Register MAP:
#define GMAC_REG_ETH_CTL                    0x00000000UL         // Network control register
#define GMAC_REG_ETH_CFG                    0x00000004UL         // Network configuration register
#define GMAC_REG_ETH_SR                     0x00000008UL         // Network status register
#define GMAC_REG_ETH_TAR                    0x0000000CUL         // Transmit address register
#define GMAC_REG_ETH_TCR                    0x00000010UL         // Transmit control register
#define GMAC_REG_ETH_TSR                    0x00000014UL         // Transmit status register
#define GMAC_REG_ETH_RBQP                   0x00000018UL         // Receive buffer queue pointer
#define GMAC_REG_ETH_BUFF                   0x0000001CUL         // Receive Buffer Configuration
#define GMAC_REG_ETH_RSR                    0x00000020UL         // Receive status register
#define GMAC_REG_ETH_ISR                    0x00000024UL         // Interrupt status register
#define GMAC_REG_ETH_IER                    0x00000028UL         // Interrupt enable register
#define GMAC_REG_ETH_IDR                    0x0000002CUL         // Interrupt disable register
#define GMAC_REG_ETH_IMR                    0x00000030UL         // Interrupt mask register
#define GMAC_REG_ETH_MAN                    0x00000034UL         // PHY maintenance register
#define GMAC_REG_ETH_BUFFRDPTR              0x00000038UL         // Receive First Full Pointer
#define GMAC_REG_ETH_BUFFWRPTR              0x0000003CUL         // Receive Current pointer
#define GMAC_REG_ETH_FRA                    0x00000040UL         // Frames transmitted OK
#define GMAC_REG_ETH_SCOL                   0x00000044UL         // Single collision frames
#define GMAC_REG_ETH_MCOL                   0x00000048UL         // Multiple collision frames
#define GMAC_REG_ETH_OK                     0x0000004CUL         // Frames received OK
#define GMAC_REG_ETH_SEQE                   0x00000050UL         // Frame check sequence errors
#define GMAC_REG_ETH_ALE                    0x00000054UL         // Alignment errors
#define GMAC_REG_ETH_DTE                    0x00000058UL         // Deferred transmission frames
#define GMAC_REG_ETH_LCOL                   0x0000005CUL         // Late collisions
#define GMAC_REG_ETH_ECOL                   0x00000060UL         // Excessive collisions
#define GMAC_REG_ETH_TUE                    0x00000064UL         // Transmit under-run errors
#define GMAC_REG_ETH_CSE                    0x00000068UL         // Carrier sense errors
#define GMAC_REG_ETH_RE                     0x0000006CUL         // Receive resource error
#define GMAC_REG_ETH_ROVR                   0x00000070UL         // Received overrun
#define GMAC_REG_ETH_SE                     0x00000074UL         // Received symbols error
#define GMAC_REG_ETH_ELR                    0x00000078UL         // Excessive length errors
#define GMAC_REG_ETH_RJB                    0x0000007CUL         // Receive jabbers
#define GMAC_REG_ETH_USF                    0x00000080UL         // Undersize frames
#define GMAC_REG_ETH_SQEE                   0x00000084UL         // SQE test errors
#define GMAC_REG_ETH_HSL                    0x00000090UL         // Hash register [31:0]
#define GMAC_REG_ETH_HSH                    0x00000094UL         // Hash register [63:32]
#define GMAC_REG_ETH_SA1L                   0x00000098UL         // Specific address 1 first 4 bytes
#define GMAC_REG_ETH_SA1H                   0x0000009CUL         // Specific address 1 last  2 bytes
#define GMAC_REG_ETH_SA2L                   0x000000A0UL         // Specific address 2 first 4 bytes
#define GMAC_REG_ETH_SA2H                   0x000000A4UL         // Specific address 2 last  2 bytes
#define GMAC_REG_ETH_SA3L                   0x000000A8UL         // Specific address 3 first 4 bytes
#define GMAC_REG_ETH_SA3H                   0x000000ACUL         // Specific address 3 last  2 bytes
#define GMAC_REG_ETH_SA4L                   0x000000B0UL         // Specific address 4 first 4 bytes
#define GMAC_REG_ETH_SA4H                   0x000000B4UL         // Specific address 4 last  2 bytes
#define GMAC_REG_TAG_TYPE                   0x000000B8UL         // tag type of the frame
#define GMAC_REG_CAMA0_l                    0x00000200UL         // 16 LSB of CAM address  0
#define GMAC_REG_CAMA0_h                    0x00000204UL         // 32 MSB of CAM address  0
#define GMAC_REG_CAMA62_l                   0x000003F0UL         // 16 LSB of CAM address 62
#define GMAC_REG_CAMA62_h                   0x000003F4UL         // 32 MSB of CAM address 62
#define REG_RW32_CFG2                       0x00000100UL
#define REG_RW32_CFG3                       0x00000104UL
#define REG_RW32_RX_DELAY_MODE_STATUS       0x00000108UL
#define REG_RW32_CFG4                       0x00000414UL
#define REG_RW32_CFG5                       0x00000418UL

u32 MHal_GMAC_ReadReg32( u32 xoffset );
u32 MHal_GMAC_ReadRam32( phys_addr_t uRamAddr, u32 xoffset);
u32 MHal_GMAC_Hardware_check( u32 padmux_type );
u32 MHal_GMAC_NegotiationPHY( u8 gmac_phyaddr );
u32 MHal_GMAC_CableConnection( u8 gmac_phyaddr );
u32 MHal_GMAC_get_SA1H_addr(void);
u32 MHal_GMAC_get_SA1L_addr(void);
u32 MHal_GMAC_get_SA2H_addr(void);
u32 MHal_GMAC_get_SA2L_addr(void);
u32 MHal_GMAC_Read_CTL(void);
u32 MHal_GMAC_Read_CFG(void);
u32 MHal_GMAC_Read_RBQP(void);
u32 MHal_GMAC_Read_ISR(void);
u32 MHal_GMAC_Read_IDR(void);
u32 MHal_GMAC_Read_IER(void);
u32 MHal_GMAC_Read_IMR(void);
u32 MHal_GMAC_Read_RDPTR(void);
u32 MHal_GMAC_Read_BUFF(void);
u32 MHal_GMAC_Read_FRA(void);
u32 MHal_GMAC_Read_SCOL(void);
u32 MHal_GMAC_Read_MCOL(void);
u32 MHal_GMAC_Read_OK(void);
u32 MHal_GMAC_Read_SEQE(void);
u32 MHal_GMAC_Read_ALE(void);
u32 MHal_GMAC_Read_LCOL(void);
u32 MHal_GMAC_Read_ECOL(void);
u32 MHal_GMAC_Read_TUE(void);
u32 MHal_GMAC_Read_TCR( void );
u32 MHal_GMAC_Read_TSR(void);
u32 MHal_GMAC_Read_RSR( void );
u32 MHal_GMAC_Read_CSE(void);
u32 MHal_GMAC_Read_RE(void);
u32 MHal_GMAC_Read_ROVR(void);
u32 MHal_GMAC_Read_SE(void);
u32 MHal_GMAC_Read_ELR(void);
u32 MHal_GMAC_Read_RJB(void);
u32 MHal_GMAC_Read_USF(void);
u32 MHal_GMAC_Read_SQEE(void);
u32 MHal_GMAC_Read_Network_config_register2(void);
u32 MHal_GMAC_Read_Network_config_register3(void);
u32 MHal_GMAC_Read_Delay_interrupt_status(void);
u32 MHal_GMAC_Read_Network_config_register4(void);
u32 MHal_GMAC_Read_Network_config_register5(void);
u32 MHal_GMAC_Get_Miu_Priority(void);
u32 MHal_GMAC_IRQ(void);
u8  MHal_GMAC_PHY_ADDR(void);
phys_addr_t MHal_GMAC_REG_ADDR_BASE(void);

void MHal_GMAC_WritReg32( u32 xoffset, u32 xval );
void MHal_GMAC_update_HSH(u32 mc0, u32 mc1);
void MHal_GMAC_Write_CTL(u32 xval);
void MHal_GMAC_Write_CFG(u32 xval);
void MHal_GMAC_Write_RBQP(u32 u32des);
void MHal_GMAC_Write_BUFF(u32 xval);
void MHal_GMAC_Write_MAN(u32 xval);
void MHal_GMAC_Write_TAR(u32 xval);
void MHal_GMAC_Write_TCR(u32 xval);
void MHal_GMAC_Write_TSR( u32 xval );
void MHal_GMAC_Write_RSR( u32 xval );
void MHal_GMAC_Write_RDPTR(u32 xval);
void MHal_GMAC_Write_WRPTR(u32 xval);
void MHal_GMAC_Write_ISR( u32 xval );
void MHal_GMAC_Write_IER(u32 xval);
void MHal_GMAC_Write_IDR(u32 xval);
void MHal_GMAC_Write_IMR(u32 xval);
void MHal_GMAC_Write_SA1H(u32 xval);
void MHal_GMAC_Write_SA1L(u32 xval);
void MHal_GMAC_Write_SA2H(u32 xval);
void MHal_GMAC_Write_SA2L(u32 xval);
void MHal_GMAC_Write_Network_config_register2(u32 xval);
void MHal_GMAC_Write_Network_config_register3(u32 xval);
void MHal_GMAC_Write_Network_config_register4(u32 xval);
void MHal_GMAC_Write_Network_config_register5(u32 xval);
void MHal_GMAC_Set_Miu_Priority(u32 xval);
void MHal_GMAC_Set_Tx_Rx_Req_Priority_Switch(u32 xval);
void MHal_GMAC_Write_Protect(u32 start_addr, u32 length);
void MHal_GMAC_HW_init(void);
void MHal_GMAC_Power_On_Clk( void );
void MHal_GMAC_Power_Off_Clk(void);
void MHal_GMAC_timer_callback(u32 value);
void MHal_GMAC_WritRam32(phys_addr_t uRamAddr, u32 xoffset,u32 xval);
void MHal_GMAC_enable_mdi(void);
void MHal_GMAC_disable_mdi(void);
void MHal_GMAC_write_phy (u8 phy_addr, u8 address, u32 value);
void MHal_GMAC_read_phy(u8 phy_addr, u8 address,u32 *value);
void MHal_GMAC_enable_phyirq (void);
void MHal_GMAC_disable_phyirq (void);
void MHal_GMAC_update_speed_duplex(u32 uspeed, u32 uduplex);
void MHal_GMAC_link_led_on(void);
void MHal_GMAC_link_led_off(void);
void MHal_GMAC_Write_SA1_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
void MHal_GMAC_Write_SA2_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
void MHal_GMAC_Write_SA3_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
void MHal_GMAC_Write_SA4_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
#endif
// -----------------------------------------------------------------------------
// Linux GMAC.c End
// -----------------------------------------------------------------------------
