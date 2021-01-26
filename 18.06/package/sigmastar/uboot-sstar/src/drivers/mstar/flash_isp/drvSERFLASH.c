/*
* drvSERFLASH.c- Sigmastar
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

#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>


// Common Definition
//#include "MsCommon.h"
//#include "MsVersion.h"
#include "MsDevice.h"
#include "MsTypes.h"
#include "drvSERFLASH.h"
#include "drvDeviceInfo.h"
#include "regSERFLASH.h"
#include "halSERFLASH.h"
//#include "drvBDMA.h"

#define printk	printf

// !!! Uranus Serial Flash Notes: !!!
//  - The clock of DMA & Read via XIU operations must be < 3*CPU clock
//  - The clock of DMA & Read via XIU operations are determined by only REG_ISP_CLK_SRC; other operations by REG_ISP_CLK_SRC only
//  - DMA program can't run on DRAM, but in flash ONLY
//  - DMA from SPI to DRAM => size/DRAM start/DRAM end must be 8-B aligned


//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#define VER_CHECK_HEADER        'M','S','V','C','0','0'
#define SERFLASH_LIB_ID         'F','1'                             // F1: libFLASH.a
#define SERFLASH_INTERFACE_VER  '0','1'
#define SERFLASH_BUILD_VER      '0','0','0','1'
#define CHANGE_LIST_NUM         '0','0','0','9','1','9','9','6'
#define PRODUCT_NAME            'A','E'                             // AE: T2
#define CUSTOMER_NAME           '0'                                 //  0: Mstar
#define DEVELOP_STAGE           'B','L','E','G'
#define OS_VERSION              '0','0'
#define CHECK_SUM               'T'

////////////////////////////////////////////////////////////////////////////////
// Local & Global Variables
////////////////////////////////////////////////////////////////////////////////
//static MSIF_Version _drv_spif_version = {
//    .DDI = { SPIF_DRV_VERSION },
//};

static SERFLASH_Info _SERFLASHInfo;
static SERFLASH_DrvStatus _SERFLASHDrvStatus;

/// Ask 51 to select flash
ms_Mcu_ChipSelect_CB McuChipSelectCB = NULL;
//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
#define DRV_FLASH_MS(x)     (5955 * x)
#define FLASH_WAIT_TIME     (DRV_FLASH_MS(100)*0x200)
#define FLASH_IS_TIMEOUT(x) ((x) ? FALSE : TRUE)

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

extern MS_BOOL HAL_DMA_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer);
extern MS_BOOL HAL_MAP_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer);
MS_BOOL MDrv_MAP_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer)
{
    //MS_U8 *pu8BufAddr = (MS_U8*)MS_PA2KSEG0((MS_U32)pu8Data); // Physical Address to Virtual Address, cache.

    MS_ASSERT( u32FlashSize > 0 );
    MS_ASSERT( u32FlashAddr + u32FlashSize <= _SERFLASHInfo.u32TotalSize );
    //ASSERT( u32Addr%4 == 0 );
    //ASSERT( u32Size%4 == 0 );
    MS_ASSERT( user_buffer != NULL );
    //#ifdef MCU_MIPS
    //MS_ASSERT( user_buffer & (0x80000000) );
    //#endif

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (unsigned int)u32FlashAddr, (int)u32FlashSize, user_buffer));
    return HAL_MAP_Read(u32FlashAddr, u32FlashSize, user_buffer);
}

//-------------------------------------------------------------------------------------------------
/// Get the information of Serial Flash
/// @return the pointer to the driver information
//-------------------------------------------------------------------------------------------------
const SERFLASH_Info *MDrv_SERFLASH_GetInfo(void)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG,
                    printk("MDrv_SERFLASH_GetInfo()\n"
                           "\tu32AccessWidth = %d\n"
                           "\tu32TotalSize   = %d\n"
                           "\tu32SecNum      = %d\n"
                           "\tu32SecSize     = %d\n",
                           (int)(_SERFLASHInfo.u32AccessWidth),
                           (int)(_SERFLASHInfo.u32TotalSize),
                           (int)(_SERFLASHInfo.u32SecNum),
                           (int)(_SERFLASHInfo.u32SecSize)
                           )
                    );

    return &_SERFLASHInfo;
}

//------------------------------------------------------------------------------
/// Description : Show the SERFLASH driver version
/// @param  ppVersion \b OUT: output SERFLASH driver version
/// @return TRUE : succeed
/// @return FALSE : failed
//------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_GetLibVer(const MSIF_Version **ppVersion)
{
//    if (!ppVersion)
//        return FALSE;
//
//    *ppVersion = &_drv_spif_version;

    return TRUE;
}

//------------------------------------------------------------------------------
/// Description : Get Serial Flash driver status
/// @param  pDrvStatus \b OUT: poniter to store the returning driver status
/// @return TRUE : succeed
/// @return FALSE : failed to get the driver status
//------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_GetStatus(SERFLASH_DrvStatus* pDrvStatus)
{
    memcpy(pDrvStatus, &_SERFLASHDrvStatus, sizeof(_SERFLASHDrvStatus));

    return TRUE;
}

//------------------------------------------------------------------------------
/// Description : Set detailed level of Parallel Flash driver debug message
/// @param u8DbgLevel    \b IN  debug level for Serial Flash driver
/// @return TRUE : succeed
/// @return FALSE : failed to set the debug level
//------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_SetDbgLevel(MS_U8 u8DbgLevel)
{
    _u8SERFLASHDbgLevel = u8DbgLevel;

    return TRUE;
}

//------------------------------------------------------------------------------
/// Description : HK ask 8051 to select flash chip by call back function
/// @param ms_Mcu_ChipSelect_CB    \b IN  call back function
/// @return TRUE : succeed
/// @return NULL :
//------------------------------------------------------------------------------
void MDrv_SERFLASH_SetMcuCSCallBack(ms_Mcu_ChipSelect_CB ChipSel_cb)
{
    McuChipSelectCB = ChipSel_cb;
}

//-------------------------------------------------------------------------------------------------
/// Description : Detect flash type by reading the MID and DID
/// @return TRUE : succeed
/// @return FALSE : unknown flash type
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_DetectType(void)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    return HAL_SERFLASH_DetectType();
}

//-------------------------------------------------------------------------------------------------
/// Description : Detect flash Size
/// @param  u32FlashSize    \b OUT: u32 ptr to store flash size
/// @return TRUE : succeed
/// @return FALSE : unknown flash size
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_DetectSize(MS_U32 *u32FlashSize)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    return HAL_SERFLASH_DetectSize(u32FlashSize);
}

//-------------------------------------------------------------------------------------------------
/// Description : Enable Flash 2XREAD mode, if support
/// @param  b2XMode    \b IN: ENABLE/DISABLE
/// @return TRUE : succeed
/// @return FALSE : not succeed
/// @note   Please ref. sprc. to confirm Flash support or not
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_Set2XRead(MS_BOOL b2XMode)
{
    MS_BOOL Ret = FALSE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
    Ret = HAL_SERFLASH_Set2XREAD(b2XMode);
    return Ret;
}

//-------------------------------------------------------------------------------------------------
/// Description : Set ckg_spi which flash supports (please ref. the spec. before using this function)
/// @param  SPI_DrvCKG    \b IN: enumerate the ckg_spi
/// @return TRUE : succeed
/// @return FALSE : not succeed
/// @note   Please ref. sprc. to confirm Flash support or not. It is safty to run at 43M (Default).
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_SetCKG(SPI_DrvCKG eCKGspi)
{
    MS_BOOL Ret = FALSE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
    Ret = HAL_SERFLASH_SetCKG(eCKGspi);
    return Ret;
}

//-------------------------------------------------------------------------------------------------
/// Description : Set clock div such that spi clock = mcu clock /clock_div.
/// @param  SPI_DrvClkDiv    \b IN: enumerate the clock_div
/// @return TRUE : succeed
/// @return FALSE : not succeed
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_SERFLASH_ClkDiv(SPI_DrvClkDiv eClkDivspi)
{
	DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
	HAL_SERFLASH_ClkDiv(eClkDivspi);
}

//-------------------------------------------------------------------------------------------------
/// Description : Set XIU/RIU mode (Default : XIU)
/// @param  bXiuRiu    \b IN: 1 for XIU, 0 for RIU
/// @return TRUE : succeed
/// @return FALSE : not succeed
/// @note   XIU mode is faster than RIU mode. It is stable to run by XIU (Default)
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_SetMode(MS_BOOL bXiuRiu)
{
    MS_BOOL Ret = FALSE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
    Ret = HAL_SERFLASH_SetMode(bXiuRiu);
    return Ret;
}

//-------------------------------------------------------------------------------------------------
/// Description :  Set active flash among multi-spi flashes
/// @param  u8FlashIndex    \b IN: The Flash index, 0 for external #1 spi flash, 1 for external #2 spi flash
/// @return TRUE : succeed
/// @return FALSE : not succeed
/// @note   For Secure booting = 0, please check hw_strapping or e-fuse (the board needs to jump)
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_ChipSelect(MS_U8 u8FlashIndex)
{
    MS_BOOL Ret = FALSE;
    MS_ASSERT((u8FlashIndex < 4));
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
    Ret = HAL_SERFLASH_ChipSelect(u8FlashIndex);
    return Ret;
}

//-------------------------------------------------------------------------------------------------
/// Description : Initialize Serial Flash
/// @return None
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
void MDrv_SERFLASH_Init(void)
{
    _u8SERFLASHDbgLevel = E_SERFLASH_DBGLV_INFO; // init debug level first         //SERFLASH_DBGLV_DEBUG

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

#if 0
    //
    //  1. HAL init
    //
    MS_U32 u32PMBank=0, u32PMBankSize=0;
    MS_U32 u32NonPMBank=0, u32NonPMBankSize=0;
    MS_U32 u32FlashBank0=0, u32FlashBank0Size=0;

    if (!MDrv_MMIO_GetBASE( &u32PMBank, &u32PMBankSize, MS_MODULE_ISP))
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("IOMap failure to get DRV_MMIO_NONPM_BANK\n"));
    }

    if (!MDrv_MMIO_GetBASE( &u32NonPMBank, &u32NonPMBankSize, MS_MODULE_MHEG5))
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("IOMap failure to get DRV_MMIO_NONPM_BANK\n"));
    }

    if (!MDrv_MMIO_GetBASE( &u32FlashBank0, &u32FlashBank0Size, MS_MODULE_FLASH))
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("IOMap failure to get DRV_MMIO_NONPM_BANK\n"));
    }
#endif

    HAL_SERFLASH_Config();

    HAL_SERFLASH_Init();

    HAL_SERFLASH_DetectType();

    //
    //  2. init SERFLASH_Info
    //
    _SERFLASHInfo.u32AccessWidth = 1;
    _SERFLASHInfo.u32SecNum      = NUMBER_OF_SERFLASH_SECTORS;
    _SERFLASHInfo.u32SecSize     = SERFLASH_SECTOR_SIZE;
    //_SERFLASHInfo.u32TotalSize   = (NUMBER_OF_SERFLASH_SECTORS * SERFLASH_SECTOR_SIZE);
    HAL_SERFLASH_DetectSize(&_SERFLASHInfo.u32TotalSize);
    //
    //  3. init other data structure of Serial Flash driver
    //
    _SERFLASHDrvStatus.bIsBusy = FALSE;
}


//-------------------------------------------------------------------------------------------------
/// Description : Erase all sectors in Serial Flash
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_EraseChip(void)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
    return HAL_SERFLASH_EraseChip();
}


//-------------------------------------------------------------------------------------------------
/// Description : Get flash start block index of a flash address
/// @param  u32FlashAddr    \b IN: flash address
/// @param  pu32BlockIndex    \b IN: poniter to store the returning block index
/// @return TRUE : succeed
/// @return FALSE : illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_AddressToBlock(MS_U32 u32FlashAddr, MS_U32 *pu32BlockIndex)
{
    MS_ASSERT(u32FlashAddr < _SERFLASHInfo.u32TotalSize);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %p)\n", __FUNCTION__, (int)u32FlashAddr, pu32BlockIndex));

    return HAL_SERFLASH_AddressToBlock(u32FlashAddr, pu32BlockIndex);
}


//-------------------------------------------------------------------------------------------------
/// Description : Get flash start address of a block index
/// @param  u32BlockIndex    \b IN: block index
/// @param  pu32FlashAddr    \b IN: pointer to store the returning flash address
/// @return TRUE : succeed
/// @return FALSE : illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_BlockToAddress(MS_U32 u32BlockIndex, MS_U32 *pu32FlashAddr)
{
    MS_ASSERT(u32BlockIndex < _SERFLASHInfo.u32SecNum);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %p)\n", __FUNCTION__, (int)u32BlockIndex, pu32FlashAddr));

    return HAL_SERFLASH_BlockToAddress(u32BlockIndex, pu32FlashAddr);
}


//-------------------------------------------------------------------------------------------------
/// Description : Erase certain sectors given starting address and size in Serial Flash
/// @param  u32StartAddr    \b IN: start address at block boundry
/// @param  u32EraseSize    \b IN: size to erase
/// @param  bWait    \b IN: wait write done or not
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_AddressErase(MS_U32 u32StartAddr, MS_U32 u32EraseSize, MS_BOOL bWait)
{
    MS_U32  u32StartBlock;
    MS_U32  u32EndBlock;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08x, 0x%08x, %d)\n", __FUNCTION__,
            (unsigned int)u32StartAddr, (unsigned int)u32EraseSize, (int)bWait));

    if (   FALSE == MDrv_SERFLASH_AddressToBlock(u32StartAddr, &u32StartBlock)
        || FALSE == MDrv_SERFLASH_AddressToBlock(u32StartAddr + u32EraseSize - 1, &u32EndBlock)
        )
    {
        return FALSE;
    }

    //return MDrv_FSP_AddressErase(u32StartAddr, u32EraseSize, E_FSP_ERASE_64K);
    return MDrv_SERFLASH_BlockErase(u32StartBlock, u32EndBlock, bWait);
}


//-------------------------------------------------------------------------------------------------
/// Description : Erase certain sectors in Serial Flash
/// @param  u32StartBlock    \b IN: start block
/// @param  u32EndBlock    \b IN: end block
/// @param  bWait    \b IN: wait write done or not
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_BlockErase(MS_U32 u32StartBlock, MS_U32 u32EndBlock, MS_BOOL bWait)
{
    MS_ASSERT( u32StartBlock<=u32EndBlock && u32EndBlock<NUMBER_OF_SERFLASH_SECTORS );

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08x, 0x%08x, %d)\n", __FUNCTION__, (unsigned int)u32StartBlock, (unsigned int)u32EndBlock, (int)bWait));
    return HAL_SERFLASH_BlockErase(u32StartBlock, u32EndBlock, bWait);
}

//-------------------------------------------------------------------------------------------------
/// Description : Erase certain 4K sectors in Serial Flash
/// @param  u32StartBlock    \b IN: start address
/// @param  u32EndBlock    \b IN: end address
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_SectorErase(MS_U32 u32StartAddr, MS_U32 u32EndAddr)
{
    MS_U32 u32I = 0;
    MS_BOOL bRet = FALSE;
    if(u32StartAddr%0x1000)
    {
        printk("\nAddress 0x%x is not 4K aligned.\n", (unsigned int)u32StartAddr);
        return FALSE;
    }

    if(_bNon4kErase)
    {
        MS_U8 u8buffer[E_FSP_ERASE_64K];
        MS_U32 u32BlockStart;
        MS_U32 u32Buffer1Size;
        MS_U32 u32Buffer2Offset, u32Buffer2Size;

        /*  |           |               |           |
            | buffer1   | erase area    | buffer2   |
            |           |               |           |*/
        if( u32EndAddr-u32StartAddr+1>E_FSP_ERASE_64K )
            return FALSE;


        u32BlockStart = u32StartAddr - u32StartAddr%E_FSP_ERASE_64K;
        u32Buffer1Size = u32StartAddr%E_FSP_ERASE_64K;
        u32Buffer2Offset = u32Buffer1Size + (u32EndAddr-u32StartAddr+1);
        u32Buffer2Size = E_FSP_ERASE_64K-u32Buffer1Size-(u32EndAddr-u32StartAddr+1);
        /*
        printf("\nErase 4K with buffering\n");
        printf("u32StartAddr:0x%08x \nu32EndAddr:0x%08x\n",
            (unsigned int)u32StartAddr,
            (unsigned int)u32EndAddr);

        printf("u32BlockStart:0x%08x \nu32Buffer1Size:0x%08x\n",
            (unsigned int)u32BlockStart,
            (unsigned int)u32Buffer1Size);
        printf("u32Buffer2Offset:0x%08x \nu32Buffer2Size:0x%08x\n\n",
            (unsigned int)u32Buffer2Offset,
            (unsigned int)u32Buffer2Size);
            */

        bRet = MDrv_SERFLASH_Read(u32BlockStart, E_FSP_ERASE_64K, u8buffer);
        if ( bRet ==  FALSE)
            return bRet;

        bRet = MDrv_SERFLASH_AddressErase(u32BlockStart, E_FSP_ERASE_64K, 1);
        if ( bRet ==  FALSE)
            return bRet;

        //write back buffer1
        if(u32Buffer1Size)
            bRet = HAL_SERFLASH_Write(u32BlockStart, u32Buffer1Size, u8buffer);

        if ( bRet ==  FALSE)
            return bRet;

        //write back buffer2
        if(u32Buffer2Size)
            bRet = HAL_SERFLASH_Write(u32BlockStart+u32Buffer2Offset, u32Buffer2Size, &u8buffer[u32Buffer2Offset]);

    }
    else
    {
        for( u32I = u32StartAddr; u32I < u32EndAddr; u32I+=0x1000)
        {
            bRet = HAL_SERFLASH_SectorErase(u32I);
            if ( bRet ==  FALSE)
            {
                printk("%s Failed at (0x%08X)\n", __FUNCTION__, (int)u32I);
                return bRet;
            }
            
        }
    }
    return bRet;
}

