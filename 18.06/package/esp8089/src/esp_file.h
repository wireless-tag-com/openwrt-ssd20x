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

#ifndef _ESP_FILE_H_
#define _ESP_FILE_H_

#include <linux/version.h>
#include <linux/firmware.h>

#define E_ROUND_UP(x, y)  ((((x) + ((y) - 1)) / (y)) * (y))

#define CONF_ATTR_LEN 24
#define CONF_VAL_LEN 3
#define MAX_ATTR_NUM 24
#define MAX_FIX_ATTR_NUM 16
#define MAX_BUF_LEN ((CONF_ATTR_LEN + CONF_VAL_LEN + 2) * MAX_ATTR_NUM + 2)

struct esp_init_table_elem {
	char attr[CONF_ATTR_LEN];
	int offset;
	short value;
};

int request_init_conf(struct device *dev);
void fix_init_data(u8 * init_data_buf, int buf_size);

#endif				/* _ESP_FILE_H_ */
