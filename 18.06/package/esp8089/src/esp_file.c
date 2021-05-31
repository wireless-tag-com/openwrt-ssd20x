/*
 * Copyright (c) 2010 -2014 Espressif System.
 *
 *   file operation in kernel space
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

#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/moduleparam.h>
#include <linux/firmware.h>
#include <linux/netdevice.h>
#include <linux/aio.h>
#include <linux/property.h>

#include "esp_file.h"
#include "esp_debug.h"
#include "esp_sif.h"

static char *modparam_init_data_conf;
module_param_named(config, modparam_init_data_conf, charp, 0444);
MODULE_PARM_DESC(config, "Firmware init config string (format: key=value;)");

struct esp_init_table_elem esp_init_table[MAX_ATTR_NUM] = {
	/*
	 * Crystal type:
	 * 0: 40MHz (default)
	 * 1: 26MHz (ESP8266 ESP-12F)
	 */
	{"crystal_26M_en", 48, 1},
	/*
	 * Output crystal clock to pin:
	 * 0: None
	 * 1: GPIO1
	 * 2: URXD0
	 */
	{"test_xtal", 49, 0},
	/*
	 * Host SDIO mode:
	 * 0: Auto by pin strapping
	 * 1: SDIO data output on negative edges (SDIO v1.1)
	 * 2: SDIO data output on positive edges (SDIO v2.0)
	 */
	{"sdio_configure", 50, 2},
	/*
	 * WiFi/Bluetooth co-existence with BK3515A BT chip
	 * 0: None
	 * 1: GPIO0->WLAN_ACTIVE, MTMS->BT_ACTIVE, MTDI->BT_PRIORITY,
	 *    U0TXD->ANT_SEL_BT, U0RXD->ANT_SEL_WIFI
	 */
	{"bt_configure", 51, 0},
	/*
	 * Antenna selection:
	 * 0: Antenna is for WiFi
	 * 1: Antenna is for Bluetooth
	 */
	{"bt_protocol", 52, 0},
	/*
	 * Dual antenna configuration mode:
	 * 0: None
	 * 1: U0RXD + XPD_DCDC
	 * 2: U0RXD + GPIO0
	 * 3: U0RXD + U0TXD
	 */
	{"dual_ant_configure", 53, 0},
	/*
	 * Firmware debugging output pin:
	 * 0: None
	 * 1: UART TX on GPIO2
	 * 2: UART TX on U0TXD
	 */
	{"test_uart_configure", 54, 2},
	/*
	 * Whether to share crystal clock with BT (in sleep mode):
	 * 0: no
	 * 1: always on
	 * 2: automatically on according to XPD_DCDC
	 */
	{"share_xtal", 55, 0},
	/*
	 * Allow chip to be woken up during sleep on pin:
	 * 0: None
	 * 1: XPD_DCDC
	 * 2: GPIO0
	 * 3: Both XPD_DCDC and GPIO0
	 */
	{"gpio_wake", 56, 0},
	{"no_auto_sleep", 57, 0},
	{"speed_suspend", 58, 0},
	{"attr11", -1, -1},
	{"attr12", -1, -1},
	{"attr13", -1, -1},
	{"attr14", -1, -1},
	{"attr15", -1, -1},
	//attr that is not send to target
	/*
	 * Allow chip to be reset by GPIO pin:
	 * 0: no
	 * 1: yes
	 */
	{"ext_rst", -1, 0},
	{"wakeup_gpio", -1, 12},
	{"ate_test", -1, 0},
	{"attr19", -1, -1},
	{"attr20", -1, -1},
	{"attr21", -1, -1},
	{"attr22", -1, -1},
	{"attr23", -1, -1},
};

/* update init config table */
static int set_init_config_attr(const char *attr, int attr_len, short value)
{
	int i;

	for (i = 0; i < MAX_ATTR_NUM; i++) {
		if (!memcmp(esp_init_table[i].attr, attr, attr_len)) {
			if (value < 0 || value > 255) {
				esp_dbg(ESP_DBG_ERROR, "%s: attribute value for %s is out of range",
					__func__, esp_init_table[i].attr);
				return -1;
			}
			esp_init_table[i].value = value;
			return 0;
		}
	}

	return -1;
}

static int update_init_config_attr(const char *attr, int attr_len,
				   const char *val, int val_len)
{
	char digits[4];
	short value;
	int i;

	for (i = 0; i < sizeof(digits) - 1 && i < val_len; i++)
		digits[i] = val[i];
	digits[i] = 0;

	if (kstrtou16(digits, 10, &value) < 0) {
		esp_dbg(ESP_DBG_ERROR, "%s: invalid attribute value: %s",
			__func__, digits);
		return -1;
	}

	return set_init_config_attr(attr, attr_len, value);
}

/* export config table settings to SDIO driver */
static void record_init_config(void)
{
	int i;

	for (i = 0; i < MAX_ATTR_NUM; i++) {
		if (esp_init_table[i].value < 0)
			continue;

		if (!strcmp(esp_init_table[i].attr, "share_xtal"))
			sif_record_bt_config(esp_init_table[i].value);
		else if (!strcmp(esp_init_table[i].attr, "ext_rst"))
			sif_record_rst_config(esp_init_table[i].value);
		else if (!strcmp(esp_init_table[i].attr, "wakeup_gpio"))
			sif_record_wakeup_gpio_config(esp_init_table[i].value);
		else if (!strcmp(esp_init_table[i].attr, "ate_test"))
			sif_record_ate_config(esp_init_table[i].value);
	}
}

int request_init_conf(struct device *dev)
{
	char *attr, *str, *p;
	int attr_len, str_len;
	int ret = 0;
	u32 val;

	/* Check for any parameters passed through devicetree (or acpi) */
	if (device_property_read_u32(dev, "esp,crystal-26M-en", &val) == 0)
		set_init_config_attr("crystal_26M_en", strlen("crystal_26M_en"),
				     val);

	/* parse optional parameter in the form of key1=value,key2=value,.. */
	attr = NULL;
	attr_len = str_len = 0;
	for (p = str = modparam_init_data_conf; p && *p; p++) {
		if (*p == '=') {
			attr = str;
			attr_len = str_len;

			str = p + 1;
			str_len = 0;
		} else if (*p == ',' || *p == ';') {
			if (attr_len)
				ret |= update_init_config_attr(attr, attr_len,
							       str, str_len);

			str = p + 1;
			attr_len = str_len = 0;
		} else
			str_len++;
	}

	if (attr_len && str != attr)
		ret |= update_init_config_attr(attr, attr_len, str, str_len);

	record_init_config();

	return ret;
}

void fix_init_data(u8 * init_data_buf, int buf_size)
{
	int i;

	for (i = 0; i < MAX_FIX_ATTR_NUM; i++) {
		if (esp_init_table[i].offset > -1
		    && esp_init_table[i].offset < buf_size
		    && esp_init_table[i].value > -1) {
			*(u8 *) (init_data_buf +
				 esp_init_table[i].offset) =
			    esp_init_table[i].value;
		} else if (esp_init_table[i].offset > buf_size) {
			esp_dbg(ESP_DBG_ERROR,
				"%s: offset[%d] longer than init_data_buf len[%d] Ignore\n",
				__FUNCTION__, esp_init_table[i].offset,
				buf_size);
		}
	}

}
