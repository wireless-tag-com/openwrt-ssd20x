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

#define __HAL_RGN_COMMON_C__
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "hal_rgn_common.h"
#include "hal_rgn_util.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define RGN_REG_CMDQ_MD 0 //I2m
#define RGN_CMDQ_ONESHOTLOCK() //(CamOsTsemDown(&gstRgnTsem))
#define RGN_CMDQ_ONESHOTUNLOCK() //(CamOsTsemUp(&gstRgnTsem))
#define RGN_CMDQ_ONESHOTCNT (32)
#define RGN_CMDQ_ST_SIZE() (sizeof(MHAL_CMDQ_MultiCmdBufMask_t))
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MHAL_CMDQ_CmdqInterface_t _tRgnCmdqLocal[E_HAL_RGN_CMDQ_ID_NUM];
MHAL_CMDQ_MultiCmdBufMask_t *pgstCmdqBuf[E_HAL_RGN_CMDQ_ID_NUM] = {NULL,NULL};
u32 gu32CmdCnt[E_HAL_RGN_CMDQ_ID_NUM];
//CamOsTsem_t gstRgnTsem;
bool gbUseCmdq = 0;
bool gbrgndbglv = 0;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
void HalRgnUpdateReg(void)
{
    HalRgnCmdqIdType_e enType;
    //RGN_CMDQ_ONESHOTLOCK();
    for(enType=E_HAL_RGN_CMDQ_VPE_ID_0;enType<E_HAL_RGN_CMDQ_ID_NUM;enType++)
    {
        if(pgstCmdqBuf[enType] && gu32CmdCnt[enType]>0 && _tRgnCmdqLocal[enType].MHAL_CMDQ_WriteRegCmdqMaskMulti)
        {
            _tRgnCmdqLocal[enType].MHAL_CMDQ_WriteRegCmdqMaskMulti(&_tRgnCmdqLocal[enType],pgstCmdqBuf[enType],gu32CmdCnt[enType]);
            gu32CmdCnt[enType] = 0;
        }
    }
    //RGN_CMDQ_ONESHOTUNLOCK();
}
bool HalRgnCheckBindRation(HalRgnOsdFlag_e *enFlag)
{
    bool bRet = 1;
    u32 u32Flag = *enFlag;
    u16 u16CheckSum = 0;
    u8 idx;
    for(idx=0;idx<HAL_RGN_OSD_NUM;idx++)
    {
        u16CheckSum += (u32Flag&0x1);
        u32Flag = u32Flag>>1;
    }
    //for I6
    if(u16CheckSum !=HAL_RGN_GOP_NUM ||
    (*enFlag&E_HAL_RGN_OSD_FLAG_PORT3))
    {
        bRet = 0;
        HALRGNERR("[GOP]%s %d: Bind Fail \n", __FUNCTION__, __LINE__);
    }
    else
    {
        bRet = 1;
    }
    return bRet;
}
void HalRgnWrite2Byte(u32 u32Reg, u16 u16Val,HalRgnCmdqIdType_e eCmdqId)
{
    if(pgstCmdqBuf[eCmdqId] && eCmdqId < E_HAL_RGN_CMDQ_ID_NUM && RGN_REG_CMDQ_MD && _tRgnCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdq)
    {
        //_tRgnCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdq(&_tRgnCmdqLocal[eCmdqId],u32Reg,u16Val);
        //RGN_CMDQ_ONESHOTLOCK();
        pgstCmdqBuf[eCmdqId][gu32CmdCnt[eCmdqId]].u16Mask = 0xFFFF;
        pgstCmdqBuf[eCmdqId][gu32CmdCnt[eCmdqId]].u16RegValue = u16Val;
        pgstCmdqBuf[eCmdqId][gu32CmdCnt[eCmdqId]].u32RegAddr = u32Reg;
        gu32CmdCnt[eCmdqId]++;
        if(gu32CmdCnt[eCmdqId]==RGN_CMDQ_ONESHOTCNT)
        {
            HALRGNERR("[GOP]Cnt overflow\n");
        }
        //RGN_CMDQ_ONESHOTUNLOCK();
    }
    else
    {
        W2BYTE(u32Reg, u16Val);
        if((u32Reg&0xFF) != 0xFE)
        HALHLRGNDBG("[GOP]%s %d: reg:%x val:%hx \n", __FUNCTION__, __LINE__,(u32Reg&0xFFFF00) + ((u32Reg&0xFF)/2),u16Val);
    }
}
u16 HalRgnRead2Byte(u32 u32Reg)
{
    return R2BYTE(u32Reg);
}

