/*
* cam_clkgen.h- Sigmastar
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

///////////////////////////////////////////////////////////////////////////////
/// @file      cam_clkgen.h
/// @brief     Cam Clk Wrapper Header File for
///            1. RTK OS
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#ifndef __CAM_CLK_GEN_H__
#define __CAM_CLK_GEN_H__

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>

typedef unsigned char       u8;
typedef signed   char       s8;
typedef unsigned short      u16;
typedef signed   short      s16;
typedef unsigned int        u32;
typedef signed   int        s32;
typedef unsigned long long  u64;
typedef signed   long long  s64;



//=============================================================================
// Description:
//      return the rate of clk
// Return:
//      the clk whose rate is being returned
//=============================================================================
unsigned long CamClkGetRate(struct clk *clk);
unsigned long CamClkHWGetNumParents(const struct clk_hw *hw);
const char *CamOfClkGetParentName(struct device_node *np, int index);
unsigned int CamOfClkGetParentCount(struct device_node *np);
struct clk_hw *__CamClkGetHw(struct clk *clk);
long CamClkRoundRate(struct clk *clk, unsigned long rate);
int CamClkSetParent(struct clk *clk, struct clk *parent);
int CamClkSetRate(struct clk *clk, unsigned long rate);
void CamClkDisable(struct clk *clk);
void CamClkUnprepare(struct clk *clk);
int CamClkEnable(struct clk *clk);
int CamClkPrepare(struct clk *clk);

static inline void CamClkDisableUnprepare(struct clk *clk)
{
    CamClkDisable(clk);
    CamClkUnprepare(clk);
}

static inline int CamClkPrepareEnable(struct clk *clk)
{
    int ret;

    ret = CamClkPrepare(clk);

    if (ret)
        return ret;
    ret = CamClkEnable(clk);

    if (ret)
        CamClkUnprepare(clk);
    return ret;
}

unsigned long CamClkHwGetNumParents(const struct clk_hw *hw);
struct clk_hw *CamClkGetParentByIndex(const struct clk_hw *hw, unsigned int index);
#endif /* __CAM_OS_WRAPPER_H__ */
