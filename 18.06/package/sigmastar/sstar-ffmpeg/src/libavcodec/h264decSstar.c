/*
 * H.26L/H.264/AVC/JVT/14496-10/... decoder
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

#define UNCHECKED_BITSTREAM_READER 1

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <errno.h>

#include "libavutil/avassert.h"
#include "libavutil/display.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/stereo3d.h"
#include "libavutil/timer.h"
#include "internal.h"
#include "bytestream.h"
#include "cabac.h"
#include "cabac_functions.h"
#include "error_resilience.h"
#include "avcodec.h"
#include "h264.h"
#include "h264decSstar.h"
#include "h2645_parse.h"
#include "h264data.h"
#include "h264chroma.h"
#include "h264_mvpred.h"
#include "h264_ps.h"
#include "golomb.h"
#include "hwaccel.h"
#include "mathops.h"
#include "me_cmp.h"
#include "mpegutils.h"
#include "profiles.h"
#include "rectangle.h"
#include "thread.h"
#include "decode.h"

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

#define VDEC_CHN_ID     0

#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
    }\
    else\
    {\
        printf("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

#if 0
typedef struct pts_list {
    int64_t pts;
    struct pts_list *next;
}pts_list_t;


typedef struct pts_queue{
    struct pts_list *first, *last;	
    uint8_t idx;
}pts_queue_t;

static void pts_queue_init(pts_queue_t *ptr)
{
    memset(ptr, 0, sizeof(pts_queue_t));
}

static int pts_queue_put(pts_queue_t *q, int64_t value)
{
    pts_list_t *pts_list;

    if (NULL == (pts_list = av_malloc(sizeof(pts_list_t))))
    {
        av_log(NULL, AV_LOG_ERROR, "malloc pts list failed!\n");
        return -1;
    }

    pts_list->pts  = value;       //strore value to queue
    pts_list->next = NULL;        

    if (!q->first)
    {
        q->first = pts_list;      //queue is null 
    }
    else
    {
        q->last->next = pts_list; //queue is not null 
    }

    q->last = pts_list;           //add new list to queue tail
    q->idx ++;                    //num of queue self-adding

    return 0;
}

static int pts_queue_get(pts_queue_t *q, int64_t *value)
{
    pts_list_t *pts_head;

    pts_head = q->first;
    if (pts_head)             // queue is not null
    {
        q->first = pts_head->next;
        if (!q->first)        // queue is the last
        {
            q->last = NULL;
        }
        q->idx --;
        *value = pts_head->pts;
        av_free(pts_head);
    } else {
        av_log(NULL, AV_LOG_INFO, "pts queue is null!\n");
    }

    return 0;
}

static int pts_queue_destroy(pts_queue_t *q)
{
    pts_list_t *tmp, *tmp1;

    for (tmp = q->first; tmp; tmp = tmp1) {
        tmp1 = tmp->next;
        av_freep(&tmp);
    }
    q->last = NULL;
    q->first = NULL;
    q->idx = 0;

    return 0;
}

pts_queue_t h264_pts;
#endif

#define ENABLE_DUMP_ES      1
#define DUMP_PATH           "/mnt/pstream_h264.es"

#if ENABLE_DUMP_ES
FILE *h264_fd = NULL;
char *ssh264_dump_path = NULL;
bool  ssh264_dump_enable = false;
#endif
/****************************************************************************************************/

#define  VDEC_ES_BUF_MAX        2 * 1024 * 1024
#define  VDEC_ES_BUF_BUSY       2 * 1024 * 768
#define  GET_FRAME_TIME_OUT     3

