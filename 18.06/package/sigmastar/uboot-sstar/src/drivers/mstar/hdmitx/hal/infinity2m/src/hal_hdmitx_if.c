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

#define _HAL_HDMITX_IF_C_
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mhal_common.h"
#include "drv_hdmitx_os.h"
#include "hdmitx_debug.h"
#include "hal_hdmitx_chip.h"
#include "hal_hdmitx_st.h"
#include "drv_hdmitx_ctx.h"
#include "apiHDMITx.h"
#include "drvHDMITx.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HAL_HDMITX_IF_SPD_INFOFRAME_DATA_SIZE   25
#define HAL_HDMITX_IF_AVI_INFOFRAME_DATA_SIZE   13
#define HAL_HDMITX_IF_AUD_INFOFRAME_DATA_SIZE   24

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    HalHdmitxQueryRet_e (*pGetInfoFunc) (void *, void *);
    void (*pSetFunc) (void * , void *);
    u16  u16CfgSize;
}HalHdmitxQueryCallBackFunc_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
bool gbHalIfInit = 0;
HalHdmitxQueryCallBackFunc_t gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_MAX];


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
HDMITX_VIDEO_COLOR_FORMAT __HalHdmitxIfTransColorFmt(HalHdmitxColorType_e enColor)
{
    HDMITX_VIDEO_COLOR_FORMAT enColorFmt;

    enColorFmt = enColor == E_HAL_HDMITX_COLOR_YUV444 ? HDMITX_VIDEO_COLOR_YUV444 :
                 enColor == E_HAL_HDMITX_COLOR_YUV422 ? HDMITX_VIDEO_COLOR_YUV422 :
                 enColor == E_HAL_HDMITX_COLOR_YUV420 ? HDMITX_VIDEO_COLOR_YUV420 :
                                                        HDMITX_VIDEO_COLOR_RGB444 ;
    return enColorFmt;
}

HalHdmitxColorType_e __HalHdmitxIfTransColorFmtToHal(HDMITX_VIDEO_COLOR_FORMAT enColorFmt)
{
    HalHdmitxColorType_e enColor;

    enColor = enColorFmt == HDMITX_VIDEO_COLOR_YUV444 ? E_HAL_HDMITX_COLOR_YUV444 :
              enColorFmt == HDMITX_VIDEO_COLOR_YUV422 ? E_HAL_HDMITX_COLOR_YUV422 :
              enColorFmt == HDMITX_VIDEO_COLOR_YUV420 ? E_HAL_HDMITX_COLOR_YUV420 :
                                                        E_HAL_HDMITX_COLOR_RGB444;
    return enColor;
}

HDMITX_VIDEO_COLORDEPTH_VAL __HalHdmitxIfTransColorDepth(HalHdmitxColorDepthType_e enColorDepth)
{
    HDMITX_VIDEO_COLORDEPTH_VAL enCD;

    enCD = enColorDepth == E_HAL_HDMITX_CD_24_BITS ? HDMITX_VIDEO_CD_24Bits :
           enColorDepth == E_HAL_HDMITX_CD_30_BITS ? HDMITX_VIDEO_CD_30Bits :
           enColorDepth == E_HAL_HDMITX_CD_36_BITS ? HDMITX_VIDEO_CD_36Bits :
           enColorDepth == E_HAL_HDMITX_CD_48_BITS ? HDMITX_VIDEO_CD_48Bits :
                                                     HDMITX_VIDEO_CD_NoID;
    return enCD;
}

HDMITX_OUTPUT_MODE __HalHdmitxIfTransOutputMode(HalHdmitxOutpuModeType_e enOutputMode)
{
    HDMITX_OUTPUT_MODE enMode;

    enMode = enOutputMode == E_HAL_HDMITX_OUTPUT_MODE_HDMI      ? HDMITX_HDMI      :
             enOutputMode == E_HAL_HDMITX_OUTPUT_MODE_HDMI_HDCP ? HDMITX_HDMI_HDCP :
             enOutputMode == E_HAL_HDMITX_OUTPUT_MODE_DVI_HDCP  ? HDMITX_DVI_HDCP  :
                                                                  HDMITX_DVI;
    return enMode;
}

HDMITX_VIDEO_TIMING __HalHdmitxIfTransTimingRes(HalHdmitxTimingResType_e enTiming)
{
    HDMITX_VIDEO_TIMING enVideoTiming;

    enVideoTiming = enTiming == E_HAL_HDMITX_RES_720X480P_60HZ   ? HDMITX_RES_720x480p :
                    enTiming == E_HAL_HDMITX_RES_720X576P_50HZ   ? HDMITX_RES_720x576p :
                    enTiming == E_HAL_HDMITX_RES_1280X720P_60HZ  ? HDMITX_RES_1280x720p_60Hz :
                    enTiming == E_HAL_HDMITX_RES_1280X720P_50HZ  ? HDMITX_RES_1280x720p_50Hz :
                    enTiming == E_HAL_HDMITX_RES_1920X1080P_24HZ ? HDMITX_RES_1920x1080p_24Hz :
                    enTiming == E_HAL_HDMITX_RES_1920X1080P_25HZ ? HDMITX_RES_1920x1080p_25Hz :
                    enTiming == E_HAL_HDMITX_RES_1920X1080P_30HZ ? HDMITX_RES_1920x1080p_30Hz :
                    enTiming == E_HAL_HDMITX_RES_1920X1080P_50HZ ? HDMITX_RES_1920x1080p_50Hz :
                    enTiming == E_HAL_HDMITX_RES_1920X1080P_60HZ ? HDMITX_RES_1920x1080p_60Hz :
                    enTiming == E_HAL_HDMITX_RES_3840X2160P_24HZ ? HDMITX_RES_3840x2160p_24Hz :
                    enTiming == E_HAL_HDMITX_RES_3840X2160P_25HZ ? HDMITX_RES_3840x2160p_25Hz :
                    enTiming == E_HAL_HDMITX_RES_3840X2160P_30HZ ? HDMITX_RES_3840x2160p_30Hz :
                    enTiming == E_HAL_HDMITX_RES_3840X2160P_50HZ ? HDMITX_RES_3840x2160p_50Hz :
                    enTiming == E_HAL_HDMITX_RES_3840X2160P_60HZ ? HDMITX_RES_3840x2160p_60Hz :
                    enTiming == E_HAL_HDMITX_RES_1024X768P_60HZ  ? HDMITX_RES_1024x768p_60Hz :
                    enTiming == E_HAL_HDMITX_RES_1366X768P_60HZ  ? HDMITX_RES_1366x768p_60Hz :
                    enTiming == E_HAL_HDMITX_RES_1440X900P_60HZ  ? HDMITX_RES_1440x900p_60Hz :
                    enTiming == E_HAL_HDMITX_RES_1280X800P_60HZ  ? HDMITX_RES_1280x800p_60Hz :
                    enTiming == E_HAL_HDMITX_RES_1280X1024P_60HZ ? HDMITX_RES_1280x1024p_60Hz :
                    enTiming == E_HAL_HDMITX_RES_1680X1050P_60HZ ? HDMITX_RES_1680x1050p_60Hz :
                    enTiming == E_HAL_HDMITX_RES_1600X1200P_60HZ ? HDMITX_RES_1600x1200p_60Hz :
                                                                   HDMITX_RES_MAX;
    return enVideoTiming;
}

HDMITX_AUDIO_FREQUENCY __HalHdmitxIfTransAudioFreq(HalHdmitxAudioFreqType_e enAudioFreq)
{
    HDMITX_AUDIO_FREQUENCY enFreq;

    enFreq = enAudioFreq == E_HAL_HDMITX_AUDIO_FREQ_NO_SIG  ? HDMITX_AUDIO_FREQ_NO_SIG :
             enAudioFreq == E_HAL_HDMITX_AUDIO_FREQ_32K     ? HDMITX_AUDIO_32K :
             enAudioFreq == E_HAL_HDMITX_AUDIO_FREQ_44K     ? HDMITX_AUDIO_44K :
             enAudioFreq == E_HAL_HDMITX_AUDIO_FREQ_48K     ? HDMITX_AUDIO_48K :
             enAudioFreq == E_HAL_HDMITX_AUDIO_FREQ_88K     ? HDMITX_AUDIO_88K :
             enAudioFreq == E_HAL_HDMITX_AUDIO_FREQ_96K     ? HDMITX_AUDIO_96K :
             enAudioFreq == E_HAL_HDMITX_AUDIO_FREQ_176K    ? HDMITX_AUDIO_176K :
             enAudioFreq == E_HAL_HDMITX_AUDIO_FREQ_192K    ? HDMITX_AUDIO_192K :
                                                              HDMITX_AUDIO_FREQ_MAX_NUM;
    return enFreq;
}

HDMITX_AUDIO_SOURCE_FORMAT __HalHdmitxIfTransAudioSrcFmt(HalHdmitxAudioSourceFormat_e enSrcFmt)
{
    HDMITX_AUDIO_SOURCE_FORMAT enFmt;

    enFmt = enSrcFmt == E_HAL_HDMITX_AUDIO_FORMAT_PCM ? HDMITX_AUDIO_FORMAT_PCM :
            enSrcFmt == E_HAL_HDMITX_AUDIO_FORMAT_DSD ? HDMITX_AUDIO_FORMAT_DSD :
            enSrcFmt == E_HAL_HDMITX_AUDIO_FORMAT_HBR ? HDMITX_AUDIO_FORMAT_HBR :
                                                        HDMITX_AUDIO_FORMAT_NA;
    return enFmt;
}

HDMITX_AUDIO_CHANNEL_COUNT __HalHdmitxIfTransAudioCh(HalHdmitxAudioChannelType_e enAudioCh)
{
    HDMITX_AUDIO_CHANNEL_COUNT enCh;

    enCh = enAudioCh == E_HAL_HDMITX_AUDIO_CH_2 ? HDMITX_AUDIO_CH_2 :
                                                  HDMITX_AUDIO_CH_8;
    return enCh;
}

HDMITX_AUDIO_CODING_TYPE __HalHdmitxIfTransAudioCoding(HalHdmitxAudioCodingType_e enAudioCoding)
{
    HDMITX_AUDIO_CODING_TYPE enCode;

    enCode = enAudioCoding == E_HAL_HDMITX_AUDIO_CODING_PCM ? HDMITX_AUDIO_PCM :
                                                              HDMITX_AUDIO_NONPCM;

    return enCode;
}

bool __HalHdmitxIfCheckEdidRdy(void)
{
    bool bEdidChk = 1;
    u32 u32Time = DrvHdmitxOsGetSystemTime();
    while(1)
    {
        DrvHdmitxOsMsSleep(10);
        bEdidChk = MApi_HDMITx_EdidChecking();

        if( bEdidChk || (DrvHdmitxOsGetSystemTime() - u32Time) > 200)
        {
            break;
        }
    }

    return bEdidChk;
}

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoClkSet(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    HalHdmitxClkConfig_t *pstHalClkCfg = (HalHdmitxClkConfig_t *)pCfg;

    if(pstHalClkCfg->u32Num != HAL_HDMITX_CLK_NUM)
    {
        enRet = E_HAL_HDMITX_QUERY_RET_CFGERR;
        HDMITX_ERR("%s %d, Clk Num is not Correct, %d != %d\n",
            __FUNCTION__, __LINE__, pstHalClkCfg->u32Num, HAL_HDMITX_CLK_NUM);
    }
    return enRet;
}

void _HalHdmitxIfSetClkSet(void *pCtx, void *pCfg)
{
    HalHdmitxClkConfig_t *pstHalClkCfg = (HalHdmitxClkConfig_t *)pCfg;
    MS_BOOL abEn[HAL_HDMITX_CLK_NUM];
    MS_U32  au32ClkRate[HAL_HDMITX_CLK_NUM];

    memcpy(abEn, pstHalClkCfg->bEn, sizeof(bool)*pstHalClkCfg->u32Num);
    memcpy(au32ClkRate, pstHalClkCfg->u32Rate, sizeof(u32)*pstHalClkCfg->u32Num);

    MDrv_HDMITx_SetClk(NULL, abEn, au32ClkRate, pstHalClkCfg->u32Num);
}

