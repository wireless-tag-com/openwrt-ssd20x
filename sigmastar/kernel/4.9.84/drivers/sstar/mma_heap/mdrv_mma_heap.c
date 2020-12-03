/*
* mdrv_mma_heap.c- Sigmastar
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
#include <mstar_chip.h>
#include "mdrv_mma_heap.h"

#include <linux/kernel.h>
#include <linux/errno.h>
//#include <linux/swap.h>
#include <linux/init.h>
//#include <linux/bootmem.h>
#include <linux/mman.h> //
#include <linux/export.h>
#include <linux/nodemask.h>
#include <linux/initrd.h>
#include <linux/of_fdt.h>
#include <linux/highmem.h>        //
#include <linux/gfp.h>            //
#include <linux/memblock.h>       //
#include <linux/dma-contiguous.h> //
#include <linux/sizes.h>

#include <asm/mach-types.h>
#include <asm/memblock.h>
#include <asm/prom.h>
#include <asm/sections.h>
#include <asm/setup.h>
#include <asm/tlb.h> //
#include <asm/fixmap.h>

#include <asm/mach/arch.h> //
#include <asm/mach/map.h>

#define MMU_MAX_SIZE 0x4000000
#define MMU_HEAP_NAME "MMU_MMA"

int mstar_driver_boot_mma_buffer_num = 0;
int mstar_driver_boot_mmap_reserved_buffer_num = 0;

struct MMA_BootArgs_Config mma_config[MAX_MMA_AREAS];
struct MMA_BootArgs_Config mmap_reserved_config[MAX_MMA_AREAS];

bool b_mma_memblock_remove = false;

extern int __init arm_add_memory(u64 start, u64 size);

EXPORT_SYMBOL(mma_config);
EXPORT_SYMBOL(mstar_driver_boot_mma_buffer_num);
EXPORT_SYMBOL(mmap_reserved_config);
EXPORT_SYMBOL(mstar_driver_boot_mmap_reserved_buffer_num);
/*
example:
mma_heap=mma_heap_name0,miu=0,sz=0x300000
mma_heap=mma_heap_name_1,miu=0,sz=0x200000
mma_heap=mma_heap_name_2,miu=0,sz=0x400000
mma_heap=mma_heap_name_3,miu=1,sz=0x100000
*/
static bool parse_mmaheap_config(char *cmdline, struct MMA_BootArgs_Config *heapconfig)
{
    char *option;
    int leng = 0;
    //bool has_start = false;
    //int i;

    if (cmdline == NULL)
        goto INVALID_HEAP_CONFIG;

    option = strstr(cmdline, ",");
    leng = (int)(option - cmdline);
    if (leng > (MMA_HEAP_NAME_LENG - 1))
        leng = MMA_HEAP_NAME_LENG - 1;

    strncpy(heapconfig->name, cmdline, leng);
    heapconfig->name[leng] = '\0';

    option = strstr(cmdline, "sz=");
    if (option == NULL)
        goto INVALID_HEAP_CONFIG;
    option = strstr(cmdline, "miu=");
    if (option == NULL)
        goto INVALID_HEAP_CONFIG;
    sscanf(option, "miu=%d,sz=%lx", &heapconfig->miu, &heapconfig->size);
    option = strstr(cmdline, "max_start_off");
    if (option)
        sscanf(option, "max_start_off=%lx", &heapconfig->max_start_offset_to_curr_bus_base);
    else
        heapconfig->max_start_offset_to_curr_bus_base = -1UL;
    option = strstr(cmdline, "max_end_off");
    if (option)
        sscanf(option, "max_end_off=%lx", &heapconfig->max_end_offset_to_curr_bus_base);
    else
        heapconfig->max_end_offset_to_curr_bus_base = -1UL;

#if 0 //debug code
    printk(" debug %s %d  success name:",__FUNCTION__,__LINE__);
        for(i=0;i<leng;i++)
            {
            printk("%c ",heapconfig->name[i]);
            }
        printk("    miu=%d,sz=%lx \n", heapconfig->miu, heapconfig->size);
        printk("Max start %lx max end %lx\n", heapconfig->max_start_offset_to_curr_bus_base, heapconfig->max_end_offset_to_curr_bus_base);
#endif

    return true;

INVALID_HEAP_CONFIG:
    heapconfig->size = 0;
#if 0 //debug code
       printk("samson debug %s %d  fail \n",__FUNCTION__,__LINE__);
#endif
    return false;
}

