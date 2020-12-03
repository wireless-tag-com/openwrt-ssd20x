/*
* miu_bw.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>
#include <linux/sys_soc.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/clocksource.h>
#include <linux/gpio.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/memory.h>
#include <asm/io.h>
#include <asm/mach/map.h>
#include "gpio.h"
#include "registers.h"
#include "mcm_id.h"
#include "ms_platform.h"
#include "ms_types.h"
#include "_ms_private.h"

#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/buffer_head.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include "cam_os_wrapper.h"

/*=============================================================*/
// Macro definition
/*=============================================================*/

#ifndef MIU_NUM
#define MIU_NUM         (1)
#endif

#define MIU0_CLIENT_NUM sizeof(miu0_clients)/sizeof(miu0_clients[0])
#define MIU1_CLIENT_NUM sizeof(miu1_clients)/sizeof(miu1_clients[0])

/*=============================================================*/
// Structure definition
/*=============================================================*/

struct miu_device {
    struct device dev;
    int index;
};

struct miu_client {
    char* name;
    short bw_client_id;
    short bw_rsvd;
    short bw_enabled;
    short bw_dump_en;
    short bw_filter_en;
    short bw_max;
    short bw_avg;
    short bw_min;
    short effi_avg;
    short effi_min;
    short effi_max;
    short bw_max_div_effi;
    short bw_avg_div_effi;
};

/*=============================================================*/
// Local variable
/*=============================================================*/

static struct miu_client miu0_clients[] =
{
    {"OVERALL   ",0x00,0,1,0,0},
    {"MIIC0     ",0x01,0,0,0,0},
    {"MIIC1     ",0x02,0,0,0,0},
    {"JPE_R     ",0x04,0,0,0,0},
    {"JPE_W     ",0x05,0,0,0,0},
    {"BACH_RW   ",0x06,0,0,0,0},
    {"AESDMA_RW ",0x07,0,0,0,0},
    {"MCU51_RW  ",0x0A,0,0,0,0},
    {"URDMA_RW  ",0x0B,0,0,0,0},
    {"BDMA_RW   ",0x0C,0,0,0,0},
    {"MOVDMA0_RW",0x0D,0,0,0,0},
    {"CMDQ_R    ",0x10,0,0,0,0},
    {"SATA      ",0x11,0,0,0,0},
    {"EMAC      ",0x12,0,0,0,0},
    {"EMAC1     ",0x13,0,0,0,0},
    {"USB20_1   ",0x17,0,0,0,0},
    {"USE20_2   ",0x18,0,0,0,0},
    {"USB20_3   ",0x19,0,0,0,0},
    {"GE        ",0x1A,0,0,0,0},
    {"SDIO30_RW ",0x1F,0,0,0,0},
    {"DIP0_R    ",0x20,0,0,0,0},
    {"DIP0_W    ",0x21,0,0,0,0},
    {"GOP0      ",0x22,0,0,0,0},
    {"GOP1      ",0x23,0,0,0,0},
    {"MOPROT0_Y ",0x24,0,0,0,0},
    {"MOPROT0_C ",0x25,0,0,0,0},
    {"MOPROT1_Y ",0x26,0,0,0,0},
    {"MOPROT1_C ",0x27,0,0,0,0},
    {"MOPS_Y    ",0x28,0,0,0,0},
    {"MOPS_C    ",0x29,0,0,0,0},
    {"MOPG_Y    ",0x2A,0,0,0,0},
    {"MOPG_C    ",0x2B,0,0,0,0},
    {"WAVE511   ",0x2E,0,0,0,0},
    {"CPU       ",0x70,0,0,0,0},
};

#if MIU_NUM > 1
static struct miu_client miu1_clients[] =
{
    {"OVERALL   ",0x00,0,1,0,0},
};
#endif

static struct miu_device miu0;
#if MIU_NUM > 1
static struct miu_device miu1;
#endif

static struct bus_type miu_subsys = {
    .name = "miu",
    .dev_name = "miu",
};

static int gmonitor_interval[MIU_NUM] = {4};//{14};
static int gmonitor_duration[MIU_NUM] = {2000};//{1800};
static int gmonitor_output_kmsg[MIU_NUM] = {1};

static char m_bOutputFilePath[128] = "/mnt/dump_miu_bw.txt";

/*=============================================================*/
// Local function
/*=============================================================*/

static struct file *miu_bw_open_file(char *path, int flag, int mode)
{
    struct file *filp = NULL;
    mm_segment_t oldfs;

    oldfs = get_fs();
    set_fs(get_ds());

    filp = filp_open(path, flag, mode);

    set_fs(oldfs);
    if (IS_ERR(filp)) {
        return NULL;
    }
    return filp;
}

static int miu_bw_write_file(struct file *fp, char *buf, int writelen)
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());
    ret = vfs_write(fp, buf, writelen, &fp->f_pos);

    set_fs(oldfs);
    return ret;
}

static int miu_bw_close_file(struct file *fp)
{
    filp_close(fp, NULL);
    return 0;
}

const char* miu_client_id_to_name(U16 id)
{
    int i = 0;

    for (i = 0; i < MIU0_CLIENT_NUM; i++) {
        if (miu0_clients[i].bw_client_id == id) {
            return miu0_clients[i].name;
        }
    }
    return NULL;
}

