

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdlib.h>
#include "asm/arch/mach/ms_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"

#include <ubi_uboot.h>
#include <cmd_osd.h>

#if defined(CONFIG_SSTAR_DISP)
#include "mhal_common.h"
#include "mhal_disp_datatype.h"
#include "mhal_disp.h"
#endif

#if defined(CONFIG_SSTAR_PNL)
#include "mhal_pnl_datatype.h"
#include "mhal_pnl.h"
#endif

#if defined(CONFIG_SSTAR_HDMITX)
#include "mhal_hdmitx_datatype.h"
#include "mhal_hdmitx.h"
#endif

#if defined(CONFIG_SSTAR_JPD)
#include "jinclude.h"
#include "jpeglib.h"
#endif

#if defined(CONFIG_MS_PARTITION)
#include "part_mxp.h"
#endif

#if defined(CONFIG_SSTAR_RGN)
#include "mhal_rgn_datatype.h"
#include "mhal_rgn.h"
#endif
#include "blit32.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define PNL_TEST_MD_EN             0

#define BOOTLOGO_DBG_LEVEL_ERR     0x01
#define BOOTLOGO_DBG_LEVEL_INFO    0x02
#define BOOTLOGO_DBG_LEVEL_JPD     0x04

#define FLAG_DELAY            0xFE
#define FLAG_END_OF_TABLE     0xFF   // END OF REGISTERS MARKER

#define BOOTLOGO_DBG_LEVEL          0 // BOOTLOGO_DBG_LEVEL_INFO

