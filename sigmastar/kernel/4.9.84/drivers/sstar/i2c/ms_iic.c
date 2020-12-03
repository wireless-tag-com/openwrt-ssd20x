/*
* ms_iic.c- Sigmastar
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
//#include "MsCommon.h"
//#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <asm/io.h>
#include <linux/clk-provider.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
//#include "mst_devid.h"
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>

#include "ms_iic.h"
#include "mhal_iic_reg.h"
#include "mhal_iic.h"
#include "ms_platform.h"
#include "cam_sysfs.h"

#define I2C_ACCESS_DUMMY_TIME   5//3
///////////////////////////////////////////&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&////////////////////////////////////////////////
///////////////////////////////////////////&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&////////////////////////////////////////////////
///////////////////////////////////////////&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&////////////////////////////////////////////////
// Local defines & local structures
////////////////////////////////////////////////////////////////////////////////
//define hwi2c ports
#define HWI2C_PORTS                   HAL_HWI2C_PORTS
#define HWI2C_PORT0                   HAL_HWI2C_PORT0
#define HWI2C_PORT1                   HAL_HWI2C_PORT1
#define HWI2C_PORT2                   HAL_HWI2C_PORT2
#define HWI2C_PORT3                   HAL_HWI2C_PORT3
#define HWI2C_STARTDLY                5 //us
#define HWI2C_STOPDLY                 5 //us
//define mutex
static DEFINE_MUTEX(i2cMutex);
I2C_DMA HWI2C_DMA[HWI2C_PORTM];

#define HWI2C_MUTEX_CREATE(_P_)          //g_s32HWI2CMutex[_P_] = MsOS_CreateMutex(E_MSOS_FIFO, (char*)gu8HWI2CMutexName[_P_] , MSOS_PROCESS_SHARED)
#define HWI2C_MUTEX_LOCK(_P_)            //OS_OBTAIN_MUTEX(g_s32HWI2CMutex[_P_],MSOS_WAIT_FOREVER)
#define HWI2C_MUTEX_UNLOCK(_P_)          //OS_RELEASE_MUTEX(g_s32HWI2CMutex[_P_])
#define HWI2C_MUTEX_DELETE(_P_)          //OS_DELETE_MUTEX(g_s32HWI2CMutex[_P_])
#define MsOS_DelayTaskUs(x)				udelay(x)
#define MS_DEBUG_MSG(x)       x

#ifdef CONFIG_MS_PADMUX
#include "mdrv_padmux.h"
#include "mdrv_puse.h"
#include "gpio.h"
#endif

//static MS_S32 g_s32HWI2CMutex[HWI2C_PORTM] = {-1,-1,-1,-1};
//static char gu8HWI2CMutexName[HWI2C_PORTM][13] = {"HWI2CMUTEXP0","HWI2CMUTEXP1","HWI2CMUTEXP2","HWI2CMUTEXP3"};

//#define EN_I2C_LOG
#ifdef EN_I2C_LOG
#define HWI2C_DBG_FUNC()               if (_geDbgLv >= E_HWI2C_DBGLV_ALL) \
                                            {MS_DEBUG_MSG(printk("\t====   %s   ====\n", __FUNCTION__);)}
#define HWI2C_DBG_INFO(x, args...)     if (_geDbgLv >= E_HWI2C_DBGLV_INFO ) \
                                            {MS_DEBUG_MSG(printk(x, ##args);)}
#define HWI2C_DBG_ERR(x, args...)      if (_geDbgLv >= E_HWI2C_DBGLV_ERR_ONLY) \
                                            {MS_DEBUG_MSG(printk(x, ##args);)}
#else
#define HWI2C_DBG_FUNC()               //printk("\t##########################   %s   ################################\n", __FUNCTION__)
#define HWI2C_DBG_INFO(x, args...)     //printk(x, ##args)
#define HWI2C_DBG_ERR(x, args...)      printk(x, ##args)
#endif


//#define I2C_ACCESS_DUMMY_TIME   50
////////////////////////////////////////////////////////////////////////////////
// Local & Global Variables
////////////////////////////////////////////////////////////////////////////////


struct mstar_i2c_dev {
	struct device *dev;
	struct i2c_adapter adapter;
	struct clk *div_clk;
	struct clk *fast_clk;
	struct reset_control *rst;
	void __iomem *base;
	void __iomem *chipbase;
	void __iomem *clkbase;
	int cont_id;
	bool irq_disabled;
	int is_dvc;
	struct completion msg_complete;
	int msg_err;
	u8 *msg_buf;
	size_t msg_buf_remaining;
	int msg_read;
	u32 bus_clk_rate;
	bool is_suspended;
	int i2cgroup;
    int i2cpadmux;
    int i2c_speed;
    int i2c_en_dma;
};
typedef struct _i2c_dev_data{
	u32 i2cirq;
	int i2cgroup;
}i2c_dev_data;


//typedef void* (*i2c_nwrite_fp)(void*);

static BOOL _gbInit = FALSE;

#ifdef EN_I2C_LOG
static HWI2C_DbgLv _geDbgLv = E_HWI2C_DBGLV_INFO; //E_HWI2C_DBGLV_ERR_ONLY;
#endif

static HWI2C_State _geState = E_HWI2C_IDLE;
static U32 g_u32StartDelay = HWI2C_STARTDLY, g_u32StopDelay = HWI2C_STOPDLY;
static HWI2C_ReadMode g_HWI2CReadMode[HWI2C_PORTS];
//static HWI2C_PORT g_HWI2CPort[HWI2C_PORTS];
static U16 g_u16DelayFactor[HWI2C_PORTS];
static BOOL g_bDMAEnable[HWI2C_PORTS];
//static U8 g_HWI2CPortIdx = HWI2C_PORT0;
////////////////////////////////////////////////////////////////////////////////
// Local Function
////////////////////////////////////////////////////////////////////////////////
#define _MDrv_HWI2C_Send_Byte HAL_HWI2C_Send_Byte
#define _MDrv_HWI2C_Recv_Byte HAL_HWI2C_Recv_Byte

#if 0
static void _MDrv_HWI2C_DelayUs(U32 u32Us)
{
    // volatile is necessary to avoid optimization
    U32 volatile u32Dummy = 0;
    //U32 u32Loop;
    U32 volatile u32Loop;

    u32Loop = (U32)(50 * u32Us);
    while (u32Loop--)
    {
        u32Dummy++;
    }
}

BOOL MDrv_MMIO_GetBASE(U32 *u32Baseaddr, U32 *u32Basesize, U32 u32Module)
{

	return TRUE ;
}
#endif


BOOL MDrv_HWI2C_Send_Byte(U16 u16PortOffset, U8 u8Data)
{
	return HAL_HWI2C_Send_Byte(u16PortOffset, u8Data);
}

BOOL MDrv_HWI2C_Recv_Byte(U16 u16PortOffset, U8 *pData)
{
	return HAL_HWI2C_Recv_Byte(u16PortOffset, pData);
}


BOOL MDrv_HWI2C_NoAck(U16 u16PortOffset)
{
    return HAL_HWI2C_NoAck(u16PortOffset);
}


BOOL MDrv_HWI2C_Reset(U16 u16PortOffset, BOOL bReset)
{
    return HAL_HWI2C_Reset(u16PortOffset, bReset);
}


BOOL _MDrv_HWI2C_GetPortRegOffset(U8 u8Port, U16 *pu16Offset)
{
    HWI2C_DBG_FUNC();

    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    return HAL_HWI2C_SetPortRegOffset(u8Port,pu16Offset);
}

BOOL _MDrv_HWI2C_ReadBytes(U8 u8Port, U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData)
{
    U8 u8SlaveID = LOW_BYTE(u16SlaveCfg);
    U16 u16Offset = 0x00;
    U16 u16Dummy = I2C_ACCESS_DUMMY_TIME; // loop dummy
    BOOL bComplete = FALSE;
    U32 uAddrCntBkp,uSizeBkp;
    U8 *pRegAddrBkp,*pDataBkp;

    HWI2C_DBG_FUNC();

    _geState = E_HWI2C_READ_DATA;
    if (!pRegAddr)
        uAddrCnt = 0;
    if (!pData)
        uSize = 0;

    //check support port index
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    //no meaning operation
    if (!uSize)
    {
        // HWI2C_DBG_ERR("Read bytes error!\n");
        return FALSE;
    }

    //configure port register offset ==> important
    if(!_MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return FALSE;
    }

    if(g_bDMAEnable[u8Port])
    {
        _geState = E_HWI2C_DMA_READ_DATA;
        return HAL_HWI2C_DMA_ReadBytes(u16Offset, u16SlaveCfg, uAddrCnt, pRegAddr, uSize, pData);
    }

    //start access routines
    uAddrCntBkp = uAddrCnt;
    pRegAddrBkp = pRegAddr;
    uSizeBkp = uSize;
    pDataBkp = pData;

    while (u16Dummy--)
    {
        if((g_HWI2CReadMode[u8Port]!=E_HWI2C_READ_MODE_DIRECT) && (uAddrCnt>0)&& (pRegAddr))
        {
            HAL_HWI2C_Start(u16Offset);
            //add extral delay by user configuration
            MsOS_DelayTaskUs(g_u32StartDelay);

            if (!_MDrv_HWI2C_Send_Byte(u16Offset,HWI2C_SET_RW_BIT(FALSE, u8SlaveID)))
                goto end;

            while(uAddrCnt--)
            {
                if (!_MDrv_HWI2C_Send_Byte(u16Offset,*pRegAddr))
                    goto end;
                pRegAddr++;
            }

            if(g_HWI2CReadMode[u8Port]==E_HWI2C_READ_MODE_DIRECTION_CHANGE_STOP_START)
            {
                HAL_HWI2C_Stop(u16Offset);
                //add extral delay by user configuration
                MsOS_DelayTaskUs(g_u32StopDelay);
            }
			else
			{
				HAL_HWI2C_Reset(u16Offset,TRUE);
				HAL_HWI2C_Reset(u16Offset,FALSE);
			}
        }

        //Very important to add delay to support all clock speeds
        //Strongly recommend that do not remove this delay routine
        HAL_HWI2C_ExtraDelay(g_u16DelayFactor[u8Port]);
        HAL_HWI2C_Start(u16Offset);

        //add extral delay by user configuration
        MsOS_DelayTaskUs(g_u32StartDelay);

        if (!_MDrv_HWI2C_Send_Byte(u16Offset,HWI2C_SET_RW_BIT(TRUE, u8SlaveID)))
            goto end;

        while(uSize)
        {
            ///////////////////////////////////
            //
            //  must set ACK/NAK before read ready
            //
            uSize--;
            if (uSize==0)
                HAL_HWI2C_NoAck(u16Offset);
            if (_MDrv_HWI2C_Recv_Byte(u16Offset,pData)==FALSE)
                goto end;
            pData++;
        }
        bComplete = TRUE;

    end:
        HAL_HWI2C_Stop(u16Offset);
        //add extral delay by user configuration
        MsOS_DelayTaskUs(g_u32StopDelay);
        if(u16Dummy&&(bComplete==FALSE))
        {
            uAddrCnt = uAddrCntBkp;
            pRegAddr = pRegAddrBkp;
            uSize = uSizeBkp;
            pData = pDataBkp;
            continue;
        }
        break;
    }
    _geState = E_HWI2C_IDLE;
	HAL_HWI2C_Reset(u16Offset,TRUE);
	HAL_HWI2C_Reset(u16Offset,FALSE);

    return bComplete;
}

BOOL _MDrv_HWI2C_WriteBytes(U8 u8Port, U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData)
{
    U8 u8SlaveID = LOW_BYTE(u16SlaveCfg);
    U16 u16Offset = 0x00;

    U16 u16Dummy = I2C_ACCESS_DUMMY_TIME; // loop dummy
    BOOL bComplete = FALSE;
    U32 uAddrCntBkp,uSizeBkp;
    U8 *pRegAddrBkp,*pDataBkp;

    HWI2C_DBG_FUNC();

    _geState = E_HWI2C_WRITE_DATA;
    if (!pRegAddr)
        uAddrCnt = 0;
    if (!pData)
        uSize = 0;

    //check support port index
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    //no meaning operation
    if (!uSize)
    {
        // HWI2C_DBG_ERR("Write bytes error!\n");
        return FALSE;
    }

    //configure port register offset ==> important
    if(!_MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return FALSE;
    }

    if(g_bDMAEnable[u8Port])
    {
        _geState = E_HWI2C_DMA_WRITE_DATA;
        return HAL_HWI2C_DMA_WriteBytes(u16Offset, u16SlaveCfg, uAddrCnt, pRegAddr, uSize, pData);
    }

    //start access routines
    uAddrCntBkp = uAddrCnt;
    pRegAddrBkp = pRegAddr;
    uSizeBkp = uSize;
    pDataBkp = pData;
    while(u16Dummy--)
    {
        HAL_HWI2C_Start(u16Offset);
        MsOS_DelayTaskUs(g_u32StartDelay);

        if (!_MDrv_HWI2C_Send_Byte(u16Offset,HWI2C_SET_RW_BIT(FALSE, u8SlaveID)))
        	{HWI2C_DBG_ERR("HWI2C_SET_RW_BIT error!\n");
            goto end;
        }
        while(uAddrCnt)
        {
            if (!_MDrv_HWI2C_Send_Byte(u16Offset, *pRegAddr))
            	{HWI2C_DBG_ERR("pRegAddr error!\n");
                goto end;
            }
            uAddrCnt--;
            pRegAddr++;
        }

        while(uSize)
        {
            if (!_MDrv_HWI2C_Send_Byte(u16Offset, *pData))
            	{HWI2C_DBG_ERR("pData error!\n");
                goto end;
            	}
            uSize--;
            pData++;
        }
        bComplete = TRUE;

    end:
        HAL_HWI2C_Stop(u16Offset);
        //add extral delay by user configuration
        MsOS_DelayTaskUs(g_u32StopDelay);
        if(u16Dummy&&(bComplete==FALSE))
        {
            uAddrCnt = uAddrCntBkp;
            pRegAddr = pRegAddrBkp;
            uSize = uSizeBkp;
            pData = pDataBkp;
            continue;
        }
        break;
    }
    _geState = E_HWI2C_IDLE;
	HAL_HWI2C_Reset(u16Offset,TRUE);
	HAL_HWI2C_Reset(u16Offset,FALSE);
    return bComplete;
}

#if defined(CONFIG_MS_PADMUX)
static int _MDrv_HWI2C_IsPadSet(HWI2C_PORT ePort)
{
    // important: need to modify if more MDRV_PUSE_I2C? defined
    if ((ePort < E_HWI2C_PORT_1 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_I2C0_SCL)) ||
        (ePort < E_HWI2C_PORT_2 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_I2C1_SCL)) )
    {
        return TRUE;
    }
    else
        return FALSE;
}
#endif

static BOOL _MDrv_HWI2C_SelectPort(HWI2C_PORT ePort)
{
    U16 u16Offset = 0x00;
    U8 u8Port = 0x00;
    BOOL bRet=TRUE;

    HWI2C_DBG_FUNC();

    //(1) Get port index by port number
    if(!HAL_HWI2C_GetPortIdxByPort((HAL_HWI2C_PORT)ePort,&u8Port))
        return FALSE;

    //(2) Set pad mux for port number
#if defined(CONFIG_MS_PADMUX)
    if (0 == mdrv_padmux_active() ||
        FALSE == _MDrv_HWI2C_IsPadSet(ePort) )
#endif
    {
        bRet &= HAL_HWI2C_SelectPort((HAL_HWI2C_PORT)ePort);
    }

    //(3) configure port register offset ==> important
    bRet &= _MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset);

    //(4) master init
    bRet &= HAL_HWI2C_Master_Enable(u16Offset);

    return bRet;
}

static BOOL _MDrv_HWI2C_SetClk(U8 u8Port, HWI2C_CLKSEL eClk)
{
    U16 u16Offset = 0x00;

    HWI2C_DBG_FUNC();
    HWI2C_DBG_INFO("Port%d clk: %u\n", u8Port, eClk);

    //check support port index
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    //check support clock speed
    if (eClk >= E_HWI2C_NOSUP)
    {
        // HWI2C_DBG_ERR("Clock [%u] is not supported!\n",eClk);
        return FALSE;
    }

    //configure port register offset ==> important
    if(!_MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return FALSE;
    }

    g_u16DelayFactor[u8Port] = (U16)(1<<(eClk));
    HWI2C_DBG_INFO("Port%d clk: %u offset:%d\n", u8Port, eClk, u16Offset);

    return HAL_HWI2C_SetClk(u16Offset,(HAL_HWI2C_CLKSEL)eClk);
}

static BOOL _MDrv_HWI2C_SetReadMode(U8 u8Port, HWI2C_ReadMode eReadMode)
{
    HWI2C_DBG_FUNC();
    HWI2C_DBG_INFO("Port%d Readmode: %u\n", u8Port, eReadMode);
    //check support port index
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    if(eReadMode>=E_HWI2C_READ_MODE_MAX)
        return FALSE;
    g_HWI2CReadMode[u8Port] = eReadMode;
    return TRUE;
}

static BOOL _MDrv_HWI2C_InitPort(HWI2C_UnitCfg *psCfg)
{
    U8 u8PortIdx = 0, u8Port = 0;
    U16 u16Offset = 0x00;
    BOOL bRet = TRUE;
    HWI2C_PortCfg stPortCfg;

    HWI2C_DBG_FUNC();

    //(1) set default value for port variables
    for(u8PortIdx=0; u8PortIdx < HWI2C_PORTS; u8PortIdx++)
    {
        stPortCfg = psCfg->sCfgPort[u8PortIdx];
        if(stPortCfg.bEnable)
        {
            if(HAL_HWI2C_GetPortIdxByPort(stPortCfg.ePort,&u8Port) && _MDrv_HWI2C_SelectPort(stPortCfg.ePort))
            {
                //set clock speed
                bRet &= _MDrv_HWI2C_SetClk(u8Port, stPortCfg.eSpeed);
                //set read mode
                bRet &= _MDrv_HWI2C_SetReadMode(u8Port, stPortCfg.eReadMode);
                //get port index
                bRet &= _MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset);
                //master init
                bRet &= HAL_HWI2C_Master_Enable(u16Offset);
                //dma init
                bRet &= HAL_HWI2C_DMA_Init(u16Offset,(HAL_HWI2C_PortCfg*)&stPortCfg);
                g_bDMAEnable[u8Port] = stPortCfg.bDmaEnable;
                //dump port information
                pr_debug("Port:%u Index=%u\n",u8Port,stPortCfg.ePort);
                pr_debug("Enable=%u\n",stPortCfg.bEnable);
                pr_debug("DmaReadMode:%u\n",stPortCfg.eReadMode);
                pr_debug("Speed:%u\n",stPortCfg.eSpeed);
                pr_debug("DmaEnable:%u\n",stPortCfg.bDmaEnable);
                pr_debug("DmaAddrMode:%u\n",stPortCfg.eDmaAddrMode);
                pr_debug("DmaMiuCh:%u\n",stPortCfg.eDmaMiuCh);
                pr_debug("DmaMiuPri:%u\n",stPortCfg.eDmaMiuPri);
                pr_debug("DmaPhyAddr:%x\n",stPortCfg.u32DmaPhyAddr);
            }
        }
    }

    //(2) check initialized port : override above port configuration
    if(HAL_HWI2C_GetPortIdxByPort(psCfg->ePort,&u8Port) && _MDrv_HWI2C_SelectPort(psCfg->ePort))
    {
        //set clock speed
        bRet &=_MDrv_HWI2C_SetClk(u8Port,psCfg->eSpeed);
        //set read mode
        bRet &=_MDrv_HWI2C_SetReadMode(u8Port,psCfg->eReadMode);
        //get port index
        //configure port register offset ==> important
        bRet &= _MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset);
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

////////////////////////////////////////////////////////////////////////////////
// Global Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_Init
/// @brief \b Function  \b Description: Init HWI2C driver
/// @param psCfg        \b IN: hw I2C config
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_Init(HWI2C_UnitCfg *psCfg)
{
    BOOL bRet = TRUE;
    U8 u8Port=0;

    HWI2C_DBG_FUNC();

    HAL_HWI2C_SetIOMapBase(psCfg->eGroup,psCfg->eBaseAddr,psCfg->eChipAddr,psCfg->eClkAddr);

    //(2) Initialize pad mux and basic settings
    pr_debug("Pinreg:%x bit:%u enable:%u speed:%u\n",psCfg->sI2CPin.u32Reg, psCfg->sI2CPin.u8BitPos, psCfg->sI2CPin.bEnable,psCfg->eSpeed);
    bRet &= HAL_HWI2C_Init_Chip();
    //(3) Initialize all port
    bRet &= _MDrv_HWI2C_InitPort(psCfg);
    //(4) Check final result
    if (!bRet)
    {
        HWI2C_DBG_ERR("I2C init fail!\n");
    }

    //(5) Extra procedure to do after initialization
    HAL_HWI2C_Init_ExtraProc();

    g_u32StartDelay = HWI2C_STARTDLY;
    g_u32StopDelay = HWI2C_STOPDLY;
    pr_debug("START default delay %d(us)\n",(int)g_u32StartDelay);
    pr_debug("STOP default delay %d(us)\n",(int)g_u32StopDelay);
    _gbInit = TRUE;

    pr_debug("HWI2C_MUTEX_CREATE!\n");
    for(u8Port=0;u8Port<(U8)HWI2C_PORTS;u8Port++)
    {
        HWI2C_MUTEX_CREATE(u8Port);
    }
    pr_debug("HWI2C(%d): initialized\n", psCfg->eGroup);
    return bRet;
}

void MDrv_HW_IIC_Init(void *base,void *chipbase,int i2cgroup,void *clkbase, int i2cpadmux, int i2cspeed, int i2c_enDma)
{
	HWI2C_UnitCfg pHwbuscfg[1];
	U8 j;

	memset(pHwbuscfg, 0, sizeof(HWI2C_UnitCfg));

	HWI2C_DMA[i2cgroup].i2c_virt_addr = dma_alloc_coherent(NULL, 4096, &HWI2C_DMA[i2cgroup].i2c_dma_addr, GFP_KERNEL);
    //We only initialze sCfgPort[0]
    for(j = 0 ; j < 1 ; j++)
    {
        pHwbuscfg[0].sCfgPort[j].bEnable = TRUE;
        //use default pad mode 1
        if(i2cgroup==0)
        {
            if(i2cpadmux == 1)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_1;
            else if(i2cpadmux == 2)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_2;
            else if(i2cpadmux == 3)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_3;
            else if(i2cpadmux == 4)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_4;
			else if(i2cpadmux == 5)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_5;
        }else if(i2cgroup==1) {
            if(i2cpadmux == 1)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_1;
            else if(i2cpadmux == 2)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_2;
            else if(i2cpadmux == 3)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_3;
            else if(i2cpadmux == 4)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_4;
            else if(i2cpadmux == 5)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_5;
        }else if(i2cgroup==2) {
            if(i2cpadmux == 1)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_1;
            else if(i2cpadmux == 2)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_2;
            else if(i2cpadmux == 3)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_3;
			else if(i2cpadmux == 4)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_4;
            else if(i2cpadmux == 5)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_5;
        }
        else if(i2cgroup==3) {
            if(i2cpadmux == 1)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_1;
            else if(i2cpadmux == 2)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_2;
            else if(i2cpadmux == 3)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_3;
            else if(i2cpadmux == 4)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_4;
            else if(i2cpadmux == 5)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_5;
        }
        pHwbuscfg[0].sCfgPort[j].eSpeed= i2cspeed; //E_HWI2C_NORMAL; //E_HAL_HWI2C_CLKSEL_VSLOW;//pIIC_Param->u8ClockIIC;//
        pHwbuscfg[0].sCfgPort[j].eReadMode = E_HWI2C_READ_MODE_DIRECT;//pIIC_Param->u8IICReadMode;//
        if (i2c_enDma == -1)
        {
            if(i2cgroup==0)
                pHwbuscfg[0].sCfgPort[j].bDmaEnable = FALSE;  //Use default setting
            else
                pHwbuscfg[0].sCfgPort[j].bDmaEnable = TRUE;  //Use default setting */
        }
        else
        {
            pHwbuscfg[0].sCfgPort[j].bDmaEnable = i2c_enDma;
        }
        pHwbuscfg[0].sCfgPort[j].eDmaAddrMode = E_HWI2C_DMA_ADDR_NORMAL;  //Use default setting
        pHwbuscfg[0].sCfgPort[j].eDmaMiuPri = E_HWI2C_DMA_PRI_LOW;  //Use default setting
        pHwbuscfg[0].sCfgPort[j].eDmaMiuCh = E_HWI2C_DMA_MIU_CH0;  //Use default setting
        pHwbuscfg[0].sCfgPort[j].u32DmaPhyAddr = HWI2C_DMA[i2cgroup].i2c_dma_addr;  //Use default setting
        j++;
    }

	pHwbuscfg[0].sI2CPin.bEnable = FALSE;
	pHwbuscfg[0].sI2CPin.u8BitPos = 0;
	pHwbuscfg[0].sI2CPin.u32Reg = 0;
	pHwbuscfg[0].eSpeed = i2cspeed; //E_HWI2C_NORMAL; //E_HAL_HWI2C_CLKSEL_VSLOW;//pIIC_Param->u8ClockIIC;//
    pHwbuscfg[0].ePort = pHwbuscfg[0].sCfgPort[0].ePort; /// port
    pHwbuscfg[0].eReadMode = E_HWI2C_READ_MODE_DIRECT; //pIIC_Param->u8IICReadMode;//
    pHwbuscfg[0].eBaseAddr=(U32)base;
    pHwbuscfg[0].eChipAddr=(U32)chipbase;
    pHwbuscfg[0].eClkAddr=(U32)clkbase;
    pHwbuscfg[0].eGroup=i2cgroup;

    MDrv_HWI2C_Init(&pHwbuscfg[0]);
}
EXPORT_SYMBOL(MDrv_HW_IIC_Init);

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_Start
/// @brief \b Function  \b Description: send start bit
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_Start(U16 u16PortOffset)
{
    HWI2C_DBG_FUNC();
    return HAL_HWI2C_Start(u16PortOffset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_Stop
/// @brief \b Function  \b Description: send stop bit
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_Stop(U16 u16PortOffset)
{
    HWI2C_DBG_FUNC();
    return HAL_HWI2C_Stop(u16PortOffset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_GetPortIndex
/// @brief \b Function  \b Description: Get port index from port number
/// @param ePort        \b IN: port number
/// @param ePort        \b OUT: pointer to port index
/// @return             \b U8: Port Index
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_GetPortIndex(HWI2C_PORT ePort, U8* pu8Port)
{
    BOOL bRet=TRUE;

    HWI2C_DBG_FUNC();

    //(1) Get port index by port number
    bRet &= HAL_HWI2C_GetPortIdxByPort((HAL_HWI2C_PORT)ePort, pu8Port);
    HWI2C_DBG_INFO("ePort:0x%02X, u8Port:0x%02X\n",(U8)ePort,(U8)*pu8Port);

    return bRet;
}

BOOL MDrv_HWI2C_CheckAbility(HWI2C_DMA_HW_FEATURE etype,ms_i2c_feature_fp *fp)
{
	if(etype >= E_HWI2C_FEATURE_MAX){
		HWI2C_DBG_ERR("etype invalid %d\n", etype);
		return FALSE;
	}
	return HAL_HWI2C_CheckAbility(etype, fp);
}

//###################
//
//  Multi-Port Support: Port 0
//
//###################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_SelectPort
/// @brief \b Function  \b Description: Decide port index and pad mux for port number
/// @param ePort        \b IN: port number
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_SelectPort(HWI2C_PORT ePort)
{
    HWI2C_DBG_FUNC();
    if(ePort >= E_HWI2C_PORT_NOSUP)
        return FALSE;
    return _MDrv_HWI2C_SelectPort(ePort);
}
#if 0
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_SetClk
/// @brief \b Function  \b Description: Set HW I2C clock
/// @param eClk         \b IN: clock rate
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_SetClk(HWI2C_CLKSEL eClk)
{
    HWI2C_DBG_FUNC();
    return _MDrv_HWI2C_SetClk(g_HWI2CPortIdx, eClk);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_SetReadMode
/// @brief \b Function  \b Description: Set HW I2C Read Mode
/// @param eClk         \b IN: ReadMode
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_SetReadMode(HWI2C_ReadMode eReadMode)
{
    return _MDrv_HWI2C_SetReadMode(g_HWI2CPortIdx, eReadMode);
}
#endif
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_ReadByte
/// @brief \b Function  \b Description: read 1 byte data
/// @param u16SlaveCfg  \b IN: [15:8]: Channel number [7:0]:Slave ID
/// @param u8RegAddr    \b IN: target register address
/// @param pData        \b Out: read 1 byte data
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_ReadByte(U16 u16SlaveCfg, U8 u8RegAddr, U8 *pData)
{
    HWI2C_DBG_FUNC();
    return MDrv_HWI2C_ReadBytes(u16SlaveCfg, 1, &u8RegAddr, 1, pData);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_ReadByte
/// @brief \b Function  \b Description: write 1 byte data
/// @param u16SlaveCfg  \b IN: [15:8]: Channel number [7:0]:Slave ID
/// @param u8RegAddr    \b IN: target register address
/// @param u8Data       \b IN: 1 byte data
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_WriteByte(U16 u16SlaveCfg, U8 u8RegAddr, U8 u8Data)
{
    HWI2C_DBG_FUNC();
    return MDrv_HWI2C_WriteBytes(u16SlaveCfg, 1, &u8RegAddr, 1, &u8Data);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_WriteBytes
/// @brief \b Function  \b Description: Init HWI2C driver
/// @param u16SlaveCfg  \b IN: [15:8]: Channel number [7:0]:Slave ID
/// @param uAddrCnt     \b IN: register address count
/// @param pRegAddr     \b IN: pointer to targert register address
/// @param uSize        \b IN: data length
/// @param pData        \b IN: data array
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_WriteBytes(U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData)
{
    BOOL bRet;
    U8 u8Port;

    u8Port = HIGH_BYTE(u16SlaveCfg);
	//u8Port = g_HWI2CPortIdx;
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    //HWI2C_MUTEX_LOCK(u8Port);
	//mutex_lock(&i2cMutex);
    bRet = _MDrv_HWI2C_WriteBytes(u8Port,u16SlaveCfg,uAddrCnt,pRegAddr,uSize,pData);
   	//HWI2C_MUTEX_UNLOCK(u8Port);
	//mutex_unlock(&i2cMutex);
    return bRet;
}
EXPORT_SYMBOL(MDrv_HWI2C_WriteBytes);

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_ReadBytes
/// @brief \b Function  \b Description: Init HWI2C driver
/// @param u16SlaveCfg  \b IN: [15:8]: Channel number [7:0]:Slave ID
/// @param uAddrCnt     \b IN: register address count
/// @param pRegAddr     \b IN: pointer to targert register address
/// @param uSize        \b IN: data length
/// @param pData        \b Out: read data aray
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL MDrv_HWI2C_ReadBytes(U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData)
{
    BOOL bRet;
    U8 u8Port;

    u8Port = HIGH_BYTE(u16SlaveCfg);
	//u8Port = g_HWI2CPortIdx;
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    //HWI2C_MUTEX_LOCK(u8Port);
	//mutex_lock(&i2cMutex);
    bRet = _MDrv_HWI2C_ReadBytes(u8Port,u16SlaveCfg,uAddrCnt,pRegAddr,uSize,pData);
    //HWI2C_MUTEX_UNLOCK(u8Port);
	//mutex_unlock(&i2cMutex);
    return bRet;
}
EXPORT_SYMBOL(MDrv_HWI2C_ReadBytes);

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

/*
+------------------------------------------------------------------------------
| FUNCTION    : ms_i2c_xfer_read
+------------------------------------------------------------------------------
| DESCRIPTION : This function is called by ms_i2c_xfer,
|                     used to read data from i2c bus
|           1. send start
|           2. send address + R (read bit), and wait ack
|           3. just set start_byte_read,
|              loop
|           4. wait interrupt is arised, then clear interrupt and read byte in
|           5. Auto generate NACK by IP,
|           6. the master does not acknowledge the final byte it receives.
|              This tells the slave that its transmission is done
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
| length             | x |   | the byte to be readed from slave
+------------------------------------------------------------------------------
*/
char errBuf[4096];
char *perrBuf;

static int
ms_i2c_xfer_read(u8 u8Port, struct i2c_msg *pmsg, u8 *pbuf, int length)
{
    BOOL ret = FALSE;
    u32 u32i = 0;
    U16 u16Offset = 0x00;

   if (NULL == pmsg)
   {
	   printk(KERN_INFO
			 "ERROR: in ms_i2c_xfer_read: pmsg is NULL pointer \r\n");
	   return -ENOTTY;
   }
   if (NULL == pbuf)
   {
	   printk(KERN_INFO
			 "ERROR: in ms_i2c_xfer_read: pbuf is NULL pointer \r\n");
	   return -ENOTTY;
   }

   //configure port register offset ==> important
    if(!_MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return FALSE;
    }

	if(g_bDMAEnable[u8Port])
    {
		//pr_err("I2C read DMA: port = %#x\n", u8Port);
        ret = MDrv_HWI2C_ReadBytes((u8Port<< 8)|(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)), length, pbuf, length, pbuf);
		if(ret==FALSE)
		{
			perrBuf = &errBuf[0];
			memset(errBuf,0,4096);

			for (u32i = 0; u32i < length; u32i++)
			{
				perrBuf += sprintf(perrBuf,"%#x ", *pbuf);
				pbuf++;
			}
			pr_info("ERROR: Bus[%d] in ms_i2c_xfer_read: Slave dev NAK, Addr: %#x, Data: %s \r\n", (u16Offset/256),(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)),errBuf);
			return -ETIMEDOUT;
		} else {
			return 0;
 		}
	}
    /* ***** 1. Send start bit ***** */
    if(!MDrv_HWI2C_Start(u16Offset))
    {
       printk(KERN_INFO
    		 "ERROR: in ms_i2c_xfer_read: Send Start error \r\n");
       return -ETIMEDOUT;

    }
    // Delay for 1 SCL cycle 10us -> 4000T
    udelay(2);
    //LOOP_DELAY(8000); //20us

    /* ***** 2. Send slave id + read bit ***** */
    if (!MDrv_HWI2C_Send_Byte(u16Offset, ((pmsg->addr & I2C_BYTE_MASK) << 1) |
    			   ((pmsg->flags & I2C_M_RD) ? 1 : 0)))
    {

		 perrBuf = &errBuf[0];
		 memset(errBuf,0,4096);

		 for (u32i = 0; u32i < length; u32i++)
		 {
			perrBuf += sprintf(perrBuf,"%#x ", *pbuf);
            pbuf++;
		 }
		 pr_info("ERROR: Bus[%d] in ms_i2c_xfer_read: Slave dev NAK, Addr: %#x, Data: %s \r\n", (u16Offset/256),(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)),errBuf);

        return -ETIMEDOUT;
    }
    udelay(1);

    /* Read data */
    for (u32i = 0; u32i < length; u32i++)
    {
      /* ***** 6. Read byte data from slave ***** */
       if ((length-1) == u32i)
       {
    	   MDrv_HWI2C_NoAck(u16Offset);
       }
       ret = MDrv_HWI2C_Recv_Byte(u16Offset, pbuf);
       pbuf++;
    }

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
static int
ms_i2c_xfer_write(u8 u8Port, struct i2c_msg *pmsg, u8 *pbuf, int length)
{
    u32 u32i = 0;
    U16 u16Offset = 0x00;
    int ret = FALSE;

    if (NULL == pmsg)
    {
        printk(KERN_INFO
              "ERROR: in ms_i2c_xfer_write: pmsg is NULL pointer \r\n");
        return -ENOTTY;
    }
    if (NULL == pbuf)
    {
        printk(KERN_INFO
              "ERROR: in ms_i2c_xfer_write: pbuf is NULL pointer \r\n");
        return -ENOTTY;
    }

    //configure port register offset ==> important
    if(!_MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return FALSE;
    }

	if(g_bDMAEnable[u8Port])
    {
		//pr_err("I2C write DMA: port = %#x\n", u8Port);
        ret = MDrv_HWI2C_WriteBytes((u8Port<< 8)|(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)), length, pbuf, length, pbuf);
		if(ret==FALSE)
		{
			perrBuf = &errBuf[0];
			memset(errBuf,0,4096);

			for (u32i = 0; u32i < length; u32i++)
			{
				perrBuf += sprintf(perrBuf,"%#x ", *pbuf);
				pbuf++;
			}
			pr_info("ERROR: Bus[%d] in ms_i2c_xfer_write: Slave dev NAK, Addr: %#x, Data: %s \r\n", (u16Offset/256),(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)),errBuf);
			return -ETIMEDOUT;
		}else{
			return 0;
		}
    }


    /* ***** 1. Send start bit ***** */
    if(!MDrv_HWI2C_Start(u16Offset))
    {
        printk(KERN_INFO
              "ERROR: in ms_i2c_xfer_write: Send Start error \r\n");
        return -ETIMEDOUT;
    }
    // Delay for 1 SCL cycle 10us -> 4000T
    //LOOP_DELAY(8000); //20us
    udelay(2);

    /* ***** 2. Send slave id + read bit ***** */
     if (!MDrv_HWI2C_Send_Byte(u16Offset, ((pmsg->addr & I2C_BYTE_MASK) << 1) |
                    ((pmsg->flags & I2C_M_RD) ? 1 : 0)))
     {

		 perrBuf = &errBuf[0];
		 memset(errBuf,0,4096);

		 for (u32i = 0; u32i < length; u32i++)
		 {
			perrBuf += sprintf(perrBuf,"%#x ", *pbuf);
            pbuf++;
		 }
		 pr_info("ERROR: Bus[%d] in ms_i2c_xfer_write: Slave dev NAK, Addr: %#x, Data: %s \r\n", (u16Offset/256),(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)),errBuf);
         return -ETIMEDOUT;
     }

    /* ***** 3. Send register address and data to write ***** */
    /* we send register is first buffer */
    for (u32i = 0; u32i < length; u32i++)
    {
        /* ***** 4. Write high byte data to slave ***** */
        if(MDrv_HWI2C_Send_Byte(u16Offset, *pbuf))
        {
            pbuf++;
        }
        else
        {
			pr_info("ERROR: Bus[%d] in ms_i2c_xfer_write: Slave data NAK, Addr: %#x, Data: %#x \r\n", (u16Offset/256),(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)),*pbuf);
            return -ETIMEDOUT;
        }
    }

    return ret;
}

/*
+------------------------------------------------------------------------------
| FUNCTION    : ms_i2c_xfer
+------------------------------------------------------------------------------
| DESCRIPTION : This function will be called by i2c-core.c i2c-transfer()
|               i2c_master_send(), and i2c_master_recv()
|               We implement the I2C communication protocol here
|               Generic i2c master transfer entrypoint.
|
| RETURN      : When the operation is success, it return the number of message
|               requrested. Negative number when error occurs.
|
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| padap               | x |   | the adaptor which the communication will be
|                    |   |   | procceed
|--------------------+---+---+-------------------------------------------------
| pmsg               | x |   | the message buffer, the buffer with message to
|                    |   |   | be sent or used to fill data readed
|--------------------+---+---+-------------------------------------------------
| num                | x |   | number of message to be transfer
+------------------------------------------------------------------------------
*/
//static int
int
ms_i2c_xfer(struct i2c_adapter *padap, struct i2c_msg *pmsg, int num)
{
    int i, err;
    U16 u16Offset = 0x00;
    BOOL bSendStop = 1;
	ms_i2c_feature_fp ms_i2c_nwrite_fp;
	struct i2c_msg *ptmpmsg = pmsg;
	BOOL bDoRead = 0;
	
    HWI2C_DBG_INFO("ms_i2c_xfer: processing %d messages:\n", num);

    i = 0;
    err = 0;

    if (NULL == padap)
    {
        printk(KERN_INFO
                "ERROR: in ms_i2c_xfer: adap is NULL pointer \r\n");
        return -ENOTTY;
    }
    if (NULL == pmsg)
    {
        printk(KERN_INFO
                "ERROR: in ms_i2c_xfer: pmsg is NULL pointer \r\n");
        return -ENOTTY;
    }

    //configure port register offset ==> important
    if(!_MDrv_HWI2C_GetPortRegOffset(padap->nr,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return FALSE;
    }

	mutex_lock(&i2cMutex);
	MDrv_HWI2C_Reset(u16Offset,TRUE);
	udelay(1);
    MDrv_HWI2C_Reset(u16Offset,FALSE);
	udelay(1);
	
    //check read cmd 
    for(i = 0; i < num; i++)
    {

        if(ptmpmsg->len && ptmpmsg->buf){
            if(ptmpmsg->flags & I2C_M_RD){
                bDoRead = 1;
                break;
            }
        }
        ptmpmsg++;  
    }
	//query nwrite mode ability and proc nwrite
	if(MDrv_HWI2C_CheckAbility(E_HWI2C_FEATURE_NWRITE, &ms_i2c_nwrite_fp) && !bDoRead)
	{
		if(ms_i2c_nwrite_fp == NULL){
			return FALSE;
		}
		
		//HWI2C_DBG_ERR("ms_i2c_nwrite_fp num %d\n", num);
		ms_i2c_nwrite_fp(u16Offset, ((pmsg->addr & I2C_BYTE_MASK) << 1), pmsg, num);
	}
	else{
/* in i2c-master_send or recv, the num is always 1,  */
/* but use i2c_transfer() can set multiple message */

	    for (i = 0; i < num; i++)
	    {
#if 0
	        printk(KERN_INFO " #%d: %sing %d byte%s %s 0x%02x\n", i,
	                pmsg->flags & I2C_M_RD ? "read" : "writ",
	                pmsg->len, pmsg->len > 1 ? "s" : "",
	                pmsg->flags & I2C_M_RD ? "from" : "to", pmsg->addr);
#endif
	        /* do Read/Write */
	        if (pmsg->len && pmsg->buf) /* sanity check */
	        {
	            bSendStop = (pmsg->flags & I2C_CUST_M_NOSTOP) ? 0 : 1;

	            if (pmsg->flags & I2C_M_RD)
	                err = ms_i2c_xfer_read(padap->nr, pmsg, pmsg->buf, pmsg->len);
	            else
	                err = ms_i2c_xfer_write(padap->nr, pmsg, pmsg->buf, pmsg->len);

	            //MDrv_HWI2C_Stop(u16Offset);

	            if (err)
				{
					mutex_unlock(&i2cMutex);
	                return err;
				}
	        }
	        pmsg++;        /* next message */
    	}
	}
    /* ***** 6. Send stop bit ***** */
    /* finish the read/write, then issues the stop condition (P).
     * for repeat start, diposit stop, two start and one stop only
     */

    if(bSendStop)
    {
        MDrv_HWI2C_Stop(u16Offset);
    }

	mutex_unlock(&i2cMutex);
    return i;
}
EXPORT_SYMBOL(ms_i2c_xfer);

/*
+------------------------------------------------------------------------------
| FUNCTION    : ms_i2c_func
+------------------------------------------------------------------------------
| DESCRIPTION : This function is returned list of supported functionality.
|
| RETURN      : return list of supported functionality
|
| Variable    : no variable
+------------------------------------------------------------------------------
*/
static u32 ms_i2c_func(struct i2c_adapter *padapter)
{
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

/* implement the i2c transfer function in algorithm structure */
static struct i2c_algorithm sg_ms_i2c_algorithm =
{
    .master_xfer = ms_i2c_xfer,
    .functionality = ms_i2c_func,
};

/* Match table for of_platform binding */
static const struct of_device_id mstar_i2c_of_match[] = {
	{ .compatible = "sstar,i2c", 0},
	{},
};

static int mstar_i2c_probe(struct platform_device *pdev)
{
	struct mstar_i2c_dev *i2c_dev;
	struct resource *res;
	void __iomem *base;
	void __iomem *chipbase;
	void __iomem *clkbase;
	struct device_node	*node = pdev->dev.of_node;
	int ret = 0;
	int i2cgroup = 0;
    int i2cpadmux = 1;
    int i2c_speed = E_HWI2C_NORMAL;
    int i2c_en_dma = -1;
	int num_parents, i;
    struct clk **iic_clks;
	i2c_dev_data *data = NULL;

    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    if(num_parents < 0)
    {
        printk( "[%s] Fail to get parent count! Error Number : %d\n", __func__, num_parents);
		ret = -ENOENT ;
		goto out;
    }
	data = kzalloc(sizeof(i2c_dev_data), GFP_KERNEL);
	if(!data){
		ret = -ENOMEM;
		goto out;
	}
    iic_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
    if(!iic_clks){
		ret = -ENOMEM;
		goto out;
    }
    //enable all clk
    for(i = 0; i < num_parents; i++)
    {
        iic_clks[i] = of_clk_get(pdev->dev.of_node, i);
        if (IS_ERR(iic_clks[i]))
        {
            printk( "[%s] Fail to get clk!\n", __func__);
			ret = -ENOENT;
        }
        else
        {
            clk_prepare_enable(iic_clks[i]);
            if(i == 0)
	            clk_set_rate(iic_clks[i], 12000000);
            clk_put(iic_clks[i]);
        }
    }
	kfree(iic_clks);
	if(ret){
		goto out;
	}
    HWI2C_DBG_INFO(" mstar_i2c_probe\n");
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res)
	{
		ret = -ENOENT;
		goto out;
	}
	base = (void *)(IO_ADDRESS(res->start));

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if(!res)
    {
    	ret = -ENOENT;
		goto out;
    }
	chipbase = (void *)(IO_ADDRESS(res->start));

	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
    if(!res)
    {
    	ret = -ENOENT;
		goto out; 
    }
	clkbase = (void *)(IO_ADDRESS(res->start));

	i2c_dev = devm_kzalloc(&pdev->dev, sizeof(*i2c_dev), GFP_KERNEL);
	if (!i2c_dev)
	{
		ret = -ENOMEM;
		goto out;
	}
	i2c_dev->base = base;
	i2c_dev->chipbase = chipbase;
	i2c_dev->adapter.algo = &sg_ms_i2c_algorithm;
	i2c_dev->cont_id = pdev->id;
	i2c_dev->dev = &pdev->dev;
	i2c_dev->clkbase = clkbase;

	of_property_read_u32(node, "i2c-group", &i2cgroup);
	//i2cgroup = of_alias_get_id(pdev->dev.of_node, "iic");
    HWI2C_DBG_INFO("i2cgroup=%d\n",i2cgroup);
	i2c_dev->i2cgroup = i2cgroup;

    of_property_read_u32(node, "i2c-padmux", &i2cpadmux);
    HWI2C_DBG_INFO("i2cpadmux=%d\n",i2cpadmux);
    i2c_dev->i2cpadmux = i2cpadmux;

    of_property_read_u32(node, "i2c-speed", &i2c_speed);
    HWI2C_DBG_INFO("i2c_speed=%d\n",i2c_speed);
    i2c_dev->i2c_speed = i2c_speed;

    of_property_read_u32(node, "i2c-en-dma", &i2c_en_dma);
    HWI2C_DBG_INFO("i2c_en_dma=%d\n",i2c_en_dma);
    i2c_dev->i2c_en_dma = i2c_en_dma;

	if (pdev->dev.of_node) {
		const struct of_device_id *match;
		match = of_match_device(mstar_i2c_of_match, &pdev->dev);
		//i2c_dev->is_dvc = of_device_is_compatible(pdev->dev.of_node,
		//				"sstar,cedric-i2c-dvc");
	} else if (pdev->id == 3) {
		i2c_dev->is_dvc = 1;
	}
 
	#ifdef CONFIG_MS_I2C_INT_ISR
	#if 0 //tmp cancel request ISR
	//init isr
	data->i2cgroup = i2c_dev->i2cgroup;
	data->i2cirq = CamIrqOfParseAndMap(node, 0);
	HAL_HWI2C_IrqRequest(data->i2cirq, i2c_dev->i2cgroup, (void*)pdev);
	//init tcond
	HAL_HWI2C_DMA_TsemInit((u8)i2c_dev->i2cgroup);
	//HWI2C_DBG_ERR("1mstar_i2c_probe i2cirq %d\n", data->i2cirq);
	#endif 
	#endif

	init_completion(&i2c_dev->msg_complete);

	platform_set_drvdata(pdev, i2c_dev);

    MDrv_HW_IIC_Init(i2c_dev->base,i2c_dev->chipbase,i2cgroup,clkbase, i2cpadmux, i2c_speed, i2c_en_dma);

	//i2c_set_adapdata(&i2c_dev->adapter, i2c_dev);
	i2c_dev->adapter.owner = THIS_MODULE;
	i2c_dev->adapter.class = I2C_CLASS_DEPRECATED;
	//strlcpy(i2c_dev->adapter.name, "Sstar I2C adapter",
	//	sizeof(i2c_dev->adapter.name));
	scnprintf(i2c_dev->adapter.name, sizeof(i2c_dev->adapter.name),
		 "Sstar I2C adapter %d", i2cgroup);
	i2c_dev->adapter.algo = &sg_ms_i2c_algorithm;

	i2c_dev->adapter.dev.parent = &pdev->dev;
	i2c_dev->adapter.nr = i2cgroup;
	i2c_dev->adapter.dev.of_node = pdev->dev.of_node;

	pdev->dev.platform_data = (void*)data;
	
    HWI2C_DBG_INFO(" i2c_dev->adapter.nr=%d\n",i2c_dev->adapter.nr);
	i2c_set_adapdata(&i2c_dev->adapter, i2c_dev);

	ret = i2c_add_numbered_adapter(&i2c_dev->adapter);
	if (ret) {
		dev_err(&pdev->dev, "Failed to add I2C adapter\n");
		goto out;
    }

	return 0;
//err return 
out:
	if(data){
		kfree(data);
	}
	//clk_unprepare(i2c_dev->div_clk);
	return ret;
}

static int mstar_i2c_remove(struct platform_device *pdev)
{
	struct mstar_i2c_dev *i2c_dev = platform_get_drvdata(pdev);
	i2c_dev_data *data;

	data = (i2c_dev_data*)pdev->dev.platform_data;
	#ifdef CONFIG_MS_I2C_INT_ISR
	//free isr and uninit I2c DMA
	HAL_HWI2C_IrqFree(data->i2cirq);
	HAL_HWI2C_DMA_TsemDeinit(data->i2cgroup);
	#endif
	kfree(data);
	i2c_del_adapter(&i2c_dev->adapter);
	
	return 0;
}

#if 0
static int mstar_i2c_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct mstar_i2c_dev *i2c_dev = platform_get_drvdata(pdev);

#if defined(CONFIG_OF)
    int num_parents, i;
    struct clk **iic_clks;

    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    iic_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);

    //disable all clk
    for(i = 0; i < num_parents; i++)
    {
        iic_clks[i] = of_clk_get(pdev->dev.of_node, i);
        if (IS_ERR(iic_clks[i]))
        {
            printk( "[iic_clks] Fail to get clk!\n" );
            kfree(iic_clks);
            return -1;
        }
        else
        {
            clk_disable_unprepare(iic_clks[i]);
        }
    }
    kfree(iic_clks);
#endif

	i2c_lock_adapter(&i2c_dev->adapter);
	i2c_dev->is_suspended = true;
	i2c_unlock_adapter(&i2c_dev->adapter);

	return 0;
}