static int set_miu_client_enable(struct device *dev, const char *buf, size_t n, int enabled)
{
    long idx = -1;

    if ('0'== (dev->kobj.name[3])) {

        if (kstrtol(buf, 10, &idx) != 0 ||
            idx < 0 ||
            idx >= MIU0_CLIENT_NUM) {
            return -EINVAL;
        }

        if (miu0_clients[idx].bw_rsvd == 0)
            miu0_clients[idx].bw_enabled = enabled;
    }
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3])) {

        if (kstrtol(buf, 10, &idx) != 0 ||
            idx < 0 ||
            idx >= MIU1_CLIENT_NUM) {
            return -EINVAL;
        }

        if (miu1_clients[idx].bw_rsvd == 0)
            miu1_clients[idx].bw_enabled = enabled;
    }
#endif

    return n;
}

static int set_miu_client_dump_enable(struct device *dev, const char *buf, size_t n, int enabled)
{
    long idx = -1;

    if ('0'== (dev->kobj.name[3])) {

        if (kstrtol(buf, 10, &idx) != 0 ||
            idx < 0 ||
            idx >= MIU0_CLIENT_NUM) {
            return -EINVAL;
        }

        if (miu0_clients[idx].bw_rsvd == 0)
            miu0_clients[idx].bw_dump_en = enabled;
    }
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3])) {

        if (kstrtol(buf, 10, &idx) != 0 ||
            idx < 0 ||
            idx >= MIU1_CLIENT_NUM) {
            return -EINVAL;
        }

        if (miu1_clients[idx].bw_rsvd == 0)
            miu1_clients[idx].bw_dump_en = enabled;
    }
#endif

    return n;
}

static int set_miu_client_filter_enable(struct device *dev, const char *buf, size_t n, int enabled)
{
    long idx = -1;

    if ('0'== (dev->kobj.name[3])) {

        if (kstrtol(buf, 10, &idx) != 0 ||
            idx < 0 ||
            idx >= MIU0_CLIENT_NUM) {
            return -EINVAL;
        }

        if (miu0_clients[idx].bw_rsvd == 0)
            miu0_clients[idx].bw_filter_en = enabled;
    }
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3])) {

        if (kstrtol(buf, 10, &idx) != 0 ||
            idx < 0 ||
            idx >= MIU1_CLIENT_NUM) {
            return -EINVAL;
        }

        if (miu1_clients[idx].bw_rsvd == 0)
            miu1_clients[idx].bw_filter_en = enabled;
    }
#endif

    return n;
}

static ssize_t monitor_client_enable_store(struct device *dev,  struct device_attribute *attr, const char *buf, size_t n)
{
    int i;
    char* pt;
    char* opt;

    if (!strncmp(buf, "all", strlen("all")))
    {
        if ('0'== (dev->kobj.name[3])) {
            for (i = 0; i < MIU0_CLIENT_NUM; i++) {
                if (miu0_clients[i].bw_rsvd == 0)
                    miu0_clients[i].bw_enabled = 1;
            }
        }
#if MIU_NUM > 1
        else if ('1'== (dev->kobj.name[3])) {
            for (i = 0; i < MIU1_CLIENT_NUM; i++) {
                if (miu1_clients[i].bw_rsvd == 0)
                    miu1_clients[i].bw_enabled = 1;
            }
        }
#endif

        return n;
    }

    pt = kmalloc(strlen(buf)+1, GFP_KERNEL);
    strcpy(pt, buf);
    while ((opt = strsep(&pt, ";, ")) != NULL)
    {
        set_miu_client_enable(dev, opt, n, 1);
    }
    kfree(pt);

    return n;
}

static ssize_t monitor_client_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i = 0;

    if ('0'== (dev->kobj.name[3]))
    {
        str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Enable(1)/Disable(0)]\n");

        for (i = 0; i < MIU0_CLIENT_NUM; i++)
        {
            //if (miu0_clients[i].bw_enabled && !miu0_clients[i].bw_rsvd)
            {
                str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",(short)i, miu0_clients[i].name, (short)miu0_clients[i].bw_client_id, (char)miu0_clients[i].bw_enabled);
            }
        }
    }
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3]))
    {
        str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Enable(1)/Disable(0)]\n");

        for (i = 0; i < MIU1_CLIENT_NUM; i++)
        {
            //if (miu1_clients[i].bw_enabled && !miu1_clients[i].bw_rsvd)
            {
                str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",(short)i, miu1_clients[i].name, (short)miu1_clients[i].bw_client_id, (char)miu1_clients[i].bw_enabled);
            }
        }
    }
#endif

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t monitor_client_disable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    int i;
    char* pt;
    char* opt;

    if (!strncmp(buf, "all", strlen("all")))
    {
        if ('0'== (dev->kobj.name[3])) {
            for (i = 0; i < MIU0_CLIENT_NUM; i++) {
                if (miu0_clients[i].bw_rsvd == 0)
                    miu0_clients[i].bw_enabled = 0;
            }
        }
#if MIU_NUM > 1
        else if ('1'== (dev->kobj.name[3])) {
            for (i = 0; i < MIU1_CLIENT_NUM; i++) {
                if (miu1_clients[i].bw_rsvd == 0)
                    miu1_clients[i].bw_enabled = 0;
            }
        }
#endif

        return n;
    }

    pt = kmalloc(strlen(buf)+1, GFP_KERNEL);
    strcpy(pt, buf);
    while ((opt = strsep(&pt, ";, ")) != NULL)
    {
        set_miu_client_enable(dev, opt, n, 0);
    }
    kfree(pt);

    return n;
}

