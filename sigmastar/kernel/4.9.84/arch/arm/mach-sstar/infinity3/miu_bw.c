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



struct miu_device {
    struct device dev;
    int index;
};

struct miu_client{
    char* name;
    short bw_client_id;
    short bw_enabled;
    short bw_max;
    short bw_avg;
    short effi_avg;
    short effi_min;
//    short bw_val_thread;
};

static struct miu_client miu0_clients[] = {
        {"OVERALL   ",0x00,1,0,0},
        {"MFE(F)    ",0x01,0,0,0},
        {"MFE(B)    ",0x02,0,0,0},
        {"VHE       ",0x03,0,0,0},
        {"JPE1      ",0x04,0,0,0},
        {"JPE0      ",0x05,0,0,0},
        {"BACH      ",0x06,0,0,0},
        {"AESDMA    ",0x07,0,0,0},
        {"UHC0      ",0x08,0,0,0},
        {"EMAC      ",0x09,0,0,0},
        {"MCU51     ",0x0A,0,0,0},
        {"URDMA     ",0x0B,0,0,0},
        {"BDMA      ",0x0C,0,0,0},
        {"NA        ",0x0D,0,0,0},
        {"NA        ",0x0E,0,0,0},
        {"MBIST     ",0x0F,0,0,0},
        {"CMDQ      ",0x10,0,0,0},
        {"ISP_DNR   ",0x11,0,0,0},
        {"ISP_ROT   ",0x12,0,0,0},
        {"ISP_STA   ",0x13,0,0,0},
        {"ISP_MNL   ",0x14,0,0,0},
        {"GOP0      ",0x15,0,0,0},
        {"SC_DNR    ",0x16,0,0,0},
        {"SC_DNR_SAD",0x17,0,0,0},
        {"SC_CROP   ",0x18,0,0,0},
        {"SC1_FRM   ",0x19,0,0,0},
        {"SC1_SNP   ",0x1A,0,0,0},
        {"SC1_DBG   ",0x1B,0,0,0},
        {"SC2_FRM   ",0x1C,0,0,0},
        {"SC3_FRM   ",0x1D,0,0,0},
        {"FCIE      ",0x1E,0,0,0},
        {"SDIO      ",0x1F,0,0,0},
        {"CMDQ1     ",0x20,0,0,0},
        {"CMDQ2     ",0x21,0,0,0},
        {"NA        ",0x22,0,0,0},
        {"ISP_STA1  ",0x23,0,0,0},
        {"GOP1      ",0x24,0,0,0},
        {"GOP2      ",0x25,0,0,0},
        {"UHC1      ",0x26,0,0,0},
        {"IVE       ",0x27,0,0,0},
        {"MIIC1     ",0x28,0,0,0},
        {"CPU       ",0x70,0,0,0},

};


static struct miu_device miu0;


static struct bus_type miu_subsys = {
    .name = "miu",
    .dev_name = "miu",
};

//static struct task_struct *pBWmonitorThread=NULL;
//struct mutex bw_monitor_mutex;

int gmonitor_interval = 14;
int gmonitor_duration = 1800;
int gmonitor_output_kmsg = 1;



/*
int BW_measure(short bwclientid)
{
    short BW_val=0;
    mutex_lock(&bw_monitor_mutex);
    OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((bwclientid << 8) & 0xFF00) | 0x50)) ;//reset
    OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((bwclientid << 8) & 0xFF00) | 0x51)) ;//set to read peak

    mdelay(300);
    BW_val=INREG16((BASE_REG_MIU_PA+REG_ID_0E));

    OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D),0) ;//reset all

    mutex_unlock(&bw_monitor_mutex);

    return BW_val;
}

static int BW_monitor(void *arg)
{
    int i=0;
    short tempBW_val=0;
    while(1)
    {
        if (kthread_should_stop()) break;
        for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
        {

            if(miu0_clients[i].bw_enabled)
            {
                //OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x50)) ;//reset
                //OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x51)) ;//set to read peak

                //mdelay(300);
                //tempBW_val=0;
                //tempBW_val=INREG16((BASE_REG_MIU_PA+REG_ID_0E));

                tempBW_val=BW_measure(miu0_clients[i].bw_client_id);

                if(miu0_clients[i].bw_val_thread<tempBW_val)
                {
                    miu0_clients[i].bw_val_thread=tempBW_val;
                }
            }
        }

        //OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D),0) ;//reset all
        mdelay(100);
    }

   return 0;

}
*/
const char* miu_client_id_to_name(U16 id)
{
    int i;
    for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
    {
        if(miu0_clients[i].bw_client_id==id)
        {
            return miu0_clients[i].name;
        }
    }
    return NULL;
}

