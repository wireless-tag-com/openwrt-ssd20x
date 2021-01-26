/*
* mhal_emac.c- Sigmastar
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

//#include <linux/mii.h>
//#include <linux/delay.h>
//include <linux/netdevice.h>
//#include <linux/ethtool.h>
//#include <linux/pci.h>
#include <asm/types.h>
#include "mhal_emac.h"

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
struct _MHalBasicConfigEMAC
{
    u8                  connected;          // 0:No, 1:Yes    <== (20070515) Wait for Julian's reply
	u8					speed;				// 10:10Mbps, 100:100Mbps
	// ETH_CTL Register:
	u8                  wes;				// 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
	// ETH_CFG Register:
	u8					duplex;				// 0:Half-duplex, 1:Full-duplex
	u8					cam;                // 0:No CAM, 1:Yes
	u8 					rcv_bcast;      	// 0:No, 1:Yes
	u8					rlf;                // 0:No, 1:Yes receive long frame(1522)
	// MAC Address:
	u8					sa1[6];				// Specific Addr 1 (MAC Address)
	u8					sa2[6];				// Specific Addr 2
	u8					sa3[6];				// Specific Addr 3
	u8					sa4[6];				// Specific Addr 4
};
typedef struct _MHalBasicConfigEMAC MHalBasicConfigEMAC;

struct _MHalUtilityVarsEMAC
{
    u32                 cntChkCableConnect;
    u32                 cntChkINTCounter;
	u32 				readIdxRBQP;		// Reset = 0x00000000
	u32 				rxOneFrameAddr;		// Reset = 0x00000000 (Store the Addr of "ReadONE_RX_Frame")

	u8                  flagISR_INT_DONE;
};
typedef struct _MHalUtilityVarsEMAC MHalUtilityVarsEMAC;

MHalBasicConfigEMAC MHalThisBCE;
MHalUtilityVarsEMAC MHalThisUVE;

typedef volatile unsigned int EMAC_REG;

typedef struct _TITANIA_EMAC {
// Constant: ----------------------------------------------------------------
// Register MAP:
  EMAC_REG   REG_EMAC_CTL_L; 	      //0x00000000 Network Control Register Low  16 bit
  EMAC_REG   REG_EMAC_CTL_H; 	      //0x00000004 Network Control Register High 16 bit
  EMAC_REG   REG_EMAC_CFG_L; 	      //0x00000008 Network Configuration Register Low  16 bit
  EMAC_REG   REG_EMAC_CFG_H; 	      //0x0000000C Network Configuration Register High 16 bit
  EMAC_REG   REG_EMAC_SR_L; 	      //0x00000010 Network Status Register Low  16 bit
  EMAC_REG   REG_EMAC_SR_H; 	      //0x00000014 Network Status Register High 16 bit
  EMAC_REG   REG_EMAC_TAR_L; 	      //0x00000018 Transmit Address Register Low  16 bit
  EMAC_REG   REG_EMAC_TAR_H; 	      //0x0000001C Transmit Address Register High 16 bit
  EMAC_REG   REG_EMAC_TCR_L; 	      //0x00000020 Transmit Control Register Low  16 bit
  EMAC_REG   REG_EMAC_TCR_H; 	      //0x00000024 Transmit Control Register High 16 bit
  EMAC_REG   REG_EMAC_TSR_L; 	      //0x00000028 Transmit Status Register Low  16 bit
  EMAC_REG   REG_EMAC_TSR_H; 	      //0x0000002C Transmit Status Register High 16 bit
  EMAC_REG   REG_EMAC_RBQP_L;         //0x00000030 Receive Buffer Queue Pointer Low  16 bit
  EMAC_REG   REG_EMAC_RBQP_H;         //0x00000034 Receive Buffer Queue Pointer High 16 bit
  EMAC_REG   REG_EMAC_RBCFG_L;        //0x00000038 Receive buffer configuration Low  16 bit
  EMAC_REG   REG_EMAC_RBCFG_H;        //0x0000003C Receive buffer configuration High 16 bit
  EMAC_REG   REG_EMAC_RSR_L; 	      //0x00000040 Receive Status Register Low  16 bit
  EMAC_REG   REG_EMAC_RSR_H; 	      //0x00000044 Receive Status Register High 16 bit
  EMAC_REG   REG_EMAC_ISR_L;          //0x00000048 Interrupt Status Register Low  16 bit
  EMAC_REG   REG_EMAC_ISR_H;          //0x0000004C Interrupt Status Register High 16 bit
  EMAC_REG   REG_EMAC_IER_L;          //0x00000050 Interrupt Enable Register Low  16 bit
  EMAC_REG   REG_EMAC_IER_H;          //0x00000054 Interrupt Enable Register High 16 bit
  EMAC_REG   REG_EMAC_IDR_L; 	      //0x00000058 Interrupt Disable Register Low  16 bit
  EMAC_REG   REG_EMAC_IDR_H; 	      //0x0000005C Interrupt Disable Register High 16 bit
  EMAC_REG   REG_EMAC_IMR_L; 	      //0x00000060 Interrupt Mask Register Low  16 bit
  EMAC_REG   REG_EMAC_IMR_H; 	      //0x00000064 Interrupt Mask Register High 16 bit
  EMAC_REG   REG_EMAC_MAN_L; 	      //0x00000068 PHY Maintenance Register Low  16 bit
  EMAC_REG   REG_EMAC_MAN_H; 	      //0x0000006C PHY Maintenance Register High 16 bit
  EMAC_REG   REG_EMAC_RBRP_L;         //0x00000070 Receive Buffer First full pointer Low  16 bit
  EMAC_REG   REG_EMAC_RBRP_H;         //0x00000074 Receive Buffer First full pointer High 16 bit
  EMAC_REG   REG_EMAC_RBWP_L;         //0x00000078 Receive Buffer Current pointer Low  16 bit
  EMAC_REG   REG_EMAC_RBWP_H;         //0x0000007C Receive Buffer Current pointer High 16 bit
  EMAC_REG   REG_EMAC_FRA_L; 	      //0x00000080 Frames Transmitted OK Register Low  16 bit
  EMAC_REG   REG_EMAC_FRA_H; 	      //0x00000084 Frames Transmitted OK Register High 16 bit
  EMAC_REG   REG_EMAC_SCOL_L;         //0x00000088 Single Collision Frame Register Low  16 bit
  EMAC_REG   REG_EMAC_SCOL_H;         //0x0000008C Single Collision Frame Register High 16 bit
  EMAC_REG   REG_EMAC_MCOL_L;         //0x00000090 Multiple Collision Frame Register Low  16 bit
  EMAC_REG   REG_EMAC_MCOL_H;         //0x00000094 Multiple Collision Frame Register High 16 bit
  EMAC_REG   REG_EMAC_OK_L; 	      //0x00000098 Frames Received OK Register Low  16 bit
  EMAC_REG   REG_EMAC_OK_H; 	      //0x0000009C Frames Received OK Register High 16 bit
  EMAC_REG   REG_EMAC_SEQE_L;         //0x000000A0 Frame Check Sequence Error Register Low  16 bit
  EMAC_REG   REG_EMAC_SEQE_H;         //0x000000A4 Frame Check Sequence Error Register High 16 bit
  EMAC_REG   REG_EMAC_ALE_L; 	      //0x000000A8 Alignment Error Register Low  16 bit
  EMAC_REG   REG_EMAC_ALE_H; 	      //0x000000AC Alignment Error Register High 16 bit
  EMAC_REG   REG_EMAC_DTE_L; 	      //0x000000B0 Deferred Transmission Frame Register Low  16 bit
  EMAC_REG   REG_EMAC_DTE_H; 	      //0x000000B4 Deferred Transmission Frame Register High 16 bit
  EMAC_REG   REG_EMAC_LCOL_L;         //0x000000B8 Late Collision Register Low  16 bit
  EMAC_REG   REG_EMAC_LCOL_H;         //0x000000BC Late Collision Register High 16 bit
  EMAC_REG   REG_EMAC_ECOL_L;         //0x000000C0 Excessive Collision Register Low  16 bit
  EMAC_REG   REG_EMAC_ECOL_H;         //0x000000C4 Excessive Collision Register High 16 bit
  EMAC_REG   REG_EMAC_TUE_L; 	      //0x000000C8 Transmit Underrun Error Register Low  16 bit
  EMAC_REG   REG_EMAC_TUE_H; 	      //0x000000CC Transmit Underrun Error Register High 16 bit
  EMAC_REG   REG_EMAC_CSE_L;          //0x000000D0 Carrier sense errors Register Low  16 bit
  EMAC_REG   REG_EMAC_CSE_H;          //0x000000D4 Carrier sense errors Register High 16 bit
  EMAC_REG   REG_EMAC_RE_L;           //0x000000D8 Receive resource error Low  16 bit
  EMAC_REG   REG_EMAC_RE_H;           //0x000000DC Receive resource error High 16 bit
  EMAC_REG   REG_EMAC_ROVR_L;         //0x000000E0 Received overrun Low  16 bit
  EMAC_REG   REG_EMAC_ROVR_H;         //0x000000E4 Received overrun High 16 bit
  EMAC_REG   REG_EMAC_SE_L;           //0x000000E8 Received symbols error Low  16 bit
  EMAC_REG   REG_EMAC_SE_H;           //0x000000EC Received symbols error High 16 bit
//  EMAC_REG	 REG_EMAC_CDE; 	      //           Code Error Register
  EMAC_REG   REG_EMAC_ELR_L; 	      //0x000000F0 Excessive Length Error Register Low  16 bit
  EMAC_REG   REG_EMAC_ELR_H; 	      //0x000000F4 Excessive Length Error Register High 16 bit

  EMAC_REG   REG_EMAC_RJB_L; 	      //0x000000F8 Receive Jabber Register Low  16 bit
  EMAC_REG   REG_EMAC_RJB_H; 	      //0x000000FC Receive Jabber Register High 16 bit
  EMAC_REG   REG_EMAC_USF_L; 	      //0x00000100 Undersize Frame Register Low  16 bit
  EMAC_REG   REG_EMAC_USF_H; 	      //0x00000104 Undersize Frame Register High 16 bit
  EMAC_REG   REG_EMAC_SQEE_L; 	      //0x00000108 SQE Test Error Register Low  16 bit
  EMAC_REG   REG_EMAC_SQEE_H; 	      //0x0000010C SQE Test Error Register High 16 bit
//  EMAC_REG	 REG_EMAC_DRFC;       //           Discarded RX Frame Register
  EMAC_REG   REG_Reserved1_L; 	      //0x00000110 Low  16 bit
  EMAC_REG   REG_Reserved1_H; 	      //0x00000114 High 16 bit
  EMAC_REG   REG_Reserved2_L; 	      //0x00000118 Low  16 bit
  EMAC_REG   REG_Reserved2_H; 	      //0x0000011C High 16 bit
  EMAC_REG   REG_EMAC_HSH_L; 	      //0x00000120 Hash Address High[63:32] Low  16 bit
  EMAC_REG   REG_EMAC_HSH_H; 	      //0x00000124 Hash Address High[63:32] High 16 bit
  EMAC_REG   REG_EMAC_HSL_L; 	      //0x00000128 Hash Address Low[31:0] Low  16 bit
  EMAC_REG   REG_EMAC_HSL_H; 	      //0x0000012C Hash Address Low[31:0] High 16 bit

  EMAC_REG   REG_EMAC_SA1L_L;         //0x00000130 Specific Address 1 Low, First 4 bytes Low  16 bit
  EMAC_REG   REG_EMAC_SA1L_H;         //0x00000134 Specific Address 1 Low, First 4 bytes High 16 bit
  EMAC_REG   REG_EMAC_SA1H_L;         //0x00000138 Specific Address 1 High, Last 2 bytes Low  16 bit
  EMAC_REG   REG_EMAC_SA1H_H;         //0x0000013C Specific Address 1 High, Last 2 bytes High 16 bit
  EMAC_REG   REG_EMAC_SA2L_L;         //0x00000140 Specific Address 2 Low, First 4 bytes Low  16 bit
  EMAC_REG   REG_EMAC_SA2L_H;         //0x00000144 Specific Address 2 Low, First 4 bytes High 16 bit
  EMAC_REG   REG_EMAC_SA2H_L;         //0x00000148 Specific Address 2 High, Last 2 bytes Low  16 bit
  EMAC_REG   REG_EMAC_SA2H_H;         //0x0000014C Specific Address 2 High, Last 2 bytes High 16 bit
  EMAC_REG   REG_EMAC_SA3L_L;         //0x00000150 Specific Address 3 Low, First 4 bytes Low  16 bit
  EMAC_REG   REG_EMAC_SA3L_H;         //0x00000154 Specific Address 3 Low, First 4 bytes High 16 bit
  EMAC_REG   REG_EMAC_SA3H_L;         //0x00000158 Specific Address 3 High, Last 2 bytes Low  16 bit
  EMAC_REG   REG_EMAC_SA3H_H;         //0x0000015C Specific Address 3 High, Last 2 bytes High 16 bit
  EMAC_REG   REG_EMAC_SA4L_L;         //0x00000160 Specific Address 4 Low, First 4 bytes Low  16 bit
  EMAC_REG   REG_EMAC_SA4L_H;         //0x00000164 Specific Address 4 Low, First 4 bytes High 16 bit
  EMAC_REG   REG_EMAC_SA4H_L;         //0x00000168 Specific Address 4 High, Last 2 bytes Low  16 bit
  EMAC_REG   REG_EMAC_SA4H_H;         //0x0000016C Specific Address 4 High, Last 2 bytes High 16 bit
  EMAC_REG   REG_TAG_TYPE_L; 	      //0x00000170 tag type of the frame Low  16 bit
  EMAC_REG   REG_TAG_TYPE_H; 	      //0x00000174 tag type of the frame High 16 bit
  EMAC_REG   REG_Reserved3[34];       //0x00000178 Low  16 bit
  EMAC_REG   REG_JULIAN_0100_L;       //0x00000200
  EMAC_REG   REG_JULIAN_0100_H;       //0x00000204
  EMAC_REG   REG_JULIAN_0104_L;       //0x00000208
  EMAC_REG   REG_JULIAN_0104_H;       //0x0000020C
  EMAC_REG   REG_JULIAN_0108_L;       //0x00000210
  EMAC_REG   REG_JULIAN_0108_H;       //0x00000214
} TITANIA_EMAC_Str, *TITANIA_EMAC;

#define MHal_MAX_INT_COUNTER    100
#define EMAC_CHECK_CNT 500000

extern eWaveTestCb eWaveCbTable[];

//-------------------------------------------------------------------------------------------------
//  EMAC hardware for Titania
//-------------------------------------------------------------------------------------------------

/*8-bit RIU address*/
u8 MHal_EMAC_ReadReg8( u32 bank, u32 reg )
{
    u8 val;
    u32 address = EMAC_RIU_REG_BASE + bank*0x100*2;
    address = address + (reg << 1) - (reg & 1);

    val = *( ( volatile u8* ) address );
    return val;
}

