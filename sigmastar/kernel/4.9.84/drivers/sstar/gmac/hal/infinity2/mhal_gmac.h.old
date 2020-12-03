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
/// @author MStar Semiconductor Inc.
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
//#define GMAC_RX_SOFTWARE_DESCRIPTOR
#define GMAC_SOFTWARE_DESCRIPTOR
#define GMAC_RX_CHECKSUM
#define GMAC_INT_JULIAN_D
#define GMAC_CHIP_FLUSH_READ
#define GMAC_TX_QUEUE_4
//#define GMAC_NAPI
#define GMAC_TX_COUNT     (13) //offset of TX counter in TSR
#define GMAC_LINK_LED_CONTROL

// Workaround for MST231E-D01A
#ifndef CONFIG_GMAC_ETHERNET_ALBANY
#define KANO_GMAC0
#endif

#ifdef CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE
#define TX_SW_QUEUE_SIZE (1024)  //effected size = TX_RING_SIZE - 1
#define TX_DESC_CLEARED  0
#define TX_DESC_WROTE    1
#define TX_DESC_READ     2
#define TX_FIFO_SIZE     4 //HW FIFO size
#endif /* CONFIG_GMAC_TX_ZERO_COPY_SW_QUEUE */

#ifndef KANO_GMAC0
#define E_IRQEXPH_SECEMAC  E_IRQHYPH_SECGMAC //aliasing for interrupt enum (GMAC1 & Internal)
#else
#define E_IRQEXPH_SECEMAC    E_IRQ_GMAC  //aliasing for interrupt enum (GMAC0)
#endif
#ifdef TX_QUEUE_4
#define GMAC_INT_MASK (0xdff)
#else
#define GMAC_INT_MASK (0xdff)
#endif

#ifdef GMAC_RX_SOFTWARE_DESCRIPTOR
//#define GMAC_RX_BYTE_ALIGN_OFFSET
#endif
// Compiler Switches
#define GMAC_REG_BIT_MAP
#define GMAC_URANUS_ETHER_ADDR_CONFIGURABLE	/* MAC address can be changed? */
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define TRUE                                1
#define FALSE                               0

#define GMAC_SOFTWARE_DESCRIPTOR_ENABLE          0x0001
#define GMAC_CHECKSUM_ENABLE                     0x0FE
#define GMAC_RX_CHECKSUM_ENABLE                  0x000E
#define CONFIG_GMAC_MOA                     1   // System Type
#define GMAC_SPEED_100                      100

#if defined(CONFIG_ARM)
#define  mstar_pm_base                      0xFD000000
#endif

#define GMAC_ALLFF                          0xFFFFFFFF
#define GMAC_ABSO_MEM_BASE                  0xA0000000//GMAC_ABSO_MEM_BASE                  0xA0000000
#define GMAC_INTERNEL_PHY_REG_BASE          mstar_pm_base + (0x3100UL<<1)

#define GMAC_RIU_REG_BASE                   mstar_pm_base

#define GMAC_ABSO_PHY_BASE                  0x80000000//GMAC_ABSO_MEM_BASE
#define GMAC_ABSO_MEM_SIZE                  0x30000//0x16000//0x180000//0x16000//(48 * 1024)     // More than: (32 + 16(0x3FFF)) KB
#define GMAC_MEM_SIZE_SQU                   4    // More than: (32 + 16(0x3FFF)) KB
#define GMAC_BUFFER_MEM_SIZE                0x0004000
#define GMAC_MAX_TX_QUEUE                   1000
// Base address here:
#define GMAC_MIU0_BUS_BASE                       MSTAR_MIU0_BUS_BASE
#ifndef KANO_GMAC0
#define GMAC_REG_ADDR_BASE                       mstar_pm_base + (0x122000UL<<1)      // The register address base. Depends on system define.
#else
#define GMAC_REG_ADDR_BASE                       mstar_pm_base + (0x121b00UL<<1)      // The register address base. Depends on system define.
#endif

