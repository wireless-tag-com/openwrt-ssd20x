#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "user_audio.h"

int main(int argc, char *argv[])
{
    int ret;
    char cmd;

    if (!argv[1]) {
        printf("please input a mp3 file!\n");
        printf("eg: ./Mp3Player file.mp3\n");
        return -1;
    }

    printf("### Mp3Player Start ###\n");

    ret = mp3_codec(argv[1]);
    if (ret != 0) {
        printf("mp3_codec init failed!\n");
        return -1;
    }

    while (1)
    {
        fflush(stdin);
        cmd = getchar();
        if (cmd == 'q') {
            Mp3PlayStopDec();
            break;
        }
        fflush(stdout);
    }

    printf("### Exit Mp3Player ###\n");

    return 0;
}
