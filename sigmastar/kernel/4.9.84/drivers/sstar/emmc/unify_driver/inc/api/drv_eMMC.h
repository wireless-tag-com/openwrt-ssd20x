/*
* drv_eMMC.h- Sigmastar
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

#ifndef __DRV_eMMC_H__
#define __DRV_eMMC_H__


/*=============================================================*/
// Include files
/*=============================================================*/
#include "../config/eMMC_config.h"

/*=============================================================*/
// Extern definition
/*=============================================================*/
typedef struct _eMMC_INFO
{
    U8  au8_Tag[16];
    U8  u8_IDByteCnt;
    U8  au8_ID[15];
    U32 u32_ChkSum;
    U16 u16_SpareByteCnt;
    U16 u16_PageByteCnt;
    U16 u16_BlkPageCnt;
    U16 u16_BlkCnt;
    U32 u32_Config;
    U16 u16_ECCType;
    U16 u16_SeqAccessTime;
    U8  padding[12];
    U8  au8_Vendor[16];
    U8  au8_PartNumber[16];

    U16 u16_ECCCodeByteCnt;
    U16 u16_PageSectorCnt;
    U8  u8_WordMode;

} eMMC_INFO_t;

typedef struct _eMMC_CIS {

    U8  au8_eMMC_nni[512];
    U8  au8_eMMC_pni[512];

} eMMC_CIS_t;

/*=============================================================*/
// Macro definition
/*=============================================================*/

/*=============================================================*/
// Data type definition
/*=============================================================*/

/*=============================================================*/
// Variable definition
/*=============================================================*/

/*=============================================================*/
// Global function definition
/*=============================================================*/
extern U32  eMMC_Init(void);
extern U32  eMMC_Init_Device(void);
extern U32  eMMC_Init_Device_Ex(void);
extern U32  eMMC_LoadImages(U32 *pu32_Addr, U32 *pu32_SectorCnt, U32 u32_ItemCnt);
//--------------------------------------------
// CAUTION: u32_DataByteCnt has to be 512B x n
//--------------------------------------------
extern U32  eMMC_WriteData_Ex(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr);
extern U32  eMMC_ReadData_Ex(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr);
// skip driver-reserved area
extern U32  eMMC_WriteData(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr);
extern U32  eMMC_ReadData(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr);
extern U32  eMMC_GetCapacity(U32 *pu32_TotalSectorCnt); // 1 sector = 512B
extern U32  eMMC_EraseBlock(U32 u32_eMMCBlkAddr_start, U32 u32_eMMCBlkAddr_end);
//--------------------------------------------
extern U32  eMMC_GetID(U8 *pu8IDByteCnt, U8 *pu8ID);

extern U32  eMMC_EraseAll(void);

extern U32 eMMC_GetExtCSD(U8* pu8_Ext_CSD);
extern U32 eMMC_SetExtCSD(U8 u8_AccessMode, U8 u8_ByteIdx, U8 u8_Value);

/*=============================================================*/
// internal function definition
/*=============================================================*/
extern U32  eMMC_ReadBootPart(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr, U8 u8_PartNo);
extern U32  eMMC_WriteBootPart(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr, U8 u8_PartNo);
extern U32  eMMC_EraseBootPart(U32 u32_eMMCBlkAddr_start, U32 u32_eMMCBlkAddr_end, U8 u8_PartNo);
extern U32  eMMC_CheckIfReady(void);
extern void eMMC_ResetReadyFlag(void);
extern void eMMC_DumpDriverStatus(void);
extern void eMMC_DumpSpeedStatus(void);
extern U32  eMMC_FCIE_BuildDDRTimingTable(void);
extern U32  eMMC_FCIE_BuildHS200TimingTable(void);
extern void eMMC_FCIE_SetSkew4Value(U32 u32Value);
extern void eMMC_FCIE_SetDelayLatch(U32 u32Value);
extern U32  eMMC_SetEnhanceUserPartition(U32 u32_StartAddr,U32 u32_Size,U8 u8_EnAttr, U8 u8_RelW);
extern U32  eMMC_SetGPPartition(U8 u8_PartNo,U32 u32_PartSize,U8 u8_EnAttr,U8 u8_ExtAttr, U8 u8_RelW);
extern U32  eMMC_SetPartitionComplete(void);
extern U32  eMMC_ReadGPPart(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr, U8 u8_PartNo);
extern U32  eMMC_WriteGPPart(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr, U8 u8_PartNo);
extern void eMMC_PrintGPPartition(void);

//--------------------------------------------
  #if defined(eMMC_DRV_G2P_UBOOT) && eMMC_DRV_G2P_UBOOT
extern U32 eMMC_SearchDevNodeStartSector(void);
  #endif
//--------------------------------------------

#endif //__DRV_eMMC_H__

