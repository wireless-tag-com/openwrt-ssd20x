/*
* drvSERFLASH.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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

#ifndef _DRV_SERFLASH_H_
#define _DRV_SERFLASH_H_

#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define MSIF_SPIF_LIB_CODE                     {'S','P','I','F'}    //Lib code
#define MSIF_SPIF_LIBVER                       {'0','1'}            //LIB version
#define MSIF_SPIF_BUILDNUM                     {'0','1'}            //Build Number
#define MSIF_SPIF_CHANGELIST                   {'0','0','1','1','4','5','3','4'} //P4 ChangeList Number

//-------------------------------------------------------------------------------------------------
//  Version String Definition
//-------------------------------------------------------------------------------------------------
// Move from MsVersion.h
#define MSIF_TAG                    {'M','S','I','F'}                   // MSIF
#define MSIF_CLASS                  {'0','0'}                           // DRV/API (DDI)
#define MSIF_CUS                    0x0000                              // Sstar Common library
#define MSIF_MOD                    0x0000                              // Sstar Common library
#define MSIF_CHIP                   0x000F
#define MSIF_CPU                    '1'
#define MSIF_OS                     '0'

#define      SPIF_DRV_VERSION                  /* Character String for DRV/API version             */  \
        MSIF_TAG,                         /* 'MSIF'                                           */  \
        MSIF_CLASS,                       /* '00'                                             */  \
        MSIF_CUS,                         /* 0x0000                                           */  \
        MSIF_MOD,                         /* 0x0000                                           */  \
        MSIF_CHIP,                                                                              \
        MSIF_CPU,                                                                               \
        MSIF_SPIF_LIB_CODE,                    /* IP__                                             */  \
        MSIF_SPIF_LIBVER,                      /* 0.0 ~ Z.Z                                        */  \
        MSIF_SPIF_BUILDNUM,                    /* 00 ~ 99                                          */  \
        MSIF_SPIF_CHANGELIST,                  /* CL#                                              */  \
        MSIF_OS

/// Operation cfg
#define SPIDMA_CFG_ADDR_DIR_BIT   (0)
#define SPIDMA_CFG_REFLECT_BIT    (1)
#define SPIDMA_CFG_CRCCOPY_BIT    (2)
#define SPIDMA_CFG_NOWAITCOPY_BIT (3)

#define SPIDMA_OPCFG_DEF          (0)
#define SPIDMA_OPCFG_INV_COPY     _LShift(1, SPIDMA_CFG_ADDR_DIR_BIT)
#define SPIDMA_OPCFG_CRC_REFLECT  _LShift(1, SPIDMA_CFG_REFLECT_BIT)    //bit reflection of each input byte
#define SPIDMA_OPCFG_CRC_COPY     _LShift(1, SPIDMA_CFG_CRCCOPY_BIT)    //copy then crc check
#define SPIDMA_OPCFG_NOWAIT_COPY  _LShift(1, SPIDMA_CFG_NOWAITCOPY_BIT) //copy then quit

#ifndef MS_ASSERT
#define MS_ASSERT(condition)    //
#endif
#define DEBUG_SER_FLASH(debug_level, x)     do { if (_u8SERFLASHDbgLevel >= (debug_level)) (x); } while(0)
//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
/// Serial Flash information structure
typedef struct
{
    MS_U32 u32AccessWidth;      //data access width in bytes
    MS_U32 u32TotalSize;        //total size in bytes
    MS_U32 u32SecNum;           //number of sectors
    MS_U32 u32SecSize;          //sector size in bytes
} SERFLASH_Info;

typedef struct
{
    MS_BOOL bIsBusy;
} SERFLASH_DrvStatus;

typedef enum
{
    E_SERFLASH_DBGLV_NONE,    //disable all the debug message
    E_SERFLASH_DBGLV_INFO,    //information
    E_SERFLASH_DBGLV_NOTICE,  //normal but significant condition
    E_SERFLASH_DBGLV_WARNING, //warning conditions
    E_SERFLASH_DBGLV_ERR,     //error conditions
    E_SERFLASH_DBGLV_CRIT,    //critical conditions
    E_SERFLASH_DBGLV_ALERT,   //action must be taken immediately
    E_SERFLASH_DBGLV_EMERG,   //system is unusable
    E_SERFLASH_DBGLV_DEBUG    //debug-level messages
} SERFLASH_DbgLv;

