/*
* mhal_emac.c- Sigmastar
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
#include <linux/mii.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/pci.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/version.h>
#include "mhal_emac.h"
#include "mdrv_types.h"
#include "ms_platform.h"
#include "registers.h"
#ifdef CONFIG_MS_PADMUX
#include "mdrv_padmux.h"
#endif

// extern unsigned char phyaddr;

//-------------------------------------------------------------------------------------------------
//  HW configurations
//-------------------------------------------------------------------------------------------------
#define MS_BASE_REG_RIU_PA            0x1F000000
#define RX_DELAY_INT            1

    #define TX_QUEUE_SIZE_NEW   (127)
    #define TX_QUEUE_CNT_SCHE   (2)     // 0 : SW patch with EMAC1_0x24
					// 1 : EMAC0_0x6C
					// 2 : HW latch with EMAC1_0x24
					// 3 : HW backward compatible with "SW patch with EMAC1_0x24"
    #define TXD_CAP             (0)

//-------------------------------------------------------------------------------------------------
//  Constant definition
//-------------------------------------------------------------------------------------------------
#define TX_QUEUE_SIZE           (4 + TX_QUEUE_SIZE_NEW)
#define EMAC_INT_MASK           (0x80000dff)

// #define JULIAN_100_VAL                      (0x0000F011)
#define JULIAN_100_VAL                      (0x00000011)

#if (RX_DELAY_INT)
// #define JULIAN_104_VAL                      (0x01010000UL)
// #define JULIAN_104_VAL                      (0x30400000UL)
#define JULIAN_104_VAL                      (0x10020000UL)
#else
#define JULIAN_104_VAL                      (0x00000000UL)
#endif



//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
#define TXD_SIZE                        sizeof(txd_t)
typedef struct
{
  u32   addr;
  u32   tag;
      #define TXD_LEN_MSK               (0x3FFF)
      #define TXD_WRAP                  (0x1 << 14) /* bit for wrap */
  u32   reserve0;
  u32   reserve1;
} __attribute__ ((packed)) txd_t;

typedef struct
{
    int         (*phy_write)(void*, u8, u32, u32);
    int         (*phy_read)(void*, u8, u32, u32*);
    void        (*phy_clk_on)(void*);
    void        (*phy_clk_off)(void*);
} phy_ops_t;

typedef struct
{
    int         (*txq_size)(void*);
    int         (*txq_free)(void*);
    int         (*txq_used)(void*);
    int         (*txq_empty)(void*);
    int         (*txq_full)(void*);
    int         (*txq_insert)(void*, u32 bus, u32 len);
    int         (*txq_mode)(void*);
} txq_ops_t;


typedef struct
{
    txd_t*              pTXD;
    u32                 TXD_num;
    u32                 TXD_write;
    // u32                 TXD_read;
    // int                 phy_type;
    txq_ops_t           txq_op;
    phy_ops_t           phy_op;
    u32                 emacRIU;
    u32                 emacX32;
    u32                 phyRIU;

    u32                 pad_reg;
    u32                 pad_msk;
    u32                 pad_val;

    u32                 pad_led_reg;
    u32                 pad_led_msk;
    u32                 pad_led_val;

    u32                 phy_mode;

#if RX_DELAY_INT
    u8                  u8RxFrameCnt;
    u8                  u8RxFrameCyc;
#endif
    spinlock_t          lock_irq;
} mhal_emac_t;

#define MHal_MAX_INT_COUNTER    100

//-------------------------------------------------------------------------------------------------
//  local variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  forward declaration
//-------------------------------------------------------------------------------------------------
static void _MHal_EMAC_TXQ_Enable(void*);
static void _MHal_EMAC_TXD_Enable(void*);
int MHal_EMAC_TXD_Dump(void*);

// TXQ
static int _MHal_EMAC_TXQ_Size(void*);
static int _MHal_EMAC_TXQ_Free(void*);
static int _MHal_EMAC_TXQ_Used(void*);
static int _MHal_EMAC_TXQ_Empty(void*);
static int _MHal_EMAC_TXQ_Full(void*);
static int _MHal_EMAC_TXQ_Insert(void*, u32 bus, u32 len);
static int _MHal_EMAC_TXQ_Mode(void*);

// TXD
static int _MHal_EMAC_TXD_Size(void*);
static int _MHal_EMAC_TXD_Free(void*);
static int _MHal_EMAC_TXD_Used(void*);
static int _MHal_EMAC_TXD_Empty(void*);
static int _MHal_EMAC_TXD_Full(void*);
static int _MHal_EMAC_TXD_Insert(void*, u32 bus, u32 len);
static int _MHal_EMAC_TXD_Mode(void*);

// phy operations for albany
static int _MHal_EMAC_albany_write(void* hal, u8 phy_addr, u32 addr, u32 val);
static int _MHal_EMAC_albany_read(void* hal, u8 phy_addr, u32 addr, u32* val);
static void _MHal_EMAC_albany_clk_on(void* hal);
static void _MHal_EMAC_albany_clk_off(void* hal);

// phy operations for external phy 
static int _MHal_EMAC_ext_write(void* hal, u8 phy_addr, u32 addr, u32 val);
static int _MHal_EMAC_ext_read(void* hal, u8 phy_addr, u32 addr, u32* val);
static void _MHal_EMAC_ext_clk_on(void* hal);
static void _MHal_EMAC_ext_clk_off(void* hal);

//-------------------------------------------------------------------------------------------------
//  Local variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  EMAC hardware for Titania
//-------------------------------------------------------------------------------------------------

/*8-bit RIU address*/
u8 MHal_EMAC_ReadReg8(u32 base, u32 bank, u32 reg)
{
    u8 val;
    // u32 address = EMAC_RIU_REG_BASE + bank*0x100*2;
    // u32 address = EMAC_RIU_REG_BASE + (bank << 9);
    u32 address = base + (bank << 9) + (reg << 1) - (reg & 1); 
    val = *((volatile u8*) address);
    return val;
}

void MHal_EMAC_WritReg8(u32 base, u32 bank, u32 reg, u8 val)
{
    // u32 address = EMAC_RIU_REG_BASE + bank*0x100*2;
    // u32 address = EMAC_RIU_REG_BASE + (bank << 9);
    u32 address = base + (bank << 9) + (reg << 1) - (reg & 1); 
    *((volatile u8*) address) = val;
}

#define MHal_EMAC_ReadReg16(base, bank, reg)          *((volatile u16*)((base) + ((bank) << 9) + ((reg) << 1)))
#define MHal_EMAC_WritReg16(base, bank, reg, val)     *((volatile u16*)((base) + ((bank) << 9) + ((reg) << 1))) = (val)

/* Read/WriteReg32 access two continuous 16bit-width register for EMAC0.
EMAC0 bank is 32bit register, that must write low 16bit-width address then high 16bit-width address. */
u32 MHal_EMAC_ReadReg32(void* hal, u32 xoffset)
{
    mhal_emac_t* pHal = (mhal_emac_t*)hal;

    if(xoffset>=REG_EMAC_JULIAN_0100)
    {
        // u32 address = EMAC_RIU_REG_BASE + (REG_BANK_EMAC0<<9) + (xoffset<< 1);
        u32 address = pHal->emacRIU + (xoffset<< 1);
        u32 xoffsetValueL = *( ( volatile u32* ) address ) & 0x0000FFFF;
        u32 xoffsetValueH = *( ( volatile u32* ) ( address + 4) ) << 0x10;
        return( xoffsetValueH | xoffsetValueL );
    }
    else
    {
        // u32 address = EMAC_RIU_REG_BASE + (REG_BANK_X32_EMAC0<<9) + (xoffset<<1);
        u32 address = pHal->emacX32 + (xoffset<<1);
        return *(( volatile u32*)address);
    }
}

void MHal_EMAC_WritReg32(void* hal, u32 xoffset, u32 xval )
{
    mhal_emac_t* pHal = (mhal_emac_t*)hal;

    if(xoffset>=REG_EMAC_JULIAN_0100)
    {
        // u32 address = EMAC_RIU_REG_BASE + (REG_BANK_EMAC0<<9) + (xoffset<< 1);
        u32 address = pHal->emacRIU + (xoffset<< 1);
        *( ( volatile u32 * ) address ) = ( u32 ) ( xval & 0x0000FFFF );
        *( ( volatile u32 * ) ( address + 4 ) ) = ( u32 ) ( xval >> 0x10 );
    }
    else
    {
        // u32 address = EMAC_RIU_REG_BASE + (REG_BANK_X32_EMAC0<<9) + (xoffset<<1);
        u32 address = pHal->emacX32 + (xoffset<<1);
        *( ( volatile u32 * ) address ) = ( u32 ) ( xval );
    }
}

void MHal_EMAC_Write_SA1_MAC_Address(void* hal, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32(hal, REG_ETH_SA1L, w0);
    MHal_EMAC_WritReg32(hal, REG_ETH_SA1H, w1);
}

void MHal_EMAC_Write_SA2_MAC_Address(void* hal, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32(hal, REG_ETH_SA2L, w0 );
    MHal_EMAC_WritReg32(hal, REG_ETH_SA2H, w1 );
}

void MHal_EMAC_Write_SA3_MAC_Address(void* hal, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32(hal, REG_ETH_SA3L, w0 );
    MHal_EMAC_WritReg32(hal, REG_ETH_SA3H, w1 );
}

void MHal_EMAC_Write_SA4_MAC_Address(void* hal, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32(hal, REG_ETH_SA4L, w0 );
    MHal_EMAC_WritReg32(hal, REG_ETH_SA4H, w1 );
}

//-------------------------------------------------------------------------------------------------
//  R/W EMAC register for Titania
//-------------------------------------------------------------------------------------------------

void MHal_EMAC_update_HSH(void* hal, u32 mc0, u32 mc1)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_HSL, mc0 );
    MHal_EMAC_WritReg32(hal, REG_ETH_HSH, mc1 );
}

//-------------------------------------------------------------------------------------------------
//  Read control register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CTL(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_CTL);
}

//-------------------------------------------------------------------------------------------------
//  Write Network control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_CTL(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_CTL, xval);
}

