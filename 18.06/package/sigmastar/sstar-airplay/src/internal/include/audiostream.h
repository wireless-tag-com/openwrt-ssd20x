#ifndef __AUDIO_H__
#define __AUDIO_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include "player.h"

int open_audio(player_stat_t *is);
int sstar_audio_deinit(int dev_id);
int sstar_audio_init(int dev_id);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif
