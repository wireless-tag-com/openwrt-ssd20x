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

#define __HAL_GOP_C__
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "hal_gop_reg.h"
#include "hal_gop.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


#define PARSING_ALPHA_TYPE(bConst)(\
                                   bConst == 0 ? "PIXEL_ALPHA": \
                                   bConst == 1 ? "CONST_ALPHA": \
                                   "Wrong_Alpha_Type")

#define PARSING_DISPLAY_MODE(x) (\
                                 x == E_HAL_GOP_DISPLAY_MODE_INTERLACE ? "INTERLACE" : \
                                 x == E_HAL_GOP_DISPLAY_MODE_PROGRESS  ? "PROGRESS"  : \
                                 "UNKNOWN")

#define PARSING_OUT_MODE(x) (\
                             x == E_HAL_GOP_OUT_FMT_RGB ? "RGB" : \
                             x == E_HAL_GOP_OUT_FMT_YUV ? "YUV" : \
                             "UNKNOWIN")

#define PARSING_SRC_FMT(x) (\
                            x == E_HAL_GOP_GWIN_SRC_RGB1555    ? "RGB1555"  :   \
                            x == E_HAL_GOP_GWIN_SRC_RGB565     ? "RGB565"   :   \
                            x == E_HAL_GOP_GWIN_SRC_ARGB4444   ? "ARGB4444" :   \
                            x == E_HAL_GOP_GWIN_SRC_2266       ? "2266"     :   \
                            x == E_HAL_GOP_GWIN_SRC_I8_PALETTE ? "I8_PALETTE" : \
                            x == E_HAL_GOP_GWIN_SRC_ARGB8888   ? "ARGB8888" :   \
                            x == E_HAL_GOP_GWIN_SRC_ARGB1555   ? "ARGB1555" :   \
                            x == E_HAL_GOP_GWIN_SRC_ABGR8888   ? "ABGR8888" :   \
                            x == E_HAL_GOP_GWIN_SRC_UV7Y8      ? "YV7Y8"    :   \
                            x == E_HAL_GOP_GWIN_SRC_UV8Y8      ? "UV8Y8"    :   \
                            x == E_HAL_GOP_GWIN_SRC_RGBA5551   ? "TGBA5551" :   \
                            x == E_HAL_GOP_GWIN_SRC_RGBA4444   ? "RGBA444"  :   \
                            x == E_HAL_GOP_GWIN_SRC_I4_PALETTE ? "I4_PALETTE" : \
                            x == E_HAL_GOP_GWIN_SRC_I2_PALETTE ? "I2_PALETTE" : \
                            "UNKNOWN")

#define HAL_GOP_PIX2BYTE(x, fmt)     (fmt)==E_HAL_GOP_GWIN_SRC_ARGB8888     || \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_ABGR8888     ? (x) * 4 : \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_RGB1555      || \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_RGB565       || \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_ARGB4444     || \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_ARGB1555     || \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_RGBA5551     || \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_RGBA4444     || \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_2266         || \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_UV7Y8        || \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_UV8Y8        ? (x) * 2 : \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_I8_PALETTE   ? (x) * 1 : \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_I4_PALETTE   ? (x) / 2 : \
                                     (fmt)==E_HAL_GOP_GWIN_SRC_I2_PALETTE   ? (x) / 4 : \
                                                                              (x)

#define IsHalGOPBaseIdType_0(x)          (x == REG_ISPGOP_00_BASE)
#define IsHalGOPBaseIdType_1(x)          (x == REG_ISPGOP_10_BASE)
#define IsHalGOPBaseIdType_all(x)       (IsHalGOPBaseIdType_0(x) ||IsHalGOPBaseIdType_1(x))
#define IsHalGWINBaseIdType_0(x)          (x == REG_ISPGOP_01_BASE)
#define IsHalGWINBaseIdType_1(x)          (x == REG_ISPGOP_11_BASE)
#define IsHalGWINBaseIdType_all(x)       (IsHalGWINBaseIdType_0(x) ||IsHalGWINBaseIdType_1(x))
#define GOP_WORD_BASE 0
#define GOP_PIXEL_BASE 1
#define GOP_PIPE_DELAY(x) 0

//if set 1, use cmdq to set register, if set 0, use cpu to set register
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
HalGopWrRegType_e _eGopWrRegMd = E_HAL_GOP_ALLBANK_DOUBLE_WR;
#if !(RIU_32_EN)
u16 u16GopShadow[HAL_RGN_GOP_NUM][E_HAL_GOP_SHADOW_Num];
u16 u16GwinShadow[HAL_RGN_GOP_NUM][HAL_RGN_GOP_GWIN_NUM][E_HAL_GWIN_SHADOW_Num];
u16 u16GopShadowReg[E_HAL_GOP_SHADOW_Num] = {REG_GOP_11,REG_GOP_12,REG_GOP_20,REG_GOP_24,REG_GOP_28};
u16 u16GwinShadowReg[E_HAL_GWIN_SHADOW_Num] = {REG_GOP_00,REG_GOP_08};
#endif
//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
bool _HalGopIdTransCmdqID(HalGopIdType_e eGopId,HalRgnCmdqIdType_e *pCmdqId)
{
    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
        case E_HAL_ISPGOP_ID_01:
        case E_HAL_ISPGOP_ID_0_ST:
        case E_HAL_ISPGOP_ID_10:
        case E_HAL_ISPGOP_ID_11:
        case E_HAL_ISPGOP_ID_1_ST:
            *pCmdqId = E_HAL_RGN_CMDQ_VPE_ID_0;
            return TRUE;
        default:
            *pCmdqId =  E_HAL_RGN_CMDQ_ID_NUM;
            HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
            break;
    }
    return FALSE;
}
HalGopGwinSrcFormatRealVal_e _HalGopSrcFmtTransRealVal(HalGopIdType_e eGopId, HalGopGwinSrcFormat_e eSrcFmt)
{
    HalGopGwinSrcFormatRealVal_e eVal = E_HAL_GOP_GWIN_SRC_VAL_NOTSUPPORT;
    if((eGopId == E_HAL_ISPGOP_ID_00)||(eGopId == E_HAL_ISPGOP_ID_01))
    {
        switch(eSrcFmt)
        {
            case E_HAL_GOP_GWIN_SRC_I8_PALETTE:
                eVal = E_HAL_GOP00_GWIN_SRC_I8_PALETTE;
                break;
            case E_HAL_GOP_GWIN_SRC_I4_PALETTE:
                eVal = E_HAL_GOP00_GWIN_SRC_I4_PALETTE;
                break;
            case E_HAL_GOP_GWIN_SRC_I2_PALETTE:
                eVal = E_HAL_GOP00_GWIN_SRC_I2_PALETTE;
                break;
            case E_HAL_GOP_GWIN_SRC_ARGB1555:
                eVal = E_HAL_GOP00_GWIN_SRC_ARGB1555;
                break;
            case E_HAL_GOP_GWIN_SRC_ARGB4444:
                eVal = E_HAL_GOP00_GWIN_SRC_ARGB4444;
                break;
            default:
                eVal = E_HAL_GOP_GWIN_SRC_VAL_NOTSUPPORT;
                HALRGNERR("[GOP]%s %d: GOPId=0x%x %s not support\n", __FUNCTION__, __LINE__, eGopId, PARSING_SRC_FMT(eSrcFmt));
                break;
        }
    }
    else if((eGopId == E_HAL_ISPGOP_ID_10)||(eGopId == E_HAL_ISPGOP_ID_11))
    {
        switch(eSrcFmt)
        {
            case E_HAL_GOP_GWIN_SRC_I8_PALETTE:
                eVal = E_HAL_GOP10_GWIN_SRC_I8_PALETTE;
                break;
            case E_HAL_GOP_GWIN_SRC_I4_PALETTE:
                eVal = E_HAL_GOP10_GWIN_SRC_I4_PALETTE;
                break;
            case E_HAL_GOP_GWIN_SRC_I2_PALETTE:
                eVal = E_HAL_GOP10_GWIN_SRC_I2_PALETTE;
                break;
            case E_HAL_GOP_GWIN_SRC_ARGB1555:
                eVal = E_HAL_GOP10_GWIN_SRC_ARGB1555;
                break;
            case E_HAL_GOP_GWIN_SRC_ARGB4444:
                eVal = E_HAL_GOP10_GWIN_SRC_ARGB4444;
                break;
            case E_HAL_GOP_GWIN_SRC_RGB1555:
                eVal = E_HAL_GOP10_GWIN_SRC_RGB1555;
                break;
            case E_HAL_GOP_GWIN_SRC_RGB565:
                eVal = E_HAL_GOP10_GWIN_SRC_RGB565;
                break;
            case E_HAL_GOP_GWIN_SRC_2266:
                eVal = E_HAL_GOP10_GWIN_SRC_2266;
                break;
            case E_HAL_GOP_GWIN_SRC_ARGB8888:
                eVal = E_HAL_GOP10_GWIN_SRC_ARGB8888;
                break;
            case E_HAL_GOP_GWIN_SRC_ABGR8888:
                eVal = E_HAL_GOP10_GWIN_SRC_ABGR8888;
                break;
            case E_HAL_GOP_GWIN_SRC_UV7Y8:
                eVal = E_HAL_GOP10_GWIN_SRC_UV7Y8;
                break;
            case E_HAL_GOP_GWIN_SRC_UV8Y8:
                eVal = E_HAL_GOP10_GWIN_SRC_UV8Y8;
                break;
            case E_HAL_GOP_GWIN_SRC_RGBA5551:
                eVal = E_HAL_GOP10_GWIN_SRC_RGBA5551;
                break;
            case E_HAL_GOP_GWIN_SRC_RGBA4444:
                eVal = E_HAL_GOP10_GWIN_SRC_RGBA4444;
                break;
            default:
                eVal = E_HAL_GOP_GWIN_SRC_VAL_NOTSUPPORT;
                HALRGNERR("[GOP]%s %d: GOPId=0x%x %s not support\n", __FUNCTION__, __LINE__, eGopId, PARSING_SRC_FMT(eSrcFmt));
                break;
        }
    }
    else
    {
        eVal = E_HAL_GOP_GWIN_SRC_NOTSUPPORT;
        HALRGNERR("[GOP]%s %d: wrong GOPId=0x%x\n", __FUNCTION__, __LINE__,eGopId);
    }
    return eVal;
}