#define BOOTLOGO_DBG(dbglv, _fmt, _args...)    \
    do                                          \
    if(dbglv & u32BootlogDvgLevel)              \
    {                                           \
            printf(_fmt, ## _args);             \
    }while(0)



#define DISP_DEVICE_NULL     0
#define DISP_DEVICE_HDMI     1
#define DISP_DEVICE_VGA      2
#define DISP_DEVICE_LCD      4

#define BOOTLOGO_TIMING_NUM  14


#define BOOTLOGO_NOT_ZOOM     0

#define BOOTLOGO_VIRTUAL_ADDRESS_OFFSET 0x20000000

//-------------------------------------------------------------------------------------------------
//  structure & Enu
//-------------------------------------------------------------------------------------------------
/*Base*/
typedef struct
{
    u8 au8Tittle[8];
    u32 u32DataInfoCnt;
}SS_HEADER_Desc_t;
typedef struct
{
    u8 au8DataInfoName[32];
    u32 u32DataTotalSize;
    u32 u32SubHeadSize;
    u32 u32SubNodeCount;
}SS_SHEADER_DataInfo_t;

/*Disp*/
typedef enum
{
    EN_DISPLAY_DEVICE_NULL,
    EN_DISPLAY_DEVICE_LCD,
    EN_DISPLAY_DEVICE_HDMI,
    EN_DISPLAY_DEVICE_VGA
}SS_SHEADER_DisplayDevice_e;
typedef struct
{
    SS_SHEADER_DataInfo_t stDataInfo;
    u32 u32FirstUseOffset;
    u32 u32DispBufSize;
    u32 u32DispBufStart;
}SS_SHEADER_DispInfo_t;

/*HDMI & VGA*/
typedef struct
{
    SS_SHEADER_DisplayDevice_e enDevice;
    u8 au8ResName[32];
    u32 u32Width;
    u32 u32Height;
    u32 u32Clock;
}SS_SHEADER_DispConfig_t;

#if defined(CONFIG_SSTAR_PNL)
/*Panel*/
typedef struct __attribute__((packed)) {
    SS_SHEADER_DisplayDevice_e enDevice;
    u8 au8PanelName[32];
    MhalPnlParamConfig_t stPnlParaCfg;
    MhalPnlMipiDsiConfig_t stMipiDsiCfg;
}SS_SHEADER_PnlPara_t;
#endif

typedef union
{
    SS_SHEADER_DispConfig_t stDispOut;
#if defined(CONFIG_SSTAR_PNL)
    SS_SHEADER_PnlPara_t stPnlPara;
#endif
}SS_SHEADER_DispPnl_u;

/*LOGO*/
typedef enum
{
    EN_PICTURE_DISPLAY_ZOOM,
    EN_PICTURE_DISPLAY_CENTER,
    EN_PICTURE_DISPLAY_USER
}SS_SHEADER_PictureAspectRatio_e;
typedef struct
{
    SS_SHEADER_DataInfo_t stDataInfo;
}SS_SHEADER_PictureDataInfo_t;


typedef struct
{
    MHAL_DISP_DeviceTiming_e enTiminId;
    u16 u16HsyncWidht;
    u16 u16HsyncBacPorch;

    u16 u16VsyncWidht;
    u16 u16VsyncBacPorch;

    u16 u16Hstart;
    u16 u16Vstart;
    u16 u16Hactive;
    u16 u16Vactive;

    u16 u16Htotal;
    u16 u16Vtotal;
    u16 u16DclkMhz;
}DisplayLogoTimingConfig_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

#define BOOTFB_DBG_LEVEL_INFO    0x01
#define BOOTFB_DBG_LEVEL_ERR     0x02

#define BOOTFB_DBG(dbglv, _fmt, _args...)    \
    do                                          \
    if(dbglv>=BOOTFB_DBG_LEVEL_ERR)              \
    {                                           \
            printf(_fmt, ## _args);             \
    }while(0)

#define MAKE_YUYV_VALUE(y,u,v)    ((y) << 24) | ((u) << 16) |((y) << 8) | (v)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define FBDEV_GOP_NUM 2

typedef enum
{
    EN_PIC_ROTATE_NONE,
    EN_PIC_ROTATE_90,
    EN_PIC_ROTATE_180,
    EN_PIC_ROTATE_270
}PIC_ROTATION_e;


u32 u32BootlogDvgLevel = BOOTLOGO_DBG_LEVEL_ERR;

DisplayLogoTimingConfig_t stTimingTable[BOOTLOGO_TIMING_NUM] =
{
    {   E_MHAL_DISP_OUTPUT_1080P60,
        44, 148,  5,  36, 192, 41, 1920, 1080, 2200, 1125, 148 },

    {   E_MHAL_DISP_OUTPUT_1080P50,
        44,  148, 5, 36, 192, 41, 1920, 1080, 2640, 1125, 148 },

    {   E_MHAL_DISP_OUTPUT_720P50,
        40, 220, 5, 20, 260, 25, 1280, 720, 1980, 750, 74},

    {   E_MHAL_DISP_OUTPUT_720P60,
        40, 220,5,20, 260, 25, 1280, 720, 1650, 750, 74},

    {   E_MHAL_DISP_OUTPUT_480P60,
        62, 60, 6, 30, 122, 36, 720, 480, 858, 525, 27},

    {   E_MHAL_DISP_OUTPUT_576P50,
        64, 68, 4, 39, 132, 44, 720, 5760, 864, 625, 27},

    {   E_MHAL_DISP_OUTPUT_1024x768_60,
        136, 160, 6, 29, 296, 35, 1024, 768, 1344, 806, 65},

    {   E_MHAL_DISP_OUTPUT_1366x768_60,
        143, 215, 3, 24, 358, 27, 1366, 768, 1792, 798, 86},

    {   E_MHAL_DISP_OUTPUT_1440x900_60,
        152, 232, 6, 25, 384, 31, 1440, 900, 1904, 934, 106},

    {   E_MHAL_DISP_OUTPUT_1280x800_60,
        128, 200, 6, 22, 328, 28, 1280, 800, 1680, 831, 84},

    {   E_MHAL_DISP_OUTPUT_1280x1024_60,
        112, 248, 3, 38, 360, 41, 1280, 1024, 1688, 1066, 108},

    {   E_MHAL_DISP_OUTPUT_1680x1050_60,
        176, 280, 6, 30, 456, 36, 1680, 1050, 2240, 1089, 146},

    {   E_MHAL_DISP_OUTPUT_1600x1200_60,
        192, 304, 3, 46, 496, 49, 1600, 1200, 2160, 1250, 162},

    {   E_MHAL_DISP_OUTPUT_USER,
        48, 46,  4,  23, 98, 27, 800, 480, 928, 525, 43},
};

#if defined(CONFIG_SSTAR_RGN)
    static u32 gu32FrameBuffer = 0;
    static u32 gu32DispWidth = 0;
    static u32 gu32DispHeight = 0;
    static u32 gu32Rotate = 0;
    static u32 gu32GOPorMOP = 0;
#endif

#if PNL_TEST_MD_EN
MhalPnlParamConfig_t stTtl00x480Param =
{
    "TTL_800x480_60",         // m_pPanelName
    0,                        //
    0,    // m_bPanelDither
    0,    // m_ePanelLinkType  0:TTL,1:LVDS,8:DAC_P

    1,    // m_bPanelDualPort
    0,    // m_bPanelSwapPort          //
    0,    // m_bPanelSwapOdd_ML
    0,    // m_bPanelSwapEven_ML
    0,    // m_bPanelSwapOdd_RB
    0,    // m_bPanelSwapEven_RB


    0,    // u8SwapLVDS_POL;          ///<  Swap LVDS Channel Polarity
    0,    // u8SwapLVDS_CH;           ///<  Swap LVDS channel
    0,    // u8PDP10BIT;              ///<  PDP 10bits on/off
    0,    // u8LVDS_TI_MODE;          ///<  Ti Mode On/Off

    0,    // m_ucPanelDCLKDelay
    0,    // m_bPanelInvDCLK
    0,    // m_bPanelInvDE
    0,    // m_bPanelInvHSync
    0,    // m_bPanelInvVSync
          //
    1,    // m_ucPanelDCKLCurrent
    1,    // m_ucPanelDECurrent
    1,    // m_ucPanelODDDataCurrent
    1,    // _ucPanelEvenDataCurrent
          //
    30,   // m_wPanelOnTiming1
    400,  // m_wPanelOnTiming2
    80,   // _wPanelOffTiming1
    30,   // m_wPanelOffTiming2
          //
    48,   // m_ucPanelHSyncWidth
    46,   // m_ucPanelHSyncBackPorch
          //
    4,    // m_ucPanelVSyncWidth
    23,   // m_ucPanelVBackPorch
          //
    98,   // m_wPanelHStart
    27,   // m_wPanelVStart
          //
    800,  // m_wPanelWidth
    480,  // m_wPanelHeight
          //
    978,  // m_wPanelMaxHTotal
    928,  // m_wPanelHTotal
    878,  // m_wPanelMinHTotal
          //
    818,  // m_wPanelMaxVTotal
    525,  // m_wPanelVTotal
    718,  // m_wPanelMinVTotal
          //
    49,   // m_dwPanelMaxDCLK
    29,   // m_dwPanelDCLK
    37,   // m_dwPanelMinDCLK
          //
    25,   // m_wSpreadSpectrumStep
    192,  // m_wSpreadSpectrumSpan
          //
    160,  // m_ucDimmingCtl
    255,  // m_ucMaxPWMVal
    80,   // m_ucMinPWMVal
          //
    0,    // m_bPanelDeinterMode

    1,    // m_ucPanelAspectRatio

    0,   //u16LVDSTxSwapValue
    2,   // m_ucTiBitMode      TI_10BIT_MODE = 0    TI_8BIT_MODE  = 2    TI_6BIT_MODE  = 3

    2, // m_ucOutputFormatBitMode  10BIT_MODE = 0  6BIT_MODE  = 1   8BIT_MODE  = 2 565BIT_MODE =3

    0, // m_bPanelSwapOdd_RG
    0, // m_bPanelSwapEven_RG
    0, // m_bPanelSwapOdd_GB
    0, // m_bPanelSwapEven_GB

    1, //m_bPanelDoubleClk
    0x001c848e,  //m_dwPanelMaxSET
    0x0018eb59,  //m_dwPanelMinSET

    2, // m_ucOutTimingMode   DCLK=0, HTOTAL=1, VTOTAL=2

    0, // m_bPanelNoiseDith

    0, // m_bPanelChannelSwap0
    1, // m_bPanelChannelSwap1
    2, // m_bPanelChannelSwap2
    3, // m_bPanelChannelSwap3
    4, // m_bPanelChannelSwap4
};


MhalPnlParamConfig_t stRm6820Param =
{
    "Rm6820",         // m_pPanelName
    0,                        //
    0,    // m_bPanelDither
    11,    // m_ePanelLinkType  0:TTL,1:LVDS,8:DAC_P, 11 MIPI_DSI

    1,    // m_bPanelDualPort
    0,    // m_bPanelSwapPort          //
    0,    // m_bPanelSwapOdd_ML
    0,    // m_bPanelSwapEven_ML
    0,    // m_bPanelSwapOdd_RB
    0,    // m_bPanelSwapEven_RB


    0,    // u8SwapLVDS_POL;          ///<  Swap LVDS Channel Polarity
    0,    // u8SwapLVDS_CH;           ///<  Swap LVDS channel
    0,    // u8PDP10BIT;              ///<  PDP 10bits on/off
    0,    // u8LVDS_TI_MODE;          ///<  Ti Mode On/Off

    0,    // m_ucPanelDCLKDelay
    0,    // m_bPanelInvDCLK
    0,    // m_bPanelInvDE
    0,    // m_bPanelInvHSync
    0,    // m_bPanelInvVSync
          //
    1,    // m_ucPanelDCKLCurrent
    1,    // m_ucPanelDECurrent
    1,    // m_ucPanelODDDataCurrent
    1,    // _ucPanelEvenDataCurrent

    30,   // m_wPanelOnTiming1
    400,  // m_wPanelOnTiming2
    80,   // _wPanelOffTiming1
    30,   // m_wPanelOffTiming2

    6,    // m_ucPanelHSyncWidth
    60,  // m_ucPanelHSyncBackPorch

    40,    // m_ucPanelVSyncWidth
    220,   // m_ucPanelVBackPorch
          //
    66,   // m_wPanelHStart
    260,   // m_wPanelVStart
          //
    720,  // m_wPanelWidth
    1280,  // m_wPanelHeight
          //
    850,  // m_wPanelMaxHTotal
    830,  // m_wPanelHTotal
    750,  // m_wPanelMinHTotal
          //
    1750,  // m_wPanelMaxVTotal
    1650,  // m_wPanelVTotal
    1550,  // m_wPanelMinVTotal
          //
    89,   // m_dwPanelMaxDCLK
    79,   // m_dwPanelDCLK
    69,   // m_dwPanelMinDCLK
          //
    25,   // m_wSpreadSpectrumStep
    192,  // m_wSpreadSpectrumSpan
          //
    160,  // m_ucDimmingCtl
    255,  // m_ucMaxPWMVal
    80,   // m_ucMinPWMVal
          //
    0,    // m_bPanelDeinterMode

    1,    // m_ucPanelAspectRatio

    0,   //u16LVDSTxSwapValue
    2,   // m_ucTiBitMode      TI_10BIT_MODE = 0    TI_8BIT_MODE  = 2    TI_6BIT_MODE  = 3

    2, // m_ucOutputFormatBitMode  10BIT_MODE = 0  6BIT_MODE  = 1   8BIT_MODE  = 2 565BIT_MODE =3

    0, // m_bPanelSwapOdd_RG
    0, // m_bPanelSwapEven_RG
    0, // m_bPanelSwapOdd_GB
    0, // m_bPanelSwapEven_GB

    1, //m_bPanelDoubleClk
    0x001c848e,  //m_dwPanelMaxSET
    0x0018eb59,  //m_dwPanelMinSET

    2, // m_ucOutTimingMode   DCLK=0, HTOTAL=1, VTOTAL=2

    0, // m_bPanelNoiseDith

    2, // m_bPanelChannelSwap0
    4, // m_bPanelChannelSwap1
    3, // m_bPanelChannelSwap2
    1, // m_bPanelChannelSwap3
    0, // m_bPanelChannelSwap4
};

u8 Rm6820TestCmd[] =
{
    0xFE, 1, 0x01,
    0x27, 1, 0x0A,
    0x29, 1, 0x0A,
    0x2B, 1, 0xE5,
    0x24, 1, 0xC0,
    0x25, 1, 0x53,
    0x26, 1, 0x00,
    0x16, 1, 0x52, //wrong
    0x2F, 1, 0x54,
    0x34, 1, 0x57,
    0x1B, 1, 0x00,
    0x12, 1, 0x0A,
    0x1A, 1, 0x06,
    0x46, 1, 0x4D,
    0x52, 1, 0x90,
    0x53, 1, 0x00,
    0x54, 1, 0x90,
    0x55, 1, 0x00,
    FLAG_DELAY, FLAG_DELAY, 200,
    0xFE, 1, 0x03,
    0x00, 1, 0x05,
    0x01, 1, 0x16,
    0x02, 1, 0x09,
    0x03, 1, 0x0D,
    0x04, 1, 0x00,
    0x05, 1, 0x00,
    0x06, 1, 0x50,
    0x07, 1, 0x05,
    0x08, 1, 0x16,
    0x09, 1, 0x0B,
    0x0A, 1, 0x0F,
    0x0B, 1, 0x00,
    0x0C, 1, 0x00,
    0x0D, 1, 0x50,
    0x0E, 1, 0x03,
    0x0F, 1, 0x04,
    0x10, 1, 0x05,
    0x11, 1, 0x06,
    0x12, 1, 0x00,
    0x13, 1, 0x54,
    0x14, 1, 0x00,
    0x15, 1, 0xC5,
    0x16, 1, 0x08,
    0x17, 1, 0x07,
    0x18, 1, 0x08,
    0x19, 1, 0x09,
    0x1A, 1, 0x0A,
    0x1B, 1, 0x00,
    0x1C, 1, 0x54,
    0x1D, 1, 0x00,
    0x1E, 1, 0x85,
    0x1F, 1, 0x08,
    0x20, 1, 0x00,
    0x21, 1, 0x00,
    0x22, 1, 0x03,
    0x23, 1, 0x1F,
    0x24, 1, 0x00,
    0x25, 1, 0x28,
    0x26, 1, 0x00,
    0x27, 1, 0x1F,
    0x28, 1, 0x00,
    0x29, 1, 0x28,
    0x2A, 1, 0x00,
    0x2B, 1, 0x00,
    0x2D, 1, 0x00,
    0x2F, 1, 0x00,
    0x30, 1, 0x00,
    0x31, 1, 0x00,
    0x32, 1, 0x00,
    0x33, 1, 0x00,
    0x34, 1, 0x00,
    0x35, 1, 0x00,
    0x36, 1, 0x00,
    0x37, 1, 0x00,
    0x38, 1, 0x00,
    0x39, 1, 0x00,
    0x3A, 1, 0x00,
    0x3B, 1, 0x00,
    0x3D, 1, 0x00,
    0x3F, 1, 0x00,
    0x40, 1, 0x00,
    0x3F, 1, 0x00,
    0x41, 1, 0x00,
    0x42, 1, 0x00,
    0x43, 1, 0x00,
    0x44, 1, 0x00,
    0x45, 1, 0x00,
    0x46, 1, 0x00,
    0x47, 1, 0x00,
    0x48, 1, 0x00,
    0x49, 1, 0x00,
    0x4A, 1, 0x00,
    0x4B, 1, 0x00,
    0x4C, 1, 0x00,
    0x4D, 1, 0x00,
    0x4E, 1, 0x00,
    0x4F, 1, 0x00,
    0x50, 1, 0x00,
    0x51, 1, 0x00,
    0x52, 1, 0x00,
    0x53, 1, 0x00,
    0x54, 1, 0x00,
    0x55, 1, 0x00,
    0x56, 1, 0x00,
    0x58, 1, 0x00,
    0x59, 1, 0x00,
    0x5A, 1, 0x00,
    0x5B, 1, 0x00,
    0x5C, 1, 0x00,
    0x5D, 1, 0x00,
    0x5E, 1, 0x00,
    0x5F, 1, 0x00,
    0x60, 1, 0x00,
    0x61, 1, 0x00,
    0x62, 1, 0x00,
    0x63, 1, 0x00,
    0x64, 1, 0x00,
    0x65, 1, 0x00,
    0x66, 1, 0x00,
    0x67, 1, 0x00,
    0x68, 1, 0x00,
    0x69, 1, 0x00,
    0x6A, 1, 0x00,
    0x6B, 1, 0x00,
    0x6C, 1, 0x00,
    0x6D, 1, 0x00,
    0x6E, 1, 0x00,
    0x6F, 1, 0x00,
    0x70, 1, 0x00,
    0x71, 1, 0x00,
    0x72, 1, 0x00,
    0x73, 1, 0x00,
    0x74, 1, 0x04,
    0x75, 1, 0x04,
    0x76, 1, 0x04,
    0x77, 1, 0x04,
    0x78, 1, 0x00,
    0x79, 1, 0x00,
    0x7A, 1, 0x00,
    0x7B, 1, 0x00,
    0x7C, 1, 0x00,
    0x7D, 1, 0x00,
    0x7E, 1, 0x86,
    0x7F, 1, 0x02,
    0x80, 1, 0x0E,
    0x81, 1, 0x0C,
    0x82, 1, 0x0A,
    0x83, 1, 0x08,
    0x84, 1, 0x3F,
    0x85, 1, 0x3F,
    0x86, 1, 0x3F,
    0x87, 1, 0x3F,
    0x88, 1, 0x3F,
    0x89, 1, 0x3F,
    0x8A, 1, 0x3F,
    0x8B, 1, 0x3F,
    0x8C, 1, 0x3F,
    0x8D, 1, 0x3F,
    0x8E, 1, 0x3F,
    0x8F, 1, 0x3F,
    0x90, 1, 0x00,
    0x91, 1, 0x04,
    0x92, 1, 0x3F,
    0x93, 1, 0x3F,
    0x94, 1, 0x3F,
    0x95, 1, 0x3F,
    0x96, 1, 0x05,
    0x97, 1, 0x01,
    0x98, 1, 0x3F,
    0x99, 1, 0x3F,
    0x9A, 1, 0x3F,
    0x9B, 1, 0x3F,
    0x9C, 1, 0x3F,
    0x9D, 1, 0x3F,
    0x9E, 1, 0x3F,
    0x9F, 1, 0x3F,
    0xA0, 1, 0x3F,
    0xA2, 1, 0x3F,
    0xA3, 1, 0x3F,
    0xA4, 1, 0x3F,
    0xA5, 1, 0x09,
    0xA6, 1, 0x0B,
    0xA7, 1, 0x0D,
    0xA9, 1, 0x0F,
    FLAG_DELAY, FLAG_DELAY, 10,
    0xAA, 1, 0x03, // wrong
    FLAG_DELAY, FLAG_DELAY, 10,
    0xAB, 1, 0x07, //wrong
    FLAG_DELAY, FLAG_DELAY, 10,
    0xAC, 1, 0x01,
    0xAD, 1, 0x05,
    0xAE, 1, 0x0D,
    0xAF, 1, 0x0F,
    0xB0, 1, 0x09,
    0xB1, 1, 0x0B,
    0xB2, 1, 0x3F,
    0xB3, 1, 0x3F,
    0xB4, 1, 0x3F,
    0xB5, 1, 0x3F,
    0xB6, 1, 0x3F,
    0xB7, 1, 0x3F,
    0xB8, 1, 0x3F,
    0xB9, 1, 0x3F,
    0xBA, 1, 0x3F,
    0xBB, 1, 0x3F,
    0xBC, 1, 0x3F,
    0xBD, 1, 0x3F,
    0xBE, 1, 0x07,
    0xBF, 1, 0x03,
    0xC0, 1, 0x3F,
    0xC1, 1, 0x3F,
    0xC2, 1, 0x3F,
    0xC3, 1, 0x3F,
    0xC4, 1, 0x02,
    0xC5, 1, 0x06,
    0xC6, 1, 0x3F,
    0xC7, 1, 0x3F,
    0xC8, 1, 0x3F,
    0xC9, 1, 0x3F,
    0xCA, 1, 0x3F,
    0xCB, 1, 0x3F,
    0xCC, 1, 0x3F,
    0xCD, 1, 0x3F,
    0xCE, 1, 0x3F,
    0xCF, 1, 0x3F,
    0xD0, 1, 0x3F,
    0xD1, 1, 0x3F,
    0xD2, 1, 0x0A,
    0xD3, 1, 0x08,
    0xD4, 1, 0x0E,
    0xD5, 1, 0x0C,
    0xD6, 1, 0x04,
    0xD7, 1, 0x00,
    0xDC, 1, 0x02,
    0xDE, 1, 0x10,
    FLAG_DELAY, FLAG_DELAY, 200,
    0xFE, 1, 0x04,
    0x60, 1, 0x00,
    0x61, 1, 0x0C,
    0x62, 1, 0x14,
    0x63, 1, 0x0F,
    0x64, 1, 0x08,
    0x65, 1, 0x15,
    0x66, 1, 0x0F,
    0x67, 1, 0x0B,
    0x68, 1, 0x17,
    0x69, 1, 0x0D,
    0x6A, 1, 0x10,
    0x6B, 1, 0x09,
    0x6C, 1, 0x0F,
    0x6D, 1, 0x11,
    0x6E, 1, 0x0B,
    0x6F, 1, 0x00,
    0x70, 1, 0x00,
    0x71, 1, 0x0C,
    0x72, 1, 0x14,
    0x73, 1, 0x0F,
    0x74, 1, 0x08,
    0x75, 1, 0x15,
    0x76, 1, 0x0F,
    0x77, 1, 0x0B,
    0x78, 1, 0x17,
    0x79, 1, 0x0D,
    0x7A, 1, 0x10,
    0x7B, 1, 0x09,
    0x7C, 1, 0x0F,
    0x7D, 1, 0x11,
    0x7E, 1, 0x0B,
    0x7F, 1, 0x00,
    FLAG_DELAY, FLAG_DELAY, 200,
    0xFE, 1, 0x0E,
    0x01, 1, 0x75,
    0x49, 1, 0x56,
    FLAG_DELAY, FLAG_DELAY, 200,
    0xFE, 1, 0x00,
    0x58, 1, 0xA9,
    0x11, 0, 0x00,
    FLAG_DELAY, FLAG_DELAY, 200,
    0x29, 0, 0x00,
    FLAG_DELAY, FLAG_DELAY, 200,
    0xFE, 1, 0x0E,
    0x35, 1, 0x80,
    FLAG_END_OF_TABLE, FLAG_END_OF_TABLE,
};


u8 Rm6820Cmd[] =
{
    0xFE, 1, 0x01,
    0x27, 1, 0x0A,
    0x29, 1, 0x0A,
    0x2B, 1, 0xE5,
    0x24, 1, 0xC0,
    0x25, 1, 0x53,
    0x26, 1, 0x00,
    0x16, 1, 0x52, //wrong
    0x2F, 1, 0x54,
    0x34, 1, 0x57,
    0x1B, 1, 0x00,
    0x12, 1, 0x0A,
    0x1A, 1, 0x06,
    0x46, 1, 0x4D,
    0x52, 1, 0x90,
    0x53, 1, 0x00,
    0x54, 1, 0x90,
    0x55, 1, 0x00,
    FLAG_DELAY, FLAG_DELAY, 200,
    0xFE, 1, 0x03,
    0x00, 1, 0x05,
    0x01, 1, 0x16,
    0x02, 1, 0x09,
    0x03, 1, 0x0D,
    0x04, 1, 0x00,
    0x05, 1, 0x00,
    0x06, 1, 0x50,
    0x07, 1, 0x05,
    0x08, 1, 0x16,
    0x09, 1, 0x0B,
    0x0A, 1, 0x0F,
    0x0B, 1, 0x00,
    0x0C, 1, 0x00,
    0x0D, 1, 0x50,
    0x0E, 1, 0x03,
    0x0F, 1, 0x04,
    0x10, 1, 0x05,
    0x11, 1, 0x06,
    0x12, 1, 0x00,
    0x13, 1, 0x54,
    0x14, 1, 0x00,
    0x15, 1, 0xC5,
    0x16, 1, 0x08,
    0x17, 1, 0x07,
    0x18, 1, 0x08,
    0x19, 1, 0x09,
    0x1A, 1, 0x0A,
    0x1B, 1, 0x00,
    0x1C, 1, 0x54,
    0x1D, 1, 0x00,
    0x1E, 1, 0x85,
    0x1F, 1, 0x08,
    0x20, 1, 0x00,
    0x21, 1, 0x00,
    0x22, 1, 0x03,
    0x23, 1, 0x1F,
    0x24, 1, 0x00,
    0x25, 1, 0x28,
    0x26, 1, 0x00,
    0x27, 1, 0x1F,
    0x28, 1, 0x00,
    0x29, 1, 0x28,
    0x2A, 1, 0x00,
    0x2B, 1, 0x00,
    0x2D, 1, 0x00,
    0x2F, 1, 0x00,
    0x30, 1, 0x00,
    0x31, 1, 0x00,
    0x32, 1, 0x00,
    0x33, 1, 0x00,
    0x34, 1, 0x00,
    0x35, 1, 0x00,
    0x36, 1, 0x00,
    0x37, 1, 0x00,
    0x38, 1, 0x00,
    0x39, 1, 0x00,
    0x3A, 1, 0x00,
    0x3B, 1, 0x00,
    0x3D, 1, 0x00,
    0x3F, 1, 0x00,
    0x40, 1, 0x00,
    0x3F, 1, 0x00,
    0x41, 1, 0x00,
    0x42, 1, 0x00,
    0x43, 1, 0x00,
    0x44, 1, 0x00,
    0x45, 1, 0x00,
    0x46, 1, 0x00,
    0x47, 1, 0x00,
    0x48, 1, 0x00,
    0x49, 1, 0x00,
    0x4A, 1, 0x00,
    0x4B, 1, 0x00,
    0x4C, 1, 0x00,
    0x4D, 1, 0x00,
    0x4E, 1, 0x00,
    0x4F, 1, 0x00,
    0x50, 1, 0x00,
    0x51, 1, 0x00,
    0x52, 1, 0x00,
    0x53, 1, 0x00,
    0x54, 1, 0x00,
    0x55, 1, 0x00,
    0x56, 1, 0x00,
    0x58, 1, 0x00,
    0x59, 1, 0x00,
    0x5A, 1, 0x00,
    0x5B, 1, 0x00,
    0x5C, 1, 0x00,
    0x5D, 1, 0x00,
    0x5E, 1, 0x00,
    0x5F, 1, 0x00,
    0x60, 1, 0x00,
    0x61, 1, 0x00,
    0x62, 1, 0x00,
    0x63, 1, 0x00,
    0x64, 1, 0x00,
    0x65, 1, 0x00,
    0x66, 1, 0x00,
    0x67, 1, 0x00,
    0x68, 1, 0x00,
    0x69, 1, 0x00,
    0x6A, 1, 0x00,
    0x6B, 1, 0x00,
    0x6C, 1, 0x00,
    0x6D, 1, 0x00,
    0x6E, 1, 0x00,
    0x6F, 1, 0x00,
    0x70, 1, 0x00,
    0x71, 1, 0x00,
    0x72, 1, 0x00,
    0x73, 1, 0x00,
    0x74, 1, 0x04,
    0x75, 1, 0x04,
    0x76, 1, 0x04,
    0x77, 1, 0x04,
    0x78, 1, 0x00,
    0x79, 1, 0x00,
    0x7A, 1, 0x00,
    0x7B, 1, 0x00,
    0x7C, 1, 0x00,
    0x7D, 1, 0x00,
    0x7E, 1, 0x86,
    0x7F, 1, 0x02,
    0x80, 1, 0x0E,
    0x81, 1, 0x0C,
    0x82, 1, 0x0A,
    0x83, 1, 0x08,
    0x84, 1, 0x3F,
    0x85, 1, 0x3F,
    0x86, 1, 0x3F,
    0x87, 1, 0x3F,
    0x88, 1, 0x3F,
    0x89, 1, 0x3F,
    0x8A, 1, 0x3F,
    0x8B, 1, 0x3F,
    0x8C, 1, 0x3F,
    0x8D, 1, 0x3F,
    0x8E, 1, 0x3F,
    0x8F, 1, 0x3F,
    0x90, 1, 0x00,
    0x91, 1, 0x04,
    0x92, 1, 0x3F,
    0x93, 1, 0x3F,
    0x94, 1, 0x3F,
    0x95, 1, 0x3F,
    0x96, 1, 0x05,
    0x97, 1, 0x01,
    0x98, 1, 0x3F,
    0x99, 1, 0x3F,
    0x9A, 1, 0x3F,
    0x9B, 1, 0x3F,
    0x9C, 1, 0x3F,
    0x9D, 1, 0x3F,
    0x9E, 1, 0x3F,
    0x9F, 1, 0x3F,
    0xA0, 1, 0x3F,
    0xA2, 1, 0x3F,
    0xA3, 1, 0x3F,
    0xA4, 1, 0x3F,
    0xA5, 1, 0x09,
    0xA6, 1, 0x0B,
    0xA7, 1, 0x0D,
    0xA9, 1, 0x0F,
    FLAG_DELAY, FLAG_DELAY, 10,
    0xAA, 1, 0x03, // wrong
    FLAG_DELAY, FLAG_DELAY, 10,
    0xAB, 1, 0x07, //wrong
    FLAG_DELAY, FLAG_DELAY, 10,
    0xAC, 1, 0x01,
    0xAD, 1, 0x05,
    0xAE, 1, 0x0D,
    0xAF, 1, 0x0F,
    0xB0, 1, 0x09,
    0xB1, 1, 0x0B,
    0xB2, 1, 0x3F,
    0xB3, 1, 0x3F,
    0xB4, 1, 0x3F,
    0xB5, 1, 0x3F,
    0xB6, 1, 0x3F,
    0xB7, 1, 0x3F,
    0xB8, 1, 0x3F,
    0xB9, 1, 0x3F,
    0xBA, 1, 0x3F,
    0xBB, 1, 0x3F,
    0xBC, 1, 0x3F,
    0xBD, 1, 0x3F,
    0xBE, 1, 0x07,
    0xBF, 1, 0x03,
    0xC0, 1, 0x3F,
    0xC1, 1, 0x3F,
    0xC2, 1, 0x3F,
    0xC3, 1, 0x3F,
    0xC4, 1, 0x02,
    0xC5, 1, 0x06,
    0xC6, 1, 0x3F,
    0xC7, 1, 0x3F,
    0xC8, 1, 0x3F,
    0xC9, 1, 0x3F,
    0xCA, 1, 0x3F,
    0xCB, 1, 0x3F,
    0xCC, 1, 0x3F,
    0xCD, 1, 0x3F,
    0xCE, 1, 0x3F,
    0xCF, 1, 0x3F,
    0xD0, 1, 0x3F,
    0xD1, 1, 0x3F,
    0xD2, 1, 0x0A,
    0xD3, 1, 0x08,
    0xD4, 1, 0x0E,
    0xD5, 1, 0x0C,
    0xD6, 1, 0x04,
    0xD7, 1, 0x00,
    0xDC, 1, 0x02,
    0xDE, 1, 0x10,
    FLAG_DELAY, FLAG_DELAY, 200,
    0xFE, 1, 0x04,
    0x60, 1, 0x00,
    0x61, 1, 0x0C,
    0x62, 1, 0x14,
    0x63, 1, 0x0F,
    0x64, 1, 0x08,
    0x65, 1, 0x15,
    0x66, 1, 0x0F,
    0x67, 1, 0x0B,
    0x68, 1, 0x17,
    0x69, 1, 0x0D,
    0x6A, 1, 0x10,
    0x6B, 1, 0x09,
    0x6C, 1, 0x0F,
    0x6D, 1, 0x11,
    0x6E, 1, 0x0B,
    0x6F, 1, 0x00,
    0x70, 1, 0x00,
    0x71, 1, 0x0C,
    0x72, 1, 0x14,
    0x73, 1, 0x0F,
    0x74, 1, 0x08,
    0x75, 1, 0x15,
    0x76, 1, 0x0F,
    0x77, 1, 0x0B,
    0x78, 1, 0x17,
    0x79, 1, 0x0D,
    0x7A, 1, 0x10,
    0x7B, 1, 0x09,
    0x7C, 1, 0x0F,
    0x7D, 1, 0x11,
    0x7E, 1, 0x0B,
    0x7F, 1, 0x00,
    FLAG_DELAY, FLAG_DELAY, 200,
    0xFE, 1, 0x0E,
    0x01, 1, 0x75,
    0x49, 1, 0x56,
    FLAG_DELAY, FLAG_DELAY, 200,
    0xFE, 1, 0x00,
    0x58, 1, 0xA9,
    0x11, 0, 0x00,
    FLAG_DELAY, FLAG_DELAY, 200,
    0x29, 0, 0x00,
    FLAG_DELAY, FLAG_DELAY, 200,
    FLAG_END_OF_TABLE, FLAG_END_OF_TABLE,
};


MhalPnlMipiDsiConfig_t stRm6820MipiCfg =
{
    5,     // HsTrail
    3,     // HsPrpr
    5,     // HsZero
    10,    // ClkHsPrpr
    14,    // ClkHsExit
    3,     // ClkTrail
    12,    // ClkZero
    10,    // ClkHsPost
    5,     // DaHsExit
    0,     // ContDet

    16,    // Lpx
    26,    // TaGet
    24,    // TaSure
    50,    // TaGo

    720,   // Hactive
    6,     // Hpw
    60,    // Hbp
    44,    // Hfp
    1280,  // Vactive
    40,    // Vpw
    220,   // Vbp
    110,   // Vfp
    0,     // Bllp
    60,    // Fps

    4,     // LaneNum
    3,     // Format   0:RGB565, 1:RGB666, 2:Loosely_RGB666, 3:RGB888
    1,     // CtrlMode 1:Sync pulse 2:Sync_event, 3:Burst
    Rm6820TestCmd,
    sizeof(Rm6820TestCmd),
};

#endif

#if defined(CONFIG_CMD_MTDPARTS)
#include <jffs2/jffs2.h>
/* partition handling routines */
int mtdparts_init(void);
int find_dev_and_part(const char *id, struct mtd_device **dev,
        u8 *part_num, struct part_info **part);
#endif


//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------
MS_S32 BootLogoMemAlloc(MS_U8 *pu8Name, MS_U32 size, unsigned long long *pu64PhyAddr)
{
    return 0;
}

MS_S32 BootLogoMemRelease(unsigned long long u64PhyAddr)
{
    return 0;
}

#if defined(CONFIG_SSTAR_DISP)
MHAL_DISP_DeviceTiming_e _BootLogoGetTiminId(u16 u16Width, u16 u16Height, u8 u8Rate)
{
    MHAL_DISP_DeviceTiming_e enTiming;
    enTiming =  ((u16Width) == 1920 && (u16Height) == 1080 && (u8Rate) == 24) ? E_MHAL_DISP_OUTPUT_1080P24 :
                ((u16Width) == 1920 && (u16Height) == 1080 && (u8Rate) == 25) ? E_MHAL_DISP_OUTPUT_1080P25 :
                ((u16Width) == 1920 && (u16Height) == 1080 && (u8Rate) == 30) ? E_MHAL_DISP_OUTPUT_1080P30 :
                ((u16Width) == 1920 && (u16Height) == 1080 && (u8Rate) == 50) ? E_MHAL_DISP_OUTPUT_1080P50 :
                ((u16Width) == 1920 && (u16Height) == 1080 && (u8Rate) == 60) ? E_MHAL_DISP_OUTPUT_1080P60 :
                ((u16Width) == 1280 && (u16Height) == 720  && (u8Rate) == 50) ? E_MHAL_DISP_OUTPUT_720P50  :
                ((u16Width) == 1280 && (u16Height) == 720  && (u8Rate) == 60) ? E_MHAL_DISP_OUTPUT_720P60  :
                ((u16Width) == 720  && (u16Height) == 480  && (u8Rate) == 60) ? E_MHAL_DISP_OUTPUT_480P60  :
                ((u16Width) == 720  && (u16Height) == 576  && (u8Rate) == 60) ? E_MHAL_DISP_OUTPUT_576P50  :
                ((u16Width) == 640  && (u16Height) == 480  && (u8Rate) == 60) ? E_MHAL_DISP_OUTPUT_640x480_60   :
                ((u16Width) == 800  && (u16Height) == 600  && (u8Rate) == 60) ? E_MHAL_DISP_OUTPUT_800x600_60   :
                ((u16Width) == 1280 && (u16Height) == 1024 && (u8Rate) == 60) ? E_MHAL_DISP_OUTPUT_1280x1024_60 :
                ((u16Width) == 1366 && (u16Height) == 768  && (u8Rate) == 60) ? E_MHAL_DISP_OUTPUT_1366x768_60  :
                ((u16Width) == 1440 && (u16Height) == 800  && (u8Rate) == 60) ? E_MHAL_DISP_OUTPUT_1440x900_60  :
                ((u16Width) == 1280 && (u16Height) == 800  && (u8Rate) == 60) ? E_MHAL_DISP_OUTPUT_1280x800_60  :
                ((u16Width) == 3840 && (u16Height) == 2160 && (u8Rate) == 30) ? E_MHAL_DISP_OUTPUT_3840x2160_30 :
                                                                                E_MHAL_DISP_OUTPUT_MAX;
    return enTiming;
}
#endif

#if defined(CONFIG_SSTAR_HDMITX)
MhaHdmitxTimingResType_e _BootLogoGetHdmitxTimingId(u16 u16Width, u16 u16Height, u8 u8Rate)
{
    MhaHdmitxTimingResType_e enTiming;
    enTiming =  ((u16Width) == 1920 && (u16Height) == 1080 && (u8Rate) == 24) ? E_MHAL_HDMITX_RES_1920X1080P_24HZ :
                ((u16Width) == 1920 && (u16Height) == 1080 && (u8Rate) == 25) ? E_MHAL_HDMITX_RES_1920X1080P_25HZ :
                ((u16Width) == 1920 && (u16Height) == 1080 && (u8Rate) == 30) ? E_MHAL_HDMITX_RES_1920X1080P_30HZ :
                ((u16Width) == 1920 && (u16Height) == 1080 && (u8Rate) == 50) ? E_MHAL_HDMITX_RES_1920X1080P_50HZ :
                ((u16Width) == 1920 && (u16Height) == 1080 && (u8Rate) == 60) ? E_MHAL_HDMITX_RES_1920X1080P_60HZ :
                ((u16Width) == 1280 && (u16Height) == 720  && (u8Rate) == 50) ? E_MHAL_HDMITX_RES_1280X720P_50HZ  :
                ((u16Width) == 1280 && (u16Height) == 720  && (u8Rate) == 60) ? E_MHAL_HDMITX_RES_1280X720P_60HZ  :
                ((u16Width) == 720  && (u16Height) == 480  && (u8Rate) == 60) ? E_MHAL_HDMITX_RES_720X480P_60HZ  :
                ((u16Width) == 720  && (u16Height) == 576  && (u8Rate) == 60) ? E_MHAL_HDMITX_RES_720X576P_50HZ  :
                ((u16Width) == 640  && (u16Height) == 480  && (u8Rate) == 60) ? E_MHAL_HDMITX_RES_MAX   :
                ((u16Width) == 800  && (u16Height) == 600  && (u8Rate) == 60) ? E_MHAL_HDMITX_RES_MAX   :
                ((u16Width) == 1280 && (u16Height) == 1024 && (u8Rate) == 60) ? E_MHAL_HDMITX_RES_1280X1024P_60HZ :
                ((u16Width) == 1366 && (u16Height) == 768  && (u8Rate) == 60) ? E_MHAL_HDMITX_RES_1366X768P_60HZ  :
                ((u16Width) == 1440 && (u16Height) == 900  && (u8Rate) == 60) ? E_MHAL_HDMITX_RES_1440X900P_60HZ  :
                ((u16Width) == 1280 && (u16Height) == 800  && (u8Rate) == 60) ? E_MHAL_HDMITX_RES_1280X800P_60HZ  :
                ((u16Width) == 3840 && (u16Height) == 2160 && (u8Rate) == 30) ? E_MHAL_HDMITX_RES_MAX :
                                                                                E_MHAL_HDMITX_RES_MAX;
    return enTiming;
}
#endif

void _BootLogoDispPnlInit(void)
{
#if defined(CONFIG_SSTAR_DISP)
    MHAL_DISP_PanelConfig_t stPnlCfg[BOOTLOGO_TIMING_NUM];
    u16 i;


    if( sizeof(stTimingTable)/sizeof(DisplayLogoTimingConfig_t) > BOOTLOGO_TIMING_NUM)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d:: Timing Talbe is bigger than %d\n",
            __FUNCTION__, __LINE__, BOOTLOGO_TIMING_NUM);
        return;
    }
    memset(stPnlCfg, 0, sizeof(MHAL_DISP_PanelConfig_t)*BOOTLOGO_TIMING_NUM);
    for(i=0; i<BOOTLOGO_TIMING_NUM; i++)
    {
        stPnlCfg[i].bValid = 1;
        stPnlCfg[i].eTiming = stTimingTable[i].enTiminId;
        stPnlCfg[i].stPanelAttr.m_ucPanelHSyncWidth     = stTimingTable[i].u16HsyncWidht;
        stPnlCfg[i].stPanelAttr.m_ucPanelHSyncBackPorch = stTimingTable[i].u16HsyncBacPorch;
        stPnlCfg[i].stPanelAttr.m_ucPanelVSyncWidth     = stTimingTable[i].u16VsyncWidht;
        stPnlCfg[i].stPanelAttr.m_ucPanelVBackPorch     = stTimingTable[i].u16VsyncBacPorch;
        stPnlCfg[i].stPanelAttr.m_wPanelHStart          = stTimingTable[i].u16Hstart;
        stPnlCfg[i].stPanelAttr.m_wPanelVStart          = stTimingTable[i].u16Vstart;
        stPnlCfg[i].stPanelAttr.m_wPanelWidth           = stTimingTable[i].u16Hactive;
        stPnlCfg[i].stPanelAttr.m_wPanelHeight          = stTimingTable[i].u16Vactive;
        stPnlCfg[i].stPanelAttr.m_wPanelHTotal          = stTimingTable[i].u16Htotal;
        stPnlCfg[i].stPanelAttr.m_wPanelVTotal          = stTimingTable[i].u16Vtotal;
        stPnlCfg[i].stPanelAttr.m_dwPanelDCLK           = stTimingTable[i].u16DclkMhz;
    }


    MHAL_DISP_InitPanelConfig(stPnlCfg, BOOTLOGO_TIMING_NUM);
#endif
}
#define YUV444_TO_YUV420_PIXEL_MAPPING(y_dst_addr, uv_dst_addr, dst_x, dst_y, dst_stride, src_addr, src_x, src_y, src_w, src_h) do {   \
        for (src_y = 0; src_y < src_h; src_y++) \
        {   \
            for (src_x = 0; src_x < src_w; src_x++) \
            {   \
                *((char *)((char *)(y_dst_addr) + (dst_y) * (dst_stride) + (dst_x)))    \
                    = *((char *)((char *)(src_addr) + (src_y) * (src_w * 3) + (src_x * 3)));  \
                if ((src_y & 0x01) && (src_x & 0x01))   \
                {   \
                    *((short *)((char *)(uv_dst_addr) + ((dst_y - 1) >> 1) * (dst_stride) + (dst_x - 1)))    \
                        = *((short *)((char *)(src_addr) + (src_y) * (src_w * 3) + (src_x * 3) + 1));  \
                }   \
            }   \
        }   \
    }while(0)

