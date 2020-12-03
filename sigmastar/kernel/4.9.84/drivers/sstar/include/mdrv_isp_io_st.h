/*
* mdrv_isp_io_st.h- Sigmastar
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
#ifndef _MDRV_ISP_IO_ST_H
#define _MDRV_ISP_IO_ST_H

#define CLK_ID_ISP 0
#define CLK_ID_SR 1
#define CLK_ID_SR_MCLK 2
#define CLK_ID_CSI_MAC 3
typedef struct
{
    unsigned int id;
    unsigned int  rate;
    unsigned int enable;
}isp_ioctl_clock_ctl;

//// for ioctl IOCTL_ISP_IQ_YUVCCM, IOCTL_ISP_IQ_RGBCCM ////
typedef struct
{
  s16 ccm[9];
} __attribute__((packed, aligned(1))) isp_ioctl_ccm_coeff;


//// for ioctl ioctl IOCTL_ISP_GET_ISP_FLAG ////
//same IspBufferFlag_e
#define ISP_FLAG_ENABLE_ROT 0x01
#define ISP_FLAG_ENABLE_DNR 0x02
typedef struct
{
  u32 flag;
} __attribute__((packed, aligned(1))) isp_ioctl_isp_flag;

//// for ioctl ioctl IOCTL_ISP_GET_MEM_INFO ////
#define ISP_GET_MEM_INFO_BASE 0x00
#define ISP_GET_MEM_INFO_AE   0x01
#define ISP_GET_MEM_INFO_AWB  0x02
#define ISP_GET_MEM_INFO_AF   0x03
#define ISP_GET_MEM_INFO_MOT  0x04
#define ISP_GET_MEM_INFO_HISTO  0x05
#define ISP_GET_MEM_INFO_RGBIR  0x06

typedef struct
{
  u32 mem_id;
  u32 phy_addr;
  u32 blk_offset;
  u32 blk_size;
} __attribute__((packed, aligned(1))) isp_ioctl_mem_info;

typedef struct
{
  u32 fcount; //frame count
}__attribute__((packed, aligned(1))) isp_isr_event_data;

typedef struct
{
    u32  frame_cnt;
    u32  hw_frame_cnt;
}__attribute__((packed, aligned(1))) ve_isr_data; //vsync end isr data

//// for IOCTL_ISP_GET_AE_IMG_INFO ////
typedef struct
{
    u32 img_w;
    u32 img_h;
    u32 blk_w;
    u32 blk_h;
    u32 rot;
}__attribute__((packed, aligned(1))) isp_isr_ae_img_info;
u32 isp_get_ae_img_info(isp_isr_ae_img_info *info); //kernel api only

//// for IOCTL_ISP_SET_AE_DGAIN ////
typedef struct
{
  u32 enable;
  u32 dgain; //1X = 1024
} __attribute__((packed, aligned(1))) isp_ioctl_ae_dgian;

//// for IOCTL_ISP_SET_FIFO_MASK////
typedef struct
{
  u32 enable;
} __attribute__((packed, aligned(1))) isp_ioctl_fifo_mask;

//// for IOCTL_ISP_TRIGGER_WDMA////


typedef enum
{
    ISP_WDMA_ICP,
    ISP_WDMA_SIF,
    ISP_WDMA_ISP,
    ISP_WDMA_ISPDS,
    ISP_WDMA_ISPSWAPYC,
    ISP_WDMA_VDOS,
    ISP_WDMA_DEFAULT_SIF,
    ISP_WDMA_DEFAULT_YC
}WDMA_PATH;

typedef struct
{
  u32 width;
  u32 height;
  u32 x;
  u32 y;
  WDMA_PATH wdma_path;
  u32  buf_addr_phy;
  u32  buf_addr_kvir;
} __attribute__((packed, aligned(1))) isp_ioctl_trigger_wdma_attr;



typedef struct
{
  u32  skip_cnt;
} __attribute__((packed, aligned(1))) isp_ioctl_trigger_skip_attr;


typedef enum
{
    ISP_COLOR_ID_R,
    ISP_COLOR_ID_GR,
    ISP_COLOR_ID_GB,
    ISP_COLOR_ID_B,
}IspColorID_e;

//frame buffer control bit
typedef enum
{
    ISP_FB_ROT = 0x01, //enable ROR frame buffer
    ISP_FB_DNR = 0x02, //enable BDNR frame buffer
}IspFrameBufferFlag_e;

typedef struct
{
    u32 nSize; //size of this struct in byte
    u32 nRawX;
    u32 nRawY;
    u32 nRawW;
    u32 nRawH;
    u32 nDnrFlag;
    //u32 nColorID;
    //u32 nPixelDepth;
} __attribute__((packed, aligned(1))) isp_ioctl_isp_init;

typedef enum
{
    WDMA_SRC_BAYER_RAW = 0,
    WDMA_SRC_ISP_OUTPUT_0 = 1,
    WDMA_SRC_ISP_OUTPUT_1 = 2,
    WDMA_SRC_ISP_VIDEO_STABILIZATION=3,
}WDMA_SRC;

typedef enum
{
    WDMA_REQ_WAIT = 0,
    WDMA_REQ_PROC = 1,
    WDMA_REQ_DONE = 2,
}WDMA_REQ_STATUS;

typedef struct
{
    u32 uSourceSel;
    u32 uBufferPhysAddr;
    WDMA_SRC uReqStatus; //WDMA_SRC
}__attribute__((packed, aligned(1))) WDMA_CFG;


typedef struct
{
  u32 bActive;    //Active data state
  u32 u4FrameCnt; //frame count
  u32 bIspInputEnable;
  u32 u4OBCInValid;
  u32 u4OBC_a;
  u32 u4OBC_b;
  u32 uIspStatusSet;           // isp status set from user space
  u32 uIspStatusReport;     //isp satus get from kernel space
  WDMA_CFG wdma_cfg;
}__attribute__((packed, aligned(1))) FRAME_STATE;


typedef enum
{
    FRAME_START_EVENT   = 0x00000001,
    FRAME_END_EVENT     = 0x00000002,
    AE_DONE_EVENT       = 0x00000004,
    AWB_DONE_EVENT      = 0x00000008,
    AF_DONE_EVENT       = 0x00000010,
    WDMA_DONE_EVENT     = 0x00000020,
    FIFO_FULL_EVENT     = 0x00000040,
    IDLE_EVENT          = 0x00000080,
    RDMA_DONE_EVENT     = 0x00000100,
    SCL_VEND_EVENT      = 0x00000200,
}INTERRUPT_EVENT_e;

typedef struct
{
  u32 u32IspInt1;
  u32 u32IspInt2;
  u32 u32IspInt3;
}__attribute__((packed, aligned(1))) ISP_INTERRUPTS_STATE;


typedef struct
{
  FRAME_STATE frame_state;
  ISP_INTERRUPTS_STATE isp_ints_state;
}__attribute__((packed, aligned(1))) ISP_SHARE_DATA;
#endif