u16 _HalGopGetDoubleRW(HalGopIdType_e eGopId)
{
    u16 u16DoubleRW;
    if(_eGopWrRegMd==E_HAL_GOP_FORCE_WR)
    {
        u16DoubleRW = GOP_BANK_FORCE_WR;
    }
    else if(_eGopWrRegMd==GOP_BANK_ALLBANK_DOUBLE_WR)
    {
        u16DoubleRW = GOP_BANK_ALL_DOUBLE_WR_G00;
    }
    else
    {
        u16DoubleRW = GOP_BANK_DOUBLE_WR_G00;
    }
    return u16DoubleRW;

}
void _HalGopWriteDoubleBuffer(u32 GOP_Reg_Base, HalGopIdType_e eGopId,HalRgnCmdqIdType_e eCmdqId)
{
    u16 GOP_Reg_DB = 0;
    GOP_Reg_DB = _HalGopGetDoubleRW(eGopId);
    // Double Buffer Write
    HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
    HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_Reg_DB, (GOP_BANK_WR_SEL_MSK | GOP_BANK_SEL_MSK),eCmdqId);
    HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0, GOP_BANK_WR_SEL_MSK,eCmdqId);
}

u32 _HalGopGetBaseAddr(HalGopIdType_e eGopId)
{
    u32 u32Base = REG_ISPGOP_00_BASE;

    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
            u32Base = REG_ISPGOP_00_BASE;
            break;
        case E_HAL_ISPGOP_ID_01:
            u32Base = REG_ISPGOP_01_BASE;
            break;
        case E_HAL_ISPGOP_ID_0_ST:
            u32Base = REG_ISPGOP_0ST_BASE;
            break;
        case E_HAL_ISPGOP_ID_10:
            u32Base = REG_ISPGOP_10_BASE;
            break;
        case E_HAL_ISPGOP_ID_11:
            u32Base = REG_ISPGOP_11_BASE;
            break;
        case E_HAL_ISPGOP_ID_1_ST:
            u32Base = REG_ISPGOP_1ST_BASE;
            break;
        default:
            u32Base = REG_ISPGOP_00_BASE;
            HALRGNERR("[%s]Wrong eGopId=%d",__FUNCTION__,eGopId);
            CamOsPanic("");
            break;
    }
    return u32Base;
}

u32 _HalGopGwinRegBaseShift(HalGopGwinIdType_e eGwinId)
{
    u32 u32RegBaseShift = REG_GWIN0_REGBASE_OFFSET;

    switch(eGwinId)
    {
        case E_HAL_GOP_GWIN_ID_0:
            u32RegBaseShift = REG_GWIN0_REGBASE_OFFSET;
            break;
        default:
            u32RegBaseShift = REG_GWIN0_REGBASE_OFFSET;
            HALRGNERR("[%s]Wrong eGwinId=%d",__FUNCTION__,eGwinId);
            CamOsPanic("");
            break;
    }
    return u32RegBaseShift;
}

u16 _HalGopGetMiuAlign(HalGopIdType_e eGopId)
{
    u16 u16MiuAlign;

    switch(eGopId)
    {
        case E_HAL_ISPGOP_ID_00:
        case E_HAL_ISPGOP_ID_01:
        case E_HAL_ISPGOP_ID_0_ST:
        case E_HAL_ISPGOP_ID_10:
        case E_HAL_ISPGOP_ID_11:
        case E_HAL_ISPGOP_ID_1_ST:
            u16MiuAlign = MIU_BUS_ALIGN_GOP_INFINITY;
            break;
        default:
            HALRGNERR("[%s]Wrong eGopId=%d",__FUNCTION__,eGopId);
            CamOsPanic("");
            break;
    }
    return u16MiuAlign;

}
void _HalGopCalStrWinRatio(u32 *ratio_floor, u32 *init, u16 src, u16 dst)
{
    if(src==dst) {
        *ratio_floor = 0x1000;
        *init = 0;
    } else {
        *ratio_floor = ((src)<<12)/(dst);
        *init = ((src)<<12)-(dst)*(*ratio_floor);
    }
}

#if !(RIU_32_EN)
u16 _HalGopGetShadow(HalGopIdType_e eGopId,HalGopGwinIdType_e eGwinId, u32 u32Reg)
{
    u32 u32Id;
    u16 u16val;
    u32Id = ((eGopId&0xF0)>>4);
    if(eGopId&E_HAL_ISPGOP_ID_01)
    {
        //Gwin
        u16val = u16GwinShadow[u32Id][eGwinId][u32Reg];
    }
    else
    {
        //Gop
        u16val = u16GopShadow[u32Id][u32Reg];
    }
    return u16val;
}
void _HalGopSetShadow(HalGopIdType_e eGopId,HalGopGwinIdType_e eGwinId, u32 u32Reg, u16 u16Val)
{
    u32 u32Id;
    u32Id = ((eGopId&0xF0)>>4);
    if(eGopId&E_HAL_ISPGOP_ID_01)
    {
        //Gwin
        u16GwinShadow[u32Id][eGwinId][u32Reg] = u16Val;
    }
    else
    {
        //Gop
        u16GopShadow[u32Id][u32Reg] = u16Val;
    }
}
void _HalGopShadowHandler(HalGopShadowConfig_t *pCfg)
{
    u16 u16Shift = (pCfg->eGopId&E_HAL_ISPGOP_ID_01) ? u16GwinShadowReg[pCfg->u32RegType]:u16GopShadowReg[pCfg->u32RegType];
    u16 u16ShadowVal;
    u16ShadowVal = _HalGopGetShadow(pCfg->eGopId,pCfg->eGwinId,pCfg->u32RegType);
    u16ShadowVal = ((pCfg->u16Val&pCfg->u16Msk)|(u16ShadowVal&(~pCfg->u16Msk)));
    _HalGopSetShadow(pCfg->eGopId,pCfg->eGwinId,pCfg->u32RegType,u16ShadowVal);
    HalRgnWrite2ByteMsk(pCfg->u32Reg + u16Shift,u16ShadowVal,0xFFFF,pCfg->eCmdqId);
}
void _HalGopShadowInit(HalGopIdType_e eGopId)
{
    u32 u32Id;
    u32 GOP_Reg_Base = 0;
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    u32Id = ((eGopId&0xF0)>>4);
    u16GopShadow[u32Id][E_HAL_GOP_SHADOW_h11] = HalRgnRead2Byte(GOP_Reg_Base+REG_GOP_11);
    u16GopShadow[u32Id][E_HAL_GOP_SHADOW_h12] = HalRgnRead2Byte(GOP_Reg_Base+REG_GOP_12);
    u16GopShadow[u32Id][E_HAL_GOP_SHADOW_h20] = HalRgnRead2Byte(GOP_Reg_Base+REG_GOP_20);
    u16GopShadow[u32Id][E_HAL_GOP_SHADOW_h24] = HalRgnRead2Byte(GOP_Reg_Base+REG_GOP_24);
    u16GopShadow[u32Id][E_HAL_GOP_SHADOW_h28] = HalRgnRead2Byte(GOP_Reg_Base+REG_GOP_28);
}
void _HalGopGwinShadowInit(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId)
{
    u32 u32Id;
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    u32Id = ((eGopId&0xF0)>>4);
    u16GwinShadow[u32Id][eGwinId][E_HAL_GWIN_SHADOW_h0] = HalRgnRead2Byte(GOP_Reg_Base+GWIN_Reg_Shift_Base+REG_GOP_00);
    u16GwinShadow[u32Id][eGwinId][E_HAL_GWIN_SHADOW_h8] = HalRgnRead2Byte(GOP_Reg_Base+GWIN_Reg_Shift_Base+REG_GOP_08);
}
#endif
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void HalGopGetClkNum(HalGopIdType_e eGopId, u32 *u32clknum)
{
	HalRgnCmdqIdType_e eCmdqId;
	if(_HalGopIdTransCmdqID(eGopId,&eCmdqId))
	{
		if(eCmdqId==E_HAL_RGN_CMDQ_DIVP_ID_1)
		{
			*u32clknum = 2;
		}
		else
		{
			*u32clknum = 1;
		}
	}
	else
	{
		HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
		*u32clknum = 0;
	}
}
bool HalGopSetClkEnable(HalGopIdType_e eGopId)
{
    HalRgnCmdqIdType_e eCmdqId = E_HAL_RGN_CMDQ_ID_NUM;
    u16 u16Clk;
    u32 u32ClkReg;
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(eCmdqId==E_HAL_RGN_CMDQ_DIVP_ID_1)
    {
        u32ClkReg = (REG_DIP_TO_GOP_CLK_BASE);
    }
    else
    {
        u32ClkReg = (REG_SC_TO_GOP_CLK_BASE);
    }
    eCmdqId = E_HAL_RGN_CMDQ_ID_NUM;
    u16Clk = HalRgnRead2Byte(u32ClkReg);
    if(u16Clk&REG_CLK_DISABLE)
    {
        HalRgnWrite2ByteMsk(u32ClkReg, REG_CLK_ENABLE, REG_CLK_EN_MSK,eCmdqId);
    }
    return (u16Clk&REG_CLK_DISABLE) ? 1 : 0;
}
bool HalGopSetClkDisable(HalGopIdType_e eGopId,bool bEn)
{
    u32 u32ClkReg;
    u16 u16Clk;
    HalRgnCmdqIdType_e eCmdqId = E_HAL_RGN_CMDQ_ID_NUM;
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(eCmdqId==E_HAL_RGN_CMDQ_DIVP_ID_1)
    {
        u32ClkReg = (REG_DIP_TO_GOP_CLK_BASE);
    }
    else
    {
        u32ClkReg = (REG_SC_TO_GOP_CLK_BASE);
    }
    if(bEn)
    {
        u16Clk = HalRgnRead2Byte(u32ClkReg);
        if(u16Clk&REG_CLK_DISABLE)
        {
            // ToDo:for ensure DIP/SC module didn't init
            //HalRgnWrite2ByteMsk(u32ClkReg, REG_CLK_DISABLE, REG_CLK_EN_MSK,E_HAL_RGN_CMDQ_ID_NUM);
        }
    }
    return bEn;
}
void HalGopMiuSelect(HalGopIdType_e eGopId, u32 *u32BaseAddr)
{
}
void HalGopUpdateBaseXoffset(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u16 u16Xoffset)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    HalGopShadowConfig_t stCfg;
    HalRgnCmdqIdType_e eCmdqId;

    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    HALRGNDBG("[GOP]%s %d: Id:%02x, in, Xoffset=0x%x \n", __FUNCTION__, __LINE__, eGopId, u16Xoffset);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    //base address register is at gwin
    if(IsHalGWINBaseIdType_all(GOP_Reg_Base))
    {
        if(IsHalGWINBaseIdType_1(GOP_Reg_Base))
        {
            HALRGNERR("[GOP]%s %d: GOP_id=0%x not support\n", __FUNCTION__, __LINE__,eGopId);
        }
        else
        {
#if !(RIU_32_EN)
            stCfg.eGopId = eGopId;
            stCfg.eGwinId = eGwinId;
            stCfg.u16Msk = GOP_dram_rblk0_hstr_MASK;
            stCfg.u16Val = u16Xoffset<<GOP_dram_rblk0_hstr_Shift;
            stCfg.u32RegType = E_HAL_GWIN_SHADOW_h8;
            stCfg.u32Reg = GOP_Reg_Base + GWIN_Reg_Shift_Base;
            stCfg.eCmdqId = eCmdqId;
            _HalGopShadowHandler(&stCfg);
#else
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_08, (u16Xoffset<<GOP_dram_rblk0_hstr_Shift),GOP_dram_rblk0_hstr_MASK, eCmdqId);
#endif
        }
        // Double Buffer Write
        _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId for base addr settings\n", __FUNCTION__, __LINE__);
    }

    HALRGNDBG("[GOP]%s %d: out \n", __FUNCTION__, __LINE__);
}
void HalGopUpdateBase(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u32 u32BaseAddr)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    u16 MIU_BUS;
    u32 u32TempVal;
    HalRgnCmdqIdType_e eCmdqId;
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    MIU_BUS = _HalGopGetMiuAlign(eGopId);
    HALRGNDBG("[GOP]%s %d: Id:%02x, in, addr=0x%x \n", __FUNCTION__, __LINE__, eGopId, u32BaseAddr);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    //base address register is at gwin
    if(IsHalGWINBaseIdType_all(GOP_Reg_Base))
    {
        u32TempVal = u32BaseAddr >> MIU_BUS; // 128-bit unit = 16 bytes
        HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_01, (u32TempVal & 0xFFFF),0xFFFF,eCmdqId); //Ring block start Low address 0x21:16
        u32TempVal = u32TempVal >> 0x10;
        HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_02, (u32TempVal & 0xFFFF),0xFFFF,eCmdqId); //Ring block start Hi address 0x22:6, page number
        // Double Buffer Write
        _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId for base addr settings\n", __FUNCTION__, __LINE__);
    }
}
// set gwin pitch directly without calcuting with color format
void HalGopSetGwinMemPitchDirect(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u32 u32MemPitch)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    u16 MIU_BUS;
    HalRgnCmdqIdType_e eCmdqId;
    HALRGNDBG("[GOP]%s %d\n", __FUNCTION__, __LINE__);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    MIU_BUS = _HalGopGetMiuAlign(eGopId);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(IsHalGWINBaseIdType_all(GOP_Reg_Base))
    {
        if(IsHalGWINBaseIdType_1(GOP_Reg_Base))
        {
            // framebuffer pitch
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_09, (u32MemPitch >> MIU_BUS),0x7FF,eCmdqId); //bytes per line for gop framebuffer
            // Double Buffer Write
            _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
        }
        else
        {
            // framebuffer pitch
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_07, (u32MemPitch >> MIU_BUS),0x7FF,eCmdqId); //bytes per line for gop framebuffer
            // Double Buffer Write
            _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
        }
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}
bool HalGopCheckIdSupport(HalGopIdType_e eGopId)
{
    bool bRet = 1;
    if(eGopId > E_HAL_ISPGOP_ID_MAX)
    {
        bRet = 0;
    }
    HALRGNDBG("[GOP]%s %hhd\n", __FUNCTION__, bRet);
    return bRet;
}
void HalGopSetAlphaPointVal(HalGopGwinAlphaDef_e enType,HalGopGwinAlphaVal_t *tAlphaVal,u8 **p8Alpha)
{
    switch(enType)
    {
        case E_HAL_GOP_GWIN_ALPHA_DEFINE_CONST:
        case E_HAL_GOP_GWIN_ALPHA_DEFINE_ALPHA0:
            *p8Alpha = &tAlphaVal->u8ConstantAlpahValue;
        break;
        case E_HAL_GOP_GWIN_ALPHA_DEFINE_ALPHA1:
            *p8Alpha = &tAlphaVal->u8Argb1555Alpha1Val;
        break;
    }
}
void HalGopUpdateParam(HalGopIdType_e eGopId, HalGopParamConfig_t *ptParamCfg)
{
    u32 GOP_Reg_Base = 0;
    u16 GOP_Pipe_Delay = 2;
    HalRgnCmdqIdType_e eCmdqId;

    HALRGNDBG("[GOP]%s %d: Id:%02x, Disp:%s, Out:%s, (%d %d %d %d)\n",
              __FUNCTION__, __LINE__, eGopId,
              PARSING_DISPLAY_MODE(ptParamCfg->eDisplayMode),
              PARSING_OUT_MODE(ptParamCfg->eOutFormat),
              ptParamCfg->tStretchWindow.u16X, ptParamCfg->tStretchWindow.u16Y,
              ptParamCfg->tStretchWindow.u16Width, ptParamCfg->tStretchWindow.u16Height);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GOP_Pipe_Delay = GOP_PIPE_DELAY(eGopId);
    ptParamCfg->eOutFormat = E_HAL_GOP_OUT_FMT_YUV;
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    _HalGopShadowInit(eGopId);
    if(IsHalGOPBaseIdType_all(GOP_Reg_Base))
    {
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00, GOP_SOFT_RESET,GOP_SOFT_RESET,eCmdqId); //reset GOP_FIELD_INV not use
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00, 0,GOP_SOFT_RESET,eCmdqId); //reset GOP_FIELD_INV not use
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_01, GOP_REGDMA_INTERVAL_START | GOP_REGDMA_INTERVAL_END,GOP_REGDMA_INTERVAL_ST_MSK|GOP_REGDMA_INTERVAL_ED_MSK,eCmdqId);
        if(IsHalGOPBaseIdType_1(GOP_Reg_Base))
        {
            HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00,
            GOP_HS_MASK | (((u16)ptParamCfg->eOutFormat) << 10) | GWIN_DISP_MD_PROGRESS,GOP_HS_MASK|GOP_OUTPUT_FORMAT|GOP_OUTPUTSEL_MASK|GOP_DISPLAY_MODE,eCmdqId); //set Progress mode(not use); mask Hsync; YUV output
            HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_0A, 0x0014,GOP_reg_fake_rdy_MSK,eCmdqId); //insert fake rdy between hs & valid rdy
            //HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_10, GOP_CLK_CTRL_DIP,GOP_CLK_CTRL_MSK,eCmdqId); //clk src
        }
        else
        {
            HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00,
            GOP_HS_MASK |GOP_OUTPUTSEL_MASK| (((u16)ptParamCfg->eOutFormat) << 10),GOP_HS_MASK |GOP_OUTPUTSEL_MASK|GOP_OUTPUT_FORMAT,eCmdqId); //set Progress mode(not use); mask Hsync; YUV output
            HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_0A, 0x0008,GOP_reg_fake_rdy_MSK,eCmdqId); //insert fake rdy between hs & valid rdy
            //HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_10, GOP_CLK_CTRL_SCX,GOP_CLK_CTRL_MSK,eCmdqId); //clk src
        }
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_02, GOP_GWIN_VS_SEL_VSYNC|GOP_GWIN_VDE_INV_DISABLE,GOP_GWIN_VDE_INV_MSK|GOP_GWIN_VS_SEL_MSK,eCmdqId);
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_0E, (ptParamCfg->tStretchWindow.u16Width >> 1) + 1,0x7FF,eCmdqId); //miu efficiency = Stretch Window H size (unit:2 pixel) /2 +1
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_0F, GOP_Pipe_Delay,0x3FF,eCmdqId); //Hsync input pipe delay
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_19, GOP_BURST_LENGTH_MASK, GOP_BURST_LENGTH_MAX,eCmdqId); // set gop dma burst length

        //HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_10, 0x501,0xFFF,eCmdqId); // h10 [14:8]reg_pause_detect_thd=5

        // GOP display area global settings
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_30, ptParamCfg->tStretchWindow.u16Width,0xFFF,eCmdqId); // Stretch Window H size (unit:1 pixel)
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_31, ptParamCfg->tStretchWindow.u16Height,0xFFF,eCmdqId); // Stretch window V size
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_32, ptParamCfg->tStretchWindow.u16X,0xFFF,eCmdqId); // Stretch Window H coordinate
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_34, ptParamCfg->tStretchWindow.u16Y,0xFFF,eCmdqId); // Stretch Window V coordinate
        // Double Buffer Write
        _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}


