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

#ifndef _HAL_DISP_PQ_H_
#define _HAL_DISP_PQ_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_DISP_PQ_FLAG_NONE     = 0x0000,
    E_HAL_DISP_PQ_FLAG_BW       = 0x0001,
    E_HAL_DISP_PQ_FLAG_LPF      = 0x0002,
    E_HAL_DISP_PQ_FLAG_HCORING  = 0x0004,
    E_HAL_DISP_PQ_FLAG_PEAKING  = 0x0008,
    E_HAL_DISP_PQ_FLAG_FCC      = 0x0010,
    E_HAL_DISP_PQ_FLAG_DLC_DCR  = 0x0020,
    E_HAL_DISP_PQ_FLAG_GAMMA    = 0x0040,
}HalDispPqFlag_e;


//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
typedef struct
{
    bool bUpdate;
    u8   u8Bw2sbri;
    u8   u8BleEn;
    u8   u8WleEn;
    u8   u8BlackStart;
    u8   u8BlackSlop;
    u8   u8WhiteStart;
    u8   u8WhiteSlop;
}HalDispPqBwExtensionConfig_t;

typedef struct
{
    bool bUpdate;
    u8   u8LpfY;
    u8   u8LpfC;
}HalDispPqLpfConfig_t;


typedef struct
{
    bool bUpdate;
    u16  u16DcrEn;
    u16  u16DcrOffset;

    u16  u16Hactive;
    u16  u16Vactive;
    u16  u16Hblank;

    u16  u16PwmDuty;
    u16  u16PwmPeriod;

    u16  u16DlcEn;
    u16  u16DlcOffset;
    u16  u16DlcGain;
    u16  u16YGain;
    u16  u16YGainOffset;
}HalDispPqDlcDcrConfig_t;


typedef struct
{
    bool bUpdate;
    u8   u8YBand1HCoringEn;
    u8   u8YBand2HCoringEn;
    u8   u8HCoringYDither_En;
    u8   u8YTableStep;

    u8   u8CBand1HCoringEn;
    u8   u8CBand2HCoringEn;
    u8   u8HCoringCDither_En;
    u8   u8CTableStep;

    u8   u8PcMode;
    u8   u8HighPassEn;
    u8   u8CoringTable0;
    u8   u8CoringTable1;
    u8   u8CoringTable2;
    u8   u8CoringTable3;
}HalDispPqHCoringConfig_t;

typedef struct
{
    bool bUpdate;

    u8   u8LtiEn;
    u8   u8LtiCoef;
    u8   u8LtiStep;
    u8   u8LtiMediaFilterOn;

    u8   u8CtiEn;
    u8   u8CtiCoef;
    u8   u8CtiStep;
    u8   u8CtiMediaFilterOn;

    u8   u8DiffAdapEn;
    u8   u8PosLimitTh;
    u8   u8NegLimitTh;

    u8   u8PeakingEn;

    u8   u8Band1Coef;
    u8   u8Band1Step;
    u8   u8Band1DiffAdapEn;
    u8   u8Band1PosLimitTh;
    u8   u8Band1NegLimitTh;

    u8   u8Band2Coef;
    u8   u8Band2Step;
    u8   u8Band2DiffAdapEn;
    u8   u8Band2PosLimitTh;
    u8   u8Band2NegLimitTh;

    u8   u8CoringTh1;
    u8   u8CoringTh2;

    u8   u8PeakingTerm1Sel;
    u8   u8PeakingTerm2Sel;
    u8   u8PeakingTerm3Sel;
    u8   u8PeakingTerm4Sel;
}HalDispPqPeakingConfig_t;

typedef struct
{
    bool bUpdate;
    u8 u8Cb_T1;
    u8 u8Cr_T1;
    u8 u8Cb_T2;
    u8 u8Cr_T2;
    u8 u8Cb_T3;
    u8 u8Cr_T3;
    u8 u8Cb_T4;
    u8 u8Cr_T4;
    u8 u8Cb_T5;
    u8 u8Cr_T5;
    u8 u8Cb_T6;
    u8 u8Cr_T6;
    u8 u8Cb_T7;
    u8 u8Cr_T7;
    u8 u8Cb_T8;
    u8 u8Cr_T8;
    u8 u8CbCr_D1D_D1U;
    u8 u8CbCr_D2D_D2U;
    u8 u8CbCr_D3D_D3U;
    u8 u8CbCr_D4D_D4U;
    u8 u8CbCr_D5D_D5U;
    u8 u8CbCr_D6D_D6U;
    u8 u8CbCr_D7D_D7U;
    u8 u8CbCr_D8D_D8U;
    u8 u8CbCr_D9;
    u8 u8K_T2_K_T1;
    u8 u8K_T4_K_T3;
    u8 u8K_T6_K_T5;
    u8 u8K_T8_K_T7;
    u8 u8En;
}HalDispPqFccConfig_t;

typedef struct
{
    bool bUpdate;
    u8 u8En;
    u8 u8R[33];
    u8 u8G[33];
    u8 u8B[33];
}HalDispPqGammaConfig_t;

typedef struct
{
    HalDispPqFlag_e enFlag;
    HalDispPqBwExtensionConfig_t stBwExtCfg;
    HalDispPqLpfConfig_t stLpfCfg;
    HalDispPqDlcDcrConfig_t stDlcDcrCfg;
    HalDispPqHCoringConfig_t stHCoringCfg;
    HalDispPqPeakingConfig_t stPeakingCfg;
    HalDispPqFccConfig_t stFccCfg;
    HalDispPqGammaConfig_t stGammaCfg;
}HalDispPqHwContext_t;

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _HAL_DISP_PQ_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE void HalDispPqSetBwExtensionConfig(HalDispPqBwExtensionConfig_t *pstBwCfg);
INTERFACE void HalDispPqSetLpfConfig(HalDispPqLpfConfig_t *pstLpfCfg);
INTERFACE void HalDispPqSetDlcDcrConfig(HalDispPqDlcDcrConfig_t *pstDlcDcrCfg);
INTERFACE void HalDispPqSetHCoringConfig(HalDispPqHCoringConfig_t *pstHCoringCfg);
INTERFACE void HalDispPqSetPeaking(HalDispPqPeakingConfig_t *pstPeakingCfg);
INTERFACE void HalDispPqSetFccConfig(HalDispPqFccConfig_t *pstFccCfg);
INTERFACE void HalDispPqSetGammaConfig(HalDispPqGammaConfig_t *pstGammaCfg);

INTERFACE void HalDispPqGetHwCtx(HalDispPqHwContext_t **pstCtx);
INTERFACE void HalDispPqSetHwCtxFlag(HalDispPqFlag_e enFlag);
INTERFACE void HalDispPqClearHwCtxFlag(HalDispPqFlag_e enFlag);

#undef INTERFACE
#endif
