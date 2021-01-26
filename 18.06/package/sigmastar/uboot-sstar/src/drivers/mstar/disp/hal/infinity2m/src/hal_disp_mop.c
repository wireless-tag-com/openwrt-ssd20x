/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#define _HAL_DISP_MOP_C_

#include "drv_disp_os.h"
#include "hal_disp_common.h"
#include "disp_debug.h"
#include "hal_disp_util.h"
#include "hal_disp_reg.h"
#include "hal_disp_chip.h"
#include "hal_disp_st.h"
#include "hal_disp_mop.h"

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HAL_DISP_MOP_MIU_BUS 4
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

//SW shadow
//bk 0x1406
u16 u16Mopg01SwShadowVal[E_HAL_DISP_LAYER_MOPG01_SHADOW_NUM];
u32 u16Mopg01SwShadowReg[E_HAL_DISP_LAYER_MOPG01_SHADOW_NUM];

//bk 0x1407
u16 u16Mopg02SwShadowVal[E_HAL_DISP_LAYER_MOPG02_SHADOW_NUM];
u32 u16Mopg02SwShadowReg[E_HAL_DISP_LAYER_MOPG02_SHADOW_NUM];

//bk 0x1408
u16 u16MopsSwShadowVal[E_HAL_DISP_LAYER_MOPS_SHADOW_NUM];
u32 u16MopsSwShadowReg[E_HAL_DISP_LAYER_MOPS_SHADOW_NUM];

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

//sw shadow
void HalMopsShadowInit(void)
{
    //bk 0x1406
    u16Mopg01SwShadowVal[E_HAL_DISP_LAYER_MOPG01_01] = R2BYTE(REG_DISP_MOPG_BK1_01_L);
    u16Mopg01SwShadowReg[E_HAL_DISP_LAYER_MOPG01_01] = REG_DISP_MOPG_BK1_01_L;
    u16Mopg01SwShadowVal[E_HAL_DISP_LAYER_MOPG01_11] = R2BYTE(REG_DISP_MOPG_BK1_11_L);
    u16Mopg01SwShadowReg[E_HAL_DISP_LAYER_MOPG01_11] = REG_DISP_MOPG_BK1_11_L;
    u16Mopg01SwShadowVal[E_HAL_DISP_LAYER_MOPG01_21] = R2BYTE(REG_DISP_MOPG_BK1_21_L);
    u16Mopg01SwShadowReg[E_HAL_DISP_LAYER_MOPG01_21] = REG_DISP_MOPG_BK1_21_L;
    u16Mopg01SwShadowVal[E_HAL_DISP_LAYER_MOPG01_31] = R2BYTE(REG_DISP_MOPG_BK1_31_L);
    u16Mopg01SwShadowReg[E_HAL_DISP_LAYER_MOPG01_31] = REG_DISP_MOPG_BK1_31_L;
    u16Mopg01SwShadowVal[E_HAL_DISP_LAYER_MOPG01_41] = R2BYTE(REG_DISP_MOPG_BK1_41_L);
    u16Mopg01SwShadowReg[E_HAL_DISP_LAYER_MOPG01_41] = REG_DISP_MOPG_BK1_41_L;
    u16Mopg01SwShadowVal[E_HAL_DISP_LAYER_MOPG01_51] = R2BYTE(REG_DISP_MOPG_BK1_51_L);
    u16Mopg01SwShadowReg[E_HAL_DISP_LAYER_MOPG01_51] = REG_DISP_MOPG_BK1_51_L;
    u16Mopg01SwShadowVal[E_HAL_DISP_LAYER_MOPG01_61] = R2BYTE(REG_DISP_MOPG_BK1_61_L);
    u16Mopg01SwShadowReg[E_HAL_DISP_LAYER_MOPG01_61] = REG_DISP_MOPG_BK1_61_L;
    u16Mopg01SwShadowVal[E_HAL_DISP_LAYER_MOPG01_71] = R2BYTE(REG_DISP_MOPG_BK1_71_L);
    u16Mopg01SwShadowReg[E_HAL_DISP_LAYER_MOPG01_71] = REG_DISP_MOPG_BK1_71_L;

    //bk 0x1407
    u16Mopg02SwShadowVal[E_HAL_DISP_LAYER_MOPG02_01] = R2BYTE(REG_DISP_MOPG_BK2_01_L);
    u16Mopg02SwShadowReg[E_HAL_DISP_LAYER_MOPG02_01] = REG_DISP_MOPG_BK2_01_L;
    u16Mopg02SwShadowVal[E_HAL_DISP_LAYER_MOPG02_11] = R2BYTE(REG_DISP_MOPG_BK2_11_L);
    u16Mopg02SwShadowReg[E_HAL_DISP_LAYER_MOPG02_11] = REG_DISP_MOPG_BK2_11_L;
    u16Mopg02SwShadowVal[E_HAL_DISP_LAYER_MOPG02_21] = R2BYTE(REG_DISP_MOPG_BK2_21_L);
    u16Mopg02SwShadowReg[E_HAL_DISP_LAYER_MOPG02_21] = REG_DISP_MOPG_BK2_21_L;
    u16Mopg02SwShadowVal[E_HAL_DISP_LAYER_MOPG02_31] = R2BYTE(REG_DISP_MOPG_BK2_31_L);
    u16Mopg02SwShadowReg[E_HAL_DISP_LAYER_MOPG02_31] = REG_DISP_MOPG_BK2_31_L;
    u16Mopg02SwShadowVal[E_HAL_DISP_LAYER_MOPG02_41] = R2BYTE(REG_DISP_MOPG_BK2_41_L);
    u16Mopg02SwShadowReg[E_HAL_DISP_LAYER_MOPG02_41] = REG_DISP_MOPG_BK2_41_L;
    u16Mopg02SwShadowVal[E_HAL_DISP_LAYER_MOPG02_51] = R2BYTE(REG_DISP_MOPG_BK2_51_L);
    u16Mopg02SwShadowReg[E_HAL_DISP_LAYER_MOPG02_51] = REG_DISP_MOPG_BK2_51_L;
    u16Mopg02SwShadowVal[E_HAL_DISP_LAYER_MOPG02_61] = R2BYTE(REG_DISP_MOPG_BK2_61_L);
    u16Mopg02SwShadowReg[E_HAL_DISP_LAYER_MOPG02_61] = REG_DISP_MOPG_BK2_61_L;
    u16Mopg02SwShadowVal[E_HAL_DISP_LAYER_MOPG02_71] = R2BYTE(REG_DISP_MOPG_BK2_71_L);
    u16Mopg02SwShadowReg[E_HAL_DISP_LAYER_MOPG02_71] = REG_DISP_MOPG_BK2_71_L;

    //bk 0x1408
    u16MopsSwShadowVal[E_HAL_DISP_LAYER_MOPS_51] = R2BYTE(REG_DISP_MOPS_BK0_51_L);
    u16MopsSwShadowReg[E_HAL_DISP_LAYER_MOPS_51] = REG_DISP_MOPS_BK0_51_L;

}
u16 _HalMopGetSwShadow(u32 eMopBk, u32 u32Reg)
{
    u16 u16val;
    if(eMopBk == REG_DISP_MOPS_BK0_BASE)
    {
        //mops sw shadow register
        u16val = u16MopsSwShadowVal[u32Reg];
    }
    else if(eMopBk == REG_DISP_MOPG_BK1_BASE)
    {
        //mopg01 sw shadow register
        u16val = u16Mopg01SwShadowVal[u32Reg];
    }
    else if(eMopBk == REG_DISP_MOPG_BK2_BASE)
    {
        //mopg02 sw shadow register
        u16val = u16Mopg02SwShadowVal[u32Reg];
    }
    else //left for other mop bank if needed
    {
        u16val=0;
    }
    return u16val;
}
void _HalMopSetSwShadow(u32 eMopBk, u32 u32Reg, u16 u16Val)
{
    if(eMopBk == REG_DISP_MOPS_BK0_BASE)
    {
        //Gwin
        u16MopsSwShadowVal[u32Reg] = u16Val;
    }
    else if(eMopBk == REG_DISP_MOPG_BK1_BASE)
    {
        //mopg01 sw shadow register
        u16Mopg01SwShadowVal[u32Reg] = u16Val;
    }
    else if(eMopBk == REG_DISP_MOPG_BK2_BASE)
    {
        //mopg02 sw shadow register
        u16Mopg02SwShadowVal[u32Reg] = u16Val;
    }
    else //left for other mop bank if needed
    {

    }
}
u32 _HalMopGetShadowReg(u32 eMopBk, u32 u32Reg)
{
    u32 u32val;
    if(eMopBk == REG_DISP_MOPS_BK0_BASE)
    {
        //mops sw shadow register
        u32val = u16MopsSwShadowReg[u32Reg];
    }
    else if(eMopBk == REG_DISP_MOPG_BK1_BASE)
    {
        //mopg01 sw shadow register
        u32val = u16Mopg01SwShadowReg[u32Reg];
    }
    else if(eMopBk == REG_DISP_MOPG_BK2_BASE)
    {
        //mopg02 sw shadow register
        u32val = u16Mopg02SwShadowReg[u32Reg];
    }
    else //left for other mop bank if needed
    {
        u32val=0;
    }
    return u32val;
}
void _HalMopSwShadowHandler(u32 eMopBk, u32 u32Reg, u16 u16Val, u16 u16Msk)
{
    u16 u16ShadowVal;
    u32 u32RegVal;
    u16ShadowVal = _HalMopGetSwShadow(eMopBk,u32Reg);
    u16ShadowVal = ((u16Val&u16Msk)|(u16ShadowVal&(~u16Msk)));
    _HalMopSetSwShadow(eMopBk,u32Reg,u16ShadowVal);
    u32RegVal=_HalMopGetShadowReg(eMopBk, u32Reg);
    W2BYTEMSK(u32RegVal, u16ShadowVal, 0xFFFF);
}