typedef enum _SPIDMA_Dev
{
    E_SPIDMA_DEV_MIU0,
    E_SPIDMA_DEV_MIU1,
    E_SPIDMA_DEV_DMDMCU = 6,
    E_SPIDMA_DEV_VDMCU,
    E_SPIDMA_DEV_DSP,
    E_SPIDMA_DEV_TSP,
    E_SPIDMA_DEV_1KSRAM_HK51,
    E_SPIDMA_DEV_NOT_SUPPORT
}SPIDMA_Dev;

typedef enum _SPI_DrvCKG
{
    E_SPI_XTALI = 0,
    E_SPI_27M,
    E_SPI_36M,
    E_SPI_43M,
    E_SPI_54M,
    E_SPI_72M,
    E_SPI_86M,
    E_SPI_108M,
    E_SPI_24M = 15, // T3 only
    E_SPI_HALCKG_NOT_SUPPORT
}SPI_DrvCKG;

typedef enum _SPI_DrvClkDiv
{
     E_SPI_DIV2
    ,E_SPI_DIV4
    ,E_SPI_DIV8
    ,E_SPI_DIV16
    ,E_SPI_DIV32
    ,E_SPI_DIV64
    ,E_SPI_DIV128
    ,E_SPI_ClkDiv_NOT_SUPPORT
}SPI_DrvClkDiv;

typedef enum _E_FSP_ERASE
{
    E_FSP_ERASE_4K  = 0x1000,
    E_FSP_ERASE_32K = 0x8000,
    E_FSP_ERASE_64K = 0x10000,
}E_FSP_ERASE;

///SPI CS callback
typedef void (*ms_Mcu_ChipSelect_CB)(void);

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/// Description : Get the information of Serial Flash
/// @return the pointer to the driver information
//-------------------------------------------------------------------------------------------------
const SERFLASH_Info *MDrv_SERFLASH_GetInfo(void);

//------------------------------------------------------------------------------
/// Description : Show the SERFLASH driver version
/// @param  ppVersion    \b OUT: output NORF driver version
/// @return TRUE : succeed
/// @return FALSE : failed
//------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_GetLibVer(const MSIF_Version **ppVersion);

//------------------------------------------------------------------------------
/// Description : Get Serial Flash driver status
/// @param  pDrvStatus    \b OUT: poniter to store the returning driver status
/// @return TRUE : succeed
/// @return FALSE : failed to get the driver status
//------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_GetStatus(SERFLASH_DrvStatus* pDrvStatus);

//------------------------------------------------------------------------------
/// Description : Set detailed level of Parallel Flash driver debug message
/// @param u8DbgLevel    \b IN  debug level for Serial Flash driver
/// @return TRUE : succeed
/// @return FALSE : failed to set the debug level
//------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_SetDbgLevel(MS_U8 u8DbgLevel);

//-------------------------------------------------------------------------------------------------
/// Description : Detect flash type by reading the MID and DID
/// @return TRUE : succeed
/// @return FALSE : unknown flash type
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_DetectType(void);

//-------------------------------------------------------------------------------------------------
/// Description : Detect flash Size
/// @return TRUE : succeed
/// @return FALSE : unknown flash size
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern MS_BOOL MDrv_SERFLASH_DetectSize(MS_U32 *u32FlashSize);

//-------------------------------------------------------------------------------------------------
/// Description : Enable Flash 2XREAD mode, if support
/// @param  b2XMode    \b IN: ENABLE/DISABLE
/// @return TRUE : succeed
/// @return FALSE : not succeed
/// @note   Please ref. sprc. to confirm Flash support or not
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_Set2XRead(MS_BOOL b2XMode);