//-------------------------------------------------------------------------------------------------
//  Read Network configuration register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CFG(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_CFG);
}

//-------------------------------------------------------------------------------------------------
//  Write Network configuration register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_CFG(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_CFG, xval);
}

//-------------------------------------------------------------------------------------------------
//  Read RBQP
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RBQP(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_RBQP );
}

//-------------------------------------------------------------------------------------------------
//  Write RBQP
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_RBQP(void* hal, u32 xval )
{
    MHal_EMAC_WritReg32(hal, REG_ETH_RBQP, xval );
}

//-------------------------------------------------------------------------------------------------
//  Write Transmit Address register
//-------------------------------------------------------------------------------------------------
/*
void MHal_EMAC_Write_TAR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_TAR, xval );
}
*/

//-------------------------------------------------------------------------------------------------
//  Read RBQP
//-------------------------------------------------------------------------------------------------
/*
u32 MHal_EMAC_Read_TCR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_TCR);
}

//-------------------------------------------------------------------------------------------------
//  Write Transmit Control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_TCR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_TCR, xval );
}
*/

//-------------------------------------------------------------------------------------------------
//  Transmit Status Register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_TSR(void* hal, u32 xval )
{
    MHal_EMAC_WritReg32(hal, REG_ETH_TSR, xval );
}

u32 MHal_EMAC_Read_TSR(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_TSR );
}

void MHal_EMAC_Write_RSR(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_RSR, xval);
}

#if 0
u32 MHal_EMAC_Read_RSR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_RSR );
}
#endif

//-------------------------------------------------------------------------------------------------
//  Read Interrupt status register
//-------------------------------------------------------------------------------------------------
/*
void MHal_EMAC_Write_ISR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_ISR, xval );
}
*/

u32 MHal_EMAC_IntStatus(void* hal)
{
    //MAC Interrupt Status register indicates what interrupts are pending.
    //It is automatically cleared once read.
    // xoffsetValue = MHal_EMAC_Read_JULIAN_0108() & 0x0000FFFFUL;
    // xReceiveNum += xoffsetValue&0xFFUL;
    // if(xoffsetValue&0x8000UL)
    u32 int_imr = ~MHal_EMAC_ReadReg32(hal, REG_ETH_IMR);
    u32 intstatus = MHal_EMAC_ReadReg32(hal, REG_ETH_ISR);
#if RX_DELAY_INT
    u32 rx_dely_int = (MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0108 ) & 0x8000) ? EMAC_INT_RCOM_DELAY : 0;
    intstatus = (intstatus & int_imr & EMAC_INT_MASK) | rx_dely_int;
#else
    intstatus = (intstatus & int_imr & EMAC_INT_MASK);
#endif
    if (intstatus & EMAC_INT_RBNA)
    {
        intstatus |= (RX_DELAY_INT) ? EMAC_INT_RCOM_DELAY : EMAC_INT_RCOM;
    }
    /*
    if (intstatus & EMAC_INT_ROVR)
    {
        // why?
        MHal_EMAC_WritReg32(REG_ETH_RSR, EMAC_RSROVR);
    }
    if(intstatus & EMAC_INT_TUND)
    {
        //write 1 clear
        MHal_EMAC_WritReg32(REG_ETH_TSR, EMAC_UND);
    }
    */
    return intstatus;
}

void MHal_EMAC_IntEnable(void* hal, u32 intMsk, int bEnable)
{
#if RX_DELAY_INT
    int bRX = (intMsk & (EMAC_INT_RCOM_DELAY | EMAC_INT_RCOM))? 1 : 0;
#endif
    unsigned long flags;
    mhal_emac_t* pHal = (mhal_emac_t*)hal;

    spin_lock_irqsave(&pHal->lock_irq, flags);
#if RX_DELAY_INT
    if (bRX)
        intMsk &= ~(EMAC_INT_RCOM_DELAY | EMAC_INT_RCOM);
    if (bEnable)
    {
        MHal_EMAC_WritReg32(hal, REG_ETH_IER, intMsk);
        if (bRX)
        {
            u32 val = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0104) | 0x00000080UL;
            MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0104, val);
        }
    }
    else
    {
        MHal_EMAC_WritReg32(hal, REG_ETH_IDR, intMsk);
        if (bRX)
        {
            u32 val = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0104) & ~(0x00000080UL);
            MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0104, val);
        }
    }
#else
    if (bEnable)
    {
        MHal_EMAC_WritReg32(hal, REG_ETH_IER, intMsk);
    }
    else
    {
        MHal_EMAC_WritReg32(hal, REG_ETH_IDR, intMsk);
    }
#endif
    spin_unlock_irqrestore(&pHal->lock_irq, flags);
}

#if 1
//-------------------------------------------------------------------------------------------------
//  Read Interrupt enable register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IER(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_IER);
}

//-------------------------------------------------------------------------------------------------
//  Write Interrupt enable register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_IER(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_IER, xval);
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt disable register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IDR(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_IDR);
}

//-------------------------------------------------------------------------------------------------
//  Write Interrupt disable register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_IDR(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_IDR, xval);
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt mask register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IMR(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_IMR );
}
#endif

//-------------------------------------------------------------------------------------------------
//  Read PHY maintenance register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_MAN(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_MAN );
}

//-------------------------------------------------------------------------------------------------
//  Write PHY maintenance register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_MAN(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_MAN, xval);
}

//-------------------------------------------------------------------------------------------------
//  Write Receive Buffer Configuration
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_BUFF(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_BUFF, xval);
}

//-------------------------------------------------------------------------------------------------
//  Read Receive Buffer Configuration
//-------------------------------------------------------------------------------------------------
#if 0
u32 MHal_EMAC_Read_BUFF( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_BUFF );
}
#endif

//-------------------------------------------------------------------------------------------------
//  Read Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
#if 0
u32 MHal_EMAC_Read_RDPTR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_BUFFRDPTR );
}
#endif

#if 0
//-------------------------------------------------------------------------------------------------
//  Write Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_RDPTR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_BUFFRDPTR, xval );
}
#endif

#if 0
//-------------------------------------------------------------------------------------------------
//  Write Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_WRPTR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_BUFFWRPTR, xval );
}
#endif

//-------------------------------------------------------------------------------------------------
//  Frames transmitted OK
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_FRA(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_FRA);
}

//-------------------------------------------------------------------------------------------------
//  Single collision frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SCOL(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_SCOL);
}

//-------------------------------------------------------------------------------------------------
//  Multiple collision frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_MCOL(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_MCOL);
}

//-------------------------------------------------------------------------------------------------
//  Frames received OK
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_OK(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_OK);
}

//-------------------------------------------------------------------------------------------------
//  Frame check sequence errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SEQE(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_SEQE);
}

//-------------------------------------------------------------------------------------------------
//  Alignment errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ALE(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_ALE);
}

//-------------------------------------------------------------------------------------------------
//  Late collisions
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_LCOL(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_LCOL);
}

//-------------------------------------------------------------------------------------------------
//  Excessive collisions
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ECOL(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_ECOL);
}

//-------------------------------------------------------------------------------------------------
//  Transmit under-run errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_TUE(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_TUE);
}

//-------------------------------------------------------------------------------------------------
//  Carrier sense errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CSE(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_CSE);
}

#if 0
//-------------------------------------------------------------------------------------------------
//  Receive resource error
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RE( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_RE );
}
#endif

//-------------------------------------------------------------------------------------------------
//  Received overrun
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ROVR(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_ROVR);
}

//-------------------------------------------------------------------------------------------------
//  Received symbols error
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SE(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_SE);
}

//-------------------------------------------------------------------------------------------------
//  Excessive length errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ELR(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_ELR);
}

//-------------------------------------------------------------------------------------------------
//  Receive jabbers
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RJB(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_RJB);
}

//-------------------------------------------------------------------------------------------------
//  Undersize frames
//-------------------------------------------------------------------------------------------------
#if 0
u32 MHal_EMAC_Read_USF( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_USF );
}
#endif

//-------------------------------------------------------------------------------------------------
//  SQE test errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SQEE(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_SQEE);
}

//-------------------------------------------------------------------------------------------------
//  Read Julian 100
//-------------------------------------------------------------------------------------------------
#if 0
u32 MHal_EMAC_Read_JULIAN_0100(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0100);
}
#endif

//-------------------------------------------------------------------------------------------------
//  Write Julian 100
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0100(void* hal, int bMIU_reset)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    u32 val = JULIAN_100_VAL; //                       (0x00000011)
    u32 val_h = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0100);

    if (!bMIU_reset)
        val |= 0xF000;
    if (!pHal->phyRIU)
        val |= 0x0002;
    val |= 0x0004;
    // val = (val_h & 0xFFFF0000) | val;
    val = (val_h & 0xFFFF00C0) | val;
    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0100, val);
}

#if 0
//-------------------------------------------------------------------------------------------------
//  Read Julian 104
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_JULIAN_0104( void )
{
    return MHal_EMAC_ReadReg32( REG_EMAC_JULIAN_0104 );
}

//-------------------------------------------------------------------------------------------------
//  Write Julian 104
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0104( u32 xval )
{
    MHal_EMAC_WritReg32( REG_EMAC_JULIAN_0104, xval );
}
#endif

//-------------------------------------------------------------------------------------------------
//  Read Julian 108
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_JULIAN_0108(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0108);
}

//-------------------------------------------------------------------------------------------------
//  Write Julian 108
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0108(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0108, xval);
}

void MHal_EMAC_Set_Tx_JULIAN_T(void* hal, u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0134);
    value &= 0xff0fffff;
    value |= xval << 20;

    MHal_EMAC_WritReg32(hal, 0x134, value);
}

#if 0
void MHal_EMAC_Set_TEST(u32 xval)
{
    u32 value = 0xffffffff;
    int i=0;

    for(i = 0x100; i< 0x160;i+=4){
        MHal_EMAC_WritReg32(i, value);
        }

}
#endif

#if 0
u32 MHal_EMAC_Get_Tx_FIFO_Threshold(void)
{
    return (MHal_EMAC_ReadReg32(REG_EMAC_JULIAN_0134) & 0x00f00000) >> 20;
}
#endif