#ifdef CONFIG_MSTAR_GMAC_V2
#define GMAC_RBQP_LENG                           0x400//0x0100  0x40//                // ==?descriptors
#define GMAC_SOFTWARE_DESC_LEN                   0x800
#else
#define GMAC_RBQP_LENG                           0x20//0x0100  0x40//                // ==?descriptors
#define GMAC_SOFTWARE_DESC_LEN                   0x4000
#endif
#define GMAC_MAX_RX_DESCR                        GMAC_RBQP_LENG//32   /* max number of receive buffers */
#ifdef GMAC_SOFTWARE_DESCRIPTOR
#define GMAC_RX_BUFFER_SEL                       0x0003          // 0x0=2KB,0x1=4KB,0x2=8KB,0x3=16KB, 0x09=1MB
#define GMAC_RX_BUFFER_SIZE                      (GMAC_RBQP_LENG*GMAC_SOFTWARE_DESC_LEN)  //0x10000//0x20000//
#else
#define GMAC_RX_BUFFER_SEL                       0x0003          // 0x0=2KB,0x1=4KB,0x2=8KB,0x3=16KB, 0x09=1MB
#define GMAC_RX_BUFFER_SIZE                      (0x2000<<GMAC_RX_BUFFER_SEL)//0x10000//0x20000//
#endif
#define GMAC_RBQP_SIZE                           (0x0020*GMAC_RBQP_LENG)      // ==8bytes*?==?descriptors

#define GMAC_MAX_INT_COUNTER                     100
#define GMAC_JULIAN_100_VAL                      (0x0000F0C7)
#ifdef CONFIG_MSTAR_GMAC_V2
#define GMAC_JULIAN_104_VAL                      (0x04010080)
#else
#define GMAC_JULIAN_104_VAL                      (0x04020080)
#endif
#define GMAC_TX_CHECKSUM_ENABLE                  (0x00000470)
#define GMAC_TX_V6_CHECKSUM_ENABLE               (0x000C0000)
#define GMAC_TX_JUMBO_FRAME_ENABLE               (0x0000ffff)
//PHY Register
#define GMAC_PHY_REG_BASIC	                    (0)
#define GMAC_PHY_REG_STATUS	                    (1)
//PHY setting
#define GMAC_PHY_REGADDR_OFFSET                  (18)
#define GMAC_PHY_ADDR_OFFSET                     (23)

//-------------------------------------------------------------------------------------------------
//  Bit Define
//-------------------------------------------------------------------------------------------------
#define GMAC_MIU_RW       (0x3 <<  10)   //GMAC power on clk

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
// ........................................................................ //
//URANUS PHY //
#define GMAC_MII_URANUS_ID                   0x01111//Test value
//URANUS specific registers //
#define GMAC_MII_USCR_REG	                16
#define GMAC_MII_USCSR_REG                   17
#define GMAC_MII_USINTR_REG                  21
/* ........................................................................ */
#define GMAC_DESC_DONE                  0x00000001  /* bit for if DMA is done */
#define GMAC_DESC_WRAP                  0x00000002  /* bit for wrap */
#define GMAC_BROADCAST                  0x80000000  /* broadcast address */
#define GMAC_MULTICAST                  0x40000000  /* multicast address */
#define GMAC_UNICAST                    0x20000000  /* unicast address */

