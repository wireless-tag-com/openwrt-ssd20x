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

#define _GOP_UT_C


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

#include "cam_os_wrapper.h"

#include "mhal_rgn.h"
#include "mhal_rgn_datatype.h"
#include "gop_ut.h"

#include "ms_msys.h"
#include "ms_platform.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
bool bGopUtInit=0;



//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
int _GopUtSplit(char **arr, char *str,  char* del)
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


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
#ifdef GOP_OS_TYPE_LINUX_KERNEL_TEST
typedef struct
{
    MSYS_DMEM_INFO stDmem;
    MS_BOOL bUsed;
}GopUtDmemConfig_t;


GopUtDmemConfig_t stGOPDmemCfg[20];
static MS_U16 Palette_I2[4][2] =
{{0xFFFF,0x00FF}, {0x0000,0x00FF}, {0xFF00,0x0000}, {0x00FF,0x0000}};

static MS_U16 Palette_I4[16][2] =
{{0x0000,0x0000}, {0x0000,0x0080}, {0x8000,0x0000}, {0x0080,0x0000},
 {0x8080,0x0000}, {0x0080,0x0080}, {0x8000,0x0080}, {0x8080,0x0080},
 {0x00FF,0x0000}, {0xFF00,0x0000}, {0x0000,0x00FF}, {0xFFFF,0x0000},
 {0x00FF,0x00FF}, {0xFF00,0x00FF}, {0xC0C0,0x00C0}, {0xFFFF,0x00FF}};

