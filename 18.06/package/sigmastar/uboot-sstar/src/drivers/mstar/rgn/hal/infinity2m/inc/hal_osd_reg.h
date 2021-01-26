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


#ifndef __HAL_OSD_REG_H__
#define __HAL_OSD_REG_H__

#define BK_REG(reg)               ((reg) * 2)

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
#define OSD_ENABLE                   0x0001
//-------------------------------------------------------------------------------------
// REG BASE
//-------------------------------------------------------------------------------------
// Scaler/DIP enable GOP register bank
// not update
#define REG_OSD_SET_BIND_GOP_BASE       0x121800
#define REG_SCL_TO_GOP_BASE0            0x121A00
#define REG_DIP_TO_GOP_BASE             0x123A00
#define REG_SCL_BIND_OSD            BK_REG(0x03)
#define REG_SCL_EN_OSD_00            BK_REG(0x60)
#define REG_SCL_EN_OSD_01            BK_REG(0x64)
#define REG_SCL_EN_OSD_02            BK_REG(0x68)
#define REG_DIP_EN_DIP_OSD_0            BK_REG(0x10)
#define REG_SCL_PIPE_GUARD_CYCLE_MASK   0x7F00
#define REG_SCL_PIPE_GUARD_CYCLE        0x2000
#define DIP_DE_MD_MASK                  0x0002
#define DIP_DE_MD_EN                    0x0002
#define DIP_VS_INV_MASK                 0x0040
#define DIP_VS_INV_EN                   0x0040
#define REG_OSD_BYPASS_PATH_MASK        0x0080
#define REG_OSD_BYPASS_PATH_EN          0x0080
#define REG_OSD_TO_GOP_MAP_MASK         0x3000
#define REG_OSD_TO_GOP2_MAP_MASK        0x4000
#define REG_SCL_BIND_OSD_01             0x0000
#define REG_SCL_BIND_OSD_02             0x1000
#define REG_SCL_BIND_OSD_12             0x2000
#define REG_SCL_BIND_OSD2_SC            0x4000


#define REG_SCL_FCLK_BASE     0x103800

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

#define REG_OSD_00                BK_REG(0x00)
#define REG_OSD_01                BK_REG(0x01)
#define REG_OSD_02                BK_REG(0x02)
#define REG_OSD_03                BK_REG(0x03)
#define REG_OSD_04                BK_REG(0x04)
#define REG_OSD_05                BK_REG(0x05)
#define REG_OSD_06                BK_REG(0x06)
#define REG_OSD_07                BK_REG(0x07)
#define REG_OSD_08                BK_REG(0x08)
#define REG_OSD_09                BK_REG(0x09)
#define REG_OSD_0A                BK_REG(0x0A)
#define REG_OSD_0B                BK_REG(0x0B)
#define REG_OSD_0C                BK_REG(0x0C)
#define REG_OSD_0D                BK_REG(0x0D)
#define REG_OSD_0E                BK_REG(0x0E)
#define REG_OSD_0F                BK_REG(0x0F)

#define REG_OSD_10                BK_REG(0x10)
#define REG_OSD_11                BK_REG(0x11)
#define REG_OSD_12                BK_REG(0x12)
#define REG_OSD_13                BK_REG(0x13)
#define REG_OSD_14                BK_REG(0x14)
#define REG_OSD_15                BK_REG(0x15)
#define REG_OSD_16                BK_REG(0x16)
#define REG_OSD_17                BK_REG(0x17)
#define REG_OSD_18                BK_REG(0x18)
#define REG_OSD_19                BK_REG(0x19)
#define REG_OSD_1A                BK_REG(0x1A)
#define REG_OSD_1B                BK_REG(0x1B)
#define REG_OSD_1C                BK_REG(0x1C)
#define REG_OSD_1D                BK_REG(0x1D)
#define REG_OSD_1E                BK_REG(0x1E)
#define REG_OSD_1F                BK_REG(0x1F)

#define REG_OSD_20                BK_REG(0x20)
#define REG_OSD_21                BK_REG(0x21)
#define REG_OSD_22                BK_REG(0x22)
#define REG_OSD_23                BK_REG(0x23)
#define REG_OSD_24                BK_REG(0x24)
#define REG_OSD_25                BK_REG(0x25)
#define REG_OSD_26                BK_REG(0x26)
#define REG_OSD_27                BK_REG(0x27)
#define REG_OSD_28                BK_REG(0x28)
#define REG_OSD_29                BK_REG(0x29)
#define REG_OSD_2A                BK_REG(0x2A)
#define REG_OSD_2B                BK_REG(0x2B)
#define REG_OSD_2C                BK_REG(0x2C)
#define REG_OSD_2D                BK_REG(0x2D)
#define REG_OSD_2E                BK_REG(0x2E)
#define REG_OSD_2F                BK_REG(0x2F)