void MHal_EMAC_WritReg8( u32 bank, u32 reg, u8 val )
{
    u32 address = EMAC_RIU_REG_BASE + bank*0x100*2;
    address = address + (reg << 1) - (reg & 1);

    *( ( volatile u8* ) address ) = val;
}

void MHal_EMAC_WritRam32(u32 uRamAddr, u32 xoffset,u32 xval)
{
	*((u32*)((char*)uRamAddr + xoffset)) = xval;
}

u32 MHal_EMAC_ReadReg32( u32 xoffset )
{
    volatile u32 *ptrs = (u32*)REG_ADDR_BASE;
    volatile u32 val_l = *(ptrs + (xoffset >> 1)) & 0x0000FFFF;
    volatile u32 val_h = (*(ptrs+ (xoffset >> 1) + 1) & 0x0000FFFF) << 0x10;
    return( val_l | val_h );
}

void MHal_EMAC_WritReg32( u32 xoffset, u32 xval )
{
    u32 address = REG_ADDR_BASE + xoffset*2;

    *( ( volatile u32 * ) address ) = ( u32 ) ( xval & 0x0000FFFF );
    *( ( volatile u32 * ) ( address + 4 ) ) = ( u32 ) ( xval >> 0x10 );
}

void MHal_EMAC_Write_SA1_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5)
{
	u32 w0 = (u32)m3 << 24 | m2 << 16 | m1 << 8 | m0;
	u32 w1 = (u32)m5 <<  8 | m4;
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;

	regs->REG_EMAC_SA1L_L= w0&0x0000FFFF;
    regs->REG_EMAC_SA1L_H= w0>>16;
	regs->REG_EMAC_SA1H_L= w1&0x0000FFFF;
    regs->REG_EMAC_SA1H_H= w1>>16;
}