void HalGopUpdateGwinParam(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopGwinParamConfig_t *ptParamCfg)
{
    u32 GOP_Reg_Base = 0;
    u16 MIU_BUS;
    u32 u32BaseAddr;
    u32 GWIN_Reg_Shift_Base = 0;
    u16 u16DispImage_HStart, u16DispImage_HEnd;
    u16 u16DispImage_VStart, u16DispImage_VEnd;
    HalGopShadowConfig_t stCfg;
    HalRgnCmdqIdType_e eCmdqId;

    HALRGNDBG("[GOP]%s %d: Id:%02x, Src:%s, (%d %d %d %d), Base:%08x Offset(%04x, %08x)\n",
              __FUNCTION__, __LINE__, eGopId,
              PARSING_SRC_FMT(ptParamCfg->eSrcFmt),
              ptParamCfg->tDispWindow.u16X, ptParamCfg->tDispWindow.u16Y,
              ptParamCfg->tDispWindow.u16Width, ptParamCfg->tDispWindow.u16Height,
              ptParamCfg->u32BaseAddr,
              ptParamCfg->u16Base_XOffset, ptParamCfg->u32Base_YOffset);
    _HalGopGwinShadowInit(eGopId,eGwinId);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    MIU_BUS        =  _HalGopGetMiuAlign(eGopId);
    u16DispImage_HStart = ptParamCfg->tDispWindow.u16X;
    u16DispImage_VStart = ptParamCfg->tDispWindow.u16Y;
    u16DispImage_HEnd   = ptParamCfg->tDispWindow.u16X + ptParamCfg->tDispWindow.u16Width;
    u16DispImage_VEnd   = ptParamCfg->tDispWindow.u16Y + ptParamCfg->tDispWindow.u16Height;
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(IsHalGWINBaseIdType_all(GOP_Reg_Base))
    {
#if !(RIU_32_EN)
        stCfg.eGopId = eGopId;
        stCfg.eGwinId = eGwinId;
        stCfg.u16Msk = 0x00F0;
        stCfg.u16Val = (((u8)_HalGopSrcFmtTransRealVal(eGopId,ptParamCfg->eSrcFmt))<<4);
        stCfg.u32RegType = E_HAL_GWIN_SHADOW_h0;
        stCfg.u32Reg = GOP_Reg_Base + GWIN_Reg_Shift_Base;
        stCfg.eCmdqId = eCmdqId;
        _HalGopShadowHandler(&stCfg);
#else
        HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_00, (((u8)_HalGopSrcFmtTransRealVal(eGopId,ptParamCfg->eSrcFmt))<<4),0x00F0, eCmdqId);
#endif
        // framebuffer starting address
        u32BaseAddr = ptParamCfg->u32BaseAddr;
        u32BaseAddr = u32BaseAddr >> MIU_BUS; // 128-bit unit = 16 bytes
        HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_01, (u32BaseAddr & 0xFFFF),0xFFFF, eCmdqId);
        u32BaseAddr = u32BaseAddr >> 0x10;
        HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_02, (u32BaseAddr & 0xFFFF),0xFFF, eCmdqId);

        if(IsHalGWINBaseIdType_1(GOP_Reg_Base))
        {
            // framebuffer pitch
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_09,
            (HAL_GOP_PIX2BYTE(ptParamCfg->tDispWindow.u16Width, ptParamCfg->eSrcFmt)) >> MIU_BUS,0x7FF,eCmdqId); //bytes per line for gop framebuffer
        }
        else
        {
            // framebuffer pitch ((h_size * byte_per_pixel)>>MIU_BUS)
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_07,
            (HAL_GOP_PIX2BYTE(ptParamCfg->tDispWindow.u16Width, ptParamCfg->eSrcFmt)) >> MIU_BUS,0x7FF,eCmdqId); //bytes per line for gop framebuffer
        }

        if(IsHalGWINBaseIdType_1(GOP_Reg_Base))
        {
            HALRGNERR("[GOP]%s %d: GOP_id=0%x not support\n", __FUNCTION__, __LINE__,eGopId);
        }
        else
        {
#if !(RIU_32_EN)
            stCfg.u16Msk = GOP_dram_rblk0_hstr_MASK|GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK;
            stCfg.u16Val = (ptParamCfg->u16Base_XOffset<<GOP_dram_rblk0_hstr_Shift)|GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK;
            stCfg.u32RegType = E_HAL_GWIN_SHADOW_h8;
            _HalGopShadowHandler(&stCfg);
#else
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_08, ((ptParamCfg->u16Base_XOffset<<GOP_dram_rblk0_hstr_Shift)|GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK),GOP_dram_rblk0_hstr_MASK|GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK, eCmdqId);
#endif
        }

        if(IsHalGWINBaseIdType_1(GOP_Reg_Base))
        {
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_04, u16DispImage_HStart >> MIU_BUS,0xFFF, eCmdqId); // H start
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_05, u16DispImage_HEnd >> MIU_BUS,0xFFF, eCmdqId); // H end
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_06, u16DispImage_VStart,0xFFF, eCmdqId); // V start line
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_08, u16DispImage_VEnd,0xFFF, eCmdqId); // V end line
        }
        else
        {
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_03, u16DispImage_HStart,0xFFF, eCmdqId); // H start
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_04, u16DispImage_HEnd,0xFFF, eCmdqId); // H end
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_05, u16DispImage_VStart,0xFFF, eCmdqId); // V start line
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_06, u16DispImage_VEnd,0xFFF, eCmdqId); // V end line
        }
        // Double Buffer Write
        _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}

