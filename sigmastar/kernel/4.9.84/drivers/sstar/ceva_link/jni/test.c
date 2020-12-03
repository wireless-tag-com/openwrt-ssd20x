/*
* test.c- Sigmastar
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
/*
 * test.c
 *
 *  Created on: Aug 16, 2013
 *  Author: Ido Reis <ido.reis@tandemg.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>

#include <ceva_linkdrv-generic.h>

#define FILENAME "/dev/" CEVA_PCIDEV_DEVICE_NAME "0"

#define LOG(fmt, args...)  printf(fmt "\n", ## args)

static void SIGIO_handler(int i) {
	LOG("IO alert, read is possible without blocking :)\n");
}

typedef struct exec_cmd_arg_t {
	const char* name;
	const char* description;
} exec_cmd_arg_t;

typedef struct exec_cmd_t {
	const char *name;
	const char *description;
	const exec_cmd_arg_t *argv;
	int argc;
	int (*exec_func)(int argc, char *argv[]);
} exec_cmd_t;

static int do_stat(int argc, char *argv[]);
static int do_bypass(int argc, char *argv[]);
static int do_write(int argc, char *argv[]);
static int do_read(int argc, char *argv[]);
static int do_geni(int argc, char *argv[]);
static int do_dma_w(int argc, char *argv[]);
static int do_dma_r(int argc, char *argv[]);
static int do_help(int argc, char *argv[]);

const exec_cmd_arg_t write_args[] = {
	{ "offset     ", "bar0 offset to write data" },
	{ "data       ", "32 bit data in hexadeciaml format" },
};

const exec_cmd_arg_t read_args[] = {
	{ "offset     ", "bar0 offset to write data" },
};

const exec_cmd_arg_t dma_w_args[] = {
	{ "offset     ", "offset within the dma buffer" },
	{ "size       ", "number of bytes to write" },
	{ "file name  ", "file content will be written into the dma buffer" },
};

const exec_cmd_arg_t dma_r_args[] = {
	{ "offset     ", "offset within the dma buffer" },
	{ "size       ", "number of bytes to write" },
	{ "file name  ", "dma buffer content will be read into the file" },
};

const exec_cmd_arg_t geni_args[] = {
	{ "interrupt  ", "interrupt to generate" },
};

static exec_cmd_t commands[] = {
	{ "stat", "retrieve and print statistics data from driver", NULL, 0, do_stat },
	{ "bypass", "perform bypass test", NULL, 0, do_bypass },
	{ "read", "read data from bar0", read_args, 1, do_read },
	{ "write", "write data to bar0", write_args, 2, do_write },
	{ "geni", "generate interrupt on device", geni_args, 1, do_geni },
	{ "dmar", "read data from dma buffer to a file", dma_r_args, 3, do_dma_r },
	{ "dmaw", "write data from file to dma buffer", dma_w_args, 3, do_dma_w },
	{ "info", "print this message", NULL, 0, do_help },
};

int fd_dev = -1;
int fd_events = -1;
char *dma = NULL;
unsigned long dma_len = (32*1024*1024);

int main (int argc, char *argv[])
{
	int ret = 0;
	int proc_ext;
	char proc_filename[128];
	int i;

	/* Register SIGIO Handler */
	signal(SIGIO, SIGIO_handler);

	fd_dev = open(FILENAME, O_RDWR);
	if (fd_dev < 0)
	{
		LOG_E("%s-> error open device\n", __FUNCTION__);
		ret = 1;
		goto err_open_dev;
	}

	if (ioctl(fd_dev, IOC_CEVADRV_GET_PID_PROCID, &proc_ext)) {
		LOG_E("%s-> error get device name (pid)\n", __FUNCTION__);
		goto err_open_events;
	}

	sprintf(proc_filename, "/proc/ceva_linkdrv/%d/events", proc_ext);
	fd_events = open(proc_filename, O_RDONLY);
	if (fd_events < 0)
	{
		LOG_E("%s-> error open events\n", __FUNCTION__);
		ret = 2;
		goto err_open_events;
	}

	dma = mmap(NULL, dma_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev, 0);
	if (!dma)
	{
		LOG_E("%s-> error mapping dma\n", __FUNCTION__);
		ret = 3;
		goto err_map;
	}

	argc--;
	argv = &argv[1];

	if (!argc)
	{
		goto print_info;
	}

	for (i = 0; i < sizeof(commands)/sizeof(exec_cmd_t); i++)
	{
		if (!strcmp(commands[i].name, argv[0]))
		{
			if (argc < commands[i].argc)
			{
				LOG_E("wrong number of arguments, expected %d, see 'info'", commands[i].argc);
				goto exit;
			}
			if (!commands[i].exec_func)
			{
				LOG_E("not supported, see 'info'");
				goto exit;
			}
			argc--;
			argv = &argv[1];
			ret = commands[i].exec_func(argc, argv);
			break;
		}
	}
	if (i != sizeof(commands)/sizeof(exec_cmd_t))
	{
		goto exit;
	}

print_info:
	ret = do_help(0, NULL);

exit:
	munmap(dma, dma_len);
err_map:
	if (fd_events != -1)
		close(fd_events);
err_open_events:
	if (fd_dev != -1)
		close(fd_dev);
err_open_dev:
	return ret;
}

