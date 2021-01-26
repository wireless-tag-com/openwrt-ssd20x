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

#define  MHAL_HDMITX_C

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
// Common Definition
#include "mhal_common.h"

// Internal Definition
#include "hal_hdmitx_chip.h"
#include "regHDMITx.h"
#include "halHDMIUtilTx.h"
#include "halHDMITx.h"
#include "drvHDMITx.h"
#include "hal_hdmitx_top_tbl.h"
#include "drv_hdmitx_os.h"
// External Definition
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define HDMITX_CLK_DOUBLE   1

#define GENERAL_PKT_NUM		0x0BU //wilson@kano
#define INFOFRM_PKT_NUM		0x08U //for HDR packet ID = 0x87;  0x06U //wilson@kano


#define HDMITX_VS_INFO_PKT_VER				0x01U
#define HDMITX_VS_INFO_PKT_LEN				0x1BU

#define HDMITX_AVI_INFO_PKT_VER				0x02U
#define HDMITX_AVI_INFO_PKT_LEN				0x0DU

#define HDMITX_SPD_INFO_PKT_VER				0x01U
#define HDMITX_SPD_INFO_PKT_LEN				0x19U

#define HDMITX_AUD_INFO_PKT_VER				0x01U
#define HDMITX_AUD_INFO_PKT_LEN				0x0AU

#define HDMITX_HDR_INFO_PKT_VER             0x01U
#define HDMITX_HDR_INFO_PKT_LEN             0x1BU //wilson@kano: temp solution



#define     IS_STOP_PKT(_X_)            ( (_X_ & E_HDMITX_STOP_PACKET) ? 1 : 0 )
#define     IS_CYCLIC_PKT(_X_)          ( (_X_ & E_HDMITX_CYCLIC_PACKET) ? 1 : 0 )

// HDMI packet cyclic frame count
#define HDMITX_PACKET_NULL_FCNT         0U           ///< 0 ~ 31
#define HDMITX_PACKET_ACR_FCNT          0U           ///< 0 ~ 15
#define HDMITX_PACKET_GC_FCNT           0U           ///< 0 ~ 1
#define HDMITX_PACKET_ACP_FCNT          15U          ///< 0 ~ 31
#define HDMITX_PACKET_ISRC_FCNT         15U          ///< 0 ~ 31

#define HDMITX_PACKET_VS_FCNT           0U          ///< 0 ~ 31
#define HDMITX_PACKET_AVI_FCNT          0U           ///< 0 ~ 31
#define HDMITX_PACKET_SPD_FCNT          1U          ///< 0 ~ 31
#define HDMITX_PACKET_AUD_FCNT          0U           ///< 0 ~ 31
#define HDMITX_PACKET_HDR_FCNT          0U

#define HDMITX_PACKET_SPD_SDI           1U   // Digital STB
#define HDMITX_CSC_SUPPORT_R2Y          1U

#define HDMITX_MAX_PIXEL_CLK            597000000   //Max cupported pixel clock

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//*********************//
//             Video   //
//*********************//

typedef enum
{
    E_HDMITX_VIDEO_INTERLACE_MODE     = 0,
    E_HDMITX_VIDEO_PROGRESSIVE_MODE = 1,
} MDrv_HDMITx_VIDEO_MODE;

typedef enum
{
    E_HDMITX_VIDEO_POLARITY_HIGH  = 0,
    E_HDMITX_VIDEO_POLARITY_LOW = 1,
} MDrv_HDMITx_VIDEO_POLARITY;

typedef struct
{
    MDrv_HDMITx_VIDEO_MODE       i_p_mode;          // interlace / progressive mode
    MDrv_HDMITx_VIDEO_POLARITY   h_polarity;        // Hsync polarity
    MDrv_HDMITx_VIDEO_POLARITY   v_polarity;        // Vsync polarity
    MS_U32                       pixel_clk;         //pixel clock
} MDrv_HDMITx_VIDEO_MODE_INFO_TYPE;

//*********************//
//             Packet  //
//*********************//

typedef enum
{
    E_HDMITX_ACT_GCP_CMD    = 0,
    E_HDMITX_ACT_ACR_CMD    = 1,
    E_HDMITX_ACT_AVI_CMD    = 2,
    E_HDMITX_ACT_AUD_CMD    = 3,
    E_HDMITX_ACT_SPD_CMD    = 4,
    E_HDMITX_ACT_MPG_CMD    = 5,
    E_HDMITX_ACT_VSP_CMD    = 6,
    E_HDMITX_ACT_NUL_CMD    = 7,
    E_HDMITX_ACT_ACP_CMD    = 8,
    E_HDMITX_ACT_ISRC_CMD    = 9,
    E_HDMITX_ACT_GCP_DC_CMD    = 10, // GCP with non-zero CD
    E_HDMITX_ACT_GMP_CMD    = 11, // Gamut Metadata packet
} MDrvHDMITX_PKTS_ACT_CMD;

typedef struct PKT
{
    MS_BOOL User_Define;
    MsHDMITX_PACKET_PROCESS Define_Process;
    MS_U8 Define_FCnt;
}PKT_Behavior;
//*********************//
//             Audio   //
//*********************//
typedef struct
{
    MS_U32 u32N;
    MS_U32 u32CTS;
}MDrv_HDMITx_AUDIO_CTS_N_TYPE;

typedef struct
{
    MS_U8     CH_Status3;
    MDrv_HDMITx_AUDIO_CTS_N_TYPE   stCtsN;
} MDrv_HDMITx_AUDIO_FREQ_TYPE;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
MS_U8     gDivider = 0x00;
MS_BOOL   gDivFlag = FALSE;
static MS_U8 gu8ChipVerNum = 0x00;
MS_BOOL     g_bSupportSCDC = FALSE;
MS_U8 ub148to165MHz = 0;

stHDMITx_PKT_ATTRIBUTE gbGeneralPktList[GENERAL_PKT_NUM]; //wilson@kano
stHDMITx_PKT_ATTRIBUTE gbInfoFrmPktList[INFOFRM_PKT_NUM]; //wilson@kano

// User defined packet behavior
PKT_Behavior NULL_PACKET = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior ACR_PACKET = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior AS_PACKET = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior GC_PACKET = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior ACP_PACKET = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior ISRC1_PACKET = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior ISRC2_PACKET = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior DSD_PACKET = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior HBR_PACKET = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior GM_PACKET = {FALSE, E_HDMITX_STOP_PACKET, 0};

PKT_Behavior VS_INFORAME = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior AVI_INFORAME = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior SPD_INFORAME = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior AUDIO_INFORAME = {FALSE, E_HDMITX_STOP_PACKET, 0};
PKT_Behavior MPEG_INFORAME = {FALSE, E_HDMITX_STOP_PACKET, 0};
//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//*********************//
//             Video   //
//*********************//

// It should be mapped with MsHDMITX_VIDEO_TIMING structure in drvHDMITx.h
MDrv_HDMITx_VIDEO_MODE_INFO_TYPE HDMITxVideoModeTbl[E_HDMITX_RES_MAX]=
{
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_LOW,    E_HDMITX_VIDEO_POLARITY_LOW,  25175000},  // 0: 640x480p
    {E_HDMITX_VIDEO_INTERLACE_MODE,     E_HDMITX_VIDEO_POLARITY_LOW,    E_HDMITX_VIDEO_POLARITY_LOW,  27000000},  // 1: 720x480i
    {E_HDMITX_VIDEO_INTERLACE_MODE,     E_HDMITX_VIDEO_POLARITY_LOW,    E_HDMITX_VIDEO_POLARITY_LOW,  27000000},  // 2: 720x576i
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_LOW,    E_HDMITX_VIDEO_POLARITY_LOW,  27000000},  // 3: 720x480p
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_LOW,    E_HDMITX_VIDEO_POLARITY_LOW,  27000000},  // 4: 720x576p
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 74250000},  // 5: 1280x720p_50Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 74250000},  // 6: 1280x720p_60Hz
    {E_HDMITX_VIDEO_INTERLACE_MODE,     E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 74250000},  // 7: 1920x1080i_50Hz
    {E_HDMITX_VIDEO_INTERLACE_MODE,     E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 74250000},  // 8: 1920x1080i_60Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 74250000},  // 9: 1920x1080p_24Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 74250000},  // 10: 1920x1080p_25Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 74250000},  // 11: 1920x1080p_30Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 148500000},  // 12: 1920x1080p_50Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 148500000},  // 13: 1920x1080p_60Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 148500000},  // 14: 1920x2205p_24Hz //3D: 1920x1080p_24Hz x2
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 148500000},  // 15: 1280x1470p_50Hz //3D: 1280x720_50Hz x2
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 148500000},  // 16: 1280x1470p_60Hz //3D: 1280x720_60Hz x2

    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 297000000},  // 17:93: 3840x2160p_24Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 297000000},  // 18:94: 3840x2160p_25Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 297000000},  // 19:95: 3840x2160p_30Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 594000000},  // 20:96: 3840x2160p_50Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 594000000},  // 21:97: 3840x2160p_60Hz

    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 297000000},  // 22:98: 4096x2160p_24Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 297000000},  // 23:99: 4096x2160p_25Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 297000000},  // 24:100: 4096x2160p_30Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 594000000},  // 25:101: 4096x2160p_50Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH,94000000},  // 26:102: 4096x2160p_60Hz

    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 162000000},  // 27: 1600x1200p_60Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 106500000},  // 28: 1440x900p_60Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 108000000},  // 29: 1280x1024p_60Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 65000000},   // 30: 1024x768p_60Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 59400000},   // 31: 1280x720p_24Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 74250000},   // 32: 1280x720p_25Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 74250000},   // 33: 1280x720p_30Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 85800000},   // 34: 1366x768_60Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 83700000},   // 35: 1280x800_60Hz
    {E_HDMITX_VIDEO_PROGRESSIVE_MODE,   E_HDMITX_VIDEO_POLARITY_HIGH,   E_HDMITX_VIDEO_POLARITY_HIGH, 146300000},  // 36: 1680x1050_60Hz
    // the following is 4k2k timing list, if not support, default is 3840x2160p@30
};

//*********************//
//             Audio   //
//*********************//
MDrv_HDMITx_AUDIO_FREQ_TYPE TxAudioFreqTbl[E_HDMITX_AUDIO_FREQ_MAX_NUM] =
{
    {0x02, {0x01800, 0x06978}},  // No signal, set to 48 KHz
    {0x03, {0x01000, 0x06978}},  // 32
    {0x00, {0x01880, 0x07530}},  // 44
    {0x02, {0x01800, 0x06978}},  // 48
    {0x08, {0x03100, 0x07530}},  // 88
    {0x0a, {0x03000, 0x06978}},  // 96
    {0x0c, {0x06200, 0x07530}},  // 176
    {0x0e, {0x06000, 0x06978}},  // 192
};

//*********************//
//             Packet  //
//*********************//

MS_U8 HDMITX_AviCmrTbl[E_HDMITX_RES_MAX] =
{
    0x48, 0x48, 0x48, 0x48, 0x48, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8,            // SDTV C=01(601),M=00(no data) ,R=1000(same)
    0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0x48,
    0xA8, 0x48, 0x48, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8
};

MS_U8 HDMITX_AviVicTbl[E_HDMITX_RES_MAX] =
{
    1,  6,  21, 2,  17, 19, 4,  20, 5,  32, 33,  34,  31,  16,           // SDTV 480i60,576i50,480p60,576p50,720p50,720p60,1080i50,1080i60,1080p24,1080p25,1080p30, 1080p50, 1080p60
    32, 19, 4,  0,  0,  0,  96, 97, 0,  99, 100, 101, 102,  0,
    0,  0,  0, 60, 61, 62,  0,  0,  0
};


MS_U8 HDMITX_SPDData[REG_HDMITX_PKT_SPD_LEN] =
{// 0~7 Vendor Name, 8~24 Producet name
    "SIGMA   HDMITX TX DEMO   ",
};


MS_U16 HDMITX_SpdPktReg[REG_HDMITX_PKT_SPD_LEN] =
{
    REG_HDMITX_PKT_SPD_1,  REG_HDMITX_PKT_SPD_2,  REG_HDMITX_PKT_SPD_3,  REG_HDMITX_PKT_SPD_4,
    REG_HDMITX_PKT_SPD_5,  REG_HDMITX_PKT_SPD_6,  REG_HDMITX_PKT_SPD_7,  REG_HDMITX_PKT_SPD_8,
    REG_HDMITX_PKT_SPD_9,  REG_HDMITX_PKT_SPD_10, REG_HDMITX_PKT_SPD_11, REG_HDMITX_PKT_SPD_12,
    REG_HDMITX_PKT_SPD_13, REG_HDMITX_PKT_SPD_14, REG_HDMITX_PKT_SPD_15, REG_HDMITX_PKT_SPD_16,
    REG_HDMITX_PKT_SPD_17, REG_HDMITX_PKT_SPD_18, REG_HDMITX_PKT_SPD_19, REG_HDMITX_PKT_SPD_20,
    REG_HDMITX_PKT_SPD_21, REG_HDMITX_PKT_SPD_22, REG_HDMITX_PKT_SPD_23, REG_HDMITX_PKT_SPD_24,
    REG_HDMITX_PKT_SPD_25,
};

