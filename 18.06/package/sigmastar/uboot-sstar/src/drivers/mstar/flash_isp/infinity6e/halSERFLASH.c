/*
* halSERFLASH.c- Sigmastar
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

//#ifndef UBOOT
//#include <linux/string.h>
//#include <linux/kernel.h>
//#include <linux/delay.h>
//#include <linux/jiffies.h>
//#else
#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#include <watchdog.h>

//#endif

// Common Definition

//#include "MsIRQ.h"
//#include "MsOS.h"
//#include "drvMMIO.h"

// Internal Definition
#include "MsTypes.h"
#include "drvDeviceInfo.h"
#include "drvSERFLASH.h"
#include "regSERFLASH.h"
#include "halSERFLASH.h"


#define printk	printf
#define CONFIG_FLASH_DISPLAY_PROGRESS   1
// !!! Uranus Serial Flash Notes: !!!
//  - The clock of DMA & Read via XIU operations must be < 3*CPU clock
//  - The clock of DMA & Read via XIU operations are determined by only REG_ISP_CLK_SRC; other operations by REG_ISP_CLK_SRC only
//  - DMA program can't run on DRAM, but in flash ONLY
//  - DMA from SPI to DRAM => size/DRAM start/DRAM end must be 8-B aligned


//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define XIUREAD_MODE                0

#if 0

    //Both read and write via IRUISP
    #define CONFIG_RIUISP               1

#else

//Select FSP read function
    //#define CONFIG_FSP_READ_RIUOP      1
    //#define CONFIG_FSP_READ_BDMA       1
    #define CONFIG_FSP_READ_DIRERECT    1

//Select FSP write function
    //#define CONFIG_FSP_WRITE_RIUOP       1
    //#define CONFIG_FSP_WRITE_RIUOP_BRUST 1
    #define CONFIG_FSP_WRITE_BDMA         1

//Select SPI Read Command interface
    //1-1-1 mode (SPI COMMAND is 0x0B)
    #define CONFIG_BURST_READ         1

    //1-1-1 mode (SPI COMMAND is 0x03)
    //#define CONFIG_NORMAL_READ        1

    //1-2-2 mode (SPI COMMAND is 0xBB)
    //#define CONFIG_DUAL_READ          1

    //1-4-4 mode (SPI COMMAND is 0xEB)
    //#define CONFIG_QUAL_READ          1

#endif


#define READ_BYTE(_reg)             (*(volatile MS_U8*)(_reg))
#define READ_WORD(_reg)             (*(volatile MS_U16*)(_reg))
#define READ_LONG(_reg)             (*(volatile MS_U32*)(_reg))
#define WRITE_BYTE(_reg, _val)      {(*((volatile MS_U8*)(_reg))) = (MS_U8)(_val); }
#define WRITE_WORD(_reg, _val)      {(*((volatile MS_U16*)(_reg))) = (MS_U16)(_val); }
#define WRITE_LONG(_reg, _val)      {(*((volatile MS_U32*)(_reg))) = (MS_U32)(_val); }
#define WRITE_WORD_MASK(_reg, _val, _mask)  {(*((volatile MS_U16*)(_reg))) = ((*((volatile MS_U16*)(_reg))) & ~(_mask)) | ((MS_U16)(_val) & (_mask)); }
//#define WRITE_WORD_MASK(_reg, _val, _mask)  {*((volatile MS_U16*)(_reg)) = (*((volatile MS_U16*)(_reg))) & ~(_mask)) | ((MS_U16)(_val) & (_mask); }


// XIU_ADDR
// #define SFSH_XIU_REG32(addr)                (*((volatile MS_U32 *)(_hal_isp.u32XiuBaseAddr + ((addr)<<2))))

//#define SFSH_XIU_READ32(addr)               (*((volatile MS_U32 *)(_hal_isp.u32XiuBaseAddr + ((addr)<<2)))) // TODO: check AEON 32 byte access order issue
//

// ISP_CMD
// #define ISP_REG16(addr)                     (*((volatile MS_U16 *)(_hal_isp.u32IspBaseAddr + ((addr)<<2))))

#define ISP_READ(addr)                      READ_WORD(_hal_isp.u32IspBaseAddr + ((addr)<<2))
#define ISP_WRITE(addr, val)                WRITE_WORD(_hal_isp.u32IspBaseAddr + ((addr)<<2), (val))
#define ISP_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_isp.u32IspBaseAddr + ((addr)<<2), (val), (mask))

#define FSP_READ(addr)                      READ_WORD(_hal_isp.u32FspBaseAddr + ((addr)<<2))
#define FSP_WRITE(addr, val)                WRITE_WORD(_hal_isp.u32FspBaseAddr + ((addr)<<2), (val))
#define FSP_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_isp.u32FspBaseAddr + ((addr)<<2), (val), (mask))

#define QSPI_READ(addr)                     READ_WORD(_hal_isp.u32QspiBaseAddr + ((addr)<<2))
#define QSPI_WRITE(addr, val)               WRITE_WORD(_hal_isp.u32QspiBaseAddr + ((addr)<<2), (val))
#define QSPI_WRITE_MASK(addr, val, mask)    WRITE_WORD_MASK(_hal_isp.u32QspiBaseAddr + ((addr)<<2), (val), (mask))

#define SPI_FLASH_CMD(u8FLASHCmd)           ISP_WRITE(REG_ISP_SPI_COMMAND, (MS_U8)u8FLASHCmd)
#define SPI_WRITE_DATA(u8Data)              ISP_WRITE(REG_ISP_SPI_WDATA, (MS_U8)u8Data)
#define SPI_READ_DATA()                     READ_BYTE(_hal_isp.u32IspBaseAddr + ((REG_ISP_SPI_RDATA)<<2))

#define CHIP_READ(addr)                    READ_WORD(BASE_REG_CHIPTOP_ADDR + (addr<<2))

//#define MHEG5_READ(addr)                    READ_WORD(_hal_isp.u32Mheg5BaseAddr + ((addr)<<2))
//#define MHEG5_WRITE(addr, val)              WRITE_WORD((_hal_isp.u32Mheg5BaseAddr + (addr << 2)), (val))
//#define MHEG5_WRITE_MASK(addr, val, mask)   WRITE_WORD_MASK(_hal_isp.u32Mheg5BaseAddr + ((addr)<<2), (val), (mask))

// PIU_DMA
//#define PIU_READ(addr)                      READ_WORD(_hal_isp.u32PiuBaseAddr + ((addr)<<2))
//#define PIU_WRITE(addr, val)                WRITE_WORD(_hal_isp.u32PiuBaseAddr + ((addr)<<2), (val))
//#define PIU_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_isp.u32PiuBaseAddr + ((addr)<<2), (val), (mask))

//// PM_SLEEP CMD.
//#define PM_READ(addr)                      READ_WORD(_hal_isp.u32PMBaseAddr+ ((addr)<<2))
//#define PM_WRITE(addr, val)                WRITE_WORD(_hal_isp.u32PMBaseAddr+ ((addr)<<2), (val))
//#define PM_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_isp.u32PMBaseAddr+ ((addr)<<2), (val), (mask))

// CLK_GEN
//#define CLK_READ(addr)                     READ_WORD(_hal_isp.u32CLK0BaseAddr + ((addr)<<2))
//#define CLK_WRITE(addr, val)               WRITE_WORD(_hal_isp.u32CLK0BaseAddr + ((addr)<<2), (val))
//#define CLK_WRITE_MASK(addr, val, mask)    WRITE_WORD_MASK(_hal_isp.u32CLK0BaseAddr + ((addr)<<2), (val), (mask))

//MS_U32<->MS_U16
#define LOU16(u32Val)   ((MS_U16)(u32Val))
#define HIU16(u32Val)   ((MS_U16)((u32Val) >> 16))

//serial flash mutex wait time
#define SERFLASH_MUTEX_WAIT_TIME    3000

// Time-out system
extern unsigned long get_timer (unsigned long base);

#define SERFLASH_SAFETY_FACTOR      3000
#define SER_FLASH_TIME(_stamp)           (_stamp = get_timer(0))
#define SER_FLASH_EXPIRE(_stamp,_msec)   (_Hal_GetMsTime(_stamp, _msec))


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct
{
//    MS_U32  u32XiuBaseAddr;     // REG_SFSH_XIU_BASE
//    MS_U32  u32Mheg5BaseAddr;
    MS_U32  u32IspBaseAddr;     // REG_ISP_BASE
    MS_U32  u32FspBaseAddr;     // REG_FSP_BASE
    MS_U32  u32QspiBaseAddr;    // REG_QSPI_BASE
//    MS_U32  u32PiuBaseAddr;     // REG_PIU_BASE
//    MS_U32  u32PMBaseAddr;      // REG_PM_BASE
//    MS_U32  u32CLK0BaseAddr;    // REG_PM_BASE
    MS_U32  u32BdmaBaseAddr;    // for supernova.lite
//    MS_U32  u32RiuBaseAddr;
//    MS_U32  u32PMGPIOBaseAddr;
} hal_isp_t;

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
hal_SERFLASH_t _hal_SERFLASH;
MS_U8 _u8SERFLASHDbgLevel;
MS_BOOL _bXIUMode = 0;      // default XIU mode, set 0 to RIU mode
MS_BOOL bDetect = FALSE;    // initial flasg : true and false
MS_BOOL _bIBPM = FALSE;     // Individual Block Protect mode : true and false
MS_BOOL _bHasEAR = FALSE;   // Extended Address Register mode supported
MS_U8 _u8RegEAR = 0xFF;     // Extended Address Register value
MS_U8 _bNon4kErase = 0;   // use buffer before erase

#if defined CONFIG_RIUISP
    SPI_READ_MODE gReadMode = E_RIUISP_MODE;
#elif defined CONFIG_BURST_READ
    SPI_READ_MODE gReadMode=E_FAST_MODE;
#elif defined CONFIG_NORMAL_READ
    SPI_READ_MODE gReadMode=E_SINGLE_MODE;
#elif defined CONFIG_DUAL_READ
    SPI_READ_MODE gReadMode=E_DUAL_AD_MODE;
#elif defined CONFIG_QUAL_READ
    SPI_READ_MODE gReadMode=E_QUAD_MODE;
#else
#endif



//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static MS_S32 _s32SERFLASH_Mutex;
//
//  Spi  Clk Table (List)
//
/*
static MS_U16 _hal_ckg_spi_pm[] = {
     PM_SPI_CLK_XTALI
    ,PM_SPI_CLK_54MHZ
    ,PM_SPI_CLK_86MHZ
    ,PM_SPI_CLK_108MHZ
};

static MS_U16 _hal_ckg_spi_nonpm[] = {
     CLK0_CKG_SPI_XTALI
    ,CLK0_CKG_SPI_54MHZ
    ,CLK0_CKG_SPI_86MHZ
    ,CLK0_CKG_SPI_108MHZ
};
*/
static hal_isp_t _hal_isp =
{
    //.u32XiuBaseAddr = BASEADDR_XIU,
    //.u32Mheg5BaseAddr = BASEADDR_RIU + BK_MHEG5,
    .u32IspBaseAddr = BASE_REG_ISP_ADDR,
    .u32FspBaseAddr = BASE_REG_FSP_PM_ADDR,
    .u32QspiBaseAddr = BASE_REG_QSPI_PM_ADDR,
    //.u32PiuBaseAddr = BASEADDR_RIU + BK_PIU,
    //.u32PMBaseAddr = BASEADDR_RIU + BK_PMSLP,
    //.u32CLK0BaseAddr = BASEADDR_NonPMBankRIU + BK_CLK0,
    .u32BdmaBaseAddr = BASE_REG_BDMACh0_ADDR,
    // .u32RiuBaseAddr= IO_ADDRESS(MS_BASE_REG_RIU_PA),
     //.u32PMGPIOBaseAddr=BASEADDR_RIU+BK_PMGPIO,
};

// For linux, thread sync is handled by mtd. So, these functions are empty.
#define MSOS_PROCESS_PRIVATE    0x00000000
#define MSOS_PROCESS_SHARED     0x00000001


extern MS_BOOL MS_SERFLASH_IN_INTERRUPT (void);
extern MS_S32 MS_SERFLASH_CREATE_MUTEX ( MsOSAttribute eAttribute, char *pMutexName, MS_U32 u32Flag);
extern MS_BOOL MS_SERFLASH_DELETE_MUTEX(MS_S32 s32MutexId);
extern MS_BOOL MS_SERFLASH_OBTAIN_MUTEX (MS_S32 s32MutexId, MS_U32 u32WaitMs);
extern MS_BOOL MS_SERFLASH_RELEASE_MUTEX (MS_S32 s32MutexId);

extern void Chip_Flush_Memory(void);
//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static void _HAL_SPI_Rest(void);
static void _HAL_ISP_Enable(void);
static void _HAL_ISP_Disable(void);
static void _HAL_ISP_2XMode(MS_BOOL bEnable);
static MS_BOOL _HAL_SERFLASH_WaitWriteCmdRdy(void);
static MS_BOOL _HAL_SERFLASH_WaitWriteDataRdy(void);
static MS_BOOL _HAL_SERFLASH_WaitReadDataRdy(void);
static MS_BOOL _HAL_SERFLASH_WaitWriteDone(void);
#ifdef CONFIG_RIUISP
static MS_BOOL _HAL_SERFLASH_CheckWriteDone(void);
//static MS_BOOL _HAL_SERFLASH_XIURead(MS_U32 u32Addr,MS_U32 u32Size,MS_U8 * pu8Data);
static MS_BOOL _HAL_SERFLASH_RIURead(MS_U32 u32Addr,MS_U32 u32Size,MS_U8 * pu8Data);
#endif
static void _HAL_SERFLASH_ActiveFlash_Set_HW_WP(MS_BOOL bEnable);
MS_BOOL HAL_FSP_EraseChip(void);
MS_BOOL HAL_FSP_CheckWriteDone(void);
MS_BOOL HAL_FSP_ReadREMS4(MS_U8 * pu8Data, MS_U32 u32Size);

void HAL_FSP_Entry(void);
void HAL_FSP_Exit(void);

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------
//BDMA_Result MDrv_BDMA_CopyHnd(MS_PHYADDR u32SrcAddr, MS_PHYADDR u32DstAddr, MS_U32 u32Len, BDMA_CpyType eCpyType, MS_U8 u8OpCfg)
//{
//    return -1;
//}

static MS_BOOL _Hal_GetMsTime( MS_U32 tPreTime, MS_U32 u32Fac)
{
    //MS_U32 u32NsTime = 0;
    MS_U32 u32CurrTime = 0;
    u32CurrTime = get_timer(0);

    if((u32CurrTime-tPreTime)  > u32Fac)
        return TRUE;
    return FALSE;
}

void HAL_SERFLASH_SelectReadMode(SPI_READ_MODE eReadMode)
{
    switch(eReadMode)
    {
    case E_SINGLE_MODE://1-1-1 mode (SPI COMMAND is 0x03)
        QSPI_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_FAST_DISABLE, SFSH_CHIP_FAST_MASK);
    break;
    case E_FAST_MODE://1-1-1 mode (SPI COMMAND is 0x0B)
        QSPI_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_FAST_ENABLE, SFSH_CHIP_FAST_MASK);
    break;
    case E_DUAL_D_MODE: //1-1-2 mode (SPI COMMAND is 0x3B)
        QSPI_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_2XREAD_ENABLE, SFSH_CHIP_2XREAD_MASK);
        QSPI_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_FAST_DISABLE, SFSH_CHIP_FAST_MASK);
    break;
    case E_DUAL_AD_MODE: //1-2-2 mode (SPI COMMAND is 0xBB)
        QSPI_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_2XREAD_ENABLE, SFSH_CHIP_2XREAD_MASK);
    break;
    case E_QUAD_MODE:  //1-4-4 mode (SPI COMMAND is 0xEB)
	    QSPI_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_4XREAD_ENABLE , SFSH_CHIP_4XREAD_MASK);
    break;

    case E_RIUISP_MODE:
    default:
        break;
    }

}

//-------------------------------------------------------------------------------------------------
// Software reset spi_burst
// @return TRUE : succeed
// @return FALSE : fail
// @note : If no spi reset, it may cause BDMA fail.
//-------------------------------------------------------------------------------------------------
static void _HAL_SPI_Rest(void)
{
    // mark for A3
    #if 1
    ISP_WRITE_MASK(REG_ISP_CHIP_RST, SFSH_CHIP_RESET, SFSH_CHIP_RESET_MASK);
    ISP_WRITE_MASK(REG_ISP_CHIP_RST, SFSH_CHIP_NOTRESET, SFSH_CHIP_RESET_MASK);

	// Call the callback function to switch back the chip selection.
	if(McuChipSelectCB != NULL )
	{
		   (*McuChipSelectCB)();
	}
    #endif
}

