/*
* mhal_iic_reg.h- Sigmastar
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
#ifndef _REG_IIC_H_
#define _REG_IIC_H_

#define _bit15		0x8000
#define _bit14		0x4000
#define _bit13		0x2000
#define _bit12		0x1000
#define _bit11		0x0800
#define _bit10		0x0400
#define _bit9		0x0200
#define _bit8		0x0100
#define _bit7		0x0080
#define _bit6		0x0040
#define _bit5		0x0020
#define _bit4		0x0010
#define _bit3		0x0008
#define _bit2		0x0004
#define _bit1		0x0002
#define _bit0		0x0001


//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------
//#define IIC_UNIT_NUM               2

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define MHal_IIC_DELAY()      { udelay(1000); udelay(1000); udelay(1000); udelay(1000); udelay(1000); }//delay 5ms

#define REG_IIC_BASE              0xFD223000 //0xFD223600 // 0xBF200000 + (0x8D80*4) //The 4th port

#define REG_IIC_CTRL               0x00
#define REG_IIC_CLK_SEL            0x01
#define REG_IIC_WDATA              0x02
#define REG_IIC_RDATA              0x03
#define REG_IIC_STATUS             0x04                                // reset, clear and status
#define MHal_IIC_REG(addr)			(*(volatile U32*)(REG_IIC_BASE + ((addr)<<2)))

#define REG_CHIP_BASE              0xFD203C00
#define REG_IIC_ALLPADIN           0x50
#define REG_IIC_MODE               0x57
#define REG_DDCR_GPIO_SEL          0x70
#define MHal_CHIP_REG(addr)             (*(volatile U32*)(REG_CHIP_BASE + ((addr)<<2)))


//the definitions of GPIO reg set to initialize
#define REG_ARM_BASE              0xFD000000//Use 8 bit addressing
//#define REG_ALL_PAD_IN              ((0x0f50<<1) )   //set all pads (except SPI) as input
#define REG_ALL_PAD_IN              (0x101ea1)   //set all pads (except SPI) as input

//the definitions of GPIO reg set to make output
#define PAD_DDCR_CK 173
#define REG_PAD_DDCR_CK_SET (0x101eae)
#define REG_PAD_DDCR_CK_OEN (0x102b87)
#define REG_PAD_DDCR_CK_IN (0x102b87)
#define REG_PAD_DDCR_CK_OUT (0x102b87)
#define PAD_DDCR_DA 172
#define REG_PAD_DDCR_DA_SET (0x101eae)
#define REG_PAD_DDCR_DA_OEN (0x102b86)
#define REG_PAD_DDCR_DA_IN (0x102b86)
#define REG_PAD_DDCR_DA_OUT (0x102b86)

#define PAD_TGPIO2 181
#define REG_PAD_TGPIO2_SET
#define REG_PAD_TGPIO2_OEN (0x102b8f)
#define REG_PAD_TGPIO2_IN (0x102b8f)
#define REG_PAD_TGPIO2_OUT (0x102b8f)
#define PAD_TGPIO3 182
#define REG_PAD_TGPIO3_SET
#define REG_PAD_TGPIO3_OEN (0x102b90)
#define REG_PAD_TGPIO3_IN (0x102b90)
#define REG_PAD_TGPIO3_OUT (0x102b90)

#define PAD_I2S_OUT_SD1 101
#define REG_PAD_I2S_OUT_SD1_SET
#define REG_PAD_I2S_OUT_SD1_OEN (0x102b3f)
#define REG_PAD_I2S_OUT_SD1_IN (0x102b3f)
#define REG_PAD_I2S_OUT_SD1_OUT (0x102b3f)
#define PAD_SPDIF_IN 95
#define REG_PAD_SPDIF_IN_SET
#define REG_PAD_SPDIF_IN_OEN (0x102b39)
#define REG_PAD_SPDIF_IN_IN (0x102b39)
#define REG_PAD_SPDIF_IN_OUT (0x102b39)

#define PAD_I2S_IN_WS 92
#define REG_PAD_I2S_IN_WS_SET
#define REG_PAD_I2S_IN_WS_OEN (0x102b36)
#define REG_PAD_I2S_IN_WS_IN (0x102b36)
#define REG_PAD_I2S_IN_WS_OUT (0x102b36)
#define PAD_I2S_IN_BCK 93
#define REG_PAD_I2S_IN_BCK_SET
#define REG_PAD_I2S_IN_BCK_OEN (0x102b37)
#define REG_PAD_I2S_IN_BCK_IN (0x102b37)
#define REG_PAD_I2S_IN_BCK_OUT (0x102b37)

#define PAD_I2S_OUT_SD3 103
#define REG_PAD_I2S_OUT_SD3_SET
#define REG_PAD_I2S_OUT_SD3_OEN (0x102b41)
#define REG_PAD_I2S_OUT_SD3_IN (0x102b41)
#define REG_PAD_I2S_OUT_SD3_OUT (0x102b41)
#define PAD_I2S_OUT_SD2 102
#define REG_PAD_I2S_OUT_SD2_SET
#define REG_PAD_I2S_OUT_SD2_OEN (0x102b40)
#define REG_PAD_I2S_OUT_SD2_IN (0x102b40)
#define REG_PAD_I2S_OUT_SD2_OUT (0x102b40)

#define PAD_GPIO_PM9 9
#define REG_PAD_GPIO_PM9_SET
#define REG_PAD_GPIO_PM9_OEN (0x0f12)
#define REG_PAD_GPIO_PM9_IN (0x0f12)
#define REG_PAD_GPIO_PM9_OUT (0x0f12)
#define PAD_GPIO_PM8 8
#define REG_PAD_GPIO_PM8_SET
#define REG_PAD_GPIO_PM8_OEN (0x0f10)
#define REG_PAD_GPIO_PM8_IN (0x0f10)
#define REG_PAD_GPIO_PM8_OUT (0x0f10)

#define MHal_GPIO_REG(addr)             (*(volatile U8*)(REG_ARM_BASE + (((addr) & ~1)<<1) + (addr & 1)))
//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    U32 SclOenReg;
    U8  SclOenBit;

    U32 SclOutReg;
    U8  SclOutBit;

    U32 SclInReg;
    U8  SclInBit;

    U32 SdaOenReg;
    U8  SdaOenBit;

    U32 SdaOutReg;
    U8  SdaOutBit;

    U32 SdaInReg;
    U8  SdaInBit;

    U8  DefDelay;
}IIC_Bus_t;
/**************************&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&**********************************/
//############################
//
//IP bank address : for pad mux in chiptop
//
//############################
#define CHIP_REG_BASE                   0//(0x101E00)
#define CHIP_GPIO1_REG_BASE            0//(0x101A00)

