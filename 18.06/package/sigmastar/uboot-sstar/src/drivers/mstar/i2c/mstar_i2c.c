/*
* mstar_i2c.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: alterman.lin <alterman.lin@sigmastar.com.tw>
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


#define __HAL_BUS_I2C_C__

/*=============================================================*/
// Include files
/*=============================================================*/
#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/types.h>
#include <malloc.h>
#include "mstar_i2c.h"
#include "mstar_i2c_reg_infinity2m.h"

/*=============================================================*/
// Macro definition
/*=============================================================*/

#define HWI2C_STARTDLY                5 //us
#define HWI2C_STOPDLY                 5 //us
/*=============================================================*/
// Data type definition
/*=============================================================*/


/*=============================================================*/
// Variable definition
/*=============================================================*/
I2C_DMA HWI2C_DMA[HAL_HWI2C_PORTS];
static BOOL g_bDMAEnable[HAL_HWI2C_PORTS];
static U32 g_u32StartDelay = HWI2C_STARTDLY, g_u32StopDelay = HWI2C_STOPDLY;
static BOOL _gbInit = FALSE;
static HWI2C_ReadMode g_HWI2CReadMode[HAL_HWI2C_PORTS];
static U16 g_u16DelayFactor[HAL_HWI2C_PORTS];
/*=============================================================*/
// Local function definition
/*=============================================================*/

/*=============================================================*/
// Global function definition
/*=============================================================*/
void _I2CTurnOn(void)
{
    // CLKGEN IIC0 & IIC1 CLK
    HAL_HWI2C_WriteClk2Byte(0x37 * 2, 0x0808);
    //UartSendTrace("clk: %#x\n", HAL_HWI2C_ReadClk2Byte(0x37 * 2));
}


void _I2CTurnOff(void)
{
    // CLKGEN IIC0 & IIC1 CLK
    HAL_HWI2C_WriteClk2Byte(0x37 * 2, 0x0);
    //UartSendTrace("clk: %#x\n", HAL_HWI2C_ReadClk2Byte(0x37 * 2));
}

void showRegAddr(volatile u16 *regBase)
{
    int i;
    UartSendTrace("Register Addr: %p \n", regBase);
    for(i = 0; i < 0x80; i += 8)
    {
        UartSendTrace("0x%02x: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x \n", i,
                   *(regBase + (i * 2)),
                   *(regBase + ((i + 1) * 2)),
                   *(regBase + ((i + 2) * 2)),
                   *(regBase + ((i + 3) * 2)),
                   *(regBase + ((i + 4) * 2)),
                   *(regBase + ((i + 5) * 2)),
                   *(regBase + ((i + 6) * 2)),
                   *(regBase + ((i + 7) * 2))
                  );
    }
}

/**
 * @brief I2C initialization
 * @param [in] Speed  clock bit rate
 * @return none
 */
I2CErrorCode_e HalI2cInitStandard(void)
{
#if 0
    U16 u16SlaveCfg;
    U32 uAddrCnt;
    U8 pRegAddr[2] = {0x30, 0x00};
#endif
#if defined(CONFIG_SYS_I2C_0_PADMUX)
    Hal_HW_IIC_Init(g_ptI2C0, g_ptCHIPTOP, 0, g_ptCLKGEN, CONFIG_SYS_I2C_0_PADMUX);
#else
    Hal_HW_IIC_Init(g_ptI2C0, g_ptCHIPTOP, 0, g_ptCLKGEN, 1);
#endif

#if defined(CONFIG_SYS_I2C_1_PADMUX)
    Hal_HW_IIC_Init(g_ptI2C1, g_ptCHIPTOP, 1, g_ptCLKGEN, CONFIG_SYS_I2C_1_PADMUX);
#else
    Hal_HW_IIC_Init(g_ptI2C1, g_ptCHIPTOP, 1, g_ptCLKGEN, 1);
#endif
    _I2CTurnOn();
#if 0
    showRegAddr(g_ptCHIPTOP);
    showRegAddr(g_ptCLKGEN);
    showRegAddr(g_ptI2C0);
    showRegAddr(g_ptI2C1);
#endif
    // I2C1 DMA test
#if 0
    *(g_ptI2C1) = 0x6e;
    *(g_ptI2C1 + (0x20 * 2)) = 0x0; //riu_w 0x1119 0x20 0
    *(g_ptI2C1 + (0x29 * 2)) = 0x3; //riu_w 0x1119 0x29 3
    *(g_ptI2C1 + (0x2a * 2)) = 0x3; //riu_w 0x1119 0x2a 3
    *(g_ptI2C1 + (0x2f * 2)) = 0x1; //riu_w 0x1119 0x2f 1
    *(g_ptI2C1 + (0x24 * 2)) = 0x0; //riu_w 0x1119 0x24 0
#endif
#if 0
    u16SlaveCfg = (0x34 + (0x1 << 8));
    uAddrCnt = 3;

    HAL_HWI2C_DMA_WriteBytes(0x100, u16SlaveCfg, uAddrCnt, pRegAddr, uAddrCnt, pRegAddr);
    HAL_HWI2C_DMA_ReadBytes(0x100, u16SlaveCfg, uAddrCnt, pRegAddr, uAddrCnt, pRegAddr);
    UartSendTrace("reg[0]: %#x \n", pRegAddr[0]);
#endif

    return I2C_PROC_DONE;
}