MS_U16 HDMITX_AviPktReg[REG_HDMITX_PKT_AVI_LEN] =
{
    REG_HDMITX_PKT_AVI_1,  REG_HDMITX_PKT_AVI_2,  REG_HDMITX_PKT_AVI_3,  REG_HDMITX_PKT_AVI_4,
    REG_HDMITX_PKT_AVI_5,  REG_HDMITX_PKT_AVI_6,  REG_HDMITX_PKT_AVI_7,  REG_HDMITX_PKT_AVI_8,
    REG_HDMITX_PKT_AVI_9,  REG_HDMITX_PKT_AVI_10, REG_HDMITX_PKT_AVI_11, REG_HDMITX_PKT_AVI_12,
    REG_HDMITX_PKT_AVI_13,
};

MS_U16 HDMITX_AdoPktReg[REG_HDMITX_PKT_ADO_LEN] =
{
    REG_HDMITX_PKT_ADO_1, REG_HDMITX_PKT_ADO_2, REG_HDMITX_PKT_ADO_3, REG_HDMITX_PKT_ADO_4,
    REG_HDMITX_PKT_ADO_5, REG_HDMITX_PKT_ADO_6,
};


MS_U16 HDMITX_VsPktReg[REG_HDMITX_PKT_VS_LEN] =
{
    REG_HDMITX_PKT_VS_1,  REG_HDMITX_PKT_VS_2,  REG_HDMITX_PKT_VS_3,  REG_HDMITX_PKT_VS_4,
    REG_HDMITX_PKT_VS_5,  REG_HDMITX_PKT_VS_6,  REG_HDMITX_PKT_VS_7,  REG_HDMITX_PKT_VS_8,
    REG_HDMITX_PKT_VS_9,  REG_HDMITX_PKT_VS_10, REG_HDMITX_PKT_VS_11, REG_HDMITX_PKT_VS_12,
    REG_HDMITX_PKT_VS_13, REG_HDMITX_PKT_VS_14, REG_HDMITX_PKT_VS_15, REG_HDMITX_PKT_VS_16,
    REG_HDMITX_PKT_VS_17, REG_HDMITX_PKT_VS_18, REG_HDMITX_PKT_VS_19, REG_HDMITX_PKT_VS_20,
    REG_HDMITX_PKT_VS_21, REG_HDMITX_PKT_VS_22, REG_HDMITX_PKT_VS_23, REG_HDMITX_PKT_VS_24,
    REG_HDMITX_PKT_VS_25, REG_HDMITX_PKT_VS_26, REG_HDMITX_PKT_VS_27,
};


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

#ifdef CAMDRV_DEBUG
#define DBG_HDMITX(_f)                  (_f)
#else
#define DBG_HDMITX(_f)
#endif



#define PARSING_ATOP_IDX(x)  (  x ==HAL_HDMITX_ID_480_60P       ?  "480_60P" : \
                                x ==HAL_HDMITX_ID_576_50P       ?  "576_50P" : \
                                x ==HAL_HDMITX_ID_720_50P       ?  "720_50P" : \
                                x ==HAL_HDMITX_ID_720_60P       ?  "720_60P" : \
                                x ==HAL_HDMITX_ID_1080_24P      ?  "1080_24P" : \
                                x ==HAL_HDMITX_ID_1080_25P      ?  "1080_25P" : \
                                x ==HAL_HDMITX_ID_1080_30P      ?  "1080_30P" : \
                                x ==HAL_HDMITX_ID_1080_50P      ?  "1080_50P" : \
                                x ==HAL_HDMITX_ID_1080_60P      ?  "1080_60P" : \
                                x ==HAL_HDMITX_ID_1024x768_60P  ?  "1024x768_60P" : \
                                x ==HAL_HDMITX_ID_1280x1024_60P ?  "1280x1024_60P" : \
                                x ==HAL_HDMITX_ID_1366x768_60P  ?  "1366x768_60P" : \
                                x ==HAL_HDMITX_ID_1440x900_60P  ?  "1440x900_60P" : \
                                x ==HAL_HDMITX_ID_1280x800_60P  ?  "1280x800_60P" : \
                                x ==HAL_HDMITX_ID_1680x1050_60P ?  "1680x1050_60P" : \
                                x ==HAL_HDMITX_ID_1600x1200_60P ?  "1600x1200_60P" : \
                                                                   "UNKWON")


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
/*********************************************************************/
/*                                                                                                                     */
/*                                         HDCP22 Relative                                                    */
/*                                                                                                                     */
/*********************************************************************/
void MHal_HDMITx_HDCP2TxInit(MS_BOOL bEnable)
{
    //MHal_HDMITx_Mask_Write(HDMITX_HDCP2TX_BASE, 0x0000, 0x11, bEnable ? 0x11 : 0x00); // bit 0: enable hdcp22; bit 4: enable EESS
    if (bEnable)
    {
        //MHal_HDMITx_Mask_Write(HDMITX_HDCP2TX_BASE, 0x0000, 0x02, 0x02); //reset hdcp22 FSM
        //MHal_HDMITx_Mask_Write(HDMITX_HDCP2TX_BASE, 0x0000, 0x02, 0x00);
    }
}

void MHal_HDMITx_HDCP2TxEnableEncryptEnable(MS_BOOL bEnable)
{
    //MHal_HDMITx_Mask_Write(HDMITX_HDCP2TX_BASE, 0x0000, 0x04, bEnable ? 0x04 : 0x00); //bit 2: authentication pass
    //MHal_HDMITx_Mask_Write(HDMITX_HDCP2TX_BASE, 0x0000, 0x08, bEnable ? 0x08 : 0x00); //bit 3: enable hdcp22 to issue encryption enable signal
}

void MHal_HDMITx_HDCP2TxFillCipherKey(MS_U8 *pu8Riv, MS_U8 *pu8KsXORLC128)
{
#define DEF_HDCP2CIPHER_DBG 0
#define SIZE_OF_KSXORLC128  16
#define SIZE_OF_RIV         8

    MS_U8 cnt = 0;

#if (DEF_HDCP2CIPHER_DBG == 1)
    printf("Ks^LC128:\r\n");
    for ( cnt = 0; cnt < SIZE_OF_KSXORLC128; cnt++ )
    {
        printf("0x%02X ", *(pu8KsXORLC128 + cnt));
    }
    printf("\r\n");

    printf("Riv:\r\n");
    for ( cnt = 0; cnt < SIZE_OF_RIV; cnt++ )
    {
        printf("0x%02X ", *(pu8Riv + cnt));
    }
    printf("\r\n");
#endif

#undef DEF_HDCP2CIPHER_DBG
    //MHal_HDMITx_Mask_Write(0x172F00, 0x01, 0x0020, 0x0020); //reverse order of cihper key
    //MHal_HDMITx_Mask_Write(0x172B00, 0x01, 0x8000, 0x0000); //disable hdcp 1.4 module
    for ( cnt = 0; cnt < (SIZE_OF_KSXORLC128>>1); cnt++)
    {
        //MHal_HDMITx_Write(HDMITX_SECUTZPC_BASE, 0x60 + (SIZE_OF_KSXORLC128 >> 1) - 1 - cnt, *(pu8KsXORLC128 + cnt*2 + 1)|(*(pu8KsXORLC128 + cnt*2)<<8));
    }
    for ( cnt = 0; cnt < (SIZE_OF_RIV>>1); cnt++)
    {
        //MHal_HDMITx_Write(HDMITX_SECUTZPC_BASE, 0x68 + (SIZE_OF_RIV >> 1) - 1 - cnt, *(pu8Riv + cnt*2 + 1)|(*(pu8Riv + cnt*2)<<8));
    }
}

void Mhal_HDMITx_SetSCDCCapability(MS_BOOL bFlag)
{
    g_bSupportSCDC = bFlag;
}

void MHal_HDMITx_SetChipVersion(MS_U8 u8ChipVer)
{
    gu8ChipVerNum = u8ChipVer;
}

// HPD: GPIO_PM[11] -> external interrupt[11], register 0x000E00[14]
// DVI disconnet: must power down clock termination resistor: TM_REG[0] = 1, TM_REG[16:15] = 00, TM_REG[35:34] = 00.
// Interrupt helper functoins
//------------------------------------------------------------------------------
/// @brief Disable interrupt
/// @param[in] u32Int interrupter value
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_Int_Disable(MS_U32 u32Int)
{

}

//------------------------------------------------------------------------------
/// @brief Enable interrupt
/// @param[in] u32Int interrupter value
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_Int_Enable(MS_U32 u32Int)
{

}

//------------------------------------------------------------------------------
/// @brief MHal_HDMITx_Int_Clear
/// @param[in] u32Int interrupter value
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_Int_Clear(MS_U32 u32Int)
{

}

//------------------------------------------------------------------------------
/// @brief MHal_HDMITx_Int_Status
/// @param[in] u32Int interrupter value
/// @return None
//------------------------------------------------------------------------------
MS_U32 MHal_HDMITx_Int_Status(void)
{
    return 0;
}


//------------------------------------------------------------------------------
/// @brief This routine is to get HDMI receiver DVI clock and HPD status.
/// @return MsHDMITX_RX_STATUS
//------------------------------------------------------------------------------
MsHDMITX_RX_STATUS MHal_HDMITx_GetRXStatus(void)
{
    MS_BOOL dviclock_s, hpd_s = FALSE;
    MsHDMITX_RX_STATUS state;

    dviclock_s = MHal_HDMITx_Read(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_18) & 0xCCCC ? FALSE : TRUE;

#if !defined(CONFIG_MBOOT)
    hpd_s = DrvHdmitxOsGetGpioValue( DrvHdmitxOsGetHpdGpinPin() );
#else
    hpd_s = 1;
#endif

    if((dviclock_s == FALSE) && (hpd_s == FALSE))
        state = E_HDMITX_DVIClock_L_HPD_L;
    else if((dviclock_s == FALSE) && (hpd_s == TRUE))
        state = E_HDMITX_DVIClock_L_HPD_H;
    else if((dviclock_s == TRUE) && (hpd_s == FALSE))
        state = E_HDMITX_DVIClock_H_HPD_L;
    else
        state = E_HDMITX_DVIClock_H_HPD_H;

    return state;
}


//------------------------------------------------------------------------------
/// @brief MHal_HDMITX_SetHDCPConfig
/// @param[in] u32Int HDCP mode
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITX_SetHDCPConfig(MS_U8 HDCP_mode)
{
    //MHal_HDMITx_Mask_Write(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_MODE_01, 0x0E00, HDCP_mode << 8);
}


//------------------------------------------------------------------------------
/// @brief MHal_HDMITX_GetM02Bytes
/// @param[in] u16Int index
/// @return M0 2 bytes
//------------------------------------------------------------------------------
MS_U16 MHal_HDMITX_GetM02Bytes(MS_U16 idx)
{
    return 0 ;//(MHal_HDMITx_Read(HDMITX_HDCP_REG_BASE, REG_HDCP_TX_MI_0C + idx));
}


//------------------------------------------------------------------------------
/// @brief MHal_HDMITx_InitSeq
/// @param[in] None
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_InitSeq(void)
{
    MHal_HDMITx_DumpAtopTable(E_HDMITX_RES_720x480p, E_HDMITX_VIDEO_CD_24Bits, FALSE, TRUE);

    // if the following reigster is 0, it means that no trim value in efuse.
    if( MHal_HDMITx_Read(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_19) == 0x0000 )
    {
        MHal_HDMITx_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_19, 0x1B1A);
    }

    if( MHal_HDMITx_Read(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_1A) == 0x0000 )
    {
        MHal_HDMITx_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_1A, 0x1A19);
    }

    if( (MHal_HDMITx_Read(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_38) && 0x00FF) == 0x0000 )
    {
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_38, 0x00FF, 0x001A);
    }
}

//------------------------------------------------------------------------------
/// @brief This routine is the initialization for Video module.
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_VideoInit(void)
{
}

//------------------------------------------------------------------------------
/// @brief This routine is the initialization for Audio module.
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_AudioInit(void)
{
}


//------------------------------------------------------------------------------
/// @brief This routine turn on/off HDMI PLL
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_PLLOnOff(MS_BOOL bflag)
{
    MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_34, 0x0001, bflag ? 0x0000 : 0x0001);
}


void MHal_HDMITx_PKT_User_Define_Clear(void)
{
    MS_U8 i = 0;

    for ( i = 0; i < GENERAL_PKT_NUM; i++ )
    {
        gbGeneralPktList[i].EnableUserDef = FALSE;
        gbGeneralPktList[i].FrmCntNum = 0x00;
        gbGeneralPktList[i].enPktCtrl = E_HDMITX_STOP_PACKET;
        memset(&gbGeneralPktList[i].PktPara, 0x00, sizeof(gbGeneralPktList[i].PktPara));
    }

    for ( i = 0; i < INFOFRM_PKT_NUM; i++ )
    {
        gbInfoFrmPktList[i].EnableUserDef = FALSE;
        gbInfoFrmPktList[i].FrmCntNum = 0x00;
        gbInfoFrmPktList[i].enPktCtrl = E_HDMITX_STOP_PACKET;
        memset(&gbInfoFrmPktList[i].PktPara, 0x00, sizeof(gbInfoFrmPktList[i].PktPara));
    }
}