static MS_U16 Palette_I8[256][2] =
{{0x0000,0x0000}, {0x0000,0x0080}, {0x8000,0x0000}, {0x8000,0x0080}, {0x0080,0x0000}, {0x0080,0x0080}, {0x8080,0x0000}, {0xc0c0,0x00c0},
 {0xdcc0,0x00c0}, {0xcaf0,0x00a6}, {0x2000,0x0040}, {0x2000,0x0060}, {0x2000,0x0080}, {0x2000,0x00a0}, {0x2000,0x00c0}, {0x2000,0x00e0},
 {0x4000,0x0000}, {0x4000,0x0020}, {0x4000,0x0040}, {0x4000,0x0060}, {0x4000,0x0080}, {0x4000,0x00a0}, {0x4000,0x00c0}, {0x4000,0x00e0},
 {0x6000,0x0000}, {0x6000,0x0020}, {0x6000,0x0040}, {0x6000,0x0060}, {0x6000,0x0080}, {0x6000,0x00a0}, {0x6000,0x00c0}, {0x6000,0x00e0},
 {0x8000,0x0000}, {0x8000,0x0020}, {0x8000,0x0040}, {0x8000,0x0060}, {0x8000,0x0080}, {0x8000,0x00a0}, {0x8000,0x00c0}, {0x8000,0x00e0},
 {0xa000,0x0000}, {0xa000,0x0020}, {0xa000,0x0040}, {0xa000,0x0060}, {0xa000,0x0080}, {0xa000,0x00a0}, {0xa000,0x00c0}, {0xa000,0x00e0},
 {0xc000,0x0000}, {0xc000,0x0020}, {0xc000,0x0040}, {0xc000,0x0060}, {0xc000,0x0080}, {0xc000,0x00a0}, {0xc000,0x00c0}, {0xc000,0x00e0},
 {0xe000,0x0000}, {0xe000,0x0020}, {0xe000,0x0040}, {0xe000,0x0060}, {0xe000,0x0080}, {0xe000,0x00a0}, {0xe000,0x00c0}, {0xe000,0x00e0}, // h3F
 {0x0040,0x0000}, {0x0040,0x0020}, {0x0040,0x0040}, {0x0040,0x0060}, {0x0040,0x0080}, {0x0040,0x00a0}, {0x0040,0x00c0}, {0x0040,0x00e0},
 {0x2040,0x0000}, {0x2040,0x0020}, {0x2040,0x0040}, {0x2040,0x0060}, {0x2040,0x0080}, {0x2040,0x00a0}, {0x2040,0x00c0}, {0x2040,0x00e0},
 {0x4040,0x0000}, {0x4040,0x0020}, {0x4040,0x0040}, {0x4040,0x0060}, {0x4040,0x0080}, {0x4040,0x00a0}, {0x4040,0x00c0}, {0x4040,0x00e0},
 {0x6040,0x0000}, {0x6040,0x0020}, {0x6040,0x0040}, {0x6040,0x0060}, {0x6040,0x0080}, {0x6040,0x00a0}, {0x6040,0x00c0}, {0x6040,0x00e0},
 {0x8040,0x0000}, {0x8040,0x0020}, {0x8040,0x0040}, {0x8040,0x0060}, {0x8040,0x0080}, {0x8040,0x00a0}, {0x8040,0x00c0}, {0x8040,0x00e0},
 {0xa040,0x0000}, {0xa040,0x0020}, {0xa040,0x0040}, {0xa040,0x0060}, {0xa040,0x0080}, {0xa040,0x00a0}, {0xa040,0x00c0}, {0xa040,0x00e0},
 {0xc040,0x0000}, {0xc040,0x0020}, {0xc040,0x0040}, {0xc040,0x0060}, {0xc040,0x0080}, {0xc040,0x00a0}, {0xc040,0x00c0}, {0xc040,0x00e0},
 {0xe040,0x0000}, {0xe040,0x0020}, {0xe040,0x0040}, {0xe040,0x0060}, {0xe040,0x0080}, {0xe040,0x00a0}, {0xe040,0x00c0}, {0xe040,0x00e0}, // h7F
 {0x0080,0x0000}, {0x0080,0x0020}, {0x0080,0x0040}, {0x0080,0x0060}, {0x0080,0x0080}, {0x0080,0x00a0}, {0x0080,0x00c0}, {0x0080,0x00e0},
 {0x2080,0x0000}, {0x2080,0x0020}, {0x2080,0x0040}, {0x2080,0x0060}, {0x2080,0x0080}, {0x2080,0x00a0}, {0x2080,0x00c0}, {0x2080,0x00e0},
 {0x4080,0x0000}, {0x4080,0x0020}, {0x4080,0x0040}, {0x4080,0x0060}, {0x4080,0x0080}, {0x4080,0x00a0}, {0x4080,0x00c0}, {0x4080,0x00e0},
 {0x6080,0x0000}, {0x6080,0x0020}, {0x6080,0x0040}, {0x6080,0x0060}, {0x6080,0x0080}, {0x6080,0x00a0}, {0x6080,0x00c0}, {0x6080,0x00e0},
 {0x8080,0x0000}, {0x8080,0x0020}, {0x8080,0x0040}, {0x8080,0x0060}, {0x8080,0x0080}, {0x8080,0x00a0}, {0x8080,0x00c0}, {0x8080,0x00e0},
 {0xa080,0x0000}, {0xa080,0x0020}, {0xa080,0x0040}, {0xa080,0x0060}, {0xa080,0x0080}, {0xa080,0x00a0}, {0xa080,0x00c0}, {0xa080,0x00e0},
 {0xc080,0x0000}, {0xc080,0x0020}, {0xc080,0x0040}, {0xc080,0x0060}, {0xc080,0x0080}, {0xc080,0x00a0}, {0xc080,0x00c0}, {0xc080,0x00e0},
 {0xe080,0x0000}, {0xe080,0x0020}, {0xe080,0x0040}, {0xe080,0x0060}, {0xe080,0x0080}, {0xe080,0x00a0}, {0xe080,0x00c0}, {0xe080,0x00e0}, // hBF
 {0x00c0,0x0000}, {0x00c0,0x0020}, {0x00c0,0x0040}, {0x00c0,0x0060}, {0x00c0,0x0080}, {0x00c0,0x00a0}, {0x00c0,0x00c0}, {0x00c0,0x00e0},
 {0x20c0,0x0000}, {0x20c0,0x0020}, {0x20c0,0x0040}, {0x20c0,0x0060}, {0x20c0,0x0080}, {0x20c0,0x00a0}, {0x20c0,0x00c0}, {0x20c0,0x00e0},
 {0x40c0,0x0000}, {0x40c0,0x0020}, {0x40c0,0x0040}, {0x40c0,0x0060}, {0x40c0,0x0080}, {0x40c0,0x00a0}, {0x40c0,0x00c0}, {0x40c0,0x00e0},
 {0x60c0,0x0000}, {0x60c0,0x0020}, {0x60c0,0x0040}, {0x60c0,0x0060}, {0x60c0,0x0080}, {0x60c0,0x00a0}, {0x60c0,0x00c0}, {0x60c0,0x00e0},
 {0x80c0,0x0000}, {0x80c0,0x0020}, {0x80c0,0x0040}, {0x80c0,0x0060}, {0x80c0,0x0080}, {0x80c0,0x00a0}, {0x80c0,0x00c0}, {0x80c0,0x00e0},
 {0xa0c0,0x0000}, {0xa0c0,0x0020}, {0xa0c0,0x0040}, {0xa0c0,0x0060}, {0xa0c0,0x0080}, {0xa0c0,0x00a0}, {0xa0c0,0x00c0}, {0xa0c0,0x00e0},
 {0xc0c0,0x0000}, {0xc0c0,0x0020}, {0xc0c0,0x0040}, {0xc0c0,0x0060}, {0xc0c0,0x0080}, {0xc0c0,0x00a0}, {0xfbf0,0x00ff}, {0xa0a4,0x00a0},
 {0x8080,0x0080}, {0x0000,0x00ff}, {0xff00,0x0000}, {0xff00,0x00ff}, {0x00ff,0x0000}, {0x00ff,0x00ff}, {0xffff,0x0000}, {0xffff,0x00ff}};


