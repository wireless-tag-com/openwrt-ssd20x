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

#ifndef _HAL_PNL_H_
#define _HAL_PNL_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _HAL_PNL_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif


INTERFACE bool HalPnlGetLpllIdx(u32 u32Dclk, u16 *pu16Idx, bool bDsi);
INTERFACE u16  HalPnlGetLpllGain(u16 u16Idx, bool bDsi);
INTERFACE u16  HalPnlGetLpllDiv(u16 u16Idx, bool bDsi);
INTERFACE void HalPnlSetTTLPadMux(HalPnlOutputFormatBitMode_e enFmt);
INTERFACE void HalPnlSetMipiDisPadMux(HalPnlMipiDsiLaneMode_e enLaneNum);
INTERFACE void HalPnlSetTtlPadCotnrol(u32 u32Ie, u32 u32Pe, u32 u32Ps, u32 u32Drv);
INTERFACE void HalPnlDumpLpllSetting(u16 u16Idx);
INTERFACE void HalPnlSetLpllSet(u32 u32LpllSet);
INTERFACE void HalPnlSetFrameColorEn(bool bEn);
INTERFACE void HalPnlSetFrameColor(u16 u16R, u16 u16G, u16 u16B);
INTERFACE void HalPnlSetVSyncSt(u16 u16Val);
INTERFACE void HalPnlSetVSyncEnd(u16 u16Val);
INTERFACE void HalPnlSetVfdeSt(u16 u16Val);
INTERFACE void HalPnlSetVfdeEnd(u16 u16Val);
INTERFACE void HalPnlSetVdeSt(u16 u16Val);
INTERFACE void HalPnlSetVdeEnd(u16 u16Val);
INTERFACE void HalPnlSetVtt(u16 u16Val);
INTERFACE void HalPnlSetHSyncSt(u16 u16Val);
INTERFACE void HalPnlSetHSyncEnd(u16 u16Val);
INTERFACE void HalPnlSetHfdeSt(u16 u16Val);
INTERFACE void HalPnlSetHfdeEnd(u16 u16Val);
INTERFACE void HalPnlSetHdeSt(u16 u16Val);
INTERFACE void HalPnlSetHdeEnd(u16 u16Val);
INTERFACE void HalPnlSetHtt(u16 u16Val);
INTERFACE void HalPnlSetClkInv(bool bEn);
INTERFACE void HalPnlSetVsyncInv(bool bEn);
INTERFACE void HalPnlSetHsyncInv(bool bEn);
INTERFACE void HalPnlSetDeInv(bool bEn);
INTERFACE void HalPnlSetVsynRefMd(bool bEn);
INTERFACE void HalPnlW2BYTEMSK(u32 u32Reg, u16 u16Val, u16 u16Msk);
INTERFACE void HalPnlSetTgenExtHsEn(u8 u8Val);
INTERFACE void HalPnlSetSwReste(u8 u8Val);
INTERFACE void HalPnlSetFifoRest(u8 u8Val);
INTERFACE void HalPnlSetDacHsyncSt(u16 u16Val);
INTERFACE void HalPnlSetDacHsyncEnd(u16 u16Val);
INTERFACE void HalPnlSetDacHdeSt(u16 u16Val);
INTERFACE void HalPnlSetDacHdeEnd(u16 u16Val);
INTERFACE void HalPnlSetDacVdeSt(u16 u16Val);
INTERFACE void HalPnlSetDacVdeEnd(u16 u16Val);
INTERFACE void HalPnlSetDitherEn(bool bEn);
INTERFACE void HalPnlSetRgbMode(HalPnlOutputFormatBitMode_e enFmt);
INTERFACE void HalPnlSetDispToDsiMd(u8 u8Val);
INTERFACE void HalPnlSetLpllSkew(u16 u16Val);
INTERFACE void HalPnlSetRgbSwap(HalPnlRgbSwapType_e enChR, HalPnlRgbSwapType_e enChG, HalPnlRgbSwapType_e enChB);
INTERFACE void HalPnlSetRgbMlSwap(u8 u8Val);
INTERFACE void HalPnlSetSscEn(u8 u8Val);
INTERFACE void HalPnlSetSscSpan(u16 u16Val);
INTERFACE void HalPnlSetSscStep(u16 u16Val);
INTERFACE u32  HalPnlGetRevision(void);
INTERFACE bool HalPnlGetTtlMipiDsiSupported(void);

INTERFACE void HalPnlInitMipiDsiDphy(void);
INTERFACE void HalPnlSetMipiDsiPadOutSel(HalPnlMipiDsiLaneMode_e enLaneMode);
INTERFACE void HalPnlResetMipiDsi(void);
INTERFACE void HalPnlEnableMipiDsiClk(void);
INTERFACE void HalPnlDisableMipiDsiClk(void);
INTERFACE void HalPnlSetMipiDsiLaneNum(HalPnlMipiDsiLaneMode_e enLaneMode);
INTERFACE void HalPnlSetMipiDsiCtrlMode(HalPnlMipiDsiCtrlMode_e enCtrlMode);
INTERFACE bool HalPnlGetMipiDsiClkHsMode(void);
INTERFACE void HalPnlSetMpiDsiClkHsMode(bool bEn);
INTERFACE bool HalPnlGetMipiDsiShortPacket(u8 u8ReadBackCount, u8 u8RegAddr);
INTERFACE bool HalPnlSetMipiDsiShortPacket(HalPnlMipiDsiPacketType_e enPacketType, u8 u8Count, u8 u8Cmd, u8 *pu8ParamList);
INTERFACE bool HalPnlSetMipiDsiLongPacket(HalPnlMipiDsiPacketType_e enPacketType, u8 u8Count, u8 u8Cmd, u8 *pu8ParamList);
INTERFACE bool HalPnlSetMipiDsiPhyTimConfig(HalPnlMipiDsiConfig_t *pstMipiDsiCfg);
INTERFACE bool HalPnlSetMipiDsiVideoTimingConfig(HalPnlMipiDsiConfig_t *pstMipiDsiCfg, HalPnlHwMipiDsiConfig_t *pstHwCfg);
INTERFACE void HalPnlGetMipiDsiReg(u32 u32Addr, u32 *pu32Val);
INTERFACE void HalPnlSetMipiDsiChPolarity(u8 *pu8ChPol);
INTERFACE void HalPnlSetMipiDsiChSel(HalPnlMipiDsiConfig_t *pstMipiDisCfg);

INTERFACE void HalPnlSetMipiDsiPatGen(void);

INTERFACE void HalPnlSetClkHdmi(bool bEn, u32 u32ClkRate);
INTERFACE void HalPnlGetClkHdmi(bool *pbEn, u32 *pu32ClkRate);
INTERFACE void HalPnlSetClkDac(bool bEn, u32 u32ClkRate);
INTERFACE void HalPnlGetClkDac(bool *pbEn, u32 *pu32ClkRate);
INTERFACE void HalPnlSetClkMipiDsiAbp(bool bEn, u32 u32ClkRate);
INTERFACE void HalPnlGetClkMipiDsiAbp(bool *pbEn, u32 *pu32ClkRate);
INTERFACE void HalPnlSetClkScPixel(bool bEn, u32 u32ClkRate);
INTERFACE void HalPnlGetClkScPixel(bool *pbEn, u32 *pu32ClkRate);
INTERFACE void HalPnlSetClkMipiDsi(bool bEn, u32 u32ClkRate);
INTERFACE void HalPnlGetClkMipiDsi(bool *pbEn, u32 *pu32ClkRate);

#undef INTERFACE
#endif