void Hal_HW_IIC_Init(volatile void *base, volatile void *chipbase, unsigned int i2cgroup, volatile void *clkbase, int i2cpadmux)
{
    HWI2C_UnitCfg pHwbuscfg[1];
    U8 j;

    if (i2cgroup >= HAL_HWI2C_PORTS)
        return;
    memset(pHwbuscfg, 0, sizeof(HWI2C_UnitCfg));

    //We only initialze sCfgPort[0]
    for(j = 0 ; j < 1 ; j++)
    {
        pHwbuscfg[0].sCfgPort[j].bEnable = TRUE;
        //use default pad mode 1
        if(i2cgroup == 0)
        {
            if(i2cpadmux == 1)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_1;
            if(i2cpadmux == 2)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_2;
            if(i2cpadmux == 3)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_3;
            if(i2cpadmux == 4)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_4;
        }else if(i2cgroup == 1)
        {
            if(i2cpadmux == 1)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_1;
            if(i2cpadmux == 2)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_2;
            if(i2cpadmux == 3)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_3;
            if(i2cpadmux == 4)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_4;
            if(i2cpadmux == 5)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_5;
        }else if(i2cgroup == 2)
            pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_1;
        else if(i2cgroup == 3)
            pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_1;
        pHwbuscfg[0].sCfgPort[j].eSpeed = E_HWI2C_NORMAL; //E_HAL_HWI2C_CLKSEL_VSLOW;//pIIC_Param->u8ClockIIC;//
        pHwbuscfg[0].sCfgPort[j].eReadMode = E_HWI2C_READ_MODE_DIRECT;//pIIC_Param->u8IICReadMode;//
		// I1 doesn't have DMA mode
        if((i2cgroup == 0) ||(i2cgroup == 1))
        {
            pHwbuscfg[0].sCfgPort[j].bDmaEnable = FALSE;  //Use default setting
            pHwbuscfg[0].sCfgPort[j].u32DmaPhyAddr = 0;  //Use default setting
        }
        else if(i2cgroup < HAL_HWI2C_PORTS)
        {
            pHwbuscfg[0].sCfgPort[j].bDmaEnable = TRUE;  //Use default setting
            HWI2C_DMA[i2cgroup].i2c_virt_addr = malloc(4096);
            UartSendTrace("bus[%d]: virt_addr %p \n", i2cgroup, HWI2C_DMA[i2cgroup].i2c_virt_addr);
            HWI2C_DMA[i2cgroup].i2c_dma_addr = HWI2C_DMA[i2cgroup].i2c_virt_addr;
            UartSendTrace("bus[%d]: phys_addr %#x \n", i2cgroup, (U32)HWI2C_DMA[i2cgroup].i2c_dma_addr);
            pHwbuscfg[0].sCfgPort[j].u32DmaPhyAddr = (u32)HWI2C_DMA[i2cgroup].i2c_dma_addr;  //Use default setting
        }
        pHwbuscfg[0].sCfgPort[j].eDmaAddrMode = E_HWI2C_DMA_ADDR_NORMAL;  //Use default setting
        pHwbuscfg[0].sCfgPort[j].eDmaMiuPri = E_HWI2C_DMA_PRI_LOW;  //Use default setting
        pHwbuscfg[0].sCfgPort[j].eDmaMiuCh = E_HWI2C_DMA_MIU_CH0;  //Use default setting
        j++;
    }

    pHwbuscfg[0].sI2CPin.bEnable = FALSE;
    pHwbuscfg[0].sI2CPin.u8BitPos = 0;
    pHwbuscfg[0].sI2CPin.u32Reg = 0;
    pHwbuscfg[0].eSpeed = E_HWI2C_NORMAL; //E_HAL_HWI2C_CLKSEL_VSLOW;//pIIC_Param->u8ClockIIC;//
    pHwbuscfg[0].ePort = pHwbuscfg[0].sCfgPort[0].ePort; /// port
    pHwbuscfg[0].eReadMode = E_HWI2C_READ_MODE_DIRECT; //pIIC_Param->u8IICReadMode;//
    pHwbuscfg[0].eBaseAddr = (U32)base;
    pHwbuscfg[0].eChipAddr = (U32)chipbase;
    pHwbuscfg[0].eClkAddr = (U32)clkbase;
    pHwbuscfg[0].eGroup = i2cgroup;

    Hal_HWI2C_Init(&pHwbuscfg[0]);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: Hal_HWI2C_Init
/// @brief \b Function  \b Description: Init HWI2C driver
/// @param psCfg        \b IN: hw I2C config
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL Hal_HWI2C_Init(HWI2C_UnitCfg *psCfg)
{
    BOOL bRet = TRUE;
    //U8 u8Port=0;

    HAL_HWI2C_SetIOMapBase(psCfg->eGroup, psCfg->eBaseAddr, psCfg->eChipAddr, psCfg->eClkAddr);

    //(2) Initialize pad mux and basic settings
    //UartSendTrace("Pinreg:%x bit:%u enable:%u speed:%u\n", psCfg->sI2CPin.u32Reg, psCfg->sI2CPin.u8BitPos, psCfg->sI2CPin.bEnable, psCfg->eSpeed);
    bRet &= HAL_HWI2C_Init_Chip();
    //(3) Initialize all port
    bRet &= _Hal_HWI2C_InitPort(psCfg);
    //(4) Check final result
    if(!bRet)
    {
        UartSendTrace("I2C init fail!\n");
    }

    //(5) Extra procedure to do after initialization
    HAL_HWI2C_Init_ExtraProc();

    g_u32StartDelay = HWI2C_STARTDLY;
    g_u32StopDelay = HWI2C_STOPDLY;
    //UartSendTrace("START default delay %d(us)\n",(int)g_u32StartDelay);
    //UartSendTrace("STOP default delay %d(us)\n",(int)g_u32StopDelay);
    _gbInit = TRUE;

#if 0
    UartSendTrace("HWI2C_MUTEX_CREATE!\n");
    for(u8Port = 0; u8Port < (U8)HAL_HWI2C_PORTS; u8Port++)
    {
        HWI2C_MUTEX_CREATE(u8Port);
    }
    UartSendTrace("HWI2C(%d): initialized\n", psCfg->eGroup);
#endif
    return bRet;
}

BOOL _Hal_HWI2C_SelectPort(HWI2C_PORT ePort)
{
    U16 u16Offset = 0x00;
    U8 u8Port = 0x00;
    BOOL bRet = TRUE;

    //(1) Get port index by port number
    if(!HAL_HWI2C_GetPortIdxByPort((HAL_HWI2C_PORT)ePort, &u8Port))
        return FALSE;

    //(2) Set pad mux for port number
    bRet &= HAL_HWI2C_SelectPort((HAL_HWI2C_PORT)ePort);

    //(3) configure port register offset ==> important
    bRet &= HAL_HWI2C_SetPortRegOffset(u8Port, &u16Offset);

    //(4) master init
    bRet &= HAL_HWI2C_Master_Enable(u16Offset);

    return bRet;
}

static BOOL _Hal_HWI2C_SetReadMode(U8 u8Port, HWI2C_ReadMode eReadMode)
{
    //UartSendTrace("Port%d Readmode: %u\n", u8Port, eReadMode);
    //check support port index
    if(u8Port >= HAL_HWI2C_PORTS)
    {
        UartSendTrace("Port index is %d >= max supported ports %d !\n", u8Port, HAL_HWI2C_PORTS);
        return FALSE;
    }
    if(eReadMode >= E_HWI2C_READ_MODE_MAX)
        return FALSE;
    g_HWI2CReadMode[u8Port] = eReadMode;
    return TRUE;
}

BOOL _Hal_HWI2C_GetPortRegOffset(U8 u8Port, U16 *pu16Offset)
{

    if(u8Port >= HAL_HWI2C_PORTS)
    {
        UartSendTrace("Port index is %d >= max supported ports %d !\n", u8Port, HAL_HWI2C_PORTS);
        return FALSE;
    }
    return HAL_HWI2C_SetPortRegOffset(u8Port, pu16Offset);
}


BOOL _Hal_HWI2C_SetClk(U8 u8Port, HWI2C_CLKSEL eClk)
{
    U16 u16Offset = 0x00;

    //UartSendTrace("Port%d clk: %u\n", u8Port, eClk);

    //check support port index
    if(u8Port >= HAL_HWI2C_PORTS)
    {
        UartSendTrace("Port index is %d >= max supported ports %d !\n", u8Port, HAL_HWI2C_PORTS);
        return FALSE;
    }
    //check support clock speed
    if(eClk >= E_HWI2C_NOSUP)
    {
        UartSendTrace("Clock [%u] is not supported!\n", eClk);
        return FALSE;
    }

    //configure port register offset ==> important
    if(!_Hal_HWI2C_GetPortRegOffset(u8Port, &u16Offset))
    {
        UartSendTrace("Port index error!\n");
        return FALSE;
    }

    g_u16DelayFactor[u8Port] = (U16)(1 << (eClk));
    //UartSendTrace("Port%d clk: %u offset:%d\n", u8Port, eClk, u16Offset);

    return HAL_HWI2C_SetClk(u16Offset, (HAL_HWI2C_CLKSEL)eClk);
}

BOOL _Hal_HWI2C_InitPort(HWI2C_UnitCfg *psCfg)
{
    U8 u8PortIdx = 0, u8Port = 0;
    U16 u16Offset = 0x00;
    BOOL bRet = TRUE;
    HWI2C_PortCfg stPortCfg;

    //(1) set default value for port variables
    for(u8PortIdx = 0; u8PortIdx < HAL_HWI2C_PORTS; u8PortIdx++)
    {
        stPortCfg = psCfg->sCfgPort[u8PortIdx];
        if(stPortCfg.bEnable)
        {
            if(HAL_HWI2C_GetPortIdxByPort(stPortCfg.ePort, &u8Port) && _Hal_HWI2C_SelectPort(stPortCfg.ePort))
            {
                //set clock speed
                bRet &= _Hal_HWI2C_SetClk(u8Port, stPortCfg.eSpeed);
                //set read mode
                bRet &= _Hal_HWI2C_SetReadMode(u8Port, stPortCfg.eReadMode);
                //get port index
                bRet &= HAL_HWI2C_SetPortRegOffset(u8Port, &u16Offset);
                //master init
                bRet &= HAL_HWI2C_Master_Enable(u16Offset);
                //dma init
                bRet &= HAL_HWI2C_DMA_Init(u16Offset, (HAL_HWI2C_PortCfg*)&stPortCfg);
                g_bDMAEnable[u8Port] = stPortCfg.bDmaEnable;
                //dump port information
                //UartSendTrace("Port:%u Index=%u\n", u8Port, stPortCfg.ePort);
                //UartSendTrace("Enable=%u\n", stPortCfg.bEnable);
                //UartSendTrace("DmaReadMode:%u\n", stPortCfg.eReadMode);
                //UartSendTrace("Speed:%u\n", stPortCfg.eSpeed);
                //UartSendTrace("DmaEnable:%u\n", stPortCfg.bDmaEnable);
                //UartSendTrace("DmaAddrMode:%u\n", stPortCfg.eDmaAddrMode);
                //UartSendTrace("DmaMiuCh:%u\n", stPortCfg.eDmaMiuCh);
                //UartSendTrace("DmaMiuPri:%u\n", stPortCfg.eDmaMiuPri);
                //UartSendTrace("DmaPhyAddr:%x\n", stPortCfg.u32DmaPhyAddr);
            }
        }
    }

    //(2) check initialized port : override above port configuration
    if(HAL_HWI2C_GetPortIdxByPort(psCfg->ePort, &u8Port) && _Hal_HWI2C_SelectPort(psCfg->ePort))
    {
        //set clock speed
        bRet &= _Hal_HWI2C_SetClk(u8Port, psCfg->eSpeed);
        //set read mode
        bRet &= _Hal_HWI2C_SetReadMode(u8Port, psCfg->eReadMode);
        //get port index
        //configure port register offset ==> important
        bRet &= HAL_HWI2C_SetPortRegOffset(u8Port, &u16Offset);
        //master init
        bRet &= HAL_HWI2C_Master_Enable(u16Offset);
    }

    //(3) dump allocated port information
    /*for(u8PortIdx=0; u8PortIdx < HWI2C_PORTS; u8PortIdx++)
    {
        HWI2C_DBG_INFO("HWI2C Allocated Port[%d] = 0x%02X\n",u8PortIdx,g_HWI2CPort[u8PortIdx]);
    }*/

    return bRet;
}


/**
 * @brief I2C send bytes through I2C controller
 * @param [in] nAddr         Address of the device to write
 * @param [in] nNbItems      Nb of bytes to send
 * @param [in] pDataToWrite Data buffer (bytes) to send
 * @param [in] nStopBit      Send a Stop bit or not (unused)
 * @return eReturnValue
 */
I2CErrorCode_e HalI2cSendDataStandard(u8 nPortNum, u8 nAddr, u8 nSpeed, bool bReStart, s32 nNbItems, const u8 *pDataToWrite, bool nStopBit)
{
    I2CErrorCode_e eReturnValue = I2C_PROC_DONE;

    HAL_HWI2C_XferWrite(nPortNum, nAddr, 0, 0, (u8 *)pDataToWrite, nNbItems);

    return eReturnValue;
}

/**
 * @brief I2C receive bytes through I2C controller
 * @param [in] nAddr          Address of the device to read
 * @param [in] nNbItems       Nb of bytes to read
 * @param [out] pDataToRead  Datas buffer where to put data (bytes) read
 * @return none
 */
I2CErrorCode_e HalI2cRead(u8 nPortNum, u8 nAddr, u8 nSpeed, bool bReStart, u32 nNbItemsToRead, u8 *pDataToRead)
{
    I2CErrorCode_e eReturnValue = I2C_PROC_DONE;
    HAL_HWI2C_XferRead(nPortNum, nAddr, 0, 0, pDataToRead, nNbItemsToRead);
    return eReturnValue;
}


I2CErrorCode_e HalI2cWriteAndRead(u8 nPortNum, u8 nAddr, u8 nSpeed, bool bReStart, u32 nNbItemsToWrite, u8 *pDataToWrite, u32 nNbItemsToRead, u8 *pDataToRead)
{
    I2CErrorCode_e eReturnValue = I2C_PROC_DONE;
    return eReturnValue;
}


/**
 * @brief I2C receive bytes through I2C controller
 * @param [in] nAddr          Address of the device to read
 * @param [in] nNbItems       Nb of bytes to read
 * @param [out] pDataToRead  Datas buffer where to put data (bytes) read
 * @return none
 */
I2CErrorCode_e HalI2cReceiveStandard(u8 nPortNum, u8 nAddr, u8 nSpeed, bool bReStart, u32 nNbItemsToWrite, u8 *pDataToWrite, u32 nNbItemsToRead, u8 *pDataToRead)
{
    if(nNbItemsToWrite == 0)
    {
        return HalI2cRead(nPortNum, nAddr, nSpeed, bReStart, nNbItemsToRead, pDataToRead);
    }
    else
    {
        return HalI2cWriteAndRead(nPortNum, nAddr, nSpeed, bReStart, nNbItemsToWrite, pDataToWrite, nNbItemsToRead, pDataToRead);
    }

}


/**
 * @brief Test if a device is connected on the bus.
 *
 * NOT IMPLEMENTED. This function always return FALSE
 *
 * @param [in] Address  Address of the device to test
 * @return TRUE if the slave answers, FALSE otherwise
 */
I2CErrorCode_e HalI2cIsConnectedStandard(u8 nAddress)
{
    return I2C_ERROR;
}

#define LOWBYTE(w)                ((w) & 0x00ff)
#define HIBYTE(w)                (((w) >> 8) & 0x00ff)


////////////////////////////////////////////////////////////////////////////////
// Define & data type
///////////////////////////////////////////////////////////////////////////////
#define HWI2C_HAL_RETRY_TIMES     (3)
#define HWI2C_HAL_WAIT_TIMEOUT    65535////30000//(1500)
#define HWI2C_HAL_FUNC()              //{UartSendTrace("=============%s\n",  __func__);}
#define HWI2C_HAL_INFO(x, args...)    //{UartSendTrace(x, ##args);}
#define HWI2C_HAL_ERR(x, args...)     {UartSendTrace(x, ##args);}
#ifndef UNUSED
#define UNUSED(x) ((x)=(x))
#endif

#define HWI2C_DMA_CMD_DATA_LEN      7
#define HWI2C_DMA_WAIT_TIMEOUT      (30000)
#define HWI2C_DMA_WRITE             0
#define HWI2C_DMA_READ              1

////////////////////////////////////////////////////////////////////////////////
// Local variable
////////////////////////////////////////////////////////////////////////////////
static U32 _gMIO_MapBase[HAL_HWI2C_PORTS] = {0};
static U32 _gMChipIO_MapBase = 0;
static U32 _gMClkIO_MapBase = 0;
static BOOL g_bLastByte[HAL_HWI2C_PORTS];
static U32 g_u32DmaPhyAddr[HAL_HWI2C_PORTS];
static HAL_HWI2C_PortCfg g_stPortCfg[HAL_HWI2C_PORTS];
static U16 g_u16DmaDelayFactor[HAL_HWI2C_PORTS];
//I2C_DMA HWI2C_DMA[HAL_HWI2C_PORTS];
////////////////////////////////////////////////////////////////////////////////
// Extern Function
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function Declaration
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Local Function
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Global Function
////////////////////////////////////////////////////////////////////////////////


void HAL_HWI2C_ExtraDelay(U32 u32Us)
{
    // volatile is necessary to avoid optimization
    U32 volatile u32Dummy = 0;
    //U32 u32Loop;
    U32 volatile u32Loop;

    u32Loop = (U32)(50 * u32Us);
    while(u32Loop--)
    {
        u32Dummy++;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_SetIOMapBase
/// @brief \b Function  \b Description: Dump bdma all register
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b None :
////////////////////////////////////////////////////////////////////////////////
void HAL_HWI2C_SetIOMapBase(U8 u8Port, U32 u32Base, U32 u32ChipBase, U32 u32ClkBase)
{
    HWI2C_HAL_FUNC();

    _gMIO_MapBase[u8Port] = u32Base;
    _gMChipIO_MapBase = u32ChipBase;
    _gMClkIO_MapBase = u32ClkBase;

    //UartSendTrace(" _gMIO_MapBase[%d]: %x \n", u8Port, u32Base);
    //UartSendTrace(" _gMChipIO_MapBase: %x\n", u32ChipBase);
    //UartSendTrace(" _gMClkIO_MapBase: %x \n", u32ClkBase);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_ReadByte
/// @brief \b Function  \b Description: read 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b U8
////////////////////////////////////////////////////////////////////////////////
U8 HAL_HWI2C_ReadByte(U32 u32RegAddr)
{
    U16 u16value;
    U8 u8Port;

    HWI2C_HAL_FUNC();
    HAL_HWI2C_GetPortIdxByOffset(u32RegAddr & (~0xFF), &u8Port);
    u32RegAddr &= 0xFF;

    HWI2C_HAL_INFO("HWI2C IOMap base:%8x u32RegAddr:%8x u32RegAddr:%8x\n", _gMIO_MapBase[u8Port], u32RegAddr, (_gMIO_MapBase[u8Port] + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2)));

    u16value = (*(volatile U32*)(_gMIO_MapBase[u8Port] + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2)));
    HWI2C_HAL_INFO("u16value===:%4x\n", u16value);
    return ((u32RegAddr & 0xFF) % 2) ? HIBYTE(u16value) : LOWBYTE(u16value);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Read4Byte
/// @brief \b Function  \b Description: read 2 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b U16
////////////////////////////////////////////////////////////////////////////////
U16 HAL_HWI2C_Read2Byte(U32 u32RegAddr)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();
    HAL_HWI2C_GetPortIdxByOffset(u32RegAddr & (~0xFF), &u8Port);
    u32RegAddr &= 0xFF;

    HWI2C_HAL_INFO("HWI2C IOMap base:%8x u32RegAddr:%8x\n", _gMIO_MapBase[u8Port], u32RegAddr);
    return (*(volatile U32*)(_gMIO_MapBase[u8Port] + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2)));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Read4Byte
/// @brief \b Function  \b Description: read 4 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b U32
////////////////////////////////////////////////////////////////////////////////
U32 HAL_HWI2C_Read4Byte(U32 u32RegAddr)
{
    HWI2C_HAL_FUNC();

    return (HAL_HWI2C_Read2Byte(u32RegAddr) | HAL_HWI2C_Read2Byte(u32RegAddr + 2) << 16);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_WriteByte
/// @brief \b Function  \b Description: write 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u8Val : 1 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_WriteByte(U32 u32RegAddr, U8 u8Val)
{
    U16 u16value;
    U8 u8Port;

    HWI2C_HAL_FUNC();
    HAL_HWI2C_GetPortIdxByOffset(u32RegAddr & (~0xFF), &u8Port);
    u32RegAddr &= 0xFF;

    HWI2C_HAL_INFO("HWI2C IOMap base:%8x u32RegAddr:%8x, u8Val:%2x\n", _gMIO_MapBase[u8Port], u32RegAddr, u8Val);

    //((volatile U8*)(_gMIO_MapBase[u8Port]))[(u32RegAddr << 1) - (u32RegAddr & 1)] = u8Val;
    if((u32RegAddr & 0xFF) % 2)
    {
        u16value = (((U16)u8Val) << 8) | ((*(volatile U32*)(_gMIO_MapBase[u8Port] + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2))) & 0xFF);
    }
    else
    {
        u16value = ((U16)u8Val) | ((*(volatile U32*)(_gMIO_MapBase[u8Port] + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2))) & 0xFF00);
    }

    (*(volatile U32*)(_gMIO_MapBase[u8Port] + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2))) = u16value;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Write2Byte
/// @brief \b Function  \b Description: write 2 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u16Val : 2 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Write2Byte(U32 u32RegAddr, U16 u16Val)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();
    HAL_HWI2C_GetPortIdxByOffset(u32RegAddr & (~0xFF), &u8Port);
    u32RegAddr &= 0xFF;

//  HWI2C_HAL_INFO("HWI2C IOMap base:%16lx u32RegAddr:%4x\n", _gMIO_MapBase[u8Port], u16Val);
    HWI2C_HAL_INFO("HWI2C u8Port: %#x IOMap base:%8x u32RegAddr:%8x u16Val:%4x\n", u8Port, _gMIO_MapBase[u8Port], u32RegAddr, u16Val);

    (*(volatile U32*)(_gMIO_MapBase[u8Port] + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2))) = u16Val;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Write4Byte
