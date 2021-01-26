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

#define _DRV_PNL_IF_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_pnl_os.h"
#include "hal_pnl_common.h"
#include "pnl_debug.h"
#include "mhal_pnl_datatype.h"
#include "hal_pnl_chip.h"
#include "hal_pnl_st.h"
#include "hal_pnl_if.h"
#include "drv_pnl_ctx.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
u32 _gu32PnlDbgLevel = 0;


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
void _DrvPnlIfTransMipiDsiConfigToHal(MhalPnlMipiDsiConfig_t *pstMhalCfg, HalPnlMipiDsiConfig_t *pstHalCfg)
{
    pstHalCfg->u8HsTrail   = pstMhalCfg->u8HsTrail;
    pstHalCfg->u8HsPrpr    = pstMhalCfg->u8HsPrpr;
    pstHalCfg->u8HsZero    = pstMhalCfg->u8HsZero;
    pstHalCfg->u8ClkHsPrpr = pstMhalCfg->u8ClkHsPrpr;
    pstHalCfg->u8ClkHsExit = pstMhalCfg->u8ClkHsExit;
    pstHalCfg->u8ClkTrail  = pstMhalCfg->u8ClkTrail;
    pstHalCfg->u8ClkZero   = pstMhalCfg->u8ClkZero;
    pstHalCfg->u8ClkHsPost = pstMhalCfg->u8ClkHsPost;
    pstHalCfg->u8DaHsExit  = pstMhalCfg->u8DaHsExit;
    pstHalCfg->u8ContDet   = pstMhalCfg->u8ContDet;
    pstHalCfg->u8Lpx       = pstMhalCfg->u8Lpx;
    pstHalCfg->u8TaGet     = pstMhalCfg->u8TaGet;
    pstHalCfg->u8TaSure    = pstMhalCfg->u8TaSure;
    pstHalCfg->u8TaGo      = pstMhalCfg->u8TaGo;
    pstHalCfg->u16Hactive  = pstMhalCfg->u16Hactive;
    pstHalCfg->u16Hpw      = pstMhalCfg->u16Hpw;
    pstHalCfg->u16Hbp      = pstMhalCfg->u16Hbp;
    pstHalCfg->u16Hfp      = pstMhalCfg->u16Hfp;
    pstHalCfg->u16Vactive  = pstMhalCfg->u16Vactive;
    pstHalCfg->u16Vpw      = pstMhalCfg->u16Vpw;
    pstHalCfg->u16Vbp      = pstMhalCfg->u16Vbp;
    pstHalCfg->u16Vfp      = pstMhalCfg->u16Vfp;
    pstHalCfg->u16Bllp     = pstMhalCfg->u16Bllp;
    pstHalCfg->u16Fps      = pstMhalCfg->u16Fps;

    pstHalCfg->enLaneNum   = pstMhalCfg->enLaneNum == E_MHAL_PNL_MIPI_DSI_LANE_1 ? E_HAL_PNL_MIPI_DSI_LANE_1 :
                             pstMhalCfg->enLaneNum == E_MHAL_PNL_MIPI_DSI_LANE_2 ? E_HAL_PNL_MIPI_DSI_LANE_2 :
                             pstMhalCfg->enLaneNum == E_MHAL_PNL_MIPI_DSI_LANE_3 ? E_HAL_PNL_MIPI_DSI_LANE_3 :
                             pstMhalCfg->enLaneNum == E_MHAL_PNL_MIPI_DSI_LANE_4 ? E_HAL_PNL_MIPI_DSI_LANE_4 :
                                                                                   E_HAL_PNL_MIPI_DSI_LANE_NONE;

    pstHalCfg->enFormat    = pstMhalCfg->enFormat  == E_MHAL_PNL_MIPI_DSI_RGB565         ? E_HAL_PNL_MIPI_DSI_RGB565 :
                             pstMhalCfg->enFormat  == E_MHAL_PNL_MIPI_DSI_RGB666         ? E_HAL_PNL_MIPI_DSI_RGB666 :
                             pstMhalCfg->enFormat  == E_MHAL_PNL_MIPI_DSI_LOOSELY_RGB666 ? E_HAL_PNL_MIPI_DSI_LOOSELY_RGB666 :
                                                                                           E_HAL_PNL_MIPI_DSI_RGB888;

    pstHalCfg->enCtrl      = pstMhalCfg->enCtrl == E_MHAL_PNL_MIPI_DSI_SYNC_PULSE ? E_HAL_PNL_MIPI_DSI_SYNC_PULSE :
                             pstMhalCfg->enCtrl == E_MHAL_PNL_MIPI_DSI_SYNC_EVENT ? E_HAL_PNL_MIPI_DSI_SYNC_EVENT :
                             pstMhalCfg->enCtrl == E_MHAL_PNL_MIPI_DSI_BURST_MODE ? E_HAL_PNL_MIPI_DSI_BURST_MODE :
                                                                                    E_HAL_PNL_MIPI_DSI_CMD_MODE;

    pstHalCfg->pu8CmdBuf       = pstMhalCfg->pu8CmdBuf;
    pstHalCfg->u32CmdBufSize   = pstMhalCfg->u32CmdBufSize;
    pstHalCfg->u8SyncCalibrate = pstMhalCfg->u8SyncCalibrate;
    pstHalCfg->u16VirHsyncSt   = pstMhalCfg->u16VirHsyncSt;
    pstHalCfg->u16VirHsyncEnd  = pstMhalCfg->u16VirHsyncEnd;
    pstHalCfg->u16VsyncRef     = pstMhalCfg->u16VsyncRef;
    pstHalCfg->u16DataClkSkew  = pstMhalCfg->u16DataClkSkew;

    pstHalCfg->u8PolCh0 = pstMhalCfg->u8PolCh0;
    pstHalCfg->u8PolCh1 = pstMhalCfg->u8PolCh1;
    pstHalCfg->u8PolCh2 = pstMhalCfg->u8PolCh2;
    pstHalCfg->u8PolCh3 = pstMhalCfg->u8PolCh3;
    pstHalCfg->u8PolCh4 = pstMhalCfg->u8PolCh4;

    pstHalCfg->enPacketType =
        pstMhalCfg->enPacketType == E_MHAL_PNL_MIPI_DSI_PACKET_TYPE_GENERIC ? E_HAL_PNL_MIPI_DSI_PACKET_TYPE_GENERIC :
                                                                              E_MHAL_PNL_MIPI_DSI_PACKET_TYPE_DCS;

}

