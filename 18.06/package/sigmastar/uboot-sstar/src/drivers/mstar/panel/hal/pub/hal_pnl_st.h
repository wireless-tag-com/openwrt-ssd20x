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

#ifndef _HAL_PNL_ST_H_
#define _HAL_PNL_ST_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------

typedef enum
{
    E_HAL_PNL_QUERY_RET_OK = 0,
    E_HAL_PNL_QUERY_RET_CFGERR,
    E_HAL_PNL_QUERY_RET_NOTSUPPORT,
    E_HAL_PNL_QUERY_RET_NONEED,
}HalPnlQueryRet_e;

typedef enum
{
    E_HAL_PNL_QUERY_PARAM,
    E_HAL_PNL_QUERY_MIPIDSI_PARAM,
    E_HAL_PNL_QUERY_SSC,
    E_HAL_PNL_QUERY_TIMING,
    E_HAL_PNL_QUERY_POWER,
    E_HAL_PNL_QUERY_BACKLIGHT_ONOFF,
    E_HAL_PNL_QUERY_BACKLIGHT_LEVEL,
    E_HAL_PNL_QUERY_CURRENT,
    E_HAL_PNL_QUERY_TESTPAT,
    E_HAL_PNL_QUERY_CLK_SET,
    E_HAL_PNL_QUERY_CLK_GET,
    E_HAL_PNL_QUERY_INIT,
    E_HAL_PNL_QUERY_MAX,
}HalPnlQueryType_e;


typedef enum
{
    E_HAL_PNL_LINK_TTL,                              ///< TTL  type
    E_HAL_PNL_LINK_LVDS,                             ///< LVDS type
    E_HAL_PNL_LINK_RSDS,                             ///< RSDS type
    E_HAL_PNL_LINK_MINILVDS,                         ///< TCON
    E_HAL_PNL_LINK_ANALOG_MINILVDS,                  ///< Analog TCON
    E_HAL_PNL_LINK_DIGITAL_MINILVDS,                 ///< Digital TCON
    E_HAL_PNL_LINK_MFC,                              ///< Ursa (TTL output to Ursa)
    E_HAL_PNL_LINK_DAC_I,                            ///< DAC output
    E_HAL_PNL_LINK_DAC_P,                            ///< DAC output
    E_HAL_PNL_LINK_PDPLVDS,                          ///< For PDP(Vsync use Manually MODE)
    E_HAL_PNL_LINK_EXT,                              ///< EXT LPLL TYPE
    E_HAL_PNL_LINK_MIPI_DSI,                         ///< Mipi DSI
}HalPnlLinkType_e;


typedef enum
{
    E_HAL_PNL_ASPECT_RATIO_4_3    = 0,         ///< set aspect ratio to 4 : 3
    E_HAL_PNL_ASPECT_RATIO_WIDE,               ///< set aspect ratio to 16 : 9
    E_HAL_PNL_ASPECT_RATIO_OTHER,              ///< resvered for other aspect ratio other than 4:3/ 16:9
}HalPnlAspectRatio_e;


typedef enum
{
    E_HAL_PNL_TI_10BIT_MODE = 0,
    E_HAL_PNL_TI_8BIT_MODE = 2,
    E_HAL_PNL_TI_6BIT_MODE = 3,
} HalPnlTiBitMode_e;

/// Define which panel output timing change mode is used to change VFreq for same panel
typedef enum
{
    E_HAL_PNL_CHG_DCLK   = 0,      ///<change output DClk to change Vfreq.
    E_HAL_PNL_CHG_HTOTAL = 1,      ///<change H total to change Vfreq.
    E_HAL_PNL_CHG_VTOTAL = 2,      ///<change V total to change Vfreq.
} HalPnlOutputTimingMode_e;

/// Define panel output format bit mode
typedef enum
{
    E_HAL_PNL_OUTPUT_10BIT_MODE  = 0, //default is 10bit, becasue 8bit panel can use 10bit config and 8bit config.
    E_HAL_PNL_OUTPUT_6BIT_MODE   = 1, //but 10bit panel(like PDP panel) can only use 10bit config.
    E_HAL_PNL_OUTPUT_8BIT_MODE   = 2, //and some PDA panel is 6bit.
    E_HAL_PNL_OUTPUT_565BIT_MODE = 3, // 565
} HalPnlOutputFormatBitMode_e;


