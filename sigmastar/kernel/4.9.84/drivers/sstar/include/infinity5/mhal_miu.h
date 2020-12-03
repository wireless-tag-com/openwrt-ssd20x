/*
* mhal_miu.h- Sigmastar
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
#ifndef _MHAL_MIU_H_
#define _MHAL_MIU_H_

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define MIU_MAX_DEVICE              (2)
#define MIU_MAX_GROUP               (8)
#define MIU_MAX_GP_CLIENT           (16)
#define MIU_MAX_TBL_CLIENT          (MIU_MAX_GROUP*MIU_MAX_GP_CLIENT)

#define MIU_PAGE_SHIFT              (13)       // Unit for MIU protect (8KB)
#define MIU_PROTECT_ADDRESS_UNIT    (0x20UL)   // Unit for MIU hitted address
#define MIU_MAX_PROTECT_BLOCK       (4)
#define MIU_MAX_PROTECT_ID          (16)

#define IDNUM_KERNELPROTECT         (16)

#ifndef BIT0
#define BIT0  0x0001UL
#define BIT1  0x0002UL
#define BIT2  0x0004UL
#define BIT3  0x0008UL
#define BIT4  0x0010UL
#define BIT5  0x0020UL
#define BIT6  0x0040UL
#define BIT7  0x0080UL
#define BIT8  0x0100UL
#define BIT9  0x0200UL
#define BIT10 0x0400UL
#define BIT11 0x0800UL
#define BIT12 0x1000UL
#define BIT13 0x2000UL
#define BIT14 0x4000UL
#define BIT15 0x8000UL
#endif

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_MIU_BLOCK_0 = 0,
    E_MIU_BLOCK_1,
    E_MIU_BLOCK_2,
    E_MIU_BLOCK_3,
    E_MIU_BLOCK_NUM,
} MIU_BLOCK_ID;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

MS_BOOL HAL_MIU_GetProtectIdEnVal(MS_U8 u8MiuDev, MS_U8 u8BlockId, MS_U8 u8ProtectIdIndex);
MS_BOOL HAL_MIU_Protect(    MS_U8   u8Blockx,
                            MS_U16  *pu8ProtectId,
                            MS_U32  u32BusStart,
                            MS_U32  u32BusEnd,
                            MS_BOOL bSetFlag);
MS_BOOL HAL_MIU_ParseOccupiedResource(void);
unsigned int HAL_MIU_ProtectDramSize(void);
int HAL_MIU_ClientIdToName(MS_U16 clientId, char *clientName);

#endif // _MHAL_MIU_H_

