/*
 * Copyright (c) 2010 -2013 Espressif System.
 *
 *   sdio serial i/f driver
 *    - sdio device control routines
 *    - sync/async DMA/PIO read/write
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
 */
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sd.h>
#include <linux/module.h>
#include <net/mac80211.h>
#include <linux/time.h>
#include <linux/pm.h>

#include "esp_pub.h"
#include "esp_sif.h"
#include "esp_sip.h"
#include "esp_debug.h"
#include "slc_host_register.h"
#include "esp_version.h"
#include "esp_ctrl.h"
#include "esp_file.h"
#ifdef USE_EXT_GPIO
#include "esp_ext.h"
#endif				/* USE_EXT_GPIO */

static struct semaphore esp_powerup_sem;

/*
 * HdG: Note:
 * 1) MMC_HAS_FORCE_DETECT_CHANGE is a hack which is set by my sunxi-wip
 *    tree. FIXME replace with a version check once mmc_force_detect_change()
 *    is added to the mainline kernel.
 * 2) This version does NOT implement keep_power, the dts must mark the
 *    regulators as regulator-always-on and not use mmc-pwrseq for this stub
 *    to work.
 */
#ifndef MMC_HAS_FORCE_DETECT_CHANGE
void mmc_force_detect_change(struct mmc_host *host, unsigned long delay,
			     bool keep_power)
{
	host->caps &= ~MMC_CAP_NONREMOVABLE;
	host->caps |= MMC_CAP_NEEDS_POLL;
	mmc_detect_change(host, delay);
}
#endif

static int /*__init*/ esp_sdio_init(void);
static void /*__exit*/ esp_sdio_exit(void);


#define ESP_DMA_IBUFSZ   2048

//unsigned int esp_msg_level = 0;
unsigned int esp_msg_level = ESP_DBG_ERROR | ESP_SHOW;

struct esp_sdio_ctrl *sif_sctrl = NULL;

#ifdef ESP_ANDROID_LOGGER
bool log_off = false;
#endif				/* ESP_ANDROID_LOGGER */

static int esdio_power_off(struct esp_sdio_ctrl *sctrl);
static int esdio_power_on(struct esp_sdio_ctrl *sctrl);

void sif_set_clock(struct sdio_func *func, int clk);

void sif_lock_bus(struct esp_pub *epub)
{
	EPUB_FUNC_CHECK(epub, _exit);

	sdio_claim_host(EPUB_TO_FUNC(epub));
      _exit:
	return;
}

void sif_unlock_bus(struct esp_pub *epub)
{
	EPUB_FUNC_CHECK(epub, _exit);

	sdio_release_host(EPUB_TO_FUNC(epub));
      _exit:
	return;
}

static inline bool bad_buf(u8 * buf)
{
	return ((unsigned long) buf & 0x3) || !virt_addr_valid(buf);
}

u8 sdio_io_readb(struct esp_pub *epub, int addr, int *res)
{
	struct esp_sdio_ctrl *sctrl = NULL;
	struct sdio_func *func = NULL;
	sctrl = (struct esp_sdio_ctrl *) epub->sif;
	func = sctrl->func;

	if (func->num == 0)
		return sdio_f0_readb(func, addr, res);
	else
		return sdio_readb(func, addr, res);
}

void sdio_io_writeb(struct esp_pub *epub, u8 value, int addr, int *res)
{
	struct esp_sdio_ctrl *sctrl = NULL;
	struct sdio_func *func = NULL;
	sctrl = (struct esp_sdio_ctrl *) epub->sif;
	func = sctrl->func;

	if (func->num == 0)
		sdio_f0_writeb(func, value, addr, res);
	else
		sdio_writeb(func, value, addr, res);
}

