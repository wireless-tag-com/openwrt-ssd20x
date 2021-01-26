/*
* eMMC_config.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: joe.su <joe.su@sigmastar.com.tw>
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

#ifndef __eMMC_CONFIG_H__
#define __eMMC_CONFIG_H__

#define UNIFIED_eMMC_DRIVER     1
#include <common.h>
//=====================================================
// select a HW platform:
//   - 1: enable, 0: disable.
//   - only one platform can be 1, others have to be 0.
//   - search and check all [FIXME] if need modify or not
//=====================================================
#ifdef CONFIG_ARCH_CEDRIC
#define ENABLE_CEDRIC           1
#elif defined(CONFIG_ARCH_CHICAGO)
#define ENABLE_CHICAGO          1
#elif defined(CONFIG_ARCH_INFINITY3)
#define ENABLE_INFINITY3        1
#elif defined(CONFIG_ARCH_INFINITY5)
#define ENABLE_INFINITY5        1
#elif defined(CONFIG_ARCH_INFINITY6E)
#define ENABLE_INFINITY6E       1
#endif

#if ENABLE_CEDRIC
#define eMMC_DRV_CEDRIC_UBOOT           1
#elif ENABLE_CHICAGO
#define eMMC_DRV_CHICAGO_UBOOT          1
#elif ENABLE_INFINITY3
#define eMMC_DRV_INFINITY3_UBOOT        1
#elif ENABLE_INFINITY5
#define eMMC_DRV_INFINITY5_UBOOT        1
#elif ENABLE_INFINITY6E
#define eMMC_DRV_INFINITY6E_UBOOT       1
#endif


//=====================================================
// do NOT edit the following content.
//=====================================================
#if defined(eMMC_DRV_CEDRIC_UBOOT) && eMMC_DRV_CEDRIC_UBOOT
    #include "eMMC_cedric_uboot.h"
#elif defined(eMMC_DRV_CHICAGO_UBOOT) && eMMC_DRV_CHICAGO_UBOOT
    #include "eMMC_chicago_uboot.h"
#elif defined(eMMC_DRV_INFINITY3_UBOOT) && eMMC_DRV_INFINITY3_UBOOT
    #include "eMMC_infinity3_uboot.h"
#elif defined(eMMC_DRV_INFINITY5_UBOOT) && eMMC_DRV_INFINITY5_UBOOT
    #include "eMMC_infinity5_uboot.h"
#elif defined(eMMC_DRV_INFINITY6E_UBOOT) && eMMC_DRV_INFINITY6E_UBOOT
    #include "eMMC_infinity6e_uboot.h"
#else
  #error "Error! no platform selected."
#endif


//=====================================================
// misc. do NOT edit the following content.
//=====================================================
#define eMMC_DMA_RACING_PATCH           1
#define eMMC_DMA_PATCH_WAIT_TIME        DELAY_10ms_in_us
#define eMMC_DMA_RACING_PATTERN0        (((U32)'M'<<24)|((U32)0<<16)|((U32)'S'<<8)|(U32)1)
#define eMMC_DMA_RACING_PATTERN1        (((U32)'T'<<24)|((U32)6<<16)|((U32)'A'<<8)|(U32)8)

//===========================================================
// Time Dalay, do NOT edit the following content
//===========================================================
#if defined(eMMC_UPDATE_FIRMWARE) && (eMMC_UPDATE_FIRMWARE)
#define TIME_WAIT_DAT0_HIGH             (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_ERASE_DAT0_HIGH       (HW_TIMER_DELAY_1s*60*10) //10*60 s
#define TIME_WAIT_FCIE_RESET            (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_FCIE_RST_TOGGLE_CNT   (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_FIFOCLK_RDY           (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_CMDRSP_END            (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_1_BLK_END             (HW_TIMER_DELAY_1s*5)
#define TIME_WAIT_n_BLK_END             (HW_TIMER_DELAY_1s*10) // safe for 512 blocks
#else
#define TIME_WAIT_DAT0_HIGH             HW_TIMER_DELAY_1s
#define TIME_WAIT_ERASE_DAT0_HIGH       (HW_TIMER_DELAY_1s*60*10) //10*60 s
#define TIME_WAIT_FCIE_RESET            HW_TIMER_DELAY_10ms
#define TIME_WAIT_FCIE_RST_TOGGLE_CNT   HW_TIMER_DELAY_1us
#define TIME_WAIT_FIFOCLK_RDY           HW_TIMER_DELAY_10ms
#define TIME_WAIT_CMDRSP_END            HW_TIMER_DELAY_100ms
#define TIME_WAIT_R_1_BLK_END           HW_TIMER_DELAY_100ms
#define TIME_WAIT_1_BLK_END             HW_TIMER_DELAY_1s
#define TIME_WAIT_n_BLK_END             HW_TIMER_DELAY_1s // safe for 512 blocks
#endif

#endif /* __eMMC_CONFIG_H__ */
