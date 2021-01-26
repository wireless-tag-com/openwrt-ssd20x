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

#ifndef _DRV_DISP_OS_HEADER_H_
#define _DRV_DISP_OS_HEADER_H_


#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#include <stdlib.h>


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define CamOsPrintf  printf


#define WRITE_SCL_REG(addr, type, data)  ((*(volatile type *)(addr)) = (data))
#define READ_SCL_REG(addr, type)         ((*(volatile type *)(addr)))


#define READ_BYTE(x)         READ_SCL_REG(x, u8)
#define READ_WORD(x)         READ_SCL_REG(x, u16)
#define READ_LONG(x)         READ_SCL_REG(x, u32)
#define WRITE_BYTE(x, y)     WRITE_SCL_REG(x, u8, y)
#define WRITE_WORD(x, y)     WRITE_SCL_REG(x, u16, y)
#define WRITE_LONG(x, y)     WRITE_SCL_REG(x, u32, y)

#define DISP_OS_INTERNAL_ISR_SUPPORT           0
//-------------------------------------------------------------------------------------------------
//  Structure & Emu
//-------------------------------------------------------------------------------------------------

// Task
typedef void* (*TaskEntryCb)(void *argv);

typedef struct
{
    s32 s32Id;
} DrvDispOsTaskConfig_t;

#endif
