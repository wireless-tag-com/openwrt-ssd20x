#include "audiostream.h"
#include "videostream.h"
#include "demux.h"
#include "player.h"

#define DEVICE_WIDTH        1920
#define DEVICE_HEIGHT       1080

#define LOCAL_X             0
#define LOCAL_Y             0

#define MIN_WIDTH           128
#define MIN_HEIGHT          64

#ifdef SUPPORT_HDMI
#define AUDIO_DEVID         3
#else 
#define AUDIO_DEVID         0
#endif

static player_stat_t *ssplayer = NULL;
static player_opts_t g_opts;

int sstar_player_open(const char *fp, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    int ret, tmp_width, tmp_height;

    if (NULL == ssplayer)
    {
        tmp_width  = DEVICE_WIDTH;
        tmp_height = DEVICE_HEIGHT;

        if (width < MIN_WIDTH || width > tmp_width)
        {
            av_log(NULL, AV_LOG_WARNING, "set width must be in [%d, %d]\n", MIN_WIDTH, tmp_width);
            if (width < MIN_WIDTH)
                width = MIN_WIDTH;
            else if (width > tmp_width)
                width = tmp_width;
        }
        if (height < MIN_HEIGHT || height > tmp_height)
        {
            av_log(NULL, AV_LOG_WARNING, "set height must be in [%d, %d]\n", MIN_HEIGHT, tmp_height);
            if (height < MIN_HEIGHT)
                height = MIN_HEIGHT;
            else if (height > tmp_height)
                height = tmp_height;
        }

        ssplayer = player_init(fp);
        if (ssplayer == NULL)
        {
            av_log(NULL, AV_LOG_ERROR, "player init failed!\n");
            return -1;
        }

        memcpy(&ssplayer->opts, &g_opts, sizeof(player_opts_t));
        ssplayer->display_mode = ssplayer->opts.rotate_attr;
        ssplayer->in_width     = width;
        ssplayer->in_height    = height;
        ssplayer->pos_x        = x;
        ssplayer->pos_y        = y;
        ssplayer->audio_dev    = AUDIO_DEVID;
        av_log(NULL, AV_LOG_INFO, "set out width : %d, height : %d\n", width, height);

        ret = open_demux(ssplayer);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "open_demux failed!\n");
            return ret;
        }
        av_log(NULL, AV_LOG_INFO, "open_demux successful!\n");
 
        ret = open_video(ssplayer);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "open_video failed!\n");
            return ret;
        }
        av_log(NULL, AV_LOG_INFO, "open video successful!\n");

        ret = open_audio(ssplayer);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "open_audio failed!\n");
            return ret;
        }
        av_log(NULL, AV_LOG_INFO, "open audio successful!\n");

        return ret;
    }
    else
    {
        printf("sstar_player_open falied!\n");
        return -1;
    }
}

int sstar_player_close(void)
{
    int ret;

    if (ssplayer != NULL)
    {
        //切换片源时可能闪花一下,提前将DISP的buf还给VDEC
        MI_DISP_ClearInputPortBuffer(DISP_LAYER, DISP_INPUTPORT, TRUE);
        MI_DISP_HideInputPort(DISP_LAYER, DISP_INPUTPORT);
        MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);

        ret = player_deinit(ssplayer);

        if (ssplayer->video_idx >= 0) {
            ret = sstar_display_unset(ssplayer);
        }

        if (ssplayer->audio_idx >= 0) {
            ret = sstar_audio_deinit(ssplayer->audio_dev);
        }

        ssplayer = NULL;
        return ret;
    }
    else
    {
        av_log(NULL, AV_LOG_ERROR, "sstar_player_close failed!\n");
        return -1;
    }
}

//暂停播放
int sstar_player_pause(void)
{
    if (!ssplayer) {
        av_log(NULL, AV_LOG_ERROR, "sstar_player_pause failed!\n");
        return -1;
    }

    if (!ssplayer->paused)
        toggle_pause(ssplayer);

    return 0;
}

