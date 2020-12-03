/*
* riu.c- Sigmastar
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
#include <unistd.h> // for usleep
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <libgen.h>

#include "../../../drivers/sstar/include/mdrv_msys_io.h"
#include "../../../drivers/sstar/include/mdrv_msys_io_st.h"
#include "../../../drivers/sstar/include/mdrv_verchk.h"

int main ( int argc, char **argv )
{
    int msys_fd=0;
    int mem_fd=0;
    unsigned char * map_base=NULL;
    MSYS_MMIO_INFO info;
    char **ptr=NULL;
    unsigned long addr;
    unsigned short content;
    unsigned int content_x32;

    int i = 0;
    FILL_VERCHK_TYPE(info, info.VerChk_Version, info.VerChk_Size, IOCTL_MSYS_VERSION);
    unsigned long bank=0, offset=0;

    msys_fd=open("/dev/msys",O_RDWR|O_SYNC);
    if(-1==msys_fd)
    {
        printf("can't open /dev/msys\n");
        goto OPEN_FAILED;
    }
    mem_fd=open("/dev/mem",O_RDWR|O_SYNC);
    if(-1==mem_fd)
    {
        printf("can't open /dev/mem\n");
        goto FAILED;
    }

    if(0!=ioctl(msys_fd, IOCTL_MSYS_GET_RIU_MAP, &info))
    {
        printf("DMEM request failed!!\n");
        goto FAILED;
    }
    //printf("PHYS=0x%08X, LENGTH=0x%08X\n",(unsigned int)info.addr, (unsigned int)info.size);

    map_base = mmap(NULL, info.size, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, info.addr);
    if (map_base == 0)
    {
        printf("NULL pointer!\n");
        goto FAILED;

    }
    //printf("PHYS=0x%08X, LENGTH=0x%08X\n",(unsigned int)map_base, (unsigned int)info.size);

    if (!strcmp((const char *)basename(argv[0]), "riu_w"))
    {
        if(argc == 4)
        {
            bank=strtol(argv[1],ptr,16);
            offset=strtol(argv[2],ptr,16);
            content=strtol(argv[3],ptr,16);
            printf("BANK:0x%04X 16bit-offset 0x%02X\n", bank, offset);
            addr = (unsigned long)(map_base + bank*0x200 + offset*4);
            *(unsigned short *)addr = content;
            content = *(unsigned short *)addr;
            printf("0x%04X\n", (unsigned int)content);
        }
    }
    else if (!strcmp((const char *)basename(argv[0]), "riu_r"))
    {
        if(argc == 2)
        {
            bank=strtol(argv[1],ptr,16);
            printf("BANK:0x%04X\n", bank);
            for (i=0; i <= 0x7f; i+=1)
            {
                if(i%0x8==0x0) printf("%02X: ", i);
                addr = (unsigned long)(map_base + bank*0x200 + i*4);
                content = *(unsigned short *)addr;
                printf("0x%04X ", (unsigned int)content);
                if(i%0x8==0x7) printf("\n");
            }
        }
        else if(argc == 3)
        {
            bank=strtol(argv[1],ptr,16);
            offset=strtol(argv[2],ptr,16);
            printf("BANK:0x%04X 16bit-offset 0x%02X\n", bank, offset);
            addr = (unsigned long)(map_base + bank*0x200 + offset*4);
            content = *(unsigned short *)addr;
            printf("0x%04X\n", (unsigned int)content);
        }
    }
    else if (!strcmp((const char *)basename(argv[0]), "riux32_w"))
    {
        if(argc == 4)
        {
            bank=strtol(argv[1],ptr,16);
            offset=strtol(argv[2],ptr,16);
            content_x32=strtol(argv[3],ptr,16);
            printf("BANK:0x%04X 16bit-offset 0x%02X\n", bank, offset);
            addr = (unsigned long)(map_base + bank*0x200 + offset*4);
            *(unsigned int *)addr = content_x32;
            content_x32 = *(unsigned int *)addr;
            printf("0x%08X\n", (unsigned int)content_x32);
        }
    }
    else if (!strcmp((const char *)basename(argv[0]), "riux32_r"))
    {
        if(argc == 2)
        {
            bank=strtol(argv[1],ptr,16);
            printf("BANK:0x%04X\n", bank);
            for (i=0; i <= 0x7f; i+=1)
            {
                if(i%0x8==0x0) printf("%02X: ", i);
                addr = (unsigned long)(map_base + bank*0x200 + i*4);
                content_x32 = *(unsigned int *)addr;
                printf("0x%08X ", content_x32);
                if(i%0x8==0x7) printf("\n");
            }
        }
        else if(argc == 3)
        {
            bank=strtol(argv[1],ptr,16);
            offset=strtol(argv[2],ptr,16);
            printf("BANK:0x%04X 16bit-offset 0x%02X\n", bank, offset);
            addr = (unsigned long)(map_base + bank*0x200 + offset*4);
            content_x32 = *(unsigned int *)addr;
            printf("0x%08X\n", content_x32);
        }
    }
    else
    {
        printf("argument error\n");
    }

    if(map_base!= NULL)
        munmap(map_base, 0xff);

    close(msys_fd);
    close(mem_fd);
    return 0; // Indicates that everything vent well.

FAILED:
    if(-1!=msys_fd)close(msys_fd);
    if(-1!=mem_fd)close(mem_fd);


OPEN_FAILED:
    return -1;

}