#define GMAC_DESC_IPV4                  (0x1 << 11)
#define GMAC_DESC_UDP                   (0x1 << 18)
#define GMAC_DESC_TCP                   (0x1 << 19)
#define GMAC_DESC_IP_CSUM               (0x1 << 20)
#define GMAC_DESC_TCP_UDP_CSUM          (0x1 << 21)
// Constant: ----------------------------------------------------------------
// Register MAP:
#define GMAC_REG_ETH_CTL                     0x00000000         // Network control register
#define GMAC_REG_ETH_CFG                     0x00000004         // Network configuration register
#define GMAC_REG_ETH_SR                      0x00000008         // Network status register
#define GMAC_REG_ETH_TAR                     0x0000000C         // Transmit address register
#define GMAC_REG_ETH_TCR                     0x00000010         // Transmit control register
#define GMAC_REG_ETH_TSR                     0x00000014         // Transmit status register
#define GMAC_REG_ETH_RBQP                    0x00000018         // Receive buffer queue pointer
#define GMAC_REG_ETH_BUFF                    0x0000001C         // Receive Buffer Configuration
#define GMAC_REG_ETH_RSR                     0x00000020         // Receive status register
#define GMAC_REG_ETH_ISR                     0x00000024         // Interrupt status register
#define GMAC_REG_ETH_IER                     0x00000028         // Interrupt enable register
#define GMAC_REG_ETH_IDR                     0x0000002C         // Interrupt disable register
#define GMAC_REG_ETH_IMR                     0x00000030         // Interrupt mask register
#define GMAC_REG_ETH_MAN                     0x00000034         // PHY maintenance register
#define GMAC_REG_ETH_BUFFRDPTR               0x00000038         // Receive First Full Pointer
#define GMAC_REG_ETH_BUFFWRPTR               0x0000003C         // Receive Current pointer
#define GMAC_REG_ETH_FRA                     0x00000040         // Frames transmitted OK
#define GMAC_REG_ETH_SCOL                    0x00000044         // Single collision frames
#define GMAC_REG_ETH_MCOL                    0x00000048         // Multiple collision frames
#define GMAC_REG_ETH_OK                      0x0000004C         // Frames received OK
#define GMAC_REG_ETH_SEQE                    0x00000050         // Frame check sequence errors
#define GMAC_REG_ETH_ALE                     0x00000054         // Alignment errors
#define GMAC_REG_ETH_DTE                     0x00000058         // Deferred transmission frames
#define GMAC_REG_ETH_LCOL                    0x0000005C         // Late collisions
#define GMAC_REG_ETH_ECOL                    0x00000060         // Excessive collisions
#define GMAC_REG_ETH_TUE                     0x00000064         // Transmit under-run errors
#define GMAC_REG_ETH_CSE                     0x00000068         // Carrier sense errors
#define GMAC_REG_ETH_RE                      0x0000006C         // Receive resource error
#define GMAC_REG_ETH_ROVR                    0x00000070         // Received overrun
#define GMAC_REG_ETH_SE                      0x00000074         // Received symbols error
#define GMAC_REG_ETH_ELR                     0x00000078         // Excessive length errors
#define GMAC_REG_ETH_RJB                     0x0000007C         // Receive jabbers
#define GMAC_REG_ETH_USF                     0x00000080         // Undersize frames
#define GMAC_REG_ETH_SQEE                    0x00000084         // SQE test errors
#define GMAC_REG_ETH_HSL                     0x00000090         // Hash register [31:0]
#define GMAC_REG_ETH_HSH                     0x00000094         // Hash register [63:32]
#define GMAC_REG_ETH_SA1L                    0x00000098         // Specific address 1 first 4 bytes
#define GMAC_REG_ETH_SA1H                    0x0000009C         // Specific address 1 last  2 bytes
#define GMAC_REG_ETH_SA2L                    0x000000A0         // Specific address 2 first 4 bytes
#define GMAC_REG_ETH_SA2H                    0x000000A4         // Specific address 2 last  2 bytes
#define GMAC_REG_ETH_SA3L                    0x000000A8         // Specific address 3 first 4 bytes
#define GMAC_REG_ETH_SA3H                    0x000000AC         // Specific address 3 last  2 bytes
#define GMAC_REG_ETH_SA4L                    0x000000B0         // Specific address 4 first 4 bytes
#define GMAC_REG_ETH_SA4H                    0x000000B4         // Specific address 4 last  2 bytes
#define GMAC_REG_TAG_TYPE                    0x000000B8         // tag type of the frame
#define GMAC_REG_CAMA0_l                     0x00000200         // 16 LSB of CAM address  0
#define GMAC_REG_CAMA0_h                     0x00000204         // 32 MSB of CAM address  0
#define GMAC_REG_CAMA62_l                    0x000003F0         // 16 LSB of CAM address 62
#define GMAC_REG_CAMA62_h                    0x000003F4         // 32 MSB of CAM address 62
#define GMAC_REG_JULIAN_0100            0x00000100
#define GMAC_REG_JULIAN_0104            0x00000104
#define GMAC_REG_JULIAN_0108            0x00000108
#define GMAC_REG_JULIAN_0138            0x00000138
#define GMAC_REG_JULIAN_0414            0x00000414
#define GMAC_REG_JULIAN_0418            0x00000418