//恢复播放
int sstar_player_resume(void)
{
    if (!ssplayer) {
        av_log(NULL, AV_LOG_ERROR, "sstar_player_resume failed!\n");
        return -1;
    }

    if (ssplayer->paused)
        toggle_pause(ssplayer);

    return 0;
}

//从当前位置向前seek到指定时间, 单位秒
int sstar_player_seek(double time)
{
    if (!ssplayer) {
        av_log(NULL, AV_LOG_ERROR, "sstar_player_seek failed!\n");
        return -1;
    }

    player_seek_file(ssplayer, time);

    return 0;
}

//在当前位置向前或向后seek一段时间, 正数向前负数向, 单位秒
int sstar_player_seek2(double time)
{
    if (!ssplayer) {
        av_log(NULL, AV_LOG_ERROR, "sstar_player_seek2 failed!\n");
        return -1;
    }

    player_seek2_file(ssplayer, time);

    return 0;
}

int sstar_player_status(void)
{
    if (!ssplayer) {
        //av_log(NULL, AV_LOG_ERROR, "sstar_player_status failed!\n");
        return -1;
    }

    if (ssplayer->audio_complete && ssplayer->video_complete) {
        return 1;
    } else {
        return 0;
    }
}

//获取视频总时长
int sstar_player_getduration(double *duration)
{
    if (!ssplayer) {
        av_log(NULL, AV_LOG_ERROR, "sstar_player_getduration failed!\n");
        return -1;
    }

    if (ssplayer->p_fmt_ctx->duration != AV_NOPTS_VALUE) {
        *duration = ssplayer->p_fmt_ctx->duration * av_q2d(AV_TIME_BASE_Q);
    } else {
        *duration = NAN;
        av_log(NULL, AV_LOG_WARNING, "get invalid duration!\n");
    }

    return 0;
}

//获取视频当前播放的时间点
int sstar_player_gettime(double *time)
{
    if (!ssplayer) {
        av_log(NULL, AV_LOG_ERROR, "sstar_player_gettime failed!\n");
        return -1;
    }

    if (ssplayer->p_fmt_ctx->start_time != AV_NOPTS_VALUE) {
        if (ssplayer->av_sync_type == AV_SYNC_AUDIO_MASTER && ssplayer->audio_clock != NAN) {
            *time = ssplayer->audio_clock - (ssplayer->p_fmt_ctx->start_time * av_q2d(AV_TIME_BASE_Q));
        } else if (ssplayer->av_sync_type == AV_SYNC_VIDEO_MASTER && ssplayer->video_clock != NAN) {
            *time = ssplayer->video_clock - (ssplayer->p_fmt_ctx->start_time * av_q2d(AV_TIME_BASE_Q));
        } else {
            *time = NAN;
            av_log(NULL, AV_LOG_WARNING, "get invalid current time!\n");
        }
    } else {
        *time = NAN;
        av_log(NULL, AV_LOG_WARNING, "get invalid current time!\n");
    }

    return 0;
}

//设置播放器的一些客制化属性, 包括单独音频模式\单独视频模式\旋转
int sstar_player_setopts(const char *key, const char *value, int flags)
{
    if (!strcmp(key, "audio_only")) {
        sscanf(value, "%d", &g_opts.audio_only);
        printf("player options audio_only = %d\n", g_opts.audio_only);
    }

    if (!strcmp(key, "video_only")) {
        sscanf(value, "%d", &g_opts.video_only);
        printf("player options video_only = %d\n", g_opts.video_only);
    }

    if (!strcmp(key, "rotate")) {
        int rotate_attr;
        sscanf(value, "%d", &rotate_attr);
        if (rotate_attr == 90) {
            g_opts.rotate_attr = E_MI_DISP_ROTATE_90;
        } else if (rotate_attr == 180) {
            g_opts.rotate_attr = E_MI_DISP_ROTATE_180;
        } else if (rotate_attr == 270) {
            g_opts.rotate_attr = E_MI_DISP_ROTATE_270;
        } else {
            g_opts.rotate_attr = E_MI_DISP_ROTATE_NONE;
        }
        printf("player options rotate_attr = %d\n", g_opts.rotate_attr);
    }

    return 0;
}