void MHal_EMAC_Write_SA2_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5)
{
	u32 w0 = (u32)m3 << 24 | m2 << 16 | m1 << 8 | m0;
	u32 w1 = (u32)m5 <<  8 | m4;
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;

	regs->REG_EMAC_SA2L_L= w0&0x0000FFFF;
    regs->REG_EMAC_SA2L_H= w0>>16;
	regs->REG_EMAC_SA2H_L= w1&0x0000FFFF;
    regs->REG_EMAC_SA2H_H= w1>>16;
}

void MHal_EMAC_Write_SA3_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5)
{
	u32 w0 = (u32)m3 << 24 | m2 << 16 | m1 << 8 | m0;
	u32 w1 = (u32)m5 <<  8 | m4;
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;

	regs->REG_EMAC_SA3L_L= w0&0x0000FFFF;
    regs->REG_EMAC_SA3L_H= w0>>16;
	regs->REG_EMAC_SA3H_L= w1&0x0000FFFF;
    regs->REG_EMAC_SA3H_H= w1>>16;
}

void MHal_EMAC_Write_SA4_MAC_Address(u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5)
{
	u32 w0 = (u32)m3 << 24 | m2 << 16 | m1 << 8 | m0;
	u32 w1 = (u32)m5 <<  8 | m4;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;

	regs->REG_EMAC_SA4L_L= w0&0x0000FFFF;
    regs->REG_EMAC_SA4L_H= w0>>16;
	regs->REG_EMAC_SA4H_L= w1&0x0000FFFF;
    regs->REG_EMAC_SA4H_H= w1>>16;
}

//-------------------------------------------------------------------------------------------------
//  R/W EMAC register for Titania
//-------------------------------------------------------------------------------------------------

void MHal_EMAC_update_HSH(u8 mc1, u8 mc0)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_HSL_L= mc1&0x0000FFFF;
    regs->REG_EMAC_HSL_H= mc1>>16;
	regs->REG_EMAC_HSH_L= mc0&0x0000FFFF;
    regs->REG_EMAC_HSH_H= mc0>>16;
}

//-------------------------------------------------------------------------------------------------
//  Read control register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CTL(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_CTL_L)&0x0000FFFF)+((regs->REG_EMAC_CTL_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Write Network control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_CTL(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_CTL_L= xval&0x0000FFFF;
    regs->REG_EMAC_CTL_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Read Network configuration register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CFG(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_CFG_L)&0x0000FFFF)+((regs->REG_EMAC_CFG_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Write Network configuration register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_CFG(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_CFG_L= xval&0x0000FFFF;
    regs->REG_EMAC_CFG_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Read RBQP
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RBQP(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_RBQP_L)&0x0000FFFF)+((regs->REG_EMAC_RBQP_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Write RBQP
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_RBQP(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_RBQP_L= xval&0x0000FFFF;
    regs->REG_EMAC_RBQP_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Write Transmit Address register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_TAR(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_TAR_L= xval&0x0000FFFF;
    regs->REG_EMAC_TAR_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Write Transmit Control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_TCR(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_TCR_L= xval&0x0000FFFF;
    regs->REG_EMAC_TCR_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Transmit Status Register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RSR(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_RSR_L)&0x0000FFFF)+((regs->REG_EMAC_RSR_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Transmit Status Register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_TSR(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_TSR_L)&0x0000FFFF)+((regs->REG_EMAC_TSR_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt status register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ISR(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_ISR_L)&0x0000FFFF)+((regs->REG_EMAC_ISR_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt enable register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IER(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_IER_L)&0x0000FFFF)+((regs->REG_EMAC_IER_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Write Interrupt enable register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_IER(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_IER_L= xval&0x0000FFFF;
    regs->REG_EMAC_IER_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt disable register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IDR(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_IDR_L)&0x0000FFFF)+((regs->REG_EMAC_IDR_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Write Interrupt disable register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_IDR(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_IDR_L= xval&0x0000FFFF;
    regs->REG_EMAC_IDR_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt mask register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IMR(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_IMR_L)&0x0000FFFF)+((regs->REG_EMAC_IMR_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Read PHY maintenance register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_MAN(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_MAN_L)&0x0000FFFF)+((regs->REG_EMAC_MAN_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Write PHY maintenance register
//-------------------------------------------------------------------------------------------------
extern unsigned char phy_id;
void MHal_EMAC_Write_MAN(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;

	xval = (xval & 0xF07FFFFF) | (phy_id << 23); // <-@@@ specify PHY ID

	regs->REG_EMAC_MAN_L= xval&0x0000FFFF;
    regs->REG_EMAC_MAN_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Write Receive Buffer Configuration
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_BUFF(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_RBCFG_L= xval&0x0000FFFF;
    regs->REG_EMAC_RBCFG_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Read Receive Buffer Configuration
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_BUFF(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_RBCFG_L)&0x0000FFFF)+((regs->REG_EMAC_RBCFG_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Read Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RDPTR(void)
{
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_RBRP_L)&0x0000FFFF)+((regs->REG_EMAC_RBRP_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Write Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_RDPTR(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_RBRP_L= xval&0x0000FFFF;
    regs->REG_EMAC_RBRP_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Write Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_WRPTR(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_RBWP_L= xval&0x0000FFFF;
    regs->REG_EMAC_RBWP_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Frames transmitted OK
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_FRA(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_FRA);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_FRA_L)&0x0000FFFF)+((regs->REG_EMAC_FRA_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Single collision frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SCOL(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_SCOL);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_SCOL_L)&0x0000FFFF)+((regs->REG_EMAC_SCOL_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Multiple collision frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_MCOL(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_MCOL);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_MCOL_L)&0x0000FFFF)+((regs->REG_EMAC_MCOL_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Frames received OK
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_OK(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_OK);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_OK_L)&0x0000FFFF)+((regs->REG_EMAC_OK_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Frame check sequence errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SEQE(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_SEQE);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_SEQE_L)&0x0000FFFF)+((regs->REG_EMAC_SEQE_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Alignment errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ALE(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_ALE);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_ALE_L)&0x0000FFFF)+((regs->REG_EMAC_ALE_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Late collisions
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_LCOL(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_LCOL);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_LCOL_L)&0x0000FFFF)+((regs->REG_EMAC_LCOL_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Excessive collisions
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ECOL(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_ECOL);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_ECOL_L)&0x0000FFFF)+((regs->REG_EMAC_ECOL_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Transmit under-run errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_TUE(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_TUE);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_TUE_L)&0x0000FFFF)+((regs->REG_EMAC_TUE_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Carrier sense errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CSE(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_CSE);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_CSE_L)&0x0000FFFF)+((regs->REG_EMAC_CSE_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Receive resource error
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RE(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_RE);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_RE_L)&0x0000FFFF)+((regs->REG_EMAC_RE_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Received overrun
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ROVR(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_ROVR);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_ROVR_L)&0x0000FFFF)+((regs->REG_EMAC_ROVR_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Received symbols error
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SE(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_SE);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_SE_L)&0x0000FFFF)+((regs->REG_EMAC_SE_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Excessive length errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ELR(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_ELR);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_ELR_L)&0x0000FFFF)+((regs->REG_EMAC_ELR_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Receive jabbers
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RJB(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_RJB);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_RJB_L)&0x0000FFFF)+((regs->REG_EMAC_RJB_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Undersize frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_USF(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_USF);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_USF_L)&0x0000FFFF)+((regs->REG_EMAC_USF_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  SQE test errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SQEE(void)
{
    //return MHal_EMAC_ReadReg32(REG_ETH_SQEE);
    u32 xval;
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    xval = ((regs->REG_EMAC_SQEE_L)&0x0000FFFF)+((regs->REG_EMAC_SQEE_H)<<0x10);
	return xval;
}

//-------------------------------------------------------------------------------------------------
//  Write Julian 100
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0100(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	//regs->REG_JULIAN_0100= xval&0x0000FFFF;
	regs->REG_JULIAN_0100_L= xval&0x0000FFFF;
    regs->REG_JULIAN_0100_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Read Julian 104
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_JULIAN_0104(void)
{
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	return ((regs->REG_JULIAN_0104_L)&0x0000FFFF)+((regs->REG_JULIAN_0104_H)<<0x10);
}

//-------------------------------------------------------------------------------------------------
//  Write Julian 104
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0104(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_JULIAN_0104_L= xval&0x0000FFFF;
    regs->REG_JULIAN_0104_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  Read Julian 108
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_JULIAN_0108(void)
{
    TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	return ((regs->REG_JULIAN_0108_L)&0x0000FFFF)+((regs->REG_JULIAN_0108_H)<<0x10);
}

//-------------------------------------------------------------------------------------------------
//  Write Julian 108
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0108(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_JULIAN_0108_L= xval&0x0000FFFF;
    regs->REG_JULIAN_0108_H= xval>>16;
}

//-------------------------------------------------------------------------------------------------
//  PHY INTERFACE
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enable the MDIO bit in MAC control register
// When not called from an interrupt-handler, access to the PHY must be
// protected by a spinlock.
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_enable_mdi(void)
{
   TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
   regs->REG_EMAC_CTL_L |= EMAC_MPE; //enable management port //
   regs->REG_EMAC_CTL_H &= EMAC_ALLFF;
}

//-------------------------------------------------------------------------------------------------
//  Disable the MDIO bit in the MAC control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_disable_mdi(void)
{
   TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
   regs->REG_EMAC_CTL_L &= ~EMAC_MPE;    // disable management port //
   regs->REG_EMAC_CTL_H &= EMAC_ALLFF;
}

//-------------------------------------------------------------------------------------------------
// Write value to the a PHY register
// Note: MDI interface is assumed to already have been enabled.
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_write_phy (unsigned char phy_addr, unsigned char address, u32 value)
{
   u32 uRegVal, uCTL = 0;
   TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;

   uRegVal =(EMAC_HIGH | EMAC_CODE_802_3 | EMAC_RW_W)
            | ((phy_addr & 0x1f) << PHY_ADDR_SHIFT) | (address << PHY_REG_SHIFT) | (value & 0xFFFF) ;

    uCTL = MHal_EMAC_Read_CTL();
    MHal_EMAC_enable_mdi();
    MHal_EMAC_Write_MAN(uRegVal);

   // Wait until IDLE bit in Network Status register is cleared //
   uRegVal = ((regs->REG_EMAC_SR_L)&0x0000FFFF)+((regs->REG_EMAC_SR_H)<<0x10);

   while (!(uRegVal& EMAC_IDLE))
   {
       uRegVal = ((regs->REG_EMAC_SR_L)&0x0000FFFF)+((regs->REG_EMAC_SR_H)<<0x10);
       barrier ();
   }

    MHal_EMAC_Write_CTL(uCTL);
}
//-------------------------------------------------------------------------------------------------
// Read value stored in a PHY register.
// Note: MDI interface is assumed to already have been enabled.
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_read_phy(unsigned char phy_addr, unsigned char address,u32 *value)
{
   u32 uRegVal, uCTL = 0;
   TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;

   uRegVal = (EMAC_HIGH | EMAC_CODE_802_3 | EMAC_RW_R)
           | ((phy_addr & 0x1f) << PHY_ADDR_SHIFT) | (address << PHY_REG_SHIFT) | (0) ;

   uCTL = MHal_EMAC_Read_CTL();
   MHal_EMAC_enable_mdi();
   MHal_EMAC_Write_MAN(uRegVal);

   //Wait until IDLE bit in Network Status register is cleared //
   uRegVal = ((regs->REG_EMAC_SR_L)&0x0000FFFF)+((regs->REG_EMAC_SR_H)<<0x10);

   while (!(uRegVal & EMAC_IDLE))
   {
       uRegVal = ((regs->REG_EMAC_SR_L)&0x0000FFFF)+((regs->REG_EMAC_SR_H)<<0x10);
       barrier ();
   }

    *value = (MHal_EMAC_Read_MAN() & 0x0000ffff);
    MHal_EMAC_Write_CTL(uCTL);
}

//-------------------------------------------------------------------------------------------------
// Update MAC speed and H/F duplex
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_update_speed_duplex(u32 uspeed, u32 uduplex)
{
   u32 mac_cfg_L/*, mac_cfg_H*/;
   TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;

   mac_cfg_L = regs->REG_EMAC_CFG_L & ~(EMAC_SPD | EMAC_FD);
  // mac_cfg_H = regs->REG_EMAC_CFG_H & ~(EMAC_SPD | EMAC_FD);

   if (uspeed == SPEED_100)
   {
       if (uduplex == DUPLEX_FULL)    // 100 Full Duplex //
       {
           regs->REG_EMAC_CFG_L = mac_cfg_L | EMAC_SPD | EMAC_FD;
       }
       else                           // 100 Half Duplex ///
       {
		   regs->REG_EMAC_CFG_L = mac_cfg_L | EMAC_SPD;
       }
   }
   else
   {
       if (uduplex == DUPLEX_FULL)    //10 Full Duplex //
       {
           regs->REG_EMAC_CFG_L = mac_cfg_L |EMAC_FD;
       }
       else                           // 10 Half Duplex //
       {
           regs->REG_EMAC_CFG_L = mac_cfg_L;
       }
   }
   regs->REG_EMAC_CFG_H &= EMAC_ALLFF;//Write to CFG
}

//-------------------------------------------------------------------------------------------------
//Initialize and enable the PHY interrupt when link-state changes
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_enable_phyirq (void)
{

}

//-------------------------------------------------------------------------------------------------
// Disable the PHY interrupt
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_disable_phyirq (void)
{

}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------

u32 MHal_EMAC_get_SA1H_addr(void)
{
   TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
   return (regs->REG_EMAC_SA1H_L&0x0000FFFF) + (regs->REG_EMAC_SA1H_H<<16);
}

u32 MHal_EMAC_get_SA1L_addr(void)
{
   TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
   return (regs->REG_EMAC_SA1L_L&0x0000FFFF) + (regs->REG_EMAC_SA1L_H<<16);
}

u32 MHal_EMAC_get_SA2H_addr(void)
{
   TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
   return (regs->REG_EMAC_SA2H_L&0x0000FFFF) + (regs->REG_EMAC_SA2H_H<<16);
}

u32 MHal_EMAC_get_SA2L_addr(void)
{
   TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
   return (regs->REG_EMAC_SA2L_L&0x0000FFFF) + (regs->REG_EMAC_SA2L_H<<16);
}

void MHal_EMAC_Write_SA1H(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_SA1H_L= xval&0x0000FFFF;
    regs->REG_EMAC_SA1H_H= xval>>16;
}

void MHal_EMAC_Write_SA1L(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_SA1L_L= xval&0x0000FFFF;
    regs->REG_EMAC_SA1L_H= xval>>16;
}

void MHal_EMAC_Write_SA2H(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_SA2H_L= xval&0x0000FFFF;
    regs->REG_EMAC_SA2H_H= xval>>16;
}

void MHal_EMAC_Write_SA2L(u32 xval)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
	regs->REG_EMAC_SA2L_L= xval&0x0000FFFF;
    regs->REG_EMAC_SA2L_H= xval>>16;
}

void * MDev_memset(void * s,int c,unsigned long count)
{
	char *xs = (char *) s;

	while (count--)
		*xs++ = c;

	return s;
}

//-------------------------------------------------------------------------------------------------
// Check INT Done
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_CheckINTDone(void)
{
   u32 retIntStatus;
   retIntStatus = MHal_EMAC_Read_ISR();
   MHalThisUVE.cntChkINTCounter = (MHalThisUVE.cntChkINTCounter%MHal_MAX_INT_COUNTER);
   MHalThisUVE.cntChkINTCounter ++;
   if((retIntStatus&EMAC_INT_DONE)||(MHalThisUVE.cntChkINTCounter==(MHal_MAX_INT_COUNTER-1)))
   {
      MHalThisUVE.flagISR_INT_DONE = 0x01;
	  return TRUE;
   }
   return FALSE;
}

void MHal_EMAC_WritCAM_Address(u32 addr,u8 m0,u8 m1,u8 m2,u8 m3,u8 m4,u8 m5)
{
	u32 w0 = (u32)m1 << 24 | m0 << 16;
	u32 w1 = (u32)m5 << 24 | m4 << 16 | m3 << 8 | m2;
	MHal_EMAC_WritReg32(addr    ,w0);
    MHal_EMAC_WritReg32(addr + 4,w1);
}

void MHal_EMAC_ReadCAM_Address(u32 addr,u32 *w0,u32 *w1)
{
	*w0 = MHal_EMAC_ReadReg32(addr);
	*w1 = MHal_EMAC_ReadReg32(addr + 4);
}

extern unsigned char phy_id;
//-------------------------------------------------------------------------------------------------
// MAC cable connection detection
//-------------------------------------------------------------------------------------------------
s32 MHal_EMAC_CableConnection(void)
{
    u32 value, retValue, partner, counter;

    //check status

    counter = 0;
    MHal_EMAC_read_phy(phy_id, PHY_REG_STATUS, &value);
    while(!(value & PHY_AN_DONE))
    {
        //wait 4 secs
        counter++;
        if(counter > 20)
        {
            printf("AN fail");
            return 0;
        }

        mdelay(200);
        MHal_EMAC_read_phy(phy_id, PHY_REG_STATUS, &value);
    }

    counter = 0;
    MHal_EMAC_read_phy(phy_id, PHY_REG_STATUS, &value);
    while(!(value & PHY_LINK_UP))
    {
        //wait 4 secs
        counter++;
        if(counter > 20)
        {
            printf("Link up fail");
            return 0;
        }

        mdelay(200);
        MHal_EMAC_read_phy(phy_id, PHY_REG_STATUS, &value);
    }

    MHal_EMAC_read_phy(phy_id, PHY_REG_LINK_PARTNER, &partner);
    value = (value >> 11) & 0x0000000F;
    partner = (partner>> 5) & 0x0000000F;
    value &= (partner);

    //100MB Full
    if (value & 0x08)
    {
        MHalThisBCE.duplex = 1;
        MHalThisBCE.speed = SPEED_100;
        retValue = 4;
    }
    //100MB Half
    else if (value & 0x04)
    {
        MHalThisBCE.speed = SPEED_100;
        retValue = 3;
    }
    //10MB Full
    else if (value & 0x02)
    {
        MHalThisBCE.duplex = 1;
        retValue = 2;
    }
    else
        retValue = 1;

    return(retValue);
}

void MHal_EMAC_CheckTSR(void)
{
    u32 check;
    u32 tsrval = 0;

    for (check = 0; check < EMAC_CHECK_CNT; check++)
    {
        tsrval = MHal_EMAC_Read_TSR();
        if ((tsrval & EMAC_IDLETSR) || (tsrval & EMAC_BNQ))
            return;
    }

    printf("Err CheckTSR:0x%x\n", tsrval);
}

//-------------------------------------------------------------------------------------------------
// EMAC Negotiation PHY
//-------------------------------------------------------------------------------------------------
s32 MHal_EMAC_NegotiationPHY(void)
{
    s32 retValue;

    // Set default as 10Mb half-duplex if negotiation fails.
    MHalThisBCE.duplex = 0;
    MHalThisBCE.speed = SPEED_10;
    //Auto-Negotiation
    printf("Auto-Negotiation...\n");
    MHal_EMAC_write_phy(phy_id, PHY_REG_BASIC, 0x1000);

    retValue = MHal_EMAC_CableConnection();

    printf("Link Status Speed:%u Full-duplex:%u\n", MHalThisBCE.speed, MHalThisBCE.duplex);
    return(retValue);
}

//-------------------------------------------------------------------------------------------------
// EMAC Hardware register set
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// EMAC Timer set for Receive function
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_timer_callback(unsigned long value)
{
	TITANIA_EMAC regs = (TITANIA_EMAC) REG_ADDR_BASE;
    regs->REG_EMAC_IER_L |= (EMAC_INT_RCOM );
    regs->REG_EMAC_IER_H &= EMAC_ALLFF;
}


void MHal_EMAC_Set_Tx_JULIAN_T(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134);
    value &= 0xff0fffff;
    value |= xval << 20;

    MHal_EMAC_WritReg32(0x134, value);
}

void MHal_EMAC_Set_TEST(u32 xval)
{
    u32 value = 0xffffffff;
    int i=0;

    for(i = 0x100; i< 0x160;i+=4)
    {
        MHal_EMAC_WritReg32(i, value);
    }
}


u32 MHal_EMAC_Get_Tx_FIFO_Threshold(void)
{
    return (MHal_EMAC_ReadReg32(0x134) & 0x00f00000) >> 20;
}

void MHal_EMAC_Set_Rx_FIFO_Enlarge(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134);
    value &= 0xfcffffff;
    value |= xval << 24;

    MHal_EMAC_WritReg32(0x134, value);
}

u32 MHal_EMAC_Get_Rx_FIFO_Enlarge(void)
{
    return (MHal_EMAC_ReadReg32(0x134) & 0x03000000) >> 24;
}

void MHal_EMAC_Set_Miu_Priority(u32 xval)
{
    u32 value;

    value = MHal_EMAC_ReadReg32(0x100);
    value &= 0xfff7ffff;
    value |= xval << 19;

    MHal_EMAC_WritReg32(0x100, value);
}

u32 MHal_EMAC_Get_Miu_Priority(void)
{
    return (MHal_EMAC_ReadReg32(0x100) & 0x00080000) >> 19;
}

void MHal_EMAC_Set_Tx_Hang_Fix_ECO(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134);
    value &= 0xfffbffff;
    value |= xval << 18;

    MHal_EMAC_WritReg32(0x134, value);
}

void MHal_EMAC_Set_MIU_Out_Of_Range_Fix(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134);
    value &= 0xefffffff;
    value |= xval << 28;

    MHal_EMAC_WritReg32(0x134, value);
}

void MHal_EMAC_Set_Rx_Tx_Burst16_Mode(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134);
    value &= 0xdfffffff;
    value |= xval << 29;

    MHal_EMAC_WritReg32(0x134, value);
}

void MHal_EMAC_Set_Tx_Rx_Req_Priority_Switch(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134);
    value &= 0xfff7ffff;
    value |= xval << 19;

    MHal_EMAC_WritReg32(0x134, value);
}

void MHal_EMAC_Set_Rx_Byte_Align_Offset(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134);
    value &= 0xf3ffffff;
    value |= xval << 26;

    MHal_EMAC_WritReg32(0x134, value);
}

//-------------------------------------------------------------------------------------------------
// EMAC Timer set for Receive function
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_WriteProtect(u8 bEnable, u32 u32AddrUB, u32 u32AddrLB)
{
    u32 val = MHal_EMAC_ReadReg32(0x100), addr = 0;;

    val |= (bEnable) ? EMAC_MIU_WP_EN : 0;
    MHal_EMAC_WritReg32(0x100, val);

    //0x11E
    addr = u32AddrUB / MIU_ADDR_UNIT;
    printf("Upper addr:%ux\n", addr);
    MHal_EMAC_WritReg32(0x11E, (addr & 0x0000FFFF));
    MHal_EMAC_WritReg32(0x120, (addr & 0x00FF0000) >> 16);
    //0x122
    addr = u32AddrLB / MIU_ADDR_UNIT;
    printf("Lower addr:%ux\n", addr);
    MHal_EMAC_WritReg32(0x122, (u32AddrLB/MIU_ADDR_UNIT) & 0x0000FFFF);
    MHal_EMAC_WritReg32(0x124, ((u32AddrLB/MIU_ADDR_UNIT) & 0x00FF0000) >> 16);
}

void MHal_EMAC_trim_phy( void )
{
    u8 uRegVal;
    u8 uEfuVal0;
    u8 uEfuVal1;
    u8 uEfuVal2;

    //efuse read
    MHal_EMAC_WritReg8(REG_BANK_EFUSE, 0x4e, 0x25);
    MHal_EMAC_WritReg8(REG_BANK_EFUSE, 0x4f, 0x00);
    MHal_EMAC_WritReg8(REG_BANK_EFUSE, 0x4c, 0x01);
    
    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_EFUSE, 0x4c);

    while((uRegVal & 0x00) != 0)
    {
        uRegVal = MHal_EMAC_ReadReg8(REG_BANK_EFUSE, 0x4c);
    }

    uEfuVal0 = MHal_EMAC_ReadReg8(REG_BANK_EFUSE, 0x84);
    uEfuVal1 = MHal_EMAC_ReadReg8(REG_BANK_EFUSE, 0x85);
    uEfuVal2 = MHal_EMAC_ReadReg8(REG_BANK_EFUSE, 0x86);
    
    //write efuse into phy trim register
    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY2, 0x60);
    uRegVal |= 0x04;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x60, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY2, 0x69);
    uRegVal |= 0x80;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x69, uRegVal);
    
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x68, uEfuVal0);
    
    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY2, 0x69);
    uRegVal &= 0xc0;
    uRegVal |= (uEfuVal1 & 0x3f);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x69, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY2, 0x61);
    uRegVal &= 0xf0;
    uRegVal |= (uEfuVal1 >> 6);
    uRegVal |= (uEfuVal2 & 0x03) << 2 ;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x61, uRegVal);
}

//-------------------------------------------------------------------------------------------------
// EMAC clock on/off
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_ClkGen(void)
{
    u8 uRegVal;

    //Triming PHY setting via efuse value
    //MHal_EMAC_trim_phy();

    //swith RX discriptor format to mode 1
    MHal_EMAC_WritReg8(REG_BANK_EMAC1, 0x3a, 0x00);
    MHal_EMAC_WritReg8(REG_BANK_EMAC1, 0x3b, 0x01);

    //RX shift patch
    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_EMAC1, 0x00);
    uRegVal |= 0x10;
    MHal_EMAC_WritReg8(REG_BANK_EMAC1, 0x00, uRegVal);

    //TX underrun patch
    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_EMAC1, 0x39);
    uRegVal |= 0x01;
    MHal_EMAC_WritReg8(REG_BANK_EMAC1, 0x39, uRegVal);

    //emac_clk gen
#ifdef CONFIG_ETHERNET_ALBANY

    /*  
        wriu    0x103884    0x00        //Set CLK_EMAC_AHB to 123MHz (Enabled)
        wriu    0x113344    0x00        //Set CLK_EMAC_RX to CLK_EMAC_RX_in (25MHz) (Enabled)
        wriu    0x113346    0x00        //Set CLK_EMAC_TX to CLK_EMAC_TX_IN (25MHz) (Enabled)
    */

    MHal_EMAC_WritReg8(REG_BANK_CLKGEN0, 0x84, 0x00);
    MHal_EMAC_WritReg8(REG_BANK_SCGPCTRL, 0x44, 0x00);
    MHal_EMAC_WritReg8(REG_BANK_SCGPCTRL, 0x46, 0x00);


    /* eth_link_sar*/
    //gain shift
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xb4, 0x02);

    //det max
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x4f, 0x02);

    //det min
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x51, 0x01);

    //snr len (emc noise)
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x77, 0x18);

    //lpbk_enable set to 0
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x72, 0xa0);

    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xfc, 0x00);   // Power-on LDO
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xfd, 0x00);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xb7, 0x17);   // Power-on ADC**
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xcb, 0x11);   // Power-on BGAP
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xcc, 0x20);   // Power-on ADCPL
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xcd, 0xd0);   // Power-on ADCPL
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xd4, 0x00);   // Power-on LPF_OP
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xb9, 0x40);   // Power-on LPF
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xbb, 0x05);   // Power-on REF
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x3a, 0x03);   // PD_TX_IDAC, PD_TX_LD = 0
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x3b, 0x00);


    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x3b, 0x01);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0xa1, 0xc0);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x8a, 0x01);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xc4, 0x44);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x80, 0x30);
    
    //100 gat
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0xc5, 0x00);

    //200 gat
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x30, 0x43);

    //en_100t_phase
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0x39, 0x41);   // en_100t_phase;  [6] save2x_tx

    // Prevent packet drop by inverted waveform
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x79, 0xd0);   // prevent packet drop by inverted waveform
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x77, 0x5a);

    //disable eee
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x2d, 0x7c);   // disable eee

    //10T waveform
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0xe8, 0x06); 
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x2b, 0x00);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0xe8, 0x00);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x2b, 0x00);
    
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0xe8, 0x06);   // shadow_ctrl
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0xaa, 0x1c);   // tin17_s2
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0xac, 0x1c);   // tin18_s2
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0xad, 0x1c);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0xae, 0x1c);   // tin19_s2
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0xaf, 0x1c);
    
    MHal_EMAC_WritReg8(REG_BANK_ALBANY2, 0xe8, 0x00);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0xaa, 0x1c);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0xab, 0x28);

    //speed up timing recovery
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0xf5, 0x02);   

    // Signal_det k
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x0f, 0xc9);

    // snr_h
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x89, 0x50);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x8b, 0x80);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x8e, 0x0e);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x90, 0x04);

