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

#define _PNL_SYSFS_C_
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/delay.h>
#include <linux/device.h>

#include "drv_pnl_os.h"
#include "cam_sysfs.h"
#include "hal_pnl_common.h"
#include "pnl_debug.h"
#include "hal_pnl_chip.h"
#include "hal_pnl_st.h"
#include "hal_pnl.h"
#include "hal_pnl_reg.h"
#include "mhal_pnl_datatype.h"
#include "mhal_pnl.h"
#include "drv_pnl_os.h"
#include "drv_pnl_if.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define PNLUT_SPRINTF_STRCAT(str, _fmt, _args...) \
    do {                                   \
        char tmpstr[1024];                 \
        sprintf(tmpstr, _fmt, ## _args);   \
        strcat(str, tmpstr);               \
    }while(0)

#define PNLUT_SPRINTF(str, _fmt, _args...)  sprintf(str, _fmt, ## _args)

#define PNLUT_DBG(_fmt, _args...)                 \
    do{                                         \
        CamOsPrintf(PRINT_GREEN _fmt PRINT_NONE, ## _args);       \
    }while(0)



//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int argc;
    char *argv[200];
}PnlSysFsStrConfig_t;


//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
u32 gu32DbgLevel = 0;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
int _PnlSysFsSplit(char **arr, char *str,  char* del)
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

void _PnlSysFsParsingString(char *str, PnlSysFsStrConfig_t *pstStrCfg)
{
    char del[] = " ";
    int len;

    pstStrCfg->argc = _PnlSysFsSplit(pstStrCfg->argv, (char *)str, del);
    len = strlen(pstStrCfg->argv[pstStrCfg->argc-1]);
    pstStrCfg->argv[pstStrCfg->argc-1][len-1] = '\0';

}


void _PnlDbgmgStore(PnlSysFsStrConfig_t *pstStringCfg)
{
    int ret;
    bool bParamSet = 0;

    if(pstStringCfg->argc == 1)
    {
        ret = kstrtol(pstStringCfg->argv[0], 16, (long *)&gu32DbgLevel);
        bParamSet = 1;
    }

    if(bParamSet)
    {
        MhalPnlSetDebugLevel((void *)&gu32DbgLevel);
        printk("dbg level=%lx\n", gu32DbgLevel);
    }
    else
    {
        PNLUT_DBG("[LEVEL]\n");
    }
}


int _PnlDbgmgShow(char *DstBuf)
{
    int RetSprintf = -1;
    char *SrcBuf;

    SrcBuf = (char *)DrvPnlOsMemAlloc(1024*3);

    if(SrcBuf)
    {
        PNLUT_SPRINTF_STRCAT(SrcBuf, "------------------- PNL DBGMG ------------------- \n");
        PNLUT_SPRINTF_STRCAT(SrcBuf, "DbgLvl: 0x%08lx\n", gu32DbgLevel);
        PNLUT_SPRINTF_STRCAT(SrcBuf, "  NONE:   0x00000000\n");
        PNLUT_SPRINTF_STRCAT(SrcBuf, "   DRV:   0x00000001\n");
        PNLUT_SPRINTF_STRCAT(SrcBuf, "   HAL:   0x00000002\n");
        PNLUT_SPRINTF_STRCAT(SrcBuf, "MODULE:   0x00000004\n");
        PNLUT_SPRINTF_STRCAT(SrcBuf, "  CTX :   0x00000008\n");

        RetSprintf = PNLUT_SPRINTF(DstBuf, "%s", SrcBuf);
        DrvPnlOsMemRelease(SrcBuf);
    }
    return RetSprintf;
}

//-----------------------------------------------------------------------------
void _PnlClkStore(PnlSysFsStrConfig_t *pstStringCfg)
{
    int ret, idx;
    char *pClkName = NULL;
    bool bEn = 0;
    u32 u3ClkRate = 0;
    bool bParamSet = 0;
    bool abEn[HAL_PNL_CLK_NUM] = HAL_PNL_CLK_OFF_SETTING;
    u32  au32ClkRate[HAL_PNL_CLK_NUM] = HAL_PNL_CLK_OFF_SETTING;
    u8   i;
    void *pPnlCtx = NULL;

    if(strcmp(pstStringCfg->argv[0], "clktree") == 0)
    {
        pClkName = pstStringCfg->argv[0];
        ret = kstrtol(pstStringCfg->argv[1], 10, (long *)&bEn);
        for(i=0; i<HAL_PNL_CLK_NUM; i++)
        {
            ret = kstrtol(pstStringCfg->argv[2+i], 10, (long *)&au32ClkRate[i]);
        }
        bParamSet = 1;
    }
    else if(pstStringCfg->argc == 3)
    {
        pClkName = pstStringCfg->argv[0];
        ret = kstrtol(pstStringCfg->argv[1], 10, (long *)&bEn);
        ret = kstrtol(pstStringCfg->argv[2], 10, (long *)&u3ClkRate);
        bParamSet = 1;
    }
    else
    {
        PNLUT_DBG("----------------- CLK TREE  -----------------\n");
        PNLUT_DBG("clktree [En] [hdmi] [dac] [scpix] [mipidis] [mipidsiabp] \n");
        PNLUT_DBG("----------------- DRV Update -----------------\n");
        PNLUT_DBG("[CLK Type] [En] [CLK Rate]\n");
        PNLUT_DBG("CLK Type: hdmi, dac, scpix, mipidis, mipidsiabp\n");
        return;
    }

    if(bParamSet)
    {
        if(strcmp(pClkName, "clktree") == 0)
        {
            if(bEn)
            {
                if(DrvPnlOsSetClkOn(au32ClkRate, HAL_PNL_CLK_NUM) == 0)
                {
                    PNL_ERR("%s %d Set Clk Tree On Fail\n", __FUNCTION__, __LINE__);
                }
            }
            else
            {
                if(DrvPnlOsSetClkOff() == 0)
                {
                    PNL_ERR("%s %d Set Clk Tree Off Fail\n", __FUNCTION__, __LINE__);
                }
            }
        }
        else
        {

            if(DrvPnlIfGetInstance(&pPnlCtx, E_MHAL_PNL_LINK_TTL, 0) == 0)
            {
                if(DrvPnlIfGetInstance(&pPnlCtx, E_MHAL_PNL_LINK_TTL, 1) == 0)
                {
                    DrvPnlIfCreateInstance(&pPnlCtx, E_MHAL_PNL_LINK_TTL, 1);
                }
            }

            if(pPnlCtx)
            {
                if(DrvPnlIfGetClkConfig(pPnlCtx, abEn, au32ClkRate, HAL_PNL_CLK_NUM) == 0)
                {
                    PNL_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
                }

                idx = (strcmp(pClkName, "hdmi") == 0)       ? 0 :
                      (strcmp(pClkName, "dac") == 0)        ? 1 :
                      (strcmp(pClkName, "scpix") == 0)      ? 2 :
                      (strcmp(pClkName, "mipidsi") == 0)    ? 3 :
                      (strcmp(pClkName, "mipidsiabp") == 0) ? 4 :
                                                              5;
                if(idx < HAL_PNL_CLK_NUM)
                {
                    abEn[idx] = bEn;
                    au32ClkRate[idx] = u3ClkRate;

                    if(DrvPnlIfSetClkConfig(pPnlCtx, abEn, au32ClkRate, HAL_PNL_CLK_NUM) == 0)
                    {
                        PNL_ERR("%s %d, Set Clk Fail\n", __FUNCTION__, __LINE__);
                    }
                }
                else
                {
                    PNL_ERR("%s %d, Unknown clk type: %s\n", __FUNCTION__, __LINE__, pClkName);
                }
            }
            else
            {
                PNL_ERR("%s %d, No Instance\n", __FUNCTION__, __LINE__);
            }
        }
    }
}


int _PnlClkShow(char *DstBuf)
{
    bool abEn[HAL_PNL_CLK_NUM] = HAL_PNL_CLK_OFF_SETTING;
    u32  au32ClkRate[HAL_PNL_CLK_NUM] = HAL_PNL_CLK_OFF_SETTING;
    u8   au8ClkName[HAL_PNL_CLK_NUM][20] = HAL_PNL_CLK_NAME;
    int RetSprintf = -1;
    char *SrcBuf;
    void *pPnlCtx = NULL;
    u8   i;

    SrcBuf = (char *)DrvPnlOsMemAlloc(1024*3);

    if(SrcBuf)
    {
        if(DrvPnlIfGetInstance(&pPnlCtx, E_MHAL_PNL_LINK_TTL, 0) == 0)
        {
            if(DrvPnlIfGetInstance(&pPnlCtx, E_MHAL_PNL_LINK_TTL, 1) == 0)
            {
                DrvPnlIfCreateInstance(&pPnlCtx, E_MHAL_PNL_LINK_TTL, 1);
            }
        }

        if(pPnlCtx)
        {
            if(DrvPnlIfGetClkConfig(pPnlCtx, abEn, au32ClkRate, HAL_PNL_CLK_NUM) == 0)
            {
                PNL_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
            }

            for(i=0; i<HAL_PNL_CLK_NUM; i++)
            {
                PNLUT_SPRINTF_STRCAT(SrcBuf, "%-20s: %d %ld\n", au8ClkName[i], abEn[i], au32ClkRate[i]);
            }

            RetSprintf = PNLUT_SPRINTF(DstBuf, "%s", SrcBuf);
        }
        else
        {
            PNL_ERR("%s %d, No Instance\n", __FUNCTION__, __LINE__);
        }
        DrvPnlOsMemRelease(SrcBuf);
    }
    return RetSprintf;
}



//-----------------------------------------------------------------------------

ssize_t check_pnldbgmg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        PnlSysFsStrConfig_t stStrCfg;

        _PnlSysFsParsingString((char *)buf, &stStrCfg);
        _PnlDbgmgStore(&stStrCfg);
        return n;
    }
    return 0;
}

ssize_t check_pnldbgmg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return _PnlDbgmgShow(buf);
}

static DEVICE_ATTR(dbgmg,0644, check_pnldbgmg_show, check_pnldbgmg_store);


ssize_t check_pnlclk_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        PnlSysFsStrConfig_t stStrCfg;

        _PnlSysFsParsingString((char *)buf, &stStrCfg);
        _PnlClkStore(&stStrCfg);
        return n;
    }
    return 0;
}

ssize_t check_pnlclk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return _PnlClkShow(buf);
}

static DEVICE_ATTR(clk,0644, check_pnlclk_show, check_pnlclk_store);




//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------

void DrvPnlSysfsInit(struct device *device)
{
    CamDeviceCreateFile(device, &dev_attr_dbgmg);
    CamDeviceCreateFile(device, &dev_attr_clk);
}
