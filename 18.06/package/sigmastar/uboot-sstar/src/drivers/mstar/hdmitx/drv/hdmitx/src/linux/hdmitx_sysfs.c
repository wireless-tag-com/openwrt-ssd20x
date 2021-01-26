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

#define _DISP_SYSFS_C_
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/device.h>

#include "mhal_common.h"
#include "drv_hdmitx_os.h"
#include "cam_sysfs.h"
#include "hdmitx_debug.h"
#include "hal_hdmitx_chip.h"
#include "mhal_hdmitx_datatype.h"
#include "mhal_hdmitx.h"
#include "drv_hdmitx_if.h"


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HDMITXSYSFS_SPRINTF_STRCAT(str, _fmt, _args...) \
    do {                                   \
        char tmpstr[1024];                 \
        sprintf(tmpstr, _fmt, ## _args);   \
        strcat(str, tmpstr);               \
    }while(0)

#define HDMITXSYSFS_SPRINTF(str, _fmt, _args...)  sprintf(str, _fmt, ## _args)


#define HDMITXSYSFS_PRINTF(_fmt, _args...)                 \
        do{                                         \
            printf(PRINT_GREEN _fmt PRINT_NONE, ## _args);       \
        }while(0)

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int argc;
    char *argv[100];
}HdmitxSysFsStrConfig_t;


//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
bool bPtGenEn = 0;
u8   u8PtGenWidth = 0;
u32 u32HdmitxDbgmgLevel = 0;


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
int _HdmitxSysFsSplit(char **arr, char *str,  char* del)
{
    char *cur = str;
    char *token = NULL;
    int cnt = 0;

    token = strsep(&cur, del);
    while (token)
    {
        arr[cnt] = token;
        token = strsep(&cur, del);
        cnt++;
    }
    return cnt;
}

void _HdmitxDbgmgStore(HdmitxSysFsStrConfig_t *pstStringCfg)
{
    int ret;
    void *pHdmitxCtx = NULL;

    if(pstStringCfg->argc < 1)
    {
        return;
    }

    ret = kstrtol(pstStringCfg->argv[0], 16, (long *)&u32HdmitxDbgmgLevel);

    if(MhalHdmitxGetInstance(&pHdmitxCtx, 0) != E_MHAL_HDMITX_RET_SUCCESS)
    {
        if(MhalHdmitxGetInstance(&pHdmitxCtx, 1) != E_MHAL_HDMITX_RET_SUCCESS)
        {
            MhalHdmitxCreateInstance(&pHdmitxCtx, 1);
            if(pHdmitxCtx == NULL)
            {
                HDMITX_ERR("%s %d, No Instance Create\n", __FUNCTION__, __LINE__);
                return;
            }
        }
    }

    MhalHdmitxSetDebugLevel(pHdmitxCtx, u32HdmitxDbgmgLevel);

    HDMITXSYSFS_PRINTF("%s %d, DebugLevel=%x\n", __FUNCTION__, __LINE__, u32HdmitxDbgmgLevel);
}

int _HdmitxDbgmgShow(char *DstBuf)
{
    int RetSprintf = -1;
    char *SrcBuf;

    SrcBuf = (char *)DrvHdmitxOsMemAlloc(1024*5);

    if(SrcBuf)
    {
        HDMITXSYSFS_SPRINTF_STRCAT(SrcBuf, "dbglevle: %08x \n", (unsigned int)u32HdmitxDbgmgLevel);
        RetSprintf = HDMITXSYSFS_SPRINTF(DstBuf, "%s", SrcBuf);
        DrvHdmitxOsMemRelease(SrcBuf);
    }
    return RetSprintf;
}


void _HdmitxPtGenStore(HdmitxSysFsStrConfig_t *pstStringCfg)
{
    #define HDMITXREG(bank, addr)              (*((volatile u16 *)(((u32)0xFD000000+(bank<<1U)) + ((addr)<<2U))))

    int ret;
    u16 u16Val;

    if(pstStringCfg->argc == 2)
    {
        ret = kstrtol(pstStringCfg->argv[0], 10, (long *)&bPtGenEn);
        ret = kstrtol(pstStringCfg->argv[1], 10, (long *)&u8PtGenWidth);

        u16Val = (bPtGenEn) ? (u8PtGenWidth << 8 | 0x03) : 0x0000;
        HDMITXREG(0x112500, 0x05) = u16Val;
    }
    else
    {
        HDMITXSYSFS_PRINTF("[En] [Pt Wdith]\n");
    }
}

int _HdmitxPtGenShow(char *DstBuf)
{
    int RetSprintf = -1;
    char *SrcBuf;

    SrcBuf = (char *)DrvHdmitxOsMemAlloc(1024*5);

    if(SrcBuf)
    {
        HDMITXSYSFS_SPRINTF_STRCAT(SrcBuf, "ptgen=%d %d \n", bPtGenEn, u8PtGenWidth);
        RetSprintf = HDMITXSYSFS_SPRINTF(DstBuf, "%s", SrcBuf);
        DrvHdmitxOsMemRelease(SrcBuf);
    }
    return RetSprintf;
}

void _HdmitxClkStore(HdmitxSysFsStrConfig_t *pstStringCfg)
{
    int ret, idx;
    bool Enable = 0;
    u32 ClkRate = 0;
    char *ClkName = NULL;
    bool abClkEn[HAL_HDMITX_CLK_NUM] = HAL_HDMITX_CLK_ON_SETTING;
    u32  au32ClkRate[HAL_HDMITX_CLK_NUM] = HAL_HDMITX_CLK_RATE_SETTING;
    void *pHdmitxCtx = NULL;
    u32 i;

    if(pstStringCfg->argc == HAL_HDMITX_CLK_NUM + 2)
    {
        ClkName = pstStringCfg->argv[0];
        ret = kstrtol(pstStringCfg->argv[1], 10, (long *)&Enable);
        for(i=0; i<HAL_HDMITX_CLK_NUM; i++)
        {
            ret = kstrtol(pstStringCfg->argv[i+2], 10, (long *)&au32ClkRate[i]);
        }
    }
    else if(pstStringCfg->argc == 3)
    {
        ClkName = pstStringCfg->argv[0];
        ret = kstrtol(pstStringCfg->argv[1], 10, (long *)&Enable);
        ret = kstrtol(pstStringCfg->argv[2], 10, (long *)&ClkRate);
    }
    else
    {
        HDMITXSYSFS_PRINTF("-----------------  CLK TREE  -----------------\n" );
        HDMITXSYSFS_PRINTF("clktree [en] [hdmi] [disp432] [disp216]\n");
        HDMITXSYSFS_PRINTF("----------------- DRV Update -----------------\n");
        HDMITXSYSFS_PRINTF("[clk type] [en] [clk rate] \n");
        HDMITXSYSFS_PRINTF("clk type: hdmi, disp432, disp216\n" );
        return;
    }

    if(strcmp(ClkName, "clktree") == 0)
    {
        if(Enable)
        {
            if(DrvHdmitxOsSetClkOn(au32ClkRate, HAL_HDMITX_CLK_NUM) == 0)
            {
                HDMITX_ERR("%s %d, Set Clk On Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            if(DrvHdmitxOsSetClkOff() == 0)
            {
                HDMITX_ERR("%s %d, Set Clk Off Fail\n", __FUNCTION__, __LINE__);
            }
        }

    }
    else
    {
        if(MhalHdmitxGetInstance(&pHdmitxCtx, 0) != E_MHAL_HDMITX_RET_SUCCESS)
        {
            if(MhalHdmitxGetInstance(&pHdmitxCtx, 1) != E_MHAL_HDMITX_RET_SUCCESS)
            {
                MhalHdmitxCreateInstance(&pHdmitxCtx, 1);
                if(pHdmitxCtx == NULL)
                {
                    HDMITX_ERR("%s %d, No Instance Create\n", __FUNCTION__, __LINE__);
                    return;
                }
            }
        }

        if(DrvHdmitxIfGetClk(pHdmitxCtx, abClkEn, au32ClkRate, HAL_HDMITX_CLK_NUM) != E_MHAL_HDMITX_RET_SUCCESS)
        {
            HDMITX_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
            return;
        }

        idx = (strcmp(ClkName, "hdmi")    == 0) ? 0 :
              (strcmp(ClkName, "disp432") == 0) ? 1 :
              (strcmp(ClkName, "disp216") == 0) ? 2 :
                                                  0;
        if(idx < HAL_HDMITX_CLK_NUM)
        {
            abClkEn[idx] = Enable,
            au32ClkRate[idx] = ClkRate;
            if(DrvHdmitxIfSetClk(pHdmitxCtx, abClkEn, au32ClkRate, HAL_HDMITX_CLK_NUM) != E_MHAL_HDMITX_RET_SUCCESS)
            {
                HDMITX_ERR("%s %d, Set Clk Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            HDMITX_ERR("%s %d, Unknown clk type: %s\n", __FUNCTION__, __LINE__, ClkName);
        }
    }
}

int _HdmitxClkShow(char *DstBuf)
{
    bool abEn[HAL_HDMITX_CLK_NUM];
    u32 au32ClkRate[HAL_HDMITX_CLK_NUM];
    int RetSprintf = -1;
    char *SrcBuf;
    void *pHdmitxCtx = NULL;
    char aCLKName[][20] = {"CLK_HDMI", "CLK_DISP_432", "CLK_DISP_216"};
    u32 i;

    SrcBuf = (char *)DrvHdmitxOsMemAlloc(1024*5);

    if(SrcBuf)
    {
        if(MhalHdmitxGetInstance(&pHdmitxCtx, 0) != E_MHAL_HDMITX_RET_SUCCESS)
        {
            if(MhalHdmitxGetInstance(&pHdmitxCtx, 1) != E_MHAL_HDMITX_RET_SUCCESS)
            {
                MhalHdmitxCreateInstance(&pHdmitxCtx, 1);
            }
        }

        if(pHdmitxCtx)
        {
            if(DrvHdmitxIfGetClk(pHdmitxCtx, abEn, au32ClkRate, HAL_HDMITX_CLK_NUM) != E_MHAL_HDMITX_RET_SUCCESS)
            {
                HDMITX_ERR("%s %d, Get CLK Fail\n", __FUNCTION__, __LINE__);
            }

            for(i=0; i<HAL_HDMITX_CLK_NUM; i++)
            {
                HDMITXSYSFS_SPRINTF_STRCAT(SrcBuf, "%-15s: En:%d, ClkRate:%d \n", aCLKName[i], abEn[i], au32ClkRate[i]);
            }
            RetSprintf = HDMITXSYSFS_SPRINTF(DstBuf, "%s", SrcBuf);
        }
        else
        {
            HDMITX_ERR("%s %d, No Instance \n", __FUNCTION__, __LINE__);
        }
        DrvHdmitxOsMemRelease(SrcBuf);

    }
    return RetSprintf;
}



void _HdmitxSysFsParsingCommand(char *str, HdmitxSysFsStrConfig_t *pstStrCfg)
{
    char del[] = " ";
    int len;

    pstStrCfg->argc = _HdmitxSysFsSplit(pstStrCfg->argv, (char *)str, del);
    len = strlen(pstStrCfg->argv[pstStrCfg->argc-1]);
    pstStrCfg->argv[pstStrCfg->argc-1][len-1] = '\0';
}



ssize_t check_hdmitxdbgmg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        HdmitxSysFsStrConfig_t stStringCfg;

        _HdmitxSysFsParsingCommand((char *)buf, &stStringCfg);
        _HdmitxDbgmgStore(&stStringCfg);
        return n;
    }
    return 0;
}

ssize_t check_hdmitxdbgmg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return _HdmitxDbgmgShow(buf);
}

static DEVICE_ATTR(dbgmg,0644, check_hdmitxdbgmg_show, check_hdmitxdbgmg_store);


ssize_t check_hdmitxptgen_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        HdmitxSysFsStrConfig_t stStringCfg;

        _HdmitxSysFsParsingCommand((char *)buf, &stStringCfg);
        _HdmitxPtGenStore(&stStringCfg);
        return n;
    }
    return 0;
}

ssize_t check_hdmitxptgen_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return _HdmitxPtGenShow(buf);
}

static DEVICE_ATTR(ptgen,0644, check_hdmitxptgen_show, check_hdmitxptgen_store);



ssize_t check_hdmitxclk_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        HdmitxSysFsStrConfig_t stStringCfg;

        _HdmitxSysFsParsingCommand((char *)buf, &stStringCfg);
        _HdmitxClkStore(&stStringCfg);
        return n;
    }
    return 0;
}

ssize_t check_hdmitxclk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return _HdmitxClkShow(buf);
}

static DEVICE_ATTR(clk, 0644, check_hdmitxclk_show, check_hdmitxclk_store);



void DrvHdmitxSysfsInit(struct device *device)
{
    int ret;

    ret = CamDeviceCreateFile(device, &dev_attr_dbgmg);
    ret = CamDeviceCreateFile(device, &dev_attr_ptgen);
    ret = CamDeviceCreateFile(device, &dev_attr_clk);
}