void MHal_HDMITx_PKT_User_Define(MsHDMITX_PACKET_TYPE packet_type, MS_BOOL def_flag,
        MsHDMITX_PACKET_PROCESS def_process, MS_U8 def_fcnt)
{
    if (packet_type & 0x80) //infoframe packet type
	{
		gbInfoFrmPktList[packet_type & (~0x80)].EnableUserDef = def_flag;
		gbInfoFrmPktList[packet_type & (~0x80)].FrmCntNum = def_fcnt;
		gbInfoFrmPktList[packet_type & (~0x80)].enPktCtrl = def_process;
	}
	else
	{
		gbGeneralPktList[packet_type].EnableUserDef = def_flag;
		gbGeneralPktList[packet_type].FrmCntNum = def_fcnt;
		gbGeneralPktList[packet_type].enPktCtrl = def_process;
	}
}

MS_BOOL MHal_HDMITx_PKT_Content_Define(MsHDMITX_PACKET_TYPE packet_type, MS_U8* data, MS_U8 length)
{
    MS_BOOL bRet = TRUE;
    MS_U8 i, *ptr;

    ptr = data;
    switch(packet_type)
    {
        case E_HDMITX_VS_INFOFRAME:
            for (i=0; i < length; i++)
            {
                if (i >= REG_HDMITX_PKT_VS_LEN)
                {
                    //Packet over size, last VS packet PB register is REG_PKT_VS_14_34[7:0]
                    DBG_HDMITX(printf("VS packet over size, length = %d \n", length));
                    break;
                }

                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, HDMITX_VsPktReg[i], 0xFF, *(ptr+i));
            }
        break;
        case E_HDMITX_SPD_INFOFRAME:
            for(i=0;i<length;i++)
            {
                if(i >= REG_HDMITX_PKT_SPD_LEN )
                {
                    //Packet over size, last SPD packet PB register is REG_PKT_SPD_13_21[7:0]
                    DBG_HDMITX(printf("VS packet over size, length = %d \n", length));
                    break;
                }
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, HDMITX_SpdPktReg[i], 0xFF, *(ptr+i));
            }
        break;

        case E_HDMITX_AVI_INFOFRAME:
            for ( i = 0; i < length; i++ )
            {
                if (length > REG_HDMITX_PKT_AVI_LEN)
                {
                    DBG_HDMITX(printf("AVI packet over size, length = %d \n", length));
                    break;
                }
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, HDMITX_AviPktReg[i], 0xFF, *(ptr+i));
            }
        break;

        //wilson@kano HDR packet
        case E_HDMITX_HDR_INFOFRAME:
            //TBD
        break;

        case E_HDMITX_AUDIO_INFOFRAME:
            {
                //total length should be 192 bits ; 24byte
                length = (length < ((192>>3)) ) ? length : ((192>>3));
                for ( i = 0; i < length; i++ )
                {
                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_1+i, 0xFF, *(ptr+i));
                }
            }
        break;

        default:
            bRet = FALSE;
            DBG_HDMITX(printf("Not implemented, packet type = %u\n", packet_type));
        break;
    }

    return bRet;
}

//------------------------------------------------------------------------------
/// @brief This routine turn on/off HDMI Tx TMDS signal
/// @param[in] bRB_Swap: R/B swap; bTMDS: TMDS flag
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetTMDSOnOff(MS_BOOL bRB_Swap, MS_BOOL bTMDS)
{
#if 0
    if(bRB_Swap) // R/B channel swap
        MHal_HDMITx_Mask_Write(HDMITX_VIDEO_REG_BASE, REG_VE_CONFIG_00, BIT13, BIT13);
    else
        MHal_HDMITx_Mask_Write(HDMITX_VIDEO_REG_BASE, REG_VE_CONFIG_00, BIT13, 0);
#endif

    //reg_atop_en_data_out[13..15]:  Enable data channel data output
    if (bTMDS == TRUE)
    {
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_16, 0xFF, 0xF0);
    }
    else
    {
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_16, 0xFF, 0x00);
    }
}

//------------------------------------------------------------------------------
/// @brief This routine return on/off status of HDMI Tx TMDS signal
/// @return On/Off
//------------------------------------------------------------------------------
MsHDMITx_TMDS_STATUS MHal_HDMITx_GetTMDSStatus(void)
{
    MsHDMITx_TMDS_STATUS enTMDSStatus = E_HDMITX_TMDS_OFF;

    if( MHal_HDMITx_Read(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_16) )
    {
        enTMDSStatus = E_HDMITX_TMDS_FULL_ON;
    }
    else
    {
        enTMDSStatus = E_HDMITX_TMDS_OFF;
    }

    return enTMDSStatus;
}

//------------------------------------------------------------------------------
/// @brief This routine turn on/off HDMI Tx video output
/// @param[in] bVideo: Video flag; bCSC: CSC flag, b709format = BT.709-5
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetVideoOnOff(MS_BOOL bVideo, MS_BOOL bCSC, MS_BOOL b709format)
{
    if (bVideo == TRUE)
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CH0_MUTE, REG_HDMITX_CH0_MUTE_MSK, 0x0000);
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CH1_MUTE, REG_HDMITX_CH1_MUTE_MSK, 0x0000);
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CH2_MUTE, REG_HDMITX_CH2_MUTE_MSK, 0x0000);
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_VIDEO_MUTE, REG_HDMITX_VIDEO_MUTE_MSK, 0x0000);
    }
    else
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CH0_MUTE, REG_HDMITX_CH0_MUTE_MSK, REG_HDMITX_CH0_MUTE_ON);
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CH1_MUTE, REG_HDMITX_CH1_MUTE_MSK, REG_HDMITX_CH1_MUTE_ON);
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CH2_MUTE, REG_HDMITX_CH2_MUTE_MSK, REG_HDMITX_CH2_MUTE_ON);
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_VIDEO_MUTE, REG_HDMITX_VIDEO_MUTE_MSK, REG_HDMITX_VIDEO_MUTE_EN);
    }
}

//------------------------------------------------------------------------------
/// @brief This routine sets video color formatt
/// @param[in] bCSC: CSC flag, YUV422 12 bit, b709format = BT.709-5
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetColorFormat(MS_BOOL bCSC, MS_BOOL bHdmi422b12, MS_BOOL b709format)
{
    if (bCSC) // R->Y
    {
        DBG_HDMITX(printf("RGB->YUV \n"));
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CSC_SEL, REG_HDMITX_CSC_SEL_MSK, REG_HDMITX_CSC_SEL_R_TO_Y);
    }
    else // bypass
    {
        DBG_HDMITX(printf("bypass \n"));
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CSC_SEL, REG_HDMITX_CSC_SEL_MSK, REG_HDMITX_CSC_SEL_BYPASS);
    }
}

//------------------------------------------------------------------------------
/// @brief This routine get CSC capability
/// @param[in]
/// @return False : not support R2Y. Ture : ok.
//------------------------------------------------------------------------------
MS_BOOL MHal_HDMITx_CSC_Support_R2Y(void* pDatatIn)
{
    pDatatIn = pDatatIn;
    return HDMITX_CSC_SUPPORT_R2Y;
}

//------------------------------------------------------------------------------
/// @brief This routine sets color domain and color range transform
/// @param[in]
/// @return False : not support this command. Ture : ok.
//------------------------------------------------------------------------------
MS_BOOL MHal_HDMITx_ColorandRange_Transform(MsHDMITX_VIDEO_COLOR_FORMAT incolor, MsHDMITX_VIDEO_COLOR_FORMAT outcolor, MsHDMITX_YCC_QUANT_RANGE inrange, MsHDMITX_YCC_QUANT_RANGE outrange)
{
    MS_BOOL bRet = TRUE;

    if(incolor == E_HDMITX_VIDEO_COLOR_RGB444 && outcolor == E_HDMITX_VIDEO_COLOR_YUV444)
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CSC_SEL, REG_HDMITX_CSC_SEL_MSK, REG_HDMITX_CSC_SEL_R_TO_Y);
    }
    else if(incolor == E_HDMITX_VIDEO_COLOR_YUV444 && outcolor == E_HDMITX_VIDEO_COLOR_RGB444)
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CSC_SEL, REG_HDMITX_CSC_SEL_MSK, REG_HDMITX_CSC_SEL_Y_TO_R);
    }
    else if((incolor == E_HDMITX_VIDEO_COLOR_RGB444 && outcolor == E_HDMITX_VIDEO_COLOR_RGB444) ||
            (incolor == E_HDMITX_VIDEO_COLOR_YUV444 && outcolor == E_HDMITX_VIDEO_COLOR_YUV444))
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_CSC_SEL, REG_HDMITX_CSC_SEL_MSK, REG_HDMITX_CSC_SEL_BYPASS);
    }
    else
    {
        bRet = FALSE;
        printf("%s %d, InColor=%d, OutColor=%d, Not Supported\n", __FUNCTION__, __LINE__, incolor, outcolor);
    }

    return bRet;
}

//------------------------------------------------------------------------------
/// @brief This routine will set or stop all HDMI packet generation
/// @param[in] bflag  True: Enable packet gen, False : Disable packet gen
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_EnablePacketGen(MS_BOOL bflag)
{
    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE,
        REG_HDMITX_DATA_PERIOD_ISLAND, REG_HDMITX_DATA_PERIOD_ISLAND_MSK, bflag ? 0x21 : 0);
}

//------------------------------------------------------------------------------
/// @brief This routine sets HDMI/DVI mode
/// @param[in] bflag
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetHDMImode(MS_BOOL bflag, MsHDMITX_VIDEO_COLORDEPTH_VAL cd_val)
{
    DBG_HDMITX(printf("[%s][%d]HDMI mode = %d, Color Depth = %d \n", __FUNCTION__, __LINE__, bflag, cd_val));

    if (bflag) // HDMI mode
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE,
            REG_HDMITX_MODE, REG_HDMITX_MODE_MSK, REG_HDMITX_MODE_HDMI);
    }
    else // DVI
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE,
            REG_HDMITX_MODE, REG_HDMITX_MODE_MSK, REG_HDMITX_MODE_DVI);
    }
}


//------------------------------------------------------------------------------
/// @brief This routine sets audio on/off
/// @param[in] bflag
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetAudioOnOff(MS_BOOL bflag)
{
    if(bflag) // audio on
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AUDIO_MUTE, REG_HDMITX_AUDIO_MUTE_MSK, 0x0000);
    }
    else // audio off
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AUDIO_MUTE, REG_HDMITX_AUDIO_MUTE_MSK, REG_HDMITX_AUDIO_MUTE_EN);
    }
}

//------------------------------------------------------------------------------
/// @brief This routine sets audio sampling freq.
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetAudioFrequency(MsHDMITX_AUDIO_FREQUENCY afidx,
        MsHDMITX_AUDIO_CHANNEL_COUNT achidx, MsHDMITX_AUDIO_CODING_TYPE actidx,
        MsHDMITX_VIDEO_TIMING vidtiming)
{
    MS_U32 u32CTS, u32N;

    // HDMI audio channel setting
    if(achidx == E_HDMITX_AUDIO_CH_2) // 2 channels
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AUDIO_LAYOUT, REG_HDMITX_AUDIO_LAYOUT_MSK, REG_HDMITX_AUDIO_LAYOUT_2_CH);
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AUIDO_SP, REG_HDMITX_AUDIO_SP_MSK, REG_HDMITX_AUDIO_SP_SUBPACKET_NONE);
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AUIDO_SF, REG_HDMITX_AUDIO_SF_MSK, 0x0000);

    }
    else // 8 channels
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AUDIO_LAYOUT, REG_HDMITX_AUDIO_LAYOUT_MSK, REG_HDMITX_AUDIO_LAYOUT_8_CH);
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AUIDO_SP, REG_HDMITX_AUDIO_SP_MSK, REG_HDMITX_AUDIO_SP_SUBPACKET_0123);
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AUIDO_SF, REG_HDMITX_AUDIO_SF_MSK, 0x0000);
    }

    // Audio channel status
    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_1, 0xFF, ((actidx == E_HDMITX_AUDIO_PCM) ? 0 : BIT1) ); // [1]: PCM / non-PCM
    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_2, 0xFF, 0x00);
    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_3, 0xFF, (achidx << 4)); //audio channel count
    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_4, 0xFF, TxAudioFreqTbl[afidx].CH_Status3 ); //audio sampling frequncy;
    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_5, 0xFF, 0x00);
    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_6, 0xFF, 0x00);

    // ACR CTS code
    u32CTS = TxAudioFreqTbl[afidx].stCtsN.u32CTS;
    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_ACR_CTS_1, REG_HDMITX_ACR_CTS_1_MSK, REG_HDMITX_ACR_CTS_1_VAL(u32CTS));
    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_ACR_CTS_2, REG_HDMITX_ACR_CTS_2_MSK, REG_HDMITX_ACR_CTS_2_VAL(u32CTS));
    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_ACR_CTS_3, REG_HDMITX_ACR_CTS_3_MSK, REG_HDMITX_ACR_CTS_3_VAL(u32CTS));

    // ACR N code
    u32N = TxAudioFreqTbl[afidx].stCtsN.u32N;
    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_ACR_N_1, REG_HDMITX_ACR_N_1_MSK, REG_HDMITX_ACR_N_1_VAL(u32N));
    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_ACR_N_2, REG_HDMITX_ACR_N_2_MSK, REG_HDMITX_ACR_N_2_VAL(u32N));
    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_ACR_N_3, REG_HDMITX_ACR_N_3_MSK, REG_HDMITX_ACR_N_3_VAL(u32N));
}