//-------------------------------------------------------------------------------------------------
/// Description : Set ckg_spi which flash supports (please ref. the spec. before using this function)
/// @param  SPI_DrvCKG    \b IN: enumerate the ckg_spi
/// @return TRUE : succeed
/// @return FALSE : not succeed
/// @note   Please ref. sprc. to confirm Flash support or not. It is safty to run at 43M (Default).
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_SetCKG(SPI_DrvCKG eCKGspi);

//-------------------------------------------------------------------------------------------------
/// Description : Set XIU/RIU mode (Default : XIU)
/// @param  bXiuRiu    \b IN: 1 for XIU, 0 for RIU
/// @return TRUE : succeed
/// @return FALSE : not succeed
/// @note   XIU mode is faster than RIU mode. It is stable to run by XIU (Default)
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_SetMode(MS_BOOL bXiuRiu);

//-------------------------------------------------------------------------------------------------
/// Description :  Set active flash among multi-spi flashes
/// @param  u8FlashIndex    \b IN: The Flash index, 0 for external #1 spi flash, 1 for external #2 spi flash
/// @return TRUE : succeed
/// @return FALSE : not succeed
/// @note   For Secure booting = 0, please check hw_strapping or e-fuse  (the board needs to jump)
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_ChipSelect(MS_U8 u8FlashIndex);

//-------------------------------------------------------------------------------------------------
/// Initialize Serial Flash
/// @return None
//-------------------------------------------------------------------------------------------------
extern void MDrv_SERFLASH_Init(void);

//-------------------------------------------------------------------------------------------------
/// Description : Read ID from Serial Flash
/// @param  pu8FlashID    \b OUT: Virtual data ptr to store the read ID
/// @param  u32IDSize    \b IN: size in Bytes
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
extern MS_BOOL MDrv_SERFLASH_ReadID(MS_U8 *pu8FlashID, MS_U32 u32IDSize);

//-------------------------------------------------------------------------------------------------
/// Description : Read data from Serial Flash
/// @param  u32FlashAddr    \b IN: Flash Address
/// @param  u32FlashSize    \b IN: Flash Size Data in Bytes
/// @param  user_buffer    \b OUT: Virtual Buffer Address ptr to store flash read data
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer);

//-------------------------------------------------------------------------------------------------
/// Description : Erase all sectors in Serial Flash
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern MS_BOOL MDrv_SERFLASH_EraseChip(void);

//-------------------------------------------------------------------------------------------------
/// Description : Get flash start block index of a flash address
/// @param  u32FlashAddr    \b IN: flash address
/// @param  pu32BlockIndex    \b IN: poniter to store the returning block index
/// @return TRUE : succeed
/// @return FALSE : illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_AddressToBlock(MS_U32 u32FlashAddr, MS_U32 *pu32BlockIndex);

//-------------------------------------------------------------------------------------------------
/// Description : Get flash start address of a block index
/// @param  u32BlockIndex    \b IN: block index
/// @param  pu32FlashAddr    \b IN: pointer to store the returning flash address
/// @return TRUE : succeed
/// @return FALSE : illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_BlockToAddress(MS_U32 u32BlockIndex, MS_U32 *pu32FlashAddr);

//-------------------------------------------------------------------------------------------------
/// Description : Erase certain sectors given starting address and size in Serial Flash
/// @param  u32StartAddr    \b IN: start address at block boundry
/// @param  u32EraseSize    \b IN: size to erase
/// @param  bWait    \b IN: wait write done or not
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern MS_BOOL MDrv_SERFLASH_AddressErase(MS_U32 u32StartAddr, MS_U32 u32EraseSize, MS_BOOL bWait);

//-------------------------------------------------------------------------------------------------
/// Description : Erase certain sectors in Serial Flash
/// @param  u32StartBlock    \b IN: start block
/// @param  u32EndBlock    \b IN: end block
/// @param  bWait    \b IN: wait write done or not
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_BlockErase(MS_U32 u32StartBlock, MS_U32 u32EndBlock, MS_BOOL bWait);

