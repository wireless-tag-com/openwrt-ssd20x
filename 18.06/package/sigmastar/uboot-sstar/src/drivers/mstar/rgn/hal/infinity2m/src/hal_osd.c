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

#define __HAL_OSD_C__
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "hal_osd.h"
#include "hal_osd_reg.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
bool _HalOsdIdTransCmdqID(HalOsdIdType_e eOsdId,HalRgnCmdqIdType_e *pCmdqId)
{
    switch(eOsdId)
    {
        case E_HAL_OSD_PORT0:
        case E_HAL_OSD_PORT1:
            *pCmdqId = E_HAL_RGN_CMDQ_VPE_ID_0;
            return TRUE;
        default:
            HALRGNERR("[OSD]%s %d: wrong OSDId\n", __FUNCTION__, __LINE__);
            pCmdqId = NULL;
            break;
    }
    return FALSE;
}
u32 _HalOsdGetBaseAddr(HalOsdIdType_e eOsdId)
{
    u32 u32Base = 0;

    switch(eOsdId)
    {
        case E_HAL_OSD_PORT0:
        case E_HAL_OSD_PORT1:
            u32Base = REG_SCL_TO_GOP_BASE0;
            break;
        default:
            HALRGNERR("[%s]Wrong eOsdId=%d",__FUNCTION__,eOsdId);
            CamOsPanic("");
            break;
    }
    return u32Base;
}
u32 _HalGopOsdbRegBaseShift(HalOsdIdType_e eOsdId)
{
    u32 u32RegBaseShift = REG_SCL_EN_OSD_00;

    switch(eOsdId)
    {
        case E_HAL_OSD_PORT0:
            u32RegBaseShift = REG_SCL_EN_OSD_00;
            break;
        case E_HAL_OSD_PORT1:
            u32RegBaseShift = REG_SCL_EN_OSD_01;
            break;
        default:
            u32RegBaseShift = REG_SCL_EN_OSD_00;
            HALRGNERR("[%s]Wrong eOsdId=%d",__FUNCTION__,eOsdId);
            CamOsPanic("");
            break;

    }
    return u32RegBaseShift;
}
void _HalOsdGetBindReg(u32 u32GopOsdFlag,u16 *u16DIP,u16 *u16GopMap)
{
    if(u32GopOsdFlag&E_HAL_RGN_OSD_FLAG_DIP)
    {
        *u16DIP = 0;
        if(u32GopOsdFlag&E_HAL_RGN_OSD_FLAG_PORT0)
        {
            if(u32GopOsdFlag&E_HAL_RGN_OSD_FLAG_PORT1)
            {
                *u16GopMap = REG_SCL_BIND_OSD_01;
            }
            else
            {
                *u16GopMap = REG_SCL_BIND_OSD_02;
            }
        }
        else
        {
            *u16GopMap = REG_SCL_BIND_OSD_12;
        }
    }
    else
    {
        *u16DIP = REG_SCL_BIND_OSD2_SC;
        *u16GopMap = REG_SCL_BIND_OSD_01;
    }
}
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
//I2m has no osdb
bool HalOsdCheckIdSupport(HalOsdIdType_e eOsdId)
{
    bool bRet = 1;
    bRet = 0;
    return bRet;
}
bool HalOsdColInvParamCheck(HalOsdColorInvParamConfig_t *ptColInvCfg)
{
    MS_BOOL bRet = TRUE;
    return bRet;
}
void HalOsdSetOsdBypassForScEnable(HalOsdIdType_e eOsdId, bool bEn)
{
}
bool HalOsdSetBindGop(u32 u32GopOsdFlag)
{
/*
    u32 OSD_Reg_Base = 0;
    u32 OSDB_Reg_Shift_Base = 0;
    bool bRet = TRUE;
    u16 u16DIP = 0;
    u16 u16GopMap;
    HALRGNDBG("[OSD]%s %d: Set BindGop  = [%lx]\n", __FUNCTION__, __LINE__, u32GopOsdFlag);
    OSD_Reg_Base = REG_OSD_SET_BIND_GOP_BASE;
    OSDB_Reg_Shift_Base = REG_SCL_BIND_OSD;
    _HalOsdGetBindReg(u32GopOsdFlag,&u16DIP,&u16GopMap);
    HalRgnWrite2ByteMsk(OSD_Reg_Base + OSDB_Reg_Shift_Base,u16GopMap , REG_OSD_TO_GOP_MAP_MASK, E_HAL_RGN_CMDQ_ID_NUM);
    HalRgnWrite2ByteMsk(OSD_Reg_Base + OSDB_Reg_Shift_Base,u16DIP, REG_OSD_TO_GOP2_MAP_MASK, E_HAL_RGN_CMDQ_ID_NUM);
    return bRet;
*/
    return FALSE;
}
void HalOsdSetOsdEnable(HalOsdIdType_e eOsdId, bool bEn)
{
/*
    u32 OSD_Reg_Base = 0;
    u32 OSDB_Reg_Shift_Base = 0;
    HalRgnCmdqIdType_e eCmdqId;

    HALRGNDBG("[OSD]%s %d: Set enable_OSD  ID:%d = [%d]\n", __FUNCTION__, __LINE__, eOsdId, bEn);
    OSD_Reg_Base = _HalOsdGetBaseAddr(eOsdId);
    if(!OSD_Reg_Base)
    {
        return;
    }
    OSDB_Reg_Shift_Base = _HalGopOsdbRegBaseShift(eOsdId);
    _HalOsdIdTransCmdqID(eOsdId,&eCmdqId);
    HALRGNDBG("[OSD]%s %d: OSD_Reg_Base=0x%x, OSDB_Reg_Shift_Base=0x%x\n", __FUNCTION__, __LINE__, OSD_Reg_Base, OSDB_Reg_Shift_Base);
    // enable/disable gop from scaler
    HalRgnWrite2ByteMsk(OSD_Reg_Base + OSDB_Reg_Shift_Base, bEn ? OSD_ENABLE : 0, OSD_ENABLE, eCmdqId);
    // if id is DIP, set DE mode ( osdb en: close DE mode (enable handshake mode); osdb disable: enable DE mode)
    if(OSD_Reg_Base == REG_DIP_TO_GOP_BASE)
    {
        HalRgnWrite2ByteMsk(OSD_Reg_Base + OSDB_Reg_Shift_Base, bEn ? 0 : DIP_DE_MD_EN, DIP_DE_MD_MASK, eCmdqId);
        // osdb en: reg_dip_osd_vs_inv=0; osdb disable: reg_dip_osd_vs_inv=1)
        HalRgnWrite2ByteMsk(OSD_Reg_Base + OSDB_Reg_Shift_Base, bEn ? 0 : DIP_VS_INV_EN, DIP_VS_INV_MASK, eCmdqId);
    }
    else // if id is scaler, set reg_osd_pipe_guard_cycle
    {
        HalRgnWrite2ByteMsk(OSD_Reg_Base + OSDB_Reg_Shift_Base + REG_OSD_01, bEn ? REG_SCL_PIPE_GUARD_CYCLE : 0, REG_SCL_PIPE_GUARD_CYCLE_MASK, eCmdqId);
    }
*/
}
void HalOsdSetColorInverseEn(HalOsdIdType_e eOsdId, bool bEn){}
void HalOsdSetColorInverseParam(HalOsdIdType_e eOsdId, HalOsdColorInvWindowIdType_e eAeId, HalOsdColorInvParamConfig_t *ptColInvCfg){}
void HalOsdSetColorInverseUpdate(HalOsdIdType_e eOsdId){}
void HalOsdWriteColorInverseData(HalOsdIdType_e eOsdId, u16 addr, u32 wdata){}
void HalOsdReadColorInverseData(HalOsdIdType_e eOsdId, u16 addr, u32 *rdata){}