static ssize_t monitor_client_disable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i = 0;

    if ('0'== (dev->kobj.name[3]))
    {
        str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Enable(1)/Disable(0)]\n");

        for (i = 0; i < MIU0_CLIENT_NUM; i++)
        {
            str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",(short)i, miu0_clients[i].name, (short)miu0_clients[i].bw_client_id, (char)miu0_clients[i].bw_enabled);
        }
    }
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3]))
    {
        str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Enable(1)/Disable(0)]\n");

        for (i = 0; i < MIU1_CLIENT_NUM; i++)
        {
            str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",(short)i, miu1_clients[i].name, (short)miu1_clients[i].bw_client_id, (char)miu1_clients[i].bw_enabled);
        }
    }
#endif

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t monitor_set_interval_avg_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    u32 input = 0;

    input = simple_strtoul(buf, NULL, 10);

    if ('0'== (dev->kobj.name[3]))
        gmonitor_interval[0] = input;
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3]))
        gmonitor_interval[1] = input;
#endif

    return count;
}

static ssize_t monitor_set_interval_avg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    if ('0'== (dev->kobj.name[3]))
        return sprintf(buf, "%d\n", gmonitor_interval[0]);
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3]))
        return sprintf(buf, "%d\n", gmonitor_interval[1]);
#endif
    else
        return 0;
}

static ssize_t monitor_set_counts_avg_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    u32 input = 0;

    input = simple_strtoul(buf, NULL, 10);

    if ('0'== (dev->kobj.name[3]))
        gmonitor_duration[0] = input;
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3]))
        gmonitor_duration[1] = input;
#endif

    return count;
}

static ssize_t monitor_set_counts_avg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    if ('0'== (dev->kobj.name[3]))
        return sprintf(buf, "%d\n", gmonitor_duration[0]);
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3]))
        return sprintf(buf, "%d\n", gmonitor_duration[1]);
#endif
    else
        return 0;
}

static ssize_t monitor_client_dump_enable_store(struct device *dev,  struct device_attribute *attr, const char *buf, size_t n)
{
    int i;
    char* pt;
    char* opt;

    if (!strncmp(buf, "all", strlen("all")))
    {
        if ('0'== (dev->kobj.name[3])) {
            for (i = 0; i < MIU0_CLIENT_NUM; i++) {
                if (miu0_clients[i].bw_rsvd == 0)
                    miu0_clients[i].bw_dump_en = 1;
            }
        }
#if MIU_NUM > 1
        else if ('1'== (dev->kobj.name[3])) {
            for (i = 0; i < MIU1_CLIENT_NUM; i++) {
                if (miu1_clients[i].bw_rsvd == 0)
                    miu1_clients[i].bw_dump_en = 1;
            }
        }
#endif

        return n;
    }

    pt = kmalloc(strlen(buf)+1, GFP_KERNEL);
    strcpy(pt, buf);
    while ((opt = strsep(&pt, ";, ")) != NULL)
    {
        set_miu_client_dump_enable(dev, opt, n, 1);
    }
    kfree(pt);

    return n;
}

static ssize_t monitor_client_dump_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i = 0;

    if ('0'== (dev->kobj.name[3]))
    {
        str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Dump Enable(1)/Disable(0)]\n");

        for (i = 0; i < MIU0_CLIENT_NUM; i++)
        {
            //if (miu0_clients[i].bw_enabled && !miu0_clients[i].bw_rsvd)
            {
                str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",(short)i, miu0_clients[i].name, (short)miu0_clients[i].bw_client_id, (char)miu0_clients[i].bw_dump_en);
            }
        }
    }
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3]))
    {
        str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Dump Enable(1)/Disable(0)]\n");

        for (i = 0; i < MIU1_CLIENT_NUM; i++)
        {
            //if (miu1_clients[i].bw_enabled && !miu1_clients[i].bw_rsvd)
            {
                str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",(short)i, miu1_clients[i].name, (short)miu1_clients[i].bw_client_id, (char)miu1_clients[i].bw_dump_en);
            }
        }
    }
#endif

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t monitor_filter_abnormal_value_store(struct device *dev,  struct device_attribute *attr, const char *buf, size_t n)
{
    int i;
    char* pt;
    char* opt;

    if (!strncmp(buf, "all", strlen("all")))
    {
        if ('0'== (dev->kobj.name[3])) {
            for (i = 0; i < MIU0_CLIENT_NUM; i++) {
                if (miu0_clients[i].bw_rsvd == 0)
                    miu0_clients[i].bw_filter_en = 1;
            }
        }
#if MIU_NUM > 1
        else if ('1'== (dev->kobj.name[3])) {
            for (i = 0; i < MIU1_CLIENT_NUM; i++) {
                if (miu1_clients[i].bw_rsvd == 0)
                    miu1_clients[i].bw_filter_en = 1;
            }
        }
#endif

        return n;
    }

    pt = kmalloc(strlen(buf)+1, GFP_KERNEL);
    strcpy(pt, buf);
    while ((opt = strsep(&pt, ";, ")) != NULL)
    {
        set_miu_client_filter_enable(dev, opt, n, 1);
    }
    kfree(pt);

    return n;
}

