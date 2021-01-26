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

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/buffer_head.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include "cam_os_wrapper.h"

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

struct file *OpenFile(char *path,int flag,int mode)
{
    struct file *filp = NULL;
    mm_segment_t oldfs;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flag, mode);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        return NULL;
    }
    return filp;
}

int ReadFile(struct file *fp, char *buf,int readlen)
{
    mm_segment_t oldfs;
    int ret;
    // loff_t pos = fp->f_pos;
    oldfs = get_fs();
    set_fs(get_ds());
    ret = vfs_read(fp, buf, readlen, &fp->f_pos);
    // fp->f_pos = pos;
    set_fs(oldfs);
    return ret;
}

int WriteFile(struct file *fp,char *buf,int writelen)
{
    mm_segment_t oldfs;
    int ret;
    // loff_t pos = fp->f_pos;
    oldfs = get_fs();
    set_fs(get_ds());
    ret = vfs_write(fp, buf, writelen, &fp->f_pos);
    // fp->f_pos = pos;
    set_fs(oldfs);
    return ret;
}

int CloseFile(struct file *fp)
{
    filp_close(fp, NULL);
    return 0;
}
