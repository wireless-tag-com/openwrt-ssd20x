/*
* MsSysUtility.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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

#ifndef __MSSYSUTILITY_C__
#define __MSSYSUTILITY_C__


#include <common.h>
#include <command.h>
#include <MsDebug.h>
#include <MsSysUtility.h>
#include "../drivers/mstar/gpio/drvGPIO.h"
#include "../arch/arm/include/asm/arch-infinity/mach/io.h"
#include "../arch/arm/include/asm/arch-infinity/mach/platform.h"


#ifndef RIUBASEADDR
#define RIUBASEADDR    (0x1F000000)
#endif


MS_U8 ReadByte(MS_U32 u32RegAddr)
{
    if(u32RegAddr & 0x01)
        return INREG8(RIUBASEADDR + ((u32RegAddr&0xFFFF00)<<1) + (((u32RegAddr-1)&0xFF)<<1) + 1);
    else
	    return INREG8(RIUBASEADDR + ((u32RegAddr&0xFFFFFF)<<1));
}

MS_U16 Read2Byte(MS_U32 u32RegAddr)
{
    MS_U16 u16Value = 0;

    if(u32RegAddr & 0x01)
    {
	    u16Value = INREG8(RIUBASEADDR + ((u32RegAddr&0xFFFF00)<<1) + (((u32RegAddr-1)&0xFF)<<1) + 1);
	    u16Value = (u16Value&0xFF) << 8;
	    u16Value += INREG8(RIUBASEADDR + ((u32RegAddr&0xFFFF00)<<1) + (((u32RegAddr-1)&0xFF)<<1));
    }
    else
    {
	    u16Value = INREG8(RIUBASEADDR + ((u32RegAddr&0xFFFFFF)<<1) + 1);
	    u16Value = (u16Value&0xFF) << 8;
	    u16Value += INREG8(RIUBASEADDR + ((u32RegAddr&0xFFFFFF)<<1));
    }

    return u16Value;
}

MS_BOOL WriteByte(MS_U32 u32RegAddr, MS_U8 u8Val)
{
    if(u32RegAddr & 0x01)
        OUTREG8(RIUBASEADDR + ((u32RegAddr&0xFFFF00)<<1) + (((u32RegAddr-1)&0xFF)<<1) + 1, u8Val);
    else
	    OUTREG8(RIUBASEADDR + ((u32RegAddr&0xFFFFFF)<<1), u8Val);

    return TRUE;
}

MS_BOOL Write2Byte(MS_U32 u32RegAddr, MS_U16 u16Val)
{
    if(u32RegAddr & 0x01)
    {
	    OUTREG8(RIUBASEADDR + ((u32RegAddr&0xFFFF00)<<1) + (((u32RegAddr-1)&0xFF)<<1) + 1, (u16Val>>8)&0xFF);
	    OUTREG8(RIUBASEADDR + ((u32RegAddr&0xFFFF00)<<1) + (((u32RegAddr-1)&0xFF)<<1), u16Val&0xFF);
    }
    else
	{
	    OUTREG8(RIUBASEADDR + ((u32RegAddr&0xFFFFFF)<<1) + 1, (u16Val>>8)&0xFF);
	    OUTREG8(RIUBASEADDR + ((u32RegAddr&0xFFFFFF)<<1), u16Val&0xFF);
    }

    return TRUE;
}

MS_BOOL WriteRegBit(MS_U32 u32RegAddr, MS_U16 u16Bit, MS_BOOL bEnable)
{
    MS_U16 u16Value = 0;

	u16Value = Read2Byte(u32RegAddr);
	u16Value = (bEnable) ? (u16Value | u16Bit) : (u16Value & ~u16Bit);
	Write2Byte(u32RegAddr, u16Value);

    return TRUE;
}

MS_BOOL WriteRegBitPos(MS_U32 u32RegAddr, MS_U8 u8Bit, MS_BOOL bEnable)
{
    MS_U16 u16Bit;
    
    if(u8Bit > 15)
    {
        UBOOT_ERROR("Over the bank boundary!\n");
        return FALSE;
    }

    u16Bit = (1 << u8Bit);
    WriteRegBit( u32RegAddr,  u16Bit,  bEnable);

    return TRUE;
}


int do_riu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MS_U32 u32Target = 0;
    MS_U32 u32Value  = 0;
    MS_U8  u8Bit = 0;
	MS_U8  u8Len = 0;
	MS_U8  i = 0;
    
    if (argc < 3)
    {
        cmd_usage(cmdtp);
        return 0;
    }
     
    if(0 == strncmp(argv[1], "wword", 2))
    {
        u32Target = simple_strtoul(argv[2], NULL, 16);
        u32Value = simple_strtoul(argv[3], NULL, 16);
        Write2Byte(u32Target, (MS_U16)u32Value);
        u32Target = simple_strtoul(argv[2], NULL, 16);
        UBOOT_INFO("[%x]:0x%04x\n", (unsigned int)u32Target, (unsigned int)Read2Byte(u32Target));
    }
    else if(0 == strncmp(argv[1], "rword", 2))
    {
        u32Target = simple_strtoul(argv[2], NULL, 16);
		if (NULL != argv[3])
		    u8Len = simple_strtoul(argv[3], NULL, 10);
		else
			u8Len = 1;

		if (1 == u8Len)
		    UBOOT_INFO("[%x]:0x%04x\n", (unsigned int)u32Target,(unsigned int)Read2Byte(u32Target));
		else
		{
            if(u32Target & 0x01)
            {
                printf("%06x: ", (unsigned int)u32Target-1);
                u32Target = u32Target -1;
            }
            else
                printf("%06x: ", (unsigned int)u32Target);

		    for (i=1; i<=u8Len/2; i++)
		    {
		        printf("%04x ", (unsigned int)Read2Byte(u32Target+i*2-1));

				if (0x80 <= ((u32Target+i-1)&0xFF))
				{
					break;
				}

				if (0 == i%4) printf(" ");
				if (0 == i%8) printf("\n%06x: ", (unsigned int)(u32Target+i));
		    }
            printf("\n");
		}
    }
    else if(0 == strncmp(argv[1], "wbyte", 2))
    {
        u32Target = simple_strtoul(argv[2], NULL, 16);
        u32Value = simple_strtoul(argv[3], NULL, 16);
        WriteByte(u32Target, (MS_U8)u32Value);
        UBOOT_INFO("[%x]:0x%x\n", (unsigned int)u32Target, (unsigned int)ReadByte(u32Target));
    }
    else if(0 == strncmp(argv[1], "rbyte", 2))
    {
        u32Target = simple_strtoul(argv[2], NULL, 16);
		if (NULL != argv[3])
		    u8Len = simple_strtoul(argv[3], NULL, 10);
		else
			u8Len = 1;

		if (1 == u8Len)
            UBOOT_INFO("[%x]:0x%02x\n", (unsigned int)u32Target, (unsigned int)ReadByte(u32Target));
		else
		{
		    MS_U32 tmpValue = 0;
            MS_U32 tmpTarget = 0;

            if(u32Target & 0x01)
            {
                printf("%06x: ", (unsigned int)u32Target-1);
                u32Target = u32Target -1;
            }
            else
		        printf("%06x: ", (unsigned int)u32Target);

            for (i=1; i<=u8Len/2; i++)
			{
				tmpTarget = u32Target + i*2 -1;
                tmpValue = ReadByte(tmpTarget);
                tmpValue = tmpValue << 8;
                tmpValue += ReadByte(tmpTarget-1);
                printf("%04x ", (unsigned int)tmpValue);

				if (0xFF == (tmpTarget&0xFF))
				{
					break;
				}

				if (0 == i%4) printf(" ");
				if (0 == i%8) printf("\n%06x: ", (unsigned int)(u32Target+i*2));
		    }
            printf("\n");
		}
    }
    else if(0 == strncmp(argv[1], "bit", 2))
    {
        u32Target = simple_strtoul(argv[2], NULL, 16);
        u8Bit = simple_strtoul(argv[3], NULL, 10);
        u32Value = simple_strtoul(argv[4], NULL, 16);
        WriteRegBitPos(u32Target, u8Bit, (MS_U8)u32Value);
        UBOOT_INFO("[%x]:0x%02x\n", (unsigned int)u32Target, (unsigned int)ReadByte(u32Target));
    }
    else
    {
      cmd_usage(cmdtp);
    }

    return 0;
}

U_BOOT_CMD(
    riu, 5, 0, do_riu,
    "riu  - riu command\n",
    "wword [target: bank+offset Addr(8bit mode)][value]\n"
    "riu rword [target: bank+offset Addr(8bit mode)][(len: default=1)]\n"
    "riu wbyte [target: bank+offset Addr(8bit mode)][value]\n"
    "riu rbyte [target: bank+offset Addr(8bit mode)][(len: default=1)]\n"
    "riu bit   [target: bank+offset Addr(8bit mode)][bit][(1/0)]\n"
);


#ifdef CONFIG_MS_GPIO
int do_gpio( cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");
    MS_GPIO_NUM gpio_num_p1;

    if(3 > argc)
    {
        if((NULL == strcmp(argv[1], "list")) && (2 == argc))
        {
        }
        else
        {
            cmd_usage(cmdtp);
            return -1;
        }
    }

    gpio_num_p1 = (MS_GPIO_NUM)simple_strtoul(argv[2], NULL, 10);

    if((NULL == strcmp(argv[1], "get")) || (NULL == strncmp(argv[1], "inp", 3)))
    {
        MDrv_GPIO_Pad_Set(gpio_num_p1);
        mdrv_gpio_set_input(gpio_num_p1);      
        UBOOT_INFO("IN  ");
        UBOOT_INFO("pin=%d\n", mdrv_gpio_get_level(gpio_num_p1));
    }
    else if(NULL == strncmp(argv[1], "output", 3))
    {
        MDrv_GPIO_Pad_Set(gpio_num_p1);

        if('1' == argv[3][0])
            mdrv_gpio_set_high(gpio_num_p1);
        else if('0' == argv[3][0])
            mdrv_gpio_set_low(gpio_num_p1);

        UBOOT_INFO("gpio[%d] is %d\n", gpio_num_p1, mdrv_gpio_get_level(gpio_num_p1));
    }
    else if(NULL == strncmp(argv[1], "toggle", 3))
    {
        MDrv_GPIO_Pad_Set(gpio_num_p1);

        if(mdrv_gpio_get_level(gpio_num_p1)) 
        {
            mdrv_gpio_set_low(gpio_num_p1);        
        }
        else 
        {    
            mdrv_gpio_set_high(gpio_num_p1);
        }

        UBOOT_INFO("gpio[%d] is %d\n", gpio_num_p1, mdrv_gpio_get_level(gpio_num_p1));
    }
    else if(NULL == strncmp(argv[1], "state", 3))
    {
        if(mdrv_gpio_get_inout(gpio_num_p1)) 
        {
            UBOOT_INFO("IN  ");
        }
        else
        {
            UBOOT_INFO("OUT ");
        }
        UBOOT_INFO("pin=%d\n", mdrv_gpio_get_level(gpio_num_p1));
    }
    else if(NULL == strncmp(argv[1], "list", 3))
    {
        U8 i;
        U8 size;

        if(2 == argc)
        {
            size = 200;
        }
        else
        {
            size = (MS_GPIO_NUM)simple_strtoul(argv[2], NULL, 10);
        }
        
        for(i=0; i<size; i++)
        {
            UBOOT_INFO("GPIO %3d ", i);
            gpio_num_p1 = i;
        
            if(mdrv_gpio_get_inout(gpio_num_p1)) 
            {
                UBOOT_INFO("IN  ");
            }
            else
            {
                UBOOT_INFO("Out ");
            }
            UBOOT_INFO("pin=%d\n", mdrv_gpio_get_level(gpio_num_p1));

            if((i>0) && (i%10==0)) UBOOT_INFO("\n");
        }

    }
    else
    {
        cmd_usage(cmdtp);
        return -1;
    }

    UBOOT_TRACE("OK\n");    
    return 0;
}


U_BOOT_CMD(
    gpio, 4, 0, do_gpio,
    "Config gpio port",
    "(for 2nd parameter, you must type at least 3 characters)\n"
    "gpio output <gpio#> <1/0>  : ex: gpio output 69 1\n"
    "gpio input/get <gpio#>     : ex: gpio input 10  (gpio 10 set as input)\n"
    "gpio toggle <gpio#>        : ex: gpio tog 49 (toggle)\n"
    "gpio state <gpio#>         : ex: gpio sta 49 (get i/o status(direction) & pin status)\n"
    "gpio list [num_of_pins]    : ex: gpio list 10 (list GPIO1~GPIO10 status)\n"
    );

#endif  //#ifdef CONFIG_MS_GPIO

#endif //#define __MSSYSUTILITY_C__