//for port 0
#define CHIP_REG_HWI2C_MIIC0            (CHIP_REG_BASE+ (0x09*2))
    #define CHIP_MIIC0_PAD_0            (0)
    #define CHIP_MIIC0_PAD_1            (__BIT0)
    #define CHIP_MIIC0_PAD_2            (__BIT1)
    #define CHIP_MIIC0_PAD_3            (__BIT0|__BIT1)
    #define CHIP_MIIC0_PAD_4            (__BIT2)
    #define CHIP_MIIC0_PAD_MSK          (__BIT0|__BIT1|__BIT2)

//for port 1
#define CHIP_REG_HWI2C_MIIC1            (CHIP_REG_BASE+ (0x09*2))
    #define CHIP_MIIC1_PAD_0            (0)
    #define CHIP_MIIC1_PAD_1            (__BIT4)
    #define CHIP_MIIC1_PAD_2            (__BIT5)
    #define CHIP_MIIC1_PAD_3            (__BIT4|__BIT5)
    #define CHIP_MIIC1_PAD_4            (__BIT6)
    #define CHIP_MIIC1_PAD_5            (__BIT4|__BIT6)
    #define CHIP_MIIC1_PAD_MSK          (__BIT4|__BIT5|__BIT6)

//for port 2
#define CHIP_REG_HWI2C_MIIC2            (CHIP_REG_BASE+ (0x16*2+1))
    #define CHIP_MIIC2_PAD_0            (0)
    #define CHIP_MIIC2_PAD_1            (__BIT0)
    #define CHIP_MIIC2_PAD_2            (__BIT1)
    #define CHIP_MIIC2_PAD_3            (__BIT0|__BIT1)
    #define CHIP_MIIC2_PAD_MSK          (__BIT0|__BIT1)

