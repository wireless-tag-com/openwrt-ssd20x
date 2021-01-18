/**
 * @file
 * H.265/HEVC codec.
 * @author jeffrey.wu <jeffrey.wu@sigmastar.com.cn>
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#include "libavutil/avassert.h"
#include "libavutil/display.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/stereo3d.h"
#include "libavutil/time.h"
#include "internal.h"
#include "bytestream.h"
#include "cabac.h"
#include "cabac_functions.h"
#include "error_resilience.h"
#include "avcodec.h"
#include "golomb.h"
#include "hwaccel.h"
#include "mathops.h"
#include "me_cmp.h"
#include "mpegutils.h"
#include "profiles.h"
#include "rectangle.h"
#include "thread.h"

#include "decode.h"
#include "hevc.h"
#include "hevc_data.h"
#include "hevc_parse.h"
#include "hevcdecSstar.h"
#include "h2645_parse.h"


#define MI_U32VALUE(data, idx) ((data[idx]<<24)|(data[idx + 1]<<16)|(data[idx + 2]<<8)|(data[idx + 3]))
#define MI_U16VALUE(data, idx) ((data[idx]<<8)|(data[idx + 1]))
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))


#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
    }\
    else\
    {\
        printf("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

#define VDEC_CHN_ID  0

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

    pts_list->pts  = value;
    pts_list->next = NULL;        

    if (!q->first)
    {
        q->first = pts_list;
    }
    else
    {
        q->last->next = pts_list;
    }

    q->last = pts_list;
    q->idx ++;

    return 0;
}

static int pts_queue_get(pts_queue_t *q, int64_t *value)
{
    pts_list_t *pts_head;

    pts_head = q->first;
    if (pts_head)
    {
        q->first = pts_head->next;
        if (!q->first)
        {
            q->last = NULL;
        }
        q->idx --;
        *value = pts_head->pts;
        av_free(pts_head);
    }
    else 
    {
        av_log(NULL, AV_LOG_INFO, "pts queue is null!\n");
    }

    return 0;
}

static int pts_queue_destroy(pts_queue_t *q)
{
    pts_list_t *tmp, *tmp1;

    for (tmp = q->first; tmp; tmp = tmp1)
    {
        tmp1 = tmp->next;
        av_freep(&tmp);
    }
    q->last = NULL;
    q->first = NULL;
    q->idx = 0;

    return 0;
}

pts_queue_t hevc_pts;
#endif

#define  ENABLE_DUMP_ES     1
#define  DUMP_PATH          "/mnt/pstream_hevc.es"

#if ENABLE_DUMP_ES
FILE *hevc_fd = NULL;
char *sshevc_dump_path = NULL;
bool  sshevc_dump_enable = false;
#endif

/**************************************************************************/

#define  VDEC_ES_BUF_MAX        2 * 1024 * 1024
#define  VDEC_ES_BUF_BUSY       2 * 1024 * 768
#define  GET_FRAME_TIME_OUT     3

