/*
* drvSPINAND_hal.c- Sigmastar
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

#include <linux/string.h>
#include <common.h>

// Common Definition
#include "MsCommon.h"
#include "MsIRQ.h"
#include "MsTypes.h"
#include "halSPINAND_common.h"
//#include "MsOS.h"
//#include "drvMMIO.h"
#include "../../inc/common/drvSPINAND.h"

//#ifndef _DRV_SPICMD_H_
//#include "../../inc/common/drvSPICMD.h"
//#endif
#include "../../inc/common/spinand.h"

//-------------------------------------------------------------------------------------------------
//  Macro definition
//-------------------------------------------------------------------------------------------------
#define WRITE_WORD_MASK(_reg, _val, _mask)  { (*((volatile U16*)(_reg))) = ((*((volatile U16*)(_reg))) & ~(_mask)) | ((U16)(_val) & (_mask)); }
#define BDMA_READ(addr)                     READ_WORD(_hal_fsp.u32BDMABaseAddr + (addr<<2))
#define BDMA_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32BDMABaseAddr + (addr<<2),(val))
#define QSPI_READ(addr)                     READ_WORD(_hal_fsp.u32QspiBaseAddr + (addr<<2))
#define QSPI_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32QspiBaseAddr + (addr<<2),(val))
#define CLK_WRITE(addr, val)                WRITE_WORD(_hal_fsp.u32CLK0BaseAddr + (addr<<2),(val))
#define CLK_READ(addr)                      READ_WORD(_hal_fsp.u32CLK0BaseAddr + (addr<<2))
#define CLK_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_fsp.u32CLK0BaseAddr + ((addr)<<2), (val), (mask))
#define CHIP_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32CHIPBaseAddr + (addr<<2),(val))
#define CHIP_READ(addr)                     READ_WORD(_hal_fsp.u32CHIPBaseAddr + (addr<<2))
#define PM_READ(addr)                       READ_WORD(_hal_fsp.u32PMBaseAddr + (addr<<2))
#define PM_WRITE(addr, val)                 WRITE_WORD(_hal_fsp.u32PMBaseAddr + (addr<<2),(val))
#define PM_WRITE_MASK(addr, val, mask)      WRITE_WORD_MASK(_hal_fsp.u32PMBaseAddr+ ((addr)<<2), (val), (mask))
#define ISP_READ(addr)                      READ_WORD(_hal_fsp.u32ISPBaseAddr + (addr<<2))
#define CLKGEN_READ(addr)                   READ_WORD(_hal_fsp.u32ClkGenBaseAddr + (addr<<2))
#define CLKGEN_WRITE(addr, val)             WRITE_WORD(_hal_fsp.u32ClkGenBaseAddr + (addr<<2),(val))
#define CLKGEN_WRITE_MASK(addr, val, mask)  WRITE_WORD_MASK(_hal_fsp.u32ClkGenBaseAddr + ((addr)<<2), (val), (mask))

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

typedef struct
{
    U8  u8Clk;
    U16 eClkCkg;
} hal_clk_ckg_t;

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
hal_fsp_t _hal_fsp =
{
    .u32FspBaseAddr = RIU_PM_BASE + BK_FSP,
    .u32QspiBaseAddr = RIU_PM_BASE + BK_QSPI,
    .u32PMBaseAddr = RIU_PM_BASE + BK_PMSLP,
    .u32CLK0BaseAddr = RIU_BASE + BK_CLK0,
    .u32CHIPBaseAddr = RIU_BASE + BK_CHIP,
    .u32RiuBaseAddr= RIU_PM_BASE,
    .u32BDMABaseAddr = RIU_BASE + BK_BDMA,
    .u32ISPBaseAddr = RIU_PM_BASE + BK_ISP,
    .u32ClkGenBaseAddr = RIU_BASE + BK_CLKGEN,
};

//
//  Spi  Clk Table (List)
//
static hal_clk_ckg_t _hal_ckg_spi_pm[] = {
     {12 , PM_SPI_CLK_XTALI }
    ,{27 , PM_SPI_CLK_27MHZ }
    ,{36 , PM_SPI_CLK_36MHZ }
    ,{43 , PM_SPI_CLK_43MHZ }
    ,{54 , PM_SPI_CLK_54MHZ }
    ,{72 , PM_SPI_CLK_72MHZ }
    ,{86 , PM_SPI_CLK_86MHZ }
    ,{108, PM_SPI_CLK_108MHZ}
};

static hal_clk_ckg_t _hal_ckg_spi_nonpm[] = {
     {12 , CLK0_CKG_SPI_XTALI }
    ,{27 , CLK0_CKG_SPI_27MHZ }
    ,{36 , CLK0_CKG_SPI_36MHZ }
    ,{43 , CLK0_CKG_SPI_43MHZ }
    ,{54 , CLK0_CKG_SPI_54MHZ }
    ,{72 , CLK0_CKG_SPI_72MHZ }
    ,{86 , CLK0_CKG_SPI_86MHZ }
    ,{108, CLK0_CKG_SPI_108MHZ}
};

static hal_clk_ckg_t _hal_ckg_spi_nonpm6[] = {
     {12 , BITS(4:2, 0)}
    ,{72 , BITS(4:2, 7)}
    ,{86 , BITS(4:2, 6)}
    ,{108, BITS(4:2, 5)}
    ,{216, BITS(4:2, 4)}
};

U8 _u8SPINANDDbgLevel;

static BOOL FSP_WRITE_BYTE(U32 u32RegAddr, U8 u8Val)
{
    if (!u32RegAddr)
    {
        printf("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    ((volatile U8*)(_hal_fsp.u32FspBaseAddr))[(u32RegAddr << 1) - (u32RegAddr & 1)] = u8Val;
    return TRUE;
}

static BOOL FSP_WRITE(U32 u32RegAddr, U16 u16Val)
{
    if (!u32RegAddr)
    {
        printf("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    ((volatile U16*)(_hal_fsp.u32FspBaseAddr))[u32RegAddr] = u16Val;
    return TRUE;
}

static U8 FSP_READ_BYTE(U32 u32RegAddr)
{
    return ((volatile U8*)(_hal_fsp.u32FspBaseAddr))[(u32RegAddr << 1) - (u32RegAddr & 1)];
}

static U16 FSP_READ(U32 u32RegAddr)
{
    return ((volatile U16*)(_hal_fsp.u32FspBaseAddr))[u32RegAddr];
}

static BOOL _HAL_FSP_ChkWaitDone(void)
{
//consider as it spend very long time to check if FSP done, so it may implment timeout method to improve
    U16 u16Try = 0;
    U8 u8DoneFlag = 0;

    while (u16Try < CHK_NUM_WAITDONE)
    {
        u8DoneFlag = FSP_READ(REG_FSP_DONE);
        if((u8DoneFlag & DONE_FSP) == DONE_FSP)
        {
            return TRUE;
        }
        //udelay(1);//Most of the time, this would repeat 3 times so udelay(1) is too much
        u16Try++;
    }
    return FALSE;
}

static void _HAL_FSP_GetRData(U8 *pu8Data, U8 u8DataSize)
{
    U8 u8Index = 0;
    //printf("Read Pointer %lx\r\n",(U32)pu8Data);
    for (u8Index = 0; u8Index < u8DataSize; u8Index++)
    {
        pu8Data[u8Index] = FSP_READ_BYTE(REG_FSP_READ_BUFF + u8Index);
    }
}

U32 _HAL_FSP_CHECK_SPINAND_DONE(U8 *pu8Status)
{
    U8 u8Data = SPI_NAND_STAT_OIP;
    U16 u16Try = 0;
    while((u8Data & SPI_NAND_STAT_OIP) &&(u16Try < CHK_NUM_WAITDONE))
    {
        udelay(1);
        //FSP init config
        FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
        FSP_WRITE(REG_FSP_CTRL2, 0);
        FSP_WRITE(REG_FSP_CTRL4, 0);
        FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_GF);
        FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, SPI_NAND_REG_STAT);
        FSP_WRITE(REG_FSP_WRITE_SIZE, 0x2);
        FSP_WRITE(REG_FSP_READ_SIZE, 0x1);
        //Trigger FSP
        FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

        //Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            printf("CD Wait FSP Done Time Out !!!!\r\n");
            return ERR_SPINAND_TIMEOUT;
        }

        //Clear FSP done flag
        FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
        u16Try++;
        _HAL_FSP_GetRData((&u8Data), 1);
    }
    if(pu8Status)
        *pu8Status = u8Data;

    return ERR_SPINAND_SUCCESS;
}

U32 _HAL_FSP_SendOneCmd(U8 u8Cmd)
{
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, u8Cmd);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x1);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 0);
    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("WE Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return ERR_SPINAND_SUCCESS;
}

static U32 _HAL_FSP_SendOneCmd3A(U8 u8Cmd, U32 u32_PageIdx, U8 *pu8Status)
{
    U32 u32Ret;
    U8  u8Addr = 0;
    U8  u8WbufIndex = 0;
    S8  s8Index;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // FIRST COMMAND PAGE PROGRAM EXECUTE
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u8Cmd);
    u8WbufIndex++;

    //set Write Start Address
    for(s8Index = (SPI_NAND_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
    {
        u8Addr = (u32_PageIdx >> (8 * s8Index) )& 0xFF;
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex),u8Addr);
        u8WbufIndex++;
    }
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x004);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x000);

    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("W Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    u32Ret = _HAL_FSP_CHECK_SPINAND_DONE(pu8Status);
    if(u32Ret == ERR_SPINAND_SUCCESS)
        if(*pu8Status & P_FAIL)
            u32Ret = ERR_SPINAND_W_FAIL;
    return u32Ret;
}

#if 0
static U32 _HAL_SPINAND_DUMMY(void)
{
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, 0x05);
	FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, 0xff);
	FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 2, 0xff);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x3);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 0);
    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("WE Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return ERR_SPINAND_SUCCESS;

}

static U32 _HAL_SPINAND_RESET(void)
{
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, 0xFF);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x1);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 0);
    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("WE Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return ERR_SPINAND_SUCCESS;

}




static U32 _HAL_SPINAND_WRITE_DISABLE(void)
{
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_WRDIS);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x1);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 0);
    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("WE Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return ERR_SPINAND_SUCCESS;

}

#endif
static U32 _HAL_SPINAND_WRITE_ENABLE(void)
{
    return _HAL_FSP_SendOneCmd(SPI_NAND_CMD_WREN);
}

static U32 _HAL_SPINAND_RIU_READ(U16 u16Addr, U32 u32DataSize, U8 *u8pData)
{
    U32 u32Index = 0;
    U8  u8Addr = 0;
    U32 u32RealLength = 0;
    U8  u8WbufIndex = 0;
    S8  s8Index = 0;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_RFC);
    u8WbufIndex++;

    //Set Write & Read Length
    FSP_WRITE(REG_FSP_WRITE_SIZE, 4); //add 4 because it is included of command and Address setting length
    if(u32DataSize > MAX_READ_BUF_CNT)
        FSP_WRITE(REG_FSP_READ_SIZE, MAX_READ_BUF_CNT);
    else
        FSP_WRITE(REG_FSP_READ_SIZE, u32DataSize);

    for(u32Index = 0; u32Index < u32DataSize; u32Index +=MAX_READ_BUF_CNT)
    {

        for(s8Index = (SPI_NAND_PAGE_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
        {
            u8Addr = (u16Addr >> (8 * s8Index) )& 0xFF;
            FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u8Addr);
            u8WbufIndex++;
        }
        // set dummy byte
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), 0x00);
        //Trigger FSP
        FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

        //Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE){
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("RIUR Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }
        //Get Read Data
        u32RealLength = u32DataSize - u32Index;
        if(u32RealLength >= MAX_READ_BUF_CNT)
            u32RealLength = MAX_READ_BUF_CNT;

        _HAL_FSP_GetRData((u8pData + u32Index), u32RealLength);

        //Clear FSP done flag
        FSP_WRITE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
        // update Read Start Address
        u16Addr += u32RealLength;
//	        printf("u16Addr = %x \r\n",u16Addr);
        u8WbufIndex -= SPI_NAND_PAGE_ADDR_LEN;

    }
    return ERR_SPINAND_SUCCESS;
}

//TODO move to a stand-alone file later.
static U32 _HAL_SPINAND_BDMA_READ(U32 u32Addr, U32 u32DataSize, U8 *u8pData)
{
    U32 u32Addr1;
    U16 u16data;
    U32 u32Timer = 0;
    U32 u32Ret = ERR_SPINAND_TIMEOUT;

    u16data = BDMA_READ(0x00);
    if(u16data & 1)
    {
         BDMA_WRITE(0x00, 0x10);
         u16data = BDMA_READ(0x00);
         if(u16data & 0x10)
             BDMA_WRITE(0x00, 0x00);
    }
    //Set source and destination path
    BDMA_WRITE(0x00, 0x0000);
    BDMA_WRITE(0x02, 0X4035);   //5:source device (spi)
                                //3:source device data width (8 bytes)
                                //0:destination channel0
                                //4:destination device data width (16 bytes)
    BDMA_WRITE(0x03, 0x0000);   //channel0: MIU0
    // Set start address
    BDMA_WRITE(0x04, (u32Addr & 0x0000FFFF));
    BDMA_WRITE(0x05, (u32Addr>>16));
    u32Addr1 = (U32)u8pData;
    flush_cache(u32Addr1, u32DataSize);
    #if defined(CONFIG_MIPS32)
    Chip_Flush_Memory();
    #endif
    //u32Addr1 = MsOS_VA2PA(u32Addr1);
    u32Addr1 = u32Addr1 & 0x1fffffff;
    // Set end address
    BDMA_WRITE(0x06, (u32Addr1 & 0x0000FFFF));
    BDMA_WRITE(0x07, (u32Addr1 >> 16));
    // Set Size

    BDMA_WRITE(0x08, (u32DataSize & 0x0000FFFF));
    BDMA_WRITE(0x09, (u32DataSize >> 16));

    // Trigger
    BDMA_WRITE(0x00, 1);

    do
    {
        //check done
        u16data = BDMA_READ(0x01);
        if(u16data & 8)
        {
            //clear done
            BDMA_WRITE(0x01, 8);
            u32Ret = ERR_SPINAND_SUCCESS;
            break;
        }
        udelay(1);
        u32Timer++;
    } while (u32Timer < CHK_NUM_WAITDONE);

    #if defined(CONFIG_MIPS32)
    Chip_Read_Memory();
    #endif

    return u32Ret;

}

static U32 _HAL_SPINAND_BDMA_Write(U32 u32Data, U32 u32DataSize, U8 *u8pData)
{
    U16 u16data;
    U32 u32Ret = ERR_SPINAND_TIMEOUT;

    u16data = BDMA_READ(0x00);
    if(u16data & 1)
    {
         BDMA_WRITE(0x00, 0x10);
         u16data = BDMA_READ(0x00);
         if(u16data & 0x10)
             BDMA_WRITE(0x00, 0x00);
    }
    //Set source and destination path
    BDMA_WRITE(0x00, 0x0000);
    BDMA_WRITE(0x02, 0X2B40);   //0:source device (channel0)
                                //4:source device data width (16 bytes)
                                //B:destination device (MIU) 1:destination device (IMI)
                                //2:destination device data width (16 bytes)
    BDMA_WRITE(0x03, 0x0000);   //channel0: MIU0
    // Set start address
    BDMA_WRITE(0x04, (u32Data & 0x0000FFFF));
    BDMA_WRITE(0x05, ((u32Data >> 16)& 0x0FFF));

    // Set end address
    BDMA_WRITE(0x06, (0x0 & 0x0000FFFF));
    BDMA_WRITE(0x07, (0x0 >> 16));

    // Set Size
    BDMA_WRITE(0x08, (u32DataSize & 0x0000FFFF));
//    BDMA_WRITE(0x09, (u32DataSize >> 16));
    // Trigger
    BDMA_WRITE(0x00, 1);

    #if defined(CONFIG_MIPS32)
    Chip_Read_Memory();
    #endif

    return u32Ret;

}

void HAL_SPINAND_PreHandle(SPINAND_MODE eMode)
{
    U8 u8Status;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    //printf("HAL_SPINAND_PreHandle: %d TODO\n", __LINE__);
    if(pSpiNandDrv->tSpinandInfo.au8_ID[0] == MID_GD &&
            (pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xD1 || pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xD9))
    {
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_NORMAL_MODE);
        switch(eMode)
        {
            case E_SPINAND_SINGLE_MODE:
            case E_SPINAND_FAST_MODE:
            case E_SPINAND_DUAL_MODE:
            case E_SPINAND_DUAL_MODE_IO:
                HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
                u8Status &= ~(QUAD_ENABLE);
                HAL_SPINAND_WriteStatusRegister(u8Status, SPI_NAND_REG_FEAT);
                break;
            case E_SPINAND_QUAD_MODE:
            case E_SPINAND_QUAD_MODE_IO:
                HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
                u8Status |= QUAD_ENABLE;
                HAL_SPINAND_WriteStatusRegister(u8Status, SPI_NAND_REG_FEAT);
                break;
        }
    }
}
BOOL HAL_FSP_GET_SPINAND_STATUS(U8 *pu8Data)
{
    //FSP init config
    //udelay(10,1);
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);
    FSP_WRITE(REG_FSP_WRITE_BUFF, (SPI_NAND_CMD_GF|(SPI_NAND_REG_STAT<<8)));
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x2);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x1);
    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        return FALSE;
    }
    //Clear FSP done flag
    FSP_WRITE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    _HAL_FSP_GetRData(pu8Data, 1);
    return TRUE;
}

void HAL_SPINAND_Config(U32 u32PMRegBaseAddr, U32 u32NonPMRegBaseAddr)
{
    DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printf("%s(0x%08X, 0x%08X)\n", __FUNCTION__, (int)u32PMRegBaseAddr, (int)u32NonPMRegBaseAddr));
    _hal_fsp.u32FspBaseAddr = u32PMRegBaseAddr + BK_FSP;
    _hal_fsp.u32PMBaseAddr = u32PMRegBaseAddr + BK_PMSLP;
    _hal_fsp.u32QspiBaseAddr = u32PMRegBaseAddr + BK_QSPI;
    _hal_fsp.u32CLK0BaseAddr = u32NonPMRegBaseAddr + BK_CLK0;//BK_CLK0;
    _hal_fsp.u32CHIPBaseAddr = u32NonPMRegBaseAddr + BK_CHIP;//BK_CHIP;
    _hal_fsp.u32BDMABaseAddr = u32NonPMRegBaseAddr + BK_BDMA;
    _hal_fsp.u32RiuBaseAddr =  u32PMRegBaseAddr;
}

BOOL HAL_SPINAND_PLANE_HANDLER(U32 u32Addr)
{
    U8 u8i = 0;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    while(!((pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt >> u8i)&1))
    {
        u8i++;
    }
    if(u32Addr & pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt)
    {
        u32Addr = u32Addr & pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
        u32Addr = u32Addr >> u8i;
        u32Addr = u32Addr << REG_SPI_WRAP_BIT_OFFSET;

        QSPI_WRITE(REG_SPI_WRAP_VAL, (U16)u32Addr);
    }
    else
    {
       u32Addr =  QSPI_READ(REG_SPI_WRAP_VAL);
       u32Addr &= ~(REG_SPI_WRAP_BIT_MASK << REG_SPI_WRAP_BIT_OFFSET);
       QSPI_WRITE(REG_SPI_WRAP_VAL, (U16)u32Addr);
    }
    return TRUE;
}

U32 HAL_SPINAND_RFC(U32 u32Addr, U8 *pu8Data)
{
    U8  u8Addr = 0;
    U8  u8WbufIndex = 0;
    S8  s8Index;
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    //SECOND COMMAND READ COMMAND + 3BYTE ADDRESS
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_PGRD);
    u8WbufIndex++;
    //set Read Start Address

    for(s8Index = (SPI_NAND_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
    {
        u8Addr = (u32Addr >> (8 * s8Index) )& 0xFF;
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex),u8Addr);
        u8WbufIndex++;
    }

    //THIRD COMMAND GET FATURE CHECK CAHCHE READY
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x004);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x000);

    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        printf("ERR_SPINAND_TIMEOUT\n");
        return FALSE;
    }
    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return _HAL_FSP_CHECK_SPINAND_DONE(pu8Data);
}

U32 HAL_SPINAND_PROGRAM_BY_BDMA(U16 u16ColumnAddr, U16 u16DataSize, U8 *pu8Data, U8 *pu8_SpareBuf)
{
	MS_U16 u16data;
    MS_BOOL bRet;
    MS_U32 u32Timer = 0;

	bRet = _HAL_SPINAND_WRITE_ENABLE();

    if(bRet != ERR_SPINAND_SUCCESS)
        return bRet;

    bRet = HAL_SPINAND_WriteProtect(FALSE);

    if(bRet != ERR_SPINAND_SUCCESS)
    {
        return bRet;
    }

	_HAL_SPINAND_BDMA_Write((U32)pu8Data, (U32)u16DataSize, (U8*)pu8Data);

    // Write Enable

	FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
	FSP_WRITE(REG_FSP_CTRL2, 0);
	FSP_WRITE(REG_FSP_CTRL4, 0);

    //FSP
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_PP);
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, (MS_U8)((u16ColumnAddr>>0x08)&0xFF));
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 2, (MS_U8)((u16ColumnAddr>>0x00)&0xFF));
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x3);
	FSP_WRITE(REG_FSP_READ_SIZE, 0x0);

    //set FSP Outside replace
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, (u16DataSize+1)&0x00FFFFFF);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x1003);
    //trigger
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);
    bRet = ERR_SPINAND_BDMA_TIMEOUT;
    //check BDMA done
    do
    {
        //check done
        u16data = BDMA_READ(0x01);

        if((u16data & 8))
        {
            //clear done
            BDMA_WRITE(0x01, 8);
            bRet = ERR_SPINAND_SUCCESS;
            break;
        }
        udelay(1);
        u32Timer++;
    } while(u32Timer < CHK_NUM_WAITDONE);

    if(bRet != ERR_SPINAND_SUCCESS)
    {
        printk("Wait for BDMA Done fails!\n");
    }

    if(_HAL_FSP_ChkWaitDone() == FALSE)
	{
		DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("PL Wait FSP Done Time Out !!!!\r\n"));
		FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, 0x0);
		FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x0);
		return ERR_SPINAND_TIMEOUT;
	}
    //reset
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, 0x0);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x0);
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return bRet;
}

U32 HAL_SPINAND_PROGRAM_BY_BDMA4(U16 u16ColumnAddr, U16 u16DataSize, U8 *pu8Data, U8 *pu8_SpareBuf)
{
    MS_U16 u16data;
    MS_U32 u32Ret = ERR_SPINAND_BDMA_TIMEOUT;
    MS_U32 u32Timer = 0;

    u32Ret = _HAL_SPINAND_WRITE_ENABLE();

    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    u32Ret = HAL_SPINAND_WriteProtect(FALSE);

    if(u32Ret != ERR_SPINAND_SUCCESS)
    {
        return u32Ret;
    }


    // Write Enable

    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //FSP
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_QPP);
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, (MS_U8)((u16ColumnAddr>>0x08)&0xFF));
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 2, (MS_U8)((u16ColumnAddr>>0x00)&0xFF));
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x3);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x0);

    //set FSP Outside replace
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, 0x0);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x0);
    //trigger
    QSPI_WRITE(REG_SPI_BURST_WRITE, 0);
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);
    if(_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("PL Wait FSP Done Time Out !!!!\r\n"));
    #if 0
        FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, 0x0);
        FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x0);
        return ERR_SPINAND_TIMEOUT;
    #else
        u32Ret = ERR_SPINAND_TIMEOUT;;
        goto clean_up;
    #endif
    }
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    //2nd phase QUAD write
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, (u16DataSize+1)&0x0FFF);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x1000);
    QSPI_WRITE(REG_SPI_BURST_WRITE,REG_SPI_ENABLE_BURST);
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x0);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x0);
    FSP_WRITE(REG_FSP_CTRL2, FSP_QUAD_MODE);
    _HAL_SPINAND_BDMA_Write((U32)pu8Data, (U32)u16DataSize, (U8*)pu8Data);
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //check BDMA done
    do
    {
        //check done
        u16data = BDMA_READ(0x01);

        if((u16data & 8))
        {
            //clear done
            BDMA_WRITE(0x01, 8);
            u32Ret = ERR_SPINAND_SUCCESS;
            break;
        }
        udelay(1);
        u32Timer++;
    } while(u32Timer < CHK_NUM_WAITDONE);

    if(u32Ret != ERR_SPINAND_SUCCESS)
    {
        printk("Wait for BDMA Done fails!\n");
    }

    if(_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("PL Wait FSP Done Time Out !!!!\r\n"));
    #if 0
        FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, 0x0);
        FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x0);
        return ERR_SPINAND_TIMEOUT;
    #else
        u32Ret = ERR_SPINAND_TIMEOUT;
        goto clean_up;
    #endif
    }
clean_up:
    //reset
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, 0x0);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x0);
    QSPI_WRITE(REG_SPI_BURST_WRITE, 0);
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x0);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x0);
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return u32Ret;
}

U32 HAL_SPINAND_PROGRAM_LOAD_DATA(U16 u16ColumnAddr, U16 u16DataSize, U8 *pu8Data, U8 *pu8_SpareBuf)
{
    U32 u32Ret = ERR_SPINAND_TIMEOUT;
    U8  u8Addr = 0, u8DataIndex =0;
    U16 u16RealLength = 0, u26FspWriteSize = SINGLE_WRITE_SIZE;
    U8  u8WbufIndex = 0;
    S8  s8Index;
    U32 u32WrteBuf = REG_FSP_WRITE_BUFF;
    U16 u16DataIndex;
    U8 *pu8Wdata=0;
    U8  u8Status;

    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    pu8Wdata = pu8Data;
    // Write Enable
    u32Ret = _HAL_SPINAND_WRITE_ENABLE();
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    u32Ret = HAL_SPINAND_WriteProtect(FALSE);
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP write Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), SPI_NAND_CMD_PP);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x00);
    u8WbufIndex++;

    //PAGE Address
    for(s8Index = (SPI_NAND_PAGE_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
    {
        u8Addr = (u16ColumnAddr >> (8 * s8Index) )& 0xFF;
        FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), u8Addr);
        u8WbufIndex++;
    }
//	printf("u16DataSize %d\n", u16DataSize);
    for(u16DataIndex = 0; u16DataIndex < u16DataSize; u16DataIndex+= u16RealLength)
    {
        u16RealLength = u16DataSize  - u16DataIndex;
        if(u16RealLength >= SINGLE_WRITE_SIZE)
        {
            u16RealLength = SINGLE_WRITE_SIZE - u8WbufIndex;
        }
        else {
            u26FspWriteSize = u16RealLength;
        }

        //Write Data
        for(u8DataIndex = 0; u8DataIndex < u16RealLength; u8DataIndex++)
        {
            if((u16DataIndex + u8DataIndex)== pSpiNandDrv->tSpinandInfo.u16_PageByteCnt)
            {
                pu8Wdata = pu8_SpareBuf;
            }
//			printf("addr:%x %x ", (u32WrteBuf + u8WbufIndex), *pu8Wdata);
            FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), *pu8Wdata);
            u8WbufIndex++;
            pu8Wdata++;

            if(u8WbufIndex >= FSP_WRITE_BUF_JUMP_OFFSET)
            {
                u32WrteBuf = REG_FSP_WRITE_BUFF2;
                u8WbufIndex = 0;
            }
        }
        FSP_WRITE(REG_FSP_WRITE_SIZE, u26FspWriteSize);
        //Trigger FSP
        FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);


        //Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("PL Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }
        QSPI_WRITE(REG_SPI_BURST_WRITE,REG_SPI_ENABLE_BURST);
        u8WbufIndex = 0;
        u32WrteBuf = REG_FSP_WRITE_BUFF;
        //Clear FSP done flag
        FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    }
//	printf("done\n");

    QSPI_WRITE(REG_SPI_BURST_WRITE,REG_SPI_DISABLE_BURST);

    return _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);

}

#if 0
U32 HAL_SPINAND_READ_STATUS(U8 *pu8Status, U8 u8Addr)
{
    U8  u8WbufIndex = 0;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    // FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_GF);
    u8WbufIndex++;
    //SECOND COMMAND SET READ PARAMETER
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u8Addr);
    // write buffer size
    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 1);

    //Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("RS Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_SUCCESS;
    }
    //Get Read Data
    _HAL_FSP_GetRData(pu8Status, 1);

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return ERR_SPINAND_SUCCESS;
}
#endif

U32 HAL_SPINAND_BLOCKERASE(U32 u32_PageIdx)
{
    U8  u8WbufIndex = 0;
    S8  s8Index;
    U8  u8Addr;
    U32 u32Ret;
    U8  u8Status;

	u32Ret = _HAL_SPINAND_WRITE_ENABLE();
    if(u32Ret != ERR_SPINAND_SUCCESS)
    {
        return u32Ret;
    }

	u32Ret = HAL_SPINAND_WriteProtect(FALSE);
	if(u32Ret != ERR_SPINAND_SUCCESS)
    {
		return u32Ret;
    }

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP|ENABLE_SEC_CMD));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    // FIRSET COMMAND PRELOAD
	FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_WREN);
	u8WbufIndex++;

    //SECOND COMMAND SET Erase Command
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_BE);
    u8WbufIndex++;

    //set erase Start Address
    for(s8Index = (SPI_NAND_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
    {
        u8Addr = (u32_PageIdx >> (8 * s8Index) )& 0xFF;
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex),u8Addr);
        u8WbufIndex++;
    }

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x041);
    FSP_WRITE(REG_FSP_READ_SIZE,  0x000);

    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("BE Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
	u32Ret = _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);

	if(u32Ret == ERR_SPINAND_SUCCESS)
    {
        if(u8Status & E_FAIL)
        {
            u32Ret = ERR_SPINAND_E_FAIL;
            printf("ERR_SPINAND_E_FAIL\r\n");
	    }
    }
    return u32Ret;
}

U32 HAL_SPINAND_EnableOtp(BOOL bEnable)
{
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    U8 u8Status;

    if(pSpiNandDrv->tSpinandInfo.u8_IDByteCnt == 2)
    {
        if(pSpiNandDrv->tSpinandInfo.au8_ID[0] == MID_GD &&
                (pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xD1 || pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xD9))
        {
            HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
            if(bEnable)
                u8Status |=   NAND_OTP_EN;
            else
                u8Status &= ~(NAND_OTP_EN);
            HAL_SPINAND_WriteStatusRegister(u8Status, SPI_NAND_REG_FEAT);
            return ERR_SPINAND_SUCCESS;
        }
    }
    else if(pSpiNandDrv->tSpinandInfo.u8_IDByteCnt == 3)
    {
         if(pSpiNandDrv->tSpinandInfo.au8_ID[0] == MID_WINBOND &&
                (pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xAA || pSpiNandDrv->tSpinandInfo.au8_ID[2] == 0x21))
        {
            HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT);

            if(bEnable)
                u8Status |=   NAND_OTP_EN;
            else
                u8Status &= ~(NAND_OTP_EN);
            HAL_SPINAND_WriteStatusRegister(u8Status, SPI_NAND_REG_FEAT);
            return ERR_SPINAND_SUCCESS;
        }
    }

    return ERR_SPINAND_UNKNOWN_ID;
}

U32 HAL_SPINAND_LockOtp(void)
{
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    U8 u8Status;
    U32 u32Ret;

    if(pSpiNandDrv->tSpinandInfo.u8_IDByteCnt == 2)
    {
        if(pSpiNandDrv->tSpinandInfo.au8_ID[0] == MID_GD &&
                (pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xD1 || pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xD9))
        {
            HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
            u8Status |=   NAND_OTP_EN | NAND_OTP_PRT;
            HAL_SPINAND_WriteStatusRegister(u8Status, SPI_NAND_REG_FEAT);
            u32Ret =  _HAL_FSP_SendOneCmd3A(SPI_NAND_CMD_PE, 0, &u8Status);
            return u32Ret;
        }
    }
    else if(pSpiNandDrv->tSpinandInfo.u8_IDByteCnt == 3)
    {
         if(pSpiNandDrv->tSpinandInfo.au8_ID[0] == MID_WINBOND &&
                (pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xAA || pSpiNandDrv->tSpinandInfo.au8_ID[2] == 0x21))
        {
            HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
            u8Status |=   NAND_OTP_EN | NAND_OTP_PRT;
            HAL_SPINAND_WriteStatusRegister(u8Status, SPI_NAND_REG_FEAT);
            u32Ret =  _HAL_FSP_SendOneCmd3A(SPI_NAND_CMD_PE, 0, &u8Status);
            return u32Ret;
        }
    }

    return ERR_SPINAND_UNKNOWN_ID;
}

U32 HAL_SPINAND_Write(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf)
{
    //U16 u16waitTime = 0;
    U32 u32Ret;
    U8  u8Addr = 0;
    U8  u8WbufIndex = 0;
    S8  s8Index;
    U16 u16DataSize;
    U8  u8Status;
    U16 u16ColumnAddr = 0;
    //U8 *pu8Data = 0;
    //calculate write data size
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    u16DataSize = pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt + pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;
    //u32Ret = HAL_SPINAND_PROGRAM_LOAD_DATA(u16ColumnAddr, u16DataSize, u8Data, pu8_SpareBuf);

    if(pSpiNandDrv->tSpinandInfo.u8PlaneCnt && (((u32_PageIdx / pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt) & 0x1) == 1))
    {
        u16ColumnAddr = (1<<12); // plane select for MICRON & 2GB
    }

    if(BDMA_W_FLAG && (pu8_SpareBuf == NULL) && (pSpiNandDrv->tSpinandInfo.u16_PageByteCnt <= 2048))
    {
#ifdef CONFIG_MS_SPINAND_QUAD_WRITE
        u32Ret = HAL_SPINAND_PROGRAM_BY_BDMA4(u16ColumnAddr, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt, u8Data, NULL);
#else
        u32Ret = HAL_SPINAND_PROGRAM_BY_BDMA(u16ColumnAddr, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt, u8Data, NULL);
#endif
    }
    else
    {
        u16DataSize = (pu8_SpareBuf == NULL) ?
                        pSpiNandDrv->tSpinandInfo.u16_PageByteCnt :
                        pSpiNandDrv->tSpinandInfo.u16_PageByteCnt + pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt;
        u32Ret = HAL_SPINAND_PROGRAM_LOAD_DATA(u16ColumnAddr, u16DataSize, u8Data, pu8_SpareBuf);
    }

    if(u32Ret != ERR_SPINAND_SUCCESS)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG,printf("PAGE LOAD FAIL %x \r\n",u32Ret));
        return u32Ret;
    }

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // FIRST COMMAND PAGE PROGRAM EXECUTE
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_PE);
    u8WbufIndex++;

    //set Write Start Address
    for(s8Index = (SPI_NAND_ADDR_LEN - 1); s8Index >= 0 ; s8Index--)
    {
        u8Addr = (u32_PageIdx >> (8 * s8Index) )& 0xFF;
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex),u8Addr);
        u8WbufIndex++;
    }
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x004);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x000);

    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("W Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    u32Ret = _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);
    if(u32Ret == ERR_SPINAND_SUCCESS)
        if(u8Status & P_FAIL)
            u32Ret = ERR_SPINAND_W_FAIL;
    return u32Ret;
}

U32 HAL_SPINAND_Read (U32 u32Addr, U32 u32DataSize, U8 *pu8Data)
{
    U16 u16Addr = u32Addr & 0xFFFF;
    U32 ret;
//    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();

    if(RIU_FLAG)
    {
        ret = _HAL_SPINAND_RIU_READ(u16Addr, u32DataSize, pu8Data);
    }
    else
    {
#if !defined(SUPPORT_SPINAND_QUAD) || (0 == SUPPORT_SPINAND_QUAD)
         /*Config SPI waveform for BDMA*/
         QSPI_WRITE(REG_SPI_FUNC_SET, REG_SPI_ADDR2_EN|REG_SPI_DUMMY_EN|REG_SPI_WRAP_EN);
         /*Set dummy cycle*/
         QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
         /*Set mode*/
         QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_FAST_READ);
