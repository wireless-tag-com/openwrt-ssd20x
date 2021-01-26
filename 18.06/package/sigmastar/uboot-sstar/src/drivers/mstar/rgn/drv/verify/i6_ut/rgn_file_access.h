/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

/*
 * file_access.h
 *
 *  Created on: Nov 28, 2017
 *      Author: giggs.huang
 */

#ifndef DRV_TEST_KERNEL_UT_FILE_ACCESS_H_
#define DRV_TEST_KERNEL_UT_FILE_ACCESS_H_

#include <linux/fs.h>

struct file *OpenFile(char *path,int flag,int mode);
int ReadFile(struct file *fp,char *buf,int readlen);
int WriteFile(struct file *fp,char *buf,int writelen);
int CloseFile(struct file *fp);

#endif /* DRV_TEST_KERNEL_UT_FILE_ACCESS_H_ */