void _BootLogoYuv444ToYuv420(u8 *pu8InBuf, u8 *pu8OutBuf, u16 *pu16Width, u16 *pu16Height, PIC_ROTATION_e eRot)
{
    u16 x, y;

    u8 *pu8DesY = NULL, *pu8DesUV = NULL;;
    u8 *pu8SrcYUV = NULL;

    pu8SrcYUV = pu8InBuf;

    pu8DesY = pu8OutBuf;
    pu8DesUV = pu8DesY + (*pu16Width) * (*pu16Height);

    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_JPD,"%s %d:: 444 To 422, In:%x, Out:%x, Width:%d, Height:%d\n",
        __FUNCTION__, __LINE__,
        (u32)pu8InBuf, (u32)pu8OutBuf, *pu16Width, *pu16Height);

    switch (eRot)
    {
        case EN_PIC_ROTATE_NONE:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, x, y, *pu16Width, pu8SrcYUV, x, y, *pu16Width, *pu16Height);
        }
        break;
        case EN_PIC_ROTATE_90:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, *pu16Height - y, x, *pu16Height, pu8SrcYUV, x, y, *pu16Width, *pu16Height);
            *pu16Width ^= *pu16Height;
            *pu16Height ^= *pu16Width;
            *pu16Width ^= *pu16Height;
        }
        break;
        case EN_PIC_ROTATE_180:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, (*pu16Width - x), (*pu16Height - y - 1), *pu16Width, pu8SrcYUV, x, y, *pu16Width, *pu16Height);
        }
        break;
        case EN_PIC_ROTATE_270:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, y, (*pu16Width - x - 1), *pu16Height, pu8SrcYUV, x, y, *pu16Width, *pu16Height);
            *pu16Width ^= *pu16Height;
            *pu16Height ^= *pu16Width;
            *pu16Width ^= *pu16Height;
        }
        break;
        default:
            return;
    }
}

