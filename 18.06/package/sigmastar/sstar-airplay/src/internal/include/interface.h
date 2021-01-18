#ifndef __SSTAR_API_H__
#define __SSTAR_API_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


int sstar_player_open(const char *fp, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
int sstar_player_close(void);
int sstar_player_pause(void);
int sstar_player_resume(void);
int sstar_player_seek(double time);
int sstar_player_seek2(double time);
int sstar_player_status(void);
int sstar_player_getduration(double *duration);
int sstar_player_gettime(double *time);
int sstar_player_setopts(const char *key, const char *value, int flags);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif

