/*
* ms_sdmmc_lnx.c- Sigmastar
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
/***************************************************************************************************************
 *
 * FileName ms_sdmmc_lnx.c
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <asm/io.h>

#include "inc/ms_sdmmc_lnx.h"
#include "inc/hal_card_timer.h"
#include "inc/hal_card_platform.h"

#include "inc/hal_sdmmc_v5.h"
#include "inc/hal_card_intr_v5.h"

#include "mdrv_padmux.h"

//###########################################################################################################
#if (EN_MSYS_REQ_DMEM)
//###########################################################################################################
#include "../include/ms_msys.h"
//###########################################################################################################
#endif


#if defined(CONFIG_OF)
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#endif

#define FORCE_HAL_CLK (1) //Set 1 to use HAL driver rather than DTB. Turn this on for debugging.
//***********************************************************************************************************
// Config Setting (Internal)
//***********************************************************************************************************
#define EN_SDMMC_TRFUNC          (FALSE)
#define EN_SDMMC_TRSDIO          (FALSE)
#define EN_SDMMC_BRO_DMA         (TRUE)
#define EN_SDMMC_ADV_DMA         (TRUE)
#define EN_SDMMC_DCACHE_FLUSH    (TRUE)

#define EN_SDMMC_NOCDZ_NDERR     (FALSE)

/****** For Allocation buffer *******/
#define MAX_BLK_SIZE              512       //Maximum Transfer Block Size
#define MAX_BLK_COUNT             1024      //Maximum Transfer Block Count
#define MAX_SEG_CNT               128

/****** For broken DMA *******/
#define MAX_BRO_BLK_COUNT         1024      //Maximum Broken DMA Transfer Block Count

/****** For SD Debounce Setting *******/
#define WT_DB_PLUG                30       //Waiting time for Insert Debounce
#define WT_DB_UNPLUG              30       //Waiting time for Unplug Debounce
#define WT_DB_SW_PLUG             300       //Waiting time for Plug Delay Process
#define WT_DB_SW_UNPLUG           0         //Waiting time for Uplug Delay Process

//***********************************************************************************************************

// DTS related
static U8_T     gu8_SlotNums                = 0;
static BOOL_T   gb_ReverseCDZ               = FALSE;
static IpOrder  ge_IPOrderSlot[3]           = {IP_ORDER_0, IP_ORDER_1, IP_ORDER_2};
static PadOrder ge_PADOrderSlot[3]          = {PAD_ORDER_0, PAD_ORDER_1, PAD_ORDER_2};
static U32_T    gu32_MaxClkSlot[3]          = {400000, 400000, 400000};
static BOOL_T   gb_IntCDZSlot[3]            = {FALSE, FALSE, FALSE};
static BOOL_T   gb_FakeCDZSlot[3]           = {FALSE, FALSE, FALSE};
static U32_T    gu32_CdzNoSlot[3]           = {DEF_CDZ_PAD_SLOT0, DEF_CDZ_PAD_SLOT1, DEF_CDZ_PAD_SLOT2};
static U32_T    gu32_PwrNoSlot[3]           = {DEF_PWR_PAD_SLOT0, DEF_PWR_PAD_SLOT1, DEF_PWR_PAD_SLOT2};
static U32_T    gu32_PwrOffDelaySlot[3]     = {WT_POWEROFF, WT_POWEROFF, WT_POWEROFF};
static BOOL_T   gb_SdioUseSlot[3]           = {FALSE, FALSE, FALSE};
static U16_T    gu16_MieIntNoSlot[3]        = {0};
static U16_T    gu16_CdzIntNoSlot[3]        = {0};

static const char gu8_mie_irq_name[3][20] = {"mie0_irq", "mie1_irq", "mie2_irq"};
static const char gu8_irq_name[3][20] = {"cdz_slot0_irq", "cdz_slot1_irq", "cdz_slot2_irq"};

//
static MutexEmType ge_MutexSlot[3]      = {EV_MUTEX1, EV_MUTEX2, EV_MUTEX3};

static IntSourceStruct  gst_IntSourceSlot[3];
static spinlock_t g_RegLockSlot[3];

static volatile IpType geIpTypeIp[3] = {IP_0_TYPE, IP_1_TYPE, IP_2_TYPE};

#if defined(CONFIG_OF)
struct clk* gp_clkSlot[3];
struct clk* gp_clkMCMSlot[3];
struct clk* gp_clkSRAMSlot[3];
#endif


// Global Variable for All Slot:
//-----------------------------------------------------------------------------------------------------------
static volatile BOOL_T   gb_RejectSuspend     = (FALSE);

DEFINE_MUTEX(sdmmc1_mutex);
DEFINE_MUTEX(sdmmc2_mutex);
DEFINE_MUTEX(sdmmc3_mutex);


#define L3_FLUSH_PIPE()   Chip_Flush_MIU_Pipe()


// String Name
//-----------------------------------------------------------------------------------------------------------
#define DRIVER_NAME "ms_sdmmc"
#define DRIVER_DESC "Mstar SD/MMC Card Interface driver"

// Trace Funcion
//-----------------------------------------------------------------------------------------------------------
#if (EN_SDMMC_TRFUNC)
    #define pr_sd_err(fmt, arg...)   printk(fmt, ##arg)
    #define pr_sd_main(fmt, arg...)  printk(fmt, ##arg)
    #define pr_sd_dbg(fmt, arg...)   //printk(fmt, ##arg)
#else
    #define pr_sd_err(fmt, arg...)   printk(fmt, ##arg)
    #define pr_sd_main(fmt, arg...)  //printk(fmt, ##arg)
    #define pr_sd_dbg(fmt, arg...)   //printk(fmt, ##arg)
#endif

#if (EN_SDMMC_TRSDIO)
    #define pr_sdio_main(fmt, arg...)  printk(fmt, ##arg)
#else
    #define pr_sdio_main(fmt, arg...)
#endif



void Hal_CARD_SetGPIOIntAttr(GPIOOptEmType eGPIOOPT, unsigned int irq)
{
#if (D_OS == D_OS__LINUX)
    if(eGPIOOPT==EV_GPIO_OPT1) //clear interrupt
    {
        struct irq_data *sd_irqdata = irq_get_irq_data(irq);
        struct irq_chip *chip = irq_get_chip(irq);

        chip->irq_ack(sd_irqdata);
    }
    else if((eGPIOOPT==EV_GPIO_OPT2))
    {
    }
    else if((eGPIOOPT==EV_GPIO_OPT3))  //sd polarity _HI Trig for remove
    {
        irq_set_irq_type(irq, IRQ_TYPE_EDGE_RISING);
    }
    else if((eGPIOOPT==EV_GPIO_OPT4)) //sd polarity _LO Trig for insert
    {
        irq_set_irq_type(irq, IRQ_TYPE_EDGE_FALLING);
    }
#endif
}

// Section Process Begin
//------------------------------------------------------------------------------------------------
static void _CRIT_SECT_BEGIN(SlotEmType eSlot)
{
    MutexEmType eMutex = ge_MutexSlot[eSlot];

    if(eMutex == EV_MUTEX1)
        mutex_lock(&sdmmc1_mutex);
    else if(eMutex == EV_MUTEX2)
        mutex_lock(&sdmmc2_mutex);
    else if(eMutex == EV_MUTEX3)
        mutex_lock(&sdmmc3_mutex);
}


// Section Process End
//------------------------------------------------------------------------------------------------
static void _CRIT_SECT_END(SlotEmType eSlot)
{
    MutexEmType eMutex = ge_MutexSlot[eSlot];

    if(eMutex == EV_MUTEX1)
        mutex_unlock(&sdmmc1_mutex);
    else if(eMutex == EV_MUTEX2)
        mutex_unlock(&sdmmc2_mutex);
    else if(eMutex == EV_MUTEX3)
        mutex_unlock(&sdmmc3_mutex);

}


// Switch PAD
//------------------------------------------------------------------------------------------------
static void _SwitchPAD(SlotEmType eSlot)
{
    IpOrder eIP = ge_IPOrderSlot[eSlot];
    PadOrder ePAD = ge_PADOrderSlot[eSlot];
    U32_T nPwrPadNum = gu32_PwrNoSlot[eSlot];
    U32_T nCdzPadNum = gu32_CdzNoSlot[eSlot];
    BOOL_T bIsFakeCdz = gb_FakeCDZSlot[eSlot];

    Hal_CARD_ConfigSdPad(eIP, ePAD);
    Hal_CARD_ConfigPowerPad(eIP, (U16_T)nPwrPadNum);
    if (!bIsFakeCdz)
    {
        Hal_CARD_ConfigCdzPad(eIP, nCdzPadNum);
    }
    Hal_CARD_InitPADPin(eIP, ePAD);
}

// Set Power
//------------------------------------------------------------------------------------------------
static void _SetPower(SlotEmType eSlot, U8_T u8PowerMode)
{
    IpOrder eIP     = ge_IPOrderSlot[eSlot];
    PadOrder ePAD   = ge_PADOrderSlot[eSlot];

    if(u8PowerMode == MMC_POWER_OFF)
    {
        //
        Hal_SDMMC_ClkCtrl(eIP, FALSE, 0);

        //
        Hal_CARD_PowerOff(eIP, 0);

        //
        Hal_CARD_PullPADPin(eIP, ePAD, EV_PULLDOWN);

        //
        Hal_Timer_mSleep(gu32_PwrOffDelaySlot[eSlot]);
    }
    else if(u8PowerMode == MMC_POWER_UP)
    {
        //
        Hal_CARD_PowerOn(eIP, 0);

        Hal_Timer_uDelay(10); // For power-up waveform looks fine.
        Hal_CARD_PullPADPin(eIP, ePAD, EV_PULLUP);

        //
        Hal_Timer_mSleep(WT_POWERUP);
    }
    else if(u8PowerMode == MMC_POWER_ON)
    {
        //
        Hal_SDMMC_ClkCtrl(eIP, TRUE, 0);
        Hal_SDMMC_Reset(eIP);

        //
        Hal_Timer_mSleep(WT_POWERON);
    }
}


