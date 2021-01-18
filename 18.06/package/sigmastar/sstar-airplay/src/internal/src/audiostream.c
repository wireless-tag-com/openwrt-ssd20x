#include "player.h"
#include "packet.h"
#include "frame.h"
#include "audiostream.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <poll.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>


//sdk audio input/outout param
#define AUDIO_INPUT_SAMPRATE        48000
#define AUDIO_INPUT_CHLAYOUT        AV_CH_LAYOUT_MONO
#define AUDIO_INPUT_SAMPFMT         AV_SAMPLE_FMT_S16

#define AUDIO_OUTPUT_SAMPRATE       E_MI_AUDIO_SAMPLE_RATE_48000
#define AUDIO_OUTPUT_CHLAYOUT       E_MI_AUDIO_SOUND_MODE_MONO
#define AUDIO_OUTPUT_SAMPFMT        E_MI_AUDIO_BIT_WIDTH_16

#define MI_AUDIO_SAMPLE_PER_FRAME           1024
#define MI_AUDIO_MAX_SAMPLES_PER_FRAME      2048
#define MI_AUDIO_MAX_FRAME_NUM              6
//#define MI_AO_PCM_BUF_SIZE_BYTE             (MI_AUDIO_SAMPLE_PER_FRAME * MI_AUDIO_MAX_FRAME_NUM * 2 * 4)
#define MI_AO_PCM_BUF_SIZE_BYTE             65536

#define MI_AUDIO_MAX_DATA_SIZE              25000

extern AVPacket a_flush_pkt;
uint64_t channel_layout = 0;