HalHdmitxQueryRet_e _HalHdmitxIfGetInfoClkGet(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    HalHdmitxClkConfig_t *pstHalClkCfg = (HalHdmitxClkConfig_t *)pCfg;
    MS_BOOL abEn[HAL_HDMITX_CLK_NUM];
    MS_U32  au32ClkRate[HAL_HDMITX_CLK_NUM];

    if(pstHalClkCfg->u32Num == HAL_HDMITX_CLK_NUM)
    {
        if(MDrv_HDMITx_GetClk(NULL, abEn, au32ClkRate, HAL_HDMITX_CLK_NUM))
        {
            memcpy(pstHalClkCfg->bEn, abEn, sizeof(bool)*HAL_HDMITX_CLK_NUM);
            memcpy(pstHalClkCfg->u32Rate, au32ClkRate, sizeof(u32)*HAL_HDMITX_CLK_NUM);
        }
        else
        {
            enRet = E_HAL_HDMITX_QUERY_RET_CFGERR;
            HDMITX_ERR("%s %d, Get CLK Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        enRet = E_HAL_HDMITX_QUERY_RET_CFGERR;
        HDMITX_ERR("%s %d, Clk Num is not Correct, %d != %d\n",
            __FUNCTION__, __LINE__, pstHalClkCfg->u32Num, HAL_HDMITX_CLK_NUM);
    }
    return enRet;
}

void _HalHdmitxIfSetClkGet(void *pCtx, void *pCfg)
{

}

HalHdmitxQueryRet_e _HalHdmitxIfGetInfoInitConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    DrvHdmitxCtxConfig_t *pstHdmitxCtx = (DrvHdmitxCtxConfig_t *)pCtx;
    DrvHdmitxCtxHalContain_t *pstHalCtx = pstHdmitxCtx->pstHalCtx;

    pstHalCtx->bInit = 1;
    pstHalCtx->bVideoOnOff = 0;
    pstHalCtx->bAudioOnOff = 0;
    pstHalCtx->bSignal = 0;
    pstHalCtx->bAvMute = 1;

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Init:%d, Video:%d, Audio:%d, AvMute:%d, Singal:%d\n",
        __FUNCTION__, __LINE__,
        pstHalCtx->bInit, pstHalCtx->bVideoOnOff,
        pstHalCtx->bAudioOnOff, pstHalCtx->bAvMute,
        pstHalCtx->bSignal);

    return enRet;
}

void _HalHdmitxIfSetInitConfig(void *pCtx, void *pCfg)
{
    DrvHdmitxCtxConfig_t *pstHdmitxCtx = (DrvHdmitxCtxConfig_t *)pCtx;
    DrvHdmitxCtxHalContain_t *pstHalCtx = pstHdmitxCtx->pstHalCtx;

    MApi_HDMITx_Init();
    MApi_HDMITx_UnHDCPRxControl(E_UNHDCPRX_BLUE_SCREEN);
    MApi_HDMITx_HDCPRxFailControl(E_UNHDCPRX_BLUE_SCREEN);
    MApi_HDMITx_SetHDCPOnOff(FALSE);
    MApi_HDMITx_HDCP_StartAuth(FALSE);

    MApi_HDMITx_SetVideoOnOff(pstHalCtx->bVideoOnOff);
    MApi_HDMITx_SetAudioOnOff(pstHalCtx->bAudioOnOff);
    MApi_HDMITx_SetTMDSOnOff(pstHalCtx->bSignal);
    MApi_HDMITx_SetAVMUTE(pstHalCtx->bAvMute);
}

//------------------------------------------------------------------------------
// DeInit
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoDeInitConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    DrvHdmitxCtxConfig_t *pstHdmitxCtx = (DrvHdmitxCtxConfig_t *)pCtx;

    pstHdmitxCtx->pstHalCtx->bInit = 0;

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Init:%d\n",
        __FUNCTION__, __LINE__,
        pstHdmitxCtx->pstHalCtx->bInit);

    return enRet;
}

void _HalHdmitxIfSetDeInitConfig(void *pCtx, void *pCfg)
{
    MApi_HDMITx_Exit();
}

//------------------------------------------------------------------------------
// AttrBegin
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoAttrBeginConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    DrvHdmitxCtxConfig_t *pstHdmitxCtx = (DrvHdmitxCtxConfig_t *)pCtx;
    DrvHdmitxCtxHalContain_t *pstHalCtx = pstHdmitxCtx->pstHalCtx;

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Attr Begin, %x\n",
        __FUNCTION__, __LINE__, pstHalCtx->enStatusFlag);

    pstHalCtx->enStatusFlag |= E_HAL_HDMITX_STATUS_FLAG_ATTR_BEGIN;
    pstHalCtx->enStatusFlag &= ~E_HAL_HDMITX_STATUS_FLAG_ATTR_END;

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Attr Begin, %x\n",
        __FUNCTION__, __LINE__, pstHalCtx->enStatusFlag);
    return enRet;
}

void _HalHdmitxIfSetAttrBeingConfig(void *pCtx, void *pCfg)
{
    MApi_HDMITx_TurnOnOff(0);
}

//------------------------------------------------------------------------------
// Attr
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoAttrConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    DrvHdmitxCtxConfig_t *pstHdmitxCtx = (DrvHdmitxCtxConfig_t *)pCtx;
    DrvHdmitxCtxHalContain_t *pstHalCtx = pstHdmitxCtx->pstHalCtx;
    HalHdmitxAttrConfig_t *pstAttrCfg = (HalHdmitxAttrConfig_t *)pCfg;

    memcpy(&pstHalCtx->stAttrCfg, pstAttrCfg, sizeof(HalHdmitxAttrConfig_t));

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Video(%d, %s %s %s %s %s), Audio(%d, %s %s %s %s)\n",
        __FUNCTION__, __LINE__,
        pstHalCtx->stAttrCfg.bVideoEn,
        PARSING_HAL_OUTPUT_MODE(pstHalCtx->stAttrCfg.enOutputMode),
        PARSING_HAL_COLOR_DEPTH(pstHalCtx->stAttrCfg.enColorDepth),
        PARSING_HAL_COLOR_FMT(pstHalCtx->stAttrCfg.enInColor),
        PARSING_HAL_COLOR_FMT(pstHalCtx->stAttrCfg.enOutColor),
        PARSING_HAL_TIMING(pstHalCtx->stAttrCfg.enTiming),
        pstHalCtx->stAttrCfg.bAudioEn,
        PARSING_HAL_AUDIO_FREQ(pstHalCtx->stAttrCfg.enAudioFreq),
        PARSING_HAL_AUDIO_CH(pstHalCtx->stAttrCfg.enAudioCh),
        PARSING_HAL_AUDIO_CODING(pstHalCtx->stAttrCfg.enAudioCode),
        PARSING_HAL_AUDIO_FMT(pstHalCtx->stAttrCfg.enAudioFmt));
    return enRet;
}

void _HalHdmitxIfSetAttrConfig(void *pCtx, void *pCfg)
{
    HalHdmitxAttrConfig_t *pstAttrCfg = (HalHdmitxAttrConfig_t *)pCfg;
    HDMITX_AUDIO_FREQUENCY enAudioFreq;
    HDMITX_AUDIO_SOURCE_FORMAT enAudioFmt;
    HDMITX_AUDIO_CODING_TYPE enAudioCode;
    HDMITX_AUDIO_CHANNEL_COUNT enAudioCh;

    HDMITX_VIDEO_COLOR_FORMAT enInColor, enOutColor;
    HDMITX_VIDEO_COLORDEPTH_VAL enColorDepth;
    HDMITX_OUTPUT_MODE enOutputMode;
    HDMITX_VIDEO_TIMING enTiming;

    if(pstAttrCfg->bAudioEn)
    {
        enAudioFreq = __HalHdmitxIfTransAudioFreq(pstAttrCfg->enAudioFreq);
        enAudioFmt  = __HalHdmitxIfTransAudioSrcFmt(pstAttrCfg->enAudioFmt);
        enAudioCode = __HalHdmitxIfTransAudioCoding(pstAttrCfg->enAudioCode);
        enAudioCh   = __HalHdmitxIfTransAudioCh(pstAttrCfg->enAudioCh);
        MApi_HDMITx_SetAudioConfiguration(enAudioFreq, enAudioCh, enAudioCode);
        MApi_HDMITx_SetAudioSourceFormat(enAudioFmt);
        MApi_HDMITx_SetAudioFrequency(enAudioFreq);
    }


    if(pstAttrCfg->bVideoEn)
    {
        enOutputMode = __HalHdmitxIfTransOutputMode(pstAttrCfg->enOutputMode);
        enInColor    = __HalHdmitxIfTransColorFmt(pstAttrCfg->enInColor);
        enOutColor   = __HalHdmitxIfTransColorFmt(pstAttrCfg->enOutColor);
        enColorDepth = __HalHdmitxIfTransColorDepth(pstAttrCfg->enColorDepth);
        enTiming     = __HalHdmitxIfTransTimingRes(pstAttrCfg->enTiming);

        MApi_HDMITx_SetHDMITxMode(enOutputMode);
        MApi_HDMITx_SetColorFormat(enInColor, enOutColor);
        MApi_HDMITx_ForceHDMIOutputColorFormat( (pstAttrCfg->enOutColor == E_HAL_HDMITX_COLOR_AUTO) ? 0 : 1, enOutColor);
        MApi_HDMITx_SetHDMITxMode_CD(enOutputMode, enColorDepth);
        MApi_HDMITx_SetVideoOutputTiming(enTiming);
    }
}

//------------------------------------------------------------------------------
// AttrEnd
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoAttrEndConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    DrvHdmitxCtxConfig_t *pstHdmitxCtx = (DrvHdmitxCtxConfig_t *)pCtx;
    DrvHdmitxCtxHalContain_t *pstHalCtx = pstHdmitxCtx->pstHalCtx;

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Attr End, %x\n", __FUNCTION__, __LINE__, pstHalCtx->enStatusFlag);

    if(pstHalCtx->enStatusFlag & E_HAL_HDMITX_STATUS_FLAG_ATTR_BEGIN)
    {
        pstHalCtx->enStatusFlag |= E_HAL_HDMITX_STATUS_FLAG_ATTR_END;
        pstHalCtx->enStatusFlag &= ~E_HAL_HDMITX_STATUS_FLAG_ATTR_BEGIN;
    }
    else
    {
        enRet = E_HAL_HDMITX_QUERY_RET_ERR;
        pstHalCtx->enStatusFlag |= E_HAL_HDMITX_STATUS_FLAG_ATTR_END;
        HDMITX_ERR("%s %d, No Begin\n", __FUNCTION__, __LINE__);
    }

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Attr End, %x\n", __FUNCTION__, __LINE__, pstHalCtx->enStatusFlag);

    return enRet;
}

void _HalHdmitxIfSetAttrEndConfig(void *pCtx, void *pCfg)
{
    HDMITX_FSM_STATUS enStatus;
    MS_U32 u32Time1, u32Time2;


    MApi_HDMITx_TurnOnOff(1);

    MApi_HDMITx_Exhibit();

    u32Time1 = DrvHdmitxOsGetSystemTime();

    while(1)
    {
        DrvHdmitxOsMsSleep(10);

        enStatus = MApi_HDMITx_GetFsmStatus();
        u32Time2 = DrvHdmitxOsGetSystemTime();

        if(enStatus == E_HDMITX_FSM_STATUS_DONE || (u32Time2 - u32Time1) > 500)
        {
            break;
        }
    }

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Status:%d, Time:%d\n",
        __FUNCTION__, __LINE__, enStatus, (u32Time2 - u32Time1));
}

//------------------------------------------------------------------------------
// Signal
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoSignalConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    DrvHdmitxCtxConfig_t *pstHdmitxCtx = (DrvHdmitxCtxConfig_t *)pCtx;
    DrvHdmitxCtxHalContain_t *pstHalCtx = pstHdmitxCtx->pstHalCtx;
    HalHdmitxSignalConfig_t *pstSignalCfg = (HalHdmitxSignalConfig_t *)pCfg;

    pstHalCtx->bSignal = pstSignalCfg->bEn;

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Signal=%d\n",
        __FUNCTION__, __LINE__,
        pstHalCtx->bSignal);

    return enRet;
}