void _DrvPnlIfTransMipiDsiConfigToMHal(MhalPnlMipiDsiConfig_t *pstMhalCfg, HalPnlMipiDsiConfig_t *pstHalCfg)
{
    pstMhalCfg->u8HsTrail   = pstHalCfg->u8HsTrail;
    pstMhalCfg->u8HsPrpr    = pstHalCfg->u8HsPrpr;
    pstMhalCfg->u8HsZero    = pstHalCfg->u8HsZero;
    pstMhalCfg->u8ClkHsPrpr = pstHalCfg->u8ClkHsPrpr;
    pstMhalCfg->u8ClkHsExit = pstHalCfg->u8ClkHsExit;
    pstMhalCfg->u8ClkTrail  = pstHalCfg->u8ClkTrail;
    pstMhalCfg->u8ClkZero   = pstHalCfg->u8ClkZero;
    pstMhalCfg->u8ClkHsPost = pstHalCfg->u8ClkHsPost;
    pstMhalCfg->u8DaHsExit  = pstHalCfg->u8DaHsExit;
    pstMhalCfg->u8ContDet   = pstHalCfg->u8ContDet;
    pstMhalCfg->u8Lpx       = pstHalCfg->u8Lpx;
    pstMhalCfg->u8TaGet     = pstHalCfg->u8TaGet;
    pstMhalCfg->u8TaSure    = pstHalCfg->u8TaSure;
    pstMhalCfg->u8TaGo      = pstHalCfg->u8TaGo;
    pstMhalCfg->u16Hactive  = pstHalCfg->u16Hactive;
    pstMhalCfg->u16Hpw      = pstHalCfg->u16Hpw;
    pstMhalCfg->u16Hbp      = pstHalCfg->u16Hbp;
    pstMhalCfg->u16Hfp      = pstHalCfg->u16Hfp;
    pstMhalCfg->u16Vactive  = pstHalCfg->u16Vactive;
    pstMhalCfg->u16Vpw      = pstHalCfg->u16Vpw;
    pstMhalCfg->u16Vbp      = pstHalCfg->u16Vbp;
    pstMhalCfg->u16Vfp      = pstHalCfg->u16Vfp;
    pstMhalCfg->u16Bllp     = pstHalCfg->u16Bllp;
    pstMhalCfg->u16Fps      = pstHalCfg->u16Fps;

    pstMhalCfg->enLaneNum   = pstHalCfg->enLaneNum == E_HAL_PNL_MIPI_DSI_LANE_1 ? E_MHAL_PNL_MIPI_DSI_LANE_1 :
                              pstHalCfg->enLaneNum == E_HAL_PNL_MIPI_DSI_LANE_2 ? E_MHAL_PNL_MIPI_DSI_LANE_2 :
                              pstHalCfg->enLaneNum == E_HAL_PNL_MIPI_DSI_LANE_3 ? E_MHAL_PNL_MIPI_DSI_LANE_3 :
                              pstHalCfg->enLaneNum == E_HAL_PNL_MIPI_DSI_LANE_4 ? E_MHAL_PNL_MIPI_DSI_LANE_4 :
                                                                                  E_MHAL_PNL_MIPI_DSI_LANE_NONE;

    pstMhalCfg->enFormat    = pstHalCfg->enFormat  == E_HAL_PNL_MIPI_DSI_RGB565         ? E_MHAL_PNL_MIPI_DSI_RGB565 :
                              pstHalCfg->enFormat  == E_HAL_PNL_MIPI_DSI_RGB666         ? E_MHAL_PNL_MIPI_DSI_RGB666 :
                              pstHalCfg->enFormat  == E_HAL_PNL_MIPI_DSI_LOOSELY_RGB666 ? E_MHAL_PNL_MIPI_DSI_LOOSELY_RGB666 :
                                                                                          E_MHAL_PNL_MIPI_DSI_RGB888;

    pstMhalCfg->enCtrl      = pstHalCfg->enCtrl == E_HAL_PNL_MIPI_DSI_SYNC_PULSE ? E_MHAL_PNL_MIPI_DSI_SYNC_PULSE :
                              pstHalCfg->enCtrl == E_HAL_PNL_MIPI_DSI_SYNC_EVENT ? E_MHAL_PNL_MIPI_DSI_SYNC_EVENT :
                              pstHalCfg->enCtrl == E_HAL_PNL_MIPI_DSI_BURST_MODE ? E_MHAL_PNL_MIPI_DSI_BURST_MODE :
                                                                                   E_MHAL_PNL_MIPI_DSI_CMD_MODE;

    pstMhalCfg->pu8CmdBuf       = pstHalCfg->pu8CmdBuf;
    pstMhalCfg->u32CmdBufSize   = pstHalCfg->u32CmdBufSize;
    pstMhalCfg->u8SyncCalibrate = pstHalCfg->u8SyncCalibrate;
    pstMhalCfg->u16VirHsyncSt   = pstHalCfg->u16VirHsyncSt;
    pstMhalCfg->u16VirHsyncEnd  = pstHalCfg->u16VirHsyncEnd;
    pstMhalCfg->u16VsyncRef     = pstHalCfg->u16VsyncRef;
    pstMhalCfg->u16DataClkSkew  = pstHalCfg->u16DataClkSkew;

    pstMhalCfg->u8PolCh0 = pstHalCfg->u8PolCh0;
    pstMhalCfg->u8PolCh1 = pstHalCfg->u8PolCh1;
    pstMhalCfg->u8PolCh2 = pstHalCfg->u8PolCh2;
    pstMhalCfg->u8PolCh3 = pstHalCfg->u8PolCh3;
    pstMhalCfg->u8PolCh4 = pstHalCfg->u8PolCh4;

    pstMhalCfg->enPacketType =
        pstHalCfg->enPacketType == E_HAL_PNL_MIPI_DSI_PACKET_TYPE_GENERIC ? E_MHAL_PNL_MIPI_DSI_PACKET_TYPE_GENERIC :
                                                                            E_MHAL_PNL_MIPI_DSI_PACKET_TYPE_DCS;
}