u32 _HalDispMopgGwinRegBkMap(HalDispMopgGwinId_e eMopgId)
{
    u32 u32RegBk;

    switch(eMopgId)
    {
        case E_HAL_DISP_MOPG_ID_00:
        case E_HAL_DISP_MOPG_ID_01:
        case E_HAL_DISP_MOPG_ID_02:
        case E_HAL_DISP_MOPG_ID_03:
        case E_HAL_DISP_MOPG_ID_04:
        case E_HAL_DISP_MOPG_ID_05:
        case E_HAL_DISP_MOPG_ID_06:
        case E_HAL_DISP_MOPG_ID_07:
            u32RegBk = REG_DISP_MOPG_BK1_BASE;
            break;
        case E_HAL_DISP_MOPG_ID_08:
        case E_HAL_DISP_MOPG_ID_09:
        case E_HAL_DISP_MOPG_ID_10:
        case E_HAL_DISP_MOPG_ID_11:
        case E_HAL_DISP_MOPG_ID_12:
        case E_HAL_DISP_MOPG_ID_13:
        case E_HAL_DISP_MOPG_ID_14:
        case E_HAL_DISP_MOPG_ID_15:
            u32RegBk = REG_DISP_MOPG_BK2_BASE;
            break;
        default:
            DISP_ERR("[HALMOP]%s %d, Mopg ID %d not support\n", __FUNCTION__, __LINE__,eMopgId);
            u32RegBk = REG_DISP_MOPG_BK1_BASE;
            break;
    }
    return u32RegBk;
}