int sif_io_raw(struct esp_pub *epub, u32 addr, u8 * buf, u32 len, u32 flag)
{
	int err = 0;
	u8 *ibuf = NULL;
	bool need_ibuf = false;
	struct esp_sdio_ctrl *sctrl = NULL;
	struct sdio_func *func = NULL;

	if (epub == NULL || buf == NULL) {
		ESSERT(0);
		err = -EINVAL;
		goto _exit;
	}

	sctrl = (struct esp_sdio_ctrl *) epub->sif;
	func = sctrl->func;
	if (func == NULL) {
		ESSERT(0);
		err = -EINVAL;
		goto _exit;
	}

	if (bad_buf(buf)) {
		esp_dbg(ESP_DBG_TRACE, "%s dst 0x%08x, len %d badbuf\n",
			__func__, addr, len);
		need_ibuf = true;
		ibuf = sctrl->dma_buffer;
	} else {
		ibuf = buf;
	}

	if (flag & SIF_BLOCK_BASIS) {
		/* round up for block data transcation */
	}

	if (flag & SIF_TO_DEVICE) {

		if (need_ibuf)
			memcpy(ibuf, buf, len);

		if (flag & SIF_FIXED_ADDR)
			err = sdio_writesb(func, addr, ibuf, len);
		else if (flag & SIF_INC_ADDR) {
			err = sdio_memcpy_toio(func, addr, ibuf, len);
		}
	} else if (flag & SIF_FROM_DEVICE) {

		if (flag & SIF_FIXED_ADDR)
			err = sdio_readsb(func, ibuf, addr, len);
		else if (flag & SIF_INC_ADDR) {
			err = sdio_memcpy_fromio(func, ibuf, addr, len);
		}


		if (!err && need_ibuf)
			memcpy(buf, ibuf, len);
	}

      _exit:
	return err;
}

int sif_io_sync(struct esp_pub *epub, u32 addr, u8 * buf, u32 len,
		u32 flag)
{
	int err = 0;
	u8 *ibuf = NULL;
	bool need_ibuf = false;
	struct esp_sdio_ctrl *sctrl = NULL;
	struct sdio_func *func = NULL;

	if (epub == NULL || buf == NULL) {
		ESSERT(0);
		err = -EINVAL;
		goto _exit;
	}

	sctrl = (struct esp_sdio_ctrl *) epub->sif;
	func = sctrl->func;
	if (func == NULL) {
		ESSERT(0);
		err = -EINVAL;
		goto _exit;
	}

	if (bad_buf(buf)) {
		esp_dbg(ESP_DBG_TRACE, "%s dst 0x%08x, len %d badbuf\n",
			__func__, addr, len);
		need_ibuf = true;
		ibuf = sctrl->dma_buffer;
	} else {
		ibuf = buf;
	}

	if (flag & SIF_BLOCK_BASIS) {
		/* round up for block data transcation */
	}

	if (flag & SIF_TO_DEVICE) {

		esp_dbg(ESP_DBG_TRACE, "%s to addr 0x%08x, len %d \n",
			__func__, addr, len);
		if (need_ibuf)
			memcpy(ibuf, buf, len);

		sdio_claim_host(func);

		if (flag & SIF_FIXED_ADDR)
			err = sdio_writesb(func, addr, ibuf, len);
		else if (flag & SIF_INC_ADDR) {
			err = sdio_memcpy_toio(func, addr, ibuf, len);
		}
		sdio_release_host(func);
	} else if (flag & SIF_FROM_DEVICE) {

		esp_dbg(ESP_DBG_TRACE, "%s from addr 0x%08x, len %d \n",
			__func__, addr, len);

		sdio_claim_host(func);

		if (flag & SIF_FIXED_ADDR)
			err = sdio_readsb(func, ibuf, addr, len);
		else if (flag & SIF_INC_ADDR) {
			err = sdio_memcpy_fromio(func, ibuf, addr, len);
		}

		sdio_release_host(func);

		if (!err && need_ibuf)
			memcpy(buf, ibuf, len);
	}

      _exit:
	return err;
}

int sif_lldesc_read_sync(struct esp_pub *epub, u8 * buf, u32 len)
{
	struct esp_sdio_ctrl *sctrl = NULL;
	u32 read_len;

	if (epub == NULL || buf == NULL) {
		ESSERT(0);
		return -EINVAL;
	}

	sctrl = (struct esp_sdio_ctrl *) epub->sif;

	switch (sctrl->target_id) {
	case 0x100:
		read_len = len;
		break;
	case 0x600:
		read_len = roundup(len, sctrl->slc_blk_sz);
		break;
	default:
		read_len = len;
		break;
	}

	return sif_io_sync((epub),
			   (sctrl->slc_window_end_addr - 2 - (len)), (buf),
			   (read_len),
			   SIF_FROM_DEVICE | SIF_BYTE_BASIS |
			   SIF_INC_ADDR);
}