//------------------------------------------------------------------------------------------------
static U32_T _SetClock(SlotEmType eSlot, unsigned int u32ReffClk)
{
    U32_T u32RealClk =0 ;
    IpOrder eIP = ge_IPOrderSlot[eSlot];

    if(u32ReffClk)
    {
        u32RealClk = Hal_CARD_FindClockSetting(eIP, (U32_T)u32ReffClk);

        Hal_SDMMC_ClkCtrl(eIP, FALSE, 0);   // disable clock first to avoid unexpected clock rate output
    #if (!FORCE_HAL_CLK) && (defined(CONFIG_OF))
        clk_set_rate(gp_clkSlot[eSlot], u32RealClk);
    #else
        Hal_CARD_SetClock(eIP, u32RealClk);
    #endif
        Hal_SDMMC_ClkCtrl(eIP, TRUE, 0);    // enable clock

        Hal_SDMMC_SetNrcDelay(eIP, u32RealClk);
    }

    return u32RealClk;
}


//------------------------------------------------------------------------------------------------
static void _SetBusWidth(SlotEmType eSlot, U8_T u8BusWidth)
{
    IpOrder eIP = ge_IPOrderSlot[eSlot];

    switch(u8BusWidth)
    {
        case MMC_BUS_WIDTH_1:
            Hal_SDMMC_SetDataWidth(eIP, EV_BUS_1BIT);
            break;
        case MMC_BUS_WIDTH_4:
            Hal_SDMMC_SetDataWidth(eIP, EV_BUS_4BITS);
            break;
        case MMC_BUS_WIDTH_8:
            Hal_SDMMC_SetDataWidth(eIP, EV_BUS_8BITS);
            break;
    }
}


//------------------------------------------------------------------------------------------------
static void _SetBusTiming(SlotEmType eSlot, U8_T u8BusTiming)
{
    IpOrder eIP = ge_IPOrderSlot[eSlot];

    switch(u8BusTiming)
    {
        case MMC_TIMING_UHS_SDR12:
        case MMC_TIMING_LEGACY:
            /****** For Default Speed ******/
            Hal_SDMMC_SetBusTiming(eIP, EV_BUS_DEF);
            break;

        case MMC_TIMING_UHS_SDR25:
        case MMC_TIMING_SD_HS:
        case MMC_TIMING_MMC_HS:
        case MMC_TIMING_UHS_SDR50:
        case MMC_TIMING_UHS_SDR104:
        case MMC_TIMING_MMC_HS200:
            /****** For High Speed ******/
            Hal_SDMMC_SetBusTiming(eIP, EV_BUS_HS);
            break;

        case MMC_TIMING_UHS_DDR50:
            Hal_SDMMC_SetBusTiming(eIP, EV_BUS_DDR50);
            break;

        default:
            /****** For 300KHz IP Issue but not for Default Speed ******/
            Hal_SDMMC_SetBusTiming(eIP, EV_BUS_LOW);
            break;
    }
}

//------------------------------------------------------------------------------------------------
static BOOL_T _GetCardDetect(SlotEmType eSlot)
{
    IpOrder eIP = ge_IPOrderSlot[eSlot];

    if (gb_FakeCDZSlot[eSlot])
    {
        return (TRUE);
    }
    else
    {
        if (gb_ReverseCDZ)
            return !Hal_CARD_GetCdzState(eIP);
        else
            return Hal_CARD_GetCdzState(eIP);
    }

    return (FALSE);
}

static BOOL_T _GetWriteProtect(SlotEmType eSlot)
{
    return FALSE;
}

static BOOL_T _CardDetect_PlugDebounce(SlotEmType eSlot, U32_T u32WaitMs, BOOL_T bPrePlugStatus)
{
    BOOL_T bCurrPlugStatus = bPrePlugStatus;
    U32_T u32DiffTime = 0;

    while(u32DiffTime < u32WaitMs)
    {
        mdelay(1);
        u32DiffTime++;

        bCurrPlugStatus = _GetCardDetect(eSlot);

        if (bPrePlugStatus != bCurrPlugStatus)
        {
            /****** Print the Debounce ******/
            /*if(bPrePlugStatus)
                printk("#");
            else
                printk("$");*/
            /*********************************/
            break;
        }
    }
    return bCurrPlugStatus;
}

//------------------------------------------------------------------------------------------------
static U16_T _PreDataBufferProcess(TransEmType eTransType, struct mmc_data *data, struct ms_sdmmc_slot *sdmmchost, volatile U32_T *pu32AddrArr)
{
    struct scatterlist *p_sg = 0;
    U8_T u8Dir = ( (data->flags & MMC_DATA_READ) ? DMA_FROM_DEVICE : DMA_TO_DEVICE );
    U16_T u16sg_idx = 0;
#if (!EN_SDMMC_BRO_DMA)
    U32_T *pSGbuf = 0;
    U32_T u32TranBytes = 0;
    U32_T u32TotalSize = data->blksz * data->blocks;
    unsigned *pDMAbuf = sdmmchost->dma_buffer;

#elif (EN_SDMMC_BRO_DMA) && (EN_SDMMC_ADV_DMA)
    U16_T u16SubBCnt = 0;
    U32_T u32SubLen = 0;
    BOOL_T bEnd = (FALSE);
    unsigned *pADMAbuf = sdmmchost->adma_buffer;
    U8_T u8MIUSel = 0;

#endif

    if(eTransType == EV_CIF)
    {
        p_sg = &data->sg[0];
        pu32AddrArr[0] = (U32_T)( page_address(sg_page(p_sg)) + p_sg->offset );
        return 1;
    }

#if (EN_SDMMC_BRO_DMA)
    for(u16sg_idx=0 ; u16sg_idx< data->sg_len ; u16sg_idx++)
#else
    if(data->sg_len==1)
#endif
    {
        p_sg = &data->sg[u16sg_idx];
        p_sg->dma_address = dma_map_page(NULL, sg_page(p_sg), p_sg->offset, p_sg->length, u8Dir);

        if(dma_mapping_error(NULL, p_sg->dma_address)) //Add to avoid unmap warning!
            return 0;

        if((p_sg->dma_address==0) || (p_sg->dma_address==~0))  //Mapping Error!
            return 0;

        pu32AddrArr[u16sg_idx] = (U32_T)p_sg->dma_address;
    }

#if (EN_SDMMC_BRO_DMA) && (EN_SDMMC_ADV_DMA)

    for(u16sg_idx=0 ; u16sg_idx< data->sg_len ; u16sg_idx++)
    {
        if( u16sg_idx==((data->sg_len)-1) )
            bEnd = (TRUE);

        u32SubLen = data->sg[u16sg_idx].length;
        u16SubBCnt = (U16_T)(u32SubLen/data->blksz);
        Hal_SDMMC_ADMASetting((volatile void *)pADMAbuf, u16sg_idx, u32SubLen, u16SubBCnt, Hal_CARD_TransMIUAddr((U32_T)pu32AddrArr[u16sg_idx], &u8MIUSel), u8MIUSel, bEnd);
    }

    L3_FLUSH_PIPE();  //Avoid L3 issue

    pu32AddrArr[0] = (U32_T) sdmmchost->adma_phy_addr;
    return 1;

#elif (EN_SDMMC_BRO_DMA)

    if(data->flags & MMC_DATA_WRITE)
        L3_FLUSH_PIPE(); //Avoid L3 issue

    return (U16_T)data->sg_len;

#else
    else
    {
        if(data->flags & MMC_DATA_WRITE)  //SGbuf => DMA buf
        {
            while(u16sg_idx < data->sg_len)
            {
                p_sg = &data->sg[u16sg_idx];

                pSGbuf = kmap_atomic(sg_page(p_sg), KM_BIO_SRC_IRQ) + p_sg->offset;
                u32TranBytes   = min(u32TotalSize, p_sg->length);
                memcpy(pDMAbuf, pSGbuf, u32TranBytes);
                u32TotalSize -= u32TranBytes;
                pDMAbuf += (u32TranBytes >> 2) ;
                kunmap_atomic(pSGbuf, KM_BIO_SRC_IRQ);

                u16sg_idx++;
            }

            L3_FLUSH_PIPE(); //Avoid L3 issue

        }

        pu32AddrArr[0] = (U32_T) sdmmchost->dma_phy_addr;

    }

    return 1;

#endif

}
//------------------------------------------------------------------------------------------------
static void _PostDataBufferProcess(TransEmType eTransType, struct mmc_data *data, struct ms_sdmmc_slot *sdmmchost)
{
    struct scatterlist *p_sg = 0;
    U8_T  u8Dir = ( (data->flags & MMC_DATA_READ) ? DMA_FROM_DEVICE : DMA_TO_DEVICE );
    U16_T u16sg_idx = 0;

#if (!EN_SDMMC_BRO_DMA)
    U32_T *pSGbuf = 0;
    U32_T u32TranBytes = 0;
    U32_T u32TotalSize = data->blksz * data->blocks;
    unsigned *pDMAbuf = sdmmchost->dma_buffer;
#endif

    if(eTransType == EV_CIF)
        return;


#if (EN_SDMMC_BRO_DMA)

    for(u16sg_idx=0 ; u16sg_idx< data->sg_len ; u16sg_idx++)
    {
        p_sg = &data->sg[u16sg_idx];
        dma_unmap_page(NULL, p_sg->dma_address, p_sg->length, u8Dir);
        if ( (u8Dir==DMA_FROM_DEVICE) && EN_SDMMC_DCACHE_FLUSH && (sg_page(p_sg) != NULL) && !PageSlab(sg_page(p_sg)) )
            flush_dcache_page(sg_page(p_sg));
    }

#else

    if(data->sg_len==1)
    {
        p_sg = &data->sg[0];
        dma_unmap_page(NULL, p_sg->dma_address, p_sg->length, u8Dir);
        if ( (u8Dir==DMA_FROM_DEVICE) && EN_SDMMC_DCACHE_FLUSH && (sg_page(p_sg) != NULL) && !PageSlab(sg_page(p_sg)) )
            flush_dcache_page(sg_page(p_sg));
    }
    else
    {
        if(data->flags & MMC_DATA_READ)  //SGbuf => DMA buf
        {
            for(u16sg_idx=0 ; u16sg_idx< data->sg_len ; u16sg_idx++)
            {
                p_sg = &data->sg[u16sg_idx];

                pSGbuf = kmap_atomic(sg_page(p_sg), KM_BIO_SRC_IRQ) + p_sg->offset;
                u32TranBytes   = min(u32TotalSize, p_sg->length);
                memcpy(pSGbuf, pDMAbuf, u32TranBytes);
                u32TotalSize -= u32TranBytes;
                pDMAbuf += (u32TranBytes >> 2) ;

                flush_dcache_page(sg_page(p_sg));
                kunmap_atomic(pSGbuf, KM_BIO_SRC_IRQ);
            }

            //Avoid L3 issue
            L3_FLUSH_PIPE();

        }
    }

#endif

}