//-------------------------------------------------------------------------------------------------
// Enable RIU ISP engine
// @return TRUE : succeed
// @return FALSE : fail
// @note : If Enable ISP engine, the XIU mode does not work
//-------------------------------------------------------------------------------------------------
static void _HAL_ISP_Enable(void)
{
    ISP_WRITE(REG_ISP_PASSWORD, 0xAAAA);
}

//-------------------------------------------------------------------------------------------------
// Disable RIU ISP engine
// @return TRUE : succeed
// @return FALSE : fail
// @note : If Disable ISP engine, the XIU mode works
//-------------------------------------------------------------------------------------------------
static void _HAL_ISP_Disable(void)
{
    ISP_WRITE(REG_ISP_PASSWORD, 0x5555);
    _HAL_SPI_Rest();
}


//-------------------------------------------------------------------------------------------------
// Enable/Disable address and data dual mode (SPI command is 0xBB)
// @return TRUE : succeed
// @return FALSE : fail
//-------------------------------------------------------------------------------------------------
static void _HAL_ISP_2XMode(MS_BOOL bEnable)
{
    if(bEnable) // on 2Xmode
    {
        QSPI_WRITE_MASK(REG_ISP_SPI_MODE,SFSH_CHIP_2XREAD_ENABLE,SFSH_CHIP_2XREAD_MASK);
    }
    else        // off 2Xmode
    {
        QSPI_WRITE_MASK(REG_ISP_SPI_MODE,SFSH_CHIP_2XREAD_DISABLE,SFSH_CHIP_2XREAD_MASK);
    }
}



//-------------------------------------------------------------------------------------------------
// Wait for SPI Write Cmd Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SERFLASH_WaitWriteCmdRdy(void)
{
    MS_BOOL bRet = FALSE;

    MS_U32 u32Timer;
    SER_FLASH_TIME(u32Timer);

    do
    {
        if ( (ISP_READ(REG_ISP_SPI_WR_CMDRDY) & ISP_SPI_WR_CMDRDY_MASK) == ISP_SPI_WR_CMDRDY )
        {
            bRet = TRUE;
            break;
        }

    } while (!SER_FLASH_EXPIRE(u32Timer, SERFLASH_SAFETY_FACTOR));




    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for SPI Write Cmd Ready fails!\n"));
    }

    return bRet;
}


//-------------------------------------------------------------------------------------------------
// Wait for SPI Write Data Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SERFLASH_WaitWriteDataRdy(void)
{
    MS_BOOL bRet = FALSE;


    MS_U32 u32Timer;
    SER_FLASH_TIME(u32Timer);

    do
    {
        if ( (ISP_READ(REG_ISP_SPI_WR_DATARDY) & ISP_SPI_WR_DATARDY_MASK) == ISP_SPI_WR_DATARDY )
        {
            bRet = TRUE;
            break;
        }

    } while (!SER_FLASH_EXPIRE(u32Timer, SERFLASH_SAFETY_FACTOR));



    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for SPI Write Data Ready fails!\n"));
    }

    return bRet;
}


//-------------------------------------------------------------------------------------------------
// Wait for SPI Read Data Ready
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SERFLASH_WaitReadDataRdy(void)
{
    MS_BOOL bRet = FALSE;


    MS_U32 u32Timer;
    SER_FLASH_TIME(u32Timer);

    do
    {
        if ( (ISP_READ(REG_ISP_SPI_RD_DATARDY) & ISP_SPI_RD_DATARDY_MASK) == ISP_SPI_RD_DATARDY )
        {
            bRet = TRUE;
            break;
        }

    } while (!SER_FLASH_EXPIRE(u32Timer, SERFLASH_SAFETY_FACTOR));




    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for SPI Read Data Ready fails!\n"));
    }

    return bRet;
}

//-------------------------------------------------------------------------------------------------
// Wait for Write/Erase to be done
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SERFLASH_WaitWriteDone(void)
{
    MS_BOOL bRet = FALSE;


    MS_U32 u32Timer;
    SER_FLASH_TIME(u32Timer);


    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        return FALSE;
    }

    do
    {

        ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_RDSR); // RDSR

        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            break;
        }

        if ( (ISP_READ(REG_ISP_SPI_RDATA) & SF_SR_WIP_MASK) == 0 ) // WIP = 0 write done
        {
            bRet = TRUE;
            break;
        }

    } while (!SER_FLASH_EXPIRE(u32Timer, SERFLASH_SAFETY_FACTOR));




    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for Write to be done fails!\n"));
    }

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    return bRet;
}
#ifdef CONFIG_RIUISP

//-------------------------------------------------------------------------------------------------
// Check Write/Erase to be done
// @return TRUE : succeed
// @return FALSE : fail before timeout
//-------------------------------------------------------------------------------------------------
static MS_BOOL _HAL_SERFLASH_CheckWriteDone(void)
{
    MS_BOOL bRet = FALSE;

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto _HAL_SERFLASH_CheckWriteDone_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_RDSR); // RDSR

    ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

    if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
    {
        goto _HAL_SERFLASH_CheckWriteDone_return;
    }

    if ( (ISP_READ(REG_ISP_SPI_RDATA) & SF_SR_WIP_MASK) == 0 ) // WIP = 0 write done
    {
        bRet = TRUE;
    }

_HAL_SERFLASH_CheckWriteDone_return:

    return bRet;
}
#endif
void msFlash_ActiveFlash_Set_HW_WP(MS_BOOL bEnable) __attribute__ ((weak));

void msFlash_ActiveFlash_Set_HW_WP(MS_BOOL bEnable)
{

	DEBUG_SER_FLASH(E_SERFLASH_DBGLV_NOTICE, printk("msFlash_ActiveFlash_Set_HW_WP() is not defined in this system\n"));

}

//-------------------------------------------------------------------------------------------------
/// Enable/Disable flash HW WP
/// @param  bEnable \b IN: enable or disable HW protection
//-------------------------------------------------------------------------------------------------
static void _HAL_SERFLASH_ActiveFlash_Set_HW_WP(MS_BOOL bEnable)
{
	msFlash_ActiveFlash_Set_HW_WP(bEnable);
    return;
}

//#if defined (MCU_AEON)
////Aeon SPI Address is 64K bytes windows
//static MS_BOOL _HAL_SetAeon_SPIMappingAddr(MS_U32 u32addr)
//{
//    MS_U16 u16MHEGAddr = (MS_U16)((_hal_isp.u32XiuBaseAddr + u32addr) >> 16);
//
//    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, 0x%08X)\n", __FUNCTION__, (int)u32addr, (int)u16MHEGAddr));
//    MHEG5_WRITE(REG_SPI_BASE, u16MHEGAddr);
//
//    return TRUE;
//}
//#endif
#ifdef CONFIG_RIUISP

static MS_BOOL _HAL_SERFLASH_RIURead(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32I;
    MS_U8 *pu8ReadBuf = pu8Data;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (int)u32Addr, (int)u32Size, pu8Data));

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    do{
        if(_HAL_SERFLASH_WaitWriteDone()) break;
        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis
    }while(1);

    ISP_WRITE(REG_ISP_SPI_ADDR_L, LOU16(u32Addr));
    ISP_WRITE(REG_ISP_SPI_ADDR_H, HIU16(u32Addr));

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_Read_return;
    }

    SPI_FLASH_CMD(ISP_SPI_CMD_READ);// READ // 0x0B fast Read : HW doesn't support now

    for ( u32I = 0; u32I < u32Size; u32I++ )
    {
        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_Read_return;
        }

            pu8ReadBuf[ u32I ] = SPI_READ_DATA();
    }
	//--- Flush OCP memory --------
   // MsOS_FlushMemory();

	bRet = TRUE;

HAL_SERFLASH_Read_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

#if defined (MCU_AEON)
    //restore default value
    _HAL_SetAeon_SPIMappingAddr(0);
#endif
     MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    return bRet;
}
#endif

//#if 0
//static MS_BOOL _HAL_SERFLASH_XIURead(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
//{
//    MS_BOOL bRet = FALSE;
//    MS_U32 u32I;
//    MS_U8 *pu8ReadBuf = pu8Data;
//    MS_U32 u32Value, u32AliSize;
//    MS_U32 u32AliAddr, u32RemSize = u32Size;
//    MS_U32 u32pos;
//
//    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (int)u32Addr, (int)u32Size, pu8Data));
//
//    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
//
//    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
//    {
//        printk("%s ENTRY fails!\n", __FUNCTION__);
//        return FALSE;
//    }
//
//    _HAL_ISP_Enable();
//
//    do{
//        if(_HAL_SERFLASH_WaitWriteDone()) break;
//        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis
//    }while(1);
//
//    _HAL_ISP_Disable();
//
//    // 4-BYTE Aligment for 32 bit CPU Aligment
//    u32AliAddr = (u32Addr & 0xFFFFFFFC);
//    u32pos = u32AliAddr >> 2;
//
//#if defined (MCU_AEON)
//    //write SPI mapping address
//    _HAL_SetAeon_SPIMappingAddr(u32AliAddr);
//#endif
//
//    //---- Read first data for not aligment address ------
//    if(u32AliAddr < u32Addr)
//    {
//        u32Value = SFSH_XIU_READ32(u32pos);
//        u32pos++;
//        for(u32I = 0; (u32I < 4) && (u32RemSize > 0); u32I++)
//        {
//            if(u32AliAddr >= u32Addr)
//            {
//                *pu8ReadBuf++ = (MS_U8)(u32Value & 0xFF);
//                u32RemSize--;
//            }
//            u32Value >>= 8;
//            u32AliAddr++;
//        }
//    }
//    //----Read datum for aligment address------
//    u32AliSize = (u32RemSize & 0xFFFFFFFC);
//    for( u32I = 0; u32I < u32AliSize; u32I += 4)
//    {
//#if defined (MCU_AEON)
//            if((u32AliAddr & 0xFFFF) == 0)
//                _HAL_SetAeon_SPIMappingAddr(u32AliAddr);
//#endif
//
//            // only indirect mode
//            u32Value = SFSH_XIU_READ32(u32pos);
//
//            *pu8ReadBuf++ = ( u32Value >> 0) & 0xFF;
//            *pu8ReadBuf++ = ( u32Value >> 8) & 0xFF;
//            *pu8ReadBuf++ = ( u32Value >> 16)& 0xFF;
//            *pu8ReadBuf++ = ( u32Value >> 24)& 0xFF;
//
//            u32pos++;
//            u32AliAddr += 4;
//        }
//
//    //--- Read remain datum --------
//    if(u32RemSize > u32AliSize)
//    {
//#if defined (MCU_AEON)
//            if((u32AliAddr & 0xFFFF) == 0)
//                _HAL_SetAeon_SPIMappingAddr(u32AliAddr);
//#endif
//            u32Value = SFSH_XIU_READ32(u32pos);
//        }
//        while(u32RemSize > u32AliSize)
//        {
//            *pu8ReadBuf++ = (u32Value & 0xFF);
//            u32Value >>= 8;
//            u32AliSize++;
//        }
//    //--- Flush OCP memory --------
//    //MsOS_FlushMemory();
//
//
//        bRet = TRUE;
//
//    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis
//
//    _HAL_ISP_Disable();
//
//#if defined (MCU_AEON)
//    //restore default value
//    _HAL_SetAeon_SPIMappingAddr(0);
//#endif
//     MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
//
//    return bRet;
//}
//
//#endif
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_SetCKG()
/// @brief \b Function \b Description: This function is used to set ckg_spi dynamically
/// @param <IN>        \b eCkgSpi    : enumerate the ckg_spi
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully , and is restricted to Flash ability
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_SERFLASH_SetCKG(SPI_DrvCKG eCkgSpi)
{
/*
	switch (eCkgSpi)
    {
        case E_SPI_108M://110M
            WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x29), BIT9, BIT9);
            WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x28),BITS(15:14, 0),BMASK(15:14)); // set ckg_spi
            break;

        case E_SPI_86M: //88.4M
            WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x29), BIT9, BIT9);
            WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x28),BITS(15:14, 1),BMASK(15:14)); // set ckg_spi
            break;

        case E_SPI_54M: //55.3M
            WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x29), BIT9, BIT9);
            WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x28),BITS(15:14, 2),BMASK(15:14)); // set ckg_spi
            break;

        case E_SPI_43M: //48M
            WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x29), BIT9, BIT9);
            WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x28),BITS(15:14, 3),BMASK(15:14)); // set ckg_spi
            break;

        case E_SPI_XTALI:
        default:
			WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x29), 0x0, BIT9);
			break;
    }
*/
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_ClkDiv()
/// @brief \b Function \b Description: This function is used to set clock div dynamically
/// @param <IN>        \b eCkgSpi    : enumerate the clk_div
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully , and is restricted to Flash ability
////////////////////////////////////////////////////////////////////////////////
void HAL_SERFLASH_ClkDiv(SPI_DrvClkDiv eClkDivSpi)
{
	switch (eClkDivSpi)
    {
        case E_SPI_DIV2:
            ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV2);
            break;
        case E_SPI_DIV4:
            ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV4);
            break;
        case E_SPI_DIV8:
            ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV8);
            break;
        case E_SPI_DIV16:
			ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV16);
            break;
		case E_SPI_DIV32:
			ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV32);
            break;
        case E_SPI_DIV64:
			ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV64);
            break;
	    case E_SPI_DIV128:
		    ISP_WRITE(REG_ISP_SPI_CLKDIV,ISP_SPI_CLKDIV128);
			break;
		case E_SPI_ClkDiv_NOT_SUPPORT:
       default:
			DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
			break;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_SetMode()
/// @brief \b Function \b Description: This function is used to set RIU/XIU dynamically
/// @param <IN>        \b bXiuRiu    : Enable for XIU (Default) Disable for RIU(Optional)
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : XIU is faster than RIU, but is sensitive to ckg.
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_SERFLASH_SetMode(MS_BOOL bXiuRiu)
{

//    MS_BOOL Ret = FALSE;
//
//    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));
//    _bXIUMode = bXiuRiu;
//    Ret = TRUE;
//    return Ret;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_Set2XREAD()
/// @brief \b Function \b Description: This function is used to set 2XREAD dynamically
/// @param <IN>        \b b2XMode    : ENABLE for 2XREAD DISABLE for NORMAL
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully, and needs Flash support
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_SERFLASH_Set2XREAD(MS_BOOL b2XMode)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

	if(!bDetect)
    {
        HAL_SERFLASH_DetectType();
    }
    MS_ASSERT(_hal_SERFLASH.b2XREAD); // check hw support or not
    if(_hal_SERFLASH.b2XREAD)
    {
        _HAL_ISP_2XMode(b2XMode);
    }
    else
    {
        UNUSED(b2XMode);
        printk("%s This flash does not support 2XREAD!!!\n", __FUNCTION__);
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SERFLASH_ChipSelect()
/// @brief \b Function \b Description: set active flash among multi-spi flashes
/// @param <IN>        \b u8FlashIndex : flash index (0 or 1)
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_SERFLASH_ChipSelect(MS_U8 u8FlashIndex)
{
    MS_BOOL Ret = FALSE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X)\n", __FUNCTION__, (int)u8FlashIndex));
    switch (u8FlashIndex)
    {
        case FLASH_ID0:
            QSPI_WRITE_MASK(REG_ISP_SPI_CHIP_SELE,SFSH_CHIP_SELE_EXT1,SFSH_CHIP_SELE_MASK);
            Ret = TRUE;
            break;
        case FLASH_ID1:
            QSPI_WRITE_MASK(REG_ISP_SPI_CHIP_SELE,SFSH_CHIP_SELE_EXT2,SFSH_CHIP_SELE_MASK);
            Ret = TRUE;
            break;
        case FLASH_ID2:
            QSPI_WRITE_MASK(REG_ISP_SPI_CHIP_SELE,SFSH_CHIP_SELE_EXT3,SFSH_CHIP_SELE_MASK);
            Ret = TRUE;
            break;
        case FLASH_ID3:
            UNUSED(u8FlashIndex); //Reserved
        default:
            UNUSED(u8FlashIndex); //Invalid flash ID
            Ret = FALSE;
            break;
    }
    WAIT_SFSH_CS_STAT(); // wait for chip select done
    return Ret;
}

void HAL_SERFLASH_Config()
{
    U8 u8BoundID;

    _hal_isp.u32IspBaseAddr = BASE_REG_ISP_ADDR;

    //chekc boundID to select pm-spi or non-pm spi
    u8BoundID = CHIP_READ(0x48) & CHIP_BOUND_TYPES;
    switch(u8BoundID)
    {
        case CHIP_BOUND_QFN88:
            _hal_isp.u32FspBaseAddr = BASE_REG_FSP_PM_ADDR;
            _hal_isp.u32QspiBaseAddr = BASE_REG_QSPI_PM_ADDR;
            break;
        default:
            _hal_isp.u32FspBaseAddr = BASE_REG_FSP_ADDR;
            _hal_isp.u32QspiBaseAddr = BASE_REG_QSPI_ADDR;
            break;
    }
}


void HAL_SERFLASH_Init(void)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    _s32SERFLASH_Mutex = MS_SERFLASH_CREATE_MUTEX(E_MSOS_FIFO, "Mutex SERFLASH", MSOS_PROCESS_SHARED);
    MS_ASSERT(_s32SERFLASH_Mutex >= 0);


    // set ckg_spi for B/GDMA
    //WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x29), BIT9, BIT9);
    //WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x28),BITS(15:14, 2),BMASK(15:14));


    //set mcu_clk to 110Mhz
    //WRITE_WORD_MASK(GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA, 0x22),BITS(11:10, 2),BMASK(11:10)); // set ckg_spi
	HAL_SERFLASH_ClkDiv(E_SPI_DIV8);


    ISP_WRITE(REG_ISP_DEV_SEL, 0x0); //mark for A3
    ISP_WRITE(REG_ISP_SPI_ENDIAN, ISP_SPI_ENDIAN_SEL);
}

void HAL_SERFLASH_SetGPIO(MS_BOOL bSwitch)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_NOTICE, printk("%s() Chicago TODO\n", __FUNCTION__));

#if 0
	MS_U32 u32PmGPIObase = _hal_isp.u32PMBaseAddr + 0x200;

	if(bSwitch)// The PAD of the SPI set as GPIO IN.
	{
		PM_WRITE_MASK(REG_PM_SPI_IS_GPIO, PM_SPI_IS_GPIO, PM_SPI_GPIO_MASK);
		WRITE_BYTE( u32PmGPIObase + ((REG_PM_GPIO_SPICZ_OEN)<<2), (READ_BYTE(u32PmGPIObase + ((REG_PM_GPIO_SPICZ_OEN)<<2))|(BIT(0))));
		WRITE_BYTE( u32PmGPIObase + ((REG_PM_GPIO_SPICK_OEN)<<2), (READ_BYTE(u32PmGPIObase + ((REG_PM_GPIO_SPICK_OEN)<<2))|(BIT(0))));
		WRITE_BYTE( u32PmGPIObase + ((REG_PM_GPIO_SPIDI_OEN)<<2), (READ_BYTE(u32PmGPIObase + ((REG_PM_GPIO_SPIDI_OEN)<<2))|(BIT(0))));
		WRITE_BYTE( u32PmGPIObase + ((REG_PM_GPIO_SPIDO_OEN)<<2), (READ_BYTE(u32PmGPIObase + ((REG_PM_GPIO_SPIDO_OEN)<<2))|(BIT(0))));
	}
	else
	{
		PM_WRITE_MASK(REG_PM_SPI_IS_GPIO, PM_SPI_NOT_GPIO, PM_SPI_GPIO_MASK);
		WRITE_BYTE( u32PmGPIObase + ((REG_PM_GPIO_SPICZ_OEN)<<2), (READ_BYTE(u32PmGPIObase + ((REG_PM_GPIO_SPICZ_OEN)<<2))|(BIT(0))));
		WRITE_BYTE( u32PmGPIObase + ((REG_PM_GPIO_SPICK_OEN)<<2), (READ_BYTE(u32PmGPIObase + ((REG_PM_GPIO_SPICK_OEN)<<2))|(BIT(0))));
		WRITE_BYTE( u32PmGPIObase + ((REG_PM_GPIO_SPIDI_OEN)<<2), (READ_BYTE(u32PmGPIObase + ((REG_PM_GPIO_SPIDI_OEN)<<2))|(BIT(0))));
		WRITE_BYTE( u32PmGPIObase + ((REG_PM_GPIO_SPIDO_OEN)<<2), (READ_BYTE(u32PmGPIObase + ((REG_PM_GPIO_SPIDO_OEN)<<2))|(BIT(0))));
	}
#endif
}
extern hal_SERFLASH_t _hal_SERFLASH_table[];
extern ST_WRITE_PROTECT _pstWriteProtectTable_MX25L6445E[];
extern ST_WRITE_PROTECT _pstWriteProtectTable_MX25L12845E[];

MS_BOOL HAL_SERFLASH_DetectType(void)
{
    #define READ_ID_SIZE    3
    #define READ_REMS4_SIZE 2

    MS_U8   u8FlashId[READ_ID_SIZE];
    MS_U8   u8FlashREMS4[READ_REMS4_SIZE] = {0};
    MS_U32  u32Index;
   // MS_U8   u8Status0, u8Status1;

    bDetect = 0; //avoid to run over twice on uboot that the value was set.

    memset(&_hal_SERFLASH, 0, sizeof(_hal_SERFLASH));

    if (HAL_SERFLASH_ReadID(u8FlashId, sizeof(u8FlashId))== TRUE)
    {
        /* find current serial flash */
        for (u32Index = 0; _hal_SERFLASH_table[u32Index].u8MID != 0; u32Index++)
        {
            if (   (_hal_SERFLASH_table[u32Index].u8MID  == u8FlashId[0])
                && (_hal_SERFLASH_table[u32Index].u8DID0 == u8FlashId[1])
                && (_hal_SERFLASH_table[u32Index].u8DID1 == u8FlashId[2])
                )
            {
                memcpy(&_hal_SERFLASH, &(_hal_SERFLASH_table[u32Index]), sizeof(_hal_SERFLASH));

                if(_hal_SERFLASH.u16FlashType == FLASH_IC_MX25L6405D||_hal_SERFLASH.u16FlashType == FLASH_IC_MX25L12805D)
                {
                    if (HAL_SERFLASH_ReadREMS(u8FlashREMS4,READ_REMS4_SIZE)== TRUE)
                    {
                        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_INFO,
                                printk("MXIC REMS: 0x%02X,0x%02X\n", u8FlashREMS4[0], u8FlashREMS4[1])  );
                        // patch : MXIC 6405D vs 6445E(MXIC 12805D vs 12845E)
                        if( u8FlashREMS4[0] == 0xC2)
                        {
                            if( u8FlashREMS4[1] == 0x16)
                            {
                                _hal_SERFLASH.u16FlashType = FLASH_IC_MX25L6445E;
                                _hal_SERFLASH.pWriteProtectTable = _pstWriteProtectTable_MX25L6445E;
                            }
                            if( u8FlashREMS4[1] == 0x17)
                            {
                                _hal_SERFLASH.u16FlashType = FLASH_IC_MX25L12845E;
                                _hal_SERFLASH.pWriteProtectTable = _pstWriteProtectTable_MX25L12845E;
                            }
                        }
                    }
                }

                DEBUG_SER_FLASH(E_SERFLASH_DBGLV_INFO,
                                printk("Flash is detected (0x%04X, 0x%02X, 0x%02X, 0x%02X)\n",
                                       _hal_SERFLASH.u16FlashType,
                                       _hal_SERFLASH.u8MID,
                                       _hal_SERFLASH.u8DID0,
                                       _hal_SERFLASH.u8DID1
                                       )
                                );
                bDetect = TRUE;
                break;
            }
            else
            {
                continue;
            }
        }
        if( _hal_SERFLASH.u8MID == MID_GD)
        {
            _hal_SERFLASH.u8WrsrBlkProtect = BITS(6:2, 0x00);
        }

        if(_hal_SERFLASH.u16FlashType == FLASH_IC_S25FL128P)
        {
            _bNon4kErase = 1;
        }

        if(_hal_SERFLASH.u32FlashSize >= 0x2000000)
        {
            if((_hal_SERFLASH.u8MID == MID_MXIC)||(_hal_SERFLASH.u8MID == MID_GD))
            {
                _bHasEAR = TRUE; // support EAR mode
                HAL_FSP_WriteExtAddrReg(0);
            }
        }

        // If the Board uses a unknown flash type, force setting a secure flash type for booting. //FLASH_IC_MX25L6405D
        if( bDetect != TRUE )
        {
            memcpy(&_hal_SERFLASH, &(_hal_SERFLASH_table[0]), sizeof(_hal_SERFLASH));
            _hal_SERFLASH.u8MID = u8FlashId[0];
            _hal_SERFLASH.u8DID0 = u8FlashId[1];
            _hal_SERFLASH.u8DID1 = u8FlashId[2];
            DEBUG_SER_FLASH(E_SERFLASH_DBGLV_INFO, printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));

            DEBUG_SER_FLASH(E_SERFLASH_DBGLV_INFO,
                            printk("Unknown flash type (0x%02X, 0x%02X, 0x%02X) and use default flash type\nplease add flash info to serial flash driver\n",
                                   _hal_SERFLASH.u8MID,
                                   _hal_SERFLASH.u8DID0,
                                   _hal_SERFLASH.u8DID1));
            DEBUG_SER_FLASH(E_SERFLASH_DBGLV_INFO, printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));

            bDetect = TRUE;
        }

    }

    if (gReadMode==E_QUAD_MODE && _hal_SERFLASH.u8MID!=MID_MXIC)
    {
		gReadMode=E_DUAL_D_MODE;
        printf("Not support quad mode!!!\n");
    }
#ifndef CONFIG_RIUISP
    ISP_WRITE(REG_ISP_DEV_SEL, ISP_DEV_SEL);
#endif
    HAL_SERFLASH_SetCKG(_hal_SERFLASH.u16SPIMaxClk[0]);

    return bDetect;

}

MS_BOOL HAL_SERFLASH_DetectSize(MS_U32  *u32FlashSize)
{
    MS_BOOL Ret = FALSE;

    do{

        *u32FlashSize = _hal_SERFLASH.u32FlashSize;
        Ret = TRUE;

    }while(0);

    return Ret;
}


MS_BOOL HAL_SERFLASH_EraseChip(void)
{
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_RIUISP

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        goto HAL_SERFLASH_EraseChip_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_EraseChip_return;
    }
    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN


    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_EraseChip_return;
    }
    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_CE); // CHIP_ERASE

    bRet = _HAL_SERFLASH_WaitWriteDone();

HAL_SERFLASH_EraseChip_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#else
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()\n", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    //MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    bRet = HAL_FSP_EraseChip();
      MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#endif
    return bRet;
}


MS_BOOL HAL_SERFLASH_AddressToBlock(MS_U32 u32FlashAddr, MS_U32 *pu32BlockIndex)
{
    MS_U32  u32NextAddr;
    MS_BOOL bRet = FALSE;

    if (_hal_SERFLASH.pSpecialBlocks == NULL)
    {
        int bs=0;
        for(bs=0;bs<24;bs++)
        {
            if((1<<bs)==SERFLASH_SECTOR_SIZE)
            break;
        }

        if(bs>=20)
        {
            printf("ERROR!! Can not get correct bit shift for sector size!!\n");
            return FALSE;
        }

        *pu32BlockIndex =(u32FlashAddr >> bs);

        bRet = TRUE;
    }
    else
    {
        // TODO: review, optimize this flow
        for (u32NextAddr = 0, *pu32BlockIndex = 0; *pu32BlockIndex < NUMBER_OF_SERFLASH_SECTORS; (*pu32BlockIndex)++)
        {
            // outside the special block
            if (   *pu32BlockIndex < _hal_SERFLASH.pSpecialBlocks->u16Start
                || *pu32BlockIndex > _hal_SERFLASH.pSpecialBlocks->u16End
                )
            {
                u32NextAddr += SERFLASH_SECTOR_SIZE; // i.e. normal block size
            }
            // inside the special block
            else
            {
                u32NextAddr += _hal_SERFLASH.pSpecialBlocks->au32SizeList[*pu32BlockIndex - _hal_SERFLASH.pSpecialBlocks->u16Start];
            }

            if (u32NextAddr > u32FlashAddr)
            {
                bRet = TRUE;
                break;
            }
        }
    }

    return bRet;
}


MS_BOOL HAL_SERFLASH_BlockToAddress(MS_U32 u32BlockIndex, MS_U32 *pu32FlashAddr)
{
    if (   _hal_SERFLASH.pSpecialBlocks == NULL
        || u32BlockIndex <= _hal_SERFLASH.pSpecialBlocks->u16Start
        )
    {
        *pu32FlashAddr = u32BlockIndex * SERFLASH_SECTOR_SIZE;
    }
    else
    {
        MS_U32 u32Index;

        *pu32FlashAddr = _hal_SERFLASH.pSpecialBlocks->u16Start * SERFLASH_SECTOR_SIZE;

        for (u32Index = _hal_SERFLASH.pSpecialBlocks->u16Start;
             u32Index < u32BlockIndex && u32Index <= _hal_SERFLASH.pSpecialBlocks->u16End;
             u32Index++
             )
        {
            *pu32FlashAddr += _hal_SERFLASH.pSpecialBlocks->au32SizeList[u32Index - _hal_SERFLASH.pSpecialBlocks->u16Start];
        }

        if (u32BlockIndex > _hal_SERFLASH.pSpecialBlocks->u16End + 1)
        {
            *pu32FlashAddr += (u32BlockIndex - _hal_SERFLASH.pSpecialBlocks->u16End - 1) * SERFLASH_SECTOR_SIZE;
        }
    }

    return TRUE;
}

MS_BOOL HAL_SERFLASH_BlockErase(MS_U32 u32StartBlock, MS_U32 u32EndBlock, MS_BOOL bWait)
{
#ifdef CONFIG_RIUISP
    MS_BOOL bRet = FALSE;
    MS_U32 u32I;
    MS_U32 u32FlashAddr = 0;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, 0x%08X, %d)\n", __FUNCTION__, (int)u32StartBlock, (int)u32EndBlock, bWait));

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    if( u32StartBlock > u32EndBlock || u32EndBlock >= _hal_SERFLASH.u32NumSec )
    {
        printk("%s (0x%08X, 0x%08X, %d)\n", __FUNCTION__, (int)u32StartBlock, (int)u32EndBlock, bWait);
        goto HAL_SERFLASH_BlockErase_return;
    }
#ifdef CONFIG_FLASH_DISPLAY_PROGRESS
            printk("     ");
#endif

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        goto HAL_SERFLASH_BlockErase_return;
    }

    for( u32I = u32StartBlock; u32I <= u32EndBlock; u32I++)
    {
        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }
        ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode

        ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_64BE); // BLOCK_ERASE

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        if (HAL_SERFLASH_BlockToAddress(u32I, &u32FlashAddr) == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, HIU16(u32FlashAddr) & 0xFF);

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32FlashAddr) >> 8);

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32FlashAddr) & 0xFF);

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_BlockErase_return;
        }

        ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x5555); // disable trigger mode

        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        if(bWait == TRUE )
        {
            if(!_HAL_SERFLASH_WaitWriteDone())
            {
                printk("%s : Wait Write Done Fail!!!\n", __FUNCTION__ );
                bRet = FALSE;
            }
            else
            {
                bRet = TRUE;
            }
        }
        else
        {
            bRet = TRUE;
        }
#ifdef CONFIG_FLASH_DISPLAY_PROGRESS
        printk("\b\b\b\b%3d%%", (int)((u32I-u32StartBlock+1)*100/(u32EndBlock-u32StartBlock+1)) );
#endif


    }

HAL_SERFLASH_BlockErase_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    return bRet;
#else
    return  HAL_FSP_BlockErase(u32StartBlock, u32EndBlock, bWait);
#endif
}

MS_BOOL HAL_SERFLASH_SectorErase(MS_U32 u32SectorAddress)
{
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_RIUISP

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X)\n", __FUNCTION__, (int)u32SectorAddress));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s ENTRY fails!\n", __FUNCTION__));
        return bRet;
    }

    if( u32SectorAddress > _hal_SERFLASH.u32FlashSize )
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s (0x%08X)\n", __FUNCTION__, (int)u32SectorAddress));
        goto HAL_SERFLASH_BlockErase_return;
    }

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        goto HAL_SERFLASH_BlockErase_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_SE);

    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, HIU16(u32SectorAddress) & 0xFF);

    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32SectorAddress) >> 8);

    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32SectorAddress) & 0xFF);

    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
            goto HAL_SERFLASH_BlockErase_return;
    }

    bRet = TRUE;

