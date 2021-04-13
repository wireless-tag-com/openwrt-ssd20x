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

#define PANEL_W                  (480)
#define PANEL_H                 (640)

#define SPI_CMD                      (0x0)
#define SPI_DATA                     (0x100)

#define BOARD_CONFIG_EVB_I5      0
#define BOARD_CONFIG_EVB_I6E      1
#define BOARD_CONFIG_CUST001      2     //i6e BGA2 512MB+512MB

#define BOARD_CONFIG_NAME       BOARD_CONFIG_CUST001
#if (BOARD_CONFIG_NAME == BOARD_CONFIG_CUST001)
#define LSCE_GPIO_PIN   (17)    //#define PAD_PM_SPI_CZ                   17
#define LSCK_GPIO_PIN   (18)    //#define PAD_PM_SPI_CK                   18
#define LSDA_GPIO_PIN   (19)    //#define PAD_PM_SPI_DI                   19
#define LSRST_GPIO_PIN  (21)    //#define PAD_PM_SPI_WPZ                  21
#define BL_EN_GPIO_PIN  (7)    //#define PAD_PM_GPIO1                    7
#elif (BOARD_CONFIG_NAME == BOARD_CONFIG_EVB_I5)
//infinity5
#define LSCE_GPIO_PIN   (107)
#define LSCK_GPIO_PIN   (108)
#define LSDA_GPIO_PIN   (109)
#define LSRST_GPIO_PIN   (15)
#define BL_EN_GPIO_PIN  (7)
#else   //(BOARD_CONFIG_NAME == BOARD_CONFIG_EVB_I6e)
//infinity6e
#define LSCE_GPIO_PIN   (59)
#define LSCK_GPIO_PIN   (60)
#define LSDA_GPIO_PIN   (61)
#define LSRST_GPIO_PIN   (6)
#define BL_EN_GPIO_PIN  (7)
#endif


#define REGFLAG_DELAY                0xFFFE
#define REGFLAG_END_OF_TABLE         0xFFFF   // END OF REGISTERS MARKER