//------------------------------------------------------------------------------------------------
static U32_T _TransArrToUInt(U8_T u8Sep1, U8_T u8Sep2, U8_T u8Sep3, U8_T u8Sep4)
{
    return ((((uint)u8Sep1)<<24) | (((uint)u8Sep2)<<16) | (((uint)u8Sep3)<<8) | ((uint)u8Sep4));
}
//------------------------------------------------------------------------------------------------
static SDMMCRspEmType _TransRspType(unsigned int u32Rsp)
{

    switch(u32Rsp)
    {
        case MMC_RSP_NONE:
            return EV_NO;
        case MMC_RSP_R1:
        //case MMC_RSP_R5:
        //case MMC_RSP_R6:
        //case MMC_RSP_R7:
            return EV_R1;
        case MMC_RSP_R1B:
            return EV_R1B;
        case MMC_RSP_R2:
            return EV_R2;
        case MMC_RSP_R3:
        //case MMC_RSP_R4:
            return EV_R3;
        default:
            return EV_R1;
    }
}
//------------------------------------------------------------------------------------------------
static BOOL_T _PassPrintCMD(SlotEmType eSlot, U8_T u32Cmd, U32_T u32Arg, BOOL_T bSDIODev)
{

    if( (u32Cmd == SD_IO_RW_DIRECT) && bSDIODev)
        return (FALSE);


    if (gb_SdioUseSlot[eSlot])
    {
        if(u32Cmd == SD_SEND_IF_COND)
        {
            return (TRUE);
        }
        else if(u32Cmd == SD_IO_RW_DIRECT)
        {
            if( (u32Arg == 0x00000C00) || (u32Arg == 0x80000C08))
                return (TRUE);
        }
        return (FALSE);
    }

    // SD Use
    switch(u32Cmd)
    {
        case MMC_SEND_OP_COND:   //MMC  =>Cmd_1
        case SD_IO_SEND_OP_COND: //SDIO =>Cmd_5
        case SD_SEND_IF_COND:    //SD   =>Cmd_8
        case SD_IO_RW_DIRECT:    //SDIO =>Cmd_52
        case MMC_SEND_STATUS:    //SD   =>CMD13
        case MMC_APP_CMD:        //SD   =>Cmd55
            return (TRUE);
            break;

    }

    return (FALSE);

}

