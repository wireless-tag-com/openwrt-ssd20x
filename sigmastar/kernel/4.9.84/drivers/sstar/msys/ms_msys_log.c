/*
* ms_msys_log.c- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
//#define FUNC_DEBUG_LEVEL
//#define FUNC_STORE_PATH

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   mi_vdec_impl.c
/// @brief vdec module impl
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <ms_msys.h>
#include <linux/version.h>


/* Porting
#include "mi/mi_sys_datatype.h"
#include "mi/mi_common.h"
*/

#if !defined(TRUE) && !defined(FALSE)
#define TRUE                                     1
#define FALSE                                    0
#endif
#define MI_SUCCESS              (0)
#define MI_ERR_SYS_FAILED       (-1)
#define MI_ERR_SYS_NOT_PERM     (-1)
typedef unsigned char                            MI_U8;         // 1 byte
typedef unsigned int                             MI_U32;        // 4 bytes
typedef signed char                              MI_S8;         // 1 byte
typedef signed int                               MI_S32;        // 4 bytes
typedef unsigned long long                       MI_PHY;        // 8 bytes
typedef unsigned char                            MI_BOOL;
#define MSG_FROM_USER   0
#define MSG_FROM_KERNEL 1


#define LOG_RINGBUFFER_SIZE (256 * 1024)
#define DEBUG_MSG_BUF_SIZE  (1024 * 4)
#define MI_LOG_LEVEL_DEFAULT E_MI_ERR_LEVEL_ERROR
#define MAX_FILENAME_LENTH 256



#ifdef  FUNC_DEBUG_LEVEL
#include "mi/mi_print.h"
#include "mi/mi_common_macro.h"

void _MSYS_IMPL_ModuleIdToPrefixName(MI_ModuleId_e eModuleId , char *prefix_name)
{
    switch(eModuleId)
    {
        case E_MI_MODULE_ID_IVE:
            strcpy(prefix_name,"mi_ive");
            break;
        case E_MI_MODULE_ID_VDF:
            strcpy(prefix_name,"mi_vdf");
            break;
        case E_MI_MODULE_ID_VENC:
            strcpy(prefix_name,"mi_venc");
            break;
        case E_MI_MODULE_ID_RGN:
            strcpy(prefix_name,"mi_rgn");
            break;
        case E_MI_MODULE_ID_AI:
            strcpy(prefix_name,"mi_ai");
            break;
        case E_MI_MODULE_ID_AO:
            strcpy(prefix_name,"mi_ao");
            break;
        case E_MI_MODULE_ID_VIF:
            strcpy(prefix_name,"mi_vif");
            break;
        case E_MI_MODULE_ID_VPE:
            strcpy(prefix_name,"mi_vpe");
            break;
        case E_MI_MODULE_ID_VDEC:
            strcpy(prefix_name,"mi_vdec");
            break;
        case E_MI_MODULE_ID_SYS:
            strcpy(prefix_name,"mi_sys");
            break;
         case E_MI_MODULE_ID_FB:
            strcpy(prefix_name,"mi_fb");
            break;
         case E_MI_MODULE_ID_HDMI:
            strcpy(prefix_name,"mi_hdmi");
            break;
         case E_MI_MODULE_ID_DIVP:
            strcpy(prefix_name,"mi_divp");
            break;
         case E_MI_MODULE_ID_GFX:
            strcpy(prefix_name,"mi_gfx");
            break;
         case E_MI_MODULE_ID_VDISP:
            strcpy(prefix_name,"mi_vdisp");
            break;
        case E_MI_MODULE_ID_DISP:
            strcpy(prefix_name,"mi_disp");
            break;
        case E_MI_MODULE_ID_OS:
            strcpy(prefix_name,"mi_os");
            break;
        case E_MI_MODULE_ID_IAE:
            strcpy(prefix_name,"mi_iae");
            break;
        case E_MI_MODULE_ID_MD:
            strcpy(prefix_name,"mi_md");
            break;
        case E_MI_MODULE_ID_OD:
            strcpy(prefix_name,"mi_od");
            break;
        case E_MI_MODULE_ID_SHADOW:
            strcpy(prefix_name,"mi_shadow");
            break;
        case E_MI_MODULE_ID_WARP:
            strcpy(prefix_name,"mi_warp");
            break;
        case E_MI_MODULE_ID_UAC:
            strcpy(prefix_name,"mi_uac");
            break;
        case E_MI_MODULE_ID_LDC:
            strcpy(prefix_name,"mi_ldc");
            break;
        case E_MI_MODULE_ID_SD:
            strcpy(prefix_name,"mi_sd");
            break;
        default:
            DBG_ERR("fail,error!!!  eModuleId is %d ,bigger than E_MI_MODULE_ID_MAX=%d\n",eModuleId,E_MI_MODULE_ID_MAX);
            MI_SYS_BUG_ON(1);
            break;

    }
    return;
}

#else
#define MI_PRINT printk
#define DBG_INFO printk
#define DBG_WRN printk
#define DBG_ERR printk
#endif


typedef struct MI_SYS_LogBufferInfo_S
{
    MI_PHY phyStartAddrPhy; /*start physic address*/    /*CNcomment:»º³åÇøÆðÊ¼ÎïÀíµØÖ·*/
    MI_U8 *pu8StartAddrVir; /*start virtual address*/    /*CNcomment:»º³åÇøÆðÊ¼ÐéÄâµØÖ·*/
    MI_U32 u32BufSize;      /*buffer size*/                /*CNcomment:»º³åÇø´óÐ¡*/
    MI_U32 u32WriteAddr;    /*write offset*/            /*CNcomment:Ð´µØÖ·Æ«ÒÆ*/
    MI_U32 u32ReadAddr;     /*read offset*/                /*CNcomment:¶ÁµØÖ·Æ«ÒÆ*/
    MI_U32 u32ResetFlag;    /*reset count*/                /*CNcomment:¸´Î»´ÎÊý*/
    MI_U32 u32WriteCount;   /*write count*/                /*CNcomment:Ð´Èë´ÎÊý*/
    wait_queue_head_t wqNoData;    /*no wait queque*/    /*CNcomment:Ã»ÓÐÊý¾ÝµÈ´ý¶ÓÁÐ*/
    struct semaphore semWrite;     /*write semaphore*/    /*CNcomment:Ð´bufferÐÅºÅÁ¿*/
}MI_SYS_LogBufferInfo_t;

