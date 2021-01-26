/*
* ms_sdmmc_ub.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: truman.yang <truman.yang@sigmastar.com.tw>
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

/***************************************************************************************************************
 *
 * FileName ms_sdmmc_ub.c
 *     @author jeremy.wang (2013/07/26)
 * Desc:
 *     This layer is between UBOOT SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any UBOOT SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc_ub.h
 *
 ***************************************************************************************************************/
#include "inc/ms_sdmmc_ub.h"
#include "inc/hal_card_platform.h"

//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__04)
//###########################################################################################################
#include "inc/hal_sdmmc.h"
//###########################################################################################################
#elif (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
#include "inc/hal_sdmmc_v5.h"
//###########################################################################################################
#endif

#include <malloc.h>
#include <mmc.h>
#include <errno.h>
#include <version.h>

//***********************************************************************************************************
// Config Setting (Internel)
//***********************************************************************************************************
#define EN_SDMMC_TRFUNC          (FALSE)

/****** For Allocation buffer *******/
#define MAX_BLK_COUNT             1024      //Maximum Transfer Block Count

//***********************************************************************************************************

// Multiple SD Cards Setting for Each Slot
//-----------------------------------------------------------------------------------------------------------
static IPEmType ge_IPSlot[3]            = {D_SDMMC1_IP, D_SDMMC2_IP, D_SDMMC3_IP};
static PortEmType ge_PORTSlot[3]        = {D_SDMMC1_PORT, D_SDMMC2_PORT, D_SDMMC3_PORT};
static PADEmType  ge_PADSlot[3]         = {D_SDMMC1_PAD, D_SDMMC2_PAD, D_SDMMC3_PAD};
static U32_T  gu32_MaxClkSlot[3]        = {V_SDMMC1_MAX_CLK, V_SDMMC2_MAX_CLK, V_SDMMC3_MAX_CLK};


// Global Variable
//-----------------------------------------------------------------------------------------------------------
static unsigned int   gu32_DevSlotDev[10] = {0};


// String Name
//-----------------------------------------------------------------------------------------------------------
#define DRIVER_NAME   "MStar SD/MMC"

// Trace Funcion
//-----------------------------------------------------------------------------------------------------------
#if (EN_SDMMC_TRFUNC)
    #define pr_sd_err(fmt, arg...)   //
    #define pr_sd_main(fmt, arg...)  printf(fmt, ##arg)
    #define pr_sd_dbg(fmt, arg...)   //printk(fmt, ##arg)
#else
    #define pr_sd_err(fmt, arg...)   printf(fmt, ##arg)
    #define pr_sd_main(fmt, arg...)  //
    #define pr_sd_dbg(fmt, arg...)   //
#endif


// UBoot Version
//-----------------------------------------------------------------------------------------------------------
#define  UBOOT_VERSION(y,m) ( (y*100) + (m) )


//------------------------------------------------------------------------------------------------
static uint _TransArrToUInt(U8_T u8Sep1, U8_T u8Sep2, U8_T u8Sep3, U8_T u8Sep4)
{
    return ((((uint)u8Sep1)<<24) | (((uint)u8Sep2)<<16) | (((uint)u8Sep3)<<8) | ((uint)u8Sep4));
}


//------------------------------------------------------------------------------------------------
static U8_T _CardDetect(SlotEmType eSlot)
{
    return Hal_CARD_GetGPIOState((GPIOEmType)eSlot);
}


//------------------------------------------------------------------------------------------------
static SDMMCRspEmType _TransRspType(unsigned int u32Rsp)
{

    switch(u32Rsp)
    {
        case MMC_RSP_NONE:
            return EV_NO;
        case MMC_RSP_R1:  //MMC_RSP_R5, MMC_RSP_R6, MMC_RSP_R7
            return EV_R1;
        case MMC_RSP_R1b:
            return EV_R1B;
        case MMC_RSP_R2:
            return EV_R2;
        case MMC_RSP_R3: //MMC_RSP_R4
            return EV_R3;
    }

    return EV_R1;
}