#endif
        ret = _HAL_SPINAND_BDMA_READ(u32Addr, u32DataSize, pu8Data);

        if(ret != ERR_SPINAND_SUCCESS)
        {
            printf("R Wait BDMA Done Time Out CLK !\r\n");
        }
    }
    return ret;
}

U32 HAL_SPINAND_ReadID(U32 u32DataSize, U8 *pu8Data)
{
    U16 u16Index =0;
    U32 u32RealLength = 0;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_RDID);
    //Set Start Address
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, 0x00);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);
    //Set Write & Read Length
    for(u16Index = 0; u16Index < u32DataSize; u16Index += u32RealLength)
    {
        if(u32DataSize > (MAX_READ_BUF_CNT))
        {
            FSP_WRITE(REG_FSP_READ_SIZE, MAX_READ_BUF_CNT);
            u32RealLength = MAX_READ_BUF_CNT;
        }
        else
        {
            FSP_WRITE(REG_FSP_READ_SIZE, u32DataSize);
            u32RealLength = u32DataSize;
        }

        //Trigger FSP
        FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

        //Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("RID Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }
        //Get Read Data
        _HAL_FSP_GetRData((pu8Data + u16Index), u32RealLength);

        //Clear FSP done flag
        FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    }
    return ERR_SPINAND_SUCCESS;

}