#ifdef  FUNC_DEBUG_LEVEL
/*structure of mode log level */
typedef struct MI_SYS_LogConfigInfo_S
{
    MI_U8 u8ModName[16+12];     /*mode name 16 + '_' 1 + pid 10 */
    MI_DBG_LEVEL_e eLogLevel;    /*log level*//*CNcomment:  Ä£¿é´òÓ¡¼¶±ð¿ØÖÆ */
    MI_SYS_LogOutputPos_e eLogPrintPos;      /*log output location, 0:serial port; 1:network;2:u-disk*//*CNcomment:  Ä£¿é´òÓ¡Î»ÖÃ¿ØÖÆ 0:´®¿Ú 1:ÍøÂç 2:UÅÌ */
    MI_U8 u8UdiskFlag;        /* u-disk log flag */
    MI_U8 reserved;    /* u-disk log flag */
}MI_SYS_LogConfigInfo_t;
#define LOG_CONFIG_BUF_SIZE   (sizeof(MI_SYS_LogConfigInfo_t) * E_MI_MODULE_ID_MAX)
static MI_SYS_LogConfigInfo_t *_gpstLogConfigInfo = NULL;
char *DebugLevelName[MI_DBG_ALL+1] = {
    "NONE",
    "ERR",
    "WRN",
    "INFO",
    "ALL",
};
typedef struct MSYS_LogBuffer_s
{
    void* pStartVirAddr;
    MI_PHY phyStartPhyAddr;
    MI_U32 u32Size;
}MSYS_LogBuffer_t;
static MSYS_LogBuffer_t _gstLogBuffer;

#endif
#if 0
typedef struct MI_SYS_LogBufRead_S
{
    MI_PHY  pHyAddr;
    MI_U32  u32BufLen;
    MI_U32  u32CopyedLen;
}MI_SYS_LogBufRead_t;

typedef struct MI_SYS_LogBufWrite_S
{
    MI_PHY  pHyAddr;
    MI_U32  u32BufLen;
}MI_SYS_LogBufWrite_t;
#endif



typedef enum
{
    E_MI_SYS_LOG_OUTPUT_SERIAL = 0,
    E_MI_SYS_LOG_OUTPUT_NETWORK,
    E_MI_SYS_LOG_OUTPUT_UDISK,
    E_MI_SYS_LOG_OUTPUT_DBG,
}MI_SYS_LogOutputPos_e;



static MI_SYS_LogBufferInfo_t _gstLogBufferInfo;
static struct task_struct *gpLogUdiskTask = NULL;
static MI_U8 _gu8LogInit = 0;
static MI_BOOL _gbSetLogFileFlag = FALSE;
static MI_BOOL _gbLogOutDbg = FALSE;
static char g_szPathBuf[MAX_FILENAME_LENTH] = {0};
static char *UdiskLogFile = g_szPathBuf;
static MI_U8 _gu8LogLevel = 7;

#ifdef FUNC_STORE_PATH
static char g_szStorePathBuf[MAX_FILENAME_LENTH] = "/mnt";
char *StorePath = g_szStorePathBuf;
#endif
struct proc_dir_entry *g_pCMPI_proc = NULL;

DEFINE_SEMAPHORE(_gLogFileMutex);
DEFINE_SPINLOCK(_gLogFileLock);

#define LOG_FILE_LOCK()   down_interruptible(&_gLogFileMutex)
#define LOG_FILE_UNLOCK() up(&_gLogFileMutex)
#define LOG_MAX_TRACE_LEN 256


struct file* FileOpen(const MI_S8* ps8FileName, MI_S32 s32Flags)
{
    struct file *pFile = NULL;

    if (NULL == ps8FileName)
    {
        return NULL;
    }

    if (s32Flags == 0)
    {
        s32Flags = O_RDONLY;
    }
    else
    {
        s32Flags = O_WRONLY | O_CREAT | O_APPEND;
    }

    pFile = filp_open(ps8FileName, s32Flags | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    return (IS_ERR(pFile)) ? NULL : pFile;
}


void FileClose(struct file * pFile)
{
    if ( NULL != pFile )
    {
        filp_close(pFile, NULL);
    }

    return;
}

MI_S32 FileRead(struct file * pFile,  MI_U8* ps8Buf, MI_U32 u32Len)
{
    MI_S32 s32ReadLen = 0;
    mm_segment_t stOld_FS = {0};

    if (pFile == NULL || NULL == ps8Buf)
    {
        return -ENOENT; /* No such file or directory */
    }

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40) || LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
    if (pFile->f_op->read == NULL)
    {
        return -ENOSYS; /* Function not implemented */
    }
#endif

    if (((pFile->f_flags & O_ACCMODE) & (O_RDONLY | O_RDWR)) != 0)
    {
        return -EACCES; /* Permission denied */
    }

    /* saved the original file space */
    stOld_FS = get_fs();

    /* extend to the kernel data space */
    set_fs(KERNEL_DS);

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40) || LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
    s32ReadLen = pFile->f_op->read(pFile, ps8Buf, u32Len, &pFile->f_pos);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(4,9,84)
    s32ReadLen = vfs_read(pFile, ps8Buf, u32Len, &pFile->f_pos);
#else
#error not support this kernel version
#endif
    /* Restore the original file space */
    set_fs(stOld_FS);

    return s32ReadLen;
}


MI_S32 FileWrite(struct file* pFile, MI_S8* ps8Buf, MI_U32 u32Len , MI_BOOL bFlag)
{
    MI_S32 s32WriteLen = 0;
    mm_segment_t stOld_FS = {0};

    if (pFile == NULL || ps8Buf == NULL)
    {
        return -ENOENT; /* No such file or directory */
    }

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40) || LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
    if (pFile->f_op->write == NULL)
    {
        return -ENOSYS; /* Function not implemented */
    }
#endif

    if (((pFile->f_flags & O_ACCMODE) & (O_WRONLY | O_RDWR)) == 0)
    {
        return -EACCES; /* Permission denied */
    }

    stOld_FS = get_fs();
    set_fs(KERNEL_DS);