/// @brief \b Function  \b Description: write 4 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u32Val : 4 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Write4Byte(U32 u32RegAddr, U32 u32Val)
{
    HWI2C_HAL_FUNC();

    HAL_HWI2C_Write2Byte(u32RegAddr, u32Val & 0x0000FFFF);
    HAL_HWI2C_Write2Byte(u32RegAddr + 2, u32Val >> 16);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_WriteRegBit
/// @brief \b Function  \b Description: write 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u8Val : 1 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_WriteRegBit(U32 u32RegAddr, U8 u8Mask, BOOL bEnable)
{
    U8 u8Val = 0;

    HWI2C_HAL_FUNC();

    u8Val = HAL_HWI2C_ReadByte(u32RegAddr);
    u8Val = (bEnable) ? (u8Val | u8Mask) : (u8Val & ~u8Mask);
    HAL_HWI2C_WriteByte(u32RegAddr, u8Val);
    HWI2C_HAL_INFO("read back u32RegAddr:%x\n", HAL_HWI2C_ReadByte(u32RegAddr));
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_WriteByteMask
/// @brief \b Function  \b Description: write data with mask bits
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u8Val : 1 byte data
/// @param <IN>         \b u8Mask : mask bits
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_WriteByteMask(U32 u32RegAddr, U8 u8Val, U8 u8Mask)
{
    HWI2C_HAL_FUNC();

    u8Val = (HAL_HWI2C_ReadByte(u32RegAddr) & ~u8Mask) | (u8Val & u8Mask);
    HAL_HWI2C_WriteByte(u32RegAddr, u8Val);
    return TRUE;
}


U8 HAL_HWI2C_ReadChipByte(U32 u32RegAddr)
{
    U16 u16value;

    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap chipbase:%8x u32RegAddr:%8x u32RegAddr:%8x\n", _gMChipIO_MapBase, u32RegAddr, (_gMChipIO_MapBase + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2)));

    u16value = (*(volatile U32*)(_gMChipIO_MapBase + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2)));
    HWI2C_HAL_INFO("u16value===:%4x\n", u16value);
    return ((u32RegAddr & 0xFF) % 2) ? HIBYTE(u16value) : LOWBYTE(u16value);
}

BOOL HAL_HWI2C_WriteChipByte(U32 u32RegAddr, U8 u8Val)
{
    U16 u16value;


    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap chipbase:%8x u32RegAddr:%8x\n", _gMChipIO_MapBase, u32RegAddr);

    if((u32RegAddr & 0xFF) % 2)
    {
        u16value = (((U16)u8Val) << 8) | ((*(volatile U32*)(_gMChipIO_MapBase + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2))) & 0xFF);
    }
    else
    {
        u16value = ((U16)u8Val) | ((*(volatile U32*)(_gMChipIO_MapBase + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2))) & 0xFF00);
    }

    (*(volatile U32*)(_gMChipIO_MapBase + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2))) = u16value;
    return TRUE;
}

BOOL HAL_HWI2C_WriteChipByteMask(U32 u32RegAddr, U8 u8Val, U8 u8Mask)
{

    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap chipbase:%8x u32RegAddr:%8x\n", _gMChipIO_MapBase, u32RegAddr);

    u8Val = (HAL_HWI2C_ReadChipByte(u32RegAddr) & ~u8Mask) | (u8Val & u8Mask);
    HAL_HWI2C_WriteChipByte(u32RegAddr, u8Val);
    return TRUE;
}

U16 HAL_HWI2C_ReadClk2Byte(U32 u32RegAddr)
{
    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap clkbase:%8x u32RegAddr:%8x\n", _gMClkIO_MapBase, u32RegAddr);
    return (*(volatile U32*)(_gMClkIO_MapBase + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2)));
}

BOOL HAL_HWI2C_WriteClk2Byte(U32 u32RegAddr, U16 u16Val)
{
    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap clkbase:%8x u32RegAddr:%8x u32RegAddr:%4x\n", _gMClkIO_MapBase, u32RegAddr, u16Val);

    (*(volatile U32*)(_gMClkIO_MapBase + ((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF) / 2) << 2))) = u16Val;
    return TRUE;
}
BOOL HAL_HWI2C_WriteClkByteMask(U32 u32RegAddr, U16 u16Val, U16 u16Mask)
{
    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap clkbase:%8x u32RegAddr:%8x\n", _gMClkIO_MapBase, u32RegAddr);

    u16Val = (HAL_HWI2C_ReadClk2Byte(u32RegAddr) & ~u16Mask) | (u16Val & u16Mask);
    HAL_HWI2C_WriteClk2Byte(u32RegAddr, u16Val);
    return TRUE;
}
//#####################
//
//  MIIC STD Related Functions
//  Static or Internal use
//
//#####################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnINT
/// @brief \b Function  \b Description: Enable Interrupt
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnINT(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG + u16PortOffset, _MIIC_CFG_EN_INT, TRUE);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnDMA
/// @brief \b Function  \b Description: Enable DMA
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnDMA(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    //UartSendTrace("[%s] u16PortOffset: %#x, bEnable: %#x \n", __func__, u16PortOffset, bEnable);
    //if(u16PortOffset == 0x100)
    //    bEnable = FALSE;
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG + u16PortOffset, _MIIC_CFG_EN_DMA, bEnable);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnClkStretch
/// @brief \b Function  \b Description: Enable Clock Stretch
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnClkStretch(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG + u16PortOffset, _MIIC_CFG_EN_CLKSTR, bEnable);
}

#if 0//RFU
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnTimeoutINT
/// @brief \b Function  \b Description: Enable Timeout Interrupt
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnTimeoutINT(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG + u16PortOffset, _MIIC_CFG_EN_TMTINT, bEnable);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnFilter
/// @brief \b Function  \b Description: Enable Filter
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnFilter(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG + u16PortOffset, _MIIC_CFG_EN_FILTER, bEnable);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnPushSda
/// @brief \b Function  \b Description: Enable push current for SDA
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnPushSda(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG + u16PortOffset, _MIIC_CFG_EN_PUSH1T, bEnable);
}

//#####################
//
//  MIIC DMA Related Functions
//  Static or Internal use
//
//#####################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetINT
/// @brief \b Function  \b Description: Initialize HWI2C DMA
/// @param <IN>         \b bEnable : TRUE: enable INT, FALSE: disable INT
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetINT(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CFG + u16PortOffset, _DMA_CFG_INTEN, bEnable);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_Reset
/// @brief \b Function  \b Description: Reset HWI2C DMA
/// @param <IN>         \b bReset : TRUE: Not Reset FALSE: Reset
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_Reset(U16 u16PortOffset, BOOL bReset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CFG + u16PortOffset, _DMA_CFG_RESET, bReset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_MiuReset
/// @brief \b Function  \b Description: Reset HWI2C DMA MIU
/// @param <IN>         \b bReset : TRUE: Not Reset FALSE: Reset
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_MiuReset(U16 u16PortOffset, BOOL bReset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CFG + u16PortOffset, _DMA_CFG_MIURST, bReset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetMiuPri
/// @brief \b Function  \b Description: Set HWI2C DMA MIU Priority
/// @param <IN>         \b eMiuPri : E_HAL_HWI2C_DMA_PRI_LOW, E_HAL_HWI2C_DMA_PRI_HIGH
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetMiuPri(U16 u16PortOffset, HAL_HWI2C_DMA_MIUPRI eMiuPri)
{
    BOOL bHighPri;

    HWI2C_HAL_FUNC();
    if(eMiuPri >= E_HAL_HWI2C_DMA_PRI_MAX)
        return FALSE;
    bHighPri = (eMiuPri == E_HAL_HWI2C_DMA_PRI_HIGH) ? TRUE : FALSE;
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CFG + u16PortOffset, _DMA_CFG_MIUPRI, bHighPri);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetMiuAddr
/// @brief \b Function  \b Description: Set HWI2C DMA MIU Address
/// @param <IN>         \b u32MiuAddr : MIU Address
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetMiuAddr(U16 u16PortOffset, U32 u32MiuAddr)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset, &u8Port) == FALSE)
        return FALSE;
    g_u32DmaPhyAddr[u8Port] = u32MiuAddr;
    return HAL_HWI2C_Write4Byte(REG_HWI2C_DMA_MIU_ADR + u16PortOffset, Chip_Phys_to_MIU(u32MiuAddr));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_Trigger
/// @brief \b Function  \b Description: Trigger HWI2C DMA
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_Trigger(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CTL_TRIG + u16PortOffset, _DMA_CTL_TRIG, TRUE);
}

