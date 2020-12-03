/*
* mdrv_spinand_hal.h- Sigmastar
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
BOOL MDrv_SPINAND_IsActive(void);
BOOL MDrv_SPINAND_Init(SPINAND_FLASH_INFO_t *tSpinandInfo);
BOOL MDrv_SPINAND_ForceInit(SPINAND_FLASH_INFO_t *tSpinandInfo);
U8 MDrv_SPINAND_ReadID(U16 u16Size, U8 *u8Data);
U32 MDrv_SPINAND_Read(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf);
U32 MDrv_SPINAND_Write(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf);
U32 MDrv_SPINAND_BLOCK_ERASE(U32 u32_BlkIdx);
U32 MDrv_SPINAND_SetMode(SPINAND_MODE eMode);
U32 MDrv_SPINAND_WriteProtect(BOOL bEnable);
U32 MDrv_SPINAND_Read_RandomIn(U32 u32_PageIdx, U32 u32_Column, U32 u32_Byte, U8 *u8Data);
U32 MDrv_SPINAND_ReadStatusRegister(U8 *u8Status, U8 u8Addr);
void MDrv_SPINAND_Device(struct device *dev);
void _spiNandMain(unsigned int dwSramAddress, unsigned int dwSramSize);

inline U32  MS_SPINAND_CREATE_MUTEX (MsOSAttribute eAttribute, char *pMutexName, U32 u32Flag);
inline BOOL MS_SPINAND_IN_INTERRUPT (void);
inline BOOL MS_SPINAND_DELETE_MUTEX(S32 s32MutexId);
inline BOOL MS_SPINAND_OBTAIN_MUTEX (S32 s32MutexId, U32 u32WaitMs);
inline BOOL MS_SPINAND_RELEASE_MUTEX (S32 s32MutexId);
