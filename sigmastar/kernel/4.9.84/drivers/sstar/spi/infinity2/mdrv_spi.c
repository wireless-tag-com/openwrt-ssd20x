/*
* mdrv_spi.c- Sigmastar
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
/*
 * Driver for Broadcom BCM2835 SPI Controllers
 *
 * Copyright (C) 2012 Chris Boot
 * Copyright (C) 2013 Stephen Warren
 *
 * This driver is inspired by:
 * spi-ath79.c, Copyright (C) 2009-2011 Gabor Juhos <juhosg@openwrt.org>
 * spi-atmel.c, Copyright (C) 2006 Atmel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/spi/spi.h>
#include <linux/of_address.h>

//#include "ms_platform.h"

#include "mdrv_spi.h"

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
#define U8 u8
#define U16 u16
#define U32 u32
#define BOOL bool
#define TRUE true
#define FALSE false

#define MSPI_DBG 1
#if MSPI_DBG
    #define mspi_dbg(args...) printk(args)
#else
    #define mspi_dbg(args...)
#endif

struct mstar_spi_dev {
    int  spinum;
	int  padmode;
	int  irq;
	void __iomem *spibase;
    void __iomem *chipbase;
    void __iomem *clkbase;
	const u8 *tx_buf;
	u8 *rx_buf;
	int len;
	struct completion done;
};

static struct mutex	 hal_mspi_lock;

#define CLK_NUM   4
#define DIV_NUM   8
#define CFG_NUM   32
static U16 spi_clk[CLK_NUM] = {108, 54, 24, 12};  //bank 100b h16
static U16 spi_div[DIV_NUM] = {2, 4, 8, 16, 32, 64, 128, 256};
static mspi_clk_cfg clk_cfg[CFG_NUM];

void _mspi_clk_init(void)
{
	U8 i = 0;
    U8 j = 0;
    mspi_clk_cfg cfg;

    memset(&cfg,0,sizeof(mspi_clk_cfg));
    memset(&clk_cfg,0,sizeof(mspi_clk_cfg)*CFG_NUM);

    for(i = 0; i < CLK_NUM; i++)
    {
        for(j = 0; j < DIV_NUM; j++)
        {
            clk_cfg[i*DIV_NUM+j].spi_clk = i;
            clk_cfg[i*DIV_NUM+j].spi_div = j ;
            clk_cfg[i*DIV_NUM+j].spi_speed = spi_clk[i]*1000000/spi_div[j];
        }
    }

    for(i = 0; i < CFG_NUM; i++)
    {
        for(j = i; j < CFG_NUM; j++)
        {
            if(clk_cfg[i].spi_speed > clk_cfg[j].spi_speed)
            {
                memcpy(&cfg, &clk_cfg[i], sizeof(mspi_clk_cfg));
                memcpy(&clk_cfg[i], &clk_cfg[j], sizeof(mspi_clk_cfg));
                memcpy(&clk_cfg[j], &cfg, sizeof(mspi_clk_cfg));
            }
        }
    }
}

U16 HAL_MSPI_GetDoneFlag(struct mstar_spi_dev *mspi_dev)
{
	return READ_WORD(mspi_dev->spibase, REG_MSPI_DONE_FLAG);
}

void HAL_MSPI_ClearDoneFlag(struct mstar_spi_dev *mspi_dev)
{
    WRITE_WORD(mspi_dev->spibase, REG_MSPI_CLEAR_DONE_FLAG, MSPI_CLEAR_DONE);
}

void HAL_MSPI_SetSpiEnable(struct mstar_spi_dev *mspi_dev, BOOL enable)
{
	U16 u16regval = 0;

	mutex_lock(&hal_mspi_lock);
	u16regval = READ_WORD(mspi_dev->spibase, REG_MSPI_CTRL);
    if(enable)
    {
        u16regval |= (MSPI_INT_EN_BIT|MSPI_RESET_BIT | MSPI_ENABLE_BIT);
    }
    else
    {
        u16regval &= (~(MSPI_INT_EN_BIT | MSPI_ENABLE_BIT));
    }
    WRITE_WORD(mspi_dev->spibase, REG_MSPI_CTRL, u16regval);
	mutex_unlock(&hal_mspi_lock);
}

void HAL_MSPI_SetPadMode(struct mstar_spi_dev *mspi_dev)
{
	U16 u16regval = 0;

	mutex_lock(&hal_mspi_lock);

	if(mspi_dev->spinum == E_MSPI0)
	{
		u16regval = READ_WORD(mspi_dev->chipbase, REG_MSPI0_MODE);
		u16regval = u16regval & (~MSPI_MSPI0_MODE_MASK);
	    u16regval = u16regval | (mspi_dev->padmode<< MSPI_MSPI0_MODE_SHIFT);
		WRITE_WORD(mspi_dev->chipbase, REG_MSPI0_MODE, u16regval);
	}
	else if(mspi_dev->spinum == E_MSPI1)
	{
		u16regval = READ_WORD(mspi_dev->chipbase, REG_MSPI1_MODE);
		u16regval = u16regval & (~MSPI_MSPI1_MODE_MASK);
	    u16regval = u16regval | (mspi_dev->padmode<< MSPI_MSPI1_MODE_SHIFT);
		WRITE_WORD(mspi_dev->chipbase, REG_MSPI1_MODE, u16regval);
	}

	mutex_unlock(&hal_mspi_lock);
}

void HAL_MSPI_SetSpiMode(struct mstar_spi_dev *mspi_dev, MSPI_MODE mode)
{
    U16 u16regval = 0;

	if (mode >= E_MSPI_MODE_MAX)
	{
        return;
    }

    mutex_lock(&hal_mspi_lock);
    u16regval = READ_WORD(mspi_dev->spibase, REG_MSPI_CTRL);
	u16regval = u16regval & (~MSPI_MODE_MASK);
	u16regval = u16regval | (mode << MSPI_MODE_SHIFT);
    WRITE_WORD(mspi_dev->spibase, REG_MSPI_CTRL, u16regval);
    mutex_unlock(&hal_mspi_lock);
}

void HAL_MSPI_SetLsb(struct mstar_spi_dev *mspi_dev, BOOL enable)
{
    mutex_lock(&hal_mspi_lock);
    WRITE_WORD(mspi_dev->spibase, REG_MSPI_LSB_FIRST, enable);
    mutex_unlock(&hal_mspi_lock);
}

void HAL_MSPI_SetClk(struct mstar_spi_dev *mspi_dev, U8 clk)
{
	U16 u16regval = 0;

	mutex_lock(&hal_mspi_lock);
	u16regval = READ_WORD(mspi_dev->clkbase, REG_MSPI_CKGEN);
    if(mspi_dev->spinum == E_MSPI0)//mspi0
    {
        u16regval &= (~MSPI_CKG_MSPI0_MASK);
        u16regval |= (clk << MSPI_CKG_MSPI0_SHIFT);
    }
    else if(mspi_dev->spinum == E_MSPI1)//mspi1
    {
        u16regval &= (~MSPI_CKG_MSPI1_MASK);
        u16regval |= (clk << MSPI_CKG_MSPI1_SHIFT);
    }
	WRITE_WORD(mspi_dev->clkbase, REG_MSPI_CKGEN, u16regval);
	mutex_unlock(&hal_mspi_lock);
}

void HAL_MSPI_SetClkDiv(struct mstar_spi_dev *mspi_dev, U8 div)
{
	U16 u16regval = 0;

	mutex_lock(&hal_mspi_lock);
	u16regval = READ_WORD(mspi_dev->spibase, REG_MSPI_CTRL);
    u16regval &= MSPI_CLK_DIV_MASK;
    u16regval |= div << MSPI_CLK_DIV_SHIFT;
    WRITE_WORD(mspi_dev->spibase, REG_MSPI_CTRL, u16regval);
	mutex_unlock(&hal_mspi_lock);
}

void HAL_MSPI_ChipSelect(struct mstar_spi_dev *mspi_dev, BOOL enable ,U8 select)
{
    U16 u16regval = 0;

	mutex_lock(&hal_mspi_lock);
    u16regval = READ_WORD(mspi_dev->spibase, REG_MSPI_CHIP_SELECT);
    if(enable)
	{
        u16regval = (~(1 << select));
    }
	else
    {
        u16regval = (1 << select);
    }
    WRITE_WORD(mspi_dev->spibase, REG_MSPI_CHIP_SELECT, u16regval);
	mutex_unlock(&hal_mspi_lock);
}

void HAL_MSPI_SetBufSize(struct mstar_spi_dev *mspi_dev, MSPI_RW_DIR direct, U8 size)
{
    U16 u16regval = 0;

	mutex_lock(&hal_mspi_lock);
    u16regval = READ_WORD(mspi_dev->spibase, REG_MSPI_RW_BUF_SIZE);
    if(direct == E_MSPI_READ)
    {
        u16regval &= MSPI_RW_BUF_MASK;
        u16regval |= size << MSPI_READ_BUF_SHIFT;
    }
    else
    {
        u16regval &= (~MSPI_RW_BUF_MASK);
        u16regval |= size;
    }
	WRITE_WORD(mspi_dev->spibase, REG_MSPI_RW_BUF_SIZE, u16regval);
	mutex_unlock(&hal_mspi_lock);
}

BOOL HAL_MSPI_Trigger(struct mstar_spi_dev *mspi_dev)
{
	unsigned int timeout = 0;

	reinit_completion(&mspi_dev->done);

    WRITE_WORD(mspi_dev->spibase, REG_MSPI_TRIGGER, MSPI_TRIGGER);

	timeout = wait_for_completion_timeout(&mspi_dev->done, msecs_to_jiffies(MSTAR_SPI_TIMEOUT_MS));

    WRITE_WORD(mspi_dev->spibase, REG_MSPI_RW_BUF_SIZE, 0x0);

    if (!timeout)
	{
        mspi_dbg("%s:%d timeout\n",__func__, __LINE__);
		return FALSE;
    }

	return TRUE;
}

BOOL HAL_MSPI_Write(struct mstar_spi_dev *mspi_dev , U8 *pdata, U16 u16size)
{
    U16  i = 0;
    U16 *u16val = (U16*)pdata;

    u16size = u16size < MSPI_MAX_RW_BUF_SIZE ? u16size : MSPI_MAX_RW_BUF_SIZE;

    mutex_lock(&hal_mspi_lock);

    for(i = 0; i < u16size/2; i++)
	{
        WRITE_WORD(mspi_dev->spibase, (REG_MSPI_WRITE_BUF + i), u16val[i]);
    }

	if(u16size%2)
	{
		WRITE_WORD(mspi_dev->spibase, (REG_MSPI_WRITE_BUF + u16size/2), pdata[u16size-1]);
	}

	mutex_unlock(&hal_mspi_lock);

    HAL_MSPI_SetBufSize(mspi_dev,E_MSPI_WRITE, u16size);

    return HAL_MSPI_Trigger(mspi_dev);

}

BOOL HAL_MSPI_Read(struct mstar_spi_dev *mspi_dev , U8 *pdata, U16 u16size)
{
    U16  i = 0;
    U16 u16val = 0;
	U16 *u16pdata = (U16*)pdata;

	u16size = u16size < MSPI_MAX_RW_BUF_SIZE ? u16size : MSPI_MAX_RW_BUF_SIZE;

	HAL_MSPI_SetBufSize(mspi_dev,E_MSPI_READ, u16size);

	if(!HAL_MSPI_Trigger(mspi_dev))
		return FALSE;

	mutex_lock(&hal_mspi_lock);
    for(i = 0; i < u16size/2; i++)
	{
        u16val = READ_WORD(mspi_dev->spibase, (REG_MSPI_READ_BUF+i));
		u16pdata[i] = u16val;
    }

	if(u16size%2)
	{
		u16val = READ_WORD(mspi_dev->spibase, (REG_MSPI_READ_BUF + u16size/2));
		pdata[u16size-1] = (u16val&0xFF);
	}

    mutex_unlock(&hal_mspi_lock);

    return TRUE;
}

void HAL_MSPI_SetTrTime(struct mstar_spi_dev *mspi_dev, MSPI_TIME_CFG cfg, U8 time)
{
    U16 u16regval = 0;

	if(time > MSPI_CFG_TIME_MAX)
		return;

    mutex_lock(&hal_mspi_lock);
	u16regval = READ_WORD(mspi_dev->spibase, REG_MSPI_TR_TIME);
	if(cfg == E_MSPI_TR_START_TIME)
	{
        u16regval &= (~MSPI_TIME_MASK);
        u16regval |= time;
	}
	else if(cfg == E_MSPI_TR_END_TIME)
	{
		u16regval &= MSPI_TIME_MASK;
        u16regval |= time << MSPI_TIME_SHIFT;
	}
    WRITE_WORD(mspi_dev->spibase, REG_MSPI_TR_TIME, u16regval);
    mutex_unlock(&hal_mspi_lock);
}


void HAL_MSPI_SetTbTime(struct mstar_spi_dev *mspi_dev, MSPI_TIME_CFG cfg, U8 time)
{
    U16 u16regval = 0;

	if(time > MSPI_CFG_TIME_MAX)
		return;

    mutex_lock(&hal_mspi_lock);
	u16regval = READ_WORD(mspi_dev->spibase, REG_MSPI_TB_TIME);
	if(cfg == E_MSPI_TB_INTERVAL_TIME)
	{
        u16regval &= (~MSPI_TIME_MASK);
        u16regval |= time;
	}
	else if(cfg == E_MSPI_RW_TURN_AROUND_TIME)
	{
		u16regval &= MSPI_TIME_MASK;
        u16regval |= time << MSPI_TIME_SHIFT;
	}
    WRITE_WORD(mspi_dev->spibase, REG_MSPI_TB_TIME, u16regval);
    mutex_unlock(&hal_mspi_lock);
}

void HAL_MSPI_SetFrameSize(struct mstar_spi_dev *mspi_dev, U16 regaddr, U8 offset, U8 size)
{
    U16 u16regval = 0;

    mutex_lock(&hal_mspi_lock);
    u16regval = READ_WORD(mspi_dev->spibase, regaddr);
    u16regval &= (~(MSPI_FRAME_SIZE_MASK << offset));
    u16regval |= (size&MSPI_FRAME_SIZE_MASK) << offset;
    WRITE_WORD(mspi_dev->spibase, regaddr, u16regval);
    mutex_unlock(&hal_mspi_lock);
}

void MDrv_MSPI_FrameSizeInit(struct mstar_spi_dev *mspi_dev)
{
	U8 i = 0;
	U16 regoff = 0;

    // read buffer bit config
    for(i = 0; i < MSPI_MAX_FRAME_BUF_NUM; i++)
	{
		regoff = (i/4) * 4;
        HAL_MSPI_SetFrameSize(mspi_dev, REG_MSPI_READ_FRAME_SIZE+regoff, MSPI_FRAME_FIELD_BITS*i, 0x07);
    }

    //write buffer bit config
    for(i = 0; i < MSPI_MAX_FRAME_BUF_NUM; i++)
	{
		regoff = (i/4) * 4;
        HAL_MSPI_SetFrameSize(mspi_dev, REG_MSPI_WRITE_FRAME_SIZE+regoff, MSPI_FRAME_FIELD_BITS*i, 0x07);
    }
}

U32 MDrv_MSPI_SetSpeed(struct mstar_spi_dev *mspi_dev, U32 speed)
{
    U8  i = 0;

    for(i = 0; i < CFG_NUM; i++)
    {
        if(speed <= clk_cfg[i].spi_speed)
        {
            break;
        }
    }

    //match Closer clk
    if((speed - clk_cfg[i-1].spi_speed) < (clk_cfg[i].spi_speed - speed))
    {
        i--;
    }

    mspi_dbg("%s:%d clk=%d,div=%d,speed=%d,spi_speed=%d\n",__func__, __LINE__,
		clk_cfg[i].spi_clk, clk_cfg[i].spi_div, speed, clk_cfg[i].spi_speed);

	HAL_MSPI_SetClk(mspi_dev, clk_cfg[i].spi_clk);
	HAL_MSPI_SetClkDiv(mspi_dev, clk_cfg[i].spi_div);

    return clk_cfg[i].spi_speed;
}

void MDrv_MSPI_Init(struct mstar_spi_dev *mspi_dev)
{
	mutex_init(&hal_mspi_lock);
    HAL_MSPI_SetTrTime(mspi_dev, E_MSPI_TR_START_TIME, 0);
	HAL_MSPI_SetTrTime(mspi_dev, E_MSPI_TR_END_TIME, 0);
	HAL_MSPI_SetTbTime(mspi_dev, E_MSPI_TB_INTERVAL_TIME, 0);
	HAL_MSPI_SetTbTime(mspi_dev, E_MSPI_RW_TURN_AROUND_TIME, 0);

	MDrv_MSPI_FrameSizeInit(mspi_dev);

	HAL_MSPI_SetSpiMode(mspi_dev, E_MSPI_MODE0);
	HAL_MSPI_SetLsb(mspi_dev, 0);
	HAL_MSPI_ChipSelect(mspi_dev, FALSE, 0);

	MDrv_MSPI_SetSpeed(mspi_dev, 54000000);

	HAL_MSPI_SetPadMode(mspi_dev);
    HAL_MSPI_SetSpiEnable(mspi_dev, 1);

}

BOOL MDrv_MSPI_Write(struct mstar_spi_dev *mspi_dev, U8 *pdata, U16 u16size)
{
    U8  i = 0;
    U8  frmcnt = 0;
	U8  lastfrmsize = 0;

    frmcnt = u16size/MSPI_MAX_RW_BUF_SIZE;
	lastfrmsize = u16size%MSPI_MAX_RW_BUF_SIZE;

    for (i = 0; i < frmcnt; i++)
    {
        if(!HAL_MSPI_Write(mspi_dev, pdata+i*MSPI_MAX_RW_BUF_SIZE, MSPI_MAX_RW_BUF_SIZE))
		{
            return FALSE;
        }
    }

    if(!lastfrmsize)
		return TRUE;

    return HAL_MSPI_Write(mspi_dev, pdata+frmcnt*MSPI_MAX_RW_BUF_SIZE, lastfrmsize);

}

BOOL MDrv_MSPI_Read(struct mstar_spi_dev *mspi_dev, U8 *pdata, U16 u16size)
{
    U8  i = 0;
    U8  frmcnt = 0;
	U8  lastfrmsize = 0;

    frmcnt = u16size/MSPI_MAX_RW_BUF_SIZE;
	lastfrmsize = u16size%MSPI_MAX_RW_BUF_SIZE;

    for (i = 0; i < frmcnt; i++)
	{
        if(!HAL_MSPI_Read(mspi_dev, pdata+i*MSPI_MAX_RW_BUF_SIZE, MSPI_MAX_RW_BUF_SIZE))
        {
            return FALSE;;
        }
    }

	if(!lastfrmsize)
		return TRUE;

    return HAL_MSPI_Read(mspi_dev, pdata+frmcnt*MSPI_MAX_RW_BUF_SIZE, lastfrmsize);

}

static int mstar_spi_setup(struct spi_device *spi)
{
	struct mstar_spi_dev *mspi_dev = spi_master_get_devdata(spi->master);

    //HAL_MSPI_SetSpiEnable(mspi_dev, 0);
	HAL_MSPI_SetSpiMode(mspi_dev, spi->mode & MSTAR_SPI_MODE_BITS);
	HAL_MSPI_SetLsb(mspi_dev, (spi->mode & SPI_LSB_FIRST)>>3);
	spi->max_speed_hz = MDrv_MSPI_SetSpeed(mspi_dev, spi->max_speed_hz);
    //HAL_MSPI_SetSpiEnable(mspi_dev, 1);
	mspi_dbg("%s:%d mode=%d,speed=%d\n",__func__, __LINE__, spi->mode, spi->max_speed_hz);
    return 0;
}

static int mstar_spi_start_transfer(struct spi_device *spi, struct spi_transfer *tfr)
{
	struct mstar_spi_dev *mspi_dev = spi_master_get_devdata(spi->master);

	//mspi_dbg("%s:%d\n",__func__, __LINE__);

	mspi_dev->tx_buf = tfr->tx_buf;
	mspi_dev->rx_buf = tfr->rx_buf;
	mspi_dev->len = tfr->len;

	HAL_MSPI_ChipSelect(mspi_dev, 1, spi->chip_select);

	if(mspi_dev->tx_buf != NULL)
	{
	    if(!MDrv_MSPI_Write(mspi_dev, (U8 *)mspi_dev->tx_buf, (U16)mspi_dev->len))
			return E_MSPI_WRITE_ERR;
    }

	if(mspi_dev->rx_buf != NULL)
	{
		if(!MDrv_MSPI_Read(mspi_dev, mspi_dev->rx_buf,(U16)mspi_dev->len))
			return E_MSPI_READ_ERR;
    }

	return E_MSPI_OK;
}

static int mstar_spi_finish_transfer(struct spi_device *spi,
	struct spi_transfer *tfr, bool cs_change)
{

	struct mstar_spi_dev *mspi_dev = spi_master_get_devdata(spi->master);

	if (tfr->delay_usecs)
		udelay(tfr->delay_usecs);

	if (cs_change){
		/* Clear TA flag */
		HAL_MSPI_ChipSelect(mspi_dev , 0, spi->chip_select);
	}

	return 0;
}