#else
    //MHal_EMAC_WritReg8(REG_BANK_CLKGEN0, 0xc0, 0x00);
    //MHal_EMAC_WritReg8(REG_BANK_CLKGEN0, 0xc1, 0x04);
    //MHal_EMAC_WritReg8(REG_BANK_CLKGEN0, 0xc2, 0x04);
    //MHal_EMAC_WritReg8(REG_BANK_CLKGEN0, 0xc3, 0x00);
    //MHal_EMAC_WritReg8(REG_BANK_CLKGEN0, 0xc4, 0x00);
    //MHal_EMAC_WritReg8(REG_BANK_CLKGEN0, 0xc5, 0x00);

    MHal_EMAC_WritReg8(REG_BANK_CLKGEN0, 0x84, 0x00);
    MHal_EMAC_WritReg8(REG_BANK_SCGPCTRL, 0x44, 0x04);
    MHal_EMAC_WritReg8(REG_BANK_SCGPCTRL, 0x45, 0x00);
    MHal_EMAC_WritReg8(REG_BANK_SCGPCTRL, 0x46, 0x04);
    MHal_EMAC_WritReg8(REG_BANK_SCGPCTRL, 0x47, 0x00);

#endif

    //chiptop [15] allpad_in
    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_CHIPTOP, 0xa1);
    uRegVal &= 0x7f;
    MHal_EMAC_WritReg8(REG_BANK_CHIPTOP, 0xa1, uRegVal);

    //et_mode = 1