U32 HAL_SPINAND_WriteProtect(BOOL bEnable)
{
#if 1
    if(bEnable)
    {
        return HAL_SPINAND_WriteStatusRegister(SPINAND_BP0 | SPINAND_BP1 | SPINAND_BP2, SPI_NAND_REG_PROT);
    }
    return HAL_SPINAND_WriteStatusRegister(0, SPI_NAND_REG_PROT);
#else
    U8 u8WbufIndex = 0;
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_SF);
    u8WbufIndex++;
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, SPI_NAND_REG_PROT);
    u8WbufIndex++;
    if(bEnable)
        FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex,0x38);
    else
        FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex,0x00);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 3);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 0);
    //Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("WP Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return ERR_SPINAND_SUCCESS;
#endif
}

U32 HAL_SPINAND_SetMode(SPINAND_MODE eMode)
{
    switch(eMode)
    {
    case E_SPINAND_SINGLE_MODE:
#if !defined(SUPPORT_SPINAND_QUAD) || (SUPPORT_SPINAND_QUAD == 0)
        HAL_SPINAND_PreHandle(E_SPINAND_SINGLE_MODE);
#endif
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_NORMAL_MODE);
        break;
    case E_SPINAND_FAST_MODE:
#if !defined(SUPPORT_SPINAND_QUAD) || (SUPPORT_SPINAND_QUAD == 0)
        HAL_SPINAND_PreHandle(E_SPINAND_FAST_MODE);
#endif
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_FAST_READ);
        break;
    case E_SPINAND_DUAL_MODE:
