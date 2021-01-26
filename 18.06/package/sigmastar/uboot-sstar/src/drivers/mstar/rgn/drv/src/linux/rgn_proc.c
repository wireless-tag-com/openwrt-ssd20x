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

#define RGN_SYSFS_C
#include "drv_gop.h"
#include "drv_osd.h"
#include "drv_cover.h"

typedef struct
{
    bool bGwinEn;
    DrvGopGwinConfig_t tGwinGenSet;
    DrvGopGwinAlphaConfig_t tGwinAblSet;
    u32 u32MemPitch;
    u8 *p8ConstantAlphaVal;
    u8 *p8Argb1555Alpha0Val;
    u8 *p8Argb1555Alpha1Val;
} _DrvGopGwinLocalSettingsConfig_t;

typedef struct
{
    DrvGopOutFmtType_e      eGopOutFmtSet;
    DrvGopDisplayModeType_e eGopDispModeSet;
    DrvGopWindowConfig_t    tGopSrcStrWinSet;
    DrvGopWindowConfig_t    tGopDstStrWinSet;
    DrvGopColorKeyConfig_t tGopColorKeySet;
    u8  u8GWinNum;
    DrvGopGwinSrcFmtType_e eSrcFmt;
    bool bMirror;
    bool bFlip;
    _DrvGopGwinLocalSettingsConfig_t tGwinLocSet[HAL_RGN_GOP_GWIN_NUM];
    DrvGopVideoTimingInfoConfig_t tVideoTiming;
} _DrvGopLocalSettingsConfig_t;
typedef struct
{
    bool bOsdEn;
    bool bBypassEn;
    bool bColorInv;
    DrvOsdColorInvParamConfig_t stColorInvCfg[E_DRV_OSD_AE_MAX];
 } _DrvOsdLocalSettingsConfig_t;
 
 typedef struct
 {
     bool bBwinEn;
     DrvCoverWindowConfig_t tWinSet;
     DrvCoverColorConfig_t tWinColorSet;
 } _DrvCoverWinLocalSettingsConfig_t;
 
 typedef struct
 {
     bool bDbEn;
     _DrvCoverWinLocalSettingsConfig_t tCoverWinsSet[E_DRV_COVER_WIN_ID_NUM];
 } _DrvCoverLocalSettingsConfig_t;


#include <linux/device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include "rgn_sysfs_st.h"
#include "ms_msys.h"
#include "cam_os_wrapper.h"
struct proc_dir_entry *gpRgnProcDir;
#define DRV_RGN_DEVICE_MAJOR    0x87
#define DRV_RGN_DEVICE_MINOR    0x66
#define DrvRgnScnprintf(buf, size, _fmt, _args...)        seq_printf(buf, _fmt, ## _args)
#define RGN_GOP_ID_SHIFTBIT 4
#define PARSING_RGN_PORT(x)       (x==0   ? "Port0" : \
                                    x==1 ? "Port1" : \
                                   x==2 ? "Port2" : \
                                    x==4 ? "DIP" : \
                                           "UNBIND")
#define PARSING_RGN_Bind(x)       (x==0   ? "GOP0" : \
                                        x==1 ? "GOP1" : \
                                       x==2 ? "GOP2" : \
                                               "UNBIND")
#define PARSING_RGN_GOPFMT(x)       (x==E_DRV_GOP_GWIN_SRC_FMT_ARGB4444   ? "ARGB4444" : \
                                     x==E_DRV_GOP_GWIN_SRC_FMT_RGB1555 ? "RGB1555" : \
                                     x==E_DRV_GOP_GWIN_SRC_FMT_I8_PALETTE ? "I8" : \
                                     x==E_DRV_GOP_GWIN_SRC_I4_PALETTE ? "I4" : \
                                     x==E_DRV_GOP_GWIN_SRC_I2_PALETTE ? "I2" : \
                                                   "UNKNOW")

//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------
DrvGopIdType_e RgnGopTransId(DrvOsdId_e eOsdId)
{
    DrvGopIdType_e DrvId;
    DrvId = ((geGopIdFromOsd &(0xF<<(eOsdId*RGN_GOP_ID_SHIFTBIT)))>>(eOsdId*RGN_GOP_ID_SHIFTBIT));
    return DrvId;
}
DrvOsdId_e RgnOsdTransId(DrvGopIdType_e eGopId)
{
    DrvOsdId_e DrvId;
    DrvGopIdType_e DrvGopId;
    for(DrvId = E_DRV_OSD_PORT0;DrvId<E_DRV_OSD_ID_MAX;DrvId++)
    {
        DrvGopId = ((geGopIdFromOsd &(0xF<<(DrvId*RGN_GOP_ID_SHIFTBIT)))>>(DrvId*RGN_GOP_ID_SHIFTBIT));
        if(eGopId == DrvGopId)
        {
            break;
        }
    }
    return DrvId;
}
int RgnDebugDbglvShow(struct seq_file *buf, void *v)
{
    struct seq_file *str = buf;
    void *end = (void *)buf + PAGE_SIZE;
    DrvRgnScnprintf(str, end - str, "=====================RGN Info====================\n");
    DrvRgnScnprintf(str, end - str, "dbglv=%hhx\n",gbrgndbglv);
    DrvRgnScnprintf(str, end - str, "=====================RGN Info====================\n");
    end = end;
    return (str - buf);
}
int RgnDebugCoverShow(struct seq_file *buf, void *v)
{
    struct seq_file *str = buf;
    void *end = (void *)buf + PAGE_SIZE;
    u32 i,j;
    DrvRgnScnprintf(str, end - str, "=====================RGN Info====================\n");
    for(i=0;i<E_DRV_COVER_ID_NUM;i++)
    {
        if(i==E_DRV_ISPSC3_COVER)
            continue;
        DrvRgnScnprintf(str, end - str, "-----------------------COVER_%s----------------------\n",PARSING_RGN_PORT(i));
        for(j=0;j<E_DRV_COVER_WIN_ID_NUM;j++)
        {
            DrvRgnScnprintf(str, end - str, "...................BWIN_%d...................\n",j);
            DrvRgnScnprintf(str, end - str, "En:%hhu\n",_tCoverLocSettings[i].tCoverWinsSet[j].bBwinEn);
            if(_tCoverLocSettings[i].tCoverWinsSet[j].bBwinEn)
            {
                DrvRgnScnprintf(str, end - str, "BWin[X,Y,W,H]:[%hd %hd %hd %hd]\n",
                    _tCoverLocSettings[i].tCoverWinsSet[j].tWinSet.u16X,
                    _tCoverLocSettings[i].tCoverWinsSet[j].tWinSet.u16Y,
                    _tCoverLocSettings[i].tCoverWinsSet[j].tWinSet.u16Width,
                    _tCoverLocSettings[i].tCoverWinsSet[j].tWinSet.u16Height);
                DrvRgnScnprintf(str, end - str, "Color R:%hhu G:%hhu B:%hhu\n",
                _tCoverLocSettings[i].tCoverWinsSet[j].tWinColorSet.u8R,
                _tCoverLocSettings[i].tCoverWinsSet[j].tWinColorSet.u8G,
                _tCoverLocSettings[i].tCoverWinsSet[j].tWinColorSet.u8B);
            }
        }
    }
    DrvRgnScnprintf(str, end - str, "=====================RGN Info====================\n");
    end = end;
    return (str - buf);
}
int RgnDebugOsdShow(struct seq_file *buf, void *v)
{
    struct seq_file *str = buf;
    void *end = (void *)buf + PAGE_SIZE;
    u32 i;
    DrvRgnScnprintf(str, end - str, "=====================RGN Info====================\n");
    for(i=0;i<HAL_RGN_OSD_NUM;i++)
    {
        if(i==E_DRV_OSD_PORT3)
            continue;
        DrvRgnScnprintf(str, end - str, "------------------------OSD_%s----------------------\n",
            PARSING_RGN_PORT(i));
        DrvRgnScnprintf(str, end - str, "En:%hhu\n",_tOsdpLocSettings[i].bOsdEn);
        DrvRgnScnprintf(str, end - str, "bUsed:%hhu\n",(geGopOsdFlag&(0x1<<i))? 1 : 0);
        DrvRgnScnprintf(str, end - str, "Bind GOP ID:%s\n",PARSING_RGN_Bind(RgnGopTransId(i)));
    }
    DrvRgnScnprintf(str, end - str, "=====================RGN Info====================\n");
    end = end;
    return (str - buf);
}
int RgnDebugGopShow(struct seq_file *buf, void *v)
{
    struct seq_file *str = buf;
    void *end = (void *)buf + PAGE_SIZE;
    u32 i,j;
    DrvRgnScnprintf(str, end - str, "=====================RGN Info====================\n");
    for(i=0;i<HAL_RGN_GOP_NUM;i++)
    {
        DrvRgnScnprintf(str, end - str, "------------------------GOP_%d----------------------\n",i);
        DrvRgnScnprintf(str, end - str, "Bind OSD ID:%s\n",PARSING_RGN_PORT(RgnOsdTransId(i)));
        DrvRgnScnprintf(str, end - str, "Color Key En: %hhu R:%hhu G:%hhu B:%hhu\n",
        _tGopLocSettings[i].tGopColorKeySet.bEn,
        _tGopLocSettings[i].tGopColorKeySet.u8R,
        _tGopLocSettings[i].tGopColorKeySet.u8G,
        _tGopLocSettings[i].tGopColorKeySet.u8B);
        DrvRgnScnprintf(str, end - str, "Str Win[X,Y,W,H]:[%hd %hd %hd %hd]\n",
            _tGopLocSettings[i].tGopSrcStrWinSet.u16X,
            _tGopLocSettings[i].tGopSrcStrWinSet.u16Y,
            _tGopLocSettings[i].tGopSrcStrWinSet.u16Width,
            _tGopLocSettings[i].tGopSrcStrWinSet.u16Height);
        DrvRgnScnprintf(str, end - str, "Str Disp Win[X,Y,W,H]:[%hd %hd %hd %hd]\n",
            _tGopLocSettings[i].tGopDstStrWinSet.u16X,
            _tGopLocSettings[i].tGopDstStrWinSet.u16Y,
            _tGopLocSettings[i].tGopDstStrWinSet.u16Width,
            _tGopLocSettings[i].tGopDstStrWinSet.u16Height);
        for(j=0;j<HAL_RGN_GOP_GWIN_NUM;j++)
        {
            DrvRgnScnprintf(str, end - str, "...................GWIN_%d...................\n",j);
            DrvRgnScnprintf(str, end - str, "En:%hhu\n",_tGopLocSettings[i].tGwinLocSet[j].bGwinEn);
            if(_tGopLocSettings[i].tGwinLocSet[j].bGwinEn)
            {
                DrvRgnScnprintf(str, end - str, "Fmt:%s\n",
                    PARSING_RGN_GOPFMT(_tGopLocSettings[i].tGwinLocSet[j].tGwinGenSet.eSrcFmt));
                DrvRgnScnprintf(str, end - str, "Mem Hsize:%u\n",
                    _tGopLocSettings[i].tGwinLocSet[j].u32MemPitch);
                DrvRgnScnprintf(str, end - str, "Mem BaseAddr:%x\n",
                    _tGopLocSettings[i].tGwinLocSet[j].tGwinGenSet.u32BaseAddr);
                DrvRgnScnprintf(str, end - str, "Mem Base_XOffset:%hu\n",
                    _tGopLocSettings[i].tGwinLocSet[j].tGwinGenSet.u16Base_XOffset);
                DrvRgnScnprintf(str, end - str, "GWin[X,Y,W,H]:[%hd %hd %hd %hd]\n",
                    _tGopLocSettings[i].tGwinLocSet[j].tGwinGenSet.tDisplayWin.u16X,
                    _tGopLocSettings[i].tGwinLocSet[j].tGwinGenSet.tDisplayWin.u16Y,
                    _tGopLocSettings[i].tGwinLocSet[j].tGwinGenSet.tDisplayWin.u16Width,
                    _tGopLocSettings[i].tGwinLocSet[j].tGwinGenSet.tDisplayWin.u16Height);
                if(_tGopLocSettings[i].eSrcFmt == E_DRV_GOP_GWIN_SRC_FMT_RGB1555 ||
                _tGopLocSettings[i].eSrcFmt == E_DRV_GOP_GWIN_SRC_FMT_ARGB4444)
                {
                    DrvRgnScnprintf(str, end - str, "Alpha Type:%u\n",
                        _tGopLocSettings[i].tGwinLocSet[j].tGwinAblSet.eAlphaType);
                    if(_tGopLocSettings[i].tGwinLocSet[j].tGwinAblSet.eAlphaType == E_DRV_GOP_GWIN_ALPHA_CONSTANT)
                    {
                        DrvRgnScnprintf(str, end - str, "Const Alpha:%hhu\n",
                            *_tGopLocSettings[i].tGwinLocSet[j].p8ConstantAlphaVal);
                    }
                    else if(_tGopLocSettings[i].eSrcFmt == E_DRV_GOP_GWIN_SRC_FMT_RGB1555)
                    {
                        DrvRgnScnprintf(str, end - str, "Alpha0:%hhu\n",
                            *_tGopLocSettings[i].tGwinLocSet[j].p8Argb1555Alpha0Val);
                        DrvRgnScnprintf(str, end - str, "Alpha1:%hhu\n",
                            *_tGopLocSettings[i].tGwinLocSet[j].p8Argb1555Alpha1Val);
                    }
                }
            }
        }
    }
    DrvRgnScnprintf(str, end - str, "=====================RGN Info====================\n");
    end = end;
    return (str - buf);
}

