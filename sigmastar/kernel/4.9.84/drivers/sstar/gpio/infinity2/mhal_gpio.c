/*
* mhal_gpio.c- Sigmastar
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
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
//#include "MsCommon.h"
//#include <linux/autoconf.h>

#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/of_irq.h>
#include "mhal_gpio.h"
#include "mhal_gpio_reg.h"
#include "ms_platform.h"
#include "gpio.h"
#include "irqs.h"
#include <linux/irqdomain.h>

#ifndef MS_ASSERT

#define MS_ASSERT(expr) do { \
	if(!(expr)) \
			printk("MVD assert fail %s %d!\n", __FILE__, __LINE__); \
} while(0)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define _CONCAT( a, b )     a##b
#define CONCAT( a, b )      _CONCAT( a, b )

// Dummy
#define GPIO999_OEN     0, 0
#define GPIO999_OUT     0, 0
#define GPIO999_IN      0, 0


#include "GPIO_TABLE.h"

U32 gPadTop1BaseAddr=0xFD205200;
U32 gChipBaseAddr=0xFD203C00;
U32 gPmSleepBaseAddr=0xFD001C00;
U32 gSarBaseAddr=0xFD002800;
U32 gRIUBaseAddr=0xFD000000;

#define MHal_PADTOP1_REG(addr)             (*(volatile U8*)(gChipBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_CHIPTOP_REG(addr)             (*(volatile U8*)(gChipBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_PM_SLEEP_REG(addr)            (*(volatile U8*)(gPmSleepBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_SAR_GPIO_REG(addr)            (*(volatile U8*)(gSarBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_RIU_REG(addr)                 (*(volatile U8*)(gRIUBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//the functions of this section set to initialize
void MHal_GPIO_Init(void)
{
//    printk("MHal_GPIO_Init gBaseAddr=%x\n",gPadBaseAddr);
    MHal_PADTOP1_REG(REG_ALL_PAD_IN) &= ~BIT0;
}

void MHal_CHIPTOP_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_CHIPTOP_REG(u32Reg) |= u8BitMsk;
    else
        MHal_CHIPTOP_REG(u32Reg) &= (~u8BitMsk);
}

U8 MHal_CHIPTOP_ReadRegBit(U32 u32Reg, U8 u8BitMsk)
{
    return ((MHal_CHIPTOP_REG(u32Reg)&u8BitMsk)? 1 : 0);
}

U8 MHal_CHIPTOP_ReadRegMsk(U32 u32Reg, U8 u8BitMsk)
{
    return (MHal_CHIPTOP_REG(u32Reg)&u8BitMsk);
}

void MHal_PM_SLEEP_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_PM_SLEEP_REG(u32Reg) |= u8BitMsk;
    else
        MHal_PM_SLEEP_REG(u32Reg) &= (~u8BitMsk);
}

U8 MHal_PM_SLEEP_ReadRegBit(U32 u32Reg, U8 u8BitMsk)
{
    return ((MHal_PM_SLEEP_REG(u32Reg)&u8BitMsk)? 1 : 0);
}

U8 MHal_PM_SLEEP_ReadRegMsk(U32 u32Reg, U8 u8BitMsk)
{
    return (MHal_PM_SLEEP_REG(u32Reg)&u8BitMsk);
}

void MHal_SAR_GPIO_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_SAR_GPIO_REG(u32Reg) |= u8BitMsk;
    else
        MHal_SAR_GPIO_REG(u32Reg) &= (~u8BitMsk);
}

void MHal_FuartPAD_DisableFunction(void)
{
	printk("TBD MHal_FuartPAD_DisableFunction\n");
}

void MHal_SPI0PAD_DisableFunction(void)
{
	printk("TBD MHal_FuartPAD_DisableFunction\n");
}

int MHal_GPIO_PadVal_Set(U16 u16IndexGPIO, U32 u32PadMode)
{
	printk("TBD MHal_GPIO_PadVal_Set\n");
	return 0;
}
int MHal_GPIO_PadGroupMode_Set(U32 u32PadMode)
{
	printk("TBD MHal_GPIO_PadGroupMode_Set\n");
	return 0;
}

void MHal_GPIO_Pad_Set(U16 u16IndexGPIO)
{
//    printk("[gpio]MHal_GPIO_Pad_Set %d\n", u16IndexGPIO);
    switch(u16IndexGPIO)
    {
		case PAD_SAR_GPIO0:
				MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE, DISABLE, BIT0);
				break;
		case PAD_SAR_GPIO1:
				MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE, DISABLE, BIT1);
				break;
		case PAD_SAR_GPIO2:
				MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE, DISABLE, BIT2);
				break;
		case PAD_SAR_GPIO3:
				MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE, DISABLE, BIT3);
				break;
		case PAD_SAR_GPIO4:
				MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE, DISABLE, BIT4);
				break;
            default:
                break;
    }
}

void MHal_GPIO_Pad_Oen(U16 u16IndexGPIO)
{
	if (u16IndexGPIO >= 0 && u16IndexGPIO < GPIO_NR)
	{
            MHal_RIU_REG(gpio_table[u16IndexGPIO].r_oen) &= (~gpio_table[u16IndexGPIO].m_oen);
	}
	else
	{
            MS_ASSERT(0);
	}
}

void MHal_GPIO_Pad_Odn(U16 u16IndexGPIO)
{
	if (u16IndexGPIO >= 0 && u16IndexGPIO < GPIO_NR)
	{
            MHal_RIU_REG(gpio_table[u16IndexGPIO].r_oen) |= gpio_table[u16IndexGPIO].m_oen;
	}
	else
	{
            MS_ASSERT(0);
	}
}

U16 MHal_GPIO_Pad_Level(U16 u16IndexGPIO)
{

	if (u16IndexGPIO >= 0 && u16IndexGPIO < GPIO_NR)
	{
		return ((MHal_RIU_REG(gpio_table[u16IndexGPIO].r_in)&gpio_table[u16IndexGPIO].m_in)? 1 : 0);
	}
	else
	{
		MS_ASSERT(0);
		return -1;
	}
}

U16 MHal_GPIO_Pad_InOut(U16 u16IndexGPIO)
{
	if (u16IndexGPIO >= 0 && u16IndexGPIO < GPIO_NR)
	{
		return ((MHal_RIU_REG(gpio_table[u16IndexGPIO].r_oen)&gpio_table[u16IndexGPIO].m_oen)? 1 : 0);
	}
	else
	{
		MS_ASSERT(0);
		return -1;
	}
}

void MHal_GPIO_Pull_High(U16 u16IndexGPIO)
{
	if (u16IndexGPIO >= 0 && u16IndexGPIO < GPIO_NR)
	{
    	MHal_RIU_REG(gpio_table[u16IndexGPIO].r_out) |= gpio_table[u16IndexGPIO].m_out;
	}
	else
	{
		MS_ASSERT(0);
	}
}

void MHal_GPIO_Pull_Low(U16 u16IndexGPIO)
{
	if (u16IndexGPIO >= 0 && u16IndexGPIO < GPIO_NR)
	{
    	MHal_RIU_REG(gpio_table[u16IndexGPIO].r_out) &= (~gpio_table[u16IndexGPIO].m_out);
	}
	else
	{
		MS_ASSERT(0);
	}
}

void MHal_GPIO_Set_High(U16 u16IndexGPIO)
{
	if (u16IndexGPIO >= 0 && u16IndexGPIO < GPIO_NR)
	{
	    MHal_RIU_REG(gpio_table[u16IndexGPIO].r_oen) &= (~gpio_table[u16IndexGPIO].m_oen);
	    MHal_RIU_REG(gpio_table[u16IndexGPIO].r_out) |= gpio_table[u16IndexGPIO].m_out;
	}
	else
	{
		MS_ASSERT(0);
	}
}

void MHal_GPIO_Set_Low(U16 u16IndexGPIO)
{
	if (u16IndexGPIO >= 0 && u16IndexGPIO < GPIO_NR)
	{
	    MHal_RIU_REG(gpio_table[u16IndexGPIO].r_oen) &= (~gpio_table[u16IndexGPIO].m_oen);
	    MHal_RIU_REG(gpio_table[u16IndexGPIO].r_out) &= (~gpio_table[u16IndexGPIO].m_out);
	}
	else
	{
		MS_ASSERT(0);
	}
}

void MHal_Enable_GPIO_INT(U16 u16IndexGPIO)
{
/*     switch(u8IndexGPIO)
    {
  case PAD_GPIO7:
        MHal_CHIPTOP_WriteRegBit(1,DISABLE,BIT7);
        MHal_CHIPTOP_WriteRegBit(2,ENABLE,BIT0);
        break;
    case PAD_GPIO8:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT1);
        break;
    case PAD_GPIO9:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT2);
        break;
    case PAD_GPIO13:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT3);
        break;
    case PAD_GPIO28:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT4);
        break;
    case PAD_GPIO29:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_21,DISABLE,BIT2);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT5);
        break;
    case PAD_GPIO30:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_20,DISABLE,BIT5);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_02,DISABLE,BIT3);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_20,DISABLE,BIT4);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT6);
        break;
    case PAD_GPIO31:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_20,DISABLE,BIT5);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_02,DISABLE,BIT3);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_20,DISABLE,BIT4);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT7);
        break;
	default:
	break;
    }*/
}
static int PMSLEEP_GPIO_To_Irq(U16 u16IndexGPIO)
{
    //256 is PMSLEEP virtual irq start

    if(u16IndexGPIO >= PAD_PM_GPIO0 && u16IndexGPIO <= PAD_PM_GPIO15)
        return (u16IndexGPIO  - (PAD_PM_GPIO0));

    else if(u16IndexGPIO >= PAD_PM_GPIO16 && u16IndexGPIO <= PAD_PM_GPIO19)
    {   
        return (u16IndexGPIO - PAD_PM_GPIO15);
    }
    else if(u16IndexGPIO == PAD_PM_IRIN)
        return INT_PMSLEEP_GPIO_20_IR;

    else if(u16IndexGPIO == PAD_PM_CEC)
        return INT_PMSLEEP_GPIO_22_CEC;

    else if(u16IndexGPIO == PAD_PM_SD30_CDZ) 
        return INT_PMSLEEP_GPIO_78_PAD_PM_SD30_CDZ;

    else if(u16IndexGPIO == PAD_PM_SD20_CDZ)
        return INT_PMSLEEP_GPIO_79_PAD_PM_SD20_CDZ;

    else if(u16IndexGPIO >= PAD_VID0 && u16IndexGPIO <= PAD_PM_LED1)
    {   
        return  (INT_PMSLEEP_GPIO_72_PAD_VID0 + (u16IndexGPIO - PAD_VID0));
    }
   else
        return -1;
}
int MHal_GPIO_To_Irq(U16 u16IndexGPIO)
{
    struct device_node *pm_intr_node;
    struct irq_domain *pm_intr_domain;
    struct irq_fwspec fwspec;
    int hwirq, virq = -1;

    hwirq = PMSLEEP_GPIO_To_Irq(u16IndexGPIO);

    if( hwirq >= 0)
    {
        //get virtual irq number for request_irq
        pm_intr_node = of_find_compatible_node(NULL, NULL, "sstar,pm-intc");
        pm_intr_domain = irq_find_host(pm_intr_node);

        if(!pm_intr_domain)
            return -ENXIO;

        fwspec.param_count = 1;
        fwspec.param[0] = hwirq;
        fwspec.fwnode = of_node_to_fwnode(pm_intr_node);
        virq = irq_create_fwspec_mapping(&fwspec);
    }
    else
    {
        printk("convert fail\r\n");
    }
    return virq;
}

