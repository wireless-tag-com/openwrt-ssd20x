/* SigmaStar trade secret */
/*
* cmd_sigauth.c - Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: raul.wang <raul.wang@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include <common.h>
#include <command.h>
#include <environment.h>
#include <malloc.h>
#include "asm/arch/mach/ms_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"

#include <../drivers/mstar/aesdma/drvAESDMA.h>
#include <rtk.h>
#include <ipl.h>

extern void halt(void);

#define SIG_SIZE            256
#define ENV_ROOTFS_SIZE     "rootfs_size"

static u32 calc_checksum(void *buf, U32 size)
{
    U32 i;
    U32 sum = 0;

    for (i = 0; size > i; i += 4)
        sum += *(volatile U32*)(buf + i);
    return sum;
}

int do_sig_auth(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u32 image = 0;
    u32 data = 0;
    u32 image_size = 0;
    u32 key = 0;
    u32 KeyN = 0;
    u32 KeyAES = 0;

    if (argc < 3)
	{
		return CMD_RET_USAGE;
	}

    image = (u32)simple_strtoul(argv[1], NULL, 16);
    key = (u32)simple_strtoul(argv[2], NULL, 16);

    if (0 == key)
    {
        KeyN = 0;
        printf("[U-Boot] SIGMASTAR Key\n\r");
    }
    else if (IPLK_HEADER_CHAR == image_get_ipl_magic(key))
    {
        KeyN = image_get_ipl_cust_keyn_data(key);
        printf("[U-Boot] CUST Key(%08x)\r\n", KeyN);
    }
    else
    {
        printf("[U-Boot] OTP Key\n\r");
        KeyN = key;
    }

    data = image;

    if (RTK_MAKER == image_get_rtk_maker(image))
    {
        image_size = image_get_rtk_size(image);
    }
    else if (image_check_magic((const image_header_t*) image))
    {
        image_size = image_get_data_size((const image_header_t*)image);
        data = image_get_data((const image_header_t*)image);
    }
    else if (getenv(ENV_ROOTFS_SIZE))
    {
        image_size = getenv_ulong(ENV_ROOTFS_SIZE, 16, 0) - SIG_SIZE;
    }

    if (0 >= image_size)
    {
        return CMD_RET_USAGE;
    }

    printf("[U-Boot] Image size = %d bytes\n\r", image_size);

    if (4 == argc && (!strncmp(argv[3], "--aes", strlen("--aes"))))
    {
        image_size = (image_size + 0x0000000f) & ~0x0000000f; //align 16 for aes
    }

    if (runAuthenticate(data, image_size, (U32*)KeyN))
    {
        printf("[U-Boot] Authenticate pass!\n\r");
    }
    else
    {
        printf("[U-Boot] Authenticate failed!\n\r");
        halt();
    }

    if (4 == argc && (!strncmp(argv[3], "--aes", strlen("--aes"))))
    {
        runDecrypt(data, image_size, (U16*)KeyAES);

        if (image_check_magic((const image_header_t*) image))
        {
            if (!image_check_dcrc((const image_header_t *) image))
            {
                printf("[U-Boot] CRC32 check error!\n\r");
                halt();
            }
        }
        else if (image_get_ipl_checksum(image) != calc_checksum((void*)data, (image_size - 16)))
        {
            printf("[U-Boot] Checksum error!\n\r");
            halt();
        }
        printf("[U-Boot] Checksum ok!\n\r");
    }
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	sigauth, CONFIG_SYS_MAXARGS, 1,	do_sig_auth,
	"Only verify digital signature and aes",
	"Usage: sigauth [Image Load Address] [Key Load Address] [--aes]\n\r"
	"if [Key Load Address] is zero, it means using sigmastar's key.\n\r"
	"command with [--aes] means using AES decryption, but don't use AES decrption without [--aes].\n\r"
);

