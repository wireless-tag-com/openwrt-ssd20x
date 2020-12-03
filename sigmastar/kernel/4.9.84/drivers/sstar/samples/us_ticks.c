/*
* us_ticks.c- Sigmastar
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
#include <sys/types.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>

#define MSYS_IOCTL_MAGIC             'S'

#define IOCTL_MSYS_GET_US_TICKS             _IO(MSYS_IOCTL_MAGIC, 0x31)

int main(int argc, char** argv)
{
	int msys_fd=open("/dev/msys",O_RDWR|O_SYNC);
	 unsigned long long t0=0,t1=0;
	int count=0;
	if(-1==msys_fd)
	{
		printf("can't open /dev/msys\n");
		goto OPEN_FAILED;
	}

	while(1)
	{
		ioctl(msys_fd, IOCTL_MSYS_GET_US_TICKS, &t0);
		if(t0<t1)
		{
			printf("!!!!!!!!!!WRAPPED!!!!!!!!!!!,t0=0x%llX,t1=0x%llX\n",t0,t1);
		}
		printf("t0=0x%llX,t1=0x%llX\n",t0,t1);
		t1=t0;
		count++;
		printf("count=%d\n",count);
		sleep(3);
	}
	return 0;

FAILED:
	if(-1!=msys_fd)close(msys_fd);
	//	if(-1!=kmem_fd)close(kmem_fd);

OPEN_FAILED:
	return -1;
}
