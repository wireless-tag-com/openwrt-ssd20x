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

#define __MHAL_RGN_C__
#include "drv_rgn_common.h"
#include "mhal_rgn.h"
#include "drv_gop.h"
#include "drv_cover.h"
#include "drv_osd.h"
#include "drv_rgn_os.h"
#include "rgn_capdef.h"
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
unsigned long geGopIdFromOsd = RGN_CHIP_GOPID_DEFAULT;
unsigned long geGopOsdFlag = RGN_CHIP_OSD_DEFAULTFLAGBIND;
DrvRgnInitType_e geRgnInit = 0;

MS_BOOL _MHAL_RGN_CoverTransId(MHAL_RGN_CoverType_e eCoverId, DrvCoverIdType_e *pDrvId)
{
    MS_BOOL bRet = TRUE;

    switch(eCoverId)
    {
        case E_MHAL_COVER_VPE_PORT0:
            *pDrvId = E_DRV_ISPSC0_COVER;
            break;

        case E_MHAL_COVER_VPE_PORT1:
            *pDrvId = E_DRV_ISPSC1_COVER;
            break;

        case E_MHAL_COVER_VPE_PORT2:
            *pDrvId = E_DRV_ISPSC2_COVER;
            break;

        case E_MHAL_COVER_VPE_PORT3:
            *pDrvId = E_DRV_ISPSC3_COVER;
            break;

        case E_MHAL_COVER_DIVP_PORT0:
            *pDrvId = E_DRV_DIP_COVER;
            break;

        case E_MHAL_COVER_TYPE_MAX:
        default:
            *pDrvId = E_DRV_COVER_ID_NUM;
            bRet = FALSE;
            break;

    }
    return bRet;
}

MS_BOOL _MHAL_RGN_CmdqCoverTransId(MHAL_RGN_CmdqType_e eCoverCmdId, DrvCoverCmdqIdType_e *pDrvId)
{
    MS_BOOL bRet = TRUE;

    switch(eCoverCmdId)
    {
        case E_MHAL_RGN_VPE_CMDQ:
            *pDrvId = E_DRV_COVER_CMDQ_VPE_ID_0;
            break;

        case E_MHAL_RGN_DIVP_CMDQ:
            *pDrvId = E_DRV_COVER_CMDQ_DIVP_ID_1;
            break;

        default:
            *pDrvId = E_DRV_COVER_CMDQ_ID_NUM;
            bRet = FALSE;
            break;

    }
    return bRet;
}

MS_BOOL _MHAL_RGN_CmdqGopTransId(MHAL_RGN_CmdqType_e eGopCmdId, DrvRgnCmdqIdType_e *pDrvId)
{
    MS_BOOL bRet = TRUE;

    switch(eGopCmdId)
    {
        case E_MHAL_RGN_VPE_CMDQ:
            *pDrvId = E_DRV_RGN_VPE_CMDQ_ID_0;
            break;

        case E_MHAL_RGN_DIVP_CMDQ:
            *pDrvId = E_DRV_RGN_DIVP_CMDQ_ID_1;
            break;
        case E_MHAL_RGN_LDC_CMDQ:
            *pDrvId = E_DRV_RGN_LDC_CMDQ_ID_2;
            break;
        default:
            *pDrvId = E_DRV_RGN_CMDQ_ID_NUM;
            bRet = FALSE;
            break;

    }
    return bRet;
}
void _MHAL_RGN_SetGopIdFromOsdFlag(void)
{
    int enId;
    geGopIdFromOsd = 0;
    for(enId = E_MHAL_GOP_VPE_PORT0;enId<E_MHAL_GOP_TYPE_MAX;enId++)
    {
        geGopIdFromOsd |=
            ((RGN_CHIP_GET_GOPID_FROM_BINDFLAG(enId,geGopOsdFlag))<<(enId*RGN_CHIP_GOP_ID_SHIFTBIT));
    }

    DRVRGNDBG("%s %d: ID %lx\n", __FUNCTION__, __LINE__,geGopIdFromOsd);
}
MS_BOOL _MHAL_RGN_GopTransId(MHAL_RGN_GopType_e eGopId, DrvGopIdType_e *pDrvId)
{
    MS_BOOL bRet = TRUE;
    //*pDrvId = RGN_CHIP_GET_GOPID_FROM_BINDFLAG(eGopId,geGopOsdFlag);
    *pDrvId = ((geGopIdFromOsd &(0xF<<(eGopId*RGN_CHIP_GOP_ID_SHIFTBIT)))>>(eGopId*RGN_CHIP_GOP_ID_SHIFTBIT));
    if(*pDrvId==E_DRV_GOP_UNBIND)
    {
        bRet = 0;
    }
    DRVRGNHLDBG("%s %d: ID %lx %x\n", __FUNCTION__, __LINE__,geGopIdFromOsd,*pDrvId);
    return bRet;
}
MS_BOOL _MHAL_RGN_OsdTransId(MHAL_RGN_GopType_e eGopId, DrvOsdId_e *pDrvId)
{
    MS_BOOL bRet = TRUE;
    *pDrvId = (DrvOsdId_e)eGopId;
    if(*pDrvId==E_DRV_OSD_ID_MAX)
    {
        bRet = 0;
    }
    return bRet;
}