u32 _HalDispMopgGwinRegShiftMap(HalDispMopgGwinId_e eMopgId)
{
    u32 u32GeinRegShift;

    switch(eMopgId)
    {
        case E_HAL_DISP_MOPG_ID_00:
        case E_HAL_DISP_MOPG_ID_08:
            u32GeinRegShift = 0x00;
            break;
        case E_HAL_DISP_MOPG_ID_01:
        case E_HAL_DISP_MOPG_ID_09:
            u32GeinRegShift = 0x20;
            break;
        case E_HAL_DISP_MOPG_ID_02:
        case E_HAL_DISP_MOPG_ID_10:
            u32GeinRegShift = 0x40;
            break;
        case E_HAL_DISP_MOPG_ID_03:
        case E_HAL_DISP_MOPG_ID_11:
            u32GeinRegShift = 0x60;
            break;
        case E_HAL_DISP_MOPG_ID_04:
        case E_HAL_DISP_MOPG_ID_12:
            u32GeinRegShift = 0x80;
            break;
        case E_HAL_DISP_MOPG_ID_05:
        case E_HAL_DISP_MOPG_ID_13:
            u32GeinRegShift = 0xA0;
            break;
        case E_HAL_DISP_MOPG_ID_06:
        case E_HAL_DISP_MOPG_ID_14:
            u32GeinRegShift = 0xC0;
            break;
        case E_HAL_DISP_MOPG_ID_07:
        case E_HAL_DISP_MOPG_ID_15:
            u32GeinRegShift = 0xE0;
            break;
        default:
            DISP_ERR("[HALMOP]%s %d, Mopg ID %d not support\n", __FUNCTION__, __LINE__,eMopgId);
            u32GeinRegShift = 0x00;
            break;
    }
    return u32GeinRegShift;
}

u32 _HalDispMopgGwinRegSwShadowMap(HalDispMopgGwinId_e eMopgId)
{
    u32 u32SwShadowMap = 0;

    switch(eMopgId)
    {
        case E_HAL_DISP_MOPG_ID_00:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG01_01;
            break;
        case E_HAL_DISP_MOPG_ID_01:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG01_11;
            break;
        case E_HAL_DISP_MOPG_ID_02:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG01_21;
            break;
        case E_HAL_DISP_MOPG_ID_03:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG01_31;
            break;
        case E_HAL_DISP_MOPG_ID_04:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG01_41;
            break;
        case E_HAL_DISP_MOPG_ID_05:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG01_51;
            break;
        case E_HAL_DISP_MOPG_ID_06:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG01_61;
            break;
        case E_HAL_DISP_MOPG_ID_07:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG01_71;
            break;
        case E_HAL_DISP_MOPG_ID_08:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG02_01;
            break;
        case E_HAL_DISP_MOPG_ID_09:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG02_11;
            break;
        case E_HAL_DISP_MOPG_ID_10:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG02_21;
            break;
        case E_HAL_DISP_MOPG_ID_11:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG02_31;
            break;
        case E_HAL_DISP_MOPG_ID_12:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG02_41;
            break;
        case E_HAL_DISP_MOPG_ID_13:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG02_51;
            break;
        case E_HAL_DISP_MOPG_ID_14:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG02_61;
            break;
        case E_HAL_DISP_MOPG_ID_15:
            u32SwShadowMap = E_HAL_DISP_LAYER_MOPG02_71;
            break;
        default:
            DISP_ERR("[HALMOP]%s %d, Mopg ID %d not support\n", __FUNCTION__, __LINE__,eMopgId);
            u32SwShadowMap = 0x00;
            break;
    }
    return u32SwShadowMap;
}


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void HalDispMopDbBfWr(void);

///MOPG
void HalDispMopgSwReset(void)
{
    W2BYTEMSK(REG_DISP_MOPG_BK0_00_L, 0x0001, 0x0001);
    W2BYTEMSK(REG_DISP_MOPG_BK0_00_L, 0x0000, 0x0001);
}

void HalDispMopgSetAutoStretchWinSizeEn(bool bEnAuto)//0: use reg_gw_hsize/vsize; 1: use display size
{
    W2BYTEMSK(REG_DISP_MOPG_BK0_01_L, bEnAuto ? DISP_BIT10 : 0, DISP_BIT10);
}

void HalDispMopgSetStretchWinSize(u16 u16Width, u16 u16Height)
{
    W2BYTEMSK(REG_DISP_MOPG_BK0_07_L, u16Width, 0x1FFF);//reg_gw_hsize
    W2BYTEMSK(REG_DISP_MOPG_BK0_08_L, u16Height, 0x1FFF);//reg_gw_vsize
}

void HalDispMopgSetPipeDelay(u8 u8PipeDelay)
{
    W2BYTEMSK(REG_DISP_MOPG_BK0_03_L, u8PipeDelay, 0x001F);//reg_gw_hsize
}