#if !defined(SUPPORT_SPINAND_QUAD) || (SUPPORT_SPINAND_QUAD == 0)
        HAL_SPINAND_PreHandle(E_SPINAND_DUAL_MODE);
#endif
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_3B);
        break;
    case E_SPINAND_DUAL_MODE_IO:
#if !defined(SUPPORT_SPINAND_QUAD) || (SUPPORT_SPINAND_QUAD == 0)
        HAL_SPINAND_PreHandle(E_SPINAND_DUAL_MODE_IO);
#endif
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_DUAL);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_BB);
        break;
    case E_SPINAND_QUAD_MODE:
#if !defined(SUPPORT_SPINAND_QUAD) || (SUPPORT_SPINAND_QUAD == 0)
        HAL_SPINAND_PreHandle(E_SPINAND_QUAD_MODE);
#endif
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_6B);
        break;
    case E_SPINAND_QUAD_MODE_IO:
#if !defined(SUPPORT_SPINAND_QUAD) || (SUPPORT_SPINAND_QUAD == 0)
        HAL_SPINAND_PreHandle(E_SPINAND_QUAD_MODE_IO);
#endif
        QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_QUAD);
        QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_EB);
        break;
    }
    return ERR_SPINAND_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SPINAND_SetCKG()
/// @brief \b Function \b Description: This function is used to set ckg_spi dynamically
/// @param <IN>        \b eCkgSpi    : enumerate the ckg_spi
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully , and is restricted to Flash ability
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_SPINAND_SetCKG(U8 u8CkgSpi)
{
    BOOL Ret = FALSE;
    U8 u8nonPmIdx = 0, u8PmIdx = 0;
    U8 u8Idx;
    U8 u8Size;
    u8Size = sizeof(_hal_ckg_spi_nonpm)/ sizeof(hal_clk_ckg_t);
    DEBUG_SPINAND(E_SPINAND_DBGLV_INFO, printf("%s() u8CkgSpi %d \n", __FUNCTION__,u8CkgSpi));
    for(u8Idx = 0; u8Idx < u8Size; u8Idx++)
    {
        if(u8CkgSpi < _hal_ckg_spi_nonpm[u8Idx].u8Clk)
        {
            if(u8Idx)
                u8nonPmIdx = u8Idx - 1;
            else
                u8nonPmIdx = u8Idx;
            break;
        }
        else
            u8nonPmIdx = u8Idx;
    }

    u8Size = sizeof(_hal_ckg_spi_pm)/ sizeof(hal_clk_ckg_t);
    for(u8Idx = 0; u8Idx < u8Size; u8Idx++)
    {
        if(u8CkgSpi < _hal_ckg_spi_pm[u8Idx].u8Clk)
        {
            if(u8Idx)
                u8PmIdx = u8Idx - 1;
            else
                u8PmIdx = u8Idx;
            break;
        }
        else
            u8PmIdx = u8Idx;
    }

    if(_hal_ckg_spi_nonpm[u8nonPmIdx].eClkCkg == NULL || _hal_ckg_spi_pm[u8PmIdx].eClkCkg == NULL)
    {
         DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("CLOCK NOT SUPPORT \n"));
         return Ret;
    }
    // NON-PM Domain
    CLK_WRITE_MASK(REG_CLK0_CKG_SPI,CLK0_CLK_SWITCH_OFF,CLK0_CLK_SWITCH_MASK);
    CLK_WRITE_MASK(REG_CLK0_CKG_SPI,CLK0_CKG_SPI_108MHZ,CLK0_CKG_SPI_MASK); // set ckg_spi
   // CLK_WRITE_MASK(REG_CLK0_CKG_SPI,CLK0_CKG_SPI_54MHZ,CLK0_CKG_SPI_MASK);
    CLK_WRITE_MASK(REG_CLK0_CKG_SPI,CLK0_CLK_SWITCH_ON,CLK0_CLK_SWITCH_MASK);       // run @ ckg_spi

    // PM Doman
    if(u8CkgSpi == 86)
    {
        printk("clk:%X\n", READ_WORD(RIU_BASE+0x7000/*BK_CLKGEN*/+(0x6d << 2)));
        WRITE_WORD(RIU_BASE+BK_CLKGEN+(0x6d << 2), READ_WORD(RIU_BASE+BK_CLKGEN+(0x6d << 2)) & ~(0x20));//86m reg_ckg_86m_2digpm
        printk("clk:%X\n", READ_WORD(RIU_BASE+0x7000/*BK_CLKGEN*/+(0x6d << 2)));
    }
    else if(u8CkgSpi == 108)
    {
        // reg_ckg_bdma, 1: select BDMA clock source, select clk_miu_p according to reg_ckg_miu
        CLKGEN_WRITE(0x60, 0x10);
        // reg_ckg_miu 1'b0
        CLKGEN_WRITE_MASK(0x17, 0x18, 0x1F);
        //reg_ckg_spi 216mhz clock source
        CLKGEN_WRITE_MASK(0x32, 0x10, 0x1F);
        // reg_ckg_mcu 216Mhz
        CLKGEN_WRITE_MASK(0x01, 0x10, 0x1F);
    }
    PM_WRITE_MASK(REG_PM_CKG_SPI,PM_SPI_CLK_SWITCH_OFF,PM_SPI_CLK_SWITCH_MASK); // run @ 12M
    PM_WRITE_MASK(REG_PM_CKG_SPI,_hal_ckg_spi_pm[u8PmIdx].eClkCkg,PM_SPI_CLK_SEL_MASK); // set ckg_spi
    //PM_WRITE_MASK(REG_PM_CKG_SPI,PM_SPI_CLK_54MHZ,PM_SPI_CLK_SEL_MASK);
    PM_WRITE_MASK(REG_PM_CKG_SPI,PM_SPI_CLK_SWITCH_ON,PM_SPI_CLK_SWITCH_MASK);  // run @ ckg_spi
    Ret = TRUE;
    return Ret;
}