typedef enum
{
    E_HAL_PNL_CH_SWAP_0,
    E_HAL_PNL_CH_SWAP_1,
    E_HAL_PNL_CH_SWAP_2,
    E_HAL_PNL_CH_SWAP_3,
    E_HAL_PNL_CH_SWAP_4,
} HalPnlChannelSwapType_e;

typedef enum
{
    E_HAL_PNL_MIPI_DSI_CMD_MODE   = 0,
    E_HAL_PNL_MIPI_DSI_SYNC_PULSE = 1,
    E_HAL_PNL_MIPI_DSI_SYNC_EVENT = 2,
    E_HAL_PNL_MIPI_DSI_BURST_MODE = 3,
} HalPnlMipiDsiCtrlMode_e;

typedef enum
{
    E_HAL_PNL_MIPI_DSI_RGB565         = 0,
    E_HAL_PNL_MIPI_DSI_RGB666         = 1,
    E_HAL_PNL_MIPI_DSI_LOOSELY_RGB666 = 2,
    E_HAL_PNL_MIPI_DSI_RGB888         = 3,
} HalPnlMipiDsiFormat_e;

typedef enum
{
    E_HAL_PNL_MIPI_DSI_LANE_NONE   = 0,
    E_HAL_PNL_MIPI_DSI_LANE_1      = 1,
    E_HAL_PNL_MIPI_DSI_LANE_2      = 2,
    E_HAL_PNL_MIPI_DSI_LANE_3      = 3,
    E_HAL_PNL_MIPI_DSI_LANE_4      = 4,
} HalPnlMipiDsiLaneMode_e;

typedef enum
{
    E_HAL_PNL_MIPI_DSI_PACKET_TYPE_DCS     = 0,
    E_HAL_PNL_MIPI_DSI_PACKET_TYPE_GENERIC = 1,
} HalPnlMipiDsiPacketType_e;

typedef enum
{
    E_HAL_PNL_RGB_SWAP_0 = 0x00,
    E_HAL_PNL_RGB_SWAP_B = 0x01,
    E_HAL_PNL_RGB_SWAP_G = 0x02,
    E_HAL_PNL_RGB_SWAP_R = 0x03,
}HalPnlRgbSwapType_e;
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    HalPnlQueryType_e enQueryType;
    void  *pInCfg;
    u32   u32CfgSize;
}HalPnlQueryInConfig_t;


typedef struct
{
    HalPnlQueryRet_e enQueryRet;
    void (*pSetFunc)(void *, void *);
}HalPnlQueryOutConfig_t;

typedef struct
{
    HalPnlQueryInConfig_t stInCfg;
    HalPnlQueryOutConfig_t stOutCfg;
}HalPnlQueryConfig_t;