#if defined(CONFIG_SSTAR_RGN)
#define RGB_PIXEL_MAPPING(dst_addr, dst_x, dst_y, dst_stride, src_addr, src_x, src_y, src_stride, src_w, src_h, type) do { \
        for (src_y = 0; src_y < src_h; src_y++) \
        {   \
            for (src_x = 0; src_x < src_w; src_x++) \
            {   \
                *((type *)((char *)(dst_addr) + (dst_y) * (dst_stride) + (dst_x) * sizeof(type)))    \
                    = *((type *)((char *)(src_addr) + (src_y) * (src_stride) + (src_x) * sizeof(type)));  \
            }   \
        }   \
    }while(0)

static void _BootLogoRgbRotate(u8 *pDstBuf, u8 *pSrcBuf, u16 u16Width, u16 u16Height, PIC_ROTATION_e eRot, u8 u8BytePerPixel)
{
    u16 x = 0, y = 0;
    switch (eRot)
    {
        case EN_PIC_ROTATE_90:
        {
            if (u8BytePerPixel == 2)
            {
                RGB_PIXEL_MAPPING(pDstBuf, u16Height - y - 1, x, u16Height * u8BytePerPixel, pSrcBuf, x, y, u8BytePerPixel * u16Width, u16Width, u16Height, u16);
            }
            else if (u8BytePerPixel == 4)
            {
                RGB_PIXEL_MAPPING(pDstBuf, u16Height - y - 1, x, u16Height * u8BytePerPixel, pSrcBuf, x, y, u8BytePerPixel * u16Width, u16Width, u16Height, u32);
            }
        }
        break;
        case EN_PIC_ROTATE_180:
        {
            if (u8BytePerPixel == 2)
            {
                RGB_PIXEL_MAPPING(pDstBuf, (u16Width - x - 1), (u16Height - y - 1), u8BytePerPixel * u16Width, pSrcBuf, x, y, u8BytePerPixel * u16Width, u16Width, u16Height, u16);
            }
            else if (u8BytePerPixel == 4)
            {
                RGB_PIXEL_MAPPING(pDstBuf, (u16Width - x - 1), (u16Height - y - 1), u8BytePerPixel * u16Width, pSrcBuf, x, y, u8BytePerPixel * u16Width, u16Width, u16Height, u32);
            }
        }
        break;
        case EN_PIC_ROTATE_270:
        {
            if (u8BytePerPixel == 2)
            {
                RGB_PIXEL_MAPPING(pDstBuf, y, (u16Width - x - 1), u16Height * u8BytePerPixel, pSrcBuf, x, y, u8BytePerPixel * u16Width, u16Width, u16Height, u16);
            }
            else if (u8BytePerPixel == 4)
            {
                RGB_PIXEL_MAPPING(pDstBuf, y, (u16Width - x - 1), u16Height * u8BytePerPixel, pSrcBuf, x, y, u8BytePerPixel * u16Width, u16Width, u16Height, u32);
            }
        }
        break;
        default:
            return;
    }
}
static void _BootJpdArgbCtrl(u32 u32InBufSzie, u32 u32InBuf, u32 u32OutBufSize, u32 u32OutBuf, u16 *pu16OutWidth, u16 *pu16OutHeight, PIC_ROTATION_e eRot)
{
#if defined(CONFIG_SSTAR_JPD)
    u32 u32JpgSize;
    u8 *pu8JpgBuffer;

    // Variables for the decompressor itself
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    u8* dstbuffer = NULL;
    u8 *framebuffer;
    u8* linebuffer;
    u8* optbuffer;
    u16 u16RowStride, u16Width, u16Height, u16PixelSize, u16FbPixleSize;
    int rc; //, i, j;

    u32JpgSize = u32OutBufSize;
    pu8JpgBuffer = (unsigned char *)(u32InBuf);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, pu8JpgBuffer, u32JpgSize);
    rc = jpeg_read_header(&cinfo, TRUE);

    if (rc != 1)
    {
        return;
    }

    cinfo.out_color_space = JCS_RGB;

    jpeg_start_decompress(&cinfo);

    u16Width = cinfo.output_width;
    u16Height = cinfo.output_height;
    u16PixelSize = cinfo.output_components;
    *pu16OutWidth = u16Width;
    *pu16OutHeight = u16Height;

    framebuffer = (unsigned char *)(u32OutBuf + 0x20000000);

    u16RowStride = u16Width * u16PixelSize;
    linebuffer = malloc(u16RowStride);
    if(!linebuffer)
        return;


#if 1 //ARGB8888
    u16FbPixleSize = 4;
    if (eRot != EN_PIC_ROTATE_NONE)
    {
        dstbuffer = (unsigned char *)malloc(u16Width * u16Height * u16FbPixleSize);
        if (!dstbuffer)
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d:: Can not alloc opt buffer.\n", __FUNCTION__, __LINE__);
            free(linebuffer);

            return;
        }
        optbuffer = dstbuffer;
    }
    else
    {
        optbuffer = framebuffer;
    }
    while (cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *buffer_array[1];
        buffer_array[0] = linebuffer ;
        u8* pixel=linebuffer;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
        for(int i = 0;i<u16Width;i++,pixel+=cinfo.output_components)
        {
            *(((int*)optbuffer)+i) = 0xFF<<24|(*(pixel))<<16|(*(pixel+1))<<8|(*(pixel+2));
        }
        optbuffer+=u16Width*4;
    }
