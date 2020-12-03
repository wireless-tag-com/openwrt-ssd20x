/*
* key_test.c- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
#include <linux/input.h>                                                                                                                                                                                     
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#define INPUT_DEV "/dev/input/event0"

int main(int argc, char * const argv[])
{
    int fd = 0;

    struct input_event event;

    int ret = 0;

    fd = open(INPUT_DEV, O_RDONLY);

    while(1){
        ret = read(fd, &event, sizeof(event));
        if(ret == -1) {
            perror("Failed to read.\n");
            exit(1);
        }

        if(event.type != EV_SYN) {
            printf("type:%d, code:%d, value:%d\n", event.type, event.code, event.value);
        }
    }   

    return 0;
}