u32 MHal_GMAC_ReadReg32( u32 xoffset );
u32 MHal_GMAC_ReadRam32( phys_addr_t uRamAddr, u32 xoffset);
u32 MHal_GMAC_NegotiationPHY(void);
u32 MHal_GMAC_CableConnection(void);
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
u32 MHal_GMAC_Read_JULIAN_0100(void);
u32 MHal_GMAC_Read_JULIAN_0104(void);
u32 MHal_GMAC_Read_JULIAN_0108(void);
u32 MHal_GMAC_Read_JULIAN_0414(void);
u32 MHal_GMAC_Read_JULIAN_0418(void);

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
void MHal_GMAC_Write_JULIAN_0100(u32 xval);
void MHal_GMAC_Write_JULIAN_0104(u32 xval);
void MHal_GMAC_Write_JULIAN_0108(u32 xval);
void MHal_GMAC_Write_JULIAN_0414(u32 xval);
void MHal_GMAC_Write_JULIAN_0418(u32 xval);
void MHal_GMAC_Set_Tx_JULIAN_T(u32 xval);
u32 MHal_GMAC_Get_Tx_FIFO_Threshold(void);
void MHal_GMAC_Set_Rx_FIFO_Enlarge(u32 xval);
u32 MHal_GMAC_Get_Rx_FIFO_Enlarge(void);
void MHal_GMAC_Set_Miu_Priority(u32 xval);
u32 MHal_GMAC_Get_Miu_Priority(void);
void MHal_GMAC_Set_Tx_Hang_Fix_ECO(u32 xval);
void MHal_GMAC_Set_MIU_Out_Of_Range_Fix(u32 xval);
void MHal_GMAC_Set_Rx_Tx_Burst16_Mode(u32 xval);
void MHal_GMAC_Set_Tx_Rx_Req_Priority_Switch(u32 xval);
void MHal_GMAC_Set_Rx_Byte_Align_Offset(u32 xval);
void MHal_GMAC_Write_Protect(u32 start_addr, u32 length);
void MHal_GMAC_HW_init(void);
void MHal_GMAC_Power_On_Clk( void );
void MHal_GMAC_Power_Off_Clk(void);
void MHal_GMAC_timer_callback(unsigned long value);
void MHal_GMAC_WritRam32(phys_addr_t uRamAddr, u32 xoffset,u32 xval);
void MHal_GMAC_enable_mdi(void);
void MHal_GMAC_disable_mdi(void);
void MHal_GMAC_write_phy (unsigned char phy_addr, unsigned char address, u32 value);
void MHal_GMAC_read_phy(unsigned char phy_addr, unsigned char address,u32 *value);
void MHal_GMAC_enable_phyirq (void);
void MHal_GMAC_disable_phyirq (void);
void MHal_GMAC_update_speed_duplex(u32 uspeed, u32 uduplex);
void MHal_GMAC_link_led_on();
void MHal_GMAC_link_led_off();
void MHal_GMAC_Write_SA1_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
void MHal_GMAC_Write_SA2_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
void MHal_GMAC_Write_SA3_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
void MHal_GMAC_Write_SA4_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5);
#endif
// -----------------------------------------------------------------------------
// Linux GMAC.c End
// -----------------------------------------------------------------------------