//------------------------------------------------------------------------------
/// @brief This routine sets audio source format.
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetAudioSourceFormat(MsHDMITX_AUDIO_SOURCE_FORMAT fmt)
{
}

//------------------------------------------------------------------------------
/// @brief This routine Get Audio CTS value
/// @return CTS
//------------------------------------------------------------------------------
MS_U32 MHal_HDMITx_GetAudioCTS(void)
{
    return 0;
}

//------------------------------------------------------------------------------
/// @brief This routine Mute Audio FIFO
/// @param[in] bflag: True: mute audio, False: unmute audio
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_MuteAudioFIFO(MS_BOOL bflag)
{
}

//------------------------------------------------------------------------------
/// @brief This routine sets HDMI Tx HDCP encryption On/Off
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetHDCPOnOff(MS_BOOL hdcp_flag, MS_BOOL hdmi_flag)
{

}

//------------------------------------------------------------------------------
/// @brief This routine calculate check sum of infoframes.
/// @param[in] packet_type packet type
/// @return checksum
//------------------------------------------------------------------------------
MS_U8 MHal_HDMITx_InfoFrameCheckSum(MsHDMITX_PACKET_TYPE packet_type)
{
    MS_U32 u32SumVal = 0;
    MS_U8 i = 0;

    switch (packet_type)
    {
        case E_HDMITX_VS_INFOFRAME:
            u32SumVal += (E_HDMITX_VS_INFOFRAME + HDMITX_VS_INFO_PKT_VER + HDMITX_VS_INFO_PKT_LEN);

            for ( i = 0; i < HDMITX_VS_INFO_PKT_LEN; i++ )
            {
                u32SumVal += MHal_HDMITx_ReadByte(HDMITX_DTOP_BASE, HDMITX_VsPktReg[i]);
            }
        break;

        case E_HDMITX_AVI_INFOFRAME:
            u32SumVal += (E_HDMITX_AVI_INFOFRAME + HDMITX_AVI_INFO_PKT_VER + HDMITX_AVI_INFO_PKT_LEN);
            for ( i = 0; i < REG_HDMITX_PKT_AVI_LEN; i++ )
            {
                u32SumVal += MHal_HDMITx_ReadByte(HDMITX_DTOP_BASE, HDMITX_AviPktReg[i]);
            }
        break;

        case E_HDMITX_SPD_INFOFRAME:
            u32SumVal += (E_HDMITX_SPD_INFOFRAME + HDMITX_SPD_INFO_PKT_VER + HDMITX_SPD_INFO_PKT_LEN);
            for( i = 0; i < HDMITX_SPD_INFO_PKT_LEN; i++)
            {
                u32SumVal += MHal_HDMITx_ReadByte(HDMITX_DTOP_BASE, HDMITX_SpdPktReg[i]);
            }
        break;

        case E_HDMITX_AUDIO_INFOFRAME:
            u32SumVal += (E_HDMITX_AUDIO_INFOFRAME + HDMITX_AUD_INFO_PKT_VER + HDMITX_AUD_INFO_PKT_LEN);
            for(i=0; i<REG_HDMITX_PKT_ADO_LEN; i++)
            {
                u32SumVal += MHal_HDMITx_ReadByte(HDMITX_DTOP_BASE, HDMITX_AdoPktReg[i]);
            }
        break;

        case E_HDMITX_MPEG_INFOFRAME:
            //TBD
        break;

        case E_HDMITX_HDR_INFOFRAME:
        break;

        default:
        break;
    }

    u32SumVal = 0x100 - (u32SumVal & 0xFF);

    return (MS_U8)(u32SumVal);

}



void _MHal_HDMITx_DumpRegTab(MS_U8 *pdata, MS_U16 u16regnum, MS_U16 u16datasize, MS_U8 u8dataoffset)
{
    MS_U16 i, j;

    for(i=0; i< u16regnum; i++)
    {
        MS_U32 base;
        MS_U16 addr, mask, value;

        j = i *  (REG_ADDR_SIZE+REG_MASK_SIZE+u16datasize);

        base = ((MS_U32)pdata[j]) << 16 | ((MS_U32)pdata[j+1]) << 8;
        addr = pdata[j+2] >> 1;

        if(pdata[j+2] & 0x01)
        {
            mask  = (MS_U16)(pdata[j+3]) << 8;
            value = (MS_U16)(pdata[j+4+u8dataoffset]) << 8;
        }
        else
        {
            mask  = pdata[j+3];
            value = pdata[j+4+u8dataoffset];
        }

        //printf("%06x, %04x %04x %04x \n", base, addr, mask, value);
        MHal_HDMITx_Mask_Write(base, addr, mask, value);
    }
}

void MHal_HDMITx_DumpAtopTable(MsHDMITX_VIDEO_TIMING video_idx, MsHDMITX_VIDEO_COLORDEPTH_VAL cd_val, MS_BOOL bIs420, MS_BOOL bCommon)
{
    MS_U16 u16TopId;

    switch(video_idx)
    {
        case E_HDMITX_RES_640x480p:
        case E_HDMITX_RES_720x480i:
        case E_HDMITX_RES_720x576i:
            u16TopId =  HAL_HDMITX_ID_NUM;
            break;

        case E_HDMITX_RES_720x480p:
            u16TopId =  HAL_HDMITX_ID_480_60P;
            break;

        case E_HDMITX_RES_720x576p:
            u16TopId =  HAL_HDMITX_ID_576_50P;
            break;

        case E_HDMITX_RES_1280x720p_50Hz:
            u16TopId =  HAL_HDMITX_ID_720_50P;
            break;

        case E_HDMITX_RES_1280x720p_60Hz:
            u16TopId =  HAL_HDMITX_ID_720_60P;
            break;

        case E_HDMITX_RES_1280x720p_25Hz:
        case E_HDMITX_RES_1280x720p_30Hz:
        case E_HDMITX_RES_1280x720p_24Hz:
            u16TopId =  HAL_HDMITX_ID_NUM;
            break;

        case E_HDMITX_RES_1920x1080i_50Hz:
        case E_HDMITX_RES_1920x1080i_60Hz:
            u16TopId =  HAL_HDMITX_ID_NUM;
            break;

        case E_HDMITX_RES_1920x1080p_24Hz:
            u16TopId =  HAL_HDMITX_ID_1080_24P;
            break;

        case E_HDMITX_RES_1920x1080p_25Hz:
            u16TopId =  HAL_HDMITX_ID_1080_25P;
            break;

        case E_HDMITX_RES_1920x1080p_30Hz:
            u16TopId =  HAL_HDMITX_ID_1080_30P;
            break;

        case E_HDMITX_RES_1920x2205p_24Hz:
        case E_HDMITX_RES_1280x1470p_50Hz:
        case E_HDMITX_RES_1280x1470p_60Hz:
            u16TopId =  HAL_HDMITX_ID_NUM;
            break;


        case E_HDMITX_RES_1920x1080p_50Hz:
            u16TopId =  HAL_HDMITX_ID_1080_50P;
            break;

        case E_HDMITX_RES_1920x1080p_60Hz:
            u16TopId =  HAL_HDMITX_ID_1080_60P;
            break;

        case E_HDMITX_RES_3840x2160p_30Hz:
            u16TopId =  HAL_HDMITX_ID_3840x2160_30P;
            break;

        case E_HDMITX_RES_3840x2160p_24Hz:
        case E_HDMITX_RES_3840x2160p_25Hz:
        case E_HDMITX_RES_4096x2160p_24Hz:
        case E_HDMITX_RES_4096x2160p_25Hz:
        case E_HDMITX_RES_4096x2160p_30Hz:
        case E_HDMITX_RES_3840x2160p_50Hz:
        case E_HDMITX_RES_3840x2160p_60Hz:
        case E_HDMITX_RES_4096x2160p_50Hz:
        case E_HDMITX_RES_4096x2160p_60Hz:
            u16TopId =  HAL_HDMITX_ID_NUM;
            break;

        case E_HDMITX_RES_1600x1200p_60Hz:
            u16TopId =  HAL_HDMITX_ID_1600x1200_60P;
            break;

        case E_HDMITX_RES_1440x900p_60Hz:
            u16TopId =  HAL_HDMITX_ID_1440x900_60P;
            break;

        case E_HDMITX_RES_1280x1024p_60Hz:
            u16TopId =  HAL_HDMITX_ID_1280x1024_60P;
            break;

        case E_HDMITX_RES_1024x768p_60Hz:
            u16TopId =  HAL_HDMITX_ID_1024x768_60P;
            break;

        case E_HDMITX_RES_1366x768p_60Hz:
            u16TopId = HAL_HDMITX_ID_1366x768_60P;
            break;

        case E_HDMITX_RES_1280x800p_60Hz:
            u16TopId = HAL_HDMITX_ID_1280x800_60P;
            break;

        case E_HDMITX_RES_1680x1050p_60Hz:
            u16TopId = HAL_HDMITX_ID_1680x1050_60P;
            break;

        default:
            u16TopId =  HAL_HDMITX_ID_NUM;
            break;
    }

    if(u16TopId == HAL_HDMITX_ID_NUM)
    {
        printf("%s %d, Atop id fail, video=%d, cd=%d \n", __FUNCTION__, __LINE__, video_idx, cd_val);
    }
    else
    {
        HAL_HDMITX_INFO   *ptxatopinfo = NULL;
        MS_U16 i;
        MS_BOOL bWrite;

        DBG_HDMITX(printf("%s %d, Atop=%s\n", __FUNCTION__, __LINE__, PARSING_ATOP_IDX(u16TopId)));
        for(i=0; i<HAL_HDMITX_TAB_NUM; i++)
        {
            ptxatopinfo = &stHAL_HDMITX_TOP_TBL[i];

            if(bCommon)
            {
                bWrite = ptxatopinfo->enIPType == HAL_HDMITX_IP_COMMON ? TRUE : FALSE;
            }
            else
            {
                bWrite = ptxatopinfo->enIPType == HAL_HDMITX_IP_NORMAL ? TRUE : FALSE;
            }


            if(bWrite)
            {
                if(ptxatopinfo->enIPType == HAL_HDMITX_IP_COMMON)
                {
                    _MHal_HDMITx_DumpRegTab(ptxatopinfo->pData, ptxatopinfo->u16RegNum, REG_DATA_SIZE, 0);
                }
                else
                {
                    _MHal_HDMITx_DumpRegTab(ptxatopinfo->pData, ptxatopinfo->u16RegNum, HAL_HDMITX_ID_NUM, u16TopId);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
/// @brief This routine sets Analog Drving current
/// @param[in] idx: drving current setting
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetAnalogDrvCur(MsHDMITX_ANALOG_TUNING *pInfo)
{
    if(pInfo->u8DrvCurTap1Ch0 || pInfo->u8DrvCurTap1Ch1 || pInfo->u8DrvCurTap1Ch2 || pInfo->u8DrvCurTap1Ch3 ||
       pInfo->u8DrvCurTap2Ch0 || pInfo->u8DrvCurTap2Ch1 || pInfo->u8DrvCurTap2Ch2 || pInfo->u8DrvCurTap2Ch3)
    {
        DBG_HDMITX(printf("%s %d, DrvCur Tap1(%x %x %x %x) Tap2(%x %x %x %x)",
            __FUNCTION__, __LINE__,
            pInfo->u8DrvCurTap1Ch0, pInfo->u8DrvCurTap1Ch1, pInfo->u8DrvCurTap1Ch2, pInfo->u8DrvCurTap1Ch3,
            pInfo->u8DrvCurTap2Ch0, pInfo->u8DrvCurTap1Ch1, pInfo->u8DrvCurTap2Ch2, pInfo->u8DrvCurTap2Ch3,));

        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_22, 0x3F3F, ((MS_U16)pInfo->u8DrvCurTap1Ch1 << 8) |  (MS_U16)pInfo->u8DrvCurTap1Ch0);
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_23, 0x3F3F, ((MS_U16)pInfo->u8DrvCurTap1Ch3 << 8) |  (MS_U16)pInfo->u8DrvCurTap1Ch2);
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_24, 0x3F3F, ((MS_U16)pInfo->u8DrvCurTap2Ch1 << 8) |  (MS_U16)pInfo->u8DrvCurTap2Ch0);
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_25, 0x3F3F, ((MS_U16)pInfo->u8DrvCurTap2Ch3 << 8) |  (MS_U16)pInfo->u8DrvCurTap2Ch2);
    }
}

//------------------------------------------------------------------------------
/// @brief This routine sets video output mode (color/repetition/regen)
/// @param[in] idx: gHDMITxInfo.output_video_timing
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetVideoOutputMode(MsHDMITX_VIDEO_TIMING idx, MS_BOOL bflag, MsHDMITX_VIDEO_COLORDEPTH_VAL cd_val, MsHDMITX_ANALOG_TUNING *pInfo, MS_U8 ubSSCEn)
{
    //MS_U16 reg_value=0;
    MS_BOOL bIsHDMI20 = FALSE;
    MS_BOOL bIs420Fmt = FALSE;
    MS_U32 uiTMDSCLK = 0;

    DBG_HDMITX(printf("video idx = 0x%X, color depth = 0x%X, timing:%d\r\n", idx, cd_val, idx));

    if ((gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME &(~0x80)].PktPara.AVIInfoPktPara.enColorFmt != E_HDMITX_VIDEO_COLOR_YUV420) &&
        (
            (idx == E_HDMITX_RES_3840x2160p_60Hz) || (idx == E_HDMITX_RES_3840x2160p_50Hz) || \
            (idx == E_HDMITX_RES_4096x2160p_50Hz) || (idx == E_HDMITX_RES_4096x2160p_60Hz) || \
            (
                (
                    (idx == E_HDMITX_RES_1280x1470p_60Hz) || \
                    (idx == E_HDMITX_RES_3840x2160p_24Hz) || \
                    (idx == E_HDMITX_RES_3840x2160p_25Hz) || \
                    (idx == E_HDMITX_RES_3840x2160p_30Hz) || \
                    (idx == E_HDMITX_RES_4096x2160p_24Hz) || \
                    (idx == E_HDMITX_RES_4096x2160p_25Hz) || \
                    (idx == E_HDMITX_RES_4096x2160p_30Hz)

                ) &&
                (
                    (cd_val != E_HDMITX_VIDEO_CD_24Bits) && (cd_val != E_HDMITX_VIDEO_CD_NoID)
                )
            )
         ))
    {
        bIsHDMI20 = TRUE;
    }

    if ((gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME &(~0x80)].PktPara.AVIInfoPktPara.enColorFmt == E_HDMITX_VIDEO_COLOR_YUV420) &&
        ((idx == E_HDMITX_RES_3840x2160p_60Hz) || (idx == E_HDMITX_RES_3840x2160p_50Hz) || (idx == E_HDMITX_RES_4096x2160p_50Hz) || (idx == E_HDMITX_RES_4096x2160p_60Hz)) &&
        (cd_val != E_HDMITX_VIDEO_CD_24Bits) && (cd_val != E_HDMITX_VIDEO_CD_NoID))
    {
        bIsHDMI20 = TRUE;
    }

    uiTMDSCLK = MHal_HDMITx_GetPixelClk_ByTiming(idx, gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME &(~0x80)].PktPara.AVIInfoPktPara.enColorFmt, cd_val);
    uiTMDSCLK /= 1000;

    if( (uiTMDSCLK >= 148) && (uiTMDSCLK <= 165) )
        ub148to165MHz = 1;
    else
        ub148to165MHz = 0;

    if (gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME &(~0x80)].PktPara.AVIInfoPktPara.enColorFmt == E_HDMITX_VIDEO_COLOR_YUV420)
        bIs420Fmt = TRUE;

    MHal_HDMITx_EnableSSC(ubSSCEn, uiTMDSCLK, idx, cd_val, bIs420Fmt);


    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_SYNC_OUT_POL, REG_HDMITX_VSYNC_OUT_POL_MSK,
        HDMITxVideoModeTbl[idx].v_polarity == E_HDMITX_VIDEO_POLARITY_LOW ?
        REG_HDMITX_VSYNC_OUT_POL_INVERT : REG_HDMITX_VSYNC_OUT_POL_NORMRAL);

    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_SYNC_OUT_POL, REG_HDMITX_HSYNC_OUT_POL_MSK,
        HDMITxVideoModeTbl[idx].h_polarity == E_HDMITX_VIDEO_POLARITY_LOW ?
        REG_HDMITX_HSYNC_OUT_POL_INVERT : REG_HDMITX_HSYNC_OUT_POL_NORMRAL);

    //deep color setting
    if(bIsHDMI20 == FALSE)
    {
        MHal_HDMITx_DumpAtopTable(idx, cd_val, bIs420Fmt, FALSE);
    }
    else
    {
        DBG_HDMITX(printf("%s %d, Hdmi20 not support\n", __FUNCTION__, __LINE__));
    }


    MHal_HDMITx_SetAnalogDrvCur(pInfo);
}

