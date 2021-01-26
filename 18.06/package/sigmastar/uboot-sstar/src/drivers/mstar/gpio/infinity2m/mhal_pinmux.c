/*
* mhal_pinmux.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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
#include "stdbool.h"
#include "asm/arch-infinity2m/mach/platform.h"
#include "asm/arch-infinity2m/mach/ms_types.h"
#include "mhal_gpio.h"
#include "padmux.h"
#include "gpio.h"

#include <common.h>
#include <MsDebug.h>

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define BASE_RIU_PA                         0x1F000000
#define PMSLEEP_BANK                        0x000E00
#define SAR_BANK                            0x001400
#define ALBANY1_BANK                        0x003200
#define ALBANY2_BANK                        0x003300
#define CHIPTOP_BANK                        0x101E00
#define UTMI0_BANK                          0x142100
#define UTMI1_BANK                          0x142500
#define UTMI2_BANK                          0x142900
#define DAC_ATOP_BANK                       0x112700

#define _GPIO_W_WORD(addr,val)              {(*(volatile u16*)(addr)) = (u16)(val);}
#define _GPIO_W_WORD_MASK(addr,val,mask)    {(*(volatile u16*)(addr)) = ((*(volatile u16*)(addr)) & ~(mask)) | ((u16)(val) & (mask));}
#define _GPIO_R_BYTE(addr)                  (*(volatile u8*)(addr))
#define _GPIO_R_WORD_MASK(addr,mask)        ((*(volatile u16*)(addr)) & (mask))

#define GET_BASE_ADDR_BY_BANK(x, y)         ((x) + ((y) << 1))
#define _RIUA_8BIT(bank , offset)           GET_BASE_ADDR_BY_BANK(BASE_RIU_PA, bank) + (((offset) & ~1)<<1) + ((offset) & 1)
#define _RIUA_16BIT(bank , offset)          GET_BASE_ADDR_BY_BANK(BASE_RIU_PA, bank) + ((offset)<<2)

/* Non PM Pad : CHIPTOP_BANK */
#include "chiptop_reg.h"

/* PM PAD : PMSLEEP_BANK */
#define REG_GPIO_PM_LOCK        0x12
    #define REG_GPIO_PM_LOCK_MASK   0xFFFF
#define REG_PM_GPIO_PM4_INV     0x1c
    #define REG_PM_GPIO_PM4_INV_MASK BIT1
#define REG_PM_LINK_WKINT2GPIO4 0x1c
    #define REG_PM_LINK_WKINT2GPIO4_MASK BIT3

#define REG_PM_IR_IS_GPIO       0x1c
    #define REG_PM_IR_IS_GPIO_MASK  BIT4
#define REG_PM_IRIN_MODE        REG_PM_IR_IS_GPIO

#define REG_PM_PWM0_MODE        0x28
    #define REG_PM_PWM0_MODE_MASK   BIT1|BIT0
#define REG_PM_PWM1_MODE        0x28
    #define REG_PM_PWM1_MODE_MASK   BIT3|BIT2
#define REG_PM_PWM2_MODE        0x28
    #define REG_PM_PWM2_MODE_MASK   BIT7|BIT6
#define REG_PM_PWM3_MODE        0x28
    #define REG_PM_PWM3_MODE_MASK   BIT9|BIT8
#define REG_PM_PWM4_MODE        0x27
    #define REG_PM_PWM4_MODE_MASK   BIT0
#define REG_PM_PWM5_MODE        0x27
    #define REG_PM_PWM5_MODE_MASK   BIT1
#define REG_PM_PWM8_MODE        0x27
    #define REG_PM_PWM8_MODE_MASK   BIT2
#define REG_PM_PWM9_MODE        0x27
    #define REG_PM_PWM9_MODE_MASK   BIT3
#define REG_PM_PWM10_MODE        0x27
    #define REG_PM_PWM10_MODE_MASK   BIT4
#define REG_PM_UART1_MODE        0x27
    #define REG_PM_UART1_MODE_MASK   BIT8
#define REG_PM_LED_MODE         0x28
    #define REG_PM_LED_MODE_MASK    BIT5|BIT4

#define REG_PM_VID_MODE         0x28
    #define REG_PM_VID_MODE_MASK    BIT13|BIT12
#define REG_PM_SD_CDZ_MODE      0x28
    #define REG_PM_SD_CDZ_MODE_MASK BIT14

#define REG_PM_SPI_IS_GPIO      0x35
    #define REG_PM_SPI_IS_GPIO_MASK BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
    #define REG_PM_SPI_GPIO_MASK            BIT0
    #define REG_PM_SPICSZ1_GPIO_MASK        BIT2
    #define REG_PM_SPICSZ2_GPIO_MASK        BIT3
    #define REG_PM_SPIWPN_GPIO_MASK         BIT4
    #define REG_PM_SPIHOLDN_MODE_MASK       BIT6 | BIT7