#ifdef CONFIG_ETHERNET_ALBANY
    //uRegVal = MHal_EMAC_ReadReg8(REG_BANK_CHIPTOP, 0xdf);
    //uRegVal &= 0xfe;
    //MHal_EMAC_WritReg8(REG_BANK_CHIPTOP, 0xdf, uRegVal);
#else
    //0x101e_0f[2]
    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_CHIPTOP, 0x1E);
    uRegVal |= 0x4;
    MHal_EMAC_WritReg8(REG_BANK_CHIPTOP, 0x1E, uRegVal);
#endif

    //enable LED //16'0x0e_28[5:4]=01
    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_PMSLEEP, 0x50);
    uRegVal = (uRegVal&~0x30)|0x10;
    MHal_EMAC_WritReg8(REG_BANK_PMSLEEP, 0x50, uRegVal);    
}

void MHal_EMAC_eWave_Table_Init(void)
{
    eWaveCbTable[0] = MHal_EMAC_Ewavetest_100M;
    eWaveCbTable[1] = MHal_EMAC_Ewavetest_10M_LTP;
    eWaveCbTable[2] = MHal_EMAC_Ewavetest_10M_ALLONE;
    eWaveCbTable[3] = MHal_EMAC_Ewavetest_10M_TPIDLE;
}