void HalGopSetStretchWindowSize(HalGopIdType_e eGopId, HalGopWindowType_t *ptSrcWinCfg, HalGopWindowType_t *ptDstWinCfg)
{
    u32 GOP_Reg_Base = 0;
    u32 ratio_floor = 0;
    u32 init = 0;
    HalRgnCmdqIdType_e eCmdqId;
    HALRGNDBG("[GOP]%s %d\n", __FUNCTION__, __LINE__);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

     //GOP0 not support any scaling type
     if(IsHalGOPBaseIdType_0(GOP_Reg_Base))
     {
         if((ptSrcWinCfg->u16Width != ptDstWinCfg->u16Width) ||
            (ptSrcWinCfg->u16Height != ptDstWinCfg->u16Height))
         {
             HALRGNERR("[GOP]%s %d: GOP0 Not Support H/V scaling\n", __FUNCTION__, __LINE__);
             HALRGNERR("[GOP]%s %d: SrcW=%d, SrcH=%d, DstW=%d, DstH=%d\n", __FUNCTION__, __LINE__,
                         ptSrcWinCfg->u16Width,ptSrcWinCfg->u16Height,ptDstWinCfg->u16Width,ptDstWinCfg->u16Height);
         }
         else
         {
             // GOP global settings
             HalRgnWrite2ByteMsk(GOP_Reg_Base+REG_GOP_0E,(ptSrcWinCfg->u16Width >>1)+1,0x7FF,eCmdqId);//miu efficiency = Stretch Window H size (unit:2 pixel) /2 +1
             // GOP display area global settings
             HalRgnWrite2ByteMsk(GOP_Reg_Base+REG_GOP_30,ptSrcWinCfg->u16Width,0xFFFF,eCmdqId); //Stretch Window H size (unit:1 pixel)
             HalRgnWrite2ByteMsk(GOP_Reg_Base+REG_GOP_31,ptSrcWinCfg->u16Height,0xFFFF,eCmdqId);  //Stretch window V size
             HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_32, ptSrcWinCfg->u16X,0xFFFF,eCmdqId); // Stretch Window H coordinate
             HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_34, ptSrcWinCfg->u16Y,0xFFFF,eCmdqId); // Stretch Window V coordinate
             // Double Buffer Write
             _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
         }
    }
    else if(IsHalGOPBaseIdType_1(GOP_Reg_Base))//GOP1 support scaling up
     {
         if((ptSrcWinCfg->u16Width > ptDstWinCfg->u16Width) ||
            (ptSrcWinCfg->u16Height > ptDstWinCfg->u16Height))
         {
             HALRGNERR("[GOP]%s %d: GOP1 Not Support H/V scaling down\n", __FUNCTION__, __LINE__);
             HALRGNERR("[GOP]%s %d: SrcW=%d, SrcH=%d, DstW=%d, DstH=%d\n", __FUNCTION__, __LINE__,
                         ptSrcWinCfg->u16Width,ptSrcWinCfg->u16Height,ptDstWinCfg->u16Width,ptDstWinCfg->u16Height);
         }
         else
         {
             // GOP global settings
             HalRgnWrite2ByteMsk(GOP_Reg_Base+REG_GOP_0E,(ptSrcWinCfg->u16Width >>1)+1,0x7FF,eCmdqId);//miu efficiency = Stretch Window H size (unit:2 pixel) /2 +1
             // GOP display area global settings
             HalRgnWrite2ByteMsk(GOP_Reg_Base+REG_GOP_30,(ptSrcWinCfg->u16Width >>1),0xFFFF,eCmdqId); //Stretch Window H size (unit:1 pixel)
             HalRgnWrite2ByteMsk(GOP_Reg_Base+REG_GOP_31,ptSrcWinCfg->u16Height,0xFFFF,eCmdqId);  //Stretch window V size
             HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_32, ptSrcWinCfg->u16X,0xFFFF,eCmdqId); // Stretch Window H coordinate
             HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_34, ptSrcWinCfg->u16Y,0xFFFF,eCmdqId); // Stretch Window V coordinate

             //H scaling
             _HalGopCalStrWinRatio(&ratio_floor,&init,ptSrcWinCfg->u16Width,ptDstWinCfg->u16Width);
            HALRGNDBG("[GOP]GOPID[%02x]W:src=%d,dst=%d,ratio=%x,init=%x\n",eGopId,ptSrcWinCfg->u16Width,ptDstWinCfg->u16Width,ratio_floor,init);
            HalRgnWrite2Byte(GOP_Reg_Base + REG_GOP_35,ratio_floor&0x1FFF,eCmdqId);  //Stretch Window H ratio (in/out * 2^12)
            HalRgnWrite2Byte(GOP_Reg_Base + REG_GOP_38,init&0x1FFF,eCmdqId);  // h38 [12:0]reg_hstrch_ini(Stretch H start value)

             //V scaling
             _HalGopCalStrWinRatio(&ratio_floor,&init,ptSrcWinCfg->u16Height,ptDstWinCfg->u16Height);
            HALRGNDBG("GOPID[%02x]H:src=%d,dst=%d,ratio=%x,init=%x\n",eGopId,ptSrcWinCfg->u16Height,ptDstWinCfg->u16Height,ratio_floor,init);
            HalRgnWrite2Byte(GOP_Reg_Base + REG_GOP_36,ratio_floor&0x1FFF,eCmdqId);  //Stretch window V ratio  (in/out * 2^12)
            HalRgnWrite2Byte(GOP_Reg_Base + REG_GOP_39,init&0x1FFF,eCmdqId);  // h39 [12:0]reg_vstrch_ini(Stretch V start value)

             //stretch mode
             HalRgnWrite2Byte(GOP_Reg_Base + REG_GOP_3A, 0,eCmdqId); // h3A [4]reg_trs_strch(0:Transparent color only duplicate if stretch
                                                                     //                      1:Stretch transparent color like normal color)
                                                                     //     [3:2]reg_vstrch_md(0:Linear 1:Duplicate 2:Nearest)
                                                                     //     [1:0]reg_hstrch_md(0:6-tap(including nearest) 1:duplicate)

             // Double Buffer Write
             _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
         }
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}

