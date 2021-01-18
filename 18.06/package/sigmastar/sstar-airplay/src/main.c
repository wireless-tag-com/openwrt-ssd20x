#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "interface.h"
#include "platform.h"

#if 0
pthread_t idle_tid;

typedef struct play_info {
    char *url;
    int  width;
    int  height;
    bool exit;
}play_info;

static void* idle_thread(void *arg)
{
    play_info *is = (play_info *)arg;
    static int counter = 0;

    printf("get in idle pthread!\n");
    fflush(stdout);
    while(!is->exit)
    {
        sleep(1);   //阻塞1s
        printf("counter value : %.2ds\r", counter);
        fflush(stdout);
        if (++ counter == 60) {
            sstar_player_close();
        } else if (counter == 70) {
            sstar_player_open(is->url, 0, 0, is->width, is->height);
            counter = 0;
        }

        if (0 < sstar_player_status()) {
            sstar_player_close();
            printf("sstar player done!\n");
        }
    }

    printf("idle pthread exit\n");

    return NULL;
}
#endif

struct player_t {
    char *file;
    int  mode;
    bool exit;
    int  width, height;
    int  flag;
};

char video_list[2][128] = {
    "1066.mkv",
    "MIss_A_Bframes.mp4"
};

static void *sstar_player_thread(void *arg)
{
    struct player_t *is = (struct player_t *)arg;
    int ret, list_num, i = 0;

    list_num = sizeof(video_list) / sizeof(video_list[0]);
    printf("video list item num : %d\n", list_num);

    printf("get in sstar_player_thread!\n");
    while (!is->exit)
    {
        usleep(10 * 1000);

        ret = sstar_player_status();
        if (ret == 1) {
            switch (is->mode)
            {
                case 0:
                break;

                case 1:
                    sstar_player_seek(0);
                break;

                case 2:
                    sstar_player_close();
replay:
                    is->file = video_list[(++ i) % list_num];
                    printf("try to play %s ...\n", is->file);
                    ret = sstar_player_open(is->file, 0, 0, is->width, is->height);
                    if (ret < 0) {
                        sstar_player_close();
                        printf("sstar_player_open failed!\n");
                        goto replay;
                    }
                break;

                default : 
                    printf("invalid mode!\n"); 
                break;
            }
        }
    }

    printf("exit sstar_player_thread!\n");
    return NULL;
}

int main(int argc, char *argv[])
{
    char cmd;
    int ret;
    pthread_t play_tid;
    struct player_t myplay;
    printf("welcome to test ssplayer!\n");

    if (argv[1]) {
        myplay.file = argv[1];
        myplay.flag = 0;
    } else {
        myplay.file = video_list[0];
        myplay.flag = 1;
    }

    myplay.exit   = false;
    myplay.mode   = 0;

    sstar_sys_init();

    #ifdef SUPPORT_HDMI
    sstar_hdmi_init(E_MI_DISP_INTF_HDMI);
    #else
    sstar_panel_init(E_MI_DISP_INTF_LCD);
    #endif

    sstar_getpanel_wh(&myplay.width, &myplay.height);

    printf("Try playing %s ...\n", myplay.file);

    //sstar_player_setopts("video_only", "0", 0);  //设置是否只播视频
    //sstar_player_setopts("rotate", "0", 0);      //设置是否旋转

    ret = sstar_player_open(myplay.file, 0, 0, myplay.width, myplay.height);
    if (ret < 0)
        goto exit;

    pthread_create(&play_tid, NULL, sstar_player_thread, (void *)&myplay);

    while (!myplay.exit)
    {
        fflush(stdin);
        cmd = getchar();
        printf("receive char = %c\n", cmd);
        switch (cmd) 
        {
            case 's':
                sstar_player_open(myplay.file, 0, 0, myplay.width, myplay.height);
            break;

            case 't':
                sstar_player_close();
            break;

            case 'p':
                sstar_player_pause();    // 暂停
            break;

            case 'c':
                sstar_player_resume();   // 恢复播放
            break;

            case 'f':
                sstar_player_seek2(10);  // forward
            break;

            case 'b':
                sstar_player_seek2(-10); // backward
            break;

            case 'm':
                if (myplay.flag) {
                    if (myplay.mode == 0) {
                        printf("change to sigle cyclic mode!\n");
                        myplay.mode = 1;
                    } else if (myplay.mode == 1) {
                        printf("change to list cyclic mode!\n");
                        myplay.mode = 2;
                    } else if (myplay.mode == 2) {
                        printf("change to sigle mode!\n");
                        myplay.mode = 0;
                    }
                } else {
                    printf("can't change mode with one file!\n");
                }
            break;

            case 'd': {
                double duration;
                if (0 == (ret = sstar_player_getduration(&duration))) {
                    printf("get video duration = [%.3lf]!\n", duration);
                }
                break;
            }

            case 'g': {
                double current_time;
                if (0 == (ret = sstar_player_gettime(&current_time))) {
                    printf("get video current time = [%.3lf]!\n", current_time);
                }
                break;
            }

            case 'q': 
                myplay.exit = true;        // 退出
            break;

            default : 
                printf("invalid cmd!\n");
            break;
        }
        fflush(stdout);
    }
    pthread_join(play_tid, NULL);
exit:
    sstar_player_close();

    #ifdef SUPPORT_HDMI
    sstar_hdmi_deinit(E_MI_DISP_INTF_HDMI);
    #else
    sstar_panel_deinit(E_MI_DISP_INTF_LCD);
    #endif

    sstar_sys_deinit();
    return 0;
}