//-------------------------------------------------------------------------------------------------
/// Description : Check write done in Serial Flash
/// @return TRUE : done
/// @return FALSE : not done
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_CheckWriteDone(void)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
    return HAL_SERFLASH_CheckWriteDone();
}


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
MS_BOOL MDrv_SERFLASH_Write(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer)
{
    //MS_U8 *pu8BufAddr = (MS_U8*)MS_PA2KSEG1((MS_U32)pu8Data); // Physical Address to Virtual Address, non-cache.
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (unsigned int)u32FlashAddr, (int)u32FlashSize, user_buffer));

    MS_ASSERT( u32FlashAddr + u32FlashSize <= _SERFLASHInfo.u32TotalSize );
    //MS_ASSERT( u32Addr%4 == 0 );
    //MS_ASSERT( u32Size%4 == 0 );
    MS_ASSERT( user_buffer != NULL );

    //#ifdef MCU_MIPS
    //MS_ASSERT( pu8Data & (0x80000000) );
    //#endif

    //return MDrv_FSP_Write(u32FlashAddr, u32FlashSize, user_buffer);
    return HAL_SERFLASH_Write(u32FlashAddr, u32FlashSize, user_buffer);
}


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
MS_BOOL MDrv_SERFLASH_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer)
{
    //MS_U8 *pu8BufAddr = (MS_U8*)MS_PA2KSEG0((MS_U32)pu8Data); // Physical Address to Virtual Address, cache.

    MS_ASSERT( u32FlashSize > 0 );
    MS_ASSERT( u32FlashAddr + u32FlashSize <= _SERFLASHInfo.u32TotalSize );
    //ASSERT( u32Addr%4 == 0 );
    //ASSERT( u32Size%4 == 0 );
    MS_ASSERT( user_buffer != NULL );
    //#ifdef MCU_MIPS
    //MS_ASSERT( user_buffer & (0x80000000) );
    //#endif

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (unsigned int)u32FlashAddr, (int)u32FlashSize, user_buffer));

    //return MDrv_FSP_Read(u32FlashAddr, u32FlashSize, user_buffer);
    //return HAL_MAP_Read(u32FlashAddr, u32FlashSize, user_buffer);
    //return HAL_DMA_Read(u32FlashAddr, u32FlashSize, user_buffer);
    return HAL_SERFLASH_Read(u32FlashAddr, u32FlashSize, user_buffer);
}

//-------------------------------------------------------------------------------------------------
/// Description : Protect blocks in Serial Flash
/// @param  bEnable    \b IN: TRUE/FALSE: enable/disable protection
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_WriteProtect(MS_BOOL bEnable)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d)\n", __FUNCTION__, (int)bEnable));
    return HAL_SERFLASH_WriteProtect(bEnable);
}


//-------------------------------------------------------------------------------------------------
/// Description : Enables all range of flash write protection
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_WriteProtect_Enable_All_Range(void)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    return HAL_SERFLASH_WriteProtect_Area(TRUE, 0 << 2);
}


//-------------------------------------------------------------------------------------------------
/// Description : Disables all range of flash write protection
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_WriteProtect_Disable_All_Range(void)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
    return MDrv_SERFLASH_WriteProtect_Disable_Range_Set(0, _SERFLASHInfo.u32TotalSize);
}


//-------------------------------------------------------------------------------------------------
/// Description : Set flash disable lower bound and size
/// @param  u32DisableLowerBound    \b IN: the lower bound to disable write protect
/// @param  u32DisableSize    \b IN: size to disable write protect
/// @return TRUE : succeed
/// @return FALSE : fail before timeout or illegal parameters
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_WriteProtect_Disable_Range_Set(MS_U32 u32DisableLowerBound, MS_U32 u32DisableSize)
{
    MS_U32  u32EnableLowerBound;
    MS_U32  u32EnableUpperBound;
    MS_U8   u8BlockProtectBit;

    MS_U32  u32DisableUpperBound;
    MS_U32  u32FlashIndexMax;

    EN_WP_AREA_EXISTED_RTN enWpAreaExistedRtn;


    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    _SERFLASHDrvStatus.bIsBusy = TRUE;

    u32DisableUpperBound = u32DisableLowerBound + u32DisableSize - 1;
    u32FlashIndexMax = _SERFLASHInfo.u32TotalSize - 1;


    if (   u32DisableLowerBound > u32FlashIndexMax
        || u32DisableUpperBound > u32FlashIndexMax
        || u32DisableLowerBound > u32DisableUpperBound
        )
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_INFO, printk(" = FALSE, u32DisableLowerBound(0x%08X), u32DisableUpperBound(0x%08X), u32FlashIndexMax(0x%08X)\n", (int)u32DisableLowerBound, (int)u32DisableUpperBound, (int)u32FlashIndexMax));

        return FALSE;
    }


    // Step 1. decide u32DisableUpperBound // TODO: review, prefer to unprotect the end of the flash
    if (   u32DisableUpperBound != u32FlashIndexMax
        && u32DisableLowerBound != 0
        )
    {
        u32DisableUpperBound = u32FlashIndexMax;
    }


    // Step 2. decide u32EnableLowerBound & u32EnableUpperBound
    if (   u32DisableUpperBound > (u32FlashIndexMax - _SERFLASHInfo.u32SecSize)
        && u32DisableLowerBound == 0
        )
    {
        // i.e. no protect
        u32EnableLowerBound = 0xFFFFFFFF;
        u32EnableUpperBound = 0xFFFFFFFF;
    }
    else if (u32DisableLowerBound == 0)
    {
        u32EnableUpperBound = u32FlashIndexMax;
        u32EnableLowerBound = u32DisableUpperBound + 1;
    }
    else // i.e. (u32DisableUpperBound == u32FlashIndexMax) because of Step 1
    {
        u32EnableUpperBound = u32DisableLowerBound - 1;
        u32EnableLowerBound = 0;
    }


    // Step 3. get u8BlockProtectBit
    enWpAreaExistedRtn = HAL_SERFLASH_WP_Area_Existed(u32EnableUpperBound, u32EnableLowerBound, &u8BlockProtectBit);

    switch (enWpAreaExistedRtn)
    {
    case WP_AREA_NOT_AVAILABLE:
    case WP_TABLE_NOT_SUPPORT:
        u8BlockProtectBit = 0;
        break;

    default:
        /* DO NOTHING */
        break;
    }

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));

    return HAL_SERFLASH_WriteProtect_Area(FALSE, u8BlockProtectBit);
}


