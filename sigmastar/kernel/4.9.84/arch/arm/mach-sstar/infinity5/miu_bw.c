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
#include "miu_bw.h"

/*=============================================================*/
// Local variable
/*=============================================================*/

static struct miu_client miu_clients[MIU_CLIENT_NUM] =
{
    {"OVERALL   ",0x00,0},
    {"DIP0_R    ",0x01,0},
    {"DIP0_W    ",0x02,0},
    {"LDC_R     ",0x03,0},
    {"SC2_FRM_W ",0x04,0},
    {"SC3_FRM   ",0x05,0},
    {"RSC_R     ",0x06,0},
    {"SC1_DBG_R ",0x07,0},
    {"CMDQ0_R   ",0x08,0},
    {"MOVDMA0   ",0x09,0},
    {"EMAC      ",0x0A,0},
    {"2DGE      ",0x0B,0},
    {"3DNR0_R   ",0x0C,0},
    {"3DNR0_W   ",0x0D,0},
    {"GOP4_R    ",0x0E,0},
    {"RSVD      ",0x0F,1},
    {"ISP_DMAG0 ",0x10,0},
    {"ISP_DMAG1 ",0x11,0},
    {"ISP_DMAG2 ",0x12,0},
    {"GOP2_R    ",0x13,0},
    {"GOP3_R    ",0x14,0},
    {"ISP_DMAG  ",0x15,0},
    {"ISP_STA   ",0x16,0},
    {"ISP_STA1_W",0x17,0},
    {"CMDQ1_R   ",0x18,0},
    {"MOVDMA1   ",0x19,0},
    {"MCU51     ",0x1A,0},
    {"DLA       ",0x1B,0},
    {"IVE       ",0x1C,0},
    {"RSVD      ",0x1D,1},
    {"RSVD      ",0x1E,1},
    {"RSVD      ",0x1F,1},
    {"RSVD      ",0x20,1},
    {"SC_ROT_R  ",0x21,0},
    {"SC_AIP_W  ",0x22,0},
    {"SC0_FRM_W ",0x23,0},
    {"SC0_SNP_W ",0x24,0},
    {"SC1_FRM_W ",0x25,0},
    {"GOP0_R    ",0x26,0},
    {"3DNR1_R   ",0x27,0},
    {"3DNR1_W   ",0x28,0},
    {"CMDQ2_R   ",0x29,0},
    {"BDMA      ",0x2A,0},
    {"AESDMA    ",0x2B,0},
    {"USB20     ",0x2C,0},
    {"USB20_H   ",0x2D,0},
    {"MIIC1     ",0x2E,0},
    {"URDMA     ",0x2F,0},
    {"VEN_R     ",0x30,0},
    {"VEN_W     ",0x31,0},
    {"JPE_W     ",0x32,0},
    {"JPE_R     ",0x33,0},
    {"DIP1_R    ",0x34,0},
    {"DIP1_W    ",0x35,0},
    {"GOP1_R    ",0x36,0},
    {"BACH0     ",0x37,0},
    {"BACH1     ",0x38,0},
    {"CMDQ3_R   ",0x39,0},
    {"SDIO      ",0x3A,0},
    {"FCIE      ",0x3B,0},
    {"MIIC2     ",0x3C,0},
    {"MIIC3     ",0x3D,0},
    {"RSVD      ",0x3E,1},
    {"RSVD      ",0x3F,1},
    {"CPU       ",CPU_CLIENT_ID,0},
};

static char miu_devname[MIU_NUM][5];
static struct miu_client_bw miu_clients_bw[MIU_NUM][MIU_CLIENT_NUM];
static struct miu_device miu[MIU_NUM];
static struct bus_type miu_subsys = {
    .name = "miu",
    .dev_name = "miu",
};

static int gmonitor_interval[MIU_NUM] = {4, 4};//{14, 14};
static int gmonitor_duration[MIU_NUM] = {2000, 2000};//{1800, 1800};
static int gmonitor_output_kmsg[MIU_NUM] = {1, 1};

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