static ssize_t monitor_filter_abnormal_value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i = 0;

    if ('0'== (dev->kobj.name[3]))
    {
        str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Filter Enable(1)/Disable(0)]\n");

        for (i = 0; i < MIU0_CLIENT_NUM; i++)
        {
            //if (miu0_clients[i].bw_enabled && !miu0_clients[i].bw_rsvd)
            {
                str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",(short)i, miu0_clients[i].name, (short)miu0_clients[i].bw_client_id, (char)miu0_clients[i].bw_filter_en);
            }
        }
    }
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3]))
    {
        str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Filter Enable(1)/Disable(0)]\n");

        for (i = 0; i < MIU1_CLIENT_NUM; i++)
        {
            //if (miu1_clients[i].bw_enabled && !miu1_clients[i].bw_rsvd)
            {
                str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",(short)i, miu1_clients[i].name, (short)miu1_clients[i].bw_client_id, (char)miu1_clients[i].bw_filter_en);
            }
        }
    }
#endif

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t measure_all_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    u32 input = 0;

    input = simple_strtoul(buf, NULL, 10);

    if ('0'== (dev->kobj.name[3]))
        gmonitor_output_kmsg[0] = input;
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3]))
        gmonitor_output_kmsg[1] = input;
#endif

    return count;
}

#define DUMP_FILE_TEMP_BUF_SZ (PAGE_SIZE*10)
static ssize_t measure_all_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i = 0, temp_loop_time = 0;
    int id;
    int iMiuClientNum = 0;
    int iMonitorInterval = 0;
    int iMonitorDuration = 0;
    int iMonitorOutputKmsg = 1;
    int iMonitorDumpToFile = 0;
    int iMiuBankAddr = 0;
    struct miu_client *pstMiuClient = NULL;

    short temp_val = 0;
    unsigned long total_temp;
    unsigned long deadline;

    char* u8Buf = NULL;
    char* u8Ptr = NULL;
    char* u8PtrEnd = NULL;
    struct file *pstOutFd = NULL;

    if ('0'== (dev->kobj.name[3])) {
        iMiuClientNum               = MIU0_CLIENT_NUM;
        iMonitorInterval            = gmonitor_interval[0];
        iMonitorDuration            = gmonitor_duration[0];
        iMonitorOutputKmsg          = gmonitor_output_kmsg[0];
        iMiuBankAddr                = BASE_REG_MIU_PA;
        pstMiuClient                = &miu0_clients[0];
    }
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3])) {
        iMiuClientNum               = MIU1_CLIENT_NUM;
        iMonitorInterval            = gmonitor_interval[1];
        iMonitorDuration            = gmonitor_duration[1];
        iMonitorOutputKmsg          = gmonitor_output_kmsg[1];
        iMiuBankAddr                = BASE_REG_MIU1_PA;
        pstMiuClient                = &miu1_clients[0];
    }