void _DrvPnlIfTransParamToHal(MhalPnlParamConfig_t *pstMhalCfg, HalPnlParamConfig_t *pstHalCfg)
{
    pstHalCfg->u8Dither  = pstMhalCfg->u8Dither;
    pstHalCfg->eLinkType = pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_LVDS             ?  E_HAL_PNL_LINK_LVDS             :
                           pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_RSDS             ?  E_HAL_PNL_LINK_RSDS             :
                           pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_MINILVDS         ?  E_HAL_PNL_LINK_MINILVDS         :
                           pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_ANALOG_MINILVDS  ?  E_HAL_PNL_LINK_ANALOG_MINILVDS  :
                           pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_DIGITAL_MINILVDS ?  E_HAL_PNL_LINK_DIGITAL_MINILVDS :
                           pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_MFC              ?  E_HAL_PNL_LINK_MFC              :
                           pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_DAC_I            ?  E_HAL_PNL_LINK_DAC_I            :
                           pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_DAC_P            ?  E_HAL_PNL_LINK_DAC_P            :
                           pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_PDPLVDS          ?  E_HAL_PNL_LINK_PDPLVDS          :
                           pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_EXT              ?  E_HAL_PNL_LINK_EXT              :
                           pstMhalCfg->eLinkType == E_MHAL_PNL_LINK_MIPI_DSI         ?  E_HAL_PNL_LINK_MIPI_DSI         :
                                                                                        E_HAL_PNL_LINK_TTL;


    pstHalCfg->u8DualPort      =  pstMhalCfg->u8DualPort;
    pstHalCfg->u8SwapPort      =  pstMhalCfg->u8SwapPort;
    pstHalCfg->u8SwapOdd_ML    =  pstMhalCfg->u8SwapOdd_ML;
    pstHalCfg->u8SwapEven_ML   =  pstMhalCfg->u8SwapEven_ML;
    pstHalCfg->u8SwapOdd_RB    =  pstMhalCfg->u8SwapOdd_RB;
    pstHalCfg->u8SwapEven_RB   =  pstMhalCfg->u8SwapEven_RB;
    pstHalCfg->u8SwapLVDS_POL  =  pstMhalCfg->u8SwapLVDS_POL;
    pstHalCfg->u8SwapLVDS_CH   =  pstMhalCfg->u8SwapLVDS_CH;
    pstHalCfg->u8PDP10BIT      =  pstMhalCfg->u8PDP10BIT;
    pstHalCfg->u8LVDS_TI_MODE  =  pstMhalCfg->u8LVDS_TI_MODE;

    pstHalCfg->u8DCLKDelay       = pstMhalCfg->u8DCLKDelay;
    pstHalCfg->u8InvDCLK         = pstMhalCfg->u8InvDCLK;
    pstHalCfg->u8InvDE           = pstMhalCfg->u8InvDE;
    pstHalCfg->u8InvHSync        = pstMhalCfg->u8InvHSync;
    pstHalCfg->u8InvVSync        = pstMhalCfg->u8InvVSync;
    pstHalCfg->u8DCKLCurrent     = pstMhalCfg->u8DCKLCurrent;
    pstHalCfg->u8DECurrent       = pstMhalCfg->u8DECurrent;
    pstHalCfg->u8ODDDataCurrent  = pstMhalCfg->u8ODDDataCurrent;
    pstHalCfg->u8EvenDataCurrent = pstMhalCfg->u8EvenDataCurrent;
    pstHalCfg->u16OnTiming1      = pstMhalCfg->u16OnTiming1;
    pstHalCfg->u16OnTiming2      = pstMhalCfg->u16OnTiming2;
    pstHalCfg->u16OffTiming1     = pstMhalCfg->u16OffTiming1;

    pstHalCfg->u16HSyncWidth     = pstMhalCfg->u16HSyncWidth;
    pstHalCfg->u16HSyncBackPorch = pstMhalCfg->u16HSyncBackPorch;
    pstHalCfg->u16VSyncWidth     = pstMhalCfg->u16VSyncWidth;
    pstHalCfg->u16VSyncBackPorch = pstMhalCfg->u16VSyncBackPorch;
    pstHalCfg->u16HStart         = pstMhalCfg->u16HStart;
    pstHalCfg->u16VStart         = pstMhalCfg->u16VStart;
    pstHalCfg->u16Width          = pstMhalCfg->u16Width;
    pstHalCfg->u16Height         = pstMhalCfg->u16Height;
    pstHalCfg->u16MaxHTotal      = pstMhalCfg->u16MaxHTotal;
    pstHalCfg->u16HTotal         = pstMhalCfg->u16HTotal;
    pstHalCfg->u16MinHTotal      = pstMhalCfg->u16MinHTotal;
    pstHalCfg->u16MaxVTotal      = pstMhalCfg->u16MaxVTotal;
    pstHalCfg->u16VTotal         = pstMhalCfg->u16VTotal;
    pstHalCfg->u16MinVTotal      = pstMhalCfg->u16MinVTotal;
    pstHalCfg->u16MaxDCLK        = pstMhalCfg->u16MaxDCLK;
    pstHalCfg->u16DCLK           = pstMhalCfg->u16DCLK;
    pstHalCfg->u16MinDCLK        = pstMhalCfg->u16MinDCLK;

    pstHalCfg->u16SpreadSpectrumStep = pstMhalCfg->u16SpreadSpectrumStep;
    pstHalCfg->u16SpreadSpectrumSpan = pstMhalCfg->u16SpreadSpectrumSpan;

    pstHalCfg->u8PwmPeriodL          = pstMhalCfg->u8PwmPeriodL;
    pstHalCfg->u8PwmPeriodH          = pstMhalCfg->u8PwmPeriodH;
    pstHalCfg->u8PwmDuty             = pstMhalCfg->u8PwmDuty;

    pstHalCfg->ePanelAspectRatio     = pstMhalCfg->ePanelAspectRatio == E_MHAL_PNL_ASPECT_RATIO_4_3  ? E_HAL_PNL_ASPECT_RATIO_4_3 :
                                       pstMhalCfg->ePanelAspectRatio == E_MHAL_PNL_ASPECT_RATIO_WIDE ? E_HAL_PNL_ASPECT_RATIO_WIDE :
                                                                                                       E_HAL_PNL_ASPECT_RATIO_OTHER;

    pstHalCfg->u16LVDSTxSwapValue   = pstMhalCfg->u16LVDSTxSwapValue;
    pstHalCfg->eTiBitMode           = pstMhalCfg->eTiBitMode == E_MHAL_PNL_TI_10BIT_MODE ? E_HAL_PNL_TI_10BIT_MODE :
                                      pstMhalCfg->eTiBitMode == E_MHAL_PNL_TI_8BIT_MODE  ? E_HAL_PNL_TI_8BIT_MODE :
                                                                                           E_HAL_PNL_TI_6BIT_MODE;

    pstHalCfg->eOutputFormatBitMode = pstMhalCfg->eOutputFormatBitMode == E_MHAL_PNL_OUTPUT_10BIT_MODE ? E_HAL_PNL_OUTPUT_10BIT_MODE :
                                      pstMhalCfg->eOutputFormatBitMode == E_MHAL_PNL_OUTPUT_6BIT_MODE  ? E_HAL_PNL_OUTPUT_6BIT_MODE :
                                      pstMhalCfg->eOutputFormatBitMode == E_MHAL_PNL_OUTPUT_8BIT_MODE  ? E_HAL_PNL_OUTPUT_8BIT_MODE :
                                                                                                         E_HAL_PNL_OUTPUT_565BIT_MODE;

    pstHalCfg->u8SwapOdd_RG  = pstMhalCfg->u8SwapOdd_RG == E_MHAL_PNL_RGB_SWAP_R ? E_HAL_PNL_RGB_SWAP_R :
                               pstMhalCfg->u8SwapOdd_RG == E_MHAL_PNL_RGB_SWAP_G ? E_HAL_PNL_RGB_SWAP_G :
                               pstMhalCfg->u8SwapOdd_RG == E_MHAL_PNL_RGB_SWAP_B ? E_HAL_PNL_RGB_SWAP_B :
                                                                                   E_HAL_PNL_RGB_SWAP_R;

    pstHalCfg->u8SwapEven_RG = pstMhalCfg->u8SwapEven_RG == E_MHAL_PNL_RGB_SWAP_R ? E_HAL_PNL_RGB_SWAP_R :
                               pstMhalCfg->u8SwapEven_RG == E_MHAL_PNL_RGB_SWAP_G ? E_HAL_PNL_RGB_SWAP_G :
                               pstMhalCfg->u8SwapEven_RG == E_MHAL_PNL_RGB_SWAP_B ? E_HAL_PNL_RGB_SWAP_B :
                                                                                    E_HAL_PNL_RGB_SWAP_G;

    pstHalCfg->u8SwapOdd_GB  = pstMhalCfg->u8SwapOdd_GB  == E_MHAL_PNL_RGB_SWAP_R ? E_HAL_PNL_RGB_SWAP_R :
                               pstMhalCfg->u8SwapOdd_GB  == E_MHAL_PNL_RGB_SWAP_G ? E_HAL_PNL_RGB_SWAP_G :
                               pstMhalCfg->u8SwapOdd_GB  == E_MHAL_PNL_RGB_SWAP_B ? E_HAL_PNL_RGB_SWAP_B :
                                                                                    E_HAL_PNL_RGB_SWAP_B;

    pstHalCfg->u8SwapEven_GB = pstMhalCfg->u8SwapEven_GB;

    pstHalCfg->u8DoubleClk   = pstMhalCfg->u8DoubleClk;
    pstHalCfg->u32MaxSET     = pstMhalCfg->u32MaxSET;
    pstHalCfg->u32MinSET     = pstMhalCfg->u32MinSET;

    pstHalCfg->eOutTimingMode = pstMhalCfg->eOutTimingMode == E_MHAL_PNL_CHG_DCLK   ? E_HAL_PNL_CHG_DCLK :
                                pstMhalCfg->eOutTimingMode == E_MHAL_PNL_CHG_HTOTAL ? E_HAL_PNL_CHG_HTOTAL :
                                                                                      E_HAL_PNL_CHG_VTOTAL;
    pstHalCfg->u8NoiseDith = pstMhalCfg->u8NoiseDith;

    pstHalCfg->eCh0 = pstMhalCfg->eCh0 == E_MHAL_PNL_CH_SWAP_0 ? E_HAL_PNL_CH_SWAP_0 :
                      pstMhalCfg->eCh0 == E_MHAL_PNL_CH_SWAP_1 ? E_HAL_PNL_CH_SWAP_1 :
                      pstMhalCfg->eCh0 == E_MHAL_PNL_CH_SWAP_2 ? E_HAL_PNL_CH_SWAP_2 :
                      pstMhalCfg->eCh0 == E_MHAL_PNL_CH_SWAP_3 ? E_HAL_PNL_CH_SWAP_3 :
                                                                 E_HAL_PNL_CH_SWAP_4;

    pstHalCfg->eCh1 = pstMhalCfg->eCh1 == E_MHAL_PNL_CH_SWAP_0 ? E_HAL_PNL_CH_SWAP_0 :
                      pstMhalCfg->eCh1 == E_MHAL_PNL_CH_SWAP_1 ? E_HAL_PNL_CH_SWAP_1 :
                      pstMhalCfg->eCh1 == E_MHAL_PNL_CH_SWAP_2 ? E_HAL_PNL_CH_SWAP_2 :
                      pstMhalCfg->eCh1 == E_MHAL_PNL_CH_SWAP_3 ? E_HAL_PNL_CH_SWAP_3 :
                                                                 E_HAL_PNL_CH_SWAP_4;

    pstHalCfg->eCh2 = pstMhalCfg->eCh2 == E_MHAL_PNL_CH_SWAP_0 ? E_HAL_PNL_CH_SWAP_0 :
                      pstMhalCfg->eCh2 == E_MHAL_PNL_CH_SWAP_1 ? E_HAL_PNL_CH_SWAP_1 :
                      pstMhalCfg->eCh2 == E_MHAL_PNL_CH_SWAP_2 ? E_HAL_PNL_CH_SWAP_2 :
                      pstMhalCfg->eCh2 == E_MHAL_PNL_CH_SWAP_3 ? E_HAL_PNL_CH_SWAP_3 :
                                                                 E_HAL_PNL_CH_SWAP_4;

    pstHalCfg->eCh3 = pstMhalCfg->eCh3 == E_MHAL_PNL_CH_SWAP_0 ? E_HAL_PNL_CH_SWAP_0 :
                      pstMhalCfg->eCh3 == E_MHAL_PNL_CH_SWAP_1 ? E_HAL_PNL_CH_SWAP_1 :
                      pstMhalCfg->eCh3 == E_MHAL_PNL_CH_SWAP_2 ? E_HAL_PNL_CH_SWAP_2 :
                      pstMhalCfg->eCh3 == E_MHAL_PNL_CH_SWAP_3 ? E_HAL_PNL_CH_SWAP_3 :
                                                                 E_HAL_PNL_CH_SWAP_4;

    pstHalCfg->eCh4 = pstMhalCfg->eCh4 == E_MHAL_PNL_CH_SWAP_0 ? E_HAL_PNL_CH_SWAP_0 :
                      pstMhalCfg->eCh4 == E_MHAL_PNL_CH_SWAP_1 ? E_HAL_PNL_CH_SWAP_1 :
                      pstMhalCfg->eCh4 == E_MHAL_PNL_CH_SWAP_2 ? E_HAL_PNL_CH_SWAP_2 :
                      pstMhalCfg->eCh4 == E_MHAL_PNL_CH_SWAP_3 ? E_HAL_PNL_CH_SWAP_3 :
                                                                 E_HAL_PNL_CH_SWAP_4;


}

