/*
* mdrv_scldma_io_st.h- Sigmastar
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

 /**
 * \ingroup scldma_group
 * @{
 */

#ifndef _MDRV_SCLDMA_IO_ST_H
#define _MDRV_SCLDMA_IO_ST_H

//=============================================================================
// Defines
//=============================================================================
// library information
//1.0.1:for clean scldma.
//1.1.1:for add buffer queue handler and swring mode.
//1.1.3:refine for 1.1.1 scl test OK,stabilize not yet.
#define IOCTL_SCLDMA_VERSION             0x0113 ///< H:Major L:minor H3:Many Change H2:adjust Struct L1:add struct L0:adjust driver
#define IOCTL_SCLDMA_BUFFER_QUEUE_OFFSET   sizeof(ST_IOCTL_SCLDMA_FRAME_BUFFER_CONFIG)///<Buffer Queue Size For OMX

//=============================================================================
// enum
//=============================================================================
/**
* Used to setup the color format of scldma device
*/
typedef enum
{
    E_IOCTL_SCLDMA_COLOR_YUV422, ///< color format: YUV422
    E_IOCTL_SCLDMA_COLOR_YUV420, ///< color format: YUV420
    E_IOCTL_SCLDMA_COLOR_NUM,    ///< The max number of color format
}EN_IOCTL_SCLDMA_COLOR_TYPE;

/**
* Used to setup the memory type of scldma device
*/
typedef enum
{
     E_IOCTL_SCLDMA_MEM_FRM =0,  ///< memory type: FRM
     E_IOCTL_SCLDMA_MEM_SNP =1,  ///< memory type: SNP
     E_IOCTL_SCLDMA_MEM_IMI =2,  ///< memory type: IMI
     E_IOCTL_SCLDMA_MEM_NUM =3,  ///< The max number of memory type
}EN_IOCTL_SCLDMA_MEM_TYPE;

/**
* Used to setup the buffer mode of scldma device
*/
typedef enum
{
    E_IOCTL_SCLDMA_BUFFER_MD_RING,   ///< buffer mode: RING
    E_IOCTL_SCLDMA_BUFFER_MD_SINGLE, ///< bufer mode : single
    E_IOCTL_SCLDMA_BUFFER_MD_SWRING, ///< bufer mode : sw control ring mode
    E_IOCTL_SCLDMA_BUFFER_MD_NUM,    ///< The max number of buffer mode
}EN_IOCTL_SCLDMA_BUFFER_MODE_TYPE;

/**
* Used to setup the flag connect with OMX of scldma device
*/
typedef enum
{
    E_IOCTL_SCLDMA_ACTIVE_BUFFER_OMX      = 0x10,  ///< active buffer type: OMX,if OMX need to update Rp
    E_IOCTL_SCLDMA_ACTIVE_BUFFER_OFF      = 0x20,  ///< acitve buffer type: OFF,if hw dma trig off
    E_IOCTL_SCLDMA_ACTIVE_BUFFER_RINGFULL = 0x40,  ///< active buffer type: RINGFULL,if without update Rp
}EN_IOCTL_SCLDMA_ACTIVE_BUFFER_TYPE;

/**
* Used to setup the flag connect with OMX of scldma device
*/
typedef enum
{
    EN_IOCTL_SCLDMA_BUFFER_QUEUE_TYPE_PEEKQUEUE,    ///< buffer queue type: user only to get information
    EN_IOCTL_SCLDMA_BUFFER_QUEUE_TYPE_DEQUEUE,      ///< buffer queue type: user can set Read information
    EN_IOCTL_SCLDMA_BUFFER_QUEUE_TYPE_ENQUEUE,      ///< TODO : buffer queue type: user can set Write information(not use)
    EN_IOCTL_SCLDMA_BUFFER_QUEUE_TYPE_NUM,          ///< buffer type: totally
}EN_IOCTL_SCLDMA_USED_BUFFER_QUEUE_TYPE;


//=============================================================================
// struct
//=============================================================================

/**
* Used to get SCLDMA drvier version
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    unsigned int   u32Version;      ///< version
    unsigned int   VerChk_Size;     ///< VerChk Size
}__attribute__ ((__packed__)) ST_IOCTL_SCLDMA_VERSION_CONFIG;


//=============================================================================
/**
* Used to setup the buffer flag of scldma device
*/
typedef struct
{
    unsigned char  btsBase_0   : 1;  ///< base0 flag
    unsigned char  btsBase_1   : 1;  ///< base1 flag
    unsigned char  btsBase_2   : 1;  ///< base2 flag
    unsigned char  btsBase_3   : 1;  ///< Base3 flag
    unsigned char  btsReserved : 4;  ///< reserved
}ST_IOCTL_SCLDMA_BUFFER_FLAG_BIT;


