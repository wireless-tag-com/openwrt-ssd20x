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
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/delay.h>
#include <linux/device.h>

#include "ms_msys.h"
#include "ms_platform.h"

#include "cam_sysfs.h"
#include "drv_disp_os.h"
#include "hal_disp_common.h"
#include "disp_debug.h"
#include "hal_disp_util.h"
#include "hal_disp_reg.h"

#include "mhal_common.h"
#include "mhal_disp_datatype.h"
#include "mhal_disp.h"
#include "hal_disp_scaler_e.h"
#include "hal_disp_pq.h"
#include "hal_disp_color.h"
#include "drv_disp_if.h"
#include "hal_disp_chip.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DISPUT_SPRINTF_STRCAT(str, _fmt, _args...) \
    do {                                   \
        char tmpstr[1024];                 \
        sprintf(tmpstr, _fmt, ## _args);   \
        strcat(str, tmpstr);               \
    }while(0)

#define DISPUT_SPRINTF(str, _fmt, _args...)  sprintf(str, _fmt, ## _args)

#define MOP_FLIP_BUFFER_CNT     2


#define DISPUT_DBG(_fmt, _args...)                 \
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
}DispSysFsStrConfig_t;

typedef enum
{
    E_DISP_SYSFS_PQ_TYPE_NONE      = 0x0000,
    E_DISP_SYSFS_PQ_TYPE_BW        = 0x0001,
    E_DISP_SYSFS_PQ_TYPE_LPF       = 0x0002,
    E_DISP_SYSFS_PQ_TYPE_HCORING   = 0x0004,
    E_DISP_SYSFS_PQ_TYPE_PEAKING   = 0x0008,
    E_DISP_SYSFS_PQ_TYPE_FCC       = 0x0010,
    E_DISP_SYSFS_PQ_TYPE_DLC_DCR   = 0x0020,
    E_DISP_SYSFS_PQ_TYPE_GAMMA     = 0x0040,
    E_DISP_SYSFS_PQ_TYPE_ALL       = 0x007F,
}DispSysFsPqType_e;


typedef struct
{
    void *pDevCtx;
    void *pVidLayerCtx;
    void *pInputPotCtx;
    s32 s32IsrRet;
}DispSysFsCtxConfig_t;

typedef struct
{
    void *pInputPot1Ctx;
    void *pInputPot2Ctx;
    void *pInputPot3Ctx;
    void *pInputPot4Ctx;
    void *pInputPot5Ctx;
    void *pInputPot6Ctx;
    void *pInputPot7Ctx;
    void *pInputPot8Ctx;
    void *pInputPot9Ctx;
    void *pInputPot10Ctx;
    void *pInputPot11Ctx;
    void *pInputPot12Ctx;
    void *pInputPot13Ctx;
    void *pInputPot14Ctx;
    void *pInputPot15Ctx;
}DispSysFsCtxMopgOtherConfig_t;


typedef struct
{
    u8 pu8Name[64];
    u8 pu8Value[64];
}DispSysFsTxtItemConfig_t;

typedef struct
{
    DispSysFsTxtItemConfig_t *pstItem;
    u32 u32Size;
}DispSysFsTxtConfig_t;

typedef struct
{
    MSYS_DMEM_INFO stDmem;
    MS_BOOL bUsed;
}DispSysFsDmemConfig_t;


typedef struct
{
    int IrqNum;
    void *pVirImageBuf;
    u32  u32ImageSize;
    u16  u16ImageWidth;
    u16  u16ImageHeight;

    u32  u32ActiveCnt;
    u32  u32UpdateFrameRate;

    u32  u32StreamCnt;
    u32  u32ActiveStremCnt;

    s32  s32TaskId;
    DrvDispOsTaskConfig_t stTaskCfg;
    bool bTaskRun;
    bool bFlip;
    unsigned long long *pu64PhyAddr;
    unsigned long long *pu64VirAddr;
    u8   u8FlipBufCnt;
    s8   s8Active_R;
    s8   s8Idx_R;
    s8   s8Idx_W;
}DispSysFsFlipConfig_t;


//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MHAL_DISP_AllocPhyMem_t gstMemAlloc = {NULL, NULL};

u32 u32DispDbgmgLevel = 0;
u8  u8PtGenMode = 0;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
int _DispSysFsSplit(char **arr, char *str,  char* del)
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

unsigned long _DispSysFsGetFileSize(char *path)
{
    struct path p;
    struct kstat ks;
    unsigned long filesize;

    kern_path(path, 0, &p);
    vfs_getattr(&p, &ks);
    filesize =  ks.size;
    return filesize;

}

