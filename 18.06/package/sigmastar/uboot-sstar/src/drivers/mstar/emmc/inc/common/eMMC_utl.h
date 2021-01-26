/*
* eMMC_utl.h- Sigmastar
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

#ifndef __eMMC_UTL_H__
#define __eMMC_UTL_H__

#include "eMMC.h"


typedef eMMC_PACK0 struct _eMMC_TEST_ALIGN_PACK {

    U8  u8_0;
    U16 u16_0;
    U32 u32_0, u32_1;

} eMMC_PACK1 eMMC_TEST_ALIGN_PACK_t;

extern  U32  eMMC_CheckAlignPack(U8 u8_AlignByteCnt);
extern void  eMMC_dump_mem(unsigned char *buf, int cnt);
extern  U32  eMMC_ComapreData(U8 *pu8_Buf0, U8 *pu8_Buf1, U32 u32_ByteCnt);
extern  U32  eMMC_ChkSum(U8 *pu8_Data, U32 u32_ByteCnt);
extern  U32  eMMC_PrintDeviceInfo(void);
extern  U32  eMMC_CompareCISTag(U8 *tag);
extern  void eMMC_dump_nni(eMMC_NNI_t *peMMCInfo);
extern  void eMMC_dump_pni(eMMC_PNI_t *pPartInfo);
#endif // __eMMC_UTL_H__