void _DrvPnlIfTransParamToMhal(MhalPnlParamConfig_t *pstMhalCfg, HalPnlParamConfig_t *pstHalCfg)
{
    pstMhalCfg->u8Dither  = pstHalCfg->u8Dither;
    pstMhalCfg->eLinkType = pstHalCfg->eLinkType == E_HAL_PNL_LINK_LVDS             ?  E_MHAL_PNL_LINK_LVDS             :
                            pstHalCfg->eLinkType == E_HAL_PNL_LINK_RSDS             ?  E_MHAL_PNL_LINK_RSDS             :
                            pstHalCfg->eLinkType == E_HAL_PNL_LINK_MINILVDS         ?  E_MHAL_PNL_LINK_MINILVDS         :
                            pstHalCfg->eLinkType == E_HAL_PNL_LINK_ANALOG_MINILVDS  ?  E_MHAL_PNL_LINK_ANALOG_MINILVDS  :
                            pstHalCfg->eLinkType == E_HAL_PNL_LINK_DIGITAL_MINILVDS ?  E_MHAL_PNL_LINK_DIGITAL_MINILVDS :
                            pstHalCfg->eLinkType == E_HAL_PNL_LINK_MFC              ?  E_MHAL_PNL_LINK_MFC              :
                            pstHalCfg->eLinkType == E_HAL_PNL_LINK_DAC_I            ?  E_MHAL_PNL_LINK_DAC_I            :
                            pstHalCfg->eLinkType == E_HAL_PNL_LINK_DAC_P            ?  E_MHAL_PNL_LINK_DAC_P            :
                            pstHalCfg->eLinkType == E_HAL_PNL_LINK_PDPLVDS          ?  E_MHAL_PNL_LINK_PDPLVDS          :
                            pstHalCfg->eLinkType == E_HAL_PNL_LINK_EXT              ?  E_MHAL_PNL_LINK_EXT              :
                            pstHalCfg->eLinkType == E_HAL_PNL_LINK_MIPI_DSI         ?  E_MHAL_PNL_LINK_MIPI_DSI         :
                                                                                       E_MHAL_PNL_LINK_TTL;


    pstMhalCfg->u8DualPort      =  pstHalCfg->u8DualPort;
    pstMhalCfg->u8SwapPort      =  pstHalCfg->u8SwapPort;
    pstMhalCfg->u8SwapOdd_ML    =  pstHalCfg->u8SwapOdd_ML;
    pstMhalCfg->u8SwapEven_ML   =  pstHalCfg->u8SwapEven_ML;
    pstMhalCfg->u8SwapOdd_RB    =  pstHalCfg->u8SwapOdd_RB;
    pstMhalCfg->u8SwapEven_RB   =  pstHalCfg->u8SwapEven_RB;
    pstMhalCfg->u8SwapLVDS_POL  =  pstHalCfg->u8SwapLVDS_POL;
    pstMhalCfg->u8SwapLVDS_CH   =  pstHalCfg->u8SwapLVDS_CH;
    pstMhalCfg->u8PDP10BIT      =  pstHalCfg->u8PDP10BIT;
    pstMhalCfg->u8LVDS_TI_MODE  =  pstHalCfg->u8LVDS_TI_MODE;

    pstMhalCfg->u8DCLKDelay       = pstHalCfg->u8DCLKDelay;
    pstMhalCfg->u8InvDCLK         = pstHalCfg->u8InvDCLK;
    pstMhalCfg->u8InvDE           = pstHalCfg->u8InvDE;
    pstMhalCfg->u8InvHSync        = pstHalCfg->u8InvHSync;
    pstMhalCfg->u8InvVSync        = pstHalCfg->u8InvVSync;
    pstMhalCfg->u8DCKLCurrent     = pstHalCfg->u8DCKLCurrent;
    pstMhalCfg->u8DECurrent       = pstHalCfg->u8DECurrent;
    pstMhalCfg->u8ODDDataCurrent  = pstHalCfg->u8ODDDataCurrent;
    pstMhalCfg->u8EvenDataCurrent = pstHalCfg->u8EvenDataCurrent;
    pstMhalCfg->u16OnTiming1      = pstHalCfg->u16OnTiming1;
    pstMhalCfg->u16OnTiming2      = pstHalCfg->u16OnTiming2;
    pstMhalCfg->u16OffTiming1     = pstHalCfg->u16OffTiming1;

    pstMhalCfg->u16HSyncWidth     = pstHalCfg->u16HSyncWidth;
    pstMhalCfg->u16HSyncBackPorch = pstHalCfg->u16HSyncBackPorch;
    pstMhalCfg->u16VSyncWidth     = pstHalCfg->u16VSyncWidth;
    pstMhalCfg->u16VSyncBackPorch = pstHalCfg->u16VSyncBackPorch;
    pstMhalCfg->u16HStart         = pstHalCfg->u16HStart;
    pstMhalCfg->u16VStart         = pstHalCfg->u16VStart;
    pstMhalCfg->u16Width          = pstHalCfg->u16Width;
    pstMhalCfg->u16Height         = pstHalCfg->u16Height;
    pstMhalCfg->u16MaxHTotal      = pstHalCfg->u16MaxHTotal;
    pstMhalCfg->u16HTotal         = pstHalCfg->u16HTotal;
    pstMhalCfg->u16MinHTotal      = pstHalCfg->u16MinHTotal;
    pstMhalCfg->u16MaxVTotal      = pstHalCfg->u16MaxVTotal;
    pstMhalCfg->u16VTotal         = pstHalCfg->u16VTotal;
    pstMhalCfg->u16MinVTotal      = pstHalCfg->u16MinVTotal;
    pstMhalCfg->u16MaxDCLK        = pstHalCfg->u16MaxDCLK;
    pstMhalCfg->u16DCLK           = pstHalCfg->u16DCLK;
    pstMhalCfg->u16MinDCLK        = pstHalCfg->u16MinDCLK;

    pstMhalCfg->u16SpreadSpectrumStep = pstHalCfg->u16SpreadSpectrumStep;
    pstMhalCfg->u16SpreadSpectrumSpan = pstHalCfg->u16SpreadSpectrumSpan;

    pstMhalCfg->u8PwmPeriodL          = pstHalCfg->u8PwmPeriodL;
    pstMhalCfg->u8PwmPeriodH          = pstHalCfg->u8PwmPeriodH;
    pstMhalCfg->u8PwmDuty             = pstHalCfg->u8PwmDuty;

    pstMhalCfg->ePanelAspectRatio     = pstHalCfg->ePanelAspectRatio == E_HAL_PNL_ASPECT_RATIO_4_3  ? E_MHAL_PNL_ASPECT_RATIO_4_3 :
                                        pstHalCfg->ePanelAspectRatio == E_HAL_PNL_ASPECT_RATIO_WIDE ? E_MHAL_PNL_ASPECT_RATIO_WIDE :
                                                                                                      E_MHAL_PNL_ASPECT_RATIO_OTHER;

    pstMhalCfg->u16LVDSTxSwapValue   = pstHalCfg->u16LVDSTxSwapValue;
    pstMhalCfg->eTiBitMode           = pstHalCfg->eTiBitMode == E_HAL_PNL_TI_10BIT_MODE ? E_MHAL_PNL_TI_10BIT_MODE :
                                       pstHalCfg->eTiBitMode == E_HAL_PNL_TI_8BIT_MODE  ? E_MHAL_PNL_TI_8BIT_MODE :
                                                                                          E_MHAL_PNL_TI_6BIT_MODE;

    pstMhalCfg->eOutputFormatBitMode = pstHalCfg->eOutputFormatBitMode == E_HAL_PNL_OUTPUT_10BIT_MODE ? E_MHAL_PNL_OUTPUT_10BIT_MODE :
                                       pstHalCfg->eOutputFormatBitMode == E_HAL_PNL_OUTPUT_6BIT_MODE  ? E_MHAL_PNL_OUTPUT_6BIT_MODE :
                                       pstHalCfg->eOutputFormatBitMode == E_HAL_PNL_OUTPUT_8BIT_MODE  ? E_MHAL_PNL_OUTPUT_8BIT_MODE :
                                                                                                        E_MHAL_PNL_OUTPUT_565BIT_MODE;

    pstMhalCfg->u8SwapOdd_RG  = pstHalCfg->u8SwapOdd_RG == E_HAL_PNL_RGB_SWAP_R ?  E_MHAL_PNL_RGB_SWAP_R :
                                pstHalCfg->u8SwapOdd_RG == E_HAL_PNL_RGB_SWAP_G ?  E_MHAL_PNL_RGB_SWAP_G :
                                pstHalCfg->u8SwapOdd_RG == E_HAL_PNL_RGB_SWAP_B ?  E_MHAL_PNL_RGB_SWAP_B :
                                                                                   E_MHAL_PNL_RGB_SWAP_R;

    pstMhalCfg->u8SwapEven_RG = pstHalCfg->u8SwapEven_RG == E_HAL_PNL_RGB_SWAP_R ?  E_MHAL_PNL_RGB_SWAP_R :
                                pstHalCfg->u8SwapEven_RG == E_HAL_PNL_RGB_SWAP_G ?  E_MHAL_PNL_RGB_SWAP_G :
                                pstHalCfg->u8SwapEven_RG == E_HAL_PNL_RGB_SWAP_B ?  E_MHAL_PNL_RGB_SWAP_B :
                                                                                    E_MHAL_PNL_RGB_SWAP_G;

    pstMhalCfg->u8SwapOdd_GB  = pstHalCfg->u8SwapOdd_GB == E_HAL_PNL_RGB_SWAP_R ?  E_MHAL_PNL_RGB_SWAP_R :
                                pstHalCfg->u8SwapOdd_GB == E_HAL_PNL_RGB_SWAP_G ?  E_MHAL_PNL_RGB_SWAP_G :
                                pstHalCfg->u8SwapOdd_GB == E_HAL_PNL_RGB_SWAP_B ?  E_MHAL_PNL_RGB_SWAP_B :
                                                                                   E_MHAL_PNL_RGB_SWAP_B;
    pstMhalCfg->u8SwapEven_GB = pstHalCfg->u8SwapEven_GB;
    pstMhalCfg->u8DoubleClk   = pstHalCfg->u8DoubleClk;
    pstMhalCfg->u32MaxSET     = pstHalCfg->u32MaxSET;
    pstMhalCfg->u32MinSET     = pstHalCfg->u32MinSET;

    pstMhalCfg->eOutTimingMode = pstHalCfg->eOutTimingMode == E_HAL_PNL_CHG_DCLK   ? E_MHAL_PNL_CHG_DCLK :
                                 pstHalCfg->eOutTimingMode == E_HAL_PNL_CHG_HTOTAL ? E_MHAL_PNL_CHG_HTOTAL :
                                                                                     E_MHAL_PNL_CHG_VTOTAL;
    pstMhalCfg->u8NoiseDith = pstHalCfg->u8NoiseDith;

    pstMhalCfg->eCh0 = pstHalCfg->eCh0 == E_HAL_PNL_CH_SWAP_0 ? E_MHAL_PNL_CH_SWAP_0 :
                       pstHalCfg->eCh0 == E_HAL_PNL_CH_SWAP_1 ? E_MHAL_PNL_CH_SWAP_1 :
                       pstHalCfg->eCh0 == E_HAL_PNL_CH_SWAP_2 ? E_MHAL_PNL_CH_SWAP_2 :
                       pstHalCfg->eCh0 == E_HAL_PNL_CH_SWAP_3 ? E_MHAL_PNL_CH_SWAP_3 :
                                                                E_MHAL_PNL_CH_SWAP_4;

    pstMhalCfg->eCh1 = pstHalCfg->eCh1 == E_HAL_PNL_CH_SWAP_0 ? E_MHAL_PNL_CH_SWAP_0 :
                       pstHalCfg->eCh1 == E_HAL_PNL_CH_SWAP_1 ? E_MHAL_PNL_CH_SWAP_1 :
                       pstHalCfg->eCh1 == E_HAL_PNL_CH_SWAP_2 ? E_MHAL_PNL_CH_SWAP_2 :
                       pstHalCfg->eCh1 == E_HAL_PNL_CH_SWAP_3 ? E_MHAL_PNL_CH_SWAP_3 :
                                                                E_MHAL_PNL_CH_SWAP_4;

    pstMhalCfg->eCh2 = pstHalCfg->eCh2 == E_HAL_PNL_CH_SWAP_0 ? E_MHAL_PNL_CH_SWAP_0 :
                       pstHalCfg->eCh2 == E_HAL_PNL_CH_SWAP_1 ? E_MHAL_PNL_CH_SWAP_1 :
                       pstHalCfg->eCh2 == E_HAL_PNL_CH_SWAP_2 ? E_MHAL_PNL_CH_SWAP_2 :
                       pstHalCfg->eCh2 == E_HAL_PNL_CH_SWAP_3 ? E_MHAL_PNL_CH_SWAP_3 :
                                                                E_MHAL_PNL_CH_SWAP_4;

    pstMhalCfg->eCh3 = pstHalCfg->eCh3 == E_HAL_PNL_CH_SWAP_0 ? E_MHAL_PNL_CH_SWAP_0 :
                       pstHalCfg->eCh3 == E_HAL_PNL_CH_SWAP_1 ? E_MHAL_PNL_CH_SWAP_1 :
                       pstHalCfg->eCh3 == E_HAL_PNL_CH_SWAP_2 ? E_MHAL_PNL_CH_SWAP_2 :
                       pstHalCfg->eCh3 == E_HAL_PNL_CH_SWAP_3 ? E_MHAL_PNL_CH_SWAP_3 :
                                                                E_MHAL_PNL_CH_SWAP_4;

}

