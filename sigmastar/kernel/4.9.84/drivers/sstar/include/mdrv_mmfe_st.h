/*
* mdrv_mmfe_st.h- Sigmastar
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
#ifndef _MDRV_MMFE_ST_H_
#define _MDRV_MMFE_ST_H_

#define MMFEIF_MAJ              1   //!< major version: Major number of driver-I/F version.
#define MMFEIF_MIN              2   //!< minor version: Minor number of driver-I/F version.
#define MMFEIF_EXT              1   //!< extended code: Extended number of version. It should increase when "mdrv_mmfe_io.h/mdrv_mmfe_st.h" changed.

//! User Interface version number.
#define MMFEIF_VERSION_ID       ((MMFEIF_MAJ<<22)|(MMFEIF_MIN<<12)|(MMFEIF_EXT))
//! Acquire version number.
#define MMFEIF_GET_VER(v)       (((v)>>12))
//! Acquire major version number.
#define MMFEIF_GET_MJR(v)       (((v)>>22)&0x3FF)
//! Acquire minor version number.
#define MMFEIF_GET_MNR(v)       (((v)>>12)&0x3FF)
//! Acquire extended number.
#define MMFEIF_GET_EXT(v)       (((v)>> 0)&0xFFF)

//! mmfe_pixfmt indicates pixels formats
enum mmfe_pixfmt {
    MMFE_PIXFMT_NV12=0, //!< pixel format NV12.
    MMFE_PIXFMT_NV21,   //!< pixel format NV21.
    MMFE_PIXFMT_YUYV,   //!< pixel format YUYV.
    MMFE_PIXFMT_YVYU,   //!< pixel format YVYU.
};

//! mmfe_subpel indicates interpolation granularity
enum mmfe_subpel
{
    MMFE_SUBPEL_INT=0,  //!< subpel integral.
    MMFE_SUBPEL_HALF,   //!< subpel half.
    MMFE_SUBPEL_QUATER, //!< subpel quater.
};

#define MMFE_MVBLK_4x4        (1<<0)    //!< bit-field of  4x4  block
#define MMFE_MVBLK_8x4        (1<<1)    //!< bit-field of  8x4  block
#define MMFE_MVBLK_4x8        (1<<2)    //!< bit-field of  4x8  block
#define MMFE_MVBLK_8x8        (1<<3)    //!< bit-field of  8x8  block
#define MMFE_MVBLK_16x8       (1<<4)    //!< bit-field of 16x8  block
#define MMFE_MVBLK_8x16       (1<<5)    //!< bit-field of  8x16 block
#define MMFE_MVBLK_16x16      (1<<6)    //!< bit-field of 16x16 block
#define MMFE_MVBLK_SKIP       (1<<7)    //!< bit-field of skip
#define MMFE_AVC_PROFILE_BP     66      //!< H.264 Base Line Profile
#define MMFE_AVC_PROFILE_MP     77      //!< H.264 Main Profile
#define MMFE_AVC_LEVEL_3        30      //!< H.264 Level-3.0
#define MMFE_AVC_LEVEL_31       31      //!< H.264 Level-3.1
#define MMFE_AVC_LEVEL_32       32      //!< H.264 Level-3.2
#define MMFE_AVC_LEVEL_4        40      //!< H.264 Level-4.0

//! mmfe_parm is used to apply/query configs out of streaming period.
typedef union mmfe_parm
{
    //! indicating parameter type.
    enum mmfe_parm_e
    {
        MMFE_PARM_IDX = 0,  //!< parameters of streamid: query stream-id.
        MMFE_PARM_RES,      //!< parameters of resource: including image's resolution and format
        MMFE_PARM_FPS,      //!< parameters of fps: fraction of framerate.
        MMFE_PARM_GOP,      //!< parameters of gop: ip frame period.
        MMFE_PARM_BPS,      //!< parameters of bps: bit per second.
        MMFE_PARM_MOT,      //!< parameters of motion: interpolation granularity, mv-partition.
        MMFE_PARM_AVC,      //!< parameters of avc: codec settings.
        MMFE_PARM_VUI,      //!< parameters of vui: vui params.
        MMFE_PARM_LTR,      //!< parameters of ltr: long term reference.
    } type;                 //!< indicating which kind of mmfe_parm is.

    //! set res parameter out of streaming.
    struct mmfe_parm_idx
    {
        enum mmfe_parm_e    i_type;         //!< i_type MUST be MMFE_PARM_IDX.
        int                 i_stream;       //!< stream-id.
    } idx;

    //! set res parameter out of streaming.
    struct mmfe_parm_res
    {
        enum mmfe_parm_e    i_type;         //!< i_type MUST be MMFE_PARM_RES.
        int                 i_pict_w;       //!< picture width.
        int                 i_pict_h;       //!< picture height.
        enum mmfe_pixfmt    i_pixfmt;       //!< pixel format.
        int                 i_outlen;       //!< output length: '<0' mmap-mode, '>=0' user-mode.
        int                 i_flags;        //!< flags.
#define STREAM_ID_DEFAULT       0xFFFF
        int                 i_strid;    //!< stream id, use STREAM_ID_DEFAULT as older naming
    } res;              //!< used to set resource parameters.

    //! set fps parameter out of streaming.
    struct mmfe_parm_fps
    {
        enum mmfe_parm_e    i_type;         //!< i_type MUST be MMFE_PARM_FPS.
        int                 i_num;          //!< numerator of fps.
        int                 i_den;          //!< denominator of fps.
    } fps;              //!< used to set fraction of frame rate.

    //! set mot parameter out of streaming.
    struct mmfe_parm_mot
    {
        enum mmfe_parm_e    i_type;         //!< i_type MUST be MMFE_PARM_MOT.
        int                 i_dmv_x;        //!< x-dir dmv.
        int                 i_dmv_y;        //!< y-dir dmv.
        enum mmfe_subpel    i_subpel;       //!< interpolation granularity.
        unsigned int        i_mvblks[2];    //!< mv-partitions.
    } mot;              //!< used to set motion configuration.

    //! set gop parameter out of streaming.
    struct mmfe_parm_gop
    {
        enum mmfe_parm_e    i_type;         //!< i_type MUST be MMFE_PARM_GOP.
        int                 i_pframes;      //!< p-frames count per i-frame
        int                 i_bframes;      //!< b-frames count per i/p-frame
    } gop;              //!< used to set gop structure.

    //! set bps parameter out of streaming.
    struct mmfe_parm_bps
    {
        enum mmfe_parm_e    i_type;         //!< i_type MUST be MMFE_PARM_BPS.
        int                 i_method;       //!< rate-control method.
        int                 i_ref_qp;       //!< ref. QP.
        int                 i_bps;          //!< bitrate.
    } bps;              //!< used to set bit rate controller.

    //! set avc parameter out of streaming.
    struct mmfe_parm_avc
    {
        enum mmfe_parm_e    i_type;         //!< i_type MUST be MMFE_PARM_AVC.
        unsigned short      i_profile;                  //!< profile.
        unsigned short      i_level;                    //!< level.
        unsigned char       i_num_ref_frames;           //!< ref.frames count.
        unsigned char       i_poc_type;                 //!< poc_type: support 0,2.
        unsigned char       b_cabac;                    //!< entropy: cabac/cavlc.
        unsigned char       b_constrained_intra_pred;   //!< contrained intra pred.
        unsigned char       b_deblock_filter_control;   //!< deblock filter control.
        unsigned char       i_disable_deblocking_idc;   //!< disable deblocking idc.
          signed char       i_alpha_c0_offset;          //!< offset alpha div2.
          signed char       i_beta_offset;              //!< offset beta div2.
          signed char       i_cqp_offset;               //!< cqp offset: -12 to 12 (inclusive)
    } avc;              //!< used to set codec configuration.

    //! set avc parameter out of streaming.
    struct mmfe_parm_vui
    {
        enum mmfe_parm_e    i_type;                     //!< i_type MUST be MMFE_PARM_VUI.
        int                 b_video_full_range;
        int                 b_timing_info_pres;
    } vui;              //!< used to set codec configuration.

    //! set ltr parameter out of streaming.
    struct mmfe_parm_ltr
    {
        enum mmfe_parm_e    i_type;                     //!< i_type MUST be MMFE_PARM_LTR.
        int                 b_long_term_reference;      //!< toggle ltr mode
        int                 b_enable_pred;              //!< ltr mode, means LTR P-frame can be ref.; 0: P ref. I, 1: P ref. P
        int                 i_ltr_period;               //!< ltr period
    } ltr;              //!< used to set ltr configuration.

    unsigned char           byte[64];   //!< dummy bytes

} mmfe_parm;

//! mmfe_ctrl is used to apply/query control configs during streaming period.
typedef union mmfe_ctrl
{
    //! indicating control type.
    enum  mmfe_ctrl_e
    {
        MMFE_CTRL_SEQ = 0,  //!< control of sequence: includes resolution, pixel format and frame-rate.
        MMFE_CTRL_AVC,      //!< control of avc codec settings.
        MMFE_CTRL_ROI,      //!< control of roi setting changing.
        MMFE_CTRL_SPL,      //!< control of slice spliting.
        MMFE_CTRL_LTR,      //!< control of long term reference.
    } type;                 //!< indicating which kind of mmfe_ctrl is.

    //! set seq parameter during streaming.
    struct mmfe_ctrl_seq
    {
        enum mmfe_ctrl_e    i_type;             //!< i_type MUST be MMFE_CTRL_SEQ.
        enum mmfe_pixfmt    i_pixfmt;           //!< pixel-format
        short               i_pixelw;           //!< pixels in width
        short               i_pixelh;           //!< pixels in height
        short               n_fps;              //!< numerator of frame-rate
        short               d_fps;              //!< denominator of frame-rate
    } seq;              //!< used to start new sequence.

    //! set avc parameter during streaming.
    struct mmfe_ctrl_avc
    {
        enum mmfe_ctrl_e    i_type;             //!< i_type MUST be MMFE_CTRL_AVC.
        unsigned short      i_profile;                  //!< profile.
        unsigned short      i_level;                    //!< level.
        unsigned char       i_num_ref_frames;           //!< ref.frames count.
        unsigned char       i_poc_type;                 //!< poc_type: support 0,2.
        unsigned char       b_cabac;                    //!< entropy: cabac/cavlc.
        unsigned char       b_constrained_intra_pred;   //!< Constrained intra pred.
        unsigned char       b_deblock_filter_control;   //!< deblock filter control.
        unsigned char       i_disable_deblocking_idc;   //!< disable deblocking idc.
          signed char       i_alpha_c0_offset;          //!< offset alpha div2.
          signed char       i_beta_offset;              //!< offset beta div2.
          signed char       i_cqp_offset;               //!< cqp offset: -12 to 12 (inclusive)
    } avc;              //!< used to set avc codec setting.

    //! set roi parameter during streaming.
    struct mmfe_ctrl_roi
    {
        enum mmfe_ctrl_e    i_type;             //!< i_type MUST be MMFE_CTRL_ROI.
        short               i_index;            //!< roi index.
        short               i_dqp;              //!< roi delta-qp: 0-disable roi, negative value.
        unsigned short      i_mbx;              //!< roi rectangle position-X.
        unsigned short      i_mby;              //!< roi rectangle position-Y.
        unsigned short      i_mbw;              //!< roi rectangle-W.
        unsigned short      i_mbh;              //!< roi rectangle-H.
    } roi;              //!< used to set roi region and dqp.

    //! set spl parameter during streaming.
    struct mmfe_ctrl_spl
    {
        enum mmfe_ctrl_e    i_type;             //!< i_type MUST be MMFE_CTRL_SPL.
        int                 i_rows;             //!< slice split by mb-rows.
        int                 i_bits;             //!< slice split by bitcnt.
    } spl;              //!< used to set slice splitting.

    //! set ltr parameter during streaming.
    struct mmfe_ctrl_ltr
    {
        enum mmfe_ctrl_e    i_type;             //!< i_type MUST be MMFE_CTRL_LTR.
        int                 b_enable_pred;      //!< ltr mode, means LTR P-frame can be ref.; 0: P ref. I, 1: P ref. P
        int                 i_ltr_period;       //!< ltr period
    } ltr;              //!< used to set long term reference.

    unsigned char           byte[64];   //!< dummy bytes
} mmfe_ctrl;

#define MMFE_FLAGS_IDR        (1<< 0)   //!< request IDR.
#define MMFE_FLAGS_DISPOSABLE (1<< 1)   //!< request unref-pic.
#define MMFE_FLAGS_NIGHT_MODE (1<< 2)   //!< night mode.
#define MMFE_FLAGS_LTR_PFRAME (1<< 4)   //!< LTR P-frame flag.
#define MMFE_FLAGS_SOP        (1<<30)   //!< start of picture.
#define MMFE_FLAGS_EOP        (1<<31)   //!< end of picture.
#define MMFE_MEMORY_USER      (0)       //!< user mode. (pass pointer)
#define MMFE_MEMORY_MMAP      (1)       //!< mmap mode. (pass physic address)

//!  mmfe_buff is used to exchange video/obits buffer between user and driver during streaming period.
typedef struct mmfe_buff
{
    int             i_index;    //!< index of buffer: '-1' invalid, '>=0' valid.
    int             i_flags;    //!< flags for request/reports.
    short           i_memory;   //!< memory mode of user/mmap.
    short           i_width;    //!< pixels in width. (if buffer is image)
    short           i_height;   //!< pixels in height. (if buffer is image)
    short           i_stride;   //!< pixels in stride. (if buffer is image) '<width': stride=width.
    long long       i_timecode; //!< timestamp of buffer.
    int             reserved;   //!< reserved.
    int             i_motion;   //!< measurement of motion.
    int             i_others;   //!< measurement of others.
    int             i_planes;   //!< buffer planes count.
    struct
    {
        union
        {
            unsigned long long  phys;       //!< physical address.
            void*               uptr;       //!< virtual pointers.
        } mem;                              //!< memory address for virtual pointer or physical address.
        int                     i_bias;
        int                     i_size;     //!< memory occupied size.
        int                     i_used;     //!< memory used size.
    } planes[2];                            //!< planar structure.
} mmfe_buff;

#endif//_MDRV_MMFE_ST_H_
//! @}