HAL_SERFLASH_BlockErase_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x5555);     // disable trigger mode

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

#else
    bRet = HAL_FSP_SectorErase(u32SectorAddress,FLASH_ERASE_04K);
#endif
    return bRet;
}

MS_BOOL HAL_SERFLASH_CheckWriteDone(void)
{
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_RIUISP
    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR);    // SPI CEB dis

    bRet = _HAL_SERFLASH_CheckWriteDone();

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR);    // SPI CEB dis

    _HAL_ISP_Disable();

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s() = %d\n", __FUNCTION__, bRet));

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#else
    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
   // MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    bRet = HAL_FSP_CheckWriteDone();
    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#endif
    return bRet;
}


MS_BOOL HAL_SERFLASH_Write(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_RIUISP
    MS_U16 u16I, u16Rem, u16WriteBytes;
    MS_U8 *u8Buf = pu8Data;
    MS_BOOL b2XREAD = FALSE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (int)u32Addr, (int)u32Size, pu8Data));
#ifdef CONFIG_FLASH_DISPLAY_PROGRESS
    MS_U32 u32TotalSize = u32Size;
    printk("    ");
#endif


    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    b2XREAD = (BIT(2) & ISP_READ(REG_ISP_SPI_MODE))? 1 : 0;
    _HAL_ISP_2XMode(DISABLE);
    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        goto HAL_SERFLASH_Write_return;
    }

    u16Rem = u32Addr % SERFLASH_PAGE_SIZE;

    if (u16Rem)
    {
        u16WriteBytes = SERFLASH_PAGE_SIZE - u16Rem;
        if (u32Size < u16WriteBytes)
        {
            u16WriteBytes = u32Size;
        }

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_Write_return;
        }

        ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

        ISP_WRITE(REG_ISP_SPI_ADDR_L, LOU16(u32Addr));
        ISP_WRITE(REG_ISP_SPI_ADDR_H, (MS_U8)HIU16(u32Addr));

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_Write_return;
        }

        ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_PP); // PAGE_PROG

        for ( u16I = 0; u16I < u16WriteBytes; u16I++ )
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, (MS_U8)*(u8Buf + u16I));//SPI_WRITE_DATA( *(u8Buf + u16I) );

            if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
            {
                goto HAL_SERFLASH_Write_return;
            }
        }

        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        bRet = _HAL_SERFLASH_WaitWriteDone();

        if ( bRet == TRUE )
        {
            u32Addr += u16WriteBytes;
            u8Buf   += u16WriteBytes;
            u32Size -= u16WriteBytes;
        }
        else
        {
            goto HAL_SERFLASH_Write_return;
        }
#ifdef CONFIG_FLASH_DISPLAY_PROGRESS
        printk("\b\b\b\b%3d%%", (int)((u32TotalSize-u32Size)*100/(u32TotalSize)) );
#endif
    }

    while(u32Size)
    {
        if( u32Size > SERFLASH_PAGE_SIZE)
        {
            u16WriteBytes = SERFLASH_PAGE_SIZE;  //write SERFLASH_PAGE_SIZE bytes one time
        }
        else
        {
            u16WriteBytes = u32Size;
        }

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_Write_return;
        }

        ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

        ISP_WRITE(REG_ISP_SPI_ADDR_L, LOU16(u32Addr));
        ISP_WRITE(REG_ISP_SPI_ADDR_H, (MS_U8)HIU16(u32Addr));

        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SERFLASH_Write_return;
        }

        ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_PP); // PAGE_PROG
/*
        // Improve flash write speed
        if(u16WriteBytes == 256)
        {
            // Write 256 bytes to flash
            MS_U8 u8Index = 0;

            do{

                ISP_WRITE(REG_ISP_SPI_WDATA, (MS_U8)*(u8Buf + u8Index));//SPI_WRITE_DATA( *(u8Buf + u8Index) );

                u8Index++;

                if( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
                {
                    goto HAL_SERFLASH_Write_return;
                }

            }while(u8Index != 0);
        }
        else*/
        {

            for ( u16I = 0; u16I < u16WriteBytes; u16I++ )
            {
                ISP_WRITE(REG_ISP_SPI_WDATA, (MS_U8)*(u8Buf + u16I));//SPI_WRITE_DATA( *(u8Buf + u16I) );

                if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
                {
                    goto HAL_SERFLASH_Write_return;
                }
            }
        }

        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        bRet = _HAL_SERFLASH_WaitWriteDone();

        if ( bRet == TRUE )
        {
            u32Addr += u16WriteBytes;
            u8Buf   += u16WriteBytes;
            u32Size -= u16WriteBytes;
        }
        else
        {
            goto HAL_SERFLASH_Write_return;
        }
#ifdef CONFIG_FLASH_DISPLAY_PROGRESS
        printk("\b\b\b\b%3d%%", (int)((u32TotalSize-u32Size)*100/(u32TotalSize)) );
#endif
    }


HAL_SERFLASH_Write_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    //  restore the 2x READ setting.
    HAL_SERFLASH_SelectReadMode(_hal_SERFLASH.u16SPIMaxClk[1]);


    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#else
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (int)u32Addr, (int)u32Size, pu8Data));

    #ifdef CONFIG_FSP_WRITE_RIUOP

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    MS_ASSERT(_HAL_SERFLASH_Check51RunMode());
    HAL_FSP_Entry();
    bRet = HAL_FSP_Write(u32Addr, u32Size, pu8Data);
    HAL_FSP_Exit();

    #elif CONFIG_FSP_WRITE_BDMA

        bRet = HAL_FSP_Write_BDMA(u32Addr, u32Size, pu8Data);

    #else
        #error "FSP write"
    #endif
    if (_u8RegEAR)
        HAL_FSP_WriteExtAddrReg(0);

#endif
    return bRet;
}

MS_BOOL HAL_SERFLASH_Read(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL Ret = FALSE;

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    //MS_ASSERT(_HAL_SERFLASH_Check51RunMode());
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }
#ifdef CONFIG_RIUISP
    /*if( _bXIUMode )
    {
        Ret = _HAL_SERFLASH_XIURead( u32Addr, u32Size, pu8Data);
    }
    else// RIU mode*/
    {
        Ret = _HAL_SERFLASH_RIURead( u32Addr, u32Size, pu8Data);
    }
#else
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (int)u32Addr, (int)u32Size, pu8Data));
    HAL_SERFLASH_SelectReadMode(gReadMode);
	switch(gReadMode)
	{
		case E_FAST_MODE:
		case E_DUAL_AD_MODE:
		case E_QUAD_MODE:
            #ifdef CONFIG_FSP_READ_DIRERECT
                 Ret = HAL_FSP_Read(u32Addr, u32Size, pu8Data);
            #elif defined(CONFIG_FSP_READ_RIUOP)
                Ret = HAL_FSP_Read(u32Addr, u32Size, pu8Data);
            #else
                #error "FPS READ"
            #endif

		break;
		default:
			Ret = HAL_FSP_Read(u32Addr, u32Size, pu8Data);
		break;
	}
#endif
    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
    return Ret;
}
EN_WP_AREA_EXISTED_RTN HAL_SERFLASH_WP_Area_Existed(MS_U32 u32UpperBound, MS_U32 u32LowerBound, MS_U8 *pu8BlockProtectBits)
{
    ST_WRITE_PROTECT   *pWriteProtectTable;
    MS_U8               u8Index;
    MS_BOOL             bPartialBoundFitted;
    MS_BOOL             bEndOfTable;
    MS_U32              u32PartialFittedLowerBound = u32UpperBound;
    MS_U32              u32PartialFittedUpperBound = u32LowerBound;


    if (NULL == _hal_SERFLASH.pWriteProtectTable)
    {
        return WP_TABLE_NOT_SUPPORT;
    }


    for (u8Index = 0, bEndOfTable = FALSE, bPartialBoundFitted = FALSE; FALSE == bEndOfTable; u8Index++)
    {
        pWriteProtectTable = &(_hal_SERFLASH.pWriteProtectTable[u8Index]);

        if (   0xFFFFFFFF == pWriteProtectTable->u32LowerBound
            && 0xFFFFFFFF == pWriteProtectTable->u32UpperBound
            )
        {
            bEndOfTable = TRUE;
        }

        if (   pWriteProtectTable->u32LowerBound == u32LowerBound
            && pWriteProtectTable->u32UpperBound == u32UpperBound
            )
        {
            *pu8BlockProtectBits = pWriteProtectTable->u8BlockProtectBits;

            return WP_AREA_EXACTLY_AVAILABLE;
        }
        else if (u32LowerBound <= pWriteProtectTable->u32LowerBound && pWriteProtectTable->u32UpperBound <= u32UpperBound)
        {
            //
            // u32PartialFittedUpperBound & u32PartialFittedLowerBound would be initialized first time when bPartialBoundFitted == FALSE (init value)
            // 1. first match:  FALSE == bPartialBoundFitted
            // 2. better match: (pWriteProtectTable->u32UpperBound - pWriteProtectTable->u32LowerBound) > (u32PartialFittedUpperBound - u32PartialFittedLowerBound)
            //

            if (   FALSE == bPartialBoundFitted
                || (pWriteProtectTable->u32UpperBound - pWriteProtectTable->u32LowerBound) > (u32PartialFittedUpperBound - u32PartialFittedLowerBound)
                )
            {
                u32PartialFittedUpperBound = pWriteProtectTable->u32UpperBound;
                u32PartialFittedLowerBound = pWriteProtectTable->u32LowerBound;
                *pu8BlockProtectBits = pWriteProtectTable->u8BlockProtectBits;
            }

            bPartialBoundFitted = TRUE;
        }
    }

    if (TRUE == bPartialBoundFitted)
    {
        return WP_AREA_PARTIALLY_AVAILABLE;
    }
    else
    {
        return WP_AREA_NOT_AVAILABLE;
    }
}


MS_BOOL HAL_SERFLASH_WriteProtect_Area(MS_BOOL bEnableAllArea, MS_U8 u8BlockProtectBits)
{
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_RIUISP
    MS_U8   u8Status0, u8Status1;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d, 0x%02X)\n", __FUNCTION__, bEnableAllArea, u8BlockProtectBits));

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_SERFLASH_ActiveFlash_Set_HW_WP(DISABLE);
    udelay(bEnableAllArea ? 5 : 20); // when disable WP, delay more time

    _HAL_ISP_Enable();

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_Flash_WriteProtect_Area_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_Flash_WriteProtect_Area_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR

    if (TRUE == bEnableAllArea)
    {
        if (_hal_SERFLASH.u8MID == MID_ATMEL)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, SERFLASH_WRSR_BLK_PROTECT); // SPRL 1 -> 0

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_Flash_WriteProtect_Area_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_Flash_WriteProtect_Area_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, SF_SR_SRWD | SERFLASH_WRSR_BLK_PROTECT); // SF_SR_SRWD: SRWD Status Register Write Protect
    }
    else
    {
        if (_hal_SERFLASH.u8MID == MID_ATMEL)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, u8BlockProtectBits); // [4:2] or [5:2] protect blocks // SPRL 1 -> 0

            // programming sector protection
            {
                int i;
                MS_U32 u32FlashAddr;

                // search write protect table
                for (i = 0;
                     0xFFFFFFFF != _hal_SERFLASH.pWriteProtectTable[i].u32LowerBound && 0xFFFFFFFF != _hal_SERFLASH.pWriteProtectTable[i].u32UpperBound; // the end of write protect table
                     i++
                     )
                {
                    // if found, write
                    if (u8BlockProtectBits == _hal_SERFLASH.pWriteProtectTable[i].u8BlockProtectBits)
                    {
                        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("u8BlockProtectBits = 0x%X, u32LowerBound = 0x%X, u32UpperBound = 0x%X\n",
                                                                       (unsigned int)u8BlockProtectBits,
                                                                       (unsigned int)_hal_SERFLASH.pWriteProtectTable[i].u32LowerBound,
                                                                       (unsigned int)_hal_SERFLASH.pWriteProtectTable[i].u32UpperBound
                                                                       )
                                        );
                        for (u32FlashAddr = 0; u32FlashAddr < _hal_SERFLASH.u32FlashSize; u32FlashAddr += _hal_SERFLASH.u32SecSize)
                        {
                            if (_hal_SERFLASH.pWriteProtectTable[i].u32LowerBound <= (u32FlashAddr + _hal_SERFLASH.u32SecSize - 1) &&
                                u32FlashAddr <= _hal_SERFLASH.pWriteProtectTable[i].u32UpperBound)
                            {
                                continue;
                            }

                            ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

                            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
                            {
                                goto HAL_Flash_WriteProtect_Area_return;
                            }

                            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

                            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
                            {
                                goto HAL_Flash_WriteProtect_Area_return;
                            }

                            ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode

                            ISP_WRITE(REG_ISP_SPI_WDATA, 0x39); // unprotect sector

                            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
                            {
                                goto HAL_Flash_WriteProtect_Area_return;
                            }

                            ISP_WRITE(REG_ISP_SPI_WDATA, (u32FlashAddr >> 16) & 0xFF);

                            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
                            {
                                goto HAL_Flash_WriteProtect_Area_return;
                            }

                            ISP_WRITE(REG_ISP_SPI_WDATA, ((MS_U16)u32FlashAddr) >> 8);

                            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
                                goto HAL_Flash_WriteProtect_Area_return;
                            }

                            ISP_WRITE(REG_ISP_SPI_WDATA, u32FlashAddr & 0xFF);

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_Flash_WriteProtect_Area_return;
            }

                            ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222); // disable trigger mode

            ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

                            bRet = _HAL_SERFLASH_WaitWriteDone();
                        }
                        break;
                    }
                }
            }

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_Flash_WriteProtect_Area_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_Flash_WriteProtect_Area_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, SF_SR_SRWD | u8BlockProtectBits); // [4:2] or [5:2] protect blocks
    }

    bRet = _HAL_SERFLASH_WaitWriteDone();

HAL_Flash_WriteProtect_Area_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

   if ((_hal_SERFLASH.u8MID == MID_GD) || (_hal_SERFLASH.u16FlashType == FLASH_IC_S25FL032K)
          || (_hal_SERFLASH.u16FlashType == FLASH_IC_W25Q64CV) || (_hal_SERFLASH.u16FlashType == FLASH_IC_W25Q32BV))//MSB S8-S15
    {
        HAL_SERFLASH_ReadStatusReg(&u8Status0);
        HAL_SERFLASH_ReadStatusReg2(&u8Status1);
        HAL_SERFLASH_WriteStatusReg(((u8Status1 << 8)|u8Status0|0x4000));//CMP = 1
        udelay(40);
    }

    if (bEnableAllArea)// _REVIEW_
    {
        _HAL_SERFLASH_ActiveFlash_Set_HW_WP(bEnableAllArea);
    }

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#else
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d, 0x%02X)\n", __FUNCTION__, bEnableAllArea, u8BlockProtectBits));

        MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
        //MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

        if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
        {
            printk("%s ENTRY fails!\n", __FUNCTION__);
            return FALSE;
        }

        bRet = HAL_FSP_WriteProtect_Area(bEnableAllArea, u8BlockProtectBits);
        MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#endif
    return bRet;
}

//-------------------------------------------------------------------------------------------------
/*spasion flash DYB unlock*/