static int set_miu_client_enable(struct device *dev, const char *buf, size_t n, int enabled)
{
    long idx=-1;
    if (kstrtol(buf, 10, &idx) != 0 || idx<0 || idx >= (sizeof(miu0_clients)/sizeof(miu0_clients[0])) ) return -EINVAL;

    if('0'== (dev->kobj.name[3]))
    {
        miu0_clients[idx].bw_enabled=enabled;
    }
    return n;
}

static ssize_t monitor_client_enable_store(struct device *dev,  struct device_attribute *attr, const char *buf, size_t n)
{
    int i;
    char* pt;
    char* opt;

    if(!strncmp(buf, "all",strlen("all")))
    {
        for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
        {
            miu0_clients[i].bw_enabled=1;
        }
        return n;
    }

    pt = kmalloc(strlen(buf)+1, GFP_KERNEL);
    strcpy(pt, buf);
    while ((opt = strsep(&pt, ";, ")) != NULL)
    {
        set_miu_client_enable(dev,opt,n,1);
    }
    kfree(pt);
    return n;
}
static ssize_t monitor_client_disable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    int i;
    char* pt;
    char* opt;

    if(!strncmp(buf, "all",strlen("all")))
    {
        for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
        {
            miu0_clients[i].bw_enabled=0;
        }
        return n;
    }

    pt = kmalloc(strlen(buf)+1, GFP_KERNEL);
    strcpy(pt, buf);
    while ((opt = strsep(&pt, ";, ")) != NULL)
    {
        set_miu_client_enable(dev,opt,n,0);
    }
    kfree(pt);
    return n;

}

static ssize_t monitor_client_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i=0;

    if('0'== (dev->kobj.name[3]))
    {
        str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Enable(1)/Disable(0)]\n");
        for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
        {
            if(miu0_clients[i].bw_enabled)
            {
                str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",(short)i,miu0_clients[i].name,(short)miu0_clients[i].bw_client_id,(char)miu0_clients[i].bw_enabled);
            }
        }
    }

    if (str > buf)  str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);

}

static ssize_t monitor_client_disable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i=0;

    if('0'== (dev->kobj.name[3]))
    {
        str += scnprintf(str, end - str, "Num:IP_name   [BW_Idx][Enable(1)/Disable(0)]\n");
        for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
        {
            str += scnprintf(str, end - str, "%3d:%s[0x%02X][%d]\n",(short)i,miu0_clients[i].name,(short)miu0_clients[i].bw_client_id,(char)miu0_clients[i].bw_enabled);
        }
    }

    if (str > buf)  str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t monitor_set_interval_avg_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    u32 input;

    input = simple_strtoul(buf, NULL, 10);
    gmonitor_interval = input;
    return count;
}
static ssize_t monitor_set_interval_avg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", gmonitor_interval);
}

static ssize_t monitor_set_counts_avg_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    u32 input;

    input = simple_strtoul(buf, NULL, 10);
    gmonitor_duration = input;
    return count;
}
static ssize_t monitor_set_counts_avg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", gmonitor_duration);
}

/*
static ssize_t measure_bandwidth_MAX_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int ip_loop_time=1;
    int i=0, temp_loop_time=0;
    short tempBW_val=0;

    //reset all bw value
    for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
    {
        miu0_clients[i].bw_max=0;
    }

    for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
    {
        if(miu0_clients[i].bw_enabled)
        {
            for (temp_loop_time=0;temp_loop_time<ip_loop_time;temp_loop_time++)
            {
                //tempBW_val=BW_measure(miu0_clients[i].bw_client_id);

                OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x50)) ;//reset
                OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x51)) ;//set to read peak
                msleep(gmonitor_duration);
                tempBW_val=INREG16((BASE_REG_MIU_PA+REG_ID_0E));
                OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D),0) ;//reset all

                if(miu0_clients[i].bw_max<tempBW_val)
                {
                    miu0_clients[i].bw_max=tempBW_val;
                }
            }
            str += scnprintf(str, end - str, "%2d:%s[0x%02X] BW_max=%3d,%3d.%02d%%\n",(short)i,miu0_clients[i].name,
            (short)miu0_clients[i].bw_client_id,
            miu0_clients[i].bw_max,
            miu0_clients[i].bw_max*100/1024,
            (miu0_clients[i].bw_max*10000/1024)%100);
        }
    }


    if (str > buf)  str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);


}

static ssize_t measure_bandwidth_AVG_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i=0, temp_loop_time=0;
    short temp_val=0;

    unsigned long total_temp;
    unsigned long deadline;

    //reset all bw value
    for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
    {
        miu0_clients[i].bw_avg=0;
        miu0_clients[i].bw_max=0;
    }

    for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
    {
        if(miu0_clients[i].bw_enabled)
        {
            total_temp = 0;
            temp_loop_time=0;
            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x40)) ;//reset
            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x41)) ;//set to read peak
            deadline = jiffies + gmonitor_duration*HZ/1000;
            do{
                if(gmonitor_interval>10)
                    msleep(gmonitor_interval);
                else
                    mdelay(gmonitor_interval);
                temp_val=INREG16((BASE_REG_MIU_PA+REG_ID_0E));
                total_temp += temp_val;
                if(miu0_clients[i].bw_max<temp_val)
                {
                    miu0_clients[i].bw_max=temp_val;
                }
                temp_loop_time++;
            } while (!time_after_eq(jiffies, deadline));
            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D),0) ;//reset all
            miu0_clients[i].bw_avg=total_temp/temp_loop_time;

            //str += scnprintf(str, end - str, "%2d:%s[0x%02X] BW_avg=%4d,%3d.%02d%%  BW_max=%3d,%3d.%02d%%\n",(short)i,miu0_clients[i].name,
            printk( "%2d:%s[0x%02X] BW_avg=%4d,%3d.%02d%%  BW_max=%3d,%3d.%02d%%, t=%d\n",(short)i,miu0_clients[i].name,
            (short)miu0_clients[i].bw_client_id,
            miu0_clients[i].bw_avg,
            miu0_clients[i].bw_avg*100/1024,
            (miu0_clients[i].bw_avg*10000/1024)%100,
            miu0_clients[i].bw_max,
            miu0_clients[i].bw_max*100/1024,
            (miu0_clients[i].bw_max*10000/1024)%100,
            temp_loop_time);

        }
    }


    if (str > buf)  str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t measure_efficiency_AVG_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i=0, temp_loop_time=0;

    short temp_val=0;
    unsigned long total_temp;
    unsigned long deadline;

    for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
    {
        miu0_clients[i].effi_min=0x3ff;
        miu0_clients[i].effi_avg=0;
    }

    for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
    {
        if(miu0_clients[i].bw_enabled)
        {
            total_temp = 0;
            temp_loop_time=0;
            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x30)) ;//reset
            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x35)) ;//set to read peak
            deadline = jiffies + gmonitor_duration*HZ/1000;
            do{
                if(gmonitor_interval>10)
                    msleep(gmonitor_interval);
                else
                    mdelay(gmonitor_interval);

                temp_val=INREG16((BASE_REG_MIU_PA+REG_ID_0E));
                total_temp += temp_val;
                if(miu0_clients[i].effi_min>temp_val)
                {
                    miu0_clients[i].effi_min=temp_val;
                }
                temp_loop_time++;
            } while (!time_after_eq(jiffies, deadline));

            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D),0) ;//reset all
            miu0_clients[i].effi_avg=total_temp/temp_loop_time;

            //str += scnprintf(str, end - str, "%2d:%s[0x%02X] EFFI_avg=%4d,%3d.%02d%%  EFFI_min=%3d,%3d.%02d%%\n",(short)i,miu0_clients[i].name,
            printk( "%2d:%s[0x%02X] EFFI_avg=%4d,%3d.%02d%%  EFFI_min=%4d,%3d.%02d%%\n",(short)i,miu0_clients[i].name,
            (short)miu0_clients[i].bw_client_id,
            miu0_clients[i].effi_avg,
            miu0_clients[i].effi_avg*100/1024,
            (miu0_clients[i].effi_avg*10000/1024)%100,
            miu0_clients[i].effi_min,
            miu0_clients[i].effi_min*100/1024,
            (miu0_clients[i].effi_min*10000/1024)%100);
        }
    }

    if (str > buf)  str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}
*/
static ssize_t measure_all_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    u32 input;
    input = simple_strtoul(buf, NULL, 10);
    gmonitor_output_kmsg = input;
    return count;
}

