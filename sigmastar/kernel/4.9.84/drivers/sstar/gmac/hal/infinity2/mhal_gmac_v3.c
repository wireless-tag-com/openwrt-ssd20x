/*
* mhal_gmac_v3.c- Sigmastar
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
/// @file   Mhal_gmac.c
/// @brief  GMAC Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/mii.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/pci.h>
#include <irqs.h>
//#include <linux/clkm.h>
#include "mhal_gmac_v3.h"
//#include "chip_int.h"
#include"gpio.h"

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
struct _MHalBasicConfigGMAC
{
    u8 connected;           // 0:No, 1:Yes
    u8 speed;               // 10:10Mbps, 100:100Mbps
    // ETH_CTL Register:
    u8 wes;                 // 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
    // ETH_CFG Register:
    u8 duplex;              // 1:Half-duplex, 2:Full-duplex
    u8 cam;                 // 0:No CAM, 1:Yes
    u8 rcv_bcast;           // 0:No, 1:Yes
    u8 rlf;                 // 0:No, 1:Yes receive long frame(1522)
    // MAC Address:
    u8 sa1[6];              // Specific Addr 1 (MAC Address)
    u8 sa2[6];              // Specific Addr 2
    u8 sa3[6];              // Specific Addr 3
    u8 sa4[6];              // Specific Addr 4
};
typedef struct _MHalBasicConfigGMAC MHalBasicConfigGMAC;

struct _MHalUtilityVarsGMAC
{
    u32 cntChkCableConnect;
    u32 cntChkINTCounter;
    u32 readIdxRBQP;        // Reset = 0x00000000
    u32 rxOneFrameAddr;     // Reset = 0x00000000 (Store the Addr of "ReadONE_RX_Frame")
    u8  flagISR_INT_DONE;
};
typedef struct _MHalUtilityVarsGMAC MHalUtilityVarsGMAC;

MHalBasicConfigGMAC MHalGMACThisBCE;
MHalUtilityVarsGMAC MHalGMACThisUVE;

struct _MHal_GMAC_Ops{
    void (*power_on)(void);
    void (*update_speed_duplex)(u32 uspeed, u32 uduplex);
    void (*write_phy)(unsigned char phy_addr, unsigned char address, u32 value);
    void (*read_phy)(unsigned char phy_addr, unsigned char address, u32* value);
    phys_addr_t GMAC_Reg_Addr_Base;
    phys_addr_t GMAC_X32_Reg_Addr_Base;
    u32 GMAC_IRQ;
    u32 GMAC_BANK;
    u8 GPHY_ADDR;
};
typedef struct _MHal_GMAC_Ops MHal_GMAC_Ops;

MHal_GMAC_Ops MHal_GMAC_This_Ops;

//-------------------------------------------------------------------------------------------------
//  GMAC hardware for Titania
//-------------------------------------------------------------------------------------------------

/*8-bit RIU address*/
u8 MHal_GMAC_ReadReg8( u32 bank, u32 reg )
{
    u8 val;
    phys_addr_t address = RIU_REG_BASE + bank*0x100*2;
    address = address + (reg << 1) - (reg & 1);

    val = *( ( volatile u8* ) address );
    return val;
}

void MHal_GMAC_WritReg8( u32 bank, u32 reg, u8 val )
{
    phys_addr_t address = RIU_REG_BASE + bank*0x100*2;
    address = address + (reg << 1) - (reg & 1);

    *( ( volatile u8* ) address ) = val;
}

u16 MHal_GMAC_ReadReg16( u32 bank, u32 reg )
{
    u16 val;
    u32 address = RIU_REG_BASE + bank*0x100*2;
    address = address + (reg << 1) - (reg & 1);

    val = *( ( volatile u16* ) address );
    return val;
}

void MHal_GMAC_WritReg16( u32 bank, u32 reg, u16 val )
{
    u32 address = RIU_REG_BASE + bank*0x100*2;
    address = address + (reg << 1) - (reg & 1);

    *( ( volatile u16* ) address ) = val;
}

u32 MHal_GMAC_ReadReg32_XIU16( u32 xoffset )
{
    phys_addr_t address = MHal_GMAC_This_Ops.GMAC_Reg_Addr_Base + xoffset*2;

    u32 xoffsetValueL = *( ( volatile u32* ) address ) & 0x0000FFFF;
    u32 xoffsetValueH = *( ( volatile u32* ) ( address + 4) ) << 0x10;
    return( xoffsetValueH | xoffsetValueL );
}

void MHal_GMAC_WritReg32_XIU16( u32 xoffset, u32 xval )
{
    phys_addr_t address = MHal_GMAC_This_Ops.GMAC_Reg_Addr_Base + xoffset*2;
    *( ( volatile u32 * ) address ) = ( u32 ) ( xval & 0x0000FFFF );
    *( ( volatile u32 * ) ( address + 4 ) ) = ( u32 ) ( xval >> 0x10 );
}

u32 MHal_GMAC_ReadReg32_XIU32( u32 xoffset )
{
    u32 val;
    phys_addr_t address = MHal_GMAC_This_Ops.GMAC_X32_Reg_Addr_Base + xoffset*2;

    val = *( ( volatile u32* ) address );
    return val;
}

void MHal_GMAC_WritReg32_XIU32( u32 xoffset, u32 xval )
{
    phys_addr_t address = MHal_GMAC_This_Ops.GMAC_X32_Reg_Addr_Base + xoffset*2;

    *( ( volatile u32 * ) address ) = ( u32 ) xval;
}

u32 MHal_GMAC_ReadReg32( u32 xoffset )
{
    #ifdef XIU32_MODE
        if(xoffset < 0x100UL)
            return MHal_GMAC_ReadReg32_XIU32(xoffset);
        else
            return MHal_GMAC_ReadReg32_XIU16(xoffset);
    #else
        return MHal_GMAC_ReadReg32_XIU16(xoffset);
    #endif
}

void MHal_GMAC_WritReg32( u32 xoffset, u32 xval )
{
    #ifdef XIU32_MODE
        if(xoffset < 0x100UL)
            MHal_GMAC_WritReg32_XIU32(xoffset,xval);
        else
            MHal_GMAC_WritReg32_XIU16(xoffset,xval);
    #else
        MHal_GMAC_WritReg32_XIU16(xoffset,xval);
    #endif
}

u32 MHal_GMAC_ReadRam32( phys_addr_t uRamAddr, u32 xoffset)
{
    return (*( u32 * ) ( ( char * ) uRamAddr + xoffset ) );
}

void MHal_GMAC_WritRam32( phys_addr_t uRamAddr, u32 xoffset, u32 xval )
{
    *( ( u32 * ) ( ( char * ) uRamAddr + xoffset ) ) = xval;
}

void MHal_GMAC_Write_SA1_MAC_Address( u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA1L, w0 );
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA1H, w1 );
}

void MHal_GMAC_Write_SA2_MAC_Address( u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA2L, w0 );
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA2H, w1 );
}

void MHal_GMAC_Write_SA3_MAC_Address( u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA3L, w0 );
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA3H, w1 );
}

void MHal_GMAC_Write_SA4_MAC_Address( u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA4L, w0 );
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA4H, w1 );
}

//-------------------------------------------------------------------------------------------------
//  R/W GMAC register for Titania
//-------------------------------------------------------------------------------------------------

void MHal_GMAC_update_HSH(u32 mc0, u32 mc1)
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_HSL, mc0 );
    MHal_GMAC_WritReg32( GMAC_REG_ETH_HSH, mc1 );
}

//-------------------------------------------------------------------------------------------------
//  Read control register
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_CTL( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_CTL );
}

//-------------------------------------------------------------------------------------------------
//  Write Network control register
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_CTL( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_CTL, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Network configuration register
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_CFG( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_CFG );
}

//-------------------------------------------------------------------------------------------------
//  Write Network configuration register
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_CFG( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_CFG, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read RBQP
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_RBQP( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_RBQP );
}

//-------------------------------------------------------------------------------------------------
//  Write RBQP
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_RBQP( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_RBQP, xval );
}

//-------------------------------------------------------------------------------------------------
//  Write Transmit Address register
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_TAR( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_TAR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read RBQP
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_TCR( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_TCR);
}

//-------------------------------------------------------------------------------------------------
//  Write Transmit Control register
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_TCR( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_TCR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Transmit Status Register
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_TSR( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_TSR, xval );
}

u32 MHal_GMAC_Read_TSR( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_TSR );
}

void MHal_GMAC_Write_RSR( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_RSR, xval );
}

u32 MHal_GMAC_Read_RSR( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_RSR );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt status register
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_ISR( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_ISR, xval );
}

u32 MHal_GMAC_Read_ISR( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_ISR );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt enable register
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_IER( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_IER );
}

