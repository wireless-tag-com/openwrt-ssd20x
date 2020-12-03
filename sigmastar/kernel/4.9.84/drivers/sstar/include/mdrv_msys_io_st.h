/*
* mdrv_msys_io_st.h- Sigmastar
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
#ifndef _MDRV_MSYS_IO_ST_H_
#define _MDRV_MSYS_IO_ST_H_

/*******************************************************/
#define IOCTL_MSYS_VERSION    0x0100
/*******************************************************/

#define PAD_CLOCK_OUT   (GPIO_NR+1)

typedef struct
{
    unsigned int VerChk_Version;
    char name[16];
    unsigned int length;       //32 bit
    unsigned long long phys;   //64 bit
    unsigned long long kvirt; //Kernel Virtual Address 64 bit
    unsigned int option; //reserved
    unsigned int VerChk_Size;
} __attribute__ ((__packed__)) MSYS_DMEM_INFO;

typedef struct
{
    unsigned int VerChk_Version;
    unsigned int VerChk_Size;
} __attribute__ ((__packed__)) MSYS_DUMMY_INFO;
typedef struct
{
    unsigned int IRQNumber; //reserved
    void* action;
    unsigned long long timeStart;
    unsigned long long timeEnd;
} __attribute__ ((__packed__)) MSYS_IRQ_INFO;
typedef struct
{
    unsigned int VerChk_Version;
    unsigned long long addr;
    unsigned int VerChk_Size;
} __attribute__ ((__packed__)) MSYS_ADDR_TRANSLATION_INFO;

typedef struct
{
    unsigned int VerChk_Version;
    unsigned long long addr;
    unsigned int size;
    unsigned int VerChk_Size;
} __attribute__ ((__packed__)) MSYS_MMIO_INFO;

typedef struct
{
    unsigned int VerChk_Version;
    unsigned long long phys;   //64 bit
    unsigned int length;       //32 bit
    unsigned char id[16];
    unsigned int r_protect;
    unsigned int w_protect;
    unsigned int inv_protect;
    unsigned int VerChk_Size;
} __attribute__ ((__packed__)) MSYS_MIU_PROTECT_INFO;

typedef struct
{
    unsigned int VerChk_Version;
    unsigned char str[32];
    unsigned int VerChk_Size;
} __attribute__ ((__packed__)) MSYS_STRING_INFO;

typedef struct
{
    unsigned int VerChk_Version;
    unsigned int temp;
    unsigned int VerChk_Size;
} __attribute__ ((__packed__)) MSYS_TEMP_INFO;

typedef struct
{
    unsigned int VerChk_Version;
    unsigned long long udid;
    unsigned int VerChk_Size;
} __attribute__ ((__packed__)) MSYS_UDID_INFO;

typedef struct
{
    unsigned int VerChk_Version;
    unsigned int chipVersion;
    unsigned int VerChk_Size;
} __attribute__ ((__packed__)) MSYS_CHIPVER_INFO;

typedef struct
{
    unsigned int VerChk_Version;
    char name[16];
    unsigned int length;       //32 bit
    unsigned long long kphy_src;   //Kernel Virtual Address src 64 bit
    unsigned long long kphy_des; //Kernel Virtual Address des64 bit
    unsigned int VerChk_Size;
} __attribute__ ((__packed__)) MSYS_DMA_INFO;

typedef enum
{
    FREQ_6MHZ   = 0,
    FREQ_12MHZ  = 1,
    FREQ_24MHZ  = 2,
    FREQ_27MHZ  = 3,
    FREQ_36MHZ  = 4,
    FREQ_37_5MHZ = 5,
    FREQ_54MHZ   = 6,
    FREQ_75MHZ   = 7,
}MSYS_FREQ_TYPE;

typedef enum
{
    U01      = 0,
    U02      = 1,
} CHIP_VERSION;

typedef struct
{
    unsigned int padnum;
    unsigned int freq;
    unsigned char bEnable;
    unsigned char bInvert;
} __attribute__ ((__packed__)) MSYS_FREQGEN_INFO;

typedef struct
{
    char name[128];
    unsigned long size;
    void *handle; //don't set this
    void *parent;
    void *data;
} MSYS_PROC_DEVICE;

typedef struct
{
    void *handle;
    char name[128];
    unsigned int type;
    unsigned int offset;
} MSYS_PROC_ATTRIBUTE;

typedef enum
{
    MSYS_PROC_ATTR_CHAR,
    MSYS_PROC_ATTR_UINT,
    MSYS_PROC_ATTR_INT,   //also for enum, bool
    MSYS_PROC_ATTR_XINT,
    MSYS_PROC_ATTR_ULONG,
    MSYS_PROC_ATTR_LONG,
    MSYS_PROC_ATTR_XLONG,
    MSYS_PROC_ATTR_ULLONG,
    MSYS_PROC_ATTR_LLONG,
    MSYS_PROC_ATTR_XLLONG,
    MSYS_PROC_ATTR_STRING,
} MSYS_PROC_ATTR_ENUM;

typedef struct
{
    unsigned long long phyaddr_src; // MIU address of source
    unsigned long long phyaddr_dst; // MIU address of destination
    unsigned int lineofst_src;      // line-offset of source, set 0 to disable line offset
    unsigned int lineofst_dst;      // line-offset of destination, set 0 to disable line offset
    unsigned int width_src;         // width of source, set 0 to disable line offset
    unsigned int width_dst;         // width of destination, set 0 to disable line offset
    unsigned int length;            // total size (bytes)
} MSYS_DMA_BLIT;

#if defined(CONFIG_MS_BDMA_LINE_OFFSET_ON)
typedef struct
{
    unsigned long long phyaddr;     // MIU address of source
    unsigned int length;            // total size (bytes)
    //unsigned int lineofst_src;      // line-offset of source, set 0 to disable line offset
    unsigned int lineofst_dst;      // line-offset of destination, set 0 to disable line offset
    //unsigned int width_src;         // width of source, set 0 to disable line offset
    unsigned int width_dst;         // width of destination, set 0 to disable line offset
    unsigned int pattern;           // pattern (4-byte)
} MSYS_DMA_FILL_BILT;
#endif

typedef struct
{
    unsigned long long phyaddr;     // MIU address of source
    unsigned int length;            // total size (bytes)
    unsigned int pattern;           // pattern (4-byte)
} MSYS_DMA_FILL;

typedef struct
{
    unsigned long long phyaddr_src; // MIU address of source
    unsigned long long phyaddr_dst; // MIU address of destination
    unsigned int length;            // total size (bytes)
}MSYS_DMA_COPY;
#endif