int sif_lldesc_write_sync(struct esp_pub *epub, u8 * buf, u32 len)
{
	struct esp_sdio_ctrl *sctrl = NULL;
	u32 write_len;

	if (epub == NULL || buf == NULL) {
		ESSERT(0);
		return -EINVAL;
	}

	sctrl = (struct esp_sdio_ctrl *) epub->sif;

	switch (sctrl->target_id) {
	case 0x100:
		write_len = len;
		break;
	case 0x600:
		write_len = roundup(len, sctrl->slc_blk_sz);
		break;
	default:
		write_len = len;
		break;
	}

	return sif_io_sync((epub), (sctrl->slc_window_end_addr - (len)),
			   (buf), (write_len),
			   SIF_TO_DEVICE | SIF_BYTE_BASIS | SIF_INC_ADDR);
}

int sif_lldesc_read_raw(struct esp_pub *epub, u8 * buf, u32 len,
			bool noround)
{
	struct esp_sdio_ctrl *sctrl = NULL;
	u32 read_len;

	if (epub == NULL || buf == NULL) {
		ESSERT(0);
		return -EINVAL;
	}

	sctrl = (struct esp_sdio_ctrl *) epub->sif;

	switch (sctrl->target_id) {
	case 0x100:
		read_len = len;
		break;
	case 0x600:
		if (!noround)
			read_len = roundup(len, sctrl->slc_blk_sz);
		else
			read_len = len;
		break;
	default:
		read_len = len;
		break;
	}

	return sif_io_raw((epub), (sctrl->slc_window_end_addr - 2 - (len)),
			  (buf), (read_len),
			  SIF_FROM_DEVICE | SIF_BYTE_BASIS | SIF_INC_ADDR);
}

int sif_lldesc_write_raw(struct esp_pub *epub, u8 * buf, u32 len)
{
	struct esp_sdio_ctrl *sctrl = NULL;
	u32 write_len;

	if (epub == NULL || buf == NULL) {
		ESSERT(0);
		return -EINVAL;
	}

	sctrl = (struct esp_sdio_ctrl *) epub->sif;

	switch (sctrl->target_id) {
	case 0x100:
		write_len = len;
		break;
	case 0x600:
		write_len = roundup(len, sctrl->slc_blk_sz);
		break;
	default:
		write_len = len;
		break;
	}
	return sif_io_raw((epub), (sctrl->slc_window_end_addr - (len)),
			  (buf), (write_len),
			  SIF_TO_DEVICE | SIF_BYTE_BASIS | SIF_INC_ADDR);

}

#define MANUFACTURER_ID_EAGLE_BASE        0x1110
#define MANUFACTURER_ID_EAGLE_BASE_MASK     0xFF00
#define MANUFACTURER_CODE                  0x6666

static const struct sdio_device_id esp_sdio_devices[] = {
	{SDIO_DEVICE
	 (MANUFACTURER_CODE, (MANUFACTURER_ID_EAGLE_BASE | 0x1))},
	{},
};

static int esdio_power_on(struct esp_sdio_ctrl *sctrl)
{
	int err = 0;

	if (sctrl->off == false)
		return err;

	sdio_claim_host(sctrl->func);
	err = sdio_enable_func(sctrl->func);

	if (err) {
		esp_dbg(ESP_DBG_ERROR, "Unable to enable sdio func: %d\n",
			err);
		sdio_release_host(sctrl->func);
		return err;
	}

	sdio_release_host(sctrl->func);

	/* ensure device is up */
	msleep(5);

	sctrl->off = false;

	return err;
}

static int esdio_power_off(struct esp_sdio_ctrl *sctrl)
{
	int err;

	if (sctrl->off)
		return 0;

	sdio_claim_host(sctrl->func);
	err = sdio_disable_func(sctrl->func);
	sdio_release_host(sctrl->func);

	if (err)
		return err;

	sctrl->off = true;

	return err;
}

void sif_enable_irq(struct esp_pub *epub)
{
	int err;
	struct esp_sdio_ctrl *sctrl = NULL;

	sctrl = (struct esp_sdio_ctrl *) epub->sif;

	sdio_claim_host(sctrl->func);

	err = sdio_claim_irq(sctrl->func, sif_dsr);

	if (err)
		esp_dbg(ESP_DBG_ERROR, "sif %s failed\n", __func__);

	atomic_set(&epub->sip->state, SIP_BOOT);

	atomic_set(&sctrl->irq_installed, 1);

	sdio_release_host(sctrl->func);
}

