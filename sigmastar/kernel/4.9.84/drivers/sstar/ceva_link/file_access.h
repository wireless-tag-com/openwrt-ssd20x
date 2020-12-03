/*
* file_access.h- Sigmastar
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
 * file_access.h
 *
 *  Created on: Nov 28, 2017
 *      Author: giggs.huang
 */

#ifndef DRV_TEST_KERNEL_UT_FILE_ACCESS_H_
#define DRV_TEST_KERNEL_UT_FILE_ACCESS_H_

#if defined(CONFIG_ARCH_INFINITY2)
#include <linux/fs.h>

typedef struct finfo_s
{
    uint file_size;
} finfo_t;

struct file *OpenFile(char *path,int flag,int mode);
int ReadFile(struct file *fp,char *buf,int readlen);
int WriteFile(struct file *fp,char *buf,int writelen);
int CloseFile(struct file *fp);
int GetFileAttr(struct file *fp, finfo_t* finfop);
#endif
#endif /* DRV_TEST_KERNEL_UT_FILE_ACCESS_H_ */