short miu_client_reserved(U16 id)
{
    if (id < MIU_CLIENT_NUM)
    {
        return miu_clients[id].rsvd;
    }
    else if (id == CPU_CLIENT_ID)
    {
        return miu_clients[MIU_CLIENT_NUM - 1].rsvd;
    }
    return 1;
}

const char* miu_client_id_to_name(U16 id)
{
    if (id < MIU_CLIENT_NUM)
    {
        return miu_clients[id].name;
    }
    else if (id == CPU_CLIENT_ID)
    {
        return miu_clients[MIU_CLIENT_NUM - 1].name;
    }
    return NULL;
}

static int set_miu_client_enable(struct device *dev, const char *buf, size_t n, int enabled)
{
    long idx = -1;
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    if (kstrtol(buf, 10, &idx) != 0 ||
        idx < 0 ||
        idx >= MIU_CLIENT_NUM) {
        return -EINVAL;
    }
    if (miu_clients[idx].rsvd == 0)
    {
        miu_clients_bw[m][idx].enabled = enabled;
    }

    return n;
}

static int set_miu_client_dump_enable(struct device *dev, const char *buf, size_t n, int enabled)
{
    long idx = -1;
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    if (kstrtol(buf, 10, &idx) != 0 ||
        idx < 0 ||
        idx >= MIU_CLIENT_NUM) {
        return -EINVAL;
    }
    if (miu_clients[idx].rsvd == 0)
    {
        miu_clients_bw[m][idx].dump_en = enabled;
    }

    return n;
}

static int set_miu_client_filter_enable(struct device *dev, const char *buf, size_t n, int enabled)
{
    long idx = -1;
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    if (kstrtol(buf, 10, &idx) != 0 ||
        idx < 0 ||
        idx >= MIU_CLIENT_NUM) {
        return -EINVAL;
    }
    if (miu_clients[idx].rsvd == 0)
    {
        miu_clients_bw[m][idx].filter_en = enabled;
    }

    return n;
}

static ssize_t monitor_client_enable_store(struct device *dev,  struct device_attribute *attr, const char *buf, size_t n)
{
    int i;
    char* pt;
    char* opt;
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    if(!strncmp(buf, "all", strlen("all")))
    {
        for (i = 0; i < MIU_CLIENT_NUM; i++) {
            if (miu_clients[i].rsvd == 0)
            {
                miu_clients_bw[m][i].enabled = 1;
            }
        }
        return n;
    }

    pt = kmalloc(strlen(buf)+1, GFP_KERNEL);
    strcpy(pt, buf);
    while((opt = strsep(&pt, ";, ")) != NULL)
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
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Enable(1)/Disable(0)]\n");
    for (i = 0; i < MIU_CLIENT_NUM; i++)
    {
        str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",
                                        i,
                                        miu_clients[i].name,
                                        miu_clients[i].id,
                                        miu_clients_bw[m][i].enabled);
    }

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
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    if (!strncmp(buf, "all", strlen("all")))
    {
        for (i = 0; i < MIU_CLIENT_NUM; i++) {
            if (miu_clients[i].rsvd == 0)
            {
                miu_clients_bw[m][i].enabled = 0;
            }
        }
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
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Enable(1)/Disable(0)]\n");
    for (i = 0; i < MIU_CLIENT_NUM; i++)
    {
        str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",
                                        i,
                                        miu_clients[i].name,
                                        miu_clients[i].id,
                                        miu_clients_bw[m][i].enabled);
    }

    if (str > buf)
        str--;
    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t monitor_set_interval_avg_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    u32 input = 0;
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    input = simple_strtoul(buf, NULL, 10);
    gmonitor_interval[m] = input;

    return count;
}

static ssize_t monitor_set_interval_avg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    return sprintf(buf, "%d\n", gmonitor_interval[m]);
}