// 从packet_queue中取一个packet，解码生成frame
static int audio_decode_frame(AVCodecContext *p_codec_ctx, packet_queue_t *p_pkt_queue, AVFrame *frame)
{
    int ret;

    while (1)
    {
        AVPacket pkt;

        while (1)
        {
            //if (d->queue->abort_request)
            //    return -1;

            // 3.2 一个音频packet含一至多个音频frame，每次avcodec_receive_frame()返回一个frame，此函数返回。
            // 下次进来此函数，继续获取一个frame，直到avcodec_receive_frame()返回AVERROR(EAGAIN)，
            // 表示解码器需要填入新的音频packet
            ret = avcodec_receive_frame(p_codec_ctx, frame);
            if (ret >= 0)
            {
                // 时基转换，从d->avctx->pkt_timebase时基转换到1/frame->sample_rate时基
                AVRational tb = (AVRational) { 1, frame->sample_rate};
                if (frame->pts != AV_NOPTS_VALUE)
                {
                    //printf("frame pts before convert : %d.\n", frame->pts);
                    frame->pts = av_rescale_q(frame->pts, p_codec_ctx->pkt_timebase, tb);
                }
                else
                {
                    //av_log(NULL, AV_LOG_WARNING, "frame->pts no\n");
                }
                //printf("audio convert before pts : %lld\n", frame->pts);
                return 1;
            }
            else if (ret == AVERROR_EOF)
            {
                av_log(NULL, AV_LOG_INFO, "audio avcodec_receive_frame(): the decoder has been flushed\n");
                avcodec_flush_buffers(p_codec_ctx);
                return 0;
            }
            else if (ret == AVERROR(EAGAIN))
            {
                //av_log(NULL, AV_LOG_INFO, "audio avcodec_receive_frame(): input is not accepted in the current state\n");
                break;
            }
            else
            {
                av_log(NULL, AV_LOG_ERROR, "audio avcodec_receive_frame(): other errors\n");
                continue;
            }
        }
        //printf("read audio pkt start\n");
        // 1. 取出一个packet。使用pkt对应的serial赋值给d->pkt_serial
        //if (packet_queue_get(p_pkt_queue, &pkt, true) < 0)
        if (packet_queue_get(p_pkt_queue, &pkt, true) < 0)
        {
            printf("audio packet_queue_get fail\n");
            return -1;
        }
        //if(pkt.data)
            //printf("read audio pkt end: %x,%x,%x\n",pkt.data[0],pkt.data[1],pkt.data[2]);
        // packet_queue中第一个总是flush_pkt。每次seek操作会插入flush_pkt，更新serial，开启新的播放序列
        if (pkt.data == a_flush_pkt.data)
        {
            // 复位解码器内部状态/刷新内部缓冲区。当seek操作或切换流时应调用此函数。
            avcodec_flush_buffers(p_codec_ctx);
            printf("audio avcodec_flush_buffers!\n");
        }
        else
        {
            // 2. 将packet发送给解码器
            //    发送packet的顺序是按dts递增的顺序，如IPBBPBB
            //    pkt.pos变量可以标识当前packet在视频文件中的地址偏移
            if (avcodec_send_packet(p_codec_ctx, &pkt) == AVERROR(EAGAIN))
            {
                av_log(NULL, AV_LOG_ERROR, "receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
            }
            av_packet_unref(&pkt);
        }
    }
}

// 音频解码线程：从音频packet_queue中取数据，解码后放入音频frame_queue
static void * audio_decode_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    AVFrame *p_frame = av_frame_alloc();
    frame_t *af;

    int got_frame = 0;
    AVRational tb;

    if (p_frame == NULL)
    {
        return NULL;
    }

    while (1)
    {
        if(is->abort_request)
        {
            printf("audio decode thread exit\n");
            break;
        }
        got_frame = audio_decode_frame(is->p_acodec_ctx, &is->audio_pkt_queue, p_frame);
        if (got_frame < 0)
        {
            goto the_end;
        }
        else
        {
            tb = (AVRational) {1, p_frame->sample_rate}; 
            if (!(af = frame_queue_peek_writable(&is->audio_frm_queue)))
                goto the_end;

            af->pts = (p_frame->pts == AV_NOPTS_VALUE) ? NAN : p_frame->pts * av_q2d(tb);
            af->pos = p_frame->pkt_pos;
            //printf("audio convert later pts : %f\n", af->pts);
            //af->serial = is->auddec.pkt_serial;
            // 当前帧包含的(单个声道)采样数/采样率就是当前帧的播放时长
            af->duration = av_q2d((AVRational) { p_frame->nb_samples, p_frame->sample_rate });
            //printf("audio frame pts : %d, time pts : %f, audio duration : %f.\n", p_frame->pts, af->pts, af->duration);
            // 将frame数据拷入af->frame，af->frame指向音频frame队列尾部
            av_frame_move_ref(af->frame, p_frame);
            // 更新音频frame队列大小及写指针
            frame_queue_push(&is->audio_frm_queue);
            av_frame_unref(p_frame);
        }
    }

the_end:
    av_frame_free(&p_frame);
    return NULL;
}

int open_audio_stream(player_stat_t *is)
{
    AVCodecContext *p_codec_ctx;
    AVCodecParameters *p_codec_par = NULL;
    AVCodec* p_codec = NULL;
    int ret;

    // 1. 为音频流构建解码器AVCodecContext

    // 1.1 获取解码器参数AVCodecParameters
    p_codec_par = is->p_audio_stream->codecpar;
    // 1.2 获取解码器
    p_codec = avcodec_find_decoder(p_codec_par->codec_id);
    if (p_codec == NULL)
    {
        av_log(NULL, AV_LOG_ERROR, "Cann't find audio codec!\n");
        return -1;
    }
    printf("open audio codec: %s\n",p_codec->name);

    // 1.3 构建解码器AVCodecContext
    // 1.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    p_codec_ctx = avcodec_alloc_context3(p_codec);
    if (p_codec_ctx == NULL)
    {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3() failed\n");
        return -1;
    }
    // 1.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    ret = avcodec_parameters_to_context(p_codec_ctx, p_codec_par);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context() failed %d\n", ret);
        return -1;
    }
    // 1.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(p_codec_ctx, p_codec, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2() failed %d\n", ret);
        return -1;
    }

    p_codec_ctx->pkt_timebase = is->p_audio_stream->time_base;
    is->p_acodec_ctx = p_codec_ctx;

    // 2. 创建视频解码线程
    ret = pthread_create(&is->audioDecode_tid, NULL, audio_decode_thread, (void *)is);
    if (ret != 0) {
         av_log(NULL, AV_LOG_ERROR, "audio_decode_thread create failed! ret = %d\n", ret);
         is->audioDecode_tid = 0;
        return -1;
    }

    return 0;
}