void HalGopSetGwinSize(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopWindowType_t *ptGwinCfg, HalGopGwinSrcFormat_e eSrcFmt)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    u16 u16DispImage_HStart, u16DispImage_HEnd;
    u16 u16DispImage_VStart, u16DispImage_VEnd;
    u16 MIU_BUS;
    HalRgnCmdqIdType_e eCmdqId;
    HALRGNDBG("[GOP]%s %d\n", __FUNCTION__, __LINE__);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    MIU_BUS = _HalGopGetMiuAlign(eGopId);

    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(IsHalGWINBaseIdType_all(GOP_Reg_Base))
    {
        if(IsHalGWINBaseIdType_1(GOP_Reg_Base))
        {
            u16DispImage_HStart = HAL_GOP_PIX2BYTE(ptGwinCfg->u16X, eSrcFmt);
            u16DispImage_VStart = ptGwinCfg->u16Y;
            u16DispImage_HEnd   = HAL_GOP_PIX2BYTE((ptGwinCfg->u16X + ptGwinCfg->u16Width), eSrcFmt);
            u16DispImage_VEnd   = ptGwinCfg->u16Y + ptGwinCfg->u16Height;
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_04, u16DispImage_HStart >> MIU_BUS,0xFFF, eCmdqId); // H start
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_05, u16DispImage_HEnd >> MIU_BUS,0xFFF, eCmdqId); // H end
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_06, u16DispImage_VStart,0xFFF, eCmdqId); // V start line
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_08, u16DispImage_VEnd,0xFFF, eCmdqId); // V end line
        }
        else
        {
            u16DispImage_HStart = ptGwinCfg->u16X;
            u16DispImage_VStart = ptGwinCfg->u16Y;
            u16DispImage_HEnd   = ptGwinCfg->u16X + ptGwinCfg->u16Width -1;
            u16DispImage_VEnd   = ptGwinCfg->u16Y + ptGwinCfg->u16Height -1;
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_03, u16DispImage_HStart,0xFFF, eCmdqId); // H start
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_04, u16DispImage_HEnd,0xFFF, eCmdqId); // H end
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_05, u16DispImage_VStart,0xFFF, eCmdqId); // V start line
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_06, u16DispImage_VEnd,0xFFF, eCmdqId); // V end line
        }
        // Double Buffer Write
        _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}

void HalGopSetGwinSrcFmt(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopGwinSrcFormat_e eSrcFmt)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    HalRgnCmdqIdType_e eCmdqId;
    HalGopShadowConfig_t stCfg;
    HalGopGwinSrcFormatRealVal_e eValSrcFmt;
    HALRGNDBG("[GOP]%s %d\n", __FUNCTION__, __LINE__);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    eValSrcFmt = _HalGopSrcFmtTransRealVal(eGopId, eSrcFmt);
    // Alpha inv, which is at GOP setting
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(IsHalGWINBaseIdType_all(GOP_Reg_Base))
    {
#if !(RIU_32_EN)
        stCfg.eGopId = eGopId;
        stCfg.eGwinId = eGwinId;
        stCfg.u16Msk = 0x00F0;
        stCfg.u16Val = (eValSrcFmt<<4);
        stCfg.u32RegType = E_HAL_GWIN_SHADOW_h0;
        stCfg.u32Reg = GOP_Reg_Base + GWIN_Reg_Shift_Base;
        stCfg.eCmdqId = eCmdqId;
        _HalGopShadowHandler(&stCfg);
#else
        HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_00, (eValSrcFmt<<4),0x00F0, eCmdqId);
