/*
* registers.h- Sigmastar
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
#ifndef ___REGS_H
#define ___REGS_H

#include "ms_types.h"
#define REG_ADDR(addr)                         (*((volatile U16*)(0xFD200000 + (addr << 1))))

#define ARM_MIU0_BUS_BASE                      0x20000000
#define ARM_MIU1_BUS_BASE                      0xFFFFFFFF
#define ARM_MIU0_BASE_ADDR                     0x00000000
#define ARM_MIU1_BASE_ADDR                     0xFFFFFFFF

#define ARM_MIU2_BUS_BASE                      0xFFFFFFFF
#define ARM_MIU3_BUS_BASE                      0xFFFFFFFF
#define ARM_MIU2_BASE_ADDR                     0xFFFFFFFF
#define ARM_MIU3_BASE_ADDR                     0xFFFFFFFF
#define MIU0_BASE               ARM_MIU0_BUS_BASE

#define IO_PHYS                 0x1F000000
#define IO_VIRT                 (IO_PHYS+IO_OFFSET)//from IO_ADDRESS(x)
#define IO_OFFSET               (MS_IO_OFFSET)
#define IO_SIZE                 0x00400000

#define SPI_PHYS                0x14000000
#define SPI_VIRT                (SPI_PHYS+SPI_OFFSET) //from IO_ADDRESS(x)
#define SPI_OFFSET              (MS_IO_OFFSET)
#define SPI_SIZE                0x02000000

#define GIC_PHYS                0x16000000
#define GIC_VIRT                (GIC_PHYS+GIC_OFFSET) //from IO_ADDRESS(x)
#define GIC_OFFSET              (MS_IO_OFFSET)
#define GIC_SIZE                0x4000

#define IMI_PHYS                0xA0000000
#define IMI_VIRT                0xF9000000
#define IMI_OFFSET              (IMI_VIRT-IMI_PHYS)
#define IMI_SIZE                0x10000

    #define IMI_ADDR_PHYS_1 (IMI_PHYS)
    #define IMI_SIZE_1      (IMI_SIZE>>1)
    #define IMI_ADDR_PHYS_2 ((IMI_PHYS)+ ((IMI_SIZE)>>1))
    #define IMI_SIZE_2      (IMI_SIZE>>1)

#ifdef CONFIG_SS_AMP
#define IPC_MEM_PHYS            0x27000000
#define IPC_MEM_VIRT            0xFE000000
#define IPC_MEM_OFFSET          (MS_IO_OFFSET)
#define IPC_MEM_SIZE            0x800000
#endif

#define BASE_REG_RIU_PA         0x1F000000
#define BK_REG(reg)             ((reg) << 2)

#define BASE_REG_PMSLEEP_PA      GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x0E00)
#define BASE_REG_PMGPIO_PA       GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x0F00)
#define BASE_REG_PMRTC_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x1200)
#define BASE_REG_PMSAR_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x1400)
#define BASE_REG_PMTOP_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x1E00)
#define BASE_REG_EFUSE_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x2000)
#define BASE_REG_LANTOP0_PA      GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x3100)
#define BASE_REG_LANTOP1_PA      GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x3200)
#define BASE_REG_LANTOP2_PA      GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x3300)
#define BASE_REG_DIDKEY_PA       GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x3800)
#define BASE_REG_BDMA0_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x100200)
#define BASE_REG_BDMA1_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x100220)
#define BASE_REG_BDMA2_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x100240)
#define BASE_REG_BDMA3_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x100260)
#define BASE_REG_MAILBOX_PA      GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x100400)
#define BASE_REG_INTRCTL_PA      GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x100900)
#define BASE_REG_MOVDMA_PA       GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x100B00)
#define BASE_REG_ATOP_PA         GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x101000)
#define BASE_REG_MIU_PA          GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x101200)
#define BASE_REG_CHIPTOP_PA      GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x101E00)
#define BASE_REG_MIUPLL_PA       GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x103100)
#define BASE_REG_CLKGEN_PA       GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x103800)
#define BASE_REG_GPI_INT_PA      GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x103D00)
#define BASE_REG_MPLL_PA         GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x103000)
#define BASE_REG_MIUPLL_PA       GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x103100)
#define BASE_REG_ARMPLL_PA       GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x103200)
#define BASE_REG_DISP_LPLL_PA    GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x103300)
#define BASE_REG_AUSDM_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x103400)
#define BASE_REG_HDMI_TX_ATOP_PA GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x112600)
#define BASE_REG_DAC_ATOP_PA     GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x112700)
#define BASE_REG_MCM_DIG_GP_PA   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x113000)
#define BASE_REG_MCM_SC_GP_PA    GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x113200)
#define BASE_REG_MCM_VHE_GP_PA   GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x113400)
#define BASE_REG_UPLL1_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x141F00)
#define BASE_REG_UPLL0_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x142000)
#define BASE_REG_UTMI0_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x142100)
#define BASE_REG_USB0_PA         GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x142300)
#define BASE_REG_UTMI1_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x142500)
#define BASE_REG_SATA_MAC_PA     GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x152500)
#define BASE_REG_SATA_PHY_PA     GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x152600)
#define BASE_REG_SATA_ATOP_PA    GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x152700)
#define BASE_REG_DPHY_DSI_PA     GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x152800)
#define BASE_REG_UTMI2_PA        GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x142900)


#define REG_ADDR_STATUS  (BASE_REG_PMRTC_PA + REG_ID_04)
#define FORCE_UBOOT_BIT  BIT15


#define REG_ID_00      BK_REG(0x00)
#define REG_ID_01      BK_REG(0x01)
#define REG_ID_02      BK_REG(0x02)
#define REG_ID_03      BK_REG(0x03)
#define REG_ID_04      BK_REG(0x04)
#define REG_ID_05      BK_REG(0x05)
#define REG_ID_06      BK_REG(0x06)
#define REG_ID_07      BK_REG(0x07)
#define REG_ID_08      BK_REG(0x08)
#define REG_ID_09      BK_REG(0x09)
#define REG_ID_0A      BK_REG(0x0A)
#define REG_ID_0B      BK_REG(0x0B)
#define REG_ID_0C      BK_REG(0x0C)
#define REG_ID_0D      BK_REG(0x0D)
#define REG_ID_0E      BK_REG(0x0E)
#define REG_ID_0F      BK_REG(0x0F)


#define REG_ID_10      BK_REG(0x10)
#define REG_ID_11      BK_REG(0x11)
#define REG_ID_12      BK_REG(0x12)
#define REG_ID_13      BK_REG(0x13)
#define REG_ID_14      BK_REG(0x14)
#define REG_ID_15      BK_REG(0x15)
#define REG_ID_16      BK_REG(0x16)
#define REG_ID_17      BK_REG(0x17)
#define REG_ID_18      BK_REG(0x18)
#define REG_ID_19      BK_REG(0x19)
#define REG_ID_1A      BK_REG(0x1A)
#define REG_ID_1B      BK_REG(0x1B)
#define REG_ID_1C      BK_REG(0x1C)
#define REG_ID_1D      BK_REG(0x1D)
#define REG_ID_1E      BK_REG(0x1E)
#define REG_ID_1F      BK_REG(0x1F)

#define REG_ID_20      BK_REG(0x20)
#define REG_ID_21      BK_REG(0x21)
#define REG_ID_22      BK_REG(0x22)
#define REG_ID_23      BK_REG(0x23)
#define REG_ID_24      BK_REG(0x24)
#define REG_ID_25      BK_REG(0x25)
#define REG_ID_26      BK_REG(0x26)
#define REG_ID_27      BK_REG(0x27)
#define REG_ID_28      BK_REG(0x28)
#define REG_ID_29      BK_REG(0x29)
#define REG_ID_2A      BK_REG(0x2A)
#define REG_ID_2B      BK_REG(0x2B)
#define REG_ID_2C      BK_REG(0x2C)
#define REG_ID_2D      BK_REG(0x2D)
#define REG_ID_2E      BK_REG(0x2E)
#define REG_ID_2F      BK_REG(0x2F)


#define REG_ID_30      BK_REG(0x30)
#define REG_ID_31      BK_REG(0x31)
#define REG_ID_32      BK_REG(0x32)
#define REG_ID_33      BK_REG(0x33)
#define REG_ID_34      BK_REG(0x34)
#define REG_ID_35      BK_REG(0x35)
#define REG_ID_36      BK_REG(0x36)
#define REG_ID_37      BK_REG(0x37)
#define REG_ID_38      BK_REG(0x38)
#define REG_ID_39      BK_REG(0x39)
#define REG_ID_3A      BK_REG(0x3A)
#define REG_ID_3B      BK_REG(0x3B)
#define REG_ID_3C      BK_REG(0x3C)
#define REG_ID_3D      BK_REG(0x3D)
#define REG_ID_3E      BK_REG(0x3E)
#define REG_ID_3F      BK_REG(0x3F)


#define REG_ID_40      BK_REG(0x40)
#define REG_ID_41      BK_REG(0x41)
#define REG_ID_42      BK_REG(0x42)
#define REG_ID_43      BK_REG(0x43)
#define REG_ID_44      BK_REG(0x44)
#define REG_ID_45      BK_REG(0x45)
#define REG_ID_46      BK_REG(0x46)
#define REG_ID_47      BK_REG(0x47)
#define REG_ID_48      BK_REG(0x48)
#define REG_ID_49      BK_REG(0x49)
#define REG_ID_4A      BK_REG(0x4A)
#define REG_ID_4B      BK_REG(0x4B)
#define REG_ID_4C      BK_REG(0x4C)
#define REG_ID_4D      BK_REG(0x4D)
#define REG_ID_4E      BK_REG(0x4E)
#define REG_ID_4F      BK_REG(0x4F)


#define REG_ID_50      BK_REG(0x50)
#define REG_ID_51      BK_REG(0x51)
#define REG_ID_52      BK_REG(0x52)
#define REG_ID_53      BK_REG(0x53)
#define REG_ID_54      BK_REG(0x54)
#define REG_ID_55      BK_REG(0x55)
#define REG_ID_56      BK_REG(0x56)
#define REG_ID_57      BK_REG(0x57)
#define REG_ID_58      BK_REG(0x58)
#define REG_ID_59      BK_REG(0x59)
#define REG_ID_5A      BK_REG(0x5A)
#define REG_ID_5B      BK_REG(0x5B)
#define REG_ID_5C      BK_REG(0x5C)
#define REG_ID_5D      BK_REG(0x5D)
#define REG_ID_5E      BK_REG(0x5E)
#define REG_ID_5F      BK_REG(0x5F)


#define REG_ID_60      BK_REG(0x60)
#define REG_ID_61      BK_REG(0x61)
#define REG_ID_62      BK_REG(0x62)
#define REG_ID_63      BK_REG(0x63)
#define REG_ID_64      BK_REG(0x64)
#define REG_ID_65      BK_REG(0x65)
#define REG_ID_66      BK_REG(0x66)
#define REG_ID_67      BK_REG(0x67)
#define REG_ID_68      BK_REG(0x68)
#define REG_ID_69      BK_REG(0x69)
#define REG_ID_6A      BK_REG(0x6A)
#define REG_ID_6B      BK_REG(0x6B)
#define REG_ID_6C      BK_REG(0x6C)
#define REG_ID_6D      BK_REG(0x6D)
#define REG_ID_6E      BK_REG(0x6E)
#define REG_ID_6F      BK_REG(0x6F)


#define REG_ID_70      BK_REG(0x70)
#define REG_ID_71      BK_REG(0x71)
#define REG_ID_72      BK_REG(0x72)
#define REG_ID_73      BK_REG(0x73)
#define REG_ID_74      BK_REG(0x74)
#define REG_ID_75      BK_REG(0x75)
#define REG_ID_76      BK_REG(0x76)
#define REG_ID_77      BK_REG(0x77)
#define REG_ID_78      BK_REG(0x78)
#define REG_ID_79      BK_REG(0x79)
#define REG_ID_7A      BK_REG(0x7A)
#define REG_ID_7B      BK_REG(0x7B)
#define REG_ID_7C      BK_REG(0x7C)
#define REG_ID_7D      BK_REG(0x7D)
#define REG_ID_7E      BK_REG(0x7E)
#define REG_ID_7F      BK_REG(0x7F)

typedef enum
{
    MS_I2M_PACKAGE_UNKNOWN =0x00,
    MS_I2M_PACKAGE_QFN_DDR2_32MB,
    MS_I2M_PACKAGE_QFN_DDR2_64MB,
    MS_I2M_PACKAGE_BGA_128MB,
    MS_I2M_PACKAGE_BGA_256MB,
    MS_I2M_PACKAGE_QFN_DDR3_128MB,
    MS_I2M_PACKAGE_EXTENDED=0x30,
    MS_I2M_PACKAGE_DDR3_1866_128MB =0x30,
    MS_I2M_PACKAGE_DDR3_1866_256MB,
    MS_I2M_PACKAGE_FPGA_128MB =0x90,
} MS_I2M_PACKAGE_TYPE;


#endif // ___REGS_H