static ssize_t measure_all_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i=0, temp_loop_time=0;

    short temp_val=0;
    unsigned long total_temp;
    unsigned long deadline;

    for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
    {
        miu0_clients[i].effi_min=0x3ff;
        miu0_clients[i].effi_avg=0;
        miu0_clients[i].bw_avg=0;
        miu0_clients[i].bw_max=0;
    }
    if(gmonitor_output_kmsg)
    {
        printk("Num:client\tEFFI\tBWavg\tBWmax\tBWavg/E\tBWmax/E\n");
        printk("---------------------------------------------------------\n");
    }
    else
    {
        str +=  scnprintf(str, end - str, "Num:client\tEFFI\tBWavg\tBWmax\tBWavg/E\tBWmax/E\n");
        str +=  scnprintf(str, end - str, "---------------------------------------------------------\n");
    }

    for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
    {
        if(miu0_clients[i].bw_enabled)
        {
            total_temp = 0;
            temp_loop_time=0;
            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x30)) ;//reset
            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x35)) ;//set to read peak
            deadline = jiffies + gmonitor_duration*HZ/1000;
            do{
                if(gmonitor_interval>10)
                    msleep(gmonitor_interval);
                else
                    mdelay(gmonitor_interval);

                temp_val=INREG16((BASE_REG_MIU_PA+REG_ID_0E));
                total_temp += temp_val;
                if(miu0_clients[i].effi_min>temp_val)
                {
                    miu0_clients[i].effi_min=temp_val;
                }
                temp_loop_time++;
            } while (!time_after_eq(jiffies, deadline));

            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D),0) ;//reset all
            miu0_clients[i].effi_avg=total_temp/temp_loop_time;

            total_temp = 0;
            temp_loop_time=0;
            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x40)) ;//reset
            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D), ( ((miu0_clients[i].bw_client_id << 8) & 0xFF00) | 0x41)) ;//set to read peak
            deadline = jiffies + gmonitor_duration*HZ/1000;
            do{
                msleep(gmonitor_interval); //mdelay(gmonitor_interval);
                temp_val=INREG16((BASE_REG_MIU_PA+REG_ID_0E));
                total_temp += temp_val;
                if(miu0_clients[i].bw_max<temp_val)
                {
                    miu0_clients[i].bw_max=temp_val;
                }
                temp_loop_time++;
            } while (!time_after_eq(jiffies, deadline));
            OUTREG16( (BASE_REG_MIU_PA+REG_ID_0D),0) ;//reset all
            miu0_clients[i].bw_avg=total_temp/temp_loop_time;
            if(gmonitor_output_kmsg)
            {
                printk("%d:%s\t%2d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\n",
                (short)i,
                miu0_clients[i].name,
                miu0_clients[i].effi_avg*100/1024, (miu0_clients[i].effi_avg*10000/1024)%100,
                miu0_clients[i].bw_avg*100/1024, (miu0_clients[i].bw_avg*10000/1024)%100,
                miu0_clients[i].bw_max*100/1024, (miu0_clients[i].bw_max*10000/1024)%100,
                miu0_clients[i].bw_avg*100/miu0_clients[i].effi_avg,
                (miu0_clients[i].bw_avg*10000/miu0_clients[i].effi_avg)%100,
                miu0_clients[i].bw_max*100/miu0_clients[i].effi_avg,
                (miu0_clients[i].bw_max*10000/miu0_clients[i].effi_avg)%100);
            }
            else
            {
                str +=  scnprintf(str, end - str, "%d:%s\t%2d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\t%02d.%02d%%\n",
                (short)i,
                miu0_clients[i].name,
                miu0_clients[i].effi_avg*100/1024, (miu0_clients[i].effi_avg*10000/1024)%100,
                miu0_clients[i].bw_avg*100/1024, (miu0_clients[i].bw_avg*10000/1024)%100,
                miu0_clients[i].bw_max*100/1024, (miu0_clients[i].bw_max*10000/1024)%100,
                miu0_clients[i].bw_avg*100/miu0_clients[i].effi_avg,
                (miu0_clients[i].bw_avg*10000/miu0_clients[i].effi_avg)%100,
                miu0_clients[i].bw_max*100/miu0_clients[i].effi_avg,
                (miu0_clients[i].bw_max*10000/miu0_clients[i].effi_avg)%100);
            }
        }
    }

    if (str > buf)  str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

