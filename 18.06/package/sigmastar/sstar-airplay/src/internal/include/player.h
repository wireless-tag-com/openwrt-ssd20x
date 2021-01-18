#ifndef __PLAYER_H__
#define __PLAYER_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/avassert.h>
#include <libavutil/log.h>

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_disp.h"
#include "mi_divp.h"
#include "mi_vdec.h"
#include "mi_ao.h"
#include "mi_gfx.h"
#include "mi_vdec_extra.h"

#define  SUCCESS            0
#define  FAIL               1

#define CheckFuncResult(result)\
    if (result != SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
    }\

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

#define SAMPLE_CORRECTION_PERCENT_MAX 10
#define AUDIO_DIFF_AVG_NB   20

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

#define MAX_QUEUE_SIZE      (5 * 1024 * 1024)
#define MIN_VIDEO_FRAMES    30
#define MIN_AUDIO_FRAMES    30

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))


#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)

enum {
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};


typedef struct {
    double pts;                     // 当前帧(待播放)显示时间戳，播放后，当前帧变成上一帧
    double pts_drift;               // 当前帧显示时间戳与当前系统时钟时间的差值
    double last_updated;            // 当前时钟(如视频时钟)最后一次更新时间，也可称当前时钟时间
    double speed;                   // 时钟速度控制，用于控制播放速度
    int serial;                     // 播放序列，所谓播放序列就是一段连续的播放动作，一个seek操作会启动一段新的播放序列
    int paused;                     // 暂停标志
    int *queue_serial;              // 指向packet_serial
}   play_clock_t;

typedef struct {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
}   audio_param_t;

typedef struct packet_queue_t {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;                 // 队列中packet的数量
    int size;                       // 队列所占内存空间大小
    int64_t duration;               // 队列中所有packet总的播放时长
    int abort_request;
    int serial;                     // 播放序列，所谓播放序列就是一段连续的播放动作，一个seek操作会启动一段新的播放序列
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}   packet_queue_t;

/* Common struct for handling all types of decoded data and allocated render buffers. */
typedef struct {
    AVFrame *frame;
    int serial;
    double pts;           /* presentation timestamp for the frame */
    double duration;      /* estimated duration of the frame */
    int64_t pos;                    // frame对应的packet在输入文件中的地址偏移
    int width;
    int height;
    int format;
    AVRational sar;
    int uploaded;
    int flip_v;
    uint8_t *vir_addr;
    unsigned long long phy_addr;
    int buf_size;
}   frame_t;

typedef struct {
    frame_t queue[FRAME_QUEUE_SIZE];
    int rindex;                     // 读索引。待播放时读取此帧进行播放，播放后此帧成为上一帧
    int windex;                     // 写索引
    int size;                       // 总帧数
    int max_size;                   // 队列可存储最大帧数
    int keep_last;
    int rindex_shown;               // 当前是否有帧在显示
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    packet_queue_t *pktq;           // 指向对应的packet_queue
}   frame_queue_t;

typedef struct {
    int audio_only;
    int video_only;
    int rotate_attr;
} player_opts_t;

typedef struct {
    char *filename;
    AVFormatContext *p_fmt_ctx;
    AVInputFormat *p_iformat;
    AVIOContext * p_avio_ctx;
    AVStream *p_audio_stream;
    AVStream *p_video_stream;
    AVCodecContext *p_acodec_ctx;
    AVCodecContext *p_vcodec_ctx;

    int audio_idx;
    int video_idx;
    //sdl_video_t sdl_video;

    play_clock_t audio_clk;                   // 音频时钟
    play_clock_t video_clk;                   // 视频时钟
    play_clock_t extclk;
    double frame_timer;

    packet_queue_t audio_pkt_queue;
    packet_queue_t video_pkt_queue;
    frame_queue_t audio_frm_queue;
    frame_queue_t video_frm_queue;

    struct SwsContext *img_convert_ctx;
    struct SwrContext *audio_swr_ctx;
    AVFrame *p_frm_yuv;

    audio_param_t audio_param_src;
    audio_param_t audio_param_tgt;
    int audio_hw_buf_size;              // SDL音频缓冲区大小(单位字节)
    uint8_t *p_audio_frm;               // 指向待播放的一帧音频数据，指向的数据区将被拷入SDL音频缓冲区。若经过重采样则指向audio_frm_rwr，否则指向frame中的音频
    uint8_t *audio_frm_rwr;             // 音频重采样的输出缓冲区
    unsigned int audio_frm_size;        // 待播放的一帧音频数据(audio_buf指向)的大小
    unsigned int audio_frm_rwr_size;    // 申请到的音频缓冲区audio_frm_rwr的实际尺寸
    int audio_cp_index;                 // 当前音频帧中已拷入SDL音频缓冲区的位置索引(指向第一个待拷贝字节)
    int audio_write_buf_size;           // 当前音频帧中尚未拷入SDL音频缓冲区的数据量，audio_frm_size = audio_cp_index + audio_write_buf_size
    double audio_clock, video_clock;
    double audio_diff_cum;
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    int audio_clock_serial;
    int audio_dev;

    int abort_request;
    int paused;
    int last_paused;
    int read_pause_return;
    int step;
    int eof;
    int audio_complete, video_complete;
    int seek_req;
    int seek_flags;
    int av_sync_type;
    int seek_by_bytes;
    int64_t seek_pos;
    int64_t seek_rel;

    unsigned long long phy_addr;
    uint8_t *vir_addr;
    int buf_size;
    int display_mode;
    int src_width, src_height;  // 保存视频原宽高
    int dst_width, dst_height;  // 保存视频旋转后的宽高
    int out_width, out_height;  // 保存视频最终显示的宽高
    int in_width, in_height;    // 保存外部输入的显示宽高
    int pos_x, pos_y;
    bool start_play, flush;

    pthread_cond_t continue_read_thread;
    pthread_t read_tid;         //demux解复用线程

    pthread_t audioDecode_tid;  //audio解码线程
    pthread_t audioPlay_tid;    //audio播放线程
    pthread_t videoDecode_tid;  //video解码线程
    pthread_t videoPlay_tid;    //video播放线程

    int decoder_type;
    bool demux_status;
    player_opts_t opts;
}   player_stat_t;

int player_running(const char *p_input_file, char *type);
double get_clock(play_clock_t *c);
void set_clock_at(play_clock_t *c, double pts, int serial, double time);
void set_clock(play_clock_t *c, double pts, int serial);
void stream_toggle_pause(player_stat_t *is);
void stream_seek(player_stat_t *is, int64_t pos, int64_t rel, int seek_by_bytes);
double get_master_clock(player_stat_t *is);
void toggle_pause(player_stat_t *is);
void player_seek_file(player_stat_t *is, double time);
void player_seek2_file(player_stat_t *is, double time);
player_stat_t *player_init(const char *p_input_file);
int player_deinit(player_stat_t *is);

#ifdef __cplusplus
}
#endif // __cplusplu


#endif
