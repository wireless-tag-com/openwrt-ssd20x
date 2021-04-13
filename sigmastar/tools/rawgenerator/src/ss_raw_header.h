/*
* main.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: malloc.peng <malloc.peng@sigmastar.com.cn>
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

#ifndef __SS_RAW_HEADER__
typedef char s8;
typedef short s16;
typedef int s32;
typedef unsigned char u8;
typedef unsigned char bool;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef struct
{
    u8 au8Tittle[8];
    u32 u32DataInfoCnt;
}SS_HEADER_Desc_t;
typedef struct
{
    u8 au8DataInfoName[32];
    u32 u32DataTotalSize;
    u32 u32SubHeadSize;
    u32 u32SubNodeCount;
}SS_SHEADER_DataInfo_t;

FILE *SS_HEADER_Create(const s8 *pPath, SS_HEADER_Desc_t *pstHeader);
FILE *SS_HEADER_OpenAppend(const s8 *pPath, SS_HEADER_Desc_t *pstHeader);
s32 SS_HEADER_Close(FILE *fp);
s32 SS_HEADER_Update(FILE *fp, SS_HEADER_Desc_t *pstHeader);
#endif