#if LINUX_VERSION_CODE == KERNEL_VERSION(3,10,40) || LINUX_VERSION_CODE == KERNEL_VERSION(3,18,30)
    if(bFlag)
        pFile->f_op->llseek(pFile, 0, SEEK_SET);

    s32WriteLen = pFile->f_op->write(pFile, ps8Buf, u32Len, &pFile->f_pos);
#elif LINUX_VERSION_CODE == KERNEL_VERSION(4,9,84)
    if(bFlag)
       vfs_llseek(pFile, 0, SEEK_SET);

    s32WriteLen =vfs_write(pFile, ps8Buf, u32Len, &pFile->f_pos);
#else
#error not support this kernel version
#endif
    set_fs(stOld_FS);

    return s32WriteLen;
}


MI_S32 FileLseek(struct file *pFile, MI_S32 s32Offset, MI_S32 s32Whence)
{
    MI_S32 s32Ret;

    loff_t res = vfs_llseek(pFile, s32Offset, s32Whence);
    s32Ret = res;
    if (res != (loff_t)s32Ret)
        s32Ret = -EOVERFLOW;

    return s32Ret;
}


static int SeperateString(char *s, char **left, char **right)
{
    char *p = s;
    /* find '=' */
    while(*p != '\0' && *p++ != '=');

    if (*--p != '=')
        return -1;

    /* seperate left from right vaule by '=' */
    *p = '\0';
    *left = s;
    *right = p + 1;
    return 0;
}

static char *StripString(char *s, char *d)
{
    char *p = d;
    do{
        if (*s == '\n')
            *s = '\0';
        if (*s != ' ')
            *p++ = *s;
    }while(*s++ != '\0');
    return d;
}
MI_S32 MSYS_LOG_IMPL_Snprintf(MI_U8 *pu8Str, MI_U32 u32Len, const MI_U8 *pszFormat, ...)
{
    MI_S32 s32Len = 0;
    va_list stArgs = {0};

    va_start(stArgs, pszFormat);
    s32Len = vsnprintf(pu8Str, u32Len, pszFormat, stArgs);
    va_end(stArgs);

    return s32Len;
}

#ifdef  FUNC_DEBUG_LEVEL

static int SearchMod(char *s)
{
    int i= 0;
    int cnt = (int)E_MI_MODULE_ID_MAX;

    for (i = 0; i < cnt; i++){
        if (!strncasecmp(_gpstLogConfigInfo[i].u8ModName, s, sizeof(_gpstLogConfigInfo[i].u8ModName)))
            return i;
    }
    return -1;
}



static char *strlwr(char *s)
{
    char *str;
    str = s;
    while(*str != '\0')
    {
        if(*str >= 'A' && *str <= 'Z')
        {
            *str += 'a'-'A';
        }
        str++;
    }
    return s;
}

MI_DBG_LEVEL_e GetModuleLevel(MI_U8* szProcName)
{
    MI_DBG_LEVEL_e eLevel = MI_LOG_LEVEL_DEFAULT;
    char path[256];
    struct file *pFile = NULL;
    MI_S8 s8buf = 0;

    snprintf(path, sizeof(path), "/sys/module/%s/parameters/debug_level", strlwr(szProcName));

    pFile = FileOpen(path , 1);
    if(pFile)
    {
        FileRead(pFile , &s8buf , 1);
        FileClose(pFile);
        eLevel = (MI_DBG_LEVEL_e)(s8buf - 48);
    }
    else
    {
        eLevel = MI_LOG_LEVEL_DEFAULT;
    }
    return eLevel;
}

MI_S32 SetModuleLevel(MI_U8* szProcName , int level)
{
    char path[256];
    struct file *pFile = NULL;
    MI_S8 s8buf = 0;

    snprintf(path, sizeof(path), "/sys/module/%s/parameters/debug_level", strlwr(szProcName));
    pFile = FileOpen(path , 1);
    if(pFile)
    {
        s8buf = 48 + level;
        FileWrite(pFile , &s8buf , 1 , 1);
        FileClose(pFile);
        return 0;
    }
    return -1;
}


MI_S32 _MSYS_LOG_IMPL_ConfigInfoInit(void)
{
    int ModuleIdx = 0;
    #if 0
    if(MI_SUCCESS != mi_sys_MMA_Alloc(NULL,LOG_CONFIG_BUF_SIZE,&_gstLogBuffer.phyStartPhyAddr))
    {
       DBG_ERR("mma alloc fail\n");
       return MI_ERR_SYS_FAILED;
    }

    _gstLogBuffer.pStartVirAddr = mi_sys_Vmap(_gstLogBuffer.phyStartPhyAddr,LOG_CONFIG_BUF_SIZE,FALSE);
    if(!_gstLogBuffer.pStartVirAddr)
    {
       DBG_ERR("call mi_sys_Vmap fail\n");
       return MI_ERR_SYS_FAILED;
    }
    #else
    _gstLogBuffer.pStartVirAddr = kmalloc(LOG_CONFIG_BUF_SIZE, GFP_KERNEL);
    if(_gstLogBuffer.pStartVirAddr==NULL)
    {
        DBG_ERR("mma alloc fail\n");
        return MI_ERR_SYS_FAILED;
    }
    #endif

    memset(_gstLogBuffer.pStartVirAddr , 0 , LOG_CONFIG_BUF_SIZE);

    _gpstLogConfigInfo = (MI_SYS_LogConfigInfo_t *)_gstLogBuffer.pStartVirAddr;

    for(ModuleIdx = 0 ; ModuleIdx < (int)E_MI_MODULE_ID_MAX ; ModuleIdx ++)
    {
       _gpstLogConfigInfo[ModuleIdx].eLogLevel = MI_LOG_LEVEL_DEFAULT;
       _gpstLogConfigInfo[ModuleIdx].eLogPrintPos = E_MI_SYS_LOG_OUTPUT_SERIAL;
       MSYS_LOG_IMPL_Snprintf(_gpstLogConfigInfo[ModuleIdx].u8ModName, sizeof(_gpstLogConfigInfo[ModuleIdx].u8ModName), "modulemax");
    }

    return MI_SUCCESS;
}
#endif

