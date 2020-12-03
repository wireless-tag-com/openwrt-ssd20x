/*
* ms_sdmmc_lnx.h- Sigmastar
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
 * FileName ms_sdmmc_lnx.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This file is the header file of ms_sdmmc_lnx.c.
 *
 ***************************************************************************************************************/

#ifndef __MS_SDMMC_LNX_H
#define __MS_SDMMC_LNX_H

#include <linux/cdev.h>
#include <linux/interrupt.h>
#include "hal_card_base.h"

//***********************************************************************************************************
// Config Setting (Externel)
//***********************************************************************************************************
#include "hal_card_platform_config.h"



//***********************************************************************************************************
//***********************************************************************************************************
typedef enum
{
    EV_SDMMC1 = 0,
    EV_SDMMC2 = 1,
    EV_SDMMC3 = 2,

} SlotEmType;

typedef enum
{
    EV_MUTEX1  = 0,
    EV_MUTEX2  = 1,
    EV_MUTEX3  = 2,
    EV_NOMUTEX = 3,

} MutexEmType;

struct ms_sdmmc_host
{
    struct platform_device  *pdev;
    struct ms_sdmmc_slot *sdmmc_slot[3];
};

struct ms_sdmmc_slot
{
    struct mmc_host     *mmc;

    unsigned int    slotNo;         //Slot No.
    unsigned int    mieIRQNo;       //MIE IRQ No.
    unsigned int    cdzIRQNo;       //CDZ IRQ No.
    unsigned int    pwrGPIONo;      //PWR GPIO No.
    unsigned int    pmrsaveClk;     //Power Saving Clock

    unsigned int    initFlag;       //First Time Init Flag
    unsigned int    sdioFlag;       //SDIO Device Flag

    unsigned int    currClk;        //Current Clock
    unsigned int    currRealClk;    //Current Real Clock
    unsigned char   currWidth;      //Current Bus Width
    unsigned char   currTiming;     //Current Bus Timning
    unsigned char   currPowrMode;   //Current PowerMode
    unsigned char   currBusMode;    //Current Bus Mode
    unsigned short  currVdd;        //Current Vdd
    unsigned char   currDDR;        //Current DDR
    unsigned char   currTimeoutCnt; //Current Timeout Count

    int read_only;                  //WP
    int card_det;                   //Card Detect

    /****** DMA buffer used for transmitting *******/
    u32 *dma_buffer;
    dma_addr_t dma_phy_addr;

    /****** ADMA buffer used for transmitting *******/
    u32 *adma_buffer;
    dma_addr_t adma_phy_addr;

    /***** Tasklet for hotplug ******/
    struct tasklet_struct   hotplug_tasklet;

};  /* struct ms_sdmmc_hot*/


#endif // End of __MS_SDMMC_LNX_H

