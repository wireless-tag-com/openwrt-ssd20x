/*
* file_access.c- Sigmastar
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
 * file_access.c
 *
 *  Created on: Nov 28, 2017
 *      Author: giggs.huang
 */
#if defined(CONFIG_ARCH_INFINITY2)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <asm/uaccess.h>

#include "file_access.h"

mm_segment_t oldfs;

void InitKernelEnv(void)
{
    oldfs = get_fs();
    set_fs(KERNEL_DS);
}

struct file *OpenFile(char *path,int flag,int mode)
{
    struct file *fp;

    InitKernelEnv();

    fp=filp_open(path, flag, mode);
    if (fp) return fp;
    else return NULL;
}

int ReadFile(struct file *fp,char *buf,int readlen)
{
    if (fp->f_op && fp->f_op->read)
        return fp->f_op->read(fp,buf,readlen, &fp->f_pos);
    else
        return -1;
}
int WriteFile(struct file *fp,char *buf,int writelen)
{
    ssize_t writeBytes = 0;
    
    writeBytes = vfs_write(fp, (char*)buf, writelen, &fp->f_pos);

    return (int)writeBytes;

}
int CloseFile(struct file *fp)
{
    filp_close(fp,NULL);
    set_fs(oldfs);
    return 0;
}

int GetFileAttr(struct file *fp, finfo_t* finfop)
{
    if(finfop && fp && fp->f_inode)
    {
        finfop->file_size = fp->f_inode->i_size;
        return 0;
    }
    return -1;
}

#endif