#if 0 //will be used later
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_ReTrigger
/// @brief \b Function  \b Description: Re-Trigger HWI2C DMA
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_ReTrigger(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CTL + u16PortOffset, _DMA_CTL_RETRIG, TRUE);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetTxfrStop
/// @brief \b Function  \b Description: Control HWI2C DMA Transfer Format with or w/o STOP
/// @param <IN>         \b bEnable : TRUE:  with STOP, FALSE: without STOP
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetTxfrStop(U16 u16PortOffset, BOOL bEnable)
{
    BOOL bTxNoStop;

    HWI2C_HAL_FUNC();
    bTxNoStop = (bEnable) ? FALSE : TRUE;
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CTL + u16PortOffset, _DMA_CTL_TXNOSTOP, bTxNoStop);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetReadMode
/// @brief \b Function  \b Description: Control HWI2C DMA Transfer Format with or w/o STOP
/// @param <IN>         \b eReadMode : E_HAL_HWI2C_DMA_READ_NOSTOP, E_HAL_HWI2C_DMA_READ_STOP
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetReadMode(U16 u16PortOffset, HAL_HWI2C_ReadMode eReadMode)
{
    HWI2C_HAL_FUNC();
    if(eReadMode >= E_HAL_HWI2C_READ_MODE_MAX)
        return FALSE;
    if(eReadMode == E_HAL_HWI2C_READ_MODE_DIRECTION_CHANGE)
        return HAL_HWI2C_DMA_SetTxfrStop(u16PortOffset, FALSE);
    else if(eReadMode == E_HAL_HWI2C_READ_MODE_DIRECTION_CHANGE_STOP_START)
        return HAL_HWI2C_DMA_SetTxfrStop(u16PortOffset, TRUE);
    else
        return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetRdWrt
/// @brief \b Function  \b Description: Control HWI2C DMA Read or Write
/// @param <IN>         \b bRdWrt : TRUE: read ,FALSE: write
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetRdWrt(U16 u16PortOffset, BOOL bRdWrt)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CTL + u16PortOffset, _DMA_CTL_RDWTCMD, bRdWrt);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetMiuChannel
/// @brief \b Function  \b Description: Control HWI2C DMA MIU channel
/// @param <IN>         \b u8MiuCh : E_HAL_HWI2C_DMA_MIU_CH0 , E_HAL_HWI2C_DMA_MIU_CH1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetMiuChannel(U16 u16PortOffset, HAL_HWI2C_DMA_MIUCH eMiuCh)
{
    BOOL bMiuCh1;

    HWI2C_HAL_FUNC();
    if(eMiuCh >= E_HAL_HWI2C_DMA_MIU_MAX)
        return FALSE;
    bMiuCh1 = (eMiuCh == E_HAL_HWI2C_DMA_MIU_CH1) ? TRUE : FALSE;
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CTL + u16PortOffset, _DMA_CTL_MIUCHSEL, bMiuCh1);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_TxfrDone
/// @brief \b Function  \b Description: Enable interrupt for HWI2C
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_TxfrDone(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_TXR + u16PortOffset, _DMA_TXR_DONE, TRUE);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_IsTxfrDone
/// @brief \b Function  \b Description: Check HWI2C DMA Tx done or not
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: DMA TX Done, FALSE: DMA TX Not Done
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_IsTxfrDone(U16 u16PortOffset, U8 u8Port)
{
    HWI2C_HAL_FUNC();
    //##########################
    //
    // [Note] : IMPORTANT !!!
    // Need to put some delay here,
    // Otherwise, reading data will fail
    //
    //##########################
    if(u8Port >= HAL_HWI2C_PORTS)
        return FALSE;
    HAL_HWI2C_ExtraDelay(g_u16DmaDelayFactor[u8Port]);
    return (HAL_HWI2C_ReadByte(REG_HWI2C_DMA_TXR + u16PortOffset) & _DMA_TXR_DONE) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetTxfrCmd
/// @brief \b Function  \b Description: Set Transfer HWI2C DMA Command & Length
/// @param <IN>         \b pu8CmdBuf : data pointer
/// @param <IN>         \b u8CmdLen : command length
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: cmd len in range, FALSE: cmd len out of range
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetTxfrCmd(U16 u16PortOffset, U8 u8CmdLen, U8* pu8CmdBuf)
{
    U8 k, u8CmdData;
    U32 u32RegAdr;

    HWI2C_HAL_FUNC();
    if(u8CmdLen > HWI2C_DMA_CMD_DATA_LEN)
        return FALSE;
    for(k = 0 ; (k < u8CmdLen) && (k < HWI2C_DMA_CMD_DATA_LEN); k++)
    {
        u32RegAdr = REG_HWI2C_DMA_CMDDAT0 + k;
        u8CmdData = *(pu8CmdBuf + k);
        HAL_HWI2C_WriteByte(u32RegAdr + u16PortOffset, u8CmdData);
    }
    HAL_HWI2C_WriteByte(REG_HWI2C_DMA_CMDLEN + u16PortOffset, u8CmdLen & _DMA_CMDLEN_MSK);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetCmdLen
/// @brief \b Function  \b Description: Set HWI2C DMA MIU command length
/// @param <IN>         \b u8CmdLen : command length
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetCmdLen(U16 u16PortOffset, U8 u8CmdLen)
{
    HWI2C_HAL_FUNC();
    if(u8CmdLen > HWI2C_DMA_CMD_DATA_LEN)
        return FALSE;
    HAL_HWI2C_WriteByte(REG_HWI2C_DMA_CMDLEN + u16PortOffset, u8CmdLen & _DMA_CMDLEN_MSK);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetDataLen
/// @brief \b Function  \b Description: Set HWI2C DMA data length
/// @param <IN>         \b u32DataLen : data length
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetDataLen(U16 u16PortOffset, U32 u32DataLen)
{
    U32 u32DataLenSet;

    HWI2C_HAL_FUNC();
    u32DataLenSet = u32DataLen;
    return HAL_HWI2C_Write4Byte(REG_HWI2C_DMA_DATLEN + u16PortOffset, u32DataLenSet);
}

#if 0 //will be used later
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_GetTxfrCnt
/// @brief \b Function  \b Description: Get MIIC DMA Transfer Count
/// @param <IN>         \b u32TxfrCnt : transfer count
/// @param <OUT>        \b None :
/// @param <RET>        \b None :
////////////////////////////////////////////////////////////////////////////////
static U32 HAL_HWI2C_DMA_GetTxfrCnt(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_Read4Byte(REG_HWI2C_DMA_TXFRCNT + u16PortOffset);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_GetAddrMode
/// @brief \b Function  \b Description: Set MIIC DMA Slave Device Address length mode
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b E_HAL_HWI2C_DMA_ADDR_10BIT(10 bits mode),
///                                \b E_HAL_HWI2C_DMA_ADDR_NORMAL(7 bits mode)
////////////////////////////////////////////////////////////////////////////////
static HAL_HWI2C_DMA_ADDRMODE HAL_HWI2C_DMA_GetAddrMode(U16 u16PortOffset)
{
    HAL_HWI2C_DMA_ADDRMODE eAddrMode;

    HWI2C_HAL_FUNC();
    if(HAL_HWI2C_ReadByte(REG_HWI2C_DMA_SLVCFG + u16PortOffset) & _DMA_10BIT_MODE)
        eAddrMode = E_HAL_HWI2C_DMA_ADDR_10BIT;
    else
        eAddrMode = E_HAL_HWI2C_DMA_ADDR_NORMAL;
    return eAddrMode;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetSlaveAddr
/// @brief \b Function  \b Description: Set MIIC DMA Slave Device Address
/// @param <IN>         \b u32TxfrCnt : slave device address
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetSlaveAddr(U16 u16PortOffset, U16 u16SlaveAddr)
{
    HWI2C_HAL_FUNC();
    if(HAL_HWI2C_DMA_GetAddrMode(u16PortOffset) == E_HAL_HWI2C_DMA_ADDR_10BIT)
        return HAL_HWI2C_Write2Byte(REG_HWI2C_DMA_SLVADR + u16PortOffset, u16SlaveAddr & _DMA_SLVADR_10BIT_MSK);
    else
        return HAL_HWI2C_Write2Byte(REG_HWI2C_DMA_SLVADR + u16PortOffset, u16SlaveAddr & _DMA_SLVADR_NORML_MSK);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetAddrMode
/// @brief \b Function  \b Description: Set MIIC DMA Slave Device Address length mode
/// @param <IN>         \b eAddrMode : E_HAL_HWI2C_DMA_ADDR_NORMAL, E_HAL_HWI2C_DMA_ADDR_10BIT
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetAddrMode(U16 u16PortOffset, HAL_HWI2C_DMA_ADDRMODE eAddrMode)
{
    BOOL b10BitMode;

    HWI2C_HAL_FUNC();
    if(eAddrMode >= E_HAL_HWI2C_DMA_ADDR_MAX)
        return FALSE;
    b10BitMode = (eAddrMode == E_HAL_HWI2C_DMA_ADDR_10BIT) ? TRUE : FALSE;
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_SLVCFG + u16PortOffset, _DMA_10BIT_MODE, b10BitMode);
}

static BOOL HAL_HWI2C_DMA_SetMiuData(U16 u16PortOffset, U32 u32Length, U8* pu8SrcData)
{
    //U32 u32PhyAddr = 0;
    U8 *pMiuData = 0;
    U8 u8Port;

    HWI2C_HAL_FUNC();
    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset, &u8Port) == FALSE)
        return FALSE;

    //pMiuData = (U8*)_PA2VA((U32)u32PhyAddr);
    //memcpy((void*)pMiuData,(void*)pu8SrcData,u32Length);

    //u32PhyAddr = g_u32DmaPhyAddr[u8Port];
    pMiuData = HWI2C_DMA[u8Port].i2c_virt_addr;
    memcpy(pMiuData, pu8SrcData, u32Length);
    HAL_HWI2C_DMA_SetDataLen(u16PortOffset, u32Length);
    return TRUE;
}

static BOOL HAL_HWI2C_DMA_GetMiuData(U16 u16PortOffset, U32 u32Length, U8* pu8DstData)
{
    //U32 u32PhyAddr = 0;
    U8 *pMiuData = 0;
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset, &u8Port) == FALSE)
        return FALSE;
    //u32PhyAddr = g_u32DmaPhyAddr[u8Port];
    //pMiuData = (U8*)_PA2VA((U32)u32PhyAddr);
    //u32PhyAddr = g_u32DmaPhyAddr[u8Port];
    pMiuData = HWI2C_DMA[u8Port].i2c_virt_addr;
    memcpy((void*)pu8DstData, (void*)pMiuData, u32Length);
    return TRUE;
}

static BOOL HAL_HWI2C_DMA_WaitDone(U16 u16PortOffset, U8 u8ReadWrite)
{
    U16 volatile u16Timeout = HWI2C_DMA_WAIT_TIMEOUT;
    U8 u8Port;

    HWI2C_HAL_FUNC();

    //################
    //
    // IMPORTANT HERE !!!
    //
    //################
    //MsOS_FlushMemory();
    //(2-1) reset DMA engine
    HAL_HWI2C_DMA_Reset(u16PortOffset, TRUE);
    HAL_HWI2C_DMA_Reset(u16PortOffset, FALSE);
    //(2-2)  reset MIU module in DMA engine
    HAL_HWI2C_DMA_MiuReset(u16PortOffset, TRUE);
    HAL_HWI2C_DMA_MiuReset(u16PortOffset, FALSE);


    //get port index for delay factor
    HAL_HWI2C_GetPortIdxByOffset(u16PortOffset, &u8Port);
    //clear transfer dine first for savfty
    HAL_HWI2C_DMA_TxfrDone(u16PortOffset);
    //set command : 0 for Write, 1 for Read
    HAL_HWI2C_DMA_SetRdWrt(u16PortOffset, u8ReadWrite);
    //issue write trigger
    HAL_HWI2C_DMA_Trigger(u16PortOffset);
    //check transfer done
    while(u16Timeout--)
    {
        if(HAL_HWI2C_DMA_IsTxfrDone(u16PortOffset, u8Port))
        {
            HAL_HWI2C_DMA_TxfrDone(u16PortOffset);
            HWI2C_HAL_INFO("[DMA]: Transfer DONE!\n");
            if(HAL_HWI2C_Get_SendAck(u16PortOffset))
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
    HWI2C_HAL_ERR("[DMA]: Transfer NOT Completely!\n");
    return FALSE;
}

static BOOL HAL_HWI2C_DMA_SetDelayFactor(U16 u16PortOffset, HAL_HWI2C_CLKSEL eClkSel)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset, &u8Port) == FALSE)
    {
        g_u16DmaDelayFactor[u8Port] = 5;
        return FALSE;
    }
    switch(eClkSel)//use Xtal = 24M Hz
    {
        case E_HAL_HWI2C_CLKSEL_HIGH: // 400 KHz
            g_u16DmaDelayFactor[u8Port] = 1;
            break;
        case E_HAL_HWI2C_CLKSEL_NORMAL: //300 KHz
            g_u16DmaDelayFactor[u8Port] = 1;
            break;
        case E_HAL_HWI2C_CLKSEL_SLOW: //200 KHz
            g_u16DmaDelayFactor[u8Port] = 1;
            break;
        case E_HAL_HWI2C_CLKSEL_VSLOW: //100 KHz
            g_u16DmaDelayFactor[u8Port] = 2;
            break;
        case E_HAL_HWI2C_CLKSEL_USLOW: //50 KHz
            g_u16DmaDelayFactor[u8Port] = 3;
            break;
        case E_HAL_HWI2C_CLKSEL_UVSLOW: //25 KHz
            g_u16DmaDelayFactor[u8Port] = 3;
            break;
        default:
            g_u16DmaDelayFactor[u8Port] = 5;
            return FALSE;
    }
    return TRUE;
}