void HalDispMopgInit(void)
{
    HalDispMopgSwReset();
    W2BYTEMSK(REG_DISP_MOPG_BK0_01_L, DISP_BIT8, DISP_BIT8);//clk_miu. 0:use free run; 1:use gated clock
    W2BYTEMSK(REG_DISP_MOPG_BK0_01_L, DISP_BIT9, DISP_BIT9);//clk_gop. 0:use free run; 1:use gated clock
    HalDispMopgSetAutoStretchWinSizeEn(TRUE);
    W2BYTEMSK(REG_DISP_MOPG_BK0_01_L, DISP_BIT11, DISP_BIT11);//auto blanking. 0:use reg h15; 1:use display blanking
    HalDispMopgSetPipeDelay(0xA);//set pipe delay (default=0xA)
    W2BYTEMSK(REG_DISP_MOPG_BK0_04_L, 0xF8, 0x00FF);//h04 [7:0]ymd_thd
    W2BYTEMSK(REG_DISP_MOPG_BK0_05_L, 0xD0, 0x00FF);//h05 [7:0]cdma_thd; default=0xf8, use 0xd0 for patch
    W2BYTEMSK(REG_DISP_MOPG_BK0_06_L, 0xF0, 0x00FF);//h06 Luma DMA priority [7]yhp_en [6:0] yhp_thd
    W2BYTEMSK(REG_DISP_MOPG_BK0_0B_L, 0xF0, 0x00FF);//h0b Chroma DMA priority [7]yhp_en [6:0] yhp_thd
    W2BYTEMSK(REG_DISP_MOPG_BK0_15_L, 0x80, 0x1FFF);//h15 [12:0]gw_hext

    W2BYTEMSK(REG_DISP_MOPG_BK0_40_L, 0x0689, 0x07FF);//h40 set 4tap

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopgGwinEn(HalDispMopgGwinId_e eMopgId, bool bEn)
{
    u32 Mopg_Gwin_Reg_BK = 0;
    u32 Mopg_Gwin_Reg_Shift = 0;
    u32 Mopg_Gwin_Reg_Base = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    Mopg_Gwin_Reg_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_00_L:
                                                                        REG_DISP_MOPG_BK1_00_L;
    W2BYTEMSK(Mopg_Gwin_Reg_Base + Mopg_Gwin_Reg_Shift, bEn ? DISP_BIT0 : 0, DISP_BIT0);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopgSetYAddr(HalDispMopgGwinId_e eMopgId, u32 u32PhyAddr)
{
    u32 u32RegVal;
    u32 Mopg_Gwin_Reg_BK = 0;
    u32 Mopg_Gwin_Reg_Shift = 0;
    u32 Mopg_Gwin_Reg_Base = 0;
    u32 Mopg_Gwin_Reg_Base2 = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    Mopg_Gwin_Reg_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_02_L:
                                                                        REG_DISP_MOPG_BK1_02_L;
    Mopg_Gwin_Reg_Base2 = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_03_L:
                                                                         REG_DISP_MOPG_BK1_03_L;
    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(Mopg_Gwin_Reg_Base + Mopg_Gwin_Reg_Shift, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(Mopg_Gwin_Reg_Base2 + Mopg_Gwin_Reg_Shift, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopgSetAddr16Offset(HalDispMopgGwinId_e eMopgId, u8 u8Offset)
{
    u32 u32RegVal;
    u32 Mopg_Gwin_Reg_BK = 0;
    //u32 Mopg_Gwin_Reg_Shift = 0;
    //u32 Mopg_Gwin_Reg_Base = 0;
    u32 u32RegForSwShd = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    //Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    /*Mopg_Gwin_Reg_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_01_L:
                                                                        REG_DISP_MOPG_BK1_01_L;*/
    u32RegVal = (u8Offset<<9);

    u32RegForSwShd = _HalDispMopgGwinRegSwShadowMap(eMopgId);

    _HalMopSwShadowHandler(Mopg_Gwin_Reg_BK, u32RegForSwShd, (u32RegVal & 0x1E00), 0x1E00);

    //W2BYTEMSK(Mopg_Gwin_Reg_Base + Mopg_Gwin_Reg_Shift, (u32RegVal & 0x1E00), 0x1E00);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopgSetCAddr(HalDispMopgGwinId_e eMopgId, u32 u32PhyAddr)
{
    u32 u32RegVal;
    u32 Mopg_Gwin_Reg_BK = 0;
    u32 Mopg_Gwin_Reg_Shift = 0;
    u32 Mopg_Gwin_Reg_Base = 0;
    u32 Mopg_Gwin_Reg_Base2 = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    Mopg_Gwin_Reg_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_04_L:
                                                                        REG_DISP_MOPG_BK1_04_L;
    Mopg_Gwin_Reg_Base2 = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_05_L:
                                                                         REG_DISP_MOPG_BK1_05_L;
    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(Mopg_Gwin_Reg_Base + Mopg_Gwin_Reg_Shift, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(Mopg_Gwin_Reg_Base2 + Mopg_Gwin_Reg_Shift, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopgSetGwinParam(HalDispMopgGwinId_e eMopgId, u16 u16Hst, u16 u16Vst, u16 u16Width, u16 u16Height)
{
    u16 u16Hend;
    u16 u16Vend;
    u32 Mopg_Gwin_Reg_BK = 0;
    u32 Mopg_Gwin_Reg_Shift = 0;
    u32 Mopg_Gwin_Reg_Hst_Base = 0;
    u32 Mopg_Gwin_Reg_Hend_Base = 0;
    u32 Mopg_Gwin_Reg_Vst_Base = 0;
    u32 Mopg_Gwin_Reg_Vend_Base = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    Mopg_Gwin_Reg_Hst_Base  = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_06_L:
                                                                             REG_DISP_MOPG_BK1_06_L;
    Mopg_Gwin_Reg_Hend_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_07_L:
                                                                             REG_DISP_MOPG_BK1_07_L;
    Mopg_Gwin_Reg_Vst_Base  = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_08_L:
                                                                             REG_DISP_MOPG_BK1_08_L;
    Mopg_Gwin_Reg_Vend_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_09_L:
                                                                             REG_DISP_MOPG_BK1_09_L;
    u16Hend = u16Hst + u16Width -1;
    u16Vend = u16Vst + u16Height -1;
    W2BYTEMSK(Mopg_Gwin_Reg_Hst_Base  + Mopg_Gwin_Reg_Shift, u16Hst,  0x1FFF);
    W2BYTEMSK(Mopg_Gwin_Reg_Hend_Base + Mopg_Gwin_Reg_Shift, u16Hend, 0x1FFF);
    W2BYTEMSK(Mopg_Gwin_Reg_Vst_Base  + Mopg_Gwin_Reg_Shift, u16Vst,  0x1FFF);
    W2BYTEMSK(Mopg_Gwin_Reg_Vend_Base + Mopg_Gwin_Reg_Shift, u16Vend, 0x1FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopgSetPitch(HalDispMopgGwinId_e eMopgId, u16 u16Pitch)
{
    u16 u16RegVal;
    u32 Mopg_Gwin_Reg_BK = 0;
    u32 Mopg_Gwin_Reg_Shift = 0;
    u32 Mopg_Gwin_Reg_Base = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    Mopg_Gwin_Reg_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_0A_L:
                                                                        REG_DISP_MOPG_BK1_0A_L;
    u16RegVal = u16Pitch >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(Mopg_Gwin_Reg_Base + Mopg_Gwin_Reg_Shift, u16RegVal, 0x1FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopgSetSourceParam(HalDispMopgGwinId_e eMopgId, u16 u16SrcWidth, u16 u16SrcHeight)
{
    u32 Mopg_Gwin_Reg_BK = 0;
    u32 Mopg_Gwin_Reg_Shift = 0;
    u32 Mopg_Gwin_Reg_SrcWidth_Base = 0;
    u32 Mopg_Gwin_Reg_SrcHeight_Base = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    Mopg_Gwin_Reg_SrcWidth_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_0B_L:
                                                                                 REG_DISP_MOPG_BK1_0B_L;
    Mopg_Gwin_Reg_SrcHeight_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_0C_L:
                                                                                  REG_DISP_MOPG_BK1_0C_L;
    W2BYTEMSK(Mopg_Gwin_Reg_SrcWidth_Base  + Mopg_Gwin_Reg_Shift, (u16SrcWidth-1),  0x1FFF);
    W2BYTEMSK(Mopg_Gwin_Reg_SrcHeight_Base + Mopg_Gwin_Reg_Shift, (u16SrcHeight-1), 0x1FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopgSetHScaleFac(HalDispMopgGwinId_e eMopgId, u16 u16HRatio)
{
    u32 Mopg_Gwin_Reg_BK = 0;
    u32 Mopg_Gwin_Reg_Shift = 0;
    u32 Mopg_Gwin_Reg_Base = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    Mopg_Gwin_Reg_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_0D_L:
                                                                        REG_DISP_MOPG_BK1_0D_L;
    W2BYTEMSK(Mopg_Gwin_Reg_Base + Mopg_Gwin_Reg_Shift, u16HRatio,  0x1FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopgSetVScaleFac(HalDispMopgGwinId_e eMopgId, u16 u16VRatio)
{
    u32 Mopg_Gwin_Reg_BK = 0;
    u32 Mopg_Gwin_Reg_Shift = 0;
    u32 Mopg_Gwin_Reg_Base = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    Mopg_Gwin_Reg_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_0E_L:
                                                                        REG_DISP_MOPG_BK1_0E_L;
    W2BYTEMSK(Mopg_Gwin_Reg_Base + Mopg_Gwin_Reg_Shift, u16VRatio,  0x1FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopgSetRblkHstr(HalDispMopgGwinId_e eMopgId, u8 u8Hshift)
{
    u32 Mopg_Gwin_Reg_BK = 0;
    //u32 Mopg_Gwin_Reg_Shift = 0;
    //u32 Mopg_Gwin_Reg_Base = 0;
    u32 u32RegForSwShd = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    //Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    /*Mopg_Gwin_Reg_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_01_L:
                                                                        REG_DISP_MOPG_BK1_01_L;
*/
    u32RegForSwShd = _HalDispMopgGwinRegSwShadowMap(eMopgId);
    _HalMopSwShadowHandler(Mopg_Gwin_Reg_BK, u32RegForSwShd, (u8Hshift<<9), 0x0E00);
    //W2BYTEMSK(Mopg_Gwin_Reg_Base + Mopg_Gwin_Reg_Shift, (u8Hshift<<9),  0x0E00);

    //set double buffer write
    HalDispMopDbBfWr();
}


void HalDispMopgSetLineBufStr(HalDispMopgGwinId_e eMopgId, u16 u16GwinHstr, u8 u8WinPri)
{
    u32 Mopg_Gwin_Reg_BK = 0;
    //u32 Mopg_Gwin_Reg_Shift = 0;
    //u32 Mopg_Gwin_Reg_Base = 0;
    u16 u16RegVal = 0;
    u32 u32RegForSwShd = 0;
    Mopg_Gwin_Reg_BK = _HalDispMopgGwinRegBkMap(eMopgId);
    //Mopg_Gwin_Reg_Shift = _HalDispMopgGwinRegShiftMap(eMopgId);
    /*Mopg_Gwin_Reg_Base = (Mopg_Gwin_Reg_BK == REG_DISP_MOPG_BK2_BASE) ? REG_DISP_MOPG_BK2_01_L:
                                                                        REG_DISP_MOPG_BK1_01_L;
*/
    u32RegForSwShd = _HalDispMopgGwinRegSwShadowMap(eMopgId);
    if(u8WinPri==0)
    {
        u16RegVal=0;
    }
    else
    {
        u16RegVal = ((u16GwinHstr>>3) + 2*u8WinPri) & 0xFF;//lb_str = gwin_hstr/8 + 2*gwin_priority; gwin_priority is the order of the position of display(from left on) 0~15
    }

    _HalMopSwShadowHandler(Mopg_Gwin_Reg_BK, u32RegForSwShd, u16RegVal, 0x00FF);
    //W2BYTEMSK(Mopg_Gwin_Reg_Base + Mopg_Gwin_Reg_Shift, u16RegVal,  0x00FF);

    //set double buffer write
    HalDispMopDbBfWr();
}


///MOPS
void HalDispMopsSwReset(void)
{
    W2BYTEMSK(REG_DISP_MOPS_BK0_00_L, 0x0001, 0x0001);
    W2BYTEMSK(REG_DISP_MOPS_BK0_00_L, 0x0000, 0x0001);
}

void HalDispMopsSetAutoStretchWinSizeEn(bool bEnAuto)//0: use reg_gw_hsize/vsize; 1: use display size
{
    W2BYTEMSK(REG_DISP_MOPS_BK0_01_L, bEnAuto ? DISP_BIT10 : 0, DISP_BIT10);
}

void HalDispMopsSetStretchWinSize(u16 u16Width, u16 u16Height)
{
    W2BYTEMSK(REG_DISP_MOPS_BK0_07_L, u16Width, 0x1FFF);//reg_gw_hsize
    W2BYTEMSK(REG_DISP_MOPS_BK0_08_L, u16Height, 0x1FFF);//reg_gw_vsize
}

void HalDispMopsSetPipeDelay(u8 u8PipeDelay)
{
    W2BYTEMSK(REG_DISP_MOPS_BK0_03_L, u8PipeDelay, 0x001F);//reg_gw_hsize
}

void HalDispMopsInit(void)
{
    HalDispMopsSwReset();
    W2BYTEMSK(REG_DISP_MOPS_BK0_01_L, DISP_BIT8, DISP_BIT8);//clk_miu. 0:use free run; 1:use gated clock
    W2BYTEMSK(REG_DISP_MOPS_BK0_01_L, DISP_BIT9, DISP_BIT9);//clk_gop. 0:use free run; 1:use gated clock
    HalDispMopsSetAutoStretchWinSizeEn(TRUE);
    W2BYTEMSK(REG_DISP_MOPS_BK0_01_L, DISP_BIT11, DISP_BIT11);//auto blanking. 0:use reg h15; 1:use display blanking
    HalDispMopsSetPipeDelay(0xA);//set pipe delay (default=0xA)
    W2BYTEMSK(REG_DISP_MOPS_BK0_04_L, 0xF8, 0x00FF);//h04 [7:0]ymd_thd
    W2BYTEMSK(REG_DISP_MOPS_BK0_05_L, 0xD0, 0x00FF);//h05 [7:0]cdma_thd; default=0xf8, use 0xd0 for patch
    W2BYTEMSK(REG_DISP_MOPS_BK0_06_L, 0xF0, 0x00FF);//h06 Luma DMA priority [7]yhp_en [6:0] yhp_thd
    W2BYTEMSK(REG_DISP_MOPS_BK0_0B_L, 0xF0, 0x00FF);//h0b Chroma DMA priority [7]yhp_en [6:0] yhp_thd
    W2BYTEMSK(REG_DISP_MOPS_BK0_15_L, 0x80, 0x1FFF);//h15 [12:0]gw_hext

    W2BYTEMSK(REG_DISP_MOPS_BK0_40_L, 0x0689, 0x07FF);//h40 set 4tap

    //set double buffer write
    HalDispMopDbBfWr();

}

void HalDispMopsGwinEn(bool bEn)
{
    W2BYTEMSK(REG_DISP_MOPS_BK0_20_L, bEn ? DISP_BIT0 : 0, DISP_BIT0);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopsSetYAddr(u32 u32PhyAddr)
{
    u32 u32RegVal;
    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOPS_BK0_22_L, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(REG_DISP_MOPS_BK0_23_L, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}
void HalDispMopsSetAddr16Offset(u8 u8Offset)
{
    u32 u32RegVal;

    u32RegVal = (u8Offset<<9);
    W2BYTEMSK(REG_DISP_MOPS_BK0_21_L, (u32RegVal & 0x1E00), 0x1E00);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopsSetCAddr(u32 u32PhyAddr)
{
    u32 u32RegVal;
    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOPS_BK0_24_L, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(REG_DISP_MOPS_BK0_25_L, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopsSetGwinParam(u16 u16Hst, u16 u16Vst, u16 u16Width, u16 u16Height)
{
    u16 u16Hend;
    u16 u16Vend;
    u16Hend = u16Hst + u16Width -1;
    u16Vend = u16Vst + u16Height -1;
    W2BYTEMSK(REG_DISP_MOPS_BK0_26_L, u16Hst,  0x1FFF);
    W2BYTEMSK(REG_DISP_MOPS_BK0_27_L, u16Hend, 0x1FFF);
    W2BYTEMSK(REG_DISP_MOPS_BK0_28_L, u16Vst,  0x1FFF);
    W2BYTEMSK(REG_DISP_MOPS_BK0_29_L, u16Vend, 0x1FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopsSetPitch(u16 u16Pitch)
{
    u16 u16RegVal;
    u16RegVal = u16Pitch >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOPS_BK0_2A_L, u16RegVal, 0x1FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopsSetSourceParam(u16 u16SrcWidth, u16 u16SrcHeight)
{
    W2BYTEMSK(REG_DISP_MOPS_BK0_2B_L, (u16SrcWidth-1),  0x1FFF);
    W2BYTEMSK(REG_DISP_MOPS_BK0_2C_L, (u16SrcHeight-1), 0x1FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopsSetHScaleFac(u16 u16HRatio)
{
    W2BYTEMSK(REG_DISP_MOPS_BK0_2D_L, u16HRatio,  0x1FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopsSetVScaleFac(u16 u16VRatio)
{
    W2BYTEMSK(REG_DISP_MOPS_BK0_2E_L, u16VRatio,  0x1FFF);

    //set double buffer write
    HalDispMopDbBfWr();
}

void HalDispMopsSetRblkHstr(u8 u8Hshift)
{
    W2BYTEMSK(REG_DISP_MOPS_BK0_21_L, (u8Hshift<<8),  0x0F00);

    //set double buffer write
    HalDispMopDbBfWr();
}


// Set ClkRate
void HalDispMopSetClk(bool bEn, u32 u32ClkRate)
{
    u16 u16RegVal;

    u16RegVal = (u32ClkRate <= CLK_MHZ(288)) ? (0x02 << 2) :
                (u32ClkRate <= CLK_MHZ(320)) ? (0x00 << 2) :
                (u32ClkRate <= CLK_MHZ(384)) ? (0x01 << 2) :
                                               (0x03 << 2);


    u16RegVal |= bEn ? 0x0000 : 0x0001;

    W2BYTEMSK(REG_CLKGEN_54_L, u16RegVal, 0x000F);
}

// Get ClkRate
void HalDispMopGetClk(bool *pbEn, u32 *pu32ClkRate)
{
    u16 u16RegVal;

    u16RegVal = R2BYTE(REG_CLKGEN_54_L);

    *pbEn = u16RegVal & 0x0001 ? 0 : 1;

    *pu32ClkRate = ((u16RegVal & 0x0C) >> 2) == 0x00 ? CLK_MHZ(320) :
                   ((u16RegVal & 0x0C) >> 2) == 0x01 ? CLK_MHZ(384) :
                   ((u16RegVal & 0x0C) >> 2) == 0x02 ? CLK_MHZ(288) :
                                                       0;
}

///MOP common
void HalDispMopDbBfWr(void)
{
    W2BYTEMSK(REG_DISP_MOPG_BK0_7F_L, DISP_BIT8, DISP_BIT8);
    W2BYTEMSK(REG_DISP_MOPG_BK0_7F_L, 0, DISP_BIT8);
}

//MOP rotate
void HalDispMopRotDbBfWr(HalDispMopRotId_e eMopRotId)
{
    u16 u16RegVal;

    u16RegVal = (eMopRotId == E_HAL_DISP_MOPROT_ROT0_ID) ? DISP_BIT2:
                (eMopRotId == E_HAL_DISP_MOPROT_ROT1_ID) ? DISP_BIT3:
                                                           DISP_BIT2;

    W2BYTEMSK(REG_DISP_MOP_ROT_01_L, u16RegVal, u16RegVal);
    W2BYTEMSK(REG_DISP_MOP_ROT_01_L, 0, u16RegVal);
}

void HalDispMopRot0DbfEn(bool bEn)
{
    W2BYTEMSK(REG_DISP_MOP_ROT_01_L, bEn ? DISP_BIT0 : 0, DISP_BIT0);
}

void HalDispMopRot1DbfEn(bool bEn)
{
    W2BYTEMSK(REG_DISP_MOP_ROT_01_L, bEn ? DISP_BIT1 : 0, DISP_BIT1);
}

void HalDispMopRot0En(bool bEn)
{
    W2BYTEMSK(REG_DISP_MOPS_BK0_0A_L, bEn ? DISP_BIT0 : 0, DISP_BIT0);
    W2BYTEMSK(REG_DISP_MOP_ROT_10_L, bEn ? DISP_BIT15 : 0, DISP_BIT15);

    HalDispMopDbBfWr();
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT0_ID);
}

void HalDispMopRot1En(bool bEn)
{
    //W2BYTEMSK(REG_DISP_MOPS_BK0_51_L, bEn ? DISP_BIT0 : 0, DISP_BIT0);
    _HalMopSwShadowHandler(REG_DISP_MOPS_BK0_BASE, E_HAL_DISP_LAYER_MOPS_51, bEn ? DISP_BIT0 : 0, DISP_BIT0);
    W2BYTEMSK(REG_DISP_MOP_ROT_30_L, bEn ? DISP_BIT15 : 0, DISP_BIT15);

    HalDispMopDbBfWr();
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT1_ID);
}

void HalDispMopRot0SourceWidth(u16 u16inputWidth)
{
    //Y
    W2BYTEMSK(REG_DISP_MOP_ROT_10_L, u16inputWidth,  0x07FF);
    //C
    W2BYTEMSK(REG_DISP_MOP_ROT_20_L, (u16inputWidth>>1),  0x07FF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT0_ID);
}

void HalDispMopRot0SourceHeight(u16 u16inputHeight)
{
    //Y
    W2BYTEMSK(REG_DISP_MOP_ROT_11_L, u16inputHeight, 0x07FF);
    //C
    W2BYTEMSK(REG_DISP_MOP_ROT_21_L, (u16inputHeight>>1), 0x07FF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT0_ID);
}

void HalDispMopRot1SourceWidth(u16 u16inputWidth)
{
    //Y
    W2BYTEMSK(REG_DISP_MOP_ROT_30_L, u16inputWidth,  0x07FF);
    //C
    W2BYTEMSK(REG_DISP_MOP_ROT_40_L, (u16inputWidth>>1),  0x07FF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT1_ID);
}

void HalDispMopRot1SourceHeight(u16 u16inputHeight)
{
    //Y
    W2BYTEMSK(REG_DISP_MOP_ROT_31_L, u16inputHeight, 0x07FF);
    //C
    W2BYTEMSK(REG_DISP_MOP_ROT_41_L, (u16inputHeight>>1), 0x07FF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT1_ID);
}


void HalDispMopRot0SetReadYAddr(u32 u32PhyAddr)
{
    u32 u32RegVal;

    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOP_ROT_14_L, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(REG_DISP_MOP_ROT_15_L, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT0_ID);

}

void HalDispMopRot0SetReadCAddr(u32 u32PhyAddr)
{
    u32 u32RegVal;

    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOP_ROT_24_L, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(REG_DISP_MOP_ROT_25_L, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT0_ID);

}

void HalDispMopRot1SetReadYAddr(u32 u32PhyAddr)
{
    u32 u32RegVal;

    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOP_ROT_34_L, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(REG_DISP_MOP_ROT_35_L, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT1_ID);

}

void HalDispMopRot1SetReadCAddr(u32 u32PhyAddr)
{
    u32 u32RegVal;

    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOP_ROT_44_L, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(REG_DISP_MOP_ROT_45_L, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT1_ID);

}

void HalDispMopRot0SetWriteYAddr(u32 u32PhyAddr)
{
    u32 u32RegVal;

    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOP_ROT_17_L, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(REG_DISP_MOP_ROT_18_L, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT0_ID);

}

void HalDispMopRot0SetWriteCAddr(u32 u32PhyAddr)
{
    u32 u32RegVal;

    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOP_ROT_27_L, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(REG_DISP_MOP_ROT_28_L, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT0_ID);

}

void HalDispMopRot1SetWriteYAddr(u32 u32PhyAddr)
{
    u32 u32RegVal;

    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOP_ROT_37_L, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(REG_DISP_MOP_ROT_38_L, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT1_ID);

}

void HalDispMopRot1SetWriteCAddr(u32 u32PhyAddr)
{
    u32 u32RegVal;

    u32RegVal = u32PhyAddr >> HAL_DISP_MOP_MIU_BUS;
    W2BYTEMSK(REG_DISP_MOP_ROT_47_L, (u32RegVal & 0xFFFF), 0xFFFF);
    W2BYTEMSK(REG_DISP_MOP_ROT_48_L, ((u32RegVal>>16) & 0x0FFF), 0x0FFF);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT1_ID);

}

void HalDispMopRot0SetRotateMode(HalDispRotateMode_e eRotAng)
{
    if((eRotAng==E_HAL_DISP_ROTATE_90)||
       (eRotAng==E_HAL_DISP_ROTATE_270)||
       (eRotAng==E_HAL_DISP_ROTATE_NONE))
    {
        //W2BYTEMSK(REG_DISP_MOPS_BK0_51_L, (eRotAng == E_HAL_DISP_ROTATE_270)? DISP_BIT1 : 0,  DISP_BIT1);
        _HalMopSwShadowHandler(REG_DISP_MOPS_BK0_BASE, E_HAL_DISP_LAYER_MOPS_51, (eRotAng == E_HAL_DISP_ROTATE_270)? DISP_BIT1 : 0, DISP_BIT1);
        //trigger dbf write
        //HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT0_ID);
        HalDispMopDbBfWr();
    }
    else
    {
        DISP_ERR("[HALMOP]%s %d, Rotate ID %d not support\n", __FUNCTION__, __LINE__,eRotAng);
    }
}

void HalDispMopRot1SetRotateMode(HalDispRotateMode_e eRotAng)
{
    if((eRotAng==E_HAL_DISP_ROTATE_90)||
       (eRotAng==E_HAL_DISP_ROTATE_270)||
       (eRotAng==E_HAL_DISP_ROTATE_NONE))
    {
        //W2BYTEMSK(REG_DISP_MOPS_BK0_51_L, (eRotAng == E_HAL_DISP_ROTATE_270)? DISP_BIT2 : 0,  DISP_BIT2);
        _HalMopSwShadowHandler(REG_DISP_MOPS_BK0_BASE, E_HAL_DISP_LAYER_MOPS_51, (eRotAng == E_HAL_DISP_ROTATE_270)? DISP_BIT2 : 0, DISP_BIT2);
        //trigger dbf write
        //HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT1_ID);
        HalDispMopDbBfWr();
    }
    else
    {
        DISP_ERR("[HALMOP]%s %d, Rotate ID %d not support\n", __FUNCTION__, __LINE__,eRotAng);
    }
}

void HalDispMopRot0SetPixDummy(u16 u16DummyPix)
{
    //Y
    W2BYTEMSK(REG_DISP_MOP_ROT_16_L, (u16DummyPix<<12), 0xF000);
    //C
    W2BYTEMSK(REG_DISP_MOP_ROT_26_L, ((u16DummyPix/2)<<12), 0x7000);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT0_ID);
}

void HalDispMopRot1SetPixDummy(u16 u16DummyPix)
{
    //Y
    W2BYTEMSK(REG_DISP_MOP_ROT_36_L, (u16DummyPix<<12), 0xF000);
    //C
    W2BYTEMSK(REG_DISP_MOP_ROT_46_L, ((u16DummyPix/2)<<12), 0x7000);

    //trigger dbf write
    HalDispMopRotDbBfWr(E_HAL_DISP_MOPROT_ROT1_ID);
}

void HalDispMopRotInit(void)
{
    //rot0 Y
    W2BYTEMSK(REG_DISP_MOP_ROT_12_L, 0x0820, 0x3F3F);//[13:8]reg_rot0_rdma_length_y=0x8; [5:0]reg_rot0_rdma_thd_y=0x20
    W2BYTEMSK(REG_DISP_MOP_ROT_13_L, 0xA01F, 0xBFFF);//[15]reg_rot0_rdma_hp_en_y=0x1; [13:8]reg_rot0_rdma_hp_thd_y=0x20; [7:0]reg_rot0_rdma_stop_y=0x1F
    W2BYTEMSK(REG_DISP_MOP_ROT_16_L, 0x0801, 0x0F07);//[11:8]reg_rot0_burst_wr_thd_y=0x8; [7:0]reg_rot0_pre_rdy_thd_y=0x1

    //rot0 C
    W2BYTEMSK(REG_DISP_MOP_ROT_22_L, 0x0820, 0x3F3F);//[13:8]reg_rot0_rdma_length_c=0x8; [5:0]reg_rot0_rdma_thd_c=0x20
    W2BYTEMSK(REG_DISP_MOP_ROT_23_L, 0xA01F, 0xBFFF);//[15]reg_rot0_rdma_hp_en_c=0x1; [13:8]reg_rot0_rdma_hp_thd_c=0x20; [7:0]reg_rot0_rdma_stop_c=0x1F
    W2BYTEMSK(REG_DISP_MOP_ROT_26_L, 0x0801, 0x0F07);//[11:8]reg_rot0_burst_wr_thd_c=0x8; [7:0]reg_rot0_pre_rdy_thd_c=0x1

    //rot1 Y
    W2BYTEMSK(REG_DISP_MOP_ROT_32_L, 0x0820, 0x3F3F);//[13:8]reg_rot1_rdma_length_y=0x8; [5:0]reg_rot1_rdma_thd_y=0x20
    W2BYTEMSK(REG_DISP_MOP_ROT_33_L, 0xA01F, 0xBFFF);//[15]reg_rot1_rdma_hp_en_y=0x1; [13:8]reg_rot1_rdma_hp_thd_y=0x20; [7:0]reg_rot1_rdma_stop_y=0x1F
    W2BYTEMSK(REG_DISP_MOP_ROT_36_L, 0x0801, 0x0F07);//[11:8]reg_rot1_burst_wr_thd_y=0x8; [7:0]reg_rot1_pre_rdy_thd_y=0x1

    //rot1 C
    W2BYTEMSK(REG_DISP_MOP_ROT_42_L, 0x0820, 0x3F3F);//[13:8]reg_rot1_rdma_length_c=0x8; [5:0]reg_rot1_rdma_thd_c=0x20
    W2BYTEMSK(REG_DISP_MOP_ROT_43_L, 0xA01F, 0xBFFF);//[15]reg_rot1_rdma_hp_en_c=0x1; [13:8]reg_rot1_rdma_hp_thd_c=0x20; [7:0]reg_rot1_rdma_stop_c=0x1F
    W2BYTEMSK(REG_DISP_MOP_ROT_46_L, 0x0801, 0x0F07);//[11:8]reg_rot1_burst_wr_thd_c=0x8; [7:0]reg_rot1_pre_rdy_thd_c=0x1

    //open double buffer write
    HalDispMopRot0DbfEn(TRUE);
    HalDispMopRot1DbfEn(TRUE);
}