static int mstar_i2c_resume(struct platform_device *pdev)
{
	struct mstar_i2c_dev *i2c_dev = platform_get_drvdata(pdev);

	//int ret;
#if defined(CONFIG_OF)
    int num_parents, i;
    struct clk **iic_clks;

    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    iic_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);

    //enable all clk
    for(i = 0; i < num_parents; i++)
    {
        iic_clks[i] = of_clk_get(pdev->dev.of_node, i);
        if (IS_ERR(iic_clks[i]))
        {
            printk( "[iic_clks] Fail to get clk!\n" );
            kfree(iic_clks);
            return -1;
        }
        else
        {
            clk_prepare_enable(iic_clks[i]);
            if(i == 0)
	            clk_set_rate(iic_clks[i], 12000000);
        }
        kfree(iic_clks);
    }
#endif

	i2c_lock_adapter(&i2c_dev->adapter);

    MDrv_HW_IIC_Init(i2c_dev->base,i2c_dev->chipbase,i2c_dev->i2cgroup,i2c_dev->clkbase, i2c_dev->i2cpadmux);

	i2c_dev->is_suspended = false;

	i2c_unlock_adapter(&i2c_dev->adapter);

	return 0;
}
#endif
MODULE_DEVICE_TABLE(of, mstar_i2c_of_match);


static struct platform_driver mstar_i2c_driver = {
	.probe   = mstar_i2c_probe,
	.remove  = mstar_i2c_remove,
#if 0
    .suspend = mstar_i2c_suspend,
    .resume = mstar_i2c_resume,
#endif
	.driver  = {
		.name  = "mstar-i2c",
		.owner = THIS_MODULE,
		.of_match_table = mstar_i2c_of_match,
	},
};

static int __init mstar_i2c_init_driver(void)
{
	return platform_driver_register(&mstar_i2c_driver);
}

static void __exit mstar_i2c_exit_driver(void)
{
	platform_driver_unregister(&mstar_i2c_driver);
}

subsys_initcall(mstar_i2c_init_driver);
module_exit(mstar_i2c_exit_driver);

MODULE_DESCRIPTION("Sstar I2C Bus Controller driver");
MODULE_AUTHOR("SSTAR");
MODULE_LICENSE("GPL");