#define REG_PM_SPICSZ1_GPIO     REG_PM_SPI_IS_GPIO
#define REG_PM_SPICSZ2_GPIO     REG_PM_SPI_IS_GPIO
#define REG_PM_SPI_GPIO         REG_PM_SPI_IS_GPIO
#define REG_PM_SPIWPN_GPIO      REG_PM_SPI_IS_GPIO
#define REG_PM_SPIHOLDN_MODE    REG_PM_SPI_IS_GPIO

#define REG_PM_UART_IS_GPIO     0x35
    #define REG_PM_UART_IS_GPIO_MASK BIT11|BIT10|BIT9|BIT8

#define REG_PM_SPICSZ2_MODE     0x36
    #define REG_PM_SPICSZ2_MODE_MASK        BIT12

/* DAC_ATOP */
#define REG_GPIO_EN_PAD_OUT       0x10
    #define REG_GPIO_EN_PAD_OUT_B_MASK       BIT0
    #define REG_GPIO_EN_PAD_OUT_G_MASK       BIT1
    #define REG_GPIO_EN_PAD_OUT_R_MASK       BIT2

/* SAR : SAR_BANK, R/W 8-bits */
#define REG_SAR_AISEL_8BIT          0x11*2
    #define REG_SAR_CH0_AISEL       BIT0
    #define REG_SAR_CH1_AISEL       BIT1
    #define REG_SAR_CH2_AISEL       BIT2
    #define REG_SAR_CH3_AISEL       BIT3

/* EMAC : ALBANY1_BANK */
#define REG_ATOP_RX_INOFF       0x69
    #define REG_ATOP_RX_INOFF_MASK  BIT15|BIT14

/* EMAC : ALBANY2_BANK */
#define REG_ETH_GPIO_EN         0x71
    #define REG_ETH_GPIO_EN_MASK    BIT3|BIT2|BIT1|BIT0

/* UTMI0/1/2 : UTMI_BANK */
#define REG_UTMI_FL_XVR_PDN    0x0
    #define REG_UTMI_FL_XVR_PDN_MASK BIT12
#define REG_UTMI_REG_PDN       0x0
    #define REG_UTMI_REG_PDN_MASK BIT15
#define REG_UTMI_CLK_EXTRA0_EN 0x4
    #define REG_UTMI_CLK_EXTRA0_EN_MASK BIT7
#define REG_UTMI_GPIO_EN       0x1f
    #define REG_UTMI_GPIO_EN_MASK  BIT14

//-------------------- configuration -----------------
#define ENABLE_CHECK_ALL_PAD_CONFLICT       0

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct stPadMux
{
    U16 padID;
    U32 base;
    U16 offset;
    U16 mask;
    U16 val;
    U16 mode;
} ST_PadMuxInfo;

typedef struct stPadMode
{
    U8  u8PadName[16];
    U32 u32ModeRIU;
    U32 u32ModeMask;
} ST_PadModeInfo;

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
// static constST_PadMuxInfo m_stPadMuxTbl[]
#include "PADMUX_TABLE.ci"

//static const ST_PadModeInfo m_stPadModeInfoTbl[] =
#include "PADMODE_INFO.ci"

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : _HalCheckPin
//  Description :
//------------------------------------------------------------------------------
static S32 _HalCheckPin(U32 padID)
{
    if (GPIO_NR <= padID) {
        return FALSE;
    }
    return TRUE;
}

static void _HalSARGPIOWriteRegBit(u32 u32RegOffset, bool bEnable, U8 u8BitMsk)
{
    if (bEnable)
        _GPIO_R_BYTE(_RIUA_8BIT(SAR_BANK, u32RegOffset)) |= u8BitMsk;
    else
        _GPIO_R_BYTE(_RIUA_8BIT(SAR_BANK, u32RegOffset)) &= (~u8BitMsk);
}

static void _HalPadDisablePadMux(U32 u32PadModeID)
{
    if (_GPIO_R_WORD_MASK(m_stPadModeInfoTbl[u32PadModeID].u32ModeRIU, m_stPadModeInfoTbl[u32PadModeID].u32ModeMask)) {
        _GPIO_W_WORD_MASK(m_stPadModeInfoTbl[u32PadModeID].u32ModeRIU, 0, m_stPadModeInfoTbl[u32PadModeID].u32ModeMask);
    }
}



