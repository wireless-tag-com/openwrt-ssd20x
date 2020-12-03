/*
* mdrv_miu.h- Sigmastar
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
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2006 - 2007 Mstar Semiconductor, Inc.
// This program is free software.
// You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   Mdrv_mtlb.h
/// @brief  MTLB Driver Interface
///
///////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// Linux Mhal_mtlb.h define start
// -----------------------------------------------------------------------------
#ifndef __DRV_MTLB_H__
#define __DRV_MTLB_H__


#ifdef CONFIG_64BIT
    typedef unsigned long          phy_addr;                             // 8 bytes
#else
	typedef unsigned long long          phy_addr;                        // 8 bytes
#endif
typedef enum
{
    // group 0
    MIU_CLIENT_NONE,  //none can access
    MIU_CLIENT_CEVAXM6_0_RW,
    MIU_CLIENT_CEVAXM6_1_RW,
    MIU_CLIENT_VD_R2I_R,
    MIU_CLIENT_VD_R2_SUBSYS_R,
    MIU_CLIENT_VD_R2D_RW,
    MIU_CLIENT_CEVAXM6_2_RW,
    MIU_CLIENT_CEVAXM6_3_RW,
    MIU_CLIENT_AUDIO_R,
    MIU_CLIENT_AUDIO_AU2_R,
    MIU_CLIENT_AUDIO_AU3_W,
    MIU_CLIENT_CMDQ_R,
    MIU_CLIENT_XD2MIU_RW,
    MIU_CLIENT_UART_DMA_RW,
    MIU_CLIENT_BDMA_RW,
    MIU_CLIENT_DUMMY_G0CF,
    // group 1
    MIU_CLIENT_SC1_CROP_LDC,     
    MIU_CLIENT_ISP_GOP1_R,   
    MIU_CLIENT_CMDQ_TOP_1_R, 
    MIU_CLIENT_NOE_RW,       
    MIU_CLIENT_USB30_RW,     
    MIU_CLIENT_ISP_STA_W,    
    MIU_CLIENT_ISP_AF_STA1_W,
    MIU_CLIENT_ISP_GOP2_R,   
    MIU_CLIENT_EMAC_RW,      
    MIU_CLIENT_IVE_TOP_RW,   
    MIU_CLIENT_ISP_GOP3_R,   
    MIU_CLIENT_MIIC0_RW,     
    MIU_CLIENT_MIIC1_RW,     
    MIU_CLIENT_MIIC2_RW,     
    MIU_CLIENT_ISP_SC1_DBG_R,
    MIU_CLIENT_ISP_CMDQ_TOP2_R,
    // group 2
    MIU_CLIENT_SDIO_RW,
    MIU_CLIENT_USB30_1_RW,
    MIU_CLIENT_USB30_2_RW,
    MIU_CLIENT_SD30_RW,
    MIU_CLIENT_JPE_W,
    MIU_CLIENT_JPE_R,
    MIU_CLIENT_U3DEV_RW,
    MIU_CLIENT_JPD_RW,
    MIU_CLIENT_GMAC_RW,
    MIU_CLIENT_FCIE5_RW,/*PNAND/EMMC*/
    MIU_CLIENT_SECGMAC,
    MIU_CLIENT_USB30M1_HS_RW,
    MIU_CLIENT_SATA0_RW,
    MIU_CLIENT_SATA1_RW,
    MIU_CLIENT_USB20_RW,
    MIU_CLIENT_USB20_P1_RW,
    // group 3
    MIU_CLIENT_ISP_GOP4_R,
    MIU_CLIENT_ISOSC_BLKS_RW,
    MIU_CLIENT_CMDQ_TOP5_R,
    MIU_CLIENT_ISP_GOP0_R,
    MIU_CLIENT_SC1_FRAME_W,
    MIU_CLIENT_SC1_SNAPSHOT_W,
    MIU_CLIENT_SC2_FRAME_W,
    MIU_CLIENT_CMDQ_TOP4_R,
    MIU_CLIENT_MFE0_R,
    MIU_CLIENT_MFE0_W,
    MIU_CLIENT_SC3_FRAME_RW,
    MIU_CLIENT_DUMMY_G3CB,
    MIU_CLIENT_DUMMY_G3CC,
    MIU_CLIENT_MFE1_R,
    MIU_CLIENT_MFE1_W,
    MIU_CLIENT_ISP_MLOAD_R,
    // group 4
    MIU_CLIENT_VE_W,
    MIU_CLIENT_EVD_ENG1_RW,
    MIU_CLIENT_MGWIN0_R,
    MIU_CLIENT_MGWIN1_R,
    MIU_CLIENT_HVD_RW,
    MIU_CLIENT_HVD1_RW,
    MIU_CLIENT_DDI_0_RW,
    MIU_CLIENT_EVD_ENG0_RW,
    MIU_CLIENT_MFDEC0_1_R,
    MIU_CLIENT_ISPSC_DMAG,
    MIU_CLIENT_EVD_BBU_R,
    MIU_CLIENT_HVD_BBU_R,
    MIU_CLIENT_SC1_IPMAIN_RW,
    MIU_CLIENT_SC1_OPM_R,
    MIU_CLIENT_MFDEC_1_1_R,
    MIU_CLIENT_LDC_R,
    // group 5
    MIU_CLIENT_GOP0_R,
    MIU_CLIENT_GOP1_R,
    MIU_CLIENT_AUTO_DOWNLOAD_R,
    MIU_CLIENT_SC_DIPW_RW,
    MIU_CLIENT_MVOP_128BIT_R,
    MIU_CLIENT_MVOP1_R,
    MIU_CLIENT_FRC_IPM0_W,
    MIU_CLIENT_SC_IPSUB_RW,
    MIU_CLIENT_FRC_OPM0_R,
    MIU_CLIENT_MDWIN0_W,
    MIU_CLIENT_MFDEC0_R,
    MIU_CLIENT_MFDEC1_R,
    MIU_CLIENT_MDWIN1_W,
    MIU_CLIENT_SC_DYN_SCL_R,
    MIU_CLIENT_VE_R,
    MIU_CLIENT_GE_RW,
    // group 6
    MIU_CLIENT_ISP_DMAG0_W,
    MIU_CLIENT_ISP_DMAG0_R,
    MIU_CLIENT_ISP_DMAG1_W,
    MIU_CLIENT_ISP_DMAG1_R,
    MIU_CLIENT_ISP_DMAG2_RW,
    MIU_CLIENT_ISP_DMAG3_RW,
    MIU_CLIENT_ISP_DMAG4_W,
    MIU_CLIENT_ISP_DMAG4_R,
    MIU_CLIENT_ISP_DMAG_RW,
    MIU_CLIENT_DMA_GENERAL_RW,
    MIU_CLIENT_SC1_DNR_RW, 
    MIU_CLIENT_DUMMY_G6CB,
    MIU_CLIENT_MHE0_R,
    MIU_CLIENT_MHE0_W,
    MIU_CLIENT_MHE1_R,
    MIU_CLIENT_MHE1_W,
    // group 7
    MIU_CLIENT_MIPS_RW, //ARM CPU
    MIU_CLIENT_G3D_RW,
    MIU_CLIENT_DUMMY_G7_C2,
    MIU_CLIENT_DUMMY_G7_C3,
    MIU_CLIENT_DUMMY_G7_C4,
    MIU_CLIENT_DUMMY_G7_C5,
    MIU_CLIENT_DUMMY_G7_C6,
    MIU_CLIENT_DUMMY_G7_C7,
    MIU_CLIENT_DUMMY_G7_C8,
    MIU_CLIENT_DUMMY_G7_C9,
    MIU_CLIENT_DUMMY_G7_CA,
    MIU_CLIENT_DUMMY_G7_CB,
    MIU_CLIENT_DUMMY_G7_CC,
    MIU_CLIENT_DUMMY_G7_CD,
    MIU_CLIENT_DUMMY_G7_CE,
    MIU_CLIENT_DUMMY_G7_CF,
    // no use
    MIU_CLIENT_VIVALDI9_DECODER_R,
    MIU_CLIENT_SECAU_R2_RW,
    MIU_CLIENT_TSP_FIQ_RW,
    MIU_CLIENT_USB3_RW,
    MIU_CLIENT_CMD_QUEUE_RW,
    MIU_CLIENT_ZDEC_RW,
    MIU_CLIENT_ZDEC_ACP_RW,
    MIU_CLIENT_EVD_RW,
    MIU_CLIENT_EVD_R2D_RW,
    MIU_CLIENT_EVD_R2I_R,
    MIU_CLIENT_3RDHVD_RW,
    MIU_CLIENT_EVD_R,
    MIU_CLIENT_MFDEC_R,
    MIU_CLIENT_MVD_RTO_RW,
    MIU_CLIENT_SC1_OP_R,
    MIU_CLIENT_SECMFDEC_R,
    MIU_CLIENT_SECURE_R2_RW,
    MIU_CLIENT_AU_R2_RW,
    MIU_CLIENT_DUMMY,
    MIU_CLIENT_MVD_RW,
    MIU_CLIENT_VIVALDI9_MAD_RW,
    MIU_CLIENT_SECEMAC_RW,
    MIU_CLIENT_USB_UHC1_RW,
    MIU_CLIENT_USB_UHC2_RW,
    MIU_CLIENT_TSP_R,
    MIU_CLIENT_TSP_PVR1_W,
    MIU_CLIENT_GPD_RW,
    MIU_CLIENT_USB_UHC0_RW,
    MIU_CLIENT_GOP2_R,
    MIU_CLIENT_GOP3_R,
    //Add new after here
}eMIUClientID;

