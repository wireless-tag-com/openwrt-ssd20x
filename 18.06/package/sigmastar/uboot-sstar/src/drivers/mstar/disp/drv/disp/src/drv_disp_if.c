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

#define _DRV_DISP_IF_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_disp_os.h"
#include "hal_disp_common.h"
#include "disp_debug.h"
#include "mhal_common.h"
#include "mhal_disp_datatype.h"
#include "hal_disp_chip.h"
#include "hal_disp_st.h"
#include "hal_disp_if.h"
#include "drv_disp_ctx.h"
#include "drv_disp_irq.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    bool bValid;
    HalDispDeviceTiming_e   enDevTimingType;
    HalDispDeviceTimingConfig_t stDevTiming;
}DrvDispIfTimingTblConfig_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvDispIfTimingTblConfig_t gstDevTimingTbl[E_HAL_DISP_OUTPUT_MAX];

u32 _gu32DispDbgLevel = 0;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
HalDispDeviceTiming_e __DrvDispIfTransDevTimingTypeToHal(MHAL_DISP_DeviceTiming_e eMhalTiming)
{
    HalDispDeviceTiming_e eHalTiming;

    eHalTiming = eMhalTiming == E_MHAL_DISP_OUTPUT_PAL           ?  E_HAL_DISP_OUTPUT_PAL          :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_NTSC          ?  E_HAL_DISP_OUTPUT_NTSC         :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_960H_PAL      ?  E_HAL_DISP_OUTPUT_960H_PAL     :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_960H_NTSC     ?  E_HAL_DISP_OUTPUT_960H_NTSC    :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1080P24       ?  E_HAL_DISP_OUTPUT_1080P24      :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1080P25       ?  E_HAL_DISP_OUTPUT_1080P25      :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1080P30       ?  E_HAL_DISP_OUTPUT_1080P30      :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_720P50        ?  E_HAL_DISP_OUTPUT_720P50       :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_720P60        ?  E_HAL_DISP_OUTPUT_720P60       :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1080I50       ?  E_HAL_DISP_OUTPUT_1080I50      :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1080I60       ?  E_HAL_DISP_OUTPUT_1080I60      :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1080P50       ?  E_HAL_DISP_OUTPUT_1080P50      :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1080P60       ?  E_HAL_DISP_OUTPUT_1080P60      :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_576P50        ?  E_HAL_DISP_OUTPUT_576P50       :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_480P60        ?  E_HAL_DISP_OUTPUT_480P60       :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_640x480_60    ?  E_HAL_DISP_OUTPUT_640x480_60   :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_800x600_60    ?  E_HAL_DISP_OUTPUT_800x600_60   :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1024x768_60   ?  E_HAL_DISP_OUTPUT_1024x768_60  :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1280x1024_60  ?  E_HAL_DISP_OUTPUT_1280x1024_60 :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1366x768_60   ?  E_HAL_DISP_OUTPUT_1366x768_60  :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1440x900_60   ?  E_HAL_DISP_OUTPUT_1440x900_60  :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1280x800_60   ?  E_HAL_DISP_OUTPUT_1280x800_60  :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1680x1050_60  ?  E_HAL_DISP_OUTPUT_1680x1050_60 :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1920x2160_30  ?  E_HAL_DISP_OUTPUT_1920x2160_30 :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1600x1200_60  ?  E_HAL_DISP_OUTPUT_1600x1200_60 :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_1920x1200_60  ?  E_HAL_DISP_OUTPUT_1920x1200_60 :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_2560x1440_30  ?  E_HAL_DISP_OUTPUT_2560x1440_30 :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_2560x1600_60  ?  E_HAL_DISP_OUTPUT_2560x1600_60 :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_3840x2160_30  ?  E_HAL_DISP_OUTPUT_3840x2160_30 :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_3840x2160_60  ?  E_HAL_DISP_OUTPUT_3840x2160_60 :
                 eMhalTiming == E_MHAL_DISP_OUTPUT_USER          ?  E_HAL_DISP_OUTPUT_USER         :
                                                                    E_HAL_DISP_OUTPUT_MAX;
    return eHalTiming;
}

HalDispCscmatrix_e __DrvDispIfTransCscMatrixToHal(MHAL_DISP_CscMattrix_e eMhalCscMatrixType)
{
    HalDispCscmatrix_e eHalCscMatrixType;

    eHalCscMatrixType =  eMhalCscMatrixType == E_MHAL_DISP_CSC_MATRIX_BYPASS          ? E_HAL_DISP_CSC_MATRIX_BYPASS          :
                         eMhalCscMatrixType == E_MHAL_DISP_CSC_MATRIX_BT601_TO_BT709  ? E_HAL_DISP_CSC_MATRIX_BT601_TO_BT709  :
                         eMhalCscMatrixType == E_MHAL_DISP_CSC_MATRIX_BT709_TO_BT601  ? E_HAL_DISP_CSC_MATRIX_BT709_TO_BT601  :
                         eMhalCscMatrixType == E_MHAL_DISP_CSC_MATRIX_BT601_TO_RGB_PC ? E_HAL_DISP_CSC_MATRIX_BT601_TO_RGB_PC :
                         eMhalCscMatrixType == E_MHAL_DISP_CSC_MATRIX_BT709_TO_RGB_PC ? E_HAL_DISP_CSC_MATRIX_BT709_TO_RGB_PC :
                         eMhalCscMatrixType == E_MHAL_DISP_CSC_MATRIX_RGB_TO_BT601_PC ? E_HAL_DISP_CSC_MATRIX_RGB_TO_BT601_PC :
                         eMhalCscMatrixType == E_MHAL_DISP_CSC_MATRIX_RGB_TO_BT709_PC ? E_HAL_DISP_CSC_MATRIX_RGB_TO_BT709_PC :
                                                                                        E_HAL_DISP_CSC_MATRIX_MAX;
    return eHalCscMatrixType;
}

HalDispPixelCompressMode_e __DrvDispIfTransCompressToHal(MHAL_DISP_PixelCompressMode_e eMhalCompress)
{
    HalDispPixelCompressMode_e eHalCompress;

    eHalCompress = eMhalCompress == E_MHAL_DISP_COMPRESS_MODE_NONE  ?  E_HAL_DISP_COMPRESS_MODE_NONE  :
                   eMhalCompress == E_MHAL_DISP_COMPRESS_MODE_SEG   ?  E_HAL_DISP_COMPRESS_MODE_SEG   :
                   eMhalCompress == E_MHAL_DISP_COMPRESS_MODE_LINE  ?  E_HAL_DISP_COMPRESS_MODE_LINE  :
                   eMhalCompress == E_MHAL_DISP_COMPRESS_MODE_FRAME ?  E_HAL_DISP_COMPRESS_MODE_FRAME :
                                                                       E_HAL_DISP_COMPRESS_MODE_MAX;
    return eHalCompress;
}

