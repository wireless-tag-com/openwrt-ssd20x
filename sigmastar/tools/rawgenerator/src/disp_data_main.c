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

#include "mhal_pnl_datatype.h"
#include "CC0702I50R_1024x600.h"
#include "FRD720X720BK_MIPI.h"

typedef enum
{
    EN_DISPLAY_DEVICE_NULL,
    EN_DISPLAY_DEVICE_LCD,
    EN_DISPLAY_DEVICE_HDMI,
    EN_DISPLAY_DEVICE_VGA
}SS_SHEADER_DisplayDevice_e;
typedef struct
{
    SS_SHEADER_DataInfo_t stDataInfo;
    u32 u32FirstUseOffset;
    u32 u32DispBufSize;
    u32 u32DispBufStart;
}SS_SHEADER_DispInfo_t;

typedef struct
{
    SS_SHEADER_DisplayDevice_e enDevice;
    u8 au8ResName[32];
    u32 u32Width;
    u32 u32Height;
    u32 u32Clock;
}SS_SHEADER_DispConfig_t;

typedef struct __attribute__((packed)) {
    SS_SHEADER_DisplayDevice_e enDevice;
    u8 au8PanelName[32];
    MhalPnlParamConfig_t stPnlParaCfg;
    MhalPnlMipiDsiConfig_t stMipiDsiCfg;
}SS_SHEADER_PnlPara_t;

typedef struct
{
    const char *pName;
    MhalPnlParamConfig_t *pstMPnlParaConfig;
    MhalPnlMipiDsiConfig_t *pMipiDsiConfig;
}SS_SHEADER_TableHandler_t;


#ifndef ALIGN_UP
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
#endif
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, alignment) (( (val)/(alignment))*(alignment))
#endif


static unsigned Atoi(const char * pStr)
{
    int intStrLen = strlen(pStr);
    unsigned short bUseHex = 0;
    unsigned int intRetNumber = 0;
    int i = 0;

    if (pStr == NULL)
    {
        return 0xFFFFFFFF;
    }

    if (intStrLen > 2)
    {
        if (pStr[0] == '0' &&(pStr[1] == 'X' || pStr[1] == 'x'))
        {
            bUseHex = 1;
            pStr += 2;
        }
    }
    if (bUseHex == 1)
    {
        for (i = 0; i < intStrLen - 2; i++)
        {
            if ((pStr[i] > '9' || pStr[i] < '0')    \
                && (pStr[i] > 'f' || pStr[i] < 'a') \
                && (pStr[i] > 'F' || pStr[i] < 'A'))
            {
                return 0xFFFFFFFF;
            }
        }
        sscanf(pStr, "%x", &intRetNumber);
    }
    else
    {
        for (i = 0; i < intStrLen; i++)
        {
            if (pStr[i] > '9' || pStr[i] < '0')
            {
                return 0xFFFFFFFF;
            }
        }
        intRetNumber =  atoi(pStr);
    }
    return intRetNumber;
}
#ifdef __x86_64__
    #You must use gcc xxx -m32 for 32bit cpu!!!!!
#endif

