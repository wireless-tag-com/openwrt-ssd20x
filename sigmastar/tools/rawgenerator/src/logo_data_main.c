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
#include <fcntl.h>  //open
#include <unistd.h> //getopt
#include <string.h> //memset
#include <stdlib.h> //strtol
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ss_raw_header.h"

typedef struct
{
    SS_SHEADER_DataInfo_t stDataInfo;
}SS_SHEADER_PictureDataInfo_t;

#ifndef ALIGN_UP
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
#endif
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, alignment) (( (val)/(alignment))*(alignment))
#endif

static s32 _FileSize(const s8* ps8Name)
{
    struct stat stStatBuf;
    if(stat(ps8Name, &stStatBuf)==0)
        return stStatBuf.st_size;
    return -1;
}
static s32 SS_SHEADER_InsertPictureData(FILE *fp, const s8 *ps8SrcFile)
{
    FILE *fpSrc = NULL;
    s32 s32FileSize;
    s8 bufLogo[32];
    u32 u32Ret = 0;
    SS_SHEADER_PictureDataInfo_t stPictureInfo;
    s32 s32Alignment = 0xFF;
    s32 s32AlignSize = 0;

    printf("Ready to open logo file %s\n", ps8SrcFile);
    memset(&stPictureInfo, 0, sizeof(SS_SHEADER_PictureDataInfo_t));
    memcpy(stPictureInfo.stDataInfo.au8DataInfoName, "LOGO", 4);
    s32FileSize = _FileSize(ps8SrcFile);
    if (s32FileSize == -1)
    {
        printf("Get file size error!\n");
        
        return -1;
    }
    stPictureInfo.stDataInfo.u32DataTotalSize = (u32)ALIGN_UP(s32FileSize, 4);
    stPictureInfo.stDataInfo.u32SubHeadSize = sizeof(SS_SHEADER_PictureDataInfo_t);
    stPictureInfo.stDataInfo.u32SubNodeCount = 1;
    fwrite(&stPictureInfo, 1, sizeof(SS_SHEADER_PictureDataInfo_t), fp);
    printf("Get file size %d sub head size %d\n", s32FileSize, stPictureInfo.stDataInfo.u32SubHeadSize);
    fpSrc = fopen(ps8SrcFile, "r");
    if (!fpSrc)
    {
        perror("fopen");

        return -1;
    }
    do
    {
        u32Ret = (u32)fread(bufLogo, 1, 32, fpSrc);
        if (u32Ret < 0)
        {
            perror("fread");
            
            break;
        }
        fwrite(bufLogo, 1, u32Ret, fp);
    }while(u32Ret);
    /*Warning!!!!! uboot ddr memory start address must be 4 byte alignment*/
    s32AlignSize = ALIGN_UP(s32FileSize, 4) - s32FileSize;
    if (s32AlignSize)
    {
        fwrite(&s32Alignment, 1, s32AlignSize, fp);
        stPictureInfo.stDataInfo.u32DataTotalSize += s32AlignSize;
    }
    fclose(fpSrc);

    return 0;
}

#ifdef __x86_64__
    #You must use gcc xxx -m32 for 32bit cpu!!!!!
#endif
void printHelp(void)
{
    printf("USAGE:");

    return;
}
int main(int argc, char *argv[]) {
    s8 s8DstFile[256];
    s8 s8SrcFile[256];
    FILE *fp = NULL;
    u8 bCreate = 0;
    u8 bAppend = 0;
    SS_HEADER_Desc_t stHeader;
    s32 s32Result = 0;

    memset(&stHeader, 0, sizeof(SS_HEADER_Desc_t));
    memset(s8DstFile, 0, 256);

    while((s32Result = getopt(argc, argv, "i:o:x:y:r:ca")) != -1 )
    {
        switch(s32Result) {
        case 'i': {
            strncpy(s8SrcFile, optarg, 256);
        }
        break;
        case 'o': {
            strncpy(s8DstFile, optarg, 256);
        }
        break;
        case 'c': {
            bCreate = 1;
        }
        break;
        case 'a': {
            bAppend = 1;
        }
        break;
        default:
            printf("no argv");
            printHelp();

            return -1;
        }
    }
    if (!(bCreate ^ bAppend))
    {
        printHelp();

        return -1;
    }
    if (bCreate)
    {
        fp = SS_HEADER_Create(s8DstFile, &stHeader);
    }
    else if (bAppend)
    {
        fp = SS_HEADER_OpenAppend(s8DstFile, &stHeader);
    }
    if (fp == NULL)
    {
        printf("Open error");
        return -1;
    }

    SS_SHEADER_InsertPictureData(fp, s8SrcFile);
    stHeader.u32DataInfoCnt++;
    SS_HEADER_Update(fp, &stHeader);
    SS_HEADER_Close(fp);

    return 0;
}