static S32 HalPadSetMode_General(U32 u32PadID, U32 u32Mode)
{
    U32 u32RegAddr = 0;
    U16 u16RegVal  = 0;
    U8  u8ModeIsFind = 0;
    U16 i = 0;

    for (i = 0; i < sizeof(m_stPadMuxTbl)/sizeof(struct stPadMux); i++)
    {
        if (u32PadID == m_stPadMuxTbl[i].padID)
        {
            u32RegAddr = _RIUA_16BIT(m_stPadMuxTbl[i].base, m_stPadMuxTbl[i].offset);

            if (u32Mode == m_stPadMuxTbl[i].mode)
            {
                //UBOOT_INFO("gpio debug %s:%d \n", __FUNCTION__,__LINE__);
                u16RegVal = _GPIO_R_WORD_MASK(u32RegAddr, 0xFFFF);
                u16RegVal &= ~(m_stPadMuxTbl[i].mask);
                u16RegVal |= m_stPadMuxTbl[i].val; // CHECK Multi-Pad Mode

                _GPIO_W_WORD_MASK(u32RegAddr, u16RegVal, 0xFFFF);

                u8ModeIsFind = 1;
#if (ENABLE_CHECK_ALL_PAD_CONFLICT == 0)
                break;
#endif
            }
            else
            {
                u16RegVal = _GPIO_R_WORD_MASK(u32RegAddr, m_stPadMuxTbl[i].mask);

                if (u16RegVal == m_stPadMuxTbl[i].val)
                {
                    if (m_stPadMuxTbl[i].val != 0)
                    {
                        _GPIO_W_WORD_MASK(u32RegAddr, 0, m_stPadMuxTbl[i].mask);
                    }
                    else
                    {
                        _GPIO_W_WORD_MASK(u32RegAddr, m_stPadMuxTbl[i].mask, m_stPadMuxTbl[i].mask);
                    }
                }
            }
        }
    }

    return (u8ModeIsFind) ? 0 : -1;
}

