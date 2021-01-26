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

#define _HAL_DISP_IF_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_disp_os.h"
#include "hal_disp_common.h"
#include "disp_debug.h"
#include "hal_disp_chip.h"
#include "hal_disp_st.h"
#include "hal_disp_scaler_e.h"
#include "hal_disp_color.h"
#include "hal_disp_mop.h"
#include "drv_disp_ctx.h"
#include "drv_disp_os.h"
#include "hal_disp_vga_timing_tbl.h"
#include "hal_disp_pq.h"
#include "hal_disp_irq.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define PARSING_MOP_ROT_ANGLE(x) (\
                            x == E_HAL_DISP_ROTATE_NONE    ? "Rotate 0"   :   \
                            x == E_HAL_DISP_ROTATE_90      ? "Rotate 90"  :   \
                            x == E_HAL_DISP_ROTATE_180     ? "Rotate 180" :   \
                            x == E_HAL_DISP_ROTATE_270     ? "Rotate 270" :   \
                            "UNKNOWN")

#define HAL_DISP_COLOR_CSC_ID   1
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    HalDispQueryRet_e (*pGetInfoFunc) (void *, void *);
    void (*pSetFunc) (void * , void *);
    u16  u16CfgSize;
}HalDispQueryCallBackFunc_t;

typedef struct
{
    HalDispMopgGwinId_e eMopgId;
    u16 u16GwinHst;
    u8 u8RealOrder;
    bool bIsAttred;
}HalDispMopgLineBufOrder_t;

//mopg support only one window do rotate at the same time, so record it
typedef struct
{
    bool bRotEn;
    HalDispMopgGwinId_e eMopgId;
    HalDispRotateMode_e stRotAng;
}HalDispMopgRotateRecord_t;


//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
bool gbDispHwIfInit = 0;
HalDispQueryCallBackFunc_t gpDispCbTbl[E_HAL_DISP_QUERY_MAX];
u8 gbMopgInitNum = 0;//for InputPort
HalDispMopgLineBufOrder_t gpMopgLbOdTbl[E_HAL_DISP_MOPG_ID_NUM];//for InputPort counting mopg line buffer start
//rotate record for MOPG
HalDispMopgRotateRecord_t _gstMopgRotRec = {0,E_HAL_DISP_MOPG_ID_NUM,E_HAL_DISP_ROTATE_NONE};
//rotate use imi address
void* u32MopRotImiTotalAddr;
u32 u32Rot0ImiYaddr;
u32 u32Rot0ImiCaddr;
u32 u32Rot1ImiYaddr;
u32 u32Rot1ImiCaddr;
bool bImiInit=0;
u16 gu16DacTrim[3] = {0, 0, 0};

s16 tVideoColorCorrectionMatrix[]=
{
   0x0425,-0x0021,-0x0003,-0x0025, 0x0466,-0x0040,-0x0038, 0x0014,
   0x0423,-0x034B, 0x0196,-0x068B, 0x03C9,-0x0439, 0x0032,-0x0004,
  -0x07EE, 0x04E7, 0x07CB,-0x04C3, 0x0404, 0x023B,-0x023E, 0x01D5,
  -0x0831, 0x0100,-0x0102, 0x0101,-0x0101, 0x0000, 0x0000, 0x0000,
};

HalDispPictureNonLinearCurveType_t stDacTrimNonLinear =
{
    0x00, 0x20, 0x40, 0x60, 0x7F,
};


HalDispPictureNonLinearCurveConfig_t stPictureNonLinearSetting =
{
    { // Hdmi
        {0,  80, 128, 160, 200},  // Contrast
        {0,  64, 128, 192, 255},  // Brightness
        {0,  96, 128, 160, 192},  // Color
        {0,  8,   24, 44,  63},   // Sharpness
        {30, 40, 50,  60,  70},   // Tint
        {0,  0,  0,   0,   0},    // Gain
    },


    { // Vga
        {0,  80,  128, 160, 200},  // Contrast
        {0,  64,  128, 192, 255},  // Brightness
        {0,  96,  128, 160, 192},  // Color
        {0,  8,   24,  44,  63},   // Sharpness
        {30, 40,  50,  60,  70},   // Tint
        {0,  32,  64,  96, 127},   //GAIN
    },

    { // Lcd
        {0,  80,  128, 160, 200},  // Contrast
        {0,  64,  128, 192, 255},  // Brightness
        {0,  96,  128, 160, 192},  // Color
        {0,  8,   24,  44,  63},   // Sharpness
        {30, 40,  50,  60,  70},   // Tint
        {20, 30,  40,  60, 100},   //GAIN
    },
};


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
HalDispPictureNonLinearCurveType_t * __HalDispIfSetNonLinearCurve(u32 u32Interface, u16 u16OSD50)
{
    HalDispPictureNonLinearCurveType_t *pNonLinearCruve = NULL;

    if((u32Interface & HAL_DISP_INTF_VGA) == HAL_DISP_INTF_VGA)
    {
        pNonLinearCruve = &stDacTrimNonLinear;

        pNonLinearCruve->u16OSD_25 = (u16OSD50 > 0x20) ? u16OSD50 - 0x20 : 0;
        pNonLinearCruve->u16OSD_50 = u16OSD50;
        pNonLinearCruve->u16OSD_75 = (u16OSD50 + 0x20) > 0x7F ? 0x7F : u16OSD50 + 0x20;
    }
    else
    {
        pNonLinearCruve = NULL;
    }
    return pNonLinearCruve;
}

HalDispPictureNonLinearCurveType_t * __HalDispIfGetNonLinearCurve(u32 u32Interface, HalDispPictureType_e enPictureType)
{
    HalDispPictureNonLinearCurveType_t *pNonLinearCruve = NULL;

    if( (u32Interface & HAL_DISP_INTF_HDMI) == HAL_DISP_INTF_HDMI)
    {
        pNonLinearCruve = &stPictureNonLinearSetting.stHdmiPictureCurve[enPictureType];
    }
    else if( (u32Interface & HAL_DISP_INTF_VGA) == HAL_DISP_INTF_VGA)
    {
        pNonLinearCruve = &stPictureNonLinearSetting.stVgaPictureCurve[enPictureType];
    }
    else if( (u32Interface & HAL_DISP_INTF_LCD) == HAL_DISP_INTF_LCD)
    {
        pNonLinearCruve = &stPictureNonLinearSetting.stLcdPictureCurve[enPictureType];
    }
    else
    {
        pNonLinearCruve = NULL;
    }
    return pNonLinearCruve;
}

u16 __HalDispIfCalculateNonLinear(HalDispPictureNonLinearCurveType_t *pNonLinearCurve, u8 u8AdjustValue)
{
    u16 u16Value,u16Y0,u16Y1,u16X0,u16X1,u16Intercept;
    u16 u16DistanceOfY, u16DistanceOfX;

    if(pNonLinearCurve == NULL)
    {
        return 0;
    }

    if (u8AdjustValue < 25)
    {
          u16Y0 = pNonLinearCurve->u16OSD_0;
          u16Y1 = pNonLinearCurve->u16OSD_25;
          u16X0 = 0;
          u16X1 = 25;
    }
    else if (u8AdjustValue < 50)
    {
          u16Y0 = pNonLinearCurve->u16OSD_25;
          u16Y1 = pNonLinearCurve->u16OSD_50;
          u16X0 = 25;
          u16X1 = 50;
    }
    else if (u8AdjustValue < 75)
    {
          u16Y0 = pNonLinearCurve->u16OSD_50;
          u16Y1 = pNonLinearCurve->u16OSD_75;
          u16X0 = 50;
          u16X1 = 75;
    }
    else
    {
          u16Y0 = pNonLinearCurve->u16OSD_75;
          u16Y1 = pNonLinearCurve->u16OSD_100;
          u16X0 = 75;
          u16X1 = 100;
    }

    if (u16Y1 > u16Y0)
    {
        u16DistanceOfY = u16Y1 - u16Y0;
        u16DistanceOfX = u16X1 - u16X0;
        u16Intercept  = u16Y0;
        u8AdjustValue  = u8AdjustValue - u16X0;
    }
    else
    {
        u16DistanceOfY = u16Y0 - u16Y1;
        u16DistanceOfX = u16X1 - u16X0;
        u16Intercept  = u16Y1;
        u8AdjustValue  = u16X1 - u8AdjustValue;
    }

 // printf("u16DistanceOfY %u\n", u16DistanceOfY);
 // printf("u16DistanceOfX %u\n", u16DistanceOfX);
 // printf("u8Intercept %bu\n", u8Intercept);
 // printf("u8AdjustValue %bu\n", u8AdjustValue);

    u16Value = ((u16)u16DistanceOfY*u8AdjustValue/(u16DistanceOfX)) + u16Intercept;
    return u16Value;
}

bool __HalDispIfPictureTransNonLinear(u32 u32Interface, HalDispCsc_t *pstCsc, u32 *pu32Sharpness)
{
    HalDispPictureNonLinearCurveType_t *pstNonLinearCurve = NULL;
    bool bRet = 1;

    if(pstCsc)
    {
        pstNonLinearCurve = __HalDispIfGetNonLinearCurve(u32Interface, E_HAL_DISP_PICTURE_CONTRAST);
        if(pstNonLinearCurve)
        {
            pstCsc->u32Contrast = __HalDispIfCalculateNonLinear(pstNonLinearCurve, (u8)pstCsc->u32Contrast);
        }
        else
        {
            DISP_ERR("%s %d:: %s, Contrast NonLinearCurve NULL\n", __FUNCTION__, __LINE__, PARSING_HAL_INTERFACE(u32Interface));
            bRet = 0;
        }

        pstNonLinearCurve = __HalDispIfGetNonLinearCurve(u32Interface, E_HAL_DISP_PICTURE_HUE);
        if(pstNonLinearCurve)
        {
            pstCsc->u32Hue = __HalDispIfCalculateNonLinear(pstNonLinearCurve, (u8)pstCsc->u32Hue);
        }
        else
        {
            DISP_ERR("%s %d:: %s Hue NonLinearCurve NULL\n", __FUNCTION__, __LINE__, PARSING_HAL_INTERFACE(u32Interface));
            bRet = 0;
        }

        pstNonLinearCurve = __HalDispIfGetNonLinearCurve(u32Interface, E_HAL_DISP_PICTURE_SATURATION);
        if(pstNonLinearCurve)
        {
            pstCsc->u32Saturation = __HalDispIfCalculateNonLinear(pstNonLinearCurve, (u8)pstCsc->u32Saturation);
        }
        else
        {
            DISP_ERR("%s %d:: %s Saturation NonLinearCurve NULL\n", __FUNCTION__, __LINE__, PARSING_HAL_INTERFACE(u32Interface));
            bRet = 0;
        }

        pstNonLinearCurve = __HalDispIfGetNonLinearCurve(u32Interface, E_HAL_DISP_PICTURE_BRIGHTNESS);
        if(pstNonLinearCurve)
        {
            pstCsc->u32Luma = __HalDispIfCalculateNonLinear(pstNonLinearCurve, (u8)pstCsc->u32Luma);
        }
        else
        {
            DISP_ERR("%s %d:: %s Saturation NonLinearCurve NULL\n", __FUNCTION__, __LINE__, PARSING_HAL_INTERFACE(u32Interface));
            bRet = 0;
        }

        DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d, Con:%ld, Bri:%ld Hue:%ld Sat:%ld\n",
            __FUNCTION__, __LINE__, pstCsc->u32Contrast, pstCsc->u32Luma, pstCsc->u32Hue, pstCsc->u32Saturation);
    }

    if(pu32Sharpness)
    {
        pstNonLinearCurve = __HalDispIfGetNonLinearCurve(u32Interface, E_HAL_DISP_PICTURE_SHARPNESS);
        if(pstNonLinearCurve)
        {
            *pu32Sharpness = __HalDispIfCalculateNonLinear(pstNonLinearCurve, (u8)*pu32Sharpness);
        }
        else
        {
            DISP_ERR("%s %d:: %s Sharpness NonLinearCurve NULL\n", __FUNCTION__, __LINE__, PARSING_HAL_INTERFACE(u32Interface));
            bRet = 0;
        }

        DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d, Sharpness:%ld\n", __FUNCTION__, __LINE__, *pu32Sharpness);
    }
    return bRet;
}


void __HalDispIfSetTimingTable(HalDispDeviceTiming_e eTimeType, bool bCommon)
{
#if defined(HDMITX_VGA_SUPPORTED)
    HAL_DISP_VGA_INFO *pstInfo = NULL;
    u16 i, u16TimingId;
    bool bWrite;

    switch(eTimeType)
    {
        case E_HAL_DISP_OUTPUT_1080P24:
            u16TimingId = HAL_DISP_VGA_ID_1080_24P;
            break;

        case E_HAL_DISP_OUTPUT_1080P25:
            u16TimingId = HAL_DISP_VGA_ID_1080_25P;
            break;

        case E_HAL_DISP_OUTPUT_1080P30:
            u16TimingId = HAL_DISP_VGA_ID_1080_30P;
            break;

        case E_HAL_DISP_OUTPUT_720P50:
            u16TimingId = HAL_DISP_VGA_ID_720_50P;
            break;

        case E_HAL_DISP_OUTPUT_720P60:
            u16TimingId = HAL_DISP_VGA_ID_720_60P;
            break;

        case E_HAL_DISP_OUTPUT_1080I50:
        case E_HAL_DISP_OUTPUT_1080I60:
            u16TimingId = HAL_DISP_VGA_ID_NUM;
            break;

        case E_HAL_DISP_OUTPUT_1080P50:
            u16TimingId = HAL_DISP_VGA_ID_1080_50P;
            break;

        case E_HAL_DISP_OUTPUT_1080P60:
            u16TimingId = HAL_DISP_VGA_ID_1080_60P;
            break;

        case E_HAL_DISP_OUTPUT_576P50:
            u16TimingId = HAL_DISP_VGA_ID_576_50P;
            break;

        case E_HAL_DISP_OUTPUT_480P60:
            u16TimingId = HAL_DISP_VGA_ID_480_60P;
            break;

        case E_HAL_DISP_OUTPUT_640x480_60:
        case E_HAL_DISP_OUTPUT_800x600_60:
            u16TimingId = HAL_DISP_VGA_ID_NUM;
            break;

        case E_HAL_DISP_OUTPUT_1024x768_60:
            u16TimingId = HAL_DISP_VGA_ID_1024x768_60P;
            break;

        case E_HAL_DISP_OUTPUT_1280x1024_60:
            u16TimingId = HAL_DISP_VGA_ID_1280x1024_60P;
            break;

        case E_HAL_DISP_OUTPUT_1366x768_60:
            u16TimingId = HAL_DISP_VGA_ID_1366x768_60P;
            break;

        case E_HAL_DISP_OUTPUT_1440x900_60:
            u16TimingId = HAL_DISP_VGA_ID_1440x900_60P;
            break;

        case E_HAL_DISP_OUTPUT_1280x800_60:
            u16TimingId = HAL_DISP_VGA_ID_1280x800_60P;
            break;

        case E_HAL_DISP_OUTPUT_1680x1050_60:
            u16TimingId = HAL_DISP_VGA_ID_1680x1050_60P;
            break;

        case E_HAL_DISP_OUTPUT_1920x2160_30:
            u16TimingId = HAL_DISP_VGA_ID_NUM;
            break;

        case E_HAL_DISP_OUTPUT_1600x1200_60:
            u16TimingId = HAL_DISP_VGA_ID_1600x1200_60P;
            break;

        case E_HAL_DISP_OUTPUT_3840x2160_30:
            u16TimingId = HAL_DISP_VGA_ID_3840x2160_30P;
            break;

        case E_HAL_DISP_OUTPUT_1920x1200_60:
        case E_HAL_DISP_OUTPUT_2560x1440_30:
        case E_HAL_DISP_OUTPUT_2560x1600_60:
        case E_HAL_DISP_OUTPUT_3840x2160_60:
            u16TimingId = HAL_DISP_VGA_ID_NUM;
            break;

        default:
            u16TimingId = HAL_DISP_VGA_ID_NUM;
            break;
    }

    if(u16TimingId == HAL_DISP_VGA_ID_NUM)
    {
        DISP_ERR("%s %d, Not Support Timing, %s \n", __FUNCTION__, __LINE__, PARSING_HAL_TMING_ID(eTimeType));
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Time:%s, TimingTblId:%d, Common=%d\n",
            __FUNCTION__, __LINE__, PARSING_HAL_TMING_ID(eTimeType), u16TimingId, bCommon);

        for(i=0; i<HAL_DISP_VGA_TAB_NUM; i++)
        {
            pstInfo = &stHAL_DISP_VGA_TIMING_TBL[i];

            if(bCommon)
            {
                bWrite = pstInfo->enIPType == HAL_DISP_VGA_IP_COMMON ? TRUE : FALSE;
            }
            else
            {
                bWrite = TRUE;
            }

            if(bWrite)
            {
                if(pstInfo->enIPType == HAL_DISP_VGA_IP_COMMON)
                {
                    HalDispDumpRegTab(pstInfo->pData, pstInfo->u16RegNum, REG_DATA_SIZE, 0);
                }
                else
                {
                    HalDispDumpRegTab(pstInfo->pData, pstInfo->u16RegNum, HAL_DISP_VGA_ID_NUM, u16TimingId);
                }
            }
        }
    }
