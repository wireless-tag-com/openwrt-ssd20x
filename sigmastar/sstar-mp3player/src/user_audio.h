#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <stdio.h>

#include "mi_common.h"
#include "mi_sys.h"
#include "mi_ao.h"


#define KEY_SOUND "audio/10.mp3"


#define ExecFunc(result, value)\
    if (result != value)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return -1;\
    }\

extern void Mp3PlayStopDec(void);
extern int mp3_codec(char *mp3_file);
#endif
