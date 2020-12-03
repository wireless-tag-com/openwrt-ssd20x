/*
* mhal_rng_reg.h- Sigmastar
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
/*
* mhal_rng_reg.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: richard.guo <richard.guo@sigmastar.com.tw>
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
#ifndef _MHAL_RNG_REG_H_
#define _MHAL_RNG_REG_H_

// unit: ms< --> ( HZ / 1000 )
#define InputRNGJiffThreshold       (10 * ( HZ / 1000 ))
#define RIU_MAP                     0xFD200000
#define RIU                         ((unsigned short volatile *) RIU_MAP)
#define REG_MIPS_BASE               (0x1D00)
#define MIPS_REG(addr)              RIU[(addr<<1)+REG_MIPS_BASE]
#define REG_RNG_OUT                 0x0e

#endif
