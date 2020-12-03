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
    {"OVERALL  ",0x00,0},
    {"RSVD     ",0x01,1},
    {"VEN      ",0x02,0},
    {"USB30    ",0x03,0},
    {"JPE_R    ",0x04,0},
    {"JPE_W    ",0x05,0},
    {"BACH     ",0x06,0},
    {"AESDMA   ",0x07,0},
    {"USB20    ",0x08,0},
    {"EMAC     ",0x09,0},
    {"MCU51    ",0x0A,0},
    {"URDMA    ",0x0B,0},
    {"BDMA     ",0x0C,0},
    {"MOVDMA   ",0x0D,0},
    {"LDC      ",0x0E,0},
    {"RSVD     ",0x0F,1},
    {"CMDQ0_R  ",0x10,0},
    {"ISP_DMA_W",0x11,0},
    {"ISP_DMA_R",0x12,0},
    {"ISP_ROT_R",0x13,0},
    {"ISP_MLOAD",0x14,0},
    {"GOP      ",0x15,0},
    {"RSVD     ",0x16,1},
    {"DIP0_R   ",0x17,0},
    {"DIP0_W   ",0x18,0},
    {"SC0_FRM_W",0x19,0},
    {"SC0_FRM_R",0x1A,0},
    {"SC0_DBG_R",0x1B,0},
    {"SC1_FRM_W",0x1C,0},
    {"SC2_FRM_W",0x1D,0},
    {"SD30     ",0x1E,0},
    {"SDIO30   ",0x1F,0},
    {"RSVD     ",0x20,1},
    {"RSVD     ",0x21,1},
    {"CSI_TX_R ",0x22,0},
    {"ISP_DMAG ",0x23,0},
    {"GOP1_R   ",0x24,0},
    {"GOP2_R   ",0x25,0},
    {"USB20_H  ",0x26,0},
    {"MIIC2    ",0x27,0},
    {"MIIC1    ",0x28,0},
    {"3DNR0_W  ",0x29,0},
    {"3DNR0_R  ",0x2A,0},
    {"DLA      ",0x2B,0},
    {"RSVD     ",0x2C,1},
    {"RSVD     ",0x2D,1},
    {"MIIC0    ",0x2E,0},
    {"IVE      ",0x2F,0},
    {"CPU      ",CPU_CLIENT_ID,0},
    {"DLA_HIWAY",DLA_HIWAY_ID,0},
};

static char miu_devname[MIU_NUM][5];
static struct miu_client_bw miu_clients_bw[MIU_NUM][MIU_CLIENT_NUM];
static struct miu_device miu[MIU_NUM];
static struct bus_type miu_subsys = {
    .name = "miu",
    .dev_name = "miu",
};

static int gmonitor_interval[MIU_NUM] = {4};//{14};
static int gmonitor_duration[MIU_NUM] = {2000};//{1800};
static int gmonitor_output_kmsg[MIU_NUM] = {1};
#ifdef SAVE_BW_PROFILING
static char m_bOutputFilePath[128] = "/mnt/dump_miu_bw.txt";
#endif

/*=============================================================*/
// Local function
/*=============================================================*/
#ifdef SAVE_BW_PROFILING
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
#endif

short miu_client_reserved(U16 id)
{
    if (id < MIU_CLIENT_NUM)
    {
        return miu_clients[id].rsvd;
    }
    else if (id == CPU_CLIENT_ID)
    {
        return miu_clients[MIU_CLIENT_NUM - 2].rsvd;
    }
    else if (id == DLA_HIWAY_ID)
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
        return miu_clients[MIU_CLIENT_NUM - 2].name;
    }
    else if (id == DLA_HIWAY_ID)
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

#ifdef SAVE_BW_PROFILING
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
#endif

