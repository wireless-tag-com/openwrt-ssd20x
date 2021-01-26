/*
* drv_eMMC.h- Sigmastar
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

#ifndef __DRV_eMMC_H__
#define __DRV_eMMC_H__

#ifndef U32
#define U32  unsigned long
#endif
#ifndef U16
#define U16  unsigned short
#endif
#ifndef U8
#define U8   unsigned char
#endif
#ifndef S32
#define S32  signed long
#endif
#ifndef S16
#define S16  signed short
#endif
#ifndef S8
#define S8   signed char
#endif


#ifndef UINT16
#define UINT16 U16
#endif
#ifndef UINT32
#define UINT32 U32
#endif
#ifndef UINT64
#define UINT64 U64
#endif


#ifndef SWAP16
#define SWAP16(x) \
    ((U16)( \
    (((U16)(x) & (U16) 0x00ffU) << 8) | \
    (((U16)(x) & (U16) 0xff00U) >> 8) ))
#endif
#ifndef SWAP32
#define SWAP32(x) \
    ((U32)( \
    (((U32)(x) & (U32) 0x000000ffUL) << 24) | \
    (((U32)(x) & (U32) 0x0000ff00UL) <<  8) | \
    (((U32)(x) & (U32) 0x00ff0000UL) >>  8) | \
    (((U32)(x) & (U32) 0xff000000UL) >> 24) ))
#endif
#ifndef SWAP64
#define SWAP64(x) \
    ((U64)( \
    (U64)(((U64)(x) & (U64) 0x00000000000000ffULL) << 56) | \
    (U64)(((U64)(x) & (U64) 0x000000000000ff00ULL) << 40) | \
    (U64)(((U64)(x) & (U64) 0x0000000000ff0000ULL) << 24) | \
    (U64)(((U64)(x) & (U64) 0x00000000ff000000ULL) <<  8) | \
    (U64)(((U64)(x) & (U64) 0x000000ff00000000ULL) >>  8) | \
    (U64)(((U64)(x) & (U64) 0x0000ff0000000000ULL) >> 24) | \
    (U64)(((U64)(x) & (U64) 0x00ff000000000000ULL) >> 40) | \
    (U64)(((U64)(x) & (U64) 0xff00000000000000ULL) >> 56) ))
#endif

#ifdef BIG_ENDIAN // project team defined macro
#define cpu2le64(x) SWAP64((x))
#define le2cpu64(x) SWAP64((x))
#define cpu2le32(x) SWAP32((x))
#define le2cpu32(x) SWAP32((x))
#define cpu2le16(x) SWAP16((x))
#define le2cpu16(x) SWAP16((x))
#define cpu2be64(x) ((UINT64)(x))
#define be2cpu64(x) ((UINT64)(x))
#define cpu2be32(x) ((UINT32)(x))
#define be2cpu32(x) ((UINT32)(x))
#define cpu2be16(x) ((UINT16)(x))
#define be2cpu16(x) ((UINT16)(x))
#else // Little_Endian
#define cpu2le64(x) ((UINT64)(x))
#define le2cpu64(x) ((UINT64)(x))
#define cpu2le32(x) ((UINT32)(x))
#define le2cpu32(x) ((UINT32)(x))
#define cpu2le16(x) ((UINT16)(x))
#define le2cpu16(x) ((UINT16)(x))
#define cpu2be64(x) SWAP64((x))
#define be2cpu64(x) SWAP64((x))
#define cpu2be32(x) SWAP32((x))
#define be2cpu32(x) SWAP32((x))
#define cpu2be16(x) SWAP16((x))
#define be2cpu16(x) SWAP16((x))
#endif

#define    B16    unsigned short            // 2 byte
#define    BOOL   unsigned int              // 4 byte

#define BOOLEAN BOOL

#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE                           1
/// definition for FALSE
#define FALSE                          0
#endif

#ifndef true
/// definition for true
#define true                           1
/// definition for false
#define false                          0
#endif


/*=============================================================*/
// Include files
/*=============================================================*/
//#include "../config/eMMC_config.h"

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
extern U32  eMMC_LoadImages(U32 *pu32_Addr, U32 *pu32_SectorCnt, U32 u32_ItemCnt);
//--------------------------------------------
// CAUTION: u32_DataByteCnt has to be 512B x n
//--------------------------------------------
extern U32  eMMC_WriteData(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr);
extern U32  eMMC_ReadData(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr);
extern U32  eMMC_EraseBlock(U32 u32_eMMCBlkAddr_start, U32 u32_eMMCBlkAddr_end);
//--------------------------------------------
extern U32  eMMC_GetID(U8 *pu8IDByteCnt, U8 *pu8ID);
extern U32  eMMC_GetDevInfo(eMMC_INFO_t *peMMCInfo_t);