int __init get_mma_memblock_remove(char *cmdline)
{
    int value = 0;
    if (cmdline)
        sscanf(cmdline, "%d", &value);
    if (value != 0)
        b_mma_memblock_remove = true;
    return 0;
}

early_param("mma_memblock_remove", get_mma_memblock_remove);

int __init setup_mma_info(char *cmdline)
{
    if (!parse_mmaheap_config(cmdline, &mma_config[mstar_driver_boot_mma_buffer_num]))
        printk(KERN_ERR "error: mma_heap args invalid\n");
    else
        mstar_driver_boot_mma_buffer_num++;

    return 0;
}

early_param("mma_heap", setup_mma_info);

int __init setup_mmap_reserved_info(char *cmdline)
{
    if (!parse_mmaheap_config(cmdline, &mmap_reserved_config[mstar_driver_boot_mmap_reserved_buffer_num]))
        printk(KERN_ERR "error: mmap_reserved args invalid\n");
    else
        mstar_driver_boot_mmap_reserved_buffer_num++;

    return 0;
}
early_param("mmap_reserved", setup_mmap_reserved_info);

void deal_with_reserved_mmap(void)
{
    int i = 0;
    phys_addr_t base;
    int ret;
    phys_addr_t start;
    phys_addr_t end;
    phys_addr_t size;

    if (mstar_driver_boot_mmap_reserved_buffer_num == 0)
    {
        //need do nothing
        printk(KERN_WARNING "no any mmap reserved\n");
        return;
    }

    for (i = 0; i < mstar_driver_boot_mmap_reserved_buffer_num; i++)
    {
        mmap_reserved_config[i].reserved_start = 0xFFFFFFFFUL; //INVALID_PHY_ADDR;//init with invalid phy addr
        size = mmap_reserved_config[i].size;
        if (size == 0)
        {
            printk(KERN_WARNING "mmap reserved size is 0, skip\n");
            continue;
        }

        if (mmap_reserved_config[i].max_start_offset_to_curr_bus_base == -1UL     
            || mmap_reserved_config[i].max_end_offset_to_curr_bus_base == -1UL )
        {
                printk(KERN_WARNING "mmap reserved[%d] overbound, skip\n", i);
                continue;
        }
        start = mmap_reserved_config[i].max_start_offset_to_curr_bus_base;
        end = mmap_reserved_config[i].max_end_offset_to_curr_bus_base;
        switch (mmap_reserved_config[i].miu)
        {
        case 0: //miu0
            start+= ARM_MIU0_BUS_BASE;
            end+= ARM_MIU0_BUS_BASE;
            break;
        case 1: //miu1
            start+= ARM_MIU1_BUS_BASE;
            end+= ARM_MIU1_BUS_BASE;
            break;
        case 2: //miu2
            start+= ARM_MIU2_BUS_BASE;
            end+= ARM_MIU2_BUS_BASE;
            break;
        default:
            printk(KERN_ERR "error!!! %s:%d not support miu %d in this chip!!!!\n", __FUNCTION__, __LINE__, mmap_reserved_config[i].miu);
            return;
            break;
        }
        arm_add_memory(start,size);
        ret = memblock_reserve(start, size);
        if (ret != 0) //fail
        {
            printk(KERN_ERR "%s memblock_reserve fail ,  ret=%d\n", __FUNCTION__, ret);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
            printk("baset=0x%llx   size=0x%llx\n", (unsigned long long)base, (unsigned long long)size);
#else
            printk("base=0x%lx   size=0x%lx\n", (unsigned long)base, (unsigned long)size);
#endif

            //In early stage of starting kernel,do not use BUG().
            //replace BUG() with much normal printk to let user notice what happened for debug easier.
            //BUG();
            printk("#####################################\n");
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("#####################################\n");
            return;
        }
        else
        {

            mmap_reserved_config[i].reserved_start = start;
            printk("%s memblock_reserve success mmap_reserved_config[%d].reserved_start=", __FUNCTION__, i);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
            printk("0x%llx", (unsigned long long)mmap_reserved_config[i].reserved_start);
#else
            printk("0x%lx", (unsigned long)mmap_reserved_config[i].reserved_start);
#endif
            printk("\n");
        }
    }
}

void deal_with_reserve_mma_heap(void)
{
    int i = 0, j = 0, k = 0;
    phys_addr_t base;
    int ret;
    phys_addr_t start;
    phys_addr_t end;
    phys_addr_t size;
    deal_with_reserved_mmap();
    if (mstar_driver_boot_mma_buffer_num == 0)
    {
        //need do nothing
        printk(KERN_WARNING "no any mma heap\n");
        return;
    }

    //check whether have same mma heap name.
    for (i = 0; i < mstar_driver_boot_mma_buffer_num - 1; i++)
    {
        if (mma_config[i].size == 0)
        {
            printk(KERN_WARNING "mma heap size is 0, skip\n");
            continue;
        }
        for (j = i + 1; j < mstar_driver_boot_mma_buffer_num; j++)
        {
            if (mma_config[j].size == 0)
            {
                printk(KERN_WARNING "mma heap size is 0, skip\n");
                continue;
            }
            if (0 == strncmp((char *)(&mma_config[i].name[0]), (char *)(&mma_config[j].name[0]), strlen((char *)(&mma_config[i].name[0])) + 1)) //use strlen() +1
            {
                printk("fail,error  ,find same mma heap!!!! \n");
                printk("Please do not use different mma heap with same name , though they may in different miu or have different size\n");

                printk("same mma heap name:  ");
                for (k = 0; k < strlen((char *)(&mma_config[i].name[0])); k++)
                {
                    printk("%c", mma_config[i].name[k]);
                }
                printk("\n");

                /*do not give BUG(),for current function deal_with_reserve_mma_heap will
                  be called by arm_memblock_init,and it is in early stage of starting kernel.
                  if BUG() while cause log only be
                  "
                  Starting kernel ...

                  Uncompressing Linux... done, booting the kernel.
                  "
                  and cause not easy to find what happened!!

                  here replace BUG() with much normal printk to let user notice what happened for debug easier.
                  */
                //BUG();
                printk("#####################################\n");
                printk("%s:%d  fail,error  ,find same mma heap!!!! \n", __FUNCTION__, __LINE__);
                printk("%s:%d  fail,error  ,find same mma heap!!!! \n", __FUNCTION__, __LINE__);
                printk("%s:%d  fail,error  ,find same mma heap!!!! \n", __FUNCTION__, __LINE__);
                printk("%s:%d  fail,error  ,find same mma heap!!!! \n", __FUNCTION__, __LINE__);
                printk("%s:%d  fail,error  ,find same mma heap!!!! \n", __FUNCTION__, __LINE__);
                printk("%s:%d  fail,error  ,find same mma heap!!!! \n", __FUNCTION__, __LINE__);
                printk("%s:%d  fail,error  ,find same mma heap!!!! \n", __FUNCTION__, __LINE__);
                printk("%s:%d  fail,error  ,find same mma heap!!!! \n", __FUNCTION__, __LINE__);
                printk("#####################################\n");

                printk("%s:%d  fail,error  return directly!!!!!!!!!!!!!!\n", __FUNCTION__, __LINE__);
                return;
            }
        }
    }

    for (i = 0; i < mstar_driver_boot_mma_buffer_num; i++)
    {
        mma_config[i].reserved_start = 0xFFFFFFFFUL; //INVALID_PHY_ADDR;//init with invalid phy addr
        size = mma_config[i].size;
        if (size == 0)
        {
            printk(KERN_WARNING "mma heap size is 0, skip\n");
            continue;
        }
        switch (mma_config[i].miu)
        {
        case 0: //miu0
            start = (mma_config[i].max_start_offset_to_curr_bus_base == -1UL) ? ARM_MIU0_BUS_BASE : mma_config[i].max_start_offset_to_curr_bus_base;
            end = (mma_config[i].max_end_offset_to_curr_bus_base == -1UL) ? ARM_MIU1_BUS_BASE : mma_config[i].max_end_offset_to_curr_bus_base;
            break;
        case 1: //miu1
            start = (mma_config[i].max_start_offset_to_curr_bus_base == -1UL) ? ARM_MIU1_BUS_BASE : mma_config[i].max_start_offset_to_curr_bus_base;
            end = (mma_config[i].max_end_offset_to_curr_bus_base == -1UL) ? ARM_MIU2_BUS_BASE : mma_config[i].max_end_offset_to_curr_bus_base;
            break;
        case 2: //miu2
            start = (mma_config[i].max_start_offset_to_curr_bus_base == -1UL) ? ARM_MIU2_BUS_BASE : mma_config[i].max_start_offset_to_curr_bus_base;
            end = (mma_config[i].max_end_offset_to_curr_bus_base == -1UL) ? ARM_MIU3_BUS_BASE : mma_config[i].max_end_offset_to_curr_bus_base;
            break;
        default:
            printk(KERN_ERR "error!!! %s:%d not support miu %d in this chip!!!!\n", __FUNCTION__, __LINE__, mma_config[i].miu);
            return;
            break;
        }

        if (0 == strncmp(MMU_HEAP_NAME, mma_config[i].name, strlen(MMU_HEAP_NAME)))
        {
            int count = 0;
            while (count < end / MMU_MAX_SIZE)
            {
                base = memblock_find_in_range(start + count * MMU_MAX_SIZE, (count + 1) * MMU_MAX_SIZE + start, size, PAGE_SIZE);
                if (base > 0)
                    break;
                count++;
            }
        }
        else
        {
            base = memblock_find_in_range(start, end, size, PAGE_SIZE);
        }

        if (base == 0) //fail
        {
            printk(KERN_ERR "memblock_find_in_range fail ");
#ifdef CONFIG_PHYS_ADDR_T_64BIT
            printk("start=0x%llx, end=0x%llx, size=0x%llx\n", (unsigned long long)start, (unsigned long long)end, (unsigned long long)size);
#else
            printk("start=0x%lx, end=0x%lx, size=0x%lx\n", (unsigned long)start, (unsigned long)end, (unsigned long)size);
#endif

            //In early stage of starting kernel,do not use BUG().
            //replace BUG() with much normal printk to let user notice what happened for debug easier.
            //BUG();
            printk("#####################################\n");
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_find_in_range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("#####################################\n");

            return;
        }

        //In early stage of starting kernel,do not use BUG_ON().
        //replace BUG_ON() with much normal printk to let user notice what happened for debug easier.
        //BUG_ON(base < start || base > end);
        if (base < start || base > end)
        {
            printk("#####################################\n");
            printk("%s:%d base range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d base range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d base range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d base range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d base range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d base range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d base range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d base range  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("#####################################\n");
            return;
        }

        //printk("memblock_find_in_range success   \n");

        ret = memblock_reserve(base, size);
        //printk("%s:%d ",__FUNCTION__,__LINE__);
        if (ret != 0) //fail
        {
            printk(KERN_ERR "%s memblock_reserve fail ,  ret=%d\n", __FUNCTION__, ret);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
            printk("baset=0x%llx   size=0x%llx\n", (unsigned long long)base, (unsigned long long)size);
#else
            printk("base=0x%lx   size=0x%lx\n", (unsigned long)base, (unsigned long)size);
#endif

            //In early stage of starting kernel,do not use BUG().
            //replace BUG() with much normal printk to let user notice what happened for debug easier.
            //BUG();
            printk("#####################################\n");
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("%s:%d memblock_reserve  fail,error !!!! \n", __FUNCTION__, __LINE__);
            printk("#####################################\n");
            return;
        }
        else
        {
            if (b_mma_memblock_remove)
                memblock_remove(base, size);
            mma_config[i].reserved_start = base;
            printk("%s memblock_reserve success mma_config[%d].reserved_start=", __FUNCTION__, i);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
            printk("0x%llx", (unsigned long long)mma_config[i].reserved_start);
#else
            printk("0x%lx", (unsigned long)mma_config[i].reserved_start);
#endif
            printk("\n");

#if 0 //debug code
    printk(" debug %s %d   name:",__FUNCTION__,__LINE__);
        for(j=0;j<strlen(mma_config[i].name);j++)
        {
            printk("%c ",mma_config[i].name[j]);
        }
        printk("    miu=%d,sz=%lx ", mma_config[i].miu, mma_config[i].size);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
        printk("reserved_start=0x%llx\n",mma_config[i].reserved_start);
#else
        printk("reserved_start=0x%lx\n",mma_config[i].reserved_start);
#endif
#endif
        }
    }
}