void MHal_EMAC_Set_Rx_FIFO_Enlarge(void* hal, u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0134);
    value &= 0xfcffffff;
    value |= xval << 24;

    MHal_EMAC_WritReg32(hal, 0x134, value);
}

u32 MHal_EMAC_Get_Rx_FIFO_Enlarge(void* hal)
{
    return (MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0134) & 0x03000000) >> 24;
}

void MHal_EMAC_Set_Miu_Priority(void* hal, u32 xval)
{
    u32 value;

    value = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0100);
    value &= 0x0000ffff; // unmask interrupt mask as well
    value |= xval << 19;

    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0100, value);
}

#if 0
u32 MHal_EMAC_Get_Miu_Priority(void)
{
    return (MHal_EMAC_ReadReg32(REG_EMAC_JULIAN_0100) & 0x00080000) >> 19;
}
#endif

void MHal_EMAC_Set_Tx_Hang_Fix_ECO(void* hal, u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0134);
    value &= 0xfffbffff;
    value |= xval << 18;

    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0134, value);
}

void MHal_EMAC_Set_MIU_Out_Of_Range_Fix(void* hal, u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0134);
    value &= 0xefffffff;
    value |= xval << 28;

    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0134, value);
}

void MHal_EMAC_Set_Rx_Tx_Burst16_Mode(void* hal, u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0134);
    value &= 0xdfffffff;
    value |= xval << 29;

    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0134, value);
}

#if 0
void MHal_EMAC_Set_Tx_Rx_Req_Priority_Switch(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(REG_EMAC_JULIAN_0134);
    value &= 0xfff7ffff;
    value |= xval << 19;

    MHal_EMAC_WritReg32(REG_EMAC_JULIAN_0134, value);
}
#endif

void MHal_EMAC_Set_Rx_Byte_Align_Offset(void* hal, u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0134);
    value &= 0xf3ffffff;
    value |= xval << 26;

    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0134, value);
}

#if 0
void MHal_EMAC_Write_Protect(u32 start_addr, u32 length)
{
    u32 value;

    value = MHal_EMAC_ReadReg32(REG_EMAC_JULIAN_011C);
    value &= 0x0000ffff;
    value |= ((start_addr+length) >> 4) << 16;
    MHal_EMAC_WritReg32(REG_EMAC_JULIAN_011C, value);

    value = MHal_EMAC_ReadReg32(REG_EMAC_JULIAN_0120);
    //value &= 0x00000000;
    value |= ((start_addr+length) >> 4) >> 16;
    value |= (start_addr >> 4) << 16;
    MHal_EMAC_WritReg32(REG_EMAC_JULIAN_0120, value);

    value = MHal_EMAC_ReadReg32(REG_EMAC_JULIAN_0124);
    value &= 0xffff0000;
    value |= (start_addr >> 4) >> 16;
    MHal_EMAC_WritReg32(REG_EMAC_JULIAN_0124, value);
}
#endif

void MHal_EMAC_Set_Miu_Highway(void* hal, u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0134);
    value &= 0xbfffffff;
    value |= xval << 30;

    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0134, value);
}

void MHal_EMAC_Set_Pipe_Line_Delay(void* hal, int enable)
{
    u32 j100;   //1511 0x00 bit[5:4], default h01
    u32 j146;   //1511 0x23 bit[15], default h1

    if(enable)
    {
        j100 = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0100);
        j100 |= PIPE_LINE_DELAY;
        MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0100, j100);

        j146 = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0146);
        j146 |= EMAC_PATCH_LOCK;
        MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0146, j146);
    }
    else
    {
        j100 = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0100);
        j100 &= ~PIPE_LINE_DELAY;
        MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0100, j100);

        j146 = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0146);
        j146 &= ~EMAC_PATCH_LOCK;
        MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0146, j146);
    }
}

void MHal_EMAC_HW_init(void* hal)
{
    // mhal_emac_t* pEEng = &hal_emac[0];
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    u32 u32Julian104 = JULIAN_104_VAL;

    MHal_EMAC_Set_Miu_Priority(hal, 1);
    MHal_EMAC_Set_Tx_JULIAN_T(hal, 4);
    MHal_EMAC_Set_Rx_Tx_Burst16_Mode(hal, 1);
    MHal_EMAC_Set_Rx_FIFO_Enlarge(hal, 2);
    MHal_EMAC_Set_Tx_Hang_Fix_ECO(hal, 1);
    MHal_EMAC_Set_MIU_Out_Of_Range_Fix(hal, 1);
    #ifdef RX_BYTE_ALIGN_OFFSET
    MHal_EMAC_Set_Rx_Byte_Align_Offset(hal, 2);
    #endif
    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0138, MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0138) | 0x00000001);
//    MHal_EMAC_Set_Miu_Highway(1);
#if 0
#if (EMAC_FLOW_CONTROL_TX == EMAC_FLOW_CONTROL_TX_HW)
    {
        u8 xval;
        xval = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h4D);
        xval = (xval&(~BIT0)) | BIT0;
        MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h4D, xval);
    }
#endif
#endif

/*
    {
#if (TX_QUEUE_CNT_SCHE == 2)
        u8 xval;
        xval = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h23);
        xval |= TXQUEUE_CNT_LATCH;
        MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h23, xval);
#elif (TX_QUEUE_CNT_SCHE == 3)
        u8 xval;
        xval = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h23);
        xval |= (0x1 << 4);
        MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h23, xval);
#endif
    }
*/

    u32Julian104 |= SOFTWARE_DESCRIPTOR_ENABLE;
#if RX_CHECKSUM
    u32Julian104 |= RX_CHECKSUM_ENABLE;
#endif
#if TX_CHECKSUM
    u32Julian104 |= TX_CHECKSUM_ENABLE;
#endif

    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0104, u32Julian104);

    if (pHal->pTXD)
        _MHal_EMAC_TXD_Enable(hal);
    else
        _MHal_EMAC_TXQ_Enable(hal);
    MHal_EMAC_IntEnable(hal, 0xFFFFFFFF, 0);
    MHal_EMAC_IntStatus(hal);

    MHal_EMAC_Write_JULIAN_0100(hal, 0);
}

void MHal_EMAC_mdio_path(void* hal, int mdio_path)
{
    u32 val = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0100);

    if (mdio_path)
        val |= 0x00000002;
    else
        val &= 0xfffffffd;

    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0100, val);
}

//-------------------------------------------------------------------------------------------------
//  PHY INTERFACE
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enable the MDIO bit in MAC control register
// When not called from an interrupt-handler, access to the PHY must be
// protected by a spinlock.
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_enable_mdi(void* hal)
{
    u32 xval;
    xval = MHal_EMAC_Read_CTL(hal);
    xval |= EMAC_MPE;
    MHal_EMAC_Write_CTL(hal, xval);
}

//-------------------------------------------------------------------------------------------------
//  Disable the MDIO bit in the MAC control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_disable_mdi(void* hal)
{
    u32 xval;
    xval = MHal_EMAC_Read_CTL(hal);
    xval &= ~EMAC_MPE;
    MHal_EMAC_Write_CTL(hal, xval);
}

//-------------------------------------------------------------------------------------------------
// Write value to the a PHY register
// Note: MDI interface is assumed to already have been enabled.
//-------------------------------------------------------------------------------------------------
int MHal_EMAC_write_phy(void* hal, unsigned char phy_addr, u32 address, u32 value )
{
    return ((mhal_emac_t*)hal)->phy_op.phy_write(hal, phy_addr, address, value);
#if 0
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

    if (0 == pHal->phy_type)
    {
        u32 uRegBase = pHal->phyRIU;
        phy_addr =0; // dummy instruction

        // *(volatile unsigned int *)(uRegBase + address*4) = value;
        *(volatile unsigned int *)(uRegBase + (address<< 2)) = value;
        udelay( 1 );
    }
    else
    {
        u32 uRegVal = 0, uCTL = 0;
        uRegVal =  ( EMAC_HIGH | EMAC_CODE_802_3 | EMAC_RW_W) | (( phy_addr & 0x1F ) << PHY_ADDR_OFFSET )
                    | ( address << PHY_REGADDR_OFFSET ) | (value & 0xFFFF);

        uCTL = MHal_EMAC_Read_CTL(hal);
        MHal_EMAC_enable_mdi(hal);

        MHal_EMAC_Write_MAN(hal, uRegVal);
        // Wait until IDLE bit in Network Status register is cleared //
        uRegVal = MHal_EMAC_ReadReg32(hal, REG_ETH_SR);  //Must read Low 16 bit.
        while (!(uRegVal & EMAC_IDLE))
        {
            uRegVal = MHal_EMAC_ReadReg32(hal, REG_ETH_SR);
            barrier();
        }
        MHal_EMAC_Write_CTL(hal, uCTL);
    }
#endif
}

//-------------------------------------------------------------------------------------------------
// Read value stored in a PHY register.
// Note: MDI interface is assumed to already have been enabled.
//-------------------------------------------------------------------------------------------------
int MHal_EMAC_read_phy(void* hal, unsigned char phy_addr, u32 address, u32* value )
{
    return ((mhal_emac_t*)hal)->phy_op.phy_read(hal, phy_addr, address, value);

#if 0
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

    if (0 == pHal->phy_type)
    {
/*
        u32 uRegBase  = INTERNEL_PHY_REG_BASE;
        u32 tempvalue ;

        phy_addr =0;

        tempvalue = *(volatile unsigned int *)(INTERNEL_PHY_REG_BASE + 0x04);
        tempvalue |= 0x0004;
        *(volatile unsigned int *)(INTERNEL_PHY_REG_BASE + 0x04) = tempvalue;
        udelay( 1 );
        *value = *(volatile unsigned int *)(uRegBase + address*4);
*/
        u32 uRegBase = pHal->phyRIU;
        u32 tempvalue ;

        phy_addr =0;

        tempvalue = *(volatile unsigned int *)(uRegBase + 0x04);
        tempvalue |= 0x0004;
        *(volatile unsigned int *)(uRegBase + 0x04) = tempvalue;
        udelay( 1 );
        *value = *(volatile unsigned int *)(uRegBase + (address<<2));
    }
    else
    {
        u32 uRegVal = 0, uCTL = 0;

        uRegVal = (EMAC_HIGH | EMAC_CODE_802_3 | EMAC_RW_R)
                | ((phy_addr & 0x1f) << PHY_ADDR_OFFSET) | (address << PHY_REGADDR_OFFSET) | (0) ;

        uCTL = MHal_EMAC_Read_CTL(hal);
        MHal_EMAC_enable_mdi(hal);
        MHal_EMAC_Write_MAN(hal, uRegVal);

        //Wait until IDLE bit in Network Status register is cleared //
        uRegVal = MHal_EMAC_ReadReg32(hal, REG_ETH_SR);  //Must read Low 16 bit.
        while (!(uRegVal & EMAC_IDLE))
        {
            uRegVal = MHal_EMAC_ReadReg32(hal, REG_ETH_SR);
            barrier();
        }
        *value = (MHal_EMAC_Read_MAN(hal) & 0x0000ffff );
        MHal_EMAC_Write_CTL(hal, uCTL);
    }
#endif
}

