/*
* cam_fs_wrapper.c- Sigmastar
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
/// @file      cam_fs_wrapper.c
/// @brief     Cam FS Wrapper Source File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#endif

#ifdef CAM_OS_RTK
#include "stdio.h"
#include "sys_sys.h"
#include "drv_spinand.h"
#include "sys_MsWrapper_cus_os_mem.h"
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"

#define UBOOT_ENV_SIZE      2048

typedef struct
{
    union {
        u32 nFdType:8;  // 1: Block type, 2: Partition type
        struct {
            u32 nFdType:8;
            u32 nBlkNo:24;
        } tBlkType;
        struct {
            u32 nFdType:8;
            u32 nPartNo:8;
            u32 nPartOffset:16;
        } tPartType;
    };
    u32 nSize;
} CamFsFdRtk_t, *pCamFsFdRtk_t;

#elif defined(CAM_OS_LINUX_USER)
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"

#elif defined(CAM_OS_LINUX_KERNEL)
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#endif

#define FLASH_ACCESS_UNIT   131072

typedef struct
{
    u32 magic;
    u32 ver;
    u32 size;
    u32 align_unit;
    u32 file_num;
    u8  dummy[44];
} LwFsPartitionInfo_t;

typedef struct
{
    char    name[32];
    u32     offset;
    u32     length;
    u32     padding;
    u32     crc32;
    u32     compressed;
    u8      dummy[12];
} LwFsFileInfo_t;

#define LWFS_MNT_PATH   "/mnt/"
#define LWFS_HEADER_FIRST_READ  2048

CamFsRet_e CamFsOpen(CamFsFd *ptFd, const char *szPath, u32 nFlag, u32 nMode)
{
#ifdef CAM_OS_RTK
    /*
      There is no file system in RTK now, CamFs will access spinand device
      directly. CamFsOpen will use PBA number instead file descriptor.
    */
    CamFsFdRtk_t *ptFdRtk;
    char *pEnd = NULL;
    void *flash_buf = NULL;
    char *str;
    u32 str_len;
    char *pch_bootargs = NULL;
    u32 exclude_path_len = 0;
    char partition_name[64] = {0};
    char file_name[64] = {0};
    u32 i = 0;;
    u32 partition_offset = 0;
    u32 file_size = 0;
    LwFsPartitionInfo_t *p_info;
    LwFsFileInfo_t *f_info;
    u32 lwfs_header_size = 0;
    u32 nPartNo;
    CamFsRet_e eRet = CAM_FS_FAIL;

    if (strncmp(szPath,"/env/",5) == 0)
    {
        // Try find file store info from uboot env.

        // Allocate 64 byte aligned buffer for cache flush operation
        flash_buf = MsGetHeapMemoryExt(UBOOT_ENV_SIZE, 6, 0);
        if (!flash_buf)
        {
            CamOsPrintf("%s: alloc buf fail\n", __FUNCTION__);
            return CAM_FS_FAIL;
        }

        DrvSpinandProbe();
        MDrv_SPINAND_LoadBL((U8*)flash_buf, UBOOT_ENV_SIZE, UNFD_PART_ENV, 0);

        CamOsMemInvalidate(flash_buf, UBOOT_ENV_SIZE);

        exclude_path_len = strlen(szPath) - 5;
        str = (char *)flash_buf + 4;
        while ((str_len = strlen(str)) != 0)
        {
            if ((pch_bootargs = strstr(str, szPath + 5)) != NULL)
            {
                pch_bootargs += exclude_path_len + 1;
                break;
            }
            str = (char *)((u32)str + str_len + 1);
        }

        if (str_len == 0)   // File not found
        {
            eRet = CAM_FS_FAIL;
        }
        else
        {
            sscanf (pch_bootargs,"%s %d 0x%x", partition_name, &partition_offset, &file_size);

            ptFdRtk = (CamFsFdRtk_t *)CamOsMemCalloc(1, sizeof(CamFsFdRtk_t));
            ptFdRtk->nFdType = 2;
            ptFdRtk->tPartType.nPartNo = DrvSpinand_FindPartTypeByName(partition_name);
            ptFdRtk->tPartType.nPartOffset = partition_offset;
            ptFdRtk->nSize = file_size;
            *ptFd = (CamFsFd *)ptFdRtk;
            eRet = CAM_FS_OK;
        }

        MsReleaseHeapMemory(flash_buf);
    }
    else if (strncmp(szPath,"/blk/",5) == 0)
    {
        ptFdRtk = (CamFsFdRtk_t *)CamOsMemCalloc(1, sizeof(CamFsFdRtk_t));
        ptFdRtk->nFdType = 1;
        ptFdRtk->tBlkType.nBlkNo = strtoul(szPath + 5, &pEnd, 10);
        ptFdRtk->nSize = 0;
        *ptFd = (CamFsFd *)ptFdRtk;
        eRet = CAM_FS_OK;
    }
    else if (strncmp(szPath, LWFS_MNT_PATH, strlen(LWFS_MNT_PATH)) == 0)
    {
        i = 0;
        while(*((szPath+strlen(LWFS_MNT_PATH))+i) != '/' && *((szPath+strlen(LWFS_MNT_PATH))+i) != 0)
        {
            i++;
        }
        if (!i || *((szPath+strlen(LWFS_MNT_PATH))+i) == 0)
        {
            return CAM_FS_FAIL;
        }
        strncpy(partition_name, szPath+strlen(LWFS_MNT_PATH), CAM_OS_MIN(sizeof(partition_name)-1, i));
        partition_name[sizeof(partition_name)-1] = '\0';
        strncpy(file_name, szPath+strlen(LWFS_MNT_PATH)+i+1, sizeof(file_name)-1);
        file_name[sizeof(file_name)-1] = '\0';

        // Allocate 64 byte aligned buffer for cache flush operation
        flash_buf = MsGetHeapMemoryExt(LWFS_HEADER_FIRST_READ, 6, 0);
        if (!flash_buf)
        {
            CamOsPrintf("%s: alloc buf fail\n", __FUNCTION__);
            eRet = CAM_FS_FAIL;
            goto cam_fs_open_lwfs_end;
        }

        DrvSpinandProbe();
        nPartNo = DrvSpinand_FindPartTypeByName(partition_name);
        MDrv_SPINAND_LoadBL((u8 *)flash_buf, LWFS_HEADER_FIRST_READ, nPartNo, 0);
        CamOsMemInvalidate(flash_buf, UBOOT_ENV_SIZE);

        p_info = flash_buf;
        f_info = flash_buf + sizeof(LwFsPartitionInfo_t);

        if (p_info->magic != 0x5346574C)
        {
            CamOsPrintf("%s: %s not in LWFS format\n", __FUNCTION__, partition_name);
            eRet = CAM_FS_FAIL;
            goto cam_fs_open_lwfs_end;
        }

        lwfs_header_size = p_info->size;

        if (lwfs_header_size > LWFS_HEADER_FIRST_READ)
        {
            /* If LWFS header size greater than LWFS_HEADER_FIRST_READ,
               realloc and reload header. */
            MsReleaseHeapMemory(flash_buf);
            flash_buf = NULL;

            // Allocate 64 byte aligned buffer for cache flush operation
            flash_buf = MsGetHeapMemoryExt(lwfs_header_size, 6, 0);
            if (!flash_buf)
            {
                CamOsPrintf("%s: alloc buf fail\n", __FUNCTION__);
                eRet = CAM_FS_FAIL;
                goto cam_fs_open_lwfs_end;
            }

            DrvSpinandProbe();
            nPartNo = DrvSpinand_FindPartTypeByName(partition_name);
            MDrv_SPINAND_LoadBL((u8 *)flash_buf, LWFS_HEADER_FIRST_READ, nPartNo, 0);
            CamOsMemInvalidate(flash_buf, UBOOT_ENV_SIZE);

            p_info = flash_buf;
            f_info = flash_buf + sizeof(LwFsPartitionInfo_t);
        }

        for (i=0; i<p_info->file_num; i++)
        {
            if (strncmp(f_info[i].name, file_name, sizeof(f_info[i].name)) == 0)
                break;
        }

        if (i == p_info->file_num)
        {
            eRet = CAM_FS_FAIL;
            goto cam_fs_open_lwfs_end;
        }

        ptFdRtk = (CamFsFdRtk_t *)CamOsMemCalloc(1, sizeof(CamFsFdRtk_t));
        if (!ptFdRtk)
        {
            CamOsPrintf("%s: alloc buf fail\n", __FUNCTION__);
            eRet = CAM_FS_FAIL;
            goto cam_fs_open_lwfs_end;
        }
        ptFdRtk->nFdType = 3;
        ptFdRtk->tPartType.nPartNo = nPartNo;
        ptFdRtk->tPartType.nPartOffset = f_info[i].offset/FLASH_ACCESS_UNIT;
        ptFdRtk->nSize = f_info[i].length;
        *ptFd = (CamFsFd *)ptFdRtk;
        eRet = CAM_FS_OK;

cam_fs_open_lwfs_end:
        if (flash_buf)
        {
            MsReleaseHeapMemory(flash_buf);
            flash_buf = NULL;
        }
    }

    return eRet;