//for port 3
#define CHIP_REG_HWI2C_MIIC3            (CHIP_REG_BASE+ (0x16*2+1))
    #define CHIP_MIIC3_PAD_0            (0)
    #define CHIP_MIIC3_PAD_1            (__BIT2)
    #define CHIP_MIIC3_PAD_2            (__BIT3)
    #define CHIP_MIIC3_PAD_3            (__BIT3|__BIT2)
    #define CHIP_MIIC3_PAD_4            (__BIT4)
    #define CHIP_MIIC3_PAD_5            (__BIT4|__BIT0)
    #define CHIP_MIIC3_PAD_MSK          (__BIT2|__BIT3|__BIT4)

//pad mux configuration
#define CHIP_REG_ALLPADIN               (CHIP_REG_BASE+0xA0)
    #define CHIP_ALLPAD_IN              (__BIT0)

//############################
//
//IP bank address : for independent port
//
//############################
//Standard mode
#define HWI2C_REG_BASE                  0//(0x111800) //0x1(11800) + offset ==> default set to port 0
#define REG_HWI2C_MIIC_CFG              (HWI2C_REG_BASE+0x00*2)
    #define _MIIC_CFG_RESET             (__BIT0)
    #define _MIIC_CFG_EN_DMA            (__BIT1)
    #define _MIIC_CFG_EN_INT            (__BIT2)
    #define _MIIC_CFG_EN_CLKSTR         (__BIT3)
    #define _MIIC_CFG_EN_TMTINT         (__BIT4)
    #define _MIIC_CFG_EN_FILTER         (__BIT5)
    #define _MIIC_CFG_EN_PUSH1T         (__BIT6)
    #define _MIIC_CFG_RESERVED          (__BIT7)
#define REG_HWI2C_CMD_START             (HWI2C_REG_BASE+0x01*2)
    #define _CMD_START                  (__BIT0)
#define REG_HWI2C_CMD_STOP              (HWI2C_REG_BASE+0x01*2+1)
    #define _CMD_STOP                   (__BIT0)
#define REG_HWI2C_WDATA                 (HWI2C_REG_BASE+0x02*2)
#define REG_HWI2C_WDATA_GET             (HWI2C_REG_BASE+0x02*2+1)
    #define _WDATA_GET_ACKBIT           (__BIT0)
#define REG_HWI2C_RDATA                 (HWI2C_REG_BASE+0x03*2)
#define REG_HWI2C_RDATA_CFG             (HWI2C_REG_BASE+0x03*2+1)
    #define _RDATA_CFG_TRIG             (__BIT0)
    #define _RDATA_CFG_ACKBIT           (__BIT1)
#define REG_HWI2C_INT_CTL               (HWI2C_REG_BASE+0x04*2)
    #define _INT_CTL                    (__BIT0) //write this register to clear int
#define REG_HWI2C_CUR_STATE             (HWI2C_REG_BASE+0x05*2) //For Debug
    #define _CUR_STATE_MSK              (__BIT4|__BIT3|__BIT2|__BIT1|__BIT0)
#define REG_HWI2C_INT_STATUS            (HWI2C_REG_BASE+0x05*2+1) //For Debug
    #define _INT_STARTDET               (__BIT0)
    #define _INT_STOPDET                (__BIT1)
    #define _INT_RXDONE                 (__BIT2)
    #define _INT_TXDONE                 (__BIT3)
    #define _INT_CLKSTR                 (__BIT4)
    #define _INT_SCLERR                 (__BIT5)
    #define _INT_TIMEOUT                (__BIT6)