static int ss_hevc_get_frame(SsHevcContext *ssctx, AVFrame *frame)
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
            av_usleep(10 * 1000);

            MI_VDEC_GetChnStat(0, &stChnStat);
            if (stChnStat.u32LeftStreamBytes > VDEC_ES_BUF_BUSY) {
                if (!time_start) {
                    time_start = av_gettime_relative();
                }
                time_wait = av_gettime_relative();
                if ((time_wait - time_start) / AV_TIME_BASE > GET_FRAME_TIME_OUT) {
                    av_freep(&frame_buf);
                    av_frame_unref(frame);
                    av_log(NULL, AV_LOG_ERROR, "hevc get frame from vdec time out!\n");
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
static int ss_hevc_get_bframe(SsHevcContext *ssctx, AVFrame *frame)
{
    MI_U32 ret, ysize, totalsize;

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
    MI_SYS_SetChnOutputPortDepth(&stVdecChnPort, 2, 5);

    if (MI_SUCCESS == (ret = MI_SYS_ChnOutputPortGetBuf(&stVdecChnPort, &stVdecBufInfo, &stVdecHandle)))
    {
        void *vdec_vir_addr = NULL;
        pstVdecInfo = (mi_vdec_DispFrame_t *)stVdecBufInfo.stMetaData.pVirAddr;

        frame->width  = pstVdecInfo->stFrmInfo.u16Stride;
        frame->height = pstVdecInfo->stFrmInfo.u16Height;
        frame->format = ssctx->format;
        //av_log(NULL, AV_LOG_INFO, "vdec output buf width : %d, height : %d\n", frame->width, frame->height);

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
            //pts_queue_get(&hevc_pts, &frame->pts);

            ysize      = frame->width * frame->height;
            totalsize  = ysize + ysize / 2;
            //使用Map地址与大小必须4K对齐
            //av_log(NULL, AV_LOG_INFO, "phyLumaAddr : 0x%llx, phyChromaAddr : 0x%llx\n", pstVdecInfo->stFrmInfo.phyLumaAddr, pstVdecInfo->stFrmInfo.phyChromaAddr);
            MI_SYS_Mmap(pstVdecInfo->stFrmInfo.phyLumaAddr, ALIGN_UP(totalsize, 4096), &vdec_vir_addr , FALSE);
            //复制图像信息到frame 
            if (frame->buf[0])
            {
                memcpy(frame->data[0], vdec_vir_addr , ysize);
                memcpy(frame->data[1], vdec_vir_addr + ysize, ysize / 2);
            }

            //FILE *fpread = fopen("/mnt/hevc_output.yuv", "a+");
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

// 此函数用于获取不带B帧的图像
static int ss_hevc_get_frame(SsHevcContext *ssctx, AVFrame *frame)
{
    MI_U32 ret, ysize;

    MI_SYS_ChnPort_t  stVdecChnPort;
    MI_SYS_BUF_HANDLE stVdecHandle;
    MI_SYS_BufInfo_t  stVdecBufInfo;

    //av_log(NULL, AV_LOG_INFO, "get in ss_hevc_get_frame!\n");

    stVdecHandle = MI_HANDLE_NULL; 
    memset(&stVdecBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
    memset(&stVdecChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stVdecChnPort.eModId        = E_MI_MODULE_ID_VDEC;
    stVdecChnPort.u32DevId      = 0;
    stVdecChnPort.u32ChnId      = VDEC_CHN_ID;
    stVdecChnPort.u32PortId     = 0;
    MI_SYS_SetChnOutputPortDepth(&stVdecChnPort, 2, 5);

    if (MI_SUCCESS == (ret = MI_SYS_ChnOutputPortGetBuf(&stVdecChnPort, &stVdecBufInfo, &stVdecHandle)))
    {
        frame->width  = stVdecBufInfo.stFrameData.u32Stride[0];
        frame->height = stVdecBufInfo.stFrameData.u16Height;
        frame->format = ssctx->format;

        ret = av_frame_get_buffer(frame, 32);
        if (ret < 0 || !frame->width || !frame->height)
        {
            av_frame_unref(frame);
            ret = AVERROR(ENOMEM);;
        }
        else
        {
            //get frame pts from vdec
            frame->pts = stVdecBufInfo.u64Pts;
            //pts_queue_get(&hevc_pts, &frame->pts);

            // get image data form vdec module 
            ysize  = frame->width * frame->height;
            //av_log(NULL, AV_LOG_INFO, "width : %d, height : %d, ysize : %d, uvsize : %d\n", ssctx->frame->width, ssctx->frame->height, ysize, uvsize);
            // copy valid frame to out frame
            if (frame->buf[0])
            {
                memcpy(frame->data[0], stVdecBufInfo.stFrameData.pVirAddr[0], ysize);
                memcpy(frame->data[1], stVdecBufInfo.stFrameData.pVirAddr[1], ysize / 2);
            }
        }

        if (MI_SUCCESS != MI_SYS_ChnOutputPortPutBuf(stVdecHandle)) {
            av_log(ssctx->avctx, AV_LOG_ERROR, "vdec output put buf error!\n");
        } 
    }
    else 
        ret = AVERROR(EAGAIN);

    //av_log(NULL, AV_LOG_INFO, "exit out ss_hevc_get_frame!\n");

    return ret;
}
#endif

static MI_U32 ss_hevc_vdec_init(AVCodecContext *avctx)
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
        av_log(avctx, AV_LOG_ERROR, "ss_hevc_vdec_init parameter check error!\n");
        return AVERROR_UNKNOWN;
    }

    av_log(avctx, AV_LOG_WARNING, "hevc has b frames : %d\n", avctx->has_b_frames);
    memset(&stVdecInitParam, 0, sizeof(MI_VDEC_InitParam_t));
    if (avctx->has_b_frames > 0)
        stVdecInitParam.bDisableLowLatency = true;
    else
        stVdecInitParam.bDisableLowLatency = false;
    STCHECKRESULT(MI_VDEC_InitDev(&stVdecInitParam));

    if (!(avctx->flags & (1 << 17))) {
        STCHECKRESULT(MI_VDEC_SetOutputPortLayoutMode(E_MI_VDEC_OUTBUF_LAYOUT_LINEAR));
    } else {
        av_log(avctx, AV_LOG_WARNING, "hevc enable vdec tilemode\n");
        STCHECKRESULT(MI_VDEC_SetOutputPortLayoutMode(E_MI_VDEC_OUTBUF_LAYOUT_TILE));
    }

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType    = E_MI_VDEC_CODEC_TYPE_H265;
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

    av_log(NULL, AV_LOG_INFO, "sshevc vdec input width, height : [%d,%d], output width, height : [%d,%d]\n", avctx->width, avctx->height, stWidth, stHeight);

    avctx->flags  &= (0xFFFF0000);
    avctx->flags2 &= (0xFFFF0000);
    if (avctx->has_b_frames > 0)
        avctx->flags |= (1 << 6);
    else
        avctx->flags &= ~(1 << 6);

    av_log(NULL, AV_LOG_INFO, "ss_hevc_vdec_init successful!\n");

    return MI_SUCCESS;
}

static MI_U32 ss_hevc_send_stream(MI_U8 *data, MI_U32 size, int64_t pts, int flag)
{
    MI_VDEC_VideoStream_t stVdecStream;
    MI_S32 s32Ret;
    MI_VDEC_CHN stVdecChn = VDEC_CHN_ID;
    int64_t time_start = 0, time_wait = 0;

    memset(&stVdecStream, 0, sizeof(MI_VDEC_VideoStream_t));
    stVdecStream.pu8Addr      = data;
    stVdecStream.u32Len       = size;
    stVdecStream.u64PTS       = pts;
    stVdecStream.bEndOfFrame  = 1;
    stVdecStream.bEndOfStream = (flag) ? TRUE : FALSE;
    if (stVdecStream.bEndOfStream) {
        av_log(NULL, AV_LOG_INFO, "vdec end of hevc stream flag set!\n");
    }

    //av_log(NULL, AV_LOG_INFO, "size : %d. data : %x,%x,%x,%x,%x,%x,%x,%x.\n", stVdecStream.u32Len, stVdecStream.pu8Addr[0],
    //stVdecStream.pu8Addr[1], stVdecStream.pu8Addr[2], stVdecStream.pu8Addr[3], stVdecStream.pu8Addr[4],
    //stVdecStream.pu8Addr[5], stVdecStream.pu8Addr[6], stVdecStream.pu8Addr[7]);

    #if ENABLE_DUMP_ES
    if (sshevc_dump_enable) {
        fwrite(stVdecStream.pu8Addr, stVdecStream.u32Len, 1, hevc_fd);
    }
    #endif

    s32Ret = MI_VDEC_SendStream(stVdecChn, &stVdecStream, 30);
    while (s32Ret == MI_ERR_VDEC_BUF_FULL) {
        av_usleep(10 * 1000);
        if (!time_start) {
            av_log(NULL, AV_LOG_ERROR, "vdec es buf is full!\n");
            time_start = av_gettime_relative();
        }
        time_wait = av_gettime_relative();
        if ((time_wait - time_start) / AV_TIME_BASE > 2) {
            av_log(NULL, AV_LOG_ERROR, "ss_hevc_send_stream time out!\n");
            return MI_ERR_VDEC_FAILED;
        }
        s32Ret = MI_VDEC_SendStream(stVdecChn, &stVdecStream, 30);
    }

    if (s32Ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "[%s %d]MI_VDEC_SendStream failed!\n", __FUNCTION__, __LINE__);
        return MI_ERR_VDEC_FAILED;
    }
    //av_log(NULL, AV_LOG_INFO, "[%s %d]MI_VDEC_SendStream success!.\n", __FUNCTION__, __LINE__);

    return s32Ret;
}

static int64_t ss_hevc_guess_correct_pts(AVCodecContext *ctx, int64_t reordered_pts, int64_t dts)
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

static int ss_hevc_decode_nalu(SsHevcContext *s, AVPacket *avpkt)
{
    int i, ret, data_idx;
    const uint8_t start_code[4] = {0,0,0,1}; 

    ret = ss_h2645_packet_split(&s->pkt, avpkt->data, avpkt->size, s->avctx, s->is_nalff,
                                 s->nal_length_size, s->avctx->codec_id, 1);
    if (ret < 0)
    {
        av_log(s->avctx, AV_LOG_ERROR,
        "Error splitting the input into NAL units.\n");
        return ret;
    }
    //av_log(NULL, AV_LOG_INFO, "avpkt size : %d, pkt.nb_nals : %d\n", avpkt->size, s->pkt.nb_nals);
    /* decode the NAL units */
    if (s->pkt_buf) {
        av_freep(&s->pkt_buf);
    }
    s->pkt_buf = av_mallocz(avpkt->size + s->max_data_size);
    if (!s->pkt_buf) {
        ff_h2645_packet_uninit(&s->pkt);
        return AVERROR(ENOMEM);
    }

    data_idx = 0;
    for (i = 0; i < s->pkt.nb_nals; i++)
    {
        H2645NAL *nal = &s->pkt.nals[i];
        switch (nal->type)
        {
            case HEVC_NAL_IDR_W_RADL:
            case HEVC_NAL_IDR_N_LP:
            case HEVC_NAL_CRA_NUT:
                if (data_idx == 0 && s->find_header)
                {
                    s->avctx->flags &= ~(1 << 7);
                    memcpy(s->pkt_buf, s->data, s->data_size);
                    data_idx = s->data_size;
                }
            case HEVC_NAL_VPS:
            case HEVC_NAL_SPS:
            case HEVC_NAL_PPS:
            case HEVC_NAL_TRAIL_R:
            case HEVC_NAL_TRAIL_N:
            case HEVC_NAL_TSA_N:
            case HEVC_NAL_TSA_R:
            case HEVC_NAL_STSA_N:
            case HEVC_NAL_STSA_R:
            case HEVC_NAL_BLA_W_LP:
            case HEVC_NAL_BLA_W_RADL:
            case HEVC_NAL_BLA_N_LP:
            case HEVC_NAL_RADL_N:
            case HEVC_NAL_RADL_R:
            case HEVC_NAL_RASL_N:
            case HEVC_NAL_RASL_R:
                //av_log(NULL, AV_LOG_INFO, "packet dts : %lld, pts : %lld.\n", avpkt->dts, avpkt->pts);
                //if (hevc_pts.idx > 10)
                //    pts_queue_get(&hevc_pts, &ret);
                //pts_queue_put(&hevc_pts, avpkt->pts);
                if (!(s->avctx->flags & (1 << 7)) && s->find_header)
                {
                    //add head to nal data
                    memcpy(s->pkt_buf + data_idx, start_code, sizeof(start_code));
                    memcpy(s->pkt_buf + data_idx + sizeof(start_code), nal->data, nal->size);
                    s->pkt_buf[data_idx + 3] = 1;
                    data_idx += (nal->size + sizeof(start_code));
                    //av_log(NULL, AV_LOG_INFO, "extra size : %d, nal size : %d, nal data : %x,%x,%x,%x,%x,%x\n", data_idx, nal->size + 4, extradata_buf[data_idx + 2], 
                    //extradata_buf[data_idx + 3], extradata_buf[data_idx + 4], extradata_buf[data_idx + 5], extradata_buf[data_idx + 6], extradata_buf[data_idx + 7]);
                }
                if (!s->find_header && (nal->type == HEVC_NAL_VPS || nal->type == HEVC_NAL_SPS || nal->type == HEVC_NAL_PPS))
                {
                    if (nal->type == HEVC_NAL_VPS)
                        s->data_size = 0;
                    else if (nal->type == HEVC_NAL_PPS)
                        s->find_header = !s->find_header;
                    memcpy(s->data + s->data_size, start_code, sizeof(start_code));
                    memcpy(s->data + s->data_size + sizeof(start_code), nal->data, nal->size);
                    s->data[s->data_size + 3] = 1;
                    s->data_size += (nal->size + sizeof(start_code));
                }
                break;
            default: break;
        }
    }

    s->pkt_size = data_idx;
    s->pts = ss_hevc_guess_correct_pts(s->avctx, avpkt->pts, avpkt->dts);

    ff_h2645_packet_uninit(&s->pkt);

    return ret;
}

static int ss_hevc_parser_nalu(SsHevcContext *s, const uint8_t *buf, int buf_size,
                                    int is_nalff, int nal_length_size, void *logctx)
{
    int ret, i;
    H2645Packet pkt = { 0 };
    const uint8_t start_code[4] = {0,0,0,1}; 

    ret = ss_h2645_packet_split(&pkt, buf, buf_size, logctx, is_nalff, nal_length_size, AV_CODEC_ID_HEVC, 1);
    if (ret < 0) {
        goto done;
    }

    for (i = 0; i < pkt.nb_nals; i++)
    {
        H2645NAL *nal = &pkt.nals[i];

        switch (nal->type)
        {
            /* ignore everything except parameter sets and VCL NALUs */
            case HEVC_NAL_VPS:
                s->data_size = 0;
            case HEVC_NAL_SPS:
            case HEVC_NAL_PPS:
                memcpy(s->data + s->data_size, start_code, sizeof(start_code));
                memcpy(s->data + s->data_size + sizeof(start_code), nal->data, nal->size);
                s->data[s->data_size + 3] = 1;
                s->data_size += (nal->size + sizeof(start_code));
                av_log(NULL, AV_LOG_INFO, "type of nal : %x. len : %d, data : %x,%x,%x,%x\n", nal->type, nal->size, nal->data[0],nal->data[1],nal->data[2],nal->data[3]);
            break;

            default : break;
        }
    }
    s->find_header = 1;
    ss_hevc_send_stream(s->data, s->data_size, 0, 0);
done:
    ff_h2645_packet_uninit(&pkt);
    return ret;
}

static av_cold int ss_hevc_decode_free(AVCodecContext *avctx)
{
    MI_VDEC_CHN stVdecChn = VDEC_CHN_ID;
    SsHevcContext *s = avctx->priv_data;

    if (s->data) {
        av_freep(&s->data);
    }

    if (s->pkt_buf) {
        av_freep(&s->pkt_buf);
    }

    ff_h2645_packet_uninit(&s->pkt);
    //pts_queue_destroy(&hevc_pts);

    #if ENABLE_DUMP_ES
    if (sshevc_dump_path) {
        av_freep(&sshevc_dump_path);
    }
    if (hevc_fd) {
        fclose(hevc_fd);
    }
    sshevc_dump_enable = false;
    #endif

    STCHECKRESULT(MI_VDEC_StopChn(stVdecChn));
    STCHECKRESULT(MI_VDEC_DestroyChn(stVdecChn));
    STCHECKRESULT(MI_VDEC_DeInitDev());
    av_log(avctx, AV_LOG_WARNING, "ss_hevc_decode_free successful!\n");

    return 0;
}

 
static int ss_hevc_decode_extradata(SsHevcContext *s, uint8_t *buf, int length, int first)
{
    int ret = 0;
    GetByteContext gb;

    bytestream2_init(&gb, buf, length);

    if (length > 3 && (buf[0] || buf[1] || buf[2] > 1)) {
        /* It seems the extradata is encoded as hvcC format.
         * Temporarily, we support configurationVersion==0 until 14496-15 3rd
         * is finalized. When finalized, configurationVersion will be 1 and we
         * can recognize hvcC by checking if avctx->extradata[0]==1 or not. */
        int i, j, num_arrays, nal_len_size;

        s->is_nalff = 1;

        bytestream2_skip(&gb, 21);
        nal_len_size = (bytestream2_get_byte(&gb) & 3) + 1;
        num_arrays   = bytestream2_get_byte(&gb);

        /* nal units in the hvcC always have length coded with 2 bytes,
         * so put a fake nal_length_size = 2 while parsing them */
        s->nal_length_size = 2;

        /* Decode nal units from hvcC. */
        for (i = 0; i < num_arrays; i++) {
            int type = bytestream2_get_byte(&gb) & 0x3f;
            int cnt  = bytestream2_get_be16(&gb);

            for (j = 0; j < cnt; j++) {
                // +2 for the nal size field
                int nalsize = bytestream2_peek_be16(&gb) + 2;
                if (bytestream2_get_bytes_left(&gb) < nalsize) {
                    av_log(s->avctx, AV_LOG_ERROR,
                           "Invalid NAL unit size in extradata.\n");
                    return AVERROR_INVALIDDATA;
                }

                ret = ss_hevc_parser_nalu(s, gb.buffer, nalsize, s->is_nalff, s->nal_length_size, s->avctx);
                if (ret < 0) {
                    av_log(s->avctx, AV_LOG_ERROR,
                           "Decoding nal unit %d %d from hvcC failed\n",
                           type, i);
                    return ret;
                }
                bytestream2_skip(&gb, nalsize);
            }
        }

        /* Now store right nal length size, that will be used to parse
         * all other nals */
        s->nal_length_size = nal_len_size;
    } else {
        s->is_nalff = 0;
        ret = ss_hevc_parser_nalu(s, buf, length, s->is_nalff, s->nal_length_size, s->avctx);
        if (ret < 0)
            return ret;
    }
    if (s->find_header)
        av_log(NULL, AV_LOG_INFO, "sshevc find valid vps, sps, pps nal header!\n");
    return ret;
}

static av_cold int ss_hevc_decode_init(AVCodecContext *avctx)
{
    int ret;
    const AVPixFmtDescriptor *desc;
    SsHevcContext *s = avctx->priv_data;

    s->avctx = avctx;
    s->eos = 0;
    s->time_sec  = 0;
    s->time_wait = 0;

    s->format        = AV_PIX_FMT_NV12;
    s->width         = (avctx->flags  & 0xFFFF);
    s->height        = (avctx->flags2 & 0xFFFF);
    s->find_header   = 0;
    s->data_size     = 0;
    s->max_data_size = 384;
    s->data = av_mallocz(s->max_data_size);
    if (!s->data)
    {
        av_log(avctx, AV_LOG_ERROR, "sshevc malloc extra data error!\n");
        return AVERROR(ENOMEM);
    }

    //av_log(NULL, AV_LOG_INFO, "ss_hevc_decode_init width: %d, height : %d\n", avctx->flags, avctx->flags2);

    if (avctx->pix_fmt != AV_PIX_FMT_NONE)
    {
        desc = av_pix_fmt_desc_get(avctx->pix_fmt);
        av_log(avctx, AV_LOG_INFO, "video prefix format : %s.\n", desc->name);
    } 
    else 
    {
        avctx->pix_fmt  = AV_PIX_FMT_NV12;
    }

    //pts_queue_init(&hevc_pts);

    #if ENABLE_DUMP_ES
    char *env = getenv("SSHEVC_DUMP");
    if (env) {
        if (!strncmp(env, "1", 1)) {
            char *path = getenv("SSHEVC_DUMP_PATH");
            if (path) {
                sshevc_dump_path = av_strdup(path);
                av_log(avctx, AV_LOG_INFO, "sshevc dump path = %s\n", sshevc_dump_path);
                hevc_fd = fopen(sshevc_dump_path, "w+");
                if (hevc_fd) {
                    sshevc_dump_enable = true;
                } else {
                    perror("sshevc open file error");
                }
            } else {
                av_log(avctx, AV_LOG_ERROR, "sshevc dump path isn't set!\n");
            }
        }
    }
    #endif

    // Init vdec module
    if (MI_SUCCESS != (ret = ss_hevc_vdec_init(avctx)))
    {
        av_log(avctx, AV_LOG_ERROR, "ss_hevc_vdec_init failed!\n");
        if (s->data) {
            av_freep(&s->data);
        }
        return AVERROR_UNKNOWN;
    }
    // parse nal data from packet
    if (avctx->extradata_size > 0 && avctx->extradata)
    {
        ret = ss_hevc_decode_extradata(s, avctx->extradata, avctx->extradata_size, 1);
        if (ret < 0 || s->data_size > s->max_data_size)
        {
            av_log(avctx, AV_LOG_ERROR, "ss_hevc_decode_extradata failed!\n");
            ss_hevc_decode_free(avctx);
            return ret;
        }
    }

    av_log(avctx, AV_LOG_INFO, "sshevc_decode_init successful!\n");

    return 0;
}


static int ss_hevc_decode_frame(AVCodecContext *avctx, void *data,
                                      int *got_frame, AVPacket *avpkt)
{
    int ret;
    SsHevcContext   *s = avctx->priv_data; 
    AVCodecInternal *avci = avctx->internal;
    AVFrame *frame = avci->buffer_frame;

    //av_log(NULL, AV_LOG_INFO, "get in ss_hevc_decode_frame!\n");

    if (true == avctx->debug)
    {
        //end of stream and vdec buf is null
        if (!avpkt->size && !avpkt->data)
        {
            av_log(avctx, AV_LOG_INFO, "packet size is 0!!\n");
            return AVERROR_EOF;
        } 
        else
        {
            *got_frame = 0;
            ret = ss_hevc_get_frame(s, frame);

            if (MI_SUCCESS != ret) {
                *got_frame = 1;
                frame->best_effort_timestamp = frame->pts;
            }

            if (s->pkt_buf) {
                if (!(s->avctx->flags & (1 << 7)) && s->find_header) {
                    ret = ss_hevc_send_stream(s->pkt_buf, s->pkt_size, s->pts, 0);
                }
                av_freep(&s->pkt_buf);
            }
            if (ret == MI_ERR_VDEC_FAILED && !(*got_frame)) {
                return MI_ERR_VDEC_FAILED;
            }

            // continue to decode nalu and fill stream data to memory
            ret = ss_hevc_decode_nalu(s, avpkt);
            if (ret < 0)
                av_log(avctx, AV_LOG_ERROR, "ss_hevc_decode_nalu failed!\n");

            if (*got_frame)
                av_assert0(frame->buf[0]);
        }
    }

    //av_log(NULL, AV_LOG_INFO, "exit out ss_hevc_decode_frame!\n");

    if (ret < 0)
        return ret;
    return avpkt->size;
}

static int ss_hevc_receive_frame(AVCodecContext *avctx, AVFrame *frame)
{
    int ret, ret1, ret2, got_frame;

    SsHevcContext        *s = avctx->priv_data; 
    AVCodecInternal   *avci = avctx->internal;
    DecodeSimpleContext *ds = &avci->ds;
    AVPacket         *avpkt = ds->in_pkt;

    if (true == avctx->debug) 
    {
        while (!frame->buf[0])
        {
            got_frame = 0;
            if (MI_SUCCESS == (ret2 = ss_hevc_get_frame(s, frame))) {
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
                            ret1 = ss_hevc_send_stream(s->pkt_buf, s->pkt_size, s->pts, 0);
                        }
                        av_freep(&s->pkt_buf);
                    }

                    if (0 > ss_hevc_decode_nalu(s, avpkt)) {
                        av_log(avctx, AV_LOG_ERROR, "ss_hevc_decode_nalu failed!\n");
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
                            ret1 = ss_hevc_send_stream(s->pkt_buf, s->pkt_size, s->pts, 1);
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

static void ss_hevc_decode_flush(AVCodecContext *avctx)
{
    if (avctx->flags & (1 << 7))
    {
        MI_VDEC_FlushChn(VDEC_CHN_ID);
        //av_log(avctx, AV_LOG_INFO, "ss_hevc_decode_flush done!\n");
    }
}

AVCodec ff_sshevc_decoder = {
    .name                  = "sshevc",
    .long_name             = NULL_IF_CONFIG_SMALL("HEVC (High Efficiency Video Coding)"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_HEVC,
    .priv_data_size        = sizeof(SsHevcContext),
    .init                  = ss_hevc_decode_init,
    .close                 = ss_hevc_decode_free,
    .decode                = ss_hevc_decode_frame,
    .receive_frame         = ss_hevc_receive_frame,
    .flush                 = ss_hevc_decode_flush,
    .capabilities          = /*AV_CODEC_CAP_DR1 |*/ AV_CODEC_CAP_DELAY |
                             AV_CODEC_CAP_SLICE_THREADS | AV_CODEC_CAP_FRAME_THREADS,
    .hw_configs            = (const AVCodecHWConfigInternal*[]) {
                               NULL
                           },

};



