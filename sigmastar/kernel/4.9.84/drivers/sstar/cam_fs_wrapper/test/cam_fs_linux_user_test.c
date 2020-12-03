/*
* cam_fs_linux_user_test.c- Sigmastar
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <cam_fs_wrapper.h>

int main(int argc, char *args[])
{
    char buff[1024];
    CamFsFd fd1, fd2;
    int i;
    int baksize = sizeof(args[1]) + 7;
    char bakfile[baksize];

    if (argc != 2)
    {
        printf("Input one file a time!\n");
        exit(1);
    }

    strcpy(bakfile, args[1]);
    strcat(bakfile, ".backup");

    if (CamFsOpen(&fd1, args[1], O_RDONLY, 0644) != CAM_FS_OK)
    {
        printf("Open Error!Check if the file is exist and you have the permission!\n");
        exit(1);
    }
    if (CamFsOpen(&fd2, bakfile, O_RDWR | O_CREAT | O_TRUNC, 755) != CAM_FS_OK)
    {
        printf("Open Error!Check if the file is exist and you have the permission!\n");
        exit(1);
    }

    while ((i = CamFsRead(fd1, buff, sizeof(buff))) > 0)
    {
        CamFsWrite(fd2, buff, i);
    }
    CamFsClose(fd1);
    CamFsClose(fd2);
    printf("Backup done!\n");
    exit(0);
}