__weak BOOL HAL_SPINAND_DumpCkg(void)
{
    BOOL bRet = TRUE;
    U16 u16Reg;
    U8 u8Size, u8Idx, u8CkgSpi;
    hal_clk_ckg_t *pTable;

    u16Reg = PM_READ(REG_PM_CKG_SPI) & PM_SPI_CLK_SEL_MASK;
    u8Size = sizeof(_hal_ckg_spi_pm)/ sizeof(hal_clk_ckg_t);
    pTable = _hal_ckg_spi_pm;
    for(u8Idx = 0; u8Idx < u8Size; u8Idx++)
    {
        if(u16Reg == pTable[u8Idx].eClkCkg)
        {
            u8CkgSpi = pTable[u8Idx].u8Clk;
            break;
        }
    }
    if(u8Idx == u8Size)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("Unknown PM CLK %X\r\n", u16Reg));
        bRet = FALSE;
    }
    else
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("found PM %d MHz\r\n", u8CkgSpi));
    }

    u16Reg = CLKGEN_READ(0x32) & CLK0_CKG_SPI_MASK;
    u8Size = sizeof(_hal_ckg_spi_nonpm6)/ sizeof(hal_clk_ckg_t);
    pTable = _hal_ckg_spi_nonpm6;
    for(u8Idx = 0; u8Idx < u8Size; u8Idx++)
    {
        if(u16Reg == pTable[u8Idx].eClkCkg)
        {
            u8CkgSpi = pTable[u8Idx].u8Clk;
            break;
        }
    }
    if(u8Idx == u8Size)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("Unknown non-PM CLK %X\r\n", u16Reg));
        bRet = FALSE;
    }
    else
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("found non-PM %d MHz\r\n", u8CkgSpi));
    }
    return bRet;
}

