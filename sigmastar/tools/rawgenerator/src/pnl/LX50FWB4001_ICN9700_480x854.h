/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

//#include "mi_panel_datatype.h"
//#include "st_common.h"
//Caution:PLS sync with
//LX50FWB4001_RM68172_480x854_v2.h (u:\workspace\cardv\carcam\sdk\verify\cardvimpl\cardv\inc\panel)
#define UNNEEDED_CONTROL 0xffff

#define REGFLAG_DELAY                0xFFFE
#define REGFLAG_END_OF_TABLE         0xFFFF   // END OF REGISTERS MARKER

//#define ExecFunc(_func_, _ret_)    _func_
unsigned short ICN9700_Cmd[] =
{
    2,                      //spi wordlen  ex: unsigned char = 1, unsigned short =2, unsigned int =4
    11,                      //spi header count exclude current field
    0,                      //latch edge 0:rising edge 1:falling edge                       (offset 1)
    27000,                  // max spi clock kHz                                            (offset 2)
    1,                      // gpio mode on                                                 (offset 3)
    19,          // mosi gpio, SDA data pin                                      (offset 4)
    109,                    // miso gpio                                                    (offset 5)
    17,          // cs gpio                                                      (offset 6)
    18,          // clk gpio                                                     (offset 7)
    21,         // rst gpio                                                     (offset 8)
    7,         // bl ctrl gpio
    16,                      // cmd=>valide bits:9=9bits, 16=16bits
    3,                      //command num, ex: {0x2001, 0 , 0},  command num is set to 3    (offset 9)
	
    0x20F0, 0,0,
    0x0001, 0,0,
    0x405A, 0,0,
    0x20F0, 0,0,
    0x0002, 0,0,
    0x405A, 0,0,
    0x20F1, 0,0,
    0x0001, 0,0,
    0x40A5, 0,0,
    0x20F1, 0,0,
    0x0002, 0,0,
    0x40A5, 0,0,
    0x20B4, 0,0,
    0x0001, 0,0,
    0x4003, 0,0,
    0x20B4, 0,0,
    0x0002, 0,0,
    0x4003, 0,0,
    0x20B4, 0,0,
    0x0003, 0,0,
    0x4003, 0,0,
    0x20B4, 0,0,
    0x0004, 0,0,
    0x4006, 0,0,
    0x20B4, 0,0,
    0x0005, 0,0,
    0x4004, 0,0,
    0x20B4, 0,0,
    0x0006, 0,0,
    0x400C, 0,0,
    0x20B4, 0,0,
    0x0007, 0,0,
    0x400E, 0,0,
    0x20B4, 0,0,
    0x0008, 0,0,
    0x4010, 0,0,
    0x20B4, 0,0,
    0x0009, 0,0,
    0x4012, 0,0,
    0x20B4, 0,0,
    0x000A, 0,0,
    0x4008, 0,0,
    0x20B4, 0,0,
    0x000B, 0,0,
    0x400A, 0,0,
    0x20B3, 0,0,
    0x0001, 0,0,
    0x4003, 0,0,
    0x20B3, 0,0,
    0x0002, 0,0,
    0x4003, 0,0,
    0x20B3, 0,0,
    0x0003, 0,0,
    0x4003, 0,0,
    0x20B3, 0,0,
    0x0004, 0,0,
    0x4007, 0,0,
    0x20B3, 0,0,
    0x0005, 0,0,
    0x4005, 0,0,
    0x20B3, 0,0,
    0x0006, 0,0,
    0x400D, 0,0,
    0x20B3, 0,0,
    0x0007, 0,0,
    0x400F, 0,0,
    0x20B3, 0,0,
    0x0008, 0,0,
    0x4011, 0,0,
    0x20B3, 0,0,
    0x0009, 0,0,
    0x4013, 0,0,
    0x20B3, 0,0,
    0x000A, 0,0,
    0x4009, 0,0,
    0x20B3, 0,0,
    0x000B, 0,0,
    0x400B, 0,0,
    0x20B0, 0,0,
    0x0001, 0,0,
    0x4054, 0,0,
    0x20B0, 0,0,
    0x0002, 0,0,
    0x4032, 0,0,
    0x20B0, 0,0,
    0x0003, 0,0,
    0x4023, 0,0,
    0x20B0, 0,0,
    0x0004, 0,0,
    0x4045, 0,0,
    0x20B0, 0,0,
    0x0005, 0,0,
    0x4044, 0,0,
    0x20B0, 0,0,
    0x0006, 0,0,
    0x4044, 0,0,
    0x20B0, 0,0,
    0x0007, 0,0,
    0x4044, 0,0,
    0x20B0, 0,0,
    0x0008, 0,0,
    0x4044, 0,0,
    0x20B0, 0,0,
    0x0009, 0,0,
    0x4060, 0,0,
    0x20B0, 0,0,
    0x000A, 0,0,
    0x4000, 0,0,
    0x20B0, 0,0,
    0x000B, 0,0,
    0x4060, 0,0,
    0x20B0, 0,0,
    0x000C, 0,0,
    0x401C, 0,0,
    0x20B1, 0,0,
    0x0001, 0,0,
    0x4032, 0,0,
    0x20B1, 0,0,
    0x0002, 0,0,
    0x4084, 0,0,
    0x20B1, 0,0,
    0x0003, 0,0,
    0x4002, 0,0,
    0x20B1, 0,0,
    0x0004, 0,0,
    0x4083, 0,0,
    0x20B1, 0,0,
    0x0005, 0,0,
    0x4012, 0,0,
    0x20B1, 0,0,
    0x0006, 0,0,
    0x4000, 0,0,
    0x20B1, 0,0,
    0x0007, 0,0,
    0x4050, 0,0,
    0x20B1, 0,0,
    0x0008, 0,0,
    0x401C, 0,0,
    0x20B2, 0,0,
    0x0001, 0,0,
    0x4073, 0,0,
    0x20B2, 0,0,
    0x0002, 0,0,
    0x4009, 0,0,
    0x20B2, 0,0,
    0x0003, 0,0,
    0x4008, 0,0,
    0x20B2, 0,0,
    0x0004, 0,0,
    0x4089, 0,0,
    0x20B2, 0,0,
    0x0005, 0,0,
    0x4048, 0,0,
    0x20B6, 0,0,
    0x0001, 0,0,
    0x4070, 0,0,
    0x20B6, 0,0,
    0x0002, 0,0,
    0x4070, 0,0,
    0x20B6, 0,0,
    0x0003, 0,0,
    0x4005, 0,0,
    0x20B7, 0,0,
    0x0001, 0,0,
    0x4041, 0,0,
    0x20C1, 0,0,
    0x0001, 0,0,
    0x4010, 0,0,
    0x20C1, 0,0,
    0x0002, 0,0,
    0x400E, 0,0,
    0x20C1, 0,0,
    0x0003, 0,0,
    0x4002, 0,0,
    0x20C1, 0,0,
    0x0004, 0,0,
    0x4014, 0,0,
    0x20C1, 0,0,
    0x0005, 0,0,
    0x4014, 0,0,
    0x20C1, 0,0,
    0x0006, 0,0,
    0x4008, 0,0,
    0x20C2, 0,0,
    0x0001, 0,0,
    0x4082, 0,0,
    0x20C3, 0,0,
    0x0001, 0,0,
    0x4022, 0,0,
    0x20C3, 0,0,
    0x0002, 0,0,
    0x4011, 0,0,
    0x20D0, 0,0,
    0x0001, 0,0,
    0x4007, 0,0,
    0x20D0, 0,0,
    0x0002, 0,0,
    0x40FF, 0,0,
    0x20D2, 0,0,
    0x0001, 0,0,
    0x4063, 0,0,
    0x20D2, 0,0,
    0x0002, 0,0,
    0x400B, 0,0,
    0x20D2, 0,0,
    0x0003, 0,0,
    0x4008, 0,0,
    0x20D2, 0,0,
    0x0004, 0,0,
    0x4088, 0,0,
    0x20C6, 0,0,
    0x0001, 0,0,
    0x4008, 0,0,
    0x20C6, 0,0,
    0x0002, 0,0,
    0x4015, 0,0,
    0x20C6, 0,0,
    0x0003, 0,0,
    0x40FF, 0,0,
    0x20C6, 0,0,
    0x0004, 0,0,
    0x4010, 0,0,
    0x20C6, 0,0,
    0x0005, 0,0,
    0x4016, 0,0,
    0x20C6, 0,0,
    0x0006, 0,0,
    0x4080, 0,0,
    0x20C6, 0,0,
    0x0007, 0,0,
    0x4060, 0,0,
    0x20C7, 0,0,
    0x0001, 0,0,
    0x4004, 0,0,
    0x20C8, 0,0,
    0x0001, 0,0,
    0x407D, 0,0,
    0x20C8, 0,0,
    0x0002, 0,0,
    0x4069, 0,0,
    0x20C8, 0,0,
    0x0003, 0,0,
    0x405A, 0,0,
    0x20C8, 0,0,
    0x0004, 0,0,
    0x404D, 0,0,
    0x20C8, 0,0,
    0x0005, 0,0,
    0x4049, 0,0,
    0x20C8, 0,0,
    0x0006, 0,0,
    0x403A, 0,0,
    0x20C8, 0,0,
    0x0007, 0,0,
    0x403E, 0,0,
    0x20C8, 0,0,
    0x0008, 0,0,
    0x402A, 0,0,
    0x20C8, 0,0,
    0x0009, 0,0,
    0x4045, 0,0,
    0x20C8, 0,0,
    0x000A, 0,0,
    0x4047, 0,0,
    0x20C8, 0,0,
    0x000B, 0,0,
    0x4048, 0,0,
    0x20C8, 0,0,
    0x000C, 0,0,
    0x4069, 0,0,
    0x20C8, 0,0,
    0x000D, 0,0,
    0x4057, 0,0,
    0x20C8, 0,0,
    0x000E, 0,0,
    0x405F, 0,0,
    0x20C8, 0,0,
    0x000F, 0,0,
    0x4052, 0,0,
    0x20C8, 0,0,
    0x0010, 0,0,
    0x404D, 0,0,
    0x20C8, 0,0,
    0x0011, 0,0,
    0x4040, 0,0,
    0x20C8, 0,0,
    0x0012, 0,0,
    0x402C, 0,0,
    0x20C8, 0,0,
    0x0013, 0,0,
    0x4007, 0,0,
    0x20C8, 0,0,
    0x0014, 0,0,
    0x407D, 0,0,
    0x20C8, 0,0,
    0x0015, 0,0,
    0x4069, 0,0,
    0x20C8, 0,0,
    0x0016, 0,0,
    0x405A, 0,0,
    0x20C8, 0,0,
    0x0017, 0,0,
    0x404D, 0,0,
    0x20C8, 0,0,
    0x0018, 0,0,
    0x4049, 0,0,
    0x20C8, 0,0,
    0x0019, 0,0,
    0x403A, 0,0,
    0x20C8, 0,0,
    0x001A, 0,0,
    0x403E, 0,0,
    0x20C8, 0,0,
    0x001B, 0,0,
    0x402A, 0,0,
    0x20C8, 0,0,
    0x001C, 0,0,
    0x4045, 0,0,
    0x20C8, 0,0,
    0x001D, 0,0,
    0x4047, 0,0,
    0x20C8, 0,0,
    0x001E, 0,0,
    0x4048, 0,0,
    0x20C8, 0,0,
    0x001F, 0,0,
    0x4069, 0,0,
    0x20C8, 0,0,
    0x0020, 0,0,
    0x4057, 0,0,
    0x20C8, 0,0,
    0x0021, 0,0,
    0x405F, 0,0,
    0x20C8, 0,0,
    0x0022, 0,0,
    0x4052, 0,0,
    0x20C8, 0,0,
    0x0023, 0,0,
    0x404D, 0,0,
    0x20C8, 0,0,
    0x0024, 0,0,
    0x4040, 0,0,
    0x20C8, 0,0,
    0x0025, 0,0,
    0x402C, 0,0,
    0x20C8, 0,0,
    0x0026, 0,0,
    0x4007, 0,0,
    0x2011, 0,0,
    0x0001, 0,0,
    0x4000, 0,0,
    REGFLAG_DELAY, 120, 120,
    0x2029, 0,0,
    0x0001, 0,0,
    0x4000, 0,0,
    REGFLAG_DELAY, 50, 50,
    REGFLAG_END_OF_TABLE, 0x00, 0x00,
	
};