void sif_disable_irq(struct esp_pub *epub)
{
	struct esp_sdio_ctrl *sctrl = (struct esp_sdio_ctrl *) epub->sif;
	int i = 0;

	if (atomic_read(&sctrl->irq_installed) == 0)
		return;

	sdio_claim_host(sctrl->func);

	while (atomic_read(&sctrl->irq_handling)) {
		sdio_release_host(sctrl->func);
		schedule_timeout(HZ / 100);
		sdio_claim_host(sctrl->func);
		if (i++ >= 400) {
			esp_dbg(ESP_DBG_ERROR, "%s force to stop irq\n",
				__func__);
			break;
		}
	}

	/* Ignore errors, we don't always use an irq. */
	sdio_release_irq(sctrl->func);

	atomic_set(&sctrl->irq_installed, 0);

	sdio_release_host(sctrl->func);

}

void sif_set_clock(struct sdio_func *func, int clk)
{
	struct mmc_host *host = NULL;
	struct mmc_card *card = NULL;

	card = func->card;
	host = card->host;

	sdio_claim_host(func);

	//currently only set clock
	host->ios.clock = clk * 1000000;

	esp_dbg(ESP_SHOW, "%s clock is %u\n", __func__, host->ios.clock);
	if (host->ios.clock > host->f_max) {
		host->ios.clock = host->f_max;
	}
	host->ops->set_ios(host, &host->ios);

	mdelay(2);

	sdio_release_host(func);
}

static int esp_sdio_probe(struct sdio_func *func,
			  const struct sdio_device_id *id);
static void esp_sdio_remove(struct sdio_func *func);