/* return the wanted number of samples to get better sync if sync_type is video
 * or external master clock */
static int synchronize_audio(player_stat_t *is, int nb_samples)
{
    int wanted_nb_samples = nb_samples;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (is->av_sync_type == AV_SYNC_VIDEO_MASTER && is->video_idx >= 0) {
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;

        diff = get_clock(&is->audio_clk) - get_master_clock(is);

        if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
            is->audio_diff_cum = diff + is->audio_diff_avg_coef * is->audio_diff_cum;
            if (is->audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
                /* not enough measures to have a correct estimate */
                is->audio_diff_avg_count++;
            } else {
                /* estimate the A-V difference */
                avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef);

                if (fabs(avg_diff) >= is->audio_diff_threshold) {
                    wanted_nb_samples = nb_samples + (int)(diff * is->audio_param_src.freq);
                    min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
                }
                av_log(NULL, AV_LOG_TRACE, "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n",
                        diff, avg_diff, wanted_nb_samples - nb_samples,
                        is->audio_clock, is->audio_diff_threshold);
            }
        } else {
            /* too big difference : may be initial PTS errors, so
               reset A-V filter */
            is->audio_diff_avg_count = 0;
            is->audio_diff_cum       = 0;
        }
    }

    return wanted_nb_samples;
}