//-------------------------------------------------------------------------------------------------
/// Description : Protect blocks in Serial Flash
/// @param  bEnableAllArea    \b IN: enable or disable protection
/// @param  u8BlockProtectBits    \b IN: block protection bits which stand for the area to enable write protect
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_WriteProtect_Area(MS_BOOL bEnableAllArea, MS_U8 u8BlockProtectBits)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d, 0x%02X)\n", __FUNCTION__, (int)bEnableAllArea, u8BlockProtectBits));
    return HAL_SERFLASH_WriteProtect_Area(bEnableAllArea, u8BlockProtectBits);
}


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
MS_BOOL MDrv_SERFLASH_ReadID(MS_U8 *pu8FlashID, MS_U32 u32IDSize)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%p, %d)\n", __FUNCTION__, pu8FlashID, (int)u32IDSize));
    return HAL_SERFLASH_ReadID(pu8FlashID, u32IDSize);
}


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
//MS_BOOL MDrv_SERFLASH_DMA(MS_U32 u32FlashStart, MS_U32 u32DRASstart, MS_U32 u32Size)
//{
//    MS_ASSERT( u32FlashStart+u32Size <= _SERFLASHInfo.u32TotalSize);
//    MS_ASSERT( u32DRASstart%8 ==0 );
//    MS_ASSERT( u32Size%8 ==0 );
//    MS_ASSERT( u32Size>=8 );
//
//    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
//    return HAL_SERFLASH_DMA(u32FlashStart, u32DRASstart, u32Size);
//}


