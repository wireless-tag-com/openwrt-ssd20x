/*
* drvSPINAND_config.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: edie.chen <edie.chen@sigmastar.com.tw>
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

#ifndef __SPINAND_CONFIG_H__
#define __SPINAND_CONFIG_H__

//=====================================================
// select a HW platform:
//   - 1: enable, 0: disable.
//   - only one platform can be 1, others have to be 0.
//=====================================================
#define SPINAND_DRV_TV_MBOOT           1


//=====================================================
// do NOT edit the following content.
//=====================================================
#include <config.h>
#include "../MsTypes.h"

#if defined(SPINAND_DRV_TV_MBOOT) && SPINAND_DRV_TV_MBOOT
  #include <config.h>
#else
  #error "Error! no platform selected."
#endif
//	extern void flush_cache(U32 start_addr, U32 size);

#include "drvSPINAND_uboot.h"



#endif /* __UNFD_CONFIG_H__ */