//------------------------------------------------------------------------------------------------
static int _RequestEndProcess(CmdEmType eCmdType, RspErrEmType eErrType, struct ms_sdmmc_slot *p_sdmmc_slot, struct mmc_data *data)
{
    int nErr = 0;
    ErrGrpEmType eErrGrp;

    if( eErrType == EV_STS_OK )
    {
        pr_sdio_main("_[%01X]", Hal_SDMMC_GetDATBusLevel(ge_IPOrderSlot[p_sdmmc_slot->slotNo]));
        pr_sd_main("@\n");
    }
    else
    {
        pr_sd_main("=> (Err: 0x%04X)", (U16_T)eErrType);
        nErr = (U32_T) eErrType;

        if(eCmdType != EV_CMDRSP)
        {
            eErrGrp = Hal_SDMMC_ErrGroup(eErrType);

            switch((U16_T)eErrGrp)
            {
                case EV_EGRP_TOUT:
                    nErr =  -ETIMEDOUT;
                    break;

                case EV_EGRP_COMM:
                    nErr = -EILSEQ;
                    break;
            }
        }
    }

    if( eErrType == EV_STS_OK )
        return nErr;


    /****** (2) Special Error Process for Stop Wait Process ******/
    if(eErrType == EV_SWPROC_ERR && data && EN_SDMMC_NOCDZ_NDERR)
    {
        data->bytes_xfered = data->blksz * data->blocks;
        nErr = 0;
        pr_sd_main("_Pass");
    }

    pr_sd_main("\n");

    return nErr;
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_cdzint
 *     @author jeremy.wang (2012/5/8)
 * Desc: Int funtion for GPIO Card Detection
 *
 * @param irq :
 * @param p_dev_id :
 *
 * @return irqreturn_t  :
 ----------------------------------------------------------------------------------------------------------*/
static irqreturn_t ms_sdmmc_cdzint(int irq, void *p_dev_id)
{
    irqreturn_t irq_t = IRQ_NONE;
    IntSourceStruct* pstIntSource = p_dev_id;
    struct ms_sdmmc_slot *p_sdmmc_slot = pstIntSource->p_data;

    //
    disable_irq_nosync(irq);

    //
    tasklet_schedule(&p_sdmmc_slot->hotplug_tasklet);
    irq_t = IRQ_HANDLED;

    return irq_t;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_hotplug
 *     @author jeremy.wang (2012/1/5)
 * Desc: Hotplug function for Card Detection
 *
 * @param data : ms_sdmmc_slot struct pointer
 ----------------------------------------------------------------------------------------------------------*/
static void ms_sdmmc_hotplug(unsigned long data)
{
    struct ms_sdmmc_slot  *p_sdmmc_slot = (struct ms_sdmmc_slot  *) data;
    SlotEmType eSlot = (SlotEmType)p_sdmmc_slot->slotNo;
    IpOrder eIP = ge_IPOrderSlot[eSlot];
    GPIOOptEmType eINSOPT = EV_GPIO_OPT3;
    GPIOOptEmType eEJTOPT = EV_GPIO_OPT4;


    if (gb_ReverseCDZ)
    {
        eINSOPT = EV_GPIO_OPT4;
        eEJTOPT = EV_GPIO_OPT3;
    }


    pr_sd_dbg("\n>> [sdmmc_%u] CDZ... ", eSlot);

LABEL_LOOP_HOTPLUG:

    if( _GetCardDetect(eSlot) ) // Insert (CDZ)
    {
        if( (FALSE) == _CardDetect_PlugDebounce(eSlot, WT_DB_PLUG, TRUE) )
            goto LABEL_LOOP_HOTPLUG;

        mmc_detect_change(p_sdmmc_slot->mmc, msecs_to_jiffies(WT_DB_SW_PLUG));
        pr_sd_dbg("(INS) OK!\n");

        Hal_CARD_SetGPIOIntAttr(EV_GPIO_OPT1, p_sdmmc_slot->cdzIRQNo);
        Hal_CARD_SetGPIOIntAttr(eINSOPT, p_sdmmc_slot->cdzIRQNo);


    }
    else // Remove (CDZ)
    {
        if( (TRUE) == _CardDetect_PlugDebounce(eSlot, WT_DB_UNPLUG, FALSE) )
                goto LABEL_LOOP_HOTPLUG;

        if (p_sdmmc_slot->mmc->card)
            mmc_card_set_removed(p_sdmmc_slot->mmc->card);

        Hal_SDMMC_StopProcessCtrl(eIP, TRUE);
        mmc_detect_change(p_sdmmc_slot->mmc, msecs_to_jiffies(WT_DB_SW_UNPLUG));
        pr_sd_dbg("(EJT) OK!\n");

        Hal_CARD_SetGPIOIntAttr(EV_GPIO_OPT1, p_sdmmc_slot->cdzIRQNo);
        Hal_CARD_SetGPIOIntAttr(eEJTOPT, p_sdmmc_slot->cdzIRQNo);

    }

    enable_irq(p_sdmmc_slot->cdzIRQNo);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_request
 *     @author jeremy.wang (2011/5/19)
 * Desc: Request funciton for any commmand
 *
 * @param p_mmc_host : mmc_host structure pointer
 * @param p_mmc_req :  mmc_request structure pointer
 ----------------------------------------------------------------------------------------------------------*/
static void ms_sdmmc_request(struct mmc_host *p_mmc_host, struct mmc_request *p_mmc_req)
{
    struct ms_sdmmc_slot   *p_sdmmc_slot  = mmc_priv(p_mmc_host);
    struct mmc_command *cmd = p_mmc_req->cmd;
    struct mmc_command *stop = p_mmc_req->stop;
    struct mmc_data *data = p_mmc_req->data;

    RspStruct * eRspSt;
    RspErrEmType eErr = EV_STS_OK;
    CmdEmType eCmdType = EV_CMDRSP;
    TransEmType eTransType = ((EN_SDMMC_ADV_DMA) && (EN_SDMMC_BRO_DMA) ? EV_ADMA : EV_DMA );
    SlotEmType eSlot = (SlotEmType)p_sdmmc_slot->slotNo;
    IpOrder eIP = ge_IPOrderSlot[eSlot];
    volatile U32_T au32Addr[MAX_SEG_CNT];

    BOOL_T bCloseClock = FALSE;
    U8_T u8CMD = 0;
    U16_T u16BlkSize = 0, u16BlkCnt = 0, u16SubBlkCnt = 0;
    U16_T u16ProcCnt = 0, u16Idx= 0;
    U32_T u32Arg = 0, u32SubLen = 0;
    _CRIT_SECT_BEGIN(eSlot);

    u8CMD =(U8_T)cmd->opcode;
    u32Arg = (U32_T)cmd->arg;

    if(!p_sdmmc_slot->mmc->card)
       Hal_SDMMC_StopProcessCtrl(eIP, FALSE);

#if (!FORCE_HAL_CLK) && (defined(CONFIG_OF))
    clk_set_rate(gp_clkSlot[eSlot], p_sdmmc_slot->currRealClk);
#else
    Hal_CARD_SetClock(eIP, p_sdmmc_slot->currRealClk);
#endif

    pr_sdio_main("_[%01X]_", Hal_SDMMC_GetDATBusLevel(eIP));
    pr_sd_main(">> [sdmmc_%u] CMD_%u (0x%08X)", eSlot, u8CMD, u32Arg);

    Hal_SDMMC_SetCmdToken(eIP, u8CMD, u32Arg);
    /****** Simple SD command *******/
    if(!data)
    {
        Hal_SDMMC_SetSDIOIntBeginSetting(eIP, u8CMD, u32Arg, EV_CMDRSP, 0);
        eErr = Hal_SDMMC_SendCmdAndWaitProcess(eIP, EV_EMP, EV_CMDRSP, _TransRspType(mmc_resp_type(cmd)), TRUE);
        Hal_SDMMC_SetSDIOIntEndSetting(eIP, eErr, 0);
    }
    else // R/W SD Command
    {
        u16BlkSize = (U16_T)data->blksz;
        u16BlkCnt  = (U16_T)data->blocks;
        u32SubLen = (U32_T) data->sg[0].length;
        u16SubBlkCnt = (U16_T)(u32SubLen/u16BlkSize);

        eCmdType = ( (data->flags & MMC_DATA_READ) ? EV_CMDREAD : EV_CMDWRITE );
        bCloseClock = ( (stop) ? FALSE : TRUE );

        pr_sd_main("__[Sgl: %u] (TB: %u)(BSz: %u)", (U16_T)data->sg_len, u16BlkCnt, u16BlkSize);

        u16ProcCnt = _PreDataBufferProcess(eTransType, data, p_sdmmc_slot, au32Addr);
        if(u16ProcCnt==0)
        {
            pr_err("\n>> [sdmmc_%u] Err: DMA Mapping Addr Error!\n", eSlot);
            eErr = EV_OTHER_ERR;
            goto LABEL_SD_ERR;
        }
        else if(u16ProcCnt==1)
        {
            u32SubLen = u16BlkSize * u16BlkCnt;
            u16SubBlkCnt = u16BlkCnt;
        }

        pr_sd_dbg("\n____[0] =>> (SBCnt: %u)__[Addr: 0x%08X]", u16SubBlkCnt, au32Addr[0]);

        Hal_SDMMC_TransCmdSetting(eIP, eTransType, u16SubBlkCnt, u16BlkSize, Hal_CARD_TransMIUAddr(au32Addr[0], NULL), (volatile U8_T*)au32Addr[0]);
        Hal_SDMMC_SetSDIOIntBeginSetting(eIP, u8CMD, u32Arg, eCmdType, u16BlkCnt);
        eErr = Hal_SDMMC_SendCmdAndWaitProcess(eIP, eTransType, eCmdType, _TransRspType(mmc_resp_type(cmd)), bCloseClock);


        if( ((U16_T)eErr) == EV_STS_OK )
        {
            data->bytes_xfered += u32SubLen;

            /****** Broken DMA *******/
            for(u16Idx=1 ; u16Idx<u16ProcCnt; u16Idx++)
            {
                u32SubLen = (U32_T) data->sg[u16Idx].length;
                u16SubBlkCnt = (U16_T)(u32SubLen/u16BlkSize);
                pr_sd_dbg("\n____[%u] =>> (SBCnt: %u)__[Addr: 0x%08X]", u16Idx, u16SubBlkCnt, au32Addr[u16Idx]);

                Hal_SDMMC_TransCmdSetting(eIP, eTransType, u16SubBlkCnt, u16BlkSize, Hal_CARD_TransMIUAddr(au32Addr[u16Idx], NULL), (volatile U8_T *)au32Addr[u16Idx]);
                eErr = Hal_SDMMC_RunBrokenDmaAndWaitProcess(eIP, eCmdType);

                if((U16_T)eErr) break;
                data->bytes_xfered += u32SubLen;
            }
        }

        Hal_SDMMC_SetSDIOIntEndSetting(eIP, eErr, u16BlkCnt);

        _PostDataBufferProcess(eTransType, data, p_sdmmc_slot);

    }

LABEL_SD_ERR:

    cmd->error = _RequestEndProcess(eCmdType, eErr, p_sdmmc_slot, data);

    if(data)
        data->error = cmd->error;

    eRspSt = Hal_SDMMC_GetRspToken(eIP);
    cmd->resp[0] = _TransArrToUInt(eRspSt->u8ArrRspToken[1], eRspSt->u8ArrRspToken[2], eRspSt->u8ArrRspToken[3], eRspSt->u8ArrRspToken[4]);
    if(eRspSt->u8RspSize == 0x10)
    {
        cmd->resp[1] = _TransArrToUInt(eRspSt->u8ArrRspToken[5], eRspSt->u8ArrRspToken[6], eRspSt->u8ArrRspToken[7], eRspSt->u8ArrRspToken[8]);
        cmd->resp[2] = _TransArrToUInt(eRspSt->u8ArrRspToken[9], eRspSt->u8ArrRspToken[10], eRspSt->u8ArrRspToken[11], eRspSt->u8ArrRspToken[12]);
        cmd->resp[3] = _TransArrToUInt(eRspSt->u8ArrRspToken[13], eRspSt->u8ArrRspToken[14], eRspSt->u8ArrRspToken[15], 0);
    }

    /****** Print Error Message******/
    if(!data && cmd->error && !_PassPrintCMD(eSlot, u8CMD, u32Arg, (BOOL_T)p_sdmmc_slot->sdioFlag)) //Cmd Err but Pass Print Some Cmds
    {
        if(cmd->error == -EILSEQ)
            pr_sd_err(">> [sdmmc_%u] Warn: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)\n", eSlot, u8CMD, u32Arg, (U16_T)eErr, cmd->resp[0], eRspSt->u32ErrLine);
        else
            pr_sd_err(">> [sdmmc_%u] Err: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)\n", eSlot, u8CMD, u32Arg, (U16_T)eErr, cmd->resp[0], eRspSt->u32ErrLine);
    }
    else if(data && data->error) //Data Err
    {
        if(data->error == -EILSEQ)
            pr_sd_err(">> [sdmmc_%u] Warn: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)(B:%u/%u)(I:%u/%u)\n", \
                  eSlot, u8CMD, u32Arg, (U16_T)eErr, cmd->resp[0], eRspSt->u32ErrLine, u16SubBlkCnt, u16BlkCnt, u16Idx, u16ProcCnt);
        else
            pr_sd_err(">> [sdmmc_%u] Err: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)(B:%u/%u)(I:%u/%u)\n", \
                  eSlot, u8CMD, u32Arg, (U16_T)eErr, cmd->resp[0], eRspSt->u32ErrLine, u16SubBlkCnt, u16BlkCnt, u16Idx, u16ProcCnt);
    }

    /****** Send Stop Cmd ******/
    if(stop)
    {
        u8CMD = (U8_T)stop->opcode;
        u32Arg = (U32_T)stop->arg;
        pr_sd_main(">> [sdmmc_%u]_CMD_%u (0x%08X)", eSlot, u8CMD, u32Arg);

        Hal_SDMMC_SetCmdToken(eIP, u8CMD, u32Arg);
        Hal_SDMMC_SetSDIOIntBeginSetting(eIP, u8CMD, u32Arg, EV_CMDRSP, 0);
        eErr = Hal_SDMMC_SendCmdAndWaitProcess(eIP, EV_EMP, EV_CMDRSP, _TransRspType(mmc_resp_type(stop)), TRUE);
        Hal_SDMMC_SetSDIOIntEndSetting(eIP, eErr, 0);

        stop->error = _RequestEndProcess(EV_CMDRSP, eErr, p_sdmmc_slot, data);

        eRspSt = Hal_SDMMC_GetRspToken(eIP);
        stop->resp[0] = _TransArrToUInt(eRspSt->u8ArrRspToken[1], eRspSt->u8ArrRspToken[2], eRspSt->u8ArrRspToken[3], eRspSt->u8ArrRspToken[4]);

        if(stop->error)
            pr_sd_err(">> [sdmmc_%u] Err: #Cmd_12 => (E: 0x%04X)(S: 0x%08X)__(L:%u)\n", eSlot, (U16_T)eErr, stop->resp[0], eRspSt->u32ErrLine);


    }

    //Hal_CARD_SetClock(eIP, p_sdmmc_slot->pmrsaveClk); // For Power Saving

    _CRIT_SECT_END(eSlot);
    mmc_request_done(p_mmc_host, p_mmc_req);

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_set_ios
 *     @author jeremy.wang (2011/5/19)
 * Desc: Set IO bus Behavior
 *
 * @param p_mmc_host : mmc_host structure pointer
 * @param p_mmc_ios :  mmc_ios  structure pointer
 ----------------------------------------------------------------------------------------------------------*/
static void ms_sdmmc_set_ios(struct mmc_host *p_mmc_host, struct mmc_ios *p_mmc_ios)
{
    struct ms_sdmmc_slot *p_sdmmc_slot = mmc_priv(p_mmc_host);
    SlotEmType eSlot = (SlotEmType)p_sdmmc_slot->slotNo;

    _CRIT_SECT_BEGIN(eSlot);

    /****** Clock Setting*******/
    if(p_sdmmc_slot->currClk != p_mmc_ios->clock)
    {
/* Modified by Spade: enable clk in probe
        #if defined(CONFIG_OF)
        if(p_mmc_ios->clock>0)
            clk_prepare_enable(gp_clkSlot[eSlot]);
        else
            clk_disable_unprepare(gp_clkSlot[eSlot]);
        #endif
*/
        p_sdmmc_slot->currClk = p_mmc_ios->clock;
        p_sdmmc_slot->currRealClk = _SetClock(eSlot, p_sdmmc_slot->currClk);

        if( (p_sdmmc_slot->currRealClk==0) && (p_sdmmc_slot->currClk!=0) )
        {
            pr_sd_err(">> [sdmmc_%u] Set IOS => Clk=Error\n", eSlot);
        }
        else if(p_sdmmc_slot->currRealClk <= 400000)
        {
            _SetBusTiming(eSlot, 0xFF);
        }
        else
        {
            pr_sd_dbg(">> [sdmmc_%u] Set IOS => Clk=%u (Real=%u)\n", eSlot, p_sdmmc_slot->currClk, p_sdmmc_slot->currRealClk);
        }
    }

    /****** Power Switch Setting *******/
    if(p_sdmmc_slot->currPowrMode != p_mmc_ios->power_mode)
    {
        p_sdmmc_slot->currPowrMode = p_mmc_ios->power_mode;
        pr_sd_main(">> [sdmmc_%u] Set IOS => Power=%u\n", eSlot, p_sdmmc_slot->currPowrMode);
        _SetPower(eSlot, p_sdmmc_slot->currPowrMode);

        if(p_sdmmc_slot->currPowrMode == MMC_POWER_OFF)
        {
            p_sdmmc_slot->initFlag = 0;
            p_sdmmc_slot->sdioFlag = 0;
        }
    }

    /****** Bus Width Setting*******/
    if( (p_sdmmc_slot->currWidth != p_mmc_ios->bus_width) || !p_sdmmc_slot->initFlag)
    {
        p_sdmmc_slot->currWidth = p_mmc_ios->bus_width;
        _SetBusWidth(eSlot, p_sdmmc_slot->currWidth);
        pr_sd_main(">> [sdmmc_%u] Set IOS => BusWidth=%u\n", eSlot, p_sdmmc_slot->currWidth);
    }

    /****** Bus Timing Setting*******/
    if( (p_sdmmc_slot->currTiming != p_mmc_ios->timing) || !p_sdmmc_slot->initFlag)
    {
        p_sdmmc_slot->currTiming = p_mmc_ios->timing;
        _SetBusTiming(eSlot, p_sdmmc_slot->currTiming);
        pr_sd_main(">> [sdmmc_%u] Set IOS => BusTiming=%u\n", eSlot, p_sdmmc_slot->currTiming);
    }

#if 0
    /****** Voltage Setting *******/
    if( (p_sdmmc_slot->currVdd != p_mmc_ios->signal_voltage) || !p_sdmmc_slot->initFlag)
    {
        p_sdmmc_slot->currVdd = p_mmc_ios->signal_voltage;

        // set voltage function
    }
#endif

    p_sdmmc_slot->initFlag = 1;

    _CRIT_SECT_END(eSlot);


}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_get_ro
 *     @author jeremy.wang (2011/5/19)
 * Desc:  Get SD card read/write permission
 *
 * @param p_mmc_host : mmc_host structure pointer
 *
 * @return int  :  1 = read-only, 0 = read-write.
 ----------------------------------------------------------------------------------------------------------*/
static int ms_sdmmc_get_ro(struct mmc_host *p_mmc_host)
{
    struct ms_sdmmc_slot   *p_sdmmc_slot  = mmc_priv(p_mmc_host);
    SlotEmType eSlot = (SlotEmType)p_sdmmc_slot->slotNo;

    _CRIT_SECT_BEGIN(eSlot);

    if( _GetWriteProtect(eSlot) )    //For CB2 HW Circuit, WP=>NWP
        p_sdmmc_slot->read_only = 1;
    else
        p_sdmmc_slot->read_only = 0;

    _CRIT_SECT_END(eSlot);

    pr_sd_main(">> [sdmmc_%u] Get RO => (%d)\n", eSlot, p_sdmmc_slot->read_only);

    return p_sdmmc_slot->read_only;
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_get_cd
 *     @author jeremy.wang (2011/6/17)
 * Desc: Get SD card detection status
 *
 * @param p_mmc_host : mmc_host structure pointer
 *
 * @return int  :  1 = Present
 ----------------------------------------------------------------------------------------------------------*/
static int ms_sdmmc_get_cd(struct mmc_host *p_mmc_host)
{
    struct ms_sdmmc_slot   *p_sdmmc_slot  = mmc_priv(p_mmc_host);
    SlotEmType eSlot = (SlotEmType)p_sdmmc_slot->slotNo;


    if( _GetCardDetect(eSlot) )
        p_sdmmc_slot->card_det = 1;
    else
        p_sdmmc_slot->card_det  = 0;

    return p_sdmmc_slot->card_det;
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_init_card
 *     @author jeremy.wang (2012/2/20)
 * Desc:
 *
 * @param p_mmc_host :
 * @param p_mmc_card :
 ----------------------------------------------------------------------------------------------------------*/
static void ms_sdmmc_init_card(struct mmc_host *p_mmc_host, struct mmc_card *p_mmc_card)
{
    struct ms_sdmmc_slot   *p_sdmmc_slot  = mmc_priv(p_mmc_host);
    SlotEmType eSlot = (SlotEmType)p_sdmmc_slot->slotNo;
    IpOrder eIP     = ge_IPOrderSlot[eSlot];

    Hal_SDMMC_SDIODeviceCtrl(eIP, TRUE);
    p_sdmmc_slot->sdioFlag = 1;

    pr_sd_dbg(">> [sdmmc_%u] Found SDIO Device!\n", eSlot);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_enable_sdio_irq
 *     @author jeremy.wang (2012/2/20)
 * Desc:
 *
 * @param p_mmc_host :
 * @param enable :
 ----------------------------------------------------------------------------------------------------------*/
static void ms_sdmmc_enable_sdio_irq(struct mmc_host *p_mmc_host, int enable)
{
    struct ms_sdmmc_slot   *p_sdmmc_slot  = mmc_priv(p_mmc_host);
    SlotEmType eSlot = (SlotEmType)p_sdmmc_slot->slotNo;
    IpOrder eIP     = ge_IPOrderSlot[eSlot];

    // Remove Original VER_04 spin lock
    // TODO: SMP consideration!!!
    // should add spin lock here?

    Hal_SDMMC_SDIOIntDetCtrl(eIP, (BOOL_T)enable);

    if(enable)
    {
        pr_sdio_main(">> [sdmmc_%u] =========> SDIO IRQ EN=> (%d)\n", eSlot, enable);
    }
}

/**********************************************************************************************************
 * Define Static Global Structs
 **********************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------
 *  st_mmc_ops
 ----------------------------------------------------------------------------------------------------------*/
static const struct mmc_host_ops st_mmc_ops =
{
    .request         = ms_sdmmc_request,
    .set_ios         = ms_sdmmc_set_ios,
    .get_ro          = ms_sdmmc_get_ro,
    .get_cd          = ms_sdmmc_get_cd,
    .init_card       = ms_sdmmc_init_card,
    .enable_sdio_irq = ms_sdmmc_enable_sdio_irq,
};



#if defined(CONFIG_OF)

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_dts_init
 *     @author jeremy.wang (2017/3/24)
 * Desc: Device Tree Init
 *
 * @param p_dev : platform device
 ----------------------------------------------------------------------------------------------------------*/
static int ms_sdmmc_dts_init(struct platform_device *p_dev)
{
    U8_T  slotNo = 0 , ipidx = 0;
    SlotEmType eSlot;

    U32_T u32_SlotNums = 0;
    U32_T u32_ReverseCDZ = 0;
    U32_T u32_IPOrderSlot[3];
    U32_T u32_PADOrderSlot[3];
    U32_T u32_MaxClkSlot[3];
    U32_T u32_IntCDZSlot[3];
    U32_T u32_FakeCDZSlot[3];
    U32_T u32_CdzNoSlot[3];
    U32_T u32_PwrNoSlot[3];
    U32_T u32_PwrOffDelaySlot[3];
    U32_T u32_SdioUseSlot[3];

    // Get u32_SlotNums first for getting other DTS entry !
    if (of_property_read_u32(p_dev->dev.of_node, "slotnum" , &u32_SlotNums))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [slotnum] option!\n");
        return 1;
    }

    if (of_property_read_u32(p_dev->dev.of_node, "revcdz" , &u32_ReverseCDZ))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [revcdz] option!\n");
        return 1;
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "slot-ip-orders", (U32_T*)u32_IPOrderSlot, u32_SlotNums))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [slot-ip-orders] option!\n");
        return 1;
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "slot-pad-orders", (U32_T*)u32_PADOrderSlot, u32_SlotNums))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [slot-pad-orders] option!\n");
        return 1;
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "slot-max-clks", (U32_T*)u32_MaxClkSlot, u32_SlotNums))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [slot-max-clks] option!\n");
        return 1;
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "slot-intcdzs", (U32_T*)u32_IntCDZSlot, u32_SlotNums))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [slot-intcdzs] option!\n");
        return 1;
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "slot-fakecdzs", (U32_T*)u32_FakeCDZSlot, u32_SlotNums))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [slot-fakecdzs] option!\n");
        return 1;
    }

    if (mdrv_padmux_active() == 0)
    {
        if (of_property_read_u32_array(p_dev->dev.of_node, "slot-cdzs-gpios", (U32_T *)u32_CdzNoSlot, u32_SlotNums))
        {
            pr_err(">> [sdmmc] Err: Could not get dts [slot-cdzs-gpios] option!\n");
            return 1;
        }

        if (of_property_read_u32_array(p_dev->dev.of_node, "slot-pwr-gpios", (U32_T *)u32_PwrNoSlot, u32_SlotNums))
        {
            pr_err(">> [sdmmc] Err: Could not get dts [slot-pwr-gpios] option!\n");
            return 1;
        }
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "slot-pwr-off-delay", (U32_T *)u32_PwrOffDelaySlot, u32_SlotNums))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [slot-pwr-off-delay] option!\n");
        return 1;
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "slot-sdio-use", (U32_T *)u32_SdioUseSlot, u32_SlotNums))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [slot-sdio-use] option!\n");
        return 1;
    }

    //
    gu8_SlotNums = (U8_T)u32_SlotNums;
    gb_ReverseCDZ = (BOOL_T)u32_ReverseCDZ;

    for (slotNo =0; slotNo<gu8_SlotNums; slotNo++)
    {
        eSlot = (SlotEmType)slotNo;

        if (mdrv_padmux_active() != 0)
        {
            if (Hal_CARD_GetPadInfoCdzPad((IpOrder)u32_IPOrderSlot[eSlot], &u32_CdzNoSlot[eSlot]) == 0 )
            {
                pr_err(">> [sdmmc] Err: slotNo = %u, Could not get dts from Padmux dts [slot-cdzs-gpios] option!\n", slotNo);
                return 1;
            }

            if (Hal_CARD_GetPadInfoPowerPad((IpOrder)u32_IPOrderSlot[eSlot], &u32_PwrNoSlot[eSlot]) == 0 )
            {
                pr_err(">> [sdmmc] Err: slotNo = %u, Could not get dts from Padmux dts [slot-pwr-gpios] option!\n", slotNo);
                return 1;
            }
        }

        // Check conflict
        if (u32_SdioUseSlot[eSlot] == TRUE)
        {
            if (geIpTypeIp[u32_IPOrderSlot[eSlot]] != IP_TYPE_SDIO)
            {
                pr_err(">> [sdmmc] slotNo = %u, When SDIO is used, IpType need to be IP_TYPE_SDIO, current setting = %u\n", slotNo, (U8_T)geIpTypeIp[u32_IPOrderSlot[eSlot]]);
                return 1;
            }

            if (u32_FakeCDZSlot[eSlot] == FALSE)
            {
                pr_err(">> [sdmmc] slotNo = %u, When SDIO is used, we strongly recommend to enable FAKE_CDZ.\n", slotNo);
                return 1;
            }
        }

        // Transfer to Global variable
        ge_IPOrderSlot[eSlot] = (IpOrder)u32_IPOrderSlot[eSlot];
        ge_PADOrderSlot[eSlot] = (PadOrder)u32_PADOrderSlot[eSlot];
        gu32_MaxClkSlot[eSlot] = (U32_T)u32_MaxClkSlot[eSlot];
        gb_IntCDZSlot[eSlot] = (BOOL_T)u32_IntCDZSlot[eSlot];
        gb_FakeCDZSlot[eSlot] = (BOOL_T)u32_FakeCDZSlot[eSlot];
        gu32_CdzNoSlot[eSlot] = (U32_T)u32_CdzNoSlot[eSlot];
        gu32_PwrNoSlot[eSlot] = (U32_T)u32_PwrNoSlot[eSlot];
        gu32_PwrOffDelaySlot[eSlot] = (U32_T)u32_PwrOffDelaySlot[eSlot];
        gb_SdioUseSlot[eSlot] = (BOOL_T)u32_SdioUseSlot[eSlot];

        // MIE irq depend on which IP
        gu16_MieIntNoSlot[eSlot] = of_irq_get_byname(p_dev->dev.of_node, gu8_mie_irq_name[u32_IPOrderSlot[eSlot]]);

        // CDZ irq
        gu16_CdzIntNoSlot[eSlot] = of_irq_get_byname(p_dev->dev.of_node, gu8_irq_name[slotNo]);
    }

    //Debug
    pr_sd_dbg(">> [sdmmc] SlotNums= %u\n", gu8_SlotNums);
    pr_sd_dbg(">> [sdmmc] RevCDZ= %u\n", gb_ReverseCDZ);
    pr_sd_dbg(">> [sdmmc] SlotIPs[0-2]= %u, %u, %u \n", (U8_T)ge_IPOrderSlot[0], (U8_T)ge_IPOrderSlot[1], (U8_T)ge_IPOrderSlot[2]);
    pr_sd_dbg(">> [sdmmc] SlotPADs[0-2]= %u, %u, %u \n", (U8_T)ge_PADOrderSlot[0], (U8_T)ge_PADOrderSlot[1], (U8_T)ge_PADOrderSlot[2]);
    pr_sd_dbg(">> [sdmmc] SlotMaxClk[0-2]= %u, %u, %u \n", gu32_MaxClkSlot[0], gu32_MaxClkSlot[1], gu32_MaxClkSlot[2]);
    pr_sd_dbg(">> [sdmmc] IntCDZSlot[0-2]= %u, %u, %u \n", gb_IntCDZSlot[0], gb_IntCDZSlot[1], gb_IntCDZSlot[2]);
    pr_sd_dbg(">> [sdmmc] SlotFakeCDZ[0-2]= %u, %u, %u \n", gb_FakeCDZSlot[0], gb_FakeCDZSlot[1], gb_FakeCDZSlot[2]);
    pr_sd_dbg(">> [sdmmc] gu32_CdzNoSlot[0-2]= %u, %u, %u \n", gu32_CdzNoSlot[0], gu32_CdzNoSlot[1], gu32_CdzNoSlot[2]);
    pr_sd_dbg(">> [sdmmc] PwrNoSlot[0-2]= %u, %u, %u \n", gu32_PwrNoSlot[0], gu32_PwrNoSlot[1], gu32_PwrNoSlot[2]);
    pr_sd_dbg(">> [sdmmc] PwrOffDelaySlot[0-2]= %u, %u, %u \n", gu32_PwrOffDelaySlot[0], gu32_PwrOffDelaySlot[1], gu32_PwrOffDelaySlot[2]);
    pr_sd_dbg(">> [sdmmc] gb_SdioUseSlot[0-2]= %u, %u, %u \n", gb_SdioUseSlot[0], gb_SdioUseSlot[1], gb_SdioUseSlot[2]);
    pr_sd_dbg(">> [sdmmc] MieIntNoSlot[0-2]= %u, %u, %u \n", gu16_MieIntNoSlot[0], gu16_MieIntNoSlot[1], gu16_MieIntNoSlot[2]);
    pr_sd_dbg(">> [sdmmc] CdzIntNoSlot[0-2]= %u, %u, %u \n", gu16_CdzIntNoSlot[0], gu16_CdzIntNoSlot[1], gu16_CdzIntNoSlot[2]);

    //
    for(slotNo =0; slotNo<gu8_SlotNums; slotNo++)
    {
        eSlot = (SlotEmType)slotNo;
        ipidx = (U8_T)ge_IPOrderSlot[eSlot];

#if 1
        /****** Get Device Tree Information******/
        gp_clkSlot[eSlot] = of_clk_get(p_dev->dev.of_node, (ipidx*3));

        if(IS_ERR(gp_clkSlot[slotNo]))
        {
            pr_err(">> [sdmmc_%u] Err: Failed to get dts clock tree!\n", slotNo);
            return 1;
        }

        clk_prepare_enable(gp_clkSlot[slotNo]);
#else
        /****** Get Device Tree Information******/
        gp_clkSlot[eSlot] = of_clk_get(p_dev->dev.of_node, (ipidx*3));
        gp_clkMCMSlot[eSlot] = of_clk_get(p_dev->dev.of_node, (ipidx*3) + 1);
        gp_clkSRAMSlot[eSlot] = of_clk_get(p_dev->dev.of_node, (ipidx*3) + 2);

        if(IS_ERR(gp_clkSlot[slotNo]) || IS_ERR(gp_clkMCMSlot[eSlot]) || IS_ERR(gp_clkSRAMSlot[eSlot]) )
        {
            pr_err(">> [sdmmc_%u] Err: Failed to get dts clock tree!\n", slotNo);
            return 1;
        }

        clk_prepare_enable(gp_clkMCMSlot[slotNo]);
        clk_prepare_enable(gp_clkSRAMSlot[slotNo]);
        clk_prepare_enable(gp_clkSlot[slotNo]);
#endif
    }

    return 0;
}