//-------------------------------------------------------------------------------------------------
/// Description : Erase certain 4K sectors in Serial Flash
/// @param  u32StartBlock    \b IN: start address
/// @param  u32EndBlock    \b IN: end address
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_SectorErase(MS_U32 u32StartAddr, MS_U32 u32EndAddr);

//-------------------------------------------------------------------------------------------------
/// Description : Check write done in Serial Flash
/// @return TRUE : done
/// @return FALSE : not done
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_CheckWriteDone(void);

//-------------------------------------------------------------------------------------------------
/// Description : Write data to Serial Flash
/// @param  u32FlashAddr    \b IN: start address (4-B aligned)
/// @param  u32FlashSize    \b IN: size in Bytes (4-B aligned)
/// @param  user_buffer    \b IN: Virtual Buffer Address ptr to flash write data
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_Write(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer);

//-------------------------------------------------------------------------------------------------
/// Description : Read data from Serial Flash to DRAM in DMA mode
/// @param  u32FlashStart    \b IN: src start address in flash (0 ~ flash size-1)
/// @param  u32DRASstart    \b IN: dst start address in DRAM (16B-aligned) (0 ~ DRAM size-1)
/// @param  u32Size    \b IN: size in Bytes (8B-aligned) (>=8)
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_DMA(MS_U32 u32FlashStart, MS_U32 u32DRASstart, MS_U32 u32Size);

//-------------------------------------------------------------------------------------------------
/// Description : Protect blocks in Serial Flash
/// @param  bEnable    \b IN: TRUE/FALSE: enable/disable protection
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern MS_BOOL MDrv_SERFLASH_WriteProtect(MS_BOOL bEnable);

//-------------------------------------------------------------------------------------------------
/// Description : Enables all range of flash write protection
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_WriteProtect_Enable_All_Range(void);

//-------------------------------------------------------------------------------------------------
/// Description : Disables all range of flash write protection
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern MS_BOOL MDrv_SERFLASH_WriteProtect_Disable_All_Range(void);

//-------------------------------------------------------------------------------------------------
/// Description : Set flash disable lower bound and size
/// @param  u32DisableLowerBound    \b IN: the lower bound to disable write protect
/// @param  u32DisableSize    \b IN: size to disable write protect
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern MS_BOOL MDrv_SERFLASH_WriteProtect_Disable_Range_Set(MS_U32 u32DisableLowerBound, MS_U32 u32DisableSize);

//-------------------------------------------------------------------------------------------------
/// Description : Protect blocks in Serial Flash
/// @param  bEnableAllArea    \b IN: enable or disable protection
/// @param  u8BlockProtectBits    \b IN: block protection bits which stand for the area to enable write protect
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
extern MS_BOOL MDrv_SERFLASH_WriteProtect_Area(MS_BOOL bEnableAllArea, MS_U8 u8BlockProtectBits);

//------- ------------------------------------------------------------------------------------------
/// Description : Read Status Register in Serial Flash
/// @param  pu8StatusReg    \b OUT: ptr to Status Register value
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_ReadStatusRegister(MS_U8 *pu8StatusReg);

//------- ------------------------------------------------------------------------------------------
/// Description : Read Status Register2 in Serial Flash
/// @param  pu8StatusReg    \b OUT: ptr to Status Register value
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note   For Specific Flash IC with 16-bit status register (high-byte)
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_ReadStatusRegister2(MS_U8 *pu8StatusReg);

//------- ------------------------------------------------------------------------------------------
/// Description : Write Status Register in Serial Flash
/// @param  u16StatusReg    \b IN: Status Register value
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note   For Specific Flash IC with 16-bit status register
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_WriteStatusRegister(MS_U16 u16StatusReg);

//-------------------------------------------------------------------------------------------------
/// Description : Write Extention Address Register in Serial Flash
/// @param  u8ExtAddrReg    \b IN: Extended Address Register value
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note   For Specific Flash IC with size over then 128Mb and support EAR mode
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_WriteExtAddrRegister(MS_U8 u8ExtAddrReg);