extern U32  eMMC_CheckCIS(eMMC_CIS_t *ptCISData);
extern U32  eMMC_WriteCIS(eMMC_CIS_t *ptCISData);
extern U32  eMMC_ReadCIS(eMMC_CIS_t *ptCISData);

extern U32  eMMC_WritePartitionInfo(U8 *pu8_Data, U32 u32_ByteCnt);
extern U32  eMMC_ReadPartitionInfo(U8 *pu8_Data);
extern U32  eMMC_DumpPartitionInfo(void);

extern U32  eMMC_WritePartition(U16 u16_PartType, 
                                U8 *pu8_DataBuf, 
                                U32 u32_StartPhySector, 
                                U32 u32_SectorCnt,
                                U32 u32_LogicIdx);
extern U32  eMMC_ReadPartition(U16 u16_PartType, 
                                U8 *pu8_DataBuf, 
                                U32 u32_StartPhySector, 
                                U32 u32_SectorCnt,
                                U32 u32_LogicIdx);
extern U32  eMMC_GetPartitionCapacity (U16 u16_PartType, 
                                U32 *pu32_Cap);
extern U32  eMMC_ErasePartition(U16 u16_PartType);
extern U32  eMMC_EraseAllPartitions(void);
extern U32  eMMC_EraseAll(void);
extern U32  eMMC_FATAutoSize(void);

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
extern U32  eMMC_FCIE_BuildDDRTimingTable(void);

//--------------------------------------------
//  #if defined(eMMC_DRV_G2P_UBOOT) && eMMC_DRV_G2P_UBOOT
//extern U32 eMMC_SearchDevNodeStartSector(void);
//  #endif
  
#if defined(eMMC_FCIE_LINUX_DRIVER) && eMMC_FCIE_LINUX_DRIVER
extern U32 eMMC_SearchDevNodeStartSector(void);
#endif
//--------------------------------------------

#define eMMC_LOGI_PART          0x8000 // bit-or if the partition needs Wear-Leveling
#define eMMC_HIDDEN_PART        0x4000 // bit-or if this partition is hidden, normally it is set for the LOGI PARTs.

#define eMMC_PART_HWCONFIG      (1|eMMC_LOGI_PART)
#define eMMC_PART_BOOTLOGO      (2|eMMC_LOGI_PART)
#define eMMC_PART_BL            (3|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_OS            (4|eMMC_LOGI_PART)
#define eMMC_PART_CUS           (5|eMMC_LOGI_PART)
#define eMMC_PART_UBOOT         (6|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_SECINFO       (7|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_OTP           (8|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_RECOVERY      (9|eMMC_LOGI_PART)
#define eMMC_PART_E2PBAK        (10|eMMC_LOGI_PART)
#define eMMC_PART_NVRAMBAK      (11|eMMC_LOGI_PART)
#define eMMC_PART_APANIC        (12|eMMC_LOGI_PART)
#define eMMC_PART_MISC          (14|eMMC_LOGI_PART)


#define eMMC_PART_FDD           (17|eMMC_LOGI_PART)
#define eMMC_PART_TDD           (18|eMMC_LOGI_PART)

#define eMMC_PART_E2P0          (19|eMMC_LOGI_PART)
#define eMMC_PART_E2P1          (20|eMMC_LOGI_PART)
#define eMMC_PART_NVRAM0        (21|eMMC_LOGI_PART)
#define eMMC_PART_NVRAM1        (22|eMMC_LOGI_PART)
#define eMMC_PART_SYSTEM        (23|eMMC_LOGI_PART)
#define eMMC_PART_CACHE         (24|eMMC_LOGI_PART)
#define eMMC_PART_DATA          (25|eMMC_LOGI_PART)
#define eMMC_PART_FAT           (26|eMMC_LOGI_PART)

#define eMMC_PART_ENV           (0x0D|eMMC_LOGI_PART|eMMC_HIDDEN_PART) // uboot env
#define eMMC_PART_DEV_NODE      (0x0F|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_MIU   (0x20|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_EBOOT (0x21|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_RTK (0x22|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_PARAMS (0x23|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_NVRAM     (0x88|eMMC_LOGI_PART) //default start from 31 count

#endif //__DRV_eMMC_H__

