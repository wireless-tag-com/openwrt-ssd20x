/*
 * H.26L/H.264/AVC/JVT/14496-10/... encoder/decoder
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * H.264 / AVC / MPEG-4 part10 codec.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#ifndef AVCODEC_SSH264DEC_H
#define AVCODEC_SSH264DEC_H

#include <stdbool.h>

#include "libavutil/buffer.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/thread.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>

#include "h264_parse.h"
#include "h264_ps.h"
#include "h2645_parse.h"

//sstar sdk lib
#include "mi_vdec.h"
#include "mi_vdec_datatype.h"
#include "mi_vdec_extra.h"
#include "mi_common.h"
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
//#include "mi_divp.h"
//#include "mi_divp_datatype.h"
//#include "mi_hdmi.h"
//#include "mi_hdmi_datatype.h"
//#include "mi_disp.h"
//#include "mi_disp_datatype.h"
/**
 * H264Context
 */
typedef struct SsH264Context {
    AVFrame *f;
    AVCodecContext *avctx;
    H2645Packet pkt;

    int width;
    int height;
    int format;

    int is_avc;           ///< this flag is != 0 if codec is avc1
    int nal_length_size;  ///< Number of bytes used for nal length (1, 2 or 4)
    int start_len;

    uint8_t * pkt_buf;
    int pkt_size;
    int64_t pts, dts;

    uint8_t * extradata;
    int       max_extradata_size;
    int       extradata_size;
    int       find_header;
    int64_t time_sec, time_wait;
} SsH264Context;

typedef struct SsCropContext {
    int x;
    int y;
    int cropwidth;
    int cropheight;
    
} SsCropContext;


typedef struct ST_Sys_BindInfo_s
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;
} ST_Sys_BindInfo_t;

typedef struct
{
    bool bResume;
    bool bGotFrame;
    bool bReleaseFrame;
    bool brevP;
    MI_SYS_BufInfo_t stBufInfo;

} Main_Context;

//parse SPS start
typedef struct mpeg_rational_s {
 
        int num;
        int den;
 
} mpeg_rational_t;
 
typedef struct video_size_s {

    uint16_t        width;
    uint16_t        height;
    mpeg_rational_t pixel_aspect;

} video_size_t;

typedef struct {
 
        uint16_t        width;
        uint16_t        height;
        mpeg_rational_t pixel_aspect;
        uint8_t   profile;
        uint8_t   level;
 
} h264_sps_data_t;

typedef struct {
    int     size;
    uint8_t *data;
    int64_t pts;
} vdec_stream_t;

#define NOCACHE

# ifdef NOCACHE
 
typedef struct {
    const uint8_t *data; //存放除了sps数据
    int            count; /* in bits */
    int            index; /* in bits */ 
} br_state;
 
#define BR_INIT(data,bytes) {(data), 8*(bytes), 0}
 
#define BR_EOF(br) ((br)->index >= (br)->count)
 
static inline void br_init(br_state *br, const uint8_t *data, int bytes)
{
 
    br->data  = data;
    br->count = 8*bytes;
    br->index = 0;
 
}
 
static inline int br_get_bit(br_state *br)
{
 
    if(br->index >= br->count)
        return 1; /* -> no infinite colomb's ... */
 
    int r = (br->data[br->index>>3] >> (7 - (br->index&7))) & 1;
    br->index++;
    return r;
 
}
 
static inline uint32_t br_get_bits(br_state *br, uint32_t n)
{
 
    uint32_t r = 0;
    while(n--)
        r = r | (br_get_bit(br) << n);
    return r;
 
}
 
#define br_skip_bit(br) br_skip_bits(br,1)
 
static inline void br_skip_bits(br_state *br, int n)
{
 
    br->index += n;
 
}
 
 
# else /* NOCACHE */
 
 
typedef struct {
 
    uint8_t *data;
    uint8_t *data_end;
    uint32_t cache;
    uint32_t cache_bits;
 
} br_state;
 
#define BR_INIT(data,bytes) {(data), (data)+(bytes), 0, 0}
 
static inline void br_init(br_state *br, const uint8_t *data, int bytes)
{
 
    br->data       = data;
    br->data_end   = data + bytes;
    br->cache      = 0;
    br->cache_bits = 0;
 
}
 
#define BR_GET_BYTE(br) \
(br->data < br->data_end ? *br->data++ : 0xff)
 
#define BR_EOF(br) ((br)->data >= (br)->data_end)
 
static inline uint32_t br_get_bits(br_state *br, uint32_t n)
{
 
    if(n > 24)
        return (br_get_bits(br, 16) << 16) | br_get_bits(br, n-16);
 
    while (br->cache_bits < 24) {
 
        br->cache = (br->cache<<8) | BR_GET_BYTE(br);
        br->cache_bits += 8;
 
    }
 
    br->cache_bits -= n;
    return (br->cache >> br->cache_bits) & ((1<<n) - 1);
 
}
 
static inline int br_get_bit(br_state *br)
{
 
    if(!br->cache_bits) {
 
        br->cache = BR_GET_BYTE(br);
        br->cache_bits = 7;
 
    } else {
 
        br->cache_bits--;
 
    }
    return (br->cache >> br->cache_bits) & 1;
 
}
 
static inline void br_skip_bit(br_state *br)
{
 
    if(!br->cache_bits) {
 
        br->cache = BR_GET_BYTE(br);
        br->cache_bits = 7;
 
    } else {
 
        br->cache_bits--;
 
    }
 
}
 
static inline void br_skip_bits(br_state *br, int n)
{
 
    if(br->cache_bits >= n) {
 
        br->cache_bits -= n;
 
    } else {
 
        /* drop cached bits */
        n -= br->cache_bits;
 
        /* drop full bytes */
        br->data += (n >> 3);
        n &= 7;
 
        /* update cache */
        if(n) {
 
            br->cache = BR_GET_BYTE(br);
            br->cache_bits = 8 - n;
 
        } else {
 
            br->cache_bits = 0;
 
        }
 
    }
 
}
 
 
# endif /* NOCACHE */

#define br_get_u8(br)         br_get_bits(br, 8)
#define br_get_u16(br)        ((br_get_bits(br, 8)<<8) | br_get_bits(br, 8))
 
static inline uint32_t br_get_ue_golomb(br_state *br)
{
 
    int n = 0;
    while (!br_get_bit(br) && n < 32)
        n++;
    return n ? ((1<<n) - 1) + br_get_bits(br, n) : 0;
 
}
 
//#pragma warning(disable: 4146)
 
static inline int32_t br_get_se_golomb(br_state *br)
{
 
    uint32_t r = br_get_ue_golomb(br) + 1;
    return (r&1) ? -(r>>1) : (r>>1);
 
}
 
static inline void br_skip_golomb(br_state *br)
{
 
    int n = 0;
    while (!br_get_bit(br) && n < 32)
        n++;
    br_skip_bits(br, n);
 
}
 
#define br_skip_ue_golomb(br) br_skip_golomb(br)
#define br_skip_se_golomb(br) br_skip_golomb(br)

#define BR_INIT(data,bytes) {(data), 8*(bytes), 0}

#define br_get_u8(br)         br_get_bits(br, 8)
#define br_get_u16(br)        ((br_get_bits(br, 8)<<8) | br_get_bits(br, 8))

int h264_parse_sps(const uint8_t *buf, int len, h264_sps_data_t *sps);


//parse SPS end


#endif /* AVCODEC_SSH264DEC_H */