/*audio resample data for sound card*/ 
static int audio_resample(player_stat_t *is)
{
    int data_size, resampled_data_size;
    int64_t dec_channel_layout;
    av_unused double audio_clock0;
    int wanted_nb_samples;
    frame_t *af = NULL;
    frame_queue_t *f = &is->audio_frm_queue;

replay:
    if (is->paused)
        return -1;

    if (is->seek_flags & (1 << 5))
    {
        frame_queue_flush(&is->audio_frm_queue);
        is->seek_flags &= ~(1 << 5);
    }

    // 若队列头部可读，则由af指向可读帧
    pthread_mutex_lock(&f->mutex);
    while (f->size - f->rindex_shown <= 0 && !f->pktq->abort_request) {
        printf("wait for audio frame\n");
        if (!is->audio_complete && is->eof && is->audio_pkt_queue.nb_packets == 0)
        {
            is->audio_complete = 1;
            //if (is->video_complete && is->audio_complete)
            //    stream_seek(is, is->p_fmt_ctx->start_time, 0, 0);
            printf("audio play completely!\n");
        } 
        pthread_cond_wait(&f->cond, &f->mutex);
    }
    pthread_mutex_unlock(&f->mutex);

    if (f->pktq->abort_request)
        return AVERROR_EOF;

    af = &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
    frame_queue_next(&is->audio_frm_queue);
    if (!af->frame->channel_layout && !af->frame->sample_rate && af->frame->format == -1)
    {
        printf("invalid audio frame layout sample_rate and format!\n");
        goto replay;
    }
    enum AVSampleFormat sample_fmt = (enum AVSampleFormat)(af->frame->format);

    // 根据frame中指定的音频参数获取缓冲区的大小
    data_size = av_samples_get_buffer_size(NULL, af->frame->channels,   // 本行两参数：linesize，声道数
                                           af->frame->nb_samples,       // 本行一参数：本帧中包含的单个声道中的样本数
                                           sample_fmt, 1);              // 本行两参数：采样格式，不对齐

    // 获取声道布局
    dec_channel_layout = (af->frame->channel_layout && af->frame->channels == av_get_channel_layout_nb_channels(af->frame->channel_layout)) ?
                          af->frame->channel_layout : av_get_default_channel_layout(af->frame->channels);
    wanted_nb_samples = synchronize_audio(is, af->frame->nb_samples);
    // is->audio_param_tgt是SDL可接受的音频帧数，是audio_open()中取得的参数
    // 在audio_open()函数中又有“is->audio_src = is->audio_param_tgt”
    // 此处表示：如果frame中的音频参数 == is->audio_src == is->audio_param_tgt，那音频重采样的过程就免了(因此时is->swr_ctr是NULL)
    // 　　　　　否则使用frame(源)和is->audio_param_tgt(目标)中的音频参数来设置is->swr_ctx，并使用frame中的音频参数来赋值is->audio_src
    if (sample_fmt             != is->audio_param_src.fmt            ||
        dec_channel_layout     != is->audio_param_src.channel_layout ||
        af->frame->sample_rate != is->audio_param_src.freq           ||
        (wanted_nb_samples     != af->frame->nb_samples && !is->audio_swr_ctx))
    {
        swr_free(&is->audio_swr_ctx);
        //printf("in layout: %lld,format: %d,samrate: %d\n",dec_channel_layout,af->frame->format,af->frame->sample_rate);
        //printf("out layout: %lld,format: %d,samrate: %d\n",is->audio_param_tgt.channel_layout,is->audio_param_tgt.fmt,is->audio_param_tgt.freq);

        // 使用frame(源)和is->audio_param_tgt(目标)中的音频参数来设置is->audio_swr_ctx
        is->audio_swr_ctx = swr_alloc_set_opts(NULL,
                                               is->audio_param_tgt.channel_layout, is->audio_param_tgt.fmt, is->audio_param_tgt.freq,
                                               dec_channel_layout, sample_fmt, af->frame->sample_rate,
                                               0, NULL);
        if (!is->audio_swr_ctx || swr_init(is->audio_swr_ctx) < 0)
        {
            av_log(NULL, AV_LOG_ERROR,
                "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                af->frame->sample_rate, av_get_sample_fmt_name(sample_fmt), af->frame->channels,
                is->audio_param_tgt.freq, av_get_sample_fmt_name(is->audio_param_tgt.fmt), is->audio_param_tgt.channels);
            swr_free(&is->audio_swr_ctx);
            return -1;
        }
        // 使用frame中的参数更新is->audio_src，第一次更新后后面基本不用执行此if分支了，因为一个音频流中各frame通用参数一样
        is->audio_param_src.channel_layout = dec_channel_layout;
        is->audio_param_src.channels       = af->frame->channels;
        is->audio_param_src.freq           = af->frame->sample_rate;
        is->audio_param_src.fmt            = sample_fmt;
    }

    if (is->audio_swr_ctx)
    {
        // 重采样输入参数1：输入音频样本数是af->frame->nb_samples
        // 重采样输入参数2：输入音频缓冲区
        const uint8_t **in = (const uint8_t **)af->frame->extended_data;
        // 重采样输出参数1：输出音频缓冲区尺寸
        // 重采样输出参数2：输出音频缓冲区
        uint8_t **out = &is->audio_frm_rwr;
        // 重采样输出参数：输出音频样本数(多加了256个样本)
        int out_count = (int64_t)wanted_nb_samples * is->audio_param_tgt.freq / af->frame->sample_rate + 256;
        // 重采样输出参数：输出音频缓冲区尺寸(以字节为单位)
        int out_size = av_samples_get_buffer_size(NULL, is->audio_param_tgt.channels, out_count, is->audio_param_tgt.fmt, 0);
        int len2;
        if (out_size < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
            return -1;
        }

        av_fast_malloc(&is->audio_frm_rwr, &is->audio_frm_rwr_size, out_size);
        if (!is->audio_frm_rwr)
        {
            return AVERROR(ENOMEM);
        }
        //printf("tgt count: %d,channel: %d,size: %d\n",out_count,is->audio_param_tgt.channels,is->audio_frm_rwr_size);
        //printf("in count: %d,channel: %d\n",af->frame->nb_samples,af->frame->channels);

        // 音频重采样：返回值是重采样后得到的音频数据中单个声道的样本数
        len2 = swr_convert(is->audio_swr_ctx, out, out_count, in, af->frame->nb_samples);
        if (len2 < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
            goto fail;
        }

        if (len2 == out_count)
        {
            av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
            if (swr_init(is->audio_swr_ctx) < 0)
                swr_free(&is->audio_swr_ctx);
            goto fail;
        }

        is->p_audio_frm = is->audio_frm_rwr;
        //printf("len2 : %d, out_count : %d, out_size : %d, nb_sample : %d\n", len2, out_count, out_size, wanted_nb_samples);
        // 重采样返回的一帧音频数据大小(以字节为单位)
        resampled_data_size = len2 * is->audio_param_tgt.channels * av_get_bytes_per_sample(is->audio_param_tgt.fmt);
        //printf("resampled_data_size : %d, channels : %d\n", resampled_data_size, is->audio_param_tgt.channels);
    }
    else
    {
        // 未经重采样，则将指针指向frame中的音频数据
        is->p_audio_frm = af->frame->data[0];
        resampled_data_size = data_size;
    }
    //printf("before audio clock: %lf,pts: %lf,nb: %d,samrate: %d\n",is->audio_clock,af->pts,af->frame->nb_samples,af->frame->sample_rate);
    /* update the audio clock with the pts */
    if (!isnan(af->pts))
    {
        is->audio_clock = af->pts + (double)af->frame->nb_samples / af->frame->sample_rate;
    }
    else
    {
        is->audio_clock = NAN;
    }
    //printf("after pts: %.4lf,clock: %.4lf\n",af->pts,is->audio_clock);
    is->audio_clock_serial = af->serial;
    return resampled_data_size;

fail:
    av_freep(&is->audio_frm_rwr);
    return -1;
}

