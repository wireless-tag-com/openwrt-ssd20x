#ifndef __VIDEO_H__
#define __VIDEO_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include "player.h"

#define HARD_DECODING       1
#define SOFT_DECODING       0

#define DISP_DEV            0
#define DISP_LAYER          0
#define DISP_INPUTPORT      0

#define MAKE_YUYV_VALUE(y,u,v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE MAKE_YUYV_VALUE(29,225,107)

#define ALIGN_UP(x, align)          (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_BACK(x, align)        (((x) / (align)) * (align))


int open_video(player_stat_t *is);
int video_buffer_flush(frame_queue_t *f);
int sstar_video_init(int pos_x, int pos_y, int width, int height);
int sstar_video_deinit(void);
int sstar_display_set(player_stat_t *is);
int sstar_display_unset(player_stat_t *is);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif

