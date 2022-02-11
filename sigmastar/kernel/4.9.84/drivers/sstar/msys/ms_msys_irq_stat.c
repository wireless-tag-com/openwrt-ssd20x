/*
* ms_msys_irq_stat.c- Sigmastar
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
#include <linux/dma-mapping.h>      /* for dma_alloc_coherent */
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include "registers.h"
#include "mdrv_msys_io_st.h"

#define IRQ_LIST 1000
#define KEPT_IRQ_LATENCY_SIZE 50
struct list_head kept_irq_head;
struct list_head kept_sirq_head;
int sirq_head_initialized =0;

static DEFINE_SPINLOCK(irq_latency_lock);
static unsigned int iIrqLatencyCount = 0;

static DEFINE_SPINLOCK(irq_lock);
int IRQ_COUNTER = 0;
int SIRQ_COUNTER = 0;
extern struct miscdevice  sys_dev;

struct IRQ_INFO_LIST
{
    MSYS_IRQ_INFO irq_info;
    struct list_head list;
};

struct IRQ_INFO_LIST kept_irq_latency[KEPT_IRQ_LATENCY_SIZE];



void msys_dump_irq_latency_info(void)
{
    int i = 0;
    unsigned long cost;
    int counter = 0;
    printk("Irq latency larger than 2ms: %u\n", iIrqLatencyCount);
    for (i = 0; i< KEPT_IRQ_LATENCY_SIZE; i++)
    {
        cost =  kept_irq_latency[i].irq_info.timeEnd - kept_irq_latency[i].irq_info.timeStart;

        counter += 1;
        if (kept_irq_latency[i].irq_info.IRQNumber <= 32)
            break;
        printk( "No: %d, IRQ: %d, cost: %lu us\n", counter, kept_irq_latency[i].irq_info.IRQNumber, (unsigned long)cost >> 10);
    }
}

EXPORT_SYMBOL(msys_dump_irq_latency_info);

void ms_record_large_latency_in_top(MSYS_IRQ_INFO *irq_info)
{
    unsigned int i = 0;
    spin_lock(&irq_latency_lock);

    i = iIrqLatencyCount % KEPT_IRQ_LATENCY_SIZE;

    iIrqLatencyCount ++;
    memcpy(&kept_irq_latency[i].irq_info, irq_info, sizeof(MSYS_IRQ_INFO));

    spin_unlock(&irq_latency_lock);
}



void ms_records_irq(MSYS_IRQ_INFO *irq_info)
{
    struct IRQ_INFO_LIST *new, *old_entry = NULL;
    struct list_head *ptr;
    spin_lock(&irq_lock);
    new=(struct IRQ_INFO_LIST *)kmalloc(sizeof(struct IRQ_INFO_LIST), GFP_KERNEL);

    if(new!=NULL)
    {
        if (IRQ_COUNTER > IRQ_LIST)
        {
            list_for_each_prev(ptr, &kept_irq_head)
            {

                old_entry = list_entry(ptr, struct IRQ_INFO_LIST, list);
                break;
            }
            list_del_init(&old_entry->list);
            kfree(old_entry);
        }
        memcpy(&new->irq_info, irq_info, sizeof(MSYS_IRQ_INFO));
        list_add(&new->list, &kept_irq_head);
        IRQ_COUNTER += 1;

    }
    else
        goto BEACH;

BEACH:
    spin_unlock(&irq_lock);
}

#define IRQ_SIZE 512
unsigned int count[IRQ_SIZE] ={0};
void ms_records_irq_count(int irq_num)
{
    count[irq_num]++;
}
void ms_dump_irq_count(void)
{
    int i;
    printk("ms_dump_irq_count\n");
    for(i=0; i<IRQ_SIZE;  i++ ){
        if(count[i]>0)
        {
            printk( "irq:%03d count:%8d\n",i, count[i]);
        }
    }
}

static ssize_t ms_dump_irqcnt_records(struct device *dev, struct device_attribute *attr, char *buf)
{

    char *str = buf;
//    char *end = buf + PAGE_SIZE;

    //str += scnprintf(str, end - str, "Start Dump IRQ\n");
    ms_dump_irq_count();
    return (str - buf);
}

DEVICE_ATTR(IRQ_COUNT, 0444, ms_dump_irqcnt_records, NULL);