unsigned short TP30P117Cmd[] =
{
    2,                      //spi wordlen  ex: unsigned char = 1, unsigned short =2, unsigned int =4
    11,                      //spi header count exclude current field
    0,                      //latch edge 0:rising edge 1:falling edge                       (offset 1)
    27000,                  // max spi clock kHz                                            (offset 2)
    1,                      // gpio mode on                                                 (offset 3)
    LSDA_GPIO_PIN,          // mosi gpio, SDA data pin                                      (offset 4)
    109,                    // miso gpio                                                    (offset 5)
    LSCE_GPIO_PIN,          // cs gpio                                                      (offset 6)
    LSCK_GPIO_PIN,          // clk gpio                                                     (offset 7)
    LSRST_GPIO_PIN,         // rst gpio                                                     (offset 8)
    BL_EN_GPIO_PIN,         // bl ctrl gpio
    9,                      // cmd=>valide bits:9=9bits, 16=16bits
    3,                      //command num, ex: {0x2001, 0 , 0},  command num is set to 3    (offset 9)

    SPI_CMD  | 0xFF, 0,0,
    SPI_DATA | 0x77, 0,0,
    SPI_DATA | 0x01, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x13, 0,0,
                         
    SPI_CMD  | 0xE8, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x0E, 0,0,
                         
    SPI_CMD  | 0xFF, 0,0,
    SPI_DATA | 0x77, 0,0,
    SPI_DATA | 0x01, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
                         
    SPI_CMD  | 0xFF, 0,0,
    SPI_DATA | 0x77, 0,0,
    SPI_DATA | 0x01, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x10, 0,0,
                         
    SPI_CMD  | 0xC6, 0,0,
    SPI_DATA | 0x07, 0,0,
                         
    SPI_CMD  | 0x11, 0,0,
    REGFLAG_DELAY, 120,0,
                         
    SPI_CMD  | 0xFF, 0,0,
    SPI_DATA | 0x77, 0,0,
    SPI_DATA | 0x01, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x13, 0,0,
                         
    SPI_CMD  | 0xE8, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x0C, 0,0,
                         
    REGFLAG_DELAY, 10, 0, 
                         
    SPI_CMD  | 0xE8, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
                         
    SPI_CMD  | 0xFF, 0,0,
    SPI_DATA | 0x77, 0,0,
    SPI_DATA | 0x01, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
                         
    SPI_CMD  | 0xFF, 0,0,
    SPI_DATA | 0x77, 0,0,
    SPI_DATA | 0x01, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x10, 0,0,
                         
    SPI_CMD  | 0xC0, 0,0,
    SPI_DATA | 0x4F, 0,0,
    SPI_DATA | 0x00, 0,0,
                         
    SPI_CMD  | 0xC1, 0,0,
    SPI_DATA | 0x07, 0,0,
    SPI_DATA | 0x02, 0,0,
                         
    SPI_CMD  | 0xC2, 0,0,
    SPI_DATA | 0x20, 0,0,
    SPI_DATA | 0x05, 0,0,
                         
    SPI_CMD  | 0xC6, 0,0,
    SPI_DATA | 0x01, 0,0,
                         
    SPI_CMD  | 0xCC, 0,0,
    SPI_DATA | 0x18, 0,0,
                         
    SPI_CMD  | 0xB0, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x0A, 0,0,
    SPI_DATA | 0x11, 0,0,
    SPI_DATA | 0x0C, 0,0,
    SPI_DATA | 0x10, 0,0,
    SPI_DATA | 0x05, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x08, 0,0,
    SPI_DATA | 0x08, 0,0,
    SPI_DATA | 0x1F, 0,0,
    SPI_DATA | 0x07, 0,0,
    SPI_DATA | 0x13, 0,0,
    SPI_DATA | 0x10, 0,0,
    SPI_DATA | 0xA9, 0,0,
    SPI_DATA | 0x30, 0,0,
    SPI_DATA | 0x18, 0,0,
                         
    SPI_CMD  | 0xB1, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x0B, 0,0,
    SPI_DATA | 0x11, 0,0,
    SPI_DATA | 0x0D, 0,0,
    SPI_DATA | 0x0F, 0,0,
    SPI_DATA | 0x05, 0,0,
    SPI_DATA | 0x02, 0,0,
    SPI_DATA | 0x07, 0,0,
    SPI_DATA | 0x06, 0,0,
    SPI_DATA | 0x20, 0,0,
    SPI_DATA | 0x05, 0,0,
    SPI_DATA | 0x15, 0,0,
    SPI_DATA | 0x13, 0,0,
    SPI_DATA | 0xA9, 0,0,
    SPI_DATA | 0x30, 0,0,
    SPI_DATA | 0x18, 0,0,
                         
    SPI_CMD  | 0xFF, 0,0,
    SPI_DATA | 0x77, 0,0,
    SPI_DATA | 0x01, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x11, 0,0,
                         
    SPI_CMD  | 0xB0, 0,0,
    SPI_DATA | 0x53, 0,0,
                         
    SPI_CMD  | 0xB1, 0,0,
    SPI_DATA | 0x5E, 0,0,
                         
    SPI_CMD  | 0xB2, 0,0,
    SPI_DATA | 0x87, 0,0,
                         
    SPI_CMD  | 0xB3, 0,0,
    SPI_DATA | 0x80, 0,0,
                         
    SPI_CMD  | 0xB5, 0,0,
    SPI_DATA | 0x49, 0,0,
                         
    SPI_CMD  | 0xB7, 0,0,
    SPI_DATA | 0x85, 0,0,
                         
    SPI_CMD  | 0xB8, 0,0,
    SPI_DATA | 0x21, 0,0,
                         
    SPI_CMD  | 0xC1, 0,0,
    SPI_DATA | 0x78, 0,0,
                         
    SPI_CMD  | 0xC2, 0,0,
    SPI_DATA | 0x78, 0,0,
                         
    SPI_CMD  | 0xEE, 0,0,
    SPI_DATA | 0x42, 0,0,
                         
    REGFLAG_DELAY, 100,0,
                         
    SPI_CMD  | 0xE0, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x02, 0,0,
                         
    SPI_CMD  | 0xE1, 0,0,
    SPI_DATA | 0x03, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x02, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x33, 0,0,
    SPI_DATA | 0x33, 0,0,
                         
    SPI_CMD  | 0xE2, 0,0,
    SPI_DATA | 0x22, 0,0,
    SPI_DATA | 0x22, 0,0,
    SPI_DATA | 0x33, 0,0,
    SPI_DATA | 0x33, 0,0,
    SPI_DATA | 0x88, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x87, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
                         
    SPI_CMD  | 0xE3, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x22, 0,0,
    SPI_DATA | 0x22, 0,0,
                         
    SPI_CMD  | 0xE4, 0,0,
    SPI_DATA | 0x44, 0,0,
    SPI_DATA | 0x44, 0,0,
                         
    SPI_CMD  | 0xE5, 0,0,
    SPI_DATA | 0x04, 0,0,
    SPI_DATA | 0x84, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0x06, 0,0,
    SPI_DATA | 0x86, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0x08, 0,0,
    SPI_DATA | 0x88, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0x0A, 0,0,
    SPI_DATA | 0x8A, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0xA0, 0,0,
                         
    SPI_CMD  | 0xE6, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x22, 0,0,
    SPI_DATA | 0x22, 0,0,
                         
    SPI_CMD  | 0xE7, 0,0,
    SPI_DATA | 0x44, 0,0,
    SPI_DATA | 0x44, 0,0,
                         
    SPI_CMD  | 0xE8, 0,0,
    SPI_DATA | 0x03, 0,0,
    SPI_DATA | 0x83, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0x05, 0,0,
    SPI_DATA | 0x85, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0x07, 0,0,
    SPI_DATA | 0x87, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0x09, 0,0,
    SPI_DATA | 0x89, 0,0,
    SPI_DATA | 0xA0, 0,0,
    SPI_DATA | 0xA0, 0,0,
                         
  //{SPI_CMD  | 0xE9,0,0,
  //{SPI_DATA | 0x36,0,0,
  //{SPI_DATA | 0x00,0,0,
                         
    SPI_CMD  | 0xEB, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x01, 0,0,
    SPI_DATA | 0xE4, 0,0,
    SPI_DATA | 0xE4, 0,0,
    SPI_DATA | 0x88, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x40, 0,0,
                         
    SPI_CMD  | 0xEC, 0,0,
    SPI_DATA | 0x3C, 0,0,
    SPI_DATA | 0x01, 0,0,
                         
    SPI_CMD  | 0xED, 0,0,
    SPI_DATA | 0xAB, 0,0,
    SPI_DATA | 0x89, 0,0,
    SPI_DATA | 0x76, 0,0,
    SPI_DATA | 0x54, 0,0,
    SPI_DATA | 0x02, 0,0,
    SPI_DATA | 0xFF, 0,0,
    SPI_DATA | 0xFF, 0,0,
    SPI_DATA | 0xFF, 0,0,
    SPI_DATA | 0xFF, 0,0,
    SPI_DATA | 0xFF, 0,0,
    SPI_DATA | 0xFF, 0,0,
    SPI_DATA | 0x20, 0,0,
    SPI_DATA | 0x45, 0,0,
    SPI_DATA | 0x67, 0,0,
    SPI_DATA | 0x98, 0,0,
    SPI_DATA | 0xBA, 0,0,
                         
    SPI_CMD  | 0xEF, 0,0,
    SPI_DATA | 0x10, 0,0,
    SPI_DATA | 0x0D, 0,0,
    SPI_DATA | 0x04, 0,0,
    SPI_DATA | 0x08, 0,0,
    SPI_DATA | 0x3F, 0,0,
    SPI_DATA | 0x1F, 0,0,
                         
    SPI_CMD  | 0xFF, 0,0,
    SPI_DATA | 0x77, 0,0,
    SPI_DATA | 0x01, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
    SPI_DATA | 0x00, 0,0,
                         
    SPI_CMD  | 0x3A, 0,0,
    SPI_DATA | 0x55, 0,0,
    REGFLAG_DELAY,  20,0, 
    SPI_CMD  | 0x29, 0,0,

    REGFLAG_END_OF_TABLE, 0,0,
};


MhalPnlParamConfig_t stPanel_T30P117 =
{
    "T30P117_360x640", // const char *m_pPanelName;                ///<  PanelName
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
    2,  //MS_U8 m_ucPanelVSyncWidth;               ///<  define PANEL_VSYNC_WIDTH
    16,  //MS_U8 m_ucPanelVBackPorch;               ///<  define PANEL_VSYNC_BACK_PORCH

    // DE related
    48,////3,  //MS_U16 m_wPanelHStart;                   ///<  VOP_04[11:0], PANEL_HSTART, DE H Start (PANEL_HSYNC_WIDTH + PANEL_HSYNC_BACK_PORCH)
    18,  //MS_U16 m_wPanelVStart;                   ///<  VOP_06[11:0], PANEL_VSTART, DE V Start
    PANEL_W,//480,  //MS_U16 m_wPanelWidth;                    ///< PANEL_WIDTH, DE width (VOP_05[11:0] = HEnd = HStart + Width - 1)
    PANEL_H,//854,  //MS_U16 m_wPanelHeight;                   ///< PANEL_HEIGHT, DE height (VOP_07[11:0], = Vend = VStart + Height - 1)

    // DClk related
    0,  //MS_U16 m_wPanelMaxHTotal;                ///<  PANEL_MAX_HTOTAL. Reserved for future using.
    PANEL_W+48,//484,  //MS_U16 m_wPanelHTotal;                   ///<  VOP_0C[11:0], PANEL_HTOTAL
    0,  //MS_U16 m_wPanelMinHTotal;                ///<  PANEL_MIN_HTOTAL. Reserved for future using.

    0,  //MS_U16 m_wPanelMaxVTotal;                ///<  PANEL_MAX_VTOTAL. Reserved for future using.
    PANEL_H+18,//888,  //MS_U16 m_wPanelVTotal;                   ///<  VOP_0D[11:0], PANEL_VTOTAL
    0,  //MS_U16 m_wPanelMinVTotal;                ///<  PANEL_MIN_VTOTAL. Reserved for future using.

    0,  //MS_U8 m_dwPanelMaxDCLK;                  ///<  PANEL_MAX_DCLK. Reserved for future using.
    24,  //MS_U8 m_dwPanelDCLK;                     ///<  LPLL_0F[23:0], PANEL_DCLK          ,{0x3100_10[7:0], 0x3100_0F[15:0]}
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
MhalPnlMipiDsiConfig_t stMipiDsiConfig_T30P117 =
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

    (u8 *)TP30P117Cmd,
    sizeof(TP30P117Cmd),
};

