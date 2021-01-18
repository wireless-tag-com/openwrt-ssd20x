#include <string.h>
#include <stdio.h>
#include "platform.h"

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK                  MAKE_YUYV_VALUE(0,128,128)

#ifdef SUPPORT_HDMI
static MI_S32 Hdmi_callback_impl(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_EventType_e Event, void *pEventParam, void *pUsrParam)
{
    switch (Event)
    {
        case E_MI_HDMI_EVENT_HOTPLUG:
            printf("E_MI_HDMI_EVENT_HOTPLUG.\n");
            break;
        case E_MI_HDMI_EVENT_NO_PLUG:
            printf("E_MI_HDMI_EVENT_NO_PLUG.\n");
            break;
        default:
            printf("Unsupport event.\n");
            break;
    }

    return MI_SUCCESS;
}

int sstar_hdmi_init(MI_DISP_Interface_e eType)
{
    MI_HDMI_InitParam_t stInitParam;
    MI_HDMI_Attr_t stAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_PubAttr_t stDispPubAttr;
    MI_DISP_InputPortAttr_t stInputPortAttr;

    if (E_MI_DISP_INTF_HDMI == eType)
    {
        stInitParam.pCallBackArgs = NULL;
        stInitParam.pfnHdmiEventCallback = Hdmi_callback_impl;
        MI_HDMI_Init(&stInitParam);
        MI_HDMI_Open(E_MI_HDMI_ID_0);

        memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
        stAttr.stEnInfoFrame.bEnableAudInfoFrame= FALSE;
        stAttr.stEnInfoFrame.bEnableAviInfoFrame= FALSE;
        stAttr.stEnInfoFrame.bEnableSpdInfoFrame= FALSE;
        stAttr.stAudioAttr.bEnableAudio         = TRUE;
        stAttr.stAudioAttr.bIsMultiChannel      = 0;
        stAttr.stAudioAttr.eBitDepth            = E_MI_HDMI_BIT_DEPTH_16;
        stAttr.stAudioAttr.eCodeType            = E_MI_HDMI_ACODE_PCM;
        stAttr.stAudioAttr.eSampleRate          = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
        stAttr.stVideoAttr.bEnableVideo         = TRUE;
        stAttr.stVideoAttr.eColorType           = E_MI_HDMI_COLOR_TYPE_RGB444;//default color type
        stAttr.stVideoAttr.eDeepColorMode       = E_MI_HDMI_DEEP_COLOR_MAX;
        stAttr.stVideoAttr.eTimingType          = E_MI_HDMI_TIMING_1080_60P;
        stAttr.stVideoAttr.eOutputMode          = E_MI_HDMI_OUTPUT_MODE_HDMI;
        MI_HDMI_SetAttr(E_MI_HDMI_ID_0, &stAttr);
        MI_HDMI_Start(E_MI_HDMI_ID_0);
        stDispPubAttr.eIntfType                 = E_MI_DISP_INTF_HDMI;
        stDispPubAttr.u32BgColor                = YUYV_BLACK;
        stDispPubAttr.eIntfSync                 = E_MI_DISP_OUTPUT_1080P60;
        MI_DISP_SetPubAttr(0, &stDispPubAttr);
        MI_DISP_Enable(0);
        MI_DISP_BindVideoLayer(0, 0);

        memset(&stLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));
        stLayerAttr.stVidLayerSize.u16Width     = HDMI_MAX_W;
        stLayerAttr.stVidLayerSize.u16Height    = HDMI_MAX_H;
        stLayerAttr.ePixFormat                  = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
        stLayerAttr.stVidLayerDispWin.u16X      = 0;
        stLayerAttr.stVidLayerDispWin.u16Y      = 0;
        stLayerAttr.stVidLayerDispWin.u16Width  = HDMI_MAX_W;
        stLayerAttr.stVidLayerDispWin.u16Height = HDMI_MAX_H;
        MI_DISP_SetVideoLayerAttr(0, &stLayerAttr);
        MI_DISP_EnableVideoLayer(0);

        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
        stInputPortAttr.u16SrcWidth             = HDMI_MAX_W;
        stInputPortAttr.u16SrcHeight            = HDMI_MAX_H;
        stInputPortAttr.stDispWin.u16X          = 0;
        stInputPortAttr.stDispWin.u16Y          = 0;
        stInputPortAttr.stDispWin.u16Width      = HDMI_MAX_W;
        stInputPortAttr.stDispWin.u16Height     = HDMI_MAX_H;

        MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
        MI_DISP_EnableInputPort(0, 0);
        MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);
    }

    return MI_SUCCESS;
}

int sstar_hdmi_deinit(MI_DISP_Interface_e eType)
{
    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_DisableVideoLayer(0);
    MI_DISP_UnBindVideoLayer(0, 0);
    MI_DISP_Disable(0);

    switch(eType)
    {
        case E_MI_DISP_INTF_HDMI:
            MI_HDMI_Stop(E_MI_HDMI_ID_0);
            MI_HDMI_Close(E_MI_HDMI_ID_0);
            MI_HDMI_DeInit();
            break;

        default: break;
    }

    return MI_SUCCESS;
}

#else

#if  DISPLAY_1024_600
    #include "CC0702I50R_1024x600.h"
    //#include "SAT070JHH_1024x600.h"