#endif
    else {
        return 0;
    }

    for (i = 0; i < iMiuClientNum; i++) {
        (pstMiuClient + i)->effi_min = 0x3FF;
        (pstMiuClient + i)->effi_avg = 0;
        (pstMiuClient + i)->bw_avg = 0;
        (pstMiuClient + i)->bw_max = 0;

        if ((pstMiuClient + i)->bw_dump_en) {
            iMonitorDumpToFile = 1;
        }
    }

    if (iMonitorDumpToFile) {
        u8Buf = kmalloc(DUMP_FILE_TEMP_BUF_SZ, GFP_KERNEL);
        u8Ptr    = u8Buf;
        u8PtrEnd = u8Buf + (DUMP_FILE_TEMP_BUF_SZ);

        pstOutFd = miu_bw_open_file(m_bOutputFilePath, O_WRONLY | O_CREAT, 0644);
    }

    if (iMonitorOutputKmsg) {
        printk("Num:client\tEFFI\tBWavg\tBWmax\tBWavg/E\tBWmax/E\n");
        printk("---------------------------------------------------------\n");
    }
    else {
        str += scnprintf(str, end - str, "Num:client\tEFFI\tBWavg\tBWmax\tBWavg/E\tBWmax/E\n");
        str += scnprintf(str, end - str, "---------------------------------------------------------\n");
    }

    for (i = 0; i < iMiuClientNum; i++)
    {
        if ((pstMiuClient + i)->bw_enabled && (pstMiuClient + i)->bw_rsvd == 0)
        {
            unsigned long diff = 0;
            short last;

            total_temp     = 0;
            temp_loop_time = 0;

            id = (pstMiuClient + i)->bw_client_id;
            OUTREG16((iMiuBankAddr+REG_ID_15), (id & 0x40) ? 0x80 : 0x00);
            id = id & 0x3F;
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x30)); // reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0000);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x35)); // set to read efficiency

            deadline = jiffies + iMonitorDuration*HZ/1000;

            do {
                if (iMonitorInterval > 10)
                    msleep(iMonitorInterval);
                else
                    mdelay(iMonitorInterval);

                temp_val = INREG16((iMiuBankAddr+REG_ID_0E));

                if ((pstMiuClient + i)->bw_filter_en) {
                    if (temp_val) {
                        total_temp += temp_val;
                    }
                }
                else {
                    total_temp += temp_val;
                }

                if (temp_loop_time) {
                    diff += (temp_val > last) ? (temp_val - last) : (last - temp_val);
                }
                last = temp_val;

                if ((pstMiuClient + i)->bw_dump_en) {
                    if (temp_loop_time == 0) {
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\n----------------------------------------------------------------\n");
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "ClientId[%d] Name[%s] Efficiency\n", (short)i, (pstMiuClient + i)->name);
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\t0x00\t0x01\t0x02\t0x03\t0x04\t0x05\t0x06\t0x07\n");
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "----------------------------------------------------------------\n");
                    }

                    u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\t%2d.%02d%%",
                                       temp_val*100/1024,
                                       (temp_val*10000/1024)%100);
                    if ((temp_loop_time+1) % 0x8 == 0) {
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\n");
                    }
                }

                if ((pstMiuClient + i)->effi_min > temp_val) {
                    (pstMiuClient + i)->effi_min = temp_val;
                }

                if ((pstMiuClient + i)->bw_filter_en) {
                    if (temp_val) {
                        temp_loop_time++;
                    }
                }
                else {
                    temp_loop_time++;
                }

            } while (!time_after_eq(jiffies, deadline));

            OUTREG16((iMiuBankAddr+REG_ID_0D), 0) ; // reset all

            (pstMiuClient + i)->effi_avg = total_temp / temp_loop_time;

            total_temp = 0;
            temp_loop_time = 0;

            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x40)); // reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0000);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x41)); // set to read bandwidth

            deadline = jiffies + iMonitorDuration*HZ/1000;

            do {
                if (iMonitorInterval > 10)
                    msleep(iMonitorInterval);
                else
                    mdelay(iMonitorInterval);

                temp_val = INREG16((iMiuBankAddr+REG_ID_0E));

                if ((pstMiuClient + i)->bw_filter_en) {
                    if (temp_val) {
                        total_temp += temp_val;
                    }
                }
                else {
                    total_temp += temp_val;
                }

                if (iMonitorDumpToFile && (pstMiuClient + i)->bw_dump_en) {
                    if (temp_loop_time == 0) {
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\n----------------------------------------------------------------\n");
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "ClientId[%d] Name[%s] BandWidth\n", (short)i, (pstMiuClient + i)->name);
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\t0x00\t0x01\t0x02\t0x03\t0x04\t0x05\t0x06\t0x07\n");
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "----------------------------------------------------------------\n");
                    }

                    u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\t%2d.%02d%%",
                                       temp_val*100/1024,
                                       (temp_val*10000/1024)%100);
                    if ((temp_loop_time+1) % 0x8 == 0) {
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\n");
                    }
                }

                if ((pstMiuClient + i)->bw_max < temp_val) {
                    (pstMiuClient + i)->bw_max = temp_val;
                }

                if ((pstMiuClient + i)->bw_filter_en) {
                    if (temp_val) {
                        temp_loop_time++;
                    }
                }
                else {
                    temp_loop_time++;
                }

            } while (!time_after_eq(jiffies, deadline));

            OUTREG16((iMiuBankAddr+REG_ID_0D), 0) ; // reset all

            (pstMiuClient + i)->bw_avg = total_temp / temp_loop_time;

            // client effiency never changes and total BW is zero
            if ((diff == 0) && (total_temp == 0)) {
                (pstMiuClient + i)->effi_avg = 0x3FF;
            }

            if (iMonitorOutputKmsg)
            {
                printk("%d:%s\t%2d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\n",
                (short)(pstMiuClient + i)->bw_client_id,
                (pstMiuClient + i)->name,
                (pstMiuClient + i)->effi_avg*100/1024,
                ((pstMiuClient + i)->effi_avg*10000/1024)%100,
                (pstMiuClient + i)->bw_avg*100/1024,
                ((pstMiuClient + i)->bw_avg*10000/1024)%100,
                (pstMiuClient + i)->bw_max*100/1024,
                ((pstMiuClient + i)->bw_max*10000/1024)%100,
                (pstMiuClient + i)->bw_avg*100/(pstMiuClient + i)->effi_avg,
                ((pstMiuClient + i)->bw_avg*10000/(pstMiuClient + i)->effi_avg)%100,
                (pstMiuClient + i)->bw_max*100/(pstMiuClient + i)->effi_avg,
                ((pstMiuClient + i)->bw_max*10000/(pstMiuClient + i)->effi_avg)%100);
            }
            else
            {
                str +=  scnprintf(str, end - str, "%d:%s\t%2d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\n",
                (short)i,
                (pstMiuClient + i)->name,
                (pstMiuClient + i)->effi_avg*100/1024,
                ((pstMiuClient + i)->effi_avg*10000/1024)%100,
                (pstMiuClient + i)->bw_avg*100/1024,
                ((pstMiuClient + i)->bw_avg*10000/1024)%100,
                (pstMiuClient + i)->bw_max*100/1024,
                ((pstMiuClient + i)->bw_max*10000/1024)%100,
                (pstMiuClient + i)->bw_avg*100/(pstMiuClient + i)->effi_avg,
                ((pstMiuClient + i)->bw_avg*10000/(pstMiuClient + i)->effi_avg)%100,
                (pstMiuClient + i)->bw_max*100/(pstMiuClient + i)->effi_avg,
                ((pstMiuClient + i)->bw_max*10000/(pstMiuClient + i)->effi_avg)%100);
            }
        }
    }

    if (iMonitorDumpToFile) {
        if (pstOutFd) {
            miu_bw_write_file(pstOutFd, u8Buf, u8Ptr - u8Buf);
            miu_bw_close_file(pstOutFd);
        }

        kfree(u8Buf);
    }

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t measure_all_hw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i = 0, temp_loop_time = 0;
    int id;
    int iMiuClientNum = 0;
    int iMonitorInterval = 0;
    int iMonitorDuration = 0;
    int iMonitorOutputKmsg = 1;
    int iMonitorDumpToFile = 0;
    int iMiuBankAddr = 0;
    struct miu_client *pstMiuClient = NULL;

    short temp_val = 0;
    unsigned long total_temp;
    unsigned long deadline;

    char* u8Buf = NULL;
    char* u8Ptr = NULL;
    char* u8PtrEnd = NULL;
    struct file *pstOutFd = NULL;

    if ('0'== (dev->kobj.name[3])) {
        iMiuClientNum               = MIU0_CLIENT_NUM;
        iMonitorInterval            = gmonitor_interval[0];
        iMonitorDuration            = gmonitor_duration[0];
        iMonitorOutputKmsg          = gmonitor_output_kmsg[0];
        iMiuBankAddr                = BASE_REG_MIU_PA;
        pstMiuClient                = &miu0_clients[0];
    }
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3])) {
        iMiuClientNum               = MIU1_CLIENT_NUM;
        iMonitorInterval            = gmonitor_interval[1];
        iMonitorDuration            = gmonitor_duration[1];
        iMonitorOutputKmsg          = gmonitor_output_kmsg[1];
        iMiuBankAddr                = BASE_REG_MIU1_PA;
        pstMiuClient                = &miu1_clients[0];
    }
#endif
    else {
        return 0;
    }

    for (i = 0; i < iMiuClientNum; i++) {
        (pstMiuClient + i)->effi_min = 0x3FF;
        (pstMiuClient + i)->effi_avg = 0;
        (pstMiuClient + i)->effi_max = 0;
        (pstMiuClient + i)->bw_avg = 0;
        (pstMiuClient + i)->bw_max = 0;
        (pstMiuClient + i)->bw_min = 0x3FF;

        if ((pstMiuClient + i)->bw_dump_en) {
            iMonitorDumpToFile = 1;
        }
    }

    if (iMonitorDumpToFile) {
        u8Buf = kmalloc(DUMP_FILE_TEMP_BUF_SZ, GFP_KERNEL);
        u8Ptr    = u8Buf;
        u8PtrEnd = u8Buf + (DUMP_FILE_TEMP_BUF_SZ);

        pstOutFd = miu_bw_open_file(m_bOutputFilePath, O_WRONLY | O_CREAT, 0644);
    }

    if (iMonitorOutputKmsg) {
        printk("Num:client\tEFFI\tBWavg\tBWmax\tBWavg/E\tBWmax/E\n");
        printk("---------------------------------------------------------\n");
    }
    else {
        str += scnprintf(str, end - str, "Num:client\tEFFI\tBWavg\tBWmax\tBWavg/E\tBWmax/E\n");
        str += scnprintf(str, end - str, "---------------------------------------------------------\n");
    }

    for (i = 0; i < iMiuClientNum; i++)
    {
        if ((pstMiuClient + i)->bw_enabled && (pstMiuClient + i)->bw_rsvd == 0)
        {
            unsigned long diff = 0;
            short last;

            total_temp     = 0;
            temp_loop_time = 0;

            id = (pstMiuClient + i)->bw_client_id;
            OUTREG16((iMiuBankAddr+REG_ID_15), (id & 0x40) ? 0x80 : 0x00);
            id = id & 0x3F;
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x30)); // reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0333);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x35)); // set to read efficiency

            deadline = jiffies + iMonitorDuration*HZ/1000;

            do {
                msleep(iMonitorInterval);

                temp_val = INREG16((iMiuBankAddr+REG_ID_0E));

                if ((pstMiuClient + i)->bw_filter_en) {
                    if (temp_val) {
                        total_temp += temp_val;
                    }
                }
                else {
                    total_temp += temp_val;
                }

                if (temp_loop_time) {
                    diff += (temp_val > last) ? (temp_val - last) : (last - temp_val);
                }
                last = temp_val;

                if ((pstMiuClient + i)->bw_dump_en) {
                    if (temp_loop_time == 0) {
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\n----------------------------------------------------------------\n");
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "ClientId[%d] Name[%s] Efficiency\n", (short)i, (pstMiuClient + i)->name);
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\t0x00\t0x01\t0x02\t0x03\t0x04\t0x05\t0x06\t0x07\n");
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "----------------------------------------------------------------\n");
                    }

                    u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\t%2d.%02d%%",
                                       temp_val*100/1024,
                                       (temp_val*10000/1024)%100);
                    if ((temp_loop_time+1) % 0x8 == 0) {
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\n");
                    }
                }

                if ((pstMiuClient + i)->effi_min > temp_val) {
                    (pstMiuClient + i)->effi_min = temp_val;
                }
                else if ((pstMiuClient + i)->effi_max < temp_val) {
                    (pstMiuClient + i)->effi_max = temp_val;
                }

                if ((pstMiuClient + i)->bw_filter_en) {
                    if (temp_val) {
                        temp_loop_time++;
                    }
                }
                else {
                    temp_loop_time++;
                }

            } while (!time_after_eq(jiffies, deadline));

            OUTREG16((iMiuBankAddr+REG_ID_0D), 0) ; // reset all

            (pstMiuClient + i)->effi_avg = total_temp / temp_loop_time;

            total_temp = 0;
            temp_loop_time = 0;

            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x40)); // reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0303);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x41)); // set to read bandwidth

            deadline = jiffies + iMonitorDuration*HZ/1000;

            do {
                msleep(iMonitorInterval);

                temp_val = INREG16((iMiuBankAddr+REG_ID_0E));

                if ((pstMiuClient + i)->bw_filter_en) {
                    if (temp_val) {
                        total_temp += temp_val;
                    }
                }
                else {
                    total_temp += temp_val;
                }

                if (iMonitorDumpToFile && (pstMiuClient + i)->bw_dump_en) {
                    if (temp_loop_time == 0) {
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\n----------------------------------------------------------------\n");
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "ClientId[%d] Name[%s] BandWidth\n", (short)i, (pstMiuClient + i)->name);
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\t0x00\t0x01\t0x02\t0x03\t0x04\t0x05\t0x06\t0x07\n");
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "----------------------------------------------------------------\n");
                    }

                    u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\t%2d.%02d%%",
                                       temp_val*100/1024,
                                       (temp_val*10000/1024)%100);
                    if ((temp_loop_time+1) % 0x8 == 0) {
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\n");
                    }
                }

                if ((pstMiuClient + i)->bw_max < temp_val) {
                    (pstMiuClient + i)->bw_max = temp_val;
                }
                else if ((pstMiuClient + i)->bw_min > temp_val) {
                    (pstMiuClient + i)->bw_min = temp_val;
                }

                if ((pstMiuClient + i)->bw_filter_en) {
                    if (temp_val) {
                        temp_loop_time++;
                    }
                }
                else {
                    temp_loop_time++;
                }

            } while (!time_after_eq(jiffies, deadline));

            OUTREG16((iMiuBankAddr+REG_ID_0D), 0) ; // reset all

            (pstMiuClient + i)->bw_avg = total_temp / temp_loop_time;
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x60)) ;//reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0303);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x61)) ;//set to read peak
            deadline = jiffies + iMonitorDuration*HZ/1000;
            do{
                msleep(iMonitorDuration);
            } while (!time_after_eq(jiffies, deadline));
            (pstMiuClient + i)->bw_max_div_effi=INREG16((BASE_REG_MIU_PA+REG_ID_0E));

            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x20)) ;//reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0303);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x21)) ;//set to read peak
            deadline = jiffies + iMonitorDuration*HZ/1000;
            do{
                msleep(iMonitorDuration);
            } while (!time_after_eq(jiffies, deadline));
            (pstMiuClient + i)->bw_avg_div_effi=INREG16((BASE_REG_MIU_PA+REG_ID_0E));

            // client effiency never changes and total BW is zero
            if ((diff == 0) && (total_temp == 0)) {
                (pstMiuClient + i)->effi_avg = 0x3FF;
            }

            if (iMonitorOutputKmsg)
            {
                printk("%d:%s\t%2d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\n",
                (short)(pstMiuClient + i)->bw_client_id,
                (pstMiuClient + i)->name,
                (pstMiuClient + i)->effi_avg*100/1024,
                ((pstMiuClient + i)->effi_avg*10000/1024)%100,
                (pstMiuClient + i)->bw_avg*100/1024,
                ((pstMiuClient + i)->bw_avg*10000/1024)%100,
                (pstMiuClient + i)->bw_max*100/1024,
                ((pstMiuClient + i)->bw_max*10000/1024)%100,
                (pstMiuClient + i)->bw_avg_div_effi*100/1024,
                ((pstMiuClient + i)->bw_avg_div_effi*10000/1024)%100,
                (pstMiuClient + i)->bw_max*100/(pstMiuClient + i)->effi_avg,
                ((pstMiuClient + i)->bw_max*10000/(pstMiuClient + i)->effi_avg)%100);
            }
            else
            {
                str +=  scnprintf(str, end - str, "%d:%s\t%2d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\n",
                (short)i,
                (pstMiuClient + i)->name,
                (pstMiuClient + i)->effi_avg*100/1024,
                ((pstMiuClient + i)->effi_avg*10000/1024)%100,
                (pstMiuClient + i)->bw_avg*100/1024,
                ((pstMiuClient + i)->bw_avg*10000/1024)%100,
                (pstMiuClient + i)->bw_max*100/1024,
                ((pstMiuClient + i)->bw_max*10000/1024)%100,
                (pstMiuClient + i)->bw_avg_div_effi*100/1024,
                ((pstMiuClient + i)->bw_avg_div_effi*10000/1024)%100,
                (pstMiuClient + i)->bw_max_div_effi*100/1024,
                ((pstMiuClient + i)->bw_max_div_effi*10000/1024)%100);
            }
        }
    }

    if (iMonitorDumpToFile) {
        if (pstOutFd) {
            miu_bw_write_file(pstOutFd, u8Buf, u8Ptr - u8Buf);
            miu_bw_close_file(pstOutFd);
        }

        kfree(u8Buf);
    }

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t dram_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    unsigned int iMiuBankAddr = 0;
    unsigned int iAtopBankAddr = 0;
    unsigned int iMiupllBankAddr = 0;
    unsigned int dram_type = 0;
    unsigned int ddfset = 0;
    unsigned int dram_freq = 0;
    unsigned int miupll_freq = 0;

    if ('0'== (dev->kobj.name[3])) {
        iMiuBankAddr = BASE_REG_MIU_PA;
        iAtopBankAddr = BASE_REG_ATOP_PA;
        iMiupllBankAddr = BASE_REG_MIUPLL_PA;
    }