MS_S32 _GopUtDmemAlloc(MS_U8 *ppu8Name, MS_U32 size, unsigned long long *pu64PhyAddr)
{
    MS_S32 sRet = 0;
    static MS_U8 u8NullNameIdx = 0;
    MSYS_DMEM_INFO dmem;
    MS_U8 u8Name[16][16] =
    {
        "GOPUT_MEM_00", "GOPUT_MEM_01", "GOPUT_MEM_02", "GOPUT_MEM_03",
        "GOPUT_MEM_04", "GOPUT_MEM_05", "GOPUT_MEM_06", "GOPUT_MEM_07",
        "GOPUT_MEM_08", "GOPUT_MEM_09", "GOPUT_MEM_0A", "GOPUT_MEM_0B",
        "GOPUT_MEM_0C", "GOPUT_MEM_0D", "GOPUT_MEM_0E", "GOPUT_MEM_0F",
    };

    *pu64PhyAddr = 0;
    if(ppu8Name == NULL)
    {
        memcpy(dmem.name, u8Name[u8NullNameIdx], strlen(u8Name[u8NullNameIdx])+1);
        u8NullNameIdx = (u8NullNameIdx + 1) & 0x0F;
        CamOsPrintf("NULL Name =%s\n", dmem.name);
    }
    else
    {
        memcpy(dmem.name,ppu8Name,strlen(ppu8Name)+1);
    }

    dmem.length = size;
    if(0 != msys_request_dmem(&dmem))
    {
        *pu64PhyAddr = 0;
        sRet = 1;
    }
    else
    {
        MS_U8 i;

        for(i=0; i<20; i++)
        {
            if(stGOPDmemCfg[i].bUsed == 0)
            {
                stGOPDmemCfg[i].bUsed = 1;
                memcpy(&stGOPDmemCfg[i].stDmem, &dmem, sizeof(MSYS_DMEM_INFO));
                sRet = 0;
                *pu64PhyAddr = Chip_Phys_to_MIU(dmem.phys);
                CamOsPrintf("%s %d, Addr=%llx\n", __FUNCTION__, __LINE__, *pu64PhyAddr);
                break;
            }
        }

    }
    return sRet;
}