typedef enum
{
  E_MIU_0 = 0,
  E_MIU_1,
  E_MIU_2,
  E_MIU_3,
  E_MIU_NUM,
} MIU_ID;

typedef enum
{
  E_PROTECT_0 = 0,
  E_PROTECT_1,
  E_PROTECT_2,
  E_PROTECT_3,
  E_SLIT_0 = 16,
  E_MIU_PROTECT_NUM,
} PROTECT_ID;
//#define CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT

typedef enum
{
    P_DISABLE = 0,
    W_EN,
    R_EN,
    WR_EN,
    W_EN_INVERT,
    R_EN_INVERT,
    WR_EN_INVERT,
} PROTECT_CTRL;

//#ifdef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT
typedef struct
{
    unsigned char   bHit;
    unsigned char   u8Group;
    unsigned char   u8ClientID;
    unsigned char   u8Block;
	unsigned int   u16ProtectType;
	unsigned int  uAddress;
}MIU_PortectInfo;
//#endif

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
unsigned char MDrv_MIU_Init(void);
unsigned short* MDrv_MIU_GetDefaultClientID_KernelProtect(void);
unsigned char MDrv_MIU_Protect(unsigned char u8Blockx, unsigned short *pu8ProtectId, phy_addr u64Start, phy_addr u64End, PROTECT_CTRL  eSetFlag);
unsigned char MDrv_MIU_Save(void);
unsigned char MDrv_MIU_Restore(void);
#ifdef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT
unsigned char MDrv_MIU_GetProtectInfo(unsigned char u8MiuDev, MIU_PortectInfo *pInfo);
#endif
unsigned char MDrv_MIU_Slits(unsigned char u8Blockx, phy_addr u64SlitsStart, phy_addr u65SlitsEnd, unsigned char bSetFlag);
unsigned char MDrv_MIU_Get_IDEnables_Value(unsigned char u8MiuDev, unsigned char u8Blockx, unsigned char u8ClientIndex);
unsigned int MDrv_MIU_ProtectDramSize(void);

unsigned char MDrv_MIU_SetSsc(unsigned short u16Fmodulation, unsigned short u16FDeviation, unsigned char bEnable);
unsigned char MDrv_MIU_SetSscValue(unsigned char u8MiuDev, unsigned short u16Fmodulation, unsigned short u16FDeviation, unsigned char bEnable);

#endif
