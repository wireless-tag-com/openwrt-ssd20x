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

#ifndef _DRV_DISP_IF_H_
#define _DRV_DISP_IF_H_


#ifdef _DRV_DISP_IF_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif
INTERFACE bool DrvDispIfInitPanelConfig(MHAL_DISP_PanelConfig_t* pstPanelConfig, u8 u8Size);
INTERFACE bool DrvDispIfDeviceCreateInstance(MHAL_DISP_AllocPhyMem_t *pstAlloc, u32 u32DeviceId, void **pDevCtx);
INTERFACE bool DrvDispIfDeviceDestroyInstance(void *pDevCtx);
INTERFACE bool DrvDispIfDeviceEnable(void *pDevCtx, bool bEnable);
INTERFACE bool DrvDispIfDeviceSetBackGroundColor(void *pDevCtx, u32 u32BgColor);
INTERFACE bool DrvDispIfDeviceAddOutInterface(void *pDevCtx, u32 u32Interface);
INTERFACE bool DrvDispIfDeviceSetOutputTiming(void *pDevCtx, u32 u32Interface, MHAL_DISP_DeviceTimingInfo_t *pstTimingInfo);
INTERFACE bool DrvDispIfDeviceSetColortemp(void *pDevCtx, MHAL_DISP_ColorTempeture_t *pstcolorTemp);
INTERFACE bool DrvDispIfDeviceSetCvbsParam(void *pDevCtx, MHAL_DISP_CvbsParam_t *pstCvbsInfo);
INTERFACE bool DrvDispIfDeviceSetHdmiParam(void *pDevCtx, MHAL_DISP_HdmiParam_t *pstHdmiInfo);
INTERFACE bool DrvDispIfDeviceSetVgaParam(void *pDevCtx, MHAL_DISP_VgaParam_t *pstVgaInfo);
INTERFACE bool DrvDispIfDeviceSetGammaParam(void *pDevCtx, MHAL_DISP_GammaParam_t *pstGammaInfo);
INTERFACE bool DrvDispIfDeviceSetLcdParam(void *pDevCtx, MHAL_DISP_LcdParam_t *pstLcdInfo);
INTERFACE bool DrvDispIfDeviceAttach(void *pSrcDevCtx, void *pDstDevCtx);
INTERFACE bool DrvDispIfDeviceDetach(void *pSrcDevCtx, void *pDstDevCtx);
INTERFACE bool DrvDispIfDeviceGetTimeZone(void *pDevCtx, MHAL_DISP_TimeZone_t *pstTimeZone);
INTERFACE bool DrvDispIfDeviceGetInstance(u32 u32DeviceId, void **pDevCtx);
INTERFACE bool DrvDispIfDeviceGetDisplayInfo(void *pDevCtx, MHAL_DISP_DisplayInfo_t *pstDisplayInfo);
INTERFACE bool DrvDispIfGetClk(void *pDevCtx, bool *pbEn, u32 *pu32ClkRate, u32 u32ClkNum);
INTERFACE bool DrvDispIfSetClk(void *pDevCtx, bool *pbEn, u32 *pu32ClkRate, u32 u32ClkNum);
INTERFACE bool DrvDispIfVideoLayerCreateInstance(MHAL_DISP_AllocPhyMem_t *pstAlloc, u32 u32LayerId, void **pVidLayerCtx);
INTERFACE bool DrvDispIfVideoLayerDestoryInstance(void *pVidLayerCtx);
INTERFACE bool DrvDispIfVideoLayerEnable(void *pVidLayerCtx,  bool bEnable);
INTERFACE bool DrvDispIfVideoLayerBind(void *pVidLayerCtx, void *pDevCtx);
INTERFACE bool DrvDispIfVideoLayerUnBind(void *pVidLayerCtx, void *pDevCtx);
INTERFACE bool DrvDispIfVideoLayerSetAttr(void *pVidLayerCtx,  MHAL_DISP_VideoLayerAttr_t *pstAttr);
INTERFACE bool DrvDispIfVideoLayerBufferFire(void *pVidLayerCtx);
INTERFACE bool DrvDispIfVideoLayerCheckBufferFired(void *pVidLayerCtx);
INTERFACE bool DrvDispIfVideoLayerSetCompress(void *pVidLayerCtx, MHAL_DISP_CompressAttr_t* pstCompressAttr);
INTERFACE bool DrvDispIfVideoLayerSetPriority(void *pVidLayerCtx, u32 u32Priority);
INTERFACE bool DrvDispIfInputPortCreateInstance(MHAL_DISP_AllocPhyMem_t *pstAlloc, void *pVidLayerCtx, u32 u32PortId, void **pCtx);
INTERFACE bool DrvDispIfInputPortDestroyInstance(void *pInputPortCtx);
INTERFACE bool DrvDispIfInputPortFlip(void *pInputPortCtx, MHAL_DISP_VideoFrameData_t *pstVideoFrameData);
INTERFACE bool DrvDispIfInputPortEnable(void *pInputPortCtx, bool bEnable);
INTERFACE bool DrvDispIfInputPortSetAttr(void *pInputPortCtx, MHAL_DISP_InputPortAttr_t *pstAttr);
INTERFACE bool DrvDispIfInputPortShow(void *pInputPortCtx);
INTERFACE bool DrvDispIfInputPortHide(void *pInputPortCtx);
INTERFACE bool DrvDispIfInputPortAttrBegin(void *pVidLayerCtx);
INTERFACE bool DrvDispIfInputPortAttrEnd(void *pVidLayerCtx);
INTERFACE bool DrvDispIfInputPortRotate(void *pInputPortCtx, MHAL_DISP_RotateConfig_t *pstRotateCfg);
INTERFACE bool DrvDispIfInputPortSetCropAttr(void *pInputPortCtx, MHAL_DISP_VidWinRect_t *pstCropAttr);
INTERFACE bool DrvDispIfSetDbgLevel(void *p);


#undef INTERFACE
#endif