void MHal_GPIO_Set_POLARITY(U16 u16IndexGPIO,U8 reverse)
{
//printk("TBD MHal_GPIO_Set_POLARITY\n");
/*
IdxGPIO  GPIOx  IdxFIQ
70  --GPIO31 -- 63 -- ext_gpio_int[7]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
71  --GPIO30 -- 58 -- ext_gpio_int[6]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
72  --GPIO29 -- 57 -- ext_gpio_int[5]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
73  -- GPIO28 -- 56 -- ext_gpio_int[4]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
113 --GPIO13 -- 55 -- ext_gpio_int[3]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
117 --GPIO9   -- 47 -- ext_gpio_int[2]  -- reg_hst0_fiq_polarity_47_32_  -- [h00a,h00a]
118 --GPIO8   -- 43 -- ext_gpio_int[1]  -- reg_hst0_fiq_polarity_47_32_  -- [h00a,h00a]
119 --GPIO7   -- 39 -- ext_gpio_int[0]  -- reg_hst0_fiq_polarity_47_32_  -- [h00a,h00a]
*/

    switch(u16IndexGPIO)
    {
/*    case 119:  //INT_FIQ_EXT_GPIO0
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 7); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 7); //Set To Raising edge trigger
        break;
    case 118:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 11); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 11); //Set To Raising edge trigger
        break;
    case 117:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 15); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 15); //Set To Raising edge trigger
        break;
    case 113:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 7); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 7); //Set To Raising edge trigger
        break;
    case 73:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 8); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 8); //Set To Raising edge trigger
        break;
    case 72:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 9); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 9); //Set To Raising edge trigger
        break;
    case 71:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 10); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 10); //Set To Raising edge trigger
        break;
    case 70:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 15); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 15); //Set To Raising edge trigger
        break;
*/
    default:
        break;
    }
}