static void * audio_playing_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    int audio_size;
    int pause = 0;
    int last_pause =0;

    while(1)
    {
        if(is->abort_request)
        {
            //printf("audio play thread exit\n");
            break;
        }
        int64_t audio_callback_time = av_gettime_relative();

        audio_size = audio_resample(is);
        if (audio_size == AVERROR_EOF)
            break;
        else if (audio_size == AVERROR_EXIT)
            continue;
        else if (audio_size < 0)
        {
            /* if error, just output silence */
            pause = 1;
            if(pause != last_pause)
            {
                last_pause = pause;
                MI_AO_PauseChn(is->audio_dev, 0);
            }
            is->p_audio_frm = NULL;
            is->audio_frm_size = SDL_AUDIO_MIN_BUFFER_SIZE / is->audio_param_tgt.frame_size * is->audio_param_tgt.frame_size;
        }
        else
        {
            pause = 0;
            if(pause != last_pause)
            {
                last_pause = pause;
                MI_AO_ResumeChn(is->audio_dev, 0);
            }
            is->audio_frm_size = audio_size;
        }

        if (is->p_audio_frm != NULL)
        {
            //memcpy(stream, (uint8_t *)is->p_audio_frm + is->audio_cp_index, len1);
            //printf("save audio len: %d\n",is->audio_frm_size);
            //write(fda, (uint8_t *)is->p_audio_frm + is->audio_cp_index, len1);
            //put audio stream to ss player
            int data_idx = 0, data_len = is->audio_frm_size;

            MI_AUDIO_Frame_t stAoSendFrame;
            MI_S32 s32RetSendStatus = 0;
            MI_AUDIO_DEV AoDevId = is->audio_dev;
            MI_AO_CHN AoChn = 0;
            MI_AO_ChnState_t stState;

            MI_AO_QueryChnStat(AoDevId, AoChn, &stState);
            is->audio_write_buf_size = stState.u32ChnBusyNum + 1024;
            //printf("remain audio_write_buf_size = %d\n", is->audio_write_buf_size);

            do {
                if (data_len <= MI_AUDIO_MAX_DATA_SIZE)
                {
                    stAoSendFrame.u32Len = data_len;
                }
                else
                {
                    stAoSendFrame.u32Len = MI_AUDIO_MAX_DATA_SIZE; 
                }
                stAoSendFrame.apVirAddr[0] = &is->p_audio_frm[data_idx];
                stAoSendFrame.apVirAddr[1] = NULL;  

                data_len -= MI_AUDIO_MAX_DATA_SIZE;
                data_idx += MI_AUDIO_MAX_DATA_SIZE;

                do{
                    s32RetSendStatus = MI_AO_SendFrame(AoDevId, AoChn, &stAoSendFrame, 24);
                }while(s32RetSendStatus == MI_AO_ERR_NOBUF);

                if(s32RetSendStatus != MI_SUCCESS)
                {
                    printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n",s32RetSendStatus);
                }
            }while(data_len > 0);
        }
        // is->audio_write_buf_size是本帧中尚未拷入SDL音频缓冲区的数据量
        //is->audio_write_buf_size = is->audio_frm_size - is->audio_cp_index;
        /* Let's assume the audio driver that is used by SDL has two periods. */
        // 3. 更新时钟
        if (!isnan(is->audio_clock))
        {
            audio_callback_time = av_gettime_relative();
            //printf("audio call back time : %lld\n", audio_callback_time);
            // 更新音频时钟，更新时刻：每次往声卡缓冲区拷入数据后
            // 前面audio_decode_frame中更新的is->audio_clock是以音频帧为单位，所以此处第二个参数要减去未拷贝数据量占用的时间
            set_clock_at(&is->audio_clk,
                is->audio_clock - (double)(is->audio_write_buf_size) / is->audio_param_tgt.bytes_per_sec,
                is->audio_clock_serial,
                audio_callback_time / 1000000.0);
            //printf("audio clk: %lf,curtime: %ld,audio_callback_time: %ld\n",is->audio_clock,is->audio_clock_serial,audio_callback_time);
            //printf("update clk pts: %lf,lud: %lf,dif: %lf\n",is->audio_clk.pts,is->audio_clk.last_updated,is->audio_clk.pts_drift);
            //printf("update pts : %lf, last pts : %lf, data size : %d, channel : %d, bytes : %d\n", is->audio_clk.pts, is->audio_clk.last_updated, is->audio_write_buf_size, is->audio_param_tgt.channels, is->audio_param_tgt.bytes_per_sec);
            is->audio_write_buf_size = 0;
        }
    }

    printf("audio play thread exit\n");
    return NULL;
}

