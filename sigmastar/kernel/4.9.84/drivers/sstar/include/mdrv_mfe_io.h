/*
* mdrv_mfe_io.h- Sigmastar
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

#ifndef _MDRV_MMFE_IO_H_
#define _MDRV_MMFE_IO_H_

/* definition of structures */

#define MEMORY_USER     0
#define MEMORY_MMAP     1

#define PIXFMT_TILED    0
#define PIXFMT_PLANE    1

typedef struct mmfe_reqbuf {
    int i_vq;
    int i_memory;
    int i_pic_w, i_pic_h;
    int i_count;
} mmfe_reqbuf;

typedef struct mmfe_buffer {
    int i_vq;
    int i_vb;
    int i_memory;
    int i_planes;
    struct {
        union {
        unsigned int addr;
        void* uptr;
        };
        int size;
        int used;
    } planes[4];
    long long timecode;
} mmfe_buffer;

#define MVIDPARAMS_PIXELS       0
#define MVIDPARAMS_FRAMERATE    1
#define MVIDPARAMS_MOTION       2
#define MVIDPARAMS_BITRATE      3
#define MVIDPARAMS_GOP          4
#define MVIDPARAMS_AVC          5

#define RC_METHOD_CBR           0
#define RC_METHOD_VBR           1
#define RC_METHOD_CONST         2

#define MV_BLOCK_4x4        (1<<0)
#define MV_BLOCK_8x4        (1<<1)
#define MV_BLOCK_4x8        (1<<2)
#define MV_BLOCK_8x8        (1<<3)
#define MV_BLOCK_16x8       (1<<4)
#define MV_BLOCK_8x16       (1<<5)
#define MV_BLOCK_16x16      (1<<6)
#define MV_BLOCK_SKIP       (1<<7)

typedef struct mmfe_params {
    int i_params;
    union {
        unsigned char bytes[60];
        struct {
        int i_pict_w;                   /* picture width    */
        int i_pict_h;                   /* picture height   */
        int i_pixfmt;                   /* pixel format     */
        } pixels;
        struct {
        int i_num;                      /* numerator of frame-rate      */
        int i_den;                      /* denominator of frame-rate    */
        } framerate;
        struct {
        int i_dmv_x;                    /* dMV x-direction (8~16)               */
        int i_dmv_y;                    /* dMV y-direction (8/16)               */ 
        int i_subpel;                   /* subpel: 0-integral,1-half,2-quarter  */
        unsigned int i_mv_block[2];     /* motion block flags                   */
        } motion;
        struct {
        int i_pframes;                  /* p-frames count per i-frame   */
        int i_bframes;                  /* b-frames count per i/p-frame */
        } gop;
        struct {
        int i_method;                   /* constant bit-rate/variable bit-rate/constant quality */
        int i_kbps;                     /* bps/1024     */
        int i_qp;                       /* qp-value     */
        } bitrate;
        struct {
        int i_profile;                  /* avc profile value        */
        int i_level;                    /* avc level value          */
        int i_num_ref_frames;           /* reference frame count    */
        int b_cabac;                    /* entropy: cabac/cavlc     */
        int b_constrained_intra_pred;   /* contrained intra pred    */
        int b_deblock_filter_control;   /* deblock filter control   */
        int i_disable_deblocking_idc;   /* disable deblocking idc   */
        int i_alpha_c0_offset;          /* offset alpha div2        */
        int i_beta_offset;              /* offset beta div2         */
        int b_multiple_slices;          /* multi-slices             */
        } avc;
    };
} mmfe_params;

typedef struct mmfe_control {
    int i_control;
    int i_value;
} mmfe_control;

/* definition of ioctl codes */

#define IOCTL_MFE_S_PARM      _IOWR('m', 0,struct mmfe_params)
#define IOCTL_MFE_G_PARM      _IOWR('m', 1,struct mmfe_params)
#define IOCTL_MFE_S_CTRL      _IOWR('m', 2,struct mmfe_control)
#define IOCTL_MFE_G_CTRL      _IOWR('m', 3,struct mmfe_control)
#define IOCTL_MFE_STREAMON      _IO('m', 4)
#define IOCTL_MFE_STREAMOFF     _IO('m', 5)
#define IOCTL_MFE_REQBUF      _IOWR('m', 6,struct mmfe_reqbuf)
#define IOCTL_MFE_S_PICT      _IOWR('m', 7,struct mmfe_buffer)
#define IOCTL_MFE_G_BITS      _IOWR('m', 8,struct mmfe_buffer)
#define IOCTL_MFE_FLUSH         _IO('m', 9)

#endif//_MDRV_MMFE_IO_H_