void ms_records_sirq(MSYS_IRQ_INFO *irq_info)
{
    struct IRQ_INFO_LIST *new, *old_entry = NULL;
    struct list_head *ptr;
    unsigned long flags;

    spin_lock_irqsave(&irq_lock, flags);
    new=(struct IRQ_INFO_LIST *)kmalloc(sizeof(struct IRQ_INFO_LIST), GFP_ATOMIC);

    if(new!=NULL)
    {
        if (SIRQ_COUNTER > IRQ_LIST)
        {
            list_for_each_prev(ptr, &kept_sirq_head)
            {

                old_entry = list_entry(ptr, struct IRQ_INFO_LIST, list);
                break;
            }
            list_del_init(&old_entry->list);
            kfree(old_entry);
        }
        memcpy(&new->irq_info, irq_info, sizeof(MSYS_IRQ_INFO));
        list_add(&new->list, &kept_sirq_head);
        SIRQ_COUNTER += 1;

    }
    else
        goto BEACH;

BEACH:
    spin_unlock_irqrestore(&irq_lock, flags);
}


void msys_dump_irq_info(void)
{
    struct list_head *ptr;
    struct IRQ_INFO_LIST *entry;
    unsigned long nanosec_rem = 0;
    unsigned long nanosec_end_rem = 0;
    unsigned long long start = 0;
    unsigned long long end = 0;
    unsigned long cost;

    int counter = 0;
    printk("IRQ_COUNTER:%d \n",IRQ_COUNTER);

    list_for_each(ptr, &kept_irq_head)
    {
        entry = list_entry(ptr, struct IRQ_INFO_LIST, list);
        cost =  entry->irq_info.timeEnd - (unsigned long) entry->irq_info.timeStart;
        start = entry->irq_info.timeStart;
        end = entry->irq_info.timeEnd;

        nanosec_rem = do_div(start, 1000000000);
        nanosec_end_rem = do_div(end, 1000000000);
        counter += 1;
        printk( "No: %03d, IRQ: %02d, cost: %lu ns, timestamp: (%5lu.%06lu) - (%5lu.%06lu) \n", counter, entry->irq_info.IRQNumber, (unsigned long)cost, (
        unsigned long)start, nanosec_rem / 1000,  (unsigned long)end, nanosec_end_rem / 1000);

    }
}

void msys_dump_sirq_info(void)
{
    struct list_head *ptr;
    struct IRQ_INFO_LIST *entry;
    unsigned long nanosec_rem = 0;
    unsigned long nanosec_end_rem = 0;
    unsigned long long start = 0;
    unsigned long long end = 0;
    unsigned long cost;

    int counter = 0;
    printk("Soft IRQ_COUNTER:%d \n",SIRQ_COUNTER);

    list_for_each(ptr, &kept_sirq_head)
    {
        entry = list_entry(ptr, struct IRQ_INFO_LIST, list);
        cost =  entry->irq_info.timeEnd - (unsigned long) entry->irq_info.timeStart;
        start = entry->irq_info.timeStart;
        end = entry->irq_info.timeEnd;

        nanosec_rem = do_div(start, 1000000000);
        nanosec_end_rem = do_div(end, 1000000000);
        counter += 1;
        printk( "No: %03d, vec_nr: %02d, cost: %lu ns, action: %p\n", counter, entry->irq_info.IRQNumber, (unsigned long)cost, entry->irq_info.action);

    }
}

static ssize_t ms_dump_irq_records(struct device *dev, struct device_attribute *attr, char *buf)
{

    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "Start Dump IRQ\n");
    msys_dump_irq_info();
    return (str - buf);
}

DEVICE_ATTR(IRQ_INFO, 0444, ms_dump_irq_records, NULL);
static ssize_t ms_dump_sirq_records(struct device *dev, struct device_attribute *attr, char *buf)
{

    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "Start Dump Soft IRQ\n");
    msys_dump_sirq_info();
    return (str - buf);
}

DEVICE_ATTR(SIRQ_INFO, 0444, ms_dump_sirq_records, NULL);


static ssize_t ms_dump_irq_latency_records(struct device *dev, struct device_attribute *attr, char *buf)
{

    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "Start Dump IRQ Latency Info\n");
    msys_dump_irq_latency_info();
    return (str - buf);
}

DEVICE_ATTR(IRQ_Latency_in_top, 0444, ms_dump_irq_latency_records, NULL);

int msys_irq_stat_init(void)
{
    sirq_head_initialized=1;
    INIT_LIST_HEAD(&kept_irq_head);
    INIT_LIST_HEAD(&kept_sirq_head);
    device_create_file(sys_dev.this_device, &dev_attr_IRQ_INFO);
    device_create_file(sys_dev.this_device, &dev_attr_SIRQ_INFO);
    device_create_file(sys_dev.this_device, &dev_attr_IRQ_COUNT);
    device_create_file(sys_dev.this_device, &dev_attr_IRQ_Latency_in_top);

    return 0;
}
device_initcall(msys_irq_stat_init);