static int open_audio_playing(void *arg)
{
    int ret;
    player_stat_t *is = (player_stat_t *)arg;

    // 2.2 根据SsPlayer音频参数构建音频重采样参数
    // wanted_spec是期望的参数，actual_spec是实际的参数，wanted_spec和auctual_spec都是SDL中的参数。
    // 此处audio_param是FFmpeg中的参数，此参数应保证是SDL播放支持的参数，后面重采样要用到此参数
    // 音频帧解码后得到的frame中的音频格式未必被SDL支持，比如frame可能是planar格式，但SDL2.0并不支持planar格式，
    // 若将解码后的frame直接送入SDL音频缓冲区，声音将无法正常播放。所以需要先将frame重采样(转换格式)为SDL支持的模式，
    // 然后送再写入SDL音频缓冲区
    // 根据输入音频声道数设置AO的声道
    if (!channel_layout) {
        channel_layout = is->p_audio_stream->codecpar->channel_layout;
    }
    sstar_audio_init(is->audio_dev);
    av_log(NULL, AV_LOG_INFO, "audio dev [%d], channel layout [%llu]\n", is->audio_dev, channel_layout);

    is->audio_param_tgt.fmt            = AUDIO_INPUT_SAMPFMT;
    is->audio_param_tgt.freq           = AUDIO_INPUT_SAMPRATE;
    is->audio_param_tgt.channel_layout = channel_layout;
    
    is->audio_param_tgt.channels       = av_get_channel_layout_nb_channels(is->audio_param_tgt.channel_layout);
    is->audio_param_tgt.frame_size     = av_samples_get_buffer_size(NULL, is->audio_param_tgt.channels, 1, is->audio_param_tgt.fmt, 1);
    is->audio_param_tgt.bytes_per_sec  = av_samples_get_buffer_size(NULL, is->audio_param_tgt.channels, is->audio_param_tgt.freq, is->audio_param_tgt.fmt, 1);
    
    if (is->audio_param_tgt.bytes_per_sec <= 0 || is->audio_param_tgt.frame_size <= 0)
    {
        av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
        return -1;
    }
    is->audio_param_src   = is->audio_param_tgt;
    is->audio_hw_buf_size = MI_AO_PCM_BUF_SIZE_BYTE;        // SDL音频缓冲区大小
    is->audio_frm_size    = 0;
    is->audio_cp_index    = 0;
    is->audio_write_buf_size = 0;
    /* init averaging filter */
    is->audio_diff_avg_coef  = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
    is->audio_diff_avg_count = 0;
    /* since we do not have a precise anough audio FIFO fullness,
       we correct audio sync only if larger than this threshold */
    is->audio_diff_threshold = (double)(is->audio_hw_buf_size) / is->audio_param_tgt.bytes_per_sec;

    ret = pthread_create(&is->audioPlay_tid, NULL, audio_playing_thread, is);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "audio_playing_thread create failed!\n");
        is->audioPlay_tid = 0;
        return -1;
    }

    return 0;
}