#endif
#if 0 //ARGB1555
    u16FbPixleSize = 2;
    if (eRot != EN_PIC_ROTATE_NONE)
    {
        dstbuffer = (unsigned char *)malloc(u16Width * u16Height * u16FbPixleSize);
        if (!dstbuffer)
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d:: Can not alloc opt buffer.\n", __FUNCTION__, __LINE__);
            free(linebuffer);

            return;
        }
        optbuffer = dstbuffer;
    }
    else
    {
        optbuffer = framebuffer;
    }
    while (cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *buffer_array[1];
        buffer_array[0] = linebuffer ;
        u8* pixel=linebuffer;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
        for(int i = 0;i<u16Width;i++,pixel+=cinfo.output_components)
        {
            *(((u16*)optbuffer)+i) = 0x1<<15|(*(pixel)&0xF8)<<7|(*(pixel+1)&0xF8)<<2|(*(pixel+2)&0xF8)>>3;
        }
        optbuffer+=u16Width*2;
    }
#endif
#if 0 //ARGB4444
    u16FbPixleSize = 2;
    if (eRot != EN_PIC_ROTATE_NONE)
    {
        dstbuffer = (unsigned char *)malloc(u16Width * u16Height * u16FbPixleSize);
        if (!dstbuffer)
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d:: Can not alloc opt buffer.\n", __FUNCTION__, __LINE__);
            free(linebuffer);

            return;
        }
        optbuffer = dstbuffer;
    }
    else
    {
        optbuffer = framebuffer;
    }
    while (cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *buffer_array[1];
        buffer_array[0] = linebuffer ;
        u8* pixel=linebuffer;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
        for(int i = 0;i<u16Width;i++,pixel+=cinfo.output_components)
        {
            *(((u16*)optbuffer)+i) = 0xF<<12|(*(pixel)&0xF0)<<4|(*(pixel+1)&0xF0)|(*(pixel+2)&0xF0)>>4;
        }
        optbuffer+=u16Width*2;
    }
#endif

    if (dstbuffer != NULL && eRot != EN_PIC_ROTATE_NONE)
    {
        _BootLogoRgbRotate(framebuffer, dstbuffer, u16Width, u16Height, eRot, u16FbPixleSize);
        free(dstbuffer);
        if (eRot == EN_PIC_ROTATE_90 || eRot == EN_PIC_ROTATE_270)
        {
            *pu16OutWidth = u16Height;
            *pu16OutHeight = u16Width;
        }
    }
    jpeg_finish_decompress(&cinfo);

    jpeg_destroy_decompress(&cinfo);
    free(linebuffer);
#endif
}
#endif
static void _BootJpdYuvCtrl(u32 u32InBufSzie, u32 u32InBuf, u32 u32OutBufSize, u32 u32OutBuf, u16 *pu16OutWidth, u16 *pu16OutHeight, PIC_ROTATION_e eRot)
{
#if defined(CONFIG_SSTAR_JPD)
    // Variables for the source jpg
    u32 u32JpgSize;
    u8 *pu8JpgBuffer;

    // Variables for the decompressor itself
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // Variables for the output buffer, and how long each row is
    u32 u32BmpSize;
    u8 *pu8BmpBuffer;

    u32 u32Yuv420Size;
    u8 *pu8Yuv420Buffer;

    u16 u16RowStride, u16Width, u16Height, u16PixelSize;

    int rc; //, i, j;

    u32JpgSize = u32InBufSzie;
    pu8JpgBuffer = (unsigned char *)u32InBuf;

    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_JPD,"%s %d::  Create Decompress struct\n", __FUNCTION__, __LINE__);
    // Allocate a new decompress struct, with the default error handler.
    // The default error handler will exit() on pretty much any issue,
    // so it's likely you'll want to replace it or supplement it with
    // your own.
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_JPD,"%s %d::  Set memory buffer as source\n", __FUNCTION__, __LINE__);
    // Configure this decompressor to read its data from a memory
    // buffer starting at unsigned char *pu8JpgBuffer, which is u32JpgSize
    // long, and which must contain a complete jpg already.
    //
    // If you need something fancier than this, you must write your
    // own data source manager, which shouldn't be too hard if you know
    // what it is you need it to do. See jpeg-8d/jdatasrc.c for the
    // implementation of the standard jpeg_mem_src and jpeg_stdio_src
    // managers as examples to work from.
    jpeg_mem_src(&cinfo, pu8JpgBuffer, u32JpgSize);

    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_JPD, "%s %d::  Read the JPEG header\n", __FUNCTION__, __LINE__);
    // Have the decompressor scan the jpeg header. This won't populate
    // the cinfo struct output fields, but will indicate if the
    // jpeg is valid.
    rc = jpeg_read_header(&cinfo, TRUE);

    if (rc != 1)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d:: File does not seem to be a normal JPEG\n", __FUNCTION__, __LINE__);
        return;
    }


    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_JPD,"%s %d::  Initiate JPEG decompression\n", __FUNCTION__, __LINE__);

    // output color space is yuv444 packet
    cinfo.out_color_space = JCS_YCbCr;

    jpeg_start_decompress(&cinfo);

    u16Width = cinfo.output_width;
    u16Height = cinfo.output_height;
    u16PixelSize = cinfo.output_components;

    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_JPD, "%s %d::  Image is %d by %d with %d components\n",
        __FUNCTION__, __LINE__, u16Width, u16Height, u16PixelSize);


    u32BmpSize = u16Width * u16Height * u16PixelSize;
    pu8BmpBuffer =(u8 *) malloc(u32BmpSize);

    if(pu8BmpBuffer == NULL)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d:: malloc fail\n", __FUNCTION__, __LINE__);
        return;
    }

    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_JPD, "%s %d:: BmpBuffer: 0x%x\n", __FUNCTION__, __LINE__, (u32)pu8BmpBuffer);
    u32Yuv420Size = u16Width * u16Height * 3 / 2;
    pu8Yuv420Buffer = (unsigned char *)(u32OutBuf + BOOTLOGO_VIRTUAL_ADDRESS_OFFSET);

    if( u32Yuv420Size > u32OutBufSize)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR,"%s %d:: Output buffer is too big, %d\n",
            __FUNCTION__, __LINE__, u16Width * u16Height * u16PixelSize);
        return;
    }

    u16RowStride = u16Width * u16PixelSize;

    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_JPD,"%s %d:: Start reading scanlines\n", __FUNCTION__, __LINE__);
    while (cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *buffer_array[1];
        buffer_array[0] = pu8BmpBuffer + \
                           (cinfo.output_scanline) * u16RowStride;

        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_JPD,"%s %d:: Done reading scanlines\n", __FUNCTION__, __LINE__);
    jpeg_finish_decompress(&cinfo);

    jpeg_destroy_decompress(&cinfo);

    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_JPD,"%s %d:: End of decompression\n", __FUNCTION__, __LINE__);
    _BootLogoYuv444ToYuv420(pu8BmpBuffer, pu8Yuv420Buffer, &u16Width, &u16Height, eRot);
    *pu16OutWidth = u16Width;
    *pu16OutHeight = u16Height;

    free(pu8BmpBuffer);
#endif
}

void _BootDispCtrl(SS_SHEADER_DispPnl_u *puDispPnlCfg, SS_SHEADER_DispInfo_t *pstDispInfo, u32 u32Shift,
                        SS_SHEADER_PictureAspectRatio_e eAspectRatio, u32 u32DstX, u32 u32DstY,
                        u16 u16ImgWidth, u16 u16ImgHeight)
{
#if defined(CONFIG_SSTAR_DISP)
    SS_SHEADER_DisplayDevice_e *penDevice = NULL;

    MHAL_DISP_AllocPhyMem_t stPhyMem;
    MHAL_DISP_DeviceTimingInfo_t stTimingInfo;
    static void *pDevCtx = NULL;
    static void *pVidLayerCtx = NULL;
    static void *pInputPortCtx = NULL;
    u32 u32Interface = 0;
    u32 u32DispDbgLevel;
    u16 u16DispOutWidht = 0;
    u16 u16DispOutHeight = 0;

    stPhyMem.alloc = BootLogoMemAlloc;
    stPhyMem.free  = BootLogoMemRelease;

    u32DispDbgLevel = 0;//0x1F;
    MHAL_DISP_DbgLevel(&u32DispDbgLevel);

    //Inint Pnl Tbl
    _BootLogoDispPnlInit();

    penDevice = (SS_SHEADER_DisplayDevice_e *)puDispPnlCfg;
    if(*penDevice == EN_DISPLAY_DEVICE_LCD)
    {
        u32Interface = MHAL_DISP_INTF_LCD;
    }
    else
    {
        if (*penDevice == EN_DISPLAY_DEVICE_HDMI)
            u32Interface = MHAL_DISP_INTF_HDMI;
        else if (*penDevice == EN_DISPLAY_DEVICE_VGA)
            u32Interface = MHAL_DISP_INTF_VGA;
    }

    if(pDevCtx == NULL)
    {
        if(MHAL_DISP_DeviceCreateInstance(&stPhyMem, 0, &pDevCtx) == FALSE)
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d, CreateDevice Fail\n", __FUNCTION__, __LINE__);
            return;
        }
    }

    if(pVidLayerCtx == NULL)
    {
        if(MHAL_DISP_VideoLayerCreateInstance(&stPhyMem, 0, &pVidLayerCtx) == FALSE)
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d, CreateVideoLayer Fail\n", __FUNCTION__, __LINE__);
            return;

        }
    }

    if(pInputPortCtx == NULL)
    {
        if(MHAL_DISP_InputPortCreateInstance(&stPhyMem, pVidLayerCtx, 0, &pInputPortCtx) == FALSE)
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d, CreaetInputPort Fail\n", __FUNCTION__, __LINE__);
            return;
        }
    }
    MHAL_DISP_DeviceSetBackGroundColor(pDevCtx, 0x800080);
    MHAL_DISP_DeviceEnable(pDevCtx, 0);
    MHAL_DISP_DeviceAddOutInterface(pDevCtx, u32Interface);
#if defined(CONFIG_SSTAR_PNL)
    if(u32Interface == MHAL_DISP_INTF_LCD)
    {
        MHAL_DISP_SyncInfo_t stSyncInfo;
    
        if(puDispPnlCfg->stPnlPara.au8PanelName[0] != 0)
        {
            stSyncInfo.u16Vact = puDispPnlCfg->stPnlPara.stPnlParaCfg.u16Height;
            stSyncInfo.u16Vbb  = puDispPnlCfg->stPnlPara.stPnlParaCfg.u16VSyncBackPorch;
            stSyncInfo.u16Vpw  = puDispPnlCfg->stPnlPara.stPnlParaCfg.u16VSyncWidth;
            stSyncInfo.u16Vfb  = puDispPnlCfg->stPnlPara.stPnlParaCfg.u16VTotal - stSyncInfo.u16Vact - stSyncInfo.u16Vbb - stSyncInfo.u16Vpw;
            stSyncInfo.u16Hact = puDispPnlCfg->stPnlPara.stPnlParaCfg.u16Width;
            stSyncInfo.u16Hbb  = puDispPnlCfg->stPnlPara.stPnlParaCfg.u16HSyncBackPorch;
            stSyncInfo.u16Hpw  = puDispPnlCfg->stPnlPara.stPnlParaCfg.u16HSyncWidth;
            stSyncInfo.u16Hfb  = puDispPnlCfg->stPnlPara.stPnlParaCfg.u16HTotal - stSyncInfo.u16Hact - stSyncInfo.u16Hbb - stSyncInfo.u16Hpw;
            stSyncInfo.u32FrameRate = puDispPnlCfg->stPnlPara.stMipiDsiCfg.u16Fps;
    
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "%s %d, H(%d %d %d %d) V(%d %d %d %d) Fps:%d\n",
                __FUNCTION__, __LINE__,
                stSyncInfo.u16Hfb, stSyncInfo.u16Hpw, stSyncInfo.u16Hbb, stSyncInfo.u16Hact,
                stSyncInfo.u16Vfb, stSyncInfo.u16Vpw, stSyncInfo.u16Vbb, stSyncInfo.u16Vact,
                stSyncInfo.u32FrameRate);
    
            stTimingInfo.eTimeType = E_MHAL_DISP_OUTPUT_USER;
            stTimingInfo.pstSyncInfo = &stSyncInfo;
            MHAL_DISP_DeviceSetOutputTiming(pDevCtx, MHAL_DISP_INTF_LCD, &stTimingInfo);
            u16DispOutWidht = puDispPnlCfg->stPnlPara.stPnlParaCfg.u16Width;
            u16DispOutHeight = puDispPnlCfg->stPnlPara.stPnlParaCfg.u16Height;
        }
        else
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d, No stPnlPara Fail\n", __FUNCTION__, __LINE__);
            return;
        }
    }
    else
#endif
    {
        stTimingInfo.eTimeType = _BootLogoGetTiminId(puDispPnlCfg->stDispOut.u32Width,
                                                     puDispPnlCfg->stDispOut.u32Height,
                                                     puDispPnlCfg->stDispOut.u32Clock);
        stTimingInfo.pstSyncInfo = NULL;
        MHAL_DISP_DeviceSetOutputTiming(pDevCtx, u32Interface, &stTimingInfo);
        u16DispOutWidht = puDispPnlCfg->stDispOut.u32Width;
        u16DispOutHeight = puDispPnlCfg->stDispOut.u32Height;
    }
    MHAL_DISP_DeviceEnable(pDevCtx, 1);