MI_S32 _MSYS_LOG_IMPL_LogBufferInit(void)
{
    memset(&_gstLogBufferInfo ,  0 , sizeof(_gstLogBufferInfo));

    _gstLogBufferInfo.u32BufSize = LOG_RINGBUFFER_SIZE;


    init_waitqueue_head(&(_gstLogBufferInfo.wqNoData));
    sema_init(&_gstLogBufferInfo.semWrite, 1);
    #if 0
    if(MI_SUCCESS != mi_sys_MMA_Alloc(NULL,LOG_RINGBUFFER_SIZE,&_gstLogBufferInfo.phyStartAddrPhy))
    {
        DBG_ERR("mma alloc fail\n");
        return MI_ERR_SYS_FAILED;
    }

    _gstLogBufferInfo.pu8StartAddrVir = mi_sys_Vmap(_gstLogBufferInfo.phyStartAddrPhy,256 * DEBUG_MSG_BUF_SIZE ,FALSE);
    if(!_gstLogBufferInfo.pu8StartAddrVir)
    {
        DBG_ERR("call mi_sys_Vmap fail\n");
        return MI_ERR_SYS_FAILED;
    }
    #else
    _gstLogBufferInfo.pu8StartAddrVir = kmalloc(_gstLogBufferInfo.u32BufSize, GFP_KERNEL);
    if(_gstLogBufferInfo.pu8StartAddrVir==NULL)
    {
        DBG_ERR("memory alloc fail\n");
        return MI_ERR_SYS_FAILED;
    }

    #endif
    return MI_SUCCESS;
}

MI_S32 LogUdiskSave(const MI_S8* pFileName, MI_S8* pData, MI_U32 u32DataLen)
{
    MI_S32 s32WriteLen = 0;
    struct file* pFile = NULL;

    pFile = FileOpen(pFileName, 1);
    if(pFile == NULL)
    {
        DBG_ERR("FileOpen %s failure..............\n", pFileName);
        return MI_ERR_SYS_FAILED;
    }

    s32WriteLen = FileWrite(pFile, pData, u32DataLen , FALSE);

    FileClose(pFile);

    return MI_SUCCESS;
}

static void LOGBufferReset(void)
{
    unsigned long flags;

    spin_lock_irqsave(&_gLogFileLock, flags);
    _gstLogBufferInfo.u32ReadAddr = _gstLogBufferInfo.u32WriteAddr;
    _gstLogBufferInfo.u32ResetFlag++;
    spin_unlock_irqrestore(&_gLogFileLock, flags);
}

static MI_SYS_LogOutputPos_e GetLogPrintMode(void)
{
    MI_SYS_LogOutputPos_e ePos = 0;

    if (0 == _gu8LogInit)
    {
        return E_MI_SYS_LOG_OUTPUT_SERIAL;
    }
    if (_gbSetLogFileFlag == TRUE)
    {
       ePos = E_MI_SYS_LOG_OUTPUT_UDISK;
    }
    else
        ePos = E_MI_SYS_LOG_OUTPUT_SERIAL;


    if (_gbLogOutDbg == TRUE)
    {
        ePos = E_MI_SYS_LOG_OUTPUT_DBG;
    }

    return ePos;
}
inline MI_U32 GetTimeMs(void)
{
    struct timeval tv;
    do_gettimeofday(&tv);
    return (((MI_U32)tv.tv_sec)*1000 + ((MI_U32)tv.tv_usec)/1000);
}


MI_S32 MSYS_LOG_IMPL_WriteBuf(MI_U8 *pu8Buf,  MI_U32 u32MsgLen, MI_U32 u32UserOrKer)
{
    MI_U32 u32CopyLen1;
    MI_U32 u32CopyLen2;
    MI_U32 u32NewWriteAddr;

    if (0 == _gstLogBufferInfo.u32BufSize)
    {
        return MI_SUCCESS;
    }
    down(&_gstLogBufferInfo.semWrite);
    if(_gstLogBufferInfo.u32WriteAddr < _gstLogBufferInfo.u32ReadAddr)
    {
        if ((_gstLogBufferInfo.u32ReadAddr - _gstLogBufferInfo.u32WriteAddr)
              < DEBUG_MSG_BUF_SIZE)
        {
            LOGBufferReset();
        }
    }
    else
    {
        if ((_gstLogBufferInfo.u32WriteAddr - _gstLogBufferInfo.u32ReadAddr)
              > (_gstLogBufferInfo.u32BufSize - DEBUG_MSG_BUF_SIZE))
        {
            LOGBufferReset();
        }
    }

    if ((u32MsgLen + _gstLogBufferInfo.u32WriteAddr) >= _gstLogBufferInfo.u32BufSize)
    {
        u32CopyLen1 = _gstLogBufferInfo.u32BufSize - _gstLogBufferInfo.u32WriteAddr;
        u32CopyLen2 = u32MsgLen - u32CopyLen1;
        u32NewWriteAddr = u32CopyLen2;
    }
    else
    {
        u32CopyLen1 = u32MsgLen;
        u32CopyLen2 = 0;
        u32NewWriteAddr = u32MsgLen + _gstLogBufferInfo.u32WriteAddr;
    }

    if(u32CopyLen1 > 0)
    {
        if(MSG_FROM_KERNEL == u32UserOrKer)
        {
            memcpy((_gstLogBufferInfo.pu8StartAddrVir + _gstLogBufferInfo.u32WriteAddr),pu8Buf, u32CopyLen1);
        }
        else
        {
            if(copy_from_user((_gstLogBufferInfo.u32WriteAddr+_gstLogBufferInfo.pu8StartAddrVir),
                    pu8Buf, u32CopyLen1))
            {
                DBG_ERR("copy_from_user error\n");
            }
        }
    }
    if(u32CopyLen2 > 0)
    {
        if(MSG_FROM_KERNEL == u32UserOrKer)
        {
            memcpy(_gstLogBufferInfo.pu8StartAddrVir, (pu8Buf + u32CopyLen1), u32CopyLen2);
        }
        else
        {
            if(copy_from_user(_gstLogBufferInfo.pu8StartAddrVir,
                    (pu8Buf + u32CopyLen1), u32CopyLen2))
            {
                DBG_ERR("copy_from_user error\n");
            }
        }
    }

    _gstLogBufferInfo.u32WriteAddr = u32NewWriteAddr;

    if (_gbSetLogFileFlag != TRUE)
    {
        wake_up_interruptible(&_gstLogBufferInfo.wqNoData);
    }
    up(&_gstLogBufferInfo.semWrite);

    return MI_SUCCESS;
}


