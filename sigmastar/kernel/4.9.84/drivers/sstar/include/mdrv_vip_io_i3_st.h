/*
* mdrv_vip_io_i3_st.h- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/

 /**
 * \ingroup vip_group
 * @{
 */

#ifndef _MDRV_VIP_IO_ST_H
#define _MDRV_VIP_IO_ST_H

//=============================================================================
// Defines
//=============================================================================
#define VIP_LCE_CURVE_SECTION_NUM       16          ///< VIP_LCE_CURVE_SECTION_NUM
#define VIP_PEAKING_BAND_NUM            8           ///< VIP_PEAKING_BAND_NUM
#define VIP_PEAKING_ADP_Y_LUT_NUM       8           ///< VIP_PEAKING_ADP_Y_LUT_NUM
#define VIP_PEAKING_BAND_TERM_NUM       16          ///< VIP_PEAKING_BAND_TERM_NUM
#define VIP_DLC_HISTOGRAM_SECTION_NUM   7           ///< VIP_DLC_HISTOGRAM_SECTION_NUM
#define VIP_DLC_HISTOGRAM_REPORT_NUM    8           ///< VIP_DLC_HISTOGRAM_REPORT_NUM
#define VIP_DLC_LUMA_SECTION_NUM        64          ///< VIP_DLC_LUMA_SECTION_NUM
#define VIP_IHC_COLOR_NUM               16          ///< VIP_IHC_COLOR_NUM
#define VIP_IHC_USER_COLOR_NUM          16          ///< VIP_IHC_USER_COLOR_NUM
#define VIP_ICE_COLOR_NUM               16          ///< VIP_ICE_COLOR_NUM
#define VIP_IBC_COLOR_NUM               16          ///< VIP_IBC_COLOR_NUM
#define VIP_FCC_YWIN_LUT_ENTRY_NUM      17          ///< VIP_FCC_YWIN_LUT_ENTRY_NUM
#define VIP_NLM_DISTWEIGHT_NUM          9           ///< VIP_NLM_DISTWEIGHT_NUM
#define VIP_NLM_WEIGHT_NUM              32          ///< VIP_NLM_WEIGHT_NUM
#define VIP_NLM_LUMAGAIN_NUM            64          ///< VIP_NLM_LUMAGAIN_NUM
#define VIP_NLM_POSTLUMA_NUM            16          ///< VIP_NLM_POSTLUMA_NUM
#define VIP_CMDQ_MEM_256K               0x0040000   ///< VIP_CMDQ_MEM_164K
#define VIP_CMDQ_MEM_196K               0x0030000   ///< VIP_CMDQ_MEM_164K
#define VIP_CMDQ_MEM_164K               0x0028000   ///< VIP_CMDQ_MEM_164K
#define VIP_CMDQ_MEM_128K               0x0020000   ///< VIP_CMDQ_MEM_128K
#define VIP_CMDQ_MEM_64K                0x0010000   ///< VIP_CMDQ_MEM_64K
#define VIP_CMDQ_MEM_32K                0x0008000   ///< VIP_CMDQ_MEM_32K
#define VIP_CMDQ_MEM_16K                0x0004000   ///< VIP_CMDQ_MEM_16K
#define VIP_CMDQ_MEM_TEST               0x0001000   ///< VIP_CMDQ_MEM_TEST
#define VIP_VTRACK_KEY_SETTING_LENGTH       8       ///< VIP_VTRACK_KEY_SETTING_LENGTH
#define VIP_VTRACK_SETTING_LENGTH           23      ///< VIP_VTRACK_SETTING_LENGTH

//
//1.0.1:for clean vip.
#define IOCTL_VIP_VERSION                   0x0101  ///< H:Major L:minor H3:Many Change H2:adjust Struct L1:add struct L0:adjust driver
//=============================================================================
// enum
//=============================================================================
/**
* Used to setup vsrc of vip device
*/
typedef enum
{
    E_VIP_SRC_ISP,  ///< VIP src ISP
    E_VIP_SRC_BT656,///< VIP src BT656
    E_VIP_SRC_DRAM, ///< VIP src DRAM
    E_VIP_SRC_NUM,  ///< VIP src max number
}__attribute__ ((__packed__))EN_VIP_SRC_TYPE;

/**
* Used to setup LCE AVE of vip device
*/
typedef enum
{
    EN_VIP_LCE_Y_AVE_5X11   = 0x0,      ///<  mask 0x10
    EN_VIP_LCE_Y_AVE_5X7    = 0x10,     ///<  mask 0x10
}__attribute__ ((__packed__))EN_VIP_LCE_Y_AVE_SEL_TYPE;


/**
* Used to setup UVC_ADP_Y_INPUT_SEL of vip device
*/
typedef enum
{
    E_VIP_UVC_ADP_Y_INPUT_SEL_UVC_LOCATE    = 0x0,      ///<  mask 0xC0
    E_VIP_UVC_ADP_Y_INPUT_SEL_RGB_Y_OUTPUT  = 0x40,     ///<  mask 0xC0
    E_VIP_UVC_ADP_Y_INPUT_SEL_DLC_Y_INPUT   = 0x80,     ///<  mask 0xC0
    E_VIP_UVC_ADP_Y_INPUT_SEL_RGB_Y_INPUT   = 0xC0,     ///<  mask 0xC0
}__attribute__ ((__packed__))EN_VIP_UVC_ADP_Y_INPUT_SEL_TYPE;

/**
* Used to setup LDC_BYPASS of vip device
*/
typedef enum
{
    E_VIP_LDC_MENULOAD,         ///< no bypass
    E_VIP_LDC_BYPASS,           ///< bypass
    E_VIP_LDC_BYPASS_TYPE_NUM,  ///< no use
}__attribute__ ((__packed__))EN_VIP_LDC_BYPASS_TYPE;
/**
* Used to setup LDCLCBANKMODE of vip device
*/
typedef enum
{
    EN_VIP_LDCLCBANKMODE_64, ///< 64p
    EN_VIP_LDCLCBANKMODE_128, ///< 128p
}__attribute__ ((__packed__))EN_VIP_LDCLCBANKMODE_TYPE;