/*
#ifdef CONFIG_ETHERNET_ALBANY
void MHal_EMAC_TX_10MB_lookUp_table( void )
{
    // tx 10T link test pulse
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x0f*4) ) = 0x9800;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x10*4) ) = 0x8484;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x11*4) ) = 0x8888;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x12*4) ) = 0x8c8c;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x13*4) ) = 0xC898;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x14*4) ) = 0x0000;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x15*4) ) = 0x1000;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x16*4) ) = ( (*( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x16*4) ) & 0xFF00) | 0x0000 );

    // tx 10T look up table.
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x44*4) ) = 0x3C3C;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x45*4) ) = 0x3C3C;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x46*4) ) = 0x3C30;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x47*4) ) = 0x687C;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x48*4) ) = 0x7834;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x49*4) ) = 0xD494;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4A*4) ) = 0x84A0;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4B*4) ) = 0xE4C8;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4C*4) ) = 0xC8C8;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4D*4) ) = 0xC8E8;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4E*4) ) = 0x3C3C;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4F*4) ) = 0x3C3C;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x50*4) ) = 0x2430;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x51*4) ) = 0x707C;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x52*4) ) = 0x6420;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x53*4) ) = 0xD4A0;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x54*4) ) = 0x8498;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x55*4) ) = 0xD0C8;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x56*4) ) = 0xC8C8;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x57*4) ) = 0xC8C8;
}
#endif
*/

//-------------------------------------------------------------------------------------------------
// Update MAC speed and H/F duplex
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_update_speed_duplex(void* hal, int speed, int duplex )
{
    u32 xval;

    xval = MHal_EMAC_ReadReg32(hal, REG_ETH_CFG ) & ~( EMAC_SPD | EMAC_FD );

    if ( speed == SPEED_100 )
    {
        if ( duplex == DUPLEX_FULL )    // 100 Full Duplex //
        {
            xval = xval | EMAC_SPD | EMAC_FD;
        }
        else                           // 100 Half Duplex ///
        {
            xval = xval | EMAC_SPD;
        }

        MHal_EMAC_Set_Pipe_Line_Delay(hal, 1);
    }
    else
    {
        if ( duplex == DUPLEX_FULL )    //10 Full Duplex //
        {
            xval = xval | EMAC_FD;
        }
        else                           // 10 Half Duplex //
        {
        }

        // PATCH for rx receive 256 byte packet only SPEED_10
        MHal_EMAC_Set_Pipe_Line_Delay(hal, 0);
    }
    MHal_EMAC_WritReg32(hal, REG_ETH_CFG, xval );
}

/*
u8 MHal_EMAC_CalcMACHash(u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
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
*/

// #if (KERNEL_PHY == 0)
//-------------------------------------------------------------------------------------------------
//Initialize and enable the PHY interrupt when link-state changes
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_enable_phyirq(void* hal)
{
#ifdef LAN_ESD_CARRIER_INTERRUPT
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    u8 uRegVal;

    //printk( KERN_ERR "[EMAC] %s\n" , __FUNCTION__);
    uRegVal = MHal_EMAC_ReadReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0x75*2);
    uRegVal |= BIT4;
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0x75*2, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x30*2);
    uRegVal = (uRegVal&~0x00F0) | 0x00A0;
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x30*2, uRegVal);
#else
    hal = hal;
#endif
}
// #endif // #if (KERNEL_PHY == 0)

// #if (KERNEL_PHY == 0)
//-------------------------------------------------------------------------------------------------
// Disable the PHY interrupt
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_disable_phyirq(void* hal)
{
    hal = hal;
#if 0

#endif
}
// #endif // #if (KERNEL_PHY == 0)

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------

u32 MHal_EMAC_get_SA1H_addr(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_SA1H);
}

u32 MHal_EMAC_get_SA1L_addr(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_SA1L);
}

u32 MHal_EMAC_get_SA2H_addr(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_SA2H);
}

u32 MHal_EMAC_get_SA2L_addr(void* hal)
{
    return MHal_EMAC_ReadReg32(hal, REG_ETH_SA2L);
}

void MHal_EMAC_Write_SA1H(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_SA1H, xval);
}

void MHal_EMAC_Write_SA1L(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_SA1L, xval);
}

void MHal_EMAC_Write_SA2H(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_SA2H, xval);
}

void MHal_EMAC_Write_SA2L(void* hal, u32 xval)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_SA2L, xval);
}

#if 0
void* MDev_memset( void* s, u32 c, unsigned long count )
{
    char* xs = ( char* ) s;

    while ( count-- )
        *xs++ = c;

    return s;
}
#endif

//-------------------------------------------------------------------------------------------------
//
// EMAC Hardware register set
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// EMAC Timer set for Receive function
//-------------------------------------------------------------------------------------------------
// #if (KERNEL_PHY == 0)
#if 0
void MHal_EMAC_trim_phy(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    U16 val;

    if( INREG16(BASE_REG_EFUSE_PA+0x0B*4) & BIT4 )
    {
        SETREG16(BASE_REG_RIU_PA+0x3360*2, BIT2);
        SETREG16(BASE_REG_RIU_PA+0x3368*2, BIT15);

        val = (INREG16(BASE_REG_EFUSE_PA+0x0A*4)) & 0x001F; //read bit[4:0]
        OUTREGMSK16((BASE_REG_RIU_PA+ 0x3384*2), val, 0x1F); //overwrite bit[4:0]
        printk("ETH 10T output swing trim=0x%x\n",val);

        val = (INREG16(BASE_REG_EFUSE_PA+0x0A*4) >> 5) & 0x001F; //read bit[9:5]
        OUTREGMSK16((BASE_REG_RIU_PA+ 0x3384*2), (val<<8), 0x1F<<8); //overwrite bit[12:8]
        printk("ETH 100T output swing trim=0x%x\n",val);

        val = (INREG16(BASE_REG_EFUSE_PA+0x0A*4) >> 10) & 0x000F; //read bit[13:10]
        OUTREGMSK16((BASE_REG_RIU_PA+ 0x3360*2), (val<<7), 0xF<<7); //overwrite bit[10:7] //different with I1
        printk("ETH RX input impedance trim=0x%x\n",val);

        val = INREG16(BASE_REG_EFUSE_PA+0x0B*4) & 0x000F; //read bit[3:0]
        OUTREGMSK16((BASE_REG_RIU_PA+ 0x3380*2), val, 0xF); //overwrite bit[3:0]
        printk("ETH TX output impedance trim=0x%x\n",val);
    }
    else
    {
        OUTREGMSK16((BASE_REG_RIU_PA+ 0x3368*2), (0x0<<5), 0x1F<<5); //overwrite bit[9:5]
    }
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x79, 0xd0);   // prevent packet drop by inverted waveform
}
#endif

void MHal_EMAC_phy_trunMax(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

    OUTREGMSK16((BASE_REG_RIU_PA+ 0x3368*2), (0xF<<5), 0x1F<<5); //overwrite bit[9:5]
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x79, 0xf0);   // prevent packet drop by inverted waveform
}
// #endif // #if (KERNEL_PHY == 0)

// clock should be set by clock tree. This function is only for the case of FPGA since clock tree is unavailable
static void _MHal_EMAC_Clk(void* hal, int bOn)
{
    if (bOn)
    {
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_CLKGEN0, 0x84, 0x00);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x44, 0x00);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x45, 0x00);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x46, 0x00);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x47, 0x00);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x66, 0x00);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x67, 0x00);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x68, 0x00);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x69, 0x00);
    }
    else
    {
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_CLKGEN0, 0x84, 0x01);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x44, 0x01);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x45, 0x01);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x46, 0x01);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x47, 0x01);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x66, 0x01);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x67, 0x01);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x68, 0x01);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x69, 0x01);
    }
}

//-------------------------------------------------------------------------------------------------
// EMAC clock on/off
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Power_On_Clk(void* hal, struct device *dev )
{
    u8 uRegVal;
    int num_parents, i;
    struct clk **emac_clks;
    struct clk *clk_parent;
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

    //Triming PHY setting via efuse value
    //MHal_EMAC_trim_phy();

    //swith RX discriptor format to mode 1
    MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, 0x3a, 0x00);
    MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, 0x3b, 0x01);

    //RX shift patch
    uRegVal = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, 0x00);
    uRegVal |= 0x10;
    MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, 0x00, uRegVal);

    //TX underrun patch
    uRegVal = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, 0x39);
    uRegVal |= 0x01;
    MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, 0x39, uRegVal);

    //chiptop [15] allpad_in
    uRegVal = MHal_EMAC_ReadReg8(EMAC_RIU_REG_BASE, REG_BANK_CHIPTOP, 0xa1);
    uRegVal &= 0x7f;
    MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_CHIPTOP, 0xa1, uRegVal);

    //emac_clk gen
    /*
        wriu    0x103884    0x00        //Set CLK_EMAC_AHB to 123MHz (Enabled)
        wriu    0x113344    0x00        //Set CLK_EMAC_RX to CLK_EMAC_RX_in (25MHz) (Enabled)
        wriu    0x113346    0x00        //Set CLK_EMAC_TX to CLK_EMAC_TX_IN (25MHz) (Enabled)
    */