MI_S32 MSYS_LOG_IMPL_ReadBuf(MI_U8 *pu8Buf,  MI_U32 u32BufLen, MI_U32 *pu32CopyLen, MI_BOOL bKernelCopy)
{
    MI_SYS_LogBufferInfo_t stCurryMsgInfo;
    MI_U32 u32BufUsedLen;
    MI_U32 u32DataLen1;
    MI_U32 u32DataLen2;
    MI_U32 u32CopyLen;
    MI_U32 u32NewReadAddr;
    unsigned long flags;

    if (0 == _gstLogBufferInfo.u32BufSize)
    {
        DBG_ERR("Log Buffer size is 0, Please confige the Buffer size\n");
        return MI_ERR_SYS_FAILED;
    }

    if(_gstLogBufferInfo.u32WriteAddr == _gstLogBufferInfo.u32ReadAddr)
    {
        if (_gbSetLogFileFlag == TRUE)
        {
            return MI_ERR_SYS_FAILED;
        }
        else
        {
            wait_event_interruptible(_gstLogBufferInfo.wqNoData,
                    (_gstLogBufferInfo.u32WriteAddr != _gstLogBufferInfo.u32ReadAddr));
        }
    }

    spin_lock_irqsave(&_gLogFileLock, flags);
    memcpy(&stCurryMsgInfo,  &_gstLogBufferInfo, sizeof(_gstLogBufferInfo));
    spin_unlock_irqrestore(&_gLogFileLock, flags);

    if(stCurryMsgInfo.u32WriteAddr < stCurryMsgInfo.u32ReadAddr)
    {
        u32BufUsedLen = stCurryMsgInfo.u32BufSize - stCurryMsgInfo.u32ReadAddr
                    + stCurryMsgInfo.u32WriteAddr;
        u32DataLen1 =  stCurryMsgInfo.u32BufSize - stCurryMsgInfo.u32ReadAddr;
        u32DataLen2 = stCurryMsgInfo.u32WriteAddr;
    }
    else
    {
        u32BufUsedLen = stCurryMsgInfo.u32WriteAddr - stCurryMsgInfo.u32ReadAddr;
        u32DataLen1 = u32BufUsedLen;
        u32DataLen2 = 0;
    }

    if (u32BufLen <= (u32DataLen1 + u32DataLen2))
    {
        u32CopyLen = u32BufLen;
    }
    else
    {
        u32CopyLen = u32DataLen1 + u32DataLen2;
    }

    if (u32DataLen1 >= u32CopyLen)
    {
        if (bKernelCopy == FALSE)
        {
            if(copy_to_user(pu8Buf, (stCurryMsgInfo.u32ReadAddr+stCurryMsgInfo.pu8StartAddrVir), u32CopyLen))
            {
                DBG_ERR("copy_to_user error\n");
                return MI_ERR_SYS_FAILED;
            }
        }
        else
        {
            memcpy(pu8Buf, (stCurryMsgInfo.u32ReadAddr+stCurryMsgInfo.pu8StartAddrVir), u32CopyLen);
        }

        u32NewReadAddr = stCurryMsgInfo.u32ReadAddr + u32CopyLen;
    }
    else
    {
        if (bKernelCopy == FALSE)
        {
            if(copy_to_user(pu8Buf, (stCurryMsgInfo.u32ReadAddr+stCurryMsgInfo.pu8StartAddrVir), u32DataLen1))
            {
                DBG_ERR("copy_to_user error\n");
                return MI_ERR_SYS_FAILED;
            }
        }
        else
        {
            memcpy(pu8Buf, (stCurryMsgInfo.u32ReadAddr+stCurryMsgInfo.pu8StartAddrVir), u32DataLen1);
        }

        if (bKernelCopy == FALSE)
        {
            if(copy_to_user((pu8Buf + u32DataLen1), stCurryMsgInfo.pu8StartAddrVir, (u32CopyLen - u32DataLen1)))
            {
                DBG_ERR("copy_to_user error\n");
                return MI_ERR_SYS_FAILED;
            }
        }
        else
        {
            memcpy((pu8Buf + u32DataLen1), stCurryMsgInfo.pu8StartAddrVir, (u32CopyLen - u32DataLen1));
        }

        u32NewReadAddr = u32CopyLen - u32DataLen1;
    }

    *pu32CopyLen = u32CopyLen;

    if (u32NewReadAddr >= stCurryMsgInfo.u32BufSize)
        u32NewReadAddr = 0;

    spin_lock_irqsave(&_gLogFileLock, flags);
    if (stCurryMsgInfo.u32ResetFlag == _gstLogBufferInfo.u32ResetFlag)
    {
        _gstLogBufferInfo.u32ReadAddr = u32NewReadAddr;
    }
    spin_unlock_irqrestore(&_gLogFileLock, flags);

    return MI_SUCCESS;
}
#if 0
static MI_S32 MSYS_LOG_IMPL_PrintLogToBuf(const char *format, ...)
{
    char    log_str[LOG_MAX_TRACE_LEN] = {0};
    MI_U32  MsgLen = 0;
    va_list args;

    if (0 == _gu8LogInit)
    {
        DBG_ERR("log device not init!\n");
        return MI_ERR_SYS_FAILED;
    }

    va_start(args, format);
    MsgLen = vsnprintf(log_str, LOG_MAX_TRACE_LEN-1, format, args);
    va_end(args);

    if (MsgLen >= (LOG_MAX_TRACE_LEN-1))
    {
        log_str[LOG_MAX_TRACE_LEN-1] = '\0';  /* even the 'vsnprintf' commond will do it */
        log_str[LOG_MAX_TRACE_LEN-2] = '\n';
        log_str[LOG_MAX_TRACE_LEN-3] = '.';
        log_str[LOG_MAX_TRACE_LEN-4] = '.';
        log_str[LOG_MAX_TRACE_LEN-5] = '.';
    }

    return MSYS_LOG_IMPL_WriteBuf((MI_U8 *)log_str, MsgLen, MSG_FROM_KERNEL);
}
#endif
void msys_print(const char *format, ...)
{
    va_list args;
    MI_U32  u32MsgLen = 0;
    char    log_str[LOG_MAX_TRACE_LEN]={'a'};

    {
        log_str[LOG_MAX_TRACE_LEN-1] = 'b';
        log_str[LOG_MAX_TRACE_LEN-2] = 'c';

        va_start(args, format);
        u32MsgLen = vsnprintf(log_str, LOG_MAX_TRACE_LEN, format, args);
        va_end(args);

        if (u32MsgLen >= LOG_MAX_TRACE_LEN)
        {
            log_str[LOG_MAX_TRACE_LEN-1] = '\0';  /* even the 'vsnprintf' commond will do it */
            log_str[LOG_MAX_TRACE_LEN-2] = '\n';
            log_str[LOG_MAX_TRACE_LEN-3] = '.';
            log_str[LOG_MAX_TRACE_LEN-4] = '.';
            log_str[LOG_MAX_TRACE_LEN-5] = '.';
        }
        /* log module has Initialized. */
        if (_gu8LogInit)
        {
            MI_SYS_LogOutputPos_e ePos = GetLogPrintMode();

            switch(ePos)
            {
                case E_MI_SYS_LOG_OUTPUT_SERIAL:
                    MI_PRINT(log_str);
                    break;
                case E_MI_SYS_LOG_OUTPUT_NETWORK:
                case E_MI_SYS_LOG_OUTPUT_UDISK:
                    MSYS_LOG_IMPL_WriteBuf(log_str, u32MsgLen, MSG_FROM_KERNEL);
                    break;
                case E_MI_SYS_LOG_OUTPUT_DBG:
                    /* No Output */
                    break;
            }
        }
        else /* log module has not Initialized. */
        {
            MI_PRINT(log_str);
        }
    }

}

