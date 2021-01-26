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


#ifndef __HAL_GOP_REG_H__
#define __HAL_GOP_REG_H__

#define BK_REG(reg)               ((reg) * 2)

#define MIU_BUS_ALIGN_GOP_INFINITY 0x4
#define MIU_BUS_ALIGN_GOP_K6 0x0

#define HAL_GOP_MIU1_BASE       0x80000000
#define REG_GWIN_MIU_SEL_MSK    0x3
#define REG_GWIN_MIU0_SEL       0x0
#define REG_GWIN_MIU1_SEL       0x1

//-------------------------------------------------------------------------------------
#define GOP_BANK_DOUBLE_WR_G00 0x0400
#define GOP_BANK_DOUBLE_WR_G10 0x0403
#define GOP_BANK_DOUBLE_WR_G20 0x0406
#define GOP_BANK_ALL_DOUBLE_WR_G00 0x0100
#define GOP_BANK_ALL_DOUBLE_WR_G10 0x0103
#define GOP_BANK_ALL_DOUBLE_WR_G20 0x0106


#define GOP_BANK_DOUBLE_WR_MSK 0x0400
#define GOP_BANK_SEL_MSK 0x000F

#define GOP_BANK_WR_SEL_MSK 0x0F00


#define GOP_BANK_GOP0_WR_ACK_MSK 0x1000
#define GOP_BANK_GOP1_WR_ACK_MSK 0x2000
#define GOP_BANK_GOP2_WR_ACK_MSK 0x4000


//-------------------------------------------------------------------------------------
// REG_GOPG00_00
#define GOP_SOFT_RESET 0x0001
#define GOP_VS_INV 0x0002
#define GOP_HS_INV 0x0004
#define GOP_DISPLAY_MODE 0x0008 // 0:interlace, 1: progress
#define GOP_FIELD_INV 0x0010
#define GOP_YUV_TRANS_COLOR_EN 0x0020
#define GOP_TEST_PATTERN_MD_EN 0x0040
#define GOP_5541_EN 0x0080 // RGB5541 alpha mask mode enable, only for RGB1555 Data type
#define GOP_REG_OUTPUT_READY 0x0100
//#define GOP_G3D_FORMATE_NEW_MODE_ENABLE   0x0200 //unuse
#define GOP_OUTPUT_FORMAT 0x0400 // 0:RGB out, 1:YUV out
#define GOP_RGB_TRANS_COLOR_EN 0x0800
#define GOP_DISP_HBACK_ENABLE 0x1000 // H Mirror enable; H pixel read out direction, 0:forward, 1:backward
#define GOP_DISP_VBACK_ENABLE 0x2000 // V Mirror enable; V line read out direction, 0:down, 1:up
#define GOP_HS_MASK 0x4000
#define GOP_OUTPUTSEL_MASK 0x0100
#define GOP_ALPHA_INV 0x8000
// GOP Display Mode
#define GWIN_DISP_MD_PROGRESS 0x0008
#define GWIN_DISP_MD_INTERLACE 0x0000
// GOP Output Color
#define GOP_YUVOUT 0x0400
#define GOP_RGBOUT 0x0000
// GOP RGB Transparent Color
#define GOP_RGB_TRANSPARENT_COLOR_ENABLE  0x0800
#define GOP_RGB_TRANSPARENT_COLOR_DISABLE 0x0000
// GOP VYU Transparent Color
#define GOP_VYU_TRANSPARENT_COLOR_ENABLE  0x0020
#define GOP_VYU_TRANSPARENT_COLOR_DISABLE 0x0000