//------------------------------------------------------------------------------------------------
static int _RequestErrProcess(RspErrEmType eErrType)
{
    ErrGrpEmType eErrGrp;
    int nErr = 0;


    if( eErrType == EV_STS_OK )
    {
        pr_sd_main("@\n");
        return nErr;
    }
    else
    {
        pr_sd_main("=> (Err: 0x%04X)\n", (U16_T)eErrType);

        nErr = (U32_T) eErrType;

        eErrGrp = Hal_SDMMC_ErrGroup(eErrType);

        switch((U16_T)eErrGrp)
        {
            case EV_EGRP_TOUT:
                nErr =  TIMEOUT;
                break;

            case EV_EGRP_COMM:
                nErr = COMM_ERR;
                break;
        }

    }

    return nErr;

}

//------------------------------------------------------------------------------------------------
static void _SetPower(SlotEmType eSlot, PowerEmType ePower)
{
    IPEmType eIP      = ge_IPSlot[eSlot];
    PADEmType ePAD    = ge_PADSlot[eSlot];

    if(ePower == EV_POWER_OFF) // Power Off
    {
        Hal_SDMMC_ClkCtrl(eIP, FALSE, 0);
        Hal_CARD_PullPADPin(ePAD, EV_PULLDOWN, FALSE);
        Hal_CARD_PowerOff(ePAD, WT_POWEROFF); //For SD PAD

    }
    else if(ePower == EV_POWER_UP)  // Power Up
    {
        Hal_CARD_PullPADPin(ePAD, EV_PULLUP, FALSE);
        Hal_CARD_PowerOn(ePAD, WT_POWERUP);
    }
    else if(ePower == EV_POWER_ON) // Power On
    {
        Hal_SDMMC_ClkCtrl(eIP, TRUE, WT_POWERON);
        Hal_SDMMC_Reset(eIP);

    }
}

//------------------------------------------------------------------------------------------------
static U32_T _SetClock(SlotEmType eSlot, unsigned int u32ReffClk, U8_T u8PassLevel, U8_T u8DownLevel)
{
    U32_T u32RealClk =0 ;
    IPEmType eIP = ge_IPSlot[eSlot];

    if(u32ReffClk)
    {
        u32RealClk = Hal_CARD_FindClockSetting(eIP, (U32_T)u32ReffClk, u8PassLevel, u8DownLevel);
        Hal_CARD_SetClock(eIP, u32RealClk);
        Hal_SDMMC_SetNrcDelay(eIP, u32RealClk);

    }

    return u32RealClk;

}

//------------------------------------------------------------------------------------------------
static void _SwitchPAD(SlotEmType eSlot)
{
    IPEmType eIP   = ge_IPSlot[eSlot];
    PortEmType ePort  = ge_PORTSlot[eSlot];
    PADEmType ePAD    = ge_PADSlot[eSlot];

    Hal_CARD_IPOnceSetting(eIP);
    Hal_CARD_SetPADToPortPath(eIP, ePort, ePAD, FALSE);
    Hal_CARD_InitPADPin(ePAD, FALSE);
    Hal_CARD_InitGPIO((GPIOEmType) eSlot, TRUE);

    Hal_CARD_IPBeginSetting(eIP);

    //Because UBoot ony run once, so put here
    Hal_CARD_IPOnceSetting(eIP);

}

//###########################################################################################################
#if (D_PROJECT == D_PROJECT__iNF) || (D_PROJECT == D_PROJECT__iNF3) || (D_PROJECT == D_PROJECT__iNF5) || (D_PROJECT == D_PROJECT__iNF6E)
//###########################################################################################################
//------------------------------------------------------------------------------------------------
static void _SetPAD(SlotEmType eSlot, IPEmType eIP, PortEmType ePort, PADEmType ePAD)
{
    ge_IPSlot[eSlot] = eIP;
    ge_PORTSlot[eSlot] = ePort;
    ge_PADSlot[eSlot] = ePAD;

}
//###########################################################################################################
#endif


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: mmc_send_cmd
 *     @author jeremy.wang (2013/7/19)
 * Desc: Request funciton for any commmand
 *
 * @param mmc :  mmc structure pointer
 * @param cmd :  mmc_cmd structure pointer
 * @param data : mmc_data structure pointer
 *
 * @return int  : 0 (pass), else (fail)
 ----------------------------------------------------------------------------------------------------------*/