static int esp_sdio_probe(struct sdio_func *func,
			  const struct sdio_device_id *id)
{
	int err = 0;
	struct esp_pub *epub = NULL;
	struct esp_sdio_ctrl *sctrl;
	struct mmc_host *host = func->card->host;

	esp_dbg(ESP_DBG_TRACE,
		"sdio_func_num: 0x%X, vendor id: 0x%X, dev id: 0x%X, block size: 0x%X/0x%X\n",
		func->num, func->vendor, func->device, func->max_blksize,
		func->cur_blksize);
	if (sif_sctrl == NULL) {
		request_init_conf(&func->dev);

		sctrl = kzalloc(sizeof(struct esp_sdio_ctrl), GFP_KERNEL);

		if (sctrl == NULL) {
			return -ENOMEM;
		}

		/* temp buffer reserved for un-dma-able request */
		sctrl->dma_buffer = kzalloc(ESP_DMA_IBUFSZ, GFP_KERNEL);

		if (sctrl->dma_buffer == NULL) {
			err = -ENOMEM;
			goto _err_last;
		}
		sif_sctrl = sctrl;
		sctrl->slc_blk_sz = SIF_SLC_BLOCK_SIZE;

		epub = esp_pub_alloc_mac80211(&func->dev);

		if (epub == NULL) {
			esp_dbg(ESP_DBG_ERROR, "no mem for epub \n");
			err = -ENOMEM;
			goto _err_dma;
		}
		epub->sif = (void *) sctrl;
		epub->sdio_state = ESP_SDIO_STATE_FIRST_INIT;
		sctrl->epub = epub;

#ifdef USE_EXT_GPIO
		if (sif_get_ate_config() == 0) {
			err = ext_gpio_init(epub);
			if (err) {
				esp_dbg(ESP_DBG_ERROR,
					"ext_irq_work_init failed %d\n",
					err);
				goto _err_epub;
			}
		}
#endif

	} else {
		sctrl = sif_sctrl;
		sif_sctrl = NULL;
		epub = sctrl->epub;
		epub->sdio_state = ESP_SDIO_STATE_SECOND_INIT;
		SET_IEEE80211_DEV(epub->hw, &func->dev);
		epub->dev = &func->dev;
	}

	sctrl->func = func;
	sdio_set_drvdata(func, sctrl);

	sctrl->id = id;
	sctrl->off = true;

	/* give us some time to enable, in ms */
	func->enable_timeout = 100;

	err = esdio_power_on(sctrl);
	esp_dbg(ESP_DBG_TRACE, " %s >> power_on err %d \n", __func__, err);

	if (err) {
		if (epub->sdio_state == ESP_SDIO_STATE_FIRST_INIT)
			goto _err_ext_gpio;
		else
			goto _err_second_init;
	}
	check_target_id(epub);

	sdio_claim_host(func);

	err = sdio_set_block_size(func, sctrl->slc_blk_sz);

	if (err) {
		esp_dbg(ESP_DBG_ERROR,
			"Set sdio block size %d failed: %d)\n",
			sctrl->slc_blk_sz, err);
		sdio_release_host(func);
		if (epub->sdio_state == ESP_SDIO_STATE_FIRST_INIT)
			goto _err_off;
		else
			goto _err_second_init;
	}

	sdio_release_host(func);

#ifdef LOWER_CLK
	/* fix clock for dongle */
	sif_set_clock(func, 23);
#endif				//LOWER_CLK

	err = esp_pub_init_all(epub);

	if (err) {
		esp_dbg(ESP_DBG_ERROR, "esp_init_all failed: %d\n", err);
		if (epub->sdio_state == ESP_SDIO_STATE_FIRST_INIT) {
			err = 0;
			goto _err_first_init;
		}
		if (epub->sdio_state == ESP_SDIO_STATE_SECOND_INIT)
			goto _err_second_init;
	}

	esp_dbg(ESP_DBG_TRACE, " %s return  %d\n", __func__, err);
	if (epub->sdio_state == ESP_SDIO_STATE_FIRST_INIT) {
		esp_dbg(ESP_DBG_TRACE, "first normal exit\n");
		epub->sdio_state = ESP_SDIO_STATE_FIRST_NORMAL_EXIT;
		/* Rescan the esp8089 after loading the initial firmware */
		mmc_force_detect_change(host, msecs_to_jiffies(100), true);
	}

	return err;

      _err_off:
	esdio_power_off(sctrl);
      _err_ext_gpio:
#ifdef USE_EXT_GPIO
	if (sif_get_ate_config() == 0)
		ext_gpio_deinit();
      _err_epub:
#endif
	esp_pub_dealloc_mac80211(epub);
      _err_dma:
	kfree(sctrl->dma_buffer);
      _err_last:
	kfree(sctrl);
      _err_first_init:
	if (epub && epub->sdio_state == ESP_SDIO_STATE_FIRST_INIT) {
		esp_dbg(ESP_DBG_ERROR, "first error exit\n");
		epub->sdio_state = ESP_SDIO_STATE_FIRST_ERROR_EXIT;
	}
	return err;
      _err_second_init:
	epub->sdio_state = ESP_SDIO_STATE_SECOND_ERROR_EXIT;
	esp_sdio_remove(func);
	return err;
}

static void esp_sdio_remove(struct sdio_func *func)
{
	struct esp_sdio_ctrl *sctrl = NULL;
	struct esp_pub *epub = NULL;

	esp_dbg(ESP_DBG_TRACE, "%s enter\n", __func__);

	sctrl = sdio_get_drvdata(func);

	if (sctrl == NULL) {
		esp_dbg(ESP_DBG_ERROR, "%s no sctrl\n", __func__);
		return;
	}

	do {
		epub = sctrl->epub;
		if (epub == NULL) {
			esp_dbg(ESP_DBG_ERROR, "%s epub null\n", __func__);
			break;
		}
		if (epub->sdio_state != ESP_SDIO_STATE_FIRST_NORMAL_EXIT) {
			if (epub->sip) {
				sip_detach(epub->sip);
				epub->sip = NULL;
				esp_dbg(ESP_DBG_TRACE,
					"%s sip detached \n", __func__);
			}
#ifdef USE_EXT_GPIO
			if (sif_get_ate_config() == 0)
				ext_gpio_deinit();
#endif
		} else {
			//sif_disable_target_interrupt(epub);
			atomic_set(&epub->sip->state, SIP_STOP);
			sif_disable_irq(epub);
		}

		if (epub->sdio_state != ESP_SDIO_STATE_FIRST_NORMAL_EXIT) {
			esp_pub_dealloc_mac80211(epub);
			esp_dbg(ESP_DBG_TRACE, "%s dealloc mac80211 \n",
				__func__);

			if (sctrl->dma_buffer) {
				kfree(sctrl->dma_buffer);
				sctrl->dma_buffer = NULL;
				esp_dbg(ESP_DBG_TRACE,
					"%s free dma_buffer \n", __func__);
			}

			kfree(sctrl);
		}

	} while (0);

	sdio_set_drvdata(func, NULL);

	esp_dbg(ESP_DBG_TRACE, "eagle sdio remove complete\n");
}

