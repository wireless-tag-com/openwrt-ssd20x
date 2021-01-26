/*
 * Copyright 2008 - 2009 Windriver, <www.windriver.com>
 * Author: Tom Rix <Tom.Rix@windriver.com>
 *
 * (C) Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <command.h>

#ifdef CONFIG_CMD_FASTBOOT
#include <fastboot.h>
#include <g_dnl.h>

static int do_fastboot_udp(int argc, char *const argv[],
			   uintptr_t buf_addr, size_t buf_size)
{
#ifdef CONFIG_UDP_FUNCTION_FASTBOOT
	int err = net_loop(FASTBOOT);

	if (err < 0) {
		printf("fastboot udp error: %d\n", err);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
#else
	printf("ERR: Fastboot UDP not enabled\n");
	return CMD_RET_FAILURE;
#endif
}

static int do_fastboot_usb(int argc, char *const argv[],
			   uintptr_t buf_addr, size_t buf_size)
{
#ifdef CONFIG_USB_FUNCTION_FASTBOOT
/*
	int controller_index;
	char *usb_controller;
	char *endp;
*/
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_fastboot");
	if (ret)
		return ret;

	while (1) {
		if (g_dnl_detach())
			break;
		if (ctrlc())
			break;
		usb_gadget_handle_interrupts();
	}

	g_dnl_unregister();
	g_dnl_clear_detach();

	return CMD_RET_SUCCESS;

#else
	printf("ERR: Fastboot USB not enabled\n");
	return CMD_RET_FAILURE;
#endif
}

static int do_fastboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	uintptr_t buf_addr = (uintptr_t)NULL;
	size_t buf_size = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	while (argc > 1 && **(argv + 1) == '-') {
		char *arg = *++argv;

		--argc;
		while (*++arg) {
			switch (*arg) {
			case 'l':
				if (--argc <= 0)
					return CMD_RET_USAGE;
				buf_addr = simple_strtoul(*++argv, NULL, 16);
				goto NXTARG;

			case 's':
				if (--argc <= 0)
					return CMD_RET_USAGE;
				buf_size = simple_strtoul(*++argv, NULL, 16);
				goto NXTARG;

			default:
				return CMD_RET_USAGE;
			}
		}
NXTARG:
		;
	}

	/* Handle case when USB controller param is just '-' */
	if (argc == 1) {
		printf("Error: Incorrect USB controller index\n");
		return CMD_RET_USAGE;
	}
#if defined(CONFIG_USB_FASTBOOT_BUF_SIZE) && defined(CONFIG_USB_FASTBOOT_BUF_ADDR) 
	fastboot_init((void *)CONFIG_USB_FASTBOOT_BUF_ADDR, CONFIG_USB_FASTBOOT_BUF_SIZE);
#else
	fastboot_init((void *)buf_addr, buf_size);
#endif
	if (!strcmp(argv[1], "udp"))
		return do_fastboot_udp(argc, argv, buf_addr, buf_size);

	if (!strcmp(argv[1], "usb")) {
		argv++;
		argc--;
	}

	return do_fastboot_usb(argc, argv, buf_addr, buf_size);
}

static char fastboot_help_text[] =
	"[-l addr] [-s size] usb <controller> | udp\n"
	"\taddr - address of buffer used during data transfers ("
	__stringify(CONFIG_FASTBOOT_BUF_ADDR) ")\n"
	"\tsize - size of buffer used during data transfers ("
	__stringify(CONFIG_FASTBOOT_BUF_SIZE) ")"
	;

U_BOOT_CMD(
	fastboot,	10,	0,	do_fastboot,
	"use USB Fastboot protocol",
	fastboot_help_text
);

#endif

#ifdef CONFIG_CMD_USBSTART
#include <net.h>

void cdc_loop(void)
{
    net_init();
    eth_halt();
	eth_set_current();

    if (eth_init(NULL) < 0)
    {
	    eth_halt();
	    return;
    }

    while (!ctrlc())
    {
        eth_rx();
    }

    eth_halt();
}

static int do_usbstart(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    usb_eth_initialize(NULL);
    cdc_loop();
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	usbstart,	1,	0,	do_usbstart,
	"Start USB Device",
	"reset  - reset USBD controller\n"
	"start  - start USBD controller\n"
);

#endif