//-------------------------------------------------------------------------------------------------
//  Write Interrupt enable register
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_IER( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_IER, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt disable register
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_IDR( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_IDR );
}

//-------------------------------------------------------------------------------------------------
//  Write Interrupt disable register
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_IDR( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_IDR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt mask register
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_IMR( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_IMR );
}

//-------------------------------------------------------------------------------------------------
//  Read PHY maintenance register
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_MAN( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_MAN );
}

//-------------------------------------------------------------------------------------------------
//  Write PHY maintenance register
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_MAN( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_MAN, xval );
}

//-------------------------------------------------------------------------------------------------
//  Write Receive Buffer Configuration
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_BUFF( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_BUFF, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Receive Buffer Configuration
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_BUFF( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_BUFF );
}

//-------------------------------------------------------------------------------------------------
//  Read Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_RDPTR( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_BUFFRDPTR );
}

//-------------------------------------------------------------------------------------------------
//  Write Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_RDPTR( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_BUFFRDPTR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Write Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_WRPTR( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_BUFFWRPTR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Frames transmitted OK
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_FRA( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_FRA );
}

//-------------------------------------------------------------------------------------------------
//  Single collision frames
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_SCOL( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_SCOL );
}

//-------------------------------------------------------------------------------------------------
//  Multiple collision frames
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_MCOL( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_MCOL );
}

//-------------------------------------------------------------------------------------------------
//  Frames received OK
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_OK( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_OK );
}

//-------------------------------------------------------------------------------------------------
//  Frame check sequence errors
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_SEQE( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_SEQE );
}

//-------------------------------------------------------------------------------------------------
//  Alignment errors
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_ALE( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_ALE );
}

//-------------------------------------------------------------------------------------------------
//  Late collisions
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_LCOL( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_LCOL );
}

//-------------------------------------------------------------------------------------------------
//  Excessive collisions
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_ECOL( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_ECOL );
}

//-------------------------------------------------------------------------------------------------
//  Transmit under-run errors
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_TUE( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_TUE );
}

//-------------------------------------------------------------------------------------------------
//  Carrier sense errors
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_CSE( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_CSE );
}

//-------------------------------------------------------------------------------------------------
//  Receive resource error
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_RE( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_RE );
}

//-------------------------------------------------------------------------------------------------
//  Received overrun
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_ROVR( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_ROVR );
}

//-------------------------------------------------------------------------------------------------
//  Received symbols error
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_SE( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_SE );
}

//-------------------------------------------------------------------------------------------------
//  Excessive length errors
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_ELR( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_ELR );
}

//-------------------------------------------------------------------------------------------------
//  Receive jabbers
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_RJB( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_RJB );
}

//-------------------------------------------------------------------------------------------------
//  Undersize frames
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_USF( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_USF );
}

//-------------------------------------------------------------------------------------------------
//  SQE test errors
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_SQEE( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_SQEE );
}

//-------------------------------------------------------------------------------------------------
//  Read Network configuration register2
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_Network_config_register2( void )
{
    return MHal_GMAC_ReadReg32( REG_RW32_CFG2 );
}