static ssize_t monitor_set_counts_avg_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    u32 input = 0;
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    input = simple_strtoul(buf, NULL, 10);
    gmonitor_duration[m] = input;

    return count;
}

static ssize_t monitor_set_counts_avg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    return sprintf(buf, "%d\n", gmonitor_duration[m]);
}

static ssize_t monitor_client_dump_enable_store(struct device *dev,  struct device_attribute *attr, const char *buf, size_t n)
{
    int i;
    char* pt;
    char* opt;
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    if (!strncmp(buf, "all", strlen("all")))
    {
        for (i = 0; i < MIU_CLIENT_NUM; i++) {
            if (miu_clients[i].rsvd == 0)
            {
                miu_clients_bw[m][i].dump_en = 1;
            }
        }
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
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Dump Enable(1)/Disable(0)]\n");
    for (i = 0; i < MIU_CLIENT_NUM; i++)
    {
        str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",
                                        i,
                                        miu_clients[i].name,
                                        miu_clients[i].id,
                                        miu_clients_bw[m][i].dump_en);
    }

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
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    if (!strncmp(buf, "all", strlen("all")))
    {
        for (i = 0; i < MIU_CLIENT_NUM; i++) {
            if (miu_clients[i].rsvd == 0)
            {
                miu_clients_bw[m][i].filter_en = 1;
            }
        }
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
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Filter Enable(1)/Disable(0)]\n");
    for (i = 0; i < MIU_CLIENT_NUM; i++)
    {
        str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",
                                        i,
                                        miu_clients[i].name,
                                        miu_clients[i].id,
                                        miu_clients_bw[m][i].filter_en);
    }

    if (str > buf)
        str--;
    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t measure_all_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    u32 input = 0;
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    input = simple_strtoul(buf, NULL, 10);
    gmonitor_output_kmsg[m] = input;

    return count;
}