static int mstar_spi_transfer_one(struct spi_master *master, struct spi_message *mesg)
{
	struct mstar_spi_dev *mspi_dev = spi_master_get_devdata(master);
	struct spi_device *spi = mesg->spi;
	struct spi_transfer *tfr;
	int err = 0;
	bool cs_change;

	//mspi_dbg("%s:%d\n",__func__, __LINE__);

	list_for_each_entry(tfr, &mesg->transfers, transfer_list){
		err = mstar_spi_start_transfer(spi, tfr);
		if (err){
			mspi_dbg("%s:%d start_transfer error\n",__func__, __LINE__);
			goto out;
		}

		cs_change = tfr->cs_change ||
			list_is_last(&tfr->transfer_list, &mesg->transfers);

		err = mstar_spi_finish_transfer(spi, tfr, cs_change);
		if (err){
			mspi_dbg("%s:%d finish transfer error\n",__func__, __LINE__);
			goto out;
		}
		mesg->actual_length += mspi_dev->len;
		mspi_dbg("%s:%d transfered:%d\n",__func__, __LINE__, mesg->actual_length);
	}

out:
	/* Clear FIFOs, and disable the HW block */
	mesg->status = err;
	spi_finalize_current_message(master);

	return 0;
}

static irqreturn_t mstar_spi_interrupt(int irq, void *dev_id)
{
	struct mstar_spi_dev *mspi_dev = dev_id;
	int flag = 0;

	flag = HAL_MSPI_GetDoneFlag(mspi_dev);
	if(flag == 1)
	{
        complete(&mspi_dev->done);
        //mspi_dbg("%s:%d spi done!\n",__func__, __LINE__);
	}

	HAL_MSPI_ClearDoneFlag(mspi_dev);

    return IRQ_HANDLED;
}

