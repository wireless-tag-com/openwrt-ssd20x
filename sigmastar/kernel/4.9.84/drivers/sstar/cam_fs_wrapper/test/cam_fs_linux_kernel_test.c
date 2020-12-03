/*
* cam_fs_linux_kernel_test.c- Sigmastar
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


///////////////////////////////////////////////////////////////////////////////
/// @file      cam_fs_linux_kernel_test.c
/// @brief     Cam FS Wrapper Test Code for Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#include <linux/kernel.h>
#include <linux/module.h>
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SStar CamFS Linux Kernel Test");
MODULE_LICENSE("GPL");

static char *filename = "";
module_param(filename, charp, 0660);

static int __init KernelTestInit(void)
{
    CamFsFd tFD;
    void *pBuf = NULL;
    u32 nReadRet = 0;
    s32 filelen;

    CamOsPrintf("Test CamFs Start\n");

    if (CAM_FS_OK != CamFsOpen(&tFD, filename, O_RDONLY, 0))
    {
        CamOsPrintf("Open %s FAIL\n", filename);
    }
    else
    {
        CamOsPrintf("Open %s SUCCESS\n", filename);

        filelen = CamFsSeek(tFD, 0, SEEK_END);
        CamOsPrintf("file len: %d\n", filelen);
        CamFsSeek(tFD, 0, SEEK_SET);
        pBuf = CamOsMemAlloc(filelen);
        nReadRet = CamFsRead(tFD, pBuf, filelen);
        CamOsHexdump(pBuf, filelen);
        CamOsMemRelease(pBuf);

        CamFsClose(tFD);
    }

    CamOsPrintf("Test CamFs End\n");
    return 0;
}

static void __exit KernelTestExit(void)
{
    CamOsPrintf(KERN_INFO "Goodbye\n");
}

module_init(KernelTestInit);
module_exit(KernelTestExit);
