/*
* eMMC_config.h- Sigmastar
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
#ifndef __eMMC_CONFIG_H__
#define __eMMC_CONFIG_H__

#define UNIFIED_eMMC_DRIVER		1

//=====================================================
// select a HW platform:
//   - 1: enable, 0: disable.
//   - only one platform can be 1, others have to be 0.
//   - search and check all [FIXME] if need modify or not
//=====================================================
#if defined(CONFIG_MSTAR_EAGLE)
#define eMMC_DRV_EAGLE_LINUX    1
#endif

#if defined(CONFIG_MSTAR_EDISON)
#define eMMC_DRV_EDISON_LINUX   1
#endif

#if defined(CONFIG_MSTAR_EIFFEL)
#define eMMC_DRV_EIFFEL_LINUX   1
#endif

#if defined(CONFIG_MSTAR_EINSTEIN)
#define eMMC_DRV_EINSTEIN_LINUX   1
#endif

#if defined(CONFIG_MS_EMMC_CEDRIC) || defined(CONFIG_MS_EMMC_CEDRIC_MODULE)
#define eMMC_DRV_CEDRIC_LINUX	 1
#endif

//#if defined(CONFIG_ARCH_CHICAGO)
//#define eMMC_DRV_CHICAGO_LINUX	 1
//#endif
#define eMMC_FCIE_MIU0_MIU1_SEL(dmaaddr){\
        dmaaddr -= MIU0_BUS_ADDR;     \
        REG_FCIE_CLRBIT(FCIE_MIU_DMA_26_16, BIT_MIU1_SELECT);\
}\
//=====================================================
// do NOT edit the following content.
//=====================================================
#if defined(eMMC_DRV_G2P_ROM) && eMMC_DRV_G2P_ROM
  #include "eMMC_g2p_rom.h"
#elif defined(eMMC_DRV_G2P_UBOOT) && eMMC_DRV_G2P_UBOOT
  #include "eMMC_msw8x68_uboot.h"
#elif defined(eMMC_DRV_G2E_UBOOT) && eMMC_DRV_G2E_UBOOT
  #include "eMMC_msw8x68t_uboot.h"
#elif defined(eMMC_DRV_G2P_BL) && eMMC_DRV_G2P_BL
  #include "eMMC_g2p_bl.h"
#elif defined(eMMC_DRV_A3_UBOOT) && eMMC_DRV_A3_UBOOT
  #include "eMMC_a3_uboot.h"
#elif defined(eMMC_DRV_AGATE_UBOOT) && eMMC_DRV_AGATE_UBOOT
  #include "eMMC_agate_uboot.h"
#elif defined(eMMC_DRV_EAGLE_UBOOT) && eMMC_DRV_EAGLE_UBOOT
  #include "eMMC_eagle_uboot.h"
#elif defined(eMMC_DRV_EDISON_UBOOT) && eMMC_DRV_EDISON_UBOOT
  #include "eMMC_edison_uboot.h"
#elif defined(eMMC_DRV_EIFFEL_UBOOT) && eMMC_DRV_EIFFEL_UBOOT
  #include "eMMC_eiffel_uboot.h"
#elif defined(eMMC_DRV_G2P_LINUX) && eMMC_DRV_G2P_LINUX
  #include "eMMC_msw8x68_linux.h"
// [FIXME] add a .h file for your platform
#elif defined(eMMC_DRV_EAGLE_LINUX) && eMMC_DRV_EAGLE_LINUX
  #include "eMMC_eagle_linux.h"
#elif defined(eMMC_DRV_EDISON_LINUX) && eMMC_DRV_EDISON_LINUX
  #include "eMMC_edison_linux.h"
#elif defined(eMMC_DRV_EIFFEL_LINUX) && eMMC_DRV_EIFFEL_LINUX
  #include "eMMC_eiffel_linux.h"
#elif defined(eMMC_DRV_EINSTEIN_LINUX) && eMMC_DRV_EINSTEIN_LINUX
  #include "eMMC_einstein_linux.h"
#elif defined(eMMC_DRV_CEDRIC_LINUX) && eMMC_DRV_CEDRIC_LINUX
  #include "eMMC_cedric_linux.h"
#elif defined(eMMC_DRV_CHICAGO_LINUX) && eMMC_DRV_CHICAGO_LINUX	
  #include "eMMC_chicago_linux.h"
#else
  #error "Error! no platform selected."
#endif

//=====================================================
// misc. do NOT edit the following content.
//=====================================================
#define eMMC_DMA_RACING_PATCH        1
#define eMMC_DMA_PATCH_WAIT_TIME     DELAY_10ms_in_us
#define eMMC_DMA_RACING_PATTERN0     (((U32)'M'<<24)|((U32)0<<16)|((U32)'S'<<8)|(U32)1)
#define eMMC_DMA_RACING_PATTERN1     (((U32)'T'<<24)|((U32)6<<16)|((U32)'A'<<8)|(U32)8)

//===========================================================
// Time Dalay, do NOT edit the following content
//===========================================================
#if defined(eMMC_UPDATE_FIRMWARE) && (eMMC_UPDATE_FIRMWARE)
#define TIME_WAIT_DAT0_HIGH            (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_FCIE_RESET           (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_FCIE_RST_TOGGLE_CNT  (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_FIFOCLK_RDY          (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_CMDRSP_END           (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_1_BLK_END            (HW_TIMER_DELAY_1s*5)
#define TIME_WAIT_n_BLK_END            (HW_TIMER_DELAY_1s*10) // safe for 512 blocks
#else
#define TIME_WAIT_DAT0_HIGH            (HW_TIMER_DELAY_1s*60*10) //10*60 s
#define TIME_WAIT_ERASE_DAT0_HIGH      (HW_TIMER_DELAY_1s*60*10) //10*60 s
#define TIME_WAIT_FCIE_RESET           HW_TIMER_DELAY_500ms
#define TIME_WAIT_FCIE_RST_TOGGLE_CNT  HW_TIMER_DELAY_1us
#define TIME_WAIT_FIFOCLK_RDY          HW_TIMER_DELAY_500ms
#define TIME_WAIT_CMDRSP_END           HW_TIMER_DELAY_1s
#define TIME_WAIT_1_BLK_END            (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_n_BLK_END            (HW_TIMER_DELAY_1s*2) // safe for 512 blocks
#endif




#endif /* __eMMC_CONFIG_H__ */