static ssize_t measure_all_show(struct device *dev, struct device_attribute *attr, char *buf)
{
#define DUMP_FILE_TEMP_BUF_SZ (PAGE_SIZE*10)

    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i = 0, temp_loop_time = 0;
    int id;
    int iMonitorDumpToFile = 0;
    int iMiuBankAddr = 0;
    short temp_val = 0;
    unsigned long total_temp;
    unsigned long deadline;
    char* u8Buf = NULL;
    char* u8Ptr = NULL;
    char* u8PtrEnd = NULL;
    struct file *pstOutFd = NULL;
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    if (m == 0) {
        iMiuBankAddr = BASE_REG_MIU_PA;
    }
    else if (m == 1) {
        iMiuBankAddr = BASE_REG_MIU1_PA;
    }
    else {
        return 0;
    }

    for (i = 0; i < MIU_CLIENT_NUM; i++) {
        miu_clients_bw[m][i].effi_min = 0x3FF;
        miu_clients_bw[m][i].effi_avg = 0;
        miu_clients_bw[m][i].avg = 0;
        miu_clients_bw[m][i].max = 0;

        if (miu_clients_bw[m][i].dump_en) {
            iMonitorDumpToFile = 1;
        }
    }

    if (iMonitorDumpToFile) {
        u8Buf = kmalloc(DUMP_FILE_TEMP_BUF_SZ, GFP_KERNEL);
        u8Ptr    = u8Buf;
        u8PtrEnd = u8Buf + (DUMP_FILE_TEMP_BUF_SZ);

        pstOutFd = miu_bw_open_file(m_bOutputFilePath, O_WRONLY | O_CREAT, 0644);
    }

    if (gmonitor_output_kmsg[m]) {
        printk("Num:client\tEFFI\tBWavg\tBWmax\tBWavg/E\tBWmax/E\n");
        printk("---------------------------------------------------------\n");
    }
    else {
        str += scnprintf(str, end - str, "Num:client\tEFFI\tBWavg\tBWmax\tBWavg/E\tBWmax/E\n");
        str += scnprintf(str, end - str, "---------------------------------------------------------\n");
    }

    for (i = 0; i < MIU_CLIENT_NUM; i++)
    {
        if (miu_clients_bw[m][i].enabled && (miu_clients[i].rsvd == 0))
        {
            total_temp     = 0;
            temp_loop_time = 0;

            id = miu_clients[i].id;
            OUTREG16((iMiuBankAddr+REG_ID_2B), (id & 0x40) ? 0x01 : 0x00);
            id = id & 0x3F;
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x30)); // reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0000);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x35)); // set to read efficiency

            deadline = jiffies + gmonitor_duration[m]*HZ/1000;

            do {
                if (gmonitor_interval[m] > 10)
                    msleep(gmonitor_interval[m]);
                else
                    mdelay(gmonitor_interval[m]);

                temp_val = INREG16((iMiuBankAddr+REG_ID_0E));

                if (miu_clients_bw[m][i].filter_en) {
                    if (temp_val) {
                        total_temp += temp_val;
                    }
                }
                else {
                    total_temp += temp_val;
                }

                if (miu_clients_bw[m][i].dump_en) {
                    if (temp_loop_time == 0) {
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\n----------------------------------------------------------------\n");
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "ClientId[%d] Name[%s] Efficiency\n", (short)i, miu_clients[i].name);
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

                if (miu_clients_bw[m][i].effi_min > temp_val) {
                    miu_clients_bw[m][i].effi_min = temp_val;
                }

                if (miu_clients_bw[m][i].filter_en) {
                    if (temp_val) {
                        temp_loop_time++;
                    }
                }
                else {
                    temp_loop_time++;
                }

            } while (!time_after_eq(jiffies, deadline));

            OUTREG16((iMiuBankAddr+REG_ID_0D), 0) ; // reset all

            miu_clients_bw[m][i].effi_avg = total_temp / temp_loop_time;

            total_temp = 0;
            temp_loop_time = 0;

            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x40)); // reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0000);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x41)); // set to read bandwidth

            deadline = jiffies + gmonitor_duration[m]*HZ/1000;

            do {
                if (gmonitor_interval[m] > 10)
                    msleep(gmonitor_interval[m]);
                else
                    mdelay(gmonitor_interval[m]);

                temp_val = INREG16((iMiuBankAddr+REG_ID_0E));

                if (miu_clients_bw[m][i].filter_en) {
                    if (temp_val) {
                        total_temp += temp_val;
                    }
                }
                else {
                    total_temp += temp_val;
                }

                if (iMonitorDumpToFile && miu_clients_bw[m][i].dump_en) {
                    if (temp_loop_time == 0) {
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "\n----------------------------------------------------------------\n");
                        u8Ptr += scnprintf(u8Ptr, u8PtrEnd - u8Ptr, "ClientId[%d] Name[%s] BandWidth\n", (short)i, miu_clients[i].name);
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

                if (miu_clients_bw[m][i].max < temp_val) {
                    miu_clients_bw[m][i].max = temp_val;
                }

                if (miu_clients_bw[m][i].filter_en) {
                    if (temp_val) {
                        temp_loop_time++;
                    }
                }
                else {
                    temp_loop_time++;
                }

            } while (!time_after_eq(jiffies, deadline));

            OUTREG16((iMiuBankAddr+REG_ID_0D), 0) ; // reset all

            miu_clients_bw[m][i].avg = total_temp / temp_loop_time;

            if (gmonitor_output_kmsg[m])
            {
                printk("%d:%s\t%2d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\n",
                (short)i,
                miu_clients[i].name,
                miu_clients_bw[m][i].effi_avg*100/1024,
                (miu_clients_bw[m][i].effi_avg*10000/1024)%100,
                miu_clients_bw[m][i].avg*100/1024,
                (miu_clients_bw[m][i].avg*10000/1024)%100,
                miu_clients_bw[m][i].max*100/1024,
                (miu_clients_bw[m][i].max*10000/1024)%100,
                miu_clients_bw[m][i].avg*100/miu_clients_bw[m][i].effi_avg,
                (miu_clients_bw[m][i].avg*10000/miu_clients_bw[m][i].effi_avg)%100,
                miu_clients_bw[m][i].max*100/miu_clients_bw[m][i].effi_avg,
                (miu_clients_bw[m][i].max*10000/miu_clients_bw[m][i].effi_avg)%100);
            }
            else
            {
                str +=  scnprintf(str, end - str, "%d:%s\t%2d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\n",
                (short)i,
                miu_clients[i].name,
                miu_clients_bw[m][i].effi_avg*100/1024,
                (miu_clients_bw[m][i].effi_avg*10000/1024)%100,
                miu_clients_bw[m][i].avg*100/1024,
                (miu_clients_bw[m][i].avg*10000/1024)%100,
                miu_clients_bw[m][i].max*100/1024,
                (miu_clients_bw[m][i].max*10000/1024)%100,
                miu_clients_bw[m][i].avg*100/miu_clients_bw[m][i].effi_avg,
                (miu_clients_bw[m][i].avg*10000/miu_clients_bw[m][i].effi_avg)%100,
                miu_clients_bw[m][i].max*100/miu_clients_bw[m][i].effi_avg,
                (miu_clients_bw[m][i].max*10000/miu_clients_bw[m][i].effi_avg)%100);
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

DEVICE_ATTR(monitor_client_enable, 0644, monitor_client_enable_show, monitor_client_enable_store);
DEVICE_ATTR(monitor_client_disable, 0644, monitor_client_disable_show, monitor_client_disable_store);
DEVICE_ATTR(monitor_set_interval_ms, 0644, monitor_set_interval_avg_show, monitor_set_interval_avg_store);
DEVICE_ATTR(monitor_set_duration_ms, 0644, monitor_set_counts_avg_show, monitor_set_counts_avg_store);
DEVICE_ATTR(monitor_client_dump_enable, 0644, monitor_client_dump_enable_show, monitor_client_dump_enable_store);
DEVICE_ATTR(monitor_client_filter_enable, 0644, monitor_filter_abnormal_value_show, monitor_filter_abnormal_value_store);
DEVICE_ATTR(measure_all, 0644, measure_all_show, measure_all_store);

void mstar_create_MIU_node(void)
{
    int ret = 0, i;

    //initialize MIU client table
    memset(miu_clients_bw, 0, sizeof(miu_clients));
    //OVERALL client monitor enable
    for(i = 0; i < MIU_NUM; i++)
    {
        miu_clients_bw[i][0].enabled = 1;
        miu_devname[i][0] = 'm'; miu_devname[i][1] = 'i'; miu_devname[i][2] = 'u'; miu_devname[i][3] = '0' + i;
    }

    ret = subsys_system_register(&miu_subsys, NULL);
    if (ret) {
        printk(KERN_ERR "Failed to register miu sub system!! %d\n",ret);
        return;
    }
    for(i = 0; i < MIU_NUM; i++)
    {
        miu[i].index = 0;
        miu[i].dev.kobj.name = (const char *)miu_devname[i];
        miu[i].dev.bus = &miu_subsys;

        ret = device_register(&miu[i].dev);
        if (ret) {
            printk(KERN_ERR "Failed to register %s device!! %d\n",miu[i].dev.kobj.name,ret);
            return;
        }

        device_create_file(&miu[i].dev, &dev_attr_monitor_client_enable);
        device_create_file(&miu[i].dev, &dev_attr_monitor_client_disable);
        device_create_file(&miu[i].dev, &dev_attr_monitor_set_interval_ms);
        device_create_file(&miu[i].dev, &dev_attr_monitor_set_duration_ms);
        device_create_file(&miu[i].dev, &dev_attr_monitor_client_dump_enable);
        device_create_file(&miu[i].dev, &dev_attr_monitor_client_filter_enable);
        device_create_file(&miu[i].dev, &dev_attr_measure_all);
    }
    create_miu_bw_node(&miu_subsys);
}