#if CONFIG_OF
    num_parents = of_clk_get_parent_count(dev->of_node);
    if(num_parents > 0)
    {
        emac_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
        if(emac_clks == NULL)
        {
            // printk( "[EMAC]kzalloc failed!\n" );
            return;
        }

        //enable all clk
        for(i = 0; i < num_parents; i++)
        {
            emac_clks[i] = of_clk_get(dev->of_node, i);
            if (IS_ERR(emac_clks[i]))
            {
                // printk( "Fail to get EMAC clk!\n" );
                clk_put(emac_clks[i]);
                kfree(emac_clks);
                return;
            }

            /* Get parent clock */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
            clk_parent = clk_hw_get_parent_by_index(__clk_get_hw(emac_clks[i]), 0)->clk;
#else
            clk_parent = clk_get_parent_by_index(emac_clks[i], 0);
#endif
            if(IS_ERR(clk_parent))
            {
                // printk( "[EMAC]can't get parent clock\n" );
                clk_put(emac_clks[i]);
                kfree(emac_clks);
                return;
            }
            /* Set clock parent */
            clk_set_parent(emac_clks[i], clk_parent);
            clk_prepare_enable(emac_clks[i]);
            clk_put(emac_clks[i]);
        }
        kfree(emac_clks);
    }
    else
    {
        _MHal_EMAC_Clk(hal, 1);
    }
    // workaround for clock gen support no clock selection
    if (PHY_INTERFACE_MODE_RMII == pHal->phy_mode)
    {
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x66, 0x04);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x67, 0x00);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x68, 0x04);
        MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x69, 0x00);
    }

#else
/*
    MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_CLKGEN0, 0x84, 0x00);
    MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x44, 0x00);
    MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x46, 0x00);
*/
    _MHal_EMAC_Clk(hal, 1);
#endif

    pHal->phy_op.phy_clk_on(hal);
}

void MHal_EMAC_Power_Off_Clk(void* hal, struct device *dev )
{
    int num_parents, i;
    struct clk **emac_clks;
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

#if CONFIG_OF
    num_parents = of_clk_get_parent_count(dev->of_node);
    if(num_parents > 0)
    {
        emac_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
        if(emac_clks == NULL)
        {
            // printk( "[EMAC]kzalloc failed!\n" );
            return;
        }

        //disable all clk
        for(i = 0; i < num_parents; i++)
        {
            emac_clks[i] = of_clk_get(dev->of_node, i);
            if (IS_ERR(emac_clks[i]))
            {
                // printk( "Fail to get EMAC clk!\n" );
                kfree(emac_clks);
                return;
            }
            else
            {
                clk_disable_unprepare(emac_clks[i]);
                clk_put(emac_clks[i]);
            }
        }
        kfree(emac_clks);
    }
    else
    {
        _MHal_EMAC_Clk(hal, 0);
    }
#else
/*
    MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_CLKGEN0, 0x84, 0x01);
    MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x44, 0x01);
    MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_SCGPCTRL, 0x46, 0x01);
*/
    _MHal_EMAC_Clk(hal, 0);
#endif

    pHal->phy_op.phy_clk_off(hal);
}

// #if (0 == KERNEL_PHY)
void MHal_EMAC_Set_Reverse_LED(void* hal, u32 xval)
{
    u8 u8Reg;
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

    u8Reg = MHal_EMAC_ReadReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xf7);
    if(xval==1)
    {
        MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xf7, u8Reg|BIT7);
    }
    else if(xval==0)
    {
        MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xf7, u8Reg&~BIT7);
    }
}

u8 MHal_EMAC_Get_Reverse_LED(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    return (MHal_EMAC_ReadReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xf7 )&BIT7)? 1:0;
}
// #endif // #if (0 == KERNEL_PHY)

/*
void MHal_EMAC_Set_TXQUEUE_INT_Level(u8 low, u8 high)
{
     if(high >= low)
     {
         MHal_EMAC_WritReg32( REG_TXQUEUE_INT_LEVEL, low|(high<<8)); // useless and un-verified
     }
}
*/

static void _MHal_EMAC_TXQ_Enable(void* hal)
{
#if (TX_QUEUE_SIZE != 4)
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    u8 xval;
    xval = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h24+1);
    xval = (xval&(~BIT7)) | BIT7;
    MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h24+1, xval);
#else
    hal = hal;
#endif
}

static inline int MHal_EMAC_QueueFree_4(void* hal)
{
    u32 tsrval = 0;
    u8  avlfifo[8] = {0};
    u8  avlfifoidx;
    u8  avlfifoval = 0;

    tsrval = MHal_EMAC_Read_TSR(hal);
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

    if (avlfifoval > 4)
    {
        avlfifoval-=4;
    }
    else
    {
        avlfifoval= 0;
    }
    return avlfifoval;
}

static inline u8 MHal_EMAC_QueueUsed_New(void* hal)
{
#if (TX_QUEUE_CNT_SCHE == 3)
    {
        mhal_emac_t* pHal = (mhal_emac_t*) hal;
        u8 xval;
        xval = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h23);
        xval |= (0x1 << 4);
        MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h23, xval);
    }
#endif

#if ((0 == TX_QUEUE_CNT_SCHE) || (3 == TX_QUEUE_CNT_SCHE))
    {
        /*patch:*/
        int i=0, ertry=0;
        u8 read_val, xval;
        mhal_emac_t* pHal = (mhal_emac_t*) hal;

        xval = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h24) & 0x7F;
        do
        {
            read_val = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h24) & 0x7F;
            if(xval==read_val)
            {
                i++;
            }else
            {
                i=0;
                xval = read_val;
                ertry++;
            }
        }while(i<2);
        return xval;
    }
#endif
#if (2 == TX_QUEUE_CNT_SCHE)
    int xval;
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

    xval = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h23);
    xval |= TXQUEUE_CNT_LATCH;
    MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h23, xval);

    xval = MHal_EMAC_ReadReg8(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h24) & 0x7F;
    return xval;
#endif
    hal = hal;
    // printk("[%s][%d] this should not happened\n", __FUNCTION__, __LINE__);
    return 0;
}

///////////////// TXQ
inline static int _MHal_EMAC_TXQ_Size(void* hal)
{
    hal = hal;
    return TX_QUEUE_SIZE;
}

inline static int _MHal_EMAC_TXQ_Free(void* hal)
{
    int ret;
#if (4 == TX_QUEUE_SIZE)
    ret = MHal_EMAC_QueueFree_4(hal);
#elif (1 == TX_QUEUE_CNT_SCHE)
    ret = TX_QUEUE_SIZE - MHal_EMAC_TXQ_Used(hal);
#else
    #if TX_QUEUE_SIZE_NEW
    ret = MHal_EMAC_QueueFree_4(hal) + TX_QUEUE_SIZE_NEW - MHal_EMAC_QueueUsed_New(hal);
    #else
    ret = MHal_EMAC_QueueFree_4(hal);
    #endif
#endif
    return ret;
}

inline static int _MHal_EMAC_TXQ_Used(void* hal)
{
    int ret;

#if (4 == TX_QUEUE_SIZE)
    ret = TX_QUEUE_SIZE - MHal_EMAC_TXQ_Free(hal);
#elif (1 == TX_QUEUE_CNT_SCHE)
    ret = MHal_EMAC_ReadReg32(hal, REG_TXQUEUE_CNT);
#else
    // ret = TX_QUEUE_SIZE - MHal_EMAC_TXQ_Free(hal);
    /////  ret = 4 - MHal_EMAC_QueueFree_4(hal) + MHal_EMAC_QueueUsed_New(hal);
    ret = 4 + MHal_EMAC_QueueUsed_New(hal) - MHal_EMAC_QueueFree_4(hal);
#endif
    return ret;
}

inline static int _MHal_EMAC_TXQ_Empty(void* hal)
{
    return (0 == _MHal_EMAC_TXQ_Used(hal)) ? 1 : 0;
}

inline static int _MHal_EMAC_TXQ_Full(void* hal)
{
    return (0 == _MHal_EMAC_TXQ_Free(hal)) ? 1 : 0;
}

inline static int _MHal_EMAC_TXQ_Insert(void* hal, u32 bus, u32 len)
{
    MHal_EMAC_WritReg32(hal, REG_ETH_TAR, bus);
    MHal_EMAC_WritReg32(hal, REG_ETH_TCR, len);
    return 1;
}

inline int _MHal_EMAC_TXQ_Mode(void* hal)
{
    hal = hal;
    return 0;
}

///////////////// TXD
static int _MHal_EMAC_TXD_Size(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    return pHal->TXD_num;
    // return hal_emac[0].TXD_num;
}

static int _MHal_EMAC_TXD_Used(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    return MHal_EMAC_ReadReg16(pHal->emacRIU, REG_BANK_EMAC0, REG_TXD_STAT) & TXD_NUM_MASK;
}

static int _MHal_EMAC_TXD_Free(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    return pHal->TXD_num - _MHal_EMAC_TXD_Used(hal);
}

static int _MHal_EMAC_TXD_Empty(void* hal)
{
    return (0 == _MHal_EMAC_TXD_Used(hal)) ? 1 : 0;
}

static int _MHal_EMAC_TXD_Full(void* hal)
{
    return (0 == _MHal_EMAC_TXD_Free(hal)) ? 1 : 0;
}

static int _MHal_EMAC_TXD_Insert(void* hal, u32 bus, u32 len)
{
    txd_t* pTXD = NULL;
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    u32 write = pHal->TXD_write;

    if (write >= pHal->TXD_num)
    {
        // printk("[%s][%d] this should not happen\n", __FUNCTION__, __LINE__);
    }
    pTXD = &(pHal->pTXD[write]);
    pTXD->addr = bus;
    pTXD->tag = len;
    pHal->TXD_write++;
    if (pHal->TXD_write >= pHal->TXD_num)
    {
        pTXD->tag |= TXD_WRAP;
        pHal->TXD_write = 0;
    }
    wmb();
    Chip_Flush_MIU_Pipe();
    // printk("[%s][%d] (bus, len, addr, tag, tag) = (0x%08x, %d, 0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__,
        // bus, len, pTXD[write].addr, pTXD[write].tag);
    MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC0, (REG_TXD_XMIT0 + ((write & 0x1) << 1)), 1); // Bad, but at least streamming