#if 0
MI_S32 init_panel_spi_cmd(void)
{
    MI_U32 n;
    MI_U32 u32SpiCmdCnt;
    MI_PANEL_GpioConfig_t stGpioCfg;

    stGpioCfg.u16GpioBL = BL_GPIO_PIN;
    stGpioCfg.u16GpioRST = RST_GPIO_PIN;
    stGpioCfg.u16GpioCS = CS_GPIO_PIN;
    stGpioCfg.u16GpioSCL = CLK_GPIO_PIN;
    stGpioCfg.u16GpioSDO = SDO_GPIO_PIN;
    stGpioCfg.u16GpioEN = UNNEEDED_CONTROL;
    ExecFunc(MI_PANEL_GPIO_Init(&stGpioCfg), MI_SUCCESS);

    ExecFunc(MI_PANEL_SetGpioStatus(BL_GPIO_PIN, 1), MI_SUCCESS);
    ExecFunc(MI_PANEL_SetGpioStatus(CS_GPIO_PIN, 1), MI_SUCCESS);
    usleep(1 * 1000);
    ExecFunc(MI_PANEL_SetGpioStatus(CS_GPIO_PIN, 0), MI_SUCCESS);
    ExecFunc(MI_PANEL_SetGpioStatus(SDO_GPIO_PIN, 1), MI_SUCCESS);
    ExecFunc(MI_PANEL_SetGpioStatus(CLK_GPIO_PIN, 0), MI_SUCCESS);

    //panel reset
    // ExecFunc(MI_PANEL_SetGpioStatus(RST_GPIO_PIN, 0), MI_SUCCESS);
    usleep(20 * 1000);
    // ExecFunc(MI_PANEL_SetGpioStatus(RST_GPIO_PIN, 1), MI_SUCCESS);

    u32SpiCmdCnt = sizeof(Initialization_CmdTable) / sizeof(MI_PANEL_SpiCmdTable_t);
    for (n = 0; n < u32SpiCmdCnt; n++)
    {
        if (REGFLAG_DELAY == Initialization_CmdTable[n].cmd)
            usleep(Initialization_CmdTable[n].count*1000);
        else if (REGFLAG_END_OF_TABLE == Initialization_CmdTable[n].cmd)
            break;
        else
        {
            MI_PANEL_SetGpioStatus(CS_GPIO_PIN, 0);
            MI_PANEL_SetCmd(Initialization_CmdTable[n].cmd, 16);
            MI_PANEL_SetGpioStatus(CS_GPIO_PIN, 1);
        }
    }

    return MI_SUCCESS;
}
#endif