static int ss_h264_get_frame(SsH264Context *ssctx, AVFrame *frame)
{
    MI_S32 ret;
    MI_SYS_ChnPort_t  stVdecChnPort;
    MI_VDEC_ChnStat_t stChnStat;
    SS_Vdec_BufInfo  *frame_buf;
    mi_vdec_DispFrame_t *pstVdecInfo = NULL;
    int64_t time_start = 0, time_wait = 0;

    frame->width  = ssctx->width;
    frame->height = ssctx->height;
    frame->format = ssctx->format;
    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0 || !frame->width || !frame->height)
    {
        av_log(ssctx, AV_LOG_ERROR, "av_frame_get_buffer failed\n");
        av_frame_unref(frame);
        return AVERROR(ENOMEM);
    }

    if (!(ssctx->avctx->flags & (1 << 8)))
    {
        frame_buf = (SS_Vdec_BufInfo *)av_mallocz(sizeof(SS_Vdec_BufInfo));
        if (!frame_buf)
        {
            av_log(ssctx, AV_LOG_ERROR, "av_mallocz for ss_frame_buf failed\n");
            av_frame_unref(frame);
            return AVERROR(ENOMEM);
        }

        memset(&frame_buf->stVdecBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
        memset(&stVdecChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stVdecChnPort.eModId      = E_MI_MODULE_ID_VDEC;
        stVdecChnPort.u32DevId    = 0;
        stVdecChnPort.u32ChnId    = VDEC_CHN_ID;
        stVdecChnPort.u32PortId   = 0;
        MI_SYS_SetChnOutputPortDepth(&stVdecChnPort, 5, 5);
regetframe:
        if (MI_SUCCESS == (ret = MI_SYS_ChnOutputPortGetBuf(&stVdecChnPort, &frame_buf->stVdecBufInfo, &frame_buf->stVdecHandle)))
        {
            if (frame_buf->stVdecBufInfo.eBufType == E_MI_SYS_BUFDATA_FRAME) {
                frame_buf->bType    = FALSE;
                frame->width        = frame_buf->stVdecBufInfo.stFrameData.u16Width;
                frame->height       = frame_buf->stVdecBufInfo.stFrameData.u16Height;
            } else if (frame_buf->stVdecBufInfo.eBufType == E_MI_SYS_BUFDATA_META) {
                pstVdecInfo         = (mi_vdec_DispFrame_t *)frame_buf->stVdecBufInfo.stMetaData.pVirAddr;
                frame_buf->bType    = TRUE;
                frame_buf->s32Index = pstVdecInfo->s32Idx;
                frame->width        = pstVdecInfo->stFrmInfo.u16Width;
                frame->height       = pstVdecInfo->stFrmInfo.u16Height;
            }
            frame->opaque = (SS_Vdec_BufInfo *)frame_buf;
            frame->pts    = frame_buf->stVdecBufInfo.u64Pts;
            frame->format = ssctx->format;
        }
        else
        {
            av_usleep(10 * 1000);    //等待一段时间让vdec解码,否则上层一旦陷入死循环,导致丢帧

            MI_VDEC_GetChnStat(0, &stChnStat);
            if (stChnStat.u32LeftStreamBytes > VDEC_ES_BUF_BUSY) {
                if (!time_start) {
                    time_start = av_gettime_relative();
                }
                time_wait = av_gettime_relative();
                if ((time_wait - time_start) / AV_TIME_BASE > GET_FRAME_TIME_OUT) {
                    av_freep(&frame_buf);
                    av_frame_unref(frame);
                    av_log(NULL, AV_LOG_ERROR, "h264 get frame from vdec time out!\n");
                    return MI_ERR_VDEC_FAILED;
                } 
                goto regetframe;
            }

            av_freep(&frame_buf);
            av_frame_unref(frame);
            return AVERROR(EAGAIN);
        }
    }

    return MI_SUCCESS;
}

#if 0
static int ss_h264_get_bframe(SsH264Context *ssctx, AVFrame *frame)
{
    MI_S32 ret, ysize, totalsize;

    MI_SYS_ChnPort_t  stVdecChnPort;
    MI_SYS_BUF_HANDLE stVdecHandle;
    MI_SYS_BufInfo_t  stVdecBufInfo;

    mi_vdec_DispFrame_t *pstVdecInfo = NULL;

    stVdecHandle = MI_HANDLE_NULL; 
    memset(&stVdecBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));

    memset(&stVdecChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stVdecChnPort.eModId      = E_MI_MODULE_ID_VDEC;
    stVdecChnPort.u32DevId    = 0;
    stVdecChnPort.u32ChnId    = VDEC_CHN_ID;
    stVdecChnPort.u32PortId   = 0;
    MI_SYS_SetChnOutputPortDepth(&stVdecChnPort, 2, 4);

    if (MI_SUCCESS == (ret = MI_SYS_ChnOutputPortGetBuf(&stVdecChnPort, &stVdecBufInfo, &stVdecHandle)))
    {
        void *vdec_vir_addr = NULL;
        pstVdecInfo = (mi_vdec_DispFrame_t *)stVdecBufInfo.stMetaData.pVirAddr;

        frame->width  = pstVdecInfo->stFrmInfo.u16Stride;
        frame->height = pstVdecInfo->stFrmInfo.u16Height;
        frame->format = ssctx->format;

        ret = av_frame_get_buffer(frame, 32);
        if (ret < 0 || !frame->width || !frame->height)
        {
            av_frame_unref(frame);
            ret = AVERROR(ENOMEM);
        }
        else
        {
            //get frame pts from vdec
            frame->pts = stVdecBufInfo.u64Pts;
            //pts_queue_get(&h264_pts, &frame->pts);
            //av_log(NULL, AV_LOG_INFO, "[%s %d]get frame pts : %lld\n", __FUNCTION__, __LINE__, stVdecBufInfo.u64Pts);
            ysize     = frame->width * frame->height;
            totalsize = ysize + ysize / 2;
            //av_log(NULL, AV_LOG_INFO, "vdec output buf width : %d, height : %d, total size : %d\n", frame->width, frame->height, totalsize);
            //使用Map地址与大小必须4K对齐
            //av_log(NULL, AV_LOG_INFO, "phyLumaAddr : 0x%llx, phyChromaAddr : 0x%llx\n", pstVdecInfo->stFrmInfo.phyLumaAddr, pstVdecInfo->stFrmInfo.phyChromaAddr);
            MI_SYS_Mmap(pstVdecInfo->stFrmInfo.phyLumaAddr, ALIGN_UP(totalsize, 4096), &vdec_vir_addr, FALSE);
            //复制图像信息到frame
            if (frame->buf[0])
            {
                memcpy(frame->data[0], vdec_vir_addr , ysize);
                memcpy(frame->data[1], vdec_vir_addr + ysize, ysize / 2);
            }

            //FILE *fpread = fopen("/mnt/h264_output.yuv", "a+");
            //fwrite(vdec_vir_addr , ysize, 1, fpread);
            //fwrite(vdec_vir_addr + ysize, ysize / 2, 1, fpread);
            //fclose(fpread);

            MI_SYS_Munmap(vdec_vir_addr, ALIGN_UP(totalsize, 4096));
        }

        if (MI_SUCCESS != MI_SYS_ChnOutputPortPutBuf(stVdecHandle)) {
            av_log(ssctx, AV_LOG_ERROR, "vdec output put buf error!\n");
        }
    } 
    else 
        ret = AVERROR(EAGAIN);

    return ret;
}

// 此函数用于获取不带B帧图像
static int ss_h264_get_frame(SsH264Context *ssctx, AVFrame *frame)
{
    MI_S32 ret, ysize;
    MI_SYS_BUF_HANDLE stVdecHandle;
    MI_SYS_BufInfo_t  stVdecBufInfo;
    MI_SYS_ChnPort_t  stVdecChnPort;

    stVdecHandle = MI_HANDLE_NULL; 
    memset(&stVdecBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
    memset(&stVdecChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stVdecChnPort.eModId      = E_MI_MODULE_ID_VDEC;
    stVdecChnPort.u32DevId    = 0;
    stVdecChnPort.u32ChnId    = VDEC_CHN_ID;
    stVdecChnPort.u32PortId   = 0;
    MI_SYS_SetChnOutputPortDepth(&stVdecChnPort, 2, 4);
    
    if (MI_SUCCESS == (ret = MI_SYS_ChnOutputPortGetBuf(&stVdecChnPort, &stVdecBufInfo, &stVdecHandle)))
    {
        frame->width  = stVdecBufInfo.stFrameData.u32Stride[0];
        frame->height = stVdecBufInfo.stFrameData.u16Height;
        frame->format = ssctx->format;

        ret = av_frame_get_buffer(frame, 32);
        if (ret < 0 || !frame->width || !frame->height) {
            av_frame_unref(frame);
            ret = AVERROR(ENOMEM);
        }
        else
        {
            //get frame pts from vdec
            frame->pts = stVdecBufInfo.u64Pts;
            //pts_queue_get(&h264_pts, &frame->pts);
            //av_log(NULL, AV_LOG_INFO, "[%s %d]get frame pts : %lld\n", __FUNCTION__, __LINE__, stVdecBufInfo.u64Pts);
            ysize  = frame->width * frame->height;
            //av_log(NULL, AV_LOG_INFO, "vdec output buf width : %d, height : %d\n", frame->width, frame->height);
            if (frame->buf[0]) {
                memcpy(frame->data[0], stVdecBufInfo.stFrameData.pVirAddr[0], ysize);
                memcpy(frame->data[1], stVdecBufInfo.stFrameData.pVirAddr[1], ysize / 2);
            }
        }
        
        if (MI_SUCCESS != MI_SYS_ChnOutputPortPutBuf(stVdecHandle)) {
            av_log(ssctx, AV_LOG_ERROR, "vdec output put buf error!\n");
        }
    } 
    else 
        ret = AVERROR(EAGAIN);

    return ret;
}
#endif

static MI_S32 ss_h264_send_stream(MI_U8 *data, MI_U32 size, int64_t pts, int flag)
{
    MI_VDEC_VideoStream_t stVdecStream;
    MI_S32 s32Ret;
    int64_t time_start = 0, time_wait = 0;

    memset(&stVdecStream, 0, sizeof(MI_VDEC_VideoStream_t));
    stVdecStream.pu8Addr      = data;
    stVdecStream.u32Len       = size;
    stVdecStream.u64PTS       = pts;
    stVdecStream.bEndOfFrame  = 1;
    stVdecStream.bEndOfStream = (flag) ? TRUE : FALSE;
    if (stVdecStream.bEndOfStream) {
        av_log(NULL, AV_LOG_INFO, "vdec end of h264 stream flag set!\n");
    }

    //av_log(NULL, AV_LOG_INFO, "size : %d. data : %x,%x,%x,%x,%x,%x,%x,%x.\n", stVdecStream.u32Len, stVdecStream.pu8Addr[0],
    //stVdecStream.pu8Addr[1], stVdecStream.pu8Addr[2], stVdecStream.pu8Addr[3], stVdecStream.pu8Addr[4],
    //stVdecStream.pu8Addr[5], stVdecStream.pu8Addr[6], stVdecStream.pu8Addr[7]);

    #if ENABLE_DUMP_ES
    if (ssh264_dump_enable) {
        fwrite(stVdecStream.pu8Addr, stVdecStream.u32Len, 1, h264_fd);
    }
    #endif
    //av_log(NULL, AV_LOG_INFO, "[%s %d]send to stream pts : %lld\n",  __FUNCTION__, __LINE__, stVdecStream.u64PTS);
    s32Ret = MI_VDEC_SendStream(VDEC_CHN_ID, &stVdecStream, 30);
    while (s32Ret == MI_ERR_VDEC_BUF_FULL) {
        av_usleep(10 * 1000);
        if (!time_start) {
            av_log(NULL, AV_LOG_ERROR, "vdec es buf is full!\n");
            time_start = av_gettime_relative();
        }
        time_wait = av_gettime_relative();
        if ((time_wait - time_start) / AV_TIME_BASE > 2) {
            av_log(NULL, AV_LOG_ERROR, "ss_h264_send_stream time out!\n");
            return MI_ERR_VDEC_FAILED;
        }
        s32Ret = MI_VDEC_SendStream(VDEC_CHN_ID, &stVdecStream, 30);
    }

    if (s32Ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "[%s %d]MI_VDEC_SendStream failed!\n", __FUNCTION__, __LINE__);
        return MI_ERR_VDEC_FAILED;
    }
    //av_log(NULL, AV_LOG_INFO, "[%s %d]MI_VDEC_SendStream success!.\n", __FUNCTION__, __LINE__);

    return s32Ret;
}	

static av_cold int ss_h264_decode_end(AVCodecContext *avctx)
{
    SsH264Context *s = (SsH264Context *)avctx->priv_data;

    if (s->extradata) {
        av_freep(&s->extradata);
    }

    if (s->pkt_buf) {
        av_freep(&s->pkt_buf);
    }

    ff_h2645_packet_uninit(&s->pkt);
    //pts_queue_destroy(&h264_pts);

    #if ENABLE_DUMP_ES
    if (ssh264_dump_path) {
        av_freep(&ssh264_dump_path);
    }
    if (h264_fd) {
        fclose(h264_fd);
    }
    ssh264_dump_enable = false;
    #endif

    STCHECKRESULT(MI_VDEC_StopChn(VDEC_CHN_ID));
    STCHECKRESULT(MI_VDEC_DestroyChn(VDEC_CHN_ID));
    STCHECKRESULT(MI_VDEC_DeInitDev());

    av_log(avctx, AV_LOG_WARNING, "ss_h264_decode_end successful!\n");

    return 0;
}

static int ss_h264_decode_parse(SsH264Context *s, const uint8_t *buf, int buf_size, int is_avc, void *logctx)
{
    H2645Packet pkt = { 0 };
    int i, ret = 0;
    const uint8_t start_code[4]={0,0,0,1};

    ret = ss_h2645_packet_split(&pkt, buf, buf_size, logctx, is_avc, 2, AV_CODEC_ID_H264, 1);
    if (ret < 0) {
        goto fail;
    }

    for (i = 0; i < pkt.nb_nals; i++) {
        H2645NAL *nal = &pkt.nals[i];
        if (!nal->data[nal->size - 1] && !s->is_avc)
            nal->size = nal->size - 1;
        switch (nal->type) {
        case H264_NAL_SPS:
            s->extradata_size = 0;
        case H264_NAL_PPS:
            // copy sps data to extradata
            memcpy(s->extradata + s->extradata_size, start_code, sizeof(start_code));
            memcpy(s->extradata + s->extradata_size + sizeof(start_code), nal->data, nal->size);
            s->extradata[s->extradata_size + 3] = 1;
            s->extradata_size += (nal->size + sizeof(start_code));
            av_log(NULL, AV_LOG_INFO, "type of nal : %x. len : %d, data : %x,%x,%x,%x\n", nal->type, nal->size, nal->data[0],nal->data[1],nal->data[2],nal->data[3]);
            break;
        default:
            av_log(logctx, AV_LOG_VERBOSE, "Ignoring NAL type %d in extradata\n",
                   nal->type);
            break;
        }
    }
    s->find_header = 1;
    ss_h264_send_stream(s->extradata, s->extradata_size, 0, 0);
fail:
    ff_h2645_packet_uninit(&pkt);
    return ret;
}

static int ss_h264_decode_extradata(SsH264Context *s, const uint8_t *data, int size,
                             int *is_avc, int *nal_length_size, void *logctx)
{
    int ret;

    if (!data || size <= 0)
        return -1;

    if (data[0] == 1) {
        int i, cnt, nalsize;
        const uint8_t *p = data;

        *is_avc = 1;
        s->start_len = 4;
        if (size < 7) {
            av_log(logctx, AV_LOG_ERROR, "avcC %d too short\n", size);
            return AVERROR_INVALIDDATA;
        }

        // Decode sps from avcC
        cnt = *(p + 5) & 0x1f; // Number of sps
        p  += 6;
        for (i = 0; i < cnt; i++) {
            nalsize = AV_RB16(p) + 2;
            if (nalsize > size - (p - data))
                return AVERROR_INVALIDDATA;
            ret = ss_h264_decode_parse(s, p, nalsize, 1, logctx);
            if (ret < 0) {
                av_log(logctx, AV_LOG_ERROR,
                       "Decoding sps %d from avcC failed\n", i);
                return ret;
            }
            p += nalsize;
        }
        // Decode pps from avcC
        cnt = *(p++); // Number of pps
        for (i = 0; i < cnt; i++) {
            nalsize = AV_RB16(p) + 2;
            if (nalsize > size - (p - data))
                return AVERROR_INVALIDDATA;
            ret = ss_h264_decode_parse(s, p, nalsize, 1, logctx);
            if (ret < 0) {
                av_log(logctx, AV_LOG_ERROR,
                       "Decoding pps %d from avcC failed\n", i);
                return ret;
            }
            p += nalsize;
        }
        // Store right nal length size that will be used to parse all other nals
        *nal_length_size = (data[4] & 0x03) + 1;
    } else {
        *is_avc = 0;
        s->start_len = 3;
        ret = ss_h264_decode_parse(s, data, size, 0, logctx);
        if (ret < 0)
            return ret;
    }
    if (s->find_header)
        av_log(NULL, AV_LOG_INFO, "ssh264 find valid sps, pps nal header!\n");
    return size;
}

static MI_U32 ss_h264_vdec_init(AVCodecContext *avctx)
{
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_CHN stVdecChn = VDEC_CHN_ID;
    MI_VDEC_InitParam_t stVdecInitParam;
    MI_S32 stWidth, stHeight;

    if (avctx->width <= 0
        || avctx->height <= 0
        || (avctx->flags  & 0xFFFF) <= 8
        || (avctx->flags2 & 0xFFFF) <= 8
        || (avctx->flags  & 0xFFFF) >= 8192
        || (avctx->flags2 & 0xFFFF) >= 8192) {
        av_log(avctx, AV_LOG_ERROR, "ss_h264_vdec_init parameter check error!\n");
        return AVERROR_UNKNOWN;
    }

    av_log(avctx, AV_LOG_WARNING, "h264 has b frames : %d\n", avctx->has_b_frames);
    memset(&stVdecInitParam, 0, sizeof(MI_VDEC_InitParam_t));
    if (avctx->has_b_frames > 0)
        stVdecInitParam.bDisableLowLatency = true;
    else
        stVdecInitParam.bDisableLowLatency = false;
    STCHECKRESULT(MI_VDEC_InitDev(&stVdecInitParam));

    if (!(avctx->flags & (1 << 17))) {
        STCHECKRESULT(MI_VDEC_SetOutputPortLayoutMode(E_MI_VDEC_OUTBUF_LAYOUT_LINEAR));
    } else {
        av_log(avctx, AV_LOG_WARNING, "h264 enable vdec tilemode\n");
        STCHECKRESULT(MI_VDEC_SetOutputPortLayoutMode(E_MI_VDEC_OUTBUF_LAYOUT_TILE));
    }

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType    = E_MI_VDEC_CODEC_TYPE_H264;
    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 16;
    stVdecChnAttr.eVideoMode    = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize    = VDEC_ES_BUF_MAX;
    stVdecChnAttr.u32PicWidth   = avctx->width;
    stVdecChnAttr.u32PicHeight  = avctx->height;
    stVdecChnAttr.eDpbBufMode   = E_MI_VDEC_DPB_MODE_NORMAL;
    stVdecChnAttr.u32Priority   = 0;

    STCHECKRESULT(MI_VDEC_CreateChn(stVdecChn, &stVdecChnAttr));
    STCHECKRESULT(MI_VDEC_StartChn(stVdecChn));

    stWidth  = (avctx->flags  & 0xFFFF);
    stHeight = (avctx->flags2 & 0xFFFF);
    memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
    stOutputPortAttr.u16Width   = (stWidth  > 0) ? stWidth  : avctx->width;
    stOutputPortAttr.u16Height  = (stHeight > 0) ? stHeight : avctx->height;
    STCHECKRESULT(MI_VDEC_SetOutputPortAttr(stVdecChn, &stOutputPortAttr));
    av_log(avctx, AV_LOG_INFO, "ssh264 vdec input width, height : [%d,%d], output width, height : [%d,%d]\n", avctx->width, avctx->height, stWidth, stHeight);

    avctx->flags  &= (0xFFFF0000);
    avctx->flags2 &= (0xFFFF0000);
    if (avctx->has_b_frames > 0)
        avctx->flags |= (1 << 6);
    else
        avctx->flags &= ~(1 << 6);

    av_log(avctx, AV_LOG_INFO, "ss_h264_vdec_init successful!\n");

    return MI_SUCCESS;
}


static av_cold int ss_h264_decode_init(AVCodecContext *avctx)
{
    MI_S32 ret;
    const AVPixFmtDescriptor *desc;
    SsH264Context *s = (SsH264Context *)avctx->priv_data;

    s->time_sec  = 0;
    s->time_wait = 0;
    s->avctx  = avctx;
    s->format = AV_PIX_FMT_NV12;
    s->width  = (avctx->flags & 0xFFFF);
    s->height = (avctx->flags2 & 0xFFFF);
    s->find_header        = 0;
    s->extradata_size     = 0;
    s->max_extradata_size = 256;
    s->extradata = av_mallocz(s->max_extradata_size);
    if (!s->extradata)
    {
        av_log(avctx, AV_LOG_ERROR, "ssh264 malloc for extra data failed!\n");
        return AVERROR(ENOMEM);
    }

    //av_log(NULL, AV_LOG_INFO, "ss_h264_decode_init width: %d, height : %d\n", avctx->flags, avctx->flags2);

    if (avctx->pix_fmt != AV_PIX_FMT_NONE) {
        desc = av_pix_fmt_desc_get(avctx->pix_fmt);
        av_log(avctx, AV_LOG_INFO, "video prefix format : %s.\n", desc->name);
    } else {
        avctx->pix_fmt  = AV_PIX_FMT_NV12;
    }

    // Init pts output
    //pts_queue_init(&h264_pts);

    #if ENABLE_DUMP_ES
    char *env = getenv("SSH264_DUMP");
    if (env) {
        if (!strncmp(env, "1", 1)) {
            char *path = getenv("SSH264_DUMP_PATH");
            if (path) {
                ssh264_dump_path = av_strdup(path);
                av_log(avctx, AV_LOG_INFO, "ssh264 dump path = %s\n", ssh264_dump_path);
                h264_fd = fopen(ssh264_dump_path, "w+");
                if (h264_fd) {
                    ssh264_dump_enable = true;
                } else {
                    perror("ssh264 open file error");
                }
            } else {
                av_log(avctx, AV_LOG_ERROR, "ssh264 dump path isn't set!\n");
            }
        }
    }
    #endif

    // Init vdec module
    if (MI_SUCCESS != (ret = ss_h264_vdec_init(avctx)))
    {
        av_log(avctx, AV_LOG_ERROR, "ss_h264_vdec_init failed!\n");
        if (s->extradata) {
            av_freep(&s->extradata);
        }
        return AVERROR_UNKNOWN;
    }

    //check h264 or avc1
    if (avctx->extradata_size > 0 && avctx->extradata) 
    {
        ret = ss_h264_decode_extradata(s, avctx->extradata, avctx->extradata_size,
                                       &s->is_avc, &s->nal_length_size, s->avctx);
        if (ret < 0 || s->extradata_size > s->max_extradata_size) {
            av_log(avctx, AV_LOG_ERROR, "ss_h264_decode_extradata failed!\n");
            ss_h264_decode_end(avctx);
            return ret;
        }
    }

    av_log(avctx, AV_LOG_INFO, "ss_h264_decode_init successful!\n");

    return 0;
}

static int64_t ss_h264_guess_correct_pts(AVCodecContext *ctx, int64_t reordered_pts, int64_t dts)
{
    int64_t pts = AV_NOPTS_VALUE;

    if (dts != AV_NOPTS_VALUE) {
        ctx->pts_correction_num_faulty_dts += dts <= ctx->pts_correction_last_dts;
        ctx->pts_correction_last_dts = dts;
    } else if (reordered_pts != AV_NOPTS_VALUE)
        ctx->pts_correction_last_dts = reordered_pts;

    if (reordered_pts != AV_NOPTS_VALUE) {
        ctx->pts_correction_num_faulty_pts += reordered_pts <= ctx->pts_correction_last_pts;
        ctx->pts_correction_last_pts = reordered_pts;
    } else if(dts != AV_NOPTS_VALUE)
        ctx->pts_correction_last_pts = dts;

    if (reordered_pts != AV_NOPTS_VALUE)
        pts = reordered_pts;
    else
        pts = ctx->pts_correction_last_pts;

    return pts;
}

static int ss_h264_decode_nalu(SsH264Context *s, AVPacket *avpkt)
{
    int ret, i, data_idx;
    const uint8_t start_code[4] = {0,0,0,1};

    ret = ss_h2645_packet_split(&s->pkt, avpkt->data, avpkt->size, s->avctx, s->is_avc,
                                 s->nal_length_size, s->avctx->codec_id, 1);
    if (ret < 0) {
        av_log(s->avctx, AV_LOG_ERROR, "Error splitting the input into NAL units.\n");
        return ret;
    }

    //av_log(NULL, AV_LOG_INFO, "avpkt size : %d, pkt.nb_nals : %d\n", avpkt->size, s->pkt.nb_nals);
    /* decode the NAL units */
    if (s->pkt_buf) {
        av_freep(&s->pkt_buf);
    }
    s->pkt_buf = av_mallocz(avpkt->size + s->max_extradata_size);
    if (!s->pkt_buf) {
        ff_h2645_packet_uninit(&s->pkt);
        return AVERROR(ENOMEM);
    }

    data_idx = 0;
    for (i = 0; i < s->pkt.nb_nals; i++)
    {
        H2645NAL *nal = &s->pkt.nals[i];
        //discard last data if is not avc1
        if (!nal->data[nal->size - 1] && !s->is_avc)
            nal->size = nal->size - 1;
        switch (nal->type) {
            case H264_NAL_IDR_SLICE:
                if (data_idx == 0 && s->find_header) {
                    s->avctx->flags &= ~(1 << 7);
                    memcpy(s->pkt_buf, s->extradata, s->extradata_size);
                    data_idx = s->extradata_size;
                }
            case H264_NAL_SLICE:
            case H264_NAL_DPA:
            case H264_NAL_DPB:
            case H264_NAL_DPC:
            case H264_NAL_SPS:
            case H264_NAL_PPS:
                //if (h264_pts.idx > 10)
                //    pts_queue_get(&h264_pts, &ret);
                //pts_queue_put(&h264_pts, avpkt->pts);
                //av_log(NULL, AV_LOG_INFO, "pps,sps,sei dts : %lld, pts : %lld\n", avpkt->dts, avpkt->pts);
                if (!(s->avctx->flags & (1 << 7)) && s->find_header)
                {
                    //add data head to nal
                    memset(s->pkt_buf + data_idx, 0, s->start_len);
                    memcpy(s->pkt_buf + data_idx + s->start_len, nal->data, nal->size);
                    s->pkt_buf[data_idx + s->start_len - 1] = 1;
                    data_idx += (nal->size + s->start_len);
                    //av_log(NULL, AV_LOG_INFO, "extra size : %d, nal size : %d, nal data : %x,%x,%x,%x,%x,%x\n", data_idx, nal->size + 4, extrabuf[data_idx + 2], 
                    //extrabuf[data_idx + 3], extrabuf[data_idx + 4], extrabuf[data_idx + 5], extrabuf[data_idx + 6], extrabuf[data_idx + 7]);
                }
                if (!s->find_header && (nal->type == H264_NAL_SPS || nal->type == H264_NAL_PPS))
                {
                    if (nal->type == H264_NAL_SPS)
                        s->extradata_size = 0;
                    else if (nal->type == H264_NAL_PPS)
                        s->find_header = !s->find_header;
                    memcpy(s->extradata + s->extradata_size, start_code, sizeof(start_code));
                    memcpy(s->extradata + s->extradata_size + sizeof(start_code), nal->data, nal->size);
                    s->extradata[s->extradata_size + 3] = 1;
                    s->extradata_size += (nal->size + sizeof(start_code));
                }
                break;
            default : break;
        }
    }

    s->pkt_size = data_idx;
    s->pts = ss_h264_guess_correct_pts(s->avctx, avpkt->pts, avpkt->dts);

    ff_h2645_packet_uninit(&s->pkt);

    return ret;
}

static int ss_h264_decode_frame(AVCodecContext *avctx, void *data,
                             int *got_frame, AVPacket *avpkt)
{
    int ret;
    SsH264Context *s = avctx->priv_data;
    AVCodecInternal *avci = avctx->internal;
    AVFrame *frame = avci->buffer_frame;

    //av_log(avctx, AV_LOG_INFO, "get in ss_h264_decode_frame\n");

    if(avctx->debug)
    {
        /* end of stream and vdec buf is null*/
        if (!avpkt->size || !avpkt->data) {
            av_log(avctx, AV_LOG_INFO, "packet size is 0!!\n");
            return AVERROR_EOF;
        } else {
            *got_frame = 0;
            ret = ss_h264_get_frame(s, frame);
            if (MI_SUCCESS == ret) {
                *got_frame = 1;
                frame->best_effort_timestamp = frame->pts;
            }

            if (s->pkt_buf) {
                if (!(s->avctx->flags & (1 << 7)) && s->find_header) {
                    ret = ss_h264_send_stream(s->pkt_buf, s->pkt_size, s->pts, 0);
                }
                av_freep(&s->pkt_buf);
            }
            if (ret == MI_ERR_VDEC_FAILED && !(*got_frame)) {
                return MI_ERR_VDEC_FAILED;
            }

            //continue to decode nalu and fill stream data to memory
            ret = ss_h264_decode_nalu(s, avpkt);
            if (ret < 0)
                av_log(avctx, AV_LOG_ERROR, "ss_h264_decode_nalu failed!\n");

            if (*got_frame)
                av_assert0(frame->buf[0]);
        }
    } 

    if (ret < 0)
        return ret;
    return avpkt->size;
}

static int ss_h264_receive_frame(AVCodecContext *avctx, AVFrame *frame)
{
    int ret, ret1, ret2, got_frame;
    SsH264Context        *s = avctx->priv_data; 
    AVCodecInternal   *avci = avctx->internal;
    DecodeSimpleContext *ds = &avci->ds;
    AVPacket         *avpkt = ds->in_pkt;

    //av_log(avctx, AV_LOG_INFO, "get in ss_h264_receive_frame\n");

    if (true == avctx->debug) 
    {
        while (!frame->buf[0])
        {
            got_frame = 0;
            if (MI_SUCCESS == (ret2 = ss_h264_get_frame(s, frame)))
            {
                got_frame = 1;
                frame->best_effort_timestamp = frame->pts;
            }

            if (!avpkt->data && !avci->draining)
            {
                av_packet_unref(avpkt);
                ret = ff_decode_get_packet(avctx, avpkt);
                if (ret >= 0 && avpkt->data) 
                {
                    if (s->pkt_buf) {
                        if (!(s->avctx->flags & (1 << 7)) && s->find_header) {
                            ret1 = ss_h264_send_stream(s->pkt_buf, s->pkt_size, s->pts, 0);
                        }
                        av_freep(&s->pkt_buf);
                    }

                    if (0 > ss_h264_decode_nalu(s, avpkt)) {
                        av_log(avctx, AV_LOG_ERROR, "ss_h264_decode_nalu failed!\n");
                    }

                    if (!(avctx->codec->caps_internal & FF_CODEC_CAP_SETS_PKT_DTS))
                        frame->pkt_dts = avpkt->dts;
                    if (avctx->codec->type == AVMEDIA_TYPE_VIDEO && !avctx->has_b_frames) {
                        frame->pkt_pos = avpkt->pos;
                    }

                    if (!got_frame)
                        av_frame_unref(frame);

                    if (ret >= 0 && avctx->codec->type == AVMEDIA_TYPE_VIDEO && !(avctx->flags & AV_CODEC_FLAG_TRUNCATED))
                        ret = avpkt->size;
                    avci->compat_decode_consumed += ret;

                    if (ret >= avpkt->size || ret < 0) {
                        av_packet_unref(avpkt);
                    } else {
                        int consumed = ret;

                        avpkt->data                += consumed;
                        avpkt->size                -= consumed;
                        avci->last_pkt_props->size -= consumed; // See extract_packet_props() comment.
                        avpkt->pts                  = AV_NOPTS_VALUE;
                        avpkt->dts                  = AV_NOPTS_VALUE;
                        avci->last_pkt_props->pts = AV_NOPTS_VALUE;
                        avci->last_pkt_props->dts = AV_NOPTS_VALUE;
                    }

                    if (got_frame)
                        av_assert0(frame->buf[0]);
                }else if (avci->draining) {
                    if (s->pkt_buf) {
                        if (!(s->avctx->flags & (1 << 7)) && s->find_header) {
                            ret1 = ss_h264_send_stream(s->pkt_buf, s->pkt_size, s->pts, 1);
                        }
                        av_freep(&s->pkt_buf);
                    }
                }

                if (ret1 == MI_ERR_VDEC_FAILED && !got_frame) {
                    return MI_ERR_VDEC_FAILED;
                }
            }

            if (ret2 < 0) {
                return ret2;
            }
        }
    }

    return avpkt->size;
}

static void ss_h264_decode_flush(AVCodecContext *avctx)
{
    if (avctx->flags & (1 << 7))
    {
        MI_VDEC_FlushChn(VDEC_CHN_ID);
        //av_log(avctx, AV_LOG_INFO, "ss_h264_decode_flush done!\n");
    }
}

AVCodec ff_ssh264_decoder = {
    .name                  = "ssh264",
    .long_name             = NULL_IF_CONFIG_SMALL("H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_H264,
    .priv_data_size        = sizeof(H264Context),
    .init                  = ss_h264_decode_init,
    .close                 = ss_h264_decode_end,
    .decode                = ss_h264_decode_frame,
    .receive_frame         = ss_h264_receive_frame,
    .flush                 = ss_h264_decode_flush,
    .capabilities          = /*AV_CODEC_CAP_DRAW_HORIZ_BAND | AV_CODEC_CAP_DR1 |*/
                             AV_CODEC_CAP_DELAY | AV_CODEC_CAP_SLICE_THREADS |
                             AV_CODEC_CAP_FRAME_THREADS,
    .hw_configs            = (const AVCodecHWConfigInternal*[]) {
                               NULL
                           },
};