#if !defined(CONFIG_SSTAR_RGN)
    MHAL_DISP_VideoFrameData_t stVideoFrameBuffer;
    MHAL_DISP_InputPortAttr_t stInputAttr;

    memset(&stInputAttr, 0, sizeof(MHAL_DISP_InputPortAttr_t));
    memset(&stVideoFrameBuffer, 0, sizeof(MHAL_DISP_VideoFrameData_t));    
    stInputAttr.u16SrcWidth = min(u16DispOutWidht, u16ImgWidth);
    stInputAttr.u16SrcHeight = min(u16DispOutHeight, u16ImgHeight);
    switch (eAspectRatio)
    {
        case EN_PICTURE_DISPLAY_ZOOM:
        {
            stInputAttr.stDispWin.u16X = 0;
            stInputAttr.stDispWin.u16Y = 0;
            stInputAttr.stDispWin.u16Width = u16DispOutWidht;
            stInputAttr.stDispWin.u16Height = u16DispOutHeight;
        }
        break;
        case EN_PICTURE_DISPLAY_CENTER:
        {
            stInputAttr.stDispWin.u16X = (u16DispOutWidht - stInputAttr.u16SrcWidth) / 2;
            stInputAttr.stDispWin.u16Y = (u16DispOutHeight - stInputAttr.u16SrcHeight) / 2;
            stInputAttr.stDispWin.u16Width = stInputAttr.u16SrcWidth;
            stInputAttr.stDispWin.u16Height = stInputAttr.u16SrcHeight;
        }
        break;
        case EN_PICTURE_DISPLAY_USER:
        {
            stInputAttr.stDispWin.u16X = u32DstX;
            stInputAttr.stDispWin.u16Y = u32DstY;
            stInputAttr.stDispWin.u16Width = stInputAttr.u16SrcWidth;
            stInputAttr.stDispWin.u16Height = stInputAttr.u16SrcHeight;
        }
        break;
        default:
            return;
    }
    stVideoFrameBuffer.ePixelFormat = E_MHAL_PIXEL_FRAME_YUV_MST_420;
    stVideoFrameBuffer.aPhyAddr[0] = (MS_PHYADDR)(pstDispInfo->u32DispBufStart + u32Shift);
    stVideoFrameBuffer.aPhyAddr[1] = (MS_PHYADDR)(pstDispInfo->u32DispBufStart + u32Shift + u16ImgWidth * u16ImgHeight);
    stVideoFrameBuffer.au32Stride[0] = u16ImgWidth;
    MHAL_DISP_InputPortSetAttr(pInputPortCtx, &stInputAttr);
    MHAL_DISP_InputPortFlip(pInputPortCtx, &stVideoFrameBuffer);
    MHAL_DISP_InputPortEnable(pInputPortCtx, TRUE);
#else
    if (0 == gu32GOPorMOP)
    {
        //BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "CONFIG_SSTAR_RGN enable, func:%s, line:%d\n", __FUNCTION__,__LINE__);
        MHAL_DISP_VideoFrameData_t stVideoFrameBuffer;
        MHAL_DISP_InputPortAttr_t stInputAttr;

        memset(&stInputAttr, 0, sizeof(MHAL_DISP_InputPortAttr_t));
        memset(&stVideoFrameBuffer, 0, sizeof(MHAL_DISP_VideoFrameData_t));    
        stInputAttr.u16SrcWidth = min(u16DispOutWidht, u16ImgWidth);
        stInputAttr.u16SrcHeight = min(u16DispOutHeight, u16ImgHeight);
        switch (eAspectRatio)
        {
            case EN_PICTURE_DISPLAY_ZOOM:
            {
                stInputAttr.stDispWin.u16X = 0;
                stInputAttr.stDispWin.u16Y = 0;
                stInputAttr.stDispWin.u16Width = u16DispOutWidht;
                stInputAttr.stDispWin.u16Height = u16DispOutHeight;
            }
            break;
            case EN_PICTURE_DISPLAY_CENTER:
            {
                stInputAttr.stDispWin.u16X = (u16DispOutWidht - stInputAttr.u16SrcWidth) / 2;
                stInputAttr.stDispWin.u16Y = (u16DispOutHeight - stInputAttr.u16SrcHeight) / 2;
                stInputAttr.stDispWin.u16Width = stInputAttr.u16SrcWidth;
                stInputAttr.stDispWin.u16Height = stInputAttr.u16SrcHeight;
            }
            break;
            case EN_PICTURE_DISPLAY_USER:
            {
                stInputAttr.stDispWin.u16X = u32DstX;
                stInputAttr.stDispWin.u16Y = u32DstY;
                stInputAttr.stDispWin.u16Width = stInputAttr.u16SrcWidth;
                stInputAttr.stDispWin.u16Height = stInputAttr.u16SrcHeight;
            }
            break;
            default:
                return;
        }
        stVideoFrameBuffer.ePixelFormat = E_MHAL_PIXEL_FRAME_YUV_MST_420;
        stVideoFrameBuffer.aPhyAddr[0] = (MS_PHYADDR)(pstDispInfo->u32DispBufStart + u32Shift);
        stVideoFrameBuffer.aPhyAddr[1] = (MS_PHYADDR)(pstDispInfo->u32DispBufStart + u32Shift + u16ImgWidth * u16ImgHeight);
        stVideoFrameBuffer.au32Stride[0] = u16ImgWidth;
        MHAL_DISP_InputPortSetAttr(pInputPortCtx, &stInputAttr);
        MHAL_DISP_InputPortFlip(pInputPortCtx, &stVideoFrameBuffer);
        MHAL_DISP_InputPortEnable(pInputPortCtx, TRUE);
    }
#endif
#if defined(CONFIG_SSTAR_PNL)
    static void *pPnlDev = NULL;
    u32 u32DbgLevel;

    if(u32Interface == MHAL_DISP_INTF_LCD && puDispPnlCfg->stPnlPara.au8PanelName[0] != 0)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "%s %d, PnlLink:%d\n",
            __FUNCTION__, __LINE__, puDispPnlCfg->stPnlPara.stPnlParaCfg.eLinkType);

        if (pPnlDev == NULL)
        {
            u32DbgLevel = 0; //0x0F;
            MhalPnlSetDebugLevel((void *)&u32DbgLevel);
            if(MhalPnlCreateInstance(&pPnlDev, puDispPnlCfg->stPnlPara.stPnlParaCfg.eLinkType) == FALSE)
            {
                BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "%s %d, PnlCreateInstance Fail\n", __FUNCTION__, __LINE__);
                return;
            }
            MhalPnlSetParamConfig(pPnlDev, &puDispPnlCfg->stPnlPara.stPnlParaCfg);
            
            if(puDispPnlCfg->stPnlPara.stPnlParaCfg.eLinkType == E_MHAL_PNL_LINK_MIPI_DSI)
            {
                MhalPnlSetMipiDsiConfig(pPnlDev, &puDispPnlCfg->stPnlPara.stMipiDsiCfg);
            }
        }
    }
#elif defined(CONFIG_SSTAR_HDMITX)
    static void *pHdmitxCtx = NULL;
    MhalHdmitxAttrConfig_t stAttrCfg;
    MhalHdmitxSignalConfig_t stSignalCfg;
    MhalHdmitxMuteConfig_t stMuteCfg;
    MhalHdmitxHpdConfig_t stHpdCfg;

    if(u32Interface == MHAL_DISP_INTF_HDMI)
    {
        if(pHdmitxCtx == NULL)
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO,  "interface %d w %d h %d clock %d\n", u32Interface, puDispPnlCfg->stDispOut.u32Width, puDispPnlCfg->stDispOut.u32Height, puDispPnlCfg->stDispOut.u32Clock);
            if(MhalHdmitxCreateInstance(&pHdmitxCtx, 0) != E_MHAL_HDMITX_RET_SUCCESS)
            {
                printf("%s %d, CreateInstance Fail\n", __FUNCTION__, __LINE__);
                return;
            }
            //MhalHdmitxSetDebugLevel(pHdmitxCtx, 0x3F);
            
            stHpdCfg.u8GpioNum = 26;
            MhalHdmitxSetHpdConfig(pHdmitxCtx, &stHpdCfg);
            
            stMuteCfg.enType = E_MHAL_HDMITX_MUTE_AUDIO | E_MHAL_HDMITX_MUTE_VIDEO | E_MHAL_HDMITX_MUTE_AVMUTE;
            stMuteCfg.bMute = 1;
            MhalHdmitxSetMute(pHdmitxCtx, &stMuteCfg);
            
            stSignalCfg.bEn = 0;
            MhalHdmitxSetSignal(pHdmitxCtx, &stSignalCfg);
            
            stAttrCfg.bVideoEn = 1;
            stAttrCfg.enInColor    = E_MHAL_HDMITX_COLOR_RGB444;
            stAttrCfg.enOutColor   = E_MHAL_HDMITX_COLOR_RGB444;
            stAttrCfg.enOutputMode = E_MHAL_HDMITX_OUTPUT_MODE_HDMI;
            stAttrCfg.enColorDepth = E_MHAL_HDMITX_CD_24_BITS;
            stAttrCfg.enTiming     = _BootLogoGetHdmitxTimingId(puDispPnlCfg->stDispOut.u32Width, puDispPnlCfg->stDispOut.u32Height, puDispPnlCfg->stDispOut.u32Clock);
            
            stAttrCfg.bAudioEn = 1;
            stAttrCfg.enAudioFreq = E_MHAL_HDMITX_AUDIO_FREQ_48K;
            stAttrCfg.enAudioCh   = E_MHAL_HDMITX_AUDIO_CH_2;
            stAttrCfg.enAudioFmt  = E_MHAL_HDMITX_AUDIO_FORMAT_PCM;
            stAttrCfg.enAudioCode = E_MHAL_HDMITX_AUDIO_CODING_PCM;
            MhalHdmitxSetAttr(pHdmitxCtx, &stAttrCfg);
            
            stSignalCfg.bEn = 1;
            MhalHdmitxSetSignal(pHdmitxCtx, &stSignalCfg);
            
            stMuteCfg.enType = E_MHAL_HDMITX_MUTE_AUDIO | E_MHAL_HDMITX_MUTE_VIDEO | E_MHAL_HDMITX_MUTE_AVMUTE;
            stMuteCfg.bMute = 0;
            MhalHdmitxSetMute(pHdmitxCtx, &stMuteCfg);
        }
    }
#endif
#if defined(CONFIG_SSTAR_RGN)
    if (1 == gu32GOPorMOP)
    {
        MHAL_RGN_GopType_e eGopId = E_MHAL_GOP_VPE_PORT1;
        MHAL_RGN_GopGwinId_e eGwinId = E_MHAL_GOP_GWIN_ID_0;
        MHAL_RGN_GopWindowConfig_t stSrcWinCfg;
        MHAL_RGN_GopWindowConfig_t stDstWinCfg;
        u8 bInitRgn = 0;
        //BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "CONFIG_SSTAR_RGN enable, func:%s, line:%d\n", __FUNCTION__,__LINE__);

        switch (eAspectRatio)
        {
            case EN_PICTURE_DISPLAY_ZOOM:
            {
                stSrcWinCfg.u32X = 0;
                stSrcWinCfg.u32Y = 0;
                stSrcWinCfg.u32Width = min(u16DispOutWidht, u16ImgWidth);
                stSrcWinCfg.u32Height = min(u16DispOutHeight, u16ImgHeight);
                stDstWinCfg.u32X = 0;
                stDstWinCfg.u32Y = 0;
                stDstWinCfg.u32Width  = u16DispOutWidht;
                stDstWinCfg.u32Height = u16DispOutHeight;
            }
            break;
            case EN_PICTURE_DISPLAY_CENTER:
            {
                stSrcWinCfg.u32X = 0;
                stSrcWinCfg.u32Y = 0;
                stSrcWinCfg.u32Width = min(u16DispOutWidht, u16ImgWidth);
                stSrcWinCfg.u32Height = min(u16DispOutHeight, u16ImgHeight);
                stDstWinCfg.u32X = (u16DispOutWidht - stSrcWinCfg.u32Width) / 2;
                stDstWinCfg.u32Y = (u16DispOutHeight - stSrcWinCfg.u32Height) / 2;
                stDstWinCfg.u32Width = stSrcWinCfg.u32Width;
                stDstWinCfg.u32Height = stSrcWinCfg.u32Height;
            }
            break;
            case EN_PICTURE_DISPLAY_USER:
            {
                stSrcWinCfg.u32X = 0;
                stSrcWinCfg.u32Y = 0;
                stSrcWinCfg.u32Width = min(u16DispOutWidht, u16ImgWidth);
                stSrcWinCfg.u32Height = min(u16DispOutHeight, u16ImgHeight);
                stDstWinCfg.u32X = u32DstX;
                stDstWinCfg.u32Y = u32DstY;
                stDstWinCfg.u32Width = min(u16DispOutWidht, u16ImgWidth);
                stDstWinCfg.u32Height = min(u16DispOutHeight, u16ImgHeight);
            }
            break;
            default:
                return;
        }
        if (!bInitRgn)
        {
            MHAL_RGN_GopInit();
            bInitRgn = 1;
        }
        MHAL_RGN_GopSetBaseWindow(eGopId, &stSrcWinCfg, &stDstWinCfg);

        MHAL_RGN_GopGwinSetPixelFormat(eGopId, eGwinId, E_MHAL_RGN_PIXEL_FORMAT_ARGB8888);

        MHAL_RGN_GopGwinSetWindow(eGopId, eGwinId, min(u16DispOutWidht, u16ImgWidth), min(u16DispOutHeight, u16ImgHeight), min(u16DispOutWidht, u16ImgWidth) * 4, 0, 0);

        MHAL_RGN_GopGwinSetBuffer(eGopId, eGwinId, (MS_PHYADDR)pstDispInfo->u32DispBufStart);

        MHAL_RGN_GopSetAlphaZeroOpaque(eGopId, FALSE, FALSE, E_MHAL_RGN_PIXEL_FORMAT_ARGB8888);

        MHAL_RGN_GopSetAlphaType(eGopId, eGwinId, E_MHAL_GOP_GWIN_ALPHA_PIXEL, 0xFF);

        MHAL_RGN_GopGwinEnable(eGopId,eGwinId);
    }
#endif

#endif
}
#ifndef ALIGN_UP
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
#endif
SS_SHEADER_DispPnl_u *_BootDbTable(SS_SHEADER_DispInfo_t *pHeadInfo, SS_SHEADER_DispPnl_u *pDispPnl, u8 *pbNeedRestorePartition)
{
    char *pDispTable = NULL;
    SS_SHEADER_DisplayDevice_e enDevice = EN_DISPLAY_DEVICE_NULL;
    SS_SHEADER_DispPnl_u *puDispPnlLoop = NULL;
    u32 u32Idx = 0;
    char tmp[64];

    pDispTable = getenv("dispout");
    enDevice = *(SS_SHEADER_DisplayDevice_e *)pDispPnl;
    *pbNeedRestorePartition = 0;
    switch (enDevice)
    {
#if defined(CONFIG_SSTAR_PNL)
        case EN_DISPLAY_DEVICE_LCD:
        {
            if (!pDispTable)
            {
                memset(tmp,0,sizeof(tmp));
                snprintf(tmp, sizeof(tmp) - 1,"dcache off");
                run_command(tmp, 0);
                setenv("dispout", (const char *)pDispPnl->stPnlPara.au8PanelName);
                BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "dispout is empty, set %s as default.\n", pDispPnl->stPnlPara.au8PanelName);
                saveenv();
                memset(tmp,0,sizeof(tmp));
                snprintf(tmp, sizeof(tmp) - 1,"dcache on");
                run_command(tmp, 0);

                return pDispPnl;
            }
            if (!strcmp((const char *)pDispPnl->stPnlPara.au8PanelName, pDispTable))
            {
                BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "DB Table and setting match.\n");

                return pDispPnl;
            }
            else
            {
                for (u32Idx = 0, puDispPnlLoop = (SS_SHEADER_DispPnl_u *)((u8 *)pHeadInfo + pHeadInfo->stDataInfo.u32SubHeadSize); 
                      u32Idx < pHeadInfo->stDataInfo.u32SubNodeCount; u32Idx++)
                {
                    if (!strcmp((const char *)puDispPnlLoop->stPnlPara.au8PanelName, pDispTable))
                    {
                        pHeadInfo->u32FirstUseOffset = (u32)puDispPnlLoop - (u32)pHeadInfo + sizeof(SS_HEADER_Desc_t);
                        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "Find dispout: %s, first offset 0x%x\n", pDispTable, pHeadInfo->u32FirstUseOffset);
                        *pbNeedRestorePartition = 1;

                        return puDispPnlLoop;
                    }
                    puDispPnlLoop = (SS_SHEADER_DispPnl_u *)((u8 *)puDispPnlLoop + ALIGN_UP(sizeof(SS_SHEADER_PnlPara_t) + puDispPnlLoop->stPnlPara.stMipiDsiCfg.u32CmdBufSize, 4));
                }
                BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "Not found dispout: %s\n", pDispTable);

                return NULL;
            }
        }
        break;
#endif
        case EN_DISPLAY_DEVICE_HDMI:
        case EN_DISPLAY_DEVICE_VGA:
        {
            if (!pDispTable)
            {
                memset(tmp,0,sizeof(tmp));
                snprintf(tmp, sizeof(tmp) - 1,"dcache off");
                run_command(tmp, 0);
                setenv("dispout", (const char *)pDispPnl->stDispOut.au8ResName);
                BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "dispout is empty, set %s as default.\n", pDispPnl->stDispOut.au8ResName);
                saveenv();
                memset(tmp,0,sizeof(tmp));
                snprintf(tmp, sizeof(tmp) - 1,"dcache on");
                run_command(tmp, 0);

                return pDispPnl;
            }
            if (!strcmp((const char *)pDispPnl->stDispOut.au8ResName, pDispTable))
            {
                BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "DB Table and setting match.\n");

                return pDispPnl;
            }
            else
            {
                for (u32Idx = 0, puDispPnlLoop = (SS_SHEADER_DispPnl_u *)((u8 *)pHeadInfo + pHeadInfo->stDataInfo.u32SubHeadSize); 
                      u32Idx < pHeadInfo->stDataInfo.u32SubNodeCount; u32Idx++)
                {
                    if (!strcmp((const char *)puDispPnlLoop->stDispOut.au8ResName, pDispTable))
                    {
                        pHeadInfo->u32FirstUseOffset = (u32)puDispPnlLoop - (u32)pHeadInfo + sizeof(SS_HEADER_Desc_t);
                        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "Find dispout: %s, first offset 0x%x\n", pDispTable, pHeadInfo->u32FirstUseOffset);
                        *pbNeedRestorePartition = 1;
                        
                        return puDispPnlLoop;
                    }
                    puDispPnlLoop = (SS_SHEADER_DispPnl_u *)((u8 *)puDispPnlLoop + sizeof(SS_SHEADER_DispConfig_t));
                }
                BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "Not found dispout: %s\n", pDispTable);

                return NULL;
            }
        }
        break;
        default:
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "Device fail\n");                   

            return NULL;
        }
    }
}

int do_display (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    #define LOGO_FLAHS_BASE     0x14000000
    
    u32     start, size;
    char strENVName[] = "LOGO";
    u32 idx;
    char strHeaderName[] = "SSTAR";
    void *pRawData = NULL;
    SS_HEADER_Desc_t *pHeader = NULL;
    SS_SHEADER_DataInfo_t *pDataHead = NULL;
    SS_SHEADER_DispInfo_t *pDispInfo = NULL;
    SS_SHEADER_DisplayDevice_e *penDevice;
    SS_SHEADER_DispPnl_u *puDispPnl = NULL;
    SS_SHEADER_PictureDataInfo_t *pstPictureInfo = NULL;
    u16 u16ImgWidth = 0;
    u16 u16ImgHeight = 0;
    u32 u32LogoCount = 0;
    u32 u32LogoId = 0;
    u32 u32X = 0, u32Y = 0;
    static u32 u32Shift = 0;
    u8 bResorePartition = 0;
    SS_SHEADER_PictureAspectRatio_e enAspectRatio = EN_PICTURE_DISPLAY_ZOOM;

    if (argc != 6)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "usage: bootlogo [logo_id] [aspect ratio '0: zoom' '1: center' '2: usr'] [x] [y] [rotation '0: none' '1: 90' '2: 180' '3: 270']\n");

        return 0;
    }

#if defined(CONFIG_CMD_MTDPARTS) || defined(CONFIG_MS_SPINAND)
    struct mtd_device *dev;
    struct part_info *part;
    u8 pnum;
    int ret;

    ret = mtdparts_init();
    if (ret)
        return ret;

    ret = find_dev_and_part(strENVName, &dev, &pnum, &part);
    if (ret)
    {
        return ret;
    }

    if (dev->id->type != MTD_DEV_TYPE_NAND)
    {
        puts("not a NAND device\n");
        return -1;
    }

    start = part->offset;
    size = part->size;
#elif defined(CONFIG_MS_PARTITION)
    mxp_record rec;
    mxp_load_table();
    idx=mxp_get_record_index(strENVName);
    if(idx<0)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "can not found mxp record: %s\n", strENVName);
        return FALSE;
    }

    if(0 != mxp_get_record_by_index(idx,&rec))
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "failed to get MXP record with name: %s\n", strENVName);
        return 0;
    }
    start = rec.start;
    size = rec.size;
#else
    start = 0;
    size = 0;
    return 0;
#endif
    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "%s in flash offset=0x%x size=0x%x\n",strENVName , start, size);

    pRawData = malloc(size);
    if(pRawData == NULL)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "allocate buffer fail\n");
        return 0;
    }
#if defined(CONFIG_CMD_MTDPARTS) || defined(CONFIG_MS_SPINAND)
    char  cmd_str[128];
    sprintf(cmd_str, "nand read.e 0x%p %s", pRawData, strENVName);
    run_command(cmd_str, 0);
#else
    //sprintf(cmd_str, "sf probe; sf read 0x%p 0x%p 0x%p", pRawData, start, size);
    //run_command(cmd_str, 0);
    memcpy(pRawData, (void*)(U32)(start | LOGO_FLAHS_BASE), size);
#endif
    flush_cache((U32)pRawData, size);

    pHeader = pRawData;
    //Parsing Header
    for(idx = 0; idx < 5; idx++)
    {
        if( strHeaderName[idx] != *((U8 *)(pHeader->au8Tittle + idx)))
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "Header check fail\n");

            free(pRawData);
            return 0;
        }
    }
    pDataHead = (SS_SHEADER_DataInfo_t *)(pRawData + sizeof(SS_HEADER_Desc_t));
    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "Header count %d\n", pHeader->u32DataInfoCnt);
    for (idx = 0; idx < pHeader->u32DataInfoCnt; idx++)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "Name %s Sub head sz %d total sz %d node cnt %d\n", pDataHead->au8DataInfoName, pDataHead->u32SubHeadSize,
                                              pDataHead->u32DataTotalSize, pDataHead->u32SubNodeCount);
        if (!strcmp((char *)pDataHead->au8DataInfoName, "DISP"))
        {
            pDispInfo = (SS_SHEADER_DispInfo_t *)pDataHead;           
            penDevice = (SS_SHEADER_DisplayDevice_e *)(pDispInfo->u32FirstUseOffset + pRawData);           
            puDispPnl = (SS_SHEADER_DispPnl_u *)penDevice;
            puDispPnl = _BootDbTable(pDispInfo, puDispPnl, &bResorePartition);
            if (!puDispPnl)
            {
                free(pRawData);
                return 0;
            }
            if (bResorePartition)
            {
                flush_dcache_all();
#if defined(CONFIG_CMD_MTDPARTS) || defined(CONFIG_MS_SPINAND)
                char  cmd_str[128];
                sprintf(cmd_str, "dcache off");
                run_command(cmd_str, 0);
                sprintf(cmd_str, "nand erase.part %s", strENVName);
                run_command(cmd_str, 0);
                sprintf(cmd_str, "nand write.e 0x%p %s 0x%x", pRawData, strENVName, size);
                run_command(cmd_str, 0);
                sprintf(cmd_str, "dcache on");
                run_command(cmd_str, 0);
#else
                char  cmd_str[128];
                sprintf(cmd_str, "dcache off");
                run_command(cmd_str, 0);
                sprintf(cmd_str, "sf probe 0; sf erase 0x%x 0x%x", start, size);
                run_command(cmd_str, 0);
                sprintf(cmd_str, "sf write 0x%p 0x%x 0x%x", pRawData, start, size);
                run_command(cmd_str, 0);
                sprintf(cmd_str, "dcache on");
                run_command(cmd_str, 0);
#endif
            }
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "First offset %d out buf size 0x%x out buf addr 0x%x en %d\n", pDispInfo->u32FirstUseOffset, pDispInfo->u32DispBufSize, pDispInfo->u32DispBufStart, *penDevice);
            switch (*penDevice)
            {
#if defined(CONFIG_SSTAR_PNL)
                case EN_DISPLAY_DEVICE_LCD:
                {
                    puDispPnl->stPnlPara.stMipiDsiCfg.pu8CmdBuf = (u8 *)puDispPnl + sizeof(SS_SHEADER_PnlPara_t);
                }
                break;
#endif
                case EN_DISPLAY_DEVICE_HDMI:
                case EN_DISPLAY_DEVICE_VGA:
                {
                }
                break;
                default:
                {
                    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "Device fail\n");                   
                    free(pRawData);

                    return 0;
                }
            }
            break;
        }
        pDataHead = (SS_SHEADER_DataInfo_t *)((u8 *)pDataHead + pDataHead->u32SubHeadSize + pDataHead->u32DataTotalSize);
    }
    if (idx == pHeader->u32DataInfoCnt || puDispPnl == NULL)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "Not found DISP header\n");
        free(pRawData);

        return 0;
    }
    pDataHead = (SS_SHEADER_DataInfo_t *)(pRawData + sizeof(SS_HEADER_Desc_t));
    u32LogoId = simple_strtoul(argv[1], NULL, 0);
    #if defined(CONFIG_SSTAR_RGN)
    if (0 == u32LogoId)
    {
        gu32GOPorMOP = 0;//power on logo use mop
    }
    else
    {
        gu32GOPorMOP = 1;//upgrade ui use gop
    }
    BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "CONFIG_SSTAR_RGN enable, u32LogoId:%d, gu32GOPorMOP:%d\n", u32LogoId, gu32GOPorMOP);
    #endif
    for (idx = 0; idx < pHeader->u32DataInfoCnt; idx++)
    {
        if (!strcmp((char *)pDataHead->au8DataInfoName, "LOGO"))
        {
            pstPictureInfo = (SS_SHEADER_PictureDataInfo_t *)pDataHead;
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "Total sz %d, Node cnt %d\n", pstPictureInfo->stDataInfo.u32DataTotalSize,
                                                 pstPictureInfo->stDataInfo.u32SubNodeCount);

            if (u32LogoId == u32LogoCount)
            {
                break;
            }
            u32LogoCount++;
        }
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "Total size %d\n", pDataHead->u32SubHeadSize + pDataHead->u32DataTotalSize);
        pDataHead = (SS_SHEADER_DataInfo_t *)((u8 *)pDataHead + pDataHead->u32SubHeadSize + pDataHead->u32DataTotalSize);
    }
    if (idx == pHeader->u32DataInfoCnt || puDispPnl == NULL)
    {
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_ERR, "Not found LOGO header\n");
        free(pRawData);

        return 0;
    }