static int rgn_gop_show(struct inode *inode, struct file *file)
{
    return single_open(file, RgnDebugGopShow, NULL);
}
static int rgn_osd_show(struct inode *inode, struct file *file)
{
    return single_open(file, RgnDebugOsdShow, NULL);
}
static int rgn_cover_show(struct inode *inode, struct file *file)
{
    return single_open(file, RgnDebugCoverShow, NULL);
}
static int rgn_dbglv_show(struct inode *inode, struct file *file)
{
    return single_open(file, RgnDebugDbglvShow, NULL);
}

static ssize_t rgn_dbglv_store (struct file *file,
        const char __user *buffer, size_t count, loff_t *pos)
{
    char buf[16];
    size_t len = min(sizeof(buf) - 1, count);

    if (copy_from_user(buf, buffer, len))
        return count;
    gbrgndbglv = CamOsStrtol(buf, NULL,16);

    return strnlen(buf, len);
}
bool bRgnInit =0;
static const struct file_operations RgnDbglvlProcOps =
{
    .owner		= THIS_MODULE,
    .open		= rgn_dbglv_show,
    .read		= seq_read,
    .llseek		= seq_lseek,
    .release	= single_release,
    .write		= rgn_dbglv_store,
};
static const struct file_operations RgnGopProcOps =
{
    .owner      = THIS_MODULE,
    .open       = rgn_gop_show,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};