typedef struct
{
    const char *pPanelName; ///<  PanelName

#if !defined (__aarch64__)
    u16 u32AlignmentDummy0;
#endif

    u8             u8Dither;         ///<  Diether On?off
    HalPnlLinkType_e eLinkType;     ///<  Panel LinkType

    ///////////////////////////////////////////////
    // Board related setting
    ///////////////////////////////////////////////
    u8 u8DualPort;              ///<  DualPort on/off
    u8 u8SwapPort;              ///<  Swap Port on/off
    u8 u8SwapOdd_ML;            ///<  Swap Odd ML
    u8 u8SwapEven_ML;           ///<  Swap Even ML
    u8 u8SwapOdd_RB;            ///<  Swap Odd RB
    u8 u8SwapEven_RB;           ///<  Swap Even RB

    u8 u8SwapLVDS_POL;          ///<  Swap LVDS Channel Polarity
    u8 u8SwapLVDS_CH;           ///<  Swap LVDS channel
    u8 u8PDP10BIT;              ///<  PDP 10bits on/off
    u8 u8LVDS_TI_MODE;          ///<  Ti Mode On/Off

    ///////////////////////////////////////////////
    // For TTL Only
    ///////////////////////////////////////////////
    u8 u8DCLKDelay;                 ///<  DCLK Delay
    u8 u8InvDCLK;                   ///<  CLK Invert
    u8 u8InvDE;                     ///<  DE Invert
    u8 u8InvHSync;                  ///<  HSync Invert
    u8 u8InvVSync;                  ///<  VSync Invert

    ///////////////////////////////////////////////
    // Output driving current setting
    ///////////////////////////////////////////////
    // driving current setting (0x00=4mA, 0x01=6mA, 0x02=8mA, 0x03=12mA)
    u8 u8DCKLCurrent;              ///< PANEL_DCLK_CURRENT
    u8 u8DECurrent;                ///< PANEL_DE_CURRENT
    u8 u8ODDDataCurrent;           ///< PANEL_ODD_DATA_CURRENT
    u8 u8EvenDataCurrent;          ///< PANEL_EVEN_DATA_CURRENT

    ///////////////////////////////////////////////
    // panel on/off timing
    ///////////////////////////////////////////////
    u16 u16OnTiming1;                ///<  time between panel & data while turn on power
    u16 u16OnTiming2;                ///<  time between data & back light while turn on power
    u16 u16OffTiming1;               ///<  time between back light & data while turn off power
    u16 u16OffTiming2;               ///<  time between data & panel while turn off power

    ///////////////////////////////////////////////
    // panel timing spec.
    ///////////////////////////////////////////////
    // sync related
    u16 u16HSyncWidth;               ///<  Hsync Width
    u16 u16HSyncBackPorch;           ///<  Hsync back porch

    u16 u16VSyncWidth;               ///<  Vsync width
    u16 u16VSyncBackPorch;           ///<  Vsync back porch

    // DE related
    u16 u16HStart;                   ///<  HDe start
    u16 u16VStart;                   ///<  VDe start
    u16 u16Width;                    ///<  Panel Width
    u16 u16Height;                   ///<  Panel Height

    // DClk related
    u16 u16MaxHTotal;                ///<  Max H Total
    u16 u16HTotal;                   ///<  H Total
    u16 u16MinHTotal;                ///<  Min H Total

    u16 u16MaxVTotal;                ///<  Max V Total
    u16 u16VTotal;                   ///<  V Total
    u16 u16MinVTotal;                ///<  Min V Total

    u16 u16MaxDCLK;                  ///<  Max DCLK
    u16 u16DCLK;                     ///<  DCLK ( Htt * Vtt * Fps)
    u16 u16MinDCLK;                  ///<  Min DCLK

    ///<  SSC
    u16 u16SpreadSpectrumStep;       ///<  Step of SSC
    u16 u16SpreadSpectrumSpan;       ///<  Span of SSC

    ///< PWM
    u8 u8PwmPeriodL;                 ///<
    u8 u8PwmPeriodH;                 ///<  Period of Pwm, 2 ~ 262143 Hz
    u8 u8PwmDuty;                    ///<  Duty of Pwm 0 ~ 100

    u8 u8DeinterMode;                ///<  DeInter Mode
    HalPnlAspectRatio_e ePanelAspectRatio; ///<  Aspec Ratio

    /*
    *
    * Board related params
    *
    *  If a board ( like BD_MST064C_D01A_S ) swap LVDS TX polarity
    *    : This polarity swap value =
    *      (LVDS_PN_SWAP_H<<8) | LVDS_PN_SWAP_L from board define,
    *  Otherwise
    *    : The value shall set to 0.
    */
    u16 u16LVDSTxSwapValue;         // LVDS Swap Value
    HalPnlTiBitMode_e eTiBitMode;  // Ti Bit Mode
    HalPnlOutputFormatBitMode_e eOutputFormatBitMode;

    u8 u8SwapOdd_RG;                ///<  Swap Channel R
    u8 u8SwapEven_RG;               ///<  Swap Channel G
    u8 u8SwapOdd_GB;                ///<  Swap Channel B
    u8 u8SwapEven_GB;               ///<  Swap RGB MSB/LSB

    /**
    *  Others
    */
    u8 u8DoubleClk;                             ///<  Double CLK On/off
    u32 u32MaxSET;                              ///<  Max Lpll Set
    u32 u32MinSET;                              ///<  Min Lpll Set
    HalPnlOutputTimingMode_e eOutTimingMode;   ///<  Define which panel output timing change mode is used to change VFreq for same panel
    u8 u8NoiseDith;                             ///<  Noise Dither On/Off
    HalPnlChannelSwapType_e eCh0;              ///<  Channel swap for CH0
    HalPnlChannelSwapType_e eCh1;              ///<  Channel swap for CH1
    HalPnlChannelSwapType_e eCh2;              ///<  Channel swap for CH2
    HalPnlChannelSwapType_e eCh3;              ///<  Channel swap for CH3
    HalPnlChannelSwapType_e eCh4;              ///<  Channel swap for CH4
} HalPnlParamConfig_t;


typedef struct
{
    u8 u8HsTrail;
    u8 u8HsPrpr;
    u8 u8HsZero;
    u8 u8ClkHsPrpr;
    u8 u8ClkHsExit;
    u8 u8ClkTrail;
    u8 u8ClkZero;
    u8 u8ClkHsPost;
    u8 u8DaHsExit;
    u8 u8ContDet;

    u8 u8Lpx;
    u8 u8TaGet;
    u8 u8TaSure;
    u8 u8TaGo;

    u16 u16Hactive;
    u16 u16Hpw;
    u16 u16Hbp;
    u16 u16Hfp;

    u16 u16Vactive;
    u16 u16Vpw;
    u16 u16Vbp;
    u16 u16Vfp;

    u16 u16Bllp;
    u16 u16Fps;

    HalPnlMipiDsiLaneMode_e enLaneNum;
    HalPnlMipiDsiFormat_e enFormat;
    HalPnlMipiDsiCtrlMode_e enCtrl;

    HalPnlChannelSwapType_e enCh[HAL_PNL_SIGNAL_CTRL_CH_MAX];
    u8  *pu8CmdBuf;
    u32 u32CmdBufSize;

    u8  u8SyncCalibrate;
    u16 u16VirHsyncSt;
    u16 u16VirHsyncEnd;
    u16 u16VsyncRef;
    u16 u16DataClkSkew;

    u8  u8PolCh0;  // channel 0 polarity, 0:HW default, 1:positive, 2:negative
    u8  u8PolCh1;  // channel 1 polarity, 0:HW default, 1:positive, 2:negative
    u8  u8PolCh2;  // channel 2 polarity, 0:HW default, 1:positive, 2:negative
    u8  u8PolCh3;  // channel 3 polarity, 0:HW default, 1:positive, 2:negative
    u8  u8PolCh4;  // channel 4 polarity, 0:HW default, 1:positive, 2:negative

    HalPnlMipiDsiPacketType_e enPacketType; // Packet Type;
}HalPnlMipiDsiConfig_t;

typedef struct
{
    bool bEn;
    u16  u16Step;
    u16  u16Span;
}HalPnlSscConfig_t;


typedef struct
{
    u16 u16HSyncWidth;
    u16 u16HSyncBackPorch;
    u16 u16HSyncFrontPorch;

    u16 u16VSyncWidth;
    u16 u16VSyncBackPorch;
    u16 u16VSyncFrontPorch;

    u16 u16HStart;
    u16 u16VStart;

    u16 u16HActive;
    u16 u16VActive;

    u16 u16HTotal;
    u16 u16VTotal;

    u16 u16Dclk;
}HalPnlTimingConfig_t;

typedef struct
{
    bool bEn;
}HalPnlPowerConfig_t;

typedef struct
{
    bool bEn;
}HalPnlBackLightOnOffConfig_t;

typedef struct
{
    u16  u16Duty;
    u16  u16Period;
}HalPnlBackLightLevelConfig_t;

typedef struct
{
    u16 u16Val;
}HalPnlCurrentConfig_t;