#endif
        // Double Buffer Write
        _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}
void HalGopSetAlphaBlending(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool bConstantAlpha, u8 Alpha)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    HalGopShadowConfig_t stCfg;
    HalRgnCmdqIdType_e eCmdqId;

    // 1. GOP Set Alpha Blending
    // 2. pixel_alpha=1, constant_alpha=0
    // 3. Alpha: constant Alpha value
    HALRGNDBG("[GOP]%s %d: Id:%02x, bConstantAlpha:%d, Alpha:%x\n", __FUNCTION__, __LINE__, eGopId, bConstantAlpha, Alpha);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(IsHalGWINBaseIdType_all(GOP_Reg_Base))
    {
        if(IsHalGWINBaseIdType_1(GOP_Reg_Base))
        {
#if !(RIU_32_EN)
            // constant or pixel Alpha
            stCfg.eGopId = eGopId;
            stCfg.eGwinId = eGwinId;
            stCfg.u16Msk = GOP_G10_ALPHA_MASK;
            stCfg.u16Val = (bConstantAlpha ? GOP_CONST_ALPHA_EN : GOP_G10_PIXEL_ALPHA_EN);
            stCfg.u32RegType = E_HAL_GWIN_SHADOW_h0;
            stCfg.u32Reg = GOP_Reg_Base + GWIN_Reg_Shift_Base;
            stCfg.eCmdqId = eCmdqId;
            _HalGopShadowHandler(&stCfg);
#else
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_00, (bConstantAlpha ? GOP_CONST_ALPHA_EN : GOP_G10_PIXEL_ALPHA_EN),GOP_G10_ALPHA_MASK, eCmdqId);
#endif
            // Alpha value
            if(bConstantAlpha)
            {
                HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_0A, Alpha,GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK, eCmdqId);
            }
        }
        else
        {
#if !(RIU_32_EN)
            // constant or pixel Alpha
            stCfg.eGopId = eGopId;
            stCfg.eGwinId = eGwinId;
            stCfg.u16Msk = GOP_ALPHA_MASK;
            stCfg.u16Val = (bConstantAlpha ? GOP_CONST_ALPHA_EN : GOP_PIXEL_ALPHA_EN);
            stCfg.u32RegType = E_HAL_GWIN_SHADOW_h0;
            stCfg.u32Reg = GOP_Reg_Base + GWIN_Reg_Shift_Base;
            stCfg.eCmdqId = eCmdqId;
            _HalGopShadowHandler(&stCfg);
#else
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_00, (bConstantAlpha ? GOP_CONST_ALPHA_EN : GOP_PIXEL_ALPHA_EN),GOP_ALPHA_MASK, eCmdqId);
#endif
            // Alpha value
            if(bConstantAlpha)
            {
#if !(RIU_32_EN)
                stCfg.u16Msk = GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK;
                stCfg.u16Val = Alpha;
                stCfg.u32RegType = E_HAL_GWIN_SHADOW_h8;
                _HalGopShadowHandler(&stCfg);
#else
                HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_08, Alpha,GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK, eCmdqId);
#endif
            }
        }
        // Double Buffer Write
        _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}
void HalGopSetColorKey(HalGopIdType_e eGopId, HalGopColorKeyConfig_t *ptCfg, bool bVYU)
{
    u32 GOP_Reg_Base = 0;
    HalRgnCmdqIdType_e eCmdqId;
    // 1. GOP Set RGB Color Key
    // 2. bEn= 0: disable; 1: enable
    // 3. u8R: red color value; u8G: green color value; u8B: blue color value ; value range: 0~255 (0x0~0xFF)
    HALRGNDBG("[GOP]%s %d: Id:%02x, bEn:%d, (R=%x, G=%x, B=%x)\n", __FUNCTION__, __LINE__,
    eGopId, ptCfg->bEn, ptCfg->u8R, ptCfg->u8G, ptCfg->u8B);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(bVYU)
    {
        // I6 not support
        return;
    }
    if(IsHalGOPBaseIdType_all(GOP_Reg_Base))
    {
        if(ptCfg->bEn)
        {
            HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00, bVYU?GOP_VYU_TRANSPARENT_COLOR_ENABLE:GOP_RGB_TRANSPARENT_COLOR_ENABLE,
                                                            GOP_VYU_TRANSPARENT_COLOR_ENABLE|GOP_RGB_TRANS_COLOR_EN,eCmdqId);
            if(IsHalGWINBaseIdType_1(GOP_Reg_Base))
            {
                HalRgnWrite2ByteMsk(GOP_Reg_Base + (bVYU?REG_GOP_26:REG_GOP_24), (ptCfg->u8B | (ptCfg->u8G << 8)), 0xFFFF,eCmdqId);
                HalRgnWrite2ByteMsk(GOP_Reg_Base + (bVYU?REG_GOP_27:REG_GOP_25),  ptCfg->u8R, 0x00FF,eCmdqId);
            }
            else
            {
                HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_24, (ptCfg->u8B | (ptCfg->u8G << 8)), 0xFFFF,eCmdqId);
                HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_25,  ptCfg->u8R, 0x00FF,eCmdqId);
            }
        }
        else
        {
            HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00, bVYU?GOP_VYU_TRANSPARENT_COLOR_ENABLE:GOP_RGB_TRANSPARENT_COLOR_DISABLE,
                                                            GOP_VYU_TRANSPARENT_COLOR_ENABLE|GOP_RGB_TRANS_COLOR_EN,eCmdqId);
        }
        // Double Buffer Write
        _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}