HalDispTileMode_e __DrvDispIfTransTileModeToHal(MHAL_DISP_TileMode_e eMhalTile)
{
    HalDispTileMode_e eHalTile;

    eHalTile = eMhalTile == E_MHAL_DISP_TILE_MODE_16x16 ? E_HAL_DISP_TILE_MODE_16x16 :
               eMhalTile == E_MHAL_DISP_TILE_MODE_16x32 ? E_HAL_DISP_TILE_MODE_16x32 :
               eMhalTile == E_MHAL_DISP_TILE_MODE_32x16 ? E_HAL_DISP_TILE_MODE_32x16 :
               eMhalTile == E_MHAL_DISP_TILE_MODE_32x32 ? E_HAL_DISP_TILE_MODE_32x32 :
                                                            E_HAL_DISP_TILE_MODE_NONE;
    return eHalTile;
}

HalDispPixelFormat_e __DrvDispIfTransPixelFormatToHal(MHalPixelFormat_e eMhalPixelFormat)
{
    HalDispPixelFormat_e eHalPixelFormat;

    eHalPixelFormat = eMhalPixelFormat ==  E_MHAL_PIXEL_FRAME_YUV422_YUYV        ? E_HAL_DISP_PIXEL_FRAME_YUV422_YUYV        :
                      eMhalPixelFormat ==  E_MHAL_PIXEL_FRAME_ARGB8888           ? E_HAL_DISP_PIXEL_FRAME_ARGB8888           :
                      eMhalPixelFormat ==  E_MHAL_PIXEL_FRAME_ABGR8888           ? E_HAL_DISP_PIXEL_FRAME_ABGR8888           :
                      eMhalPixelFormat ==  E_MHAL_PIXEL_FRAME_BGRA8888           ? E_HAL_DISP_PIXEL_FRAME_BGRA8888           :
                      eMhalPixelFormat ==  E_MHAL_PIXEL_FRAME_RGB565             ? E_HAL_DISP_PIXEL_FRAME_RGB565             :
                      eMhalPixelFormat ==  E_MHAL_PIXEL_FRAME_ARGB1555           ? E_HAL_DISP_PIXEL_FRAME_ARGB1555           :
                      eMhalPixelFormat ==  E_MHAL_PIXEL_FRAME_ARGB4444           ? E_HAL_DISP_PIXEL_FRAME_ARGB4444           :
                      eMhalPixelFormat ==  E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422 ? E_HAL_DISP_PIXEL_FRAME_YUV_SEMIPLANAR_422 :
                      eMhalPixelFormat ==  E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420 ? E_HAL_DISP_PIXEL_FRAME_YUV_SEMIPLANAR_420 :
                      eMhalPixelFormat ==  E_MHAL_PIXEL_FRAME_YUV_MST_420        ? E_HAL_DISP_PIXEL_FRAME_YUV_MST_420        :
                                                                                  E_HAL_DISP_PIXEL_FRAME_FORMAT_MAX;
    return eHalPixelFormat;
}

HalDispDeviceTimingConfig_t * __DrvDispIfGetTimingFromTbl(HalDispDeviceTiming_e enHalTiminId)
{
    u32 i;
    HalDispDeviceTimingConfig_t *pstTimingCfg = NULL;

    for(i=0; i<E_HAL_DISP_OUTPUT_MAX; i++)
    {
        if(gstDevTimingTbl[i].bValid && gstDevTimingTbl[i].enDevTimingType == enHalTiminId)
        {
            pstTimingCfg = &gstDevTimingTbl[i].stDevTiming;
            break;
        }
    }

    return pstTimingCfg;
}

bool _DrvDispIfTransDeviceOutpuTimingInfoToHal(MHAL_DISP_DeviceTimingInfo_t *pstMhalCfg, HalDispDeviceTimingInfo_t *pstHalCfg)
{
    bool bRet = 1;
    HalDispDeviceTimingConfig_t *pstDeviceTimingCfg = NULL;

    pstHalCfg->eTimeType = __DrvDispIfTransDevTimingTypeToHal(pstMhalCfg->eTimeType);

    if(pstHalCfg->eTimeType == E_HAL_DISP_OUTPUT_USER)
    {
        if(pstMhalCfg->pstSyncInfo)
        {
            pstHalCfg->stDeviceTimingCfg.u16HsyncWidth     = pstMhalCfg->pstSyncInfo->u16Hpw;
            pstHalCfg->stDeviceTimingCfg.u16HsyncBackPorch = pstMhalCfg->pstSyncInfo->u16Hbb;
            pstHalCfg->stDeviceTimingCfg.u16Hactive        = pstMhalCfg->pstSyncInfo->u16Hact;
            pstHalCfg->stDeviceTimingCfg.u16Hstart         = pstMhalCfg->pstSyncInfo->u16Hpw + pstMhalCfg->pstSyncInfo->u16Hbb;
            pstHalCfg->stDeviceTimingCfg.u16Htotal         = pstMhalCfg->pstSyncInfo->u16Hpw + pstMhalCfg->pstSyncInfo->u16Hbb +
                                                             pstMhalCfg->pstSyncInfo->u16Hact + pstMhalCfg->pstSyncInfo->u16Hfb;
            pstHalCfg->stDeviceTimingCfg.u16VsyncWidth     = pstMhalCfg->pstSyncInfo->u16Vpw;
            pstHalCfg->stDeviceTimingCfg.u16VsyncBackPorch = pstMhalCfg->pstSyncInfo->u16Vbb;
            pstHalCfg->stDeviceTimingCfg.u16Vactive        = pstMhalCfg->pstSyncInfo->u16Vact;
            pstHalCfg->stDeviceTimingCfg.u16Vstart         = pstMhalCfg->pstSyncInfo->u16Vpw + pstMhalCfg->pstSyncInfo->u16Vbb;
            pstHalCfg->stDeviceTimingCfg.u16Vtotal         = pstMhalCfg->pstSyncInfo->u16Vpw + pstMhalCfg->pstSyncInfo->u16Vbb +
                                                             pstMhalCfg->pstSyncInfo->u16Vact + pstMhalCfg->pstSyncInfo->u16Vfb;
            pstHalCfg->stDeviceTimingCfg.u16Fps            = pstMhalCfg->pstSyncInfo->u32FrameRate;
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, SyncInfo is Null\n ", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        pstDeviceTimingCfg = __DrvDispIfGetTimingFromTbl(pstHalCfg->eTimeType);
        if(pstDeviceTimingCfg)
        {
            memcpy(&pstHalCfg->stDeviceTimingCfg, pstDeviceTimingCfg, sizeof(HalDispDeviceTimingConfig_t));
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, Can't find Timing(%s) in Table\n",
                __FUNCTION__, __LINE__, PARSING_HAL_TMING_ID(pstHalCfg->eTimeType));
        }
    }
    return bRet;
}

void _DrvDispIfTransDeviceCvbsParamToHal(MHAL_DISP_CvbsParam_t *pstMhalCfg, HalDispCvbsParam_t *pstHalCfg)
{
    pstHalCfg->bEnable= pstMhalCfg->bEnable;
    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, En:%d\n",
        __FUNCTION__, __LINE__,  pstHalCfg->bEnable);

}