MS_S32 _GopUtDMemFree(unsigned long long u64PhyAddr)
{
    MSYS_DMEM_INFO dmem;
    MS_U8 i;
    MS_S32 sRet = 1;

    for(i=0; i<20; i++)
    {
        if(stGOPDmemCfg[i].bUsed  == 1 && stGOPDmemCfg[i].stDmem.phys == u64PhyAddr)
        {
            memcpy(&dmem, &stGOPDmemCfg[i].stDmem, sizeof(MSYS_DMEM_INFO));
            msys_release_dmem(&dmem);
            stGOPDmemCfg[i].bUsed = 0;
            sRet = 0;
            break;
        }
    }
    return sRet;
}
void _GopUtSetPalette(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopPixelFormat_e eFormat)
{
    int i;
    switch(eFormat) {
        case E_MHAL_RGN_PIXEL_FORMAT_I2:
            CamOsPrintf("Set I2 Palette...\r\n");
            for(i=0;i<4;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I2[i][1]>>8), (MS_U8)(Palette_I2[i][1]&0xFF),
                                               (MS_U8)(Palette_I2[i][0]>>8), (MS_U8)(Palette_I2[i][0]&0xFF), (MS_U8)i);
            }
            MHAL_RGN_GopSetPalette(eGopId, (MS_U8)0, (MS_U8)0,
                                           (MS_U8)0, (MS_U8)0, (MS_U8)255);
            break;
        case E_MHAL_RGN_PIXEL_FORMAT_I4:
            CamOsPrintf("Set I4 Palette...\r\n");
            for(i=0;i<16;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I4[i][1]>>8), (MS_U8)(Palette_I4[i][1]&0xFF),
                                               (MS_U8)(Palette_I4[i][0]>>8), (MS_U8)(Palette_I4[i][0]&0xFF), (MS_U8)i);
            }
            MHAL_RGN_GopSetPalette(eGopId, (MS_U8)0, (MS_U8)0,
                                           (MS_U8)0, (MS_U8)0, (MS_U8)255);
            break;
        case E_MHAL_RGN_PIXEL_FORMAT_I8:
            CamOsPrintf("Set I8 Palette...\r\n");
            for(i=0;i<256;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I8[i][1]>>8), (MS_U8)(Palette_I8[i][1]&0xFF),
                                               (MS_U8)(Palette_I8[i][0]>>8), (MS_U8)(Palette_I8[i][0]&0xFF), (MS_U8)i);
            }
            break;
        default:
            CamOsPrintf("No Palette!Fmt=%d\r\n",eFormat);
            break;
    }
}

struct file *_GopUtOpenFile(char *path,int flag,int mode)
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
        CamOsPrintf("%s %d, Open File Fail\n", __FUNCTION__, __LINE__);
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}
unsigned long _GopUtGetileSize(char *path)
{
    struct path p;
    struct kstat ks;
    unsigned long filesize;

    kern_path(path, 0, &p);
    vfs_getattr(&p, &ks);
    filesize =  ks.size;
    return filesize;

}

bool _GopUtDmemGetVirAddr(unsigned long long u64PhyAddr, MS_U32 u32Size, unsigned long long *pu64VirAddr)
{
    MS_U16 i;
    MS_BOOL bRet = 0;
    for(i=0; i<16; i++)
    {
        if(stGOPDmemCfg[i].bUsed == 1 && Chip_Phys_to_MIU(stGOPDmemCfg[i].stDmem.phys) == u64PhyAddr)
        {
            *pu64VirAddr = stGOPDmemCfg[i].stDmem.kvirt;
            bRet = 1;
            break;
        }
    }
    return bRet;
}

int _GopUtReadFile(struct file *fp,char *buf,int readlen)
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

void _GopUtInit(void)
{

}

