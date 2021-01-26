/*
* MsDebug.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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

#ifndef _MSTAR_DEBUG_H_
#define _MSTAR_DEBUG_H_

#include "command.h"
#include "MsTypes.h"

typedef enum
{
    EN_DEBUG_LEVEL_DISABLE=0,    
    EN_DEBUG_LEVEL_ERROR=0x01,
    EN_DEBUG_LEVEL_INFO=0x02,
    EN_DEBUG_LEVEL_TRACE=0x04,
    EN_DEBUG_LEVEL_DEBUG=0x08,
    EN_DEBUG_LEVEL_BOOTTIME=0x10,
    EN_DEBUG_LEVEL_INVALID=0x1000,    
    EN_DEBUG_LEVEL_MAX=EN_DEBUG_LEVEL_INVALID      
}EN_DEBUG_LEVEL;

typedef enum
{
    EN_DEBUG_MODULE_DISABLE=0,
    EN_DEBUG_MODULE_UPGRADE=(0x01<<0),    
    EN_DEBUG_MODULE_FAT=(0x01<<1),
    EN_DEBUG_MODULE_ALL=0xFFFFFFFF
}EN_DEBUG_MODULE;

extern EN_DEBUG_LEVEL dbgLevel;
extern EN_DEBUG_MODULE dbgModule;

#define DEFAULT_DEBUG_LEVEL (EN_DEBUG_LEVEL_INFO+EN_DEBUG_LEVEL_ERROR)
#define DEFAULT_DEBUG_MODULE (EN_DEBUG_MODULE_ALL)

#ifdef CONFIG_MINIUBOOT
#define UBOOT_ERRDUMP(msg...) 
#define UBOOT_ERROR(msg...)  \
    do{\
        printf("\033[0;31m[ERROR] %s:%d: \033[0m",__FUNCTION__,__LINE__);\
        printf(msg);\
    }while(0)
#define UBOOT_INFO(msg...)  printf(msg)
#define UBOOT_TRACE(msg...) 
#define UBOOT_DEBUG(msg...) 
#define UBOOT_DUMP(addr, size) 
#else
#define UBOOT_ERROR(msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_ERROR) \
        { \
            printf("\033[0;31m[ERROR] %s:%d: \033[0m",__FUNCTION__,__LINE__);\
            printf(msg);\
        } \
    }while(0)
    
#define UBOOT_INFO(msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_INFO) \
        { \
                printf(msg);\
        } \
    }while(0)

#define UBOOT_BOOTTIME(msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_BOOTTIME) \
        { \
                printf(msg);\
        } \
    }while(0)



            
#if 1
#define UBOOT_TRACE(msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_TRACE) \
        { \
                printf("\033[0;32m[TRACE] %s \033[0m",__FUNCTION__);\
                printf(msg);\
        } \
    }while(0)
    
#else
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_TRACE) \
        { \
            if(dbgModule&module) \
            {               \
                printf("\033[0;32m[TRACE]%s\033[0m",__FUNCTION__);\
                printf(msg);\
            }\
        } \
    }while(0)
    
#define UBOOT_TRACE(msg...)   _UBOOT_TRACE(EN_DEBUG_MODULE_ALL,msg)  

#endif

#if 1
#define UBOOT_DEBUG(msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_DEBUG) \
        { \
                printf("\033[0;34m[DEBUG] %s:%d: \033[0m",__FUNCTION__,__LINE__);\
                printf(msg);\
        } \
    }while(0)

#else
#define _UBOOT_DEBUG(module, msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_DEBUG) \
        { \
            if(dbgModule&module) \
            {               \
                printf("\033[0;34m[DEBUG]%s:%d:\033[0m",__FUNCTION__,__LINE__);\
                printf(msg);\
            } \
        } \
    }while(0)

#define UBOOT_DEBUG(msg...)   _UBOOT_DEBUG(EN_DEBUG_MODULE_ALL,msg)      
#endif

#define UBOOT_DUMP(addr, size)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_DEBUG) \
        { \
                printf("\033[0;34m[DUMP] %s:%d:\033[0m\n",__FUNCTION__,__LINE__);\
                _dump(addr,size);\
        } \
    }while(0)
            
#define UBOOT_ERRDUMP(addr, size)\
            do{\
                if(dbgLevel&EN_DEBUG_LEVEL_ERROR) \
                { \
                        printf("\033[0;31m[DUMP] %s:%d:\033[0m\n",__FUNCTION__,__LINE__);\
                        _dump(addr,size);\
                } \
            }while(0)


int do_set_debugging_message_level(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_initDebugLevel(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);        
void _dump(unsigned int addr, unsigned int size);

#endif
#endif