#if defined(CONFIG_SSTAR_RGN)
    if (1 == gu32GOPorMOP)
    {
        //BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "CONFIG_SSTAR_RGN enable, func:%s, line:%d\n", __FUNCTION__,__LINE__);
        _BootJpdArgbCtrl(pstPictureInfo->stDataInfo.u32DataTotalSize, (u32)((s8 *)pstPictureInfo + pstPictureInfo->stDataInfo.u32SubHeadSize),
                     pDispInfo->u32DispBufSize, pDispInfo->u32DispBufStart, &u16ImgWidth, &u16ImgHeight, (PIC_ROTATION_e)simple_strtoul(argv[5], NULL, 0));
    }
    else
    {
        //BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "CONFIG_SSTAR_RGN enable, func:%s, line:%d\n", __FUNCTION__,__LINE__);
        _BootJpdYuvCtrl(pstPictureInfo->stDataInfo.u32DataTotalSize, (u32)((s8 *)pstPictureInfo + pstPictureInfo->stDataInfo.u32SubHeadSize),
             pDispInfo->u32DispBufSize, pDispInfo->u32DispBufStart, &u16ImgWidth, &u16ImgHeight, (PIC_ROTATION_e)simple_strtoul(argv[5], NULL, 0));
    }
#else
    _BootJpdYuvCtrl(pstPictureInfo->stDataInfo.u32DataTotalSize, (u32)((s8 *)pstPictureInfo + pstPictureInfo->stDataInfo.u32SubHeadSize),
             pDispInfo->u32DispBufSize, pDispInfo->u32DispBufStart, &u16ImgWidth, &u16ImgHeight, (PIC_ROTATION_e)simple_strtoul(argv[5], NULL, 0));
#endif
    flush_dcache_all();

    enAspectRatio = simple_strtoul(argv[2], NULL, 0);
    u32X = simple_strtoul(argv[3], NULL, 0);
    u32Y = simple_strtoul(argv[4], NULL, 0);
    _BootDispCtrl(puDispPnl, pDispInfo, u32Shift, enAspectRatio, u32X, u32Y, u16ImgWidth, u16ImgHeight);
    if (pRawData)
        free(pRawData);
#if defined(CONFIG_SSTAR_RGN)
    if (1 == gu32GOPorMOP)
    {
        //BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "CONFIG_SSTAR_RGN enable, func:%s, line:%d\n", __FUNCTION__,__LINE__);
        gu32FrameBuffer = pDispInfo->u32DispBufStart + 0x20000000;
        gu32DispWidth = u16ImgWidth;
        gu32DispHeight = u16ImgHeight;
        gu32Rotate = simple_strtoul(argv[5], NULL, 0);
        BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "Framebuffer addr 0x%x width %d height %d Rotate %d\n", gu32FrameBuffer, gu32DispWidth, gu32DispHeight,gu32Rotate);
    }
#endif

    return 0;
}
/**
 * draw Rectangle. the colormat of Framebuffer is ARGB8888
 */
void drawRect_rgb32 (int x0, int y0, int width, int height, int stride, 
    int color,unsigned char* frameBuffer )
{
    int *dest = (int *)((char *) (frameBuffer)
        + y0 * stride + x0 * 4);

    int x, y;
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dest[x] = color;
        }
        dest = (int *)((char *)dest + stride);
    }
}
void  drawRect_rgb12(int x0, int y0, int width, int height, int stride, int color, unsigned char* frameBuffer)
{
    const int bytesPerPixel =2;
    const int red = (color & 0xff0000) >> (16 + 4);
    const int green = (color & 0xff00) >> (8 + 4);
    const int blue = (color & 0xff) >> 4;
    const short color16 = blue | (green << 4) | (red << (4+4)) |0xf000;
    short *dest = NULL;
    int x, y;

    dest = (short *)((char *)(frameBuffer)
        + y0 * stride + x0 * bytesPerPixel);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            dest[x] = color16;
        }
        dest = (short *)((char *)dest + stride);
    }
}
void drawRect_rgb15 (int x0, int y0, int width, int height, int stride, int color, unsigned char* frameBuffer)
{
    const int bytesPerPixel = 2;
    const int red = (color & 0xff0000) >> (16 + 3);
    const int green = (color & 0xff00) >> (8 + 3);
    const int blue = (color & 0xff) >> 3;
    const short color15 = blue | (green << 5) | (red << (5 + 5)) | 0x8000;
    short *dest = NULL;
    int x = 0, y = 0;

    dest = (short *)((char *)(frameBuffer)
        + y0 * stride + x0 * bytesPerPixel);

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dest[x] = color15;
        }
        dest = (short *)((char *)dest + stride);
    }
}
#define BGCOLOR 0xFFFFFFFF
#define BARCOLOR 0xFF00FF00
#define BARCOLOR_DARK 0xFF00AEEF

void do_bootfb_bar(u8 progress,char* message, u32 width, u32 height, u32 Rotate, u32 u32Framebuffer)
{

    int total_bar_width = 0;
    int bar_x0 = 0;
    int bar_y0 = 0;
    int bar_width = 0;
    int bar_height = 0;
    int stride = width * 4;
    blit_props props;
    props.Buffer = (blit_pixel *)(u32Framebuffer);
    props.BufHeight = height;
    props.BufWidth= width;
    props.Value = 0xFFFF0000;
    props.Wrap = 0;
    props.Scale = 4;
    char str[]="Update OK,System will reboot!";
    //printf("progress = %d rot %d w %d h %d s %d\n", progress, Rotate, width, height, stride);

    total_bar_width = width * 618 / 1000;
    int offset = (width -blit32_ADVANCE*props.Scale*strlen(str))/2;
#if 0
    int total_bar_height = 0;
    total_bar_height = height * 618 / 1000;
    if (progress > 0 && progress < 100)
    {
        bar_x0 = width * 250 / 1000;
        bar_y0 = (height - total_bar_height) / 2;
        bar_width = 8;
        bar_height = total_bar_height * progress / 100;
        drawRect_rgb32(bar_x0, bar_y0, bar_width, bar_height, stride, BARCOLOR, (unsigned char *)u32Framebuffer);
        printf("b.w %d h %d x %d y %d\n", bar_width, bar_height, bar_x0, bar_y0);
    }
    if (progress < 100)
    {
        bar_x0 = width * 250 / 1000;
        bar_y0 = (height - total_bar_height) / 2 + bar_height;
        bar_width = 8;
        bar_height = total_bar_height - bar_height;
        drawRect_rgb32(bar_x0, bar_y0, bar_width, bar_height, stride, BARCOLOR_DARK, (unsigned char *)u32Framebuffer);
        printf("b.w %d h %d x %d y %d\n", bar_width, bar_height, bar_x0, bar_y0);
    }
#endif
    if(progress >= 100)
    {
        progress = 100;
    }
    if(EN_PIC_ROTATE_180 == Rotate)
    {
        if (progress > 0 && progress < 100)
        {
            bar_width = total_bar_width * progress / 100;
            bar_height = 8;
            bar_x0 = width - (width - total_bar_width)/2 - bar_width;
            bar_y0 = height- height * 750 / 1000;
            drawRect_rgb32(bar_x0, bar_y0, bar_width, bar_height, stride, BARCOLOR, (unsigned char *)u32Framebuffer);
            //printf("180:w %d h %d x %d y %d\n", bar_width, bar_height, bar_x0, bar_y0);
        }
        if (progress < 100)
        {
            bar_width = total_bar_width - bar_width;
            bar_height = 8;
            bar_x0 = (width - total_bar_width) / 2;
            bar_y0 = height- height * 750 / 1000;
            drawRect_rgb32(bar_x0, bar_y0, bar_width, bar_height, stride, BARCOLOR_DARK, (unsigned char *)u32Framebuffer);
            //printf("180:w %d h %d x %d y %d\n", bar_width, bar_height, bar_x0, bar_y0);
        }
    }
    else
    {
        if (progress > 0 && progress <= 100)
        {
            bar_x0 = (width - total_bar_width) / 2;
            bar_y0 = height * 750 / 1000;
            bar_width = total_bar_width * progress / 100;
            bar_height = 8;
            drawRect_rgb32(bar_x0, bar_y0, bar_width, bar_height, stride, BARCOLOR, (unsigned char *)u32Framebuffer);
            //printf("w %d h %d x %d y %d\n", bar_width, bar_height, bar_x0, bar_y0);
        }
        if (progress < 100)
        {
            bar_x0 = (width - total_bar_width) / 2 + bar_width;
            bar_y0 = height * 750 / 1000;
            bar_width = total_bar_width - bar_width;
            bar_height = 8;
            drawRect_rgb32(bar_x0, bar_y0, bar_width, bar_height, stride, BARCOLOR_DARK, (unsigned char *)u32Framebuffer);
            //printf("w %d h %d x %d y %d\n", bar_width, bar_height, bar_x0, bar_y0);
        }
        if (progress == 100)
        {
            BOOTLOGO_DBG(BOOTLOGO_DBG_LEVEL_INFO, "%s %d,process 100 waiting:w:%d:h:%d\n",__FUNCTION__,__LINE__,offset,height * 800/1000);
            blit32_TextProps(props,offset,height * 800/1000,str);
        }
    }

    flush_dcache_all();
}
void do_bootfb_progress(u8 progress,char* message, u16 width, u16 height, u32 u32Framebuffer)
{
    static u8 bInited = 0;
    static u8 u8Completed = 0;
    static u8 u8PreMsgLen = 0;
    blit_props props;
    int completed = progress/10;
    unsigned char* framebuffer = (unsigned char*)(u32Framebuffer);
    int stride = width * 4;
    int i = 0;

    props.Buffer = (blit_pixel *)(framebuffer);
    props.BufHeight = height;
    props.BufWidth= width;
    props.Value = 0xFF000000;
    props.Wrap = 0;
    props.Scale = 4;

    
    int offset = 10+blit32_ADVANCE*props.Scale*strlen("[progress][100%]");
    int w = (width-offset)/20;
    if(!bInited)
    {
        drawRect_rgb32(0,0,width,height,stride,BGCOLOR,(unsigned char *)framebuffer);
        char str[]="[progess]";
        blit32_TextProps(props,10,height/3,str);
        
        for(i = 0;i<completed;i++)
        {
            drawRect_rgb32(offset+w*i*2,height/3,w,blit32_ADVANCE*props.Scale,stride,BARCOLOR_DARK,framebuffer);
        }
        for(i = completed;i<10;i++)
        {
            drawRect_rgb32(offset+w*i*2,height/3,w,blit32_ADVANCE*props.Scale,stride,BARCOLOR,framebuffer);
        }
        u8Completed = completed;
        bInited = 1;
    }
    else
    {
        for(i = u8Completed;i<completed;i++)
        {
            drawRect_rgb32(offset+w*i*2,height/3,w,blit32_ADVANCE*props.Scale,stride,BARCOLOR_DARK,framebuffer);
        }
    }

    char str[] = "[   %]";
    str[3] = '0'+progress%10;
    if(progress>=10)
    {
        str[2] = '0'+(progress%100)/10;
    }
    if(progress>=100)
    {
        str[1] = '0'+progress/100;
    }
    offset = 10+blit32_ADVANCE*props.Scale*strlen("[progress]");
    drawRect_rgb32(offset,height/3,blit32_ADVANCE*props.Scale*strlen(str),blit32_ADVANCE*props.Scale,stride,BGCOLOR,framebuffer);
    blit32_TextProps(props,offset,height/3,str);
    if(message!=NULL)
    {
        props.Scale = 8;
        if(u8PreMsgLen>0)
        {
            offset = (width-blit32_ADVANCE*props.Scale*u8PreMsgLen)>>1;
            drawRect_rgb32(offset,height*2/3,blit32_ADVANCE*props.Scale*u8PreMsgLen,(blit32_ADVANCE+1)*props.Scale,stride,BGCOLOR,framebuffer);
        }
        u8PreMsgLen  = strlen(message);
        offset = (width-blit32_ADVANCE*props.Scale*u8PreMsgLen)>>1;
        blit32_TextProps(props,offset,height*2/3,message);
    }
    flush_dcache_all();
}
void do_bootfb_blank(u16 width, u16 height, u32 u32Framebuffer)
{
    memset((unsigned char*)(u32Framebuffer), 0, width * height * 4);
}

int do_bootfb (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined(CONFIG_SSTAR_RGN)
    if(argc < 2)
    {
        printf("usage\n");
        printf("1.bootframebuffer progressbar <percentage> [message]\n");
        printf("2.bootframebuffer blank\n");
    }
    else
    {
        if(strcmp("progressbar",argv[1])==0)
        {
            u8 progress = simple_strtoul(argv[2], NULL, 10);

            if(argc>=4)
            {
                do_bootfb_progress(progress,argv[3], gu32DispWidth, gu32DispHeight, gu32FrameBuffer);
            }
            else
            {
                do_bootfb_progress(progress,NULL, gu32DispWidth, gu32DispHeight, gu32FrameBuffer);
            }
        }
        else if(strcmp("bar",argv[1])==0)
        {
            u8 progress = simple_strtoul(argv[2], NULL, 10);

            do_bootfb_bar(progress,NULL, gu32DispWidth, gu32DispHeight, gu32Rotate, gu32FrameBuffer);
        }      
        else if(strcmp("blank",argv[1])==0)
        {
            do_bootfb_blank(gu32DispWidth, gu32DispHeight, gu32FrameBuffer);
        }
    }
#endif
    return 0;
}

U_BOOT_CMD(
    bootlogo, CONFIG_SYS_MAXARGS, 1,    do_display,
    "show bootlogo",
    NULL
);

U_BOOT_CMD(
    bootframebuffer, CONFIG_SYS_MAXARGS, 1,    do_bootfb,
    "boot framebuffer \n" \
    "                 1.bootframebuffer progressbar <percentage> [message]\n" \
    "                 2.bootframebuffer blank\n",
    NULL
);