void _DrvDispIfTransDeviceVgaParamToHal(MHAL_DISP_VgaParam_t *pstMhalCfg, HalDispVgaParam_t *pstHalCfg)
{
    pstHalCfg->stCsc.eCscMatrix    = __DrvDispIfTransCscMatrixToHal(pstMhalCfg->stCsc.eCscMatrix);
    pstHalCfg->stCsc.u32Luma       = pstMhalCfg->stCsc.u32Luma;
    pstHalCfg->stCsc.u32Hue        = pstMhalCfg->stCsc.u32Hue;
    pstHalCfg->stCsc.u32Contrast   = pstMhalCfg->stCsc.u32Contrast;
    pstHalCfg->stCsc.u32Saturation = pstMhalCfg->stCsc.u32Saturation;

    pstHalCfg->u32Gain = pstMhalCfg->u32Gain;
    pstHalCfg->u32Sharpness = pstMhalCfg->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Csc:%s, Luma:%ld, Hue:%ld, Con:%ld, Sat%ld, Gain:%ld Sharp:%ld\n",
        __FUNCTION__, __LINE__,
        PARSING_HAL_CSC_MATRIX(pstHalCfg->stCsc.eCscMatrix),
        pstHalCfg->stCsc.u32Luma, pstHalCfg->stCsc.u32Hue, pstHalCfg->stCsc.u32Contrast,
        pstHalCfg->stCsc.u32Saturation, pstHalCfg->u32Gain, pstHalCfg->u32Sharpness);
}

void _DrvDispIfTransDeviceHdmiParamToHal(MHAL_DISP_HdmiParam_t *pstMhalCfg, HalDispHdmiParam_t *pstHalCfg)
{
    pstHalCfg->stCsc.eCscMatrix    = __DrvDispIfTransCscMatrixToHal(pstMhalCfg->stCsc.eCscMatrix);
    pstHalCfg->stCsc.u32Luma       = pstMhalCfg->stCsc.u32Luma;
    pstHalCfg->stCsc.u32Hue        = pstMhalCfg->stCsc.u32Hue;
    pstHalCfg->stCsc.u32Contrast   = pstMhalCfg->stCsc.u32Contrast;
    pstHalCfg->stCsc.u32Saturation = pstMhalCfg->stCsc.u32Saturation;
    pstHalCfg->u32Sharpness        = pstMhalCfg->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Csc:%s, Luma:%ld, Hue:%ld, Con:%ld, Sat:%ld, Sharp:%ld\n",
        __FUNCTION__, __LINE__,
        PARSING_HAL_CSC_MATRIX(pstHalCfg->stCsc.eCscMatrix),
        pstHalCfg->stCsc.u32Luma, pstHalCfg->stCsc.u32Hue, pstHalCfg->stCsc.u32Contrast,
        pstHalCfg->stCsc.u32Saturation,
        pstHalCfg->u32Sharpness);
}

void _DrvDispIfTransDeviceLcdParamToHal(MHAL_DISP_LcdParam_t *pstMhalCfg, HalDispLcdParam_t *pstHalCfg)
{
    pstHalCfg->stCsc.eCscMatrix    = __DrvDispIfTransCscMatrixToHal(pstMhalCfg->stCsc.eCscMatrix);
    pstHalCfg->stCsc.u32Luma       = pstMhalCfg->stCsc.u32Luma;
    pstHalCfg->stCsc.u32Hue        = pstMhalCfg->stCsc.u32Hue;
    pstHalCfg->stCsc.u32Contrast   = pstMhalCfg->stCsc.u32Contrast;
    pstHalCfg->stCsc.u32Saturation = pstMhalCfg->stCsc.u32Saturation;

    pstHalCfg->u32Sharpness = pstMhalCfg->u32Sharpness;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Csc:%s, Luma:%ld, Hue:%ld, Con:%ld, Sat%ld, Sharp:%ld\n",
        __FUNCTION__, __LINE__,
        PARSING_HAL_CSC_MATRIX(pstHalCfg->stCsc.eCscMatrix),
        pstHalCfg->stCsc.u32Luma, pstHalCfg->stCsc.u32Hue, pstHalCfg->stCsc.u32Contrast,
        pstHalCfg->stCsc.u32Saturation, pstHalCfg->u32Sharpness);
}

void _DrvDispIfTransdeviceGammaParamToHal(MHAL_DISP_GammaParam_t *pstMhalCfg, HalDispGammaParam_t *pstHalCfg)
{
    pstHalCfg->bEn = pstMhalCfg->bEn;
    pstHalCfg->u16EntryNum = pstMhalCfg->u16EntryNum;
    pstHalCfg->pu8ColorR = pstMhalCfg->pu8ColorR;
    pstHalCfg->pu8ColorG = pstMhalCfg->pu8ColorG;
    pstHalCfg->pu8ColorB = pstMhalCfg->pu8ColorB;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, En:%d, EntryNum:%d\n",
        __FUNCTION__, __LINE__,
        pstHalCfg->bEn, pstHalCfg->u16EntryNum);
}



void _DrvDispIfTransDeviceColorTempToHal(MHAL_DISP_ColorTempeture_t *pstMhalCfg, HalDispColorTemp_t *pstHalCfg)
{
    pstHalCfg->u16RedColor   = pstMhalCfg->u16RedColor;
    pstHalCfg->u16GreenColor = pstMhalCfg->u16GreenColor;
    pstHalCfg->u16BlueColor  = pstMhalCfg->u16BlueColor;
    pstHalCfg->u16RedOffset   = pstMhalCfg->u16RedOffset;
    pstHalCfg->u16GreenOffset = pstMhalCfg->u16GreenOffset;
    pstHalCfg->u16BlueOffset  = pstMhalCfg->u16BlueOffset;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Color(%d %d %d), Offset(%d %d %d)\n",
        __FUNCTION__, __LINE__,
        pstHalCfg->u16RedColor, pstHalCfg->u16GreenColor, pstHalCfg->u16BlueColor,
        pstHalCfg->u16RedOffset, pstHalCfg->u16GreenOffset, pstHalCfg->u16BlueOffset);
}

void _DrvDispIfTransVidLayerAttrToHal(MHAL_DISP_VideoLayerAttr_t *pstMhalCfg, HalDispVideoLayerAttr_t *pstHalCfg)
{
    pstHalCfg->stVidLayerDispWin.u16X      = pstMhalCfg->stVidLayerDispWin.u16X;
    pstHalCfg->stVidLayerDispWin.u16Y      = pstMhalCfg->stVidLayerDispWin.u16Y;
    pstHalCfg->stVidLayerDispWin.u16Width  = pstMhalCfg->stVidLayerDispWin.u16Width;
    pstHalCfg->stVidLayerDispWin.u16Height = pstMhalCfg->stVidLayerDispWin.u16Height;

    pstHalCfg->stVidLayerSize.u32Height = pstMhalCfg->stVidLayerSize.u32Height;
    pstHalCfg->stVidLayerSize.u32Width = pstMhalCfg->stVidLayerSize.u32Width;

    pstHalCfg->ePixFormat = __DrvDispIfTransPixelFormatToHal(pstMhalCfg->ePixFormat);

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Pixel:%s, Disp(%d %d %d %d) Size(%ld %ld)\n",
        __FUNCTION__, __LINE__,
        PARSING_HAL_PIXEL_FMT(pstHalCfg->ePixFormat),
        pstHalCfg->stVidLayerDispWin.u16X, pstHalCfg->stVidLayerDispWin.u16Y,
        pstHalCfg->stVidLayerDispWin.u16Width, pstHalCfg->stVidLayerDispWin.u16Height,
        pstHalCfg->stVidLayerSize.u32Width, pstHalCfg->stVidLayerSize.u32Height);
}