#define REG_OSD_30                BK_REG(0x30)
#define REG_OSD_31                BK_REG(0x31)
#define REG_OSD_32                BK_REG(0x32)
#define REG_OSD_33                BK_REG(0x33)
#define REG_OSD_34                BK_REG(0x34)
#define REG_OSD_35                BK_REG(0x35)
#define REG_OSD_36                BK_REG(0x36)
#define REG_OSD_37                BK_REG(0x37)
#define REG_OSD_38                BK_REG(0x38)
#define REG_OSD_39                BK_REG(0x39)
#define REG_OSD_3A                BK_REG(0x3A)
#define REG_OSD_3B                BK_REG(0x3B)
#define REG_OSD_3C                BK_REG(0x3C)
#define REG_OSD_3D                BK_REG(0x3D)
#define REG_OSD_3E                BK_REG(0x3E)
#define REG_OSD_3F                BK_REG(0x3F)

#define REG_OSD_40                BK_REG(0x40)
#define REG_OSD_41                BK_REG(0x41)
#define REG_OSD_42                BK_REG(0x42)
#define REG_OSD_43                BK_REG(0x43)
#define REG_OSD_44                BK_REG(0x44)
#define REG_OSD_45                BK_REG(0x45)
#define REG_OSD_46                BK_REG(0x46)
#define REG_OSD_47                BK_REG(0x47)
#define REG_OSD_48                BK_REG(0x48)
#define REG_OSD_49                BK_REG(0x49)
#define REG_OSD_4A                BK_REG(0x4A)
#define REG_OSD_4B                BK_REG(0x4B)
#define REG_OSD_4C                BK_REG(0x4C)
#define REG_OSD_4D                BK_REG(0x4D)
#define REG_OSD_4E                BK_REG(0x4E)
#define REG_OSD_4F                BK_REG(0x4F)

#define REG_OSD_50                BK_REG(0x50)
#define REG_OSD_51                BK_REG(0x51)
#define REG_OSD_52                BK_REG(0x52)
#define REG_OSD_53                BK_REG(0x53)
#define REG_OSD_54                BK_REG(0x54)
#define REG_OSD_55                BK_REG(0x55)
#define REG_OSD_56                BK_REG(0x56)
#define REG_OSD_57                BK_REG(0x57)
#define REG_OSD_58                BK_REG(0x58)
#define REG_OSD_59                BK_REG(0x59)
#define REG_OSD_5A                BK_REG(0x5A)
#define REG_OSD_5B                BK_REG(0x5B)
#define REG_OSD_5C                BK_REG(0x5C)
#define REG_OSD_5D                BK_REG(0x5D)
#define REG_OSD_5E                BK_REG(0x5E)
#define REG_OSD_5F                BK_REG(0x5F)

#define REG_OSD_60                BK_REG(0x60)
#define REG_OSD_61                BK_REG(0x61)
#define REG_OSD_62                BK_REG(0x62)
#define REG_OSD_63                BK_REG(0x63)
#define REG_OSD_64                BK_REG(0x64)
#define REG_OSD_65                BK_REG(0x65)
#define REG_OSD_66                BK_REG(0x66)
#define REG_OSD_67                BK_REG(0x67)
#define REG_OSD_68                BK_REG(0x68)
#define REG_OSD_69                BK_REG(0x69)
#define REG_OSD_6A                BK_REG(0x6A)
#define REG_OSD_6B                BK_REG(0x6B)
#define REG_OSD_6C                BK_REG(0x6C)
#define REG_OSD_6D                BK_REG(0x6D)
#define REG_OSD_6E                BK_REG(0x6E)
#define REG_OSD_6F                BK_REG(0x6F)

#define REG_OSD_70                BK_REG(0x70)
#define REG_OSD_71                BK_REG(0x71)
#define REG_OSD_72                BK_REG(0x72)
#define REG_OSD_73                BK_REG(0x73)
#define REG_OSD_74                BK_REG(0x74)
#define REG_OSD_75                BK_REG(0x75)
#define REG_OSD_76                BK_REG(0x76)
#define REG_OSD_77                BK_REG(0x77)
#define REG_OSD_78                BK_REG(0x78)
#define REG_OSD_79                BK_REG(0x79)
#define REG_OSD_7A                BK_REG(0x7A)
#define REG_OSD_7B                BK_REG(0x7B)
#define REG_OSD_7C                BK_REG(0x7C)
#define REG_OSD_7D                BK_REG(0x7D)
#define REG_OSD_7E                BK_REG(0x7E)
#define REG_OSD_7F                BK_REG(0x7F)


#endif /* __HAL_OSD_REG_H__ */