typedef struct
{
    bool bEn;
    u16  u16R;
    u16  u16G;
    u16  u16B;
}HalPnlTestPatternConfig_t;

typedef struct
{
    bool bEn[HAL_PNL_CLK_NUM];
    u32  u32Rate[HAL_PNL_CLK_NUM];
    u32 u32Num;
} HalPnlClkConfig_t;

typedef struct
{
    u16 u16VsyncSt;
    u16 u16VsyncWidth;
    u16 u16VsyncBackPorch;
    u16 u16Vactive;
    u16 u16VTotal;
    u16 u16Vstart;

    u16 u16HsyncSt;
    u16 u16HsyncWidth;
    u16 u16HsyncBackPorch;
    u16 u16Hactive;
    u16 u16HTotal;
    u16 u16Hstart;
}HalPnlHwTimeGenConfig_t;

typedef struct
{
    u16 u16LoopGain;
    u16 u16LoopDiv;
    u32 u32Dclk;
    u32 u32Fps;
}HalPnlHwLpllConfig_t;

typedef struct
{
    bool bClkInv;
    bool bDeInv;
    bool bHsyncInv;
    bool bVsyncInv;
    HalPnlRgbSwapType_e enChR;
    HalPnlRgbSwapType_e enChG;
    HalPnlRgbSwapType_e enChB;
    bool bRgbMlSwap;
}HalPnlHwPolarityConfig_t;


typedef struct
{
    u16 u16Vbp;
    u16 u16Vfp;
    u16 u16Vsa;
    u16 u16Vact;
    u16 u16Hbp;
    u16 u16Hfp;
    u16 u16Hsa;
    u16 u16Hact;
}HalPnlHwMipiDsiConfig_t;
typedef struct
{
    u8      u8Type    : 2;
    u8      u8Bta     : 1;
    u8      u8Hs      : 1;
    u8      u8Cl      : 1;
    u8      u8Te      : 1;
    u8      u8Rsv     : 1;
    u8      u8Rpt     : 1;
} HalPnlMipiDsiCmdqConfig_t;

//Type0 Used for DSI short packet read/write command
typedef struct
{
    u8 u8Confg;
    u8 u8DataId;
    u8 u8Data0;
    u8 u8Data1;
} HalPnlMipiDsiT0Ins_t;


//Type2 Used for DSI generic long packet write command
typedef struct
{
    u8  u8Confg      :8;
    u8  u8DataId     :8;
    u16 u16Wc        :16;
    //u8 *pu8data;
} HalPnlMipiDsiT2Ins_t;

//Type3 Used for DSI frame buffer read command (short packet)
typedef struct
{
    u8 u8Confg       : 8;
    u8 u8DataId      : 8;
    u8 u8MemStart0   : 8;
    u8 u8MemStart1   : 8;
} HalPnlMipiDsiT3Ins_t;



typedef struct
{
    u8 byte0;
    u8 byte1;
    u8 byte2;
    u8 byte3;
} HalPnlMipiDsiCmdq_t;

typedef struct
{
    HalPnlMipiDsiCmdq_t data[32];
} HalPnlMipiDsiCmdqRegs_t;

typedef struct
{
    u8 u8Lpx;
    u8 u8HsPrpr;
    u8 u8HsZero;
    u8 u8HsTrail;
} HalPnlMipiDsiPhyTimCon0Reg_t;

typedef struct
{
    u8 u8TaGo;
    u8 u8TaSure;
    u8 u8TaGet;
    u8 u8DaHsExit;
} HalPnlMipiDsiPhyTimCon1Reg_t;


typedef struct
{
    u8 u8ContDet;
    u8 RSV8;
    u8 u8ClkZero;
    u8 u8ClkTrail;
} HalPnlMipiDsiPhyTimCon2Reg_t;


typedef struct
{
    u8 u8ClkHsPrpr;
    u8 u8ClkHsPost;
    u8 u8ClkHsExit;
    u8 u8rsv24 : 8;
} HalPnlMipiDsiPhyTimCon3Reg_t;

#endif

