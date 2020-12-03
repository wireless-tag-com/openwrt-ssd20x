/*
* cam_fs_wrapper.h- Sigmastar
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
/// @file      cam_fs_wrapper.h
/// @brief     Cam FS Wrapper Header File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#ifndef __CAM_FS_WRAPPER_H__
#define __CAM_FS_WRAPPER_H__

#define CAM_FS_WRAPPER_VERSION "v0.0.5"

#include "cam_os_wrapper.h"

#ifndef O_CLOEXEC
#define O_CLOEXEC   02000000
#endif
#ifndef O_NOATIME
#define O_NOATIME   01000000
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW  00400000
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 00200000
#endif
#ifndef O_LARGEFILE
#define O_LARGEFILE 00100000
#endif
#ifndef O_DIRECT
#define O_DIRECT    00040000
#endif
#ifndef FASYNC
#define FASYNC      00020000
#endif
#ifndef O_DSYNC
#define O_DSYNC     00010000
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK  00004000
#endif
#ifndef O_APPEND
#define O_APPEND    00002000
#endif
#ifndef O_TRUNC
#define O_TRUNC     00001000
#endif
#ifndef O_NOCTTY
#define O_NOCTTY    00000400
#endif
#ifndef O_EXCL
#define O_EXCL      00000200
#endif
#ifndef O_CREAT
#define O_CREAT     00000100
#endif
#ifndef O_PATH
#define O_PATH      010000000
#endif
#ifndef O_RDWR
#define O_RDWR      00000002
#endif
#ifndef O_WRONLY
#define O_WRONLY    00000001
#endif
#ifndef O_RDONLY
#define O_RDONLY    00000000
#endif
#ifndef O_ACCMODE
#define O_ACCMODE   00000003
#endif
#ifndef O_SYNC
#define __O_SYNC    04000000
#define O_SYNC      (__O_SYNC|O_DSYNC)
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif
#ifndef SEEK_END
#define SEEK_END    2
#endif

typedef enum
{
    CAM_FS_OK               = 0,
    CAM_FS_FAIL             = -1,
} CamFsRet_e;

typedef void * CamFsFd;


//=============================================================================
// Description:
//      Get cam_os_wrapper version with C string format.
// Parameters:
//      [in]  ptFd: Pointer to file descriptor.
//      [in]  szPath: Point to a pathname naming the file.
//      [in]  nFlag: File status flags.
//      [in]  nMode: File access modes.
// Return:
//      CAM_FS_OK on success. On error, CAM_FS_FAIL is returned.
//=============================================================================
CamFsRet_e CamFsOpen(CamFsFd *ptFd, const char *szPath, u32 nFlag, u32 nMode);

//=============================================================================
// Description:
//      Get cam_os_wrapper version with C string format.
// Parameters:
//      [in]  tFd: File descriptor.
// Return:
//      CAM_FS_OK on success. On error, CAM_FS_FAIL is returned.
//=============================================================================
CamFsRet_e CamFsClose(CamFsFd tFd);

//=============================================================================
// Description:
//      Get cam_os_wrapper version with C string format.
// Parameters:
//      [in]  tFd: File descriptor.
//      [in]  pBuf: Pointer to the buffer start address.
//      [in]  nByte: Read up to nCount bytes from file descriptor nFd.
// Return:
//      On success, the number of bytes read is returned. On error, -1 is returned.
//=============================================================================
s32 CamFsRead(CamFsFd tFd, void *pBuf, u32 nCount);

//=============================================================================
// Description:
//      Get cam_os_wrapper version with C string format.
// Parameters:
//      [in]  tFd: File descriptor.
//      [in]  pBuf: Pointer to the buffer start address.
//      [in]  nByte: Write up to nCount bytes to the file referred to by the file
//                   descriptor nFd.
// Return:
//      On success, the number of bytes written is returned (zero indicates nothing
//      was written). On error, -1 is returned.
//=============================================================================
s32 CamFsWrite(CamFsFd tFd, const void *pBuf, u32 nCount);

//=============================================================================
// Description:
//      Reposition read/write file offset
// Parameters:
//      [in]  tFd: File descriptor.
//      [in]  nOffset: Number of bytes to offset from nWhence.
//      [in]  nWhence: Position used as reference for the offset.
//                     ---------------------------------------------------
//                     | Constant |          Reference position          |
//                     ---------------------------------------------------
//                     | SEEK_SET | Beginning of file                    |
//                     ---------------------------------------------------
//                     | SEEK_CUR | Current position of the file pointer |
//                     ---------------------------------------------------
//                     | SEEK_END | End of file                          |
//                     ---------------------------------------------------
// Return:
//      On success, returns the resulting offset location as measured in bytes
//      from the beginning of the file. On error, -1 is returned.
//=============================================================================
s32 CamFsSeek(CamFsFd tFd, u32 nOffset, u32 nWhence);

#endif /* __CAM_FS_WRAPPER_H__ */
