/*
* hal_clk.c- Sigmastar
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
#include "hal_clk.h"
#include "hal_debug.h"

#define RIU_BASE_ADDR   (0x1F000000)
#define BANK_CAL(addr)  ((addr<<9) + (RIU_BASE_ADDR))

#define BANK_TOP_RESET  (BANK_CAL(0x101e))
#define BANK_TOP_CLK    (BANK_CAL(0x112a))
#define BANK_WD_EN      (BANK_CAL(0x1410))
#define BANK_PLL        (BANK_CAL(0x162e))

#if (HAL_MSG_LEVL < HAL_MSG_DBG)
#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))
#define REGW(base,idx,val)  ms_writew(val,((uint)base+(idx)*4))
#else
#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))
#define REGW(base,idx,val)  do{HAL_MSG(HAL_MSG_DBG, "write 0x%08X = 0x%04X\n", ((uint)base+(idx)*4), val); ms_writew(val,((uint)base+(idx)*4));} while(0)
#endif

/*******************************************************************************************************************
 * clk_hal_init
 *   init device clock
 *
 * Parameters:
 *   RIU_BASE_ADDR:  clock base address
 *
 * Return:
 *   0: OK, othes: failed
 */
//<MStar Software>//CEVA PLL: 600MHz setting
int clk_hal_init(void)
{
    int err_state = 0;
    unsigned short reg_cevapll_pd;
    unsigned short pll_value_low, pll_value_high;

    pll_value_low = REGR(BANK_PLL, 0x60);
    pll_value_high = REGR(BANK_PLL, 0x61);

    HAL_MSG(HAL_MSG_WRN, "reg_pll_value = %04X %04X\n", pll_value_high, pll_value_low);

	// 0x00374BC6 is the default value of pll.
    // Check default value to prevent init pll twice.
    if((pll_value_high != 0x0037) && (pll_value_low != 0x4BC6))
    {
        HAL_MSG(HAL_MSG_WRN, "pll clock is set already (%04X %04X)\n", pll_value_high, pll_value_low);
        return 0;
    }

    reg_cevapll_pd = REGR(BANK_PLL, 0x11);
    reg_cevapll_pd = reg_cevapll_pd & 0x00ff;

    // Set PLL as 500 MHz
    REGW(BANK_PLL, 0x60, 0x978c);
    REGW(BANK_PLL, 0x61, 0x006e);

    REGW(BANK_PLL, 0x62, 0x0001);
    REGW(BANK_PLL, 0x11, reg_cevapll_pd);

    REGW(BANK_WD_EN, 0x50, 0x00c6);
    REGW(BANK_TOP_RESET, 0x24, 0x0005);

    // REGW(BANK_TOP_CLK, 0x20, 0x0000); // 12 MHz
    REGW(BANK_TOP_CLK, 0x20, 0x0001);    // From PLL

    return err_state;
}