/**
* Used to setup VIP_LDC_422_444 of vip device
*/
typedef enum
{
    E_VIP_LDC_422_444_DUPLICATE = 0x1,  ///< mask 0x3
    E_VIP_LDC_422_444_QUARTER   = 0x2,  ///< mask 0x3
    E_VIP_LDC_422_444_AVERAGE   = 0x3,  ///< mask 0x3
}__attribute__ ((__packed__))EN_VIP_LDC_422_444_TYPE;
/**
* Used to setup VIP_LDC_444_422 of vip device
*/
typedef enum
{
    E_VIP_LDC_444_422_DROP      = 0x0,  ///< mask 0x1C
    E_VIP_LDC_444_422_AVERAGE   = 0x4,  ///< mask 0x1C
}__attribute__ ((__packed__))EN_VIP_LDC_444_422_TYPE;
/**
* Used to VIP_NLM_Average of vip device
*/
typedef enum
{
    E_VIP_NLM_Average_3x3_mode = 0x0,   ///< mask 0x2
    E_VIP_NLM_Average_5x5_mode = 0x2,   ///< mask 0x2
}__attribute__ ((__packed__))EN_VIP_NLM_Average_TYPE;
/**
* Used to NLM_DSW of vip device
*/
typedef enum
{
    E_VIP_NLM_DSW_16x8_mode     = 0x0,  ///< mask 0x20
    E_VIP_NLM_DSW_32x16_mode    = 0x20, ///< mask 0x20
}__attribute__ ((__packed__))EN_VIP_NLM_DSW_TYPE;
/**
* Used to VIP_FCC_Y of vip device
*/
typedef enum
{
    E_VIP_FCC_Y_DIS_CR_DOWN,            ///< cr down
    E_VIP_FCC_Y_DIS_CR_UP,              ///< cr up
    E_VIP_FCC_Y_DIS_CB_DOWN,            ///< cb down
    E_VIP_FCC_Y_DIS_CB_UP,              ///< cb up
    E_VIP_FCC_Y_DIS_NUM,                ///< 4type
}__attribute__ ((__packed__))EN_VIP_FCC_Y_DIS_TYPE;
/**
* Used to setup IHC_ICE_ADP_Y of vip device
*/
typedef enum
{
    E_VIP_IHC_ICE_ADP_Y_SECTION_0,      ///< section
    E_VIP_IHC_ICE_ADP_Y_SECTION_1,      ///< section
    E_VIP_IHC_ICE_ADP_Y_SECTION_2,      ///< section
    E_VIP_IHC_ICE_ADP_Y_SECTION_3,      ///< section
    E_VIP_IHC_ICE_ADP_Y_SECTION_NUM,    ///< section
}__attribute__ ((__packed__))EN_VIP_IHC_ICE_ADP_Y_SECTION_TYPE;
/**
* Used to setup suspend of vip device
*/
typedef enum
{
    EN_VIP_ACK_CONFIG           = 0x1,      ///< ACK
    EN_VIP_IBC_CONFIG           = 0x2,      ///< IBC
    EN_VIP_IHCICC_CONFIG        = 0x4,      ///< ICCIHC
    EN_VIP_ICC_CONFIG           = 0x8,      ///< ICE
    EN_VIP_IHC_CONFIG           = 0x10,     ///< IHC
    EN_VIP_FCC_CONFIG           = 0x20,     ///< FCC
    EN_VIP_UVC_CONFIG           = 0x40,     ///< UVC
    EN_VIP_DLC_HISTOGRAM_CONFIG = 0x80,    ///< HIST
    EN_VIP_DLC_CONFIG           = 0x100,     ///< DLC
    EN_VIP_LCE_CONFIG           = 0x200,    ///< LCE
    EN_VIP_PEAKING_CONFIG       = 0x400,    ///< PK
    EN_VIP_NLM_CONFIG           = 0x800,    ///< NLM
    EN_VIP_LDC_MD_CONFIG        = 0x1000,   ///< LDCMD
    EN_VIP_LDC_DMAP_CONFIG      = 0x2000,   ///< LDCDMAP
    EN_VIP_LDC_SRAM_CONFIG      = 0x4000,   ///< LDC SRAM
    EN_VIP_LDC_CONFIG           = 0x8000,   ///< LDC
    EN_VIP_CONFIG               = 0x40000,  ///< 19 bit to control 19 IOCTL
}__attribute__ ((__packed__))EN_VIP_CONFIG_TYPE;
/**
* Used to setup the vtrack status of vip device
*/
typedef enum
{
    EN_VIP_IOCTL_VTRACK_ENABLE_ON,      ///< Vtrack on
    EN_VIP_IOCTL_VTRACK_ENABLE_OFF,     ///< Vtrack off
    EN_VIP_IOCTL_VTRACK_ENABLE_DEBUG,   ///< Vtrack debug
}EN_VIP_IOCTL_VTRACK_ENABLE_TYPE;
/**
* Used to setup the AIP  of vip device
*/
typedef enum
{
    EN_VIP_IOCTL_AIP_YEE = 0,           ///< yee
    EN_VIP_IOCTL_AIP_YEE_AC_LUT,        ///< yee ac lut
    EN_VIP_IOCTL_AIP_WDR_GLOB,          ///< wdr glob
    EN_VIP_IOCTL_AIP_WDR_LOC,           ///< wdr loc
    EN_VIP_IOCTL_AIP_MXNR,              ///< mxnr
    EN_VIP_IOCTL_AIP_UVADJ,             ///< uvadj
    EN_VIP_IOCTL_AIP_XNR,               ///< xnr
    EN_VIP_IOCTL_AIP_YCUVM,             ///< ycuvm
    EN_VIP_IOCTL_AIP_COLORTRAN,         ///< ct
    EN_VIP_IOCTL_AIP_GAMMA,             ///< gamma
    EN_VIP_IOCTL_AIP_422TO444,          ///< 422to444
    EN_VIP_IOCTL_AIP_YUVTORGB,          ///< yuv2rgb
    EN_VIP_IOCTL_AIP_GM10TO12,          ///< 10 to 12
    EN_VIP_IOCTL_AIP_CCM,               ///< ccm
    EN_VIP_IOCTL_AIP_HSV,               ///< hsv
    EN_VIP_IOCTL_AIP_GM12TO10,          ///< gm12to10
    EN_VIP_IOCTL_AIP_RGBTOYUV,          ///< rgb2yuv
    EN_VIP_IOCTL_AIP_444TO422,          ///< 4442422
    EN_VIP_IOCTL_AIP_NUM,               ///< Num
}EN_VIP_IOCTL_AIP_TYPE;

/**
* Used to setup the AIP SRAM of vip device
*/
typedef enum
{
    EN_VIP_IOCTL_AIP_SRAM_GAMMA_Y, ///< gamma y
    EN_VIP_IOCTL_AIP_SRAM_GAMMA_U, ///< gamma u
    EN_VIP_IOCTL_AIP_SRAM_GAMMA_V, ///< gamma v
    EN_VIP_IOCTL_AIP_SRAM_GM10to12_R, ///< gamma R
    EN_VIP_IOCTL_AIP_SRAM_GM10to12_G, ///< gamma G
    EN_VIP_IOCTL_AIP_SRAM_GM10to12_B, ///< gamma B
    EN_VIP_IOCTL_AIP_SRAM_GM12to10_R, ///< gamma R
    EN_VIP_IOCTL_AIP_SRAM_GM12to10_G, ///< gamma G
    EN_VIP_IOCTL_AIP_SRAM_GM12to10_B, ///< gamma B
    EN_VIP_IOCTL_AIP_SRAM_WDR, ///< wdr
    EN_VIP_IOCTL_AIP_SRAM_NUM, ///< wdr
}EN_VIP_IOCTL_AIP_SRAM_TYPE;

//=============================================================================
// struct
//=============================================================================

/**
* Used to get VIP drvier version
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    unsigned int   u32Version;      ///< version
    unsigned int   VerChk_Size;     ///< VerChk Size
}__attribute__ ((__packed__)) ST_IOCTL_VIP_VERSION_CONFIG;

/**
* Used to setup CMDQ be used of vip device
*/
typedef struct
{
    unsigned char bEn;         ///<  enable CMDQ
    unsigned char u8framecnt;  ///<  assign framecount
}ST_IOCTL_VIP_FC_CONFIG;
/**
* Used to setup AIP config of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;      ///< CMDQ
    unsigned long u32Viraddr; ///< AIP setting
    EN_VIP_IOCTL_AIP_TYPE enAIPType;
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_AIP_CONFIG;
/**
* Used to setup AIP SRAM config of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    unsigned long u32Viraddr; ///< AIP setting
    EN_VIP_IOCTL_AIP_SRAM_TYPE enAIPType;
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_AIP_SRAM_CONFIG;

/**
* Used to setup MCNR config of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;      ///< CMDQ
    unsigned char bEnMCNR;
    unsigned char bEnCIIR;
    unsigned long u32Viraddr; ///< MCNR setting
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_MCNR_CONFIG;


/**
* Used to setup LDC onoff of vip device
*/
typedef struct
{
    unsigned char bLdc_path_sel;   ///<  reg_ldc_path_sel
    unsigned char bEn_ldc;         ///<  reg_en_ldc(nonuse just bypass)
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LDC_OnOffCONFIG;
/**
* Used to setup LDC mode of vip device
*/
typedef struct
{
    EN_VIP_LDC_422_444_TYPE en422to444;    ///<  reg_422to444_md
    EN_VIP_LDC_444_422_TYPE en444to422;    ///<  reg_444to422_md
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LDC_422_444_CONFIG;

/**
* Used to setup LDC config of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;          ///< CMDQ
    ST_IOCTL_VIP_LDC_OnOffCONFIG stEn;      ///< be bypass
    ST_IOCTL_VIP_LDC_422_444_CONFIG stmd;   ///< set mode
    EN_VIP_LDCLCBANKMODE_TYPE enLDCType;   ///< LDC 64p or 128p
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LDC_CONFIG;

/**
* Used to setup LDC mode of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;      ///< CMDQ
    unsigned char u8FBidx;              ///<  reg_ldc_fb_sw_idx
    unsigned char u8FBrwdiff;           ///<  reg_ldc_fb_hw_rw_diff
    unsigned char bEnSWMode;            ///<  reg_ldc_fb_sw_mode
    EN_VIP_LDC_BYPASS_TYPE enbypass;    ///<  reg_ldc_ml_bypass
    EN_VIP_LDCLCBANKMODE_TYPE enLDCType;///< LDC 64p or 128p
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LDC_MD_CONFIG;
/**
* Used to setup LDC DMAP address  of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;      ///< CMDQ
    unsigned long u32DMAPaddr;          ///<  reg_ldc_dmap_st_addr
    unsigned short u16DMAPWidth;        ///<  reg_ldc_dmap_pitch
    unsigned char u8DMAPoffset;         ///<  reg_ldc_dmap_blk_xstart
    unsigned char bEnPowerSave;         ///<  reg_ldc_en_power_saving_mode
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LDC_DMAP_CONFIG;

/**
* Used to setup SRAM address of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;      ///< CMDQ
    unsigned long u32loadhoraddr;       ///<  reg_ldc_load_st_addr0
    unsigned short u16SRAMhorstr;       ///<  reg_ldc_sram_st_addr0
    unsigned short u16SRAMhoramount;    ///<  reg_ldc_load_amount0
    unsigned long u32loadveraddr;       ///<  reg_ldc_load_st_addr1
    unsigned short u16SRAMverstr;       ///<  reg_ldc_sram_st_addr1
    unsigned short u16SRAMveramount;    ///<  reg_ldc_load_amount1
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LDC_SRAM_CONFIG;

/**
* Used to setup NLM setting of vip device
*/
typedef struct
{
    unsigned char bNlm_en;                                             ///<  reg_nlm_en
    EN_VIP_NLM_Average_TYPE enAvgmode;                                 ///<  reg_nlm_avg_mode :0:3x3 1:5x5
    unsigned char bnlm_bdry_en;                                        ///<  reg_nlm_bdry_en
    unsigned char bnlm_post_luma_adap_en;                              ///<  reg_nlm_post_luma_adap_en
    unsigned char bnlm_luma_adap_en;                                   ///<  reg_nlm_luma_adap_en
    unsigned char bnlm_dsw_adap_en;                                    ///<  reg_nlm_dsw_adap_en
    unsigned char bnlmdsw_lpf_en;                                      ///<  reg_nlm_dsw_lpf_en
    unsigned char bnlm_region_adap_en;                                 ///<  reg_nlm_region_adap_en
    EN_VIP_NLM_DSW_TYPE u8nlm_region_adap_size_config;                 ///<  reg_nlm_region_adap_size_config 0:16x8 1:32x16
    unsigned char bnlm_histIIR_en;                                     ///<  reg_nlm_histiir_adap_en
    unsigned char bnlm_bypass_en;                                      ///<  reg_nlm_bypass_en
    unsigned char u8nlm_fin_gain;                                      ///<  reg_nlm_fin_gain
    unsigned char u8nlm_histIIR;                                       ///<  reg_nlm_histiir_adap_ratio
    unsigned char u8nlm_sad_shift;                                      ///<  reg_nlm_sad_shift
    unsigned char u8nlm_sad_gain;                                      ///<  reg_nlm_sad_gain
    unsigned char u8nlm_dsw_ratio;                                     ///<  reg_nlm_dsw_ratio
    unsigned char u8nlm_dsw_offset;                                    ///<  reg_nlm_dsw_offset
    unsigned char u8nlm_dsw_shift;                                     ///<  reg_nlm_dsw_shift
    unsigned char u8nlm_weight_lut[VIP_NLM_WEIGHT_NUM];                ///<  reg_nlm_weight_lut0-31 ,Qmap has adjust register squence
    unsigned char u8nlm_luma_adap_gain_lut[VIP_NLM_LUMAGAIN_NUM];      ///<  reg_nlm_luma_adap_gain_lut0-63,adjust register squence
    unsigned char u8nlm_post_luma_adap_gain_lut[VIP_NLM_POSTLUMA_NUM]; ///<  reg_nlm_post_luma_adap_gain_lut0-15,adjust register squence
    unsigned char u8nlm_dist_weight_7x7_lut[VIP_NLM_DISTWEIGHT_NUM];   ///<  reg_nlm_dist_weight_7x7_lut0-8,adjust register squence
    unsigned char u8nlm_main_snr_lut[VIP_NLM_POSTLUMA_NUM];   ///<  reg_main_snr_lut
    unsigned char u8nlm_wb_snr_lut[VIP_NLM_POSTLUMA_NUM];   ///<  reg_wb_snr_lut
} __attribute__ ((__packed__)) ST_IOCTL_VIP_NLM_MAIN_CONFIG;
/**
* Used to setup NLM autodown load of vip device
*/
typedef struct
{
    unsigned char bEn;         ///<  enable auto downlaod
    unsigned long u32Baseadr;  ///<  auto download phy addr
    unsigned long u32viradr;   ///<  disable auto download need virtual
} __attribute__ ((__packed__)) ST_IOCTL_VIP_NLM_SRAM_CONFIG;
/**
* Used to setup NLM config of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;          ///< CMDQ
    ST_IOCTL_VIP_NLM_MAIN_CONFIG stNLM;     ///< NLM setting
    ST_IOCTL_VIP_NLM_SRAM_CONFIG stSRAM;    ///< Autodownload
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_NLM_CONFIG;


/**
* Used to setup 422to444 of vip device
*/
typedef struct
{
    unsigned char bvip_422to444_en;    ///<  reg_vip_422to444_en
    unsigned char u8vip_422to444_md;   ///<  reg_vip_422to444_md
} __attribute__ ((__packed__)) ST_IOCTL_VIP_422_444_CONFIG;

/**
* Used to setup bypass MACE of vip device
*/
typedef struct
{
    unsigned char bvip_fun_bypass_en;  ///<  reg_vip_fun_bypass_en :except DNR,SNR,NLM,LDC
} __attribute__ ((__packed__)) ST_IOCTL_VIP_BYPASS_CONFIG;

/**
* Used to setup the LB of vip device
*/
typedef struct
{
    unsigned char u8vps_sram_act;      ///<  reg_vps_sram_act
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LINEBUFFER_CONFIG;
/**
* Used to setup mix vip  of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;              ///< CMDQ
    ST_IOCTL_VIP_422_444_CONFIG st422_444;      ///< 422 to 444
    ST_IOCTL_VIP_BYPASS_CONFIG stBypass;        ///< bypass
    ST_IOCTL_VIP_LINEBUFFER_CONFIG stLB;        ///<  VIP Mixed
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_CONFIG;

/**
* Used to setup PK HLPF of vip device
*/
typedef struct
{
    unsigned char u8main_y_lpf_coef;           ///<  reg_main_y_lpf_coef
} __attribute__ ((__packed__)) ST_IOCTL_VIP_HLPF_CONFIG;

/**
* Used to setup PK HLPF dither of vip device
*/
typedef struct
{
    unsigned char hlpf_dither_en;              ///<  reg_hlpf_dither_en
} __attribute__ ((__packed__)) ST_IOCTL_VIP_HLPF_DITHER_CONFIG;

/**
* Used to setup PK VLPF of vip device
*/
typedef struct
{
    unsigned char main_v_lpf_coef;             ///<  reg_main_v_lpf_coef_1,2
} __attribute__ ((__packed__)) ST_IOCTL_VIP_VLPF_COEF_CONFIG;

/**
* Used to PK VLPF dither of vip device
*/
typedef struct
{
    unsigned char vlpf_dither_en;              ///<  reg_vlpf_dither_en
} __attribute__ ((__packed__)) ST_IOCTL_VIP_VLPF_DITHER_CONFIG;


/**
* Used to setup PK onoff of vip device
*/
typedef struct
{
    unsigned char bpost_peaking_en;            ///<  reg_main_post_peaking_en
    unsigned char u8vps_sram_act;              ///<  reg_vps_sram_act
    unsigned char u8band6_dia_filter_sel;      ///<  reg_main_band6_dia_filter_sel
    unsigned char u8peaking_ac_yee_mode;      ///<  reg_peaking_ac_yee_mode
} __attribute__ ((__packed__)) ST_IOCTL_VIP_PEAKING_ONOFFCONFIG;

/**
* Used to setup PK band of vip device
*/
typedef struct
{
    unsigned char bBand_En[VIP_PEAKING_BAND_NUM];              ///<  reg_main_band1_peaking_en 1-8
    unsigned char u8Band_COEF_STEP[VIP_PEAKING_BAND_NUM];      ///<  reg_main_band1_coef_step 1-8
    unsigned char u8Band_COEF[VIP_PEAKING_BAND_NUM];           ///<  reg_main_band1_coef 1-8
    unsigned char u8peaking_term[VIP_PEAKING_BAND_TERM_NUM];   ///< reg_main_peaking_term1_select 1-16
    unsigned char u8Band_Over[VIP_PEAKING_BAND_NUM];           ///<  reg_band1_overshoot_limit 1-8
    unsigned char u8Band_Under[VIP_PEAKING_BAND_NUM];          ///<  reg_band1_undershoot_limit 1-8
    unsigned char u8Band_coring_thrd[VIP_PEAKING_BAND_NUM];    ///<  reg_main_band1_coring_thrd 1-8
    unsigned char u8alpha_thrd;                                ///<  reg_main_alpha_thrd
} __attribute__ ((__packed__)) ST_IOCTL_VIP_PEAKING_BAND_CONFIG;

/**
* Used to setup pk adptive of vip device
*/
typedef struct
{
    unsigned char badaptive_en[VIP_PEAKING_BAND_NUM];                                  ///<  reg_main_band1_adaptive_en 1-8
    unsigned char u8hor_lut[VIP_PEAKING_BAND_TERM_NUM];                                ///<  reg_hor_lut_0-15
    unsigned char u8ver_lut[VIP_PEAKING_BAND_TERM_NUM];                                ///<  reg_ver_lut_0-15
    unsigned char u8low_diff_thrd_and_adaptive_gain_step[VIP_PEAKING_BAND_TERM_NUM];   ///<  alternation(reg_band1_adaptive_gain_step, reg_band1_low_diff_thrd) 1-8
    unsigned char u8dia_lut[VIP_PEAKING_BAND_TERM_NUM];                                ///<  reg_dia_lut_0-15
} __attribute__ ((__packed__)) ST_IOCTL_VIP_PEAKING_ADPTIVE_CONFIG;

/**
* Used to setup PK preCoring of vip device
*/
typedef struct
{
    unsigned char u8coring_thrd_1;     ///<  reg_main_coring_thrd_1
    unsigned char u8coring_thrd_2;     ///<  reg_main_coring_thrd_2
    unsigned char u8coring_thrd_step;  ///<  reg_main_coring_thrd_step
} __attribute__ ((__packed__)) ST_IOCTL_VIP_PEAKING_PCORING_CONFIG;

/**
* Used to setup pk ADP_Y of vip device
*/
typedef struct
{
    unsigned char bcoring_adp_y_en;                                    ///<  reg_main_coring_adp_y_en
    unsigned char bcoring_adp_y_alpha_lpf_en;                          ///<  reg_main_coring_adp_y_alpha_lpf_en
    unsigned char u8coring_y_low_thrd;                                 ///<  reg_main_coring_y_low_thrd
    unsigned char u8coring_adp_y_step;                                 ///<  reg_main_coring_adp_y_step
    unsigned char u8coring_adp_y_alpha_lut[VIP_PEAKING_ADP_Y_LUT_NUM]; ///<  reg_main_coring_adp_y_alpha_lut_0-7
} __attribute__ ((__packed__)) ST_IOCTL_VIP_PEAKING_ADP_Y_CONFIG;

/**
* Used to setup PK gain of vip device
*/
typedef struct
{
    unsigned char u8osd_sharpness_ctrl ;       ///<  reg_main_osd_sharpness_ctrl
    unsigned char bosd_sharpness_sep_hv_en;    ///<  reg_main_osd_sharpness_sep_hv_en
    unsigned char u8osd_sharpness_ctrl_h  ;    ///<  reg_main_osd_sharpness_ctrl_h
    unsigned char u8osd_sharpness_ctrl_v ;     ///<  reg_main_osd_sharpness_ctrl_v
} __attribute__ ((__packed__)) ST_IOCTL_VIP_PEAKING_GAIN_CONFIG;

/**
* Used to setup PK adpy gain of vip device
*/
typedef struct
{
    unsigned char bpk_adp_y_en;                                    ///<  reg_main_coring_adp_y_en
    unsigned char bpk_adp_y_alpha_lpf_en;                          ///<  reg_main_coring_adp_y_alpha_lpf_en
    unsigned char u8pk_y_low_thrd;                                 ///<  reg_main_coring_y_low_thrd
    unsigned char u8pk_adp_y_step;                                 ///<  reg_main_coring_adp_y_step
    unsigned char u8pk_adp_y_alpha_lut[VIP_PEAKING_ADP_Y_LUT_NUM]; ///<  reg_main_coring_adp_y_alpha_lut_0-7
} __attribute__ ((__packed__)) ST_IOCTL_VIP_PEAKING_GAIN_ADP_Y_CONFIG;

/**
* Used to setup pk yc gain of vip device
*/
typedef struct
{
    unsigned short u16Dlc_in_y_gain;   ///<  reg_dlc_in_y_gain 16bit
    unsigned short u16Dlc_in_y_offset; ///<  reg_dlc_in_y_offset 16 bit
    unsigned short u16Dlc_in_c_gain;   ///<  reg_dlc_in_c_gain 16 bit
    unsigned short u16Dlc_in_c_offset; ///<  reg_dlc_in_c_offset 16 bit
    unsigned short u16Dlc_out_y_gain;  ///<  reg_dlc_out_y_gain 16 bit
    unsigned short u16Dlc_out_y_offset;///<  reg_dlc_out_y_offset 16 bit
    unsigned short u16Dlc_out_c_gain;  ///<  reg_dlc_out_c_gain 16 bit
    unsigned short u16Dlc_out_c_offset;///<  reg_dlc_out_c_offset 16 bit
} __attribute__ ((__packed__)) ST_IOCTL_VIP_YC_GAIN_OFFSET_CONFIG;

/**
* Used to setup pk config of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;                      ///< CMDQ
    ST_IOCTL_VIP_HLPF_CONFIG stHLPF;                    ///< HLPF
    ST_IOCTL_VIP_HLPF_DITHER_CONFIG stHLPFDith;         ///< HDither
    ST_IOCTL_VIP_VLPF_COEF_CONFIG stVLPFcoef1;          ///< VLPF coef1
    ST_IOCTL_VIP_VLPF_COEF_CONFIG stVLPFcoef2;          ///< VLPF coef2
    ST_IOCTL_VIP_VLPF_DITHER_CONFIG stVLPFDith;         ///< VDither
    ST_IOCTL_VIP_PEAKING_ONOFFCONFIG stOnOff;           ///< pkonoff
    ST_IOCTL_VIP_PEAKING_BAND_CONFIG stBand;            ///< pkband
    ST_IOCTL_VIP_PEAKING_ADPTIVE_CONFIG stAdp;          ///< pk adp
    ST_IOCTL_VIP_PEAKING_PCORING_CONFIG stPcor;         ///< pk precore
    ST_IOCTL_VIP_PEAKING_ADP_Y_CONFIG stAdpY;           ///< pk adp y
    ST_IOCTL_VIP_PEAKING_GAIN_CONFIG stGain;            ///< pk gain
    ST_IOCTL_VIP_PEAKING_GAIN_ADP_Y_CONFIG stGainAdpY;  ///< pk Y gain
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_PEAKING_CONFIG;


/**
* Used to setup LCE of vip device
*/
typedef struct
{
    unsigned char bLCE_En;  ///< LCE en
    unsigned char u8ControlNum;  ///<  vip control guard pipe number

} __attribute__ ((__packed__)) ST_IOCTL_VIP_LCE_ONOFF_CONFIG;


/**
* Used to setup LCE dither of vip device
*/
typedef struct
{
    unsigned char bLCE_Dither_En;   ///< LCE dither
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LCE_DITHER_CONFIG;

/**
* Used to setup LCE config of vip device
*/
typedef struct
{
    unsigned char bLCE_sodc_alpha_en;          ///<  reg_main_lce_sodc_alpha_en
    unsigned char bLce_dering_alpha_en;        ///<  reg_main_lce_dering_alpha_en
    EN_VIP_LCE_Y_AVE_SEL_TYPE enLce_y_ave_sel; ///<  reg_main_lce_y_ave_sel (1'b1: 5x7; 1'b0:5x11)
    unsigned char bLce_3curve_en;              ///<  reg_lce_3curve_en
    unsigned char u8Lce_std_slop1;             ///<  reg_main_lce_std_slop1
    unsigned char u8Lce_std_slop2;             ///<  reg_main_lce_std_slop2
    unsigned char u8Lce_std_th1;               ///<  reg_main_lce_std_th1
    unsigned char u8Lce_std_th2;               ///<  reg_main_lce_std_th2
    unsigned char u8Lce_gain_min;              ///<  reg_main_lce_gain_min
    unsigned char u8Lce_gain_max;              ///<  reg_main_lce_gain_max
    unsigned char u8Lce_sodc_low_alpha;        ///<  reg_main_lce_sodc_low_alpha
    unsigned char u8Lce_sodc_low_th;           ///<  reg_main_lce_sodc_low_th
    unsigned char u8Lce_sodc_slop;             ///<  reg_main_lce_sodc_slop
    unsigned char u8Lce_diff_gain;             ///<  reg_main_lce_diff_gain
    unsigned char u8Lce_gain_complex;          ///<  reg_main_lce_gain_complex
    unsigned char u8Lce_dsw_minsadgain;        ///<  reg_dsptch_lce_dsw_minsadgain
    unsigned char u8Lce_dsw_gain;              ///<  reg_dsptch_lce_dsw_gian
    unsigned char u8LCE_dsw_thrd;              ///<  reg_dsptch_lce_dsw_thrd
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LCE_SETTING_CONFIG;

/**
* Used to setup LCE curve of vip device
*/
typedef struct
{
    unsigned char u8Curve_Thread[4];                       ///<  reg_main_lce_curve_a-d
    unsigned short u16Curve1[VIP_LCE_CURVE_SECTION_NUM];   ///<  reg_lce_curve_lut1_08-f8 (lsb|msb) 16bit
    unsigned short u16Curve2[VIP_LCE_CURVE_SECTION_NUM];   ///<  reg_lce_curve_lut2_08-f8 (lsb|msb) 16bit
    unsigned short u16Curve3[VIP_LCE_CURVE_SECTION_NUM];   ///<  reg_lce_curve_lut3_08-f8 (lsb|msb) 16bit
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LCE_CRUVE_CONFIG;

/**
* Used to setup LCE of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;              ///< CMDQ
    ST_IOCTL_VIP_LCE_ONOFF_CONFIG stOnOff;      ///< bEn
    ST_IOCTL_VIP_LCE_DITHER_CONFIG stDITHER;    ///< dither
    ST_IOCTL_VIP_LCE_SETTING_CONFIG stSet;      ///< config
    ST_IOCTL_VIP_LCE_CRUVE_CONFIG stCurve;      ///< curve
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_LCE_CONFIG;

/**
* Used to setup DLC prog of vip device
*/
typedef struct
{
    unsigned char   u8VARCP[VIP_DLC_LUMA_SECTION_NUM]; ///<  reg_curve_fit_var_cp1 -64
    unsigned char   u8Tbln0;                           ///<  reg_main_curve_table_n0
    unsigned char   u8Tbln0LSB;                        ///<  reg_main_curve_table_n0_LSB
    unsigned char   u8Tbln0sign;                       ///<  reg_main_curve_table_n0_sign
    unsigned short  u16Tbl64;                          ///<  reg_main_curve_table64 16 bit
    unsigned char   u8Tbl64LSB;                        ///<  reg_main_curve_table64_LSB
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_PROG_CONFIG;

//IOCTL_VIP_SET_DLC_CURVE_CONFIG
/**
* Used to setup DLC curve config of vip device
*/
typedef struct
{
    unsigned char   u8InLuma[VIP_DLC_LUMA_SECTION_NUM];    ///< reg_main_curve_table0-63
    unsigned char   u8InLumaLSB[VIP_DLC_LUMA_SECTION_NUM]; ///<  reg_main_curve_table0-63 LSB
    ST_IOCTL_VIP_DLC_PROG_CONFIG ProgCfg;                   ///< dlc proc
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_CURVE_CONFIG;

/**
* Used to DLC enable of vip device
*/
typedef struct
{
    unsigned char  bcurve_fit_var_pw_en;   ///<  reg_main_curve_fit_var_pw_en
    ST_IOCTL_VIP_DLC_CURVE_CONFIG stCurve;  ///< curve config
    unsigned char  bcurve_fit_en;          ///<  reg_main_curve_fit_en
    unsigned char  bstatistic_en;          ///<  reg_main_statistic_en
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_ENABLE_CONFIG;

/**
* Used to setup dic dither of vip device
*/
typedef struct
{
    unsigned char  bDLCdither_en;   ///< bdlc dither
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_DITHER_CONFIG;

/**
* Used to setup DLC range of vip device
*/
typedef struct
{
    unsigned char u8brange_en;  ///< brange
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_HISTOGRAM_EN_CONFIG;

/**
* Used to setup DLC H of vip device
*/
typedef struct
{
    unsigned short u16statistic_h_start;   ///< reg_main_statistic_h_start 16 bit
    unsigned short u16statistic_h_end;     ///< reg_main_statistic_h_end  16 bit
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_HISTOGRAM_H_CONFIG;

/**
* Used to setup  DLC V of vip device
*/
typedef struct
{
    unsigned short u16statistic_v_start;   ///< reg_main_statistic_v_start 16 bit
    unsigned short u16statistic_v_end;     ///< reg_main_statistic_v_end  16 bit
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_HISTOGRAM_V_CONFIG;

/**
* Used to setup DLC PC of vip device
*/
typedef struct
{
    unsigned char bhis_y_rgb_mode_en;      ///<  reg_his_y_rgb_mode_en
    unsigned char bcurve_fit_rgb_en;       ///<  reg_main_curve_fit_rgb_en
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_PC_CONFIG;
/**
* Used to setup DLC of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;                      ///< CMDQ
    ST_IOCTL_VIP_YC_GAIN_OFFSET_CONFIG stGainOffset;    ///< Gain
    ST_IOCTL_VIP_DLC_ENABLE_CONFIG stEn;                ///< EN
    ST_IOCTL_VIP_DLC_DITHER_CONFIG stDither;            ///< Dither
    ST_IOCTL_VIP_DLC_HISTOGRAM_EN_CONFIG sthist;        ///< hist
    ST_IOCTL_VIP_DLC_HISTOGRAM_H_CONFIG stHistH;        ///<hhsit
    ST_IOCTL_VIP_DLC_HISTOGRAM_V_CONFIG stHistV;        ///<vhist
    ST_IOCTL_VIP_DLC_PC_CONFIG stPC;                    ///< PC
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_CONFIG;
/**
* Used to setup HIST of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;
    unsigned char bVariable_Section;                               ///< 1E04 reg_variable_range_en
    unsigned char bstatic;                                         ///< 1E04 reg_main_statistic_en
    unsigned char bcurve_fit_en;                                   ///< 1E04 reg_main_curve_fit_en
    unsigned char bhis_y_rgb_mode_en;                              ///< 1E04 reg_his_y_rgb_mode_en
    unsigned char bcurve_fit_rgb_en;                               ///< 1E04 reg_main_curve_fit_rgb_en
    unsigned char bDLCdither_en;                                   ///< 1E04 reg_ycv_dither_en
    unsigned char bstat_MIU;                                       ///<  reg_vip_stat_miu_en
    unsigned char bRange;                                          ///<  reg_main_range_en
    unsigned short u16Vst;                                         ///<  reg_main_statistic_v_start
    unsigned short u16Vnd;                                         ///<  reg_main_statistic_v_end
    unsigned short u16Hst;                                         ///<  reg_main_statistic_h_start
    unsigned short u16Hnd;                                         ///<  reg_main_statistic_v_end
    unsigned char u8HistSft;                                       ///<  reg_histrpt_sft
    unsigned char u8trig_ref_mode;                                 ///<  reg_vip_stat_trig_ref_md
    unsigned long u32StatBase[2];                                  ///<  reg_vip_stat_base0,reg_vip_stat_base1
    unsigned char u8Histogram_Range[VIP_DLC_HISTOGRAM_SECTION_NUM];///<  reg_histogram_range1-7
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_HISTOGRAM_CONFIG;

//IOCTL_VIP_GET_DLC_HISTOGRAM_REPORT
/**
* Used to setup hist report of vip device
*/
typedef struct
{
    unsigned long u32Histogram[VIP_DLC_HISTOGRAM_REPORT_NUM];  ///<  reg_total_1f-ff_00
    unsigned long u32PixelWeight;                              ///<  reg_main_total_pixel_weight
    unsigned long u32PixelCount;                               ///<  reg_main_total_pixel_count
    unsigned char u8MinPixel;                                  ///<  reg_main_min_pixel
    unsigned char u8MaxPixel;                                  ///<  reg_main_max_pixel
    unsigned char u8Baseidx;                                   ///<  reg_vip_stat_baseidx
} __attribute__ ((__packed__)) ST_IOCTL_VIP_DLC_HISTOGRAM_REPORT;

/**
* Used to setup the UVC of vip device
*/
typedef struct
{
    unsigned char buvc_en;                         ///<  reg_main_uvc_en
    unsigned char u8uvc_locate;                    ///<  reg_main_uvc_locate
    unsigned char u8uvc_gain_high_limit_lsb;       ///<  reg_main_uvc_gain_high_limit_lsb
    unsigned char u8uvc_gain_high_limit_msb;       ///<  reg_main_uvc_gain_high_limit_msb
    unsigned char u8uvc_gain_low_limit_lsb;        ///<  reg_main_uvc_gain_low_limit_lsb
    unsigned char u8uvc_gain_low_limit_msb;        ///<  reg_main_uvc_gain_low_limit_msb
    unsigned char buvc_adaptive_luma_en;           ///<  reg_main_uvc_adaptive_luma_en
    unsigned char u8uvc_adaptive_luma_y_input;     ///<  reg_main_uvc_adaptive_luma_y_input
    unsigned char u8uvc_adaptive_luma_black_step;  ///<  reg_main_uvc_adaptive_luma_black_step
    unsigned char u8uvc_adaptive_luma_white_step;  ///<  reg_main_uvc_adaptive_luma_white_step
    unsigned char u8uvc_adaptive_luma_black_th;    ///<  reg_main_uvc_adaptive_luma_black_thrd
    unsigned char u8uvc_adaptive_luma_white_th;    ///<  reg_main_uvc_adaptive_luma_white_thrd
    unsigned char u8uvc_adaptive_luma_gain_low;    ///<  reg_main_uvc_adaptive_luma_gain_low
    unsigned char u8uvc_adaptive_luma_gain_med;    ///<  reg_main_uvc_adaptive_luma_gain_med
    unsigned char u8uvc_adaptive_luma_gain_high;   ///<  reg_main_uvc_adaptive_luma_gain_high
    unsigned char buvc_rgb_en;                     ///<  reg_main_uvc_rgb_en
    unsigned char buvc_dlc_fullrange_en;           ///<  reg_main_uvc_dlc_fullrange_en
    unsigned char u8uvc_low_sat_prot_thrd;         ///<  reg_main_uvc_low_sat_prot_thrd
    unsigned char u8uvc_low_sat_min_strength;      ///<  reg_main_uvc_low_sat_min_strength
    unsigned char buvc_low_y_sat_prot_en;          ///<  reg_main_uvc_low_y_sat_prot_en
    unsigned char buvc_lpf_en;                     ///<  reg_main_uvc_lpf_en
    unsigned char buvc_low_sat_prot_en;            ///<  reg_main_uvc_low_sat_prot_en
    unsigned char buvc_low_y_prot_en;              ///<  reg_main_uvc_low_y_prot_en
    unsigned char u8uvc_low_sat_prot_slope;        ///<  reg_main_uvc_low_sat_prot_slope
    unsigned char u8uvc_low_y_prot_slope;          ///<  reg_main_uvc_low_y_prot_slope
    EN_VIP_UVC_ADP_Y_INPUT_SEL_TYPE enAdp_Ysel;    ///<  reg_main_uvc_low_y_sel
    unsigned char u8uvc_low_y_prot_thrd;           ///<  reg_main_uvc_low_y_prot_thrd
    unsigned char u8uvc_low_y_min_strength;        ///<  reg_main_uvc_low_y_min_strength
} __attribute__ ((__packed__)) ST_IOCTL_VIP_UVC_MAIN_CONFIG;
/**
* Used to setup UVC format of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;      ///< CMDQ
    ST_IOCTL_VIP_UVC_MAIN_CONFIG stUVC; ///< UVC
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_UVC_CONFIG;
/**
* Used to setup FCC adp of vip device
*/
typedef struct
{
    unsigned char u8fcc_adp_Y_LUT[VIP_FCC_YWIN_LUT_ENTRY_NUM];  ///< ///<  reg_main_fcc_adp_Y_LUT_win0_0-16
} __attribute__ ((__packed__)) ST_IOCTL_VIP_FCC_ADP_YLUT_CONFIG;

/**
* Used to setup fcc full range of vip device
*/
typedef struct
{
    unsigned char bfcc_fr_en;                      ///<  reg_main_fcc_fr_en
    unsigned char bEn[4];                          ///<  reg_main_fcc_adp_Y_win0_en 0-3
    unsigned char u8YWinNum[4];                    ///<  reg_main_fcc_adp_Y_win0_NUM 0-3
    ST_IOCTL_VIP_FCC_ADP_YLUT_CONFIG u8Ywin_LUT[4];///<  reg_main_fcc_adp_Y_LUT_win0-3_0
} __attribute__ ((__packed__)) ST_IOCTL_VIP_FCC_FULLRANGE_CONFIG;

/**
* Used to setup FCC Tx of vip device
*/
typedef struct
{
    unsigned char bEn;                                 ///<  reg_main_fcc_t_en
    unsigned char u8Cb;                                ///<  reg_fcc_cb_t
    unsigned char u8Cr;                                ///<  reg_fcc_cr_t
    unsigned char u8K;                                 ///<  reg_fcc_k_t
    unsigned char u8Range[E_VIP_FCC_Y_DIS_NUM];        ///<  reg_fcc_win_cr_down~reg_fcc_win_cb_up
    unsigned char u8LSB_Cb;                            ///<  reg_fcc_fr_cb_t_lsb  for FR reg_fcc_cb_t lsb
    unsigned char u8LSB_Cr;                            ///<  reg_fcc_fr_cr_t_lsb  for FR reg_fcc_cr_t lab
    unsigned short u16FullRange[E_VIP_FCC_Y_DIS_NUM];  ///<  reg_fcc_fr_win_cr_down ~reg_fcc_fr_win_cb_up full range 16 bit
} __attribute__ ((__packed__)) ST_IOCTL_VIP_FCC_T_CONFIG;

/**
* Used to setup FCC T9 of vip device
*/
typedef struct
{
    unsigned char bEn;                                 ///<  reg_main_fcc_9t_en
    unsigned char u8K;                                 ///<  reg_fcc_k_9t
    unsigned char u8Cr;                                ///<  reg_fcc_win9_cr
    unsigned char u8Cb;                                ///<  reg_fcc_win9_cb
    unsigned char bfirstEn;                            ///<  reg_main_fcc_9t_first_en
    unsigned char u8frCb;                              ///<  reg_fcc_fr_cb_t9
    unsigned char u8LSB_Cb;                            ///<  reg_fcc_fr_cb_t9_lsb
    unsigned char u8frCr;                              ///<  reg_fcc_fr_cr_t9
    unsigned char u8LSB_Cr;                            ///<  reg_fcc_fr_cr_t9_lsb
    unsigned short u16FullRange[E_VIP_FCC_Y_DIS_NUM];  ///<  reg_fcc_fr_win9_cr_down ~reg_fcc_fr_win9_cb_up full range 16 bit
} __attribute__ ((__packed__)) ST_IOCTL_VIP_FCC_T9_CONFIG;

/**
* Used to setup FCC of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;          ///< CMDQ
    ST_IOCTL_VIP_FCC_FULLRANGE_CONFIG stfr; ///< full range
    ST_IOCTL_VIP_FCC_T_CONFIG stT[8];       ///< STx
    ST_IOCTL_VIP_FCC_T9_CONFIG stT9;        ///< ST9
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_FCC_CONFIG;

/**
* Used to setup IHC on of vip device
*/
typedef struct
{
    unsigned char bIHC_en;    ///<  reg_main_ihc_en
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IHC_ONOFF_CONFIG;

/**
* Used to setup IHC Ymode of vip device
*/
typedef struct
{
    unsigned char bIHC_y_mode_en;                  ///<  reg_main_ihc_y_mode_en
    unsigned char bIHC_y_mode_diff_color_en;       ///<  reg_main_ihc_y_mode_diff_color_en
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IHC_Ymode_CONFIG;

/**
* Used to setup IHC dither of vip device
*/
typedef struct
{
    unsigned char bIHC_dither_en;    ///<  reg_ihc_dither_en
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IHC_DITHER_CONFIG;

/**
* Used to setup IHC user of vip device
*/
typedef struct
{
    unsigned char u8hue_user_color;        ///<  reg_main_hue_user_color0
    unsigned char u8hue_user_color_sec0;   ///<  reg_main_hue_user_color0_0
    unsigned char u8hue_user_color_sec1;   ///<  reg_main_hue_user_color0_1
    unsigned char u8hue_user_color_sec2;   ///<  reg_main_hue_user_color0_2
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IHC_USER_CONFIG;

/**
* Used to setup IHC setting of vip device
*/
typedef struct
{
    ST_IOCTL_VIP_IHC_USER_CONFIG stIHC_color[VIP_IHC_USER_COLOR_NUM];///<  0-15
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IHC_SETTING_CONFIG;

/**
* Used to setup IHC of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;          ///<CMDQ
    ST_IOCTL_VIP_IHC_ONOFF_CONFIG stOnOff;  ///<onoff
    ST_IOCTL_VIP_IHC_Ymode_CONFIG stYmd;    ///<Ymode
    ST_IOCTL_VIP_IHC_DITHER_CONFIG stDither;///<dither
    ST_IOCTL_VIP_IHC_SETTING_CONFIG stset;  ///<set
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IHC_CONFIG;

/**
* Used to setup ICC of vip device
*/
typedef struct
{
    unsigned char bICC_en;             ///<  reg_main_icc_en
    unsigned char bcbcr_to_uv_en;      ///<  reg_main_cbcr_to_uv
    unsigned char u8common_minus_gain; ///<  reg_common_minus_gain
    unsigned char u8sa_min;            ///<  reg_sa_min
    unsigned short u16step_sa_user;    ///<  reg_main_step_sa_user 16 bit
} __attribute__ ((__packed__)) ST_IOCTL_VIP_ICC_ENABLE_CONFIG;

/**
* Used to setup ICC Ymode of vip device
*/
typedef struct
{
    unsigned char u8sa_user_color_sec0;    ///<  control en
    unsigned char u8sign_sa_user_color_sec0; ///< [6:0] User adjust hue, color 0~9
    unsigned char u8sa_user_color_sec1;    ///<  control en
    unsigned char u8sign_sa_user_color_sec1; ///< [6:0] User adjust hue, color 0~9
    unsigned char u8sa_user_color_sec2;    ///<  control en
    unsigned char u8sign_sa_user_color_sec2; ///< [6:0] User adjust hue, color 0~9
} __attribute__ ((__packed__)) ST_IOCTL_VIP_ICC_Ymodein_CONFIG;

/**
* Used to setup ICC Ymode of vip device
*/
typedef struct
{
    unsigned char icc_y_mode_en;                                   ///<  reg_main_icc_y_mode_en
    unsigned char icc_y_mode_diff_color_en;                        ///<  reg_main_icc_y_mode_diff_color_en
    ST_IOCTL_VIP_ICC_Ymodein_CONFIG stICC_color[VIP_ICE_COLOR_NUM];///<  ST_IOCTL_VIP_ICC_Ymodein_CONFIG
} __attribute__ ((__packed__)) ST_IOCTL_VIP_ICC_Ymode_CONFIG;

/**
* Used to setup icc dither of vip device
*/
typedef struct
{
    unsigned char bICC_dither_en;    ///<  reg_icc_dither_en
} __attribute__ ((__packed__)) ST_IOCTL_VIP_ICC_DITHER_CONFIG;
/**
* Used to setup ICC user of vip device
*/
typedef struct
{
    unsigned char u8sa_user_color;     ///<  reg_main_sa_user_colo
    unsigned char u8sign_sa_user_color;///< reg_main_sign_sa_user_color
} __attribute__ ((__packed__)) ST_IOCTL_VIP_ICC_USER_CONFIG;

/**
* Used to setup ICC of vip device
*/
typedef struct
{
    ST_IOCTL_VIP_ICC_USER_CONFIG stICC_color[VIP_ICE_COLOR_NUM];   ///< 0-15
} __attribute__ ((__packed__)) ST_IOCTL_VIP_ICC_SETTING_CONFIG;

/**
* Used to setup ICC of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;              ///<CMDQ
    ST_IOCTL_VIP_ICC_ENABLE_CONFIG stEn;        ///<En
    ST_IOCTL_VIP_ICC_Ymode_CONFIG stYmd;        ///<Ymode
    ST_IOCTL_VIP_ICC_DITHER_CONFIG stDither;    ///<dither
    ST_IOCTL_VIP_ICC_SETTING_CONFIG stSet;      ///<set
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_ICC_CONFIG;

/**
* Used to setup ihcicc of vip device
*/
typedef struct
{
    unsigned char u8ihc_icc_y[E_VIP_IHC_ICE_ADP_Y_SECTION_NUM];               ///< reg_main_ihc_icc_y_0-3
} __attribute__ ((__packed__)) ST_IOCTL_VIP_Ymode_Yvalue_ALL_CONFIG;

/**
* Used to setup ihcicc of vip device
*/
typedef struct
{
    unsigned char u8ihc_icc_color_sec0; ///<sec0
    unsigned char u8ihc_icc_color_sec1; ///<sec1
    unsigned char u8ihc_icc_color_sec2; ///<sec2
    unsigned char u8ihc_icc_color_sec3; ///<sec3
} __attribute__ ((__packed__)) ST_IOCTL_VIP_Yvalue_USER_CONFIG;

/**
* Used to setup ICCIHC of vip device
*/
typedef struct
{
    ST_IOCTL_VIP_Yvalue_USER_CONFIG stYmode_Yvalue_color[VIP_ICE_COLOR_NUM];///< 0-15
} __attribute__ ((__packed__)) ST_IOCTL_VIP_Ymode_Yvalue_SETTING_CONFIG;

/**
* Used to setup ICCIHC of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;                      ///<CMDQ
    ST_IOCTL_VIP_Ymode_Yvalue_ALL_CONFIG stYmdall;      ///<Ymode
    ST_IOCTL_VIP_Ymode_Yvalue_SETTING_CONFIG stYmdset;  ///<Ymodeset
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IHCICC_CONFIG;

/**
* Used to setup IBC of vip device
*/
typedef struct
{
    unsigned char bIBC_en;             ///<  reg_main_ibc_en
    unsigned char u8IBC_coring_thrd;   ///<  reg_ibc_coring_thrd
    unsigned char bIBC_y_adjust_lpf_en;///<  reg_ibc_y_adjust_lpf_en
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IBC_ENABLE_CONFIG;

/**
* Used to setup IBC dither of vip device
*/
typedef struct
{
    unsigned char bIBC_dither_en;    ///<  reg_ibc_dither_en
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IBC_DITHER_CONFIG;

/**
* Used to setup IBC setting of vip device
*/
typedef struct
{
    unsigned char u8ycolor_adj[VIP_IBC_COLOR_NUM];  ///<  reg_main_ycolor0_adj 0-15
    unsigned char u8weightcminlimit;                ///< reg_weight_c_min_limit
    unsigned char u8weightcmaxlimit;                ///< reg_weight_y_min_limit
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IBC_SETTING_CONFIG;

/**
* Used to setup IBC of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;              ///< CMDQ
    ST_IOCTL_VIP_IBC_ENABLE_CONFIG stEn;        ///< bEn
    ST_IOCTL_VIP_IBC_DITHER_CONFIG stDither;    ///< dither
    ST_IOCTL_VIP_IBC_SETTING_CONFIG stSet;      ///< setting
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_IBC_CONFIG;

/**
* Used to setup ack of vip device
*/
typedef struct
{
    unsigned char backen;              ///<  reg_main_ack_en
    unsigned char bYswitch_dithen;     ///<  reg_y_switch_dither_en
    unsigned char bYswitchen;          ///<  reg_main_y_switch_en
    unsigned char u8Uswitch;           ///<  reg_u_switch_coef
    unsigned char u8Vswitch;           ///<  reg_v_switch_coef
    unsigned char u8Ythrd;             ///<  reg_ack_y_thrd
    unsigned char u8offset;            ///<  reg_ack_offset
    unsigned char u8yslot;             ///<  reg_ack_y_slop
    unsigned char u8limit;             ///<  reg_ack_limit
    unsigned char bCcompen;            ///<  reg_ack_c_comp_en
    unsigned char u8Cthrd;             ///<  reg_ack_c_thrd
    unsigned char u8Crange;            ///<  reg_ack_c_range
} __attribute__ ((__packed__)) ST_IOCTL_VIP_ACK_MAIN_CONFIG;
/**
* Used to setup clamp of vip device
*/
typedef struct
{
    unsigned char bclamp_en;       ///<  reg_vip_main_clamp_en
    unsigned short u16y_max_clamp; ///<  reg_main_y_max_clamp 16bit
    unsigned short u16y_min_clamp; ///<  reg_main_y_min_clamp 16bit
    unsigned short u16cb_max_clamp;///<  reg_main_cb_max_clamp 16bit
    unsigned short u16cb_min_clamp;///<  reg_main_cr_min_clamp 16bit
    unsigned short u16cr_max_clamp;///<  reg_main_cb_max_clamp 16bit
    unsigned short u16cr_min_clamp;///<  reg_main_cr_min_clamp 16bit
} __attribute__ ((__packed__)) ST_IOCTL_VIP_YCbCr_Clip_MAIN_CONFIG;

/**
* Used to setup ack config of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_FC_CONFIG stFCfg;              ///<CMDQ
    ST_IOCTL_VIP_ACK_MAIN_CONFIG stACK;         ///<ACK
    ST_IOCTL_VIP_YCbCr_Clip_MAIN_CONFIG stclip; ///<clamp
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
} __attribute__ ((__packed__)) ST_IOCTL_VIP_ACK_CONFIG;


/**
* Used to setup the CMDQ of vip device
*/
typedef struct
{
    ST_IOCTL_VIP_FC_CONFIG stFCfg;  ///< CMDQ
    unsigned long u32Addr;          ///< address
    unsigned short u16Data;         ///< cmd
    unsigned short u16Mask;         ///< mask
    unsigned char u8framecnt;       ///< count
    unsigned char bfire;            ///< fire
    unsigned char bCnt;             ///< bframecount
} __attribute__ ((__packed__)) ST_IOCTL_VIP_CMDQ_CONFIG;

/**
* Used to setup the susupend of vip device
*/
typedef struct
{
    unsigned long bresetflag;                   ///< flag
    unsigned long bAIPreflag;                   ///< flag
    ST_IOCTL_VIP_ACK_CONFIG stack;              ///< ack
    ST_IOCTL_VIP_IBC_CONFIG stibc;              ///< ibc
    ST_IOCTL_VIP_IHCICC_CONFIG stihcicc;        ///< iccihc
    ST_IOCTL_VIP_ICC_CONFIG sticc;              ///< icc
    ST_IOCTL_VIP_IHC_CONFIG stihc;              ///< ihc
    ST_IOCTL_VIP_FCC_CONFIG stfcc;              ///< fcc
    ST_IOCTL_VIP_UVC_CONFIG stuvc;              ///< uvc
    ST_IOCTL_VIP_DLC_HISTOGRAM_CONFIG sthist;   ///< hist
    ST_IOCTL_VIP_DLC_CONFIG stdlc;              ///< dlc
    ST_IOCTL_VIP_LCE_CONFIG stlce;              ///< lce
    ST_IOCTL_VIP_PEAKING_CONFIG stpk;           ///< pk
    ST_IOCTL_VIP_NLM_CONFIG stnlm;              ///< nlm
    ST_IOCTL_VIP_LDC_MD_CONFIG stldcmd;         ///< ldc
    ST_IOCTL_VIP_LDC_DMAP_CONFIG stldcdmap;     ///< ldc
    ST_IOCTL_VIP_LDC_SRAM_CONFIG stldcsram;     ///< ldc
    ST_IOCTL_VIP_LDC_CONFIG stldc;              ///< ldc
    ST_IOCTL_VIP_MCNR_CONFIG stmcnr;            ///<Mcnr
    ST_IOCTL_VIP_CONFIG stvip;                  ///< vipmix
    ST_IOCTL_VIP_AIP_CONFIG staip[EN_VIP_IOCTL_AIP_NUM]; ///<AIP
} __attribute__ ((__packed__)) ST_IOCTL_VIP_SUSPEND_CONFIG;
/**
* Used to setup the susupend of vip device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    ST_IOCTL_VIP_SUSPEND_CONFIG stvipCfg;
    unsigned int   VerChk_Size; ///< VerChk Size
}__attribute__ ((__packed__)) ST_IOCTL_VIP_AllSET_CONFIG;


//=============================================================================

//=============================================================================

#endif//
/** @} */ // end of vip_group