//------- ------------------------------------------------------------------------------------------
/// Description : Read Status Register in Serial Flash
/// @param  pu8StatusReg    \b OUT: ptr to Status Register value
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_ReadStatusRegister(MS_U8 *pu8StatusReg)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    return HAL_SERFLASH_ReadStatusReg(pu8StatusReg);
}

//------- ------------------------------------------------------------------------------------------
/// Description : Read Status Register2 in Serial Flash
/// @param  pu8StatusReg    \b OUT: ptr to Status Register value
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note   For Specific Flash IC with 16-bit status register (high-byte)
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_ReadStatusRegister2(MS_U8 *pu8StatusReg)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    return HAL_SERFLASH_ReadStatusReg2(pu8StatusReg);
}

//------- ------------------------------------------------------------------------------------------
/// Description : Write Status Register in Serial Flash
/// @param  u16StatusReg    \b IN: Status Register value
/// @return TRUE : succeed
/// @return FALSE : fail before timeout
/// @note   Not allowed in interrupt context
/// @note   For Specific Flash IC with 16-bit status register
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SERFLASH_WriteStatusRegister(MS_U16 u16StatusReg)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    return HAL_SERFLASH_WriteStatusReg(u16StatusReg);
}

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
MS_BOOL MDrv_SERFLASH_WriteExtAddrRegister(MS_U8 u8ExtAddrReg)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d)\n", __FUNCTION__, (int)u8ExtAddrReg));
    return HAL_SERFLASH_WriteExtAddrRegister(u8ExtAddrReg);
}