//-------------------------------------------------------------------------------------------------
//  Write Network configuration register2
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_Network_config_register2( u32 xval )
{
    MHal_GMAC_WritReg32( REG_RW32_CFG2, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Network configuration register3
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_Network_config_register3( void )
{
    return MHal_GMAC_ReadReg32( REG_RW32_CFG3 );
}

//-------------------------------------------------------------------------------------------------
//  Write Network configuration register3
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_Network_config_register3( u32 xval )
{
    MHal_GMAC_WritReg32( REG_RW32_CFG3, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Delay_interrupt_status
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_Delay_interrupt_status( void )
{
    return MHal_GMAC_ReadReg32( REG_RW32_RX_DELAY_MODE_STATUS );
}

//-------------------------------------------------------------------------------------------------
//  Read Low-Priority TX Descriptor Base Address
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_LOW_PRI_TX_DESC_BASE( void )
{
    return MHal_GMAC_ReadReg32_XIU16( REG_RW32_LOW_PRI_TX_DESC_BASE );
}

//-------------------------------------------------------------------------------------------------
//  Write Low-Priority TX Descriptor Base Address
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_LOW_PRI_TX_DESC_BASE( u32 xval )
{
    MHal_GMAC_WritReg32_XIU16( REG_RW32_LOW_PRI_TX_DESC_BASE, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Low-Priority TX Descriptor Pointer Address
//-------------------------------------------------------------------------------------------------
u16 MHal_GMAC_Read_LOW_PRI_TX_DESC_PTR( void )
{
    return MHal_GMAC_ReadReg16( REG_GMAC0_BANK, REG_RO16_LOW_PRI_TX_DESC_PTR );
}

//-------------------------------------------------------------------------------------------------
//  Read Low-Priority TX Descriptor Queued Packets Number
//-------------------------------------------------------------------------------------------------
u16 MHal_GMAC_Read_LOW_PRI_TX_DESC_QUEUED( void )
{
    return MHal_GMAC_ReadReg16( REG_GMAC0_BANK, REG_RO16_LOW_PRI_TX_DESC_QUEUED );
}

//-------------------------------------------------------------------------------------------------
//  Read Low-Priority TX Descriptor THRESHOLD
//-------------------------------------------------------------------------------------------------
u16 MHal_GMAC_Read_LOW_PRI_TX_DESC_THRESHOLD( void )
{
    return MHal_GMAC_ReadReg16( REG_GMAC0_BANK, REG_RW16_LOW_PRI_TX_DESC_THRESHOLD );
}

//-------------------------------------------------------------------------------------------------
//  Write Low-Priority TX Descriptor THRESHOLD
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_LOW_PRI_TX_DESC_THRESHOLD( u16 xval )
{
    MHal_GMAC_WritReg16( REG_GMAC0_BANK, REG_RW16_LOW_PRI_TX_DESC_THRESHOLD , xval);
}

//-------------------------------------------------------------------------------------------------
//  Read High-Priority TX Descriptor Base Address
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_HIGH_PRI_TX_DESC_BASE( void )
{
    return MHal_GMAC_ReadReg32_XIU16( REG_RW32_HIGH_PRI_TX_DESC_BASE );
}

//-------------------------------------------------------------------------------------------------
//  Write High-Priority TX Descriptor Base Address
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_HIGH_PRI_TX_DESC_BASE( u32 xval )
{
    MHal_GMAC_WritReg32_XIU16( REG_RW32_HIGH_PRI_TX_DESC_BASE, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read High-Priority TX Descriptor Pointer Address
//-------------------------------------------------------------------------------------------------
u16 MHal_GMAC_Read_HIGH_PRI_TX_DESC_PTR( void )
{
    return MHal_GMAC_ReadReg16( REG_GMAC0_BANK, REG_RO16_HIGH_PRI_TX_DESC_PTR );
}

//-------------------------------------------------------------------------------------------------
//  Read High-Priority TX Descriptor Queued Packets Number
//-------------------------------------------------------------------------------------------------
u16 MHal_GMAC_Read_HIGH_PRI_TX_DESC_QUEUED( void )
{
    return MHal_GMAC_ReadReg16( REG_GMAC0_BANK, REG_RO16_HIGH_PRI_TX_DESC_QUEUED );
}

//-------------------------------------------------------------------------------------------------
//  Read High-Priority TX Descriptor THRESHOLD
//-------------------------------------------------------------------------------------------------
u16 MHal_GMAC_Read_HIGH_PRI_TX_DESC_THRESHOLD( void )
{
    return MHal_GMAC_ReadReg16( REG_GMAC0_BANK, REG_RW16_HIGH_PRI_TX_DESC_THRESHOLD );
}

//-------------------------------------------------------------------------------------------------
//  Write High-Priority TX Descriptor THRESHOLD
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_HIGH_PRI_TX_DESC_THRESHOLD( u16 xval )
{
    MHal_GMAC_WritReg16( REG_GMAC0_BANK, REG_RW16_HIGH_PRI_TX_DESC_THRESHOLD , xval);
}

//-------------------------------------------------------------------------------------------------
//  Write Low-Priority TX Descriptor TRANSMIT0
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_LOW_PRI_TX_DESC_TRANSMIT0( u8 xval )
{
    MHal_GMAC_WritReg8( REG_GMAC0_BANK, REG_WO08_LOW_PRI_TX_DESC_TRANSMIT0 , xval);
}

//-------------------------------------------------------------------------------------------------
//  Write Low-Priority TX Descriptor TRANSMIT1
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_LOW_PRI_TX_DESC_TRANSMIT1( u8 xval )
{
    MHal_GMAC_WritReg8( REG_GMAC0_BANK, REG_WO08_LOW_PRI_TX_DESC_TRANSMIT1 , xval);
}

//-------------------------------------------------------------------------------------------------
//  Write High-Priority TX Descriptor TRANSMIT0
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_HIGH_PRI_TX_DESC_TRANSMIT0( u8 xval )
{
    MHal_GMAC_WritReg8( REG_GMAC0_BANK, REG_WO08_HIGH_PRI_TX_DESC_TRANSMIT0 , xval);
}

//-------------------------------------------------------------------------------------------------
//  Write High-Priority TX Descriptor TRANSMIT1
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_HIGH_PRI_TX_DESC_TRANSMIT1( u8 xval )
{
    MHal_GMAC_WritReg8( REG_GMAC0_BANK, REG_WO08_HIGH_PRI_TX_DESC_TRANSMIT1 , xval);
}

//-------------------------------------------------------------------------------------------------
//  Read Network configuration register4
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_Network_config_register4( void )
{
    return MHal_GMAC_ReadReg32( REG_RW32_CFG4 );
}

//-------------------------------------------------------------------------------------------------
//  Write Network configuration register4
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_Network_config_register4( u32 xval )
{
    MHal_GMAC_WritReg32( REG_RW32_CFG4, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Network configuration register5
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Read_Network_config_register5( void )
{
    return MHal_GMAC_ReadReg32( REG_RW32_CFG5 );
}

//-------------------------------------------------------------------------------------------------
//  Write Network configuration register5
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Write_Network_config_register5( u32 xval )
{
    MHal_GMAC_WritReg32( REG_RW32_CFG5, xval );
}
#ifdef TX_DESC_MODE

u32 MHal_GMAC_LOW_PRI_TX_DESC_MODE_OVRN_Get(void)
{
    u16 val;

    val = MHal_GMAC_Read_LOW_PRI_TX_DESC_QUEUED();
    if ((val & GMAC_RO_TX_DESC_OVERRUN) != 0)
        return 1; //is overrun
    else
        return 0; //is normal
}

#endif

#ifdef NEW_TX_QUEUE
void MHal_GMAC_New_TX_QUEUE_Enable(void)
{
    u32 val;

    val = MHal_GMAC_ReadReg32_XIU16(GMAC_REG_NEW_TX_QUEUE);
    val |= GMAC_NEW_TXQ_EN;
    MHal_GMAC_WritReg32_XIU16(GMAC_REG_NEW_TX_QUEUE, val);
}

u32 MHal_GMAC_New_TX_QUEUE_COUNT_Get(void)
{
    u32 val;

    val = MHal_GMAC_ReadReg32_XIU16(GMAC_REG_NEW_TX_QUEUE);
    val &= GMAC_NEW_TXQ_CNT;
    return val;
}

u32 MHal_GMAC_New_TX_QUEUE_OVRN_Get(void)
{
    u32 val;

    val = MHal_GMAC_ReadReg32_XIU16(GMAC_REG_NEW_TX_QUEUE);
    if ((val & GMAC_NEW_TXQ_OV) != 0)
        return 1; //is overrun
    else
        return 0; //is normal
}

void MHal_GMAC_New_TX_QUEUE_Threshold_Set(u32 thr)
{
    u32 val;

    val = MHal_GMAC_ReadReg32_XIU16(GMAC_REG_NEW_TX_QUEUE);
    val &= ~(GMAC_NEW_TXQ_THR);
    val |= ((thr << GMAC_NEW_TXQ_THR_OFFSET) & GMAC_NEW_TXQ_THR);
    MHal_GMAC_WritReg32_XIU16(GMAC_REG_NEW_TX_QUEUE, val);
}
#endif /* NEW_TX_QUEUE */

void MHal_GMAC_Set_TEST(u32 xval)
{
    u32 value = 0xffffffff;
    int i=0;

    for(i = 0x100; i< 0x160;i+=4){
        MHal_GMAC_WritReg32(i, value);
        }

}

void MHal_GMAC_Set_Miu_Priority(u32 xval)
{
    u32 value;

    value = MHal_GMAC_ReadReg32(REG_RW32_CFG2);
    value &= 0xfff7ffff;
    value |= xval << 19;

    MHal_GMAC_WritReg32(REG_RW32_CFG2, value);
}

u32 MHal_GMAC_Get_Miu_Priority(void)
{
    return (MHal_GMAC_ReadReg32(REG_RW32_CFG2) & 0x00080000) >> 19;
}

void MHal_GMAC_Set_Tx_Rx_Req_Priority_Switch(u32 xval)
{
    u32 value;
    value = MHal_GMAC_ReadReg32(0x134);
    value &= 0xfff7ffff;
    value |= xval << 19;

    MHal_GMAC_WritReg32(0x134, value);
}

void MHal_GMAC_Write_Protect(u32 start_addr, u32 length)
{
    u32 value;

    value = MHal_GMAC_ReadReg32(0x11c);
    value &= 0x0000ffff;
    value |= ((start_addr+length) >> 4) << 16;
    MHal_GMAC_WritReg32(0x11c, value);

    value = MHal_GMAC_ReadReg32(0x120);
    value &= 0x00000000;
    value |= ((start_addr+length) >> 4) >> 16;
    value |= (start_addr >> 4) << 16;
    MHal_GMAC_WritReg32(0x120, value);

    value = MHal_GMAC_ReadReg32(0x124);
    value &= 0xffff0000;
    value |= (start_addr >> 4) >> 16;
    MHal_GMAC_WritReg32(0x124, value);

    value = MHal_GMAC_ReadReg32(REG_RW32_CFG2);
    value |= 0x00000040;
    MHal_GMAC_WritReg32(REG_RW32_CFG2, value);
}

void MHal_GMAC_Set_Miu_Highway(u32 xval)
{
    u32 value;
    value = MHal_GMAC_ReadReg32(0x134);
    value &= 0xbfffffff;
    value |= xval << 30;

    MHal_GMAC_WritReg32(0x134, value);
}

void MHal_GMAC_HW_init(void)
{
    MHal_GMAC_Set_Miu_Priority(1);
}

//-------------------------------------------------------------------------------------------------
//  PHY INTERFACE
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enable the MDIO bit in MAC control register
// When not called from an interrupt-handler, access to the PHY must be
// protected by a spinlock.
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_enable_mdi( void )
{
    u32 xval;
    xval = MHal_GMAC_Read_CTL();
    xval |= GMAC_MPE;
    MHal_GMAC_Write_CTL( xval );
}

//-------------------------------------------------------------------------------------------------
//  Disable the MDIO bit in the MAC control register
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_disable_mdi( void )
{
    u32 xval;
    xval = MHal_GMAC_Read_CTL();
    xval &= ~GMAC_MPE;
    MHal_GMAC_Write_CTL( xval );
}

//-------------------------------------------------------------------------------------------------
// Write value to the a PHY register
// Note: MDI interface is assumed to already have been enabled.
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_write_phy_InternalEPHY( unsigned char phy_addr, unsigned char address, u32 value )
{
    phys_addr_t uRegBase = INTERNEL_PHY_REG_BASE;
    if(phy_addr >= 32) return; // invalid phy address

    phy_addr =0;

    *(volatile unsigned int *)(uRegBase + address*4) = value;
    udelay( 1 );
}

void MHal_GMAC_write_phy_GPHY( unsigned char phy_addr, unsigned char address, u32 value )
{
    u32 uRegVal = 0, uCTL = 0;
    if(phy_addr >= 32) return; // invalid phy address

    uRegVal =  ( GMAC_HIGH | GMAC_CODE_802_3 | GMAC_RW_W) | (( phy_addr & 0x1F ) << GMAC_PHY_ADDR_OFFSET )
                | ( address << GMAC_PHY_REGADDR_OFFSET ) | (value & 0xFFFF);

    uCTL = MHal_GMAC_Read_CTL();
    MHal_GMAC_enable_mdi();

    MHal_GMAC_Write_MAN( uRegVal );
    // Wait until IDLE bit in Network Status register is cleared //
    uRegVal = MHal_GMAC_ReadReg32( GMAC_REG_ETH_SR );  //Must read Low 16 bit.
    while ( !( uRegVal & GMAC_IDLE ) )
    {
        uRegVal = MHal_GMAC_ReadReg32( GMAC_REG_ETH_SR );
        barrier();
    }
    MHal_GMAC_Write_CTL(uCTL);
}

unsigned int mdio_bb_read(int phy,int reg);
unsigned int mdio_bb_write(unsigned int phy,unsigned int reg,unsigned int val);
void mdio_bb_init(int pin_mdio, int pin_mdc);

void MHal_GMAC_write_phy_swgpio( unsigned char phy_addr, unsigned char address, u32 value )
{
    if(phy_addr >= 32) return; // invalid phy address

    mdio_bb_write(phy_addr, address, value);
}

void MHal_GMAC_write_phy( unsigned char phy_addr, unsigned char address, u32 value )
{
    MHal_GMAC_This_Ops.write_phy(phy_addr,address,value);
    //printk("write phy_addr:%d offs:%d val:0x%04x\n", phy_addr, address, value);
}
//-------------------------------------------------------------------------------------------------
// Read value stored in a PHY register.
// Note: MDI interface is assumed to already have been enabled.
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_read_phy_InternalEPHY( unsigned char phy_addr, unsigned char address, u32* value )
{
    phys_addr_t uRegBase = INTERNEL_PHY_REG_BASE;
    u32 tempvalue ;

    if(phy_addr >= 32) return; // invalid phy address
    phy_addr =0;

    tempvalue = *(volatile unsigned int *)(INTERNEL_PHY_REG_BASE + 0x04);
    tempvalue |= 0x0004;
    *(volatile unsigned int *)(INTERNEL_PHY_REG_BASE + 0x04) = tempvalue;
    udelay( 1 );
    *value = *(volatile unsigned int *)(uRegBase + address*4);

    return;
}

void MHal_GMAC_read_phy_GPHY( unsigned char phy_addr, unsigned char address, u32* value )
{
    u32 uRegVal = 0, uCTL = 0;
    if(phy_addr >= 32) return; // invalid phy address

    uRegVal = (GMAC_HIGH | GMAC_CODE_802_3 | GMAC_RW_R)
            | ((phy_addr & 0x1f) << GMAC_PHY_ADDR_OFFSET) | (address << GMAC_PHY_REGADDR_OFFSET) | (0) ;

    uCTL = MHal_GMAC_Read_CTL();
    MHal_GMAC_enable_mdi();
    MHal_GMAC_Write_MAN(uRegVal);

    //Wait until IDLE bit in Network Status register is cleared //
    uRegVal = MHal_GMAC_ReadReg32( GMAC_REG_ETH_SR );  //Must read Low 16 bit.
    while ( !( uRegVal & GMAC_IDLE ) )
    {
        uRegVal = MHal_GMAC_ReadReg32( GMAC_REG_ETH_SR );
        barrier();
    }
    *value = ( MHal_GMAC_Read_MAN() & 0x0000ffff );
    MHal_GMAC_Write_CTL(uCTL);

    return;
}

void MHal_GMAC_read_phy_swgpio( unsigned char phy_addr, unsigned char address, u32* value )
{
    if(phy_addr >= 32) return; // invalid phy address

    *value = mdio_bb_read(phy_addr, address);

    return;
}
void MHal_GMAC_read_phy( unsigned char phy_addr, unsigned char address, u32* value )
{
    MHal_GMAC_This_Ops.read_phy(phy_addr,address,value);
    //printk("read phy_addr:%d offs:%d val:0x%04x\n", phy_addr, address, *value);
}
//-------------------------------------------------------------------------------------------------
// Update MAC speed and H/F duplex
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_update_speed_duplex_GMAC1_EPHY( u32 uspeed, u32 uduplex)
{
    u32 xval;
    u8 uRegVal;

    xval = MHal_GMAC_ReadReg32( GMAC_REG_ETH_CFG ) & ~( GMAC_SPD | GMAC_FD );

    if(uspeed == SPEED_1000)
    {
        /* Disable reg_rgmii_slow */
        uRegVal = MHal_GMAC_ReadReg8(0x0033, 0xb4);
        uRegVal &= 0xfe;
        MHal_GMAC_WritReg8(0x0033, 0xb4, uRegVal);

        if ( uduplex == DUPLEX_FULL )    // 1000 Full Duplex //
        {
            xval = xval | GMAC_FD;
        }
    }
    else if ( uspeed == SPEED_100 )
    {
        /* Enable reg_rgmii_slow */
        uRegVal = MHal_GMAC_ReadReg8(0x0033, 0xb4);
        uRegVal |= 0x01;
        MHal_GMAC_WritReg8(0x0033, 0xb4, uRegVal);

        /* Set reg_rgmii10_100 as 100M*/
        uRegVal = MHal_GMAC_ReadReg8(0x0033, 0xb4);
        uRegVal |= 0x02;
        MHal_GMAC_WritReg8(0x0033, 0xb4, uRegVal);

        if ( uduplex == DUPLEX_FULL )    // 100 Full Duplex //
        {
            xval = xval | GMAC_SPD | GMAC_FD;
        }
        else                           // 100 Half Duplex ///
        {
            xval = xval | GMAC_SPD;
        }
    }
    else
    {
        /* Enable reg_rgmii_slow */
        uRegVal = MHal_GMAC_ReadReg8(0x0033, 0xb4);
        uRegVal |= 0x01;
        MHal_GMAC_WritReg8(0x0033, 0xb4, uRegVal);

        /* Enable reg_rgmii10_100 as 10M*/
        uRegVal = MHal_GMAC_ReadReg8(0x0033, 0xb4);
        uRegVal &= 0xfd;
        MHal_GMAC_WritReg8(0x0033, 0xb4, uRegVal);

        if ( uduplex == DUPLEX_FULL )    //10 Full Duplex //
        {
            xval = xval | GMAC_FD;
        }
        else                           // 10 Half Duplex //
        {
        }
    }
    MHal_GMAC_WritReg32( GMAC_REG_ETH_CFG, xval );
}

void MHal_GMAC_update_speed_duplex_GMAC1_GPHY( u32 uspeed, u32 uduplex)
{
    u32 xval;
    u8 uRegVal;

    xval = MHal_GMAC_ReadReg32( GMAC_REG_ETH_CFG ) & ~( GMAC_SPD | GMAC_FD );

    if(uspeed == SPEED_1000)
    {
        /* Disable reg_rgmii_slow */
        uRegVal = MHal_GMAC_ReadReg8(0x1224, 0xA5);
        uRegVal &= 0xbf;
        MHal_GMAC_WritReg8(0x1224, 0xA5, uRegVal);

        /* Set reg_xmii_type as 1G */
        uRegVal = MHal_GMAC_ReadReg8(0x140f, 0x60);
        uRegVal &= 0xfc;
        MHal_GMAC_WritReg8(0x140f, 0x60, uRegVal);

        if ( uduplex == DUPLEX_FULL )    // 1000 Full Duplex //
        {
            xval = xval | GMAC_FD;
        }
    }
    else if ( uspeed == SPEED_100 )
    {
    /*For io0
    BK_1224_12[15:14] = 2'11 when 100M
                        2'01 when 10M
    BK_121F_30[1:0] = 2'01 when 10 or 100M
    */

        /* Enable reg_rgmii_slow, Set reg_rgmii10_100 as 100M*/
        uRegVal = MHal_GMAC_ReadReg8(0x1224, 0xa5);
        uRegVal |= 0xC0;
        MHal_GMAC_WritReg8(0x1224, 0xa5, uRegVal);

        /* Set reg_xmii_type as 10M/100M */
        uRegVal = MHal_GMAC_ReadReg8(0x140f, 0x60);
        uRegVal &= 0xfc;
        uRegVal |= 0x01;
        MHal_GMAC_WritReg8(0x140f, 0x60, uRegVal);

        if ( uduplex == DUPLEX_FULL )
        {
            /* 100 Full Duplex */
            xval = xval | GMAC_SPD | GMAC_FD;
        }
        else
        {
            /* 100 Half Duplex */
            xval = xval | GMAC_SPD;
        }
    }
    else
    {
        /*For io0
        BK_1224_12[15:14] = 2'11 when 100M
                            2'01 when 10M
        BK_121F_30[1:0] = 2'01 when 10 or 100M
        */

        /* Enable reg_rgmii_slow, Enable reg_rgmii10_100 as 10M*/
        uRegVal = MHal_GMAC_ReadReg8(0x1224, 0xa5);
        uRegVal &= 0x3F;
        uRegVal |= 0x40;
        MHal_GMAC_WritReg8(0x1224, 0xa5, uRegVal);

        /* Set reg_xmii_type as 10M/100M */
        uRegVal = MHal_GMAC_ReadReg8(0x140f, 0x60);
        uRegVal &= 0xfc;
        uRegVal |= 0x01;
        MHal_GMAC_WritReg8(0x140f, 0x60, uRegVal);

        if ( uduplex == DUPLEX_FULL )
        {
            /* 10 Full Duplex */
            xval = xval | GMAC_FD;
        }
        else
        {
            /* 10 Half Duplex */
        }
    }
    MHal_GMAC_WritReg32( GMAC_REG_ETH_CFG, xval );

}

void MHal_GMAC_update_speed_duplex_GMAC0_GPHY( u32 uspeed, u32 uduplex )
{
    u32 xval;
    u8 uRegVal;

    xval = MHal_GMAC_ReadReg32( GMAC_REG_ETH_CFG ) & ~( GMAC_SPD | GMAC_FD );

    if(uspeed == SPEED_1000)
    {
        /* Disable reg_rgmii_slow */
        uRegVal = MHal_GMAC_ReadReg8(0x1224, 0x25);
        uRegVal &= 0xbf;
        MHal_GMAC_WritReg8(0x1224, 0x25, uRegVal);

        /* Set reg_xmii_type as 1G */
        uRegVal = MHal_GMAC_ReadReg8(0x121f, 0x60);
        uRegVal &= 0xfc;
        MHal_GMAC_WritReg8(0x121f, 0x60, uRegVal);

        if ( uduplex == DUPLEX_FULL )
        {
            /* 1000 Full Duplex */
            xval = xval | GMAC_FD;
        }
    }
    else if ( uspeed == SPEED_100 )
    {
    /*For io0
    BK_1224_12[15:14] = 2'11 when 100M
                        2'01 when 10M
    BK_121F_30[1:0] = 2'01 when 10 or 100M
    */

        /* Enable reg_rgmii_slow, Set reg_rgmii10_100 as 100M*/
        uRegVal = MHal_GMAC_ReadReg8(0x1224, 0x25);
        uRegVal |= 0xC0;
        MHal_GMAC_WritReg8(0x1224, 0x25, uRegVal);

        /* Set reg_xmii_type as 10M/100M */
        uRegVal = MHal_GMAC_ReadReg8(0x121f, 0x60);
        uRegVal &= 0xfc;
        uRegVal |= 0x01;
        MHal_GMAC_WritReg8(0x121f, 0x60, uRegVal);

        if ( uduplex == DUPLEX_FULL )
        {
            /* 100 Full Duplex */
            xval = xval | GMAC_SPD | GMAC_FD;
        }
        else
        {
            /* 100 Half Duplex */
            xval = xval | GMAC_SPD;
        }
    }
    else
    {
        /*For io0
        BK_1224_12[15:14] = 2'11 when 100M
                            2'01 when 10M
        BK_121F_30[1:0] = 2'01 when 10 or 100M
        */

        /* Enable reg_rgmii_slow, Enable reg_rgmii10_100 as 10M*/
        uRegVal = MHal_GMAC_ReadReg8(0x1224, 0x25);
        uRegVal &= 0x3F;
        uRegVal |= 0x40;
        MHal_GMAC_WritReg8(0x1224, 0x25, uRegVal);

        /* Set reg_xmii_type as 10M/100M */
        uRegVal = MHal_GMAC_ReadReg8(0x121f, 0x60);
        uRegVal &= 0xfc;
        uRegVal |= 0x01;
        MHal_GMAC_WritReg8(0x121f, 0x60, uRegVal);

        if ( uduplex == DUPLEX_FULL )
        {
            /* 10 Full Duplex */
            xval = xval | GMAC_FD;
        }
        else
        {
            /* 10 Half Duplex */
        }
    }
    MHal_GMAC_WritReg32( GMAC_REG_ETH_CFG, xval );
}

void MHal_GMAC_update_speed_duplex_RMII_EPHY( u32 uspeed, u32 uduplex )
{
    u32 xval;

    xval = MHal_GMAC_ReadReg32( GMAC_REG_ETH_CFG ) & ~( GMAC_SPD | GMAC_FD );
    if ( uspeed == SPEED_100 )
    {

        if ( uduplex == DUPLEX_FULL )
        {
            /* 100 Full Duplex */
            xval = xval | GMAC_SPD | GMAC_FD;
        }
        else
        {
            /* 100 Half Duplex */
            xval = xval | GMAC_SPD;
        }
    }
    else
    {

        if ( uduplex == DUPLEX_FULL )
        {
            /* 10 Full Duplex */
            xval = xval | GMAC_FD;
        }
        else
        {
            /* 10 Half Duplex */
        }
    }
    MHal_GMAC_WritReg32( GMAC_REG_ETH_CFG, xval );
}

void MHal_GMAC_update_speed_duplex( u32 uspeed, u32 uduplex )
{
    MHal_GMAC_This_Ops.update_speed_duplex(uspeed, uduplex);
}

void MHal_GMAC_link_led_on()
{
    u8 uRegVal;


    uRegVal = MHal_GMAC_ReadReg8(0x000F, 0x1E);
    uRegVal &= ~0x01;
    uRegVal |= 0x02;
    MHal_GMAC_WritReg8(0x000F, 0x1E, uRegVal);
}

void MHal_GMAC_link_led_off()
{
    u8 uRegVal;


    uRegVal = MHal_GMAC_ReadReg8(0x000F, 0x1E);
    uRegVal &= ~0x03;
    MHal_GMAC_WritReg8(0x000F, 0x1E, uRegVal);
}

u8 MHal_GMAC_CalcMACHash( u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u8 hashIdx0 = ( m0&0x01 ) ^ ( ( m0&0x40 ) >> 6 ) ^ ( ( m1&0x10 ) >> 4 ) ^ ( ( m2&0x04 ) >> 2 )
                ^ ( m3&0x01 ) ^ ( ( m3&0x40 ) >> 6 ) ^ ( ( m4&0x10 ) >> 4 ) ^ ( ( m5&0x04 ) >> 2 );

    u8 hashIdx1 = ( m0&0x02 ) ^ ( ( m0&0x80 ) >> 6 ) ^ ( ( m1&0x20 ) >> 4 ) ^ ( ( m2&0x08 ) >> 2 )
                ^ ( m3&0x02 ) ^ ( ( m3&0x80 ) >> 6 ) ^ ( ( m4&0x20 ) >> 4 ) ^ ( ( m5&0x08 ) >> 2 );

    u8 hashIdx2 = ( m0&0x04 ) ^ ( ( m1&0x01 ) << 2 ) ^ ( ( m1&0x40 ) >> 4 ) ^ ( ( m2&0x10 ) >> 2 )
                ^ ( m3&0x04 ) ^ ( ( m4&0x01 ) << 2 ) ^ ( ( m4&0x40 ) >> 4 ) ^ ( ( m5&0x10 ) >> 2 );

    u8 hashIdx3 = ( m0&0x08 ) ^ ( ( m1&0x02 ) << 2 ) ^ ( ( m1&0x80 ) >> 4 ) ^ ( ( m2&0x20 ) >> 2 )
                ^ ( m3&0x08 ) ^ ( ( m4&0x02 ) << 2 ) ^ ( ( m4&0x80 ) >> 4 ) ^ ( ( m5&0x20 ) >> 2 );

    u8 hashIdx4 = ( m0&0x10 ) ^ ( ( m1&0x04 ) << 2 ) ^ ( ( m2&0x01 ) << 4 ) ^ ( ( m2&0x40 ) >> 2 )
                ^ ( m3&0x10 ) ^ ( ( m4&0x04 ) << 2 ) ^ ( ( m5&0x01 ) << 4 ) ^ ( ( m5&0x40 ) >> 2 );

    u8 hashIdx5 = ( m0&0x20 ) ^ ( ( m1&0x08 ) << 2 ) ^ ( ( m2&0x02 ) << 4 ) ^ ( ( m2&0x80 ) >> 2 )
                ^ ( m3&0x20 ) ^ ( ( m4&0x08 ) << 2 ) ^ ( ( m5&0x02 ) << 4 ) ^ ( ( m5&0x80 ) >> 2 );

    return( hashIdx0 | hashIdx1 | hashIdx2 | hashIdx3 | hashIdx4 | hashIdx5 );
}

//-------------------------------------------------------------------------------------------------
//Initialize and enable the PHY interrupt when link-state changes
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_enable_phyirq( void )
{
#if 0

#endif
}

//-------------------------------------------------------------------------------------------------
// Disable the PHY interrupt
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_disable_phyirq( void )
{
#if 0

#endif
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------

u32 MHal_GMAC_get_SA1H_addr( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_SA1H );
}

u32 MHal_GMAC_get_SA1L_addr( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_SA1L );
}

u32 MHal_GMAC_get_SA2H_addr( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_SA2H );
}

u32 MHal_GMAC_get_SA2L_addr( void )
{
    return MHal_GMAC_ReadReg32( GMAC_REG_ETH_SA2L );
}

void MHal_GMAC_Write_SA1H( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA1H, xval );
}

void MHal_GMAC_Write_SA1L( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA1L, xval );
}

void MHal_GMAC_Write_SA2H( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA2H, xval );
}

void MHal_GMAC_Write_SA2L( u32 xval )
{
    MHal_GMAC_WritReg32( GMAC_REG_ETH_SA2L, xval );
}

void* MDev_GMAC_memset( void* s, u32 c, unsigned long count )
{
    char* xs = ( char* ) s;

    while ( count-- )
        *xs++ = c;

    return s;
}

//-------------------------------------------------------------------------------------------------
// Check INT Done
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_CheckINTDone( void )
{
    u32 retIntStatus;
    retIntStatus = MHal_GMAC_Read_ISR();
    MHalGMACThisUVE.cntChkINTCounter = ( MHalGMACThisUVE.cntChkINTCounter %
                                     GMAC_MAX_INT_COUNTER );
    MHalGMACThisUVE.cntChkINTCounter ++;
    if ( ( retIntStatus & GMAC_INT_DONE ) ||
         ( MHalGMACThisUVE.cntChkINTCounter == ( GMAC_MAX_INT_COUNTER - 1 ) ) )
    {
        MHalGMACThisUVE.flagISR_INT_DONE = 0x01;
        return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
// MAC cable connection detection
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_CableConnection( u8 gmac_phyaddr )
{
    u32 retValue = 0;
    u32 word_ETH_MAN = 0x00000000;
    u32 word_ETH_CTL = MHal_GMAC_Read_CTL();

    MHal_GMAC_Write_CTL( 0x00000010 | word_ETH_CTL );
    MHalGMACThisUVE.flagISR_INT_DONE = 0x00;
    MHalGMACThisUVE.cntChkINTCounter = 0;
    MHal_GMAC_read_phy(gmac_phyaddr, MII_BMSR, &word_ETH_MAN);

    if ( word_ETH_MAN & BMSR_LSTATUS )
    {
        retValue = 1;
    }
    else
    {
        retValue = 0;
    }
    MHal_GMAC_Write_CTL( word_ETH_CTL );
    return(retValue);
}

//-------------------------------------------------------------------------------------------------
// GMAC Negotiation PHY
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_NegotiationPHY( u8 gmac_phyaddr )
{
    // Set PHY --------------------------------------------------------------
    u32 retValue = 0;
    u32 bmsr;

    // IMPORTANT: Get real duplex by negotiation with peer.
    u32 word_ETH_CTL = MHal_GMAC_Read_CTL();
    MHal_GMAC_Write_CTL( 0x0000001C | word_ETH_CTL );

    MHalGMACThisBCE.duplex = 1;   // Set default as Half-duplex if negotiation fails.
    retValue = 1;

    MHalGMACThisUVE.flagISR_INT_DONE = 0x00;
    MHalGMACThisUVE.cntChkINTCounter = 0;
    MHalGMACThisUVE.cntChkCableConnect = 0;


    MHal_GMAC_read_phy(gmac_phyaddr, MII_BMSR, &bmsr);
    if ( (bmsr & BMSR_100FULL) || (bmsr & BMSR_10FULL) )
    {
       MHalGMACThisBCE.duplex = 2;
       retValue = 2;
    }
    else
    {
        MHalGMACThisBCE.duplex = 1;
        retValue = 1;
    }

    // NOTE: REG_ETH_CFG must be set according to new ThisGMACBCE.duplex.

    MHal_GMAC_Write_CTL( word_ETH_CTL );
    // Set PHY --------------------------------------------------------------
    return(retValue);
}

//-------------------------------------------------------------------------------------------------
// GMAC Hardware register set
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// GMAC Timer set for Receive function
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_timer_callback( u32 value )
{
    u32 uRegVal;
    uRegVal = MHal_GMAC_Read_IER();
    uRegVal |= ( GMAC_INT_RCOM );
    MHal_GMAC_Write_IER( uRegVal );
}
//-------------------------------------------------------------------------------------------------
// GMAC clock on/off
//-------------------------------------------------------------------------------------------------
void MHal_GMAC_Power_On_Clk_GMAC1_EPHY( void )
{
    u8 uRegVal;
    //int clk_handle;


    //printk("internal ephy flow\n");
    /* Set reg_xmii_type as int PHY mode */
    uRegVal = MHal_GMAC_ReadReg8(0x1224, 0x60);
    uRegVal &= 0xfc;
    uRegVal |= 0x02;
    MHal_GMAC_WritReg8(0x1224, 0x60, uRegVal);

    /* Set gmac ahb clock to 172MHZ */
#ifdef CONFIG_MSTAR_CLKM
    clk_handle = get_handle("g_clk_secgamc_ahb");
    set_clk_source(clk_handle, "clk_secgmac_ahb");
#else
    MHal_GMAC_WritReg8(0x1033, 0x64, 0x04);
#endif

    /* Enable gmac tx clock */
#ifdef CONFIG_MSTAR_CLKM
    clk_handle = get_handle("g_clk_secgamc_tx");
    set_clk_source(clk_handle, "clk_secgmac_tx_rgmii");
#else
    MHal_GMAC_WritReg8(0x1224, 0x23, 0x08);
#endif

    /* Enable gmac rx clock */
#ifdef CONFIG_MSTAR_CLKM
    clk_handle = get_handle("g_clk_secgamc_rx");
    set_clk_source(clk_handle, "clk_secgmac_rx_rgmii");
#else
    MHal_GMAC_WritReg8(0x1224, 0x24, 0x08);
#endif
#ifdef CONFIG_MSTAR_CLKM
    clk_handle = get_handle("g_clk_secgamc_rx_ref");
    set_clk_source(clk_handle, "clk_secgmac_rx_ref");
#else
    MHal_GMAC_WritReg8(0x1224, 0x25, 0x00);
#endif

    /* Init ePHY */
    //gain shift
    MHal_GMAC_WritReg8(0x0032, 0xb4, 0x02);

    //det max
    MHal_GMAC_WritReg8(0x0032, 0x4f, 0x02);

    //det min
    MHal_GMAC_WritReg8(0x0032, 0x51, 0x01);

    //snr len (emc noise)
    MHal_GMAC_WritReg8(0x0032, 0x77, 0x18);

    //lpbk_enable set to 0
    MHal_GMAC_WritReg8(0x0031, 0x72, 0xa0);

    MHal_GMAC_WritReg8(0x0032, 0xfc, 0x00);
    MHal_GMAC_WritReg8(0x0032, 0xfd, 0x00);
    MHal_GMAC_WritReg8(0x0033, 0xa1, 0x80);
    MHal_GMAC_WritReg8(0x0032, 0xcc, 0x40);
    MHal_GMAC_WritReg8(0x0032, 0xbb, 0x04);
    MHal_GMAC_WritReg8(0x0033, 0x3a, 0x00);
    MHal_GMAC_WritReg8(0x0033, 0xf1, 0x00);
    MHal_GMAC_WritReg8(0x0033, 0x8a, 0x01);
    MHal_GMAC_WritReg8(0x0032, 0x3b, 0x01);
    MHal_GMAC_WritReg8(0x0032, 0xc4, 0x44);
    MHal_GMAC_WritReg8(0x0033, 0x80, 0x30);

    //100 gat
    MHal_GMAC_WritReg8(0x0033, 0xc5, 0x00);

    //200 gat
    MHal_GMAC_WritReg8(0x0033, 0x30, 0x43);

    //en_100t_phase
    MHal_GMAC_WritReg8(0x0033, 0x39, 0x41);

    //Low power mode
    MHal_GMAC_WritReg8(0x0033, 0xf2, 0xf5);
    MHal_GMAC_WritReg8(0x0033, 0xf3, 0x0d);

    // Prevent packet drop by inverted waveform
    MHal_GMAC_WritReg8(0x0031, 0x79, 0xd0);
    MHal_GMAC_WritReg8(0x0031, 0x77, 0x5a);

    //10T waveform
    MHal_GMAC_WritReg8(0x0033, 0xe8, 0x06);
    MHal_GMAC_WritReg8(0x0031, 0x2b, 0x00);
    MHal_GMAC_WritReg8(0x0033, 0xe8, 0x00);
    MHal_GMAC_WritReg8(0x0031, 0x2b, 0x00);
    MHal_GMAC_WritReg8(0x0033, 0xe8, 0x06);
    MHal_GMAC_WritReg8(0x0031, 0xaa, 0x19);
    MHal_GMAC_WritReg8(0x0031, 0xac, 0x19);
    MHal_GMAC_WritReg8(0x0031, 0xad, 0x19);
    MHal_GMAC_WritReg8(0x0031, 0xae, 0x19);
    MHal_GMAC_WritReg8(0x0031, 0xaf, 0x19);
    MHal_GMAC_WritReg8(0x0033, 0xe8, 0x00);
    MHal_GMAC_WritReg8(0x0031, 0xab, 0x28);
    MHal_GMAC_WritReg8(0x0031, 0xaa, 0x19);

    //Disable eee
    MHal_GMAC_WritReg8(0x0031, 0x2d, 0x7c);

    //speed up timing recovery
    MHal_GMAC_WritReg8(0x0032, 0xf5, 0x02);

    //signal_det_k
    MHal_GMAC_WritReg8(0x0032, 0x0f, 0xc9);

    //snr_h
    MHal_GMAC_WritReg8(0x0032, 0x89, 0x50);
    MHal_GMAC_WritReg8(0x0032, 0x8b, 0x80);
    MHal_GMAC_WritReg8(0x0032, 0x8e, 0x0e);
    MHal_GMAC_WritReg8(0x0032, 0x90, 0x04);

    return;
}

void MHal_GMAC_Power_On_Clk_GMAC1_GPHY( void )
{
    //u8 uRegVal;
    //int clk_handle;

    //GMACPLL setting
    MHal_GMAC_WritReg8(0x100b, 0xc6, 0x00);
    MHal_GMAC_WritReg8(0x110c, 0xd2, 0x14);
    MHal_GMAC_WritReg8(0x110c, 0xc6, 0x00);

    //reg_gt1_mode
    //MHal_GMAC_WritReg8(0x1026, 0x0a, 0x00); //mdio_en=0, gt0
    MHal_GMAC_WritReg8(0x1026, 0x0b, 0x02); //gt1
    MHal_GMAC_WritReg8(0x1026, 0x00, 0x00); //allpadin

    //GMAC mux setting
    MHal_GMAC_WritReg8(0x1224, 0xA2, 0x88);
    MHal_GMAC_WritReg8(0x1224, 0xA4, 0x91);
    MHal_GMAC_WritReg8(0x1224, 0xA5, 0x21);
    MHal_GMAC_WritReg8(0x1224, 0xA6, 0x02);
    MHal_GMAC_WritReg8(0x1224, 0xA7, 0x6c);
    MHal_GMAC_WritReg8(0x1224, 0xA0, 0x00);
    MHal_GMAC_WritReg8(0x1224, 0xA1, 0xa2);
    MHal_GMAC_WritReg8(0x1224, 0xee, 0x08);
    MHal_GMAC_WritReg8(0x1224, 0xef, 0x01);
    MHal_GMAC_WritReg8(0x1224, 0x84, 0x0c);
    MHal_GMAC_WritReg8(0x1224, 0x85, 0x00);

    //MHal_GMAC_WritReg8(0x1224, 0x04, 0x00); //disable gmac and noe

    //clkgen
    MHal_GMAC_WritReg8(0x100a, 0xa1, 0x0c);
    MHal_GMAC_WritReg8(0x140f, 0x22, 0x88);
    MHal_GMAC_WritReg8(0x140f, 0x23, 0x00);
    MHal_GMAC_WritReg8(0x140f, 0x24, 0x80);
    MHal_GMAC_WritReg8(0x140f, 0x25, 0x28);
    MHal_GMAC_WritReg8(0x140f, 0x60, 0x00);

    //Enable GPHY RX delay
    //MHal_GMAC_write_phy(0x0,0x1f,0x0007);
    //MHal_GMAC_write_phy(0x0,0x1e,0x00a4);
    //MHal_GMAC_write_phy(0x0,0x1c,0xad91);
    //MHal_GMAC_write_phy(0x0,0x1f,0x0000);

    return;
}

void MHal_GMAC_Power_On_Clk_GMAC0_GPHY( void )
{
    //GMACPLL setting
    MHal_GMAC_WritReg8(0x100b, 0xc6, 0x00);
    MHal_GMAC_WritReg8(0x110c, 0xd2, 0x14);
    MHal_GMAC_WritReg8(0x110c, 0xc6, 0x00);

    //reg_gt0_mode
    MHal_GMAC_WritReg8(0x1026, 0x0a, 0xc0);
    MHal_GMAC_WritReg8(0x1026, 0x0b, 0x00);
    MHal_GMAC_WritReg8(0x1026, 0x00, 0x00);

    //GMAC mux setting
    MHal_GMAC_WritReg8(0x1224, 0x22, 0x88);
    MHal_GMAC_WritReg8(0x1224, 0x24, 0x91);
    MHal_GMAC_WritReg8(0x1224, 0x25, 0x21);
    MHal_GMAC_WritReg8(0x1224, 0x26, 0x02);
    MHal_GMAC_WritReg8(0x1224, 0x27, 0x6c);
    MHal_GMAC_WritReg8(0x1224, 0x20, 0x00);
    MHal_GMAC_WritReg8(0x1224, 0x21, 0xa2);
    MHal_GMAC_WritReg8(0x1224, 0x6e, 0x08);
    MHal_GMAC_WritReg8(0x1224, 0x6f, 0x01);
    MHal_GMAC_WritReg8(0x1224, 0x04, 0x0c);
    MHal_GMAC_WritReg8(0x1224, 0x05, 0x00);

    //clkgen
    MHal_GMAC_WritReg8(0x100a, 0x03, 0x0c);
    MHal_GMAC_WritReg8(0x121f, 0x22, 0x88);
    MHal_GMAC_WritReg8(0x121f, 0x23, 0x00);
    MHal_GMAC_WritReg8(0x121f, 0x24, 0x80);
    MHal_GMAC_WritReg8(0x121f, 0x25, 0x28);
    MHal_GMAC_WritReg8(0x121f, 0x60, 0x00);

    //Enable GPHY RX delay
    //MHal_GMAC_write_phy(0x0,0x1f,0x0007);
    //MHal_GMAC_write_phy(0x0,0x1e,0x00a4);
    //MHal_GMAC_write_phy(0x0,0x1c,0xad91);
    //MHal_GMAC_write_phy(0x0,0x1f,0x0000);

    return;
}

void MHal_GMAC_Power_On_Clk_GMAC0_EXT_EPHY( void )
{
    //GMACPLL setting
    MHal_GMAC_WritReg8(0x100b, 0xc6, 0x00);
    MHal_GMAC_WritReg8(0x110c, 0xd2, 0x14);
    MHal_GMAC_WritReg8(0x110c, 0xc6, 0x00);

    //reg_gt0_mode
    MHal_GMAC_WritReg8(0x1026, 0x0a, 0xc0); //mdio, gt0
    MHal_GMAC_WritReg8(0x1026, 0x0b, 0x00); //gt1
    MHal_GMAC_WritReg8(0x1026, 0x00, 0x00); //allpadin

    //GMAC mux setting
    MHal_GMAC_WritReg8(0x1224, 0x22, 0x88);
    MHal_GMAC_WritReg8(0x1224, 0x24, 0x11);
    MHal_GMAC_WritReg8(0x1224, 0x25, 0x21);
    MHal_GMAC_WritReg8(0x1224, 0x26, 0x02);
    MHal_GMAC_WritReg8(0x1224, 0x27, 0x6c);
    MHal_GMAC_WritReg8(0x1224, 0x20, 0x00);
    MHal_GMAC_WritReg8(0x1224, 0x21, 0xa2);
    MHal_GMAC_WritReg8(0x1224, 0x6e, 0x08);
    MHal_GMAC_WritReg8(0x1224, 0x6f, 0x01);
    MHal_GMAC_WritReg8(0x1224, 0x04, 0x0c);
    MHal_GMAC_WritReg8(0x1224, 0x05, 0x00);

    //clkgen
    MHal_GMAC_WritReg8(0x100a, 0x03, 0x0c); //reg_ckg_gmac_ahb
    MHal_GMAC_WritReg8(0x121f, 0x22, 0x88);
    MHal_GMAC_WritReg8(0x121f, 0x23, 0x04);
    MHal_GMAC_WritReg8(0x121f, 0x24, 0x04);
    MHal_GMAC_WritReg8(0x121f, 0x25, 0xe8);
    MHal_GMAC_WritReg8(0x121f, 0x26, 0xc2);
    MHal_GMAC_WritReg8(0x121f, 0x60, 0x03);
    MHal_GMAC_WritReg8(0x121c, 0x00, 0x07);
    MHal_GMAC_WritReg8(0x121c, 0x01, 0xf0);

    //Enable GPHY RX delay
    //MHal_GMAC_write_phy(0x0,0x1f,0x0007);
    //MHal_GMAC_write_phy(0x0,0x1e,0x00a4);
    //MHal_GMAC_write_phy(0x0,0x1c,0xad91);
    //MHal_GMAC_write_phy(0x0,0x1f,0x0000);

    return;
}
void MHal_GMAC_Power_On_Clk_GMAC1_EXT_EPHY( void )
{
    //GMACPLL setting
    MHal_GMAC_WritReg8(0x100b, 0xc6, 0x00);
    MHal_GMAC_WritReg8(0x110c, 0xd2, 0x14);
    MHal_GMAC_WritReg8(0x110c, 0xc6, 0x00);

    //reg_gt1_mode
    //MHal_GMAC_WritReg8(0x1026, 0x0a, 0x00); //mdio_en=0, gt0
    MHal_GMAC_WritReg8(0x1026, 0x0b, 0x04); //gt1
    MHal_GMAC_WritReg8(0x1026, 0x00, 0x00); //allpadin

    //GMAC mux setting
    MHal_GMAC_WritReg8(0x1224, 0xa2, 0x88);
    MHal_GMAC_WritReg8(0x1224, 0xa4, 0x11);
    MHal_GMAC_WritReg8(0x1224, 0xa5, 0x21);
    MHal_GMAC_WritReg8(0x1224, 0xa6, 0x02);
    MHal_GMAC_WritReg8(0x1224, 0xa7, 0x6c);
    MHal_GMAC_WritReg8(0x1224, 0xa0, 0x00);
    MHal_GMAC_WritReg8(0x1224, 0xa1, 0xa2);
    MHal_GMAC_WritReg8(0x1224, 0xee, 0x08);
    MHal_GMAC_WritReg8(0x1224, 0xef, 0x01);
    MHal_GMAC_WritReg8(0x1224, 0x84, 0x0c);
    MHal_GMAC_WritReg8(0x1224, 0x85, 0x00);
    
    //MHal_GMAC_WritReg8(0x1224, 0x04, 0x00); //disable gmac, noe and mdio

    //clkgen
    MHal_GMAC_WritReg8(0x100a, 0xa1, 0x0c); //reg_ckg_gmac_ahb
    MHal_GMAC_WritReg8(0x140f, 0x22, 0x88);
    MHal_GMAC_WritReg8(0x140f, 0x23, 0x04);
    MHal_GMAC_WritReg8(0x140f, 0x24, 0x04);
    MHal_GMAC_WritReg8(0x140f, 0x25, 0xe8);
    MHal_GMAC_WritReg8(0x140f, 0x26, 0xc2);
    MHal_GMAC_WritReg8(0x140f, 0x60, 0x03);
    MHal_GMAC_WritReg8(0x140b, 0x00, 0x07);
    MHal_GMAC_WritReg8(0x140b, 0x01, 0xf0);

    return;
}

void MHal_GMAC_Power_On_Clk( void )
{
    MHal_GMAC_This_Ops.power_on();
}

void MHal_GMAC_Power_Off_Clk( void )
{
    //u32 uRegVal;

#ifdef CONFIG_MSTAR_SRAMPD
    /* Close GMAC0 SRAM Power */
    uRegVal = MHal_GMAC_ReadReg8(0x1712, 0x20);
    uRegVal |= 0x80;
    MHal_GMAC_WriteReg8(0x1712, 0x20, uRegVal);

    /* Close GMAC1 SRAM Power */
    uRegVal = MHal_GMAC_ReadReg8(0x1712, 0x21);
    uRegVal |= 0x04;
    MHal_GMAC_WriteReg8(0x1712, 0x21, uRegVal);
#endif
}

//-------------------------------------------------------------------------------------------------
// GMAC Hardware type check
//-------------------------------------------------------------------------------------------------
u32 MHal_GMAC_Hardware_check( u32 padmux_type )
{
    if(padmux_type == GMAC1_EPHY)
    {
        MHal_GMAC_This_Ops.power_on = MHal_GMAC_Power_On_Clk_GMAC1_EPHY;
        MHal_GMAC_This_Ops.update_speed_duplex = MHal_GMAC_update_speed_duplex_GMAC1_EPHY;
        MHal_GMAC_This_Ops.read_phy = MHal_GMAC_read_phy_InternalEPHY;
        MHal_GMAC_This_Ops.write_phy = MHal_GMAC_write_phy_InternalEPHY;
        MHal_GMAC_This_Ops.GMAC_Reg_Addr_Base = GMAC1_REG_ADDR_BASE;
        MHal_GMAC_This_Ops.GMAC_X32_Reg_Addr_Base = GMAC1_XIU32_REG_ADDR_BASE;
        MHal_GMAC_This_Ops.GMAC_IRQ = INT_IRQ_30_SECGMAC_INT+32;
        MHal_GMAC_This_Ops.GPHY_ADDR = 0;
        return GMAC_EPHY;
    }
    else if(padmux_type == GMAC1_GPHY)
    {
        MHal_GMAC_This_Ops.power_on = MHal_GMAC_Power_On_Clk_GMAC1_GPHY;
        MHal_GMAC_This_Ops.update_speed_duplex = MHal_GMAC_update_speed_duplex_GMAC1_GPHY;
        MHal_GMAC_This_Ops.read_phy = MHal_GMAC_read_phy_swgpio;
        MHal_GMAC_This_Ops.write_phy = MHal_GMAC_write_phy_swgpio;
        mdio_bb_init(PAD_SNR0_GPIO1, PAD_SNR0_GPIO0);
        MHal_GMAC_This_Ops.GMAC_Reg_Addr_Base = GMAC1_REG_ADDR_BASE;
        MHal_GMAC_This_Ops.GMAC_X32_Reg_Addr_Base = GMAC1_XIU32_REG_ADDR_BASE;
        MHal_GMAC_This_Ops.GMAC_IRQ = INT_IRQ_30_SECGMAC_INT+32;
        MHal_GMAC_This_Ops.GPHY_ADDR = CONFIG_GMAC_EXT_PHY_ADDR;
        return GMAC_GPHY;
    }
    else if(padmux_type == GMAC0_GPHY)
    {
        MHal_GMAC_This_Ops.power_on = MHal_GMAC_Power_On_Clk_GMAC0_GPHY;
        MHal_GMAC_This_Ops.update_speed_duplex = MHal_GMAC_update_speed_duplex_GMAC0_GPHY;
        MHal_GMAC_This_Ops.read_phy = MHal_GMAC_read_phy_GPHY;
        MHal_GMAC_This_Ops.write_phy = MHal_GMAC_write_phy_GPHY;
        MHal_GMAC_This_Ops.GMAC_Reg_Addr_Base = GMAC0_REG_ADDR_BASE;
        MHal_GMAC_This_Ops.GMAC_X32_Reg_Addr_Base = GMAC0_XIU32_REG_ADDR_BASE;
        MHal_GMAC_This_Ops.GMAC_IRQ = INT_IRQ_09_GMAC_INT+32;
        MHal_GMAC_This_Ops.GPHY_ADDR = CONFIG_GMAC_EXT_PHY_ADDR;
        return GMAC_GPHY;
    }
    else if(padmux_type == GMAC0_RMII_EXT_EPHY)
    {
        MHal_GMAC_This_Ops.power_on = MHal_GMAC_Power_On_Clk_GMAC0_EXT_EPHY;
        MHal_GMAC_This_Ops.update_speed_duplex = MHal_GMAC_update_speed_duplex_RMII_EPHY;
        MHal_GMAC_This_Ops.read_phy = MHal_GMAC_read_phy_GPHY;
        MHal_GMAC_This_Ops.write_phy = MHal_GMAC_write_phy_GPHY;
        MHal_GMAC_This_Ops.GMAC_Reg_Addr_Base = GMAC0_REG_ADDR_BASE;
        MHal_GMAC_This_Ops.GMAC_X32_Reg_Addr_Base = GMAC0_XIU32_REG_ADDR_BASE;
        MHal_GMAC_This_Ops.GMAC_IRQ = INT_IRQ_09_GMAC_INT+32;
        MHal_GMAC_This_Ops.GPHY_ADDR = CONFIG_GMAC_EXT_PHY_ADDR;
        return GMAC_EPHY;
    }
    else if(padmux_type == GMAC1_RMII_EXT_EPHY)
    {
        MHal_GMAC_This_Ops.power_on = MHal_GMAC_Power_On_Clk_GMAC1_EXT_EPHY;
        MHal_GMAC_This_Ops.update_speed_duplex = MHal_GMAC_update_speed_duplex_RMII_EPHY;
        MHal_GMAC_This_Ops.read_phy = MHal_GMAC_read_phy_swgpio;
        MHal_GMAC_This_Ops.write_phy = MHal_GMAC_write_phy_swgpio;
        mdio_bb_init(PAD_SNR0_GPIO1, PAD_SNR0_GPIO0);
        MHal_GMAC_This_Ops.GMAC_Reg_Addr_Base = GMAC1_REG_ADDR_BASE;
        MHal_GMAC_This_Ops.GMAC_X32_Reg_Addr_Base = GMAC1_XIU32_REG_ADDR_BASE;
        MHal_GMAC_This_Ops.GMAC_IRQ = INT_IRQ_30_SECGMAC_INT+32;
        MHal_GMAC_This_Ops.GPHY_ADDR = CONFIG_GMAC_EXT_PHY_ADDR;
        return GMAC_EPHY;
    }
    else
    {
        panic("not support padset");
    }
}

phys_addr_t MHal_GMAC_REG_ADDR_BASE(void)
{
    return MHal_GMAC_This_Ops.GMAC_Reg_Addr_Base;
}

u32 MHal_GMAC_IRQ(void)
{
    return MHal_GMAC_This_Ops.GMAC_IRQ;
}

u8 MHal_GMAC_PHY_ADDR(void)
{
    return MHal_GMAC_This_Ops.GPHY_ADDR;
}