void _DrvDispIfTransDeviceTimeZoneToMhal(MHAL_DISP_TimeZone_t *pstMhalCfg, HalDispTimeZone_t *pstHalCfg)
{
    pstMhalCfg->enType = pstHalCfg->enType == E_HAL_DISP_TIMEZONE_SYNC       ? E_MHAL_DISP_TIMEZONE_SYNC :
                         pstHalCfg->enType == E_HAL_DISP_TIMEZONE_BACKPORCH  ? E_MHAL_DISP_TIMEZONE_BACKPORCH :
                         pstHalCfg->enType == E_HAL_DISP_TIMEZONE_ACTIVE     ? E_MHAL_DISP_TIMEZONE_ACTIVE :
                         pstHalCfg->enType == E_HAL_DISP_TIMEZONE_FRONTPORCH ? E_MHAL_DISP_TIMEZONE_FRONTPORCH :
                                                                               E_MHAL_DISP_TIMEZONE_UNKONWN;

    //DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, TimeZone=%s\n",
    //    __FUNCTION__, __LINE__,
    //    PARSING_HAL_TIMEZONE(pstHalCfg->enType));

}

void _DrvDispIfTransDeviceDisplayInfoToMhal(MHAL_DISP_DisplayInfo_t *pstMhalCfg, HalDispDisplayInfo_t *pstHalCfg)
{
    pstMhalCfg->u16Htotal = pstHalCfg->u16Htotal;
    pstMhalCfg->u16Vtotal = pstHalCfg->u16Vtotal;
    pstMhalCfg->u16HdeStart = pstHalCfg->u16HdeStart;
    pstMhalCfg->u16VdeStart = pstHalCfg->u16VdeStart;
    pstMhalCfg->u16Width = pstHalCfg->u16Width;
    pstMhalCfg->u16Height = pstHalCfg->u16Height;
    pstMhalCfg->bInterlaceMode = pstHalCfg->bInterlace;
    pstMhalCfg->bYuvOutput = pstHalCfg->bYuvOutput;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d: Htt:%d, Vtt:%d, Hst:%d Vst:%d, Interlace:%d, Yuv:%d\n",
        __FUNCTION__, __LINE__,
        pstMhalCfg->u16Htotal, pstMhalCfg->u16Vtotal,
        pstMhalCfg->u16HdeStart, pstMhalCfg->u16VdeStart,
        pstMhalCfg->bInterlaceMode, pstMhalCfg->bYuvOutput);
}

void _DrvDispIfTransVidLayerCompressToHal(MHAL_DISP_CompressAttr_t *pstMhalCfg, HalDispVideoLayerCompressAttr_t *pstHalCfg)
{

   pstHalCfg->bEnbale = pstMhalCfg->bEnbale;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, bEn:%d\n", __FUNCTION__, __LINE__, pstHalCfg->bEnbale);
}

void _DrvDispIfTransInputPortFrameDataToHal(MHAL_DISP_VideoFrameData_t *pstMhalcfg, HalDispVideoFrameData_t *pstHalCfg)
{
    pstHalCfg->ePixelFormat = __DrvDispIfTransPixelFormatToHal(pstMhalcfg->ePixelFormat);
    pstHalCfg->eCompressMode = __DrvDispIfTransCompressToHal(pstMhalcfg->eCompressMode);
    pstHalCfg->eTileMode = __DrvDispIfTransTileModeToHal(pstMhalcfg->eTileMode);
    pstHalCfg->au32Stride[0] = pstMhalcfg->au32Stride[0];
    pstHalCfg->au32Stride[1] = pstMhalcfg->au32Stride[1];
    pstHalCfg->au32Stride[2] = pstMhalcfg->au32Stride[2];

    pstHalCfg->au64PhyAddr[0] = pstMhalcfg->aPhyAddr[0];
    pstHalCfg->au64PhyAddr[1] = pstMhalcfg->aPhyAddr[1];
    pstHalCfg->au64PhyAddr[2] = pstMhalcfg->aPhyAddr[2];

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Pixel:%s, Compress:%s, Title:%s, (%llx, %llx, %llx), (%ld %ld %ld)\n",
        __FUNCTION__, __LINE__,
        PARSING_HAL_PIXEL_FMT(pstHalCfg->ePixelFormat ),
        PARSING_HAL_COMPRESS_MD(pstHalCfg->eCompressMode),
        PARSING_HAL_TILE_MD(pstHalCfg->eTileMode),
        pstHalCfg->au64PhyAddr[0], pstHalCfg->au64PhyAddr[1], pstHalCfg->au64PhyAddr[2],
        pstHalCfg->au32Stride[0], pstHalCfg->au32Stride[1], pstHalCfg->au32Stride[2]);
}

void _DrvDispIfTransInputPortRotateToHal(MHAL_DISP_RotateConfig_t *pstMhalCfg, HalDispInputPortRotate_t *pstHalCfg)
{
    pstHalCfg->enRotate = pstMhalCfg->enRotate == E_MHAL_DISP_ROTATE_NONE ? E_HAL_DISP_ROTATE_NONE :
                          pstMhalCfg->enRotate == E_MHAL_DISP_ROTATE_90   ? E_HAL_DISP_ROTATE_90 :
                          pstMhalCfg->enRotate == E_MHAL_DISP_ROTATE_180  ? E_HAL_DISP_ROTATE_180 :
                          pstMhalCfg->enRotate == E_MHAL_DISP_ROTATE_270  ? E_HAL_DISP_ROTATE_270 :
                                                                            E_HAL_DISP_ROTATE_NUM;


    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Rotate:%s\n",
        __FUNCTION__, __LINE__,
        PARSING_HAL_ROTATE(pstHalCfg->enRotate));
}

void _DrvDispIfTransInputPortAttrToHal(MHAL_DISP_InputPortAttr_t *pMhalCfg, HalDispInputPortAttr_t *pstHalCfg)
{
    pstHalCfg->stDispWin.u16X      = pMhalCfg->stDispWin.u16X;
    pstHalCfg->stDispWin.u16Y      = pMhalCfg->stDispWin.u16Y;
    pstHalCfg->stDispWin.u16Width  = pMhalCfg->stDispWin.u16Width;
    pstHalCfg->stDispWin.u16Height = pMhalCfg->stDispWin.u16Height;
    pstHalCfg->u16SrcHeight        = pMhalCfg->u16SrcHeight;
    pstHalCfg->u16SrcWidth         = pMhalCfg->u16SrcWidth;

    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, Rect(%d %d %d %d) Src(%d %d)\n",
        __FUNCTION__, __LINE__,
        pstHalCfg->stDispWin.u16X, pstHalCfg->stDispWin.u16Y,
        pstHalCfg->stDispWin.u16Width, pstHalCfg->stDispWin.u16Height,
        pstHalCfg->u16SrcWidth, pstHalCfg->u16SrcHeight);
}
void _DrvDispIfTransInputPortCropAttrToHal(MHAL_DISP_VidWinRect_t *pMhalCfg, HalDispVidWinRect_t *pstHalCfg)
{
    pstHalCfg->u16X      = pMhalCfg->u16X;
    pstHalCfg->u16Y      = pMhalCfg->u16Y;
    pstHalCfg->u16Width  = pMhalCfg->u16Width;
    pstHalCfg->u16Height = pMhalCfg->u16Height;


    DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, crop Rect(%d %d %d %d)\n",
        __FUNCTION__, __LINE__,
        pstHalCfg->u16X, pstHalCfg->u16Y,
        pstHalCfg->u16Width, pstHalCfg->u16Height);
}