static S32 HalPadSetMode_MISC(U32 u32PadID, U32 u32Mode)
{
    U32 utmi_bank = UTMI0_BANK;
    U16 u16BitMask;
    U8  u8BitMask;

    switch(u32PadID)
    {
    /* DAC_ATOP */
    case PADA_IDAC_OUT_B: /* reg_gpio_en_pad_out_b; reg[112720]#0=1b */
    case PADA_IDAC_OUT_G: /* reg_gpio_en_pad_out_b; reg[112720]#1=1b */
    case PADA_IDAC_OUT_R: /* reg_gpio_en_pad_out_b; reg[112720]#2=1b */
        if (u32PadID == PADA_IDAC_OUT_B)
            u16BitMask = REG_GPIO_EN_PAD_OUT_B_MASK;
        else if (u32PadID == PADA_IDAC_OUT_G)
            u16BitMask = REG_GPIO_EN_PAD_OUT_G_MASK;
        else if (u32PadID == PADA_IDAC_OUT_R)
            u16BitMask = REG_GPIO_EN_PAD_OUT_R_MASK;

        if (u32Mode == PINMUX_FOR_GPIO_MODE) {
            _GPIO_W_WORD_MASK(_RIUA_16BIT(DAC_ATOP_BANK,REG_GPIO_EN_PAD_OUT), u16BitMask, u16BitMask);
        }
        else if (u32Mode == PINMUX_FOR_DAC_MODE) {
            _GPIO_W_WORD_MASK(_RIUA_16BIT(DAC_ATOP_BANK,REG_GPIO_EN_PAD_OUT), 0, u16BitMask);
        }
        else {
            return -1;
        }
        break;

    /* SAR */
    case PAD_SAR_GPIO0: /* reg_sar_aisel; reg[1422]#5 ~ #0=0b */
        if (u32Mode == PINMUX_FOR_GPIO_MODE) {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPICSZ2_MODE), 0, REG_PM_SPICSZ2_MODE_MASK);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, 0, REG_SAR_CH0_AISEL);
        }
        else if (u32Mode == PINMUX_FOR_SAR_MODE) {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPICSZ2_MODE), 0, REG_PM_SPICSZ2_MODE_MASK);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, REG_SAR_CH0_AISEL, REG_SAR_CH0_AISEL);
        }
        else {
            return -1;
        }
        break;
    case PAD_SAR_GPIO1:
    case PAD_SAR_GPIO2:
    case PAD_SAR_GPIO3:
        if (u32PadID == PAD_SAR_GPIO1)
            u8BitMask = REG_SAR_CH1_AISEL;
        else if (u32PadID == PAD_SAR_GPIO2)
            u8BitMask = REG_SAR_CH2_AISEL;
        else if (u32PadID == PAD_SAR_GPIO3)
            u8BitMask = REG_SAR_CH3_AISEL;

        if (u32Mode == PINMUX_FOR_GPIO_MODE) {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, 0, u8BitMask);
        }
        else if (u32Mode == PINMUX_FOR_SAR_MODE) {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, u8BitMask, u8BitMask);
        }
        else {
            return -1;
        }
        break;

     /* lan-top */
     case PAD_ETH_RN:
     case PAD_ETH_RP:
     case PAD_ETH_TN:
     case PAD_ETH_TP:
         if (u32PadID == PAD_ETH_RN)
             u16BitMask = BIT0;
         else if (u32PadID == PAD_ETH_RP)
             u16BitMask = BIT1;
         else if (u32PadID == PAD_ETH_TN)
             u16BitMask = BIT2;
         else if (u32PadID == PAD_ETH_TP)
             u16BitMask = BIT3;

         if (u32Mode == PINMUX_FOR_GPIO_MODE) {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), u16BitMask, u16BitMask);
        }
        else if (u32Mode == PINMUX_FOR_ETH_MODE) {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), 0, u16BitMask);
        }
        else {
            return -1;
        }
        break;

    /* UTMI */
    case PAD_DM_P1:
    case PAD_DP_P1:
    case PAD_DM_P2:
    case PAD_DP_P2:
    case PAD_DM_P3:
    case PAD_DP_P3:
        if ((u32PadID == PAD_DM_P1) || (u32PadID == PAD_DP_P1))
            utmi_bank = UTMI0_BANK;
        else if ((u32PadID == PAD_DM_P2) || (u32PadID == PAD_DP_P2))
            utmi_bank = UTMI1_BANK;
        if ((u32PadID == PAD_DM_P3) || (u32PadID == PAD_DP_P3))
            utmi_bank = UTMI2_BANK;

        if (u32Mode == PINMUX_FOR_GPIO_MODE) {
            //_HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE);
            //_HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(utmi_bank,REG_UTMI_GPIO_EN), REG_UTMI_GPIO_EN_MASK, REG_UTMI_GPIO_EN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(utmi_bank,REG_UTMI_CLK_EXTRA0_EN), REG_UTMI_CLK_EXTRA0_EN_MASK, REG_UTMI_CLK_EXTRA0_EN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(utmi_bank,REG_UTMI_REG_PDN), REG_UTMI_REG_PDN_MASK, REG_UTMI_REG_PDN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(utmi_bank,REG_UTMI_FL_XVR_PDN), REG_UTMI_FL_XVR_PDN_MASK, REG_UTMI_FL_XVR_PDN_MASK);
        }
        else if (u32Mode == PINMUX_FOR_USB_MODE) {
            _GPIO_W_WORD_MASK(_RIUA_16BIT(utmi_bank,REG_UTMI_GPIO_EN), ~REG_UTMI_GPIO_EN_MASK, REG_UTMI_GPIO_EN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(utmi_bank,REG_UTMI_CLK_EXTRA0_EN), ~REG_UTMI_CLK_EXTRA0_EN_MASK, REG_UTMI_CLK_EXTRA0_EN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(utmi_bank,REG_UTMI_REG_PDN), ~REG_UTMI_REG_PDN_MASK, REG_UTMI_REG_PDN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(utmi_bank,REG_UTMI_FL_XVR_PDN), REG_UTMI_FL_XVR_PDN_MASK, REG_UTMI_FL_XVR_PDN_MASK);
        }
        else {
            return -1;
        }
        break;

    default:
        return -1;
    }

    return 0;
}
//------------------------------------------------------------------------------
//  Function    : HalPadSetVal
//  Description :
//------------------------------------------------------------------------------
S32 HalPadSetVal(U32 u32PadID, U32 u32Mode)
{
    if (FALSE == _HalCheckPin(u32PadID)) {
        return FALSE;
    }

    if ((u32PadID >= PADA_IDAC_OUT_B && u32PadID <= PADA_IDAC_OUT_R) ||
        (u32PadID >= PAD_SAR_GPIO0 && u32PadID <= PAD_DP_P3)) {
        return HalPadSetMode_MISC(u32PadID, u32Mode);
    }
    else {
        return HalPadSetMode_General(u32PadID, u32Mode);
    }
}

//------------------------------------------------------------------------------
//  Function    : HalPadSet
//  Description :
//------------------------------------------------------------------------------
S32 HalPadSetMode(U32 u32Mode)
{
    U32 u32PadID;
    U8  u8ModeIsFind = 0;
    U16 k = 0;

    for (k = 0; k < sizeof(m_stPadMuxTbl)/sizeof(struct stPadMux); k++)
    {
        if (u32Mode == m_stPadMuxTbl[k].mode)
        {
            u32PadID = m_stPadMuxTbl[k].padID;
            u8ModeIsFind = u8ModeIsFind || HalPadSetMode_General( u32PadID, u32Mode);
         }
    }

    return (u8ModeIsFind) ? TRUE : FALSE;
}