static s32 SS_SHEADER_InsertTimmingTable(FILE *fp, SS_SHEADER_DispInfo_t *pstDispHead, SS_SHEADER_DispConfig_t *pstTable, u32 u32TableCnt, const s8 *pFirstUse)
{
    u32 i = 0;
    SS_SHEADER_DispConfig_t stDispCfg;

    pstDispHead->u32FirstUseOffset = ftell(fp);
    for (i = 0; i < u32TableCnt; i++)
    {
        if (!strcmp(pFirstUse, (s8 *)pstTable[i].au8ResName))
        {
            pstDispHead->u32FirstUseOffset = ftell(fp);
        }
        memcpy(&stDispCfg, &pstTable[i], sizeof(SS_SHEADER_DispConfig_t));
        fwrite(&stDispCfg, 1, sizeof(SS_SHEADER_DispConfig_t), fp);
        pstDispHead->stDataInfo.u32DataTotalSize += sizeof(SS_SHEADER_DispConfig_t);
        pstDispHead->stDataInfo.u32SubNodeCount++;
    }
    printf("Data count %d, total size %d(0x%x), first use addr %d(0x%x)\n", pstDispHead->stDataInfo.u32SubNodeCount,
                                                                            pstDispHead->stDataInfo.u32DataTotalSize,
                                                                            pstDispHead->stDataInfo.u32DataTotalSize,
                                                                            pstDispHead->u32FirstUseOffset,
                                                                            pstDispHead->u32FirstUseOffset);

    return 0;
}
static s32 SS_SHEADER_InsertPanelTable(FILE *fp, SS_SHEADER_DispInfo_t *pstDispHead, SS_SHEADER_TableHandler_t *pstTabelHandler, u32 u32TableCnt, const s8 *pFirstUse)
{
    u32 i = 0;
    SS_SHEADER_PnlPara_t stPnlPara;
    s32 s32Alignment = 0xFF;
    s32 s32AlignSize = 0;

    pstDispHead->u32FirstUseOffset = ftell(fp);
    for (i = 0; i < u32TableCnt; i++)
    {
        memset(&stPnlPara, 0, sizeof(SS_SHEADER_PnlPara_t));
        stPnlPara.enDevice = EN_DISPLAY_DEVICE_LCD;
        if (!strcmp(pFirstUse, pstTabelHandler[i].pName))
        {
            pstDispHead->u32FirstUseOffset = ftell(fp);
        }
        if (pstTabelHandler[i].pstMPnlParaConfig)
        {
            memcpy(&stPnlPara.stPnlParaCfg, pstTabelHandler[i].pstMPnlParaConfig, sizeof(MhalPnlParamConfig_t));
        }
        if (pstTabelHandler[i].pMipiDsiConfig)
        {
            memcpy(&stPnlPara.stMipiDsiCfg, pstTabelHandler[i].pMipiDsiConfig, sizeof(MhalPnlMipiDsiConfig_t));
        }
        strncpy((s8 *)stPnlPara.au8PanelName, pstTabelHandler[i].pName, 20);
        fwrite(&stPnlPara, 1, sizeof(SS_SHEADER_PnlPara_t), fp);
        pstDispHead->stDataInfo.u32DataTotalSize += sizeof(SS_SHEADER_PnlPara_t);
        printf("Insert disp name: %s\n",stPnlPara.au8PanelName);
        if (stPnlPara.stMipiDsiCfg.u32CmdBufSize != 0)
        {
            fwrite(stPnlPara.stMipiDsiCfg.pu8CmdBuf, 1, stPnlPara.stMipiDsiCfg.u32CmdBufSize, fp);
            pstDispHead->stDataInfo.u32DataTotalSize += stPnlPara.stMipiDsiCfg.u32CmdBufSize;
        }
        printf("Disp header size (%d) mhal pnl para size(%d) spi cmd size(%d)\n", sizeof(SS_SHEADER_PnlPara_t), sizeof(MhalPnlParamConfig_t), stPnlPara.stMipiDsiCfg.u32CmdBufSize);
        pstDispHead->stDataInfo.u32SubNodeCount++;
        /*Warning!!!!! uboot ddr memory start address must be 4 byte alignment*/
        s32AlignSize = ALIGN_UP(pstDispHead->stDataInfo.u32DataTotalSize, 4) - pstDispHead->stDataInfo.u32DataTotalSize;
        if (s32AlignSize)
        {
            fwrite(&s32Alignment, 1, s32AlignSize, fp);
            pstDispHead->stDataInfo.u32DataTotalSize += s32AlignSize;
        }
    }
    printf("Data count %d, total size %d(0x%x), first use addr %d(0x%x)\n", pstDispHead->stDataInfo.u32SubNodeCount,
                                                                            pstDispHead->stDataInfo.u32DataTotalSize,
                                                                            pstDispHead->stDataInfo.u32DataTotalSize,
                                                                            pstDispHead->u32FirstUseOffset,
                                                                            pstDispHead->u32FirstUseOffset);

    return 0;
}
static s32 SS_SHEADER_InsertDispTable(FILE *fp, u64 u64Start, u32 u32Size, SS_SHEADER_DisplayDevice_e enDevice, const s8 *pFirstUse)
{
    s32 s32BeginPos = 0;
    s32 s32CurPos = 0;
    SS_SHEADER_DispInfo_t stDispInfo;
    SS_SHEADER_DispConfig_t stTimmingTable[] = {{EN_DISPLAY_DEVICE_HDMI, "HDMI_1080P60", 1920, 1080, 60},
                                                {EN_DISPLAY_DEVICE_HDMI, "HDMI_720P60", 1280, 720, 60},
                                                {EN_DISPLAY_DEVICE_VGA, "VGA_1080P60", 1920, 1080, 60},
                                                {EN_DISPLAY_DEVICE_VGA, "VGA_720P60", 1280, 720, 60}};
    SS_SHEADER_TableHandler_t stTable[] = { {"CC0702I50R", &stPanel_CC0702I50R_1024x600, NULL},
                                            {"FRD720X720BK", &stPanel_FRD720X720BK, &stMipiDsiConfig_FRD720X720BK},};

    memset(&stDispInfo, 0, sizeof(SS_SHEADER_DispInfo_t));
    stDispInfo.u32DispBufStart = u64Start;
    stDispInfo.u32DispBufSize = u32Size;
    s32CurPos = ftell(fp);
    printf("LOGO header size %d\n", sizeof(SS_SHEADER_DispInfo_t));
    memcpy(stDispInfo.stDataInfo.au8DataInfoName, "DISP", 4);
    stDispInfo.stDataInfo.u32SubHeadSize = sizeof(SS_SHEADER_DispInfo_t);
    fwrite(&stDispInfo, 1, sizeof(SS_SHEADER_DispInfo_t), fp);
    switch (enDevice)
    {
        case EN_DISPLAY_DEVICE_LCD:
        {
            printf("First use: %s\n", pFirstUse);
            SS_SHEADER_InsertPanelTable(fp, &stDispInfo, stTable, sizeof(stTable) / sizeof(SS_SHEADER_TableHandler_t), (const s8 *)pFirstUse);
        }
        break;
        case EN_DISPLAY_DEVICE_HDMI:
        case EN_DISPLAY_DEVICE_VGA:
        {
            printf("First use: %s\n", pFirstUse);
            SS_SHEADER_InsertTimmingTable(fp, &stDispInfo, stTimmingTable, sizeof(stTimmingTable) / sizeof(SS_SHEADER_DispConfig_t), (const s8 *)pFirstUse);
        }
        break;
        default:
            return -1;
    }
    s32BeginPos = ftell(fp);
    fseek(fp, s32CurPos, SEEK_SET);
    fwrite(&stDispInfo, 1, sizeof(SS_SHEADER_DispInfo_t), fp);
    fseek(fp, s32BeginPos, SEEK_SET);
    printf("dipslay_start=0x%08x\n", (stDispInfo.u32DispBufStart));
    printf("dipslay_size=0x%08x\n", (unsigned int)(stDispInfo.u32DispBufSize));

    return 0;
}

