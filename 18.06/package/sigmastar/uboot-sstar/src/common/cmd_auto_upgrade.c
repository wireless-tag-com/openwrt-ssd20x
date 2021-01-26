/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* #define	DEBUG	*/

#include <common.h>
#include <command.h>
#include <environment.h>
#include <autoboot.h>
#include <malloc.h>



#ifdef CONFIG_AUTO_UPGRADE_SD
int do_auto_upgrade_by_SD(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = -1;
    char cmdbuf[128];
    char *penv = NULL;
    unsigned int file_size = 0;
    unsigned int crc32_env = 0;
    unsigned int crc32_cal = 0;
    char SD_upgrade_cmd[5][64] = {
                "fatsize mmc 0 one.bin",
                "fatload mmc 0 0x21000000 one.bin",
                "crc32 0x21000000 $(filesize)",
                "sf erase 0x0 0x1000000",
                "sf write 0x21000000 0x000000 $(filesize)",
    };

    // clean the filesize in env
    if(NULL != (penv = getenv("filesize")))
    {
        if(run_command("setenv filesize", 0) < 0)
        {
            printf("[%s:%d] run cmd \"setenv filesize\" fail!\n",__func__,__LINE__);
            return -1;
        }
    }

    // get one.bin filesize
    if(run_command((const char*)SD_upgrade_cmd[0], 0) < 0)
    {
        printf("[%s:%d] run cmd \"%s\" fail!\n",__func__,__LINE__, SD_upgrade_cmd[0]);
        return -1;
    }
    else
    {
        if(NULL == (penv = getenv("filesize")))
        {
            printf("[%s:%d] not detect one.bin in SD card!\n",__func__,__LINE__);
            return -1;
        }
    }

    // download the one.bin from SD card
    printf ("\n>> %s \n", SD_upgrade_cmd[1]);
    if(run_command((const char*)SD_upgrade_cmd[1], 0) < 0)
    {
        printf("[%s:%d] run cmd \"%s\" fail!\n",__func__,__LINE__, (const char*)SD_upgrade_cmd[1]);
        return -1;
    }
    else
    {
        penv = NULL;
        if(NULL == (penv = getenv("filesize")))
        {
            printf("[%s:%d] get \"filesize\" from env fail!\n",__func__,__LINE__);
            return -1;
        }
        else
            file_size = simple_strtoul(penv, NULL, 16);
    }

    if (0 >= file_size)
    {
        printf("[%s:%d] not find \"one.bin\" in SD card and return!\n",__func__,__LINE__);
        return -1;
    }

    // calculate the crc32 value of one.bin
    memset(cmdbuf, 0x00, sizeof(cmdbuf));
    snprintf(cmdbuf, sizeof(cmdbuf), "%s 0x%p", (const char*)SD_upgrade_cmd[2], &crc32_cal);
    printf ("\n>> %s \n", cmdbuf);
    if(run_command(cmdbuf, 0) < 0)
        printf("[%s:%d] run cmd \"%s\" fail!\n",__func__,__LINE__, cmdbuf);

    // get the crc32 value from env
    if(NULL != (penv = getenv("bin_crc32")))
        crc32_env = simple_strtoul(penv, NULL, 16);


    if(crc32_env == crc32_cal)
        return 0;


    // do sf probe
    printf ("\n>> %s \n", "sf probe 0");
    if((ret = run_command("sf probe 0", 0)) < 0)
    {
        printf("[%s:%d] run cmd \"sf probe 0\" fail!\n",__func__,__LINE__);
        return -1;
    }

    // erase the flash
    printf ("\n>> %s \n", SD_upgrade_cmd[3]);
    if((0 <= ret) && (run_command((const char*)SD_upgrade_cmd[3], 0) < 0))
        printf("[%s:%d] run cmd \"%s\" fail!\n",__func__,__LINE__, (const char*)SD_upgrade_cmd[3]);

    // write the one.bin to flash
    printf ("\n>> %s \n", SD_upgrade_cmd[4]);
    if((0 <= ret) && (run_command((const char*)SD_upgrade_cmd[4], 0) < 0))
        printf("[%s:%d] run cmd \"%s\" fail!\n",__func__,__LINE__, SD_upgrade_cmd[4]);
    else
    {
        memset(cmdbuf, 0x00, sizeof(cmdbuf));
        snprintf(cmdbuf, sizeof(cmdbuf), "setenv bin_crc32 %x", crc32_cal);
        if(run_command(cmdbuf, 0) < 0)
            printf("[%s:%d] run cmd \"%s\" fail!\n",__func__,__LINE__, cmdbuf);

        run_command("mxp t.load", 0);
        run_command("mxp r.info KERNEL", 0);
        run_command("setenv sf_kernel_start $(sf_part_start)", 0);
        run_command("setenv sf_kernel_size $(sf_part_size)", 0);
        run_command("saveenv", 0);

        printf("[%s:%d] do \"one.bin\" upgrade done\n",__func__,__LINE__);
        run_command("reset", 0);
    }

    return 0;
}

U_BOOT_CMD(
     sdupgrade,  1, 0, do_auto_upgrade_by_SD,
     "do SD card auto upgrade - one.bin\n",
     "    -  \n"
);
#endif ////#ifdef CONFIG_AUTO_UPGRADE_SD