MODULE_DEVICE_TABLE(sdio, esp_sdio_devices);

static int esp_sdio_suspend(struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	struct esp_sdio_ctrl *sctrl = sdio_get_drvdata(func);
	struct esp_pub *epub = sctrl->epub;

	printk("%s", __func__);
	atomic_set(&epub->ps.state, ESP_PM_ON);

	do {
		u32 sdio_flags = 0;
		int ret = 0;
		sdio_flags = sdio_get_host_pm_caps(func);

		if (!(sdio_flags & MMC_PM_KEEP_POWER)) {
			printk
			    ("%s can't keep power while host is suspended\n",
			     __func__);
		}

		/* keep power while host suspended */
		ret = sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);
		if (ret) {
			printk("%s error while trying to keep power\n",
			       __func__);
		}
	} while (0);


	return 0;

}

static int esp_sdio_resume(struct device *dev)
{
	esp_dbg(ESP_DBG_ERROR, "%s", __func__);

	return 0;
}

static const struct dev_pm_ops esp_sdio_pm_ops = {
	.suspend = esp_sdio_suspend,
	.resume = esp_sdio_resume,
};

static struct sdio_driver esp_sdio_driver = {
	.name = "eagle_sdio",
	.id_table = esp_sdio_devices,
	.probe = esp_sdio_probe,
	.remove = esp_sdio_remove,
	.drv = {.pm = &esp_sdio_pm_ops,},
};

static int esp_sdio_dummy_probe(struct sdio_func *func, const struct sdio_device_id *id)
{
	printk("%s enter\n", __func__);

	up(&esp_powerup_sem);

	return 0;
}

static void esp_sdio_dummy_remove(struct sdio_func *func) 
{
	return;
}

static struct sdio_driver esp_sdio_dummy_driver = {
	.name = "eagle_sdio_dummy",
	.id_table = esp_sdio_devices,
	.probe = esp_sdio_dummy_probe,
	.remove = esp_sdio_dummy_remove,
};

static int /*__init*/ esp_sdio_init(void)
{
	esp_debugfs_init();

#define ESP_WAIT_UP_TIME_MS 11000
	int err;
	int retry = 3;
	bool powerup = false;

	esp_dbg(ESP_DBG_TRACE, "%s \n", __func__);

	do {
		sema_init(&esp_powerup_sem, 0);

		sif_platform_target_poweron();
		msleep(200);

		sif_platform_rescan_card(1);

		err = sdio_register_driver(&esp_sdio_dummy_driver);
		if (err) {
			esp_dbg(ESP_DBG_ERROR, "eagle sdio driver registration failed, error code: %d\n", err);
			goto _fail;
		}

		if (down_timeout(&esp_powerup_sem, msecs_to_jiffies(ESP_WAIT_UP_TIME_MS)) == 0) 
		{
			powerup = true;
			msleep(200);
			break;
		}

		esp_dbg(ESP_SHOW, "%s ------ RETRY ------ \n", __func__);

		sif_record_retry_config();

		sdio_unregister_driver(&esp_sdio_dummy_driver);

		sif_platform_rescan_card(0);

		sif_platform_target_poweroff();
		msleep(200);
	} while (retry--);

	if (powerup) {
		esp_dbg(ESP_SHOW, "%s power up OK\n", __func__);
		sdio_unregister_driver(&esp_sdio_dummy_driver);

		sdio_register_driver(&esp_sdio_driver);
	}

_fail:
	return 0;
}

static void /*__exit*/ esp_sdio_exit(void)
{
	sdio_unregister_driver(&esp_sdio_driver);
	esp_debugfs_exit();
}

MODULE_AUTHOR("Espressif System");
MODULE_DESCRIPTION
    ("Driver for SDIO interconnected eagle low-power WLAN devices");
MODULE_LICENSE("GPL");

module_init(esp_sdio_init);
module_exit(esp_sdio_exit);