#endif
}

void __HalDispIfSetTgenConfig(HalDispDeviceTimingConfig_t *pstDeviceTimingCfg)
{
#if defined(HDMITX_VGA_SUPPORTED)
    u16 u16HsyncSt, u16VsyncSt;

    u16HsyncSt = (pstDeviceTimingCfg->u16Hstart) ?
        pstDeviceTimingCfg->u16Hstart - pstDeviceTimingCfg->u16HsyncWidth - pstDeviceTimingCfg->u16HsyncBackPorch :
        pstDeviceTimingCfg->u16Htotal - pstDeviceTimingCfg->u16HsyncWidth - pstDeviceTimingCfg->u16HsyncBackPorch;

    u16VsyncSt = (pstDeviceTimingCfg->u16Vstart) ?
        pstDeviceTimingCfg->u16Vstart - pstDeviceTimingCfg->u16VsyncWidth - pstDeviceTimingCfg->u16VsyncBackPorch :
        pstDeviceTimingCfg->u16Vtotal - pstDeviceTimingCfg->u16VsyncWidth - pstDeviceTimingCfg->u16VsyncBackPorch;

    //u16VsyncSt -= 1 ;// patch HW

    HalDispSetTgenHtt(pstDeviceTimingCfg->u16Htotal - 1);
    HalDispSetTgenVtt(pstDeviceTimingCfg->u16Vtotal - 1);

    HalDispSetTgenHsyncSt(u16HsyncSt);
    HalDispSetTgenHsyncEnd(u16HsyncSt + pstDeviceTimingCfg->u16HsyncWidth - 1);
    HalDispSetTgenVsyncSt(u16VsyncSt);
    HalDispSetTgenVsyncEnd(u16VsyncSt + pstDeviceTimingCfg->u16VsyncWidth - 1);


    HalDispSetTgenHfdeSt(pstDeviceTimingCfg->u16Hstart);
    HalDispSetTgenHfdeEnd(pstDeviceTimingCfg->u16Hstart + pstDeviceTimingCfg->u16Hactive - 1);
    HalDispSetTgenVfdeSt(pstDeviceTimingCfg->u16Vstart);
    HalDispSetTgenVfdeEnd(pstDeviceTimingCfg->u16Vstart + pstDeviceTimingCfg->u16Vactive - 1);

    //TgenDac
    HalDispSetTgenDacHsyncSt(u16HsyncSt);
    HalDispSetTgenDacHsyncEnd(u16HsyncSt + pstDeviceTimingCfg->u16HsyncWidth - 1);
    HalDispSetTgenDacHdeSt(pstDeviceTimingCfg->u16Hstart);
    HalDispSetTgenDacHdeEnd(pstDeviceTimingCfg->u16Hstart + pstDeviceTimingCfg->u16Hactive - 1);
    HalDispSetTgenDacVdeSt(pstDeviceTimingCfg->u16Vstart);
    HalDispSetTgenDacVdeEnd(pstDeviceTimingCfg->u16Vstart + pstDeviceTimingCfg->u16Vactive - 1);


    HalDispSetTgenHdeSt(pstDeviceTimingCfg->u16Hstart);
    HalDispSetTgenHdeEnd(pstDeviceTimingCfg->u16Hstart + pstDeviceTimingCfg->u16Hactive - 1);
    HalDispSetTgenVdeSt(pstDeviceTimingCfg->u16Vstart);
    HalDispSetTgenVdeEnd(pstDeviceTimingCfg->u16Vstart + pstDeviceTimingCfg->u16Vactive - 1);
#endif
}

void __HalDispIfSetCscConfig(HalDispCsc_t *pstCscCfg)
{
    if(pstCscCfg->eCscMatrix == E_HAL_DISP_CSC_MATRIX_BYPASS)
    {
        HalDispColorSeletYuvToRgbMatrix(HAL_DISP_COLOR_CSC_ID, E_DISP_COLOR_YUV_2_RGB_MATRIX_BYPASS, NULL);
    }
    else if(pstCscCfg->eCscMatrix == E_HAL_DISP_CSC_MATRIX_BT601_TO_RGB_PC)
    {
        HalDispColorSeletYuvToRgbMatrix(HAL_DISP_COLOR_CSC_ID, E_DISP_COLOR_YUV_2_RGB_MATRIX_SDTV, NULL);
    }
    else if(pstCscCfg->eCscMatrix == E_HAL_DISP_CSC_MATRIX_BT709_TO_RGB_PC)
    {
        HalDispColorSeletYuvToRgbMatrix(HAL_DISP_COLOR_CSC_ID, E_DISP_COLOR_YUV_2_RGB_MATRIX_HDTV, NULL);
    }

    HalDispColorAdjustHCS(HAL_DISP_COLOR_CSC_ID, pstCscCfg->u32Hue, pstCscCfg->u32Saturation, pstCscCfg->u32Contrast);
    HalDispColorAdjustBrightness(HAL_DISP_COLOR_CSC_ID, pstCscCfg->u32Luma);
}

HalDispQueryRet_e _HalDispIfGetInfoDeviceInit(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];

    pstDeviceContain->stVgaParam.stCsc.u32Contrast = 0x80;
    pstDeviceContain->stVgaParam.stCsc.u32Hue = 50;
    pstDeviceContain->stVgaParam.stCsc.u32Saturation = 0x80;
    pstDeviceContain->stVgaParam.stCsc.u32Luma = 0x80;

    pstDeviceContain->stHdmiParam.stCsc.u32Contrast = 0x80;
    pstDeviceContain->stHdmiParam.stCsc.u32Hue = 50;
    pstDeviceContain->stHdmiParam.stCsc.u32Saturation = 0x80;
    pstDeviceContain->stHdmiParam.stCsc.u32Luma = 0x80;

    pstDeviceContain->u32BgColor = 0;

    return enRet;
}

void _HalDispIfSetDeviceInit(void *pCtx, void *pCfg)
{
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];

    // scaler_e
    HalDispSetMaceSrc(1); // extVideo

    HalDispSetFrameColor(0x10, 0, 0x10); // frameColor
    HalDispSetDispWinColor(0x10, 0, 0x10); // DispWinColor
    HalDispSetDispWinColorForce(1);

    // color
    HalDispColorInitVar();
    HalDispColorSetColorCorrectMatrix(0, tVideoColorCorrectionMatrix);
    HalDispColorSetColorCorrectMatrix(1, tVideoColorCorrectionMatrix);
    HalDispColorSeletYuvToRgbMatrix(0, E_DISP_COLOR_YUV_2_RGB_MATRIX_HDTV, NULL);
    HalDispColorSeletYuvToRgbMatrix(1, E_DISP_COLOR_YUV_2_RGB_MATRIX_HDTV, NULL);


    HalDispColorAdjustHCS(HAL_DISP_COLOR_CSC_ID,
        pstDeviceContain->stHdmiParam.stCsc.u32Hue,
        pstDeviceContain->stHdmiParam.stCsc.u32Saturation,
        pstDeviceContain->stHdmiParam.stCsc.u32Contrast);

    HalDispColorAdjustBrightness(HAL_DISP_COLOR_CSC_ID, pstDeviceContain->stHdmiParam.stCsc.u32Luma);

    __HalDispIfSetTimingTable(E_HAL_DISP_OUTPUT_480P60, TRUE);

#if defined(HDMITX_VGA_SUPPORTED)
    HalDispGetDacTriming(&gu16DacTrim[0], &gu16DacTrim[1], &gu16DacTrim[2]);
    //HalDispSetVgaHpdInit();
#endif
}

HalDispQueryRet_e _HalDispIfGetInfoDeviceEnable(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    bool *pbEn;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pbEn = (bool *)pCfg;

    pstDeviceContain->bEnable = *pbEn;

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Enable:%x\n",
        __FUNCTION__, __LINE__, pstDeviceContain->bEnable);

    return enRet;
}

void _HalDispIfSetDeviceEnable(void *pCtx, void *pCfg)
{
    bool bEn;

    bEn = *(bool *)pCfg;
    HalDispSetDispWinColorForce(bEn ? 0 : 1);
}

HalDispQueryRet_e _HalDispIfGetInfoDeviceAttach(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetDeviceAttach(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoDeviceDetach(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetDeviceDetach(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoDeviceBackGroundColor(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    u32 *pu32BgColor;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pu32BgColor = (u32 *)pCfg;

    pstDeviceContain->u32BgColor = *pu32BgColor;

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, BgColor:%lx\n",
        __FUNCTION__, __LINE__, pstDeviceContain->u32BgColor);

    return enRet;
}

void _HalDispIfSetDeviceBackGroundColor(void *pCtx, void *pCfg)
{
    u32 *pu32BgColor;
    u8 u8R, u8G, u8B;

    pu32BgColor = (u32 *)pCfg;

    u8R = (*pu32BgColor & 0x000000FF) >> 3; // HW is 5 bits
    u8G = (*pu32BgColor & 0x0000FF00) >> (8+3);
    u8B = (*pu32BgColor & 0x00FF0000) >> (16+3);
    HalDispSetFrameColor(u8R, u8G, u8B);
}

HalDispQueryRet_e _HalDispIfGetInfoDeviceInterface(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    u32 *pu32Interface;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pu32Interface = (u32 *)pCfg;

    if( *pu32Interface & HAL_DISP_INTF_CVBS)
    {
        enRet = E_HAL_DISP_QUERY_RET_CFGERR;
    }
    else
    {
        pstDeviceContain->u32Interface |= *pu32Interface;
    }

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Interface:%s, DeviceContain:%lx\n",
        __FUNCTION__, __LINE__,
        PARSING_HAL_INTERFACE(*pu32Interface), pstDeviceContain->u32Interface);

    return enRet;
}

void _HalDispIfSetDeviceInterface(void *pCtx, void *pCfg)
{
    u32 *pu32Interface;

    pu32Interface = (u32 *)pCfg;

    if(*pu32Interface & (HAL_DISP_INTF_HDMI | HAL_DISP_INTF_VGA))
    {
        // For HDMI & VGA

    }
    else if(*pu32Interface & HAL_DISP_INTF_LCD)
    {
        // For LCD
    }
    else
    {

    }
}

HalDispQueryRet_e _HalDispIfGetInfoDeviceOutputTiming(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    HalDispDeviceTimingInfo_t *pstDeviceTimingCfg;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pstDeviceTimingCfg = (HalDispDeviceTimingInfo_t *)pCfg;

    memcpy(&pstDeviceContain->stDevTimingCfg, &pstDeviceTimingCfg->stDeviceTimingCfg, sizeof(HalDispDeviceTimingConfig_t));

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, H(%d %d %d %d %d) V(%d %d %d %d %d) Fps:%d, Ssc(%x %x)\n",
        __FUNCTION__, __LINE__,
        pstDeviceContain->stDevTimingCfg.u16HsyncWidth, pstDeviceContain->stDevTimingCfg.u16HsyncBackPorch,
        pstDeviceContain->stDevTimingCfg.u16Hstart, pstDeviceContain->stDevTimingCfg.u16Hactive,
        pstDeviceContain->stDevTimingCfg.u16Htotal,
        pstDeviceContain->stDevTimingCfg.u16VsyncWidth, pstDeviceContain->stDevTimingCfg.u16VsyncBackPorch,
        pstDeviceContain->stDevTimingCfg.u16Vstart, pstDeviceContain->stDevTimingCfg.u16Vactive,
        pstDeviceContain->stDevTimingCfg.u16Vtotal,
        pstDeviceContain->stDevTimingCfg.u16Fps,
        pstDeviceContain->stDevTimingCfg.u16SscStep, pstDeviceContain->stDevTimingCfg.u16SscSpan);

    return enRet;
}

void _HalDispIfSetDeviceOutputTiming(void *pCtx, void *pCfg)
{
#if defined(HDMITX_VGA_SUPPORTED)

    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    HalDispDeviceTimingInfo_t *pstDeviceTimingCfg;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pstDeviceTimingCfg = (HalDispDeviceTimingInfo_t *)pCfg;

    if((pstDeviceContain->u32Interface & ~(HAL_DISP_INTF_HDMI | HAL_DISP_INTF_VGA | HAL_DISP_INTF_LCD)))
    {
        DISP_ERR("%s %d, Unknown Interface:%lx\n", __FUNCTION__, __LINE__, pstDeviceContain->u32Interface);
    }
    else
    {
        HalDispSetFpllEn(0); //Disable Fpll En
        HalDispSetSwReste(1);

        if(pstDeviceContain->u32Interface & (HAL_DISP_INTF_HDMI | HAL_DISP_INTF_VGA))
        {
            __HalDispIfSetTgenConfig(&pstDeviceTimingCfg->stDeviceTimingCfg);
            __HalDispIfSetTimingTable(pstDeviceTimingCfg->eTimeType, FALSE);

            HalDispSetHdmitxSsc(
                pstDeviceTimingCfg->stDeviceTimingCfg.u16SscStep,
                pstDeviceTimingCfg->stDeviceTimingCfg.u16SscSpan);

            HalDispSetFpllEn(1); //Disable Fpll En

            HalDispSetSwReste(0);
            DrvDispOsMsSleep(200);

            HalDispSetDacReset(1);
            DrvDispOsMsSleep(100);
            HalDispSetDacReset(0);

            if(pstDeviceContain->u32Interface & HAL_DISP_INTF_VGA)
            {
                HalDispIrqSetDacEn(1);
                if(DrvDispOsPadMuxActive() == 0)
                {
                    HalDispSetDacMux(1);
                }
            }
            else
            {
                HalDispIrqSetDacEn(0);

                if(DrvDispOsPadMuxActive() == 0)
                {
                    HalDispSetDacMux(0);
                }
            }
        }
        else
        {  // LCD
            //__HalDispIfSetTgenConfig(&pstDeviceTimingCfg->stDeviceTimingCfg);
        }
    }
#endif
}

HalDispQueryRet_e _HalDispIfGetInfoDeviceVgaParam(void *pCtx, void *pCfg)
{
#if defined(HDMITX_VGA_SUPPORTED)
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;
    HalDispVgaParam_t *pstVgaParm = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pstVgaParm = (HalDispVgaParam_t *)pCfg;

    if(pstVgaParm->stCsc.eCscMatrix == E_HAL_DISP_CSC_MATRIX_BYPASS ||
       pstVgaParm->stCsc.eCscMatrix == E_HAL_DISP_CSC_MATRIX_BT601_TO_RGB_PC ||
       pstVgaParm->stCsc.eCscMatrix == E_HAL_DISP_CSC_MATRIX_BT709_TO_RGB_PC)
    {
        memcpy(&pstDeviceContain->stVgaParam, pstVgaParm, sizeof(HalDispVgaParam_t));

        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Matrix:%s, Luma:%ld, Contrast:%ld, Hue:%ld, Sat:%ld, Gain:%ld Sharp:%ld\n",
            __FUNCTION__, __LINE__,
            PARSING_HAL_CSC_MATRIX(pstVgaParm->stCsc.eCscMatrix),
            pstVgaParm->stCsc.u32Luma, pstVgaParm->stCsc.u32Contrast,
            pstVgaParm->stCsc.u32Hue, pstVgaParm->stCsc.u32Saturation,
            pstVgaParm->u32Gain, pstVgaParm->u32Sharpness);
    }
    else
    {
        enRet = E_HAL_DISP_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Not Support %s\n", __FUNCTION__, __LINE__, PARSING_HAL_CSC_MATRIX(pstVgaParm->stCsc.eCscMatrix));
    }

    return enRet;
#else
    return E_HAL_DISP_QUERY_RET_NOTSUPPORT;
#endif
}