MS_BOOL HAL_SERFLASH_SPSNDYB_UNLOCK(MS_U32 u32StartBlock, MS_U32 u32EndBlock)
{
//u32StartBlock, u32EndBlock --> 0~285
  MS_BOOL bRet = FALSE;
  MS_U32 u32I;
  MS_U32 u32FlashAddr = 0;
  MS_U8 dybStatusReg=0;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG,printk("%s ENTRY \n", __FUNCTION__));
    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_SERFLASH_ActiveFlash_Set_HW_WP(DISABLE);
    //MsOS_DelayTask(20); // when disable WP, delay more time
    udelay(20);
    _HAL_ISP_Enable();
    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SPSNFLASH_DYB_UNLOCK_return;
    }

    for( u32I = u32StartBlock; u32I <= u32EndBlock; u32I++)
    {
        bRet = FALSE;
        dybStatusReg=0;
        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis
        ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN
        if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode
        ISP_WRITE(REG_ISP_SPI_WDATA, 0xE1); // DYB write
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        if(u32I>=0 && u32I<=31)
            u32FlashAddr = u32I * 4096;
        else
            u32FlashAddr = (u32I -30) * 64 * 1024;
        ISP_WRITE(REG_ISP_SPI_WDATA, HIU16(u32FlashAddr)>>8); //MSB , 4th byte
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, HIU16(u32FlashAddr) & 0xFF);//3th byte

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32FlashAddr) >> 8);//2th byte

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32FlashAddr) & 0xFF);//1th byte

        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, 0xFF);//0xFF --> unlock;  0x00-->lock
        ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x5555); // disable trigger mode
        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis
        ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);
        //read DYB status register to check the result
        ISP_WRITE(REG_ISP_SPI_WDATA, 0xE0); // DYB read
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, HIU16(u32FlashAddr)>>8); //MSB , 4th byte
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, HIU16(u32FlashAddr) & 0xFF);//3th byte
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32FlashAddr) >> 8);//2th byte
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, LOU16(u32FlashAddr) & 0xFF);//1th byte
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }

        dybStatusReg = (MS_U8)ISP_READ(REG_ISP_SPI_RDATA);//SPI_READ_DATA();
        ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x5555); // disable trigger mode
        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis
        if(dybStatusReg != 0xFF )
        {
            bRet = FALSE;
            goto HAL_SPSNFLASH_DYB_UNLOCK_return;
        }
        else
            bRet = TRUE;
    }

HAL_SPSNFLASH_DYB_UNLOCK_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis
    _HAL_ISP_Disable();
    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
    return bRet;
}


MS_BOOL HAL_SERFLASH_WriteProtect(MS_BOOL bEnable)
{
// Note: Temporarily don't call this function until MSTV_Tool ready
    MS_BOOL bRet = FALSE;

#ifdef CONFIG_RIUISP

    MS_U8   u8Status0, u8Status1;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d)\n", __FUNCTION__, bEnable));

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_SERFLASH_ActiveFlash_Set_HW_WP(DISABLE);
    //udelay(bEnable ? 5 : 20); //mark for A3

    _HAL_ISP_Enable();

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_WriteProtect_return;
    }
    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_WriteProtect_return;
    }
    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR

    if (bEnable)
    {
        if (_hal_SERFLASH.u8MID == MID_ATMEL)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, SERFLASH_WRSR_BLK_PROTECT); // SPRL 1 -> 0

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_SERFLASH_WriteProtect_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_SERFLASH_WriteProtect_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR
        }

        ISP_WRITE(REG_ISP_SPI_WDATA, SF_SR_SRWD | SERFLASH_WRSR_BLK_PROTECT); // SF_SR_SRWD: SRWD Status Register Write Protect

        if ((_hal_SERFLASH.u8MID == MID_GD) || (_hal_SERFLASH.u16FlashType == FLASH_IC_S25FL032K)
          || (_hal_SERFLASH.u16FlashType == FLASH_IC_W25Q64CV) || (_hal_SERFLASH.u16FlashType == FLASH_IC_W25Q32BV))//MSB S8-S15
        {
            if ( _HAL_SERFLASH_WaitWriteDone() == FALSE )
            {
                goto HAL_SERFLASH_WriteProtect_return;
            }
            ISP_WRITE(REG_ISP_SPI_WDATA, 0x40);
        }

    }
    else
    {
        if (_hal_SERFLASH.u8MID == MID_ATMEL)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, 0 << 2); // [4:2] or [5:2] protect blocks // SPRL 1 -> 0

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_SERFLASH_WriteProtect_return;
            }

            ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

            if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
            {
                goto HAL_SERFLASH_WriteProtect_return;
            }

            ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WRSR); // WRSR
        }

        if ((_hal_SERFLASH.u8MID == MID_GD) || (_hal_SERFLASH.u16FlashType == FLASH_IC_S25FL032K)
          || (_hal_SERFLASH.u16FlashType == FLASH_IC_W25Q64CV) || (_hal_SERFLASH.u16FlashType == FLASH_IC_W25Q32BV))//MSB S8-S15
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, BITS(6:2, 0x1F)); // [6:2] protect blocks

            if ( _HAL_SERFLASH_WaitWriteDone() == FALSE )
            {
                goto HAL_SERFLASH_WriteProtect_return;
            }
            ISP_WRITE(REG_ISP_SPI_WDATA, 0x40);
        }
        else
        {
        ISP_WRITE(REG_ISP_SPI_WDATA, SF_SR_SRWD | 0 << 2); // [4:2] or [5:2] protect blocks
    }

    }

    bRet = _HAL_SERFLASH_WaitWriteDone();

HAL_SERFLASH_WriteProtect_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();
   if ((_hal_SERFLASH.u8MID == MID_GD) || (_hal_SERFLASH.u16FlashType == FLASH_IC_S25FL032K)
          || (_hal_SERFLASH.u16FlashType == FLASH_IC_W25Q64CV) || (_hal_SERFLASH.u16FlashType == FLASH_IC_W25Q32BV))//MSB S8-S15
    {
        HAL_SERFLASH_ReadStatusReg(&u8Status0);
        HAL_SERFLASH_ReadStatusReg2(&u8Status1);
        HAL_SERFLASH_WriteStatusReg(((u8Status1 << 8)|u8Status0|0x4000));//CMP = 1
        udelay(40);
    }

    if (bEnable) // _REVIEW_
    {
        _HAL_SERFLASH_ActiveFlash_Set_HW_WP(bEnable);
    }

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    return bRet;
#else
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d)\n", __FUNCTION__, bEnable));

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
   // MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    bRet = HAL_FSP_WriteProtect(bEnable);
    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#endif
    return bRet;
}


MS_BOOL HAL_SERFLASH_ReadID(MS_U8 *pu8Data, MS_U32 u32Size)
{
    // HW doesn't support ReadID on MX/ST flash; use trigger mode instead.
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_RIUISP
    MS_U32 u32I;
    MS_U8 *u8ptr = pu8Data;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        goto HAL_SERFLASH_ReadID_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadID_return;
    }
    // SFSH_RIU_REG16(REG_SFSH_SPI_COMMAND) = ISP_SPI_CMD_RDID; // RDID
    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDID); // RDID
    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadID_return;
    }

    for ( u32I = 0; u32I < u32Size; u32I++ )
    {
        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_ReadID_return;
        }

        u8ptr[u32I] = ISP_READ(REG_ISP_SPI_RDATA);

        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk(" 0x%02X", u8ptr[u32I]));
    }
    bRet = TRUE;

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222); // disable trigger mode


HAL_SERFLASH_ReadID_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));
#else
   MS_ASSERT( MsOS_In_Interrupt() == FALSE );
   MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

   if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
   {
       printk("%s ENTRY fails!\n", __FUNCTION__);
       return FALSE;
   }


   bRet = HAL_FSP_ReadID(pu8Data, u32Size);

   MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#endif
    return bRet;
}

MS_U64 HAL_SERFLASH_ReadUID(void)
{
    #define READ_UID_SIZE 8
    MS_U8 u8I = 0;
    MS_U8  u8ptr[READ_UID_SIZE];
    MS_U8  u8Size = READ_UID_SIZE;

    MS_U64   u64FlashUId = 0;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if(!_HAL_SERFLASH_WaitWriteDone())
    {
        goto HAL_SERFLASH_READUID_RETURN;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_READUID_RETURN;
    }
    // SFSH_RIU_REG16(REG_SFSH_SPI_COMMAND) = ISP_SPI_CMD_RDID; // RDID
    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333); // enable trigger mode

    if(_hal_SERFLASH.u16FlashType == FLASH_IC_EN25QH16)
    {
        ISP_WRITE(REG_ISP_SPI_WDATA, 0x5A); // RDUID
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_READUID_RETURN;
        }
        for(u8I = 0; u8I < 2; u8I++)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, 0x00);
            if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
            {
                goto HAL_SERFLASH_READUID_RETURN;
            }
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, 0x80); //start address
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_READUID_RETURN;
        }
        ISP_WRITE(REG_ISP_SPI_WDATA, 0xFF);
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_READUID_RETURN;
        }
    }
    else if( _hal_SERFLASH.u16FlashType == FLASH_IC_W25Q16)
    {
        ISP_WRITE(REG_ISP_SPI_WDATA, 0x4B); // RDUID
        if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_READUID_RETURN;
        }
        for(u8I = 0;u8I < 4;u8I++)
        {
            ISP_WRITE(REG_ISP_SPI_WDATA, 0xFF); // RDUID
    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
                goto HAL_SERFLASH_READUID_RETURN;
            }
        }
    }
    else
    {
         goto HAL_SERFLASH_READUID_RETURN;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_READ);  // READ

    for ( u8I = 0; u8I < u8Size; u8I++ )
    {
        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ); // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_READUID_RETURN;
        }

        u8ptr[u8I] = ISP_READ(REG_ISP_SPI_RDATA);
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s, %d 0x%02X\n",__FUNCTION__, u8I, u8ptr[u8I]));
    }

    for(u8I = 0;u8I < 8;u8I++)
    {
        u64FlashUId <<= 8;
        u64FlashUId += u8ptr[u8I];
    }

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222); // disable trigger mode


HAL_SERFLASH_READUID_RETURN:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));

    return u64FlashUId;
}



MS_BOOL HAL_SERFLASH_ReadREMS(MS_U8 * pu8Data, MS_U32 u32Size)
{
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_RIUISP
    MS_U32 u32Index;
    MS_U8 *u8ptr = pu8Data;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if ( !_HAL_SERFLASH_WaitWriteDone() )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    if ( !_HAL_SERFLASH_WaitWriteCmdRdy() )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);           // Enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_REMS);   // READ_REMS for new MXIC Flash

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_WDATA_DUMMY);
    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_WDATA_DUMMY);
    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, 0x00); // if ADD is 0x00, MID first. if ADD is 0x01, DID first
    if ( _HAL_SERFLASH_WaitWriteDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadREMS4_return;
    }

    for ( u32Index = 0; u32Index < u32Size; u32Index++ )
    {
        ISP_WRITE(REG_ISP_SPI_RDREQ, ISP_SPI_RDREQ);   // SPI read request

        if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
        {
            goto HAL_SERFLASH_ReadREMS4_return;
        }

        u8ptr[u32Index] = ISP_READ(REG_ISP_SPI_RDATA);

        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk(" 0x%02X",  u8ptr[u32Index]));
    }

    bRet = TRUE;

        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

HAL_SERFLASH_ReadREMS4_return:

        ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

        _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#else
    return HAL_FSP_ReadREMS4(pu8Data, u32Size);
#endif
return bRet;

}
/*
MS_BOOL HAL_SERFLASH_DMA(MS_U32 u32FlashStart, MS_U32 u32DRASstart, MS_U32 u32Size)
{
    MS_BOOL bRet = FALSE;

#if defined (MSOS_TYPE_LINUX)
    struct timeval time_st;
    MS_U32 u32Timeout = SERFLASH_SAFETY_FACTOR*u32Size/(108*1000/4/8);
#else
    MS_U32 u32Timer;
    MS_U32 u32Timeout = SERFLASH_SAFETY_FACTOR;
#endif

    u32Timeout=u32Timeout; //to make compiler happy
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, 0x%08X, %d)\n", __FUNCTION__, (int)u32FlashStart, (int)u32DRASstart, (int)u32Size));

    // [URANUS_REV_A][OBSOLETE] // TODO: <-@@@ CHIP SPECIFIC
    #if 0   // TODO: review
    if (MDrv_SYS_GetChipRev() == 0x00)
    {
        // DMA program can't run on DRAM, but in flash ONLY
        return FALSE;
    }
    #endif  // TODO: review
    // [URANUS_REV_A][OBSOLETE] // TODO: <-@@@ CHIP SPECIFIC

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    ISP_WRITE_MASK(REG_ISP_CHIP_SEL, SFSH_CHIP_SEL_RIU, SFSH_CHIP_SEL_MODE_SEL_MASK);   // For DMA only

    _HAL_ISP_Disable();

    // SFSH_RIU_REG16(REG_SFSH_SPI_CLK_DIV) = 0x02; // 108MHz div3 (max. 50MHz for this ST flash) for FAST_READ

    PIU_WRITE(REG_PIU_DMA_SIZE_L, LOU16(u32Size));
    PIU_WRITE(REG_PIU_DMA_SIZE_H, HIU16(u32Size));
    PIU_WRITE(REG_PIU_DMA_DRAMSTART_L, LOU16(u32DRASstart));
    PIU_WRITE(REG_PIU_DMA_DRAMSTART_H, HIU16(u32DRASstart));
    PIU_WRITE(REG_PIU_DMA_SPISTART_L, LOU16(u32FlashStart));
    PIU_WRITE(REG_PIU_DMA_SPISTART_H, HIU16(u32FlashStart));
    // SFSH_PIU_REG16(REG_SFSH_DMA_CMD) = 0 << 5; // 0: little-endian 1: big-endian
    // SFSH_PIU_REG16(REG_SFSH_DMA_CMD) |= 1; // trigger
    PIU_WRITE(REG_PIU_DMA_CMD, PIU_DMA_CMD_LE | PIU_DMA_CMD_FIRE); // trigger

    // Wait for DMA to be done
#if defined (MSOS_TYPE_LINUX)
    SER_FLASH_TIME(time_st);
#else
    SER_FLASH_TIME(u32Timer, u32Timeout);
#endif
    do
    {
        if ( (PIU_READ(REG_PIU_DMA_STATUS) & PIU_DMA_DONE_MASK) == PIU_DMA_DONE ) // finished
        {
            bRet = TRUE;
            break;
        }
#if defined (MSOS_TYPE_LINUX)
    } while (!SER_FLASH_EXPIRE(time_st, u32Timeout));
#else
    } while (!SER_FLASH_EXPIRE(u32Timer));
#endif

    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("DMA timeout!\n"));
    }

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    return bRet;
}
*/

MS_BOOL HAL_SERFLASH_ReadStatusReg(MS_U8 *pu8StatusReg)
{
#ifndef CONFIG_RIUISP
    return HAL_FSP_ReadStatusReg(pu8StatusReg);
#else

    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    *pu8StatusReg = 0xFF;

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }
    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);           // Enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDSR);   // RDSR

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    ISP_WRITE(REG_ISP_SPI_RDREQ, 0x01); // SPI read request

    if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    *pu8StatusReg = ISP_READ(REG_ISP_SPI_RDATA);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk(" 0x%02X", *pu8StatusReg));

    bRet = TRUE;

HAL_SERFLASH_ReadStatusReg_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));

    return bRet;
#endif
}

MS_BOOL HAL_SERFLASH_ReadStatusReg2(MS_U8 *pu8StatusReg)
{
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_RIUISP

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    *pu8StatusReg = 0x00;

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if ( !_HAL_SERFLASH_WaitWriteDone() )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);           // Enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDSR2);   // RDSR2

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    ISP_WRITE(REG_ISP_SPI_RDREQ, 0x01); // SPI read request

    if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
    {
        goto HAL_SERFLASH_ReadStatusReg_return;
    }

    *pu8StatusReg = ISP_READ(REG_ISP_SPI_RDATA);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk(" 0x%02X", *pu8StatusReg));

    bRet = TRUE;

HAL_SERFLASH_ReadStatusReg_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));
#else
    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
   // MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }
    bRet = HAL_FSP_ReadStatusReg2(pu8StatusReg);
    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#endif
    return bRet;
}

MS_BOOL HAL_SERFLASH_WriteStatusReg(MS_U16 u16StatusReg)
{
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_RIUISP
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if ( !_HAL_SERFLASH_WaitWriteDone() )
    {
        goto HAL_SERFLASH_WriteStatusReg_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);          // Enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_WREN);   // WREN

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_WriteStatusReg_return;
    }

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR);      // SPI CEB dis

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_WRSR);   // WRSR

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_WriteStatusReg_return;
    }

    ISP_WRITE(REG_ISP_SPI_WDATA, (MS_U8)(u16StatusReg & 0xFF ));   // LSB

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_WriteStatusReg_return;
    }

    if((_hal_SERFLASH.u8MID == MID_GD) || (_hal_SERFLASH.u16FlashType == FLASH_IC_S25FL032K)
     ||(_hal_SERFLASH.u16FlashType == FLASH_IC_W25Q64CV) || (_hal_SERFLASH.u16FlashType == FLASH_IC_W25Q32BV))
    {
        ISP_WRITE(REG_ISP_SPI_WDATA, (MS_U8)(u16StatusReg >> 8 ));   // MSB
        //printf("write_StatusReg=[%x]\n",u16StatusReg);

        if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
        {
            goto HAL_SERFLASH_WriteStatusReg_return;
        }
    }

    bRet = TRUE;