//------------------------------------------------------------------------------
/// @brief This routine will power on or off HDMITx clock (power saving)
/// @param[in] bEnable: TRUE/FALSE
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_Power_OnOff(MS_BOOL bEnable)
{
    if (bEnable)
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PD, REG_HDMITX_PD_MSK, REG_HDMITX_PD_EN);
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_33, 0x0001, 0x0000);
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_34, 0x0001, 0x0000);
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_35, 0x001F, 0x0000);
    }
    else
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PD, REG_HDMITX_PD_MSK, 0x0000);
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_33, 0x0001, 0x0001);
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_34, 0x0001, 0x0001);
        MHal_HDMITx_Mask_Write(HDMITX_ATOP_BASE, REG_HDMITX_ATOP_35, 0x001F, 0x001F);
    }
}


MS_BOOL MHal_HDMITx_RxBypass_Mode(MsHDMITX_INPUT_FREQ freq, MS_BOOL bflag)
{
    return FALSE;
}


/// @brief This routine will disable TMDS clock, data, and DDC... bypass mode
/// @return None
//------------------------------------------------------------------------------
MS_BOOL MHal_HDMITx_Disable_RxBypass(void)
{
    return FALSE;
}


//------------------------------------------------------------------------------
/// @brief This routine will set GPIO pin for HPD
/// @param[in] u8pin: GPIO0 ~ 12
/// @return None
//------------------------------------------------------------------------------
void MHal_HDMITx_SetHPDGpioPin(MS_U8 u8pin)
{
    DBG_HDMITX(printf("_gHPDGpioPin = 0x%x\r\n", u8pin));
    DrvHdmitxOsSetHpdGpinPin(u8pin);
    DrvHdmitxOsGpioRequestInput(u8pin);
}

//------------------------------------------------------------------------------
/// @brief This routine return CHIP capability of DVI mode
/// @return TRUE, FALSE
//------------------------------------------------------------------------------
MS_BOOL MHal_HDMITx_IsSupportDVIMode(void)
{
    return TRUE;
}

//------------------------------------------------------------------------------
/// @brief This routine return CHIP capability of hdcp mode
/// @return TRUE, FALSE
//------------------------------------------------------------------------------
MS_BOOL MHal_HDMITx_IsSupportHDCP(void)
{
    return FALSE;
}

// *************  For customer NDS **************//

void MHal_HDMITx_Set_AVI_InfoFrame(MsHDMITX_PACKET_PROCESS packet_process, MsHDMITX_AVI_CONTENT_TYPE content_type, MS_U16 *data)
{
    MS_U8 u8ChkSum = 0;
    MS_U8 *pu8Data = NULL;

    if (IS_STOP_PKT(packet_process))   // Stop sending packet
    {
        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_AVI_MSK, 0x0000);  // Stop AVI packet
    }
    else
    {
        pu8Data = (MS_U8 *)data;

        switch(content_type)
        {
            case E_HDMITX_AVI_PIXEL_FROMAT:
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_1, 0x0060, *pu8Data);
                break;
            case E_HDMITX_AVI_ASPECT_RATIO:
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_1, 0x1F, *pu8Data);
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_2, 0x3F, *(pu8Data+1));
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_3, 0x03, *(pu8Data+2));
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_7, 0xFF, *(pu8Data+6));
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_8, 0xFF, *(pu8Data+7));
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_9, 0xFF, *(pu8Data+8));
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_10, 0xFF, *(pu8Data+9));
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_11, 0xFF, *(pu8Data+10));
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_12, 0xFF, *(pu8Data+11));
                MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_13, 0xFF, *(pu8Data+12));
                break;
            case E_HDMITX_AVI_COLORIMETRY:
                MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_2, 0xC0, *pu8Data);
                MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_3, 0x30, *(pu8Data+1));
                break;
            default:
                break;
        }

        u8ChkSum = MHal_HDMITx_InfoFrameCheckSum(E_HDMITX_AVI_INFOFRAME);
        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_0, 0xFF, u8ChkSum);
        if (IS_CYCLIC_PKT(packet_process))
        { // cyclic packet
            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_AVI_MSK, REG_HDMITX_PKT_AVI_EN);
        }
        else
        { // single packet
            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_AVI_MSK, REG_HDMITX_PKT_AVI_EN);
        }
    }
}

//wilson@kano
//**************************************************************************
//  [Function Name]:
//                      MHal_HDMITx_SetGCPParameter()
//  [Description]:
//                      setting General Control packet attribute
//  [Arguments]:
//                      [stGC_PktPara] stPktPara
//  [Return]:
//                      void
//
//**************************************************************************
void MHal_HDMITx_SetGCPParameter(stGC_PktPara stPktPara)
{
    gbGeneralPktList[E_HDMITX_GC_PACKET].PktPara.GCPktPara.enAVMute = stPktPara.enAVMute;
    gbGeneralPktList[E_HDMITX_GC_PACKET].PktPara.GCPktPara.enColorDepInfo = stPktPara.enColorDepInfo;
}

//**************************************************************************
//  [Function Name]:
//                      MHal_HDMITx_SetPktAttribute()
//  [Description]:
//                      configure settings to corresponding packet
//  [Arguments]:
//                      [MsHDMITX_PACKET_TYPE] enPktType
//                      [MS_BOOL] bEnUserDef
//                      [MS_U8] u8FrmCntNum
//                      [MsHDMITX_PACKET_PROCESS] enPktCtrl
//  [Return]:
//                      void
//
//**************************************************************************
void MHal_HDMITx_SetPktAttribute(MsHDMITX_PACKET_TYPE enPktType, MS_BOOL bEnUserDef, MS_U8 u8FrmCntNum, MsHDMITX_PACKET_PROCESS enPktCtrl)
{
    if (enPktType & 0x80) //infoframe packet type
    {
        gbInfoFrmPktList[enPktType & (~0x80)].EnableUserDef = bEnUserDef;
        gbInfoFrmPktList[enPktType & (~0x80)].FrmCntNum = u8FrmCntNum;
        gbInfoFrmPktList[enPktType & (~0x80)].enPktCtrl = enPktCtrl;
    }
    else
    {
        gbGeneralPktList[enPktType].EnableUserDef = bEnUserDef;
        gbGeneralPktList[enPktType].FrmCntNum = u8FrmCntNum;
        gbGeneralPktList[enPktType].enPktCtrl = enPktCtrl;
    }
}

//**************************************************************************
//  [Function Name]:
//                      MHal_HDMITx_SetAVIInfoParameter()
//  [Description]:
//                      Assign content to AVI Infoframe packet
//  [Arguments]:
//                      [stAVIInfo_PktPara] stPktPara
//  [Return]:
//                      void
//
//**************************************************************************
void MHal_HDMITx_SetAVIInfoParameter(stAVIInfo_PktPara stPktPara)
{
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.A0Value = stPktPara.A0Value;
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.enableAFDoverWrite = stPktPara.enableAFDoverWrite;
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.enColorFmt = stPktPara.enColorFmt;
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.enColorimetry = stPktPara.enColorimetry;
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.enExtColorimetry = stPktPara.enExtColorimetry;
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.enYCCQuantRange = stPktPara.enYCCQuantRange;
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.enRGBQuantRange = stPktPara.enRGBQuantRange;
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.enVidTiming = stPktPara.enVidTiming;
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.enAFDRatio = stPktPara.enAFDRatio;
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.enScanInfo = stPktPara.enScanInfo;
    gbInfoFrmPktList[E_HDMITX_AVI_INFOFRAME & (~0x80)].PktPara.AVIInfoPktPara.enAspectRatio = stPktPara.enAspectRatio;
}

//**************************************************************************
//  [Function Name]:
//                      MHal_HDMITx_SetVSInfoParameter()
//  [Description]:
//                      Assign content to VendorSpecific Infoframe packet
//  [Arguments]:
//                      [stVSInfo_PktPara] stPketPara
//  [Return]:
//                      void
//
//**************************************************************************
void MHal_HDMITx_SetVSInfoParameter(stVSInfo_PktPara stPktPara)
{
    gbInfoFrmPktList[E_HDMITX_VS_INFOFRAME & (~0x80)].PktPara.VSInfoPktPara.en3DStruct = stPktPara.en3DStruct;
    gbInfoFrmPktList[E_HDMITX_VS_INFOFRAME & (~0x80)].PktPara.VSInfoPktPara.en4k2kVIC = stPktPara.en4k2kVIC;
    gbInfoFrmPktList[E_HDMITX_VS_INFOFRAME & (~0x80)].PktPara.VSInfoPktPara.enVSFmt = stPktPara.enVSFmt;
}

//**************************************************************************
//  [Function Name]:
//                      MHal_HDMITx_SetVSInfoParameter()
//  [Description]:
//                      Assign content to VendorSpecific Infoframe packet
//  [Arguments]:
//                      [stVSInfo_PktPara] stPketPara
//  [Return]:
//                      void
//
//**************************************************************************
void MHal_HDMITx_SetAudioInfoParameter(stAUDInfo_PktPara stPktPara)
{
    gbInfoFrmPktList[E_HDMITX_AUDIO_INFOFRAME & (~0x80)].PktPara.AUDInfoPktPara.enAudChCnt = stPktPara.enAudChCnt;
    gbInfoFrmPktList[E_HDMITX_AUDIO_INFOFRAME & (~0x80)].PktPara.AUDInfoPktPara.enAudType = stPktPara.enAudType;
    gbInfoFrmPktList[E_HDMITX_AUDIO_INFOFRAME & (~0x80)].PktPara.AUDInfoPktPara.enAudFreq = stPktPara.enAudFreq;
}

//**************************************************************************
//  [Function Name]:
//                      MHal_HDMITx_SendPacket()
//  [Description]:
//                      configure  packet content and process according to user define or defalut setting
//  [Arguments]:
//                      [MsHDMITX_PACKET_TYPE] enPktType
//  [Return]:
//                      void
//
//**************************************************************************
void MHal_HDMITx_SendPacket(MsHDMITX_PACKET_TYPE enPktType, MsHDMITX_PACKET_PROCESS packet_process)
{
    if (enPktType & 0x80) //info frame packet
    {
        MS_U8 ucInfoPktType = enPktType & (~0x80);
        MS_U8 u8ChkSum;

        gbInfoFrmPktList[ucInfoPktType].enPktCtrl = packet_process;

        switch (enPktType)
        {
            case E_HDMITX_VS_INFOFRAME:
                if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_STOP_PACKET)
                {
                    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_VS_MSK, 0x0000);
                    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_PKT_RDY_SEL, REG_PKT1_RDY_MSK, 0x0000);
                }
                else
                {
                    if (gbInfoFrmPktList[ucInfoPktType].EnableUserDef == TRUE)
                    {
                        u8ChkSum = MHal_HDMITx_InfoFrameCheckSum(E_HDMITX_VS_INFOFRAME);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_6, 0xFF, u8ChkSum);
                        if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_SEND_PACKET) //single
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_VS_MSK, REG_HDMITX_PKT_VS_EN);
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_PKT_RDY_SEL, REG_PKT1_RDY_MSK, REG_PKT1_RDY_EN);
                        }
                        else
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_VS_MSK, REG_HDMITX_PKT_VS_EN);
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_PKT_RDY_SEL, REG_PKT1_RDY_MSK, REG_PKT1_RDY_EN);
                        }
                    }
                    else
                    {
                        //fill IEEE HDMI tag
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_HB0, 0xFF, E_HDMITX_VS_INFOFRAME);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_HB1, 0xFF, 0x01);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_HB2, 0xFF, HDMITX_VS_INFO_PKT_LEN);

                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_1, 0xFF, 0x03);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_2, 0xFF, 0x0C);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_3, 0xFF, 0x00); // 24-bit IEEE Registration ID

                        //check content
                        if (gbInfoFrmPktList[ucInfoPktType].PktPara.VSInfoPktPara.enVSFmt == E_HDMITX_VIDEO_VS_3D)
                        {
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_4, 0xE0, E_HDMITX_VIDEO_VS_3D);  // video format
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_6, 0xFF, (gbInfoFrmPktList[ucInfoPktType].PktPara.VSInfoPktPara.en3DStruct) << 4);  // 3D structure
                        }
                        else if (gbInfoFrmPktList[ucInfoPktType].PktPara.VSInfoPktPara.enVSFmt == E_HDMITX_VIDEO_VS_4k_2k)
                        {
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_4, 0xE0, E_HDMITX_VIDEO_VS_4k_2k);  // video format
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_6, 0xFF, (gbInfoFrmPktList[ucInfoPktType].PktPara.VSInfoPktPara.en4k2kVIC));	// 4k2k vic
                        }
                        else
                        {
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_4, 0xE0, 0x00);  // video format
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_6, 0xFF, 0x00);
                        }

                        u8ChkSum = MHal_HDMITx_InfoFrameCheckSum(E_HDMITX_VS_INFOFRAME);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_VS_6, 0xFF, u8ChkSum);
                        if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_SEND_PACKET) //single
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_VS_MSK, REG_HDMITX_PKT_VS_EN);
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_PKT_RDY_SEL, REG_PKT1_RDY_MSK, REG_PKT1_RDY_EN);
                        }
                        else
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_VS_MSK, REG_HDMITX_PKT_VS_EN);
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_PKT_RDY_SEL, REG_PKT1_RDY_MSK, REG_PKT1_RDY_EN);
                        }
                    }
                }
            break;

            case E_HDMITX_AVI_INFOFRAME:
                if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_STOP_PACKET)
                {
                    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_AVI_MSK, 0x0000);
                }
                else
                {
                    if (gbInfoFrmPktList[ucInfoPktType].EnableUserDef == TRUE)
                    {
                        u8ChkSum = MHal_HDMITx_InfoFrameCheckSum(E_HDMITX_AVI_INFOFRAME);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_0, 0x0FF, u8ChkSum);

                        if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_SEND_PACKET) //single
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_AVI_MSK, REG_HDMITX_PKT_AVI_EN);
                        }
                        else
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_AVI_MSK, REG_HDMITX_PKT_AVI_EN);
                        }
                    }
                    else
                    {
                        MS_U16 u16PktVal = 0;

                        //Y2, Y1, Y0: RGB, YCbCr 422, 444, 420
                        u16PktVal = (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enColorFmt << 5);// | 0x10;
                        //A0 field
                        u16PktVal |= ((gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.A0Value == 0x01) ? 0x10 : 0x00);
                        //S1, S0 field
                        u16PktVal |= (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enScanInfo);

                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_1, 0xFF, (MS_U8)u16PktVal);

                        //C1, C0, M1, M0
                        if ((gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming >= E_HDMITX_RES_720x480i) &&
                            (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming <= E_HDMITX_RES_720x576p))
                        {
                            u16PktVal = HDMITX_AviCmrTbl[gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming];
                            u16PktVal |= (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enAspectRatio << 4);
                        }
                        else
                        {
                            //HD timing is always 16:9
                            u16PktVal = HDMITX_AviCmrTbl[gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming];
                        }

                        //R3, R2, R1, R0: active porting aspect ration
                        if (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enableAFDoverWrite == TRUE)
                        {
                            u16PktVal |= (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enAFDRatio & 0x0F);
                        }

                        //u16PktVal |= ((MS_U8)gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enExtColorimetry == 0) ? 0 : 0xC0; //set [C1, C0] = [1, 1]

                        if(gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enColorimetry != E_HDMITX_COLORIMETRY_MAX)
                        {
                            u16PktVal = (u16PktVal & 0x3F) | ( ((MS_U8)gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enColorimetry & 0x03) << 6);
                        }

                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_2, 0xFF, (MS_U8)u16PktVal); //MDrv_WriteByte(REG_HDMITX_09_H, u16PktVal);

                        //EC0, EC1, EC2
                        u16PktVal = (MS_U8)gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enExtColorimetry;
                        u16PktVal = (u16PktVal > 6) ? 6 : u16PktVal; //BT2020 RGB & BT2020 YCbCr share same value 6; 7 is reserved;
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_3, 0x0070, (MS_U8)u16PktVal << 4); //MDrv_WriteByteMask(REG_HDMITX_0A_L, u16PktVal << 4, 0x70);

                        if(gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enColorFmt == E_HDMITX_VIDEO_COLOR_RGB444)
                        {
                            //Q1, Q0
                            if(gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enRGBQuantRange == E_HDMITX_RGB_QUANT_LIMIT)
                            {
                                u16PktVal = 1;
                            }
                            else if(gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enRGBQuantRange == E_HDMITX_RGB_QUANT_FULL)
                            {
                                u16PktVal = 2;
                            }
                            else
                            {
                                u16PktVal = 0;
                            }

                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_3, 0x0C, (MS_U8)u16PktVal << 2);
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_5, 0xC0, 0x00);
                        }
                        else
                        {
                            //YQ1, YQ0
                            if(gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enYCCQuantRange == E_HDMITX_YCC_QUANT_LIMIT)
                            {
                                u16PktVal = 0;
                            }
                            else if(gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enYCCQuantRange == E_HDMITX_YCC_QUANT_FULL)
                            {
                                u16PktVal = 1;
                            }
                            else
                            {
                                u16PktVal = 3;
                            }

                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_3, 0x0C, 0x00);
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_5, 0xC0, (MS_U8)u16PktVal << 6);
                        }

                        //VIC code: VIC code shoud +1 if aspect ration is 16:9
                        u16PktVal = HDMITX_AviVicTbl[gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming];

                        if (((gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming >= E_HDMITX_RES_720x480i) &&
                            (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming <= E_HDMITX_RES_720x576p)) &&
                            (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enAspectRatio == E_HDMITX_VIDEO_AR_16_9))
                        {
                            u16PktVal += 1;
                        }
                        else if (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enAspectRatio == E_HDMITX_VIDEO_AR_21_9)
                        {
                            MS_U8 AR21_9MappingTbl[14][2] = {
                                    {60, 65},
                                    {61, 66},
                                    {62, 67},
                                    {19, 68},
                                    { 4, 69},
                                    {41, 70},
                                    {47, 71},
                                    {32, 72},
                                    {33, 73},
                                    {34, 74},
                                    {31, 75},
                                    {16, 76},
                                    {64, 77},
                                    {63, 78}
                                };

                            if ((u16PktVal >= 93) && (u16PktVal <= 97))//3840*2160p@24 ~ 3840*2160@60
                            {
                                u16PktVal += 10;
                            }
                            else if ((gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming >= E_HDMITX_RES_3840x2160p_24Hz) &&\
                                (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming <= E_HDMITX_RES_3840x2160p_30Hz))
                            {
                                u16PktVal += (103 + (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming - E_HDMITX_RES_3840x2160p_24Hz));
                            }
                            else if ((u16PktVal > 78) && (u16PktVal <= 92))
                            {
                                //do nothing;
                            }
                            else
                            {
                                MS_U8 i = 0;
                                MS_BOOL bValidVIC = FALSE;

                                for ( i = 0; i < 14; i++ )
                                {
                                    if (AR21_9MappingTbl[i][0] == u16PktVal)
                                    {
                                        u16PktVal = AR21_9MappingTbl[i][1];
                                        bValidVIC = TRUE;
                                        break;
                                    }
                                }

                                if (!bValidVIC)
                                {
                                    printf("%s :: Invalid VIC Code for 21:9 Aspect Ratio!!!\r\n", __FUNCTION__);
                                }
                            }
                        }

                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_4, 0x7F, (MS_U8)u16PktVal); //MDrv_WriteByte(REG_HDMITX_0A_H, (u16PktVal & 0x7F));

                        //check repetition
                        if ((HDMITxVideoModeTbl[gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming].i_p_mode == E_HDMITX_VIDEO_INTERLACE_MODE) &&
                            (gbInfoFrmPktList[ucInfoPktType].PktPara.AVIInfoPktPara.enVidTiming <= E_HDMITX_RES_720x576i))
                        {
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_5, 0x0F, 0x01); //MDrv_WriteByteMask(REG_HDMITX_0B_L, 0x01, 0x0F);
                        }
                        else
                        {
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_5, 0x0F, 0x00); //MDrv_WriteByteMask(REG_HDMITX_0B_L, 0x00, 0x0F);
                        }

                        u8ChkSum = MHal_HDMITx_InfoFrameCheckSum(E_HDMITX_AVI_INFOFRAME);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_AVI_0, 0x0FF, u8ChkSum);

                        if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_SEND_PACKET) //single
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_AVI_MSK, REG_HDMITX_PKT_AVI_EN);
                        }
                        else
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_AVI_MSK, REG_HDMITX_PKT_AVI_EN);
                        }
                    }
                }
            break;

                case E_HDMITX_SPD_INFOFRAME:
                    if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_STOP_PACKET)
                    {
                        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_SPD_MSK, 0x0000); //MDrv_WriteByteMask(REG_HDMITX_22_L, 0x00, 0x05);
                    }
                    else
                    {
                        if (gbInfoFrmPktList[ucInfoPktType].EnableUserDef == TRUE)
                        {
                            u8ChkSum = MHal_HDMITx_InfoFrameCheckSum(E_HDMITX_SPD_INFOFRAME);
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_SPD_0, 0xFF, u8ChkSum);

                            if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_SEND_PACKET) //single
                            {
                                MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_SPD_MSK, REG_HDMITX_PKT_SPD_EN);
                                MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_PKT_RDY_SEL, REG_PKT0_RDY_MSK, REG_PKT0_RDY_EN);
                            }
                            else
                            {
                                MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_SPD_MSK, REG_HDMITX_PKT_SPD_EN);
                                MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_PKT_RDY_SEL, REG_PKT0_RDY_MSK, REG_PKT0_RDY_EN);
                            }
                        }
                        else
                        {
                            MS_U8 i = 0;

                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_SPD_HB0, 0xFF, E_HDMITX_SPD_INFOFRAME);
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_SPD_HB1, 0xFF, 0x01);
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_SPD_HB2, 0x00FF, HDMITX_SPD_INFO_PKT_LEN);

                            for(i=0; i<HDMITX_SPD_INFO_PKT_LEN; i++)
                            {
                                if(i == HDMITX_SPD_INFO_PKT_LEN-1)
                                {
                                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, HDMITX_SpdPktReg[i], 0xFF,HDMITX_PACKET_SPD_SDI);
                                }
                                else
                                {
                                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, HDMITX_SpdPktReg[i], 0xFF, HDMITX_SPDData[i]);
                                }

                            }

                            u8ChkSum = MHal_HDMITx_InfoFrameCheckSum(E_HDMITX_SPD_INFOFRAME);
                            MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_SPD_0, 0xFF, u8ChkSum);
                            if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_SEND_PACKET) //single
                            {
                                MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_SPD_MSK, REG_HDMITX_PKT_SPD_EN);
                                MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_PKT_RDY_SEL, REG_PKT0_RDY_MSK, REG_PKT0_RDY_EN);
                            }
                            else
                            {
                                MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_SPD_MSK, REG_HDMITX_PKT_SPD_EN);
                                MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_PKT_RDY_SEL, REG_PKT0_RDY_MSK, REG_PKT0_RDY_EN);
                            }
                        }
                }

            break;

            case E_HDMITX_AUDIO_INFOFRAME:
                if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_STOP_PACKET)
                {
                    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN,
                        REG_HDMITX_PKT_AUDIO_MSK | REG_HDMITX_PKT_ADO_MSK, 0x0000);
                }
                else
                {
                    // Modified for HDMI CTS test -
                    //     - Audio Coding Type (CT3~CT0) is 0x0 then continue else then FAIL
                    //     - Sampling Frequency (SF2~ SF0) is zero then continue else then FAIL.
                    //     - Sample Size (SS1~ SS0) is zero then continue else then FAIL.
                    //tmp_value = (gHDMITxInfo.output_audio_frequncy << 10) | 0x11;  // audio sampling frequency, PCM and 2 channel.
                    //MHal_HDMITx_Mask_Write(REG_HDMITX_BANK1, REG_PKT_AUD_1_11, 0x1FFF, tmp_value);

                    if (gbInfoFrmPktList[ucInfoPktType].PktPara.AUDInfoPktPara.enAudChCnt == E_HDMITX_AUDIO_CH_2) // 2-channel
                    {
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ADO_1, 0xFF, (E_HDMITX_AUDIO_CH_2 - 1) & 0x07); // 2 channels
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ADO_2, 0xFF, 0x00);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ADO_3, 0xFF, 0x00);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ADO_4, 0xFF, 0x00); // Channel allocation
                    }
                    else //8- channel
                    {
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ADO_1, 0xFF, (E_HDMITX_AUDIO_CH_8 - 1)&0x07); // 8 channels
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ADO_2, 0xFF, 0x00);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ADO_3, 0xFF, 0x00);
                        MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ADO_4, 0xFF, 0x1F); // Channel allocation
                    }
                    //clear LFEP value
                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ADO_5, 0x01, 0x00); //LFEP_BL1, LFEP_BL0

                    u8ChkSum = MHal_HDMITx_InfoFrameCheckSum(E_HDMITX_AUDIO_INFOFRAME);
                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ADO_0, 0xFF, u8ChkSum);

                    if (gbInfoFrmPktList[ucInfoPktType].enPktCtrl == E_HDMITX_SEND_PACKET) //single
                    {
                        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN,
                            REG_HDMITX_PKT_AUDIO_MSK | REG_HDMITX_PKT_ADO_MSK, REG_HDMITX_PKT_AUDIO_EN|REG_HDMITX_PKT_ADO_EN);
                    }
                    else
                    {
                        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN,
                            REG_HDMITX_PKT_AUDIO_MSK | REG_HDMITX_PKT_ADO_MSK, REG_HDMITX_PKT_AUDIO_EN|REG_HDMITX_PKT_ADO_EN);
                    }
                }

                if (gbInfoFrmPktList[ucInfoPktType].EnableUserDef == FALSE)
                {
                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_1, 0xFF, ((gbInfoFrmPktList[ucInfoPktType].PktPara.AUDInfoPktPara.enAudType == E_HDMITX_AUDIO_PCM) ? 0 : BIT1)); // [1]: PCM / non-PCM
                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_2, 0xFF, 0x00);
                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_3, 0xFF, (gbInfoFrmPktList[ucInfoPktType].PktPara.AUDInfoPktPara.enAudChCnt << 4)); // audio channel count
                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_4, 0xFF, (TxAudioFreqTbl[gbInfoFrmPktList[ucInfoPktType].PktPara.AUDInfoPktPara.enAudFreq].CH_Status3)); //audio sampling frequncy
                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_5, 0xFF, 0x00);
                    MHal_HDMITx_Mask_WriteByte(HDMITX_DTOP_BASE, REG_HDMITX_PKT_ASP_CS_6, 0xFF, 0x00);
                }
                // Audio sampling frequency
                // 1           1        0        0        32 kHz
                // 0           0        0        0        44.1 kHz
                // 0           0        0        1        88.2 kHz
                // 0           0        1        1        176.4 kHz
                // 0           1        0        0        48 kHz
                // 0           1        0        1        96 kHz
                // 0           1        1        1        192 kHz
                // 1           0        0        1        768 kHz
            break;

            case E_HDMITX_MPEG_INFOFRAME:
                //TBD
                break;

            case E_HDMITX_HDR_INFOFRAME:
                // TBD
                break;

            default:
                printf("hal_HDMITx_SendPacket():: Invalid Packet Type!!\r\n");
            break;
        }
    }
    else //general packet
    {
        gbGeneralPktList[enPktType].enPktCtrl = packet_process;

        switch (enPktType)
        {
            case E_HDMITX_NULL_PACKET:
                if (gbGeneralPktList[enPktType].enPktCtrl == E_HDMITX_STOP_PACKET)
                {
                    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_NULL_MSK, 0x0000);
                }
                else
                {
                    if (gbGeneralPktList[enPktType].enPktCtrl == E_HDMITX_SEND_PACKET) //single
                    {
                        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_NULL_MSK, REG_HDMITX_PKT_NULL_EN);
                    }
                    else
                    {
                        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_NULL_MSK, REG_HDMITX_PKT_NULL_EN);
                    }
                }
            break;

            case E_HDMITX_ACR_PACKET:
                if (gbGeneralPktList[enPktType].enPktCtrl == E_HDMITX_STOP_PACKET) //stop packet
                {
                    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_ACR_MSK, 0x0000);
                }
                else
                {
                    if (gbGeneralPktList[enPktType].enPktCtrl == E_HDMITX_SEND_PACKET) //single
                    {
                        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_ACR_MSK, REG_HDMITX_PKT_ACR_EN);
                    }
                    else //cyclic
                    {
                        if (gbGeneralPktList[enPktType].EnableUserDef == TRUE)
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_ACR_MSK, REG_HDMITX_PKT_ACR_EN);
                        }
                        else
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_ACR_MSK, REG_HDMITX_PKT_ACR_EN);
                        }
                    }
                }
            break;

            case E_HDMITX_AS_PACKET:
                if (gbGeneralPktList[enPktType].enPktCtrl == E_HDMITX_STOP_PACKET)
                {
                    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_AUDIO_MSK, 0x0000);
                }
                else
                {
                    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_AUDIO_MSK, REG_HDMITX_PKT_AUDIO_EN);
                }
            break;

            case E_HDMITX_GC_PACKET:
                if (gbGeneralPktList[enPktType].enPktCtrl == E_HDMITX_STOP_PACKET) //stop packet
                {
                    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_GC_MSK, 0x0000);
                }
                else
                {
                    if (gbGeneralPktList[enPktType].enPktCtrl == E_HDMITX_SEND_PACKET) //single
                    {
                        if(gbGeneralPktList[enPktType].PktPara.GCPktPara.enAVMute == E_HDMITX_GCP_SET_AVMUTE)
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AVMUTE, REG_HDMITX_AVMUTE_MSK, REG_HDMITX_AVMUTE_ON);
                        }
                        else
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AVMUTE, REG_HDMITX_AVMUTE_MSK, 0x0000);
                        }
                        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_GC_MSK, REG_HDMITX_PKT_GC_EN);
                    }
                    else //cyclic
                    {
                        if(gbGeneralPktList[enPktType].PktPara.GCPktPara.enAVMute == E_HDMITX_GCP_SET_AVMUTE)
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AVMUTE, REG_HDMITX_AVMUTE_MSK, REG_HDMITX_AVMUTE_ON);
                        }
                        else
                        {
                            MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_AVMUTE, REG_HDMITX_AVMUTE_MSK, 0x0000);
                        }
                        MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_PKT_EN, REG_HDMITX_PKT_GC_MSK, REG_HDMITX_PKT_GC_EN);
                    }
                }
            break;

            case E_HDMITX_ACP_PACKET:
                //TBD
            break;

            case E_HDMITX_ISRC1_PACKET:
                //TBD
            break;

            case E_HDMITX_ISRC2_PACKET:
                //TBD
            break;

            case E_HDMITX_DSD_PACKET:
                //TBD
            break;

            case E_HDMITX_HBR_PACKET:
                //TBD
            break;

            case E_HDMITX_GM_PACKET:
                //TBD
            break;

            default:
                printf("hal_HDMITx_SendPacket():: Invalid Packet Type!!\r\n");
            break;
        }
    }
}