MhalPnlParamConfig_t stPanel_LX50_ICN9700 =
{
    "LX50_ICN9700", // const char *m_pPanelName;                ///<  PanelName
#if !defined (__aarch64__)
    0,
#endif
    0, 	//MS_U8 m_bPanelDither :1;                 ///<  PANEL_DITHER, keep the setting
    E_MHAL_PNL_LINK_TTL_SPI_IF, //MHAL_DISP_ApiPnlLinkType_e m_ePanelLinkType   :4;  ///<  PANEL_LINK

    ///////////////////////////////////////////////
    // Board related setting
    ///////////////////////////////////////////////
    0,  //MS_U8 m_bPanelDualPort  :1;              ///<  VOP_21[8], MOD_4A[1],    PANEL_DUAL_PORT, refer to m_bPanelDoubleClk
    0,  //MS_U8 m_bPanelSwapPort  :1;              ///<  MOD_4A[0],               PANEL_SWAP_PORT, refer to "LVDS output app note" A/B channel swap
    0,  //MS_U8 m_bPanelSwapOdd_ML    :1;          ///<  PANEL_SWAP_ODD_ML
    0,  //MS_U8 m_bPanelSwapEven_ML   :1;          ///<  PANEL_SWAP_EVEN_ML
    0,  //MS_U8 m_bPanelSwapOdd_RB    :1;          ///<  PANEL_SWAP_ODD_RB
    0,  //MS_U8 m_bPanelSwapEven_RB   :1;          ///<  PANEL_SWAP_EVEN_RB

    0,  //MS_U8 m_bPanelSwapLVDS_POL  :1;          ///<  MOD_40[5], PANEL_SWAP_LVDS_POL, for differential P/N swap
    0,  //MS_U8 m_bPanelSwapLVDS_CH   :1;          ///<  MOD_40[6], PANEL_SWAP_LVDS_CH, for pair swap
    0,  //MS_U8 m_bPanelPDP10BIT      :1;          ///<  MOD_40[3], PANEL_PDP_10BIT ,for pair swap
    0,  //MS_U8 m_bPanelLVDS_TI_MODE  :1;          ///<  MOD_40[2], PANEL_LVDS_TI_MODE, refer to "LVDS output app note"

    ///////////////////////////////////////////////
    // For TTL Only
    ///////////////////////////////////////////////
    0,  //MS_U8 m_ucPanelDCLKDelay;                ///<  PANEL_DCLK_DELAY
    1,  //MS_U8 m_bPanelInvDCLK   :1;              ///<  MOD_4A[4],                   PANEL_INV_DCLK
    0,  //MS_U8 m_bPanelInvDE     :1;              ///<  MOD_4A[2],                   PANEL_INV_DE
    1,  //MS_U8 m_bPanelInvHSync  :1;              ///<  MOD_4A[12],                  PANEL_INV_HSYNC
    1,  //MS_U8 m_bPanelInvVSync  :1;              ///<  MOD_4A[3],                   PANEL_INV_VSYNC

    ///////////////////////////////////////////////
    // Output driving current setting
    ///////////////////////////////////////////////
    // driving current setting (0x00=4mA, 0x01=6mA, 0x02=8mA, 0x03=12mA)
    0,  //MS_U8 m_ucPanelDCKLCurrent;              ///<  define PANEL_DCLK_CURRENT
    0,  //MS_U8 m_ucPanelDECurrent;                ///<  define PANEL_DE_CURRENT
    0,  //MS_U8 m_ucPanelODDDataCurrent;           ///<  define PANEL_ODD_DATA_CURRENT
    0,  //MS_U8 m_ucPanelEvenDataCurrent;          ///<  define PANEL_EVEN_DATA_CURRENT

    ///////////////////////////////////////////////
    // panel on/off timing
    ///////////////////////////////////////////////
    0,  //MS_U16 m_wPanelOnTiming1;                ///<  time between panel & data while turn on power
    0,  //MS_U16 m_wPanelOnTiming2;                ///<  time between data & back light while turn on power
    0,  //MS_U16 m_wPanelOffTiming1;               ///<  time between back light & data while turn off power
    0,  //MS_U16 m_wPanelOffTiming2;               ///<  time between data & panel while turn off power

    ///////////////////////////////////////////////
    // panel timing spec.
    ///////////////////////////////////////////////
    // sync related
    2,  //MS_U8 m_ucPanelHSyncWidth;               ///<  VOP_01[7:0], PANEL_HSYNC_WIDTH
    0,  //MS_U8 m_ucPanelHSyncBackPorch;           ///<  PANEL_HSYNC_BACK_PORCH, no register setting, provide value for query only

                                             ///<  not support Manuel VSync Start/End now
                                             ///<  VOP_02[10:0] VSync start = Vtt - VBackPorch - VSyncWidth
                                             ///<  VOP_03[10:0] VSync end = Vtt - VBackPorch
    3,  //MS_U8 m_ucPanelVSyncWidth;               ///<  define PANEL_VSYNC_WIDTH
    16,  //MS_U8 m_ucPanelVBackPorch;               ///<  define PANEL_VSYNC_BACK_PORCH

    // DE related
    0,  //MS_U16 m_wPanelHStart;                   ///<  VOP_04[11:0], PANEL_HSTART, DE H Start (PANEL_HSYNC_WIDTH + PANEL_HSYNC_BACK_PORCH)
    22,  //MS_U16 m_wPanelVStart;                   ///<  VOP_06[11:0], PANEL_VSTART, DE V Start
    480,  //MS_U16 m_wPanelWidth;                    ///< PANEL_WIDTH, DE width (VOP_05[11:0] = HEnd = HStart + Width - 1)
    854,  //MS_U16 m_wPanelHeight;                   ///< PANEL_HEIGHT, DE height (VOP_07[11:0], = Vend = VStart + Height - 1)

    // DClk related
    0,  //MS_U16 m_wPanelMaxHTotal;                ///<  PANEL_MAX_HTOTAL. Reserved for future using.
    484,  //MS_U16 m_wPanelHTotal;                   ///<  VOP_0C[11:0], PANEL_HTOTAL
    0,  //MS_U16 m_wPanelMinHTotal;                ///<  PANEL_MIN_HTOTAL. Reserved for future using.

    0,  //MS_U16 m_wPanelMaxVTotal;                ///<  PANEL_MAX_VTOTAL. Reserved for future using.
    950,  //MS_U16 m_wPanelVTotal;                   ///<  VOP_0D[11:0], PANEL_VTOTAL
    0,  //MS_U16 m_wPanelMinVTotal;                ///<  PANEL_MIN_VTOTAL. Reserved for future using.

    0,  //MS_U8 m_dwPanelMaxDCLK;                  ///<  PANEL_MAX_DCLK. Reserved for future using.
    28,  //MS_U8 m_dwPanelDCLK;                     ///<  LPLL_0F[23:0], PANEL_DCLK          ,{0x3100_10[7:0], 0x3100_0F[15:0]}
    0,  //MS_U8 m_dwPanelMinDCLK;                  ///<  PANEL_MIN_DCLK. Reserved for future using.
                                             ///<  spread spectrum
    0,  //MS_U16 m_wSpreadSpectrumStep;            ///<  move to board define, no use now.
    0,  //MS_U16 m_wSpreadSpectrumSpan;            ///<  move to board define, no use now.

    0,  //MS_U8 m_ucDimmingCtl;                    ///<  Initial Dimming Value
    0,  //MS_U8 m_ucMaxPWMVal;                     ///<  Max Dimming Value
    0,  //MS_U8 m_ucMinPWMVal;                     ///<  Min Dimming Value

    3,  //MS_U8 m_bPanelDeinterMode   :1;          ///<  define PANEL_DEINTER_MODE,  no use now
    E_MHAL_PNL_ASPECT_RATIO_OTHER,  //MHAL_DISP_PnlAspectRatio_e m_ucPanelAspectRatio; ///<  Panel Aspect Ratio, provide information to upper layer application for aspect ratio setting.
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
    0,  //MS_U16 m_u16LVDSTxSwapValue;
    E_MHAL_PNL_TI_10BIT_MODE,  //MHAL_DISP_ApiPnlTiBitMode_e m_ucTiBitMode;                         ///< MOD_4B[1:0], refer to "LVDS output app note"
    E_MHAL_PNL_OUTPUT_565BIT_MODE, //E_MI_PNL_OUTPUT_6BIT_MODE, //E_MI_PNL_OUTPUT_565BIT_MODE,  //MHAL_DISP_ApiPnlOutPutFormatBitMode_e m_ucOutputFormatBitMode;

    0,  //MS_U8 m_bPanelSwapOdd_RG    :1;          ///<  define PANEL_SWAP_ODD_RG
    0,  //MS_U8 m_bPanelSwapEven_RG   :1;          ///<  define PANEL_SWAP_EVEN_RG
    0,  //MS_U8 m_bPanelSwapOdd_GB    :1;          ///<  define PANEL_SWAP_ODD_GB
    0,  //MS_U8 m_bPanelSwapEven_GB   :1;          ///<  define PANEL_SWAP_EVEN_GB

    /**
    *  Others
    */
    0,  //MS_U8 m_bPanelDoubleClk     :1;             ///<  LPLL_03[7], define Double Clock ,LVDS dual mode
    0,  //MS_U32 m_dwPanelMaxSET;                     ///<  define PANEL_MAX_SET
    0,  //MS_U32 m_dwPanelMinSET;                     ///<  define PANEL_MIN_SET
    E_MHAL_PNL_CHG_DCLK,  //MHAL_DISP_ApiPnlOutTimingMode_e m_ucOutTimingMode;   ///<Define which panel output timing change mode is used to change VFreq for same panel
    0,  //MS_U8 m_bPanelNoiseDith     :1;             ///<  PAFRC mixed with noise dither disable
    E_MHAL_PNL_CH_SWAP_0,
    E_MHAL_PNL_CH_SWAP_1,
    E_MHAL_PNL_CH_SWAP_2,
    E_MHAL_PNL_CH_SWAP_3,
    E_MHAL_PNL_CH_SWAP_4,
};

//Caustion: For an spi panel, all the parameters for mipi are invalid.
//But the pu8CmdBuf will fill in SPI cmd.
MhalPnlMipiDsiConfig_t stMipiDsiConfig_ICN9700 =
{
    //HsTrail HsPrpr HsZero ClkHsPrpr ClkHsExit ClkTrail ClkZero ClkHsPost DaHsExit ContDet
    5,      3,     5,     10,       14,       3,       12,     10,       5,       0,
    //Lpx   TaGet  TaSure  TaGo
    16,   26,    24,     50,

    //Hac,  Hpw,  Hbp,  Hfp,  Vac,  Vpw, Vbp, Vfp,  Bllp, Fps
    1024,  10,    160,   160,   600, 1,   23,  12,  0,    60,

    E_MHAL_PNL_MIPI_DSI_LANE_4,      // MhalPnlMipiDsiLaneMode_e enLaneNum;
    E_MHAL_PNL_MIPI_DSI_RGB888,      // MhalPnlMipiDsiFormat_e enFormat;
    E_MHAL_PNL_MIPI_DSI_SYNC_PULSE,  // MhalPnlMipiDsiCtrlMode_e enCtrl;

    (u8 *)ICN9700_Cmd,
    sizeof(ICN9700_Cmd),
};