#define REG_HWI2C_STP_CNT               (HWI2C_REG_BASE+0x08*2)
#define REG_HWI2C_CKH_CNT               (HWI2C_REG_BASE+0x09*2)
#define REG_HWI2C_CKL_CNT               (HWI2C_REG_BASE+0x0A*2)
#define REG_HWI2C_SDA_CNT               (HWI2C_REG_BASE+0x0B*2)
#define REG_HWI2C_STT_CNT               (HWI2C_REG_BASE+0x0C*2)
#define REG_HWI2C_LTH_CNT               (HWI2C_REG_BASE+0x0D*2)
#define REG_HWI2C_TMT_CNT               (HWI2C_REG_BASE+0x0E*2)
#define REG_HWI2C_SCLI_DELAY            (HWI2C_REG_BASE+0x0F*2)
    #define _SCLI_DELAY                 (__BIT2|__BIT1|__BIT0)


//DMA mode
#define REG_HWI2C_DMA_CFG               (HWI2C_REG_BASE+0x20*2)
    #define _DMA_CFG_RESET              (__BIT1)
    #define _DMA_CFG_INTEN              (__BIT2)
    #define _DMA_CFG_MIURST             (__BIT3)
    #define _DMA_CFG_MIUPRI             (__BIT4)
#define REG_HWI2C_DMA_MIU_ADR           (HWI2C_REG_BASE+0x21*2) // 4 bytes
#define REG_HWI2C_DMA_CTL               (HWI2C_REG_BASE+0x23*2)
//    #define _DMA_CTL_TRIG               (__BIT0)
//    #define _DMA_CTL_RETRIG             (__BIT1)
    #define _DMA_CTL_TXNOSTOP           (__BIT5) //miic transfer format, 1: S+data..., 0: S+data...+P
    #define _DMA_CTL_RDWTCMD            (__BIT6) //miic transfer format, 1:read, 0:write
    #define _DMA_CTL_MIUCHSEL           (__BIT7) //0: miu0, 1:miu1
#define REG_HWI2C_DMA_TXR               (HWI2C_REG_BASE+0x24*2)
    #define _DMA_TXR_DONE               (__BIT0)
#define REG_HWI2C_DMA_CMDDAT0           (HWI2C_REG_BASE+0x25*2) // 8 bytes
#define REG_HWI2C_DMA_CMDDAT1           (HWI2C_REG_BASE+0x25*2+1)
#define REG_HWI2C_DMA_CMDDAT2           (HWI2C_REG_BASE+0x26*2)
#define REG_HWI2C_DMA_CMDDAT3           (HWI2C_REG_BASE+0x26*2+1)
#define REG_HWI2C_DMA_CMDDAT4           (HWI2C_REG_BASE+0x27*2)
#define REG_HWI2C_DMA_CMDDAT5           (HWI2C_REG_BASE+0x27*2+1)
#define REG_HWI2C_DMA_CMDDAT6           (HWI2C_REG_BASE+0x28*2)
#define REG_HWI2C_DMA_CMDDAT7           (HWI2C_REG_BASE+0x28*2+1)
#define REG_HWI2C_DMA_CMDLEN            (HWI2C_REG_BASE+0x29*2)
    #define _DMA_CMDLEN_MSK             (__BIT2|__BIT1|__BIT0)
#define REG_HWI2C_DMA_DATLEN            (HWI2C_REG_BASE+0x2A*2) // 4 bytes
#define REG_HWI2C_DMA_TXFRCNT           (HWI2C_REG_BASE+0x2C*2) // 4 bytes
#define REG_HWI2C_DMA_SLVADR            (HWI2C_REG_BASE+0x2E*2)
    #define _DMA_SLVADR_10BIT_MSK       0x3FF //10 bits
    #define _DMA_SLVADR_NORML_MSK       0x7F //7 bits
#define REG_HWI2C_DMA_SLVCFG            (HWI2C_REG_BASE+0x2E*2+1)
    #define _DMA_10BIT_MODE             (__BIT2)
#define REG_HWI2C_DMA_CTL_TRIG          (HWI2C_REG_BASE+0x2F*2)
    #define _DMA_CTL_TRIG               (__BIT0)
#define REG_HWI2C_DMA_CTL_RETRIG        (HWI2C_REG_BASE+0x2F*2+1)
    #define _DMA_CTL_RETRIG             (__BIT0)
#define REG_HWI2C_DMA_RESERVED0         (HWI2C_REG_BASE+0x30*2)

/**************************&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&**********************************/

#endif // _REG_IIC_H_