void MHal_HDMITx_EnableSSC(MS_BOOL bEnable, MS_U32 uiTMDSCLK, MsHDMITX_VIDEO_TIMING idx, MsHDMITX_VIDEO_COLORDEPTH_VAL cd_val, MS_BOOL bIs420Fmt)
{
    #if 0
    #define HDMITX_MPLL_CLK                        432 //432MHz
    #define HDMITX_SSC_CLK                          30 //30KHz
    #define HDMITX_SSC_DEVIATION              1 // 0.1%
    #define HDMITX_SSC_DEVIATION_DIVIDER      1000
    #define HDMITX_SSC_THREAD_LEVEL1      150 ///1080p 8bits
    #define HDMITX_SSC_THREAD_LEVEL2      300 ///4K30 8bits
    #define HDMITX_SSC_SPAN_REG                REG_HDMITxPHY_CONFIG_06
    #define HDMITX_SSC_STEP_REG                 REG_HDMITxPHY_CONFIG_07
    #define HDMITX_SSC_SUB_DIVIDER_REG   REG_HDMITxPHY_CONFIG_01

    MS_U32 dPixel_Clk = 0;

    if(!bEnable)
    {
        MHal_HDMITx_Mask_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x3FFF, 0 );//Span
        MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0);//Step
        MHal_HDMITx_Mask_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SUB_DIVIDER_REG, 0x7000, 0 );//Sub-Divider
        return;
    }

    switch(idx)
    {
        case E_HDMITX_RES_1600x1200p_60Hz: // 27 (162MHz)
            if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x2A3);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x34B);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x2);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x3F4);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x1);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x2A3);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else
                printf("[HDMITx] SSC Color Depth Not Supported\n");
            break;
        case E_HDMITX_RES_1440x900p_60Hz: // 28 (106.47MHz)
            if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x1BB);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x9);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x22A);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x6);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x299);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x1BB);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x9);//Step
            }
            else
                printf("[HDMITx] SSC Color Depth Not Supported\n");
            break;
        case E_HDMITX_RES_1280x1024p_60Hz: // 29 (108MHz)
            if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x1C2);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x9);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x232);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x5);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x2A3);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x384);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x2);//Step
            }
            else
                printf("[HDMITx] SSC Color Depth Not Supported\n");
            break;
        case E_HDMITX_RES_1024x768p_60Hz: // 30 (65MHz)
            if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x21D);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x6);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x2A5);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x32C);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x2);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x21D);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x6);//Step
            }
            else
                printf("[HDMITx] SSC Color Depth Not Supported\n");
            break;
        case E_HDMITX_RES_1280x720p_24Hz: // 31 (59.4MHz)
        case E_HDMITX_RES_1280x720p_25Hz: // 32
        case E_HDMITX_RES_1280x720p_30Hz: // 33
            if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x1EF);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x7);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x26A);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x2E6);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x3);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x1EF);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x7);//Step
            }
            else
                printf("[HDMITx] SSC Color Depth Not Supported\n");
            break;
        case E_HDMITX_RES_720x480i: // 1 (27MHz)
        case E_HDMITX_RES_720x576i: // 2
        case E_HDMITX_RES_720x480p: // 3
        case E_HDMITX_RES_720x576p: // 4
            if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x1C2);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x9);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x232);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x5);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x2A3);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x1C2);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x9);//Step
            }
            else
                printf("[HDMITx] SSC Color Depth Not Supported\n");
            break;
        case E_HDMITX_RES_1920x1080i_60Hz: // 8 (74.25MHz/1.001)
            if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x26A);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x304);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x3);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x39F);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x2);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x26A);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else
                printf("[HDMITx] SSC Color Depth Not Supported\n");
            break;
        case E_HDMITX_RES_1280x720p_50Hz:  // 5 (74.25MHz)
        case E_HDMITX_RES_1280x720p_60Hz:  // 6
        case E_HDMITX_RES_1920x1080i_50Hz: // 7
        case E_HDMITX_RES_1920x1080p_24Hz: // 9
        case E_HDMITX_RES_1920x1080p_25Hz: // 10
        case E_HDMITX_RES_1920x1080p_30Hz: // 11
            if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x26A);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x305);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x3);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x3A0);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x2);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x26A);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else
                printf("[HDMITx] SSC Color Depth Not Supported\n");
            break;
        case E_HDMITX_RES_1920x1080p_50Hz: // 12 (148.5MHz)
        case E_HDMITX_RES_1920x1080p_60Hz: // 13
        case E_HDMITX_RES_1920x2205p_24Hz: // 14
        case E_HDMITX_RES_1280x1470p_50Hz: // 15
        case E_HDMITX_RES_1280x1470p_60Hz: // 16
            if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x26A);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x305);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x3);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x3A0);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x2);//Step
            }
            else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
            {
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x26A);//Span
                MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
            }
            else
                printf("[HDMITx] SSC Color Depth Not Supported\n");
            break;
        case E_HDMITX_RES_3840x2160p_24Hz: // 17 (297MHz)
        case E_HDMITX_RES_3840x2160p_25Hz: // 18
        case E_HDMITX_RES_3840x2160p_30Hz: // 19
        case E_HDMITX_RES_4096x2160p_24Hz: // 22
        case E_HDMITX_RES_4096x2160p_25Hz: // 23
        case E_HDMITX_RES_4096x2160p_30Hz: // 24
            if(bIs420Fmt)
            {
                if(cd_val==E_HDMITX_VIDEO_CD_24Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x26A);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
                }
                else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x305);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x3);//Step
                }
                else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x3A0);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x2);//Step
                }
                else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x26A);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x4);//Step
                }
                else
                    printf("[HDMITx] SSC Color Depth Not Supported\n");
                break;
            }
            else
            {
                if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x4D5);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x1);//Step
                }
                else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x305);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x3);//Step
                }
                else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x3A0);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x2);//Step
                }
                else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x4D5);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x1);//Step
                }
                else
                    printf("[HDMITx] SSC Color Depth Not Supported\n");
                break;
            }
        case E_HDMITX_RES_3840x2160p_50Hz: // 20 (594MHz)
        case E_HDMITX_RES_3840x2160p_60Hz: // 21
        case E_HDMITX_RES_4096x2160p_50Hz: // 25
        case E_HDMITX_RES_4096x2160p_60Hz: // 26
            if(bIs420Fmt)
            {
                if(cd_val==E_HDMITX_VIDEO_CD_24Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x4D5);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x1);//Step
                }
                else if(cd_val==E_HDMITX_VIDEO_CD_30Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x305);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x3);//Step
                }
                else if(cd_val==E_HDMITX_VIDEO_CD_36Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x3A0);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x2);//Step
                }
                else if(cd_val==E_HDMITX_VIDEO_CD_48Bits)
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x4D5);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x1);//Step
                }
                else
                    printf("[HDMITx] SSC Color Depth Not Supported\n");
                break;
            }
            else
            {
                if((cd_val==E_HDMITX_VIDEO_CD_24Bits) || (cd_val == E_HDMITX_VIDEO_CD_NoID))
                {
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SPAN_REG, 0x4D5);//Span
                    MHal_HDMITx_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_STEP_REG, 0x1);//Step
                }
                else
                    printf("[HDMITx] SSC Color Depth Not Supported\n");
                break;
            }
        default :
            printf("[HDMITx] SSC Timing Not Support\n");
            break;
    }

    //Read tmds clock
    #if 0
    MHal_HDMITx_Mask_Write(HDMITX_MISC_REG_BASE, REG_MISC_CONFIG_20, 0x3F, 0x3F);
    dPixel_Clk = ((MHal_HDMITx_Read(HDMITX_MISC_REG_BASE, REG_MISC_CONFIG_21) << 1) * 12 / 128);
    #else
    dPixel_Clk = uiTMDSCLK;
    #endif
    printf("dPixel_Clk=%d\r\n", dPixel_Clk);

    if(dPixel_Clk < HDMITX_SSC_THREAD_LEVEL1)
        MHal_HDMITx_Mask_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SUB_DIVIDER_REG, 0x7000, 0x0000 );//Sub-Divider
    else if(dPixel_Clk < HDMITX_SSC_THREAD_LEVEL2)
        MHal_HDMITx_Mask_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SUB_DIVIDER_REG, 0x7000, 0x1000 );//Sub-Divider
    else
        MHal_HDMITx_Mask_Write(HDMITX_PHY_REG_BASE, HDMITX_SSC_SUB_DIVIDER_REG, 0x7000, 0x3000 );//Sub-Divider

    #endif
}