#else
    #if USE_MIPI
    #include "ST7701S_480x480_MIPI.h"
    #else
    #include "SAT070AT50_800x480.h"
    #endif
#endif

int sstar_panel_init(MI_DISP_Interface_e eType)
{
    MI_PANEL_LinkType_e eLinkType;
    MI_DISP_PubAttr_t stDispPubAttr;
    MI_DISP_InputPortAttr_t stInputPortAttr;

    if (E_MI_DISP_INTF_LCD == eType)
    {
        stDispPubAttr.stSyncInfo.u16Vact       = stPanelParam.u16Height;
        stDispPubAttr.stSyncInfo.u16Vbb        = stPanelParam.u16VSyncBackPorch;
        stDispPubAttr.stSyncInfo.u16Vfb        = stPanelParam.u16VTotal - (stPanelParam.u16VSyncWidth +
                                                 stPanelParam.u16Height + stPanelParam.u16VSyncBackPorch);
        stDispPubAttr.stSyncInfo.u16Hact       = stPanelParam.u16Width;
        stDispPubAttr.stSyncInfo.u16Hbb        = stPanelParam.u16HSyncBackPorch;
        stDispPubAttr.stSyncInfo.u16Hfb        = stPanelParam.u16HTotal - (stPanelParam.u16HSyncWidth +
                                                  stPanelParam.u16Width + stPanelParam.u16HSyncBackPorch);
        stDispPubAttr.stSyncInfo.u16Bvact      = 0;
        stDispPubAttr.stSyncInfo.u16Bvbb       = 0;
        stDispPubAttr.stSyncInfo.u16Bvfb       = 0;
        stDispPubAttr.stSyncInfo.u16Hpw        = stPanelParam.u16HSyncWidth;
        stDispPubAttr.stSyncInfo.u16Vpw        = stPanelParam.u16VSyncWidth;
        stDispPubAttr.stSyncInfo.u32FrameRate  = stPanelParam.u16DCLK * 1000000 / (stPanelParam.u16HTotal * stPanelParam.u16VTotal);
        stDispPubAttr.eIntfSync                = E_MI_DISP_OUTPUT_USER;
        stDispPubAttr.eIntfType                = E_MI_DISP_INTF_LCD;
        stDispPubAttr.u32BgColor               = YUYV_BLACK;

        #if USE_MIPI
        eLinkType = E_MI_PNL_LINK_MIPI_DSI;
        #else
        eLinkType = E_MI_PNL_LINK_TTL;
        #endif

        MI_DISP_SetPubAttr(0, &stDispPubAttr);
        MI_DISP_Enable(0);
        MI_DISP_BindVideoLayer(0, 0);
        MI_DISP_EnableVideoLayer(0);

        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
        stInputPortAttr.u16SrcWidth             = PANEL_MAX_W;
        stInputPortAttr.u16SrcHeight            = PANEL_MAX_H;
        stInputPortAttr.stDispWin.u16X          = 0;
        stInputPortAttr.stDispWin.u16Y          = 0;
        stInputPortAttr.stDispWin.u16Width      = PANEL_MAX_W;
        stInputPortAttr.stDispWin.u16Height     = PANEL_MAX_H;

        MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
        MI_DISP_EnableInputPort(0, 0);
        MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);

        MI_PANEL_Init(eLinkType);
        MI_PANEL_SetPanelParam(&stPanelParam);
        if(eLinkType == E_MI_PNL_LINK_MIPI_DSI)
        {
            #if USE_MIPI
            MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
            #endif
        }
    }
    
    return MI_SUCCESS;
}

int sstar_panel_deinit(MI_DISP_Interface_e eType)
{
    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_DisableVideoLayer(0);
    MI_DISP_UnBindVideoLayer(0, 0);
    MI_DISP_Disable(0);

    switch(eType)
    {
        case E_MI_DISP_INTF_VGA:
            break;

        case E_MI_DISP_INTF_LCD:
            MI_PANEL_DeInit();
            break;

        default: break;
    }

    return MI_SUCCESS;
}

#endif

void sstar_getpanel_wh(int *width, int *height)
{
#ifndef SUPPORT_HDMI
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_GetPubAttr(0,&stPubAttr);
    *width = stPubAttr.stSyncInfo.u16Hact;
    *height = stPubAttr.stSyncInfo.u16Vact;
#else
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_GetVideoLayerAttr(0, &stLayerAttr);
    *width = stLayerAttr.stVidLayerDispWin.u16Width;
    *height = stLayerAttr.stVidLayerDispWin.u16Height;
#endif
    printf("sstar_getpanel_wh = [%d %d]\n", *width, *height);
}

int sstar_sys_init(void)
{
    MI_SYS_Version_t stVersion;
    MI_U64 u64Pts = 0;

    MI_SYS_Init();

    memset(&stVersion, 0x0, sizeof(MI_SYS_Version_t));

    MI_SYS_GetVersion(&stVersion);

    MI_SYS_GetCurPts(&u64Pts);

    u64Pts = 0xF1237890F1237890;
    MI_SYS_InitPtsBase(u64Pts);

    u64Pts = 0xE1237890E1237890;
    MI_SYS_SyncPts(u64Pts);

    return MI_SUCCESS;
}

int sstar_sys_deinit(void)
{
    MI_SYS_Exit();
    return MI_SUCCESS;
}