HAL_SERFLASH_WriteStatusReg_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("\n"));
#else

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    bRet = HAL_FSP_WriteStatusReg(u16StatusReg);
    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#endif
    return bRet;
}

MS_BOOL HAL_SERFLASH_WriteExtAddrRegister(MS_U8 u8ExtAddrReg)
{
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_RIUISP
    printk("%s not implement!\n", __FUNCTION__);
    return FALSE;
#else
    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    bRet = HAL_FSP_WriteExtAddrReg(u8ExtAddrReg);
    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
#endif
    return bRet;
}

MS_BOOL HAL_SPI_EnterIBPM(void)
{
	MS_BOOL bRet = FALSE;

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    _HAL_ISP_Enable();

    if ( !_HAL_SERFLASH_WaitWriteDone() )
    {
        goto HAL_SPI_EnableIBPM_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SPI_EnableIBPM_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);           // Enable trigger mode

    ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_WPSEL);   // WPSEL

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SPI_EnableIBPM_return;
    }

	ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

	ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDSCUR);	// Read Security Register

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SPI_EnableIBPM_return;
    }

	ISP_WRITE(REG_ISP_SPI_RDREQ, 0x01); // SPI read request

    if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
    {
        goto HAL_SPI_EnableIBPM_return;
    }

    if((ISP_READ(REG_ISP_SPI_RDATA) & BIT(7)) == BIT(7))
    {
		bRet = TRUE;
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG,
        printk("MXIC Security Register 0x%02X\n", ISP_READ(REG_ISP_SPI_RDATA)));
    }

HAL_SPI_EnableIBPM_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    return bRet;
}

MS_BOOL HAL_SPI_SingleBlockLock(MS_PHYADDR u32FlashAddr, MS_BOOL bLock)
{
	MS_BOOL bRet = FALSE;

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return bRet;
    }

	if ( _bIBPM != TRUE )
	{
		printk("%s not in Individual Block Protect Mode\n", __FUNCTION__);
        return bRet;
	}

    _HAL_ISP_Enable();

	if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

	ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);	// Enable trigger mode

	if( bLock )
	{
		ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_SBLK);		// Single Block Lock Protection
	}
	else
	{
		ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_SBULK);	// Single Block unLock Protection
	}

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

	ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x10)&0xFF)));
	if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

	ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x08)&0xFF)));
	if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

	ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x00)&0xFF)));
	if(!_HAL_SERFLASH_WaitWriteDataRdy())
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

#if defined (MS_DEBUG)
	ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

	ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDBLOCK);	// Read Block Lock Status

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SPI_SingleBlockLock_return;
    }

	ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x10)&0xFF)));
	if(!_HAL_SERFLASH_WaitWriteDataRdy())
	{
	    goto HAL_SPI_SingleBlockLock_return;
	}

	ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x08)&0xFF)));
	if(!_HAL_SERFLASH_WaitWriteDataRdy())
	{
	    goto HAL_SPI_SingleBlockLock_return;
	}

	ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x00)&0xFF)));
	if(!_HAL_SERFLASH_WaitWriteDataRdy())
	{
	    goto HAL_SPI_SingleBlockLock_return;
	}

	ISP_WRITE(REG_ISP_SPI_RDREQ, 0x01); // SPI read request

	if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
	{
	    goto HAL_SPI_SingleBlockLock_return;
	}

	if( bLock )
	{
		if( ISP_READ(REG_ISP_SPI_RDATA) == 0xFF )
			bRet = TRUE;
	}
	else
	{
		if( ISP_READ(REG_ISP_SPI_RDATA) == 0x00 )
			bRet = TRUE;
	}
#else//No Ceck
	bRet = TRUE;
#endif

HAL_SPI_SingleBlockLock_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    return bRet;
}

MS_BOOL HAL_SPI_GangBlockLock(MS_BOOL bLock)
{
    MS_BOOL bRet = FALSE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d)\n", __FUNCTION__, bLock));

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return bRet;
    }

	if ( _bIBPM != TRUE )
	{
		printk("%s not in Individual Block Protect Mode\n", __FUNCTION__);
        return bRet;
	}

    _HAL_SERFLASH_ActiveFlash_Set_HW_WP(bLock);
    udelay(bLock ? 5 : 20); // when disable WP, delay more time

    _HAL_ISP_Enable();

	if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_WriteProtect_return;
    }

    ISP_WRITE(REG_ISP_SPI_COMMAND, ISP_SPI_CMD_WREN); // WREN

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SERFLASH_WriteProtect_return;
    }

	ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);	// Enable trigger mode

	if( bLock )
	{
		ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_GBLK);		// Gang Block Lock Protection
	}
	else
	{
		ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_GBULK);	// Gang Block unLock Protection
	}

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SERFLASH_WriteProtect_return;
    }

	bRet = TRUE;

HAL_SERFLASH_WriteProtect_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

	ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    if (bLock) // _REVIEW_
    {
        _HAL_SERFLASH_ActiveFlash_Set_HW_WP(bLock);
    }

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    return bRet;
}

MS_U8 HAL_SPI_ReadBlockStatus(MS_PHYADDR u32FlashAddr)
{
	MS_U8 u8Val = 0xA5;

    MS_ASSERT( MS_SERFLASH_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return u8Val;
    }

	if ( _bIBPM != TRUE )
	{
		printk("%s not in Individual Block Protect Mode\n", __FUNCTION__);
        return u8Val;
	}

    _HAL_ISP_Enable();

    if ( !_HAL_SERFLASH_WaitWriteDone() )
    {
        goto HAL_SPI_ReadBlockStatus_return;
    }

    if ( _HAL_SERFLASH_WaitWriteCmdRdy() == FALSE )
    {
        goto HAL_SPI_ReadBlockStatus_return;
    }

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x3333);          // Enable trigger mode

	ISP_WRITE(REG_ISP_SPI_WDATA, ISP_SPI_CMD_RDBLOCK);	// Read Block Lock Status

    if ( !_HAL_SERFLASH_WaitWriteDataRdy() )
    {
        goto HAL_SPI_ReadBlockStatus_return;
    }

	ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x10)&0xFF)));
	if(!_HAL_SERFLASH_WaitWriteDataRdy())
	{
	    goto HAL_SPI_ReadBlockStatus_return;
	}

	ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x08)&0xFF)));
	if(!_HAL_SERFLASH_WaitWriteDataRdy())
	{
	    goto HAL_SPI_ReadBlockStatus_return;
	}

	ISP_WRITE(REG_ISP_SPI_WDATA, BITS(7:0, ((u32FlashAddr >> 0x00)&0xFF)));
	if(!_HAL_SERFLASH_WaitWriteDataRdy())
	{
	    goto HAL_SPI_ReadBlockStatus_return;
	}

	ISP_WRITE(REG_ISP_SPI_RDREQ, 0x01); // SPI read request

	if ( _HAL_SERFLASH_WaitReadDataRdy() == FALSE )
	{
	    goto HAL_SPI_ReadBlockStatus_return;
	}

	u8Val = ISP_READ(REG_ISP_SPI_RDATA);

HAL_SPI_ReadBlockStatus_return:

    ISP_WRITE(REG_ISP_SPI_CECLR, ISP_SPI_CECLR); // SPI CEB dis

    ISP_WRITE(REG_ISP_TRIGGER_MODE, 0x2222);     // disable trigger mode

    _HAL_ISP_Disable();

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    return u8Val;
}


/*static MS_U32 _Get_Time(void)
{
    struct timespec         ts;

    getnstimeofday(&ts);
    return ts.tv_sec* 1000+ ts.tv_nsec/1000000;
}*/

/*
//FSP command , note that it cannot be access only by PM51 if secure boot on.
MS_U8 HAL_SERFLASH_ReadStatusByFSP(void)
{
   MS_U8  u8datget;
   MS_BOOL bRet = FALSE;
   bRet=HAL_FSP_ReadStatusReg(&u8datget);
   if(!bRet)
    printk("Read status failed \n");
   return  u8datget ;
}

void HAL_SERFLASH_ReadWordFlashByFSP(MS_U32 u32Addr, MS_U8 *pu8Buf)
{
#define HAL_FSP_READ_SIZE	4
    if(!HAL_FSP_Read(u32Addr,HAL_FSP_READ_SIZE,pu8Buf))
        printk("HAL_FSP_Read Fail:Addr is %x pu8Buf is :%x\n",(unsigned int)u32Addr, (unsigned int)pu8Buf);

}

void HAL_SERFLASH_CheckEmptyByFSP(MS_U32 u32Addr, MS_U32 u32ChkSize)
{
    MS_U32 i;
	MS_U32 u32FlashData = 0;

    while(HAL_SERFLASH_ReadStatusByFSP()==0x01)
    {
        printk("device is busy\n");
    }
    for(i=0;i<u32ChkSize;i+=4)
    {
        HAL_SERFLASH_ReadWordFlashByFSP(u32Addr+i,(MS_U8 *)u32FlashData);
        printk("[FSP Debug] fr = 0x%x\n", (int)u32FlashData);
        if(u32FlashData != 0xFFFFFFFF)
        {
            printk("check failed in addr:%lx value:%lx\n",u32Addr+i, u32FlashData);
            while(1);
        }
    }
}

static void HAL_SERFLASH_EraseByFSP(MS_U32 u32Addr, MS_U8 EraseCmd)
{
    HAL_FSP_SectorErase(u32Addr,EraseCmd);
}

void HAL_SERFLASH_EraseSectorByFSP(MS_U32 u32Addr) //4 // erase 4K bytes, need to aligned in 4K boundary
{
    HAL_SERFLASH_EraseByFSP(u32Addr,0x20);
}

void HAL_SERFLASH_EraseBlock32KByFSP(MS_U32 u32Addr) //4 // erase 32K bytes, need to aligned in 4K boundary
{
    HAL_SERFLASH_EraseByFSP(u32Addr,0x52);
}

void HAL_SERFLASH_EraseBlock64KByFSP(MS_U32 u32Addr) //4 // erase 64K bytes, need to aligned in 4K boundary
{
    HAL_SERFLASH_EraseByFSP(u32Addr,0xD8);
}

void HAL_SERFLASH_ProgramFlashByFSP(MS_U32 u32Addr, MS_U32 u32Data)
{

   #define HAL_FSP_WRITE_SIZE	4
   MS_U8 *pu8Write_Data = NULL;
   MS_U8 u8index;
   for (u8index=0;u8index<HAL_FSP_WRITE_SIZE;u8index++)
   {
     *pu8Write_Data=(MS_U8)(u32Data >>(u8index*8));
     if(!HAL_FSP_Write(u32Addr,HAL_FSP_WRITE_SIZE,pu8Write_Data))
     printk("HAL_FSP_WRITE Fail:Addr is %x pu8Buf is :%x\n",(int)u32Addr, (int)pu8Write_Data);
   }

}
*/
static MS_BOOL _HAL_FSP_WaitDone(void)
{
    MS_BOOL bRet = FALSE;
    MS_U32 time_st;
    SER_FLASH_TIME(time_st);
   do{
        if ( (FSP_READ(REG_FSP_DONE_FLAG) & REG_FSP_DONE_FLAG_MASK) == REG_FSP_DONE )
        {
            bRet = TRUE;
            break;
        }
    }while (!SER_FLASH_EXPIRE(time_st, SERFLASH_SAFETY_FACTOR));

    FSP_WRITE_MASK(REG_FSP_DONE_CLR,REG_FSP_CLR,REG_FSP_DONE_CLR_MASK);
    if (bRet == FALSE)
    {
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("Wait for SPI Write Cmd Ready fails!\n"));
    }
    return bRet;
}



MS_U8 HAL_FSP_ReadBufs(MS_U8 u8Idx)
{
    switch ( u8Idx )
    {
    case 0:    return (MS_U8)((FSP_READ(REG_FSP_RDB0) & 0x00FF) >> 0);
        break;
    case 1:    return (MS_U8)((FSP_READ(REG_FSP_RDB1) & 0xFF00) >> 8);
        break;
    case 2:    return (MS_U8)((FSP_READ(REG_FSP_RDB2) & 0x00FF) >> 0);
        break;
    case 3:    return (MS_U8)((FSP_READ(REG_FSP_RDB3) & 0xFF00) >> 8);
        break;
    case 4:    return (MS_U8)((FSP_READ(REG_FSP_RDB4) & 0x00FF) >> 0);
        break;
    case 5:    return (MS_U8)((FSP_READ(REG_FSP_RDB5) & 0xFF00) >> 8);
        break;
    case 6:    return (MS_U8)((FSP_READ(REG_FSP_RDB6) & 0x00FF) >> 0);
        break;
    case 7:    return (MS_U8)((FSP_READ(REG_FSP_RDB7) & 0xFF00) >> 8);
        break;
    case 8:    return (MS_U8)((FSP_READ(REG_FSP_RDB8) & 0x00FF) >> 0);
        break;
    case 9:    return (MS_U8)((FSP_READ(REG_FSP_RDB9) & 0xFF00) >> 8);
        break;
    }
    return -1;
}

void HAL_FSP_WriteBufs(MS_U8 u8Idx, MS_U8 u8Data)
{
    switch ( u8Idx )
    {
    case 0:
        FSP_WRITE_MASK(REG_FSP_WDB0,REG_FSP_WDB0_DATA(u8Data),REG_FSP_WDB0_MASK);
        break;
    case 1:
        FSP_WRITE_MASK(REG_FSP_WDB1,REG_FSP_WDB1_DATA(u8Data),REG_FSP_WDB1_MASK);
        break;
    case 2:
        FSP_WRITE_MASK(REG_FSP_WDB2,REG_FSP_WDB2_DATA(u8Data),REG_FSP_WDB2_MASK);
        break;
    case 3:
        FSP_WRITE_MASK(REG_FSP_WDB3,REG_FSP_WDB3_DATA(u8Data),REG_FSP_WDB3_MASK);
        break;
    case 4:
        FSP_WRITE_MASK(REG_FSP_WDB4,REG_FSP_WDB4_DATA(u8Data),REG_FSP_WDB4_MASK);
        break;
    case 5:
        FSP_WRITE_MASK(REG_FSP_WDB5,REG_FSP_WDB5_DATA(u8Data),REG_FSP_WDB5_MASK);
        break;
    case 6:
        FSP_WRITE_MASK(REG_FSP_WDB6,REG_FSP_WDB6_DATA(u8Data),REG_FSP_WDB6_MASK);
        break;
    case 7:
        FSP_WRITE_MASK(REG_FSP_WDB7,REG_FSP_WDB7_DATA(u8Data),REG_FSP_WDB7_MASK);
        break;
    case 8:
        FSP_WRITE_MASK(REG_FSP_WDB8,REG_FSP_WDB8_DATA(u8Data),REG_FSP_WDB8_MASK);
        break;
    case 9:
        FSP_WRITE_MASK(REG_FSP_WDB9,REG_FSP_WDB9_DATA(u8Data),REG_FSP_WDB9_MASK);
        break;
    default:
        DEBUG_SER_FLASH(E_SERFLASH_DBGLV_ERR, printk("HAL_FSP_WriteBufs fails\n"));
        break;
    }
}

void HAL_FSP_Entry(void)
{
    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return;
    }
    QSPI_WRITE_MASK(REG_ISP_SPI_MODE, SFSH_CHIP_FAST_ENABLE, SFSH_CHIP_FAST_MASK);
    if (gReadMode==E_QUAD_MODE)
    {
	    //PM_WRITE_MASK(REG_PM_SPI_IS_GPIO, PM_SPI_HOLD_IS_GPIO, PM_SPI_HOLD_GPIO_MASK);
	    //PM_WRITE_MASK(REG_PM_SPI_IS_GPIO, PM_SPI_WP_IS_GPIO, PM_SPI_WP_GPIO_MASK);
	    HAL_FSP_WriteStatusReg(0x0);
    }
}

void HAL_FSP_Exit(void)
{
    if (gReadMode==E_QUAD_MODE)
		HAL_FSP_WriteStatusReg(SF_SR_QUALD);
    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);
}


MS_BOOL HAL_FSP_EraseChip(void)
{
    MS_BOOL bRet = TRUE;
    //DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printf("%s()\n", __FUNCTION__));
    HAL_FSP_WriteBufs(0,SPI_CMD_WREN);
    HAL_FSP_WriteBufs(1,SPI_CMD_CE);
    HAL_FSP_WriteBufs(2,SPI_CMD_RDSR);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(1),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(1),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(1),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_ON,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_ON,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD,REG_FSP_RDSR_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_ON,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    /*!!! it's too short timer for warting, maybe failes!!!*/
    bRet &= _HAL_FSP_WaitDone();
    return bRet;
}

static MS_BOOL _HAL_FSP_BlockErase(MS_U32 u32FlashAddr, EN_FLASH_ERASE eSize)
{
    MS_BOOL bRet = TRUE;
    MS_U8 u8EAR = u32FlashAddr >> 24; // address bit 25:31
   // DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printf("%s(0x%08X, 0x%08X)\n", __FUNCTION__, (int)u32FlashAddr, (int)eSize));
    if (u8EAR != _u8RegEAR)
    {
        HAL_FSP_WriteExtAddrReg(u8EAR);
    }
    HAL_FSP_WriteBufs(0,SPI_CMD_WREN);
    HAL_FSP_WriteBufs(1,eSize);
    HAL_FSP_WriteBufs(2,(MS_U8)((u32FlashAddr>>0x10)&0xFF));
    HAL_FSP_WriteBufs(3,(MS_U8)((u32FlashAddr>>0x08)&0xFF));
    HAL_FSP_WriteBufs(4,(MS_U8)((u32FlashAddr>>0x00)&0xFF));
    HAL_FSP_WriteBufs(5,SPI_CMD_RDSR);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(4),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(1),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(1),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_ON,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_ON,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD,REG_FSP_RDSR_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_ON,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    bRet &= _HAL_FSP_WaitDone();
    return bRet;
}

MS_BOOL HAL_FSP_BlockErase(MS_U32 u32StartBlock, MS_U32 u32EndBlock, MS_BOOL bWait)
{
    MS_BOOL bRet = TRUE;
    MS_U32	u32Idx;
    MS_U32	u32FlashAddr = 0;
    //DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printf("%s(0x%08x, 0x%08x, %d)\n", __FUNCTION__, (unsigned int)u32StartBlock, (unsigned int)u32EndBlock, (int)bWait));
    MS_ASSERT( u32StartBlock<=u32EndBlock && u32EndBlock<NUMBER_OF_SERFLASH_SECTORS );
#ifdef CONFIG_FLASH_DISPLAY_PROGRESS
    MS_U32 u32start;
    printk("    ");
    u32start = get_timer(0);
#endif

    HAL_FSP_Entry();
    for( u32Idx = u32StartBlock; u32Idx <= u32EndBlock; u32Idx++ )
    {
        HAL_SERFLASH_BlockToAddress(u32Idx, &u32FlashAddr);
        bRet = _HAL_FSP_BlockErase(u32FlashAddr,FLASH_ERASE_64K);
        if(bRet !=TRUE)
            break;
#ifdef CONFIG_FLASH_DISPLAY_PROGRESS
        if( ((get_timer(0)-u32start)>500) ||  (u32Idx==u32EndBlock))
        {
            printk("\b\b\b\b%3d%%", (int)((u32Idx-u32StartBlock+1)*100/(u32EndBlock-u32StartBlock+1)) );
            u32start = get_timer(0);
        }
#endif
		WATCHDOG_RESET();

    }
    HAL_FSP_Exit();

    return bRet;
}

MS_BOOL HAL_FSP_SectorErase(MS_U32 u32SectorAddress,MS_U8 u8cmd)
{
    MS_BOOL bRet = TRUE;

    HAL_FSP_Entry();
    bRet &= _HAL_FSP_BlockErase(u32SectorAddress,u8cmd);
    HAL_FSP_Exit();
    return bRet;
}


MS_BOOL HAL_FSP_CheckWriteDone(void)
{
    MS_BOOL bRet = TRUE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));
    HAL_FSP_WriteBufs(0,SPI_CMD_RDSR);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(0),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(1),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_OFF,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_OFF,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_1STCMD,REG_FSP_RDSR_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_ON,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    bRet &= _HAL_FSP_WaitDone();
    return bRet;
}

MS_BOOL HAL_FSP_Write_Burst(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL bRet = TRUE;
    MS_U8  u8Status = 0;
    MS_U16 u16Data = 0;
    MS_U32 u32I, u32J, u32K;
    MS_U32 u32quotient;
    MS_U32 u32remainder;
    MS_U32 u32PageIdx;
    MS_U32 u32PageRemainder;
    //write enable
    HAL_FSP_WriteBufs(0,SPI_CMD_WREN);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(0),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    u16Data = FSP_READ(REG_FSP_CTRL);
    u16Data &= 0xFF;
    FSP_WRITE(REG_FSP_CTRL, u16Data);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);

    bRet &= _HAL_FSP_WaitDone();
    if(!bRet)
    {
        printk("Write enable command Fail!!!!\r\n");
        return bRet;
    }

    u32PageIdx = u32Size / FLASH_PAGE_SIZE;
    u32PageRemainder = u32Size % FLASH_PAGE_SIZE;
    if(u32PageRemainder)
    {
        u32PageIdx++;
        u32Size = u32PageRemainder;
    }
    for(u32K = 0; u32K < u32PageIdx; u32K++)
    {
        //write command
        HAL_FSP_WriteBufs(0,SPI_CMD_PP);
        HAL_FSP_WriteBufs(1,(MS_U8)((u32Addr>>0x10)&0xFF));
        HAL_FSP_WriteBufs(2,(MS_U8)((u32Addr>>0x08)&0xFF));
        HAL_FSP_WriteBufs(3,(MS_U8)((u32Addr>>0x00)&0xFF));
        FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(4),REG_FSP_WBF_SIZE0_MASK);
        FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
        bRet &= _HAL_FSP_WaitDone();

        if(!bRet)
        {
            printk("Write command Fail!!!!\r\n");
            return bRet;
        }
        QSPI_WRITE(REG_SPI_BURST_WRITE,REG_SPI_ENABLE_BURST);

        u32quotient = (u32Size / REG_FSP_MAX_WRITEDATA_SIZE);
        u32remainder = (u32Size % REG_FSP_MAX_WRITEDATA_SIZE);
        if(u32remainder)
        {
            u32quotient++;
            u32Size = u32remainder;
        }
        else
            u32Size = REG_FSP_MAX_WRITEDATA_SIZE;

        for( u32I = 0; u32I < u32quotient; u32I++ )
        {
            for( u32J = 0; u32J < u32Size; u32J++ )
            {
                HAL_FSP_WriteBufs(u32J,*(pu8Data+u32J));
            }
            pu8Data += u32I;
            u32Size = REG_FSP_MAX_WRITEDATA_SIZE;
            FSP_WRITE_MASK(REG_FSP_WBF_SIZE, REG_FSP_WBF_SIZE0(u32J), REG_FSP_WBF_SIZE0_MASK);
            FSP_WRITE_MASK(REG_FSP_TRIGGER, REG_FSP_FIRE, REG_FSP_TRIGGER_MASK);
            bRet &= _HAL_FSP_WaitDone();
        }

        QSPI_WRITE(REG_SPI_BURST_WRITE,REG_SPI_DISABLE_BURST);
        do
        {
            HAL_FSP_WriteBufs(0,SPI_CMD_RDSR);
            FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
            FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(1),REG_FSP_RBF_SIZE0_MASK);
            FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
            bRet &= _HAL_FSP_WaitDone();
            u8Status = HAL_FSP_ReadBufs(0);
        }while(u8Status & FLASH_OIP);
        u32Size = FLASH_PAGE_SIZE;
    }
    return bRet;
}

MS_BOOL HAL_QPI_Enable(MS_BOOL bEnable)
{
	MS_BOOL bRet = TRUE;
	HAL_FSP_WriteBufs(0,SPI_CMD_QPI);
	FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_OFF,REG_FSP_2NDCMD_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_OFF,REG_FSP_3THCMD_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_ON,REG_FSP_FSCHK_MASK);
	FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
	bRet &= _HAL_FSP_WaitDone();
	if(!bRet)
	{
		printk("FSP FAIL Timeout !!!!\r\n");
	}
    return bRet;
}

MS_BOOL HAL_QPI_RESET(MS_BOOL bEnable)
{
	MS_BOOL bRet = TRUE;
	HAL_FSP_WriteBufs(0,SPI_CMD_QPI_RST);
	FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_OFF,REG_FSP_2NDCMD_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_OFF,REG_FSP_3THCMD_MASK);
	FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_ON,REG_FSP_FSCHK_MASK);
	FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
	bRet &= _HAL_FSP_WaitDone();
	if(!bRet)
	{
		printk("FSP FAIL Timeout !!!!\r\n");
	}
    return bRet;
}

void HAL_FSP_Write_Egine(MS_U32 u32Write_Addr, MS_U8 u8Size, MS_U8 *pu8Data)
{
    MS_BOOL bRet = TRUE;
    MS_U32 u32I;
    HAL_FSP_WriteBufs(0,SPI_CMD_WREN);
    HAL_FSP_WriteBufs(1,SPI_CMD_PP);
    HAL_FSP_WriteBufs(2,(MS_U8)((u32Write_Addr>>0x10)&0xFF));
    HAL_FSP_WriteBufs(3,(MS_U8)((u32Write_Addr>>0x08)&0xFF));
    HAL_FSP_WriteBufs(4,(MS_U8)((u32Write_Addr>>0x00)&0xFF));
    for( u32I = 0; u32I < u8Size; u32I++ )
    {
        HAL_FSP_WriteBufs((5+u32I),*(pu8Data+u32I));
    }
    HAL_FSP_WriteBufs((5 + u8Size),SPI_CMD_RDSR);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1((4+u8Size)),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(1),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(1),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_ON,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_ON,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD,REG_FSP_RDSR_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_ON,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    bRet &= _HAL_FSP_WaitDone();
    if(!bRet)
    {
        printk("FSP FAIL Timeout !!!!\r\n");
    }
}

MS_BOOL HAL_FSP_Write(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
//    MS_U32 u32I;
    MS_BOOL bRet = TRUE;
    MS_U32 u32J;
    MS_U32 u32quotient;
    MS_U8  u8BufSize = REG_FSP_WRITEDATA_SIZE;
    MS_U8  u8dat_align=0;
    MS_U8  u8addr_align=0;
    MS_U8  u8remainder=0;
    MS_U8  u8writebytes=0;

    u8addr_align=((MS_U8)(u32Addr))&0xF;
    u8dat_align=u8addr_align%4;
    if(u8dat_align)
    {
        u8writebytes = 4-u8dat_align;
        HAL_FSP_Write_Egine(u32Addr,u8writebytes,pu8Data);
        u32Addr += u8writebytes;
         pu8Data += u8writebytes;
    }
    u32quotient = ((u32Size-u8writebytes)  / u8BufSize);
    u8remainder = ((u32Size-u8writebytes)  % u8BufSize);
    for(u32J = 0; u32J < u32quotient; u32J++)
    {
        HAL_FSP_Write_Egine(u32Addr,u8BufSize,pu8Data);
        u32Addr += u8BufSize;
        pu8Data += u8BufSize;
    }

    if(u8remainder)
    {
        HAL_FSP_Write_Egine(u32Addr,u8remainder,pu8Data);
        u32Addr += u8remainder;
        pu8Data += u8remainder;
    }

	return bRet;
}

MS_BOOL HAL_FSP_Read(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL bRet = TRUE;
    MS_U32 u32Idx, u32J;
    MS_U32 u32quotient;
    MS_U32 u32remainder;
    MS_U8  u8BufSize = REG_FSP_READDATA_SIZE;
    u32quotient = (u32Size / u8BufSize);
    u32remainder = (u32Size % u8BufSize);
    if(u32remainder)
    {
        u32quotient++;
        u32Size = u32remainder;
    }
    else
        u32Size = u8BufSize;

    for(u32J = 0; u32J < u32quotient; u32J++)
    {
        HAL_FSP_WriteBufs(0, SPI_CMD_READ);
        HAL_FSP_WriteBufs(1,(MS_U8)((u32Addr>>0x10)&0xFF));
        HAL_FSP_WriteBufs(2,(MS_U8)((u32Addr>>0x08)&0xFF));
        HAL_FSP_WriteBufs(3,(MS_U8)((u32Addr>>0x00)&0xFF));
	/*Lost first byte in using normal read command;Dummy Byte for Fast Read*/
        //HAL_FSP_WriteBufs(4, 0x00);
        FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(4),REG_FSP_WBF_SIZE0_MASK);
        FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(0),REG_FSP_WBF_SIZE1_MASK);
        FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);
        FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(u32Size),REG_FSP_RBF_SIZE0_MASK);
        FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
        FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);
        FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
        FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
        FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
        FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_OFF,REG_FSP_2NDCMD_MASK);
        FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_OFF,REG_FSP_3THCMD_MASK);
        FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_OFF,REG_FSP_FSCHK_MASK);
        FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
        bRet &= _HAL_FSP_WaitDone();
        if(!bRet)
        {
            printk("FSP FAIL Timeout !!!!\r\n");
        }

        for( u32Idx = 0; u32Idx < u32Size; u32Idx++ )
            *(pu8Data + u32Idx) = HAL_FSP_ReadBufs(u32Idx);
        pu8Data += u32Idx;
        u32Addr += u32Idx;
        u32Size = u8BufSize;
    }
    return bRet;
}

MS_BOOL HAL_FSP_BurstRead(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL bRet = TRUE;
    MS_U32 u32Idx, u32J;
    MS_U32 u32quotient;
    MS_U32 u32remainder;
    MS_U8  u8BufSize = REG_FSP_READDATA_SIZE;

    u32quotient = (u32Size / u8BufSize);
    u32remainder = (u32Size % u8BufSize);
    if(u32remainder)
    {
        u32quotient++;
        u32Size = u32remainder;
    }
    else
        u32Size = u8BufSize;

    HAL_FSP_WriteBufs(0, SPI_CMD_FASTREAD);
    HAL_FSP_WriteBufs(1,(MS_U8)((u32Addr>>0x10)&0xFF));
    HAL_FSP_WriteBufs(2,(MS_U8)((u32Addr>>0x08)&0xFF));
    HAL_FSP_WriteBufs(3,(MS_U8)((u32Addr>>0x00)&0xFF));
    HAL_FSP_WriteBufs(4, 0x00);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(5),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(0),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(u32Size),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_OFF,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_OFF,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_OFF,REG_FSP_FSCHK_MASK);

    for(u32J = 0; u32J < u32quotient; u32J++)
    {
        FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
        bRet &= _HAL_FSP_WaitDone();
        QSPI_WRITE(REG_SPI_BURST_WRITE,REG_SPI_ENABLE_BURST);
        FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(0),REG_FSP_WBF_SIZE0_MASK);
        if(!bRet)
	    {
            printk("FSP FAIL Timeout !!!!\r\n");
        }
        for( u32Idx = 0; u32Idx < u32Size; u32Idx++ )
            *(pu8Data + u32Idx) = HAL_FSP_ReadBufs(u32Idx);
        pu8Data += u32Idx;
        if(u32remainder && (u32remainder != u8BufSize))
        {
            u32Size = u8BufSize;
            u32remainder = u8BufSize;
            FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(u32Size),REG_FSP_RBF_SIZE0_MASK);
        }

    }
    QSPI_WRITE(REG_SPI_BURST_WRITE,REG_SPI_DISABLE_BURST);
    return bRet;
}

MS_BOOL HAL_FSP_WriteStatus(MS_U8 u8CMD, MS_U8 u8CountData, MS_U8* pu8Data)
{
    MS_BOOL bRet = TRUE;
    MS_U32 u32I;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    if(u8CountData>(REG_FSP_MAX_WRITEDATA_SIZE-2))
        return FALSE;

    HAL_FSP_WriteBufs(0,SPI_CMD_WREN);
    HAL_FSP_WriteBufs(1,u8CMD);
    for( u32I = 0; u32I < u8CountData; u32I++ )
    {
        HAL_FSP_WriteBufs((2+u32I),*(pu8Data+u32I));
    }
    HAL_FSP_WriteBufs(2+u8CountData,SPI_CMD_RDSR);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1((1+u8CountData)),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(1),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(1),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_ON,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_ON,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD,REG_FSP_RDSR_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_ON,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    bRet &= _HAL_FSP_WaitDone();
    if(!bRet)
    {
        printk("[FSP] WriteStatus FAIL Timeout !!!!\r\n");
    }
    return bRet;
}