MS_U32 MHal_HDMITx_GetPixelClk_ByTiming(MsHDMITX_VIDEO_TIMING idx, MsHDMITX_VIDEO_COLOR_FORMAT color_fmt, MsHDMITX_VIDEO_COLORDEPTH_VAL color_depth)
{
    MS_U32 dwTMDSDataRate = 0;
    MS_U8 ubBitNum = 8;

    switch(color_depth)
    {
        case E_HDMITX_VIDEO_CD_NoID:
        case E_HDMITX_VIDEO_CD_24Bits:
        {
            ubBitNum = 8;
        }
        break;

        case E_HDMITX_VIDEO_CD_30Bits:
        {
            ubBitNum = 10;
        }
        break;

        case E_HDMITX_VIDEO_CD_36Bits:
        {
            ubBitNum = 12;
        }
        break;

        case E_HDMITX_VIDEO_CD_48Bits:
        {
            ubBitNum = 16;
        }
        break;

        default:
        {
            ubBitNum = 8;
        }
        break;
    }

    dwTMDSDataRate = (HDMITxVideoModeTbl[idx].pixel_clk/1000) * ubBitNum / 8;

    if(color_fmt == E_HDMITX_VIDEO_COLOR_YUV420)
        dwTMDSDataRate = dwTMDSDataRate/2;

    //printf("Time_ID = %d, C_FMT = %d, C_DEP = %d, PXL_CLK = %d\r\n", idx, color_fmt, color_depth, dwTMDSDataRate);
    return dwTMDSDataRate;
}

MS_U32 MHal_HDMITx_GetMaxPixelClk(void)
{
    return HDMITX_MAX_PIXEL_CLK/1000;
}

MsHDMITX_VIDEO_COLOR_FORMAT MHal_HDMITx_GetAVIInfo_ColorFmt(void)
{
    MsHDMITX_VIDEO_COLOR_FORMAT enOutColorFmt = E_HDMITX_VIDEO_COLOR_RGB444;
    MS_U16 usAVI09 = 0;

    usAVI09 = MHal_HDMITx_Read(HDMITX_DTOP_BASE,  REG_HDMITX_PKT_AVI_1);
    usAVI09 &= 0x00E0;
    usAVI09 = usAVI09 >> 5;

    switch(usAVI09)
    {
        case 0:
        {
            enOutColorFmt = E_HDMITX_VIDEO_COLOR_RGB444;
        }
        break;

        case 1:
        {
            enOutColorFmt = E_HDMITX_VIDEO_COLOR_YUV422;
        }
        break;

        case 2:
        {
            enOutColorFmt = E_HDMITX_VIDEO_COLOR_YUV444;
        }
        break;

        case 3:
        {
            enOutColorFmt = E_HDMITX_VIDEO_COLOR_YUV420;
        }
        break;

        default:
            //printf("Wrong Color Format\r\n");
        break;
    }

    return enOutColorFmt;
}

MsHDMITX_VIDEO_COLORDEPTH_VAL MHal_HDMITx_GetGC_ColorDepth(void)
{
    return E_HDMITX_VIDEO_CD_24Bits;
}

void MHal_HDMITx_Init_PadTop(void)
{

}

MS_U32 MHal_HDMITX_GetClkNum(void)
{
    return HAL_HDMITX_CLK_NUM;
}

MS_BOOL MHal_HDMITX_GetDefaultClkRate(MS_U32 *pu32ClkRate, MS_U32 u32Size)
{
    MS_U32 au32ClkRate[HAL_HDMITX_CLK_NUM] = HAL_HDMITX_CLK_RATE_SETTING;
    MS_BOOL bRet;

    if(u32Size == HAL_HDMITX_CLK_NUM && pu32ClkRate)
    {
        memcpy(pu32ClkRate, au32ClkRate, sizeof(au32ClkRate));
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

MS_BOOL MHal_HDMITx_GetDefaultClkMuxAttr(MS_BOOL *pbClkMuxAttr, MS_U32 u32Size)
{
    MS_BOOL abClkMuxAttr[HAL_HDMITX_CLK_NUM] = HAL_HDMITX_CLK_MUX_ATTR;
    MS_BOOL bRet = TRUE;

    if(u32Size == HAL_HDMITX_CLK_NUM && pbClkMuxAttr)
    {
        memcpy(pbClkMuxAttr, abClkMuxAttr, sizeof(abClkMuxAttr));
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

MS_BOOL MHal_HDMITx_GetDefualtClkOnOff(MS_BOOL bEn, MS_BOOL *pbClkEn, MS_U32 u32Size)
{
    MS_BOOL abClkOn[HAL_HDMITX_CLK_NUM] = HAL_HDMITX_CLK_ON_SETTING;
    MS_BOOL abClkOff[HAL_HDMITX_CLK_NUM] = HAL_HDMITX_CLK_OFF_SETTING;
    MS_BOOL bRet = TRUE;

    if(u32Size == HAL_HDMITX_CLK_NUM && pbClkEn)
    {
        if(bEn)
        {
            memcpy(pbClkEn, abClkOn, sizeof(abClkOn));
        }
        else
        {
            memcpy(pbClkEn, abClkOff, sizeof(abClkOff));
        }
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

MS_BOOL MHal_HDMITx_SetClk(MS_BOOL *pbEn, MS_U32 *pu32ClkRate, MS_U32 u32Size)
{
    u16 u16RegVal;
    MS_BOOL bRet = TRUE;

    if(pbEn && pu32ClkRate && u32Size == HAL_HDMITX_CLK_NUM)
    {
        u16RegVal = pu32ClkRate[0] <= 0 ? 0x00 << 2 :
                    pu32ClkRate[0] <= 1 ? 0x01 << 2 :
                                          0x00;
        u16RegVal |= pbEn[0] ? 0x0000 : 0x0001;
        MHal_HDMITx_Mask_Write(HDMITX_SC_GP_CTRL_BASE, REG_HDMITX_CLK_HDMI, 0x000F, u16RegVal);


        u16RegVal = pu32ClkRate[1] <= CLK_MHZ(432) ? 0x00 << 2 :
                                                     0x00;
        u16RegVal |= pbEn[1] ? 0x0000 : 0x0001;
        MHal_HDMITx_Mask_Write(HDMITX_CLKGEN_BASE, REG_HDMITX_CLK_DISP_432, 0x000F, u16RegVal);


        u16RegVal = pu32ClkRate[2] <= CLK_MHZ(108) ? 0x01 << 10 :
                    pu32ClkRate[2] <= CLK_MHZ(216) ? 0x00 << 10 :
                                                     0x00;
        u16RegVal |= pbEn[2] ? 0x0000 : 0x0100;
        MHal_HDMITx_Mask_Write(HDMITX_CLKGEN_BASE, REG_HDMITX_CLK_DISP_216, 0x0F00, u16RegVal);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

MS_BOOL MHal_HDMITx_GetClk(MS_BOOL *pbEn, MS_U32 *pu32ClkRate, MS_U32 u32Size)
{
    u16 u16RegVal;
    MS_BOOL bRet = TRUE;

    if(pbEn && pu32ClkRate && u32Size == HAL_HDMITX_CLK_NUM)
    {
        u16RegVal = MHal_HDMITx_Read(HDMITX_SC_GP_CTRL_BASE, REG_HDMITX_CLK_HDMI);

        pbEn[0] = (u16RegVal & 0x0001) ? 0 : 1;

        u16RegVal = (u16RegVal >> 2) & 0x03;

        pu32ClkRate[0] = (u16RegVal == 0x00) ? 0 :
                         (u16RegVal == 0x01) ? 1 :
                                               999;

        u16RegVal = MHal_HDMITx_Read(HDMITX_CLKGEN_BASE, REG_HDMITX_CLK_DISP_432);

        pbEn[1] = (u16RegVal & 0x0001) ? 0 : 1;

        u16RegVal = (u16RegVal >> 2) & 0x03;

        pu32ClkRate[1] = (u16RegVal == 0x00) ? CLK_MHZ(432) : 999;


        u16RegVal = MHal_HDMITx_Read(HDMITX_CLKGEN_BASE, REG_HDMITX_CLK_DISP_216);

        pbEn[2] = (u16RegVal & 0x0100) ? 0 : 1;

        u16RegVal = (u16RegVal >> 10) & 0x03;

        pu32ClkRate[2] = (u16RegVal == 0x00) ? CLK_MHZ(216) :
                         (u16RegVal == 0x01) ? CLK_MHZ(108) :
                                               999;
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

void MHal_HDMITx_SwReset(MS_BOOL bEn)
{
    MHal_HDMITx_Mask_Write(HDMITX_DTOP_BASE, REG_HDMITX_SW_RST, REG_HDMITX_SW_RST_MSK, bEn ? REG_HDMITX_SW_RST_EN : 0);
}