#ifdef SUPPRESS_ZEROS
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
#endif

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

    str += scnprintf(str, end - str, "Num:IP_name\t[ Idx][Enable(1)/Disable(0)]\n");
    for (i = 0; i < MIU_CLIENT_NUM; i++)
    {
        str += scnprintf(str, end - str, "%3d:%s\t[0x%02X][%d]\n",
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

    str += scnprintf(str, end - str, "Num:IP_name\t[ Idx][Enable(1)/Disable(0)]\n");
    for (i = 0; i < MIU_CLIENT_NUM; i++)
    {
        str += scnprintf(str, end - str, "%3d:%s\t[0x%02X][%d]\n",
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

#ifdef SAVE_BW_PROFILING
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

    str += scnprintf(str, end - str, "Num:IP_name\t[ Idx][Dump Enable(1)/Disable(0)]\n");
    for (i = 0; i < MIU_CLIENT_NUM; i++)
    {
        str += scnprintf(str, end - str, "%3d:%s\t[0x%02X][%d]\n",
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
#endif

#ifdef SUPPRESS_ZEROS
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

    str += scnprintf(str, end - str, "Num:IP_name\t[ Idx][Filter Enable(1)/Disable(0)]\n");
    for (i = 0; i < MIU_CLIENT_NUM; i++)
    {
        str += scnprintf(str, end - str, "%3d:%s\t[0x%02X][%d]\n",
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
#endif

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
#ifdef SAVE_BW_PROFILING
    int iMonitorDumpToFile = 0;
#endif
    int iMiuBankAddr = 0;
    short temp_val = 0;
    unsigned long total_temp;
    unsigned long deadline;
#ifdef SAVE_BW_PROFILING
    char* u8Buf = NULL;
    char* u8Ptr = NULL;
    char* u8PtrEnd = NULL;
    struct file *pstOutFd = NULL;
#endif
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    if (m == 0) {
        iMiuBankAddr = BASE_REG_MIU_PA;
    }
#if MIU_NUM > 1
    else if (m == 1) {
        iMiuBankAddr = BASE_REG_MIU1_PA;
    }
#endif
    else {
        return 0;
    }

    for (i = 0; i < MIU_CLIENT_NUM; i++) {
        miu_clients_bw[m][i].effi_avg = 0;
        miu_clients_bw[m][i].avg = 0;
        miu_clients_bw[m][i].max = 0;
#ifdef SAVE_BW_PROFILING
        if (miu_clients_bw[m][i].dump_en) {
            iMonitorDumpToFile = 1;
        }
#endif
    }

#ifdef SAVE_BW_PROFILING
    if (iMonitorDumpToFile) {
        u8Buf = kmalloc(DUMP_FILE_TEMP_BUF_SZ, GFP_KERNEL);
        u8Ptr    = u8Buf;
        u8PtrEnd = u8Buf + (DUMP_FILE_TEMP_BUF_SZ);

        pstOutFd = miu_bw_open_file(m_bOutputFilePath, O_WRONLY | O_CREAT, 0644);
    }
#endif

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
            unsigned long diff = 0;
            short last;

            total_temp     = 0;
            temp_loop_time = 0;

            id = miu_clients[i].id;
            OUTREG16((iMiuBankAddr+REG_ID_15), (id & 0x40) ? 0x80 : 0x00);
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
                total_temp += temp_val;

                if (temp_loop_time) {
                    diff += (temp_val > last) ? temp_val - last : last - temp_val;
                }
                last = temp_val;
#ifdef SAVE_BW_PROFILING
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
#endif
#ifdef SUPPRESS_ZEROS
                if (miu_clients_bw[m][i].filter_en) {
                    if (temp_val) {
                        temp_loop_time++;
                    }
                }
                else
#endif
                {
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

#ifdef SUPPRESS_ZEROS
                if (miu_clients_bw[m][i].filter_en) {
                    if (temp_val) {
                        total_temp += temp_val;
                    }
                }
                else
#endif
                {
                    total_temp += temp_val;
                }

#ifdef SAVE_BW_PROFILING
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
#endif

                if (miu_clients_bw[m][i].max < temp_val) {
                    miu_clients_bw[m][i].max = temp_val;
                }

#ifdef SUPPRESS_ZEROS
                if (miu_clients_bw[m][i].filter_en) {
                    if (temp_val) {
                        temp_loop_time++;
                    }
                }
                else
#endif
                {
                    temp_loop_time++;
                }
            } while (!time_after_eq(jiffies, deadline));

            OUTREG16((iMiuBankAddr+REG_ID_0D), 0) ; // reset all
            if (temp_loop_time)
                miu_clients_bw[m][i].avg = total_temp / temp_loop_time;
            // client effiency never changes and total BW is zero
            if ((diff == 0) && (total_temp == 0)) {
                miu_clients_bw[m][i].effi_avg = 0x3FF;
            }

            if (gmonitor_output_kmsg[m])
            {
                printk("%3d:%s\t%2d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\n",
                i,
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
                str +=  scnprintf(str, end - str, "%3d:%s\t%2d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\n",
                i,
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

#ifdef SAVE_BW_PROFILING
    if (iMonitorDumpToFile) {
        if (pstOutFd) {
            miu_bw_write_file(pstOutFd, u8Buf, u8Ptr - u8Buf);
            miu_bw_close_file(pstOutFd);
        }

        kfree(u8Buf);
    }
#endif

    if (str > buf)
        str--;
    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t measure_all_hw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i = 0;
    int id;
    int iMiuBankAddr = 0;
    unsigned char m = MIU_IDX(dev->kobj.name[3]);

    if (m == 0) {
        iMiuBankAddr = BASE_REG_MIU_PA;
    }
#if MIU_NUM > 1
    else {
        printk("NOT support multiple MIUs\n");
        return 0;
    }
#endif

    if (gmonitor_output_kmsg[m]) {
        printk("Num:client\tEFFI\tBWavg\tBWmax\n");
        printk("---------------------------------------\n");
    }
    else {
        str += scnprintf(str, end - str, "Num:client\tEFFI\tBWavg\tBWmax\n");
        str += scnprintf(str, end - str, "---------------------------------------\n");
    }

    for (i = 0; i < MIU_CLIENT_NUM; i++)
    {
        short effi_avg = 0, util_avg = 0, peak_avg = 0;
        short val = 0, last = 0;
        int times = 0;
        unsigned long val_total, diff = 0;
        unsigned long deadline;

        if (miu_clients_bw[m][i].enabled && (miu_clients[i].rsvd == 0))
        {
            id = miu_clients[i].id;
            OUTREG16((iMiuBankAddr+REG_ID_15), (id & 0x40) ? 0x80 : 0x00);
            id = id & 0x3F;

            /* client efficiency */
            val_total = times = 0;
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x30)); // reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0303);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x35)); // set to read efficiency

            deadline = jiffies + gmonitor_duration[m]*HZ/1000;

            do {
                if (gmonitor_interval[m] > 10)
                    msleep(gmonitor_interval[m]);
                else
                    usleep_range(gmonitor_interval[m]*1000, gmonitor_interval[m]*1500);

                val = INREG16((iMiuBankAddr+REG_ID_0E));
                if (val_total) {
                    diff += (val > last) ? val - last : last - val;
                }
                last = val;
                val_total += val;
                times++;
            } while(!time_after_eq(jiffies, deadline));

            OUTREG16((iMiuBankAddr+REG_ID_0D), 0) ; // reset all
            effi_avg = val_total / times;

            /* client utilization */
            val_total = times = 0;
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x20)); // reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0303);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x21)); // set to read bandwidth

            deadline = jiffies + gmonitor_duration[m]*HZ/1000;

            do {
                if (gmonitor_interval[m] > 10)
                    msleep(gmonitor_interval[m]);
                else
                    usleep_range(gmonitor_interval[m]*1000, gmonitor_interval[m]*1500);

                val = INREG16((iMiuBankAddr+REG_ID_0E));
                val_total += val;
#ifdef SUPPRESS_ZEROS
                if (miu_clients_bw[m][i].filter_en)
                {
                    if (val)
                    {
                        times++;
                    }
                }
                else
#endif
                {
                    times++;
                }
            } while(!time_after_eq(jiffies, deadline));

            OUTREG16((iMiuBankAddr+REG_ID_0D), 0) ; // reset all
            if (times)
                util_avg = val_total / times;

            /* client peak bandwidth */
            val_total = times = 0;
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x60)); // reset
            OUTREG16((iMiuBankAddr+REG_ID_29), 0x0303);
            OUTREG16((iMiuBankAddr+REG_ID_0D), ((id << 8) | 0x61)); // set to read bandwidth

            deadline = jiffies + gmonitor_duration[m]*HZ/1000;

            do {
                if (gmonitor_interval[m] > 10)
                    msleep(gmonitor_interval[m]);
                else
                    usleep_range(gmonitor_interval[m]*1000, gmonitor_interval[m]*1500);

                val = INREG16((iMiuBankAddr+REG_ID_0E));
                val_total += val;
#ifdef SUPPRESS_ZEROS
                if (miu_clients_bw[m][i].filter_en)
                {
                    if (val)
                    {
                        times++;
                    }
                }
                else
#endif
                {
                    times++;
                }
            } while(!time_after_eq(jiffies, deadline));

            OUTREG16((iMiuBankAddr+REG_ID_0D), 0) ; // reset all
            if (times)
                peak_avg = val_total / times;

            // client efficiency never changed and total peak BW is zero
            if ((diff == 0) && (val_total == 0)) {
                effi_avg = 0x3FF;
            }

            if (gmonitor_output_kmsg[m]) {
                printk("%3d:%s\t%2d.%02d%%\t%2d.%02d%%\t%2d.%02d%%\n",
                i,
                miu_clients[i].name,
                effi_avg*100/1024,
                (effi_avg*10000/1024)%100,
                util_avg*100/1024,
                (util_avg*10000/1024)%100,
                peak_avg*100/1024,
                (peak_avg*10000/1024)%100);
            }
            else {
                str +=  scnprintf(str, end - str, "%3d:%s\t%2d.%02d%%\t%2d.%02d%%\t%2d.%02d%%\n",
                i,
                miu_clients[i].name,
                effi_avg*100/1024,
                (effi_avg*10000/1024)%100,
                util_avg*100/1024,
                (util_avg*10000/1024)%100,
                peak_avg*100/1024,
                (peak_avg*10000/1024)%100);
            }
        }
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
    unsigned char m = MIU_IDX(dev->kobj.name[3]);
    unsigned int iMiuBankAddr = 0;
    unsigned int iAtopBankAddr = 0;
    unsigned int iMiupllBankAddr = 0;
    unsigned int dram_type = 0;
    unsigned int ddfset = 0;
    unsigned int dram_freq = 0;
    unsigned int miupll_freq = 0;

    if (m == 0) {
        iMiuBankAddr = BASE_REG_MIU_PA;
        iAtopBankAddr = BASE_REG_ATOP_PA;
        iMiupllBankAddr = BASE_REG_MIUPLL_PA;
    }
#if MIU_NUM > 1
    else {
        printk("NOT support multiple MIUs\n");
        return 0;
    }
#endif

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
    str += scnprintf(str, end - str, "SSC:         %s\n", (INREGMSK16(iAtopBankAddr + REG_ID_14, 0x8000)==0x8000)? "OFF" : "ON");

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

DEVICE_ATTR(monitor_client_enable, 0644, monitor_client_enable_show, monitor_client_enable_store);
DEVICE_ATTR(monitor_client_disable, 0644, monitor_client_disable_show, monitor_client_disable_store);
DEVICE_ATTR(monitor_set_interval_ms, 0644, monitor_set_interval_avg_show, monitor_set_interval_avg_store);
DEVICE_ATTR(monitor_set_duration_ms, 0644, monitor_set_counts_avg_show, monitor_set_counts_avg_store);
#ifdef SAVE_BW_PROFILING
DEVICE_ATTR(monitor_client_dump_enable, 0644, monitor_client_dump_enable_show, monitor_client_dump_enable_store);
#endif
#ifdef SUPPRESS_ZEROS
DEVICE_ATTR(monitor_client_filter_enable, 0644, monitor_filter_abnormal_value_show, monitor_filter_abnormal_value_store);
#endif
DEVICE_ATTR(measure_all, 0644, measure_all_show, measure_all_store);
#ifdef HW_BW_PROFILING
DEVICE_ATTR(measure_all_hw, 0644, measure_all_hw_show, measure_all_store);
#endif
DEVICE_ATTR(dram_info, 0444, dram_info_show, NULL);

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
#ifdef SAVE_BW_PROFILING
        device_create_file(&miu[i].dev, &dev_attr_monitor_client_dump_enable);
#endif
#ifdef SUPPRESS_ZEROS
        device_create_file(&miu[i].dev, &dev_attr_monitor_client_filter_enable);
#endif
        device_create_file(&miu[i].dev, &dev_attr_measure_all);
#ifdef HW_BW_PROFILING
        device_create_file(&miu[i].dev, &dev_attr_measure_all_hw);
#endif
        device_create_file(&miu[i].dev, &dev_attr_dram_info);
    }
#if CONFIG_SS_MIU_ARBITRATION
    create_miu_arb_node(&miu_subsys);
#endif
}