static const struct file_operations RgnOsdProcOps =
{
    .owner      = THIS_MODULE,
    .open       = rgn_osd_show,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};
static const struct file_operations RgnCoverProcOps =
{
    .owner      = THIS_MODULE,
    .open       = rgn_cover_show,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};
void RgnSysfsInit(void)
{
    struct proc_dir_entry *pde;

    if(bRgnInit)
        return;
        
    bRgnInit = 1;
    gbrgndbglv = 0;
    gpRgnProcDir = proc_mkdir("mrgn", NULL);
    if (!gpRgnProcDir)
    {
        CamOsPrintf("[RGN] Can not create proc\n");
        return;
    }

    pde = proc_create("dbglv", S_IRUGO, gpRgnProcDir, &RgnDbglvlProcOps);
    if (!pde)
        goto out_dbglvl;
    pde = proc_create("gop", S_IRUGO, gpRgnProcDir, &RgnGopProcOps);
    if (!pde)
        goto out_gop;
    pde = proc_create("osd", S_IRUGO, gpRgnProcDir, &RgnOsdProcOps);
    if (!pde)
        goto out_osd;
    pde = proc_create("cover", S_IRUGO, gpRgnProcDir, &RgnCoverProcOps);
    if (!pde)
        goto out_cover;
    return ;
out_cover:
    remove_proc_entry("osd", gpRgnProcDir);
out_osd:
    remove_proc_entry("gop", gpRgnProcDir);
out_gop:
    remove_proc_entry("dbglvl", gpRgnProcDir);
out_dbglvl:
    return ;
}
void RgnSysfsDeInit(void)
{
    if(!bRgnInit)
        return;
    //ToDo
    bRgnInit = 0;
    gbrgndbglv = 0;
    remove_proc_entry("gop", gpRgnProcDir);
    remove_proc_entry("osd", gpRgnProcDir);
    remove_proc_entry("cover", gpRgnProcDir);
    remove_proc_entry("dbglv", gpRgnProcDir);
}
#undef RGN_SYSFS_C