EXPORT_SYMBOL(msys_print);

static inline int get_log_level(const char *buffer)
{
	if (buffer[0] == KERN_SOH_ASCII && buffer[1]) {
		switch (buffer[1]) {
		case '0' ... '7':
			return buffer[1]-'0';
		}
	}
	return 7;
}


void msys_prints(const char *string, int length)
{
     /* log module has Initialized. */
    if (_gu8LogInit)
    {
        MI_SYS_LogOutputPos_e ePos = GetLogPrintMode();

        switch(ePos)
        {
            case E_MI_SYS_LOG_OUTPUT_SERIAL:
                printk(string);
                break;
            case E_MI_SYS_LOG_OUTPUT_NETWORK:
            case E_MI_SYS_LOG_OUTPUT_UDISK:
                MSYS_LOG_IMPL_WriteBuf((MI_U8 *)string, length, MSG_FROM_KERNEL);
                break;
            case E_MI_SYS_LOG_OUTPUT_DBG:
                if (get_log_level(string)<=_gu8LogLevel)
                    printk(string);
                break;
        }
    }
    else /* log module has not Initialized. */
    {
         printk(string);
    }

}

EXPORT_SYMBOL(msys_prints);

#ifdef  FUNC_DEBUG_LEVEL
MI_S32 MSYS_LOG_IMPL_LogAddModule(const MI_U8* szProcName, MI_ModuleId_e eModuleID)
{
    if (NULL == _gpstLogConfigInfo || eModuleID >= E_MI_MODULE_ID_MAX)
    {
        return MI_ERR_SYS_FAILED;
    }

    MSYS_LOG_IMPL_Snprintf(_gpstLogConfigInfo[eModuleID].u8ModName, sizeof(_gpstLogConfigInfo[eModuleID].u8ModName),"%s", szProcName);

    _gpstLogConfigInfo[eModuleID].eLogLevel = GetModuleLevel(strlwr(_gpstLogConfigInfo[eModuleID].u8ModName));

    return MI_SUCCESS;
}

MI_S32 MSYS_LOG_IMPL_LogRemoveModule(const MI_U8* szProcName, MI_ModuleId_e eModuleID)
{

    if (NULL == _gpstLogConfigInfo || eModuleID >= E_MI_MODULE_ID_MAX)
    {
        return MI_ERR_SYS_FAILED;
    }

    _gpstLogConfigInfo[eModuleID].eLogLevel = E_MI_ERR_LEVEL_ERROR;
    MSYS_LOG_IMPL_Snprintf(_gpstLogConfigInfo[eModuleID].u8ModName, sizeof(_gpstLogConfigInfo[eModuleID].u8ModName), "ModuleMax");

    return MI_SUCCESS;
}
#endif



int MSYS_LOG_IMPL_ProcRead(struct seq_file *s, void *pArg)
{
#ifdef FUNC_DEBUG_LEVEL
    MI_U32 i;
    MI_U8 u8Level;
    MI_U32 u32Total = E_MI_MODULE_ID_MAX;
#endif
    if (0 == _gu8LogInit)
    {
        seq_printf(s,"    Log module not init\n");
        return 0;
    }
    seq_printf(s,"---------------- Log Path ------------------------\n");
    seq_printf(s,"log path:  %s\n", UdiskLogFile);

#ifdef FUNC_STORE_PATH
    seq_printf(s,"---------------- Store Path ----------------------\n");
    seq_printf(s,"store path:  %s\n", StorePath);
#endif
#ifdef FUNC_DEBUG_LEVEL
    seq_printf(s,"---------------- Module Log Level ----------------\n");
    seq_printf(s,"Log module\t  Level\n");
    seq_printf(s,"--------------------------\n");
    for (i = 0; i < u32Total; i++)
    {
        if (strncmp(_gpstLogConfigInfo[i].u8ModName, "modulemax", 10))
        {
            _gpstLogConfigInfo[i].eLogLevel = GetModuleLevel(_gpstLogConfigInfo[i].u8ModName);
            u8Level = _gpstLogConfigInfo[i].eLogLevel;
            seq_printf(s,"%-16s  %d(%s)\n",
                _gpstLogConfigInfo[i].u8ModName, u8Level, DebugLevelName[u8Level]);
        }
    }

    seq_printf(s,"\nhelp example:\n");
    seq_printf(s,"echo mi_sys=2 > /proc/mi_modules/mi_log_info \n");
    seq_printf(s,"echo mi_vdisp=1 > /proc/mi_modules/mi_log_info \n");
#endif
    seq_printf(s,"echo log=/mnt > /proc/msys/msys_log_info \n");
    seq_printf(s,"echo log=/dev/null > /proc/msys/msys_log_info \n");
    seq_printf(s,"echo log=dbg_lv_2 > /proc/msys/msys_log_info \n");
#ifdef FUNC_STORE_PATH
    seq_printf(s,"echo storepath=/mnt > /proc/msys/msys_log_info \n");
#endif

    return 0;
}




