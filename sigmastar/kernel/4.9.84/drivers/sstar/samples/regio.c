/*
* regio.c- Sigmastar
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <stdio.h>
#include "../../drivers/sstar/include/mdrv_msys_io.h"
#include "../../drivers/sstar/include/mdrv_msys_io_st.h"

int main(int argc, char *argv[])
{
	int msys_fd=-1;
	int mem_fd=-1;
	void *reg_map;
	char **ptr=NULL;
	int rlen=8;
	int roffset;
	MSYS_MMIO_INFO reg_map_info;
	//printf("[mmio]START\n");

	msys_fd = open("/dev/msys", O_RDWR|O_SYNC);
	if (msys_fd == -1)
	{
	    printf("Can't open /dev/msys\n");
	    goto OUT;
	}
	mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
	if (mem_fd == -1)
	{
		printf("Can't open /dev/mem\n");
		goto OUT;
	}

	//printf("devices open success!!\n");

	 if(0!=ioctl(msys_fd, IOCTL_MSYS_GET_RIU_MAP, &reg_map_info))
	{
		printf("IOCTL_MSYS_GET_RIU_MAP failed!!\n");
		  goto OUT;
	}
//	printf("reg_map_info: addr=0x%08X, size=0x%08X\n",(unsigned int)reg_map_info.addr,reg_map_info.size);

	if(((unsigned int)(reg_map=mmap (NULL, reg_map_info.size, PROT_READ|PROT_WRITE, MAP_SHARED , mem_fd, reg_map_info.addr))) == (unsigned int)MAP_FAILED)
	{
		  printf("reg_map failed!!\n");
		  goto OUT;
	}

	sleep(1);
//	printf("The reg_map is ready, addr=0x%08X\n",(unsigned int)reg_map);

	if(0==strncmp("-r",argv[1],4))
	{
		int i=0;
		if(0==strncmp("0x",argv[2],2))
		{
			roffset=strtol(argv[2],ptr,16);
		}
		else
		{
			roffset=atoi(argv[2]);
		}


		if(0==strncmp("0x",argv[3],2))
		{
			rlen=strtol(argv[3],ptr,16);
		}
		else
		{
			rlen=atoi(argv[3]);
		}

		rlen=(rlen<8)?8:rlen;


		int *p=(int *)(reg_map+roffset);
		for(i=0;i<rlen;i+=8){
			printf(
					"%08X:%04X %04X %04X %04X %04X %04X %04X %04X\n",
					(unsigned int)(reg_map_info.addr+roffset+i*sizeof(int)),
					(unsigned short)(*(p+0)),(unsigned short)(*(p+1)),(unsigned short)(*(p+2)),(unsigned short)(*(p+3)),
					(unsigned short)(*(p+4)),(unsigned short)(*(p+5)),(unsigned short)(*(p+6)),(unsigned short)(*(p+7))
				  );

			p+=8;
		}

	}else if(0==strncmp("-w",argv[1],4)){
		printf("you got me!!");
	}
	else{
		printf("unsupported usage for reg\n");
		printf("usage:\n");
		printf("  -r  <address>(32bit reg offset) <reg count>(8 aligned)\n");
		printf("  -w  <address>(32bit reg offset) <value>\n");

	}



    if(0!=munmap(reg_map,reg_map_info.size-1))
    {
        printf("munmap failed!!\n");
        reg_map=NULL;
    }

OUT:
	if(msys_fd!=-1)close(msys_fd);
	if(mem_fd!=-1)close(mem_fd);

//	printf("[regio]END\n");
}