bool _DrvDispIfExecuteQuery(void *pCtx, HalDispQueryConfig_t *pstQueryCfg)
{
    bool bRet = 1;

    if(HalDispIfQuery(pCtx, pstQueryCfg))
    {
        if(pstQueryCfg->stOutCfg.enQueryRet == E_HAL_DISP_QUERY_RET_OK ||
           pstQueryCfg->stOutCfg.enQueryRet == E_HAL_DISP_QUERY_RET_NONEED)
        {
            if(pstQueryCfg->stOutCfg.pSetFunc)
            {
                pstQueryCfg->stOutCfg.pSetFunc(pCtx, pstQueryCfg->stInCfg.pInCfg);
            }
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, Query:%s, Ret:%s\n",
                __FUNCTION__,__LINE__,
                PARSING_HAL_QUERY_TYPE(pstQueryCfg->stInCfg.enQueryType),
                PARSING_HAL_QUERY_RET(pstQueryCfg->stOutCfg.enQueryRet));
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Query Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool DrvDispIfGetClk(void *pDevCtx, bool *pbEn, u32 *pu32ClkRate, u32 u32ClkNum)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispClkConfig_t stHalClkCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_CLK_GET;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(stHalClkCfg);
    stQueryCfg.stInCfg.pInCfg = &stHalClkCfg;

    if(_DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg))
    {
        if(stHalClkCfg.u32Num == u32ClkNum)
        {
            memcpy(pu32ClkRate, stHalClkCfg.u32Rate, sizeof(stHalClkCfg.u32Rate));
            memcpy(pbEn, stHalClkCfg.bEn, sizeof(stHalClkCfg.bEn));
            bRet = 1;
        }
    }
    else
    {
        bRet = FALSE;
        DISP_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool DrvDispIfSetClk(void *pDevCtx, bool *pbEn, u32 *pu32ClkRate, u32 u32ClkNum)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispClkConfig_t stHalClkCfg;

    if( sizeof(stHalClkCfg.u32Rate) != sizeof(u32)*u32ClkNum ||
        sizeof(stHalClkCfg.bEn) != sizeof(bool)*u32ClkNum)
    {
        bRet = 0;
        DISP_ERR("%s %d, Clk Num is not correct: Rate:%d != %ld, En:%d != %ld",
            __FUNCTION__, __LINE__,
            sizeof(stHalClkCfg.u32Rate), sizeof(u32)*u32ClkNum,
            sizeof(stHalClkCfg.bEn), sizeof(bool)*u32ClkNum);
    }
    else
    {

        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_CLK_SET;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispClkConfig_t);
        stQueryCfg.stInCfg.pInCfg = &stHalClkCfg;


        stHalClkCfg.u32Num = u32ClkNum;
        memcpy(stHalClkCfg.u32Rate, pu32ClkRate, sizeof(u32)*u32ClkNum);
        memcpy(stHalClkCfg.bEn, pbEn, sizeof(bool)*u32ClkNum);

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}


bool DrvDispIfInitPanelConfig(MHAL_DISP_PanelConfig_t* pstPanelConfig, u8 u8Size)
{
    u8 i;
    bool bRet = TRUE;
    u32 u32Mod, u32HttVtt,u32DClkhz;

    if(u8Size > E_HAL_DISP_OUTPUT_MAX)
    {
        bRet = FALSE;
        DISP_ERR("%s %d, The size (%d) is bigger than %d\n", __FUNCTION__, __LINE__, u8Size, E_HAL_DISP_OUTPUT_MAX);
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_DRV, "%s %d, u8Size:%d\n", __FUNCTION__, __LINE__, u8Size);
        for(i=0; i<u8Size; i++)
        {
            gstDevTimingTbl[i].bValid = pstPanelConfig[i].bValid;
            gstDevTimingTbl[i].enDevTimingType = __DrvDispIfTransDevTimingTypeToHal(pstPanelConfig[i].eTiming);

            gstDevTimingTbl[i].stDevTiming.u16HsyncWidth     = pstPanelConfig[i].stPanelAttr.m_ucPanelHSyncWidth;
            gstDevTimingTbl[i].stDevTiming.u16HsyncBackPorch = pstPanelConfig[i].stPanelAttr.m_ucPanelHSyncBackPorch;
            gstDevTimingTbl[i].stDevTiming.u16Hstart         = pstPanelConfig[i].stPanelAttr.m_wPanelHStart;
            gstDevTimingTbl[i].stDevTiming.u16Hactive        = pstPanelConfig[i].stPanelAttr.m_wPanelWidth;
            gstDevTimingTbl[i].stDevTiming.u16Htotal         = pstPanelConfig[i].stPanelAttr.m_wPanelHTotal;

            gstDevTimingTbl[i].stDevTiming.u16VsyncWidth     = pstPanelConfig[i].stPanelAttr.m_ucPanelVSyncWidth;
            gstDevTimingTbl[i].stDevTiming.u16VsyncBackPorch = pstPanelConfig[i].stPanelAttr.m_ucPanelVBackPorch;
            gstDevTimingTbl[i].stDevTiming.u16Vstart         = pstPanelConfig[i].stPanelAttr.m_wPanelVStart;
            gstDevTimingTbl[i].stDevTiming.u16Vactive        = pstPanelConfig[i].stPanelAttr.m_wPanelHeight;
            gstDevTimingTbl[i].stDevTiming.u16Vtotal         = pstPanelConfig[i].stPanelAttr.m_wPanelVTotal;


            u32DClkhz = ((u32)pstPanelConfig[i].stPanelAttr.m_dwPanelDCLK * 1000000);
            u32HttVtt = ((u32)pstPanelConfig[i].stPanelAttr.m_wPanelHTotal * (u32)pstPanelConfig[i].stPanelAttr.m_wPanelVTotal);
            u32Mod = u32DClkhz % u32HttVtt;

            if(u32Mod > (u32HttVtt/2))
            {
                gstDevTimingTbl[i].stDevTiming.u16Fps = (u32DClkhz + u32HttVtt -1) / u32HttVtt;
            }
            else
            {
                gstDevTimingTbl[i].stDevTiming.u16Fps = u32DClkhz / u32HttVtt;
            }

            gstDevTimingTbl[i].stDevTiming.u16SscSpan = pstPanelConfig[i].stPanelAttr.m_wSpreadSpectrumSpan;
            gstDevTimingTbl[i].stDevTiming.u16SscStep = pstPanelConfig[i].stPanelAttr.m_wSpreadSpectrumStep;
        }
    }
    return bRet;
}