/*
#if 0
    if (write & 0x1)
    {
        MHal_EMAC_WritReg8(REG_BANK_EMAC0, REG_TXD_XMIT1, 1);
    }
    else
    {
        MHal_EMAC_WritReg8(REG_BANK_EMAC0, REG_TXD_XMIT0, 1);
    }
#else
    // if (0 == (MHal_EMAC_ReadReg8(REG_BANK_EMAC0, (REG_TXD_XMIT0 + ((write & 0x1) << 1))) & 0x1))
    MHal_EMAC_WritReg8(REG_BANK_EMAC0, (REG_TXD_XMIT0 + ((write & 0x1) << 1)), 1); // Bad, but at least streamming
    // MHal_EMAC_WritReg8(REG_BANK_EMAC0, (REG_TXD_XMIT0 + ((write & 0x0) << 1)), 1); // Bad, but at least streamming
    // MHal_EMAC_WritReg8(REG_BANK_EMAC0, (REG_TXD_XMIT0 + (((write & 0x3)>>1) << 1)), 2); // Bad, but at least streamming
    // MHal_EMAC_WritReg8(REG_BANK_EMAC0, (REG_TXD_XMIT0 + (((write & 0x3)>>1) << 1)), 1); // Bad, but at least streamming
#endif
*/
    // printk("[%s][%d] TXD used number = %d\n", __FUNCTION__, __LINE__, MHal_EMAC_TXD_Used());
    // if (write != MHal_EMAC_ReadReg16(REG_BANK_EMAC0, REG_TXD_PTR_L))
        // printk("[%s][%d] (write, txd_ptr, read) = (%d, %d, %d)\n", __FUNCTION__, __LINE__, write, MHal_EMAC_ReadReg16(REG_BANK_EMAC0, REG_TXD_PTR_L), hal_emac[0].TXD_read);
    return 1;
}

int _MHal_EMAC_TXD_Mode(void* hal)
{
    hal = hal;
    return 1;
}

#if 0
///////////////// wrapper
int MHal_EMAC_TXQ_Size(void* hal)
{
    return ((mhal_emac_t*)hal)->txq_op.txq_size(hal);
    // return hal_emac[0].txq_op.txq_size();
}

int MHal_EMAC_TXQ_Free(void* hal)
{
    return ((mhal_emac_t*)hal)->txq_op.txq_free(hal);
    // return hal_emac[0].txq_op.txq_free();
}

int MHal_EMAC_TXQ_Used(void* hal)
{
    return ((mhal_emac_t*)hal)->txq_op.txq_used(hal);
    // return hal_emac[0].txq_op.txq_used();
}

int MHal_EMAC_TXQ_Empty(void* hal)
{
    return ((mhal_emac_t*)hal)->txq_op.txq_empty(hal);
    // return hal_emac[0].txq_op.txq_empty();
}

int MHal_EMAC_TXQ_Full(void* hal)
{
    return ((mhal_emac_t*)hal)->txq_op.txq_full(hal);
    // return hal_emac[0].txq_op.txq_full();
}

int MHal_EMAC_TXQ_Insert(void* hal, u32 bus, u32 len)
{
    return ((mhal_emac_t*)hal)->txq_op.txq_insert(hal, bus, len);
    // return hal_emac[0].txq_op.txq_insert(bus, len);
}

int MHal_EMAC_TXQ_Mode(void* hal)
{
    return ((mhal_emac_t*)hal)->txq_op.txq_mode(hal);
    // return hal_emac[0].txq_op.txq_mode();
}
#else
inline int MHal_EMAC_TXQ_Size(void* hal)
{
    return _MHal_EMAC_TXQ_Size(hal);
}

inline int MHal_EMAC_TXQ_Free(void* hal)
{
    return _MHal_EMAC_TXQ_Free(hal);
}

inline int MHal_EMAC_TXQ_Used(void* hal)
{
    return _MHal_EMAC_TXQ_Used(hal);
}

inline int MHal_EMAC_TXQ_Empty(void* hal)
{
    return _MHal_EMAC_TXQ_Empty(hal);
}

inline int MHal_EMAC_TXQ_Full(void* hal)
{
    return _MHal_EMAC_TXQ_Full(hal);
}

inline int MHal_EMAC_TXQ_Insert(void* hal, u32 bus, u32 len)
{
    return _MHal_EMAC_TXQ_Insert(hal, bus, len);
}

inline int MHal_EMAC_TXQ_Mode(void* hal)
{
    return _MHal_EMAC_TXQ_Mode(hal);
}
#endif

/*
int MHal_EMAC_TXQ_Done(void* hal)
{
    u32 val;
    {
        mhal_emac_t* pHal = (mhal_emac_t*) hal;
        u16 xval;

        xval = MHal_EMAC_ReadReg16(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h23);
        xval |= 0x0100;
        MHal_EMAC_WritReg16(pHal->emacRIU, REG_BANK_EMAC1, REG_ETH_EMAC1_h23, xval);
    }
    val = MHal_EMAC_ReadReg32(hal, 0x00000162);
    return val & 0x1FFFFFFF;
}
*/

u32 MHal_EMAC_RX_ParamSet(void* hal, u32 frm_num, u32 frm_cyc)
{
#if (RX_DELAY_INT)
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    u32 j104;

    if (frm_num > 0x30)
        frm_num = 0x30;

    j104 = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0104);
    pHal->u8RxFrameCnt = (u8)((j104 >> 16) & 0xFF);
    pHal->u8RxFrameCyc = (u8)((j104 >> 24) & 0xFF);

    // printk("[%s][%d] frame number = 0x%02x\n", __FUNCTION__, __LINE__, frm_num);
    // upper 16 bits for julian 104
/*
    MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, 0x06, (frm_num & 0xFF));
    if (0xFFFFFFFF != frm_cyc)
        MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, 0x07, (frm_cyc & 0xFF));
*/
    if (0xFFFFFFFF == frm_cyc)
        frm_cyc = pHal->u8RxFrameCyc;
    j104 = (j104 & 0x0000FFFF) | ((frm_cyc & 0xFF) << 24) | ((frm_num & 0xFF) << 16);
    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0104, j104);
#else
    if (frm_num < 0x80)
        MHal_EMAC_WritReg32(hal, REG_ETH_IER, EMAC_INT_RCOM);
    else
        MHal_EMAC_WritReg32(hal, REG_ETH_IDR, EMAC_INT_RCOM);
#endif
    return RX_DELAY_INT;
}

u32 MHal_EMAC_RX_ParamRestore(void* hal)
{
#if (RX_DELAY_INT)
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
/*
    // upper 16 bits for julian 104
    MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, 0x06, (pHal->u8RxFrameCnt & 0xFF));
    MHal_EMAC_WritReg8(pHal->emacRIU, REG_BANK_EMAC1, 0x07, (pHal->u8RxFrameCyc & 0xFF));
*/
    MHal_EMAC_RX_ParamSet(hal, pHal->u8RxFrameCnt, pHal->u8RxFrameCyc);
#else
#endif
    return RX_DELAY_INT;
}

// int MHal_EMAC_TXD_Cfg(u32 emacId, u32 TXD_num)
int MHal_EMAC_TXD_Cfg(void* hal, u32 TXD_num)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

    if (0 == TXD_CAP)
        return 0;
    if (TXD_num & ~TXD_NUM_MASK)
    {
        // printk("[%s][%d] Invalid TXD number %d\n", __FUNCTION__, __LINE__, TXD_num);
        return 0;
    }
    // hal_emac[0].TXD_num = TXD_num;
    pHal->TXD_num = TXD_num;
    return TXD_num * TXD_SIZE;
}

int MHal_EMAC_TXD_Buf(void* hal, void* p, dma_addr_t bus, u32 len)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    // mhal_emac_t* pEEng = &hal_emac[0];
    int ret = 0;

    if (0 == TXD_CAP)
        goto jmp_ptr_set;
    if ((!p) || (!bus))
        goto jmp_ptr_set;
    if (0 == pHal->TXD_num)
        goto jmp_ptr_set;
    if (len/TXD_SIZE != pHal->TXD_num)
        goto jmp_ptr_set;
    memset(p, 0, len);
    wmb();
    Chip_Flush_MIU_Pipe();
    pHal->pTXD = (txd_t*)p;
    pHal->TXD_write = 0;
    MHal_EMAC_WritReg16(pHal->emacRIU, REG_BANK_EMAC0, REG_TXD_CFG, 0);
    MHal_EMAC_WritReg32(hal, REG_TXD_BASE, bus);
    ret = 1;
jmp_ptr_set:
    if (pHal->pTXD)
    {
        pHal->txq_op.txq_size = _MHal_EMAC_TXD_Size;
        pHal->txq_op.txq_free = _MHal_EMAC_TXD_Free;
        pHal->txq_op.txq_used = _MHal_EMAC_TXD_Used;
        pHal->txq_op.txq_empty = _MHal_EMAC_TXD_Empty;
        pHal->txq_op.txq_full = _MHal_EMAC_TXD_Full;
        pHal->txq_op.txq_insert = _MHal_EMAC_TXD_Insert;
        pHal->txq_op.txq_mode = _MHal_EMAC_TXD_Mode;
    }
    else
    {
        pHal->txq_op.txq_size = _MHal_EMAC_TXQ_Size;
        pHal->txq_op.txq_free = _MHal_EMAC_TXQ_Free;
        pHal->txq_op.txq_used = _MHal_EMAC_TXQ_Used;
        pHal->txq_op.txq_empty = _MHal_EMAC_TXQ_Empty;
        pHal->txq_op.txq_full = _MHal_EMAC_TXQ_Full;
        pHal->txq_op.txq_insert = _MHal_EMAC_TXQ_Insert;
        pHal->txq_op.txq_mode = _MHal_EMAC_TXQ_Mode;
    }
    return ret;
}