#if MIU_NUM > 1
    else if ('1'== (dev->kobj.name[3])) {
        iMiuBankAddr = BASE_REG_MIU1_PA;
        iAtopBankAddr = BASE_REG_ATOP1_PA;
        iMiupllBankAddr = BASE_REG_MIUPLL_PA;
    }
#endif
    else {
        return 0;
    }

    dram_type = INREGMSK16(iMiuBankAddr + REG_ID_01, 0x0003);
    ddfset = (INREGMSK16(iAtopBankAddr + REG_ID_19, 0x00FF) << 16) + INREGMSK16(iAtopBankAddr + REG_ID_18, 0xFFFF);
    dram_freq = ((432 * 4 * 4) << 19) / ddfset;
    miupll_freq = 24 * INREGMSK16(iMiupllBankAddr + REG_ID_03, 0x00FF) / ((INREGMSK16(iMiupllBankAddr + REG_ID_03, 0x0700) >> 8) + 2);

    str += scnprintf(str, end - str, "DRAM Type:   %s\n", (dram_type==3)? "DDR3" : (dram_type==2)? "DDR2" : "Unknown");
    str += scnprintf(str, end - str, "DRAM Size:   %dMB\n", 1 << (INREGMSK16(iMiuBankAddr + REG_ID_69, 0xF000) >> 12));
    str += scnprintf(str, end - str, "DRAM Freq:   %dMHz\n", dram_freq);
    str += scnprintf(str, end - str, "MIUPLL Freq: %dMHz\n", miupll_freq);
    str += scnprintf(str, end - str, "Data Rate:   %dx Mode\n", 1 << (INREGMSK16(iMiuBankAddr + REG_ID_01, 0x0300) >> 8));
    str += scnprintf(str, end - str, "Bus Width:   %dbit\n", 16 << (INREGMSK16(iMiuBankAddr + REG_ID_01, 0x000C) >> 2));

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}