// SSC
void _DrvPnlIfTransSscConfigToHal(MhalPnlSscConfig_t *pstMhalCfg, HalPnlSscConfig_t *pstHalCfg)
{
    pstHalCfg->bEn     = pstMhalCfg->bEn;
    pstHalCfg->u16Step = pstMhalCfg->u16Step;
    pstHalCfg->u16Span = pstMhalCfg->u16Span;
}

void _DrvPnlIfTransSscConfigToMhal(MhalPnlSscConfig_t *pstMhalCfg, HalPnlSscConfig_t *pstHalCfg)
{
    pstMhalCfg->bEn     = pstHalCfg->bEn;
    pstMhalCfg->u16Step = pstHalCfg->u16Step;
    pstMhalCfg->u16Span = pstHalCfg->u16Span;
}

// Timing
void _DrvPnlIfTransTimingConfigToHal(MhalPnlTimingConfig_t *pstMhalCfg, HalPnlTimingConfig_t *pstHalCfg)
{
    pstHalCfg->u16HSyncWidth      =  pstMhalCfg->u16HSyncWidth;
    pstHalCfg->u16HSyncBackPorch  =  pstMhalCfg->u16HSyncBackPorch;
    pstHalCfg->u16HSyncFrontPorch =  pstMhalCfg->u16HSyncFrontPorch;
    pstHalCfg->u16VSyncWidth      =  pstMhalCfg->u16VSyncWidth;
    pstHalCfg->u16VSyncBackPorch  =  pstMhalCfg->u16VSyncBackPorch;
    pstHalCfg->u16VSyncFrontPorch =  pstMhalCfg->u16VSyncFrontPorch;
    pstHalCfg->u16HStart          =  pstMhalCfg->u16HStart;
    pstHalCfg->u16VStart          =  pstMhalCfg->u16VStart;
    pstHalCfg->u16HActive         =  pstMhalCfg->u16HActive;
    pstHalCfg->u16VActive         =  pstMhalCfg->u16VActive;
    pstHalCfg->u16HTotal          =  pstMhalCfg->u16HTotal;
    pstHalCfg->u16VTotal          =  pstMhalCfg->u16VTotal;
    pstHalCfg->u16Dclk            =  pstMhalCfg->u16Dclk;
}