//-------------------------------------------------------------------------------------
// REG_GOPG00_01
// GOP Destination, not in use
//#define GOP_DST_IP_MAIN 0x0000
//#define GOP_DST_IP_SUB 0x0001
//#define GOP_DST_OP 0x0002
//#define GOP_DST_MVOP 0x0003
//#define GOP_DST_SUB_MVOP 0x0004
//#define GOP_DST_FRC 0x0006
//GOP DMA use 64 or 128bit
#define GOP_REGDMA_TYPE_MSK         0x0010
#define GOP_REGDMA_TYPE_128         0x0000
#define GOP_REGDMA_TYPE_64          0x0010
// GOP interval settings
#define GOP_REGDMA_INTERVAL_ST_MSK  0x0F00
#define GOP_REGDMA_INTERVAL_ED_MSK  0xF000
#define GOP_REGDMA_INTERVAL_START   0x0200
#define GOP_REGDMA_INTERVAL_END     0x4000

//-------------------------------------------------------------------------------------
// REG_GOPG00_02
// GOP VDE inverse
#define GOP_GWIN_VDE_INV_MSK     0x0080
#define GOP_GWIN_VDE_INV_DISABLE 0x0000
#define GOP_GWIN_VDE_INV_ENABLE  0x0080
// GOP Vsync or VDE falling trigger select
#define GOP_GWIN_VS_SEL_MSK     0x0040
#define GOP_GWIN_VS_SEL_VSYNC   0x0000
#define GOP_GWIN_VS_SEL_VFDE    0x0040
// GOP blink
#define GOP_BLINK_DISABLE       0x0000
#define GOP_BLINK_ENABLE        0x8000
// GOP Clk Src
#define GOP_CLK_CTRL_MSK     0x7000
#define GOP_CLK_CTRL_DIP     0x0000
#define GOP_CLK_CTRL_SCX     0x4000

//-------------------------------------------------------------------------------------
// REG_GOPG00_03
// GOP ARGB1555 Alpha define mask
#define GOP_GWIN_ARGB1555_ALPHA0_DEF_MSK 0x00FF
#define GOP_GWIN_ARGB1555_ALPHA1_DEF_MSK 0x00FF

//-------------------------------------------------------------------------------------
// REG_GOPG00_05
#define GOP_PALETTE_TABLE_ADDRESS_MASK  0x00FF //palette index
#define GOP_PALETTE_WRITE_ENABLE_MASK   0x0100
#define GOP_PALETTE_READ_ENABLE_MASK    0x0200
#define GOP_PALETTE_SRAM_CTRL_MASK      0x3000 //0:RIU mode; 1:REGDMA mode; 2,3:for external gop
#define GOP_PALETTE_SRAM_CTRL_RIU       0x0000
#define GOP_PALETTE_SRAM_CTRL_REGDMA    0x1000

//-------------------------------------------------------------------------------------
// REG_GOPG00_06
//#define GOP_REGDMA_END 0x0034

//-------------------------------------------------------------------------------------
// REG_GOPG00_07
//#define GOP_REGDMA_STR 0x0000
#define GOP_dram_rblk0_hstr_MASK             0x3F00
#define GOP_dram_rblk0_hstr_Shift             8

#define GOP_reg_fake_rdy_MSK             0x3F
//-------------------------------------------------------------------------------------
// REG_GOPG00_0F
#define GOP_HSYNC_PIPE_DLY_FOR_IP_SUB     0x0080
#define GOP_HSYNC_PIPE_DLY_FOR_SC1_OP     0x00A0
#define HSYNC_PIPE_DLY_FOR_SC0_2GOP       0x0044

// inifinity settings
#define INIFINITY_GOP_HSYNC_PIPE_DLY      0x0000
#define GOP_ISP_HSYNC_PIPE_DLY            INIFINITY_GOP_HSYNC_PIPE_DLY

#define GOP_HSYNC_PIPE_DLY_FOR_IP_SUB     0x0080

//-------------------------------------------------------------------------------------
// REG_GOPG00_19
#define GOP_BURST_LENGTH_MASK             0x1F00
#define GOP_BURST_LENGTH_MAX              0x1F00
#define GOP_BURST_LENGTH_AUTO             0x0000

//-------------------------------------------------------------------------------------
// REG_GOPG00_20
#define GOP_GWIN_HIGH_PRI_0               0x0000
#define GOP_GWIN_HIGH_PRI_1               0x0001
//-------------------------------------------------------------------------------------
// REG_GOPG00_32
#define GOP_STRETCH_HST                   0x0000