void HAL_SPINAND_CSCONFIG(void)
{

}

U32 HAL_SPINAND_Read_RandomIn(U32 u32_PageIdx, U32 u32_Column, U32 u32_Byte, U8 *u8Data)
{
    U8 u8Status;
    U32 u32Ret = ERR_SPINAND_SUCCESS;

    //read data to cache first
    u32Ret = HAL_SPINAND_RFC(u32_PageIdx, &u8Status);
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    HAL_SPINAND_PLANE_HANDLER(u32_PageIdx);

    // Read SPINand Data
    u32Ret = HAL_SPINAND_Read (u32_Column, u32_Byte, u8Data);
    if(u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    if(u32Ret == ERR_SPINAND_SUCCESS)
    {
        if(u8Status == ECC_1_3_CORRECTED)
            u32Ret = ERR_SPINAND_ECC_1_3_CORRECTED;
        if(u8Status == ECC_4_6_CORRECTED)
           u32Ret = ERR_SPINAND_ECC_4_6_CORRECTED;
        if(u8Status == ECC_7_8_CORRECTED)
            u32Ret = ERR_SPINAND_ECC_7_8_CORRECTED;        
        if(u8Status == ECC_NOT_CORRECTED){
            u32Ret = ERR_SPINAND_ECC_ERROR;
            printf("ecc error P: 0x%x\r\n", u32_PageIdx);
        }
    }

    return u32Ret;

}
U8 HAL_SPINAND_ReadStatusRegister(MS_U8 *pu8Status, MS_U8 u8Addr)
{
    MS_U8 u8WbufIndex = 0;
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_GF);
    u8WbufIndex++;
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, u8Addr);
    u8WbufIndex++;

    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);

    FSP_WRITE(REG_FSP_READ_SIZE, 1);

    //Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("WP Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }
    _HAL_FSP_GetRData((pu8Status), 1);

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return ERR_SPINAND_SUCCESS;
}