void _DrvPnlIfTransTimingConfigToMhal(MhalPnlTimingConfig_t *pstMhalCfg, HalPnlTimingConfig_t *pstHalCfg)
{
    pstMhalCfg->u16HSyncWidth      =  pstHalCfg->u16HSyncWidth;
    pstMhalCfg->u16HSyncBackPorch  =  pstHalCfg->u16HSyncBackPorch;
    pstMhalCfg->u16HSyncFrontPorch =  pstHalCfg->u16HSyncFrontPorch;
    pstMhalCfg->u16VSyncWidth      =  pstHalCfg->u16VSyncWidth;
    pstMhalCfg->u16VSyncBackPorch  =  pstHalCfg->u16VSyncBackPorch;
    pstMhalCfg->u16VSyncFrontPorch =  pstHalCfg->u16VSyncFrontPorch;
    pstMhalCfg->u16HStart          =  pstHalCfg->u16HStart;
    pstMhalCfg->u16VStart          =  pstHalCfg->u16VStart;
    pstMhalCfg->u16HActive         =  pstHalCfg->u16HActive;
    pstMhalCfg->u16VActive         =  pstHalCfg->u16VActive;
    pstMhalCfg->u16HTotal          =  pstHalCfg->u16HTotal;
    pstMhalCfg->u16VTotal          =  pstHalCfg->u16VTotal;
    pstMhalCfg->u16Dclk            =  pstHalCfg->u16Dclk;
}

// Power
void _DrvPnlIfTransPowerConfigToHal(MhalPnlPowerConfig_t *pstMhalCfg, HalPnlPowerConfig_t *pstHalCfg)
{
    pstHalCfg->bEn = pstMhalCfg->bEn;
}

void _DrvPnlIfTransPowerConfigToMhal(MhalPnlPowerConfig_t *pstMhalCfg, HalPnlPowerConfig_t *pstHalCfg)
{
    pstMhalCfg->bEn = pstHalCfg->bEn;
}

//BackLightOnOff
void _DrvPnlIfTransBackLightOnOffConfigToHal(MhalPnlBackLightOnOffConfig_t *pstMhalCfg, HalPnlBackLightOnOffConfig_t *pstHalCfg)
{
    pstHalCfg->bEn = pstMhalCfg->bEn;
}

void _DrvPnlIfTransBackLightOnOffConfigToMhal(MhalPnlBackLightOnOffConfig_t *pstMhalCfg, HalPnlBackLightOnOffConfig_t *pstHalCfg)
{
    pstMhalCfg->bEn = pstHalCfg->bEn;
}

//BackLightLevel
void _DrvPnlIfTransBackLightLevelConfigToHal(MhalPnlBackLightLevelConfig_t *pstMhalCfg, HalPnlBackLightLevelConfig_t *pstHalCfg)
{
    pstHalCfg->u16Duty = pstMhalCfg->u16Duty;
    pstHalCfg->u16Period = pstMhalCfg->u16Period;
}

void _DrvPnlIfTransBackLightLevelConfigToMhal(MhalPnlBackLightLevelConfig_t *pstMhalCfg, HalPnlBackLightLevelConfig_t *pstHalCfg)
{
    pstMhalCfg->u16Duty = pstHalCfg->u16Duty;
    pstMhalCfg->u16Period = pstHalCfg->u16Period;
}

// DrvCurrent
void _DrvPnlIfTransDrvCurrentConfigToHal(MhalPnlDrvCurrentConfig_t *pstMhalCfg, HalPnlCurrentConfig_t *pstHalCfg)
{
    pstMhalCfg->u16Val = pstHalCfg->u16Val;
}

void _DrvPnlIfTransDrvCurrentConfigToMhal(MhalPnlDrvCurrentConfig_t *pstMhalCfg, HalPnlCurrentConfig_t *pstHalCfg)
{
    pstHalCfg->u16Val = pstMhalCfg->u16Val;
}

// TestPat
void _DrvPnlIfTransTestPatConfigToHal(MhalPnlTestPatternConfig_t *pstMhalCfg, HalPnlTestPatternConfig_t *pstHalCfg)
{
    pstHalCfg->bEn = pstMhalCfg->bEn;
    pstHalCfg->u16R = pstMhalCfg->u16R;
    pstHalCfg->u16G = pstMhalCfg->u16G;
    pstHalCfg->u16B = pstMhalCfg->u16B;
}