#elif defined(CAM_OS_LINUX_USER)
    if ((*ptFd = (CamFsFd *)open(szPath, nFlag, nMode)) >= 0)
        return CAM_FS_OK;
    else
        return CAM_FS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct file *ptFp = NULL;
    mm_segment_t tFs;

    tFs = get_fs();
    set_fs(get_ds());
    ptFp = filp_open(szPath, nFlag, nMode);
    set_fs(tFs);

    if(IS_ERR(ptFp))
    {
        *ptFd = NULL;
        return CAM_FS_FAIL;
    }
    else
    {
        *ptFd = (CamFsFd)ptFp;
        return CAM_FS_OK;
    }
#endif
}

CamFsRet_e CamFsClose(CamFsFd tFd)
{
#ifdef CAM_OS_RTK
    if (tFd)
    {
        CamOsMemRelease(tFd);
        return CAM_FS_OK;
    }
    else
    {
        return CAM_FS_FAIL;
    }
#elif defined(CAM_OS_LINUX_USER)
    if (!close((int)tFd))
        return CAM_FS_OK;
    else
        return CAM_FS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct file *ptFp = (struct file *)tFd;

    if (ptFp)
    {
        return (!filp_close(ptFp, NULL))? CAM_FS_OK : CAM_FS_FAIL;
    }
    else
    {
        return CAM_FS_FAIL;
    }
#endif
}