#endif

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_init_slot
 *     @author jeremy.wang (2015/12/9)
 * Desc: Init Slot Setting
 *
 * @param slotNo : Slot Number
 * @param p_sdmmc_host : ms_sdmmc_host
 *
 * @return int  : Error Status; Return 0 if no error
 ----------------------------------------------------------------------------------------------------------*/
static int ms_sdmmc_init_slot(unsigned int slotNo, struct ms_sdmmc_host *p_sdmmc_host)
{
    struct ms_sdmmc_slot    *p_sdmmc_slot;
    struct mmc_host         *p_mmc_host;
    SlotEmType eSlot = (SlotEmType)slotNo;
    IpOrder eIP     = ge_IPOrderSlot[eSlot];
    int nRet = 0;


//###########################################################################################################
#if (EN_MSYS_REQ_DMEM)
    //###########################################################################################################
    MSYS_DMEM_INFO mem_info;
//###########################################################################################################
#endif
//###########################################################################################################


    /****** (1) Allocte MMC and SDMMC host ******/
    p_mmc_host = mmc_alloc_host(sizeof(struct ms_sdmmc_slot), &p_sdmmc_host->pdev->dev);

    if (!p_mmc_host)
    {
        pr_err(">> [sdmmc_%u] Err: Failed to Allocate mmc_host!\n", slotNo);
        return -ENOMEM;
    }

    /****** (2) SDMMC host setting ******/
    p_sdmmc_slot               = mmc_priv(p_mmc_host);

#if (!EN_SDMMC_BRO_DMA)

//###########################################################################################################
#if !(EN_MSYS_REQ_DMEM)
    //###########################################################################################################
    p_sdmmc_slot->dma_buffer   = dma_alloc_coherent(NULL, MAX_BLK_COUNT * MAX_BLK_SIZE, &p_sdmmc_slot->dma_phy_addr, GFP_KERNEL);
    if (!p_sdmmc_slot->dma_buffer)
    {
        pr_err(">> [sdmmc_%u] Err: Failed to Allocate sdmmc_host DMA buffer\n", slotNo);
        return -ENOMEM;
    }
//###########################################################################################################
#else
//###########################################################################################################
    mem_info.length = MAX_BLK_COUNT * MAX_BLK_SIZE;
    strcpy(mem_info.name, "SDMMC_SGBUF");
    if(msys_request_dmem(&mem_info))
    {
        pr_err(">> [sdmmc_%u] Err: Failed to Allocate sdmmc_host DMA buffer\n", slotNo);
        return -ENOMEM;
    }

    p_sdmmc_slot->dma_phy_addr = (dma_addr_t)mem_info.phys;
    p_sdmmc_slot->dma_buffer   = (U32_T*)((U32_T)mem_info.kvirt);
//###########################################################################################################
#endif

#elif (EN_SDMMC_ADV_DMA)

//###########################################################################################################
#if !(EN_MSYS_REQ_DMEM)
    //###########################################################################################################
    p_sdmmc_slot->adma_buffer   = dma_alloc_coherent(NULL, sizeof(AdmaDescStruct) * MAX_SEG_CNT, &p_sdmmc_slot->adma_phy_addr, GFP_KERNEL);
    if (!p_sdmmc_slot->adma_buffer)
    {
        pr_err(">> [sdmmc_%u] Err: Failed to Allocate sdmmc_host ADMA buffer\n", slotNo);
        return -ENOMEM;
    }
//###########################################################################################################
#else
//###########################################################################################################
    mem_info.length = sizeof(AdmaDescStruct) * MAX_SEG_CNT;
    sprintf(mem_info.name,"%s%01d","SDMMC_ADMABUF",slotNo);
    if(msys_request_dmem(&mem_info))
    {
        pr_err(">> [sdmmc_%u] Err: Failed to Allocate sdmmc_host ADMA buffer\n", slotNo);
        return -ENOMEM;
    }

    p_sdmmc_slot->adma_phy_addr = (dma_addr_t)mem_info.phys;
    p_sdmmc_slot->adma_buffer   = (U32_T*)((U32_T)mem_info.kvirt);

//###########################################################################################################
#endif



#endif

    p_sdmmc_slot->mmc          = p_mmc_host;
    p_sdmmc_slot->slotNo       = slotNo;
    p_sdmmc_slot->pmrsaveClk   = Hal_CARD_FindClockSetting(eIP, 400000);
    p_sdmmc_slot->mieIRQNo     = gu16_MieIntNoSlot[eSlot];
    p_sdmmc_slot->cdzIRQNo     = gu16_CdzIntNoSlot[eSlot];
    p_sdmmc_slot->pwrGPIONo    = gu32_PwrNoSlot[eSlot];
    p_sdmmc_slot->initFlag     = 0;
    p_sdmmc_slot->sdioFlag     = 0;

    p_sdmmc_slot->currClk      = 0;
    p_sdmmc_slot->currWidth    = 0;
    p_sdmmc_slot->currTiming   = 0;
    p_sdmmc_slot->currPowrMode = MMC_POWER_OFF;
    p_sdmmc_slot->currVdd      = 0;
    p_sdmmc_slot->currDDR      = 0;


    /***** (3) MMC host setting ******/
    p_mmc_host->ops = &st_mmc_ops;
    p_mmc_host->f_min = p_sdmmc_slot->pmrsaveClk;

    p_mmc_host->f_max = gu32_MaxClkSlot[eSlot];

    p_mmc_host->ocr_avail = MMC_VDD_32_33|MMC_VDD_31_32|MMC_VDD_30_31|MMC_VDD_29_30|MMC_VDD_28_29|MMC_VDD_27_28|MMC_VDD_165_195;
    p_mmc_host->caps = MMC_CAP_4_BIT_DATA|MMC_CAP_MMC_HIGHSPEED|MMC_CAP_SD_HIGHSPEED;

    if (gb_SdioUseSlot[eSlot]) // SDIO Card is non-removable and polling is unnecessary. (so cdz is)
    {
        p_mmc_host->caps |= MMC_CAP_NONREMOVABLE;
    }
    else
    {
        if (!gb_IntCDZSlot[eSlot])
        {
            p_mmc_host->caps |= MMC_CAP_NEEDS_POLL;
        }
    }

#if (EN_SDMMC_BRO_DMA)
    p_mmc_host->max_blk_count  = MAX_BRO_BLK_COUNT;
#else
    p_mmc_host->max_blk_count  = MAX_BLK_COUNT;
#endif
    p_mmc_host->max_blk_size   = MAX_BLK_SIZE;

    p_mmc_host->max_req_size   = p_mmc_host->max_blk_count  * p_mmc_host->max_blk_size;
    p_mmc_host->max_seg_size   = p_mmc_host->max_req_size;

    p_mmc_host->max_segs       = MAX_SEG_CNT;

    p_sdmmc_host->sdmmc_slot[slotNo] = p_sdmmc_slot;

    /****** (4) IP Once Setting for Different Platform ******/
    Hal_CARD_IPOnceSetting(eIP);


    /****** (5) Init GPIO Setting ******/
    _SwitchPAD(eSlot);

#if 0 // Not all platform's power come from sdmmc DTS, so mask it.
    if (1)
    {
        nRet = gpio_request(p_sdmmc_slot->pwrGPIONo, "SD Power Pin");

        if (nRet)
        {
            pr_sd_err(">> [sdmmc_%u] Err: Failed to request PWR GPIO (%u)\n", slotNo, p_sdmmc_slot->pwrGPIONo);
            goto INIT_FAIL_2;
        }
    }
#endif

    /****** (6) Interrupt Source Setting ******/
    gst_IntSourceSlot[eSlot].slotNo = slotNo;
    gst_IntSourceSlot[eSlot].eIP = eIP;
    gst_IntSourceSlot[eSlot].p_data = p_sdmmc_slot;

    /*****  (7) Spinlock Init for Reg Protection ******/
    spin_lock_init(&g_RegLockSlot[slotNo]);

    /****** (8) Register IP IRQ *******/
#if(EN_BIND_CARD_INT)
    Hal_SDMMC_MIEIntCtrl(eIP, FALSE);

#if 1 // MIE
    nRet = request_irq(p_sdmmc_slot->mieIRQNo, Hal_CARD_INT_MIE, IRQF_TRIGGER_NONE, DRIVER_NAME "_mie", &gst_IntSourceSlot[eSlot]);
    if (nRet)
    {
        pr_err(">> [sdmmc_%u] Err: Failed to request MIE Interrupt (%u)!\n", slotNo, p_sdmmc_slot->mieIRQNo);
        goto INIT_FAIL_2;
    }

    Hal_SDMMC_MIEIntCtrl(eIP, TRUE);

    if (gb_SdioUseSlot[eSlot]) // When SDIO Card is used, we enable SDIO int.
    {
        p_mmc_host->caps |= MMC_CAP_SDIO_IRQ;
        Hal_CARD_INT_SetMIEIntEn_ForSDIO(eIP, TRUE);

        pr_sd_dbg(">> [sdmmc_%u] Enable SDIO Interrupt Mode! \n", slotNo);
    }
    else
    {
         p_mmc_host->caps2 = MMC_CAP2_NO_SDIO;
    }
#endif

#endif

    // Don't pre power up
    p_mmc_host->caps2 |= MMC_CAP2_NO_PRESCAN_POWERUP;

    //
    mmc_add_host(p_mmc_host);

    // CDZ IRQ
    if (gb_IntCDZSlot[eSlot])
    {
        tasklet_init(&p_sdmmc_slot->hotplug_tasklet, ms_sdmmc_hotplug, (unsigned long)p_sdmmc_slot);

        //
        Hal_CARD_SetGPIOIntAttr((_GetCardDetect(eSlot)? EV_GPIO_OPT3 : EV_GPIO_OPT4), p_sdmmc_slot->cdzIRQNo);
        Hal_CARD_SetGPIOIntAttr(EV_GPIO_OPT1, p_sdmmc_slot->cdzIRQNo);

        nRet = request_irq(p_sdmmc_slot->cdzIRQNo, ms_sdmmc_cdzint, IRQF_TRIGGER_NONE, DRIVER_NAME "_cdz", &gst_IntSourceSlot[eSlot]);
        if (nRet)
        {
            pr_err(">> [sdmmc_%u] Err: Failed to request CDZ Interrupt (%u)!\n", slotNo, p_sdmmc_slot->cdzIRQNo);
            goto INIT_FAIL_1;
        }

        pr_sd_dbg(">> [sdmmc_%u] Int CDZ use Ext GPIO IRQ: (%u)\n", slotNo, p_sdmmc_slot->cdzIRQNo);

        Hal_CARD_SetGPIOIntAttr( EV_GPIO_OPT2, p_sdmmc_slot->cdzIRQNo);

#if 0 // Make irq wake up system from suspend.
        irq_set_irq_wake(p_sdmmc_slot->cdzIRQNo, TRUE);
#endif
    }

    // Return Success
    return 0;

INIT_FAIL_1:
    tasklet_kill(&p_sdmmc_slot->hotplug_tasklet);
    free_irq(p_sdmmc_slot->mieIRQNo, &gst_IntSourceSlot[eSlot]);

    mmc_remove_host(p_mmc_host);
    mmc_free_host(p_mmc_host);

#if(EN_BIND_CARD_INT)
INIT_FAIL_2:
#endif
#if (!EN_SDMMC_BRO_DMA)

//###########################################################################################################
#if !(EN_MSYS_REQ_DMEM)
    //###########################################################################################################
    if (p_sdmmc_slot->dma_buffer)
        dma_free_coherent(NULL, MAX_BLK_COUNT * MAX_BLK_SIZE, p_sdmmc_slot->dma_buffer, p_sdmmc_slot->dma_phy_addr);
//###########################################################################################################
#else
//###########################################################################################################
    mem_info.length = MAX_BLK_COUNT * MAX_BLK_SIZE;
    strcpy(mem_info.name, "SDMMC_SGBUF");
    mem_info.phys = (unsigned long long)p_sdmmc_slot->dma_phy_addr;
    msys_release_dmem(&mem_info);
//###########################################################################################################
#endif


#endif

    return nRet;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_probe
 *     @author jeremy.wang (2011/5/18)
 * Desc: Probe Platform Device
 *
 * @param p_dev : platform_device
 *
 * @return int : Error Status; Return 0 if no error.
 ----------------------------------------------------------------------------------------------------------*/
//struct ms_sdmmc_host *p_sdmmc_host;
static int ms_sdmmc_probe(struct platform_device *p_dev)
{
    struct ms_sdmmc_host *p_sdmmc_host;
    unsigned int slotNo = 0;
    int ret = 0;

    pr_info(">> [sdmmc] ms_sdmmc_probe \n");

    p_sdmmc_host = kzalloc(sizeof(struct ms_sdmmc_host), GFP_KERNEL);

    if (!p_sdmmc_host)
    {
        pr_err(">> [sdmmc] Err: Failed to Allocate p_sdmmc_host!\n\n");
        return -ENOMEM;
    }

    p_sdmmc_host->pdev = p_dev;

    /***** device data setting ******/
    platform_set_drvdata(p_dev, p_sdmmc_host);

    /***** device PM wakeup setting ******/
    device_init_wakeup(&p_dev->dev, 1);

#if defined(CONFIG_OF)
    if (ms_sdmmc_dts_init(p_dev))
    {
        pr_err(">> [sdmmc] Err: Failed to use DTS function!\n\n");
        return -EINVAL;
    }
#else
    // CONFIG_OF is necessary
    return 1;
#endif

    for (slotNo=0; slotNo<gu8_SlotNums; slotNo++)
    {
        ret = ms_sdmmc_init_slot(slotNo, p_sdmmc_host);
        pr_info(">> [sdmmc_%u] Probe Platform Devices\n", slotNo);
        if (ret!=0)
        {
            pr_err(">> [sdmmc_%u] Err: Failed to init slot!\n",slotNo);
            kfree(p_sdmmc_host);
            return ret;
        }
    }

    return 0;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_remove_slot
 *     @author jeremy.wang (2015/12/9)
 * Desc: Remove Slot Setting
 *
 * @param slotNo : Slot Number
 * @param p_sdmmc_host : ms_sdmmc_host
 ----------------------------------------------------------------------------------------------------------*/
static void ms_sdmmc_remove_slot(unsigned int slotNo, struct ms_sdmmc_host *p_sdmmc_host)
{
    struct ms_sdmmc_slot *p_sdmmc_slot = p_sdmmc_host->sdmmc_slot[slotNo];
    struct mmc_host      *p_mmc_host = p_sdmmc_slot->mmc;
    SlotEmType eSlot = (SlotEmType)slotNo;

//###########################################################################################################
#if (EN_MSYS_REQ_DMEM)
//###########################################################################################################
    MSYS_DMEM_INFO mem_info;
//###########################################################################################################
#endif
//###########################################################################################################

    mmc_remove_host(p_mmc_host);


//###########################################################################################################
#if !(EN_MSYS_REQ_DMEM)
//###########################################################################################################
    if (p_sdmmc_slot->dma_buffer)
        dma_free_coherent(NULL, MAX_BLK_COUNT*MAX_BLK_SIZE, p_sdmmc_slot->dma_buffer, p_sdmmc_slot->dma_phy_addr);
//###########################################################################################################
#else
//###########################################################################################################
    mem_info.length = MAX_BLK_COUNT * MAX_BLK_SIZE;
    strcpy(mem_info.name, "SDMMC_SGBUF");
    mem_info.phys = (unsigned long long)p_sdmmc_slot->dma_phy_addr;
    msys_release_dmem(&mem_info);
//###########################################################################################################
#endif


#if 1 // MIE
    free_irq(p_sdmmc_slot->mieIRQNo, &gst_IntSourceSlot[eSlot]);
#endif

    mmc_free_host(p_mmc_host);

    if (gb_IntCDZSlot[eSlot])
    {
        tasklet_kill(&p_sdmmc_slot->hotplug_tasklet);
        if(p_sdmmc_slot->cdzIRQNo)
            free_irq(p_sdmmc_slot->cdzIRQNo, &gst_IntSourceSlot[eSlot]);
    }

#if defined(CONFIG_OF)
    if(gp_clkSlot[eSlot])
    {
        clk_put(gp_clkSlot[eSlot]);
        gp_clkSlot[eSlot] = NULL;
    }
    if(gp_clkMCMSlot[eSlot])
    {
        clk_put(gp_clkMCMSlot[eSlot]);
        gp_clkMCMSlot[eSlot] = NULL;
    }
    if(gp_clkSRAMSlot[eSlot])
    {
        clk_put(gp_clkSRAMSlot[eSlot]);
        gp_clkSRAMSlot[eSlot] = NULL;
    }
#endif

}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_remove
 *     @author jeremy.wang (2011/5/18)
 * Desc: Revmoe MMC host
 *
 * @param p_dev :  platform device structure
 *
 * @return int  : Error Status; Return 0 if no error.
 ----------------------------------------------------------------------------------------------------------*/
static int ms_sdmmc_remove(struct platform_device *p_dev)
{
    struct ms_sdmmc_host *p_sdmmc_host = platform_get_drvdata(p_dev);
    unsigned int slotNo = 0;

    platform_set_drvdata(p_dev, NULL);

    for(slotNo=0; slotNo<gu8_SlotNums; slotNo++)
    {
        ms_sdmmc_remove_slot(slotNo, p_sdmmc_host);
        pr_sd_dbg(">> [sdmmc_%u] Remove devices...\n", slotNo);

    }

    return 0;
}


#if defined(CONFIG_OF)

    #ifdef CONFIG_PM
    static int ms_sdmmc_devpm_prepare(struct device *dev)
    {
        return 0;
    }

    static void ms_sdmmc_devpm_complete(struct device *dev)
    {

    }

    static int ms_sdmmc_devpm_suspend(struct device *dev)
    {
        unsigned int slotNo = 0;
        int tret = 0;

        for(slotNo=0; slotNo<gu8_SlotNums; slotNo++)
        {
            clk_disable_unprepare(gp_clkSlot[slotNo]);
            clk_disable_unprepare(gp_clkMCMSlot[slotNo]);
            clk_disable_unprepare(gp_clkSRAMSlot[slotNo]);

            pr_sd_dbg(">> [sdmmc_%u] Suspend device pm...(Ret:%u) \n", slotNo);
        }

        return tret;
    }

    static int ms_sdmmc_devpm_resume(struct device *dev)
    {
        unsigned int slotNo = 0;
        int ret = 0, tret = 0;

        for(slotNo=0; slotNo<gu8_SlotNums; slotNo++)
        {
            ret = clk_prepare_enable(gp_clkMCMSlot[slotNo]);

            if(ret!=0)
                tret = ret;

            ret = clk_prepare_enable(gp_clkSRAMSlot[slotNo]);

            if(ret!=0)
                tret = ret;

            ret = clk_prepare_enable(gp_clkSlot[slotNo]);

            if(ret!=0)
                tret = ret;

            pr_sd_dbg(">> [sdmmc_%u] Resume device pm...(Ret:%u) \n", slotNo, ret);
        }

        return tret;
    }

    #else

    #define ms_sdmmc_devpm_prepare	NULL
    #define ms_sdmmc_devpm_complete	NULL
    #define ms_sdmmc_devpm_suspend	NULL
    #define ms_sdmmc_devpm_resume	NULL

    #endif

    static int ms_sdmmc_devpm_runtime_suspend(struct device *dev)
    {
        pr_sd_dbg(">> [sdmmc] Runtime Suspend device pm...\n");
        return 0;
    }

    static int ms_sdmmc_devpm_runtime_resume(struct device *dev)
    {
        pr_sd_dbg(">> [sdmmc] Runtime Resume device pm...\n");
        return 0;
    }

    #define ms_sdmmc_suspend        NULL
    #define ms_sdmmc_resume         NULL


#else

    #ifdef CONFIG_PM

        #if 0 //( LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0) ) // CONFIG_PM

        /*----------------------------------------------------------------------------------------------------------
         *
         * Function: ms_sdmmc_suspend
         *     @author jeremy.wang (2011/5/18)
         * Desc: Suspend MMC host
         *
         * @param p_dev :   platform device structure
         * @param state :   Power Management Transition State
         *
         * @return int  :   Error Status; Return 0 if no error.
         ----------------------------------------------------------------------------------------------------------*/
        static int ms_sdmmc_suspend(struct platform_device *p_dev, pm_message_t state)
        {
            struct ms_sdmmc_host *p_sdmmc_host = platform_get_drvdata(p_dev);
            struct mmc_host      *p_mmc_host;
            unsigned int slotNo = 0;
            int ret = 0, tret = 0;


            for(slotNo=0; slotNo<gu8_SlotNums; slotNo++)
            {
                if(gb_RejectSuspend)
                    return -1;

                p_mmc_host = p_sdmmc_host->sdmmc_slot[slotNo]->mmc;

                if (p_mmc_host)
                {

                #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
                    ret = mmc_suspend_host(p_mmc_host);
                #else
                    ret = mmc_suspend_host(p_mmc_host, state);
                #endif
                    pr_sd_dbg(">> [sdmmc_%u] Suspend host...(Ret:%u) \n", slotNo, ret);

                    if(ret!=0)
                        tret = ret;
                }

            }

            return tret;
        }

        /*----------------------------------------------------------------------------------------------------------
         *
         * Function: ms_sdmmc_resume
         *     @author jeremy.wang (2011/5/18)
         * Desc:   Resume MMC host
         *
         * @param p_dev :   platform device structure
         * @return int  :   Error Status; Return 0 if no error.
         ----------------------------------------------------------------------------------------------------------*/
        static int ms_sdmmc_resume(struct platform_device *p_dev)
        {
            struct ms_sdmmc_host *p_sdmmc_host = platform_get_drvdata(p_dev);
            struct mmc_host      *p_mmc_host;
            unsigned int slotNo = 0;
            int ret = 0, tret = 0;

            for(slotNo=0; slotNo<gu8_SlotNums; slotNo++)
            {
                p_mmc_host = p_sdmmc_host->sdmmc_slot[slotNo]->mmc;
                if (p_mmc_host)
                {
                    ret = mmc_resume_host(p_mmc_host);
                    pr_sd_dbg(">> [sdmmc_%u] Resume host...(Ret:%u) \n", slotNo, ret);
                    if(ret!=0)
                        tret = ret;
                }
            }

            return tret;
        }

        #else

        static int ms_sdmmc_suspend(struct platform_device *p_dev, pm_message_t state)
        {
            int ret = 0;
            return ret;
        }

        static int ms_sdmmc_resume(struct platform_device *p_dev)
        {
            int ret = 0;
            return ret;
        }

        #endif

     #else  // !CONFIG_PM

        //Current driver does not support following two functions, therefore set them to NULL.
        #define ms_sdmmc_suspend        NULL
        #define ms_sdmmc_resume         NULL

     #endif  // End of CONFIG_PM



#endif


/**********************************************************************************************************
 * Define Static Global Structs
 **********************************************************************************************************/

#if defined(CONFIG_OF)
/*----------------------------------------------------------------------------------------------------------
 *  ms_sdmmc_of_match_table
 ----------------------------------------------------------------------------------------------------------*/
static const struct of_device_id ms_sdmmc_of_match_table[] = {
    { .compatible = "sstar,sdmmc" },
    {}
};


/*----------------------------------------------------------------------------------------------------------
 *  ms_sdmmc_dev_pm_ops
 ----------------------------------------------------------------------------------------------------------*/
static struct dev_pm_ops ms_sdmmc_dev_pm_ops = {
	.suspend	= ms_sdmmc_devpm_suspend,
	.resume		= ms_sdmmc_devpm_resume,
	.prepare	= ms_sdmmc_devpm_prepare,
	.complete	= ms_sdmmc_devpm_complete,
	.runtime_suspend = ms_sdmmc_devpm_runtime_suspend,
	.runtime_resume = ms_sdmmc_devpm_runtime_resume,
};

#else

/*----------------------------------------------------------------------------------------------------------
 *  st_ms_sdmmc_device
 ----------------------------------------------------------------------------------------------------------*/
static u64 mmc_dmamask = 0xffffffffUL;
static struct platform_device ms_sdmmc_pltdev =
{
    .name = DRIVER_NAME,
    .id = 0,
    .dev =
    {
        .dma_mask = &mmc_dmamask,
        .coherent_dma_mask = 0xffffffffUL,
    },
};

#endif //End of (defined(CONFIG_OF))


/*----------------------------------------------------------------------------------------------------------
 *  st_ms_sdmmc_driver
 ----------------------------------------------------------------------------------------------------------*/
static struct platform_driver ms_sdmmc_pltdrv =
{
    .remove  = ms_sdmmc_remove,/*__exit_p(ms_sdmmc_remove)*/
    .suspend = ms_sdmmc_suspend,
    .resume  = ms_sdmmc_resume,
    .probe   = ms_sdmmc_probe,
    .driver  =
    {
        .name  = DRIVER_NAME,
        .owner = THIS_MODULE,

#if defined(CONFIG_OF)
        .of_match_table = of_match_ptr(ms_sdmmc_of_match_table),
        .pm = &ms_sdmmc_dev_pm_ops,
#endif

    },
};


/**********************************************************************************************************
 * Init & Exit Modules
 **********************************************************************************************************/

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_mci_init
 *     @author jeremy.wang (2011/7/18)
 * Desc: Linux Module Function for Init
 *
 * @return s32 __init  :  Error Status; Return 0 if no error.
 ----------------------------------------------------------------------------------------------------------*/
static s32 ms_sdmmc_init(void)
{
    pr_sd_dbg(KERN_INFO ">> [sdmmc] %s Driver Initializing... \n", DRIVER_NAME);

#if !(defined(CONFIG_OF))
    platform_device_register(&ms_sdmmc_pltdev);
#endif

    return platform_driver_register(&ms_sdmmc_pltdrv);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: ms_sdmmc_exit
 *     @author jeremy.wang (2011/9/8)
 * Desc: Linux Module Function for Exit
 ----------------------------------------------------------------------------------------------------------*/
static void ms_sdmmc_exit(void)
{
    platform_driver_unregister(&ms_sdmmc_pltdrv);
}



module_init(ms_sdmmc_init);
module_exit(ms_sdmmc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("SSTAR");