void MHal_EMAC_Power_On_Clk(void)
{
    MHal_EMAC_ClkGen();
    MHal_EMAC_eWave_Table_Init();
}

void MHal_EMAC_Power_Off_Clk(void)
{

}

void MHal_EMAC_HW_init(void)
{
    MHal_EMAC_Set_Miu_Priority(1);
    MHal_EMAC_Set_Tx_JULIAN_T(4);
    MHal_EMAC_Set_Rx_Tx_Burst16_Mode(1);
    MHal_EMAC_Set_Rx_FIFO_Enlarge(2);
    MHal_EMAC_Set_Tx_Hang_Fix_ECO(1);
    MHal_EMAC_Set_MIU_Out_Of_Range_Fix(1);
    #ifdef RX_BYTE_ALIGN_OFFSET
    MHal_EMAC_Set_Rx_Byte_Align_Offset(2);
    #endif
    MHal_EMAC_WritReg32(0x138, MHal_EMAC_ReadReg32(0x138) | 0x00000001);
    //    MHal_EMAC_Set_Miu_Highway(1);

}

void MHal_EMAC_Ewavetest_100M(void)
{
    u8 uRegVal;

    printf("eWave 100M mode\n");

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x2d);
    uRegVal &= ~(0x38);
    uRegVal |= 0x08;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x2d, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x90);
    uRegVal |= 0x80;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x90);
    uRegVal |= 0x80;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x90);
    uRegVal |= 0x80;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x90);
    uRegVal |= 0x80;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x90);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);
}