MS_BOOL _MHAL_RGN_GopTransSrcFmt(MHAL_RGN_GopPixelFormat_e eRgnSrcFmt, DrvGopGwinSrcFmtType_e *pDrvSrcFmt)
{
    MS_BOOL bRet = TRUE;

    switch(eRgnSrcFmt)
    {
        case E_MHAL_RGN_PIXEL_FORMAT_ARGB1555:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_ARGB1555;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_ARGB4444:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_ARGB4444;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_I2:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_I2_PALETTE;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_I4:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_I4_PALETTE;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_I8:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_I8_PALETTE;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_RGB565:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_RGB565;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_ARGB8888:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_ARGB8888;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_UV8Y8:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_UV8Y8;
            break;

        case E_MHAL_RGN_PIXEL_FORMAT_MAX:
        default:
            *pDrvSrcFmt = E_DRV_GOP_GWIN_SRC_FMT_NUM;
            bRet = FALSE;
            break;

    }
    return bRet;
}

/*Gop Bind Osd (can't dynamic)*/
MS_S32 MHAL_RGN_SetGopBindOsd(MS_U32 eGopNum)
{
    bool bRet = MHAL_SUCCESS;
    if(!DrvRgnSetOsdBindGop(eGopNum))
    {
        bRet = MHAL_FAILURE;
    }
    else
    {
        _MHAL_RGN_SetGopIdFromOsdFlag();
        DrvOsdSetBindGop(eGopNum);
        DrvGopSetBindOsd();
        DrvRgnUpdateReg();
    }
    return bRet;
}
MS_S32 MHAL_RGN_GetGopBindOsd(MS_U32 *eGopNum)
{
    DrvRgnGetOsdBindGop(eGopNum);
    return MHAL_SUCCESS;
}

/*Setup cmdq*/
MS_S32 MHAL_RGN_SetupCmdQ(MHAL_CMDQ_CmdqInterface_t* pstCmdInf,MHAL_RGN_CmdqType_e eRgnCmdqId)
{
    DrvRgnCmdqIdType_e eCmdqId;
    if(_MHAL_RGN_CmdqGopTransId(eRgnCmdqId,&eCmdqId))
    {
        DrvRgnSetCmdq(pstCmdInf,eCmdqId);
    }
    else
    {
        return MHAL_FAILURE;
    }
    return MHAL_SUCCESS;
}