// struct for IOCTL_SCLDMA_SET_BUFFER_CONFIG
/**
* Used to setup the buffer configuration of scldma device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    union
    {
        unsigned char  u8Flag;
        ST_IOCTL_SCLDMA_BUFFER_FLAG_BIT bvFlag;   ///< buffer flag
    };

    EN_IOCTL_SCLDMA_MEM_TYPE   enMemType;         ///< memory type
    EN_IOCTL_SCLDMA_COLOR_TYPE enColorType;       ///< color type
    EN_IOCTL_SCLDMA_BUFFER_MODE_TYPE enBufMDType; ///< buffer mode

    unsigned short      u16BufNum;    ///< number of buffer
    unsigned long       u32Base_Y[4]; ///< base address of Y
    unsigned long       u32Base_C[4]; ///< base address of CbCr
    unsigned short      u16Width;     ///< width of buffer
    unsigned short      u16Height;    ///< height of buffer
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
}__attribute__ ((__packed__)) ST_IOCTL_SCLDMA_BUFFER_CONFIG;

// struct for IOCTL_SCLDMA_SET_TRIGGER_CONFIG
/**
* Used to setup the trigger configuration of scldma device
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    unsigned char       bEn;              ///< enable/disable trigger
    EN_IOCTL_SCLDMA_MEM_TYPE   enMemType; ///< memory type
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
}__attribute__ ((__packed__)) ST_IOCTL_SCLDMA_TRIGGER_CONFIG;

// struct for IOCTL_SCLDMA_GET_IN_ACTIVE_BUFFER_CONFIG / IOCTL_SCLDMA_GET_OUT_ACTIVE_BUFFER_CONFIG
/**
* Used to setup the active buffer configuration of scldma device
*/

typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    EN_IOCTL_SCLDMA_MEM_TYPE   enMemType; ///< memory type
    unsigned char   u8ActiveBuffer;       ///< ID of ActiveBuffer
    unsigned char   u8ISPcount;           ///< ISP counter
    unsigned long   u32FRMDoneTime;       ///< Time of FRMDone
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
}__attribute__ ((__packed__))ST_IOCTL_SCLDMA_ACTIVE_BUFFER_CONFIG;

typedef struct
{
    unsigned char   u8FrameAddrIdx;       ///< ID of Frame address
    unsigned long   u32FrameAddr;         ///< Frame Address
    unsigned char   u8ISPcount;           ///< ISP counter
    unsigned short   u16FrameWidth;         ///< Frame Width
    unsigned short   u16FrameHeight;        ///< Frame Height
    unsigned long long   u64FRMDoneTime;  ///< Time of FRMDone
}__attribute__ ((__packed__))ST_IOCTL_SCLDMA_FRAME_BUFFER_CONFIG;

typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    EN_IOCTL_SCLDMA_MEM_TYPE   enMemType; ///< memory type
    EN_IOCTL_SCLDMA_USED_BUFFER_QUEUE_TYPE  enUsedType;
    ST_IOCTL_SCLDMA_FRAME_BUFFER_CONFIG stRead;
    unsigned char  u8InQueueCount;
    unsigned char  u8EnqueueIdx;
    unsigned int   VerChk_Size; ///< VerChk Size
}__attribute__ ((__packed__))ST_IOCTL_SCLDMA_BUFFER_QUEUE_CONFIG;

// structu for IOCTL_SCLDMA_GET_PRIVATE_ID_CONFIG
/**
* Used to setup the private id configuration of scldma device for mutiinst
*/
typedef struct
{
    signed long s32Id;  ///< private ID
}ST_IOCTL_SCLDMA_PRIVATE_ID_CONFIG;

// struct for IOCTL_SCLDMA_SET_LOCK_CONFIG/ IOCTL_SCLDMA_SET_UNLOCK_CONFIG
/**
* Used to setup the lock configuration of scldma device for mutiinst
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    signed long *ps32IdBuf;  ///< buffer ID
    unsigned char u8BufSize; ///< buffer size
    // VerChk_Version & VerChk_Size must be the latest 2 parameter and
    // the order can't be changed
    unsigned int   VerChk_Size; ///< VerChk Size
}__attribute__ ((__packed__)) ST_IOCTL_SCLDMA_LOCK_CONFIG;

/**
* Used to setup the lock configuration of scldma device for mutiinst
*/
typedef struct
{
    unsigned int   VerChk_Version ; ///< VerChk version
    EN_IOCTL_SCLDMA_MEM_TYPE   enMemType; ///< memory type
    unsigned short u16DMAH;///< DMAH
    unsigned short u16DMAV;///< DMAV
    EN_IOCTL_SCLDMA_COLOR_TYPE enColorType;       ///< color type
    EN_IOCTL_SCLDMA_BUFFER_MODE_TYPE enBufMDType; ///< buffer mode
    unsigned short      u16BufNum;    ///< number of buffer
    unsigned long       u32Base_Y[4]; ///< base address of Y
    unsigned long       u32Base_C[4]; ///< base address of CbCr
    unsigned int   VerChk_Size; ///< VerChk Size
}__attribute__ ((__packed__)) ST_IOCTL_SCLDMA_GET_INFORMATION_CONFIG;


//=============================================================================

//=============================================================================



#endif //

/** @} */ // end of scldma_group
