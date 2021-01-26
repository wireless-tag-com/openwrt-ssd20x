/*
* ms_wdt.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: sylvia.nain <sylvia.nain@sigmastar.com.tw>
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


/*
 * The Watchdog Timer Mode Register can be only written to once. If the
 * timeout need to be set from U-Boot, be sure that the bootstrap doesn't
 * write to this register. Inform Linux to it too
 */
#if 0   // old driver 
#include <common.h>
#include <watchdog.h>
#include "environment.h"
#include "asm/arch/mach/ms_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include "ms_version.h"

/* Hardware timeout in seconds */
#define WDT_HW_TIMEOUT 60

void hw_watchdog_disable(void)
{
    U32 u32Timeout = (U32)CONFIG_WDT_CLOCK * WDT_HW_TIMEOUT;
	OUTREG16(GET_REG_ADDR(REG_ADDR_BASE_WDT, WDT_MAX_PRD_H), ((u32Timeout>>16) & 0x0));
    OUTREG16(GET_REG_ADDR(REG_ADDR_BASE_WDT, WDT_MAX_PRD_L), (u32Timeout & 0x0));
}


void hw_watchdog_reset(void)
{
	OUTREG16(GET_REG_ADDR(REG_ADDR_BASE_WDT, WDT_WDT_CLR), 0x1); //clear
}

void hw_watchdog_init(void)
{
    U32 u32Timeout = (U32)CONFIG_WDT_CLOCK * WDT_HW_TIMEOUT;
	printf("[WDT] Enalbe WATCHDOG 60s\n");
	OUTREG16(GET_REG_ADDR(REG_ADDR_BASE_WDT, WDT_MAX_PRD_H), ((u32Timeout>>16) & 0xFFFF));
    OUTREG16(GET_REG_ADDR(REG_ADDR_BASE_WDT, WDT_MAX_PRD_L), (u32Timeout & 0xFFFF));
	hw_watchdog_reset();
}
#else   // porting from kernel wdt driver 
#include "MsTypes.h"
#include <common.h>

#define BOOL    MS_BOOL

#define WDT_WDT_CLR             0x00*2
#define WDT_DUMMY_REG_1         0x01*2
#define WDT_RST_RSTLEN          0x02*2
#define WDT_INTR_PERIOD         0x03*2
#define WDT_MAX_PRD_L           0x04*2
#define WDT_MAX_PRD_H           0x05*2

#define CLEAR_WDT   0x1
#define WDT_BASE_ADDR 0x1F006000
#define WDT_CLK 12000000

BOOL HAL_WDT_Write2Byte(U32 u32RegAddr, U16 u16Val)
{
    (*(volatile U32*)(WDT_BASE_ADDR+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) = u16Val;
    return TRUE;
}

U16 HAL_WDT_Read2Byte(U32 u32RegAddr)
{
    return (*(volatile U32*)(WDT_BASE_ADDR+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));
}

void wdt_reset(void)
{
    printf("[WDT] reset watchdog \n");
    HAL_WDT_Write2Byte(WDT_WDT_CLR, CLEAR_WDT);
}

void wdt_set_timeout(U32 u32Timeout)
{
    printf("[WDT] set watchdog timeout = %d sec\n",u32Timeout);
    HAL_WDT_Write2Byte(WDT_MAX_PRD_H, (((WDT_CLK*u32Timeout)>>16) & 0xFFFF));
    HAL_WDT_Write2Byte(WDT_MAX_PRD_L, ((WDT_CLK*u32Timeout) & 0xFFFF));
}

void wdt_stop(void)
{
    printf("[WDT] stop watchdog\n");
    HAL_WDT_Write2Byte(WDT_WDT_CLR, 0);
    HAL_WDT_Write2Byte(WDT_MAX_PRD_H, 0x0000);
    HAL_WDT_Write2Byte(WDT_MAX_PRD_L, 0x0000);
}

void wdt_start(U32 u32LaunchTim)
{
    wdt_stop();
    printf("[WDT] start watchdog , launch time=%d sec \n",u32LaunchTim);
    HAL_WDT_Write2Byte(WDT_WDT_CLR, CLEAR_WDT);
    HAL_WDT_Write2Byte(WDT_MAX_PRD_H, (((WDT_CLK*u32LaunchTim)>>16) & 0xFFFF));
    HAL_WDT_Write2Byte(WDT_MAX_PRD_L, ((WDT_CLK*u32LaunchTim) & 0xFFFF));
}


// dump symbol (don't use)
void hw_watchdog_disable(void)
{

}

void hw_watchdog_reset(void)
{

}

void hw_watchdog_init(void)
{

}
#endif