void _HalHdmitxIfSetSignalConfig(void *pCtx, void *pCfg)
{
    HalHdmitxSignalConfig_t *pstSignalCfg = (HalHdmitxSignalConfig_t *)pCfg;

    if(pstSignalCfg->bEn)
    {
        MApi_HDMITx_DisableTMDSCtrl(0);
        MApi_HDMITx_SetTMDSOnOff(1);
    }
    else
    {
        MApi_HDMITx_SetTMDSOnOff(0);
        MApi_HDMITx_DisableTMDSCtrl(1);
    }
}

//------------------------------------------------------------------------------
// Mute
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoMuteConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    DrvHdmitxCtxConfig_t *pstHdmitxCtx = (DrvHdmitxCtxConfig_t *)pCtx;
    DrvHdmitxCtxHalContain_t *pstHalCtx = pstHdmitxCtx->pstHalCtx;
    HalHdmitxMuteConfig_t *pstMuteCfg = (HalHdmitxMuteConfig_t *)pCfg;

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Vidoe:%d, Audio:%d, AvMute:%d\n",
        __FUNCTION__, __LINE__,
        pstHalCtx->bVideoOnOff, pstHalCtx->bAudioOnOff, pstHalCtx->bAvMute);

    if(pstMuteCfg->enType & E_HAL_HDMITX_MUTE_VIDEO)
    {
        pstHalCtx->bVideoOnOff = pstMuteCfg->bMute ? 0 : 1;
    }

    if(pstMuteCfg->enType & E_HAL_HDMITX_MUTE_AUDIO)
    {
        pstHalCtx->bAudioOnOff = pstMuteCfg->bMute ? 0 : 1;
    }

    if(pstMuteCfg->enType & E_HAL_HDMITX_MUTE_AVMUTE)
    {
        pstHalCtx->bAvMute = pstMuteCfg->bMute;
    }

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, enType:%x, Vidoe:%d, Audio:%d, AvMute:%d\n",
        __FUNCTION__, __LINE__,
        pstMuteCfg->enType,  pstHalCtx->bVideoOnOff,
        pstHalCtx->bAudioOnOff,  pstHalCtx->bAvMute);

    return enRet;
}

void _HalHdmitxIfSetMuteConfig(void *pCtx, void *pCfg)
{
    HalHdmitxMuteConfig_t *pstMuteCfg = (HalHdmitxMuteConfig_t *)pCfg;

    if(pstMuteCfg->bMute)
    {
        if(pstMuteCfg->enType & E_HAL_HDMITX_MUTE_AVMUTE)
        {
            MApi_HDMITx_SetAVMUTE(1);
            MApi_HDMITx_DisableAvMuteCtrl(1);
        }

        if(pstMuteCfg->enType & E_HAL_HDMITX_MUTE_VIDEO)
        {
            MApi_HDMITx_SetVideoOnOff(0);
        }

        if(pstMuteCfg->enType & E_HAL_HDMITX_MUTE_AUDIO)
        {
            MApi_HDMITx_SetAudioOnOff(0);
        }
    }
    else
    {
        if(pstMuteCfg->enType & E_HAL_HDMITX_MUTE_AUDIO)
        {
            MApi_HDMITx_SetAudioOnOff(1);
        }

        if(pstMuteCfg->enType & E_HAL_HDMITX_MUTE_VIDEO)
        {
            MApi_HDMITx_SetVideoOnOff(1);
        }

        if(pstMuteCfg->enType & E_HAL_HDMITX_MUTE_AVMUTE)
        {
            MApi_HDMITx_DisableAvMuteCtrl(0);
            MApi_HDMITx_SetAVMUTE(0);
        }
    }
}

//------------------------------------------------------------------------------
// Analog Driving Current
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoAnalogDrvCurConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    DrvHdmitxCtxConfig_t *pstHdmitxCtxCfg = (DrvHdmitxCtxConfig_t *)pCtx;
    HalHdmitxAnaloDrvCurConfig_t *pstAnalgDrvCurCfg = (HalHdmitxAnaloDrvCurConfig_t *)pCfg;


    memcpy(&pstHdmitxCtxCfg->pstHalCtx->stAnalogDrvCurCfg, pstAnalgDrvCurCfg, sizeof(HalHdmitxAnaloDrvCurConfig_t));
    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, (%x %x %x %x), (%x %x %x %x)\n",
        __FUNCTION__, __LINE__,
        pstHdmitxCtxCfg->pstHalCtx->stAnalogDrvCurCfg.u8DrvCurTap1Ch0,
        pstHdmitxCtxCfg->pstHalCtx->stAnalogDrvCurCfg.u8DrvCurTap1Ch1,
        pstHdmitxCtxCfg->pstHalCtx->stAnalogDrvCurCfg.u8DrvCurTap1Ch2,
        pstHdmitxCtxCfg->pstHalCtx->stAnalogDrvCurCfg.u8DrvCurTap1Ch3,
        pstHdmitxCtxCfg->pstHalCtx->stAnalogDrvCurCfg.u8DrvCurTap2Ch0,
        pstHdmitxCtxCfg->pstHalCtx->stAnalogDrvCurCfg.u8DrvCurTap2Ch1,
        pstHdmitxCtxCfg->pstHalCtx->stAnalogDrvCurCfg.u8DrvCurTap2Ch2,
        pstHdmitxCtxCfg->pstHalCtx->stAnalogDrvCurCfg.u8DrvCurTap2Ch3);

    return enRet;
}

void _HalHdmitxIfSetAnalogDrvCurConfig(void *pCtx, void *pCfg)
{
    HalHdmitxAnaloDrvCurConfig_t *pstAnalgDrvCurCfg = (HalHdmitxAnaloDrvCurConfig_t *)pCfg;
    HDMITX_ANALOG_DRV_CUR_CONFIG stAnalogCfg;

    memset(&stAnalogCfg, 0, sizeof(HDMITX_ANALOG_DRV_CUR_CONFIG));
    stAnalogCfg.u8DrvCurTap1Ch0 = pstAnalgDrvCurCfg->u8DrvCurTap1Ch0;
    stAnalogCfg.u8DrvCurTap1Ch1 = pstAnalgDrvCurCfg->u8DrvCurTap1Ch1;
    stAnalogCfg.u8DrvCurTap1Ch2 = pstAnalgDrvCurCfg->u8DrvCurTap1Ch2;
    stAnalogCfg.u8DrvCurTap1Ch3 = pstAnalgDrvCurCfg->u8DrvCurTap1Ch3;

    stAnalogCfg.u8DrvCurTap2Ch0 = pstAnalgDrvCurCfg->u8DrvCurTap2Ch0;
    stAnalogCfg.u8DrvCurTap2Ch1 = pstAnalgDrvCurCfg->u8DrvCurTap2Ch1;
    stAnalogCfg.u8DrvCurTap2Ch2 = pstAnalgDrvCurCfg->u8DrvCurTap2Ch2;
    stAnalogCfg.u8DrvCurTap2Ch3 = pstAnalgDrvCurCfg->u8DrvCurTap2Ch3;

    MApi_HDMITX_SetAnalogDrvCur(&stAnalogCfg);
}

//------------------------------------------------------------------------------
// Sink Info
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoSinkInfoConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    HalHdmitxSinkInfoConfig_t *pstHalSinkInfCfg = (HalHdmitxSinkInfoConfig_t *)pCfg;

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, enType: %s\n",
        __FUNCTION__, __LINE__, PARSING_HAL_SINK_TYPE(pstHalSinkInfCfg->enType));

    switch(pstHalSinkInfCfg->enType)
    {
        case E_HAL_HDMITX_SINK_INFO_EDID_DATA:
        {
            HalHdmitxSinkEdidDataConfig_t *pstEdidData = &pstHalSinkInfCfg->stInfoUnit.stEdidData;

            if(MApi_HDMITx_GetEDIDData(pstEdidData->au8EdidData, pstEdidData->u8BlockId) == 0)
            {
                if( __HalHdmitxIfCheckEdidRdy())
                {
                    if(MApi_HDMITx_GetEDIDData(pstEdidData->au8EdidData, pstEdidData->u8BlockId) == 0)
                    {
                        enRet = E_HAL_HDMITX_QUERY_RET_ERR;
                        HDMITX_ERR("%s %d, Get Edid Data Fail\n", __FUNCTION__, __LINE__);
                    }
                }
                else
                {
                    enRet = E_HAL_HDMITX_QUERY_RET_ERR;
                    HDMITX_ERR("%s %d, Check Edid Data Fail\n", __FUNCTION__, __LINE__);
                }
            }
            break;
        }

        case E_HAL_HDMITX_SINK_INFO_COLOR_FORMAT:
        {
            HalHdmitxSinkColorFormatConfig_t *pstSinkColorCfg = &pstHalSinkInfCfg->stInfoUnit.stColoFmt;
            HDMITX_VIDEO_TIMING timing = __HalHdmitxIfTransTimingRes(pstSinkColorCfg->enTiming);
            HDMITX_EDID_COLOR_FORMAT eColorFmt = 0;

            if(MApi_HDMITx_GetColorFormatFromEDID(timing, &eColorFmt) == 0)
            {
                if( __HalHdmitxIfCheckEdidRdy())
                {
                    if(MApi_HDMITx_GetColorFormatFromEDID(timing, &eColorFmt) == 0)
                    {
                        enRet = E_HAL_HDMITX_QUERY_RET_ERR;
                        HDMITX_ERR("%s %d, Get Color FmtFail\n", __FUNCTION__, __LINE__);
                    }
                }
                else
                {
                    enRet = E_HAL_HDMITX_QUERY_RET_ERR;
                    HDMITX_ERR("%s %d, Check Edid Data Fail\n", __FUNCTION__, __LINE__);
                }
            }

            if(enRet == E_HAL_HDMITX_QUERY_RET_OK)
            {
                pstSinkColorCfg->enColor = 0;

                if(eColorFmt & ( 1<< HDMITX_EDID_Color_RGB_444))
                {
                    pstSinkColorCfg->enColor |= E_HAL_HDMITX_COLOR_RGB444;
                }

                if(eColorFmt & ( 1<< HDMITX_EDID_Color_YCbCr_444))
                {
                    pstSinkColorCfg->enColor |= E_HAL_HDMITX_COLOR_YUV444;
                }

                if(eColorFmt & ( 1<< HDMITX_EDID_Color_YCbCr_422))
                {
                    pstSinkColorCfg->enColor |= E_HAL_HDMITX_COLOR_YUV422;
                }

                if(eColorFmt & ( 1<< HDMITX_EDID_Color_YCbCr_420))
                {
                    pstSinkColorCfg->enColor |=  E_HAL_HDMITX_COLOR_YUV420;
                }
            }
            break;
        }

        case E_HAL_HDMITX_SINK_INFO_HDMI_SUPPORT:
        {
            HalHdmitxSinkSupportHdmiConfig_t *pstHdmiSupportedCfg = &pstHalSinkInfCfg->stInfoUnit.stSupportedHdmi;
            MS_BOOL  bHdmiSupport = 0;

            if(MApi_HDMITx_EDID_HDMISupport(&bHdmiSupport) == 0)
            {
                if( __HalHdmitxIfCheckEdidRdy())
                {
                    if(MApi_HDMITx_EDID_HDMISupport(&bHdmiSupport) == 0)
                    {
                        enRet = E_HAL_HDMITX_QUERY_RET_ERR;
                        HDMITX_ERR("%s %d, Get HDMI Supported Fail\n", __FUNCTION__, __LINE__);
                    }
                }
                else
                {
                    enRet = E_HAL_HDMITX_QUERY_RET_ERR;
                    HDMITX_ERR("%s %d, Check Edid Data Fail\n", __FUNCTION__, __LINE__);
                }
            }

            if(enRet == E_HAL_HDMITX_QUERY_RET_OK)
            {
                pstHdmiSupportedCfg->bSupported = bHdmiSupport;
            }
            break;
        }

        case E_HAL_HDMITX_SINK_INFO_HPD_STATUS:
            pstHalSinkInfCfg->stInfoUnit.stHpdStatus.bHpd = MApi_HDMITx_GetRxStatus();
            break;

        default:
            enRet = E_HAL_HDMITX_QUERY_RET_CFGERR;
            HDMITX_ERR("%s %d, Unkonw Type %d\n", __FUNCTION__, __LINE__, pstHalSinkInfCfg->enType);
            break;
    }

    return enRet;
}

void _HalHdmitxIfSetSinkInfoConfig(void *pCtx, void *pCfg)
{

}

//------------------------------------------------------------------------------
// Info Frame
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoInfoFrameConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;
    HalHdmitxInfoFrameConfig_t *pstInfoFrameCfg = (HalHdmitxInfoFrameConfig_t *)pCfg;

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, enType:%s, En:%d\n",
        __FUNCTION__, __LINE__,
        PARSING_HAL_INFOFRAME_TYPE(pstInfoFrameCfg->enType),
        pstInfoFrameCfg->bEn);

    if(pstInfoFrameCfg->enType == E_HAL_HDMITX_INFOFRAM_TYPE_MAX)
    {
        enRet = E_HAL_HDMITX_QUERY_RET_CFGERR;
        HDMITX_ERR("%s %d, enType is Unknown %d", __FUNCTION__, __LINE__, pstInfoFrameCfg->enType);
    }
    else
    {
        u8 u8DataLen;

        u8DataLen = pstInfoFrameCfg->enType == E_HAL_HDMITX_INFOFRAM_TYPE_AVI   ? HAL_HDMITX_IF_AVI_INFOFRAME_DATA_SIZE :
                    pstInfoFrameCfg->enType == E_HAL_HDMITX_INFOFRAM_TYPE_SPD   ? HAL_HDMITX_IF_SPD_INFOFRAME_DATA_SIZE :
                    pstInfoFrameCfg->enType == E_HAL_HDMITX_INFOFRAM_TYPE_AUDIO ? HAL_HDMITX_IF_AUD_INFOFRAME_DATA_SIZE :
                                                                                  0;

        if(pstInfoFrameCfg->u8DataLen > u8DataLen)
        {
            enRet = E_HAL_HDMITX_QUERY_RET_CFGERR;
            HDMITX_ERR("%s %d, %s InfoFrame Data Len is not mathc %d != %d",
                __FUNCTION__, __LINE__,
                PARSING_HAL_INFOFRAME_TYPE(pstInfoFrameCfg->enType),
                pstInfoFrameCfg->u8DataLen, u8DataLen);
        }
    }
    return enRet;
}

void _HalHdmitxIfSetInfoFrameConfig(void *pCtx, void *pCfg)
{
    HalHdmitxInfoFrameConfig_t *pstInfoFrameCfg = (HalHdmitxInfoFrameConfig_t *)pCfg;
    HDMITX_PACKET_TYPE enPacketTyep;

    enPacketTyep = pstInfoFrameCfg->enType == E_HAL_HDMITX_INFOFRAM_TYPE_AVI ? HDMITX_AVI_INFOFRAME :
                   pstInfoFrameCfg->enType == E_HAL_HDMITX_INFOFRAM_TYPE_SPD ? HDMITX_SPD_INFOFRAME :
                                                                               HDMITX_AUDIO_INFOFRAME;
    if(pstInfoFrameCfg->bEn)
    {
        if(MApi_HDMITx_PKT_Content_Define(enPacketTyep, pstInfoFrameCfg->au8Data, pstInfoFrameCfg->u8DataLen) == 0)
        {
            HDMITX_ERR("%s %d, Set %s Info Fail\n",
                __FUNCTION__, __LINE__, PARSING_HAL_INFOFRAME_TYPE(pstInfoFrameCfg->enType));
        }
        else
        {
            MApi_HDMITx_PKT_User_Define(enPacketTyep, 1, HDMITX_CYCLIC_PACKET, 1);
        }
    }
    else
    {
        MApi_HDMITx_PKT_User_Define(enPacketTyep, 0, HDMITX_CYCLIC_PACKET, 1);
    }
}

//------------------------------------------------------------------------------
// Debug Level
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetInfoDebugLevelConfig(void *pCtx, void *pCfg)
{
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;


    return enRet;
}

void _HalHdmitxIfSetDebugLevelConfig(void *pCtx, void *pCfg)
{
    u32 *pu32DebugLevel = (u32 *)pCfg;
    u16 u16DbgLevel;

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, DbgLevel:%x\n", __FUNCTION__, __LINE__, *pu32DebugLevel);
    u16DbgLevel = *pu32DebugLevel & 0xFFFF;
    MApi_HDMITx_SetDbgLevel(u16DbgLevel);
}

//------------------------------------------------------------------------------
// Hpd
//------------------------------------------------------------------------------
HalHdmitxQueryRet_e _HalHdmitxIfGetHpdConfig(void *pCtx, void *pCfg)
{
    DrvHdmitxCtxConfig_t *pstHdmitxCtx = (DrvHdmitxCtxConfig_t *)pCtx;
    HalHdmitxHpdConfig_t *pstHpdCfg = (HalHdmitxHpdConfig_t *)pCfg;
    HalHdmitxQueryRet_e enRet = E_HAL_HDMITX_QUERY_RET_OK;

    memcpy(&pstHdmitxCtx->pstHalCtx->stHpdCfg, pstHpdCfg, sizeof(HalHdmitxHpdConfig_t));

    HDMITX_MSG(HDMITX_DBG_LEVEL_HAL_IF, "%s %d, Hpd GpioNum=%d\n",
        __FUNCTION__, __LINE__,
        pstHdmitxCtx->pstHalCtx->stHpdCfg.u8GpioNum);
    return enRet;
}

void _HalHdmitxIfSetHpdConfig(void *pCtx, void *pCfg)
{
    HalHdmitxHpdConfig_t *pstHpdCfg = (HalHdmitxHpdConfig_t *)pCfg;

    MApi_HDMITx_SetHPDGpioPin(pstHpdCfg->u8GpioNum);
}