//------- ------------------------------------------------------------------------------------------
/// Description : Handle for BDMA copy data from ONLY Flash src to other dst
/// @param u32FlashAddr \b IN: Physical Source address in spi flash
/// @param u32DramAddr \b IN: Physical Dst address
/// @param u32Len \b IN: data length
/// @param eDstDev \b IN: The Dst Device of Flash BDMA
/// @param u8OpCfg \b IN: u8OpCfg: default is SPIDMA_OPCFG_DEF
/// - Bit0: inverse mode --> SPIDMA_OPCFG_INV_COPY
/// - Bit2: Copy & CRC check in wait mode --> SPIDMA_OPCFG_CRC_COPY
/// - Bit3: Copy without waiting --> SPIDMA_OPCFG_NOWAIT_COPY
/// @return \b MS_BOOL
/// [NONOS_SUPPORT]
/// [fw : drvBDMA ]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_CopyHnd(MS_PHYADDR u32FlashAddr, MS_PHYADDR u32DstAddr, MS_U32 u32Len, SPIDMA_Dev eDstDev, MS_U8 u8OpCfg);

//------- ------------------------------------------------------------------------------------------
/// Description : Switch SPI as GPIO Input
/// @param  bSwitch    \b IN: 1 for GPIO, 0 for NORMAL
/// @note   Not allowed in interrupt context
/// @note   For project's power consumption
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
void MDrv_SERFLASH_SetGPIO(MS_BOOL bSwitch);

//-------------------------------------------------------------------------------------------------
//  WRAPPER FOR CHAKRA
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_FLASH_Write(MS_U32 u32FlashAddr,MS_U32 u32FlashSize,MS_U8 * user_buffer);
MS_BOOL MDrv_FLASH_Read(MS_U32 u32FlashAddr,MS_U32 u32FlashSize,MS_U8 * user_buffer);
MS_BOOL MDrv_FLASH_WriteProtect(MS_BOOL bEnable);

MS_BOOL MDrv_FLASH_WriteProtect_Enable_All_Range(void);
MS_BOOL MDrv_FLASH_WriteProtect_Disable_All_Range(void);
MS_BOOL MDrv_FLASH_WriteProtect_Disable_Range_Set(MS_U32 DisableLowerBound, MS_U32 DisableSize);
MS_BOOL MDrv_FLASH_WriteProtect_Area(MS_BOOL bEnableAllArea, MS_U8 BlockProtectBits);
MS_BOOL MDrv_FLASH_ReadStatusRegister(MS_U8 *pu8StatusReg);
MS_BOOL MDrv_FLASH_ReadStatusRegister2(MS_U8 *pu8StatusReg);
MS_BOOL MDrv_FLASH_WriteStatusRegister(MS_U16 u16StatusReg);

MS_BOOL MDrv_FLASH_DetectType(void);
MS_BOOL MDrv_FLASH_DetectSize(MS_U32 *u32FlashSize);
MS_BOOL MDrv_FLASH_AddressToBlock(MS_U32 u32FlashAddr, MS_U32 *pu32BlockIndex);
MS_BOOL MDrv_FLASH_BlockToAddress(MS_U32 u32BlockIndex, MS_U32 *pu32FlashAddr);
MS_BOOL MDrv_FLASH_AddressErase(MS_U32 u32StartAddr,MS_U32 u32EraseSize,MS_BOOL bWait);
MS_BOOL MDrv_FLASH_BlockErase(MS_U16 u16StartBlock,MS_U16 u16EndBlock,MS_BOOL bWait);
MS_BOOL MDrv_FLASH_CheckWriteDone(void);

//Flash Self-Programming (FSP)
//MS_BOOL MDrv_FSP_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer);
//MS_BOOL MDrv_FSP_Write(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer);
//MS_BOOL MDrv_FSP_ReadStatusRegister(MS_U8 *pu8StatusReg);
//MS_BOOL MDrv_FSP_AddressErase(MS_U32 u32StartAddr, MS_U32 u32EraseSize, E_FSP_ERASE eERASE);


extern ms_Mcu_ChipSelect_CB McuChipSelectCB;
#ifdef __cplusplus
}
#endif

#endif // _DRV_SERFLASH_H_