U8 HAL_SPINAND_WriteStatusRegister(U8 u8Status, MS_U8 u8Addr)
{
    MS_U8 u8WbufIndex = 0;
    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_SF);
    u8WbufIndex++;
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, u8Addr);
    u8WbufIndex++;

    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, u8Status);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 3);
    FSP_WRITE(REG_FSP_READ_SIZE, 0);

    //Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("WP Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return ERR_SPINAND_SUCCESS;
}

U8 HAL_SPINAND_WB_BBM(U32 u32LBA, U32 u32PBA)
{
    U8 u8Status =0;
    _HAL_SPINAND_WRITE_ENABLE();
    HAL_SPINAND_WriteProtect(FALSE);

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    //SECOND COMMAND READ COMMAND + 3BYTE ADDRESS
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_BBM);
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, (U8)(u32LBA&0xFF00)>>8);
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 2, (U8)(u32LBA&0x00FF));
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 3, (U8)(u32PBA&0xFF00)>>8);
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 4, (U8)(u32PBA&0x00FF));
    //THIRD COMMAND GET FATURE CHECK CAHCHE READY
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x005);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x000);

    //Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        printf("ERR_SPINAND_TIMEOUT\n");
        return FALSE;
    }
    //Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);
    return ERR_SPINAND_SUCCESS;
}