////------- ------------------------------------------------------------------------------------------
///// Description : Handle for BDMA copy data from ONLY Flash src to other dst
///// @param u32FlashAddr \b IN: Physical Source address in spi flash
///// @param u32DramAddr \b IN: Physical Dst address
///// @param u32Len \b IN: data length
///// @param eDstDev \b IN: The Dst Device of Flash BDMA
///// @param u8OpCfg \b IN: u8OpCfg: default is SPIDMA_OPCFG_DEF
///// - Bit0: inverse mode --> SPIDMA_OPCFG_INV_COPY
///// - Bit2: Copy & CRC check in wait mode --> SPIDMA_OPCFG_CRC_COPY
///// - Bit3: Copy without waiting --> SPIDMA_OPCFG_NOWAIT_COPY
///// @return \b MS_BOOL
///// [NONOS_SUPPORT]
///// [fw : drvBDMA ]
////-------------------------------------------------------------------------------------------------
//MS_BOOL MDrv_SERFLASH_CopyHnd(MS_PHYADDR u32FlashAddr,
//                                          MS_PHYADDR u32DstAddr,
//                                          MS_U32 u32Len,
//                                          SPIDMA_Dev eDstDev,
//                                          MS_U8 u8OpCfg)
//{
//    #define BDMA_DEV_FLASH  5
//    MS_U16 CpyType = ((BDMA_DEV_FLASH & 0x0F) | _LShift((eDstDev &0x0F), 8));
//    MS_U32 u32Delay = FLASH_WAIT_TIME;
//
//    while (!HAL_SERFLASH_CheckWriteDone())
//    {
//        if (FLASH_IS_TIMEOUT(u32Delay))
//        {
//            printk("%s() : DMA flash is busy!\n",__FUNCTION__);
//            return FALSE;
//        }
//        u32Delay--;
//    }
//
//    return MDrv_BDMA_CopyHnd(u32FlashAddr, u32DstAddr, u32Len, (BDMA_CpyType) CpyType, u8OpCfg);
//}