//------------------------------------------------------------------------------
// Call Back
//------------------------------------------------------------------------------
bool _HalHdmitxIfGetCallBack(DrvHdmitxCtxConfig_t *pstHdmitxCfg, HalHdmitxQueryConfig_t *pstQueryCfg)
{
    memset(&pstQueryCfg->stOutCfg, 0, sizeof(HalHdmitxQueryOutConfig_t));

    if(pstQueryCfg->stInCfg.u32CfgSize != gpHdmitxCbTbl[pstQueryCfg->stInCfg.enQueryType].u16CfgSize)
    {
        pstQueryCfg->stOutCfg.enQueryRet = E_HAL_HDMITX_QUERY_RET_CFGERR;
        HDMITX_ERR("%s %d, Size %d != %d\n",
            __FUNCTION__, __LINE__,
            pstQueryCfg->stInCfg.u32CfgSize,
            gpHdmitxCbTbl[pstQueryCfg->stInCfg.enQueryType].u16CfgSize);
    }
    else
    {
        pstQueryCfg->stOutCfg.pSetFunc = gpHdmitxCbTbl[pstQueryCfg->stInCfg.enQueryType].pSetFunc;

        if(pstQueryCfg->stOutCfg.pSetFunc == NULL)
        {
            pstQueryCfg->stOutCfg.enQueryRet = E_HAL_HDMITX_QUERY_RET_NOTSUPPORT;
        }
        else
        {
            pstQueryCfg->stOutCfg.enQueryRet =
                gpHdmitxCbTbl[pstQueryCfg->stInCfg.enQueryType].pGetInfoFunc(pstHdmitxCfg, pstQueryCfg->stInCfg.pInCfg);

        }
    }

    return 1;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool HalHdmitxIfInit(void)
{
    if(gbHalIfInit)
    {
        return 1;
    }

    memset(gpHdmitxCbTbl, 0, sizeof(HalHdmitxQueryCallBackFunc_t)* E_HAL_HDMITX_QUERY_MAX);
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_INIT].pSetFunc     = _HalHdmitxIfSetInitConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_INIT].pGetInfoFunc = _HalHdmitxIfGetInfoInitConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_INIT].u16CfgSize   = 0;

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_DEINIT].pSetFunc     = _HalHdmitxIfSetDeInitConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_DEINIT].pGetInfoFunc = _HalHdmitxIfGetInfoDeInitConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_DEINIT].u16CfgSize   = 0;

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ATTR_BEGIN].pSetFunc     = _HalHdmitxIfSetAttrBeingConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ATTR_BEGIN].pGetInfoFunc = _HalHdmitxIfGetInfoAttrBeginConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ATTR_BEGIN].u16CfgSize   = 0;

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ATTR].pSetFunc     = _HalHdmitxIfSetAttrConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ATTR].pGetInfoFunc = _HalHdmitxIfGetInfoAttrConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ATTR].u16CfgSize   =  sizeof(HalHdmitxAttrConfig_t);

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ATTR_END].pSetFunc     = _HalHdmitxIfSetAttrEndConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ATTR_END].pGetInfoFunc = _HalHdmitxIfGetInfoAttrEndConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ATTR_END].u16CfgSize   =  0;

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_SIGNAL].pSetFunc     = _HalHdmitxIfSetSignalConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_SIGNAL].pGetInfoFunc = _HalHdmitxIfGetInfoSignalConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_SIGNAL].u16CfgSize   =  sizeof(HalHdmitxSignalConfig_t);

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_MUTE].pSetFunc     = _HalHdmitxIfSetMuteConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_MUTE].pGetInfoFunc = _HalHdmitxIfGetInfoMuteConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_MUTE].u16CfgSize   =  sizeof(HalHdmitxMuteConfig_t);

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ANALOG_DRV_CUR].pSetFunc     = _HalHdmitxIfSetAnalogDrvCurConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ANALOG_DRV_CUR].pGetInfoFunc = _HalHdmitxIfGetInfoAnalogDrvCurConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_ANALOG_DRV_CUR].u16CfgSize   =  sizeof(HalHdmitxAnaloDrvCurConfig_t);

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_SINK_INFO].pSetFunc     = _HalHdmitxIfSetSinkInfoConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_SINK_INFO].pGetInfoFunc = _HalHdmitxIfGetInfoSinkInfoConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_SINK_INFO].u16CfgSize   =  sizeof(HalHdmitxSinkInfoConfig_t);

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_INFO_FRAME].pSetFunc     = _HalHdmitxIfSetInfoFrameConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_INFO_FRAME].pGetInfoFunc = _HalHdmitxIfGetInfoInfoFrameConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_INFO_FRAME].u16CfgSize   =  sizeof(HalHdmitxInfoFrameConfig_t);


    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_DEBUG_LEVEL].pSetFunc     = _HalHdmitxIfSetDebugLevelConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_DEBUG_LEVEL].pGetInfoFunc = _HalHdmitxIfGetInfoDebugLevelConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_DEBUG_LEVEL].u16CfgSize   =  sizeof(u32);

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_HPD].pSetFunc     = _HalHdmitxIfSetHpdConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_HPD].pGetInfoFunc = _HalHdmitxIfGetHpdConfig;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_HPD].u16CfgSize   =  sizeof(HalHdmitxHpdConfig_t);

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_CLK_SET].pSetFunc     = _HalHdmitxIfSetClkSet;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_CLK_SET].pGetInfoFunc = _HalHdmitxIfGetInfoClkSet;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_CLK_SET].u16CfgSize   =  sizeof(HalHdmitxClkConfig_t);

    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_CLK_GET].pSetFunc     = _HalHdmitxIfSetClkGet;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_CLK_GET].pGetInfoFunc = _HalHdmitxIfGetInfoClkGet;
    gpHdmitxCbTbl[E_HAL_HDMITX_QUERY_CLK_GET].u16CfgSize   =  sizeof(HalHdmitxClkConfig_t);

    gbHalIfInit = 1;
    return 1;
}

bool HalHdmitxIfDeInit(void)
{
    if(gbHalIfInit == 0)
    {
        HDMITX_ERR("%s %d, HalIf not init\n", __FUNCTION__, __LINE__);
        return 0;
    }
    gbHalIfInit = 0;
    memset(gpHdmitxCbTbl, 0, sizeof(HalHdmitxQueryCallBackFunc_t)* E_HAL_HDMITX_QUERY_MAX);

    return 1;
}

bool HalHdmitxIfQuery(void *pCtx,  void *pCfg)
{
    bool bRet = 1;

    if(pCtx == NULL || pCfg == NULL)
    {
        HDMITX_ERR("%s %d, Input Param is Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = _HalHdmitxIfGetCallBack((DrvHdmitxCtxConfig_t *)pCtx, (HalHdmitxQueryConfig_t *)pCfg);
    }

    return bRet;
}