DEVICE_ATTR(monitor_client_enable, 0644, monitor_client_enable_show, monitor_client_enable_store);
DEVICE_ATTR(monitor_client_disable, 0644, monitor_client_disable_show, monitor_client_disable_store);
DEVICE_ATTR(monitor_set_interval_ms, 0644, monitor_set_interval_avg_show, monitor_set_interval_avg_store);
DEVICE_ATTR(monitor_set_duration_ms, 0644, monitor_set_counts_avg_show, monitor_set_counts_avg_store);
DEVICE_ATTR(monitor_client_dump_enable, 0644, monitor_client_dump_enable_show, monitor_client_dump_enable_store);
DEVICE_ATTR(monitor_client_filter_enable, 0644, monitor_filter_abnormal_value_show, monitor_filter_abnormal_value_store);
DEVICE_ATTR(measure_all, 0644, measure_all_show, measure_all_store);
DEVICE_ATTR(measure_all_hw, 0644, measure_all_hw_show, measure_all_store);
DEVICE_ATTR(dram_info, 0444, dram_info_show, NULL);

void mstar_create_MIU_node(void)
{
    int ret = 0;

    miu0.index = 0;
    miu0.dev.kobj.name = "miu0";
    miu0.dev.bus = &miu_subsys;

    ret = subsys_system_register(&miu_subsys, NULL);
    if (ret) {
        printk(KERN_ERR "Failed to register miu sub system!! %d\n",ret);
        return;
    }

    ret = device_register(&miu0.dev);
    if (ret) {
        printk(KERN_ERR "Failed to register miu0 device!! %d\n",ret);
        return;
    }

    device_create_file(&miu0.dev, &dev_attr_monitor_client_enable);
    device_create_file(&miu0.dev, &dev_attr_monitor_client_disable);
    device_create_file(&miu0.dev, &dev_attr_monitor_set_interval_ms);
    device_create_file(&miu0.dev, &dev_attr_monitor_set_duration_ms);
    device_create_file(&miu0.dev, &dev_attr_monitor_client_dump_enable);
    device_create_file(&miu0.dev, &dev_attr_monitor_client_filter_enable);
    device_create_file(&miu0.dev, &dev_attr_measure_all);
    device_create_file(&miu0.dev, &dev_attr_measure_all_hw);
    device_create_file(&miu0.dev, &dev_attr_dram_info);

#if MIU_NUM > 1
    miu1.index = 0;
    miu1.dev.kobj.name = "miu1";
    miu1.dev.bus = &miu_subsys;

#if 0 // Don't register again
    ret = subsys_system_register(&miu_subsys, NULL);
    if (ret) {
        printk(KERN_ERR "Failed to register miu sub system!! %d\n",ret);
        return;
    }
#endif

    ret = device_register(&miu1.dev);
    if (ret) {
        printk(KERN_ERR "Failed to register miu1 device!! %d\n",ret);
        return;
    }

    device_create_file(&miu1.dev, &dev_attr_monitor_client_enable);
    device_create_file(&miu1.dev, &dev_attr_monitor_client_disable);
    device_create_file(&miu1.dev, &dev_attr_monitor_set_interval_ms);
    device_create_file(&miu1.dev, &dev_attr_monitor_set_duration_ms);
    device_create_file(&miu1.dev, &dev_attr_monitor_client_dump_enable);
    device_create_file(&miu1.dev, &dev_attr_monitor_client_filter_enable);
    device_create_file(&miu1.dev, &dev_attr_measure_all);
    device_create_file(&miu1.dev, &dev_attr_measure_all_hw);
    device_create_file(&miu1.dev, &dev_attr_dram_info);
#endif
}