bool DrvDispIfDeviceCreateInstance(MHAL_DISP_AllocPhyMem_t *pstAlloc, u32 u32DeviceId, void **pDevCtx)
{
    bool bRet = TRUE;
    DrvDispCtxAllocConfig_t stCtxAllocCfg;
    HalDispQueryConfig_t stQueryCfg;
    DrvDispCtxConfig_t *pstDispCtx = NULL;

    DrvDispCtxInit();

    stCtxAllocCfg.enType = E_DISP_CTX_TYPE_DEVICE;
    stCtxAllocCfg.u32Id = u32DeviceId;
    stCtxAllocCfg.pstBindCtx = NULL;
    stCtxAllocCfg.stMemAllcCfg.alloc = (pDispCtxMemAlloc)pstAlloc->alloc;
    stCtxAllocCfg.stMemAllcCfg.free =  (pDispCtxMemFree)pstAlloc->free;

    if(DrvDispCtxAllocate(&stCtxAllocCfg, &pstDispCtx) == FALSE)
    {
        bRet = 0;
        *pDevCtx = NULL;
        DISP_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
    }
    else
    {
        u32 au32ClkRate[HAL_DISP_CLK_NUM] = HAL_DISP_CLK_RATE_SETTING;
        bool abClkEn[HAL_DISP_CLK_NUM] = HAL_DISP_CLK_ON_SETTING;

        *pDevCtx = (void *)pstDispCtx;

        if(DrvDispOsSetClkOn(au32ClkRate, HAL_DISP_CLK_NUM) == 0)
        {
            if(DrvDispIfSetClk((void *)pstDispCtx, abClkEn, au32ClkRate, HAL_DISP_CLK_NUM) == 0)
            {
                DISP_ERR("%s %d:: SetClk Fail\n", __FUNCTION__, __LINE__);
                bRet = 0;
            }
        }

        if(bRet == 1)
        {
            memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));
            stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_INIT;
            stQueryCfg.stInCfg.pInCfg = NULL;
            stQueryCfg.stInCfg.u32CfgSize = 0;

            if(_DrvDispIfExecuteQuery(pstDispCtx, &stQueryCfg))
            {
                DrvDispCtxSetCurCtx(pstDispCtx, u32DeviceId);
                DrvDispIrqCreateInternalIsr();
            }
            else
            {
                DISP_ERR("%s %d:: Device Init Fail\n", __FUNCTION__, __LINE__);
                bRet = 0;
            }
        }
    }

    return bRet;
}

bool DrvDispIfDeviceDestroyInstance(void *pDevCtx)
{
    bool bRet = TRUE;
    DrvDispCtxConfig_t *pstDispCtx = (DrvDispCtxConfig_t *) pDevCtx;

    if(DrvDispCtxSetCurCtx(pstDispCtx, pstDispCtx->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        if(DrvDispOsSetClkOff() == 0)
        {
            u32 au32ClkRate[HAL_DISP_CLK_NUM] = HAL_DISP_CLK_RATE_SETTING;
            bool abClkEn[HAL_DISP_CLK_NUM] = HAL_DISP_CLK_OFF_SETTING;

            if(DrvDispIfSetClk(pDevCtx, abClkEn, au32ClkRate, HAL_DISP_CLK_NUM) ==0)
            {
                bRet = FALSE;
                DISP_ERR("%s %d, Set Clk Off Fail\n", __FUNCTION__, __LINE__);
            }
        }

        if(bRet && DrvDispCtxFree(pstDispCtx))
        {
            if(DrvDispCtxIsAllFree())
            {
                DrvDispCtxDeInit();
            }
            DrvDispIrqDestroyInternalIsr();
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, DestroyInstance Fail\n", __FUNCTION__, __LINE__);
        }
    }

    return bRet;
}

bool DrvDispIfDeviceEnable(void *pDevCtx, bool bEnable)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_ENABLE;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(bool);
        stQueryCfg.stInCfg.pInCfg = (void *)&bEnable;

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

bool DrvDispIfDeviceSetBackGroundColor(void *pDevCtx, u32 u32BgColor)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_BACKGROUND_COLOR;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(u32);
        stQueryCfg.stInCfg.pInCfg = (void *)&u32BgColor;

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}


bool DrvDispIfDeviceAddOutInterface(void *pDevCtx, u32 u32Interface)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_INTERFACE;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(u32);
        stQueryCfg.stInCfg.pInCfg = (void *)&u32Interface;

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

bool DrvDispIfDeviceSetOutputTiming(void *pDevCtx, u32 u32Interface, MHAL_DISP_DeviceTimingInfo_t *pstTimingInfo)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispDeviceTimingInfo_t stHalTimingIfo;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_OUTPUTTIMING;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispDeviceTimingInfo_t);
        stQueryCfg.stInCfg.pInCfg = (void *)&stHalTimingIfo;

        if(_DrvDispIfTransDeviceOutpuTimingInfoToHal(pstTimingInfo, &stHalTimingIfo))
        {
            bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
        }
        else
        {
            bRet = 0;
        }
    }

    return bRet;
}

bool DrvDispIfDeviceSetColortemp(void *pDevCtx, MHAL_DISP_ColorTempeture_t *pstcolorTemp)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispColorTemp_t stHalColorTemp;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_COLORTEMP;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispColorTemp_t);
        stQueryCfg.stInCfg.pInCfg = (void *)&stHalColorTemp;

        _DrvDispIfTransDeviceColorTempToHal(pstcolorTemp, &stHalColorTemp);

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

bool DrvDispIfDeviceSetCvbsParam(void *pDevCtx, MHAL_DISP_CvbsParam_t *pstCvbsInfo)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispCvbsParam_t stHalCvbsParam;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_CVBS_PARAM;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispCvbsParam_t);
        stQueryCfg.stInCfg.pInCfg = (void *)&stHalCvbsParam;

        _DrvDispIfTransDeviceCvbsParamToHal(pstCvbsInfo, &stHalCvbsParam);

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

bool DrvDispIfDeviceSetHdmiParam(void *pDevCtx, MHAL_DISP_HdmiParam_t *pstHdmiInfo)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispHdmiParam_t stHalHdmiParam;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_HDMI_PARAM;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispHdmiParam_t);
        stQueryCfg.stInCfg.pInCfg = (void *)&stHalHdmiParam;

        _DrvDispIfTransDeviceHdmiParamToHal(pstHdmiInfo, &stHalHdmiParam);

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

bool DrvDispIfDeviceSetLcdParam(void *pDevCtx, MHAL_DISP_LcdParam_t *pstLcdInfo)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispLcdParam_t stHalLcdParam;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_LCD_PARAM;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispLcdParam_t);
        stQueryCfg.stInCfg.pInCfg = (void *)&stHalLcdParam;

        _DrvDispIfTransDeviceLcdParamToHal(pstLcdInfo, &stHalLcdParam);

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

bool DrvDispIfDeviceSetGammaParam(void *pDevCtx, MHAL_DISP_GammaParam_t *pstGammaInfo)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispGammaParam_t stHalGammaaram;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_GAMMA_PARAM;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispGammaParam_t);
        stQueryCfg.stInCfg.pInCfg = (void *)&stHalGammaaram;

        _DrvDispIfTransdeviceGammaParamToHal(pstGammaInfo, &stHalGammaaram);

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}

bool DrvDispIfDeviceSetVgaParam(void *pDevCtx, MHAL_DISP_VgaParam_t *pstVgaInfo)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispVgaParam_t stHalVgaParam;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_VGA_PARAM;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispVgaParam_t);
        stQueryCfg.stInCfg.pInCfg = (void *)&stHalVgaParam;

        _DrvDispIfTransDeviceVgaParamToHal(pstVgaInfo, &stHalVgaParam);

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);
    }
    return bRet;
}


bool DrvDispIfDeviceAttach(void *pSrcDevCtx, void *pDstDevCtx)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;


    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pSrcDevCtx, ((DrvDispCtxConfig_t *)pSrcDevCtx)->u32Idx) == FALSE ||
       DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDstDevCtx, ((DrvDispCtxConfig_t *)pDstDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_ATTACH;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(pDstDevCtx);
        stQueryCfg.stInCfg.pInCfg = pDstDevCtx;

        bRet = _DrvDispIfExecuteQuery(pSrcDevCtx, &stQueryCfg);
    }
    return bRet;
}

bool DrvDispIfDeviceDetach(void *pSrcDevCtx, void *pDstDevCtx)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pSrcDevCtx, ((DrvDispCtxConfig_t *)pSrcDevCtx)->u32Idx) == FALSE ||
       DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDstDevCtx, ((DrvDispCtxConfig_t *)pDstDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_DETACH;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(pDstDevCtx);
        stQueryCfg.stInCfg.pInCfg = pDstDevCtx;

        bRet = _DrvDispIfExecuteQuery(pSrcDevCtx, &stQueryCfg);
    }
    return bRet;
}

bool DrvDispIfDeviceGetTimeZone(void *pDevCtx, MHAL_DISP_TimeZone_t *pstTimeZone)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispTimeZone_t stHalTimeZone;

    if(DrvDispCtxSetCurCtx((DrvDispCtxConfig_t *)pDevCtx, ((DrvDispCtxConfig_t *)pDevCtx)->u32Idx) == FALSE)
    {
        bRet = FALSE;
    }
    else
    {
        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_TIME_ZONE;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispTimeZone_t);
        stQueryCfg.stInCfg.pInCfg = &stHalTimeZone;

        bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);

        if(bRet)
        {
            _DrvDispIfTransDeviceTimeZoneToMhal(pstTimeZone, &stHalTimeZone);
        }
        else
        {
            pstTimeZone->enType = E_MHAL_DISP_TIMEZONE_UNKONWN;
        }
    }

    return bRet;
}

bool DrvDispIfDeviceGetDisplayInfo(void *pDevCtx, MHAL_DISP_DisplayInfo_t *pstDisplayInfo)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispDisplayInfo_t stHalDisplayInfo;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_DEVICE_DISPLAY_INFO;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispDisplayInfo_t);
    stQueryCfg.stInCfg.pInCfg = &stHalDisplayInfo;

    bRet = _DrvDispIfExecuteQuery(pDevCtx, &stQueryCfg);

    if(bRet)
    {
        _DrvDispIfTransDeviceDisplayInfoToMhal(pstDisplayInfo, &stHalDisplayInfo);
    }
    else
    {
        memset(pstDisplayInfo, 0, sizeof(MHAL_DISP_DisplayInfo_t));
    }

    return bRet;
}

bool DrvDispIfDeviceGetInstance(u32 u32DeviceId, void **pDevCtx)
{
    bool bRet = TRUE;
    DrvDispCtxConfig_t *pstDispCtx = NULL;

    if(DrvDispCtxGetCurCtx(E_DISP_CTX_TYPE_DEVICE, u32DeviceId, &pstDispCtx))
    {
        *pDevCtx = (void *)pstDispCtx;
    }
    else
    {
        bRet = FALSE;
        *pDevCtx = NULL;
    }
    return bRet;
}

// VideoLayer
bool DrvDispIfVideoLayerCreateInstance(MHAL_DISP_AllocPhyMem_t *pstAlloc, u32 u32LayerId, void **pVidLayerCtx)
{
    bool bRet = TRUE;
    DrvDispCtxAllocConfig_t stCtxAllocCfg;
    HalDispQueryConfig_t stQueryCfg;
    DrvDispCtxConfig_t *pstDispCtx = NULL;

    DrvDispCtxInit();

    stCtxAllocCfg.enType = E_DISP_CTX_TYPE_VIDLAYER;
    stCtxAllocCfg.u32Id = u32LayerId;
    stCtxAllocCfg.pstBindCtx = NULL;
    stCtxAllocCfg.stMemAllcCfg.alloc = NULL;
    stCtxAllocCfg.stMemAllcCfg.free = NULL;

    if(DrvDispCtxAllocate(&stCtxAllocCfg, &pstDispCtx) == FALSE)
    {
        bRet = 0;
        *pVidLayerCtx = NULL;
        DISP_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
    }
    else
    {
        *pVidLayerCtx = (void *)pstDispCtx;

        memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_VIDEOLAYER_INIT;
        stQueryCfg.stInCfg.pInCfg = NULL;
        stQueryCfg.stInCfg.u32CfgSize = 0;
        bRet = _DrvDispIfExecuteQuery(pstDispCtx, &stQueryCfg);
    }

    return bRet;
}

bool DrvDispIfVideoLayerDestoryInstance(void *pVidLayerCtx)
{
    bool bRet = TRUE;
    DrvDispCtxConfig_t *pstDispCtx = (DrvDispCtxConfig_t *) pVidLayerCtx;

    if(DrvDispCtxFree(pstDispCtx))
    {
        if(DrvDispCtxIsAllFree())
        {
            DrvDispCtxDeInit();
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, DestroyInstance Fail\n", __FUNCTION__, __LINE__);
    }

    return bRet;
}


bool DrvDispIfVideoLayerEnable(void *pVidLayerCtx,  bool bEnable)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_VIDEOLAYER_ENABLE;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(bool);
    stQueryCfg.stInCfg.pInCfg = &bEnable;

    bRet = _DrvDispIfExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfVideoLayerBind(void *pVidLayerCtx, void *pDevCtx)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_VIDEOLAYER_BIND;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(pDevCtx);
    stQueryCfg.stInCfg.pInCfg = pDevCtx;

    bRet = _DrvDispIfExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfVideoLayerUnBind(void *pVidLayerCtx, void *pDevCtx)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_VIDEOLAYER_UNBIND;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(pDevCtx);
    stQueryCfg.stInCfg.pInCfg = pDevCtx;

    bRet = _DrvDispIfExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfVideoLayerSetAttr(void *pVidLayerCtx,  MHAL_DISP_VideoLayerAttr_t *pstAttr)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispVideoLayerAttr_t stHalAttrCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_VIDEOLAYER_ATTR;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispVideoLayerAttr_t);
    stQueryCfg.stInCfg.pInCfg = &stHalAttrCfg;

    _DrvDispIfTransVidLayerAttrToHal(pstAttr, &stHalAttrCfg);

    bRet = _DrvDispIfExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfVideoLayerBufferFire(void *pVidLayerCtx)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_VIDEOLAYER_BUFFER_FIRE;
    stQueryCfg.stInCfg.u32CfgSize = 0;
    stQueryCfg.stInCfg.pInCfg = NULL;

    bRet = _DrvDispIfExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfVideoLayerCheckBufferFired(void *pVidLayerCtx)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_VIDEOLAYER_CHECK_FIRE;
    stQueryCfg.stInCfg.u32CfgSize = 0;
    stQueryCfg.stInCfg.pInCfg = NULL;

    bRet = _DrvDispIfExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfVideoLayerSetCompress(void *pVidLayerCtx, MHAL_DISP_CompressAttr_t* pstCompressAttr)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispVideoLayerCompressAttr_t stHalCompressCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_VIDEOLAYER_COMPRESS;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispVideoLayerCompressAttr_t);
    stQueryCfg.stInCfg.pInCfg = &stHalCompressCfg;

    _DrvDispIfTransVidLayerCompressToHal(pstCompressAttr, &stHalCompressCfg);
    bRet = _DrvDispIfExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfVideoLayerSetPriority(void *pVidLayerCtx, u32 u32Priority)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_VIDEOLAYER_PRIORITY;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(u32);
    stQueryCfg.stInCfg.pInCfg = &u32Priority;

    bRet = _DrvDispIfExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

// InputPort
bool DrvDispIfInputPortCreateInstance(MHAL_DISP_AllocPhyMem_t *pstAlloc, void *pVidLayerCtx, u32 u32PortId, void **pCtx)
{
    bool bRet = TRUE;
    DrvDispCtxAllocConfig_t stCtxAllocCfg;
    HalDispQueryConfig_t stQueryCfg;
    DrvDispCtxConfig_t *pstDispCtx = NULL;

    DrvDispCtxInit();

    stCtxAllocCfg.enType = E_DISP_CTX_TYPE_INPUTPORT;
    stCtxAllocCfg.u32Id = u32PortId;
    stCtxAllocCfg.pstBindCtx = (DrvDispCtxConfig_t *)pVidLayerCtx;
    stCtxAllocCfg.stMemAllcCfg.alloc = NULL;
    stCtxAllocCfg.stMemAllcCfg.free = NULL;

    if(DrvDispCtxAllocate(&stCtxAllocCfg, &pstDispCtx) == FALSE)
    {
        bRet = 0;
        *pCtx = NULL;
        DISP_ERR("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
    }
    else
    {
        *pCtx = (void *)pstDispCtx;

         memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));
         stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_INPUTPORT_INIT;
         stQueryCfg.stInCfg.pInCfg = NULL;
         stQueryCfg.stInCfg.u32CfgSize = 0;
         bRet = _DrvDispIfExecuteQuery(pstDispCtx, &stQueryCfg);
    }

    return bRet;
}


bool DrvDispIfInputPortDestroyInstance(void *pInputPortCtx)
{
    bool bRet = TRUE;
    DrvDispCtxConfig_t *pstDispCtx = (DrvDispCtxConfig_t *) pInputPortCtx;

    if(DrvDispCtxFree(pstDispCtx))
    {
        if(DrvDispCtxIsAllFree())
        {
            DrvDispCtxDeInit();
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, DestroyInstance Fail\n", __FUNCTION__, __LINE__);
    }

    return bRet;
}

bool DrvDispIfInputPortRotate(void *pInputPortCtx, MHAL_DISP_RotateConfig_t *pstRotateCfg)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispInputPortRotate_t stRotate;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_INPUTPORT_ROTATE;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispInputPortRotate_t);
    stQueryCfg.stInCfg.pInCfg = &stRotate;

    _DrvDispIfTransInputPortRotateToHal(pstRotateCfg, &stRotate);
    bRet = _DrvDispIfExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfInputPortSetCropAttr(void *pInputPortCtx, MHAL_DISP_VidWinRect_t *pstCropAttr)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispVidWinRect_t stHalCropAttr;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_INPUTPORT_CROP;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispVidWinRect_t);
    stQueryCfg.stInCfg.pInCfg = &stHalCropAttr;

    _DrvDispIfTransInputPortCropAttrToHal(pstCropAttr, &stHalCropAttr);
    bRet = _DrvDispIfExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}
bool DrvDispIfInputPortFlip(void *pInputPortCtx, MHAL_DISP_VideoFrameData_t *pstVideoFrameData)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispVideoFrameData_t stHalFrameData;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_INPUTPORT_FLIP;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispVideoFrameData_t);
    stQueryCfg.stInCfg.pInCfg = &stHalFrameData;

    _DrvDispIfTransInputPortFrameDataToHal(pstVideoFrameData, &stHalFrameData);
    bRet = _DrvDispIfExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}


bool DrvDispIfInputPortEnable(void *pInputPortCtx, bool bEnable)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_INPUTPORT_ENABLE;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(bool);
    stQueryCfg.stInCfg.pInCfg = &bEnable;

    bRet = _DrvDispIfExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}


bool DrvDispIfInputPortSetAttr(void *pInputPortCtx, MHAL_DISP_InputPortAttr_t *pstAttr)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;
    HalDispInputPortAttr_t stHalAttr;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_INPUTPORT_ATTR;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalDispInputPortAttr_t);
    stQueryCfg.stInCfg.pInCfg = &stHalAttr;

    _DrvDispIfTransInputPortAttrToHal(pstAttr, &stHalAttr);
    bRet = _DrvDispIfExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfInputPortShow(void *pInputPortCtx)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_INPUTPORT_SHOW;
    stQueryCfg.stInCfg.u32CfgSize = 0;
    stQueryCfg.stInCfg.pInCfg = NULL;

    bRet = _DrvDispIfExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfInputPortHide(void *pInputPortCtx)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_INPUTPORT_HIDE;
    stQueryCfg.stInCfg.u32CfgSize = 0;
    stQueryCfg.stInCfg.pInCfg = NULL;

    bRet = _DrvDispIfExecuteQuery(pInputPortCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfInputPortAttrBegin(void *pVidLayerCtx)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_INPUTPORT_BEGIN;
    stQueryCfg.stInCfg.u32CfgSize = 0;
    stQueryCfg.stInCfg.pInCfg = NULL;

    bRet = _DrvDispIfExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfInputPortAttrEnd(void *pVidLayerCtx)
{
    bool bRet = 1;
    HalDispQueryConfig_t stQueryCfg;

    memset(&stQueryCfg, 0, sizeof(HalDispQueryConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_DISP_QUERY_INPUTPORT_END;
    stQueryCfg.stInCfg.u32CfgSize = 0;
    stQueryCfg.stInCfg.pInCfg = NULL;

    bRet = _DrvDispIfExecuteQuery(pVidLayerCtx, &stQueryCfg);

    return bRet;
}

bool DrvDispIfSetDbgLevel(void *p)
{
    _gu32DispDbgLevel = *((u32 *)p);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