void HalGopSetPaletteRiu(HalGopIdType_e eGopId, HalGopPaletteConfig_t *ptCfg)
{
    u32 GOP_Reg_Base = 0;
    HalRgnCmdqIdType_e eCmdqId = E_HAL_RGN_CMDQ_ID_NUM;
    // 1. GOP Set Palette by RIU Mode
    // 2. GOP Palette SRAM Clock should be opened
    // 3. Set Force Write
    // 4. Set RIU Mode
    // 5. Set Palette Index, A, R, G, B Values
    // 6. Trigger Palette Write
    // 7. Close Force Write
    HALRGNDBG("[GOP]%s %d\n", __FUNCTION__, __LINE__);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    if(IsHalGOPBaseIdType_all(GOP_Reg_Base))
    {
        //set force write
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, GOP_BANK_FORCE_WR, GOP_BANK_FORCE_WR,eCmdqId);
        //set RIU mode
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_05, GOP_PALETTE_SRAM_CTRL_RIU, GOP_PALETTE_SRAM_CTRL_MASK,eCmdqId);
        //set palette value
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_03, ptCfg->u8G << 8 | ptCfg->u8B,0xFFFF,eCmdqId);
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_04, ptCfg->u8A << 8 | ptCfg->u8R,0xFFFF,eCmdqId);
        //set RIU mode
        //set palette index
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_05, ptCfg->u8Idx, GOP_PALETTE_TABLE_ADDRESS_MASK,eCmdqId);
        //write trigger
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_05, 0x0100, GOP_PALETTE_WRITE_ENABLE_MASK,eCmdqId);
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_05, 0x0000, GOP_PALETTE_WRITE_ENABLE_MASK,eCmdqId);
        //close force write
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_7F, 0x0000, GOP_BANK_FORCE_WR,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}
bool HalGopCheckAlphaZeroOpaque(HalGopIdType_e eGopId,bool bEn,bool bConAlpha,HalGopGwinSrcFormat_e eFmt)
{
    if((eGopId==E_HAL_ISPGOP_ID_00)||(eGopId==E_HAL_ISPGOP_ID_01))//GOP0 use I6 spec
    {
        return bEn;
    }
    else if((eGopId==E_HAL_ISPGOP_ID_10)||(eGopId==E_HAL_ISPGOP_ID_11))//GOP1 use I5 spec
    {
        if((eFmt==E_HAL_GOP_GWIN_SRC_ARGB1555) && (!bConAlpha))
        {
            return bEn;
        }
        else
        {
            return !bEn;
        }
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
        return bEn;
    }
}
void HalGopSetAlphaInvert(HalGopIdType_e eGopId, bool bInv)
{
    u32 GOP_Reg_Base = 0;
    HalRgnCmdqIdType_e eCmdqId;
    HALRGNDBG("[GOP]%s %d\n", __FUNCTION__, __LINE__);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00, bInv?GOP_ALPHA_INV:0, GOP_ALPHA_INV, eCmdqId);
    //write gop register
    _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
}
void HalGopBindOsd(HalGopIdType_e eGopId)
{
    u32 GOP_Reg_Base = 0;
    HalRgnCmdqIdType_e eCmdqId;
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(eCmdqId == E_HAL_RGN_CMDQ_DIVP_ID_1)
    {
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00,
        0,GOP_OUTPUTSEL_MASK,eCmdqId); //set Progress mode(not use); mask Hsync; YUV output
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_02, GOP_GWIN_VDE_INV_ENABLE,GOP_GWIN_VDE_INV_MSK,eCmdqId);
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_02, GOP_GWIN_VS_SEL_VFDE,GOP_GWIN_VS_SEL_MSK,eCmdqId);
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_0A, 0x0008,GOP_reg_fake_rdy_MSK,eCmdqId); //insert fake rdy between hs & valid rdy
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_10, GOP_CLK_CTRL_DIP,GOP_CLK_CTRL_MSK,eCmdqId); //clk src
    }
    else
    {
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_00,
        GOP_OUTPUTSEL_MASK, GOP_OUTPUTSEL_MASK,eCmdqId); //set Progress mode(not use); mask Hsync; YUV output
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_02, GOP_GWIN_VS_SEL_VSYNC,GOP_GWIN_VS_SEL_MSK,eCmdqId);
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_02, GOP_GWIN_VDE_INV_ENABLE,GOP_GWIN_VDE_INV_MSK,eCmdqId);
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_0A, 0x0009,GOP_reg_fake_rdy_MSK,eCmdqId); //insert fake rdy between hs & valid rdy
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_10, GOP_CLK_CTRL_SCX,GOP_CLK_CTRL_MSK,eCmdqId); //clk src
    }
}
void HalGopSetEnableGwin(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool bEn)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    HalRgnCmdqIdType_e eCmdqId;
    HalGopShadowConfig_t stCfg;

    HALRGNDBG("[GOP]%s %d: Set GopId=%d GwinId=%d enable_GOP = [%d]\n", __FUNCTION__, __LINE__, eGopId, eGwinId, bEn);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(IsHalGWINBaseIdType_all(GOP_Reg_Base))
    {
#if !(RIU_32_EN)
        // enable/disable gwin
        stCfg.eGopId = eGopId;
        stCfg.eGwinId = eGwinId;
        stCfg.u16Msk = GOP_GWIN_ENABLE;
        stCfg.u16Val = (bEn ? GOP_GWIN_ENABLE : 0);
        stCfg.u32RegType = E_HAL_GWIN_SHADOW_h0;
        stCfg.u32Reg = GOP_Reg_Base + GWIN_Reg_Shift_Base;
        stCfg.eCmdqId = eCmdqId;
        _HalGopShadowHandler(&stCfg);
#else
        HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_00, (bEn ? GOP_GWIN_ENABLE : 0),GOP_GWIN_ENABLE, eCmdqId);
#endif
        //write gop register
        _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}
void HalGopSetArgb1555Alpha(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopGwinArgb1555Def_e eAlphaType, u8 u8AlphaVal)
{
    u32 GOP_Reg_Base = 0;
    u32 GWIN_Reg_Shift_Base = 0;
    HalRgnCmdqIdType_e eCmdqId;
    HalGopShadowConfig_t stCfg;

    HALRGNDBG("[GOP]%s %d: Set GopId=%d GwinId=%d, argb1555 alpha type=%d, val=%d\n", __FUNCTION__, __LINE__, eGopId, eGwinId, eAlphaType, u8AlphaVal);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    GWIN_Reg_Shift_Base = _HalGopGwinRegBaseShift(eGwinId);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);
    if(IsHalGWINBaseIdType_0(GOP_Reg_Base))
    {
        // enable/disable gwin
        if(eAlphaType==E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0)
        {
#if !(RIU_32_EN)
            stCfg.eGopId = eGopId;
            stCfg.eGwinId = eGwinId;
            stCfg.u16Msk = GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK;
            stCfg.u16Val = (u8AlphaVal);
            stCfg.u32RegType = E_HAL_GWIN_SHADOW_h8;
            stCfg.u32Reg = GOP_Reg_Base + GWIN_Reg_Shift_Base;
            stCfg.eCmdqId = eCmdqId;
            _HalGopShadowHandler(&stCfg);
#else
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base + REG_GOP_08, (u8AlphaVal),GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK, eCmdqId);
#endif
        }
        else if(eAlphaType==E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1)
        {
            HalRgnWrite2ByteMsk(GOP_Reg_Base + GWIN_Reg_Shift_Base  + REG_GOP_09, (u8AlphaVal), GOP_GWIN_ARGB1555_ALPHA1_DEF_MSK,eCmdqId);
        }
        //write gop register
        _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        HALRGNERR("[GOP]%s %d: GOPId=0x%x not support\n", __FUNCTION__, __LINE__,eGopId);
    }
}

void HalGopSetVideoTimingInfo(HalGopIdType_e eGopId, HalGopVideoTimingInfoConfig_t *ptVideoTimingCfg)
{
    u32 GOP_Reg_Base = 0;
    u16 u16PipeDelay = 0;
    HalRgnCmdqIdType_e eCmdqId;
    HALRGNDBG("[GOP]%s %d\n", __FUNCTION__, __LINE__);
    GOP_Reg_Base = _HalGopGetBaseAddr(eGopId);
    _HalGopIdTransCmdqID(eGopId,&eCmdqId);

    u16PipeDelay = ptVideoTimingCfg->u16Htotal-ptVideoTimingCfg->u16Width-28;

    if(IsHalGOPBaseIdType_1(GOP_Reg_Base))//GOP1 issue patch
    {
        HalRgnWrite2ByteMsk(GOP_Reg_Base + REG_GOP_0F, u16PipeDelay, 0x1FFF,eCmdqId);
         // Double Buffer Write
         _HalGopWriteDoubleBuffer(GOP_Reg_Base,eGopId,eCmdqId);
    }
    else
    {
        //HALRGNDBG("[GOP]%s %d: wrong GOPId\n", __FUNCTION__, __LINE__);
    }
}