int open_audio(player_stat_t *is)
{
    int ret;

    if (is && is->audio_idx >= 0) 
    {
        ret = open_audio_stream(is);
        if (ret < 0)
            return ret;

        ret = open_audio_playing(is);
        if (ret < 0)
            return ret;
    }
    
    return 0;
}

int sstar_audio_deinit(int dev_id)
{
    MI_AUDIO_DEV AoDevId = dev_id;
    MI_AO_CHN AoChn = 0;

    /* disable ao channel of */
    CheckFuncResult(MI_AO_DisableChn(AoDevId, AoChn));

    /* disable ao device */
    CheckFuncResult(MI_AO_Disable(AoDevId));
    
    return MI_SUCCESS;
}

int sstar_audio_init(int dev_id)
{
    MI_AUDIO_Attr_t stSetAttr;
    MI_AUDIO_Attr_t stGetAttr;
    MI_AUDIO_DEV AoDevId = dev_id;
    MI_AO_CHN AoChn = 0;

    MI_S32 s32SetVolumeDb;
    MI_S32 s32GetVolumeDb;

    //set Ao Attr struct
    memset(&stSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stSetAttr.u32FrmNum = 6;
    stSetAttr.u32PtNumPerFrm = MI_AUDIO_SAMPLE_PER_FRAME;

    if (channel_layout & AV_CH_LAYOUT_STEREO) {
        stSetAttr.u32ChnCnt = 2;
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
    }
    else if(channel_layout & AV_CH_LAYOUT_MONO)
    {
        stSetAttr.u32ChnCnt = 1;
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    }

    stSetAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_48000;

    /* set ao public attr*/
    CheckFuncResult(MI_AO_SetPubAttr(AoDevId, &stSetAttr));

    /* get ao device*/
    CheckFuncResult(MI_AO_GetPubAttr(AoDevId, &stGetAttr));

    /* enable ao device */
    CheckFuncResult(MI_AO_Enable(AoDevId));

    /* enable ao channel of device*/
    CheckFuncResult(MI_AO_EnableChn(AoDevId, AoChn));

    /* if test AO Volume */
    s32SetVolumeDb = 0;
    CheckFuncResult(MI_AO_SetVolume(AoDevId, s32SetVolumeDb));

    /* get AO volume */
    CheckFuncResult(MI_AO_GetVolume(AoDevId, &s32GetVolumeDb));

    return MI_SUCCESS;
}