void MHal_EMAC_Ewavetest_10M_LTP(void)
{
    u8 uRegVal;

    printf("eWave 10M_LTP mode\n");

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x73);
    uRegVal &= ~(0x08);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x73, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x87);
    uRegVal &= ~(0x04);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x87, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x2d);
    uRegVal &= ~(0x38);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x2d, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0xeb);
    uRegVal &= ~(0x80);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0xeb, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x80);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x80);
    uRegVal |= 0x80;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x80);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);
}

void MHal_EMAC_Ewavetest_10M_ALLONE(void)
{
    u8 uRegVal;

    printf("eWave 10M_ALLONE mode\n");

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x87);
    uRegVal &= ~(0x04);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x87, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY1, 0x3f);
    uRegVal &= ~(0x40);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x3f, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x2d);
    uRegVal &= ~(0x38);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x2d, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x73);
    uRegVal &= ~(0x08);
    uRegVal |= 0x08;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x73, uRegVal);

    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x80, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7e, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7f, 0x1e);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7a, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7b, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7c, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7d, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7d, 0xff);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x80);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x80);
    uRegVal |= 0x80;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x80);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x40);
    uRegVal &= ~(0x80);
    uRegVal |= 0x80;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x40, uRegVal);
}

void MHal_EMAC_Ewavetest_10M_TPIDLE(void)
{
    u8 uRegVal;

    printf("eWave 10M_TPIDLE mode\n");

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x87);
    uRegVal &= ~(0x04);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x87, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY1, 0x3f);
    uRegVal &= ~(0x40);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY1, 0x3f, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x2d);
    uRegVal &= ~(0x38);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x2d, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x73);
    uRegVal &= ~(0x08);
    uRegVal |= 0x08;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x73, uRegVal);

    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x80, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7e, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7f, 0x1e);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7a, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7b, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7c, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7d, 0xff);
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x7d, 0x00);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x80);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x80);
    uRegVal |= 0x80;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x01);
    uRegVal &= ~(0x80);
    uRegVal |= 0x00;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x01, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(REG_BANK_ALBANY0, 0x40);
    uRegVal &= ~(0x80);
    uRegVal |= 0x80;
    MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0x40, uRegVal);
}

void MHal_EMAC_PHY_reset(void)
{
    MHal_EMAC_write_phy(phy_id, PHY_REG_BASIC, 0x1000);
}
