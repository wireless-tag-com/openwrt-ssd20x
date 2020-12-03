/*
* mdrv_mvhe_st.h- Sigmastar
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
#ifndef _MDRV_MVHE_ST_H_
#define _MDRV_MVHE_ST_H_

#define MVHEIF_MAJ              1   //!< major version: Major number of driver-I/F version.
#define MVHEIF_MIN              2   //!< minor version: Minor number of driver-I/F version.
#define MVHEIF_EXT              1   //!< extended code: Extended number of version. It should increase when "mdrv_mvhe_io.h/mdrv_mvhe_st.h" changed.

//! User Interface version number.
#define MVHEIF_VERSION_ID       ((MVHEIF_MAJ<<22)|(MVHEIF_MIN<<12)|(MVHEIF_EXT))
//! Acquire version number.
#define MVHEIF_GET_VER(v)       (((v)>>12))
//! Acquire major version number.
#define MVHEIF_GET_MJR(v)       (((v)>>22)&0x3FF)
//! Acquire minor version number.
#define MVHEIF_GET_MNR(v)       (((v)>>12)&0x3FF)
//! Acquire extended number.
#define MVHEIF_GET_EXT(v)       (((v)>> 0)&0xFFF)

//! mvhe_pixfmt indicates pixels formats
enum mvhe_pixfmt
{
    MVHE_PIXFMT_NV12=0, //!< pixel format NV12.
    MVHE_PIXFMT_NV21,   //!< pixel format NV21.
    MVHE_PIXFMT_YUYV,   //!< pixel format YUYV.
    MVHE_PIXFMT_YVYU,   //!< pixel format YVYU.
};

#define MVHE_HEVC_PROFILE_MAIN       1  //!< hevc main   profile
#define MVHE_HEVC_PROFILE_MAIN10     2  //!< hevc main10 profile
#define MVHE_HEVC_LEVEL_1           30  //!< hevc level 1.0
#define MVHE_HEVC_LEVEL_2           60  //!< hevc level 2.0
#define MVHE_HEVC_LEVEL_2_1         63  //!< hevc level 2.1
#define MVHE_HEVC_LEVEL_3           90  //!< hevc level 3.0
#define MVHE_HEVC_LEVEL_3_1         93  //!< hevc level 3.1
#define MVHE_HEVC_LEVEL_4          120  //!< hevc level 4.0
#define MVHE_HEVC_LEVEL_4_1        124  //!< hevc level 4.1
#define MVHE_HEVC_LEVEL_5          150  //!< hevc level 5.0
#define MVHE_HEVC_LEVEL_5_1        153  //!< hevc level 5.1
#define MVHE_HEVC_LEVEL_5_2        156  //!< hevc level 5.2
#define MVHE_HEVC_LEVEL_6          180  //!< hevc level 6.0
#define MVHE_HEVC_LEVEL_6_1        183  //!< hevc level 6.1
#define MVHE_HEVC_LEVEL_6_2        186  //!< hevc level 6.2
#define MVHE_FLAGS_CMPRY        (1<< 0)
#define MVHE_FLAGS_COMPR        (MVHE_FLAGS_CMPRY)

//! mvhe_parm is used to apply/query configs out of streaming period.
typedef union mvhe_parm
{
    //! mvhe_parm_e indicates parameter type.
    enum mvhe_parm_e
    {
        MVHE_PARM_IDX=0,    //!< parameters of streamid: query stream-id.
        MVHE_PARM_RES,      //!< parameters of resource: including image's resolution and format
        MVHE_PARM_FPS,      //!< parameters of fps: fraction of framerate.
        MVHE_PARM_GOP,      //!< parameters of gop: ip frame period.
        MVHE_PARM_BPS,      //!< parameters of bps: bit per second.
        MVHE_PARM_HEVC,     //!< parameters of hevc: codec settings.
        MVHE_PARM_VUI,      //!< parameters of vui: vui params.
        MVHE_PARM_LTR,      //!< parameters of ltr: long term reference.
        MVHE_PARM_PEN,      //!< parameters of pen: penalty params.
    } type;             //!< indicating which kind of mvhe_parm is.

    //! get ver parameter out of streaming.
    struct mvhe_parm_idx
    {
        enum mvhe_parm_e    i_type;         //!< i_type MUST be MMFE_PARM_IDX.
        unsigned int        i_stream;       //!< stream-id.
    } idx;              //!< used to get version/id parameters.

    //! set res parameters out of streaming.
    struct mvhe_parm_res
    {
        enum mvhe_parm_e    i_type;     //!< i_type MUST be MVHE_PARM_RES.
        int                 i_pict_w;   //!< picture width.
        int                 i_pict_h;   //!< picture height.
        enum mvhe_pixfmt    i_pixfmt;   //!< pixel format.
        int                 i_outlen;   //!< output length: '<0' mmap-out, '=0' default, '>0' aligned to 512K
        int                 i_flags;    //!< flags.
#define STREAM_ID_DEFAULT       0xFFFF
        int                 i_strid;    //!< stream id, use STREAM_ID_DEFAULT as older naming
    } res;              //!< used to set resource parameters.

    //! set fps parameters out of streaming.
    struct mvhe_parm_fps
    {
        enum mvhe_parm_e    i_type;     //!< i_type MUST be MVHE_PARM_FPS.
        int                 i_num;      //!< numerator of fps.
        int                 i_den;      //!< denominator of fps.
    } fps;              //!< used to set fraction of frame rate.

    //! set gop parameters out of streaming.
    struct mvhe_parm_gop
    {
        enum mvhe_parm_e    i_type;     //!< i_type MUST be MVHE_PARM_GOP.
        int                 i_pframes;  //!< p-frames count per i-frame.
        int                 i_bframes;  //!< b-frames count per i/p-frame.
    } gop;              //!< used to set gop structure.

    //! set bps parameters out of streaming.
    struct mvhe_parm_bps
    {
        enum mvhe_parm_e    i_type;     //!< i_type MUST be MVHE_PARM_BPS.
        int                 i_method;   //!< rate-control method.
        int                 i_ref_qp;   //!< ref. QP.
        int                 i_bps;      //!< bitrate.
    } bps;              //!< used to set bit rate controller.

    //! set hevc parameters out of streaming.
    struct mvhe_parm_hevc
    {
        enum mvhe_parm_e    i_type;                     //!< i_type MUST be MVHE_PARM_HEVC.
        unsigned short      i_profile;                  //!< profile.
        unsigned short      i_level;                    //!< level.
        unsigned char       i_log2_max_cb_size;         //!< max ctb size.
        unsigned char       i_log2_min_cb_size;         //!< min ctb size.
        unsigned char       i_log2_max_tr_size;         //!< max trb size.
        unsigned char       i_log2_min_tr_size;         //!< min trb size.
        unsigned char       i_tr_depth_intra;           //!< tr depth intra.
        unsigned char       i_tr_depth_inter;           //!< tr depth inter.
        unsigned char       b_scaling_list_enable;      //!< scaling list enable.
        unsigned char       b_ctu_qp_delta_enable;      //!< ctu dqp enable.
        unsigned char       b_sao_enable;               //!< sao enable.
          signed char       i_cqp_offset;               //!< cqp offset: -12 to 12 (inclusive)
        unsigned char       b_strong_intra_smoothing;   //!< strong intra smoothing.
        unsigned char       b_constrained_intra_pred;   //!< intra prediction constrained.
        unsigned char       b_deblocking_override_enable;   //!< deblocking override enable.
        unsigned char       b_deblocking_disable;       //!< deblocking disable.
          signed char       i_tc_offset_div2;           //!< tc_offset_div2: -6 to 6 (inclusive)
          signed char       i_beta_offset_div2;         //!< beta_offset_div2: -6 to 6 (inclusive)
    } hevc;             //!< used to set codec configuration.

    //! set vui parameter out of streaming.
    struct mvhe_parm_vui
    {
        enum mvhe_parm_e    i_type;         //!< i_type MUST be MVHE_PARM_VUI.
        int                 b_video_full_range;
        int                 b_timing_info_pres;
    } vui;              //!< used to set codec configuration.

    //! set ltr parameter out of streaming.
    struct mvhe_parm_ltr
    {
        enum mvhe_parm_e    i_type;                     //!< i_type MUST be MVHE_PARM_LTR.
        int                 b_long_term_reference;      //!< toggle ltr mode
        int                 b_enable_pred;              //!< ltr mode, means LTR P-frame can be ref.; 0: P ref. I, 1: P ref. P
        int                 i_ltr_period;               //!< ltr period
    } ltr;              //!< used to set ltr configuration.

    unsigned char           byte[64];   //!< dummy bytes
} mvhe_parm;

//! mvhe_ctrl is used to apply/query control configs during streaming period.
typedef union mvhe_ctrl
{
    //! mvhe_ctrl_e indicates control type.
    enum mvhe_ctrl_e
    {
        MVHE_CTRL_SEQ=0,//!< control of sequence: includes resolution, pixel format and frame-rate.
        MVHE_CTRL_HEVC, //!< control of hevc codec settings.
        MVHE_CTRL_ROI,  //!< control of roi setting changing.
        MVHE_CTRL_SPL,  //!< control of slice spliting.
        MVHE_CTRL_DBK,  //!< control of deblocking.
        MVHE_CTRL_BAC,  //!< control of cabac_init.
        MVHE_CTRL_LTR,  //!< control of long term reference.
    } type;             //!< indicating which kind of mvhe_ctrl is.

    //! set seq control during streaming.
    struct mvhe_ctrl_seq
    {
        enum mvhe_ctrl_e    i_type;     //!< i_type MUST be MVHE_CTRL_SEQ.
        enum mvhe_pixfmt    i_pixfmt;   //!< pixel-format
        short               i_pixelw;   //!< pixels in width.
        short               i_pixelh;   //!< pixels in height.
        short               n_fps;      //!< numerator of frame-rate.
        short               d_fps;      //!< denominator of frame-rate.
    } seq;              //!< used to start new sequence.

    //! set hevc control during streaming.
    struct mvhe_ctrl_hevc
    {
        enum mvhe_ctrl_e    i_type;     //!< i_type MUST be MVHE_CTRL_SEQ.
        unsigned short      i_profile;                      //!< profile.
        unsigned short      i_level;                        //!< level.
        unsigned char       i_log2_max_cb_size;             //!< max ctb size.
        unsigned char       i_log2_min_cb_size;             //!< min ctb size.
        unsigned char       i_log2_max_tr_size;             //!< max trb size.
        unsigned char       i_log2_min_tr_size;             //!< min trb size.
        unsigned char       i_tr_depth_intra;               //!< tr depth intra.
        unsigned char       i_tr_depth_inter;               //!< tr depth inter.
        unsigned char       b_scaling_list_enable;          //!< scaling list enable.
        unsigned char       b_ctu_qp_delta_enable;          //!< ctu dqp enable.
        unsigned char       b_sao_enable;                   //!< sao enable.
          signed char       i_cqp_offset;                   //!< cqp offset: -12 to 12 (inclusive)
        unsigned char       b_strong_intra_smoothing;       //!< strong intra smoothing.
        unsigned char       b_constrained_intra_pred;       //!< intra prediction constrained.
        unsigned char       b_deblocking_override_enable;   //!< deblocking override enable.
        unsigned char       b_deblocking_disable;           //!< deblocking disable.
          signed char       i_tc_offset_div2;               //!< tc_offset_div2: -6 to 6 (inclusive)
          signed char       i_beta_offset_div2;             //!< beta_offset_div2: -6 to 6 (inclusive)
    } hevc;             //!< used to set hevc codec setting.

    //! set roi control during streaming.
    struct mvhe_ctrl_roi
    {
        enum mvhe_ctrl_e    i_type;     //!< i_type MUST be MVHE_CTRL_ROI.
        short               i_index;    //!< roi index.
        short               i_dqp;      //!< dqp: -15~0, =0: disable, !=0: enable.
        unsigned short      i_cbx;      //!< roi region posotion-X. (in CTB unit)
        unsigned short      i_cby;      //!< roi region posotion-Y. (in CTB unit)
        unsigned short      i_cbw;      //!< roi region rectangle-W. (in CTB unit)
        unsigned short      i_cbh;      //!< roi region rectangle-H. (in CTB unit)
    } roi;              //!< used to set roi region and dqp.

    //! set spl control during streaming.
    struct mvhe_ctrl_spl
    {
        enum mvhe_ctrl_e    i_type;     //!< i_type MUST be MVHE_CTRL_SPL.
        int                 i_rows;     //!< split slice by cb-rows.
        int                 i_bits;     //!< split slice by cb-bits.
    } spl;              //!< used to set slice splitting.

    //! set dbk control during streaming.
    struct mvhe_ctrl_dbk {
        enum mvhe_ctrl_e    i_type;     //!< i_type MUST be MVHE_CTRL_DBK.
        unsigned char       b_override; //!< override deblocking setting.
        unsigned char       b_disable;  //!< deblocking disable.
          signed char       i_tc_offset_div2;   //!< tc_offset_div2: -6 to 6 (inclusive)
          signed char       i_beta_offset_div2; //!< beta_offset_div2: -6 to 6 (inclusive)
    } dbk;              //!< used to set deblocking splitting.

    //! set bac control during streaming.
    struct mvhe_ctrl_bac
    {
        enum mvhe_ctrl_e    i_type;     //!< i_type MUST be MVHE_CTRL_BAC.
        int                 b_init;     //!< cabac_init_flag: 0,1
    } bac;              //!< used to set cabac_init.

    //! set ltr parameter during streaming.
    struct mvhe_ctrl_ltr
    {
        enum mvhe_ctrl_e    i_type;             //!< i_type MUST be MVHE_CTRL_LTR.
        int                 b_enable_pred;      //!< ltr mode, means LTR P-frame can be ref.; 0: P ref. I, 1: P ref. P
        int                 i_ltr_period;       //!< ltr period
    } ltr;              //!< used to set long term reference.

    unsigned char           byte[64];   //!< dummy bytes
} mvhe_ctrl;

#define MVHE_FLAGS_IDR        (1<< 0)   //!< request IDR.
#define MVHE_FLAGS_DISPOSABLE (1<< 1)   //!< request unref-pic.
#define MVHE_FLAGS_NIGHT_MODE (1<< 2)   //!< night mode.
#define MVHE_FLAGS_LTR_PFRAME (1<< 4)   //!< LTR P-frame flag.
#define MVHE_FLAGS_SOP        (1<<30)   //!< start of picture.
#define MVHE_FLAGS_EOP        (1<<31)   //!< end of picture.
#define MVHE_MEMORY_USER      (0)       //!< user mode. (pass pointer)
#define MVHE_MEMORY_MMAP      (1)       //!< mmap mode. (pass physic address)

//! mvhe_buff is used to exchange video/obits buffer between user and driver during streaming period.
typedef struct mvhe_buff
{
    int             i_index;        //!< index of buffer: '-1' invalid, '>=0' valid.
    int             i_flags;        //!< flags for request/reports.
    short           i_memory;       //!< memory mode of user/mmap.
    short           i_width;        //!< pixels in width. (if buffer is image)
    short           i_height;       //!< pixels in height. (if buffer is image)
    short           i_stride;       //!< pixels in stride. (if buffer is image) '<width': stride=width.
    long long       i_timecode;     //!< timestamp of buffer.
    struct
    {
        unsigned char   b_override;     //!< override deblocking.
        unsigned char   b_disable;      //!< diable deblocking.
      signed char       i_tc_offset_div2;   //!< tc_offset_div2: -6 to 6 (inclusive)
      signed char       i_beta_offset_div2; //!< beta_offset_div2: -6 to 6 (inclusive)
    } deblock;      //!< deblock for slice layer.
    int             i_motion;       //!< measurement of motion.
    int             i_others;       //!< measurement of others.
    int             i_planes;       //!< buffer planes count.
    struct
    {
        union
        {
            unsigned long long  phys;       //!< physical address.
            void*               uptr;       //!< virtual pointers.
        } mem;          //!< memory address for virtual pointer or physical address.
    int             i_bias;
    int             i_size;         //!< memory occupied size.
    int             i_used;         //!< memory used size.
    } planes[2];    //!< planar structure.
} mvhe_buff;

#endif//_MDRV_MVHE_ST_H_
//! @}