void _GopUtQuickTest(GopUtStrConfig_t *pstStrCfg)
{
    int  u16ImgWidth =0;
    int  u16ImgHeight = 0;
    int  u16StrWidth = 0;
    int  u16StrHeight = 0;
    int  u16DispPitch = 0;
    int ret, FileSize;
    char *pFileName = NULL;
    unsigned long long u64PhyAddr = 0;
    unsigned long long u64VirAddr = 0;
    char *pBuf;
    struct file *pReadFile = NULL;
    MHAL_RGN_GopType_e eGopId;
    MHAL_RGN_GopGwinId_e eGwinId;
    MHAL_RGN_GopWindowConfig_t ptSrcWinCfg;
    MHAL_RGN_GopPixelFormat_e eFormat;
    MHAL_RGN_GopGwinAlphaType_e eAlphaType;
    MS_U8 u8ConstAlphaVal;

    if(pstStrCfg->argc == 1)
    {

    }
    else if(pstStrCfg->argc == 8)
    {
        pFileName = pstStrCfg->argv[1];
        ret = kstrtol(pstStrCfg->argv[2], 10, (long *)&u16ImgWidth);
        ret = kstrtol(pstStrCfg->argv[3], 10, (long *)&u16ImgHeight);
        ret = kstrtol(pstStrCfg->argv[4], 10, (long *)&u16DispPitch);
        ret = kstrtol(pstStrCfg->argv[5], 10, (long *)&u16StrWidth);
        ret = kstrtol(pstStrCfg->argv[6], 10, (long *)&u16StrHeight);
        ret = kstrtol(pstStrCfg->argv[7], 10, (long *)&eFormat);

    }
    else
    {
        CamOsPrintf("gop [file] [image width] [image height] [Pitch] [Stretch win width] [Stretch win height] [Format]\n");
        return;
    }

    if(pFileName == NULL)
    {
        CamOsPrintf("input file is NULL\n");
        return;
    }
    else
    {
        pReadFile = _GopUtOpenFile(pFileName, O_RDONLY, 0);

        if(pReadFile == NULL)
        {
            CamOsPrintf("%s %d, OpenFile Fail\n", __FUNCTION__, __LINE__);
            return;
        }
        FileSize = _GopUtGetileSize(pFileName);

        _GopUtDmemAlloc("GOP", FileSize, &u64PhyAddr);

        _GopUtDmemGetVirAddr(u64PhyAddr, FileSize, &u64VirAddr);

        pBuf = (char *)(u32)u64VirAddr;

        _GopUtReadFile(pReadFile, pBuf, FileSize);

        CamOsPrintf("%s %d, Phy:%x, Vir:%x, FileSize=%x\n", __FUNCTION__, __LINE__, (u32)u64PhyAddr, (u32)u64VirAddr, FileSize);


        eGopId=E_MHAL_GOP_VPE_PORT0;
        eGwinId=E_MHAL_GOP_GWIN_ID_0;
        ptSrcWinCfg.u32X=0;
        ptSrcWinCfg.u32Y=0;
        ptSrcWinCfg.u32Width=u16StrWidth;
        ptSrcWinCfg.u32Height=u16StrHeight;
        eAlphaType=E_MHAL_GOP_GWIN_ALPHA_CONSTANT;
        u8ConstAlphaVal=0xFF;

        MHAL_RGN_GopInit();

        _GopUtSetPalette(eGopId, E_MHAL_RGN_PIXEL_FORMAT_I8);

        MHAL_RGN_GopSetBaseWindow(eGopId, &ptSrcWinCfg, &ptSrcWinCfg);

        MHAL_RGN_GopGwinSetPixelFormat(eGopId, eGwinId, eFormat);
        MHAL_RGN_GopGwinSetWindow(eGopId, eGwinId, u16ImgWidth, u16ImgHeight, u16DispPitch, 0, 0);

        MHAL_RGN_GopGwinSetBuffer(eGopId, eGwinId, u64PhyAddr);

        MHAL_RGN_GopSetAlphaZeroOpaque(eGopId, FALSE, TRUE, eFormat);
        MHAL_RGN_GopSetAlphaType(eGopId, eGwinId, eAlphaType, u8ConstAlphaVal);
        MHAL_RGN_GopGwinEnable(eGopId,eGwinId);


    }

}
void GopTestStore(GopUtStrConfig_t *pstStringCfg)
{
    if(pstStringCfg->argc < 1)
    {
        return;
    }

    if(strcmp(pstStringCfg->argv[0], "test") == 0)
    {
        _GopUtQuickTest(pstStringCfg);
    }

}
int GopTestShow(char *DstBuf)
{
    int RetSprintf = 0 ;
/*    char *SrcBuf = NULL;

    SrcBuf = CamOsMemAlloc(1024*5);

    if(SrcBuf)
    {
        HDMITXUT_SPRINTF_STRCAT(SrcBuf, "------------------- HDMITX UT ------------------- \n");

        RetSprintf = HDMITXUT_SPRINTF(DstBuf, "%s", SrcBuf);
        CamOsMemRelease(SrcBuf);
    }*/
    return RetSprintf;

}
#endif

void GopUtParsingCommand(char *str, GopUtStrConfig_t *pstStrCfg)
{
    char del[] = " ";
    int len;

    pstStrCfg->argc = _GopUtSplit(pstStrCfg->argv, (char *)str, del);
    len = strlen(pstStrCfg->argv[pstStrCfg->argc-1]);
    pstStrCfg->argv[pstStrCfg->argc-1][len-1] = '\0';
}