static int mstar_spi_probe(struct platform_device *pdev)
{
	int err = 0;
	int proval = 0;
	struct resource *res;
    struct spi_master *master;
	struct mstar_spi_dev *mspi_dev;

    mspi_dbg("%s:%d enter\n",__func__, __LINE__);

	master = spi_alloc_master(&pdev->dev, sizeof(*mspi_dev));
	if (!master)
	{
		mspi_dbg("%s:%d spi_alloc_master() failed\n",__func__, __LINE__);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, master);

	master->dev.of_node = pdev->dev.of_node;
	master->bus_num = pdev->id;
	master->mode_bits = MSTAR_SPI_MODE_BITS;
	master->num_chipselect = 3;
	master->max_speed_hz = 54000000;    // 108M/2
	master->min_speed_hz = 46875;       // 12M/256
	master->bits_per_word_mask = SPI_BPW_MASK(8);
	master->setup = mstar_spi_setup;
	master->transfer_one_message = mstar_spi_transfer_one;

    mspi_dev = spi_master_get_devdata(master);

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mspi_dev->spibase = (void *)(IO_ADDRESS(res->start));

    res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	mspi_dev->chipbase = (void *)(IO_ADDRESS(res->start));

	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	mspi_dev->clkbase= (void *)(IO_ADDRESS(res->start));

	of_property_read_u32(pdev->dev.of_node, "spi-num", &proval);
	mspi_dev->spinum = proval;
	master->bus_num = proval;

    of_property_read_u32(pdev->dev.of_node, "pad-mode", &proval);
	mspi_dev->padmode = proval;

	init_completion(&mspi_dev->done);

	_mspi_clk_init();

	mspi_dev->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (!mspi_dev->irq)
    {
		mspi_dbg("%s:%d irq_of_parse_and_map() failed\n",__func__, __LINE__);
		goto err0;
    }

    if (devm_request_irq(&pdev->dev, mspi_dev->irq, mstar_spi_interrupt, 0/*SA_INTERRUPT*/, pdev->name, mspi_dev))
    {
		mspi_dbg("%s:%d request_irq() failed\n",__func__, __LINE__);
		goto err0;
    }

	MDrv_MSPI_Init(mspi_dev);

	err = devm_spi_register_master(&pdev->dev, master);
	if (err)
	{
		mspi_dbg("%s:%d register error(%d)\n",__func__, __LINE__, err);
		goto err0;
	}

	mspi_dbg("%s:%d exit\n",__func__, __LINE__);

	return 0;

err0:
	spi_master_put(master);
	mspi_dbg("%s:%d spi error exit\n",__func__, __LINE__);
	return err;
}

static int mstar_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master = platform_get_drvdata(pdev);

	spi_master_put(master);

	return 0;
}

static const struct of_device_id mstar_spi_match[] = {
	{ .compatible = "mstar_spi", },
	{}
};
MODULE_DEVICE_TABLE(of, mstar_spi_match);

static struct platform_driver mstar_spi_driver = {
	.driver		= {
		.name		= DRV_NAME,
		.owner		= THIS_MODULE,
		.of_match_table	= mstar_spi_match,
	},
	.probe		= mstar_spi_probe,
	.remove		= mstar_spi_remove,
};
module_platform_driver(mstar_spi_driver);

MODULE_DESCRIPTION("SPI controller driver for Broadcom BCM2835");
MODULE_AUTHOR("mstar Boot <bootc@bootc.net>");
MODULE_LICENSE("GPL v2");