static void _MHal_EMAC_TXD_Enable(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    MHal_EMAC_WritReg16(pHal->emacRIU, REG_BANK_EMAC0, REG_TXD_CFG, (pHal->TXD_num & TXD_NUM_MASK) | TXD_ENABLE);
}

#if 0
u32 MHal_EMAC_TXD_OVR(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    return (MHal_EMAC_ReadReg16(pHal->emacRIU, REG_BANK_EMAC0, REG_TXD_STAT) & TXD_OVR) ? 1 : 0;
}
#endif

#if 0
int MHal_EMAC_TXD_Dump(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    u32 used = _MHal_EMAC_TXD_Used(hal);
    // int i;
#if 0
    printk("[%s][%d] ******************************\n", __FUNCTION__, __LINE__);
    printk("[%s][%d] (p, num, used, write, read, ptr) = (0x%08x, %3d, %3d, %3d, %3d, %3d)\n", __FUNCTION__, __LINE__, 
        (int)pHal->pTXD, pHal->TXD_num, used,
        pHal->TXD_write, pHal->TXD_read, MHal_EMAC_ReadReg16(REG_BANK_EMAC0, REG_TXD_PTR_L));
#else
    printk("[%s][%d] (p, num, used, write, ptr) = (0x%08x, %3d, %3d, %3d, %3d)\n", __FUNCTION__, __LINE__, 
        (int)pHal->pTXD, pHal->TXD_num, used,
        pHal->TXD_write, MHal_EMAC_ReadReg16(pHal->emacRIU, REG_BANK_EMAC0, REG_TXD_PTR_L));
#endif

#if 0
    if (NULL == pHal->pTXD)
        return 1;
    for (i = 0; i < pHal->TXD_num; i++)
    {
        txd_t* pTXD = &(pHal->pTXD[i]);
        printk("[%d] (addr, tag, reserved0, reserved1) = (0x%08x, 0x%08x, 0x%08x, 0x%08x)\n", i, pTXD->addr, pTXD->tag, pTXD->reserve0, pTXD->reserve1);
    }
#endif
    return 0;
}
#endif

void* MHal_EMAC_Alloc(u32 riu, u32 x32, u32 riu_phy)
{
    mhal_emac_t* pHal = kzalloc(sizeof(mhal_emac_t), GFP_KERNEL);

    if (0 == pHal)
    {
        printk("[%s][%d] allocate emac hal handle fail\n", __FUNCTION__, __LINE__);
        return NULL;
    }
#if 0
    pHal->emacRIU = GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), riu);
    pHal->emacX32 = GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), x32);
    pHal->phyRIU = GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), riu_phy);
#else
    pHal->emacRIU = riu;
    pHal->emacX32 = x32;
    pHal->phyRIU = riu_phy;
#endif
    pHal->pTXD = NULL;
    pHal->TXD_num = 0;
    pHal->TXD_write = 0;
    // pHal->phy_type = 0;
    pHal->txq_op.txq_size = _MHal_EMAC_TXQ_Size;
    pHal->txq_op.txq_free = _MHal_EMAC_TXQ_Free;
    pHal->txq_op.txq_used = _MHal_EMAC_TXQ_Used;
    pHal->txq_op.txq_empty = _MHal_EMAC_TXQ_Empty;
    pHal->txq_op.txq_full = _MHal_EMAC_TXQ_Full;
    pHal->txq_op.txq_insert = _MHal_EMAC_TXQ_Insert;
    pHal->txq_op.txq_mode = _MHal_EMAC_TXQ_Mode;

    if (pHal->phyRIU)
    {
#if 0
        // internal
        printk("[%s][%d] ***********************************************\n", __FUNCTION__, __LINE__);
        printk("[%s][%d] use internal phy\n", __FUNCTION__, __LINE__);
        printk("[%s][%d] ***********************************************\n", __FUNCTION__, __LINE__);
#endif
        pHal->phy_op.phy_write = _MHal_EMAC_albany_write;
        pHal->phy_op.phy_read = _MHal_EMAC_albany_read;
        pHal->phy_op.phy_clk_on = _MHal_EMAC_albany_clk_on;
        pHal->phy_op.phy_clk_off = _MHal_EMAC_albany_clk_off;
    }
    else
    {
#if 0
        // external
        printk("[%s][%d] ***********************************************\n", __FUNCTION__, __LINE__);
        printk("[%s][%d] use external phy\n", __FUNCTION__, __LINE__);
        printk("[%s][%d] ***********************************************\n", __FUNCTION__, __LINE__);
#endif
        pHal->phy_op.phy_write = _MHal_EMAC_ext_write;
        pHal->phy_op.phy_read = _MHal_EMAC_ext_read;
        pHal->phy_op.phy_clk_on = _MHal_EMAC_ext_clk_on;
        pHal->phy_op.phy_clk_off = _MHal_EMAC_ext_clk_off;
    }
#if (RX_DELAY_INT)
    pHal->u8RxFrameCnt = (JULIAN_104_VAL >> 16) & 0xFF;
    pHal->u8RxFrameCyc = (JULIAN_104_VAL >> 24) & 0xFF;
#endif

    // MHal_EMAC_Write_JULIAN_0100((void*)pHal, 0);
    spin_lock_init (&pHal->lock_irq);
    return (void*)pHal;
}

void MHal_EMAC_Free(void* hal)
{
    if (NULL == hal)
    {
        // printk("[%s][%d] Try to free NULL emac hal handle\n", __FUNCTION__, __LINE__);
        return;
    }
    kfree(hal);
}

#define INTERNAL_MDIO_ADDR      0

static int _MHal_EMAC_albany_write(void* hal, u8 phy_addr, u32 addr, u32 val)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    u32 uRegBase = pHal->phyRIU;

/*
    if (0 != phy_addr)
        return -1;
*/
    if (INTERNAL_MDIO_ADDR != phy_addr)
        return -1;

    *(volatile unsigned int *)(uRegBase + (addr << 2)) = val;
    udelay(1);
    return 0;
}

static int _MHal_EMAC_albany_read(void* hal, u8 phy_addr, u32 addr, u32* val)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    u32 uRegBase = pHal->phyRIU;
    u32 tempvalue ;

    *val = 0xffff;
/*
    if (0 != phy_addr)
        return 0xffff;
*/
    if (INTERNAL_MDIO_ADDR != phy_addr)
        return 0xffff;

    if (MII_PHYSID1 == addr)
    {
        *val = 0x1111;
        return 0x1111;
    }
    if (MII_PHYSID2 == addr)
    {
        *val = 0x2222;
        return 0x2222;
    }

    tempvalue = *(volatile unsigned int *)(uRegBase + 0x04);
    tempvalue |= 0x0004;
    *(volatile unsigned int *)(uRegBase + 0x04) = tempvalue;
    udelay( 1 );
    *val = *(volatile unsigned int *)(uRegBase + (addr <<2));
    // printk("[%s][%d] val = 0x%08x\n", __FUNCTION__, __LINE__, *val);
    return *val;
}

static void _MHal_EMAC_albany_clk_on(void* hal)
{
    u8 uRegVal;
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

    /* eth_link_sar*/
    //gain shift
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xb4, 0x02);

    //det max
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x4f, 0x02);

    //det min
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x51, 0x01);

    //snr len (emc noise)
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x77, 0x18);

    //lpbk_enable set to 0
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x72, 0xa0);

    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xfc, 0x00);   // Power-on LDO
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xfd, 0x00);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xa1, 0x80);   // Power-on SADC
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xcc, 0x40);   // Power-on ADCPL
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xbb, 0x04);   // Power-on REF
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0x3a, 0x00);   // Power-on TX
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xf1, 0x00);   // Power-on TX

    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0x8a, 0x01);    // CLKO_ADC_SEL
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x3b, 0x01);   // reg_adc_clk_select
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xc4, 0x44);  // TEST
    uRegVal = MHal_EMAC_ReadReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0x80);
    uRegVal = (uRegVal & 0xCF) | 0x30;
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0x80, uRegVal); // sadc timer

    //100 gat
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xc5, 0x00);

    //200 gat
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0x30, 0x43);

    //en_100t_phase
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0x39, 0x41);   // en_100t_phase;  [6] save2x_tx

    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xf2, 0xf5);  // LP mode, DAC OFF
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xf3, 0x0d); // DAC off

    // Prevent packet drop by inverted waveform
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x79, 0xd0);   // prevent packet drop by inverted waveform
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x77, 0x5a);

    //disable eee
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x2d, 0x7c);   // disable eee

    //10T waveform
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xe8, 0x06);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x2b, 0x00);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xe8, 0x00);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x2b, 0x00);

    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xe8, 0x06);   // shadow_ctrl
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xaa, 0x1c);   // tin17_s2
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xac, 0x1c);   // tin18_s2
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xad, 0x1c);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xae, 0x1c);   // tin19_s2
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xaf, 0x1c);

    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xe8, 0x00);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xaa, 0x1c);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0xab, 0x28);

    //speed up timing recovery
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xf5, 0x02);

    // Signal_det k
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x0f, 0xc9);

    // snr_h
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x89, 0x50);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x8b, 0x80);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x8e, 0x0e);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x90, 0x04);

    //set CLKsource to hv
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xC7, 0x80);

#if 0
    //enable LED //16'0x0e_28[5:4]=01
    uRegVal = MHal_EMAC_ReadReg8(EMAC_RIU_REG_BASE, REG_BANK_PMSLEEP, 0x50);
    uRegVal = (uRegVal&~0x30)|0x10;
    MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_PMSLEEP, 0x50, uRegVal);
#else
    #ifdef CONFIG_MS_PADMUX
    if (0== mdrv_padmux_active())
    #endif
    {
        if (pHal->pad_led_reg)
        {
            u32 u32Val;

            u32Val = *((volatile u32*)pHal->pad_led_reg);
            u32Val = (u32Val & ~pHal->pad_led_msk) | (pHal->pad_led_val & pHal->pad_led_msk);
            *((volatile u32*)pHal->pad_led_reg) = u32Val;
        }
    }