U8 HAL_SPINAND_WB_DumpBBM(U8 *pu8Data)
{
    U8 u8DataSize = 0x28;
    U8 u8Index = 0;
    U8 u8RealLength = 0;

    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_READBBM);
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, 0x0);//need 1 dummy byte
    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);
    FSP_WRITE(REG_FSP_READ_SIZE, u8DataSize);
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE){
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("RIUR Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
    }

    for(u8Index = 0; u8Index < u8DataSize; u8Index +=MAX_READ_BUF_CNT)
    {
        //Get Read Data
        u8RealLength = u8DataSize - u8Index;

        if(u8RealLength >= MAX_READ_BUF_CNT)
            u8RealLength = MAX_READ_BUF_CNT;
        _HAL_FSP_GetRData((pu8Data + u8Index), u8RealLength);
        //Clear FSP done flag
        FSP_WRITE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
//	        printf("u16Addr = %x \r\n",u16Addr);
    }
    return ERR_SPINAND_SUCCESS;
}

void HAL_SPINAND_DieSelect(U8 u8Die)
{
    if(u8Die != 0)//only 2 die
        u8Die = 1;

    //FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP|RESET_FSP|INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    //Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_DIESELECT);
    //Set Start Address
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, u8Die);
    //Set Write & Read Length
    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);
    FSP_WRITE(REG_FSP_READ_SIZE, 0);

    //Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    //Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("RID Wait FSP Done Time Out !!!!\r\n"));
    }

    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
}

__weak BOOL PalSpinand_SetDriving(U32 u32Pins, u16 u16Driving)
{
    printf("\n\n Set driving on your platform is not implemented\n\n");
    return FALSE;
}