s32 CamFsRead(CamFsFd tFd, void *pBuf, u32 nCount)
{
#ifdef CAM_OS_RTK
    CamFsFdRtk_t *ptFdRtk = (CamFsFdRtk_t *)tFd;

    DrvSpinandProbe();

    if (ptFdRtk == NULL)
    {
        return 0;
    }

    if (ptFdRtk->nFdType == 1)          // Block number type
    {
        //DrvSpinand_ReadBlockPba((u8 *)pBuf, ptFdRtk->tBlkType.nBlkNo, nCount);
    }
    else if (ptFdRtk->nFdType == 2 || ptFdRtk->nFdType == 3)     // ENV type & LwFs type
    {
        MDrv_SPINAND_LoadBL((u8 *)pBuf, nCount, ptFdRtk->tPartType.nPartNo, ptFdRtk->tPartType.nPartOffset);
    }
    else
    {
        return 0;
    }

    return nCount;
#elif defined(CAM_OS_LINUX_USER)
    return read((int)tFd, pBuf, nCount);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct file *ptFp = (struct file *)tFd;
    mm_segment_t tFs;
    loff_t tPos;
    s32 nRet;

    if (ptFp)
    {
        tFs = get_fs();
        set_fs(get_ds());
        tPos = ptFp->f_pos;
        nRet = vfs_read(ptFp, pBuf, nCount, &tPos);
        ptFp->f_pos = tPos;
        set_fs(tFs);
        return nRet;
    }
    else
    {
        return -1;
    }
#endif
}

s32 CamFsWrite(CamFsFd tFd, const void *pBuf, u32 nCount)
{
#ifdef CAM_OS_RTK
    return 0;
#elif defined(CAM_OS_LINUX_USER)
    return write((int)tFd, pBuf, nCount);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct file *ptFp = (struct file *)tFd;
    mm_segment_t tFs;
    loff_t tPos;
    s32 nRet;

    if (ptFp)
    {
        tFs = get_fs();
        set_fs(get_ds());
        tPos = ptFp->f_pos;
        nRet = vfs_write(ptFp, pBuf, nCount, &tPos);
        ptFp->f_pos = tPos;
        set_fs(tFs);
        return nRet;
    }
    else
    {
        return -1;
    }
#endif
}

s32 CamFsSeek(CamFsFd tFd, u32 nOffset, u32 nWhence)
{
#ifdef CAM_OS_RTK
    CamFsFdRtk_t *ptFdRtk = (CamFsFdRtk_t *)tFd;
    s32 nRet = 0;

    if (tFd)
    {
        switch (nWhence)
        {
        case SEEK_SET:
            nRet = 0;   // Do nothing
            break;
        case SEEK_CUR:
            nRet = 0;   // Do nothing
            break;
        case SEEK_END:
            nRet = ptFdRtk->nSize;
            break;
        default:
            nRet = 0;
            break;
        }
    }
    else
    {
        nRet = -1;
    }

    return nRet;
#elif defined(CAM_OS_LINUX_USER)
    return lseek((int)tFd, nOffset, nWhence);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct file *ptFp = (struct file *)tFd;
    s32 ret = 999;
    ret = vfs_llseek(ptFp, nOffset, nWhence);
    return ret;
#endif
}