/*Cover init*/
MS_S32 MHAL_RGN_CoverInit(void)
{
    DrvRgnInit();
    DrvCoverRgnInit();
    geRgnInit |= E_DRV_RGN_INIT_COVER;
    DrvRgnUpdateReg();
    return MHAL_SUCCESS;
}
MS_S32 MHAL_RGN_CoverDeinit(void)
{
    DrvRgnUseCmdq(0);
    DrvCoverRgnInit();
    DrvRgnUseCmdq(1);
    geRgnInit &= (~E_DRV_RGN_INIT_COVER);
    if(!geRgnInit)
    {
        DrvRgnDeinit();
    }
    DrvRgnUpdateReg();
    return MHAL_SUCCESS;
}

/*Set cover color NA:[31~24] Cr:[23~16] Y:[15~8] Cb:[7~0]*/
MS_S32 MHAL_RGN_CoverSetColor(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer, MS_U32 u32Color)
{
    DrvCoverIdType_e eDrvCoverId;
    DrvCoverWinIdType_e eDrvWinId;
    DrvCoverColorConfig_t tColorCfg;

    if(_MHAL_RGN_CoverTransId(eCoverId, &eDrvCoverId) == TRUE)
    {
        eDrvWinId = (DrvCoverWinIdType_e)eLayer;
        tColorCfg.u8B = (MS_U8)(u32Color & 0xFF);
        tColorCfg.u8G = (MS_U8)((u32Color>>8) & 0xFF);
        tColorCfg.u8R = (MS_U8)((u32Color>>16) & 0xFF);
        DrvCoverSetColor(eDrvCoverId, eDrvWinId, &tColorCfg);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Cover  size*/
MS_S32 MHAL_RGN_CoverSetWindow(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer, MS_U32 u32X, MS_U32 u32Y, MS_U32 u32Width, MS_U32 u32Height)
{
    DrvCoverIdType_e eDrvCoverId;
    DrvCoverWinIdType_e eDrvWinId;
    DrvCoverWindowConfig_t tCoverWinCfg;

    if(_MHAL_RGN_CoverTransId(eCoverId, &eDrvCoverId) == TRUE)
    {
        eDrvWinId = (DrvCoverWinIdType_e)eLayer;
        tCoverWinCfg.u16X = (MS_U16)u32X;
        tCoverWinCfg.u16Y = (MS_U16)u32Y;
        tCoverWinCfg.u16Width = (MS_U16)u32Width;
        tCoverWinCfg.u16Height = (MS_U16)u32Height;
        DrvCoverSetWinSize(eDrvCoverId, eDrvWinId, &tCoverWinCfg);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Enable cover*/
MS_S32 MHAL_RGN_CoverEnable(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer)
{
    DrvCoverIdType_e eDrvCoverId;
    DrvCoverWinIdType_e eDrvWinId;
    MS_BOOL bEn = TRUE;

    if(_MHAL_RGN_CoverTransId(eCoverId, &eDrvCoverId) == TRUE)
    {
        eDrvWinId = (DrvCoverWinIdType_e)eLayer;
        DrvCoverSetEnableWin(eDrvCoverId, eDrvWinId, bEn);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Disable cover*/
MS_S32 MHAL_RGN_CoverDisable(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer)
{
    DrvCoverIdType_e eDrvCoverId;
    DrvCoverWinIdType_e eDrvWinId;
    MS_BOOL bDisEn = FALSE;

    if(_MHAL_RGN_CoverTransId(eCoverId, &eDrvCoverId) == TRUE)
    {
        eDrvWinId = (DrvCoverWinIdType_e)eLayer;
        DrvCoverSetEnableWin(eDrvCoverId, eDrvWinId, bDisEn);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Gop init*/
MS_S32 MHAL_RGN_GopInit(void)
{
    DrvRgnInit();
    if(!DrvOsdInit())
    {
        geGopOsdFlag = RGN_CHIP_OSD_DEFAULTFLAGBIND;
    }
    _MHAL_RGN_SetGopIdFromOsdFlag();
    DrvGopRgnInit();
    geRgnInit |= E_DRV_RGN_INIT_GOP;
    DrvRgnUpdateReg();
    return MHAL_SUCCESS;
}
MS_S32 MHAL_RGN_GopDeinit(void)
{
    DrvRgnUseCmdq(0);
    DrvGopRgnInit();
    DrvOsdInit();
    DrvRgnUseCmdq(1);
    geRgnInit &= (~E_DRV_RGN_INIT_GOP);
    geGopOsdFlag = RGN_CHIP_OSD_DEFAULTFLAGBIND;
    _MHAL_RGN_SetGopIdFromOsdFlag();
    if(!geRgnInit)
    {
        DrvRgnDeinit();
    }
    DrvRgnUpdateReg();
    return MHAL_SUCCESS;
}

/*Set palette*/
MS_S32 MHAL_RGN_GopSetPalette(MHAL_RGN_GopType_e eGopId, MS_U8 u8Alpha, MS_U8 u8Red, MS_U8 u8Green, MS_U8 u8Blue, MS_U8 u8Idx)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopPaletteConfig_t tCfg;
	static bool bForceClk[E_MHAL_GOP_TYPE_MAX] = {0,0,0,0,0};
    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        tCfg.u8Idx = u8Idx;
        tCfg.u8A = u8Alpha;
        tCfg.u8B = u8Blue;
        tCfg.u8G = u8Green;
        tCfg.u8R = u8Red;
        if(u8Idx == 0)
        {
		    bForceClk[eGopId] = DrvRgnOsSetGopClkEnable(eDrvGopId);
        }
        DrvGopSetPaletteRiuOneMem(eDrvGopId, &tCfg);
        if(u8Idx == FINAL_PALETTE_NUM)
        {
		    DrvRgnOsSetGopClkDisable(eDrvGopId,bForceClk[eGopId]);
		    bForceClk[eGopId] = 0;
        }
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*Set base gop window size*/
MS_S32 MHAL_RGN_GopSetBaseWindow(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopWindowConfig_t *ptSrcWinCfg, MHAL_RGN_GopWindowConfig_t *ptDstWinCfg)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopWindowConfig_t tDrvSrcWinCfg, tDrvDstWinCfg;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        tDrvSrcWinCfg.u16X      = (MS_U16)(ptSrcWinCfg->u32X);
        tDrvSrcWinCfg.u16Y      = (MS_U16)(ptSrcWinCfg->u32Y);
        tDrvSrcWinCfg.u16Width  = (MS_U16)(ptSrcWinCfg->u32Width);
        tDrvSrcWinCfg.u16Height = (MS_U16)(ptSrcWinCfg->u32Height);
        tDrvDstWinCfg.u16X      = (MS_U16)(ptDstWinCfg->u32X);
        tDrvDstWinCfg.u16Y      = (MS_U16)(ptDstWinCfg->u32Y);
        tDrvDstWinCfg.u16Width  = (MS_U16)(ptDstWinCfg->u32Width);
        tDrvDstWinCfg.u16Height = (MS_U16)(ptDstWinCfg->u32Height);
        DrvGopSetStretchWindowSize(eDrvGopId, &tDrvSrcWinCfg, &tDrvDstWinCfg);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/* Set mirror flip */
MS_S32 MHAL_RGN_GopSetMirFlip(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopMirFlip_e eMode)
{
    DrvGopIdType_e eDrvGopId;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopSetMirror(eDrvGopId, (eMode&E_MHAL_RGN_MIRFLIP_MIRROR)?TRUE:FALSE);
        DrvGopSetFlip(eDrvGopId, (eMode&E_MHAL_RGN_MIRFLIP_FLIP)?TRUE:FALSE);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*Set base gop gwin pix format*/
MS_S32 MHAL_RGN_GopGwinSetPixelFormat(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopPixelFormat_e eFormat)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopGwinSrcFmtType_e eSrcFmt;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        if(_MHAL_RGN_GopTransSrcFmt(eFormat, &eSrcFmt)== TRUE)
        {
            if(DrvGopSetSrcFmt(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, eSrcFmt))
            {
                DrvRgnUpdateReg();
                return MHAL_SUCCESS;
            }
            else
            {
                return MHAL_FAILURE;
            }
        }
        else
        {
            return MHAL_FAILURE;
        }
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*Set base gop gwin osd window*/
MS_S32 MHAL_RGN_GopGwinSetWindow(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_U32 u32Width, MS_U32 u32Height, MS_U32 u32Stride, MS_U32 u32X, MS_U32 u32Y)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopWindowConfig_t tDrvWinCfg;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        //set gwin size
        tDrvWinCfg.u16X = (MS_U16)u32X;
        tDrvWinCfg.u16Y = (MS_U16)u32Y;
        tDrvWinCfg.u16Width = (MS_U16)u32Width;
        tDrvWinCfg.u16Height = (MS_U16)u32Height;
        DrvGopSetGwinSizeWithoutFmt(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, &tDrvWinCfg);
        // set memory pitch
        DrvGopSetMemPitchDirect(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, u32Stride);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}
/*Set base gop gwin buffer H offset*/
MS_S32 MHAL_RGN_GopGwinSetBufferXoffset(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_U16 u16Xoffset)
{
    DrvGopIdType_e eDrvGopId;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopUpdateBufferXoffset(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, u16Xoffset);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*Set base gop gwin osd bufdfer addr*/
MS_S32 MHAL_RGN_GopGwinSetBuffer(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_PHYADDR phyAddr)
{
    DrvGopIdType_e eDrvGopId;
    MS_U32 u32BaseAddr=0;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        u32BaseAddr = phyAddr;
        DrvGopUpdateBase(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, u32BaseAddr);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

/*enable Gwin*/
MS_S32 MHAL_RGN_GopGwinEnable(MHAL_RGN_GopType_e eGopId,  MHAL_RGN_GopGwinId_e eGwinId)
{
    DrvGopIdType_e eDrvGopId;
    DrvOsdId_e eDrvOsdId;
    MS_BOOL bEn = TRUE;
    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopRgnSetEnable(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, bEn);
        _MHAL_RGN_OsdTransId(eGopId,&eDrvOsdId);
        DrvOsdSetEnable(eDrvOsdId,bEn);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*disable Gwin*/
MS_S32 MHAL_RGN_GopGwinDisable(MHAL_RGN_GopType_e eGopId,  MHAL_RGN_GopGwinId_e eGwinId)
{
    DrvGopIdType_e eDrvGopId;
    DrvOsdId_e eDrvOsdId;
    MS_BOOL bEn = FALSE;
    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopRgnSetEnable(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, bEn);
        if(DrvGopGetGwinEnableCount(eDrvGopId)==0)
        {
            _MHAL_RGN_OsdTransId(eGopId,&eDrvOsdId);
            DrvOsdSetEnable(eDrvOsdId,bEn);
        }
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*Set gop colorkey*/
MS_S32 MHAL_RGN_GopSetColorkey(MHAL_RGN_GopType_e eGopId, MS_BOOL bEn, MS_U8 u8R, MS_U8 u8G, MS_U8 u8B)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopColorKeyConfig_t tColorKeyCfg;
    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        tColorKeyCfg.bEn = bEn;
        tColorKeyCfg.u8R = u8R;
        tColorKeyCfg.u8G = u8G;
        tColorKeyCfg.u8B = u8B;
        DrvGopSetColorKey(eDrvGopId, &tColorKeyCfg, FALSE);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*Set gop alpha blending type*/
MS_S32 MHAL_RGN_GopSetAlphaType(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopGwinAlphaType_e eAlphaType, MS_U8 u8ConstAlphaVal)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopGwinAlphaConfig_t tAlphaCfg;

    switch(eAlphaType)
    {
        case E_MHAL_GOP_GWIN_ALPHA_CONSTANT:
            tAlphaCfg.eAlphaType = E_DRV_GOP_GWIN_ALPHA_CONSTANT;
            break;

        case E_MHAL_GOP_GWIN_ALPHA_PIXEL:
            tAlphaCfg.eAlphaType = E_DRV_GOP_GWIN_ALPHA_PIXEL;
            break;

        default:
            return MHAL_FAILURE;
            break;

    }
    tAlphaCfg.tAlphaVal.u8ConstantAlpahValue = u8ConstAlphaVal;
    tAlphaCfg.ePixelAlphaType = E_DRV_GOP_GWIN_ARGB1555_DEFINE_NOUSE;
    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopSetAlphaBlending(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, &tAlphaCfg);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

/*Set gop ARGB1555 alpha value for alpha0 or alpha1*/
MS_S32 MHAL_RGN_GopSetArgb1555AlphaVal(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopGwinArgb1555Def_e eAlphaType, MS_U8 u8AlphaVal)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopGwinAlphaConfig_t tAlphaCfg;

    switch(eAlphaType)
    {
        case E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0:
            tAlphaCfg.ePixelAlphaType = E_DRV_GOP_GWIN_ARGB1555_DEFINE_ALPHA0;
            tAlphaCfg.tAlphaVal.u8Argb1555Alpha0Val = u8AlphaVal;
            break;

        case E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1:
            tAlphaCfg.ePixelAlphaType = E_DRV_GOP_GWIN_ARGB1555_DEFINE_ALPHA1;
            tAlphaCfg.tAlphaVal.u8Argb1555Alpha1Val = u8AlphaVal;
            break;

        default:
            return MHAL_FAILURE;
            break;

    }
    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        DrvGopSetAlphaBlending(eDrvGopId, (DrvGopGwinIdType_e)eGwinId, &tAlphaCfg);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

MS_S32 MHAL_RGN_GopSetColorInverseEnable(MHAL_RGN_GopType_e eGopId, MS_BOOL bEn)
{
    DrvOsdId_e eDrvOsdId;
    if(_MHAL_RGN_OsdTransId(eGopId,&eDrvOsdId))
    {
        DrvOsdSetColorInverseEn(eDrvOsdId, bEn);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

MS_S32 MHAL_RGN_GopSetColorInverseParam(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopColorInvWindowIdType_e eAeId, MHAL_RGN_GopColorInvConfig_t tColInvCfg)
{
    DrvOsdColorInvParamConfig_t tDrvColInvCfg;
    DrvOsdId_e eDrvOsdId;
    if(_MHAL_RGN_OsdTransId(eGopId,&eDrvOsdId))
    {
        tDrvColInvCfg.u16Th1 = tColInvCfg.u16Th1;
        tDrvColInvCfg.u16Th2 = tColInvCfg.u16Th2;
        tDrvColInvCfg.u16Th3 = tColInvCfg.u16Th3;
        tDrvColInvCfg.u16W = tColInvCfg.u16W;
        tDrvColInvCfg.u16H = tColInvCfg.u16H;
        tDrvColInvCfg.u16X = tColInvCfg.u16X;
        tDrvColInvCfg.u16Y = tColInvCfg.u16Y;
        tDrvColInvCfg.u16BlkNumX= tColInvCfg.u16BlkNumX;
        tDrvColInvCfg.u16BlkNumY= tColInvCfg.u16BlkNumY;
        DrvOsdSetColorInverseParam(eDrvOsdId, (DrvOsdColorInvWindowIdType_e)eAeId, &tDrvColInvCfg);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

MS_S32 MHAL_RGN_GopSetColorInverseUpdate(MHAL_RGN_GopType_e eGopId)
{
    DrvOsdId_e eDrvOsdId;
    if(_MHAL_RGN_OsdTransId(eGopId,&eDrvOsdId))
    {
        DrvOsdSetColorInverseUpdate(eDrvOsdId);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

MS_S32 MHAL_RGN_GopWriteColorInverseData(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopColorInvWindowIdType_e eAeId, MS_U16 u16DataLength, MS_U32 *pu32WData)
{
    u16 i;
    u16 startAddr = eAeId*64;
    DrvOsdId_e eDrvOsdId;
    if(_MHAL_RGN_OsdTransId(eGopId,&eDrvOsdId))
    {
        for(i=0;i<u16DataLength;i++)
        {
            DrvOsdWriteColorInverseData(eDrvOsdId,startAddr+i,*(pu32WData+i));
        }
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}
MS_S32 MHAL_RGN_GopReadColorInverseData(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopColorInvWindowIdType_e eAeId, MS_U16 u16DataLength, MS_U32 *pu32RData)
{
    u16 i;
    u16 startAddr = eAeId*64;
    DrvOsdId_e eDrvOsdId;
    if(_MHAL_RGN_OsdTransId(eGopId,&eDrvOsdId))
    {
        for(i=0;i<u16DataLength;i++)
        {
            DrvOsdReadColorInverseData(eDrvOsdId,startAddr+i,(pu32RData+i));
        }
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

MS_S32 MHAL_RGN_GopSetAlphaZeroOpaque(MHAL_RGN_GopType_e eGopId, MS_BOOL bEn, MS_BOOL bConstAlpha, MHAL_RGN_GopPixelFormat_e eFormat)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopGwinSrcFmtType_e eSrcFmt;
    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        _MHAL_RGN_GopTransSrcFmt(eFormat, &eSrcFmt);
        DrvGopSetAlphaZeroOpaque(eDrvGopId, bEn ,bConstAlpha ,eSrcFmt);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }
}

//ToDo: by chip DrvDefine
MS_S32 MHAL_RGN_GetChipCapability(MHAL_RGN_ChipCapType_e eType, MHAL_RGN_GopPixelFormat_e eFormat)
{
    switch(eType) {
        case E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT:
            return RGN_CHIP_GOP_GWIN_FMT_SUPPORT(eFormat);
        case E_MHAL_RGN_CHIP_OSD_OVERLAP_SUPPORT:
            return RGN_CHIP_GOP_GWIN_OVERLAP_SUPPORT;
        case E_MHAL_RGN_CHIP_OSD_XPOS_OVERLAP_SUPPORT:
            return RGN_CHIP_GOP_GWIN_XPOS_OVERLAP_SUPPORT;
        case E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT:   // unit:pixel
            return RGN_CHIP_GOP_GWIN_WIDTH_ALIGNMENT(eFormat);
        case E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT:
            return RGN_CHIP_GOP_GWIN_XPOS_ALIGNMENT(eFormat);
        case E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT:
            return RGN_CHIP_GOP_PITCH_ALIGNMENT; // unit:byte
        case E_MHAL_RGN_CHIP_OSD_HW_GWIN_CNT: // gwin number
            return RGN_CHIP_GOP_HW_GWIN_CNT;
        default:
            return MHAL_FAILURE;
    }
}

MS_S32 MHAL_RGN_GopSetVideoTimingInfo(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopVideoTimingInfoConfig_t *ptTimingCfg)
{
    DrvGopIdType_e eDrvGopId;
    DrvGopVideoTimingInfoConfig_t tDrvVdTmgCfg;

    if(_MHAL_RGN_GopTransId(eGopId, &eDrvGopId) == TRUE)
    {
        tDrvVdTmgCfg.u16Htotal     =ptTimingCfg->u16Htotal;
        tDrvVdTmgCfg.u16Vtotal     =ptTimingCfg->u16Vtotal;
        tDrvVdTmgCfg.u16HdeStart   =ptTimingCfg->u16HdeStart;
        tDrvVdTmgCfg.u16VdeStart   =ptTimingCfg->u16VdeStart;
        tDrvVdTmgCfg.u16Width      =ptTimingCfg->u16Width;
        tDrvVdTmgCfg.u16Height     =ptTimingCfg->u16Height;
        tDrvVdTmgCfg.bInterlaceMode=ptTimingCfg->bInterlaceMode;
        tDrvVdTmgCfg.bYuvOutput    =ptTimingCfg->bYuvOutput;
        DrvGopSetVideoTimingInfo(eDrvGopId, &tDrvVdTmgCfg);
        DrvRgnUpdateReg();
        return MHAL_SUCCESS;
    }
    else
    {
        return MHAL_FAILURE;
    }

}