ssize_t MSYS_LOG_IMPL_ProcWrite( struct file * file,  const char __user * buf,
                     size_t count, loff_t *ppos)
{
    char m[MAX_FILENAME_LENTH] = {0};
    char d[MAX_FILENAME_LENTH] = {0};
    size_t len = MAX_FILENAME_LENTH;
    char *left, *right;
    int log_level = 7;
//    int idx, level;
    int nRet = 0;
    if (*ppos >= MAX_FILENAME_LENTH)
        return -EFBIG;

    len = min(len, count);
    if(copy_from_user(m, buf, len ))
        return -EFAULT;

    if (0 == _gu8LogInit)
    {
        DBG_ERR("    Log module not init!\n");
        goto out;
    }

    StripString(m, d);

    /* echo help info to current terinmal */
    if (!strncasecmp("help", m, 4))
    {
#ifndef DISABLE_FUNC_DEBUG_LEVEL
        printk("To modify the level, use command line in shell: \n");
        printk("    echo module_name = level_number > /proc/msp/log\n");
        printk("    level_number: 0-fatal, 1-error, 2-warning, 3-info\n");
        printk("    example: 'echo HI_DEMUX=3 > /proc/msp/log'\n");
        printk("    will change log levle of module \"HI_DEMUX\" to 3, then, \n");
        printk("all message with level higher than \"info\" will be printed.\n");
        printk("Use 'echo \"all = x\" > /proc/msp/log' to change all modules.\n");

        printk("\n\nTo modify the log path, use command line in shell: \n");
        printk("Use 'echo \"log = x\" > /proc/msp/log' to set log path.\n");
        printk("Use 'echo \"log = /dev/null\" > /proc/msp/log' to close log udisk output.\n");
        printk("Use 'echo \"log = dbg_lv_#\" > /proc/msys/msys_log_info' to configure log output level. #=0~7\n");
        printk("    example: 'echo log=/home > /proc/msp/log'\n");
        printk("    example: 'echo log=dbg_lv_4 > /proc/msys/msys_log_info'\n");
#endif
        printk("\n\nTo modify the debug file store path, use command line in shell: \n");
        printk("Use 'echo \"storepath = x\" > /proc/msp/log' to set debug file path.\n");
        printk("    example: 'echo storepath=/tmp > /proc/msp/log'\n");
    }

    if (SeperateString(d, &left, &right)){
        DBG_WRN("string is unkown!\n");
        goto out;
    }

    if (!strncasecmp("log", left, 4))
    {
        if (strlen(right) >= sizeof(g_szPathBuf))
        {
            DBG_ERR("    Log path length is over than %d!\n",sizeof(g_szPathBuf));
            goto out;
        }

        nRet = LOG_FILE_LOCK();

        memset(g_szPathBuf, 0, sizeof(g_szPathBuf));
        memcpy(g_szPathBuf, right, strlen(right));

        if ( memcmp(g_szPathBuf, "/dev/null", strlen("/dev/null")) != 0 )
        {
            if ( strncasecmp(g_szPathBuf, "dbg_lv_", strlen("dbg_lv_")) != 0 )
            {
                _MSYS_LOG_IMPL_LogBufferInit();
                if(!IS_ERR(gpLogUdiskTask))
                {
                   wake_up_process(gpLogUdiskTask);
                }
                _gbSetLogFileFlag = TRUE;
                _gbLogOutDbg = FALSE;
            }
            else
            {
                sscanf(g_szPathBuf,"dbg_lv_%d", &log_level);
                _gu8LogLevel = log_level;
                _gbLogOutDbg = TRUE;
            }
        }
        else
        {
            _gbSetLogFileFlag = FALSE;
            _gbLogOutDbg = FALSE;
        }
#ifdef FUNC_DEBUG_LEVEL
        //bug??
        _gpstLogConfigInfo->u8UdiskFlag = (MI_U8)_gbSetLogFileFlag;
#endif
        UdiskLogFile = g_szPathBuf;

        LOG_FILE_UNLOCK();

        DBG_INFO("set log path is g_szPathBuf = %s\n", UdiskLogFile);

        goto out;
    }
#ifdef FUNC_STORE_PATH
    else if (!strncasecmp("storepath", left, strlen("storepath")+1))
    {
        if (strlen(right) >= sizeof(g_szStorePathBuf))
        {
            DBG_ERR("    Store path length is over than %d!\n",sizeof(g_szStorePathBuf));
            goto out;
        }

        nRet = LOG_FILE_LOCK();

        memset(g_szStorePathBuf, 0, sizeof(g_szStorePathBuf));
        memcpy(g_szStorePathBuf, right, strlen(right));

        StorePath = g_szStorePathBuf;

        LOG_FILE_UNLOCK();

        DBG_ERR("set output path is StorePath = %s\n", g_szStorePathBuf);

        goto out;
    }
#endif

#ifdef FUNC_DEBUG_LEVEL
    else
    {
        level = simple_strtol(right, NULL, 10);
        if (!level && *right != '0'){
            DBG_WRN("invalid value!\n");
            goto out;
        }
        if (!strncasecmp("all", left, 4)){
            int i = 0;
            MI_U32 u32Total = E_MI_MODULE_ID_MAX;
            for (i = 0; i < u32Total; i++)
            {
                _gpstLogConfigInfo[i].eLogLevel = (MI_DBG_LEVEL_e)level;
                SetModuleLevel(strlwr(_gpstLogConfigInfo[i].u8ModName) , level);
            }
            goto out;
        }

        idx = SearchMod(left);
        if (-1 == idx){
            DBG_WRN("%s not found in array!\n", left);
            return count;
        }
        _gpstLogConfigInfo[idx].eLogLevel = (MI_DBG_LEVEL_e)level;
        if(!SetModuleLevel(strlwr(_gpstLogConfigInfo[idx].u8ModName) , level))
        {
            goto out;
        }
        return count;
    }
#endif

out:
    *ppos = len;
    return len;
}