void HalRgnWrite2ByteMsk(u32 u32Reg, u16 u16Val, u16 u16Mask,HalRgnCmdqIdType_e eCmdqId)
{
    if(pgstCmdqBuf[eCmdqId] && eCmdqId < E_HAL_RGN_CMDQ_ID_NUM && RGN_REG_CMDQ_MD && _tRgnCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdqMask)
    {
        //_tRgnCmdqLocal[eCmdqId].MHAL_CMDQ_WriteRegCmdqMask(&_tRgnCmdqLocal[eCmdqId],u32Reg,u16Val,u16Mask);
        //RGN_CMDQ_ONESHOTLOCK();
        pgstCmdqBuf[eCmdqId][gu32CmdCnt[eCmdqId]].u16RegValue = u16Val;
        pgstCmdqBuf[eCmdqId][gu32CmdCnt[eCmdqId]].u16Mask = u16Mask;
        pgstCmdqBuf[eCmdqId][gu32CmdCnt[eCmdqId]].u32RegAddr = u32Reg;
        gu32CmdCnt[eCmdqId]++;
        if(gu32CmdCnt[eCmdqId]==RGN_CMDQ_ONESHOTCNT)
        {
            HALRGNERR("[GOP]Cnt overflow\n");
        }
        //RGN_CMDQ_ONESHOTUNLOCK();
    }
    else
    {
        //u32ValandMask = (u32)(u16Val | (u32)((~u16Mask)<<16));
        W2BYTEMSK(u32Reg, u16Val,u16Mask);
        if((u32Reg&0xFF) != 0xFE)
        HALHLRGNDBG("[GOP]%s %d: reg:%x val:%hx mask:%hx\n", __FUNCTION__, __LINE__,(u32Reg&0xFFFF00) + ((u32Reg&0xFF)/2),u16Val,u16Mask);
    }
}
void HalRgnInitCmdq(void)
{
    memset(&_tRgnCmdqLocal[0],0,sizeof(MHAL_CMDQ_CmdqInterface_t)*E_HAL_RGN_CMDQ_ID_NUM);
    HALRGNDBG("[GOP]%s %d: out \n", __FUNCTION__, __LINE__);
}
void HalRgnUseCmdq(bool bEn)
{
    gbUseCmdq = bEn;
}

void HalRgnInit(void)
{
    HalRgnCmdqIdType_e enType;
    // if multi cmdq for vpe,maybe need to mutex lock.
    //CamOsTsemInit(&gstRgnTsem,1);
    //RGN_CMDQ_ONESHOTLOCK();
    for(enType=E_HAL_RGN_CMDQ_VPE_ID_0;enType<E_HAL_RGN_CMDQ_ID_NUM;enType++)
    {
        if(!pgstCmdqBuf[enType])
        {
            pgstCmdqBuf[enType] = CamOsMemAlloc(RGN_CMDQ_ST_SIZE()*RGN_CMDQ_ONESHOTCNT);
            gu32CmdCnt[enType] = 0;
        }
    }
    HalRgnUseCmdq(1);
    //RGN_CMDQ_ONESHOTUNLOCK();
}
void HalRgnDeinit(void)
{
    HalRgnCmdqIdType_e enType;
    // if multi cmdq for vpe,maybe need to mutex lock.
    //CamOsTsemInit(&gstRgnTsem,1);
    //RGN_CMDQ_ONESHOTLOCK();
    for(enType=E_HAL_RGN_CMDQ_VPE_ID_0;enType<E_HAL_RGN_CMDQ_ID_NUM;enType++)
    {
        if(pgstCmdqBuf[enType])
        {
            CamOsMemRelease(pgstCmdqBuf[enType]);
            pgstCmdqBuf[enType] = NULL;
            gu32CmdCnt[enType] = 0;
        }
    }
    HalRgnUseCmdq(0);
    //RGN_CMDQ_ONESHOTUNLOCK();
}
void HalRgnSetCmdq(MHAL_CMDQ_CmdqInterface_t *pstCmdq,HalRgnCmdqIdType_e eHalCmdqId)
{
    if(eHalCmdqId >= E_HAL_RGN_CMDQ_ID_NUM)
    {
        HALRGNERR("[GOP]%s %d: wrong CMDQId\n", __FUNCTION__, __LINE__);
        return ;
    }
    if(pstCmdq)
    {
        memcpy(&_tRgnCmdqLocal[eHalCmdqId],pstCmdq,sizeof(MHAL_CMDQ_CmdqInterface_t));
    }
    else
    {
        if(_tRgnCmdqLocal[eHalCmdqId].MHAL_CMDQ_WriteRegCmdqMask != NULL)
        {
            memset(&_tRgnCmdqLocal[eHalCmdqId],0,sizeof(MHAL_CMDQ_CmdqInterface_t));
        }
    }
}