void printHelp(void)
{
    printf("USAGE:");
    return;
}
int main(int argc, char *argv[]) {
    s32 s32Result;
    s8 s8UsedName[20];
    s8 s8DstFile[256];
    FILE *fp = NULL;
    u64 u64Start = 0;
    u32 u32Size = 0;
    u8 bCreate = 0;
    u8 bAppend = 0;
    SS_HEADER_Desc_t stHeader;
    SS_SHEADER_DisplayDevice_e enInterface = 0;

    memset(&stHeader, 0, sizeof(SS_HEADER_Desc_t));
    memset(&s8UsedName, 0, 20);
    memset(s8DstFile, 0, 256);

    while((s32Result = getopt(argc, argv, "p:s:d:o:ca")) != -1 )
    {
        switch(s32Result) {
        case 'p': {
            u64Start = Atoi(optarg);
        }
        break;
        case 's': {
            u32Size = Atoi(optarg);
        }
        break;
        case 'd': {
            if (!strncasecmp(optarg, "HDMI", 4))
            {
                strncpy(s8UsedName, optarg, 20);
                enInterface = EN_DISPLAY_DEVICE_HDMI;
            }
            else if (!strncasecmp(optarg, "VGA", 3))
            {
                strncpy(s8UsedName, optarg, 20);
                enInterface = EN_DISPLAY_DEVICE_VGA;
            }
            else
            {
                strncpy(s8UsedName, optarg, 20);
                enInterface = EN_DISPLAY_DEVICE_LCD;
            }
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
        printf("Open error\n");
        return -1;
    }
    SS_SHEADER_InsertDispTable(fp, u64Start, u32Size, enInterface, s8UsedName);
    stHeader.u32DataInfoCnt++;
    SS_HEADER_Update(fp, &stHeader);
    SS_HEADER_Close(fp);


    return 0;
}