int LogUdiskWriteThread(void* pArg)
{
    MI_U8 szBuf[700] = {0};
    MI_U32 u32ReadLen = 0;
    MI_U32 s32Ret = 0;
    MI_U8 szFileName[MAX_FILENAME_LENTH] = {0};
    MI_BOOL bSetFileFlag = FALSE;

    while (1)
    {
        s32Ret = LOG_FILE_LOCK();

        bSetFileFlag = _gbSetLogFileFlag;

        MSYS_LOG_IMPL_Snprintf(szFileName, sizeof(szFileName)-1, "%s/msys.log", (const MI_S8*)UdiskLogFile);

        LOG_FILE_UNLOCK();

        set_current_state(TASK_INTERRUPTIBLE);

        if(kthread_should_stop())
        {
            break;
        }

        if ( bSetFileFlag == FALSE)
        {
            msleep(10);
            continue;
        }

        memset(szBuf, 0, sizeof(szBuf));

        s32Ret = MSYS_LOG_IMPL_ReadBuf(szBuf, sizeof(szBuf)-1, &u32ReadLen, TRUE);
        if (s32Ret == MI_SUCCESS)
        {
            LogUdiskSave((const MI_S8*)szFileName, szBuf, u32ReadLen);
        }

        msleep(100);
    }

    return 0;
}

MI_S32 _MSYS_LOG_IMPL_LogUdiskInit(const MI_U8* pUdiskFolder)
{
    int err;

    if (pUdiskFolder == NULL )
    {
        return MI_ERR_SYS_NOT_PERM;
    }

    if (gpLogUdiskTask == NULL)
    {
        gpLogUdiskTask = kthread_create(LogUdiskWriteThread, (void*)pUdiskFolder, "msysLodUdiskTask");
        if(IS_ERR(gpLogUdiskTask))
        {
            DBG_ERR("create log Udisk write thread failed\n");

            err = PTR_ERR(gpLogUdiskTask);
            gpLogUdiskTask = NULL;

            return err;
        }

        wake_up_process(gpLogUdiskTask);
    }

    return MI_SUCCESS;
}


static int log_info_open(struct inode *inode, struct file *file)
{
    single_open(file,MSYS_LOG_IMPL_ProcRead,PDE_DATA(inode));
    return MI_SUCCESS;
}

static const struct file_operations mi_log_info_fops = {
    .owner   = THIS_MODULE,
    .open = log_info_open,
    .read = seq_read,
    .write = MSYS_LOG_IMPL_ProcWrite,
    .llseek  = seq_lseek,
    .release = single_release,
};

MI_S32 MSYS_LOG_Init(void)
{
#ifdef FUNC_DEBUG_LEVEL
    MI_U32 u32ModuleId = 0;
    MI_U8 u8ModuleName[10] = {0};
#endif
    struct  proc_dir_entry  *entry;

    g_pCMPI_proc = proc_mkdir("msys", NULL);

    entry =  proc_create("msys_log_info", 0666, g_pCMPI_proc, &mi_log_info_fops);
    if (!entry)
    {
        printk(KERN_ERR "failed  to  create  procfs  file  mi_log_info.\n");
        return MI_ERR_SYS_FAILED;
    }

#ifdef FUNC_DEBUG_LEVEL
    _MSYS_LOG_IMPL_ConfigInfoInit();

    while(u32ModuleId < E_MI_MODULE_ID_MAX)
    {
        _MSYS_IMPL_ModuleIdToPrefixName((MI_ModuleId_e)u32ModuleId , u8ModuleName);
        MSYS_LOG_IMPL_LogAddModule(u8ModuleName , (MI_ModuleId_e)u32ModuleId);
        u32ModuleId ++;
    }
#endif
    //_MSYS_LOG_IMPL_LogBufferInit();  //move to proc_wirite to trigger
    _MSYS_LOG_IMPL_LogUdiskInit(UdiskLogFile);

    _gu8LogInit = 1;

    return MI_SUCCESS;
}

MI_S32 _MSYS_LOG_IMPL_LogBufferExit(void)
{
    if(NULL != _gstLogBufferInfo.pu8StartAddrVir)
    {
        //mi_sys_UnVmap(_gstLogBufferInfo.pu8StartAddrVir);
        //mi_sys_MMA_Free(_gstLogBufferInfo.phyStartAddrPhy);
        kfree(_gstLogBufferInfo.pu8StartAddrVir);

        _gstLogBufferInfo.pu8StartAddrVir = NULL;
        _gstLogBufferInfo.phyStartAddrPhy = 0;
    }
    return MI_SUCCESS;
}
#ifdef  FUNC_DEBUG_LEVEL

MI_S32 _MSYS_LOG_IMPL_ConfigInfoExit(void)
{
    if (NULL != _gstLogBuffer.pStartVirAddr)
    {
        //mi_sys_UnVmap(_gstLogBuffer.pStartVirAddr);
        //mi_sys_MMA_Free(_gstLogBuffer.phyStartPhyAddr);
        kfree(_gstLogBuffer.pStartVirAddr);
        _gstLogBuffer.pStartVirAddr = NULL;
        _gstLogBuffer.phyStartPhyAddr = 0;
        //_gpstLogConfigInfo = NULL;
    }
    return MI_SUCCESS;
}
#endif

MI_S32 MSYS_LOG_Exit(void)
{
#ifdef  FUNC_DEBUG_LEVEL

    MI_U32 u32ModuleId = 0;
    MI_U8 u8ModuleName[10] = {0};

    while(u32ModuleId < E_MI_MODULE_ID_MAX)
    {
        _MSYS_IMPL_ModuleIdToPrefixName((MI_ModuleId_e)u32ModuleId , u8ModuleName);
        MSYS_LOG_IMPL_LogRemoveModule(u8ModuleName , (MI_ModuleId_e)u32ModuleId);
        u32ModuleId ++;
    }
    _gu8LogInit = 0;
    _MSYS_LOG_IMPL_ConfigInfoExit();
#endif
    _MSYS_LOG_IMPL_LogBufferExit();

    remove_proc_entry("msys_log_info",g_pCMPI_proc);

    return MI_SUCCESS;
}


subsys_initcall(MSYS_LOG_Init);