static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{

    RspStruct * eRspSt;
    RspErrEmType eErr = EV_STS_OK;
    TransEmType eTransType = EV_DMA;
    CmdEmType eCmdType = EV_CMDRSP;
    SlotEmType eSlot = (SlotEmType)gu32_DevSlotDev[mmc->block_dev.dev];
    IPEmType eIP   = ge_IPSlot[eSlot];

    BOOL_T bCloseClock = FALSE;
    U8_T u8CMD = 0;
    U16_T u16BlkSize = 0, u16BlkCnt = 0;
    U32_T u32Arg = 0;

    volatile U8_T *pu8Buf;
    int nErr = 0;

    u8CMD = (U8_T)cmd->cmdidx;
    u32Arg = (U32_T)cmd->cmdarg;

    pr_sd_main("_[sdmmc_%u] CMD_%u (0x%08X)", eSlot, u8CMD, u32Arg);

    Hal_SDMMC_SetCmdToken(eIP, u8CMD, u32Arg);

    /****** Simple SD command *******/
    if(!data)
        eErr = Hal_SDMMC_SendCmdAndWaitProcess(eIP, EV_EMP, EV_CMDRSP, _TransRspType(cmd->resp_type), TRUE);
    else  //SD R/W Command and DATA
    {
        u16BlkSize = (U16_T)data->blocksize;
        u16BlkCnt = (U16_T)data->blocks;

        pr_sd_main("__(TB: %u)(BSz: %u)", u16BlkCnt, u16BlkSize);

        if(data->flags & MMC_DATA_READ)
        {
            eCmdType = EV_CMDREAD;
            pu8Buf = (volatile U8_T *)data->dest;
        }
        else
        {
            eCmdType = EV_CMDWRITE;
            pu8Buf = (volatile U8_T *)data->src;
        }

        if(u16BlkCnt>1)
            bCloseClock = FALSE;
        else
            bCloseClock = TRUE;

        Hal_SDMMC_TransCmdSetting(eIP, eTransType, u16BlkCnt, u16BlkSize, Hal_CARD_TransMIUAddr((U32_T)pu8Buf), pu8Buf);
        eErr = Hal_SDMMC_SendCmdAndWaitProcess(eIP, eTransType, eCmdType, _TransRspType(cmd->resp_type), bCloseClock);

    }

    nErr = _RequestErrProcess(eErr);

    eRspSt = Hal_SDMMC_GetRspToken(eIP);

    cmd->response[0] = _TransArrToUInt(eRspSt->u8ArrRspToken[1], eRspSt->u8ArrRspToken[2], eRspSt->u8ArrRspToken[3], eRspSt->u8ArrRspToken[4]);
    cmd->response[1] = _TransArrToUInt(eRspSt->u8ArrRspToken[5], eRspSt->u8ArrRspToken[6], eRspSt->u8ArrRspToken[7], eRspSt->u8ArrRspToken[8]);
    cmd->response[2] = _TransArrToUInt(eRspSt->u8ArrRspToken[9], eRspSt->u8ArrRspToken[10], eRspSt->u8ArrRspToken[11], eRspSt->u8ArrRspToken[12]);
    cmd->response[3] = _TransArrToUInt(eRspSt->u8ArrRspToken[13], eRspSt->u8ArrRspToken[14], eRspSt->u8ArrRspToken[15], 0);


    if(!data && nErr)
    {
        if(nErr == COMM_ERR)
            pr_sd_err("_[sdmmc_%u] Warn: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)\n", eSlot, u8CMD, u32Arg, (U16_T)eErr, cmd->response[0], eRspSt->u32ErrLine);
        else
            pr_sd_err("_[sdmmc_%u] Err: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)\n", eSlot, u8CMD, u32Arg, (U16_T)eErr, cmd->response[0], eRspSt->u32ErrLine);
    }
    else if(data && nErr)
    {
        if(nErr == COMM_ERR)
            pr_sd_err(">> [sdmmc_%u] Warn: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)(B:%u)\n", \
                  eSlot, u8CMD, u32Arg, (U16_T)eErr, cmd->response[0], eRspSt->u32ErrLine, u16BlkCnt);
        else
            pr_sd_err(">> [sdmmc_%u] Err: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)(B:%u)\n", \
                  eSlot, u8CMD, u32Arg, (U16_T)eErr, cmd->response[0], eRspSt->u32ErrLine, u16BlkCnt);

    }

    return nErr;
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: mmc_set_ios
 *     @author jeremy.wang (2013/7/19)
 * Desc: Set IO bus Behavior
 *
 * @param mmc : mmc structure pointer
 ----------------------------------------------------------------------------------------------------------*/
static void mmc_set_ios(struct mmc *mmc)
{
    unsigned int u32RealClk = 0;
    SlotEmType eSlot = (SlotEmType)gu32_DevSlotDev[mmc->block_dev.dev];
    IPEmType eIP = ge_IPSlot[eSlot];

    pr_sd_main("_[sdmmc_%u] mmc_set_ios =>", (U32_T)eSlot);

    //Set up Clock
    u32RealClk = _SetClock(eSlot, mmc->clock, 0, 0);

    pr_sd_main("RealClk=%u ", u32RealClk);

    if(mmc->card_caps & MMC_MODE_HS)
    {
        Hal_SDMMC_SetBusTiming(eIP, EV_BUS_HS);
        Hal_CARD_SetBusTiming(eIP, EV_BUS_HS);
        pr_sd_main("[HS] ");
    }
    else
    {
        if(u32RealClk<=400000)
        {
            Hal_SDMMC_SetBusTiming(eIP, EV_BUS_LOW);
            Hal_CARD_SetBusTiming(eIP, EV_BUS_LOW);
            pr_sd_main("[LS] ");
        }
        else
        {
            Hal_SDMMC_SetBusTiming(eIP, EV_BUS_DEF);
            Hal_CARD_SetBusTiming(eIP, EV_BUS_DEF);
            pr_sd_main("[DS] ");
        }
    }

    //Set up DataWidth
    if (mmc->bus_width == 1)
        Hal_SDMMC_SetDataWidth(eIP, EV_BUS_1BIT);
    else if(mmc->bus_width == 4)
        Hal_SDMMC_SetDataWidth(eIP, EV_BUS_4BITS);
    else if(mmc->bus_width == 8)
        Hal_SDMMC_SetDataWidth(eIP, EV_BUS_8BITS);

    pr_sd_main(", BusWidth=%u\n", mmc->bus_width);

}

#include <asm/arch/mach/platform.h>
extern DEVINFO_BOOT_TYPE ms_devinfo_boot_type(void);
/*----------------------------------------------------------------------------------------------------------
 *
 * Function: mmc_core_init
 *     @author jeremy.wang (2013/7/19)
 * Desc: Init Slot Setting
 *
 * @param mmc : mmc structure pointer
 *
 * @return int  : 0 (pass), else (fail
 ----------------------------------------------------------------------------------------------------------*/
static int mmc_core_init(struct mmc *mmc)
{
    SlotEmType eSlot = (SlotEmType)gu32_DevSlotDev[mmc->block_dev.dev];
    pr_sd_main("\n_[sdmmc_%u] mmc_core_init\n", (U32_T)eSlot);


//###########################################################################################################
#if (D_PROJECT == D_PROJECT__iNF)
//###########################################################################################################
    if(1)
    {
        pr_sd_main("_[sdmmc_%u] [sdio ip]\n", (U32_T)eSlot);

        _SetPAD(eSlot, EV_IP_FCIE1, EV_PFCIE5_SDIO, EV_PAD1);
    }
    else
    {
        pr_sd_main("_[sdmmc_%u] [fcie ip]\n", (U32_T)eSlot);
        _SetPAD(eSlot, EV_IP_FCIE2, EV_PFCIE5_FCIE, EV_PAD2);
    }
//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF3)
//###########################################################################################################
    if(1)
    {
        pr_sd_main("_[sdmmc_%u] [sdio ip]\n", (U32_T)eSlot);
        _SetPAD(eSlot, EV_IP_FCIE1, EV_PFCIE5_SDIO, EV_PAD1);
    }
    else
    {
        pr_sd_main("_[sdmmc_%u] [fcie ip]\n", (U32_T)eSlot);
        _SetPAD(eSlot, EV_IP_FCIE2, EV_PFCIE5_FCIE, EV_PAD2);
    }
//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF5)
//###########################################################################################################
    if(1)
    {
        pr_sd_main("_[sdmmc_%u] [sdio ip]\n", (U32_T)eSlot);
        _SetPAD(eSlot, EV_IP_FCIE1, EV_PFCIE5_SDIO, EV_PAD1);
    }
    else
    {
        pr_sd_main("_[sdmmc_%u] [fcie ip]\n", (U32_T)eSlot);
        _SetPAD(eSlot, EV_IP_FCIE2, EV_PFCIE5_FCIE, EV_PAD2);
    }
//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF6E)
//###########################################################################################################
    if(1)
    {
        pr_sd_main("_[sdmmc_%u] [sdio ip]\n", (U32_T)eSlot);
        _SetPAD(eSlot, EV_IP_FCIE1, EV_PFCIE5_SDIO, EV_PAD1);
    }
    else
    {
        pr_sd_main("_[sdmmc_%u] [fcie ip]\n", (U32_T)eSlot);
        _SetPAD(eSlot, EV_IP_FCIE2, EV_PFCIE5_FCIE, EV_PAD2);
    }
#endif

    _SwitchPAD(eSlot);

    _SetPower(eSlot, EV_POWER_OFF);

    if(!_CardDetect(eSlot))
    {
        printf("_[sdmmc_%u] Card Detect Fail! \n", (U32_T)eSlot);
        return NO_CARD_ERR;
    }

    _SetPower(eSlot, EV_POWER_UP);
    _SetPower(eSlot, EV_POWER_ON);


    return 0;
}

#if (U_BOOT_VERSION_CODE >= UBOOT_VERSION(2015, 1))

static const struct mmc_ops ms_mmc_ops = {
    .send_cmd   = mmc_send_cmd,
    .set_ios    = mmc_set_ios,
    .init       = mmc_core_init,
    .getcd      = NULL
};

#endif

static int ms_sdmmc_init_slot(unsigned int slotNo)
{
    SlotEmType eSlot = (SlotEmType)slotNo;
    IPEmType eIP     = ge_IPSlot[eSlot];

#if (U_BOOT_VERSION_CODE >= UBOOT_VERSION(2015, 1))

    struct mmc_config *mmc_cfg;

    mmc_cfg=malloc(sizeof(struct mmc_config));
    if(!mmc_cfg)
        return -ENOMEM;

    mmc_cfg->ops = &ms_mmc_ops;

    mmc_cfg->name=DRIVER_NAME;
    mmc_cfg->host_caps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;
    mmc_cfg->voltages = MMC_VDD_32_33| MMC_VDD_31_32| MMC_VDD_30_31| MMC_VDD_29_30 | MMC_VDD_28_29 | MMC_VDD_27_28;
    mmc_cfg->f_min = Hal_CARD_FindClockSetting(eIP, 400000, 0, 0);
    mmc_cfg->f_max = gu32_MaxClkSlot[eSlot];
    mmc_cfg->part_type=PART_TYPE_DOS;
    mmc_cfg->b_max = MAX_BLK_COUNT;

    mmc_create(mmc_cfg, NULL);

#else
    struct mmc *mmc;

    mmc = malloc(sizeof(struct mmc));
    if (!mmc)
        return -ENOMEM;

    mmc->send_cmd = mmc_send_cmd;
    mmc->set_ios = mmc_set_ios;
    mmc->init = mmc_core_init;
    mmc->getcd = NULL;

    sprintf((void *)mmc->name, DRIVER_NAME);
    mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;
    mmc->voltages = MMC_VDD_32_33| MMC_VDD_31_32| MMC_VDD_30_31| MMC_VDD_29_30 | MMC_VDD_28_29 | MMC_VDD_27_28;
    mmc->f_min = Hal_CARD_FindClockSetting(eIP, 400000, 0, 0);
    mmc->f_max = gu32_MaxClkSlot[eSlot];
    mmc->block_dev.part_type = PART_TYPE_DOS;
    mmc->b_max = MAX_BLK_COUNT;

    mmc_register(mmc);

#endif

    gu32_DevSlotDev[get_mmc_num()]= slotNo;

    return 0;


}


static int ms_sdmmc_probe(bd_t *bis)
{
    unsigned int slotNo = 0;
    int ret = 0, tret = 0;

    for(slotNo=0; slotNo<V_SDMMC_CARDNUMS; slotNo++)
    {

        ret = ms_sdmmc_init_slot(slotNo);
        if(ret!=0)
            tret = ret;
    }
    return tret;

}



int board_mmc_init(bd_t *bis)
{
    return ms_sdmmc_probe(bis);
}
