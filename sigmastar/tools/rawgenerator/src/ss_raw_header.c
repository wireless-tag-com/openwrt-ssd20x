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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ss_raw_header.h"

FILE *SS_HEADER_Create(const s8 *pPath, SS_HEADER_Desc_t *pstHeader)
{
    FILE *fp = NULL;

    fp = fopen(pPath, "w+");
    if(fp == NULL)
    {
        perror("fopen");
        return NULL;
    }
    memcpy(pstHeader->au8Tittle, "SSTAR", 5);
    fwrite(pstHeader, 1, sizeof(SS_HEADER_Desc_t), fp);

    return fp;
}
FILE *SS_HEADER_OpenAppend(const s8 *pPath, SS_HEADER_Desc_t *pstHeader)
{
    FILE *fp = NULL;

    fp = fopen(pPath, "r+");
    if(fp == NULL)
    {
        perror("fopen");
        return NULL;
    }    
    fread(pstHeader, 1, sizeof(SS_HEADER_Desc_t), fp);
    fseek(fp, 0, SEEK_END);

    return fp;
}
s32 SS_HEADER_Close(FILE *fp)
{
    return fclose(fp);
}
s32 SS_HEADER_Update(FILE *fp, SS_HEADER_Desc_t *pstHeader)
{
    s32 s32CurPos = 0;

    s32CurPos = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fwrite(pstHeader, 1, sizeof(SS_HEADER_Desc_t), fp);
    fseek(fp, s32CurPos, SEEK_SET);

    return 0;
}
