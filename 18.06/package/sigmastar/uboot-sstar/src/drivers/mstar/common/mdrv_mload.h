/*
* mdrv_mload.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: ryan.hsiao <ryan.hsiao@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/


#ifndef MDRV_MLOAD_H_
#define MDRV_MLOAD_H_

#include <mload_common.h>
#include <hal_mload.h>

typedef void*  MLOAD_HANDLE;

//-------------------------------------

typedef enum
{
    CMDQ_MODE,
    RIU_MODE
}FrameSyncMode;

typedef struct
{
    FrameSyncMode mode;
    MloadCmdqIf_t *pCmqInterface_t;
}MLOAD_ATTR;

//-------------------------------------

typedef enum
{
    MLOAD_ID_ACT_FRAME_ACTIVE = 0x01,
    MLOAD_ID_ACT_FRAME_BLANK = 0x02,
    MLOAD_ID_ACT_SKIP_WAIT_HW_NOT_BUSY = 0x40,
    MLOAD_ID_ACT_FORCE = 0x80,
}MLOAD_ID_ACT_ENUM;

//-------------------------------------

MLOAD_HANDLE IspMLoadInit(void);
int IspMLoadDeInit(MLOAD_HANDLE handle);

void IspMLoadReleaseModule(void);

int IspMLoadNextFrame(MLOAD_HANDLE handle);
int IspMLoadFreeFrame(MLOAD_HANDLE handle, int iToFreeFrameIndex);

int IspMLoadSetMemAllocator(MloadMemAllocator_t *ptMemAllocator);

MloadCmdqIf_t* IspMLoadChangeCmdqIF(MLOAD_HANDLE handle, MloadCmdqIf_t* pNew);

int IspMLoadTableSet(MLOAD_HANDLE handle, int id, void *table);
const u16* IspMLoadTableGet(MLOAD_HANDLE handle, int id);
int IspMLoadApply(MLOAD_HANDLE handle, int nReserved);
int SclMLoadApply(MLOAD_HANDLE handle);

int IspMloadCombieSet(MLOAD_HANDLE handle, void *pMloadShd, MloadCmdqIf_t *pCmdqIF, MLOAD_ID_ACT_ENUM eMloadIdActType);

void IspMloadUT(MHAL_CMDQ_CmdqInterface_t *cmdq);

int mload_init (void);
void mload_cleanup(void);

#endif //MDRV_MLOAD_H_