#endif

    ////swap LED0 and LED1
    //MHal_EMAC_WritReg8(REG_BANK_ALBANY0, 0xf7, BIT7);

#ifdef HARDWARE_DISCONNECT_DELAY
    /*
    wriu -w 0x003162    0x112b    // [14:12] slow_cnt_sel 001 42usx4 = 168us
                                  // [9:0] dsp_cnt_sel
    wriu -w 0x003160    0x06a4    // [11:9] slow_rst_sel 011 counter_hit 1334us x 6
                                  // [7:4] 1010 enable
                                  // [3:0] 0000 rst_cnt_sel  counter_hit
    */
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x62, 0x2b);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x63, 0x11);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x60, 0xa4);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY0, 0x61, 0x06);
#endif
}

static void _MHal_EMAC_albany_clk_off(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    /*
        wriu      0x003204  0x31     // Reset analog

        wait 100

        wriu  -w  0x0032fc  0x0102   // Power-off LDO
        wriu      0x0033a1  0xa0     // Power-off SADC
        wriu      0x0032cc  0x50     // Power-off ADCPL
        wriu      0x0032bb  0xc4     // Power-off REF
        wriu      0x00333a  0xf3     // Power-off TX
        wriu      0x0033f1  0x3c     // Power-off TX

        wriu      0x0033f3  0x0f     // DAC off

        wriu      0x003204  0x11     // Release reset analog
    */
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x04, 0x31);   // Reset analog
    mdelay(100);
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xfc, 0x02);   // Power-on LDO
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xfd, 0x01);

    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xa1, 0xa0);   // Power-off SADC
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xcc, 0x50);   // Power-off ADCPL
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0xbb, 0xc4);   // Power-off REF
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0x3a, 0xf3);   // Power-off TX
    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xf1, 0x3c);   // Power-off TX

    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY2, 0xf3, 0x0f);   // DAC off

    MHal_EMAC_WritReg8(pHal->phyRIU, REG_BANK_ALBANY1, 0x04, 0x11);   // Release reset analog

#if 0
    //turn off LED //16'0x0e_28[5:4]=01
    uRegVal = MHal_EMAC_ReadReg8(EMAC_RIU_REG_BASE, REG_BANK_PMSLEEP, 0x50);
    uRegVal = (uRegVal&~0x30);
    MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_PMSLEEP, 0x50, uRegVal);
#else
    #ifdef CONFIG_MS_PADMUX
    if (0 == mdrv_padmux_active())
    #endif
    {
        if (pHal->pad_led_reg)
        {
            u32 u32Val;

            u32Val = *((volatile u32*)pHal->pad_led_reg);
            u32Val = (u32Val & ~pHal->pad_led_msk);
            *((volatile u32*)pHal->pad_led_reg) = u32Val;
        }
    }
#endif
}

#define PHY_IAC_TIMEOUT		HZ

static int _MHal_EMAC_ext_busy_wait(void* hal)
{
    unsigned long t_start = jiffies;
    u32 uRegVal = 0;

    while (1)
    {
        uRegVal = MHal_EMAC_ReadReg32(hal, REG_ETH_SR);  //Must read Low 16 bit.
        if (uRegVal & EMAC_IDLE)
            return 0;
        if (time_after(jiffies, t_start + PHY_IAC_TIMEOUT))
            break;
        barrier();
    }
    printk("[%s][%d] mdio: MDIO timeout\n", __FUNCTION__, __LINE__);
    return -1;
}

static int _MHal_EMAC_ext_write(void* hal, u8 phy_addr, u32 addr, u32 val)
{
    u32 uRegVal = 0, uCTL = 0;

    uRegVal =  ( EMAC_HIGH | EMAC_CODE_802_3 | EMAC_RW_W) | (( phy_addr & 0x1F ) << PHY_ADDR_OFFSET )
                | ( addr << PHY_REGADDR_OFFSET ) | (val & 0xFFFF);

    uCTL = MHal_EMAC_Read_CTL(hal);
    MHal_EMAC_enable_mdi(hal);

    MHal_EMAC_Write_MAN(hal, uRegVal);
#if 0
    // Wait until IDLE bit in Network Status register is cleared //
    uRegVal = MHal_EMAC_ReadReg32(hal, REG_ETH_SR);  //Must read Low 16 bit.
    while (!(uRegVal & EMAC_IDLE))
    {
        uRegVal = MHal_EMAC_ReadReg32(hal, REG_ETH_SR);
        barrier();
    }
#else
    if (_MHal_EMAC_ext_busy_wait(hal))
    {
        printk("[%s][%d] (addr, reg, val) = (%d, %d, 0x%08x) timeout\n", __FUNCTION__, __LINE__, phy_addr, addr, val);
        return -1;
    }
#endif
    // printk("[%s][%d] (addr, reg, val) = (%d, %d, 0x%08x) OK\n", __FUNCTION__, __LINE__, phy_addr, addr, val);
    MHal_EMAC_Write_CTL(hal, uCTL);
    return 0;
}

static int _MHal_EMAC_ext_read(void* hal, u8 phy_addr, u32 addr, u32* val)
{
    u32 uRegVal = 0, uCTL = 0;

    *val = 0xffff;
    uRegVal = (EMAC_HIGH | EMAC_CODE_802_3 | EMAC_RW_R)
            | ((phy_addr & 0x1f) << PHY_ADDR_OFFSET) | (addr << PHY_REGADDR_OFFSET) | (0) ;

    uCTL = MHal_EMAC_Read_CTL(hal);
    MHal_EMAC_enable_mdi(hal);
    MHal_EMAC_Write_MAN(hal, uRegVal);

#if 0
    //Wait until IDLE bit in Network Status register is cleared //
    uRegVal = MHal_EMAC_ReadReg32(hal, REG_ETH_SR);  //Must read Low 16 bit.
    while (!(uRegVal & EMAC_IDLE))
    {
        uRegVal = MHal_EMAC_ReadReg32(hal, REG_ETH_SR);
        barrier();
    }
#else
    if (_MHal_EMAC_ext_busy_wait(hal))
    {
        printk("[%s][%d] (addr, reg) = (%d, %d) timeout\n", __FUNCTION__, __LINE__, phy_addr, addr);
        return 0xffff;
    }
#endif
    *val = (MHal_EMAC_Read_MAN(hal) & 0x0000ffff );
    MHal_EMAC_Write_CTL(hal, uCTL);
    // printk("[%s][%d] (addr, reg, val) = (%d, %d, 0x%08x) OK\n", __FUNCTION__, __LINE__, phy_addr, addr, *val);
    // printk("[%s][%d] val = 0x%08x\n", __FUNCTION__, __LINE__, *val);
    return *val;
}

static void _MHal_EMAC_ext_clk_on(void* hal)
{
#if 0
    //0x101e_0f[2]
    uRegVal = MHal_EMAC_ReadReg8(EMAC_RIU_REG_BASE, REG_BANK_CHIPTOP, 0x1E);
    uRegVal |= BIT2;
    MHal_EMAC_WritReg8(EMAC_RIU_REG_BASE, REG_BANK_CHIPTOP, 0x1E, uRegVal);
#else
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

#ifdef CONFIG_MS_PADMUX
    if (mdrv_padmux_active())
        return;
#endif

    if (pHal->pad_reg)
    {
        u32 u32Val;

        u32Val = *((volatile u32*)pHal->pad_reg);
        u32Val = (u32Val & ~pHal->pad_msk) | (pHal->pad_val & pHal->pad_msk);
        *((volatile u32*)pHal->pad_reg) = u32Val;
    }
#endif
}

static void _MHal_EMAC_ext_clk_off(void* hal)
{
    hal = hal;
}

void MHal_EMAC_Pad(void* hal, u32 reg, u32 msk, u32 val)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

    pHal->pad_reg = reg;
    pHal->pad_msk = msk;
    pHal->pad_val = val;
}

void MHal_EMAC_PadLed(void* hal, u32 reg, u32 msk, u32 val)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;

    pHal->pad_led_reg = reg;
    pHal->pad_led_msk = msk;
    pHal->pad_led_val = val;
}

void MHal_EMAC_PhyMode(void* hal, u32 phy_mode)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    pHal->phy_mode = phy_mode;
}

int MHal_EMAC_FlowControl_TX(void* hal)
{
    hal = hal;
    return 0;
}

void MHal_EMAC_MIU_Protect_RX(void* hal, u32 start, u32 end)
{
    u32 j100, j11c, j120, j124;

    if ((0xF & start) || (0xF& end))
    {
        printk("[%s][%d] warning for protection area without 16 byte alignment (start, end) = (0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, start, end);
        printk("[%s][%d] warning for protection area without 16 byte alignment (start, end) = (0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, start, end);
        printk("[%s][%d] warning for protection area without 16 byte alignment (start, end) = (0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, start, end);
    }
    start >>= 4;
    end >>= 4;

    j100 = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0100);
    j11c = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_011C);
    j120 = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0120);
    j124 = MHal_EMAC_ReadReg32(hal, REG_EMAC_JULIAN_0124);

    j11c = ((j11c & 0x0000ffff) | ((end & 0x0000ffff) << 16));
    j120 = ((j120 & 0x0000e000) | ((end & 0x1fff0000) >> 16) | ((start & 0x0000ffff) << 16) );
    j124 = ((j124 & 0xffffe000) | ((start & 0x1fff0000) >> 16) );
    j100 |= 0x40;

    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_011C, j11c);
    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0120, j120);
    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0124, j124);
    MHal_EMAC_WritReg32(hal, REG_EMAC_JULIAN_0100, j100);
}

void MHal_EMAC_Phy_Restart_An(void* hal)
{
    mhal_emac_t* pHal = (mhal_emac_t*) hal;
    MHal_EMAC_WritReg16(pHal->phyRIU, REG_BANK_ALBANY0, 0x00, 0x0000);
    udelay( 1 );
    MHal_EMAC_WritReg16(pHal->phyRIU, REG_BANK_ALBANY0, 0x00, 0x1000);
}