struct file *_DispSysFsOpenFile(char *path,int flag,int mode)
{
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flag, mode);
    set_fs(oldfs);
    if(IS_ERR(filp))
    {
        DISP_ERR("%s %d, Open File Fail\n", __FUNCTION__, __LINE__);
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

int _DispSysFsWritFile(struct file *fp,char *buf,int writelen)
{
    mm_segment_t oldfs;
    int ret;
    unsigned long long offset = 0;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(fp, buf, writelen, &offset);

    set_fs(oldfs);
    return ret;
}

int _DIspSysFsReadFile(struct file *fp,char *buf,int readlen)
{
    mm_segment_t oldfs;
    int ret;
    unsigned long long offset = 0;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(fp, buf, readlen, &offset);

    set_fs(oldfs);
    return ret;
}


int _DispSysFsCloseFile(struct file *fp)
{
    filp_close(fp,NULL);
    return 0;
}

void _DispSysFsSplitByToken(DispSysFsStrConfig_t *pstStrCfg, char *pBuf, char *token)
{
    pstStrCfg->argc = _DispSysFsSplit(pstStrCfg->argv, (char *)pBuf, token);
}


void _DispSysFsParsingTxt(char *pFileName, DispSysFsTxtConfig_t *pstTxtCfg)
{
    struct file *pReadFile;
    char *pBuf;
    int FileSize;
    char filedel[] = "\n";
    char linedel[] = " ";
    int i, j, len;
    char LinePattern[1024];
    DispSysFsStrConfig_t stLineStrCfg;
    DispSysFsStrConfig_t stFileStrCfg;
    DispSysFsTxtItemConfig_t *pstTxtItemCfg;
    u32 u32LineCnt;

    pReadFile = _DispSysFsOpenFile(pFileName,O_RDONLY, 0);

    if(pReadFile)
    {
        FileSize = _DispSysFsGetFileSize(pFileName);

        pBuf =DrvDispOsMemAlloc(FileSize+1);

        if(pBuf)
        {
            _DIspSysFsReadFile(pReadFile, pBuf, FileSize);
            _DispSysFsCloseFile(pReadFile);
            pBuf[FileSize] = '\0';

            _DispSysFsSplitByToken(&stFileStrCfg, pBuf, filedel);
            u32LineCnt = stFileStrCfg.argc;

            pstTxtCfg->pstItem = DrvDispOsMemAlloc(sizeof(DispSysFsTxtItemConfig_t) * u32LineCnt);
            pstTxtCfg->u32Size = 0;

            if(pstTxtCfg->pstItem)
            {
                for(i=0; i<u32LineCnt; i++)
                {
                    if(stFileStrCfg.argv[i] == NULL)
                    {
                        DISP_ERR("NULL Point, %d\n", i);
                        continue;
                    }

                    memset(LinePattern, 0, 1024);
                    len = strlen(stFileStrCfg.argv[i]);
                    memcpy(LinePattern, stFileStrCfg.argv[i], len);
                    _DispSysFsSplitByToken(&stLineStrCfg, LinePattern, linedel);

                    len = strlen(stLineStrCfg.argv[0]);
                    if((len == 0) || (stLineStrCfg.argv[0][0] == '#'))
                    {
                        continue;
                    }

                    pstTxtItemCfg = &pstTxtCfg->pstItem[pstTxtCfg->u32Size++];
                    len = strlen(stLineStrCfg.argv[0]);
                    memcpy(pstTxtItemCfg->pu8Name, stLineStrCfg.argv[0], len);
                    pstTxtItemCfg->pu8Name[len] = '\0';

                    for(j=1; j<stLineStrCfg.argc; j++)
                    {
                        if(stLineStrCfg.argv[j][0] == '#')
                        {
                            break;
                        }

                        len = strlen(stLineStrCfg.argv[j]);

                        if(len != 0 && (strcmp(stLineStrCfg.argv[j], "=")!=0))
                        {
                            memcpy(pstTxtItemCfg->pu8Value, stLineStrCfg.argv[j], len);
                            pstTxtItemCfg->pu8Value[len] = '\0';
                        }
                    }
                    //printk("line:%d, %s = %s\n", i, pstTxtItemCfg->pu8Name, pstTxtItemCfg->pu8Value);
                }
            }

           CamOsMemRelease(pBuf);
        }
    }
}


void _DispSysFsUpdatePqConfig(DispSysFsTxtConfig_t *pstTxtCfg, HalDispPqHwContext_t *pstPqCfg, DispSysFsPqType_e enPqType)
{
    int ret;
    DispSysFsTxtItemConfig_t *pstTxtItem;
    u32 i;
    u16 u16Value;
    HalDispPqBwExtensionConfig_t *pstBwCfg = &pstPqCfg->stBwExtCfg;
    HalDispPqDlcDcrConfig_t *pstDlcDcrCfg = &pstPqCfg->stDlcDcrCfg;
    HalDispPqLpfConfig_t *pstLpfCfg = &pstPqCfg->stLpfCfg;
    HalDispPqHCoringConfig_t *pstHCoringCfg = &pstPqCfg->stHCoringCfg;
    HalDispPqPeakingConfig_t *pstPeakingCfg = &pstPqCfg->stPeakingCfg;
    HalDispPqFccConfig_t *pstFccCfg = &pstPqCfg->stFccCfg;
    HalDispPqGammaConfig_t *pstGammaCfg = &pstPqCfg->stGammaCfg;

    for(i=0; i<pstTxtCfg->u32Size; i++)
    {
        pstTxtItem = &pstTxtCfg->pstItem[i];

        if(pstTxtItem->pu8Value[0]=='0' && pstTxtItem->pu8Value[1]=='x')
        {
            ret = kstrtol(&pstTxtItem->pu8Value[2], 16, (long *)&u16Value);
        }
        else
        {
            ret = kstrtol(pstTxtItem->pu8Value, 10, (long *)&u16Value);
        }

        if(enPqType & E_DISP_SYSFS_PQ_TYPE_BW)
        {
            pstBwCfg->bUpdate = 1;
            if(strcmp(pstTxtItem->pu8Name, "bw2sbri") == 0)
            {
                pstBwCfg->u8Bw2sbri = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "ble_en") == 0)
            {
                 pstBwCfg->u8BleEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "black_start") == 0)
            {
                 pstBwCfg->u8BlackStart = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "black_slop") == 0)
            {
                 pstBwCfg->u8BlackSlop = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "wle_en") == 0)
            {
                 pstBwCfg->u8WleEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "white_start") == 0)
            {
                 pstBwCfg->u8WhiteStart = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "white_slop") == 0)
            {
                 pstBwCfg->u8WhiteSlop = u16Value;
            }
        }

        if(enPqType & E_DISP_SYSFS_PQ_TYPE_LPF)
        {
            pstLpfCfg->bUpdate = 1;
            if(strcmp(pstTxtItem->pu8Name, "LpfY") == 0)
            {
                 pstLpfCfg->u8LpfY = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "LpfC") == 0)
            {
                 pstLpfCfg->u8LpfC = u16Value;
            }
        }


        if(enPqType & E_DISP_SYSFS_PQ_TYPE_DLC_DCR)
        {
            pstDlcDcrCfg->bUpdate = 1;
            if(strcmp(pstTxtItem->pu8Name, "Dcr_En") == 0)
            {
                 pstDlcDcrCfg->u16DcrEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dcr_Offset") == 0)
            {
                 pstDlcDcrCfg->u16DcrOffset= u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dlc_Hact") == 0)
            {
                 pstDlcDcrCfg->u16Hactive = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dlc_Vact") == 0)
            {
                 pstDlcDcrCfg->u16Vactive = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dlc_Hblank") == 0)
            {
                 pstDlcDcrCfg->u16Hblank = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dlc_PwmDuty") == 0)
            {
                 pstDlcDcrCfg->u16PwmDuty = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dlc_PwmPeriod") == 0)
            {
                 pstDlcDcrCfg->u16PwmPeriod = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dlc_En") == 0)
            {
                 pstDlcDcrCfg->u16DlcEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dlc_Gain") == 0)
            {
                 pstDlcDcrCfg->u16DlcGain = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dlc_Offset") == 0)
            {
                 pstDlcDcrCfg->u16DlcOffset = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dlc_YGain") == 0)
            {
                 pstDlcDcrCfg->u16YGain = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "Dlc_YGainOffset") == 0)
            {
                 pstDlcDcrCfg->u16YGainOffset = u16Value;
            }
        }


        if(enPqType & E_DISP_SYSFS_PQ_TYPE_HCORING)
        {
            pstHCoringCfg->bUpdate = 1;
            if(strcmp(pstTxtItem->pu8Name, "y_band1_h_coring_en") == 0)
            {
                 pstHCoringCfg->u8YBand1HCoringEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "y_band2_h_coring_en") == 0)
            {
                 pstHCoringCfg->u8YBand2HCoringEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "h_coring_y_dither_en") == 0)
            {
                 pstHCoringCfg->u8HCoringYDither_En = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "y_table_step") == 0)
            {
                 pstHCoringCfg->u8YTableStep= u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "c_band1_h_coring_en") == 0)
            {
                 pstHCoringCfg->u8CBand1HCoringEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "c_band2_h_coring_en") == 0)
            {
                 pstHCoringCfg->u8CBand2HCoringEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "h_coring_c_dither_en") == 0)
            {
                 pstHCoringCfg->u8HCoringCDither_En = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "c_table_step") == 0)
            {
                 pstHCoringCfg->u8CTableStep= u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "pc_mode") == 0)
            {
                 pstHCoringCfg->u8PcMode= u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "high_pass_en") == 0)
            {
                 pstHCoringCfg->u8HighPassEn= u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "coring_table0") == 0)
            {
                 pstHCoringCfg->u8CoringTable0 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "coring_table1") == 0)
            {
                 pstHCoringCfg->u8CoringTable1 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "coring_table2") == 0)
            {
                 pstHCoringCfg->u8CoringTable2 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "coring_table3") == 0)
            {
                 pstHCoringCfg->u8CoringTable3 = u16Value;
            }
        }

        if(enPqType & E_DISP_SYSFS_PQ_TYPE_PEAKING)
        {
            pstPeakingCfg->bUpdate = 1;
            if(strcmp(pstTxtItem->pu8Name, "peak_en") == 0)
            {
                pstPeakingCfg->u8PeakingEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "lti_en") == 0)
            {
                pstPeakingCfg->u8LtiEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "lti_median_filter_on") == 0)
            {
                pstPeakingCfg->u8LtiMediaFilterOn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cit_en") == 0)
            {
                pstPeakingCfg->u8CtiEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cti_median_filter_on") == 0)
            {
                pstPeakingCfg->u8CtiMediaFilterOn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "diff_adap_en") == 0)
            {
                pstPeakingCfg->u8DiffAdapEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "band1_coef") == 0)
            {
                pstPeakingCfg->u8Band1Coef = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "band1_step") == 0)
            {
                pstPeakingCfg->u8Band1Step = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "bnad2_coef") == 0)
            {
                pstPeakingCfg->u8Band2Coef = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "band2_step") == 0)
            {
                pstPeakingCfg->u8Band2Step = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "lti_coef") == 0)
            {
                pstPeakingCfg->u8LtiCoef = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "lti_step") == 0)
            {
                pstPeakingCfg->u8LtiStep = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "peaking_term1_sel") == 0)
            {
                pstPeakingCfg->u8PeakingTerm1Sel = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "peaking_term2_sel") == 0)
            {
                pstPeakingCfg->u8PeakingTerm2Sel = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "peaking_term3_sel") == 0)
            {
                pstPeakingCfg->u8PeakingTerm3Sel = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "peaking_term4_sel") == 0)
            {
                pstPeakingCfg->u8PeakingTerm4Sel = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "coring_th1") == 0)
            {
                pstPeakingCfg->u8CoringTh1 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "coring_th2") == 0)
            {
                pstPeakingCfg->u8CoringTh2 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cti_coef") == 0)
            {
                pstPeakingCfg->u8CtiCoef = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cti_step") == 0)
            {
                pstPeakingCfg->u8CtiStep = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "band1_pos_limit_th") == 0)
            {
                pstPeakingCfg->u8Band1PosLimitTh = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "band1_neg_limit_th") == 0)
            {
                pstPeakingCfg->u8Band1NegLimitTh = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "band2_pos_limit_th") == 0)
            {
                pstPeakingCfg->u8Band2PosLimitTh = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "band2_neg_limit_th") == 0)
            {
                pstPeakingCfg->u8Band2NegLimitTh = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "pos_limit_th") == 0)
            {
                pstPeakingCfg->u8PosLimitTh = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "neg_limit_th") == 0)
            {
                pstPeakingCfg->u8NegLimitTh = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "band2_diff_adp_en") == 0)
            {
                pstPeakingCfg->u8Band2DiffAdapEn = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "band1_diff_adp_en") == 0)
            {
                pstPeakingCfg->u8Band1DiffAdapEn = u16Value;
            }
        }

        if(enPqType & E_DISP_SYSFS_PQ_TYPE_FCC)
        {
            pstFccCfg->bUpdate = 1;
            if(strcmp(pstTxtItem->pu8Name, "cb_t1") == 0)
            {
                pstFccCfg->u8Cb_T1 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cr_t1") == 0)
            {
                pstFccCfg->u8Cr_T1 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cb_t2") == 0)
            {
                pstFccCfg->u8Cb_T2 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cr_t2") == 0)
            {
                pstFccCfg->u8Cr_T2 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cb_t3") == 0)
            {
                pstFccCfg->u8Cb_T3 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cr_t3") == 0)
            {
                pstFccCfg->u8Cr_T3 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cb_t4") == 0)
            {
                pstFccCfg->u8Cb_T4 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cr_t4") == 0)
            {
                pstFccCfg->u8Cr_T4 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cb_t5") == 0)
            {
                pstFccCfg->u8Cb_T5 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cr_t5") == 0)
            {
                pstFccCfg->u8Cr_T5 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cb_t6") == 0)
            {
                pstFccCfg->u8Cb_T6 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cr_t6") == 0)
            {
                pstFccCfg->u8Cr_T6 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cb_t7") == 0)
            {
                pstFccCfg->u8Cb_T7 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cr_t7") == 0)
            {
                pstFccCfg->u8Cr_T7 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cb_t8") == 0)
            {
                pstFccCfg->u8Cb_T8 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cr_t8") == 0)
            {
                pstFccCfg->u8Cr_T8 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cbcr_d1d_d1u") == 0)
            {
                pstFccCfg->u8CbCr_D1D_D1U = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cbcr_d2d_d2u") == 0)
            {
                pstFccCfg->u8CbCr_D2D_D2U = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cbcr_d3d_d3u") == 0)
            {
                pstFccCfg->u8CbCr_D3D_D3U = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cbcr_d4d_d4u") == 0)
            {
                pstFccCfg->u8CbCr_D4D_D4U = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cbcr_d5d_d5u") == 0)
            {
                pstFccCfg->u8CbCr_D5D_D5U = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cbcr_d6d_d6u") == 0)
            {
                pstFccCfg->u8CbCr_D6D_D6U = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cbcr_d7d_d7u") == 0)
            {
                pstFccCfg->u8CbCr_D7D_D7U = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cbcr_d8d_d8u") == 0)
            {
                pstFccCfg->u8CbCr_D8D_D8U = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "cbcr_d9") == 0)
            {
                pstFccCfg->u8CbCr_D9 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "k_t2_k_t1") == 0)
            {
                pstFccCfg->u8K_T2_K_T1 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "k_t4_k_t3") == 0)
            {
                pstFccCfg->u8K_T4_K_T3 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "k_t6_k_t5") == 0)
            {
                pstFccCfg->u8K_T6_K_T5 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "k_t8_k_t7") == 0)
            {
                pstFccCfg->u8K_T8_K_T7 = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "fcc_en") == 0)
            {
                pstFccCfg->u8En = u16Value;
            }
        }

        if(enPqType & E_DISP_SYSFS_PQ_TYPE_GAMMA)
        {
            pstGammaCfg->bUpdate = 1;
            if(strcmp(pstTxtItem->pu8Name, "gamma_en") == 0)
            {
                pstGammaCfg->u8En = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r00") == 0)
            {
                pstGammaCfg->u8R[0] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r01") == 0)
            {
                pstGammaCfg->u8R[1] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r02") == 0)
            {
                pstGammaCfg->u8R[2] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r03") == 0)
            {
                pstGammaCfg->u8R[3] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r04") == 0)
            {
                pstGammaCfg->u8R[4] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r05") == 0)
            {
                pstGammaCfg->u8R[5] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r06") == 0)
            {
                pstGammaCfg->u8R[6] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r07") == 0)
            {
                pstGammaCfg->u8R[7] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r08") == 0)
            {
                pstGammaCfg->u8R[8] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r09") == 0)
            {
                pstGammaCfg->u8R[9] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r10") == 0)
            {
                pstGammaCfg->u8R[10] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r11") == 0)
            {
                pstGammaCfg->u8R[11] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r12") == 0)
            {
                pstGammaCfg->u8R[12] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r13") == 0)
            {
                pstGammaCfg->u8R[13] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r14") == 0)
            {
                pstGammaCfg->u8R[14] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r15") == 0)
            {
                pstGammaCfg->u8R[15] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r16") == 0)
            {
                pstGammaCfg->u8R[16] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r17") == 0)
            {
                pstGammaCfg->u8R[17] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r18") == 0)
            {
                pstGammaCfg->u8R[18] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r19") == 0)
            {
                pstGammaCfg->u8R[19] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r20") == 0)
            {
                pstGammaCfg->u8R[20] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r21") == 0)
            {
                pstGammaCfg->u8R[21] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r22") == 0)
            {
                pstGammaCfg->u8R[22] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r23") == 0)
            {
                pstGammaCfg->u8R[23] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r24") == 0)
            {
                pstGammaCfg->u8R[24] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r25") == 0)
            {
                pstGammaCfg->u8R[25] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r26") == 0)
            {
                pstGammaCfg->u8R[26] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r27") == 0)
            {
                pstGammaCfg->u8R[27] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r28") == 0)
            {
                pstGammaCfg->u8R[28] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r29") == 0)
            {
                pstGammaCfg->u8R[29] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r30") == 0)
            {
                pstGammaCfg->u8R[30] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r31") == 0)
            {
                pstGammaCfg->u8R[31] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_r32") == 0)
            {
                pstGammaCfg->u8R[32] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g00") == 0)
            {
                pstGammaCfg->u8G[0] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g01") == 0)
            {
                pstGammaCfg->u8G[1] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g02") == 0)
            {
                pstGammaCfg->u8G[2] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g03") == 0)
            {
                pstGammaCfg->u8G[3] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g04") == 0)
            {
                pstGammaCfg->u8G[4] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g05") == 0)
            {
                pstGammaCfg->u8G[5] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g06") == 0)
            {
                pstGammaCfg->u8G[6] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g07") == 0)
            {
                pstGammaCfg->u8G[7] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g08") == 0)
            {
                pstGammaCfg->u8G[8] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g09") == 0)
            {
                pstGammaCfg->u8G[9] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g10") == 0)
            {
                pstGammaCfg->u8G[10] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g11") == 0)
            {
                pstGammaCfg->u8G[11] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g12") == 0)
            {
                pstGammaCfg->u8G[12] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g13") == 0)
            {
                pstGammaCfg->u8G[13] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g14") == 0)
            {
                pstGammaCfg->u8G[14] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g15") == 0)
            {
                pstGammaCfg->u8G[15] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g16") == 0)
            {
                pstGammaCfg->u8G[16] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g17") == 0)
            {
                pstGammaCfg->u8G[17] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g18") == 0)
            {
                pstGammaCfg->u8G[18] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g19") == 0)
            {
                pstGammaCfg->u8G[19] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g20") == 0)
            {
                pstGammaCfg->u8G[20] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g21") == 0)
            {
                pstGammaCfg->u8G[21] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g22") == 0)
            {
                pstGammaCfg->u8G[22] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g23") == 0)
            {
                pstGammaCfg->u8G[23] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g24") == 0)
            {
                pstGammaCfg->u8G[24] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g25") == 0)
            {
                pstGammaCfg->u8G[25] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g26") == 0)
            {
                pstGammaCfg->u8G[26] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g27") == 0)
            {
                pstGammaCfg->u8G[27] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g28") == 0)
            {
                pstGammaCfg->u8G[28] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g29") == 0)
            {
                pstGammaCfg->u8G[29] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g30") == 0)
            {
                pstGammaCfg->u8G[30] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g31") == 0)
            {
                pstGammaCfg->u8G[31] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_g32") == 0)
            {
                pstGammaCfg->u8G[32] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b00") == 0)
            {
                pstGammaCfg->u8B[0] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b01") == 0)
            {
                pstGammaCfg->u8B[1] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b02") == 0)
            {
                pstGammaCfg->u8B[2] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b03") == 0)
            {
                pstGammaCfg->u8B[3] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b04") == 0)
            {
                pstGammaCfg->u8B[4] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b05") == 0)
            {
                pstGammaCfg->u8B[5] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b06") == 0)
            {
                pstGammaCfg->u8B[6] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b07") == 0)
            {
                pstGammaCfg->u8B[7] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b08") == 0)
            {
                pstGammaCfg->u8B[8] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b09") == 0)
            {
                pstGammaCfg->u8B[9] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b10") == 0)
            {
                pstGammaCfg->u8B[10] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b11") == 0)
            {
                pstGammaCfg->u8B[11] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b12") == 0)
            {
                pstGammaCfg->u8B[12] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b13") == 0)
            {
                pstGammaCfg->u8B[13] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b14") == 0)
            {
                pstGammaCfg->u8B[14] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b15") == 0)
            {
                pstGammaCfg->u8B[15] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b16") == 0)
            {
                pstGammaCfg->u8B[16] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b17") == 0)
            {
                pstGammaCfg->u8B[17] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b18") == 0)
            {
                pstGammaCfg->u8B[18] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b19") == 0)
            {
                pstGammaCfg->u8B[19] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b20") == 0)
            {
                pstGammaCfg->u8B[20] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b21") == 0)
            {
                pstGammaCfg->u8B[21] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b22") == 0)
            {
                pstGammaCfg->u8B[22] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b23") == 0)
            {
                pstGammaCfg->u8B[23] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b24") == 0)
            {
                pstGammaCfg->u8B[24] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b25") == 0)
            {
                pstGammaCfg->u8B[25] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b26") == 0)
            {
                pstGammaCfg->u8B[26] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b27") == 0)
            {
                pstGammaCfg->u8B[27] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b28") == 0)
            {
                pstGammaCfg->u8B[28] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b29") == 0)
            {
                pstGammaCfg->u8B[29] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b30") == 0)
            {
                pstGammaCfg->u8B[30] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b31") == 0)
            {
                pstGammaCfg->u8B[31] = u16Value;
            }
            else if(strcmp(pstTxtItem->pu8Name, "gamma_b32") == 0)
            {
                pstGammaCfg->u8B[32] = u16Value;
            }
        }
    }
}

void DispDbgmgStore(DispSysFsStrConfig_t *pstStringCfg)
{
    int ret, u32Level;

    if(pstStringCfg->argc < 1)
    {
        return;
    }

    if(pstStringCfg->argc == 1)
    {
        ret = kstrtol(pstStringCfg->argv[0], 16, (long *)&u32Level);
    }
    else
    {
        DISPUT_DBG("dbgg [level] \n");
        DISPUT_DBG("DRV:       0x00000001 \n");
        DISPUT_DBG("HAL:       0x00000002 \n");
        DISPUT_DBG("IO:        0x00000004 \n");
        DISPUT_DBG("CTX:       0x00000008 \n");
        DISPUT_DBG("COLOR:     0x00000010 \n");
        DISPUT_DBG("IRQ:       0x00000020 \n");
        DISPUT_DBG("IRQ:       0x00000040 \n");
        DISPUT_DBG("Inter IRQ: 0x00000080 \n");
        DISPUT_DBG("CLK:       0x00000100 \n");
        return;
    }

    u32DispDbgmgLevel = u32Level;
    MHAL_DISP_DbgLevel((void *)&u32Level);
    printk("DbgLevel:%08x\n", u32Level);

}

int DispDbgmgShow(char *DstBuf)
{
    int RetSprintf = -1;
    char *SrcBuf;

    SrcBuf = (char *)DrvDispOsMemAlloc(1024*3);

    if(SrcBuf)
    {
        DISPUT_SPRINTF_STRCAT(SrcBuf, "dbglevle: %08x \n", (unsigned int)u32DispDbgmgLevel);
        RetSprintf = DISPUT_SPRINTF(DstBuf, "%s", SrcBuf);
        DrvDispOsMemRelease(SrcBuf);
    }
    return RetSprintf;
}


void DispPtGenStore(DispSysFsStrConfig_t *pstStringCfg)
{
    int ret, Enable, PtGenMode;

    if(pstStringCfg->argc < 1)
    {
        return;
    }

    if(pstStringCfg->argc == 2)
    {
        ret = kstrtol(pstStringCfg->argv[0], 16, (long *)&Enable);
        ret = kstrtol(pstStringCfg->argv[1], 16, (long *)&PtGenMode);
    }
    else
    {
        DISPUT_DBG("[En] [MODE] \n");
        DISPUT_DBG("1  pix Gray Ramp : 0 \n");
        DISPUT_DBG("16 pix Gray Ramp : 1 \n");
        DISPUT_DBG("32 Pix Gray Ramp : 2 \n");
        DISPUT_DBG("64 Pix Gray Ramp : 3 \n");
        DISPUT_DBG("16 Pix Gray Stick: 4 \n");
        DISPUT_DBG("16 Pix ColorBar  : 5 \n");
        DISPUT_DBG("32 Pix ColorBar  : 6 \n");
        DISPUT_DBG("64 Pix ColorBar  : 7 \n");
        return;
    }

    u8PtGenMode = Enable << 7 | PtGenMode;
    HalDispSetMaceSrc(Enable ? 0 : 1 );
    HalDispSetPatGenMd(PtGenMode);
}

int DispPtGenShow(char *DstBuf)
{
    int Enable, Mode;
    int RetSprintf = -1;
    char *SrcBuf;

    SrcBuf = (char *)DrvDispOsMemAlloc(1024*3);

    if(SrcBuf)
    {
        Enable = u8PtGenMode& 0x80 ? 1: 0;
        Mode = u8PtGenMode & 0x0F;
        DISPUT_SPRINTF_STRCAT(SrcBuf, "Ptgen: En:%d, Mode:%d \n", Enable, Mode);
        RetSprintf = DISPUT_SPRINTF(DstBuf, "%s", SrcBuf);
        DrvDispOsMemRelease(SrcBuf);
    }
    return RetSprintf;
}


void DispClkStore(DispSysFsStrConfig_t *pstStringCfg)
{
    int ret, idx;
    bool Enable = 0;
    u32 ClkRate = 0;
    void *pDevInst = NULL;
    char *ClkName = NULL;
    bool abClkEn[HAL_DISP_CLK_NUM] = HAL_DISP_CLK_ON_SETTING;
    u32  au32ClkRate[HAL_DISP_CLK_NUM] = HAL_DISP_CLK_RATE_SETTING;
    char au8ClkName[HAL_DISP_CLK_NUM][20] = HAL_DISP_CLK_NAME;
    u32 i;
    bool bRet;

    if(pstStringCfg->argc < 1)
    {
        return;
    }

    if(pstStringCfg->argc == HAL_DISP_CLK_NUM + 2)
    {
        ClkName = pstStringCfg->argv[0];
        ret = kstrtol(pstStringCfg->argv[1], 10, (long *)&Enable);
        for(i=0; i<HAL_DISP_CLK_NUM; i++)
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
        DISPUT_DBG("----------------- CLK TREE -----------------\n");
        DISPUT_DBG("clktree [EN] [mop] [hdmi] [dac] [dsip432] [disp216]\n");
        DISPUT_DBG("--------------- DRV UPDATE ----------\n");
        DISPUT_DBG("[Clk Type] [En] [ClkRate] \n");
        DISPUT_DBG("Clk Tyep: mop, hdmi, dac disp432, disp216 \n");
        return;
    }

    if(strcmp(ClkName, "clktree") == 0)
    {
        if(Enable)
        {
            if(DrvDispOsSetClkOn(au32ClkRate, HAL_DISP_CLK_NUM) == 0)
            {
                DISP_ERR("%s %d, Set Clk On Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            if(DrvDispOsSetClkOff() == 0)
            {
                DISP_ERR("%s %d, Set Clk Off Fail\n", __FUNCTION__, __LINE__);
            }
        }
    }
    else
    {
        bRet = MHAL_DISP_DeviceGetInstance(0, &pDevInst);

        if(bRet == 0 || pDevInst == NULL)
        {
            MHAL_DISP_DeviceCreateInstance(&gstMemAlloc, 0, &pDevInst);
            if(pDevInst == NULL)
            {
                DISP_ERR("%s %d, No Isntance \n", __FUNCTION__, __LINE__);
                return ;
            }
        }

        if(DrvDispIfGetClk(pDevInst, abClkEn, au32ClkRate, HAL_DISP_CLK_NUM) == 0)
        {
            DISP_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
            return;

        }

        for(idx =0; idx < HAL_DISP_CLK_NUM; i++)
        {
            if(strcmp(ClkName, au8ClkName[i]) == 0)
            {
                abClkEn[idx] = Enable,
                au32ClkRate[idx] = ClkRate;
                if(DrvDispIfSetClk(pDevInst, abClkEn, au32ClkRate, HAL_DISP_CLK_NUM) == 0)
                {
                    DISP_ERR("%s %d, Set Clk Fail\n", __FUNCTION__, __LINE__);
                }
                break;
            }
        }

        if(idx == HAL_DISP_CLK_NUM)
        {
            DISP_ERR("%s %d, ClkName (%s) is not correct\n", __FUNCTION__, __LINE__, ClkName);
        }
    }
}

int DispClkShow(char *DstBuf)
{
    bool abEn[HAL_DISP_CLK_NUM];
    u32 au32ClkRate[HAL_DISP_CLK_NUM];
    int RetSprintf = -1;
    char *SrcBuf;
    void *pDevInst = NULL;
    char  aClkName[HAL_DISP_CLK_NUM][20] = HAL_DISP_CLK_NAME;
    u32 i;
    bool bRet;

    SrcBuf = (char *)DrvDispOsMemAlloc(1024*3);

    if(SrcBuf)
    {
        bRet = MHAL_DISP_DeviceGetInstance(0, &pDevInst);

        if(bRet == 0 || pDevInst == NULL)
        {
            if(MHAL_DISP_DeviceCreateInstance(&gstMemAlloc, 0, &pDevInst) == 0)
            {
                DISP_ERR("%s %d, CreateInstance Fail \n", __FUNCTION__, __LINE__);
            }
        }

        if(pDevInst)
        {
            if(DrvDispIfGetClk(pDevInst, abEn, au32ClkRate, HAL_DISP_CLK_NUM) == 0)
            {
                DISP_ERR("%s %d, Get Clk Fail\n", __FUNCTION__, __LINE__);
            }

            for(i=0; i<HAL_DISP_CLK_NUM; i++)
            {
                DISPUT_SPRINTF_STRCAT(SrcBuf, "CLK_%-15s: En:%d, ClkRate:%ld \n", aClkName[i], abEn[i], au32ClkRate[i]);
            }
            RetSprintf = DISPUT_SPRINTF(DstBuf, "%s", SrcBuf);
        }
        else
        {
            DISP_ERR("%s %d, No Isntance \n", __FUNCTION__, __LINE__);
        }

        DrvDispOsMemRelease(SrcBuf);
    }
    return RetSprintf;
}


void DispPqStore(DispSysFsStrConfig_t *pstStringCfg)
{
    char *pFileName;
    DispSysFsTxtConfig_t stTxtCfg;
    int ret, PqType;
    HalDispPqHwContext_t *pstPqHwCtx = NULL;

    if(pstStringCfg->argc < 1)
    {
        return;
    }

    if(pstStringCfg->argc == 2)
    {
        pFileName = pstStringCfg->argv[0];

        ret = kstrtol(pstStringCfg->argv[1], 16, (long *)&PqType);
    }
    else
    {
        PqType = E_DISP_SYSFS_PQ_TYPE_NONE;
    }

    if(PqType == E_DISP_SYSFS_PQ_TYPE_NONE ||  ((PqType & E_DISP_SYSFS_PQ_TYPE_ALL) == 0))
    {
        DISPUT_DBG("[fileName] [pqtype]\n");
        DISPUT_DBG("0x0001 : bw\n");
        DISPUT_DBG("0x0002 : lpf\n");
        DISPUT_DBG("0x0004 : hcoring\n");
        DISPUT_DBG("0x0008 : peaking\n");
        DISPUT_DBG("0x0010 : fcc\n");
        DISPUT_DBG("0x0020 : dlc_dcr\n");
        DISPUT_DBG("0x0040 : gamma\n");
        return;
    }

    HalDispPqGetHwCtx(&pstPqHwCtx);

    if(pstPqHwCtx == NULL)
    {
       DISPUT_DBG("%s %d, PqCtx is NULL\n", __FUNCTION__, __LINE__);
    }

    _DispSysFsParsingTxt(pFileName, &stTxtCfg);
    _DispSysFsUpdatePqConfig(&stTxtCfg, pstPqHwCtx, PqType);
    CamOsMemRelease(stTxtCfg.pstItem);

    if(PqType & E_HAL_DISP_PQ_FLAG_BW)
    {
        HalDispPqSetBwExtensionConfig(&pstPqHwCtx->stBwExtCfg);
    }

    if(PqType & E_HAL_DISP_PQ_FLAG_DLC_DCR)
    {
        HalDispPqSetDlcDcrConfig(&pstPqHwCtx->stDlcDcrCfg);
    }

    if(PqType & E_HAL_DISP_PQ_FLAG_LPF)
    {
        HalDispPqSetLpfConfig(&pstPqHwCtx->stLpfCfg);
    }

    if(PqType & E_HAL_DISP_PQ_FLAG_HCORING)
    {
        HalDispPqSetHCoringConfig(&pstPqHwCtx->stHCoringCfg);
    }

    if(PqType & E_HAL_DISP_PQ_FLAG_PEAKING)
    {
        HalDispPqSetPeaking(&pstPqHwCtx->stPeakingCfg);
    }

    if(PqType & E_HAL_DISP_PQ_FLAG_FCC)
    {
        HalDispPqSetFccConfig(&pstPqHwCtx->stFccCfg);
    }

    if(PqType & E_HAL_DISP_PQ_FLAG_GAMMA)
    {
        HalDispPqSetGammaConfig(&pstPqHwCtx->stGammaCfg);
    }
}

int DispPqShow(char *DstBuf)
{
    u8 i;
    HalDispPqHwContext_t *pstPqCtx = NULL;
    int RetSprintf = -1;
    char *SrcBuf;

    SrcBuf = (char *)DrvDispOsMemAlloc(1024*10);

    if(SrcBuf)
    {
        HalDispPqGetHwCtx(&pstPqCtx);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "Flag:%x \n", pstPqCtx->enFlag);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "------------ BW Extension ------------ \n");
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "bs2sbri", pstPqCtx->stBwExtCfg.u8Bw2sbri);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "ble_en", pstPqCtx->stBwExtCfg.u8BleEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "black_start", pstPqCtx->stBwExtCfg.u8BlackStart);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "black_slop", pstPqCtx->stBwExtCfg.u8BlackSlop);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "wle_en", pstPqCtx->stBwExtCfg.u8WleEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "white_start", pstPqCtx->stBwExtCfg.u8WhiteStart);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "white_slop", pstPqCtx->stBwExtCfg.u8WhiteSlop);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "------------      LPF     ------------ \n");
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "LpfY", pstPqCtx->stLpfCfg.u8LpfY);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "LpfC", pstPqCtx->stLpfCfg.u8LpfC);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "------------   DLC / DCR   ------------ \n");
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dcr_En", pstPqCtx->stDlcDcrCfg.u16DcrEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dcr_Offset", pstPqCtx->stDlcDcrCfg.u16DcrOffset);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dlc_Hac", pstPqCtx->stDlcDcrCfg.u16Hactive);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dlc_Vac", pstPqCtx->stDlcDcrCfg.u16Vactive);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dlc_Hblank", pstPqCtx->stDlcDcrCfg.u16Hblank);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dlc_PwmDuty", pstPqCtx->stDlcDcrCfg.u16PwmDuty);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dlc_PwmPeriod", pstPqCtx->stDlcDcrCfg.u16PwmPeriod);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dlc_En", pstPqCtx->stDlcDcrCfg.u16DlcEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dlc_Offset", pstPqCtx->stDlcDcrCfg.u16DlcOffset);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dlc_Gain", pstPqCtx->stDlcDcrCfg.u16DlcGain);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dlc_YGainOffset", pstPqCtx->stDlcDcrCfg.u16YGainOffset);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "Dlc_YGain", pstPqCtx->stDlcDcrCfg.u16YGain);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "------------    H Coring    ------------ \n");
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "y_band1_h_coring_en", pstPqCtx->stHCoringCfg.u8YBand1HCoringEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "y_band2_h_coring_en", pstPqCtx->stHCoringCfg.u8YBand2HCoringEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "h_coring_y_dither_en", pstPqCtx->stHCoringCfg.u8HCoringYDither_En);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "y_table_step", pstPqCtx->stHCoringCfg.u8YTableStep);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "c_band1_h_coring_en", pstPqCtx->stHCoringCfg.u8CBand1HCoringEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "c_band2_h_coring_en", pstPqCtx->stHCoringCfg.u8CBand2HCoringEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "h_coring_c_dither_en", pstPqCtx->stHCoringCfg.u8HCoringCDither_En);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "c_table_step", pstPqCtx->stHCoringCfg.u8CTableStep);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "pc_mode", pstPqCtx->stHCoringCfg.u8PcMode);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "high_pass_en", pstPqCtx->stHCoringCfg.u8HighPassEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "coring_table0", pstPqCtx->stHCoringCfg.u8CoringTable0);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "coring_table1", pstPqCtx->stHCoringCfg.u8CoringTable1);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "coring_table2", pstPqCtx->stHCoringCfg.u8CoringTable2);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "coring_table3", pstPqCtx->stHCoringCfg.u8CoringTable3);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "------------    Peaking    ------------ \n");
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "peak_en", pstPqCtx->stPeakingCfg.u8PeakingEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "lti_en", pstPqCtx->stPeakingCfg.u8LtiEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "lti_median_filter_on", pstPqCtx->stPeakingCfg.u8LtiMediaFilterOn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cit_en", pstPqCtx->stPeakingCfg.u8CtiEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cti_median_filter_on", pstPqCtx->stPeakingCfg.u8CtiMediaFilterOn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "diff_adap_en", pstPqCtx->stPeakingCfg.u8DiffAdapEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "band1_coef", pstPqCtx->stPeakingCfg.u8Band1Coef);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "band1_step", pstPqCtx->stPeakingCfg.u8Band1Step);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "band2_coef", pstPqCtx->stPeakingCfg.u8Band2Coef);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "band2_step", pstPqCtx->stPeakingCfg.u8Band2Step);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "lti_coef", pstPqCtx->stPeakingCfg.u8LtiCoef);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "lti_step", pstPqCtx->stPeakingCfg.u8LtiStep);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "peaking_term1_sel", pstPqCtx->stPeakingCfg.u8PeakingTerm1Sel);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "peaking_term2_sel", pstPqCtx->stPeakingCfg.u8PeakingTerm2Sel);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "peaking_term3_sel", pstPqCtx->stPeakingCfg.u8PeakingTerm3Sel);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "peaking_term4_sel", pstPqCtx->stPeakingCfg.u8PeakingTerm4Sel);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "coring_th1", pstPqCtx->stPeakingCfg.u8CoringTh1);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "coring_th2", pstPqCtx->stPeakingCfg.u8CoringTh2);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cti_coef", pstPqCtx->stPeakingCfg.u8CtiCoef);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cti_step", pstPqCtx->stPeakingCfg.u8CtiStep);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "band1_pos_limit_th", pstPqCtx->stPeakingCfg.u8Band1PosLimitTh);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "band1_neg_limit_th", pstPqCtx->stPeakingCfg.u8Band1NegLimitTh);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "band2_pos_limit_th", pstPqCtx->stPeakingCfg.u8Band2PosLimitTh);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "band2_neg_limit_th", pstPqCtx->stPeakingCfg.u8Band2NegLimitTh);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "pos_limit_th", pstPqCtx->stPeakingCfg.u8PosLimitTh);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "neg_limit_th", pstPqCtx->stPeakingCfg.u8NegLimitTh);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "band2_diff_adp_en", pstPqCtx->stPeakingCfg.u8Band2DiffAdapEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "band1_diff_adp_en", pstPqCtx->stPeakingCfg.u8Band1DiffAdapEn);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "------------    FCC    ------------ \n");
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cb_t1", pstPqCtx->stFccCfg.u8Cb_T1);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cr_t1", pstPqCtx->stFccCfg.u8Cr_T1);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cb_t2", pstPqCtx->stFccCfg.u8Cb_T2);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cr_t2", pstPqCtx->stFccCfg.u8Cr_T2);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cb_t3", pstPqCtx->stFccCfg.u8Cb_T3);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cr_t3", pstPqCtx->stFccCfg.u8Cr_T3);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cb_t4", pstPqCtx->stFccCfg.u8Cb_T4);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cr_t4", pstPqCtx->stFccCfg.u8Cr_T4);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cb_t5", pstPqCtx->stFccCfg.u8Cb_T5);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cr_t5", pstPqCtx->stFccCfg.u8Cr_T5);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cb_t6", pstPqCtx->stFccCfg.u8Cb_T6);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cr_t6", pstPqCtx->stFccCfg.u8Cr_T6);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cb_t7", pstPqCtx->stFccCfg.u8Cb_T7);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cr_t7", pstPqCtx->stFccCfg.u8Cr_T7);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cb_t8", pstPqCtx->stFccCfg.u8Cb_T8);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cr_t8", pstPqCtx->stFccCfg.u8Cr_T8);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cbcr_d1d_d1u", pstPqCtx->stFccCfg.u8CbCr_D1D_D1U);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cbcr_d2d_d2u", pstPqCtx->stFccCfg.u8CbCr_D2D_D2U);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cbcr_d3d_d3u", pstPqCtx->stFccCfg.u8CbCr_D3D_D3U);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cbcr_d4d_d4u", pstPqCtx->stFccCfg.u8CbCr_D4D_D4U);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cbcr_d5d_d5u", pstPqCtx->stFccCfg.u8CbCr_D5D_D5U);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cbcr_d6d_d6u", pstPqCtx->stFccCfg.u8CbCr_D6D_D6U);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cbcr_d7d_d7u", pstPqCtx->stFccCfg.u8CbCr_D7D_D7U);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cbcr_d8d_d8u", pstPqCtx->stFccCfg.u8CbCr_D8D_D8U);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "cbcr_d9", pstPqCtx->stFccCfg.u8CbCr_D9);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "k_t2_k_t1", pstPqCtx->stFccCfg.u8K_T2_K_T1);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "k_t4_k_t3", pstPqCtx->stFccCfg.u8K_T4_K_T3);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "k_t6_k_t5", pstPqCtx->stFccCfg.u8K_T6_K_T5);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "k_t8_k_t7", pstPqCtx->stFccCfg.u8K_T8_K_T7);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "fcc_en", pstPqCtx->stFccCfg.u8En);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "------------    Gamma    ------------ \n");
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s = %04x\n", "gamma_en", pstPqCtx->stGammaCfg.u8En);
        DISPUT_SPRINTF_STRCAT(SrcBuf, "%-25s\n", "gamma_r");
        for(i =0; i< 33; i++)
        {
            DISPUT_SPRINTF_STRCAT(SrcBuf, "%02x, ", pstPqCtx->stGammaCfg.u8R[i]);
        }
        DISPUT_SPRINTF_STRCAT(SrcBuf, "\n%-25s\n", "gamma_g");
        for(i =0; i< 33; i++)
        {
            DISPUT_SPRINTF_STRCAT(SrcBuf, "%02x, ", pstPqCtx->stGammaCfg.u8G[i]);
        }
        DISPUT_SPRINTF_STRCAT(SrcBuf, "\n%-25s\n", "gamma_b");
        for(i =0; i< 33; i++)
        {
            DISPUT_SPRINTF_STRCAT(SrcBuf, "%02x, ", pstPqCtx->stGammaCfg.u8B[i]);
        }
        DISPUT_SPRINTF_STRCAT(SrcBuf, "\n");
        RetSprintf = DISPUT_SPRINTF(DstBuf, "%s", SrcBuf);
        DrvDispOsMemRelease(SrcBuf);
    }
    return RetSprintf;
}

void _DispSysFsParsingCommand(char *str, DispSysFsStrConfig_t *pstStrCfg)
{
    char del[] = " ";
    int len;

    pstStrCfg->argc = _DispSysFsSplit(pstStrCfg->argv, (char *)str, del);
    len = strlen(pstStrCfg->argv[pstStrCfg->argc-1]);
    pstStrCfg->argv[pstStrCfg->argc-1][len-1] = '\0';
}

//------------------------------------------------------------------------------
ssize_t check_dispdbgmg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        DispSysFsStrConfig_t stStringCfg;

        _DispSysFsParsingCommand((char *)buf, &stStringCfg);
        DispDbgmgStore(&stStringCfg);
        return n;
    }
    return 0;
}

ssize_t check_dispdbgmg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return DispDbgmgShow(buf);
}

static DEVICE_ATTR(dbgmg,0644, check_dispdbgmg_show, check_dispdbgmg_store);


ssize_t check_dispptgen_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        DispSysFsStrConfig_t stStringCfg;

        _DispSysFsParsingCommand((char *)buf, &stStringCfg);
        DispPtGenStore(&stStringCfg);
        return n;
    }
    return 0;
}

ssize_t check_dispptgen_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return DispPtGenShow(buf);
}

static DEVICE_ATTR(ptgen,0644, check_dispptgen_show, check_dispptgen_store);


ssize_t check_disppq_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        DispSysFsStrConfig_t stStringCfg;

        _DispSysFsParsingCommand((char *)buf, &stStringCfg);

        DispPqStore(&stStringCfg);

        return n;
    }
    return 0;
}

ssize_t check_disppq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return DispPqShow(buf);
}

static DEVICE_ATTR(pq,0644, check_disppq_show, check_disppq_store);


ssize_t check_dispclk_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        DispSysFsStrConfig_t stStringCfg;

        _DispSysFsParsingCommand((char *)buf, &stStringCfg);
        DispClkStore(&stStringCfg);
        return n;
    }
    return 0;
}

ssize_t check_dispclk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return DispClkShow(buf);
}

static DEVICE_ATTR(clk,0644, check_dispclk_show, check_dispclk_store);



void DrvDispSysfsInit(struct device *device)
{
    CamDeviceCreateFile(device, &dev_attr_dbgmg);
    CamDeviceCreateFile(device, &dev_attr_ptgen);
    CamDeviceCreateFile(device, &dev_attr_pq);
    CamDeviceCreateFile(device, &dev_attr_clk);
}