static int do_stat(int argc, char *argv[])
{
	union ceva_linkdrv_debug_info debug;

	if (ioctl(fd_dev, IOC_CEVADRV_READ_DEBUG, &debug) != 0)
	{
		LOG_E("%s-> error debug ioctl", __FUNCTION__);
		return -1;
	}
	LOG("debug info retrieved:\n"
		  "  bypass_interrupts_success = %lu\n"
		  "  bypass_interrupts_failed = %lu\n"
		  "  events_recieved = %lu\n"
		  "  fifo_full = %lu\n"
		  "  generated_interrupts = %lu\n"
		  "  generate_interrupt_failures = %lu\n",
		  debug.data.bypass_interrupts_success,
		  debug.data.bypass_interrupts_failed,
		  debug.data.events_recieved,
		  debug.data.fifo_full,
		  debug.data.generated_interrupts,
		  debug.data.generate_interrupt_failures);
	return 0;
}

static void* test_bypass_event_thread(void* fd_dev) {
	int n, ret;

	for (n = 0; n < 3; n++) {
		usleep(1500*1000);
		ret = ioctl((int) fd_dev, IOC_CEVADRV_BYPASS_REQ, 0);
		if (ret != 0)
		{
			LOG_E("%s-> error ioctl\n", __FUNCTION__);
			return (void*) -EPIPE;
		}
		LOG("%s-> bypass command sent\n", __FUNCTION__);
		usleep(1500*1000);
	}
	return 0;
}

static void* test_bypass_read_thread(void* fd_events) {
	int read_bytes;
	unsigned long buf[10];

	while(1) {
		read_bytes = read((int) fd_events, buf, sizeof(buf));
		LOG("%d bytes read, %d events, data: %lx\n", read_bytes, read_bytes / 4, buf[0]);
		if (read_bytes < 0)
		{
			LOG_E("%s-> invalid bytes read from procfs entry\n", __FUNCTION__);
			break;
		}
	}
	return 0;
}

static int do_bypass(int argc, char *argv[])
{
	pthread_t thread1, thread2;
	pthread_attr_t attr1, attr2;

	pthread_attr_init(&attr1);
	pthread_attr_init(&attr2);
	pthread_create(&thread1, &attr1, test_bypass_event_thread, (void*) fd_dev);
	pthread_create(&thread2, &attr2, test_bypass_read_thread, (void*) fd_events);

	pthread_join(thread1, NULL);

	close(fd_dev);
	fd_dev = -1;
	pthread_join(thread2, NULL);
	return 0;
}

static int do_write(int argc, char *argv[])
{
	struct RWBuffer rwbuf;
	unsigned long offset, data;

	sscanf(argv[0], "%lx", &offset);
	sscanf(argv[1], "%lx", &data);

	rwbuf.offset = offset;
	rwbuf.buf = &data;

	LOG("writing to bar0, offset: 0x%0lx, data: 0x%0lx", offset, data);
	if (write(fd_dev, &rwbuf, sizeof(unsigned long)) != sizeof(unsigned long))
	{
		LOG_E("write execution failed");
		return -1;
	}
	LOG("success!");
	return 0;
}

static int do_read(int argc, char *argv[])
{
	struct RWBuffer rwbuf;
	unsigned long offset, data;

	sscanf(argv[0], "%lx", &offset);

	rwbuf.offset = offset;
	rwbuf.buf = &data;

	LOG("reading from bar0, offset: 0x%0lx", offset);
	if (read(fd_dev, &rwbuf, sizeof(unsigned long)) != sizeof(unsigned long))
	{
		LOG_E("read execution failed");
		return -1;
	}
	LOG("success!, data: 0x%0lx", data);
	return 0;
}

static int do_geni(int argc, char *argv[])
{
	unsigned long int_val;

	sscanf(argv[0], "%lx", &int_val);

	if (ioctl(fd_dev, IOC_CEVADRV_GENERATE_INT, int_val) != 0)
	{
		LOG_E("error ioctl IOC_CEVADRV_GENERATE_INT");
		return -1;
	}
	LOG("success!");
	return 0;
}

static int do_dma_r(int argc, char *argv[])
{
	FILE *fd;
	char filename[128];
	unsigned long offset, size;
	int ret;

	sscanf(argv[0], "%lx", &offset);
	sscanf(argv[1], "%lx", &size);
	sscanf(argv[2], "%s", filename);

	fd = fopen(filename, "wb");
	if (!fd) {
		LOG_E("unable to open file");
		return -1;
	}

	ret = fwrite(dma + offset, 1, size, fd);
	if (ret != size)
	{
		LOG_E("fwrite failed, bytes written: %d", ret);
		return -2;
	}
	LOG("success!");

	return 0;
}

static int do_dma_w(int argc, char *argv[])
{
	FILE *fd;
	char filename[128];
	unsigned long offset, size;
	int ret;

	sscanf(argv[0], "%lx", &offset);
	sscanf(argv[1], "%lx", &size);
	sscanf(argv[2], "%s", filename);

	fd = fopen(filename, "rb");
	if (!fd) {
		LOG_E("unable to open file");
		return -1;
	}

	ret = fread(dma + offset, 1, size, fd);
	if (ret != size)
	{
		LOG_E("fread failed, read bytes: %d", ret);
		return -2;
	}
	LOG("success!");

	return 0;
}

static int do_help(int argc, char *argv[])
{
	int i;

	LOG("command\t\tdescription");
	LOG("--------------------------------------------------------------------------------");
	for (i = 0; i < sizeof(commands)/sizeof(exec_cmd_t); i++)
	{
		int j;
		LOG(" %s\t\t%s", commands[i].name, commands[i].description);
		if (commands[i].argc)
		{
//			LOG("  arguments:");
		}
		for (j = 0; j < commands[i].argc; j++)
		{
			LOG("  %s\t%s",
					commands[i].argv[j].name,
					commands[i].argv[j].description);
		}
	}
	return 0;
}