bool _DrvPnlIfExecuteQuery(void *pCtx, HalPnlQueryConfig_t *pstQueryCfg)
{
    bool bRet = 1;

    if(HalPnlIfQuery(pCtx, pstQueryCfg))
    {
        if(pstQueryCfg->stOutCfg.enQueryRet == E_HAL_PNL_QUERY_RET_OK ||
           pstQueryCfg->stOutCfg.enQueryRet == E_HAL_PNL_QUERY_RET_NONEED)
        {
            if(pstQueryCfg->stOutCfg.pSetFunc)
            {
                pstQueryCfg->stOutCfg.pSetFunc(pCtx, pstQueryCfg->stInCfg.pInCfg);
            }
        }
        else
        {
            bRet = 0;
            PNL_ERR("%s %d, Query:%s, Ret:%s\n",
                __FUNCTION__,__LINE__,
                PARSING_HAL_QUERY_TYPE(pstQueryCfg->stInCfg.enQueryType),
                PARSING_HAL_QUERY_RET(pstQueryCfg->stOutCfg.enQueryRet));
        }
    }
    else
    {
        bRet = 0;
        PNL_ERR("%s %d, Query Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool DrvPnlIfSetClkConfig(void *pCtx, bool *pbEn, u32 *pu32ClkRate, u32 u32ClkNum)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlClkConfig_t stHalClkCfg;

    if( sizeof(stHalClkCfg.u32Rate) != sizeof(u32)*u32ClkNum ||
        sizeof(stHalClkCfg.bEn) != sizeof(bool)*u32ClkNum)
    {
        bRet = 0;
        PNL_ERR("%s %d, Clk Num is not correct: Rate:%d != %ld, En:%d != %ld",
            __FUNCTION__, __LINE__,
            sizeof(stHalClkCfg.u32Rate), sizeof(u32)*u32ClkNum,
            sizeof(stHalClkCfg.bEn), sizeof(bool)*u32ClkNum);
    }
    else
    {

        memset(&stQueryCfg, 0, sizeof(HalPnlQueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_CLK_SET;
        stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlClkConfig_t);
        stQueryCfg.stInCfg.pInCfg = &stHalClkCfg;


        stHalClkCfg.u32Num = u32ClkNum;
        memcpy(stHalClkCfg.u32Rate, pu32ClkRate, sizeof(u32)*u32ClkNum);
        memcpy(stHalClkCfg.bEn, pbEn, sizeof(bool)*u32ClkNum);

        bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);
    }
    return bRet;
}

bool DrvPnlIfGetClkConfig(void *pCtx, bool *pbEn, u32 *pu32ClkRate, u32 u32ClkNum)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlClkConfig_t stHalClkCfg;

    memset(&stQueryCfg, 0, sizeof(HalPnlQueryConfig_t));
    memset(&stHalClkCfg, 0, sizeof(HalPnlClkConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_CLK_GET;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlClkConfig_t);
    stQueryCfg.stInCfg.pInCfg = &stHalClkCfg;

    if(_DrvPnlIfExecuteQuery(pCtx, &stQueryCfg))
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
        PNL_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool DrvPnlIfGetInstance(void **pCtx, MhalPnlLinkType_e enLinkType, u16 u16Id)
{
    DrvPnlCtxAllocConfig_t stCtxAllcCfg;
    DrvPnlCtxConfig_t *pPnlCtxCfg = NULL;
    bool bRet = 1;

    stCtxAllcCfg.s16CtxId = u16Id;
    if(DrvPnlCtxGet(&stCtxAllcCfg, &pPnlCtxCfg))
    {
        *pCtx = (void *)pPnlCtxCfg;
    }
    else
    {
        *pCtx = NULL;
        bRet = 0;
        PNL_ERR("%s %d: No Instance Id=%d\n", __FUNCTION__, __LINE__, u16Id);
    }
    return bRet;
}

bool DrvPnlIfCreateInstance(void **pCtx, MhalPnlLinkType_e enLinkType, u16 u16Id)
{
    DrvPnlCtxAllocConfig_t stCtxAllcCfg;
    DrvPnlCtxConfig_t *pPnlCtxCfg = NULL;
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;

    DrvPnlCtxInit();
    HalPnlIfInit();

    stCtxAllcCfg.s16CtxId = u16Id;

    if(DrvPnlCtxAllocate(&stCtxAllcCfg, &pPnlCtxCfg))
    {
        u32 au32ClkRate[HAL_PNL_CLK_NUM] = HAL_PNL_CLK_RATE_SETTING;
        bool abClkEn[HAL_PNL_CLK_NUM] = HAL_PNL_CLK_ON_SETTING;

        pPnlCtxCfg->pstHalCtx->enLinkType = enLinkType;
        *pCtx = (void *)pPnlCtxCfg;

        if(DrvPnlOsSetClkOn(au32ClkRate, HAL_PNL_CLK_NUM) == 0)
        {
            if(DrvPnlIfSetClkConfig((void *)pPnlCtxCfg, abClkEn, au32ClkRate, HAL_PNL_CLK_NUM) == 0)
            {
                PNL_ERR("%s %d:: SetClk Fail\n", __FUNCTION__, __LINE__);
                bRet = 0;
            }
            else
            {
                bRet = 1;
            }
        }
        else
        {
            bRet = 1;
        }

        memset(&stQueryCfg, 0, sizeof(HalPnlQueryConfig_t));
        stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_INIT;
        stQueryCfg.stInCfg.u32CfgSize = 0;
        stQueryCfg.stInCfg.pInCfg = NULL;
        bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);
    }
    else
    {
        *pCtx = NULL;
        bRet = 0;
        PNL_ERR("%s %d, CreateInstance Ctx Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool DrvPnlIfDestroyInstance(void *pCtx)
{
    bool bRet = 1;
    DrvPnlCtxConfig_t *pPnlCtxCfg =  (DrvPnlCtxConfig_t *)pCtx;
    u32 au32ClkRate[HAL_PNL_CLK_NUM] = HAL_PNL_CLK_RATE_SETTING;
    bool abClkEn[HAL_PNL_CLK_NUM] = HAL_PNL_CLK_OFF_SETTING;

    if(DrvPnlOsSetClkOff() == 0)
    {
        if(DrvPnlIfSetClkConfig(pCtx, abClkEn, au32ClkRate, HAL_PNL_CLK_NUM) ==0)
        {
            bRet = 0;
            PNL_ERR("%s %d, Set Clk Off Fail\n", __FUNCTION__, __LINE__);
        }
    }

    if(bRet && DrvPnlCtxFree(pPnlCtxCfg) == 0)
    {
        bRet = 0;
        PNL_ERR("%s %d, DestroyInstance Fail\n", __FUNCTION__, __LINE__);
    }

    if( DrvPnlCtxIsAllFree())
    {
        HalPnlIfDeInit();
    }

    return bRet;
}


bool DrvPnlIfSetParamConfig(void *pCtx, MhalPnlParamConfig_t *pParamCfg)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlParamConfig_t stHalParamCfg;

    memset(&stQueryCfg, 0, sizeof(HalPnlQueryConfig_t));
    memset(&stHalParamCfg, 0, sizeof(HalPnlParamConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_PARAM;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlParamConfig_t);
    stQueryCfg.stInCfg.pInCfg = (void *)&stHalParamCfg;

    _DrvPnlIfTransParamToHal(pParamCfg, &stHalParamCfg);
    bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);
    return bRet;
}


bool DrvPnlIfGetParamConfig(void *pCtx, MhalPnlParamConfig_t *pParamCfg)
{
    bool bRet = 1;
    DrvPnlCtxConfig_t *pstPnlCtx = (DrvPnlCtxConfig_t *)pCtx;

    _DrvPnlIfTransParamToMhal(pParamCfg, &pstPnlCtx->pstHalCtx->stParamCfg);

    return bRet;
}



bool DrvPnlIfSetMipiDsiConfig(void *pCtx, MhalPnlMipiDsiConfig_t *stpMipiDsiCfg)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlMipiDsiConfig_t stHalMipiDsiParamCfg;

    memset(&stQueryCfg, 0, sizeof(HalPnlQueryConfig_t));
    memset(&stHalMipiDsiParamCfg, 0, sizeof(HalPnlMipiDsiConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_MIPIDSI_PARAM;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlMipiDsiConfig_t);
    stQueryCfg.stInCfg.pInCfg = (void *)&stHalMipiDsiParamCfg;

    _DrvPnlIfTransMipiDsiConfigToHal(stpMipiDsiCfg, &stHalMipiDsiParamCfg);

    bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);

    return bRet;
}


bool DrvPnlIfGetMipiDsiConfig(void *pCtx, MhalPnlMipiDsiConfig_t *pstMipiDsiCfg)
{
    bool bRet = 1;
    DrvPnlCtxConfig_t *pstPnlCtx = (DrvPnlCtxConfig_t *)pCtx;

    _DrvPnlIfTransMipiDsiConfigToMHal(pstMipiDsiCfg, &pstPnlCtx->pstHalCtx->stMipiDisCfg);

    return bRet;
}


bool DrvPnlIfSetSscConfig(void *pCtx, MhalPnlSscConfig_t *pstSscCfg)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlSscConfig_t stHalSscCfg;

    memset(&stQueryCfg, 0, sizeof(HalPnlQueryConfig_t));
    memset(&stHalSscCfg, 0, sizeof(HalPnlSscConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_SSC;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlSscConfig_t);
    stQueryCfg.stInCfg.pInCfg = (void *)&stHalSscCfg;

    _DrvPnlIfTransSscConfigToHal(pstSscCfg, &stHalSscCfg);

    bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);

    return bRet;
}


bool DrvPnlIfSetTimingConfig(void *pCtx, MhalPnlTimingConfig_t *pstTimingCfg)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlTimingConfig_t stHalTimingCfg;

    memset(&stQueryCfg, 0, sizeof(HalPnlQueryConfig_t));
    memset(&stHalTimingCfg, 0, sizeof(HalPnlTimingConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_TIMING;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlTimingConfig_t);
    stQueryCfg.stInCfg.pInCfg = (void *)&stHalTimingCfg;

    _DrvPnlIfTransTimingConfigToHal(pstTimingCfg, &stHalTimingCfg);

    bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);

    return bRet;
}


bool DrvPnlIfGetTimingConfig(void *pCtx, MhalPnlTimingConfig_t *pstTimingCfg)
{
    bool bRet = 1;
    DrvPnlCtxConfig_t *pstPnlCtx = (DrvPnlCtxConfig_t *)pCtx;

    _DrvPnlIfTransTimingConfigToMhal(pstTimingCfg, &pstPnlCtx->pstHalCtx->stTimingCfg);

    return bRet;
}


bool DrvPnlIfSetPowerConfig(void *pCtx, MhalPnlPowerConfig_t *pstPowerCfg)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlPowerConfig_t stHalPowerCfg;

    memset(&stQueryCfg, 0, sizeof(HalPnlQueryConfig_t));
    memset(&stHalPowerCfg, 0, sizeof(HalPnlPowerConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_POWER;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlPowerConfig_t);
    stQueryCfg.stInCfg.pInCfg = (void *)&stHalPowerCfg;

    _DrvPnlIfTransPowerConfigToHal(pstPowerCfg, &stHalPowerCfg);

    bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);

    return bRet;
}