//#####################
//
//  MIIC STD Related Functions
//  External
//
//#####################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Init_Chip
/// @brief \b Function  \b Description: Init HWI2C chip
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Init_Chip(void)
{
    BOOL bRet = TRUE;

    HWI2C_HAL_FUNC();
    //not set all pads (except SPI) as input
    //bRet &= HAL_HWI2C_WriteRegBit(CHIP_REG_ALLPADIN, CHIP_ALLPAD_IN, FALSE);
    // CLKGEN IIC0 & IIC1 CLK
    //HAL_HWI2C_WriteClk2Byte(0x37*2, 0x0808);
    //UartSendTrace("clk: %#x\n",HAL_HWI2C_ReadClk2Byte(0x37*2));

    // Sensor CLK
    //HAL_HWI2C_WriteClk2Byte(0x62*2, 0x0804);


    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_IsMaster
/// @brief \b Function  \b Description: Check if Master I2C
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Master, FALSE: Slave
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_IsMaster(void)
{
    HWI2C_HAL_FUNC();
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Master_Enable
/// @brief \b Function  \b Description: Master I2C enable
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Master_Enable(U16 u16PortOffset)
{
    U8 u8Port = 0;
    BOOL bRet = TRUE;
    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset, &u8Port) == FALSE)
        return FALSE;
    g_bLastByte[u8Port] = FALSE;

    //(1) clear interrupt
    HAL_HWI2C_Clear_INT(u16PortOffset);
    //(2) reset standard master iic
    HAL_HWI2C_Reset(u16PortOffset, TRUE);
    HAL_HWI2C_Reset(u16PortOffset, FALSE);
    //(3) configuration
    HAL_HWI2C_EnINT(u16PortOffset, TRUE);
    HAL_HWI2C_EnClkStretch(u16PortOffset, TRUE);
    HAL_HWI2C_EnFilter(u16PortOffset, TRUE);
    HAL_HWI2C_EnPushSda(u16PortOffset, TRUE);
#if 0
    HAL_HWI2C_EnTimeoutINT(u16PortOffset, TRUE);
    HAL_HWI2C_Write2Byte(REG_HWI2C_TMT_CNT + u16PortOffset, 0x100);
#endif
    //(4) Disable DMA
    HAL_HWI2C_DMA_MiuReset(u16PortOffset, TRUE);
    HAL_HWI2C_DMA_Reset(u16PortOffset, TRUE);
    //bRet = HAL_HWI2C_DMA_Enable(u16PortOffset, FALSE);
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_SetPortRegOffset
/// @brief \b Function  \b Description: Set HWI2C port register offset
/// @param <IN>         \b ePort : HWI2C port number
/// @param <OUT>         \b pu16Offset : port register offset
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_SetPortRegOffset(U8 u8Port, U16* pu16Offset)
{
    HWI2C_HAL_FUNC();

    if(u8Port == HAL_HWI2C_PORT0)
    {
        //port 0 : bank register address 0x111800
        *pu16Offset = (U16)0x00;
    }
    else if(u8Port == HAL_HWI2C_PORT1)
    {
        //port 1 : bank register address 0x111900
        *pu16Offset = (U16)0x100;
    }
    else
    {
        *pu16Offset = (U16)0x00;
        return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_GetPortIdxByRegOffset
/// @brief \b Function  \b Description: Get HWI2C port index by register offset
/// @param <IN>         \b u16Offset : port register offset
/// @param <OUT>         \b pu8Port :  port index
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_GetPortIdxByOffset(U16 u16Offset, U8* pu8Port)
{
    HWI2C_HAL_FUNC();

    if(u16Offset == (U16)0x00)
    {
        //port 0 : bank register address 0x11800
        *pu8Port = HAL_HWI2C_PORT0;
    }
    else if(u16Offset == (U16)0x100)
    {
        //port 1 : bank register address 0x11900
        *pu8Port = HAL_HWI2C_PORT1;
    }
    else if(u16Offset == (U16)0x200)
    {
        //port 2 : bank register address 0x11A00
        *pu8Port = HAL_HWI2C_PORT2;
    }
    else if(u16Offset == (U16)0x300)
    {
        //port 3 : bank register address 0x11B00
        *pu8Port = HAL_HWI2C_PORT3;
    }
    else
    {
        *pu8Port = HAL_HWI2C_PORT0;
        return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_GetPortIdxByPort
/// @brief \b Function  \b Description: Get HWI2C port index by port number
/// @param <IN>         \b ePort : port number
/// @param <OUT>         \b pu8Port :  port index
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_GetPortIdxByPort(HAL_HWI2C_PORT ePort, U8* pu8Port)
{
    HWI2C_HAL_FUNC();

    if((ePort >= E_HAL_HWI2C_PORT0_0) && (ePort <= E_HAL_HWI2C_PORT0_7))
    {
        *pu8Port = HAL_HWI2C_PORT0;
    }
    else if((ePort >= E_HAL_HWI2C_PORT1_0) && (ePort <= E_HAL_HWI2C_PORT1_7))
    {
        *pu8Port = HAL_HWI2C_PORT1;
    }
    else if((ePort >= E_HAL_HWI2C_PORT2_0) && (ePort <= E_HAL_HWI2C_PORT2_7))
    {
        *pu8Port = HAL_HWI2C_PORT2;
    }
    else if((ePort >= E_HAL_HWI2C_PORT3_0) && (ePort <= E_HAL_HWI2C_PORT3_7))
    {
        *pu8Port = HAL_HWI2C_PORT3;
    }
    else
    {
        *pu8Port = HAL_HWI2C_PORT0;
        return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_SelectPort
/// @brief \b Function  \b Description: Select HWI2C port
/// @param <IN>         \b None : HWI2C port
/// @param param        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
#if 0
BOOL HAL_HWI2C_SelectPort(HAL_HWI2C_PORT ePort)
{
    U8 u8Value1 = 0;

    HWI2C_HAL_FUNC();

    if(ePort == E_HAL_HWI2C_PORT0_0)
    {
        u8Value1 = CHIP_MIIC0_PAD_1;
        HAL_HWI2C_WriteChipByteMask(CHIP_REG_HWI2C_MIIC0, u8Value1, CHIP_MIIC0_PAD_MSK);
    }
    else if(ePort == E_HAL_HWI2C_PORT0_1)
    {
        u8Value1 = CHIP_MIIC0_PAD_1;
        HAL_HWI2C_WriteChipByteMask(CHIP_REG_HWI2C_MIIC0, u8Value1, CHIP_MIIC0_PAD_MSK);
    }
    else if(ePort == E_HAL_HWI2C_PORT0_2)
    {
        u8Value1 = CHIP_MIIC0_PAD_2;
        HAL_HWI2C_WriteChipByteMask(CHIP_REG_HWI2C_MIIC0, u8Value1, CHIP_MIIC0_PAD_MSK);
    }
    else if(ePort == E_HAL_HWI2C_PORT0_3)
    {
        u8Value1 = CHIP_MIIC0_PAD_3;
        HAL_HWI2C_WriteChipByteMask(CHIP_REG_HWI2C_MIIC0, u8Value1, CHIP_MIIC0_PAD_MSK);
    }
    else if(ePort == E_HAL_HWI2C_PORT1_0)
    {
        u8Value1 = CHIP_MIIC1_PAD_0;
        HAL_HWI2C_WriteChipByteMask(CHIP_REG_HWI2C_MIIC1, u8Value1, CHIP_MIIC1_PAD_MSK);
    }
    else if(ePort == E_HAL_HWI2C_PORT1_1)
    {
        u8Value1 = CHIP_MIIC1_PAD_1;
        HAL_HWI2C_WriteChipByteMask(CHIP_REG_HWI2C_MIIC1, u8Value1, CHIP_MIIC1_PAD_MSK);
    }
    else if(ePort == E_HAL_HWI2C_PORT1_2)
    {
        u8Value1 = CHIP_MIIC1_PAD_2;
        HAL_HWI2C_WriteChipByteMask(CHIP_REG_HWI2C_MIIC1, u8Value1, CHIP_MIIC1_PAD_MSK);
    }
    else
    {
        HWI2C_HAL_ERR("[%s]: Port(%d) not support\n", __func__, ePort);
        return FALSE;
    }
    return TRUE;
}
#else
BOOL HAL_HWI2C_SelectPort(HAL_HWI2C_PORT ePort)
{
    U32 u32RegAddr = CHIP_REG_HWI2C_MIIC0;
    U8  u8Val = CHIP_MIIC0_PAD_1;
    U8  u8Mask = CHIP_MIIC0_PAD_MSK;

    HWI2C_HAL_FUNC();

    if((ePort >= E_HAL_HWI2C_PORT0_0) && (ePort <= E_HAL_HWI2C_PORT0_7))
    {
        u32RegAddr = CHIP_REG_HWI2C_MIIC0;
        u8Mask = CHIP_MIIC0_PAD_MSK;
    }
    else if((ePort >= E_HAL_HWI2C_PORT1_0) && (ePort <= E_HAL_HWI2C_PORT1_7))
    {
        u32RegAddr = CHIP_REG_HWI2C_MIIC1;
        u8Mask = CHIP_MIIC1_PAD_MSK;
    }
    else if((ePort >= E_HAL_HWI2C_PORT2_0) && (ePort <= E_HAL_HWI2C_PORT2_7))
    {
        u32RegAddr = CHIP_REG_HWI2C_MIIC2;
        u8Mask = CHIP_MIIC2_PAD_MSK;
    }
    else if((ePort >= E_HAL_HWI2C_PORT3_0) && (ePort <= E_HAL_HWI2C_PORT3_7))
    {
        u32RegAddr = CHIP_REG_HWI2C_MIIC3;
        u8Mask = CHIP_MIIC3_PAD_MSK;
    }

    switch(ePort) {
    case E_HAL_HWI2C_PORT0_0: u8Val = CHIP_MIIC0_PAD_0; break;
    case E_HAL_HWI2C_PORT0_1: u8Val = CHIP_MIIC0_PAD_1; break;
    case E_HAL_HWI2C_PORT0_2: u8Val = CHIP_MIIC0_PAD_2; break;
    case E_HAL_HWI2C_PORT0_3: u8Val = CHIP_MIIC0_PAD_3; break;
    case E_HAL_HWI2C_PORT0_4: u8Val = CHIP_MIIC0_PAD_4; break;
    case E_HAL_HWI2C_PORT1_0: u8Val = CHIP_MIIC1_PAD_0; break;
    case E_HAL_HWI2C_PORT1_1: u8Val = CHIP_MIIC1_PAD_1; break;
    case E_HAL_HWI2C_PORT1_2: u8Val = CHIP_MIIC1_PAD_2; break;
    case E_HAL_HWI2C_PORT1_3: u8Val = CHIP_MIIC1_PAD_3; break;
    case E_HAL_HWI2C_PORT1_4: u8Val = CHIP_MIIC1_PAD_4; break;
    case E_HAL_HWI2C_PORT1_5: u8Val = CHIP_MIIC1_PAD_5; break;
    case E_HAL_HWI2C_PORT2_0: u8Val = CHIP_MIIC2_PAD_0; break;
    case E_HAL_HWI2C_PORT2_1: u8Val = CHIP_MIIC2_PAD_1; break;
    case E_HAL_HWI2C_PORT2_2: u8Val = CHIP_MIIC2_PAD_2; break;
    case E_HAL_HWI2C_PORT2_3: u8Val = CHIP_MIIC2_PAD_3; break;
    case E_HAL_HWI2C_PORT3_0: u8Val = CHIP_MIIC3_PAD_0; break;
    case E_HAL_HWI2C_PORT3_1: u8Val = CHIP_MIIC3_PAD_1; break;
    case E_HAL_HWI2C_PORT3_2: u8Val = CHIP_MIIC3_PAD_2; break;
    case E_HAL_HWI2C_PORT3_3: u8Val = CHIP_MIIC3_PAD_3; break;
    case E_HAL_HWI2C_PORT3_4: u8Val = CHIP_MIIC3_PAD_4; break;
    case E_HAL_HWI2C_PORT3_5: u8Val = CHIP_MIIC3_PAD_5; break;
    default:
        HWI2C_HAL_ERR("[%s]: Port(%d) not support\n", __func__, ePort);
        return FALSE;
    }

    HAL_HWI2C_WriteChipByteMask(u32RegAddr, u8Val, u8Mask);

    return TRUE;
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_SetClk
/// @brief \b Function  \b Description: Set I2C clock
/// @param <IN>         \b u8Clk: clock rate
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_SetClk(U16 u16PortOffset, HAL_HWI2C_CLKSEL eClkSel)
{
    U16 u16ClkHCnt = 0, u16ClkLCnt = 0;
    U16 u16StpCnt = 0, u16SdaCnt = 0, u16SttCnt = 0, u16LchCnt = 0;

    HWI2C_HAL_FUNC();
    if(eClkSel >= E_HAL_HWI2C_CLKSEL_NOSUP)
        return FALSE;

    switch(eClkSel)//use Xtal = 12M Hz
    {
        case E_HAL_HWI2C_CLKSEL_HIGH: // 400 KHz
            u16ClkHCnt =   9;
            u16ClkLCnt =   13;
            break;
        case E_HAL_HWI2C_CLKSEL_NORMAL: //300 KHz
            u16ClkHCnt =  15;
            u16ClkLCnt =   17;
            break;
        case E_HAL_HWI2C_CLKSEL_SLOW: //200 KHz
            u16ClkHCnt =  25;
            u16ClkLCnt =   27;
            break;
        case E_HAL_HWI2C_CLKSEL_VSLOW: //100 KHz
            u16ClkHCnt =  55;
            u16ClkLCnt =   57;
            break;
        case E_HAL_HWI2C_CLKSEL_USLOW: //50 KHz
            u16ClkHCnt =  115;
            u16ClkLCnt = 117;
            break;
        case E_HAL_HWI2C_CLKSEL_UVSLOW: //25 KHz
            u16ClkHCnt =  235;
            u16ClkLCnt = 237;
            break;
        default:
            u16ClkHCnt =  15;
            u16ClkLCnt =  17;
            break;
    }
    u16SttCnt = 38;
    u16StpCnt = 38;
    u16SdaCnt = 5;
    u16LchCnt = 5;

    HAL_HWI2C_Write2Byte(REG_HWI2C_CKH_CNT + u16PortOffset, u16ClkHCnt);
    HAL_HWI2C_Write2Byte(REG_HWI2C_CKL_CNT + u16PortOffset, u16ClkLCnt);
    HAL_HWI2C_Write2Byte(REG_HWI2C_STP_CNT + u16PortOffset, u16StpCnt);
    HAL_HWI2C_Write2Byte(REG_HWI2C_SDA_CNT + u16PortOffset, u16SdaCnt);
    HAL_HWI2C_Write2Byte(REG_HWI2C_STT_CNT + u16PortOffset, u16SttCnt);
    HAL_HWI2C_Write2Byte(REG_HWI2C_LTH_CNT + u16PortOffset, u16LchCnt);
    //HAL_HWI2C_Write2Byte(REG_HWI2C_TMT_CNT+u16PortOffset, 0x0000);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Start
/// @brief \b Function  \b Description: Send start condition
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Start(U16 u16PortOffset)
{
    U16 u16Count = HWI2C_HAL_WAIT_TIMEOUT;

    HWI2C_HAL_FUNC();
    //reset I2C
    HAL_HWI2C_WriteRegBit(REG_HWI2C_CMD_START + u16PortOffset, _CMD_START, TRUE);
    while((!HAL_HWI2C_Is_INT(u16PortOffset)) && (u16Count > 0))
        u16Count--;
    //udelay(5);
    HAL_HWI2C_Clear_INT(u16PortOffset);
    return (u16Count) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Stop
/// @brief \b Function  \b Description: Send Stop condition
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Stop(U16 u16PortOffset)
{
    U16 u16Count = HWI2C_HAL_WAIT_TIMEOUT;

    HWI2C_HAL_FUNC();
    //udelay(5);
    HAL_HWI2C_WriteRegBit(REG_HWI2C_CMD_STOP + u16PortOffset, _CMD_STOP, TRUE);
    while((!HAL_HWI2C_Is_Idle(u16PortOffset)) && (!HAL_HWI2C_Is_INT(u16PortOffset)) && (u16Count > 0))
        u16Count--;
    HAL_HWI2C_Clear_INT(u16PortOffset);
    return (u16Count) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_ReadRdy
/// @brief \b Function  \b Description: Start byte reading
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_ReadRdy(U16 u16PortOffset)
{
    U8 u8Value = 0;
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset, &u8Port) == FALSE)
        return FALSE;
    u8Value = (g_bLastByte[u8Port]) ? (_RDATA_CFG_TRIG | _RDATA_CFG_ACKBIT) : (_RDATA_CFG_TRIG);
    g_bLastByte[u8Port] = FALSE;
    return HAL_HWI2C_WriteByte(REG_HWI2C_RDATA_CFG + u16PortOffset, u8Value);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_SendData
/// @brief \b Function  \b Description: Send 1 byte data to SDA
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_SendData(U16 u16PortOffset, U8 u8Data)
{
    HWI2C_HAL_FUNC();

    return HAL_HWI2C_WriteByte(REG_HWI2C_WDATA + u16PortOffset, u8Data);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_RecvData
/// @brief \b Function  \b Description: Receive 1 byte data from SDA
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b U8 :
////////////////////////////////////////////////////////////////////////////////
U8 HAL_HWI2C_RecvData(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();

    return HAL_HWI2C_ReadByte(REG_HWI2C_RDATA + u16PortOffset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Get_SendAck
/// @brief \b Function  \b Description: Get ack after sending data
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Valid ack, FALSE: No ack
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Get_SendAck(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return (HAL_HWI2C_ReadByte(REG_HWI2C_WDATA_GET + u16PortOffset) & _WDATA_GET_ACKBIT) ? FALSE : TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_NoAck
/// @brief \b Function  \b Description: generate no ack pulse
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_NoAck(U16 u16PortOffset)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset, &u8Port) == FALSE)
        return FALSE;
    g_bLastByte[u8Port] = TRUE;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Ack
/// @brief \b Function  \b Description: generate ack pulse
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Ack(U16 u16PortOffset)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset, &u8Port) == FALSE)
        return FALSE;
    g_bLastByte[u8Port] = FALSE;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_GetStae
/// @brief \b Function  \b Description: Get i2c Current State
/// @param <IN>         \b u16PortOffset: HWI2C Port Offset
/// @param <OUT>        \b None
/// @param <RET>        \b HWI2C current status
////////////////////////////////////////////////////////////////////////////////
U8 HAL_HWI2C_GetState(U16 u16PortOffset)
{

    U8 cur_state =  HAL_HWI2C_ReadByte(REG_HWI2C_CUR_STATE + u16PortOffset) & _CUR_STATE_MSK;
    HWI2C_HAL_FUNC();

    if(cur_state <= 0)  // 0: idle
        return E_HAL_HWI2C_STATE_IDEL;
    else if(cur_state <= 2)  // 1~2:start
        return E_HAL_HWI2C_STATE_START;
    else if(cur_state <= 6)  // 3~6:write
        return E_HAL_HWI2C_STATE_WRITE;
    else if(cur_state <= 10)  // 7~10:read
        return E_HAL_HWI2C_STATE_READ;
    else if(cur_state <= 11)  // 11:interrupt
        return E_HAL_HWI2C_STATE_INT;
    else if(cur_state <= 12)  // 12:wait
        return E_HAL_HWI2C_STATE_WAIT;
    else  // 13~15:stop
        return E_HAL_HWI2C_STATE_STOP;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Is_Idle
/// @brief \b Function  \b Description: Check if i2c is idle
/// @param <IN>         \b u16PortOffset: HWI2C Port Offset
/// @param <OUT>        \b None
/// @param <RET>        \b TRUE : idle, FALSE : not idle
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Is_Idle(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();

    return ((HAL_HWI2C_GetState(u16PortOffset) == E_HAL_HWI2C_STATE_IDEL) ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Is_INT
/// @brief \b Function  \b Description: Check if i2c is interrupted
/// @param <IN>         \b u8Status : queried status
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Is_INT(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return (HAL_HWI2C_ReadByte(REG_HWI2C_INT_CTL + u16PortOffset) & _INT_CTL) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Clear_INT
/// @brief \b Function  \b Description: Enable interrupt for HWI2C
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Clear_INT(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();

    return HAL_HWI2C_WriteRegBit(REG_HWI2C_INT_CTL + u16PortOffset, _INT_CTL, TRUE);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Reset
/// @brief \b Function  \b Description: Reset HWI2C state machine
/// @param <IN>         \b bReset : TRUE: Reset FALSE: Not reset
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Reset(U16 u16PortOffset, BOOL bReset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG + u16PortOffset, _MIIC_CFG_RESET, bReset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Send_Byte
/// @brief \b Function  \b Description: Send one byte
/// @param u8Data       \b IN: 1 byte data
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Send_Byte(U16 u16PortOffset, U8 u8Data)
{
    U8 u8Retry = HWI2C_HAL_RETRY_TIMES;
    U32 u16Count = HWI2C_HAL_WAIT_TIMEOUT;

    HWI2C_HAL_FUNC();
    //HWI2C_HAL_ERR("Send byte 0x%X !\n", u8Data);

    while(u8Retry--)
    {
        HAL_HWI2C_Clear_INT(u16PortOffset);
        if(HAL_HWI2C_SendData(u16PortOffset, u8Data))
        {
            u16Count = HWI2C_HAL_WAIT_TIMEOUT;
            while(u16Count--)
            {
                if(HAL_HWI2C_Is_INT(u16PortOffset))
                {
                    HAL_HWI2C_Clear_INT(u16PortOffset);
                    if(HAL_HWI2C_Get_SendAck(u16PortOffset))
                    {
#if 1
                        HAL_HWI2C_ExtraDelay(1);
#else
                        MsOS_DelayTaskUs(1);
#endif
                        return TRUE;
                    }
                    //break;
                }
            }
        }
        //pr_err("REG_HWI2C_INT_STATUS %#x\n", HAL_HWI2C_ReadByte(REG_HWI2C_INT_STATUS+u16PortOffset));
        //pr_err("REG_HWI2C_CUR_STATE %#x\n", HAL_HWI2C_ReadByte(REG_HWI2C_CUR_STATE+u16PortOffset));
        //Hal_GPIO_Set_High(72);
        // check if in Idle state
        if(HAL_HWI2C_ReadByte(REG_HWI2C_CUR_STATE + u16PortOffset) == 0)
        {
            //pr_err("## START bit");
            HAL_HWI2C_Start(u16PortOffset);
            udelay(2);
        }
        else
        {
            //pr_err("REG_HWI2C_CUR_STATE %#x\n", HAL_HWI2C_ReadByte(REG_HWI2C_CUR_STATE+u16PortOffset));
            HAL_HWI2C_Stop(u16PortOffset);
            udelay(2);
            // reset I2C IP
            HAL_HWI2C_Reset(u16PortOffset, TRUE);
            HAL_HWI2C_Reset(u16PortOffset, FALSE);
            udelay(2);
            HAL_HWI2C_Start(u16PortOffset);
            udelay(2);
        }
        //Hal_GPIO_Set_Low(72);
    }
    //HWI2C_HAL_ERR("Send byte 0x%X fail!\n", u8Data);
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Recv_Byte
/// @brief \b Function  \b Description: Init HWI2C driver and auto generate ACK
/// @param *pData       \b Out: received data
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Recv_Byte(U16 u16PortOffset, U8 *pData)
{
    U16 u16Count = HWI2C_HAL_WAIT_TIMEOUT;

    HWI2C_HAL_FUNC();

    if(!pData)
        return FALSE;

    HAL_HWI2C_ReadRdy(u16PortOffset);
    while((!HAL_HWI2C_Is_INT(u16PortOffset)) && (u16Count > 0))
        u16Count--;
    HAL_HWI2C_Clear_INT(u16PortOffset);
    if(u16Count)
    {
        //get data before clear int and stop
        *pData = HAL_HWI2C_RecvData(u16PortOffset);
        HWI2C_HAL_INFO("Recv byte =%x\n", *pData);
        //clear interrupt
        HAL_HWI2C_Clear_INT(u16PortOffset);
#if 1
        HAL_HWI2C_ExtraDelay(1);
#else
        MsOS_DelayTaskUs(1);
#endif
        return TRUE;
    }
    HWI2C_HAL_INFO("Recv byte fail!\n");
    return FALSE;
}

//#####################
//
//  MIIC DMA Related Functions
//  External
//
//#####################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_Enable
/// @brief \b Function  \b Description: Enable HWI2C DMA
/// @param <IN>         \b bEnable : TRUE: enable DMA, FALSE: disable DMA
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_DMA_Enable(U16 u16PortOffset, BOOL bEnable)
{
    BOOL bRet = TRUE;

    HWI2C_HAL_FUNC();

    bRet &= HAL_HWI2C_DMA_SetINT(u16PortOffset, bEnable);
    bRet &= HAL_HWI2C_EnDMA(u16PortOffset, bEnable);
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_Init
/// @brief \b Function  \b Description: Initialize HWI2C DMA
/// @param <IN>         \b pstCfg : Init structure
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_DMA_Init(U16 u16PortOffset, HAL_HWI2C_PortCfg* pstPortCfg)
{
    U8 u8Port = 0;
    BOOL bRet = TRUE;

    HWI2C_HAL_FUNC();
    //UartSendTrace("[%s] u16PortOffset: %#x \n", __func__, u16PortOffset);

    //check pointer
    if(!pstPortCfg)
    {
        HWI2C_HAL_ERR("Port cfg null pointer!\n");
        return FALSE;
    }
    //(1) clear interrupt
    HAL_HWI2C_DMA_TxfrDone(u16PortOffset);
    //(2) reset DMA
    //(2-1) reset DMA engine
    HAL_HWI2C_DMA_Reset(u16PortOffset, TRUE);
    HAL_HWI2C_DMA_Reset(u16PortOffset, FALSE);
    //(2-2)  reset MIU module in DMA engine
    HAL_HWI2C_DMA_MiuReset(u16PortOffset, TRUE);
    HAL_HWI2C_DMA_MiuReset(u16PortOffset, FALSE);
    //(3) default configursation
    bRet &= HAL_HWI2C_DMA_SetAddrMode(u16PortOffset, pstPortCfg->eDmaAddrMode);
    bRet &= HAL_HWI2C_DMA_SetMiuPri(u16PortOffset, pstPortCfg->eDmaMiuPri);
    bRet &= HAL_HWI2C_DMA_SetMiuChannel(u16PortOffset, pstPortCfg->eDmaMiuCh);
    bRet &= HAL_HWI2C_DMA_SetMiuAddr(u16PortOffset, pstPortCfg->u32DmaPhyAddr);
    bRet &= HAL_HWI2C_DMA_Enable(u16PortOffset, pstPortCfg->bDmaEnable);
    bRet &= HAL_HWI2C_DMA_SetDelayFactor(u16PortOffset, pstPortCfg->eSpeed);
    //(4) backup configuration info
    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset, &u8Port))
    {
        memcpy(&g_stPortCfg[u8Port], pstPortCfg, sizeof(HAL_HWI2C_PortCfg));
    }

    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_WriteBytes
/// @brief \b Function  \b Description: Initialize HWI2C DMA
/// @param <IN>         \b u16SlaveCfg : slave id
/// @param <IN>         \b uAddrCnt : address size in bytes
/// @param <IN>         \b pRegAddr : address pointer
/// @param <IN>         \b uSize : data size in bytes
/// @param <IN>         \b pData : data pointer
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_DMA_WriteBytes(U16 u16PortOffset, U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData)
{
    U8 u8SlaveAddr = LOW_BYTE(u16SlaveCfg) >> 1;

    HWI2C_HAL_FUNC();

    if(!pRegAddr)
        uAddrCnt = 0;
    if(!pData)
        uSize = 0;
    //no meaning operation
    if(!uSize)
    {
        HWI2C_HAL_ERR("[DMA_W]: No data for writing!\n");
        return FALSE;
    }

    //set transfer with stop
    HAL_HWI2C_DMA_SetTxfrStop(u16PortOffset, TRUE);
    //set slave address
    HAL_HWI2C_DMA_SetSlaveAddr(u16PortOffset, u8SlaveAddr);

    //#################
    //  Set WRITE command if length 0 , cmd buffer will not be used
    //#################
    //set command buffer
    if(HAL_HWI2C_DMA_SetTxfrCmd(u16PortOffset, (U8)uAddrCnt, pRegAddr) == FALSE)
    {
        HWI2C_HAL_ERR("[DMA_W]: Set command buffer error!\n");
        return FALSE;
    }
    //set data to dram
    if(HAL_HWI2C_DMA_SetMiuData(u16PortOffset, uSize, pData) == FALSE)
    {
        HWI2C_HAL_ERR("[DMA_W]: Set MIU data error!\n");
        return FALSE;
    }
    //##################
    //  Trigger to WRITE
    if(HAL_HWI2C_DMA_WaitDone(u16PortOffset, HWI2C_DMA_WRITE) == FALSE)
    {
        //HWI2C_HAL_ERR("[DMA_W]: Transfer command error!\n");
        return FALSE;
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_ReadBytes
/// @brief \b Function  \b Description: Initialize HWI2C DMA
/// @param <IN>         \b u16SlaveCfg : slave id
/// @param <IN>         \b uAddrCnt : address size in bytes
/// @param <IN>         \b pRegAddr : address pointer
/// @param <IN>         \b uSize : data size in bytes
/// @param <IN>         \b pData : data pointer
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_DMA_ReadBytes(U16 u16PortOffset, U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData)
{
    U8 u8SlaveAddr = LOW_BYTE(u16SlaveCfg) >> 1;
    U8 u8Port = HIGH_BYTE(u16SlaveCfg);
    HAL_HWI2C_ReadMode eReadMode;

    HWI2C_HAL_FUNC();

    if(!pRegAddr)
        uAddrCnt = 0;
    if(!pData)
        uSize = 0;
    //no meaning operation
    if(!uSize)
    {
        HWI2C_HAL_ERR("[DMA_R]: No data for reading!\n");
        return FALSE;
    }
    if(u8Port >= HAL_HWI2C_PORTS)
    {
        HWI2C_HAL_ERR("[DMA_R]: Port failure!\n");
        return FALSE;
    }

    eReadMode = g_stPortCfg[u8Port].eReadMode;
    if(eReadMode >= E_HAL_HWI2C_READ_MODE_MAX)
    {
        HWI2C_HAL_ERR("[DMA_R]: Read mode failure!\n");
        return FALSE;
    }

    if(eReadMode != E_HAL_HWI2C_READ_MODE_DIRECT)
    {
        //set transfer read mode
        HAL_HWI2C_DMA_SetReadMode(u16PortOffset, eReadMode);
        //set slave address
        HAL_HWI2C_DMA_SetSlaveAddr(u16PortOffset, u8SlaveAddr);

        //#################
        //  Set WRITE command
        //#################
        //set command buffer
        if(HAL_HWI2C_DMA_SetTxfrCmd(u16PortOffset, (U8)uAddrCnt, pRegAddr) == FALSE)
        {
            HWI2C_HAL_ERR("[DMA_R:W]: Set command buffer error!\n");
            return FALSE;
        }
        HAL_HWI2C_DMA_SetDataLen(u16PortOffset, uSize);

        //##################
        //  Trigger to WRITE
        if(HAL_HWI2C_DMA_WaitDone(u16PortOffset, HWI2C_DMA_WRITE) == FALSE)
        {
            HWI2C_HAL_ERR("[DMA_R:W]: Transfer command error!\n");
            return FALSE;
        }
    }


    //#################
    //  Set READ command
    //#################
    //set transfer with stop
    HAL_HWI2C_DMA_SetTxfrStop(u16PortOffset, TRUE);
    //set slave address
    HAL_HWI2C_DMA_SetSlaveAddr(u16PortOffset, u8SlaveAddr);
    //set command length to 0
    HAL_HWI2C_DMA_SetCmdLen(u16PortOffset, 0);
    //set command length for reading
    HAL_HWI2C_DMA_SetDataLen(u16PortOffset, uSize);
    //##################
    //  Trigger to READ
    if(HAL_HWI2C_DMA_WaitDone(u16PortOffset, HWI2C_DMA_READ) == FALSE)
    {
        HWI2C_HAL_ERR("[DMA_R:R]: Transfer command error!\n");
        return FALSE;
    }
    //get data to dram
    if(HAL_HWI2C_DMA_GetMiuData(u16PortOffset, uSize, pData) == FALSE)
    {
        HWI2C_HAL_ERR("[DMA_R:R]: Get MIU data error!\n");
        return FALSE;
    }

    return TRUE;
}

//#####################
//
//  MIIC Miscellaneous Functions
//
//#####################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Init_ExtraProc
/// @brief \b Function  \b Description: Do extral procedure after initialization
/// @param <IN>         \b None :
/// @param param        \b None :
/// @param <RET>        \b None :
////////////////////////////////////////////////////////////////////////////////
void HAL_HWI2C_Init_ExtraProc(void)
{
    HWI2C_HAL_FUNC();
    //Extra procedure TODO
}

char errBuf[4096];
char *perrBuf;

U32 HAL_HWI2C_XferRead(u8 u8Port, u8 u8slaveAddr, u32 addr, int alen, u8 *pbuf, int length)
{
    BOOL ret = FALSE;
    u32 u32i = 0;
    U16 u16Offset = 0x00;
	u8 regAddr[2];

	regAddr[0] = (addr & 0xFF00)>>8;
	regAddr[1] = addr & 0xFF;


    if(u8Port >= HAL_HWI2C_PORTS)
    {
        UartSendTrace("Port index is %d >= max supported ports %d !\n", u8Port, HAL_HWI2C_PORTS);
        return FALSE;
    }


    if(NULL == pbuf)
    {
        UartSendTrace("[%s(%d)] ERROR: Send Start error \r\n", __func__, __LINE__);
        return FALSE;
    }

    //configure port register offset ==> important
    if(!_Hal_HWI2C_GetPortRegOffset(u8Port, &u16Offset))
    {
        UartSendTrace("Port index error!\n");
        return FALSE;
    }

	// write dev register addr
	if(alen > 0)
	{
		HAL_HWI2C_XferWrite(u8Port, u8slaveAddr, addr, alen, 0, 0);
	}

	HAL_HWI2C_Reset(u16Offset, TRUE);
    udelay(1);
    HAL_HWI2C_Reset(u16Offset, FALSE);
    udelay(1);

    if(g_bDMAEnable[u8Port])
    {
        UartSendTrace("I2C read DMA: port = %#x\n", u8Port);
        ret = HAL_HWI2C_DMA_ReadBytes(u16Offset, (((u8Port << 8) | u8slaveAddr)), alen, regAddr, length, pbuf);
        if(ret == FALSE)
        {
            perrBuf = &errBuf[0];
            memset(errBuf, 0, 4096);

            for(u32i = 0; u32i < length; u32i++)
            {
                perrBuf += sprintf(perrBuf, "%#x ", *pbuf);
                pbuf++;
            }
            UartSendTrace("[%s(%d)] Read ERROR: Slave dev NAK, Addr: %#x, Data: %s \r\n", __func__, __LINE__, u8slaveAddr, errBuf);
            return FALSE;
        }
        else
        {
            return 0;
        }
    }
    /* ***** 1. Send start bit ***** */
    if(!HAL_HWI2C_Start(u16Offset))
    {
        UartSendTrace("[%s(%d)] Read ERROR: Send Start error \r\n", __func__, __LINE__);
        return FALSE;

    }
    // Delay for 1 SCL cycle 10us -> 4000T
    udelay(2);
    //LOOP_DELAY(8000); //20us

    /* ***** 2. Send slave id + read bit ***** */
    if(!HAL_HWI2C_Send_Byte(u16Offset, u8slaveAddr + 1))
    {

        perrBuf = &errBuf[0];
        memset(errBuf, 0, 4096);

        for(u32i = 0; u32i < length; u32i++)
        {
            perrBuf += sprintf(perrBuf, "%#x ", *pbuf);
            pbuf++;
        }
        UartSendTrace("[%s(%d)] Read ERROR: Slave dev NAK, Addr: %#x, Data: %s \r\n", __func__, __LINE__, u8slaveAddr+1, errBuf);

        return FALSE;
    }
    udelay(1);


    /* Read data */
    for(u32i = 0; u32i < length; u32i++)
    {
        /* ***** 6. Read byte data from slave ***** */
        if((length - 1) == u32i)
        {
            HAL_HWI2C_NoAck(u16Offset);
        }
        ret = HAL_HWI2C_Recv_Byte(u16Offset, pbuf);
        pbuf++;
    }

    HAL_HWI2C_Stop(u16Offset);
    return 0;
}

/*
+------------------------------------------------------------------------------
| FUNCTION    : ms_i2c_xfer_write
+------------------------------------------------------------------------------
| DESCRIPTION : This function is called by ms_i2c_xfer
|               used to write data to i2c bus the procedure is as following

|           1. send start
|           2. send address, and wait ack and clear interrupt in wait_ack()
|           loop
|           3. send byte
|           4. wait interrupt is arised, then clear interrupt
|                             and check if recieve ACK
|
| RETURN      : When the operation is success, it return 0.
|               Otherwise Negative number will be returned.
|
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| pmsg               | x |   | pass in the slave id (addr) and R/W flag
|--------------------+---+---+-------------------------------------------------
| pbuf                | x |   | the message buffer, the buffer used to fill
|                    |   |   | data readed
|--------------------+---+---+-------------------------------------------------
| length             | x |   | the byte to be writen from slave
+------------------------------------------------------------------------------
*/
U32 HAL_HWI2C_XferWrite(u8 u8Port, u8 u8slaveAddr, u32 addr, int alen, u8 *pbuf, int length)
{
    u32 u32i = 0;
    U16 u16Offset = 0x00;
	u8 regAddr[2];

	regAddr[0] = addr & 0xFF;
	regAddr[1] = (addr & 0xFF00)>>8;

    int ret = FALSE;

    if(u8Port >= HAL_HWI2C_PORTS)
    {
        UartSendTrace("Port index is %d >= max supported ports %d !\n", u8Port, HAL_HWI2C_PORTS);
        return FALSE;
    }

    //configure port register offset ==> important
    if(!_Hal_HWI2C_GetPortRegOffset(u8Port, &u16Offset))
    {
        UartSendTrace("Port index error!\n");
        return FALSE;
    }

    HAL_HWI2C_Reset(u16Offset, TRUE);
    udelay(1);
    HAL_HWI2C_Reset(u16Offset, FALSE);
    udelay(1);

    // DMA transfer
    if(g_bDMAEnable[u8Port])
    {
        UartSendTrace("I2C write DMA: port = %#x\n", u8Port);
        ret = HAL_HWI2C_DMA_WriteBytes(u16Offset, ((u8Port << 8) | u8slaveAddr), alen, regAddr, length, pbuf);
        if(ret == FALSE)
        {
            perrBuf = &errBuf[0];
            memset(errBuf, 0, 4096);

            for(u32i = 0; u32i < length; u32i++)
            {
                perrBuf += sprintf(perrBuf, "%#x ", *pbuf);
                pbuf++;
            }

            UartSendTrace("[%s(%d)] Bus[%d] Write ERROR: Slave dev NAK, Addr: %#x, Data: %s \r\n", __func__, __LINE__, u8Port, u8slaveAddr, errBuf);
            HAL_HWI2C_Stop(u16Offset);
            return FALSE;
        }
        else
        {

            HAL_HWI2C_Stop(u16Offset);
            return 0;
        }
    }

    // write by byte
    /* ***** 1. Send start bit ***** */
    if(!HAL_HWI2C_Start(u16Offset))
    {
        UartSendTrace("[%s(%d)] Write ERROR: Send Start bit error \r\n", __func__, __LINE__);
        return FALSE;
    }
    // Delay for 1 SCL cycle 10us -> 4000T
    //LOOP_DELAY(8000); //20us
    udelay(2);
    /* ***** 2. Send slave id + write bit ***** */
    if(!HAL_HWI2C_Send_Byte(u16Offset, u8slaveAddr))
    {

        perrBuf = &errBuf[0];
        memset(errBuf, 0, 4096);

        for(u32i = 0; u32i < length; u32i++)
        {
            perrBuf += sprintf(perrBuf, "%#x ", *pbuf);
            pbuf++;
        }
        UartSendTrace("[%s(%d)] Bus[%d] Write ERROR: Slave dev NAK, Addr: %#x, Data: %s \r\n", __func__, __LINE__, u8Port, u8slaveAddr, errBuf);
        return FALSE;
    }

	/* write dev register Addr array (maximium 2 bytes)*/
	if (alen > 0) {
		for(u32i = 0; u32i < alen; u32i++)
		{

			/* ***** 4. Write high byte data to slave ***** */
			if(!HAL_HWI2C_Send_Byte(u16Offset, regAddr[u32i]))
			{
				UartSendTrace("[%s(%d)] Bus[%d] Write ERROR: Slave regAddr NAK, Addr: %#x, Data(%p): %#x \r\n", __func__, __LINE__, u8Port, u8slaveAddr, regAddr,regAddr[u32i]);
				return FALSE;
			}
		}
	}

	/* write dev register data array*/
	if((length > 0) && (pbuf != NULL))
	{
		for(u32i = 0; u32i < length; u32i++)
		{
			if(HAL_HWI2C_Send_Byte(u16Offset, *pbuf))
			{
				pbuf++;
			}
			else
			{
				UartSendTrace("[%s(%d)] Bus[%d] Write ERROR: Slave data NAK, Addr: %#x, Data(%p): %#x \r\n", __func__, __LINE__, u8Port, u8slaveAddr, pbuf,*pbuf);
				return FALSE;
			}
		}
	}

    HAL_HWI2C_Stop(u16Offset);
    return 0;
}
static int msI2CInit = false;
/*-----------------------------------------------------------------------
 * Initialization
 */
static void ms_i2c_init(struct i2c_adapter *adap, int speed, int slaveaddr)
{
    //UartSendTrace("[%s] adap->hwadapnr = %x, speed = %d, slaveaddr = %x \n", __func__, adap->hwadapnr, speed, slaveaddr);
	if(msI2CInit == false)
	{
		HalI2cInitStandard();
		msI2CInit = true;
	}
}

/*-----------------------------------------------------------------------
 * Probe to see if a chip is present.  Also good for checking for the
 * completion of EEPROM writes since the chip stops responding until
 * the write completes (typically 10mSec).
 */
static int ms_i2c_probe(struct i2c_adapter *adap, uint8_t addr)
{
    U16 u16Offset = 0x00;
	int I2cAddr8Bit = addr << 1;
    //UartSendTrace("[%s] adap->hwadapnr = %x, addr = %x \n", __func__, adap->hwadapnr, addr);

	/*
	 * perform 1 byte write transaction with just address byte
	 * (fake write)
	 */
#if 0
	send_start();
	rc = write_byte ((addr << 1) | 0);
	send_stop();
#endif

    //configure port register offset ==> important
    if(!_Hal_HWI2C_GetPortRegOffset(adap->hwadapnr, &u16Offset))
    {
        UartSendTrace("Port index error!\n");
        return !FALSE;
    }


    HAL_HWI2C_Reset(u16Offset, TRUE);
    udelay(1);
    HAL_HWI2C_Reset(u16Offset, FALSE);
    udelay(1);

    // write by byte
    /* ***** 1. Send start bit ***** */
    if(!HAL_HWI2C_Start(u16Offset))
    {
        UartSendTrace("[%s(%d)] Write ERROR: Send Start bit error \r\n", __func__, __LINE__);
        return !FALSE;
    }
    // Delay for 1 SCL cycle 10us -> 4000T
    //LOOP_DELAY(8000); //20us
    udelay(2);

    /* ***** 2. Send slave id + read bit ***** */
    if(!HAL_HWI2C_Send_Byte(u16Offset, I2cAddr8Bit))
    {
        return !FALSE;
    }

    HAL_HWI2C_Stop(u16Offset);

	return !TRUE;
}

static int  ms_i2c_read(struct i2c_adapter *adap, uchar chip, uint addr,
			int alen, uchar *buffer, int len)
{
	int I2cAddr8Bit = chip << 1;
	int failures = 0;
    //UartSendTrace("[%s] adap->hwadapnr = %x \n", __func__, adap->hwadapnr);
	//UartSendTrace("i2c_read: chip %02X addr %02X alen %d buffer(%p) %#x len %d\n",
	//	chip, addr, alen, buffer, *buffer, len);

	failures = HAL_HWI2C_XferRead(adap->hwadapnr,I2cAddr8Bit, addr, alen, buffer, len);

	return failures;
}

static int  ms_i2c_write(struct i2c_adapter *adap, uchar chip, uint addr,
			int alen, uchar *buffer, int len)
{
	int I2cAddr8Bit = chip << 1;
	int failures = 0;
    //UartSendTrace("[%s] adap->hwadapnr = %x \n", __func__, adap->hwadapnr);
	//UartSendTrace("i2c_read: chip %02X addr %02X alen %d buffer(%p) %#x len %d\n",
	//	chip, addr, alen, buffer, *buffer, len);

	failures = HAL_HWI2C_XferWrite(adap->hwadapnr, I2cAddr8Bit, addr, alen, buffer, len);
	return failures;
}

static unsigned int ms_i2c_set_bus_speed(struct i2c_adapter *adap,
			unsigned int speed)
{
    HWI2C_CLKSEL eClk;
    //UartSendTrace("[%s] adap->hwadapnr = %x, speed = %d \n", __func__, adap->hwadapnr, speed);

    switch(speed)//use Xtal = 12M Hz
    {
        case 400000: // 400 KHz
            eClk = E_HAL_HWI2C_CLKSEL_HIGH;
            break;
        case 300000: //300 KHz
            eClk = E_HAL_HWI2C_CLKSEL_NORMAL;
            break;
        case 200000: //200 KHz
            eClk = E_HAL_HWI2C_CLKSEL_SLOW;
            break;
        case 100000: //100 KHz
            eClk = E_HAL_HWI2C_CLKSEL_VSLOW;
            break;
        case 50000: //50 KHz
            eClk = E_HAL_HWI2C_CLKSEL_USLOW;
            break;
        case 25000: //25 KHz
            eClk = E_HAL_HWI2C_CLKSEL_UVSLOW;
            break;
        default:
            eClk = E_HAL_HWI2C_CLKSEL_VSLOW; // 100 KHz
            break;
    }

    _Hal_HWI2C_SetClk(adap->hwadapnr, eClk);
    return 0;
}


/*
 * Register mstar i2c adapters
 */
U_BOOT_I2C_ADAP_COMPLETE(ms_i2c_0, ms_i2c_init, ms_i2c_probe,
			 ms_i2c_read, ms_i2c_write, ms_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_MS_SPEED, CONFIG_SYS_I2C_MS_SLAVE,
			 0)
#if defined(I2C_MS_DECLARATIONS2)
U_BOOT_I2C_ADAP_COMPLETE(ms_i2c_1, ms_i2c_init, ms_i2c_probe,
			 ms_i2c_read, ms_i2c_write, ms_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_MS_SPEED,
			 CONFIG_SYS_I2C_MS_SLAVE,
			 1)
#endif