//-------------------------------------------------------------------------------------
// REG_GOPG00_34
#define GOP_STRETCH_VST                   0x0000

//-------------------------------------------------------------------------------------
// REG_GOPG00_35
#define GOP_STRETCH_HRATIO                0x1000
//-------------------------------------------------------------------------------------
// REG_GOPG00_36
#define GOP_STRETCH_VRATIO                0x1000
//-------------------------------------------------------------------------------------
// REG_GOPG00_38
#define GOP_STRETCH_HINI                  0x0000
//-------------------------------------------------------------------------------------
// REG_GOPG00_39
#define GOP_STRETCH_VINI                  0x0000

//-------------------------------------------------------------------------------------
// REG_GOPG01_00
#define GOP_GWIN_ENABLE                   0x0001
#define GOP_ALPHA_MASK                    0x0002
#define GOP_PIXEL_ALPHA_EN                0x0002
#define GOP_CONST_ALPHA_EN                0x0000
#define GOP_G10_ALPHA_MASK                0x4000
#define GOP_G10_PIXEL_ALPHA_EN            0x4000
#define GOP_CONST_ALPHA_Shift             1


//-------------------------------------------------------------------------------------
// REG BASE
//-------------------------------------------------------------------------------------

// GOP register bank
#define REG_ISPGOP_00_BASE     0x123100
#define REG_ISPGOP_01_BASE     0x123200
#define REG_ISPGOP_0ST_BASE    0x123300
#define REG_ISPGOP_10_BASE     0x123400
#define REG_ISPGOP_11_BASE     0x123500
#define REG_ISPGOP_1ST_BASE    0x123600


// Scaler/DIP enable GOP register bank
// not update
#define REG_OSD_SET_BIND_GOP_BASE       0x121800
#define REG_SCL_TO_GOP_BASE0            0x121A00
#define REG_DIP_TO_GOP_BASE             0x123A00
#define REG_DIP_TO_GOP_CLK_BASE         0x1038A4
#define REG_SC_TO_GOP_CLK_BASE          0x1038C8
#define REG_CLK_EN_MSK              0x1
#define REG_CLK_ENABLE              0x0
#define REG_CLK_DISABLE              0x1

#define REG_SCL_EN_ISPGOP_00            BK_REG(0x60)
#define REG_SCL_EN_ISPGOP_01            BK_REG(0x64)
#define REG_SCL_EN_ISPGOP_02            BK_REG(0x68)
#define REG_DIP_EN_DIPGOP_04            BK_REG(0x10)
#define REG_SCL_PIPE_GUARD_CYCLE_MASK   0x7F00
#define REG_SCL_PIPE_GUARD_CYCLE        0x2000
#define DIP_DE_MD_MASK                  0x0002
#define DIP_DE_MD_EN                    0x0002
#define DIP_VS_INV_MASK                 0x0040
#define DIP_VS_INV_EN                   0x0040
#define REG_OSD_BYPASS_PATH_MASK        0x0080
#define REG_OSD_BYPASS_PATH_EN          0x0080


#define REG_SCL_FCLK_BASE     0x103800

#define REG_GOP_PSRAM_CLK_BASE     0x113300

#define REG_GWIN0_REGBASE_OFFSET BK_REG(0x00)
#define REG_GWIN1_REGBASE_OFFSET BK_REG(0x10)
#define REG_GWIN2_REGBASE_OFFSET BK_REG(0x20)
#define REG_GWIN3_REGBASE_OFFSET BK_REG(0x30)

//-------------------------------------------------------------------------------------
// Color inverse related
//-------------------------------------------------------------------------------------

#define REG_SC2GOP_HS_WID_MASK      0xF
#define REG_OSD_IRQ_MASK            0xF
#define REG_OSD_INV_EN              0x1
#define REG_OSD_ACC_EN              0x2
#define REG_OSD_INV_OPR_FORCE       0x4
#define REG_OSD_INV_GOP_FORMAT      0x8
#define REG_OSD_INV_SW_MODE         0x10
#define REG_OSD_INV_COLOR_MODE      0x20
#define REG_OSD_INV_SRAM_ADDR_MASK  0x1FF
#define REG_OSD_INV_SRAM_WE         0x200
#define REG_OSD_INV_SRAM_RE         0x400
#define REG_OSD_INV_SRAM_UPD        0x800
#define REG_OSD_INV_SRAM_RST        0x1000

//-------------------------------------------------------------------------------------

#define REG_GOP_00                BK_REG(0x00)
#define REG_GOP_01                BK_REG(0x01)
#define REG_GOP_02                BK_REG(0x02)
#define REG_GOP_03                BK_REG(0x03)
#define REG_GOP_04                BK_REG(0x04)
#define REG_GOP_05                BK_REG(0x05)
#define REG_GOP_06                BK_REG(0x06)
#define REG_GOP_07                BK_REG(0x07)
#define REG_GOP_08                BK_REG(0x08)
#define REG_GOP_09                BK_REG(0x09)
#define REG_GOP_0A                BK_REG(0x0A)
#define REG_GOP_0B                BK_REG(0x0B)
#define REG_GOP_0C                BK_REG(0x0C)
#define REG_GOP_0D                BK_REG(0x0D)
#define REG_GOP_0E                BK_REG(0x0E)
#define REG_GOP_0F                BK_REG(0x0F)

#define REG_GOP_10                BK_REG(0x10)
#define REG_GOP_11                BK_REG(0x11)
#define REG_GOP_12                BK_REG(0x12)
#define REG_GOP_13                BK_REG(0x13)
#define REG_GOP_14                BK_REG(0x14)
#define REG_GOP_15                BK_REG(0x15)
#define REG_GOP_16                BK_REG(0x16)
#define REG_GOP_17                BK_REG(0x17)
#define REG_GOP_18                BK_REG(0x18)
#define REG_GOP_19                BK_REG(0x19)
#define REG_GOP_1A                BK_REG(0x1A)
#define REG_GOP_1B                BK_REG(0x1B)
#define REG_GOP_1C                BK_REG(0x1C)
#define REG_GOP_1D                BK_REG(0x1D)
#define REG_GOP_1E                BK_REG(0x1E)
#define REG_GOP_1F                BK_REG(0x1F)

#define REG_GOP_20                BK_REG(0x20)
#define REG_GOP_21                BK_REG(0x21)
#define REG_GOP_22                BK_REG(0x22)
#define REG_GOP_23                BK_REG(0x23)
#define REG_GOP_24                BK_REG(0x24)
#define REG_GOP_25                BK_REG(0x25)
#define REG_GOP_26                BK_REG(0x26)
#define REG_GOP_27                BK_REG(0x27)
#define REG_GOP_28                BK_REG(0x28)
#define REG_GOP_29                BK_REG(0x29)
#define REG_GOP_2A                BK_REG(0x2A)
#define REG_GOP_2B                BK_REG(0x2B)
#define REG_GOP_2C                BK_REG(0x2C)
#define REG_GOP_2D                BK_REG(0x2D)
#define REG_GOP_2E                BK_REG(0x2E)
#define REG_GOP_2F                BK_REG(0x2F)

#define REG_GOP_30                BK_REG(0x30)
#define REG_GOP_31                BK_REG(0x31)
#define REG_GOP_32                BK_REG(0x32)
#define REG_GOP_33                BK_REG(0x33)
#define REG_GOP_34                BK_REG(0x34)
#define REG_GOP_35                BK_REG(0x35)
#define REG_GOP_36                BK_REG(0x36)
#define REG_GOP_37                BK_REG(0x37)
#define REG_GOP_38                BK_REG(0x38)
#define REG_GOP_39                BK_REG(0x39)
#define REG_GOP_3A                BK_REG(0x3A)
#define REG_GOP_3B                BK_REG(0x3B)
#define REG_GOP_3C                BK_REG(0x3C)
#define REG_GOP_3D                BK_REG(0x3D)
#define REG_GOP_3E                BK_REG(0x3E)
#define REG_GOP_3F                BK_REG(0x3F)

#define REG_GOP_40                BK_REG(0x40)
#define REG_GOP_41                BK_REG(0x41)
#define REG_GOP_42                BK_REG(0x42)
#define REG_GOP_43                BK_REG(0x43)
#define REG_GOP_44                BK_REG(0x44)
#define REG_GOP_45                BK_REG(0x45)
#define REG_GOP_46                BK_REG(0x46)
#define REG_GOP_47                BK_REG(0x47)
#define REG_GOP_48                BK_REG(0x48)
#define REG_GOP_49                BK_REG(0x49)
#define REG_GOP_4A                BK_REG(0x4A)
#define REG_GOP_4B                BK_REG(0x4B)
#define REG_GOP_4C                BK_REG(0x4C)
#define REG_GOP_4D                BK_REG(0x4D)
#define REG_GOP_4E                BK_REG(0x4E)
#define REG_GOP_4F                BK_REG(0x4F)

#define REG_GOP_50                BK_REG(0x50)
#define REG_GOP_51                BK_REG(0x51)
#define REG_GOP_52                BK_REG(0x52)
#define REG_GOP_53                BK_REG(0x53)
#define REG_GOP_54                BK_REG(0x54)
#define REG_GOP_55                BK_REG(0x55)
#define REG_GOP_56                BK_REG(0x56)
#define REG_GOP_57                BK_REG(0x57)
#define REG_GOP_58                BK_REG(0x58)
#define REG_GOP_59                BK_REG(0x59)
#define REG_GOP_5A                BK_REG(0x5A)
#define REG_GOP_5B                BK_REG(0x5B)
#define REG_GOP_5C                BK_REG(0x5C)
#define REG_GOP_5D                BK_REG(0x5D)
#define REG_GOP_5E                BK_REG(0x5E)
#define REG_GOP_5F                BK_REG(0x5F)

#define REG_GOP_60                BK_REG(0x60)
#define REG_GOP_61                BK_REG(0x61)
#define REG_GOP_62                BK_REG(0x62)
#define REG_GOP_63                BK_REG(0x63)
#define REG_GOP_64                BK_REG(0x64)
#define REG_GOP_65                BK_REG(0x65)
#define REG_GOP_66                BK_REG(0x66)
#define REG_GOP_67                BK_REG(0x67)
#define REG_GOP_68                BK_REG(0x68)
#define REG_GOP_69                BK_REG(0x69)
#define REG_GOP_6A                BK_REG(0x6A)
#define REG_GOP_6B                BK_REG(0x6B)
#define REG_GOP_6C                BK_REG(0x6C)
#define REG_GOP_6D                BK_REG(0x6D)
#define REG_GOP_6E                BK_REG(0x6E)
#define REG_GOP_6F                BK_REG(0x6F)

#define REG_GOP_70                BK_REG(0x70)
#define REG_GOP_71                BK_REG(0x71)
#define REG_GOP_72                BK_REG(0x72)
#define REG_GOP_73                BK_REG(0x73)
#define REG_GOP_74                BK_REG(0x74)
#define REG_GOP_75                BK_REG(0x75)
#define REG_GOP_76                BK_REG(0x76)
#define REG_GOP_77                BK_REG(0x77)
#define REG_GOP_78                BK_REG(0x78)
#define REG_GOP_79                BK_REG(0x79)
#define REG_GOP_7A                BK_REG(0x7A)
#define REG_GOP_7B                BK_REG(0x7B)
#define REG_GOP_7C                BK_REG(0x7C)
#define REG_GOP_7D                BK_REG(0x7D)
#define REG_GOP_7E                BK_REG(0x7E)
#define REG_GOP_7F                BK_REG(0x7F)


#endif /* __HAL_GOP_REG_H__ */