//------- ------------------------------------------------------------------------------------------
/// Description : Switch SPI as GPIO Input
/// @param  bSwitch    \b IN: 1 for GPIO, 0 for NORMAL
/// @note   Not allowed in interrupt context
/// @note   For project's power consumption
/// [NONOS_SUPPORT]
//-------------------------------------------------------------------------------------------------
void MDrv_SERFLASH_SetGPIO(MS_BOOL bSwitch)
{
	HAL_SERFLASH_SetGPIO(bSwitch);
}
/*
//-------------------------------------------------------------------------------------------------
//  FSP
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_FSP_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer)
{
	MS_U32 Index;

#define FSP_READ_SIZE	4

	for(Index = 0; Index < u32FlashSize; )
	{
		HAL_SERFLASH_ReadWordFlashByFSP(u32FlashAddr+Index, user_buffer+Index);
		Index += FSP_READ_SIZE;
	}

	return 1;
}

MS_BOOL MDrv_FSP_Write(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer)
{
		MS_U32 Index;
		MS_U32 u32ProgData;

#define FSP_WRITE_SIZE	4

		for(Index = 0; Index < u32FlashSize; )
		{
			u32ProgData = (*(user_buffer + Index))|(*(user_buffer + Index + 1)<<8)|(*(user_buffer + Index + 2)<<16)|(*(user_buffer + Index + 3)<<24);
			HAL_SERFLASH_ProgramFlashByFSP(u32FlashAddr+Index, u32ProgData);
			Index += FSP_WRITE_SIZE;
		}

	return 1;
}

MS_BOOL MDrv_FSP_ReadStatusRegister(MS_U8 *pu8StatusReg)
{
	*pu8StatusReg = HAL_SERFLASH_ReadStatusByFSP();

	return 1;
}

MS_BOOL MDrv_FSP_AddressErase(MS_U32 u32StartAddr, MS_U32 u32EraseSize, E_FSP_ERASE eERASE)
{
    MS_U32 Index;

	switch ( eERASE )
    {
    case E_FSP_ERASE_4K:
		{
			for(Index = 0; Index < u32EraseSize; )
			{
				HAL_SERFLASH_EraseSectorByFSP(u32StartAddr + Index);
				Index += (MS_U32)E_FSP_ERASE_4K;
			}
		}
		break;
    case E_FSP_ERASE_32K:
        {
			for(Index = 0; Index < u32EraseSize; )
			{
				HAL_SERFLASH_EraseBlock32KByFSP(u32StartAddr + Index);
				Index += (MS_U32)E_FSP_ERASE_32K;
			}
		}
        break;
    case E_FSP_ERASE_64K:
        {
			for(Index = 0; Index < u32EraseSize; )
			{
				HAL_SERFLASH_EraseBlock64KByFSP(u32StartAddr + Index);
				Index += (MS_U32)E_FSP_ERASE_64K;
			}
		}
        break;
    default :
		{
			for(Index = 0; Index < u32EraseSize; )
			{
				HAL_SERFLASH_EraseBlock64KByFSP(u32StartAddr + Index);
				Index += (MS_U32)E_FSP_ERASE_64K;
			}
		}
        break;
    }
	return 1;
}
*/
/*internal MUTEX function which must be declared as extern where inline is specified*/
extern MS_S32 MS_SERFLASH_CREATE_MUTEX ( MsOSAttribute eAttribute, char *pMutexName, MS_U32 u32Flag);
extern MS_BOOL MS_SERFLASH_DELETE_MUTEX(MS_S32 s32MutexId);
extern MS_BOOL MS_SERFLASH_OBTAIN_MUTEX (MS_S32 s32MutexId, MS_U32 u32WaitMs);
extern MS_BOOL MS_SERFLASH_RELEASE_MUTEX (MS_S32 s32MutexId);

//MS_BOOL MsOS_In_Interrupt (void)
inline MS_BOOL MS_SERFLASH_IN_INTERRUPT (void)
{
    return FALSE;
}

//MS_S32 MsOS_CreateMutex ( MsOSAttribute eAttribute, char *pMutexName, MS_U32 u32Flag)
inline  MS_S32 MS_SERFLASH_CREATE_MUTEX ( MsOSAttribute eAttribute, char *pMutexName, MS_U32 u32Flag)
{
    return 1;
}

//MS_BOOL MsOS_DeleteMutex (MS_S32 s32MutexId)
inline MS_BOOL MS_SERFLASH_DELETE_MUTEX(MS_S32 s32MutexId)
{
    return TRUE;
}

//MS_BOOL MsOS_ObtainMutex (MS_S32 s32MutexId, MS_U32 u32WaitMs)
inline  MS_BOOL MS_SERFLASH_OBTAIN_MUTEX (MS_S32 s32MutexId, MS_U32 u32WaitMs)
{
    return TRUE;
}

//MS_BOOL MsOS_ReleaseMutex (MS_S32 s32MutexId)
inline MS_BOOL MS_SERFLASH_RELEASE_MUTEX (MS_S32 s32MutexId)
{
    return TRUE;
}
