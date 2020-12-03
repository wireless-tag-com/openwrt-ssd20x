/*
* dmem.c- Sigmastar
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



#include "../../../drivers/sstar/include/mdrv_msys_io.h"
#include "../../../drivers/sstar/include/mdrv_msys_io_st.h"
#include "../../../drivers/sstar/include/mdrv_verchk.h"

//void sleep_ms(int milliseconds) // cross-platform sleep function
//{
//#ifdef WIN32
//    Sleep(milliseconds);
//#elif _POSIX_C_SOURCE >= 199309L
//    struct timespec ts;
//    ts.tv_sec = 0;
//    ts.tv_nsec = milliseconds * 1000000;
//    nanosleep(&ts, NULL);
//#else
//    usleep(milliseconds * 1000);
//#endif
//}




int main ( int argc, char **argv )
{
	int msys_fd=open("/dev/msys",O_RDWR|O_SYNC);
	int mem_fd=open("/dev/mem",O_RDWR|O_SYNC);
//	int kmem_fd=open("/dev/kmem",O_RDWR|O_SYNC);
	unsigned char * map_base;
	MSYS_DMEM_INFO info;
	char **ptr=NULL;
	unsigned long addr;
	unsigned char content;
	int i = 0;
	FILL_VERCHK_TYPE(info, info.VerChk_Version, info.VerChk_Size, IOCTL_MSYS_VERSION);
	if(-1==msys_fd)
	{
		printf("can't open /dev/msys\n");
		goto OPEN_FAILED;
	}

	if(-1==mem_fd)
	{
		printf("can't open /dev/mem\n");
		goto FAILED;
	}
//
//	if(-1==kmem_fd){
//		printf("can't open /dev/kmem\n");
//		goto OPEN_FAILED;
//	}

	if(0==strncmp("-req",argv[1],4))
	{

		if(0==strncmp("0x",argv[2],2))
		{
			info.length=strtol(argv[2],ptr,16);
		}
		else
		{
			info.length=atoi(argv[2]);
		}

		memset(info.name,16,0);
		if(argc>3)
		{
			strncpy(info.name,argv[3],15);
		}


		if(0!=ioctl(msys_fd, IOCTL_MSYS_REQUEST_DMEM, &info))
		{
			printf("DMEM request failed!!\n");
			goto FAILED;
		}
		printf("PHYS=0x%08X,KVIRT=0x%08X, LENGTH=0x%08X\n",(unsigned int)info.phys,(unsigned int)info.kvirt,(unsigned int)info.length);

		map_base = mmap(NULL, info.length, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, info.phys);

		if (map_base == 0)
		{
			printf("NULL pointer!\n");
			goto FAILED;

		}
		else
		{
			printf("Successfull!\n");
		}


		//for (; i < 0xff; ++i)
		//{
		//	addr = (unsigned long)(map_base + i);
		//	content = map_base[i];
		//	printf("address: 0x%lx   content 0x%x\t\t", addr, (unsigned int)content);
        //
		//	map_base[i] = (unsigned char)i;
		//	content = map_base[i];
		//	printf("updated address: 0x%lx   content 0x%x\n", addr, (unsigned int)content);
		//}
		munmap(map_base, 0xff);

	}
	else if(0==strncmp("-rel",argv[1],4))
	{
		if(0==strncmp("0x",argv[2],2))
		{
			info.phys=strtol(argv[2],ptr,16);
		}
		else
		{
			info.phys=atoi(argv[2]);
		}

		memset(info.name,16,0);
		if(argc>3)
		{
			strncpy(info.name,argv[3],15);
		}

		if(0!=ioctl(msys_fd, IOCTL_MSYS_RELEASE_DMEM, &info))
		{
			printf("DMEM release failed!!\n");
			goto FAILED;
		}
	}
	else
	{
		printf("unsupported usage for dmem\n");
		printf("usage:\n");
		printf("  -req  <size>  [name]\n");
		printf("  -rel  <phys>  [name]\n");


	}


	close(msys_fd);
	close(mem_fd);
//	close(kmem_fd);
	return 0; // Indicates that everything vent well.

FAILED:
	if(-1!=msys_fd)close(msys_fd);
	if(-1!=mem_fd)close(mem_fd);
//	if(-1!=kmem_fd)close(kmem_fd);

OPEN_FAILED:
	return -1;

}