void _HalDispIfSetDeviceVgaParam(void *pCtx, void *pCfg)
{
#if defined(HDMITX_VGA_SUPPORTED)
    HalDispVgaParam_t *pstVgaParm = NULL;
    HalDispPqHwContext_t *pstPqHwCtx;
    HalDispPictureNonLinearCurveType_t *pstNonLinearCurve = NULL;
    u16 i, u16Gain[3];

    pstVgaParm = (HalDispVgaParam_t *)pCfg;
    HalDispPqGetHwCtx(&pstPqHwCtx);

    if(__HalDispIfPictureTransNonLinear(HAL_DISP_INTF_VGA, &pstVgaParm->stCsc, &pstVgaParm->u32Sharpness))
    {
        __HalDispIfSetCscConfig(&pstVgaParm->stCsc);

        if(pstPqHwCtx->enFlag & E_HAL_DISP_PQ_FLAG_PEAKING)
        {
            pstPqHwCtx->stPeakingCfg.bUpdate = 1;
            pstPqHwCtx->stPeakingCfg.u8Band1Coef = (u8)pstVgaParm->u32Sharpness;
            pstPqHwCtx->stPeakingCfg.u8Band2Coef = (u8)pstVgaParm->u32Sharpness;
            HalDispPqSetPeaking(&pstPqHwCtx->stPeakingCfg);
        }
        else
        {
            DISP_ERR("%s %d, Peaking is not set\n", __FUNCTION__, __LINE__);
        }


        for(i=0; i<3; i++)
        {
            pstNonLinearCurve = (gu16DacTrim[i]) ?
                                __HalDispIfSetNonLinearCurve(HAL_DISP_INTF_VGA, gu16DacTrim[i]) :
                                __HalDispIfGetNonLinearCurve(HAL_DISP_INTF_VGA, E_HAL_DISP_PICTURE_GAIN);

            if(pstNonLinearCurve)
            {
                u16Gain[i] = __HalDispIfCalculateNonLinear(pstNonLinearCurve, (u8)pstVgaParm->u32Gain);
            }
            else
            {
                DISP_ERR("%s %d, Gain %s NonlinearCurve Fail\n",
                    __FUNCTION__, __LINE__,
                    i==0 ? "CH R" : i==1 ? "CH G" : "CH B");
            }
        }

        DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d, ChipTrim(%x %x %x), Gain:%ld, Trim(%x %x %x)\n",
            __FUNCTION__, __LINE__,
            gu16DacTrim[0], gu16DacTrim[1], gu16DacTrim[2],
            pstVgaParm->u32Gain, u16Gain[0], u16Gain[1], u16Gain[2]);

        // DAC Triming: 0x00 ~ 0x7F
        HalDispSetDacTrimming(u16Gain[0], u16Gain[1], u16Gain[2]);
    }
    else
    {
        DISP_ERR("%s %d, Trans NonLiear Fail\n", __FUNCTION__, __LINE__);
    }
#endif
}

HalDispQueryRet_e _HalDispIfGetInfoDeviceHdmiParam(void *pCtx, void *pCfg)
{
#if defined(HDMITX_VGA_SUPPORTED)
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;
    HalDispHdmiParam_t *pstHdmiParm = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pstHdmiParm = (HalDispHdmiParam_t *)pCfg;

    if(pstHdmiParm->stCsc.eCscMatrix == E_HAL_DISP_CSC_MATRIX_BYPASS ||
       pstHdmiParm->stCsc.eCscMatrix == E_HAL_DISP_CSC_MATRIX_BT601_TO_RGB_PC ||
       pstHdmiParm->stCsc.eCscMatrix == E_HAL_DISP_CSC_MATRIX_BT709_TO_RGB_PC)
    {
        memcpy(&pstDeviceContain->stHdmiParam, pstHdmiParm, sizeof(HalDispHdmiParam_t));

        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Matrix:%s, Luma:%ld, Contrsat:%ld, Hue:%ld, Sat:%ld, Sharp:%ld\n",
            __FUNCTION__, __LINE__,
            PARSING_HAL_CSC_MATRIX(pstHdmiParm->stCsc.eCscMatrix),
            pstHdmiParm->stCsc.u32Luma, pstHdmiParm->stCsc.u32Contrast,
            pstHdmiParm->stCsc.u32Hue, pstHdmiParm->stCsc.u32Saturation,
            pstHdmiParm->u32Sharpness);
    }
    else
    {
        enRet = E_HAL_DISP_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Not Support %s\n", __FUNCTION__, __LINE__, PARSING_HAL_CSC_MATRIX(pstHdmiParm->stCsc.eCscMatrix));
    }

    return enRet;
#else
    return E_HAL_DISP_QUERY_RET_NOTSUPPORT;
#endif
}

void _HalDispIfSetDeviceHdmiParam(void *pCtx, void *pCfg)
{
#if defined(HDMITX_VGA_SUPPORTED)
    HalDispHdmiParam_t *pstHdmiParm = NULL;
    HalDispPqHwContext_t *pstPqHwCtx = NULL;;

    pstHdmiParm = (HalDispHdmiParam_t *)pCfg;
    HalDispPqGetHwCtx(&pstPqHwCtx);

    if(__HalDispIfPictureTransNonLinear(HAL_DISP_INTF_HDMI, &pstHdmiParm->stCsc, &pstHdmiParm->u32Sharpness))
    {
        __HalDispIfSetCscConfig(&pstHdmiParm->stCsc);

        if(pstPqHwCtx->enFlag & E_HAL_DISP_PQ_FLAG_PEAKING)
        {
            pstPqHwCtx->stPeakingCfg.bUpdate = 1;
            pstPqHwCtx->stPeakingCfg.u8Band1Coef = (u8)pstHdmiParm->u32Sharpness;
            pstPqHwCtx->stPeakingCfg.u8Band2Coef = (u8)pstHdmiParm->u32Sharpness;
            HalDispPqSetPeaking(&pstPqHwCtx->stPeakingCfg);
        }
        else
        {
            DISP_ERR("%s %d, Peaking is not set\n", __FUNCTION__, __LINE__);
        }

    }
    else
    {
        DISP_ERR("%s %d, Trans NonLiear Fail\n", __FUNCTION__, __LINE__);
    }
#endif
}

HalDispQueryRet_e _HalDispIfGetInfoDeviceLcdParam(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;
    HalDispLcdParam_t *pstLcdParm = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pstLcdParm = (HalDispLcdParam_t *)pCfg;

    if(pstLcdParm->stCsc.eCscMatrix == E_HAL_DISP_CSC_MATRIX_BYPASS ||
       pstLcdParm->stCsc.eCscMatrix == E_HAL_DISP_CSC_MATRIX_BT601_TO_RGB_PC ||
       pstLcdParm->stCsc.eCscMatrix == E_HAL_DISP_CSC_MATRIX_BT709_TO_RGB_PC)
    {
        memcpy(&pstDeviceContain->stLcdParam, pstLcdParm, sizeof(HalDispLcdParam_t));

        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Matrix:%s, Luma:%ld, Contrast:%ld, Hue:%ld, Sat:%ld, Sharp:%ld\n",
            __FUNCTION__, __LINE__,
            PARSING_HAL_CSC_MATRIX(pstLcdParm->stCsc.eCscMatrix),
            pstLcdParm->stCsc.u32Luma, pstLcdParm->stCsc.u32Contrast,
            pstLcdParm->stCsc.u32Hue, pstLcdParm->stCsc.u32Saturation,
            pstLcdParm->u32Sharpness);
    }
    else
    {
        enRet = E_HAL_DISP_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Not Support %s\n", __FUNCTION__, __LINE__, PARSING_HAL_CSC_MATRIX(pstLcdParm->stCsc.eCscMatrix));
    }

    return enRet;
}

void _HalDispIfSetDeviceLcdParam(void *pCtx, void *pCfg)
{
    HalDispLcdParam_t *pstLcdParm = NULL;
    HalDispPqHwContext_t *pstPqHwCtx = NULL;

    pstLcdParm = (HalDispLcdParam_t *)pCfg;
    HalDispPqGetHwCtx(&pstPqHwCtx);

    if(__HalDispIfPictureTransNonLinear(HAL_DISP_INTF_LCD, &pstLcdParm->stCsc, &pstLcdParm->u32Sharpness))
    {
        __HalDispIfSetCscConfig(&pstLcdParm->stCsc);

        if(pstPqHwCtx->enFlag & E_HAL_DISP_PQ_FLAG_PEAKING)
        {
            pstPqHwCtx->stPeakingCfg.bUpdate = 1;
            pstPqHwCtx->stPeakingCfg.u8Band1Coef = (u8)pstLcdParm->u32Sharpness;
            pstPqHwCtx->stPeakingCfg.u8Band2Coef = (u8)pstLcdParm->u32Sharpness;
            HalDispPqSetPeaking(&pstPqHwCtx->stPeakingCfg);
        }
        else
        {
            DISP_ERR("%s %d, Peaking is not set\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        DISP_ERR("%s %d, Trans NonLiear Fail\n", __FUNCTION__, __LINE__);
    }
}


HalDispQueryRet_e _HalDispIfGetInfoDeviceGammaParam(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;
    HalDispGammaParam_t *pstGammaParam = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pstGammaParam = (HalDispGammaParam_t *)pCfg;


    if(pstGammaParam->bEn && pstGammaParam->u16EntryNum != 33)
    {
        enRet = E_HAL_DISP_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, ColorEntyr is %d, not 32\n", __FUNCTION__, __LINE__, pstGammaParam->u16EntryNum);
    }
    else
    {
        if(pstDeviceContain->stGammaParam.pu8ColorR == NULL &&
           pstDeviceContain->stGammaParam.pu8ColorB == NULL &&
           pstDeviceContain->stGammaParam.pu8ColorG == NULL)
        {
            pstDeviceContain->stGammaParam.pu8ColorR = DrvDispOsMemAlloc(pstGammaParam->u16EntryNum);
            pstDeviceContain->stGammaParam.pu8ColorG = DrvDispOsMemAlloc(pstGammaParam->u16EntryNum);
            pstDeviceContain->stGammaParam.pu8ColorB = DrvDispOsMemAlloc(pstGammaParam->u16EntryNum);
        }
        else if(pstDeviceContain->stGammaParam.u16EntryNum < pstGammaParam->u16EntryNum)
        {
            DrvDispOsMemRelease(pstDeviceContain->stGammaParam.pu8ColorR);
            DrvDispOsMemRelease(pstDeviceContain->stGammaParam.pu8ColorG);
            DrvDispOsMemRelease(pstDeviceContain->stGammaParam.pu8ColorB);
            pstDeviceContain->stGammaParam.pu8ColorR = DrvDispOsMemAlloc(pstGammaParam->u16EntryNum);
            pstDeviceContain->stGammaParam.pu8ColorG = DrvDispOsMemAlloc(pstGammaParam->u16EntryNum);
            pstDeviceContain->stGammaParam.pu8ColorB = DrvDispOsMemAlloc(pstGammaParam->u16EntryNum);
        }


        if(pstDeviceContain->stGammaParam.pu8ColorR == NULL ||
           pstDeviceContain->stGammaParam.pu8ColorG == NULL ||
           pstDeviceContain->stGammaParam.pu8ColorB == NULL)
        {
            enRet = E_HAL_DISP_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, Alloc Memory Fail\n", __FUNCTION__, __LINE__);
        }
        else
        {
            memcpy(pstDeviceContain->stGammaParam.pu8ColorR, pstGammaParam->pu8ColorR, pstGammaParam->u16EntryNum);
            memcpy(pstDeviceContain->stGammaParam.pu8ColorG, pstGammaParam->pu8ColorG, pstGammaParam->u16EntryNum);
            memcpy(pstDeviceContain->stGammaParam.pu8ColorB, pstGammaParam->pu8ColorB, pstGammaParam->u16EntryNum);
            pstDeviceContain->stGammaParam.u16EntryNum = pstGammaParam->u16EntryNum;
            pstDeviceContain->stGammaParam.bEn = pstGammaParam->bEn;
        }
    }
    return enRet;
}

void _HalDispIfSetDeviceGammaParam(void *pCtx, void *pCfg)
{
    HalDispGammaParam_t *pstGammaParam = NULL;
    HalDispPqGammaConfig_t stHalPqGammaCfg;

    pstGammaParam = (HalDispGammaParam_t *)pCfg;

    stHalPqGammaCfg.bUpdate = 1;
    stHalPqGammaCfg.u8En = pstGammaParam->bEn;
    memcpy(stHalPqGammaCfg.u8R, pstGammaParam->pu8ColorR, 33);
    memcpy(stHalPqGammaCfg.u8G, pstGammaParam->pu8ColorG, 33);
    memcpy(stHalPqGammaCfg.u8B, pstGammaParam->pu8ColorB, 33);

    HalDispPqSetGammaConfig(&stHalPqGammaCfg);
}


HalDispQueryRet_e _HalDispIfGetInfoDeviceCvbsParam(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetDeviceCvbsParam(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoDeviceColorTemp(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;
    HalDispColorTemp_t *pstColorTemp = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pstColorTemp = (HalDispColorTemp_t *)pCfg;

    memcpy(&pstDeviceContain->stColorTemp, pstColorTemp, sizeof(HalDispColorTemp_t));

    return enRet;
}

void _HalDispIfSetDeviceColorTemp(void *pCtx, void *pCfg)
{
    HalDispColorTemp_t *pstColorTemp = NULL;

    pstColorTemp = (HalDispColorTemp_t *)pCfg;
    HalDispColorAdjustVideoRGB(HAL_DISP_COLOR_CSC_ID, pstColorTemp->u16RedColor, pstColorTemp->u16GreenColor, pstColorTemp->u16BlueColor);
}

HalDispQueryRet_e _HalDispIfGetInfoDevicTimeZone(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    HalDispTimeZone_t *pstTimeZone = NULL;
    u32 u32IrqFlag;

    pstTimeZone = (HalDispTimeZone_t *)pCfg;

    HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_INTERNAL_GET_STATUS, E_HAL_DISP_IRQ_TYPE_NONE, (void *)&u32IrqFlag);

    DISP_DBG(DISP_DBG_LEVEL_HAL,"%s %d, IrqFlag=%lx\n", __FUNCTION__, __LINE__, u32IrqFlag);

    if(u32IrqFlag == E_HAL_DISP_IRQ_TYPE_INTERNAL_VSYNC_POSITIVE)
    {
        pstTimeZone->enType = E_HAL_DISP_TIMEZONE_BACKPORCH;
    }
    else if(u32IrqFlag == E_HAL_DISP_IRQ_TYPE_INTERNAL_VDE_POSITIVE)
    {
        pstTimeZone->enType = E_HAL_DISP_TIMEZONE_ACTIVE;
    }
    else if(u32IrqFlag == E_HAL_DISP_IRQ_TYPE_INTERNAL_VDE_NEGATIVE)
    {
        pstTimeZone->enType = E_HAL_DISP_TIMEZONE_FRONTPORCH;
    }

    return enRet;
}

void _HalDispIfSetDeviceTimeZone(void *pCtx, void *pCfg)
{

}


HalDispQueryRet_e _HalDispIfGetInfoDevicDisplayInfo(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxDeviceContain_t *pstDeviceContain = NULL;
    HalDispDeviceTimingConfig_t *pstDevTimingCfg = NULL;
    HalDispDisplayInfo_t *pstDisplayInfo = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32Idx];
    pstDevTimingCfg = &pstDeviceContain->stDevTimingCfg;
    pstDisplayInfo = (HalDispDisplayInfo_t *)pCfg;

    pstDisplayInfo->u16Htotal = pstDevTimingCfg->u16Htotal;
    pstDisplayInfo->u16Vtotal = pstDevTimingCfg->u16Vtotal;
    pstDisplayInfo->u16HdeStart = pstDevTimingCfg->u16Hstart;
    pstDisplayInfo->u16VdeStart = pstDevTimingCfg->u16Vstart;
    pstDisplayInfo->u16Width = pstDevTimingCfg->u16Hactive;
    pstDisplayInfo->u16Height = pstDevTimingCfg->u16Vactive;
    pstDisplayInfo->bInterlace = FALSE;
    pstDisplayInfo->bYuvOutput = FALSE;

    return enRet;
}

void _HalDispIfSetDeviceDisplayInfo(void *pCtx, void *pCfg)
{

}

//-------------------------------------------------------------------------------
// VidLayer
//-------------------------------------------------------------------------------
HalDispQueryRet_e _HalDispIfGetInfoVidLayerInit(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    return enRet;
}

void _HalDispIfSetVidLayerInit(void *pCtx, void *pCfg)
{
}

HalDispQueryRet_e _HalDispIfGetInfoVidLayerEnable(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NONEED;
    return enRet;
}

void _HalDispIfSetVidLayerEnable(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoVidLayerBind(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NONEED;
    return enRet;
}

void _HalDispIfSetVidLayerBind(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoVidLayerUnBind(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NONEED;
    return enRet;
}

void _HalDispIfSetVidLayerUnBind(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoVidLayerAttr(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NONEED;
    return enRet;
}

void _HalDispIfSetVidLayerAttr(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoVidLayerCompress(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetVidLayerComporess(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoVidLayerPriority(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetVidLayerPriority(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoVidLayerBufferFire(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetVidLayerBufferFire(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoVidLayerCheckFire(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetVidLayerCheckFire(void *pCtx, void *pCfg)
{

}

//-------------------------------------------------------------------------------
// InputPort
//-------------------------------------------------------------------------------
HalDispQueryRet_e _HalDispIfGetInfoInputPortInit(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];

    if(pstInputPortContain->u32PortId > E_HAL_DISP_MOPG_ID_NUM)
    {
        enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }

    return enRet;
}

void _HalDispIfSetInputPortInit(void *pCtx, void *pCfg)
{
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain;
    static bool sbMopgHwInit = 0;
    static bool sbMopsHwInit = 0;
    static bool sbMopRotHwInit = 0;
    static bool sbMopsSwShadowInit = 0;
    HalDispMopgGwinId_e stTemp = 0;
    u8 i = 0;
    u8 j = 0;
    bool bPortInit = 0;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstVidLayerContain = (DrvDispCtxVideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;

    //init MOP rotate HW
    if(!sbMopRotHwInit)
    {
        HalDispMopRotInit();
        sbMopRotHwInit=1;
        if(!sbMopsSwShadowInit)
        {
            HalMopsShadowInit();
            sbMopsSwShadowInit=1;
        }
    }

    if(pstVidLayerContain)
    {
        if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPG_ID)&&
           (pstInputPortContain->u32PortId < E_HAL_DISP_MOPG_ID_NUM))//init mopg
        {
            if(sbMopgHwInit==0)
            {
                HalDispMopgInit();
                sbMopgHwInit=1;
            }
            //reset gwin register when create instance
            DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, init Port_id:%ld\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
            HalDispMopgGwinEn((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, FALSE);
            HalDispMopgSetSourceParam((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, 1, 1);
            HalDispMopgSetGwinParam((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, 0, 0, 1, 1);
            for(i=0;i<gbMopgInitNum;i++)//find this id is init or not
            {
                if(gpMopgLbOdTbl[i].eMopgId == pstInputPortContain->u32PortId)
                {
                    bPortInit=1;
                    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Port_id:%ld has been initialized, reinit\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
                    break;
                }
            }
            if(bPortInit)//reinit for lb count list
            {
                stTemp = gpMopgLbOdTbl[i].eMopgId;
                for(j=i;j>0;j--)//shift all member to next
                {
                    gpMopgLbOdTbl[j].eMopgId = gpMopgLbOdTbl[j-1].eMopgId;
                    gpMopgLbOdTbl[j].u16GwinHst = gpMopgLbOdTbl[j-1].u16GwinHst;
                    gpMopgLbOdTbl[j].u8RealOrder = gpMopgLbOdTbl[j-1].u8RealOrder;
                    gpMopgLbOdTbl[j].bIsAttred = gpMopgLbOdTbl[j-1].bIsAttred;
                }
                gpMopgLbOdTbl[0].eMopgId = stTemp;//add new member at [0]
                gpMopgLbOdTbl[0].u16GwinHst = 0;
                gpMopgLbOdTbl[0].u8RealOrder = 0;
                gpMopgLbOdTbl[0].bIsAttred = 0;
            }
            if(!bPortInit && (gbMopgInitNum<E_HAL_DISP_MOPG_ID_NUM))//add new member into lb count list
            {
                for(i=gbMopgInitNum;i>0;i--)//shift all member to next
                {
                    gpMopgLbOdTbl[i].eMopgId = gpMopgLbOdTbl[i-1].eMopgId;
                    gpMopgLbOdTbl[i].u16GwinHst = gpMopgLbOdTbl[i-1].u16GwinHst;
                    gpMopgLbOdTbl[i].u8RealOrder = gpMopgLbOdTbl[i-1].u8RealOrder;
                    gpMopgLbOdTbl[i].bIsAttred = gpMopgLbOdTbl[i-1].bIsAttred;
                }
                gpMopgLbOdTbl[0].eMopgId = (HalDispMopgGwinId_e)pstInputPortContain->u32PortId;//add new member at [0]
                gpMopgLbOdTbl[0].u16GwinHst = 0;
                gpMopgLbOdTbl[0].u8RealOrder = 0;
                gpMopgLbOdTbl[0].bIsAttred = 0;
                gbMopgInitNum++;
            }
        }
        else if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPS_ID)&&
                (pstInputPortContain->u32PortId == E_HAL_DISP_MOPS_ID_00))//init mops
        {
            if(sbMopsHwInit==0)
            {
                HalDispMopsInit();
                sbMopsHwInit=1;
                if(!sbMopsSwShadowInit)
                {
                    HalMopsShadowInit();
                    sbMopsSwShadowInit=1;
                }
                //mops will work when mopg is init...
                if(!sbMopgHwInit)
                {
                    HalDispMopgInit();
                    sbMopgHwInit=1;
                }
            }
            //reset gwin register when create instance
            HalDispMopsGwinEn(FALSE);
            HalDispMopsSetSourceParam(0, 0);
            HalDispMopsSetGwinParam(0, 0, 1, 1);
        }
        else
        {
            DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
        }
    }
    else
    {
        DISP_ERR("%s %d, Port_id:%ld VidoeLayer Not Bind\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }
}

void _HalDispIfSetAllMopgLineBufVal(void)
{
    bool bFirstAttred=0;
    int i=0;
    u16 u16TmpHstr=0;
    u8 u8TmpOrder=0;

    for(i=0;i<gbMopgInitNum;i++)//set line_buffer_start
    {
        gpMopgLbOdTbl[i].u8RealOrder=0;//reset order first
        if(gpMopgLbOdTbl[i].bIsAttred)//find real order for those actually setting attr
        {
            if(bFirstAttred==0)
            {
                gpMopgLbOdTbl[i].u8RealOrder=0;
                u16TmpHstr = gpMopgLbOdTbl[i].u16GwinHst;
                u8TmpOrder = gpMopgLbOdTbl[i].u8RealOrder;
                HalDispMopgSetLineBufStr(gpMopgLbOdTbl[i].eMopgId,
                                         gpMopgLbOdTbl[i].u16GwinHst,
                                         gpMopgLbOdTbl[i].u8RealOrder);
                bFirstAttred=1;
            }
            else
            {
                if(gpMopgLbOdTbl[i].u16GwinHst == u16TmpHstr)
                {
                    gpMopgLbOdTbl[i].u8RealOrder = u8TmpOrder;
                }
                else
                {
                    gpMopgLbOdTbl[i].u8RealOrder = u8TmpOrder+1;
                    u8TmpOrder = gpMopgLbOdTbl[i].u8RealOrder;
                    u16TmpHstr = gpMopgLbOdTbl[i].u16GwinHst;
                }
                HalDispMopgSetLineBufStr(gpMopgLbOdTbl[i].eMopgId,
                                         gpMopgLbOdTbl[i].u16GwinHst,
                                         gpMopgLbOdTbl[i].u8RealOrder);
            }
        }
        DISP_DBG(DISP_DBG_LEVEL_HAL, "[%d] ID=%d, Win_st=%d, order=%d, attred=%d\n",i, gpMopgLbOdTbl[i].eMopgId,gpMopgLbOdTbl[i].u16GwinHst,gpMopgLbOdTbl[i].u8RealOrder,gpMopgLbOdTbl[i].bIsAttred);

    }
}


HalDispQueryRet_e _HalDispIfGetInfoInputPortEnable(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    bool *pbEnable = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstVidLayerContain = (DrvDispCtxVideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;
    pbEnable = (bool *)pCfg;

    pstInputPortContain->bEnable = *pbEnable;

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, video_id=%ld, Port_id:%ld settings\n", __FUNCTION__, __LINE__,pstVidLayerContain->u32VidLayerId, pstInputPortContain->u32PortId);

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, En:%d\n", __FUNCTION__, __LINE__, *pbEnable);

    if(pstInputPortContain->u32PortId > E_HAL_DISP_MOPG_ID_NUM)
    {
        enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }

    return enRet;
}

void _HalDispIfSetMopgWinIsUsedForLbstart(HalDispMopgGwinId_e eMopgId,bool bAttred)
{
    int i=0;
    for(i=0;i<gbMopgInitNum;i++)//set line_buffer_start
    {
        if(gpMopgLbOdTbl[i].eMopgId == eMopgId)
        {
            gpMopgLbOdTbl[i].bIsAttred = bAttred;
        }
    }
    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, MOPG num=%d in use, set gop=%d, en=%d\n", __FUNCTION__, __LINE__, gbMopgInitNum,eMopgId,bAttred);
    _HalDispIfSetAllMopgLineBufVal();
}

void _HalDispIfSetInputPortEnable(void *pCtx, void *pCfg)
{
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    bool *pbEnable = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstVidLayerContain = (DrvDispCtxVideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;
    pbEnable = (bool *)pCfg;

    if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPG_ID)&&
       (pstInputPortContain->u32PortId < E_HAL_DISP_MOPG_ID_NUM))//mopg
    {
        //go to lb start algorithm
        _HalDispIfSetMopgWinIsUsedForLbstart((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, *pbEnable);

        HalDispMopgGwinEn((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, *pbEnable);

    }
    else if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPS_ID)&&
            (pstInputPortContain->u32PortId == E_HAL_DISP_MOPS_ID_00))//mops
    {
        HalDispMopsGwinEn(*pbEnable);
    }
    else
    {
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }

}

HalDispQueryRet_e _HalDispIfGetInfoInputPortAttr(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    HalDispInputPortAttr_t *pstHalInputPortCfg = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstVidLayerContain = (DrvDispCtxVideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;
    pstHalInputPortCfg = (HalDispInputPortAttr_t *)pCfg;

    memcpy(&pstInputPortContain->stAttr, pstHalInputPortCfg, sizeof(HalDispInputPortAttr_t));

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, video_id=%ld, Port_id:%ld settings\n", __FUNCTION__, __LINE__,pstVidLayerContain->u32VidLayerId, pstInputPortContain->u32PortId);

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Disp(%d %d %d %d) Src(%d %d)\n",
        __FUNCTION__, __LINE__,
        pstHalInputPortCfg->stDispWin.u16X, pstHalInputPortCfg->stDispWin.u16Y,
        pstHalInputPortCfg->stDispWin.u16Width, pstHalInputPortCfg->stDispWin.u16Height,
        pstHalInputPortCfg->u16SrcWidth, pstHalInputPortCfg->u16SrcHeight);

    if(pstInputPortContain->u32PortId > E_HAL_DISP_MOPG_ID_NUM)
    {
        enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }

    return enRet;
}


void _HalDispIfSetMopgLineBufOrder(HalDispMopgGwinId_e eMopgId,u16 Hstr, bool bEn)
{
    u8 i=0;
    bool bFoundOrder=0;
    bool bFoundId=0;
    HalDispMopgGwinId_e eTmpId=0;
    HalDispMopgGwinId_e eTmpId2=0;
    u16 u16TmpHstr=0;
    u16 u16TmpHstr2=0;
    //u8 u8TmpOrder=0;
    bool bTmpAttred=0;
    bool bTmpAttred2=0;

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, set MOPG_id=%d Hst=%d\n", __FUNCTION__, __LINE__, eMopgId,Hstr);

    for(i=0;i<gbMopgInitNum;i++)
    {
        if(i==(gbMopgInitNum-1))//last one case
        {
            if(!bFoundOrder)
            {
                bFoundId=1;
                bFoundOrder = 1;
                gpMopgLbOdTbl[i].eMopgId = eMopgId;
                gpMopgLbOdTbl[i].u16GwinHst = Hstr;
                //gpMopgLbOdTbl[i].bIsAttred = 1;
                gpMopgLbOdTbl[i].bIsAttred = bEn;
                break;
            }
        }
        if(!bFoundId)
        {
            if(!bFoundOrder)//id not found, order not found
            {
                if(gpMopgLbOdTbl[i].eMopgId ==eMopgId)
                {
                    bFoundId=1;
                    gpMopgLbOdTbl[i].eMopgId = gpMopgLbOdTbl[i+1].eMopgId;
                    gpMopgLbOdTbl[i].u16GwinHst = gpMopgLbOdTbl[i+1].u16GwinHst;
                    gpMopgLbOdTbl[i].bIsAttred = gpMopgLbOdTbl[i+1].bIsAttred;
                    if(gpMopgLbOdTbl[i].u16GwinHst < Hstr)
                    {
                        bFoundOrder=0;
                    }
                    else
                    {
                        bFoundOrder = 1;
                        gpMopgLbOdTbl[i].eMopgId = eMopgId;
                        gpMopgLbOdTbl[i].u16GwinHst = Hstr;
                        //gpMopgLbOdTbl[i].bIsAttred = 1;
                        gpMopgLbOdTbl[i].bIsAttred = bEn;
                    }
                }
                else
                {
                    if(gpMopgLbOdTbl[i].u16GwinHst < Hstr)
                    {
                        bFoundOrder=0;
                    }
                    else
                    {
                        bFoundOrder = 1;
                        eTmpId = gpMopgLbOdTbl[i].eMopgId;
                        u16TmpHstr = gpMopgLbOdTbl[i].u16GwinHst;
                        bTmpAttred = gpMopgLbOdTbl[i].bIsAttred;
                        gpMopgLbOdTbl[i].eMopgId = eMopgId;
                        gpMopgLbOdTbl[i].u16GwinHst = Hstr;
                        //gpMopgLbOdTbl[i].bIsAttred = 1;
                        gpMopgLbOdTbl[i].bIsAttred = bEn;
                    }
                }
            }
            else//id not found, order found
            {
                if(gpMopgLbOdTbl[i].eMopgId ==eMopgId)
                {
                    bFoundId=1;
                    gpMopgLbOdTbl[i].eMopgId = eTmpId;
                    gpMopgLbOdTbl[i].u16GwinHst = u16TmpHstr;
                    gpMopgLbOdTbl[i].bIsAttred = bTmpAttred;
                }
                else
                {
                    bFoundId=0;
                    eTmpId2 = gpMopgLbOdTbl[i].eMopgId;
                    u16TmpHstr2 = gpMopgLbOdTbl[i].u16GwinHst;
                    bTmpAttred2 = gpMopgLbOdTbl[i].bIsAttred;
                    gpMopgLbOdTbl[i].eMopgId = eTmpId;
                    gpMopgLbOdTbl[i].u16GwinHst = u16TmpHstr;
                    gpMopgLbOdTbl[i].bIsAttred = bTmpAttred;
                    eTmpId = eTmpId2;
                    u16TmpHstr = u16TmpHstr2;
                    bTmpAttred = bTmpAttred2;
                }
            }
        }
        else
        {
            if(!bFoundOrder)//id found, order not found
            {
                gpMopgLbOdTbl[i].eMopgId = gpMopgLbOdTbl[i+1].eMopgId;
                gpMopgLbOdTbl[i].u16GwinHst = gpMopgLbOdTbl[i+1].u16GwinHst;
                gpMopgLbOdTbl[i].bIsAttred = gpMopgLbOdTbl[i+1].bIsAttred;
                if(gpMopgLbOdTbl[i].u16GwinHst < Hstr)
                {
                    bFoundOrder=0;
                }
                else
                {
                    bFoundOrder = 1;
                    gpMopgLbOdTbl[i].eMopgId = eMopgId;
                    gpMopgLbOdTbl[i].u16GwinHst = Hstr;
                    //gpMopgLbOdTbl[i].bIsAttred = 1;
                    gpMopgLbOdTbl[i].bIsAttred = bEn;
                }
            }
            else//id found, order found
            {
                break;
            }
        }
    }

    u16TmpHstr=0;
    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, MOPG num=%d in use\n", __FUNCTION__, __LINE__, gbMopgInitNum);

    _HalDispIfSetAllMopgLineBufVal();


}

void _HalDispIfSetInputPortAttr(void *pCtx, void *pCfg)
{
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    HalDispInputPortAttr_t *pstHalInputPortCfg = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain;
    u16 u16Hratio = 0x1000;
    u16 u16Vratio = 0x1000;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstVidLayerContain = (DrvDispCtxVideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;
    pstHalInputPortCfg = (HalDispInputPortAttr_t *)pCfg;

    if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPG_ID)&&
       (pstInputPortContain->u32PortId < E_HAL_DISP_MOPG_ID_NUM))//mopg
    {
        HalDispMopgSetSourceParam((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, pstHalInputPortCfg->u16SrcWidth,
                                                                              pstHalInputPortCfg->u16SrcHeight);
        HalDispMopgSetGwinParam((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, pstHalInputPortCfg->stDispWin.u16X,
                                                                            pstHalInputPortCfg->stDispWin.u16Y,
                                                                            pstHalInputPortCfg->stDispWin.u16Width,
                                                                            pstHalInputPortCfg->stDispWin.u16Height);
        _HalDispIfSetMopgLineBufOrder((HalDispMopgGwinId_e)pstInputPortContain->u32PortId,pstHalInputPortCfg->stDispWin.u16X
                                                                                         ,pstInputPortContain->bEnable);
        if(pstHalInputPortCfg->stDispWin.u16Width > pstHalInputPortCfg->u16SrcWidth)
        {
            u16Hratio = ((pstHalInputPortCfg->u16SrcWidth*4096)/pstHalInputPortCfg->stDispWin.u16Width);
            HalDispMopgSetHScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Hratio);
        }
        else
        {
            u16Hratio = 0x1000;
            HalDispMopgSetHScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Hratio);
        }
        if(pstHalInputPortCfg->stDispWin.u16Height > pstHalInputPortCfg->u16SrcHeight)
        {
            u16Vratio = ((pstHalInputPortCfg->u16SrcHeight*4096)/pstHalInputPortCfg->stDispWin.u16Height);
            HalDispMopgSetVScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Vratio);
        }
        else
        {
            u16Vratio = 0x1000;
            HalDispMopgSetVScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Vratio);
        }
    }
    else if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPS_ID)&&
            (pstInputPortContain->u32PortId == E_HAL_DISP_MOPS_ID_00))//mops
    {
        HalDispMopsSetSourceParam(pstHalInputPortCfg->u16SrcWidth,
                                  pstHalInputPortCfg->u16SrcHeight);
        HalDispMopsSetGwinParam(pstHalInputPortCfg->stDispWin.u16X,
                                pstHalInputPortCfg->stDispWin.u16Y,
                                pstHalInputPortCfg->stDispWin.u16Width,
                                pstHalInputPortCfg->stDispWin.u16Height);
        if(pstHalInputPortCfg->stDispWin.u16Width > pstHalInputPortCfg->u16SrcWidth)
        {
            u16Hratio = ((pstHalInputPortCfg->u16SrcWidth*4096)/pstHalInputPortCfg->stDispWin.u16Width);
            HalDispMopsSetHScaleFac(u16Hratio);
        }
        else
        {
            u16Hratio = 0x1000;
            HalDispMopsSetHScaleFac(u16Hratio);
        }
        if(pstHalInputPortCfg->stDispWin.u16Height > pstHalInputPortCfg->u16SrcHeight)
        {
            u16Vratio = ((pstHalInputPortCfg->u16SrcHeight*4096)/pstHalInputPortCfg->stDispWin.u16Height);
            HalDispMopsSetVScaleFac(u16Vratio);
        }
        else
        {
            u16Vratio = 0x1000;
            HalDispMopsSetVScaleFac(u16Vratio);
        }
    }
    else
    {
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }
}

HalDispQueryRet_e _HalDispIfGetInfoInputPortShow(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetInputPortShow(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoInputPortHide(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetInputPortHide(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoInputPortBegin(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetInputPortBegin(void *pCtx, void *pCfg)
{

}

HalDispQueryRet_e _HalDispIfGetInfoInputPortEnd(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
    return enRet;
}

void _HalDispIfSetInputPortEnd(void *pCtx, void *pCfg)
{

}

void _HalDispIfSetMopRotImiAddrManage(bool bMopRotId, u32 u32Ysize, u32 u32Csize)//Id=0: Rot0; Id=1: Rot1
{
    u32 u32TmpSize=0;

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, rot_id=%d, Ysize=0x%lx, Csize=0x%lx\n",
            __FUNCTION__, __LINE__,
            bMopRotId,u32Ysize,u32Csize);
    //imi property: Rot0_Y -> Rot0_C -> Rot1_Y -> Rot1_C
    if(bMopRotId==0)//Rot0
    {
        u32TmpSize = u32Rot1ImiCaddr - u32Rot1ImiYaddr;
        u32Rot0ImiCaddr = u32Rot0ImiYaddr + u32Ysize;
        u32Rot1ImiYaddr = u32Rot0ImiCaddr + u32Csize;
        u32Rot1ImiCaddr = u32Rot1ImiYaddr + u32TmpSize;
    }
    else//Rot1
    {
        u32Rot1ImiCaddr = u32Rot1ImiYaddr + u32Ysize;
    }
}

HalDispQueryRet_e _HalDispIfGetInfoInputPortFlip(void *pCtx, void *pCfg)
{
    u16 i;
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    HalDispVideoFrameData_t *pstFramedata = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstFramedata = (HalDispVideoFrameData_t *)pCfg;

    memcpy(&pstInputPortContain->stFrameData, pstFramedata, sizeof(HalDispVideoFrameData_t));

    for(i=0; i<3; i++)
     {
         if(pstInputPortContain->stFrameData.au64PhyAddr[i]&0xF)//physical address should be 16 align
         {
             enRet = E_HAL_DISP_QUERY_RET_CFGERR;
             DISP_ERR("%s %d, PHY_ADDR should be 16 align, addr[%d](%08llx)\n", __FUNCTION__, __LINE__,i, pstInputPortContain->stFrameData.au64PhyAddr[i]);
         }

         if(pstInputPortContain->stFrameData.au32Stride[i]&0xF)//stride should be 16 align
         {
             enRet = E_HAL_DISP_QUERY_RET_CFGERR;
             DISP_ERR("%s %d, Stride should be 16 align, addr[%d](%ld)\n", __FUNCTION__, __LINE__,i, pstInputPortContain->stFrameData.au32Stride[i]);
         }
     }

    if(pstFramedata->ePixelFormat == E_HAL_DISP_PIXEL_FRAME_YUV422_YUYV ||
       pstFramedata->ePixelFormat == E_HAL_DISP_PIXEL_FRAME_YUV_SEMIPLANAR_422)
    {
        for(i=0; i<3; i++)
        {
            pstInputPortContain->stFrameData.au32Stride[i] /= 2;

           if(i==1)
           {
               pstInputPortContain->stFrameData.au64PhyAddr[1] =
                pstInputPortContain->stFrameData.au64PhyAddr[0] + 0x10;
           }
        }
    }

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Fmt:%s, PhyAddr(%08llx, %08llx, %08llx), Stride(%ld %ld %ld)\n",
        __FUNCTION__, __LINE__,
        PARSING_HAL_PIXEL_FMT(pstInputPortContain->stFrameData.ePixelFormat),
        pstInputPortContain->stFrameData.au64PhyAddr[0], pstInputPortContain->stFrameData.au64PhyAddr[1], pstInputPortContain->stFrameData.au64PhyAddr[2],
        pstInputPortContain->stFrameData.au32Stride[0], pstInputPortContain->stFrameData.au32Stride[1], pstInputPortContain->stFrameData.au32Stride[2]);

    if(pstInputPortContain->u32PortId > E_HAL_DISP_MOPG_ID_NUM)
    {
        enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }

    return enRet;
}

void _HalDispIfSetInputPortFlip(void *pCtx, void *pCfg)
{
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain;
    u64 u64Yaddr=0;
    u64 u64Caddr=0;
    u8 u8AddrOffset=0;
    u16 u16tempRotInW = 0;
    u16 u16RotDummyPix = 0;
    u16 u16tempRotInH = 0;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstVidLayerContain = (DrvDispCtxVideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;

    //Y Address, after counting crop, Yaddr = Yaddr + offset_y*stride + offset_x
    u64Yaddr = pstInputPortContain->stFrameData.au64PhyAddr[0] +
               pstInputPortContain->stCrop.u16Y * pstInputPortContain->stFrameData.au32Stride[0]+
               pstInputPortContain->stCrop.u16X;
    //C Address, after counting crop, Caddr = Caddr + offset_y/2*stride + offset_x
    u64Caddr = pstInputPortContain->stFrameData.au64PhyAddr[1] +
               (pstInputPortContain->stCrop.u16Y/2) * pstInputPortContain->stFrameData.au32Stride[0]+
               pstInputPortContain->stCrop.u16X;

    u8AddrOffset = (u8)(pstInputPortContain->stCrop.u16X & 0xF);

    if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPG_ID)&&
       (pstInputPortContain->u32PortId < E_HAL_DISP_MOPG_ID_NUM))//mopg
    {
        if(pstInputPortContain->stRot.enRotate==E_HAL_DISP_ROTATE_90)
        {
            u16tempRotInW = pstInputPortContain->stAttr.u16SrcWidth;
            u16RotDummyPix = (u16tempRotInW & 0xF);
            if(u16RotDummyPix)
            {
                u16tempRotInW = ((u16tempRotInW>>4) + 1)<<4;
            }
            //rot_r source_width=source_width align 16
            HalDispMopRot0SourceWidth(u16tempRotInW);
            //calculate width in imi (mop pitch)(~source height)
            u16tempRotInH = pstInputPortContain->stAttr.u16SrcHeight;
            //pitch is the same as source height, but 256 align
            if((u16tempRotInH & 0xFF))
            {
                u16tempRotInH = ((u16tempRotInH>>8) + 1)<<8;
            }
            _HalDispIfSetMopRotImiAddrManage(0, (u16tempRotInH*16), (u16tempRotInH*8));

            //set rot_r read address
            HalDispMopRot0SetReadYAddr(u64Yaddr);
            HalDispMopRot0SetReadCAddr(u64Caddr);
            //set rot_r write imi address
            HalDispMopRot0SetWriteYAddr(u32Rot0ImiYaddr);
            HalDispMopRot0SetWriteCAddr(u32Rot0ImiCaddr);
            //set MOPG read imi address
            HalDispMopgSetYAddr((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u32Rot0ImiYaddr);
            HalDispMopgSetCAddr((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u32Rot0ImiCaddr);
            //pitch is set in rotate/crop config
            //crop offset 16 change to dummy pixel in imi
            HalDispMopRot0SetPixDummy(u8AddrOffset);

            //reset Rot1 imi address because the change in management function
            //set rot_r write imi address
            HalDispMopRot1SetWriteYAddr((u32)u32Rot1ImiYaddr);
            HalDispMopRot1SetWriteCAddr((u32)u32Rot1ImiCaddr);
            //set MOPG read imi address
            HalDispMopsSetYAddr((u32)u32Rot1ImiYaddr);
            HalDispMopsSetCAddr((u32)u32Rot1ImiCaddr);

        }
        else if(pstInputPortContain->stRot.enRotate==E_HAL_DISP_ROTATE_270)
        {
            u16tempRotInW = pstInputPortContain->stAttr.u16SrcWidth;
            u16RotDummyPix = (u16tempRotInW & 0xF);
            if(u16RotDummyPix)
            {
                u16tempRotInW = ((u16tempRotInW>>4) + 1)<<4;
            }
            //rot_r source_width=source_width align 16
            HalDispMopRot0SourceWidth(u16tempRotInW);
            //calculate width in imi (mop pitch)(~source height)
            u16tempRotInH = pstInputPortContain->stAttr.u16SrcHeight;
            //pitch is the same as source height, but 256 align
            if((u16tempRotInH & 0xFF))
            {
                u16tempRotInH = ((u16tempRotInH>>8) + 1)<<8;
            }
            _HalDispIfSetMopRotImiAddrManage(0, (u16tempRotInH*16), (u16tempRotInH*8));

            //set rot_r read address
            HalDispMopRot0SetReadYAddr(u64Yaddr);
            HalDispMopRot0SetReadCAddr(u64Caddr);
            //set rot_r write imi address
            HalDispMopRot0SetWriteYAddr((u32)u32Rot0ImiYaddr);
            HalDispMopRot0SetWriteCAddr((u32)u32Rot0ImiCaddr);
            //set MOPG read imi address
            HalDispMopgSetYAddr((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, (u32)u32Rot0ImiYaddr);
            HalDispMopgSetCAddr((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, (u32)u32Rot0ImiCaddr);
            //pitch is set in rotate/crop config
            //crop offset 16 change to cooperated with dummy pixel in imi
            u8AddrOffset = u16tempRotInW-u8AddrOffset-pstInputPortContain->stAttr.u16SrcWidth;
            HalDispMopRot0SetPixDummy(u8AddrOffset);

            //reset Rot1 imi address because the change in management function
            //set rot_r write imi address
            HalDispMopRot1SetWriteYAddr((u32)u32Rot1ImiYaddr);
            HalDispMopRot1SetWriteCAddr((u32)u32Rot1ImiCaddr);
            //set MOPG read imi address
            HalDispMopsSetYAddr((u32)u32Rot1ImiYaddr);
            HalDispMopsSetCAddr((u32)u32Rot1ImiCaddr);
        }
        else
        {
            HalDispMopgSetYAddr((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u64Yaddr);
            HalDispMopgSetCAddr((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u64Caddr);
            HalDispMopgSetPitch((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, pstInputPortContain->stFrameData.au32Stride[0]);
            HalDispMopgSetAddr16Offset((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u8AddrOffset);
        }
    }
    else if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPS_ID)&&
            (pstInputPortContain->u32PortId == E_HAL_DISP_MOPS_ID_00))//mops
    {
        if(pstInputPortContain->stRot.enRotate==E_HAL_DISP_ROTATE_90)
        {
            u16tempRotInW = pstInputPortContain->stAttr.u16SrcWidth;
            u16RotDummyPix = (u16tempRotInW & 0xF);
            if(u16RotDummyPix)
            {
                u16tempRotInW = ((u16tempRotInW>>4) + 1)<<4;
            }
            //rot_r source_width=source_width align 16
            HalDispMopRot1SourceWidth(u16tempRotInW);
            //calculate width in imi (mop pitch)(~source height)
            u16tempRotInH = pstInputPortContain->stAttr.u16SrcHeight;
            //pitch is the same as source height, but 256 align
            if((u16tempRotInH & 0xFF))
            {
                u16tempRotInH = ((u16tempRotInH>>8) + 1)<<8;
            }
            _HalDispIfSetMopRotImiAddrManage(1, (u16tempRotInH*16), (u16tempRotInH*8));
            //set rot_r read address
            HalDispMopRot1SetReadYAddr(u64Yaddr);
            HalDispMopRot1SetReadCAddr(u64Caddr);
            //set rot_r write imi address
            HalDispMopRot1SetWriteYAddr((u32)u32Rot1ImiYaddr);
            HalDispMopRot1SetWriteCAddr((u32)u32Rot1ImiCaddr);
            //set MOPG read imi address
            HalDispMopsSetYAddr((u32)u32Rot1ImiYaddr);
            HalDispMopsSetCAddr((u32)u32Rot1ImiCaddr);
            //pitch is set in rotate/crop config
            //crop offset 16 change to dummy pixel in imi
            HalDispMopRot1SetPixDummy(u8AddrOffset);
        }
        else if(pstInputPortContain->stRot.enRotate==E_HAL_DISP_ROTATE_270)
        {
            u16tempRotInW = pstInputPortContain->stAttr.u16SrcWidth;
            u16RotDummyPix = (u16tempRotInW & 0xF);
            if(u16RotDummyPix)
            {
                u16tempRotInW = ((u16tempRotInW>>4) + 1)<<4;
            }
            //rot_r source_width=source_width align 16
            HalDispMopRot1SourceWidth(u16tempRotInW);
            //calculate width in imi (mop pitch)(~source height)
            u16tempRotInH = pstInputPortContain->stAttr.u16SrcHeight;
            //pitch is the same as source height, but 256 align
            if((u16tempRotInH & 0xFF))
            {
                u16tempRotInH = ((u16tempRotInH>>8) + 1)<<8;
            }
            _HalDispIfSetMopRotImiAddrManage(1, (u16tempRotInH*16), (u16tempRotInH*8));
            //set rot_r read address
            HalDispMopRot1SetReadYAddr(u64Yaddr);
            HalDispMopRot1SetReadCAddr(u64Caddr);
            //set rot_r write imi address
            HalDispMopRot1SetWriteYAddr((u32)u32Rot1ImiYaddr);
            HalDispMopRot1SetWriteCAddr((u32)u32Rot1ImiCaddr);
            //set MOPG read imi address
            HalDispMopsSetYAddr((u32)u32Rot1ImiYaddr);
            HalDispMopsSetCAddr((u32)u32Rot1ImiCaddr);
            //pitch is set in rotate/crop config
            //crop offset 16 change to cooperated with dummy pixel in imi
            u8AddrOffset = u16tempRotInW-u8AddrOffset-pstInputPortContain->stAttr.u16SrcWidth;
            HalDispMopRot1SetPixDummy(u8AddrOffset);
        }
        else
        {
            HalDispMopsSetYAddr(u64Yaddr);
            HalDispMopsSetCAddr(u64Caddr);
            HalDispMopsSetPitch(pstInputPortContain->stFrameData.au32Stride[0]);
            HalDispMopsSetAddr16Offset(u8AddrOffset);
        }
    }
    else
    {
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }
}


HalDispQueryRet_e _HalDispIfGetInfoInputPortRotate(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    HalDispInputPortRotate_t *pstHalInputPortCfg = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain;
    s32 imi_ret;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstVidLayerContain = (DrvDispCtxVideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;
    pstHalInputPortCfg = (HalDispInputPortRotate_t *)pCfg;

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, video_id=%ld, Port_id:%ld settings\n", __FUNCTION__, __LINE__,pstVidLayerContain->u32VidLayerId, pstInputPortContain->u32PortId);

    if(!bImiInit)
    {
        imi_ret = DrvDispOsImiHeapAlloc(0xC000, &u32MopRotImiTotalAddr);
        if(imi_ret)
        {
            DISP_ERR("%s %d, imi_heap init Rot 0x4000 fail, err=%ld\n", __FUNCTION__, __LINE__, imi_ret);
        }
        u32Rot0ImiYaddr=(u32)u32MopRotImiTotalAddr;
        u32Rot0ImiCaddr=(u32)u32MopRotImiTotalAddr;
        u32Rot1ImiYaddr=(u32)u32MopRotImiTotalAddr;
        u32Rot1ImiCaddr=(u32)u32MopRotImiTotalAddr;
        bImiInit=1;
    }

    if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPG_ID)&&
       (pstInputPortContain->u32PortId < E_HAL_DISP_MOPG_ID_NUM))//mopg
    {
        if((_gstMopgRotRec.bRotEn)&&(pstInputPortContain->u32PortId!=_gstMopgRotRec.eMopgId))
        {
            DISP_ERR("%s error, MOPG rotate only support one window\n", __FUNCTION__);
            DISP_ERR("%s %d, MOPG rotate is used by mopg_id=%d\n", __FUNCTION__, __LINE__, _gstMopgRotRec.eMopgId);
            return E_HAL_DISP_QUERY_RET_CFGERR;
        }
        else
        {
            if(pstHalInputPortCfg->enRotate==E_HAL_DISP_ROTATE_NONE)
            {
                _gstMopgRotRec.bRotEn = 0;
            }
            else if((pstHalInputPortCfg->enRotate==E_HAL_DISP_ROTATE_90)||
                    (pstHalInputPortCfg->enRotate==E_HAL_DISP_ROTATE_270))
            {
                _gstMopgRotRec.bRotEn = 1;
            }
            else
            {
                _gstMopgRotRec.bRotEn = 0;
                DISP_ERR("%s %d, MOPG %s is not supported\n", __FUNCTION__, __LINE__, PARSING_MOP_ROT_ANGLE(pstHalInputPortCfg->enRotate));
                return E_HAL_DISP_QUERY_RET_NOTSUPPORT;
            }
            _gstMopgRotRec.eMopgId = (HalDispMopgGwinId_e)pstInputPortContain->u32PortId;
            _gstMopgRotRec.stRotAng = pstHalInputPortCfg->enRotate;
        }
    }
    else if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPS_ID)&&
            (pstInputPortContain->u32PortId == E_HAL_DISP_MOPS_ID_00))//mops
    {
        if((pstHalInputPortCfg->enRotate!=E_HAL_DISP_ROTATE_NONE)&&
           (pstHalInputPortCfg->enRotate!=E_HAL_DISP_ROTATE_90)&&
           (pstHalInputPortCfg->enRotate!=E_HAL_DISP_ROTATE_270))
        {
            DISP_ERR("%s %d, MOPS %s is not supported\n", __FUNCTION__, __LINE__, PARSING_MOP_ROT_ANGLE(pstHalInputPortCfg->enRotate));
            return E_HAL_DISP_QUERY_RET_NOTSUPPORT;
        }
    }
    else
    {
        enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }

    memcpy(&pstInputPortContain->stRot, pstHalInputPortCfg, sizeof(HalDispInputPortRotate_t));

    return enRet;
}

void _HalDispIfSetInputPortRotate(void *pCtx, void *pCfg)
{
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    HalDispInputPortRotate_t *pstHalInputPortCfg = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain;
    u16 u16Hratio = 0x1000;
    u16 u16Vratio = 0x1000;
    u16 u16tempRotInH = 0;
    u16 u16RotDummyPix = 0;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstVidLayerContain = (DrvDispCtxVideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;
    pstHalInputPortCfg = (HalDispInputPortRotate_t *)pCfg;

    if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPG_ID)&&
   (pstInputPortContain->u32PortId < E_HAL_DISP_MOPG_ID_NUM))//mopg
    {
        if(pstHalInputPortCfg->enRotate==E_HAL_DISP_ROTATE_NONE)
        {
            HalDispMopRot0En(FALSE);
            HalDispMopRot0SetRotateMode(pstHalInputPortCfg->enRotate);
        }
        else if(pstHalInputPortCfg->enRotate==E_HAL_DISP_ROTATE_90)
        {
            HalDispMopRot0En(TRUE);
            HalDispMopRot0SetRotateMode(pstHalInputPortCfg->enRotate);
            //Rot_r input_width=pitch, height=source_height align 16
            u16tempRotInH = pstInputPortContain->stAttr.u16SrcHeight;
            u16RotDummyPix = (u16tempRotInH & 0xF);
            if(u16RotDummyPix)
            {
                u16tempRotInH = ((u16tempRotInH>>4) + 1)<<4;
            }
            HalDispMopRot0SourceHeight(u16tempRotInH);
            //pitch is the same as source width, but 256 align
            if((u16tempRotInH & 0xFF))
            {
                u16tempRotInH = ((u16tempRotInH>>8) + 1)<<8;
            }
            HalDispMopgSetPitch((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16tempRotInH);

            HalDispMopgSetAddr16Offset((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16RotDummyPix);

            //rotate, exchange source W&H
            HalDispMopgSetSourceParam((HalDispMopgGwinId_e)pstInputPortContain->u32PortId,
                                       pstInputPortContain->stAttr.u16SrcHeight,
                                       pstInputPortContain->stAttr.u16SrcWidth);
            if(pstInputPortContain->stAttr.stDispWin.u16Width > pstInputPortContain->stAttr.u16SrcHeight)
            {
                u16Hratio = ((pstInputPortContain->stAttr.u16SrcHeight*4096)/pstInputPortContain->stAttr.stDispWin.u16Width);
                HalDispMopgSetHScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Hratio);
            }
            else
            {
                u16Hratio = 0x1000;
                HalDispMopgSetHScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Hratio);
            }
            if(pstInputPortContain->stAttr.stDispWin.u16Height > pstInputPortContain->stAttr.u16SrcWidth)
            {
                u16Vratio = ((pstInputPortContain->stAttr.u16SrcWidth*4096)/pstInputPortContain->stAttr.stDispWin.u16Height);
                HalDispMopgSetVScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Vratio);
            }
            else
            {
                u16Vratio = 0x1000;
                HalDispMopgSetVScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Vratio);
            }
        }
        else if(pstHalInputPortCfg->enRotate==E_HAL_DISP_ROTATE_270)
        {
            HalDispMopRot0En(TRUE);
            HalDispMopRot0SetRotateMode(pstHalInputPortCfg->enRotate);
            u16tempRotInH = pstInputPortContain->stAttr.u16SrcHeight;
            u16RotDummyPix = (u16tempRotInH & 0xF);
            if(u16RotDummyPix)
            {
                u16tempRotInH = ((u16tempRotInH>>4) + 1)<<4;
            }
            HalDispMopRot0SourceHeight(u16tempRotInH);
            //pitch is the same as source width, but 256 align
            if((u16tempRotInH & 0xFF))
            {
                u16tempRotInH = ((u16tempRotInH>>8) + 1)<<8;
            }
            HalDispMopgSetPitch((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16tempRotInH);

            //rotate, exchange source W&H
            HalDispMopgSetSourceParam((HalDispMopgGwinId_e)pstInputPortContain->u32PortId,
                                       pstInputPortContain->stAttr.u16SrcHeight,
                                       pstInputPortContain->stAttr.u16SrcWidth);
            if(pstInputPortContain->stAttr.stDispWin.u16Width > pstInputPortContain->stAttr.u16SrcHeight)
            {
                u16Hratio = ((pstInputPortContain->stAttr.u16SrcHeight*4096)/pstInputPortContain->stAttr.stDispWin.u16Width);
                HalDispMopgSetHScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Hratio);
            }
            else
            {
                u16Hratio = 0x1000;
                HalDispMopgSetHScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Hratio);
            }
            if(pstInputPortContain->stAttr.stDispWin.u16Height > pstInputPortContain->stAttr.u16SrcWidth)
            {
                u16Vratio = ((pstInputPortContain->stAttr.u16SrcWidth*4096)/pstInputPortContain->stAttr.stDispWin.u16Height);
                HalDispMopgSetVScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Vratio);
            }
            else
            {
                u16Vratio = 0x1000;
                HalDispMopgSetVScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Vratio);
            }

            //reset to 0, 270 no need
            HalDispMopgSetAddr16Offset((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, 0);
        }
        else
        {
            HalDispMopRot0En(FALSE);
            HalDispMopRot0SetRotateMode(E_HAL_DISP_ROTATE_NONE);
            DISP_ERR("%s %d, MOPG %s is not supported\n", __FUNCTION__, __LINE__, PARSING_MOP_ROT_ANGLE(pstHalInputPortCfg->enRotate));
        }

    }
    else if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPS_ID)&&
            (pstInputPortContain->u32PortId == E_HAL_DISP_MOPS_ID_00))//mops
    {
        if(pstHalInputPortCfg->enRotate==E_HAL_DISP_ROTATE_NONE)
        {
            HalDispMopRot1En(FALSE);
            HalDispMopRot1SetRotateMode(pstHalInputPortCfg->enRotate);
        }
        else if(pstHalInputPortCfg->enRotate==E_HAL_DISP_ROTATE_90)
        {
            HalDispMopRot1En(TRUE);
            HalDispMopRot1SetRotateMode(pstHalInputPortCfg->enRotate);
            //Rot_r input_width=pitch, height=source_height align 16
            u16tempRotInH = pstInputPortContain->stAttr.u16SrcHeight;
            u16RotDummyPix = (u16tempRotInH & 0xF);
            if(u16RotDummyPix)
            {
                u16tempRotInH = ((u16tempRotInH>>4) + 1)<<4;
            }
            HalDispMopRot1SourceHeight(u16tempRotInH);
            //pitch is the same as source width, but 256 align
            if((u16tempRotInH & 0xFF))
            {
                u16tempRotInH = ((u16tempRotInH>>8) + 1)<<8;
            }
            HalDispMopsSetPitch(u16tempRotInH);

            HalDispMopsSetAddr16Offset(u16RotDummyPix);

            //rotate, exchange source W&H
            HalDispMopsSetSourceParam(pstInputPortContain->stAttr.u16SrcHeight,
                                      pstInputPortContain->stAttr.u16SrcWidth);
            if(pstInputPortContain->stAttr.stDispWin.u16Width > pstInputPortContain->stAttr.u16SrcHeight)
            {
                u16Hratio = ((pstInputPortContain->stAttr.u16SrcHeight*4096)/pstInputPortContain->stAttr.stDispWin.u16Width);
                HalDispMopsSetHScaleFac(u16Hratio);
            }
            else
            {
                u16Hratio = 0x1000;
                HalDispMopsSetHScaleFac(u16Hratio);
            }
            if(pstInputPortContain->stAttr.stDispWin.u16Height > pstInputPortContain->stAttr.u16SrcWidth)
            {
                u16Vratio = ((pstInputPortContain->stAttr.u16SrcWidth*4096)/pstInputPortContain->stAttr.stDispWin.u16Height);
                HalDispMopsSetVScaleFac(u16Vratio);
            }
            else
            {
                u16Vratio = 0x1000;
                HalDispMopsSetVScaleFac(u16Vratio);
            }
        }
        else if(pstHalInputPortCfg->enRotate==E_HAL_DISP_ROTATE_270)
        {
            HalDispMopRot1En(TRUE);
            HalDispMopRot1SetRotateMode(pstHalInputPortCfg->enRotate);
            //Rot_r input_width=pitch, height=source_height align 16
            u16tempRotInH = pstInputPortContain->stAttr.u16SrcHeight;
            u16RotDummyPix = (u16tempRotInH & 0xF);
            if(u16RotDummyPix)
            {
                u16tempRotInH = ((u16tempRotInH>>4) + 1)<<4;
            }
            HalDispMopRot1SourceHeight(u16tempRotInH);
            //pitch is the same as source width, but 256 align
            if((u16tempRotInH & 0xFF))
            {
                u16tempRotInH = ((u16tempRotInH>>8) + 1)<<8;
            }
            HalDispMopsSetPitch(u16tempRotInH);

            //rotate, exchange source W&H
            HalDispMopsSetSourceParam(pstInputPortContain->stAttr.u16SrcHeight,
                                      pstInputPortContain->stAttr.u16SrcWidth);
            if(pstInputPortContain->stAttr.stDispWin.u16Width > pstInputPortContain->stAttr.u16SrcHeight)
            {
                u16Hratio = ((pstInputPortContain->stAttr.u16SrcHeight*4096)/pstInputPortContain->stAttr.stDispWin.u16Width);
                HalDispMopsSetHScaleFac(u16Hratio);
            }
            else
            {
                u16Hratio = 0x1000;
                HalDispMopsSetHScaleFac(u16Hratio);
            }
            if(pstInputPortContain->stAttr.stDispWin.u16Height > pstInputPortContain->stAttr.u16SrcWidth)
            {
                u16Vratio = ((pstInputPortContain->stAttr.u16SrcWidth*4096)/pstInputPortContain->stAttr.stDispWin.u16Height);
                HalDispMopsSetVScaleFac(u16Vratio);
            }
            else
            {
                u16Vratio = 0x1000;
                HalDispMopsSetVScaleFac(u16Vratio);
            }

            //reset to 0, 270 no need
            HalDispMopsSetAddr16Offset(0);
        }
        else
        {
            HalDispMopRot1En(FALSE);
            HalDispMopRot1SetRotateMode(E_HAL_DISP_ROTATE_NONE);
            DISP_ERR("%s %d, MOPG %s is not supported\n", __FUNCTION__, __LINE__, PARSING_MOP_ROT_ANGLE(pstHalInputPortCfg->enRotate));
        }
    }
    else
    {
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }

}

HalDispQueryRet_e _HalDispIfGetInfoInputPortCrop(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    HalDispVidWinRect_t *pstHalInputPortCfg = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstHalInputPortCfg = (HalDispVidWinRect_t *)pCfg;

    if((pstHalInputPortCfg->u16X&0x1)||
       (pstHalInputPortCfg->u16Y&0x1)||
       (pstHalInputPortCfg->u16Width&0x1)||
       (pstHalInputPortCfg->u16Height&0x1))//crop parameters should be all 2 align
    {
        enRet = E_HAL_DISP_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, crop parameters should be 2 align\n", __FUNCTION__, __LINE__);
        DISP_ERR("%s %d, Crop(%d %d %d %d) not support\n",
        __FUNCTION__, __LINE__,
        pstHalInputPortCfg->u16X, pstHalInputPortCfg->u16Y,
        pstHalInputPortCfg->u16Width, pstHalInputPortCfg->u16Height);
    }

    memcpy(&pstInputPortContain->stCrop, pstHalInputPortCfg, sizeof(HalDispVidWinRect_t));

    pstInputPortContain->stAttr.u16SrcWidth = pstHalInputPortCfg->u16Width;
    pstInputPortContain->stAttr.u16SrcHeight = pstHalInputPortCfg->u16Height;

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Crop(%d %d %d %d)\n",
        __FUNCTION__, __LINE__,
        pstHalInputPortCfg->u16X, pstHalInputPortCfg->u16Y,
        pstHalInputPortCfg->u16Width, pstHalInputPortCfg->u16Height);

    if(pstInputPortContain->u32PortId > E_HAL_DISP_MOPG_ID_NUM)
    {
        enRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }

    return enRet;
}
void _HalDispIfSetInputPortCrop(void *pCtx, void *pCfg)
{
    DrvDispCtxConfig_t *pstDispCtxCfg = NULL;
    HalDispVidWinRect_t *pstHalInputPortCfg = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain;
    u16 u16Hratio = 0x1000;
    u16 u16Vratio = 0x1000;
    u16 u16tempRotInH = 0;
    u16 u16RotDummyPix = 0;

    pstDispCtxCfg = (DrvDispCtxConfig_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32Idx];
    pstVidLayerContain = (DrvDispCtxVideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;
    pstHalInputPortCfg = (HalDispVidWinRect_t *)pCfg;

    if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPG_ID)&&
       (pstInputPortContain->u32PortId < E_HAL_DISP_MOPG_ID_NUM))//mopg
    {
        if((pstInputPortContain->stRot.enRotate==E_HAL_DISP_ROTATE_90)||
           (pstInputPortContain->stRot.enRotate==E_HAL_DISP_ROTATE_270))//rotate case
        {
            //rotate, exchange source W&H
            HalDispMopgSetSourceParam((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, pstHalInputPortCfg->u16Height,
                                                                                  pstHalInputPortCfg->u16Width);
            if(pstInputPortContain->stAttr.stDispWin.u16Width > pstHalInputPortCfg->u16Height)
            {
                u16Hratio = ((pstHalInputPortCfg->u16Height*4096)/pstInputPortContain->stAttr.stDispWin.u16Width);
                HalDispMopgSetHScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Hratio);
            }
            else
            {
                u16Hratio = 0x1000;
                HalDispMopgSetHScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Hratio);
            }
            if(pstInputPortContain->stAttr.stDispWin.u16Height > pstHalInputPortCfg->u16Width)
            {
                u16Vratio = ((pstHalInputPortCfg->u16Width*4096)/pstInputPortContain->stAttr.stDispWin.u16Height);
                HalDispMopgSetVScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Vratio);
            }
            else
            {
                u16Vratio = 0x1000;
                HalDispMopgSetVScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Vratio);
            }

            //Rot_r input_width=pitch, height=source_height align 16
            u16tempRotInH = pstInputPortContain->stAttr.u16SrcHeight;
            u16RotDummyPix = (u16tempRotInH & 0xF);
            if(u16RotDummyPix)
            {
                u16tempRotInH = ((u16tempRotInH>>4) + 1)<<4;
            }
            HalDispMopRot0SourceHeight(u16tempRotInH);
            //pitch is the same as source width, but 256 align
            if((u16tempRotInH & 0xFF))
            {
                u16tempRotInH = ((u16tempRotInH>>8) + 1)<<8;
            }
            HalDispMopgSetPitch((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16tempRotInH);

            if(pstInputPortContain->stRot.enRotate==E_HAL_DISP_ROTATE_90)
            {
                HalDispMopgSetAddr16Offset((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16RotDummyPix);
            }
            else
            {
                //reset to 0, 270 no need
                HalDispMopgSetAddr16Offset((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, 0);
            }
        }
        else//not rotate case
        {
            HalDispMopgSetSourceParam((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, pstHalInputPortCfg->u16Width,
                                                                                  pstHalInputPortCfg->u16Height);
            if(pstInputPortContain->stAttr.stDispWin.u16Width > pstHalInputPortCfg->u16Width)
            {
                u16Hratio = ((pstHalInputPortCfg->u16Width*4096)/pstInputPortContain->stAttr.stDispWin.u16Width);
                HalDispMopgSetHScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Hratio);
            }
            else
            {
                u16Hratio = 0x1000;
                HalDispMopgSetHScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Hratio);
            }
            if(pstInputPortContain->stAttr.stDispWin.u16Height > pstHalInputPortCfg->u16Height)
            {
                u16Vratio = ((pstHalInputPortCfg->u16Height*4096)/pstInputPortContain->stAttr.stDispWin.u16Height);
                HalDispMopgSetVScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Vratio);
            }
            else
            {
                u16Vratio = 0x1000;
                HalDispMopgSetVScaleFac((HalDispMopgGwinId_e)pstInputPortContain->u32PortId, u16Vratio);
            }
        }
    }
    else if((pstVidLayerContain->u32VidLayerId==E_HAL_DISP_LAYER_MOPS_ID)&&
            (pstInputPortContain->u32PortId == E_HAL_DISP_MOPS_ID_00))//mops
    {
        if((pstInputPortContain->stRot.enRotate==E_HAL_DISP_ROTATE_90)||
           (pstInputPortContain->stRot.enRotate==E_HAL_DISP_ROTATE_270))//rotate case
        {
            //rotate, exchange source W&H
            HalDispMopsSetSourceParam(pstHalInputPortCfg->u16Height,
                                      pstHalInputPortCfg->u16Width);
            if(pstInputPortContain->stAttr.stDispWin.u16Width > pstHalInputPortCfg->u16Height)
            {
                u16Hratio = ((pstHalInputPortCfg->u16Height*4096)/pstInputPortContain->stAttr.stDispWin.u16Width);
                HalDispMopsSetHScaleFac(u16Hratio);
            }
            else
            {
                u16Hratio = 0x1000;
                HalDispMopsSetHScaleFac(u16Hratio);
            }
            if(pstInputPortContain->stAttr.stDispWin.u16Height > pstHalInputPortCfg->u16Width)
            {
                u16Vratio = ((pstHalInputPortCfg->u16Width*4096)/pstInputPortContain->stAttr.stDispWin.u16Height);
                HalDispMopsSetVScaleFac(u16Vratio);
            }
            else
            {
                u16Vratio = 0x1000;
                HalDispMopsSetVScaleFac(u16Vratio);
            }
            //Rot_r input_width=pitch, height=source_height align 16
            u16tempRotInH = pstInputPortContain->stAttr.u16SrcHeight;
            u16RotDummyPix = (u16tempRotInH & 0xF);
            if(u16RotDummyPix)
            {
                u16tempRotInH = ((u16tempRotInH>>4) + 1)<<4;
            }
            HalDispMopRot1SourceHeight(u16tempRotInH);
            //pitch is the same as source width, but 256 align
            if((u16tempRotInH & 0xFF))
            {
                u16tempRotInH = ((u16tempRotInH>>8) + 1)<<8;
            }
            HalDispMopsSetPitch(u16tempRotInH);

            if(pstInputPortContain->stRot.enRotate==E_HAL_DISP_ROTATE_90)
            {
                HalDispMopsSetAddr16Offset(u16RotDummyPix);
            }
            else
            {
                //reset to 0, 270 no need
                HalDispMopsSetAddr16Offset(0);
            }
        }
        else//not rotate case
        {
            HalDispMopsSetSourceParam(pstHalInputPortCfg->u16Width,
                                      pstHalInputPortCfg->u16Height);
            if(pstInputPortContain->stAttr.stDispWin.u16Width > pstHalInputPortCfg->u16Width)
            {
                u16Hratio = ((pstHalInputPortCfg->u16Width*4096)/pstInputPortContain->stAttr.stDispWin.u16Width);
                HalDispMopsSetHScaleFac(u16Hratio);
            }
            else
            {
                u16Hratio = 0x1000;
                HalDispMopsSetHScaleFac(u16Hratio);
            }
            if(pstInputPortContain->stAttr.stDispWin.u16Height > pstHalInputPortCfg->u16Height)
            {
                u16Vratio = ((pstHalInputPortCfg->u16Height*4096)/pstInputPortContain->stAttr.stDispWin.u16Height);
                HalDispMopsSetVScaleFac(u16Vratio);
            }
            else
            {
                u16Vratio = 0x1000;
                HalDispMopsSetVScaleFac(u16Vratio);
            }
        }

    }
    else
    {
        DISP_ERR("%s %d, Port_id:%ld not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }
}


HalDispQueryRet_e _HalDispIfGetInfoClkGet(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    HalDispClkConfig_t *pHalClkCfg = (HalDispClkConfig_t *)pCfg;
    u32 u32ClkRate;
    bool bEn;

    pHalClkCfg->u32Num = HAL_DISP_CLK_NUM;

    HalDispMopGetClk(&bEn, &u32ClkRate);
    pHalClkCfg->bEn[0] = bEn;
    pHalClkCfg->u32Rate[0] = u32ClkRate;

#if defined(HDMITX_VGA_SUPPORTED)
    HalDispGetClkHdmi(&bEn, &u32ClkRate);
    pHalClkCfg->bEn[1] = bEn;
    pHalClkCfg->u32Rate[1] = u32ClkRate;

    HalDispGetClkDac(&bEn, &u32ClkRate);
    pHalClkCfg->bEn[2] = bEn;
    pHalClkCfg->u32Rate[2] = u32ClkRate;

    HalDispGetClkDisp432(&bEn, &u32ClkRate);
    pHalClkCfg->bEn[3] = bEn;
    pHalClkCfg->u32Rate[3] = u32ClkRate;


    HalDispGetClkDisp216(&bEn, &u32ClkRate);
    pHalClkCfg->bEn[4] = bEn;
    pHalClkCfg->u32Rate[4] = u32ClkRate;


    DISP_DBG(DISP_DBG_LEVEL_HAL,"%s %d, Num:%ld, Mop(%d, %ld), Hdmi(%d, %ld), Dac(%d, %ld), Disp432(%d, %ld), Disp216(%d, %ld)\n",
        __FUNCTION__, __LINE__,
        pHalClkCfg->u32Num,
        pHalClkCfg->bEn[0], pHalClkCfg->u32Rate[0],
        pHalClkCfg->bEn[1], pHalClkCfg->u32Rate[1],
        pHalClkCfg->bEn[2], pHalClkCfg->u32Rate[2],
        pHalClkCfg->bEn[3], pHalClkCfg->u32Rate[3],
        pHalClkCfg->bEn[4], pHalClkCfg->u32Rate[4]);
#else
    DISP_DBG(DISP_DBG_LEVEL_HAL,"%s %d, Num:%ld, Mop(%d, %ld)\n",
        __FUNCTION__, __LINE__,
        pHalClkCfg->u32Num,
        pHalClkCfg->bEn[0], pHalClkCfg->u32Rate[0]);
#endif
    return enRet;
}

void _HalDispIfSetClkGet(void *pCtx, void *pCfg)
{
}

HalDispQueryRet_e _HalDispIfGetInfoClkSet(void *pCtx, void *pCfg)
{
    HalDispQueryRet_e enRet = E_HAL_DISP_QUERY_RET_OK;
    HalDispClkConfig_t *pHalClkCfg = (HalDispClkConfig_t *)pCfg;

    if(pHalClkCfg->u32Num != HAL_DISP_CLK_NUM)
    {
        enRet = E_HAL_DISP_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Clk Num is not Correct %ld != %d\n",
            __FUNCTION__, __LINE__, pHalClkCfg->u32Num, HAL_DISP_CLK_NUM);
    }

    return enRet;
}

void _HalDispIfSetClkSet(void *pCtx, void *pCfg)
{
    HalDispClkConfig_t *pHalClkCfg = (HalDispClkConfig_t *)pCfg;

    HalDispMopSetClk(pHalClkCfg->bEn[0], pHalClkCfg->u32Rate[0]);

#if defined(HDMITX_VGA_SUPPORTED)
    HalDispSetClkHdmi(pHalClkCfg->bEn[1], pHalClkCfg->u32Rate[1]);
    HalDispSetClkDac(pHalClkCfg->bEn[2], pHalClkCfg->u32Rate[2]);
    HalDispSetClkDisp432(pHalClkCfg->bEn[3], pHalClkCfg->u32Rate[3]);
    HalDispSetClkDisp216(pHalClkCfg->bEn[4], pHalClkCfg->u32Rate[4]);
    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Mop(%d, %ld), Hdmi(%d, %ld), Dac(%d, %ld), Disp432(%d, %ld), Disp216(%d, %ld)\n",
        __FUNCTION__, __LINE__,
        pHalClkCfg->bEn[0], pHalClkCfg->u32Rate[0],
        pHalClkCfg->bEn[1], pHalClkCfg->u32Rate[1],
        pHalClkCfg->bEn[2], pHalClkCfg->u32Rate[2],
        pHalClkCfg->bEn[3], pHalClkCfg->u32Rate[3],
        pHalClkCfg->bEn[4], pHalClkCfg->u32Rate[4]);
#else
    HalDispSetClkDisp432(pHalClkCfg->bEn[1], pHalClkCfg->u32Rate[1]);
    HalDispSetClkDisp216(pHalClkCfg->bEn[2], pHalClkCfg->u32Rate[2]);

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Mop(%d, %ld), Disp432(%d, %ld), Disp216(%d, %ld)\n",
        __FUNCTION__, __LINE__,
        pHalClkCfg->bEn[0], pHalClkCfg->u32Rate[0],
        pHalClkCfg->bEn[1], pHalClkCfg->u32Rate[1],
        pHalClkCfg->bEn[2], pHalClkCfg->u32Rate[2]);
#endif
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------

bool _HalDispIfGetCallBack(DrvDispCtxConfig_t *pstDispCfg, HalDispQueryConfig_t *pstQueryCfg)
{
    memset(&pstQueryCfg->stOutCfg, 0, sizeof(HalDispQueryOutConfig_t));

    if(pstQueryCfg->stInCfg.u32CfgSize != gpDispCbTbl[pstQueryCfg->stInCfg.enQueryType].u16CfgSize)
    {
        pstQueryCfg->stOutCfg.enQueryRet = E_HAL_DISP_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Query:%s, Size %ld != %d\n",
            __FUNCTION__, __LINE__,
            PARSING_HAL_QUERY_TYPE(pstQueryCfg->stInCfg.enQueryType),
            pstQueryCfg->stInCfg.u32CfgSize,
            gpDispCbTbl[pstQueryCfg->stInCfg.enQueryType].u16CfgSize);
    }
    else
    {
        pstQueryCfg->stOutCfg.pSetFunc = gpDispCbTbl[pstQueryCfg->stInCfg.enQueryType].pSetFunc;

        if(pstQueryCfg->stOutCfg.pSetFunc == NULL)
        {
            pstQueryCfg->stOutCfg.enQueryRet = E_HAL_DISP_QUERY_RET_NOTSUPPORT;
            DISP_ERR("%s %d, Query:%s, SetFunc Empty\n",
                __FUNCTION__, __LINE__,
                PARSING_HAL_QUERY_TYPE(pstQueryCfg->stInCfg.enQueryType));
        }
        else
        {
            pstQueryCfg->stOutCfg.enQueryRet =
                gpDispCbTbl[pstQueryCfg->stInCfg.enQueryType].pGetInfoFunc(pstDispCfg, pstQueryCfg->stInCfg.pInCfg);

        }
    }

    return 1;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool HalDispIfInit(void)
{
    void *pNull;
    //s32 imi_ret;
    if(gbDispHwIfInit)
    {
        return 1;
    }

    pNull = NULL;

    memset(gpDispCbTbl, 0, sizeof(HalDispQueryCallBackFunc_t)*E_HAL_DISP_QUERY_MAX);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_INIT].pGetInfoFunc = _HalDispIfGetInfoDeviceInit;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_INIT].pSetFunc     = _HalDispIfSetDeviceInit;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_INIT].u16CfgSize   = 0;

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_ENABLE].pGetInfoFunc = _HalDispIfGetInfoDeviceEnable;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_ENABLE].pSetFunc     = _HalDispIfSetDeviceEnable;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_ENABLE].u16CfgSize   = sizeof(bool);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_ATTACH].pGetInfoFunc = _HalDispIfGetInfoDeviceAttach;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_ATTACH].pSetFunc     = _HalDispIfSetDeviceAttach;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_ATTACH].u16CfgSize   = sizeof(pNull);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_DETACH].pGetInfoFunc = _HalDispIfGetInfoDeviceDetach;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_DETACH].pSetFunc     = _HalDispIfSetDeviceDetach;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_DETACH].u16CfgSize   = sizeof(pNull);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_BACKGROUND_COLOR].pGetInfoFunc = _HalDispIfGetInfoDeviceBackGroundColor;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_BACKGROUND_COLOR].pSetFunc     = _HalDispIfSetDeviceBackGroundColor;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_BACKGROUND_COLOR].u16CfgSize   = sizeof(u32);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_INTERFACE].pGetInfoFunc = _HalDispIfGetInfoDeviceInterface;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_INTERFACE].pSetFunc     = _HalDispIfSetDeviceInterface;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_INTERFACE].u16CfgSize   = sizeof(u32);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_OUTPUTTIMING].pGetInfoFunc = _HalDispIfGetInfoDeviceOutputTiming;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_OUTPUTTIMING].pSetFunc     = _HalDispIfSetDeviceOutputTiming;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_OUTPUTTIMING].u16CfgSize   = sizeof(HalDispDeviceTimingInfo_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_VGA_PARAM].pGetInfoFunc = _HalDispIfGetInfoDeviceVgaParam;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_VGA_PARAM].pSetFunc     = _HalDispIfSetDeviceVgaParam;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_VGA_PARAM].u16CfgSize   = sizeof(HalDispVgaParam_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_CVBS_PARAM].pGetInfoFunc = _HalDispIfGetInfoDeviceCvbsParam;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_CVBS_PARAM].pSetFunc     = _HalDispIfSetDeviceCvbsParam;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_CVBS_PARAM].u16CfgSize   = sizeof(HalDispCvbsParam_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_COLORTEMP].pGetInfoFunc = _HalDispIfGetInfoDeviceColorTemp;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_COLORTEMP].pSetFunc     = _HalDispIfSetDeviceColorTemp;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_COLORTEMP].u16CfgSize   = sizeof(HalDispColorTemp_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_HDMI_PARAM].pGetInfoFunc =_HalDispIfGetInfoDeviceHdmiParam;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_HDMI_PARAM].pSetFunc     = _HalDispIfSetDeviceHdmiParam;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_HDMI_PARAM].u16CfgSize   = sizeof(HalDispHdmiParam_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_LCD_PARAM].pGetInfoFunc =_HalDispIfGetInfoDeviceLcdParam;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_LCD_PARAM].pSetFunc     = _HalDispIfSetDeviceLcdParam;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_LCD_PARAM].u16CfgSize   = sizeof(HalDispLcdParam_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_GAMMA_PARAM].pGetInfoFunc =_HalDispIfGetInfoDeviceGammaParam;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_GAMMA_PARAM].pSetFunc     = _HalDispIfSetDeviceGammaParam;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_GAMMA_PARAM].u16CfgSize   = sizeof(HalDispGammaParam_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_TIME_ZONE].pGetInfoFunc =_HalDispIfGetInfoDevicTimeZone;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_TIME_ZONE].pSetFunc     = _HalDispIfSetDeviceTimeZone;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_TIME_ZONE].u16CfgSize   = sizeof(HalDispTimeZone_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_DISPLAY_INFO].pGetInfoFunc =_HalDispIfGetInfoDevicDisplayInfo;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_DISPLAY_INFO].pSetFunc     = _HalDispIfSetDeviceDisplayInfo;
    gpDispCbTbl[E_HAL_DISP_QUERY_DEVICE_DISPLAY_INFO].u16CfgSize   = sizeof(HalDispDisplayInfo_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_INIT].pGetInfoFunc = _HalDispIfGetInfoVidLayerInit;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_INIT].pSetFunc     = _HalDispIfSetVidLayerInit;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_INIT].u16CfgSize   = 0;

    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_ENABLE].pGetInfoFunc = _HalDispIfGetInfoVidLayerEnable;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_ENABLE].pSetFunc     = _HalDispIfSetVidLayerEnable;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_ENABLE].u16CfgSize   = sizeof(bool);

    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_BIND].pGetInfoFunc = _HalDispIfGetInfoVidLayerBind;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_BIND].pSetFunc     = _HalDispIfSetVidLayerBind;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_BIND].u16CfgSize   = sizeof(pNull);

    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_UNBIND].pGetInfoFunc = _HalDispIfGetInfoVidLayerUnBind;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_UNBIND].pSetFunc     = _HalDispIfSetVidLayerUnBind;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_UNBIND].u16CfgSize   = sizeof(pNull);

    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_ATTR].pGetInfoFunc = _HalDispIfGetInfoVidLayerAttr;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_ATTR].pSetFunc     = _HalDispIfSetVidLayerAttr;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_ATTR].u16CfgSize   = sizeof(HalDispVideoLayerAttr_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_COMPRESS].pGetInfoFunc = _HalDispIfGetInfoVidLayerCompress;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_COMPRESS].pSetFunc     = _HalDispIfSetVidLayerComporess;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_COMPRESS].u16CfgSize   = sizeof(HalDispVideoLayerCompressAttr_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_PRIORITY].pGetInfoFunc = _HalDispIfGetInfoVidLayerPriority;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_PRIORITY].pSetFunc     = _HalDispIfSetVidLayerPriority;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_PRIORITY].u16CfgSize   = sizeof(u32);

    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_BUFFER_FIRE].pGetInfoFunc = _HalDispIfGetInfoVidLayerBufferFire;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_BUFFER_FIRE].pSetFunc     = _HalDispIfSetVidLayerBufferFire;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_BUFFER_FIRE].u16CfgSize   = 0;

    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_CHECK_FIRE].pGetInfoFunc = _HalDispIfGetInfoVidLayerCheckFire;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_CHECK_FIRE].pSetFunc     = _HalDispIfSetVidLayerCheckFire;
    gpDispCbTbl[E_HAL_DISP_QUERY_VIDEOLAYER_CHECK_FIRE].u16CfgSize   = 0;

    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_INIT].pGetInfoFunc = _HalDispIfGetInfoInputPortInit;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_INIT].pSetFunc     = _HalDispIfSetInputPortInit;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_INIT].u16CfgSize   = 0;

    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_ENABLE].pGetInfoFunc = _HalDispIfGetInfoInputPortEnable;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_ENABLE].pSetFunc     = _HalDispIfSetInputPortEnable;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_ENABLE].u16CfgSize   = sizeof(bool);

    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_ATTR].pGetInfoFunc = _HalDispIfGetInfoInputPortAttr;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_ATTR].pSetFunc     = _HalDispIfSetInputPortAttr;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_ATTR].u16CfgSize   = sizeof(HalDispInputPortAttr_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_SHOW].pGetInfoFunc = _HalDispIfGetInfoInputPortShow;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_SHOW].pSetFunc     = _HalDispIfSetInputPortShow;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_SHOW].u16CfgSize   = 0;

    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_HIDE].pGetInfoFunc = _HalDispIfGetInfoInputPortHide;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_HIDE].pSetFunc     = _HalDispIfSetInputPortHide;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_HIDE].u16CfgSize   = 0;

    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_BEGIN].pGetInfoFunc = _HalDispIfGetInfoInputPortBegin;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_BEGIN].pSetFunc     = _HalDispIfSetInputPortBegin;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_BEGIN].u16CfgSize   = 0;

    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_END].pGetInfoFunc = _HalDispIfGetInfoInputPortEnd;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_END].pSetFunc     = _HalDispIfSetInputPortEnd;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_END].u16CfgSize   = 0;

    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_FLIP].pGetInfoFunc = _HalDispIfGetInfoInputPortFlip;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_FLIP].pSetFunc     = _HalDispIfSetInputPortFlip;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_FLIP].u16CfgSize   = sizeof(HalDispVideoFrameData_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_ROTATE].pGetInfoFunc = _HalDispIfGetInfoInputPortRotate;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_ROTATE].pSetFunc     = _HalDispIfSetInputPortRotate;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_ROTATE].u16CfgSize   = sizeof(HalDispInputPortRotate_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_CROP].pGetInfoFunc = _HalDispIfGetInfoInputPortCrop;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_CROP].pSetFunc     = _HalDispIfSetInputPortCrop;
    gpDispCbTbl[E_HAL_DISP_QUERY_INPUTPORT_CROP].u16CfgSize   = sizeof(HalDispVidWinRect_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_CLK_SET].pGetInfoFunc = _HalDispIfGetInfoClkSet;
    gpDispCbTbl[E_HAL_DISP_QUERY_CLK_SET].pSetFunc     = _HalDispIfSetClkSet;
    gpDispCbTbl[E_HAL_DISP_QUERY_CLK_SET].u16CfgSize   = sizeof(HalDispClkConfig_t);

    gpDispCbTbl[E_HAL_DISP_QUERY_CLK_GET].pGetInfoFunc = _HalDispIfGetInfoClkGet;
    gpDispCbTbl[E_HAL_DISP_QUERY_CLK_GET].pSetFunc     = _HalDispIfSetClkGet;
    gpDispCbTbl[E_HAL_DISP_QUERY_CLK_GET].u16CfgSize   = sizeof(HalDispClkConfig_t);

    gbDispHwIfInit = 1;

    //rotate init
    _gstMopgRotRec.bRotEn = 0;
    _gstMopgRotRec.eMopgId = E_HAL_DISP_MOPG_ID_NUM;
    _gstMopgRotRec.stRotAng = E_HAL_DISP_ROTATE_NONE;
    //allocate imi for rotate
    /*
    imi_ret = DrvDispOsImiHeapAlloc(0x4000, &u32Rot0ImiYaddr);
    if(imi_ret)
    {
        DISP_ERR("%s %d, imi_heap init Rot0 Y 0x4000 fail, err=%ld\n", __FUNCTION__, __LINE__, imi_ret);
    }

    imi_ret = DrvDispOsImiHeapAlloc(0x2000, &u32Rot0ImiCaddr);
    if(imi_ret)
    {
        DISP_ERR("%s %d, imi_heap init Rot0 C 0x2000 fail, err=%ld\n", __FUNCTION__, __LINE__, imi_ret);
    }

    imi_ret = DrvDispOsImiHeapAlloc(0x4000, &u32Rot1ImiYaddr);
    if(imi_ret)
    {
        DISP_ERR("%s %d, imi_heap init Rot1 Y 0x4000 fail, err=%ld\n", __FUNCTION__, __LINE__, imi_ret);
    }

    imi_ret = DrvDispOsImiHeapAlloc(0x2000, &u32Rot1ImiCaddr);
    if(imi_ret)
    {
        DISP_ERR("%s %d, imi_heap init Rot1 C 0x2000 fail, err=%ld\n", __FUNCTION__, __LINE__, imi_ret);
    }
    */

    return 1;
}

bool HalDispIfDeInit(void)
{
    if(gbDispHwIfInit == 0)
    {
        DISP_ERR("%s %d, HalIf not init\n", __FUNCTION__, __LINE__);
        return 0;
    }
    gbDispHwIfInit = 0;
    memset(gpDispCbTbl, 0, sizeof(HalDispQueryCallBackFunc_t)* E_HAL_DISP_QUERY_MAX);

    //rotate deinit
    _gstMopgRotRec.bRotEn = 0;
    _gstMopgRotRec.eMopgId = E_HAL_DISP_MOPG_ID_NUM;
    _gstMopgRotRec.stRotAng = E_HAL_DISP_ROTATE_NONE;
    //free imi
    if(bImiInit)
    {
        DrvDispOsImiHeapFree(u32MopRotImiTotalAddr);
        bImiInit=0;
    }
    /*
    DrvDispOsImiHeapFree(u32Rot0ImiYaddr);
    DrvDispOsImiHeapFree(u32Rot0ImiCaddr);
    DrvDispOsImiHeapFree(u32Rot1ImiYaddr);
    DrvDispOsImiHeapFree(u32Rot1ImiCaddr);
    */

    return 1;
}

bool HalDispIfQuery(void *pCtx,  void *pCfg)
{
    bool bRet = 1;

    if(pCtx == NULL)
    {
        DISP_ERR("%s %d, Input Ctx is Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else if(pCfg == NULL)
    {
        DISP_ERR("%s %d, Input Cfg is Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = _HalDispIfGetCallBack((DrvDispCtxConfig_t *)pCtx, (HalDispQueryConfig_t *)pCfg);
    }

    return bRet;
}