MS_BOOL HAL_FSP_WriteProtect_Area(MS_BOOL bEnableAllArea, MS_U8 u8BlockProtectBits)
{
    MS_BOOL bRet = TRUE;
    MS_U8 u8Status=0;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d)\n", __FUNCTION__, bEnableAllArea));
    _HAL_SERFLASH_ActiveFlash_Set_HW_WP(0);
    udelay(20);
    if (gReadMode==E_QUAD_MODE)
        u8Status |= SF_SR_QUALD;

    u8Status |= SF_SR_SRWD;

    if (bEnableAllArea)
        u8Status |= SERFLASH_WRSR_BLK_PROTECT;
	else
        u8Status |= u8BlockProtectBits;

    bRet &= HAL_FSP_WriteStatus(SPI_CMD_WRSR, 1, &u8Status);

    if( bEnableAllArea )
        _HAL_SERFLASH_ActiveFlash_Set_HW_WP(bEnableAllArea);
    return bRet;
}

MS_BOOL HAL_FSP_WriteProtect(MS_BOOL bEnable)
{
    MS_BOOL bRet = TRUE;
    MS_U8 u8Status=0;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(%d)\n", __FUNCTION__, bEnable));
    _HAL_SERFLASH_ActiveFlash_Set_HW_WP(0);
    udelay(20);
    if (gReadMode==E_QUAD_MODE)
        u8Status |= SF_SR_QUALD;

    u8Status |= SF_SR_SRWD;

    if (bEnable)
        u8Status |= SERFLASH_WRSR_BLK_PROTECT;

    bRet &= HAL_FSP_WriteStatus(SPI_CMD_WRSR, 1, &u8Status);

    if( bEnable )
        _HAL_SERFLASH_ActiveFlash_Set_HW_WP(bEnable);
    return bRet;
}

MS_BOOL HAL_FSP_ReadID(MS_U8 *pu8Data, MS_U32 u32Size)
{
    MS_BOOL bRet = TRUE;
    MS_U8 *u8ptr = pu8Data;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s() in\n", __FUNCTION__));

    HAL_SERFLASH_SelectReadMode(E_FAST_MODE);//set normal mode(1-1-1)

    HAL_FSP_WriteBufs(0, SPI_CMD_RDID);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(0),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(u32Size),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_OFF,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_OFF,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_OFF,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    bRet &= _HAL_FSP_WaitDone();
    *(u8ptr + 0) = HAL_FSP_ReadBufs(0);
    *(u8ptr + 1) = HAL_FSP_ReadBufs(1);
    *(u8ptr + 2) = HAL_FSP_ReadBufs(2);
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s() out\n", __FUNCTION__));
    return bRet;
}

MS_BOOL HAL_FSP_ReadStatusReg(MS_U8 *pu8StatusReg)
{
    MS_BOOL bRet = TRUE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));
    *pu8StatusReg = 0xFF;
    HAL_FSP_WriteBufs(0,SPI_CMD_RDSR);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(0),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(1),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_OFF,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_OFF,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_OFF,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    bRet &= _HAL_FSP_WaitDone();
    *pu8StatusReg = HAL_FSP_ReadBufs(0);
    printf("%s status 0x%02x\n", __FUNCTION__, *pu8StatusReg);
    return bRet;
}

MS_BOOL HAL_FSP_ReadStatusReg2(MS_U8 *pu8StatusReg)
{
    MS_BOOL bRet = TRUE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));
    *pu8StatusReg = 0xFF;
    HAL_FSP_WriteBufs(0,SPI_CMD_RDSR2);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(0),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(1),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_OFF,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_OFF,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_OFF,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    bRet &= _HAL_FSP_WaitDone();
    *pu8StatusReg = HAL_FSP_ReadBufs(0);
    printf("%s status 0x%02x\n", __FUNCTION__, *pu8StatusReg);
    return bRet;
}

MS_BOOL HAL_FSP_ReadREMS4(MS_U8 * pu8Data, MS_U32 u32Size)
{
    MS_BOOL bRet = FALSE;
    MS_U32 u32Index;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));

    MS_ASSERT( MsOS_In_Interrupt() == FALSE );
    MS_ASSERT(_HAL_SERFLASH_Check51RunMode());

    if (FALSE == MS_SERFLASH_OBTAIN_MUTEX(_s32SERFLASH_Mutex, SERFLASH_MUTEX_WAIT_TIME))
    {
        printk("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }
    HAL_SERFLASH_SelectReadMode(E_FAST_MODE);//set normal mode(1-1-1)

    HAL_FSP_WriteBufs(0,ISP_SPI_CMD_REMS);
    HAL_FSP_WriteBufs(1,0); //dummy
    HAL_FSP_WriteBufs(2,0); //dummy
    HAL_FSP_WriteBufs(3,0); // start address
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(4),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(0),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(u32Size),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_OFF,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_OFF,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_OFF,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    bRet = _HAL_FSP_WaitDone();
    if(!bRet)
    {
        printk("FSP FAIL Timeout !!!!\r\n");
	}
    for( u32Index = 0; u32Index < u32Size; u32Index++ )
         *(pu8Data + u32Index) = HAL_FSP_ReadBufs(u32Index);

    MS_SERFLASH_RELEASE_MUTEX(_s32SERFLASH_Mutex);

    return bRet;

}

MS_BOOL HAL_FSP_WriteStatusReg(MS_U16 u16StatusReg)
{
    MS_BOOL bRet = TRUE;
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));
    HAL_FSP_WriteBufs(0,SPI_CMD_WREN);
    HAL_FSP_WriteBufs(1,SPI_CMD_WRSR);
    HAL_FSP_WriteBufs(2,(MS_U8)((u16StatusReg>>0x00)&0xFF));
    HAL_FSP_WriteBufs(3,(MS_U8)((u16StatusReg>>0x08)&0xFF));
    HAL_FSP_WriteBufs(4,SPI_CMD_RDSR);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(3),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(1),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(1),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_ON,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_ON,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD,REG_FSP_RDSR_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_ON,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    bRet &= _HAL_FSP_WaitDone();
    return bRet;
}

MS_BOOL HAL_FSP_WriteExtAddrReg(MS_U8 u8ExtAddrReg)
{
    MS_BOOL bRet = TRUE;

    if (!_bHasEAR)
        return FALSE;
    else if (u8ExtAddrReg == _u8RegEAR)
        return TRUE;

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s()", __FUNCTION__));
    HAL_FSP_WriteBufs(0,SPI_CMD_WREN);
    HAL_FSP_WriteBufs(1,SPI_CMD_WREAR);
    HAL_FSP_WriteBufs(2,u8ExtAddrReg);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE0(1),REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE1(2),REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE,REG_FSP_WBF_SIZE2(0),REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE0(0),REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE1(0),REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE,REG_FSP_RBF_SIZE2(0),REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_ENABLE,REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_NRESET,REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_INT,REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_2NDCMD_ON,REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_3THCMD_OFF,REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL,REG_FSP_FSCHK_ON,REG_FSP_FSCHK_MASK);
    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE,REG_FSP_TRIGGER_MASK);
    bRet &= _HAL_FSP_WaitDone();
    if (bRet)
    {
        _u8RegEAR = u8ExtAddrReg;
    }
    return bRet;
}

/*
void DumpMemory(MS_U32 u32Addr, MS_U32 u32Size)
{
    MS_U8* pdata = (MS_U8*)u32Addr;

    printk("Address 0x%p:", (void*)u32Addr);

    while( (MS_U32)pdata != (u32Addr+u32Size))
    {
        printk(" %02x", *pdata);
        pdata++;
    }
    printk("\n");
}

MS_BOOL CompareMemory(MS_U32 u32Addr, MS_U32 u32Addr2, MS_U32 u32Size)
{
    MS_U8* pdata = (MS_U8*)u32Addr;
    MS_U8* pdata2 = (MS_U8*)u32Addr2;

    while( (MS_U32)pdata != (u32Addr+u32Size))
    {
        if(*pdata != *pdata2)
        {
            DumpMemory(u32Addr, u32Size);
            DumpMemory(u32Addr2, u32Size);

            return FALSE;
        }
        pdata++,pdata2++;
    }

    return TRUE;
}
*/

static volatile u8 u8BDMA[256];
static MS_BOOL _HAL_PP_BDMAToFSP(MS_U32 u32Src_off, MS_U32 u32Dst_off, MS_U32 u32Len)
{

    MS_U16 u16data;
    MS_BOOL bRet = FALSE;
    MS_U32 u32Timer;
    MS_U8 u8EAR = u32Dst_off >> 24; // address bit 25:31
    U32 u32DMAAddr = (U32)u8BDMA;

    if(u32Len>256)
    {
        printk("BDMA size is too long !!!!\r\n");
        return FALSE;
    }
    if((u8EAR == 0) && ((u32Dst_off + u32Len) > 0x1000000))
    {
        printk("BDMA cross the 16MB boundary\r\n");
        return FALSE;
    }
    if(u8EAR != _u8RegEAR)
    {
        HAL_FSP_WriteExtAddrReg(u8EAR);
    }

    memcpy((void *)u32DMAAddr, (const void *)u32Src_off, u32Len);
    Chip_Flush_Memory();


    //printk("%s(0x%08X, %d)\n", __FUNCTION__, (int)u32Dst_off, (int)u32Len);


    /*BDMA */
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x0<<2)), 0x00);

    //Set source and destination path
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x2<<2)), 0X2B40); //MIU to FSP(0xB)
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x3<<2)), 0X8000); //channel0 to MIU

    // Set start address
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x4<<2)), (u32DMAAddr & 0x0000FFFF));
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x5<<2)), (u32DMAAddr >> 16)&0x0FFF);
    // Set end address
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x6<<2)), 0x0);
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x7<<2)), 0x0);
    // Set Size
    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x8<<2)), (u32Len & 0x0000FFFF));
    //WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x9<<2)), (u32Len >> 16));

    WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x0<<2)), 1); // Trigger


    /*FSP*/
    HAL_FSP_WriteBufs(0, SPI_CMD_WREN);
    HAL_FSP_WriteBufs(1, SPI_CMD_PP);
    HAL_FSP_WriteBufs(2, (MS_U8)((u32Dst_off>>0x10)&0xFF));
    HAL_FSP_WriteBufs(3, (MS_U8)((u32Dst_off>>0x08)&0xFF));
    HAL_FSP_WriteBufs(4, (MS_U8)((u32Dst_off>>0x00)&0xFF));
    HAL_FSP_WriteBufs(5, 0x00); //dummy
    HAL_FSP_WriteBufs(6, SPI_CMD_RDSR);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE, REG_FSP_WBF_SIZE0(1), REG_FSP_WBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE, REG_FSP_WBF_SIZE1(5), REG_FSP_WBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_SIZE, REG_FSP_WBF_SIZE2(1), REG_FSP_WBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE, REG_FSP_RBF_SIZE0(0), REG_FSP_RBF_SIZE0_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE, REG_FSP_RBF_SIZE1(0), REG_FSP_RBF_SIZE1_MASK);
    FSP_WRITE_MASK(REG_FSP_RBF_SIZE, REG_FSP_RBF_SIZE2(1), REG_FSP_RBF_SIZE2_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL, REG_FSP_ENABLE, REG_FSP_ENABLE_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL, REG_FSP_NRESET, REG_FSP_RESET_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL, REG_FSP_INT, REG_FSP_INT_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL, REG_FSP_2NDCMD_ON, REG_FSP_2NDCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL, REG_FSP_3THCMD_ON, REG_FSP_3THCMD_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL, REG_FSP_3THCMD, REG_FSP_RDSR_MASK);
    FSP_WRITE_MASK(REG_FSP_CTRL, REG_FSP_FSCHK_ON, REG_FSP_FSCHK_MASK);

    //set FSP Outside replace
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, u32Len&0x00FFFFFF);
    FSP_WRITE_MASK(REG_FSP_WBF_OUTSIDE, REG_FSP_WBF_REPLACED(5), REG_FSP_WBF_REPLACED_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_OUTSIDE, REG_FSP_WBF_MODE(1), REG_FSP_WBF_MODE_MASK);
    FSP_WRITE_MASK(REG_FSP_WBF_OUTSIDE, REG_FSP_WBF_OUTSIDE_ENABLE, REG_FSP_WBF_OUTSIDE_ENABLE_MASK);

    FSP_WRITE_MASK(REG_FSP_TRIGGER,REG_FSP_FIRE, REG_FSP_TRIGGER_MASK);

    //check BDMA done
    SER_FLASH_TIME(u32Timer);

    do
    {

        //check done
        u16data = READ_WORD(_hal_isp.u32BdmaBaseAddr + ((0x1)<<2));
        if(u16data & 8)
        {
            //clear done
            WRITE_WORD((_hal_isp.u32BdmaBaseAddr + (0x1<<2)), 8);
            bRet = TRUE;
            break;
        }
    } while(!SER_FLASH_EXPIRE(u32Timer, SERFLASH_SAFETY_FACTOR));

    if(bRet == FALSE)
    {
        printk("Wait for BDMA Done fails!\n");
    }

    //check FSP done
    bRet = _HAL_FSP_WaitDone();
    if(!bRet)
    {
        printk("FSP FAIL Timeout !!!!\r\n");
    }

    //reset
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, 0x0);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x0);

    //return CompareMemory(u32Src_off, u32Dst_off+0x14000000, u32Len);

    return bRet;
}



MS_BOOL HAL_FSP_Write_BDMA(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data)
{
    MS_BOOL bRet = FALSE;
    MS_U16 u16Rem, u16WriteBytes;
    MS_U8 *u8Buf = pu8Data;
#ifdef CONFIG_FLASH_DISPLAY_PROGRESS
    MS_U32 u32TotalSize = u32Size;
    MS_U32 u32start;
    printk("    ");
    u32start = get_timer(0);
#endif

    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n", __FUNCTION__, (int)u32Addr, (int)u32Size, pu8Data));
    //printk("%s(0x%08X, %d, 0x%p)\n", __FUNCTION__, (int)u32Addr, (int)u32Size, pu8Data);

    u16Rem = u32Addr % SERFLASH_PAGE_SIZE;

    if (u16Rem)
    {
        u16WriteBytes = SERFLASH_PAGE_SIZE - u16Rem;
        if (u32Size < u16WriteBytes)
        {
            u16WriteBytes = u32Size;
        }


        bRet = _HAL_PP_BDMAToFSP((MS_U32)u8Buf, u32Addr, u16WriteBytes);

        if ( bRet == TRUE )
        {
            u32Addr += u16WriteBytes;
            u8Buf   += u16WriteBytes;
            u32Size -= u16WriteBytes;
        }
        else
        {
            goto HAL_SERFLASH_Write_return;
        }
    }

    while(u32Size)
    {
        u16WriteBytes =(u32Size>SERFLASH_PAGE_SIZE) ? SERFLASH_PAGE_SIZE:u32Size;

        bRet = _HAL_PP_BDMAToFSP((MS_U32)u8Buf, u32Addr, u16WriteBytes);

        if ( bRet == TRUE  )
        {
            u32Addr += u16WriteBytes;
            u8Buf   += u16WriteBytes;
            u32Size -= u16WriteBytes;
        }
        else
        {
            goto HAL_SERFLASH_Write_return;
        }
#ifdef CONFIG_FLASH_DISPLAY_PROGRESS
        if(((get_timer(0)-u32start)>500)|| (u32Size==0))
        {
            printk("\b\b\b\b%3d%%", (int)((u32TotalSize-u32Size)*100/(u32TotalSize)) );
            u32start = get_timer(0);
        }
#endif
    }

HAL_SERFLASH_Write_return:

    return bRet;
}

MS_BOOL HAL_MAP_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer)
{
    DEBUG_SER_FLASH(E_SERFLASH_DBGLV_DEBUG, printk("%s(0x%08X, %d, %p)\n",
        __FUNCTION__, (int)u32FlashAddr, (int)u32FlashSize, user_buffer));

    if(u32FlashAddr>0xFFFFFF)
    {
        DEBUG_SER_FLASH(1, printk("%s length is out of range\n", __FUNCTION__));
        return FALSE;
    }
    memcpy(user_buffer, (const void *)(MS_SPI_ADDR|u32FlashAddr), u32FlashSize);
    return TRUE;
}
