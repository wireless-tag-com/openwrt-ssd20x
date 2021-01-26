/*
* MsIRQ.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: edie.chen <edie.chen@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef _MS_IRQ_H_
#define _MS_IRQ_H_



#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
// Type and Structure Declaration
//-------------------------------------------------------------------------------------------------
#define MS_IRQ_MAX          (128) //64 IRQs + 64 FIQs
#define ENABLE_USB_PORT0
#define E_IRQ_FIQ_INVALID   0xFFFF

// Interrupt related
typedef enum
{
    // IRQ
    E_INT_IRQ_0x00_START                = 0x00,
    E_INT_IRQ_UART0                     = E_INT_IRQ_0x00_START+0,
    E_INT_IRQ_BDMA_CH0                  = E_INT_IRQ_0x00_START+1,
    E_INT_IRQ_BDMA_CH1                  = E_INT_IRQ_0x00_START+2,
    E_INT_IRQ_MVD                       = E_INT_IRQ_0x00_START+3,
    E_INT_IRQ_PS                        = E_INT_IRQ_0x00_START+4,
    E_INT_IRQ_NFIE                      = E_INT_IRQ_0x00_START+5,
    E_INT_IRQ_USB                       = E_INT_IRQ_0x00_START+6,
    E_INT_IRQ_UHC                       = E_INT_IRQ_0x00_START+7,
    E_INT_IRQ_EC_BRIDGE                 = E_INT_IRQ_0x00_START+8,
    E_INT_IRQ_EMAC                      = E_INT_IRQ_0x00_START+9,
    E_INT_IRQ_DISP                      = E_INT_IRQ_0x00_START+10,
    E_INT_IRQ_DHC                       = E_INT_IRQ_0x00_START+11,
    E_INT_IRQ_PMSLEEP                   = E_INT_IRQ_0x00_START+12,
    E_INT_IRQ_SBM                       = E_INT_IRQ_0x00_START+13,
    E_INT_IRQ_COMB                      = E_INT_IRQ_0x00_START+14,
    E_INT_IRQ_ECC_DERR                  = E_INT_IRQ_0x00_START+15,
    E_INT_IRQ_0x00_END                  = 0x0F,

    E_INT_IRQ_0x10_START                = 0x10,
    E_INT_IRQ_TSP2HK                    = E_INT_IRQ_0x10_START+0,
    E_INT_IRQ_VE                        = E_INT_IRQ_0x10_START+1,
    E_INT_IRQ_CIMAX2MCU                 = E_INT_IRQ_0x10_START+2,
    E_INT_IRQ_DC                        = E_INT_IRQ_0x10_START+3,
    E_INT_IRQ_GOP                       = E_INT_IRQ_0x10_START+4,
    E_INT_IRQ_PCM                       = E_INT_IRQ_0x10_START+5,
    E_INT_IRQ_IIC0                      = E_INT_IRQ_0x10_START+6,
    E_INT_IRQ_RTC                       = E_INT_IRQ_0x10_START+7,
    E_INT_IRQ_KEYPAD                    = E_INT_IRQ_0x10_START+8,
    E_INT_IRQ_PM                        = E_INT_IRQ_0x10_START+9,
    E_INT_IRQ_DDC2BI                    = E_INT_IRQ_0x10_START+10,
    E_INT_IRQ_SCM                       = E_INT_IRQ_0x10_START+11,
    E_INT_IRQ_VBI                       = E_INT_IRQ_0x10_START+12,
    E_INT_IRQ_M4VD                      = E_INT_IRQ_0x10_START+13,
    E_INT_IRQ_FCIE2RIU                  = E_INT_IRQ_0x10_START+14,
    E_INT_IRQ_ADCDVI2RIU                = E_INT_IRQ_0x10_START+15,
    E_INT_IRQ_0x10_END                  = 0x1F,

    // FIQ
    E_INT_FIQ_0x20_START                = 0x20,
    E_INT_FIQ_EXTIMER0                  = E_INT_FIQ_0x20_START+0,
    E_INT_FIQ_EXTIMER1                  = E_INT_FIQ_0x20_START+1,
    E_INT_FIQ_WDT                       = E_INT_FIQ_0x20_START+2,
    E_INT_FIQ_AEON_TO_8051              = E_INT_FIQ_0x20_START+3,
    E_INT_FIQ_8051_TO_AEON              = E_INT_FIQ_0x20_START+4,
    E_INT_FIQ_8051_TO_BEON              = E_INT_FIQ_0x20_START+5,
    E_INT_FIQ_BEON_TO_8051              = E_INT_FIQ_0x20_START+6,
    E_INT_FIQ_BEON_TO_AEON              = E_INT_FIQ_0x20_START+7,
    E_INT_FIQ_AEON_TO_BEON              = E_INT_FIQ_0x20_START+8,
    E_INT_FIQ_JPD                       = E_INT_FIQ_0x20_START+9,
    E_INT_FIQ_MENULOAD                  = E_INT_FIQ_0x20_START+10,
    E_INT_FIQ_HDMI_NON_PCM              = E_INT_FIQ_0x20_START+11,
    E_INT_FIQ_SPDIF_IN_NON_PCM          = E_INT_FIQ_0x20_START+12,
    E_INT_FIQ_EMAC                      = E_INT_FIQ_0x20_START+13,
    E_INT_FIQ_SE_DSP2UP                 = E_INT_FIQ_0x20_START+14,
    E_INT_FIQ_TSP2AEON                  = E_INT_FIQ_0x20_START+15,
    E_INT_FIQ_0x20_END                  = 0x2F,

    E_INT_FIQ_0x30_START                = 0x30,
    E_INT_FIQ_VIVALDI_STR               = E_INT_FIQ_0x30_START+0,
    E_INT_FIQ_VIVALDI_PTS               = E_INT_FIQ_0x30_START+1,
    E_INT_FIQ_DSP_MIU_PROT              = E_INT_FIQ_0x30_START+2,
    E_INT_FIQ_XIU_TIMEOUT               = E_INT_FIQ_0x30_START+3,
    E_INT_FIQ_DMA_DONE                  = E_INT_FIQ_0x30_START+4,
    E_INT_FIQ_VSYNC_VE4VBI              = E_INT_FIQ_0x30_START+5,
    E_INT_FIQ_FIELD_VE4VBI              = E_INT_FIQ_0x30_START+6,
    E_INT_FIQ_VDMCU2HK                  = E_INT_FIQ_0x30_START+7,
    E_INT_FIQ_VE_DONE_TT                = E_INT_FIQ_0x30_START+8,
    E_INT_FIQ_INT_CCFL                  = E_INT_FIQ_0x30_START+9,
    E_INT_FIQ_INT                       = E_INT_FIQ_0x30_START+10,
    E_INT_FIQ_IR                        = E_INT_FIQ_0x30_START+11,
    E_INT_FIQ_AFEC_VSYNC                = E_INT_FIQ_0x30_START+12,
    E_INT_FIQ_DEC_DSP2UP                = E_INT_FIQ_0x30_START+13,
    E_INT_FIQ_MIPS_WDT                  = E_INT_FIQ_0x30_START+14,  //U3
    E_INT_FIQ_DEC_DSP2MIPS              = E_INT_FIQ_0x30_START+15,
    E_INT_FIQ_0x30_END                  = 0x3F,

    E_INT_IRQ_0x40_START                = 0x40,
    E_INT_IRQ_SVD_HVD                   = E_INT_IRQ_0x40_START+0,
    E_INT_IRQ_USB2                      = E_INT_IRQ_0x40_START+1,
    E_INT_IRQ_UHC2                      = E_INT_IRQ_0x40_START+2,
    E_INT_IRQ_MIU                       = E_INT_IRQ_0x40_START+3,
    E_INT_IRQ_GDMA                      = E_INT_IRQ_0x40_START+4,   //U3
    E_INT_IRQ_UART2                     = E_INT_IRQ_0x40_START+5,   //U3
    E_INT_IRQ_UART1                     = E_INT_IRQ_0x40_START+6,   //U3
    E_INT_IRQ_DEMOD                     = E_INT_IRQ_0x40_START+7,   //U3
    E_INT_IRQ_MPIF                      = E_INT_IRQ_0x40_START+8,   //U3
    E_INT_IRQ_JPD                       = E_INT_IRQ_0x40_START+9,   //U3
    E_INT_IRQ_AEON2HI                   = E_INT_IRQ_0x40_START+10,  //U3
    E_INT_IRQ_BDMA0                     = E_INT_IRQ_0x40_START+11,  //U3
    E_INT_IRQ_BDMA1                     = E_INT_IRQ_0x40_START+12,  //U3
    E_INT_IRQ_OTG                       = E_INT_IRQ_0x40_START+13,  //U3
    E_INT_IRQ_MVD_CHECKSUM_FAIL         = E_INT_IRQ_0x40_START+14,  //U3
    E_INT_IRQ_TSP_CHECKSUM_FAIL         = E_INT_IRQ_0x40_START+15,  //U3
    E_INT_IRQ_0x40_END                  = 0x4F,

    E_INT_IRQ_0x50_START                = 0x50,
    E_INT_IRQ_CA_I3                     = E_INT_IRQ_0x50_START+0,   //U3
    E_INT_IRQ_HDMI_LEVEL                = E_INT_IRQ_0x50_START+1,   //U3
    E_INT_IRQ_MIPS_WADR_ERR             = E_INT_IRQ_0x50_START+2,   //U3
    E_INT_IRQ_RASP                      = E_INT_IRQ_0x50_START+3,   //U3
    E_INT_IRQ_CA_SVP                    = E_INT_IRQ_0x50_START+4,   //U3
    E_INT_IRQ_UART2MCU                  = E_INT_IRQ_0x50_START+5,   //U3
    E_INT_IRQ_URDMA2MCU                 = E_INT_IRQ_0x50_START+6,   //U3
    E_INT_IRQ_IIC1                      = E_INT_IRQ_0x50_START+7,   //U3
    E_INT_IRQ_HDCP                      = E_INT_IRQ_0x50_START+8,   //U3
    E_INT_IRQ_DMA_WADR_ERR              = E_INT_IRQ_0x50_START+9,   //U3
    E_INT_IRQ_UP_IRQ_UART_CA            = E_INT_IRQ_0x50_START+10,  //U3
    E_INT_IRQ_UP_IRQ_EMM_ECM            = E_INT_IRQ_0x50_START+11,  //U3
    E_INT_IRQ_ONIF                      = E_INT_IRQ_0x50_START+12,  //T8
    E_INT_IRQ_USB1                      = E_INT_IRQ_0x50_START+13,  //T8
    E_INT_IRQ_UHC1                      = E_INT_IRQ_0x50_START+14,  //T8
    E_INT_IRQ_MFE                       = E_INT_IRQ_0x50_START+15,  //T8
    E_INT_IRQ_0x50_END                  = 0x5F,

    E_INT_FIQ_0x60_START                = 0x60,
    E_INT_FIQ_IR_INT_RC                 = E_INT_FIQ_0x60_START+0,   //U3
    E_INT_FIQ_HDMITX_IRQ_EDGE           = E_INT_FIQ_0x60_START+1,   //U3
    E_INT_FIQ_UP_IRQ_UART_CA            = E_INT_FIQ_0x60_START+2,   //U3
    E_INT_FIQ_UP_IRQ_EMM_ECM            = E_INT_FIQ_0x60_START+3,   //U3
    E_INT_FIQ_PVR2MI_INT0               = E_INT_FIQ_0x60_START+4,   //U3
    E_INT_FIQ_PVR2MI_INT1               = E_INT_FIQ_0x60_START+5,   //U3
    //E_INT_FIQ_8051_TO_AEON            = E_INT_FIQ_0x60_START+6,   //T3, E_INT_FIQ_8051_TO_AEON
    //Not Used                          = E_INT_FIQ_0x60_START+7,
    E_INT_FIQ_AEON_TO_MIPS_VPE0         = E_INT_FIQ_0x60_START+8,   //T3,
    E_INT_FIQ_AEON_TO_MIPS_VPE1         = E_INT_FIQ_0x60_START+9,   //T3, E_INT_FIQ_AEON_TO_BEON
    //E_INT_FIQ_AEON_TO_8051            = E_INT_FIQ_0x60_START+10,  //T3, E_INT_FIQ_AEON_TO_8051
    //Not Used                          = E_INT_FIQ_0x60_START+11,
    E_INT_FIQ_MIPS_VPE1_TO_MIPS_VPE0    = E_INT_FIQ_0x60_START+12,  //T3
    E_INT_FIQ_MIPS_VPE1_TO_AEON         = E_INT_FIQ_0x60_START+13,  //T3
    E_INT_FIQ_MIPS_VPE1_TO_8051         = E_INT_FIQ_0x60_START+14,  //T3
    //Not Used                          = E_INT_FIQ_0x60_START+15,
    E_INT_FIQ_0x60_END                  = 0x6F,

    E_INT_FIQ_0x70_START                = 0x70,
    E_INT_FIQ_MIPS_VPE0_TO_MIPS_VPE1    = E_INT_FIQ_0x70_START+0,   //T3
    E_INT_FIQ_MIPS_VPE0_TO_AEON         = E_INT_FIQ_0x70_START+1,   //T3, E_INT_FIQ_AEON_TO_BEON
    E_INT_FIQ_MIPS_VPE0_TO_8051         = E_INT_FIQ_0x70_START+2,   //T3, E_INT_FIQ_BEON_TO_8051
    E_INT_FIQ_IR_IN                     = E_INT_FIQ_0x70_START+3,   //T8
    E_INT_FIQ_DMDMCU2HK                 = E_INT_FIQ_0x70_START+4,
    E_INT_FIQ_R2TOMCU_INT0              = E_INT_FIQ_0x70_START+5,   //T8
    E_INT_FIQ_R2TOMCU_INT1              = E_INT_FIQ_0x70_START+6,   //T8
    E_INT_FIQ_DSPTOMCU_INT0             = E_INT_FIQ_0x70_START+7,   //T8
    E_INT_FIQ_DSPTOMCU_INT1             = E_INT_FIQ_0x70_START+8,   //T8
    E_INT_FIQ_USB                       = E_INT_FIQ_0x70_START+9,   //T8
    E_INT_FIQ_UHC                       = E_INT_FIQ_0x70_START+10,  //T8
    E_INT_FIQ_USB1                      = E_INT_FIQ_0x70_START+11,  //T8
    E_INT_FIQ_UHC1                      = E_INT_FIQ_0x70_START+12,  //T8
    E_INT_FIQ_USB2                      = E_INT_FIQ_0x70_START+13,  //T8
    E_INT_FIQ_UHC2                      = E_INT_FIQ_0x70_START+14,  //T8
    //Not Used                          = E_INT_FIQ_0x70_START+15,
    E_INT_FIQ_0x70_END                  = 0x7F,


    // Add IRQ from 0x80 ~ 0xBF,
    // if IRQ enum from 0x00 ~ 0x1F, and 0x40 ~ 0x5F is occupied
    E_INT_IRQ_0x80_START                = 0x80,
    E_INT_IRQ_MLINK                     = E_INT_IRQ_0x80_START+0,   //U3
    E_INT_IRQ_AFEC                      = E_INT_IRQ_0x80_START+1,   //T3
    E_INT_IRQ_DPTX                      = E_INT_IRQ_0x80_START+2,   //T3
    E_INT_IRQ_TMDDRLINK                 = E_INT_IRQ_0x80_START+3,   //T3
    E_INT_IRQ_DISPI                     = E_INT_IRQ_0x80_START+4,   //T3
    E_INT_IRQ_EXP_MLINK                 = E_INT_IRQ_0x80_START+5,   //T3
    E_INT_IRQ_M4VE                      = E_INT_IRQ_0x80_START+6,   //T3
    E_INT_IRQ_DVI_HDMI_HDCP             = E_INT_IRQ_0x80_START+7,   //T3
    E_INT_IRQ_G3D2MCU                   = E_INT_IRQ_0x80_START+8,   //T3
    E_INT_IRQ_VP6                       = E_INT_IRQ_0x80_START+9,   //A3
    //Not Used                          = E_INT_IRQ_0x80_START+10,
    E_INT_IRQ_CEC                       = E_INT_IRQ_0x80_START+11,  //T8
    E_INT_IRQ_HDCP_IIC                  = E_INT_IRQ_0x80_START+12,  //T8
    E_INT_IRQ_HDCP_X74                  = E_INT_IRQ_0x80_START+13,  //T8
    E_INT_IRQ_WADR_ERR                  = E_INT_IRQ_0x80_START+14,  //T8
    E_INT_IRQ_DCSUB                     = E_INT_IRQ_0x80_START+15,  //T8
    E_INT_IRQ_0x80_END                  = 0x8F,

    E_INT_IRQ_0x90_START                = 0x90,
    E_INT_IRQ_GE                        = E_INT_IRQ_0x90_START+0,   //T8
    E_INT_IRQ_SYNC_DET                  = E_INT_IRQ_0x90_START+1,   //M10
    E_INT_IRQ_FSP                       = E_INT_IRQ_0x90_START+2,   //M10
    E_INT_IRQ_PWM_RP_L                  = E_INT_IRQ_0x90_START+3,   //M10
    E_INT_IRQ_PWM_FP_L                  = E_INT_IRQ_0x90_START+4,   //M10
    E_INT_IRQ_PWM_RP_R                  = E_INT_IRQ_0x90_START+5,   //M10
    E_INT_IRQ_PWM_FP_R                  = E_INT_IRQ_0x90_START+6,   //M10
    E_INT_IRQ_FRC_SC                    = E_INT_IRQ_0x90_START+7,   //A5
    E_INT_IRQ_FRC_INT_FIQ2HST0          = E_INT_IRQ_0x90_START+8,   //A5
    E_INT_IRQ_SMART                     = E_INT_IRQ_0x90_START+9,   //A5
    E_INT_IRQ_MVD2MIPS                  = E_INT_IRQ_0x90_START+10,  //A5
    E_INT_IRQ_GPD                       = E_INT_IRQ_0x90_START+11,  //A5
    //Not Used                          = E_INT_IRQ_0x90_START+12,  //A5
    E_INT_IRQ_FRC_INT_IRQ2HST0          = E_INT_IRQ_0x90_START+13,  //A5
    E_INT_IRQ_MIIC_DMA_INT3             = E_INT_IRQ_0x90_START+14,  //A5
    E_INT_IRQ_MIIC_INT3                 = E_INT_IRQ_0x90_START+15,  //A5
    E_INT_IRQ_0x90_END                  = 0x9F,

    E_INT_IRQ_0xA0_START                = 0xA0,
    E_INT_IRQ_IIC2                      = E_INT_IRQ_0xA0_START+0,   //A1
    E_INT_IRQ_MIIC_DMA0                 = E_INT_IRQ_0xA0_START+1,   //A1
    E_INT_IRQ_MIIC_DMA1                 = E_INT_IRQ_0xA0_START+2,   //A1
    E_INT_IRQ_MIIC_DMA2                 = E_INT_IRQ_0xA0_START+3,   //A1
    E_INT_IRQ_MSPI0                     = E_INT_IRQ_0xA0_START+4,   //A1
    E_INT_IRQ_MSPI1                     = E_INT_IRQ_0xA0_START+5,   //A1
    E_INT_IRQ_EXT_GPIO0                 = E_INT_IRQ_0xA0_START+6,   //A1
    E_INT_IRQ_EXT_GPIO1                 = E_INT_IRQ_0xA0_START+7,   //A1
    E_INT_IRQ_EXT_GPIO2                 = E_INT_IRQ_0xA0_START+8,   //A1
    E_INT_IRQ_EXT_GPIO3                 = E_INT_IRQ_0xA0_START+9,   //A1
    E_INT_IRQ_EXT_GPIO4                 = E_INT_IRQ_0xA0_START+10,  //A1
    E_INT_IRQ_EXT_GPIO5                 = E_INT_IRQ_0xA0_START+11,  //A1
    E_INT_IRQ_EXT_GPIO6                 = E_INT_IRQ_0xA0_START+12,  //A1
    E_INT_IRQ_EXT_GPIO7                 = E_INT_IRQ_0xA0_START+13,  //A1
    E_INT_IRQ_MIIC_DMA_INT2             = E_INT_IRQ_0xA0_START+14,  //A5
    E_INT_IRQ_MIIC_INT2                 = E_INT_IRQ_0xA0_START+15,  //A5
    E_INT_IRQ_0xA0_END                  = 0xAF,

    E_INT_IRQ_0xB0_START                = 0xB0,
    E_INT_IRQ_MIIC_DMA_INT1             = E_INT_IRQ_0xB0_START+0,   //A5
    E_INT_IRQ_MIIC_INT1                 = E_INT_IRQ_0xB0_START+1,   //A5
    E_INT_IRQ_MIIC_DMA_INT0             = E_INT_IRQ_0xB0_START+2,   //A5
    E_INT_IRQ_MIIC_INT0                 = E_INT_IRQ_0xB0_START+3,   //A5
    //Not Used                          = E_INT_IRQ_0xB0_START+4,
    //Not Used                          = E_INT_IRQ_0xB0_START+5,
    //Not Used                          = E_INT_IRQ_0xB0_START+6,
    //Not Used                          = E_INT_IRQ_0xB0_START+7,
    //Not Used                          = E_INT_IRQ_0xB0_START+8,
    //Not Used                          = E_INT_IRQ_0xB0_START+9,
    //Not Used                          = E_INT_IRQ_0xB0_START+10,
    //Not Used                          = E_INT_IRQ_0xB0_START+11,
    //Not Used                          = E_INT_IRQ_0xB0_START+12,
    //Not Used                          = E_INT_IRQ_0xB0_START+13,
    //Not Used                          = E_INT_IRQ_0xB0_START+14,
    //Not Used                          = E_INT_IRQ_0xB0_START+15,
    E_INT_IRQ_0xB0_END                  = 0xBF,


    // Add FIQ from 0xC0 ~ 0xFD,
    // if FIQ enum from 0x20 ~ 0x4F, and 0x60 ~ 0x7F is occupied
    E_INT_FIQ_0xC0_START                = 0xC0,
    E_INT_FIQ_DMARD                     = E_INT_FIQ_0xC0_START+0,   //U3
    E_INT_FIQ_AU_DMA_BUF_INT            = E_INT_FIQ_0xC0_START+1,   //T3
    E_INT_FIQ_8051_TO_MIPS_VPE1         = E_INT_FIQ_0xC0_START+2,   //T3
    E_INT_FIQ_DVI_DET                   = E_INT_FIQ_0xC0_START+3,   //M10
    E_INT_FIQ_PM_GPIO0                  = E_INT_FIQ_0xC0_START+4,   //M10
    E_INT_FIQ_PM_GPIO1                  = E_INT_FIQ_0xC0_START+5,   //M10
    E_INT_FIQ_PM_GPIO2                  = E_INT_FIQ_0xC0_START+6,   //M10
    E_INT_FIQ_PM_GPIO3                  = E_INT_FIQ_0xC0_START+7,   //M10
    E_INT_FIQ_PM_XIU_TIMEOUT            = E_INT_FIQ_0xC0_START+8,   //M10
    E_INT_FIQ_PWM_RP_RP_L               = E_INT_FIQ_0xC0_START+9,   //M10
    E_INT_FIQ_PWM_RP_FP_L               = E_INT_FIQ_0xC0_START+10,  //M10
    E_INT_FIQ_PWM_RP_RP_R               = E_INT_FIQ_0xC0_START+11,  //M10
    E_INT_FIQ_PWM_RP_FP_R               = E_INT_FIQ_0xC0_START+12,  //M10
    E_INT_FIQ_8051_TO_MIPS_VPE0         = E_INT_FIQ_0xC0_START+13,  //A5
    E_INT_FIQ_FRC_R2_TO_MIPS            = E_INT_FIQ_0xC0_START+14,
    E_INT_FIQ_VP6                       = E_INT_FIQ_0xC0_START+15,  //A3
    E_INT_FIQ_0xC0_END                  = 0xCF,

    E_INT_FIQ_0xD0_START                = 0xD0,
    E_INT_FIQ_STRETCH                   = E_INT_FIQ_0xD0_START+0,
    E_INT_FIQ_GPIO0                     = E_INT_FIQ_0xD0_START+1,   //T12
    E_INT_FIQ_GPIO1                     = E_INT_FIQ_0xD0_START+2,   //T12
    E_INT_FIQ_GPIO2                     = E_INT_FIQ_0xD0_START+3,   //T12
    E_INT_FIQ_GPIO3                     = E_INT_FIQ_0xD0_START+4,   //T12
    E_INT_FIQ_GPIO4                     = E_INT_FIQ_0xD0_START+5,   //T12
    E_INT_FIQ_GPIO5                     = E_INT_FIQ_0xD0_START+6,   //T12
    E_INT_FIQ_GPIO6                     = E_INT_FIQ_0xD0_START+7,   //T12
    E_INT_FIQ_GPIO7                     = E_INT_FIQ_0xD0_START+8,   //T12
    //Not Used                          = E_INT_FIQ_0xD0_START+9,
    //Not Used                          = E_INT_FIQ_0xD0_START+10,
    //Not Used                          = E_INT_FIQ_0xD0_START+11,
    //Not Used                          = E_INT_FIQ_0xD0_START+12,
    //Not Used                          = E_INT_FIQ_0xD0_START+13,
    //Not Used                          = E_INT_FIQ_0xD0_START+14,
    //Not Used                          = E_INT_FIQ_0xD0_START+15,
    E_INT_FIQ_0xD0_END                  = 0xDF,

    E_INT_FIQ_0xE0_START                = 0xE0,
    E_INT_FIQ_LDM_DMA0                  = E_INT_FIQ_0xE0_START+0,   //A1
    E_INT_FIQ_LDM_DMA1                  = E_INT_FIQ_0xE0_START+1,   //A1
    E_INT_IRQ_SDIO                       = E_INT_FIQ_0xE0_START+2,   //K2
    E_INT_IRQ_UHC3                      = E_INT_FIQ_0xE0_START+3,    //K2
    E_INT_IRQ_USB3                      = E_INT_FIQ_0xE0_START+4,   //K2
    //Not Used                          = E_INT_FIQ_0xE0_START+2,
    //Not Used                          = E_INT_FIQ_0xE0_START+3,
    //Not Used                          = E_INT_FIQ_0xE0_START+4,
    //Not Used                          = E_INT_FIQ_0xE0_START+5,
    //Not Used                          = E_INT_FIQ_0xE0_START+6,
    //Not Used                          = E_INT_FIQ_0xE0_START+7,
    //Not Used                          = E_INT_FIQ_0xE0_START+8,
    //Not Used                          = E_INT_FIQ_0xE0_START+9,
    //Not Used                          = E_INT_FIQ_0xE0_START+10,
    //Not Used                          = E_INT_FIQ_0xE0_START+11,
    //Not Used                          = E_INT_FIQ_0xE0_START+12,
    //Not Used                          = E_INT_FIQ_0xE0_START+13,
    //Not Used                          = E_INT_FIQ_0xE0_START+14,
    //Not Used                          = E_INT_FIQ_0xE0_START+15,
    E_INT_FIQ_0xE0_END                  = 0xEF,

    E_INT_FIQ_0xF0_START                = 0xF0,
    E_INT_FIQ_DEC_DSP2R2M               = E_INT_FIQ_0xF0_START + 0,
    E_INT_FIQ_AEON_TO_R2M               = E_INT_FIQ_0xF0_START + 1,
    E_INT_FIQ_R2M_TO_AEON               = E_INT_FIQ_0xF0_START + 2,
    E_INT_FIQ_R2M_TO_8051               = E_INT_FIQ_0xF0_START + 3,
    E_INT_IRQ_VIVALDI_DMA_INTR2         = E_INT_FIQ_0xF0_START + 4,
    E_INT_IRQ_VIVALDI_DMA_INTR1         = E_INT_FIQ_0xF0_START + 5,
    E_INT_IRQ_AFEC_INT                  = E_INT_FIQ_0xF0_START + 6,
    E_INT_IRQ_FRM_PM                    = E_INT_FIQ_0xF0_START + 11,
    E_INT_FIQ_FRM_PM                    = E_INT_FIQ_0xF0_START + 12,
    //Not Used                          = E_INT_FIQ_0xF0_START+0,
    //Not Used                          = E_INT_FIQ_0xF0_START+1,
    //Not Used                          = E_INT_FIQ_0xF0_START+2,
    //Not Used                          = E_INT_FIQ_0xF0_START+3,
    //Not Used                          = E_INT_FIQ_0xF0_START+4,
    //Not Used                          = E_INT_FIQ_0xF0_START+5,
    //Not Used                          = E_INT_FIQ_0xF0_START+6,
    //Not Used                          = E_INT_FIQ_0xF0_START+7,
    //Not Used                          = E_INT_FIQ_0xF0_START+8,
    //Not Used                          = E_INT_FIQ_0xF0_START+9,
    //Not Used                          = E_INT_FIQ_0xF0_START+10,
    //Not Used                          = E_INT_FIQ_0xF0_START+11,
    //Not Used                          = E_INT_FIQ_0xF0_START+12,
    //Not Used                          = E_INT_FIQ_0xF0_START+13,
    E_INT_IRQ_FIQ_NONE                  = E_INT_FIQ_0xF0_START+14,
    E_INT_IRQ_FIQ_ALL                   = E_INT_FIQ_0xF0_START+15,
    E_INT_FIQ_0xF0_END                  = 0xFF,

} InterruptNum;


typedef enum
{
    // IRQ
    E_FRCINT_IRQ_0x00_START             = 0x00,
    E_FRCINT_IRQ_0x00_END               = 0x0F,

    E_FRCINT_IRQ_0x10_START             = 0x10,
    E_FRCINT_IRQ_FRC_XIU_TIMEOUT        = E_FRCINT_IRQ_0x10_START+5,
    E_FRCINT_IRQ_PWM_RP_L               = E_FRCINT_IRQ_0x10_START+6,
    E_FRCINT_IRQ_PWM_FP_L               = E_FRCINT_IRQ_0x10_START+7,
    E_FRCINT_IRQ_PWM_RP_R               = E_FRCINT_IRQ_0x10_START+8,
    E_FRCINT_IRQ_PWM_FP_R               = E_FRCINT_IRQ_0x10_START+9,
    E_FRCINT_IRQ_SC                     = E_FRCINT_IRQ_0x10_START+10,
    E_FRCINT_IRQ_D2B                    = E_FRCINT_IRQ_0x10_START+11,
    E_FRCINT_IRQ_MSPI1                  = E_FRCINT_IRQ_0x10_START+14,
    E_FRCINT_IRQ_MSPI0                  = E_FRCINT_IRQ_0x10_START+15,
    E_FRCINT_IRQ_0x10_END               = 0x1F,

    // FIQ
    E_FRCINT_FIQ_0x20_START             = 0x20,
    E_FRCINT_FIQ_MIPS_TO_FRCR2          = E_FRCINT_FIQ_0x20_START+0,
    E_FRCINT_FIQ_FRCR2_TO_MIPS          = E_FRCINT_FIQ_0x20_START+4,
    E_FRCINT_FIQ_0x20_END               = 0x2F,

    E_FRCINT_FIQ_0x30_START             = 0x30,
    E_FRCINT_FIQ_PWM_RP_L               = E_FRCINT_FIQ_0x30_START+6,
    E_FRCINT_FIQ_PWM_FP_L               = E_FRCINT_FIQ_0x30_START+7,
    E_FRCINT_FIQ_PWM_RP_R               = E_FRCINT_FIQ_0x30_START+8,
    E_FRCINT_FIQ_PWM_FP_R               = E_FRCINT_FIQ_0x30_START+9,
    E_FRCINT_FIQ_LDM_DMA_DONE1          = E_FRCINT_FIQ_0x30_START+12,
    E_FRCINT_FIQ_LDM_DMA_DONE0          = E_FRCINT_FIQ_0x30_START+13,
    E_FRCINT_FIQ_SC                     = E_FRCINT_FIQ_0x30_START+14,
    E_FRCINT_FIQ_OP2_VS                 = E_FRCINT_FIQ_0x30_START+15,
    E_FRCINT_FIQ_0x30_END               = 0x3F,

    // END
    E_FRCINT_FIQ_0xF0_START             = 0xF0,
    E_FRCINT_IRQ_FIQ_NONE               = E_FRCINT_FIQ_0xF0_START+14,
    E_FRCINT_IRQ_FIQ_ALL                = E_FRCINT_FIQ_0xF0_START+15,
    E_FRCINT_FIQ_0xF0_END               = 0xFF,

} InterruptNum_Frc;

#ifdef __cplusplus
}
#endif

#endif // _MS_IRQ_H_