/*
static int set_bw_thread_enable(struct device *dev, const char *buf, size_t n)
{
    long idx=-1;
    int i=0;
    int ret;

    if (kstrtol(buf, 10, &idx) != 0 || idx<0 || idx >= 2 ) return -EINVAL;

    if(idx==1)//enable thread
    {
        for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++) //reset all bandwidth value
        {
            miu0_clients[i].bw_val_thread=0;
        }

        if(pBWmonitorThread==NULL)
        {
            pBWmonitorThread = kthread_create(BW_monitor,(void *)&pBWmonitorThread,"BW Monitor");
            if (IS_ERR(pBWmonitorThread))
            {
                ret = PTR_ERR(pBWmonitorThread);
                pBWmonitorThread = NULL;
                return ret;
            }
            wake_up_process(pBWmonitorThread);
        }
    }
    else if (idx==0 && (pBWmonitorThread!=NULL))//disable thread
    {
        kthread_stop(pBWmonitorThread);
        pBWmonitorThread = NULL;
    }
    return n;
}


static ssize_t bw_thread_store(struct device *dev,  struct device_attribute *attr, const char *buf, size_t n)
{
    return set_bw_thread_enable(dev,buf,n);
}


static ssize_t bw_thread_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i=0;

    if('0'== (dev->kobj.name[3]))
    {
        for(i=0; i<(sizeof(miu0_clients)/sizeof(miu0_clients[0]));i++)
        {
            if(miu0_clients[i].bw_enabled)
            {
                //read from bw register and saved back to bw_val
                str += scnprintf(str, end - str, "%2d:%s[0x%04X] BW_val_thread=%3d,%3d.%02d%%\n",(short)i,miu0_clients[i].name,
                (short)miu0_clients[i].bw_client_id,miu0_clients[i].bw_val_thread,miu0_clients[i].bw_val_thread*100/1024,
                (miu0_clients[i].bw_val_thread*10000/1024)%100);

            }
        }
    }

    if (str > buf)  str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);


}
DEVICE_ATTR(bw_thread, 0644, bw_thread_show, bw_thread_store);
*/

DEVICE_ATTR(monitor_client_enable, 0644, monitor_client_enable_show, monitor_client_enable_store);
DEVICE_ATTR(monitor_client_disable, 0644, monitor_client_disable_show, monitor_client_disable_store);
//DEVICE_ATTR(measure_max_bandwidth, 0444, measure_bandwidth_MAX_show, NULL);
//DEVICE_ATTR(measure_avg_bandwidth, 0444, measure_bandwidth_AVG_show, NULL);
//DEVICE_ATTR(measure_avg_efficiency, 0444, measure_efficiency_AVG_show, NULL);
DEVICE_ATTR(measure_all, 0644, measure_all_show, measure_all_store);

DEVICE_ATTR(monitor_set_interval_ms, 0644, monitor_set_interval_avg_show, monitor_set_interval_avg_store);
DEVICE_ATTR(monitor_set_duration_ms, 0644, monitor_set_counts_avg_show, monitor_set_counts_avg_store);

void  mstar_create_MIU_node(void)
{
    int ret;

    miu0.index=0;
    miu0.dev.kobj.name="miu0";
    miu0.dev.bus=&miu_subsys;

    ret = subsys_system_register(&miu_subsys, NULL);
    if (ret)
    {
        printk(KERN_ERR "Failed to register miu sub system!! %d\n",ret);
        return;
    }


    ret=device_register(&miu0.dev);

    if(ret)
    {
        printk(KERN_ERR "Failed to register miu0 device!! %d\n",ret);
        return;
    }

    device_create_file(&miu0.dev, &dev_attr_monitor_client_enable);
    device_create_file(&miu0.dev, &dev_attr_monitor_client_disable);
    //device_create_file(&miu0.dev, &dev_attr_bw_thread);

    //device_create_file(&miu0.dev, &dev_attr_measure_max_bandwidth);
    //device_create_file(&miu0.dev, &dev_attr_measure_avg_bandwidth);
    //device_create_file(&miu0.dev, &dev_attr_measure_avg_efficiency);
    device_create_file(&miu0.dev, &dev_attr_measure_all);
    device_create_file(&miu0.dev, &dev_attr_monitor_set_interval_ms);
    device_create_file(&miu0.dev, &dev_attr_monitor_set_duration_ms);

    //mutex_init(&bw_monitor_mutex);
}