bool DrvPnlIfGetPowerConfig(void *pCtx, MhalPnlPowerConfig_t *pstPowerCfg)
{
    bool bRet = 1;
    DrvPnlCtxConfig_t *pstPnlCtx = (DrvPnlCtxConfig_t *)pCtx;

    _DrvPnlIfTransPowerConfigToMhal(pstPowerCfg, &pstPnlCtx->pstHalCtx->stPowerCfg);

    return bRet;
}


bool DrvPnlIfSetBackLightOnOffConfig(void *pCtx, MhalPnlBackLightOnOffConfig_t *pstBackLightOnOffCfg)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlBackLightOnOffConfig_t stHalBackLightOnOffCfg;

    memset(&stQueryCfg, 0, sizeof(HalPnlQueryConfig_t));
    memset(&stHalBackLightOnOffCfg, 0, sizeof(HalPnlBackLightOnOffConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_BACKLIGHT_ONOFF;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlBackLightOnOffConfig_t);
    stQueryCfg.stInCfg.pInCfg = (void *)&stHalBackLightOnOffCfg;

    _DrvPnlIfTransBackLightOnOffConfigToHal(pstBackLightOnOffCfg, &stHalBackLightOnOffCfg);

    bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);

    return bRet;
}

bool DrvPnlIfGetBackLightOnOffConfig(void *pCtx, MhalPnlBackLightOnOffConfig_t *pstBackLightOnOffCfg)
{
    bool bRet = 1;
    DrvPnlCtxConfig_t *pstPnlCtx = (DrvPnlCtxConfig_t *)pCtx;

    _DrvPnlIfTransBackLightOnOffConfigToMhal(pstBackLightOnOffCfg, &pstPnlCtx->pstHalCtx->stBackLightOnOffCfg);

    return bRet;
}

bool DrvPnlIfSetBackLightLevelConfig(void *pCtx, MhalPnlBackLightLevelConfig_t *pstBackLightLevelCfg)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlBackLightLevelConfig_t stHalBackLightLevelCfg;

    memset(&stQueryCfg, 0, sizeof(HalPnlBackLightLevelConfig_t));
    memset(&stHalBackLightLevelCfg, 0, sizeof(HalPnlBackLightLevelConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_BACKLIGHT_LEVEL;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlBackLightLevelConfig_t);
    stQueryCfg.stInCfg.pInCfg = (void *)&stHalBackLightLevelCfg;

    _DrvPnlIfTransBackLightLevelConfigToHal(pstBackLightLevelCfg, &stHalBackLightLevelCfg);

    bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);

    return bRet;
}

bool DrvPnlIfGetBackLightLevelConfig(void *pCtx, MhalPnlBackLightLevelConfig_t *pstBackLightLevelCfg)
{
    bool bRet = 1;
    DrvPnlCtxConfig_t *pstPnlCtx = (DrvPnlCtxConfig_t *)pCtx;

    _DrvPnlIfTransBackLightLevelConfigToMhal(pstBackLightLevelCfg, &pstPnlCtx->pstHalCtx->stBackLightLevelCfg);

    return bRet;
}


bool DrvPnlIfSetDrvCurrentConfig(void *pCtx, MhalPnlDrvCurrentConfig_t* pstCurpDrvCurrentCfgrentCfg)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlCurrentConfig_t stHalCurrentCfg;

    memset(&stQueryCfg, 0, sizeof(HalPnlCurrentConfig_t));
    memset(&stHalCurrentCfg, 0, sizeof(HalPnlCurrentConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_CURRENT;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlCurrentConfig_t);
    stQueryCfg.stInCfg.pInCfg = (void *)&stHalCurrentCfg;

    _DrvPnlIfTransDrvCurrentConfigToHal(pstCurpDrvCurrentCfgrentCfg, &stHalCurrentCfg);

    bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);

    return bRet;
}


bool DrvPnlIfSetTestPatternConfig(void *pCtx, MhalPnlTestPatternConfig_t *pstTestPatternCfg)
{
    bool bRet = 1;
    HalPnlQueryConfig_t stQueryCfg;
    HalPnlTestPatternConfig_t stHalTestPatCfg;

    memset(&stQueryCfg, 0, sizeof(HalPnlCurrentConfig_t));
    memset(&stHalTestPatCfg, 0, sizeof(HalPnlTestPatternConfig_t));

    stQueryCfg.stInCfg.enQueryType = E_HAL_PNL_QUERY_TESTPAT;
    stQueryCfg.stInCfg.u32CfgSize = sizeof(HalPnlTestPatternConfig_t);
    stQueryCfg.stInCfg.pInCfg = (void *)&stHalTestPatCfg;

    _DrvPnlIfTransTestPatConfigToHal(pstTestPatternCfg, &stHalTestPatCfg);

    bRet = _DrvPnlIfExecuteQuery(pCtx, &stQueryCfg);

    return bRet;
}

bool DrvPnlIfSetDbgLevel(void *pDbgLevel)
{
   _gu32PnlDbgLevel = *((u32 *)pDbgLevel);
    return 1;
}

//-------------------------------------------------------------------------------------------------
