/*
* ms_msys.c- Sigmastar
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
 * mdrv_system.c
 *
 *  Created on: 2012/9/21
 *      Author: Administrator
 */
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/dma-mapping.h>      /* for dma_alloc_coherent */
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/ctype.h>
#include <linux/swap.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/compaction.h>
#include <asm/cacheflush.h>

#include "ms_platform.h"
#include "registers.h"
#include "mdrv_msys_io_st.h"
#include "mdrv_msys_io.h"
#include "platform_msys.h"
#include "mdrv_verchk.h"

#include "mdrv_system.h"
#ifdef CONFIG_MS_CPU_FREQ
#include "cpu_freq.h"
#endif
#include "cam_os_wrapper.h"

//#define CONFIG_MS_SYSTEM_PART_STRING /*used to in I3*/
//#define CONFIG_MSYS_REQUEST_PROC  /*used to in I3*/
//#define CONFIG_MSYS_KFILE_API /*used to in I3*/
//#define CONFIG_IOCTL_MSYS_MIU_PROTECT /*used to in I3*/
//#define CONFIG_MS_US_TICK_API
//#define CONFIG_MS_PIU_TICK_API
#define CONFIG_IOCTL_MSYS_USER_TO_PHYSICAL
#define CONFIG_IOCTL_MSYS_DMEM
#ifdef CONFIG_ARCH_INFINITY2
#define CONFIG_IOCTL_MSYS_GET_UDID
#endif
#define CONFIG_IOCTL_MSYS_ADDR_TRANS
#define CONFIG_IOCTL_FLUSH_CACHE
//#define CONFIG_IOCTL_IPCM_USELESS



#define MSYS_DEBUG                   0
#define MINOR_SYS_NUM               128
#define MAJOR_SYS_NUM               233


#if MSYS_DEBUG
#define MSYS_PRINT(fmt, args...)    printk("[MSYS] " fmt, ## args)
#else
#define MSYS_PRINT(fmt, args...)
#endif

#define MSYS_ERROR(fmt, args...)    printk(KERN_ERR"MSYS: " fmt, ## args)
#define MSYS_WARN(fmt, args...)     printk(KERN_WARNING"MSYS: " fmt, ## args)

extern void Chip_Flush_Memory(void);

static int msys_open(struct inode *inode, struct file *filp);
static int msys_release(struct inode *inode, struct file *filp);
static long msys_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

typedef struct
{
  MSYS_PROC_DEVICE proc_dev;
  void *proc_addr;
  struct proc_dir_entry* proc_entry;
  struct list_head list;
} PROC_INFO_LIST;

#ifdef CONFIG_MSYS_REQUEST_PROC
static int msys_request_proc_attr(MSYS_PROC_ATTRIBUTE* proc_attr);
static int msys_release_proc_attr(MSYS_PROC_ATTRIBUTE* proc_attr);
static int msys_request_proc_dev(MSYS_PROC_DEVICE* proc_dev);
static int msys_release_proc_dev(MSYS_PROC_DEVICE* proc_dev);
#endif


static struct file_operations msys_fops = {
    .owner = THIS_MODULE,
    .open = msys_open,
    .release = msys_release,
    .unlocked_ioctl=msys_ioctl,
};


struct miscdevice sys_dev = {
    .minor      = MINOR_SYS_NUM,
    .name       = "msys",
    .fops       = &msys_fops,
};

#ifdef CONFIG_MS_SYSTEM_PART_STRING
static unsigned char data_part_string[32]={0};
static unsigned char system_part_string[32]={0};
static unsigned char mstar_property_path[32]="/data";
#endif


static u64 sys_dma_mask = 0xffffffffUL;
struct list_head kept_mem_head;
struct list_head fixed_mem_head;
static struct mutex dmem_mutex;
static unsigned char fixed_dmem_enabled=0;
static unsigned char dmem_realloc_enabled=0;
static unsigned int dmem_retry_count=16;


struct DMEM_INFO_LIST
{
    struct list_head list;
    MSYS_DMEM_INFO dmem_info;
};
#if 0
//port from fs/proc/meminfo.c
unsigned int meminfo_free_in_K(void)
{
    struct sysinfo i;
#if 0
    unsigned long committed;
    struct vmalloc_info vmi;
    long cached;
    long available;
    unsigned long pagecache;
    unsigned long wmark_low = 0;
    unsigned long pages[NR_LRU_LISTS];
    struct zone *zone;
    int lru;
#endif
    /*
     * display in kilobytes.
     */
    #define K(x) ((x) << (PAGE_SHIFT - 10))
    si_meminfo(&i);
    si_swapinfo(&i);
#if 0
    committed = percpu_counter_read_positive(&vm_committed_as);
    cached = global_page_state(NR_FILE_PAGES) -
            total_swapcache_pages() - i.bufferram;
    if (cached < 0)
        cached = 0;

    get_vmalloc_info(&vmi);

    for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
        pages[lru] = global_page_state(NR_LRU_BASE + lru);

    for_each_zone(zone)
        wmark_low += zone->watermark[WMARK_LOW];

    /*
     * Estimate the amount of memory available for userspace allocations,
     * without causing swapping.
     *
     * Free memory cannot be taken below the low watermark, before the
     * system starts swapping.
     */
    available = i.freeram - wmark_low;

    /*
     * Not all the page cache can be freed, otherwise the system will
     * start swapping. Assume at least half of the page cache, or the
     * low watermark worth of cache, needs to stay.
     */
    pagecache = pages[LRU_ACTIVE_FILE] + pages[LRU_INACTIVE_FILE];
    pagecache -= min(pagecache / 2, wmark_low);
    available += pagecache;

    /*
     * Part of the reclaimable slab consists of items that are in use,
     * and cannot be freed. Cap this estimate at the low watermark.
     */
    available += global_page_state(NR_SLAB_RECLAIMABLE) -
             min(global_page_state(NR_SLAB_RECLAIMABLE) / 2, wmark_low);

    if (available < 0)
        available = 0;
#endif
    return K(i.freeram);

}
EXPORT_SYMBOL(meminfo_free_in_K);
#endif


//static void *mm_mem_virt = NULL; /* virtual address of frame buffer 1 */


static int msys_open(struct inode *inode, struct file *filp)
{
//    printk(KERN_WARNING"%s():\n", __FUNCTION__);
    return 0;
}

static int msys_release(struct inode *inode, struct file *filp)
{
//    MSYS_PRINT(KERN_WARNING"%s():\n", __FUNCTION__);
    return 0;
}


int msys_fix_dmem(char* name)
{
    int err=0;
    struct list_head *ptr;
    struct DMEM_INFO_LIST *entry,*match_entry;
    match_entry=NULL;


    mutex_lock(&dmem_mutex);

    if(name!=NULL && name[0]!=0)
    {
        struct DMEM_INFO_LIST *new=NULL;
        list_for_each(ptr, &fixed_mem_head)
        {
            entry = list_entry(ptr, struct DMEM_INFO_LIST, list);
            if (0==strncmp(entry->dmem_info.name, name,strnlen(name,15)))
            {
                match_entry=entry;
                goto BEACH;
            }
        }


        new=(struct DMEM_INFO_LIST *)kmalloc(sizeof(struct DMEM_INFO_LIST),GFP_KERNEL);
        if(new==NULL)
        {
            MSYS_ERROR("allocate memory for fixed_mem_list entry error\n" ) ;
            err = -ENOMEM;
            goto BEACH;
        }
        memset(new->dmem_info.name,0,16);
        memcpy(new->dmem_info.name,name,strnlen(name,15));
        //memcpy(&new->dmem_info,&mem_info,sizeof(MSYS_DMEM_INFO));

        list_add(&new->list, &fixed_mem_head);
    }

BEACH:
    mutex_unlock(&dmem_mutex);
    return err;
}
EXPORT_SYMBOL(msys_fix_dmem);

int msys_unfix_dmem(char* name)
{

    //MSYS_DMEM_INFO mem_info;
    struct list_head *ptr;
    struct DMEM_INFO_LIST *entry,*match_entry;
    match_entry=NULL;

    mutex_lock(&dmem_mutex);

    if(name!=NULL && name[0]!=0)
    {
        list_for_each(ptr, &fixed_mem_head)
        {
            entry = list_entry(ptr, struct DMEM_INFO_LIST, list);
            if (0==strncmp(entry->dmem_info.name, name,strnlen(name,15)))
            {
                match_entry=entry;
                break;
            }
        }
    }

    if(match_entry!=NULL)
    {
        list_del_init(&match_entry->list);
        kfree(match_entry);
    }

//BEACH:
    mutex_unlock(&dmem_mutex);
    return 0;

}
EXPORT_SYMBOL(msys_unfix_dmem);


int msys_find_dmem_by_phys(unsigned long long phys,MSYS_DMEM_INFO *mem_info)
{

    //MSYS_DMEM_INFO mem_info;
    struct list_head *ptr;
    struct DMEM_INFO_LIST *entry;

    int res=-EINVAL;

    mutex_lock(&dmem_mutex);

    if(0!=phys)
    {
        list_for_each(ptr, &kept_mem_head)
        {
            entry = list_entry(ptr, struct DMEM_INFO_LIST, list);
            if ((entry->dmem_info.phys<=phys) && phys<(entry->dmem_info.phys+entry->dmem_info.length))
            {
                memcpy(mem_info,&entry->dmem_info,sizeof(MSYS_DMEM_INFO));
                res=0;
                goto BEACH;
                ;
            }
        }
    }


BEACH:
    mutex_unlock(&dmem_mutex);
    return res;
}
EXPORT_SYMBOL(msys_find_dmem_by_phys);

int msys_find_dmem_by_name(const char *name, MSYS_DMEM_INFO *mem_info)
{
    struct list_head *ptr;
    struct DMEM_INFO_LIST *entry, *match_entry=NULL;
    int res=-EINVAL;

    mutex_lock(&dmem_mutex);

    if(name!=NULL && name[0]!=0)
    {
        list_for_each(ptr, &kept_mem_head)
        {
            entry = list_entry(ptr, struct DMEM_INFO_LIST, list);
            res=strncmp(entry->dmem_info.name, name, 16);
            if (0==res)
            {
                //MSYS_ERROR("%s: Find name\n", __func__);
                match_entry=entry;
                break;
            }
        }
    }
    else
    {
        MSYS_ERROR("%s: Invalid name\n", __func__);
    }

    if(match_entry!=NULL)
    {
        memcpy(mem_info, &match_entry->dmem_info, sizeof(MSYS_DMEM_INFO));
    }
    else
    {
        memset(mem_info->name,0,16);
    }

    mutex_unlock(&dmem_mutex);

    return res;
}


int msys_release_dmem(MSYS_DMEM_INFO *mem_info)
{

    //MSYS_DMEM_INFO mem_info;
    struct list_head *ptr;
    struct DMEM_INFO_LIST *entry,*match_entry;

    int dmem_fixed=0;

    mutex_lock(&dmem_mutex);
    match_entry=NULL;

//  MSYS_PRINT("\nFREEING DMEM [%s]\n\n",mem_info->name);
    if(mem_info->name[0]!=0)
    {
        list_for_each(ptr, &kept_mem_head)
        {
            int res=0;
            entry = list_entry(ptr, struct DMEM_INFO_LIST, list);
            res=strncmp(entry->dmem_info.name, mem_info->name,strnlen(mem_info->name,15));
//          MSYS_PRINT("DMEM0 [%s],%s %d\n",entry->dmem_info.name,match_entry->dmem_info.name,res);
            if (0==res)
            {
                match_entry=entry;
                break;
            }
        }
    }


    if(match_entry==NULL && (0!=mem_info->phys))
    {
        MSYS_ERROR("WARNING!! DMEM [%s]@0x%08X can not be found by name, try to find by phys address\n",mem_info->name, (unsigned int)mem_info->phys);
        list_for_each(ptr, &kept_mem_head)
        {
            entry = list_entry(ptr, struct DMEM_INFO_LIST, list);
            if (entry->dmem_info.phys==mem_info->phys)
            {
                match_entry=entry;
                break;
            }
        }

    }


    if(match_entry==NULL)
    {
        MSYS_ERROR("DMEM [%s]@0x%08X not found, skipping release...\n",mem_info->name, (unsigned int)mem_info->phys);
        goto BEACH;
    }

    if(fixed_dmem_enabled)
    {
        //check if entry is fixed
        list_for_each(ptr, &fixed_mem_head)
        {
            int res=0;
            entry = list_entry(ptr, struct DMEM_INFO_LIST, list);
            res=strcmp(entry->dmem_info.name, match_entry->dmem_info.name);
            if (0==res)
            {
                dmem_fixed=1;
                MSYS_PRINT("DMEM [%s]@0x%08X is fixed, skipping release...\n",match_entry->dmem_info.name,(unsigned int)match_entry->dmem_info.phys);
                goto BEACH;
            }
        }
    }


    dma_free_coherent(sys_dev.this_device, PAGE_ALIGN(match_entry->dmem_info.length),(void *)(uintptr_t)match_entry->dmem_info.kvirt,match_entry->dmem_info.phys);

    MSYS_PRINT("DMEM [%s]@0x%08X successfully released\n",match_entry->dmem_info.name,(unsigned int)match_entry->dmem_info.phys);

    list_del_init(&match_entry->list);
    kfree(match_entry);




BEACH:
    mutex_unlock(&dmem_mutex);
    return 0;

}
EXPORT_SYMBOL(msys_release_dmem);

int msys_request_dmem(MSYS_DMEM_INFO *mem_info)
{
    dma_addr_t phys_addr;
    int err=0;
    int retry=0;

    if(mem_info->name[0]==0||strlen(mem_info->name)>15)
    {
        MSYS_ERROR( "Invalid DMEM name!! Either garbage or empty name!!\n");
        return -EINVAL;
    }

	/*if(mem_info->length<=0)
	{
		MSYS_ERROR( "Invalid DMEM length!! [%s]:0x%08X\n",mem_info->name,(unsigned int)mem_info->length);
		return -EFAULT;
	}*/

    MSYS_ERROR("DMEM request: [%s]:0x%08X\n",mem_info->name,(unsigned int)mem_info->length);

    mutex_lock(&dmem_mutex);
//  if(mem_info->name[0]!=0)
    {
        struct list_head *ptr;
        struct DMEM_INFO_LIST *entry;

        list_for_each(ptr, &kept_mem_head)
        {
            entry = list_entry(ptr, struct DMEM_INFO_LIST, list);
            if (0==strncmp(entry->dmem_info.name, mem_info->name,strnlen(mem_info->name,15)))
            {
                if(dmem_realloc_enabled && (entry->dmem_info.length != mem_info->length))
                {
                    MSYS_ERROR("dmem realloc %s", entry->dmem_info.name);
                    dma_free_coherent(sys_dev.this_device, PAGE_ALIGN(entry->dmem_info.length),(void *)(uintptr_t)entry->dmem_info.kvirt,entry->dmem_info.phys);
                    MSYS_ERROR("DMEM [%s]@0x%08X successfully released\n",entry->dmem_info.name,(unsigned int)entry->dmem_info.phys);
                    list_del_init(&entry->list);
                    break;
                }
                else
                {
                    memcpy(mem_info,&entry->dmem_info,sizeof(MSYS_DMEM_INFO));
                    MSYS_ERROR("DMEM kept entry found: name=%s, phys=0x%08X, length=0x%08X\n",mem_info->name,(unsigned int)mem_info->phys,(unsigned int)mem_info->length);
                    goto BEACH_ENTRY_FOUND;
                }
            }
        }

        //MSYS_PRINT(KERN_WARNING"can not found kept direct requested memory entry name=%s\n",mem_info.name);

    }
//  else
//  {
//      MSYS_PRINT("    !!ERROR!! Anonymous DMEM request is forbidden !!\n");
//      return -EFAULT;
//  }

    while( !(mem_info->kvirt = (u64)(uintptr_t)dma_alloc_coherent(sys_dev.this_device, PAGE_ALIGN(mem_info->length), &phys_addr, GFP_KERNEL)) )
    {
        if(retry >= dmem_retry_count)
        {
            MSYS_ERROR( "unable to allocate direct memory\n");
            err = -ENOMEM;
            goto BEACH_ALLOCATE_FAILED;
        }
        MSYS_ERROR( "retry ALLOC_DMEM %d [%s]:0x%08X\n", retry, mem_info->name, (unsigned int)mem_info->length);
        sysctl_compaction_handler(NULL, 1, NULL, NULL, NULL);
        msleep(1000);
        retry++;
    }

    mem_info->phys=(u64)phys_addr;


    {
        struct DMEM_INFO_LIST *new=(struct DMEM_INFO_LIST *)kmalloc(sizeof(struct DMEM_INFO_LIST),GFP_KERNEL);
        if(new==NULL)
        {
            MSYS_ERROR("allocate memory for mem_list entry error\n" ) ;
            err = -ENOMEM;
            goto BEACH;

        }

        memset(new->dmem_info.name,0,16);
/*
        new->dmem_info.kvirt=mem_info->kvirt;
        new->dmem_info.phys=mem_info->phys;
        new->dmem_info.length=mem_info->length;
        if(mem_info->name!=NULL){
            memcpy(new->dmem_info.name,mem_info->name,strnlen(mem_info->name,15));
        }
*/
        memcpy(&new->dmem_info,mem_info,sizeof(MSYS_DMEM_INFO));

        list_add(&new->list, &kept_mem_head);

    }

    if(retry)
        MSYS_ERROR("DMEM request: [%s]:0x%08X success, @0x%08X (retry=%d)\n",mem_info->name,(unsigned int)mem_info->length, (unsigned int)mem_info->phys, retry);
    else
        MSYS_ERROR("DMEM request: [%s]:0x%08X success, CPU phy:@0x%08X, virt:@0x%08X\n",mem_info->name,(unsigned int)mem_info->length, (unsigned int)mem_info->phys, (unsigned int)mem_info->kvirt);


BEACH:
    if(err==-ENOMEM)
    {
        msys_release_dmem(mem_info);
    }

BEACH_ALLOCATE_FAILED:
BEACH_ENTRY_FOUND:
    if(err)
    {
        MSYS_ERROR("DMEM request: [%s]:0x%08X FAILED!! (retry=%d)\n",mem_info->name,(unsigned int)mem_info->length, retry);
    }

#if 0
    if(0==err){
        memset((void *)((unsigned int)mem_info->kvirt),0,mem_info->length);
        Chip_Flush_CacheAll();
        MSYS_PRINT("DMEM CLEAR!!\n");
    }

#endif

    mutex_unlock(&dmem_mutex);
    return err;

}
EXPORT_SYMBOL(msys_request_dmem);

#ifdef CONFIG_MS_PIU_TICK_API
unsigned int get_PIU_tick_count(void)
{
    return ( INREG16(0x1F006050) | (INREG16(0x1F006054)<<16) );
}

EXPORT_SYMBOL(get_PIU_tick_count);

static ssize_t PIU_T_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%X\n",get_PIU_tick_count());

    return (str - buf);
}

DEVICE_ATTR(PIU_T, 0444, PIU_T_show, NULL);
#endif

#ifdef CONFIG_IOCTL_MSYS_USER_TO_PHYSICAL
int msys_user_to_physical(unsigned long addr,unsigned long *phys)
{

	unsigned long paddr=0;
	struct page *page;
	down_read(&current->mm->mmap_sem);
	//if (get_user_pages(current, current->mm, addr, 1, 1, 0, &page, NULL) <= 0)//3.18
	if (get_user_pages(addr, 1, FOLL_WRITE, &page, NULL) <= 0)
	{
		up_read(&current->mm->mmap_sem);
		printk(KERN_WARNING"ERR!!\n");
		return -EINVAL;
	}
	up_read(&current->mm->mmap_sem);

	paddr= page_to_phys(page);

	*phys=paddr;
//	if(paddr>0x21E00000)
//	{
//		printk(KERN_WARNING"\nKXX:0x%08X,0x%08X\n",(unsigned int)addr,(unsigned int)paddr);
//	}

	return 0;
}
EXPORT_SYMBOL(msys_user_to_physical);
#endif

#ifdef CONFIG_IOCTL_MSYS_DMEM
int msys_find_dmem_by_name_verchk(unsigned long arg)
{
    MSYS_DMEM_INFO mem_info;
    int err=0;
	if(copy_from_user((void*)&mem_info, (void __user *)arg, sizeof(MSYS_DMEM_INFO)))
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(mem_info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(mem_info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                mem_info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(mem_info.VerChk_Size), sizeof(MSYS_DMEM_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_DMEM_INFO), (mem_info.VerChk_Size));
                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }

	if( (err=msys_find_dmem_by_name(mem_info.name, &mem_info)) )
	{
		//return -ENOENT;
	}

	if(copy_to_user((void __user *)arg, (void*)&mem_info, sizeof(MSYS_DMEM_INFO)))
	{
		return -EFAULT;
	}

    return 0;
}

int msys_request_dmem_verchk(unsigned long arg)
{
    MSYS_DMEM_INFO mem_info;
    int err=0;
	if(copy_from_user((void*)&mem_info, (void __user *)arg, sizeof(MSYS_DMEM_INFO)))
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(mem_info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(mem_info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                mem_info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(mem_info.VerChk_Size), sizeof(MSYS_DMEM_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_DMEM_INFO), (mem_info.VerChk_Size));
                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }

	if( (err=msys_request_dmem(&mem_info)) )
	{
		//MSYS_ERROR("request direct memory failed!!\n" );
		return err;
	}

	if(copy_to_user((void __user *)arg, (void*)&mem_info, sizeof(MSYS_DMEM_INFO)))
	{
		return -EFAULT;
	}

    return 0;
}

int msys_release_dmem_verchk(unsigned long arg)
{
    MSYS_DMEM_INFO mem_info;
	if(copy_from_user((void*)&mem_info, (void __user *)arg, sizeof(MSYS_DMEM_INFO)))
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(mem_info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(mem_info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                mem_info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(mem_info.VerChk_Size), sizeof(MSYS_DMEM_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_DMEM_INFO), (mem_info.VerChk_Size));
                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }

	return msys_release_dmem(&mem_info);

}
#endif

int msys_flush_cache(unsigned long arg)
{
	MSYS_DUMMY_INFO info;
	if(copy_from_user((void*)&info, (void __user *)arg, sizeof(MSYS_DUMMY_INFO)))
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(info.VerChk_Size), sizeof(MSYS_DUMMY_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_DUMMY_INFO), (info.VerChk_Size));

                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }

	Chip_Flush_CacheAll();

    return 0;
}

#ifdef CONFIG_IOCTL_MSYS_ADDR_TRANS
int msys_addr_translation_verchk(unsigned long arg, bool direction)
{
    MSYS_ADDR_TRANSLATION_INFO addr_info;

	if(copy_from_user((void*)&addr_info,  (void __user *)arg, sizeof(addr_info)))
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(addr_info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(addr_info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                addr_info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(addr_info.VerChk_Size), sizeof(MSYS_ADDR_TRANSLATION_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_ADDR_TRANSLATION_INFO), (addr_info.VerChk_Size));

                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }

	if(direction)
		addr_info.addr=Chip_MIU_to_Phys(addr_info.addr);
	else
		addr_info.addr=Chip_Phys_to_MIU(addr_info.addr);

	if(copy_to_user((void __user *)arg, (void*)&addr_info, sizeof(addr_info)))
	{
		return -EFAULT;
	}

    return 0;
}
#endif

int msys_get_riu_map_verchk(unsigned long arg)
{
    MSYS_MMIO_INFO mmio_info;
	if( copy_from_user((void*)&mmio_info, (void __user *)arg, sizeof(MSYS_MMIO_INFO)) )
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(mmio_info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&mmio_info, IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                mmio_info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(mmio_info.VerChk_Size), sizeof(MSYS_MMIO_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_MMIO_INFO), (mmio_info.VerChk_Size));

                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }

	mmio_info.addr=Chip_Get_RIU_Phys();
	mmio_info.size=Chip_Get_RIU_Size();
	if( copy_to_user((void __user *)arg, (void*)&mmio_info, sizeof(MSYS_MMIO_INFO)) )
	{
		return -EFAULT;
	}

    return 0;
}

#ifdef CONFIG_IOCTL_MSYS_DMEM
int msys_fix_dmem_verchk(unsigned long arg)
{
    MSYS_DMEM_INFO mem_info;
    int err=0;
	if(copy_from_user((void*)&mem_info, (void __user *)arg, sizeof(MSYS_DMEM_INFO)))
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(mem_info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(mem_info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                mem_info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(mem_info.VerChk_Size), sizeof(MSYS_DMEM_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_DMEM_INFO), (mem_info.VerChk_Size));
                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }
	if( (err=msys_fix_dmem(mem_info.name)) )
	{
		MSYS_ERROR("fix direct memory failed!! %s\n", mem_info.name);
		return err;
	}

    return 0;
}

int msys_unfix_dmem_verchk(unsigned long arg)
{
    MSYS_DMEM_INFO mem_info;
    int err=0;
	if(copy_from_user((void*)&mem_info, (void __user *)arg, sizeof(MSYS_DMEM_INFO)))
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(mem_info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(mem_info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                mem_info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(mem_info.VerChk_Size), sizeof(MSYS_DMEM_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_DMEM_INFO), (mem_info.VerChk_Size));
                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }

	if( (err=msys_unfix_dmem(mem_info.name)) )
	{
		MSYS_ERROR("unfix direct memory failed!! %s\n", mem_info.name);
		return err;
	}

    return 0;
}
#endif

#ifdef CONFIG_IOCTL_MSYS_MIU_PROTECT
int msys_miu_protect_verchk(unsigned long arg)
{
    MSYS_MIU_PROTECT_INFO protect_info;
    u64 miu_addr_start;
    u64 miu_addr_end;
    u32 start_unit, end_unit;
    u8 i=0;

	if(copy_from_user((void*)&protect_info, (void __user *)arg, sizeof(MSYS_MIU_PROTECT_INFO)))
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(protect_info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(protect_info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                protect_info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(protect_info.VerChk_Size), sizeof(MSYS_MIU_PROTECT_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_MIU_PROTECT_INFO), (((MSYS_MIU_PROTECT_INFO __user *)arg)->VerChk_Size));
                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }

    miu_addr_start = Chip_Phys_to_MIU(protect_info.phys);
    miu_addr_end = Chip_Phys_to_MIU(protect_info.phys + protect_info.length) - 1;

    if(miu_addr_start & (0x2000-1)) /*check 8KB align*/
    {
        MSYS_WARN("MIU protect start=0x%08X is not 8KB aligned!\n", (u32)miu_addr_start);
    }

    start_unit = (u32)((miu_addr_start & ~(0x2000-1)) >> 13); // 8KB unit

    OUTREG16(BASE_REG_MIU_PA + REG_ID_60, (u16)(start_unit & 0xFFFF));
    OUTREG16(BASE_REG_MIU_PA + REG_ID_68, (INREG16(BASE_REG_MIU_PA + REG_ID_68)) | ((start_unit>>16) & 0x3));

    if( (miu_addr_end & (0x2000-1)) != (0x2000-1) ) /*check 8KB align*/
    {
        MSYS_WARN("MIU protect end=0x%08X is not 8KB aligned!\n", (u32)miu_addr_end);
    }

    end_unit = (u32)((miu_addr_end & ~(0x2000-1)) >> 13); // 8KB unit

    OUTREG16(BASE_REG_MIU_PA + REG_ID_61, (u16)(end_unit & 0xFFFF));
    OUTREG16(BASE_REG_MIU_PA + REG_ID_68, (INREG16(BASE_REG_MIU_PA + REG_ID_68)) | (((end_unit>>16) & 0x3) << 2));

    printk("\n\tMIU protect start=0x%08X\n", start_unit << 13);
    printk("\tMIU protect end=0x%08X\n", ((end_unit+1) << 13) -1);
    printk("\tMIU protect id=");

    do
    {
        OUTREG16(BASE_REG_MIU_PA + REG_ID_17 + (i*2), (protect_info.id[i] & 0x7F) | (protect_info.id[i+1]&0x7F)<<8 );
        printk(" 0x%02X 0x%02X", protect_info.id[i], protect_info.id[i+1]);
        i+=2;
    } while(protect_info.id[i]!=0x00 && i<16);

    printk("\n");

    OUTREG16(BASE_REG_MIU_PA + REG_ID_10, 0xFFFF);  // for test, we set all id enable

    if(protect_info.w_protect)
        SETREG16(BASE_REG_MIU_PA + REG_ID_69, BIT0);
    if(protect_info.r_protect)
        SETREG16(BASE_REG_MIU_PA + REG_ID_69, BIT4);
    if(protect_info.inv_protect)
        SETREG16(BASE_REG_MIU_PA + REG_ID_69, BIT8);

    printk("\tMIU protect W_protect=%d\n", protect_info.w_protect);
    printk("\tMIU protect R_protect=%d\n", protect_info.r_protect);
    printk("\tMIU protect INV_protect=%d\n", protect_info.inv_protect);

    return 0;
}
#endif

#ifdef CONFIG_MS_SYSTEM_PART_STRING
int msys_string_verchk(unsigned long arg, unsigned int op)
{
	MSYS_STRING_INFO info;

	if(copy_from_user((void*)&info, (void __user *)arg, sizeof(MSYS_STRING_INFO)))
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(info.VerChk_Size), sizeof(MSYS_STRING_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_STRING_INFO), (info.VerChk_Size));

                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EINVAL;
    }

	if(op==0)
	{
		if(copy_to_user(&(((MSYS_STRING_INFO __user *)arg)->str[0]), (void*)system_part_string, sizeof(system_part_string)))
		{
			return -EFAULT;
		}
	}
	else if(op==1)
	{
		if(copy_to_user(&(((MSYS_STRING_INFO __user *)arg)->str[0]), (void*)data_part_string, sizeof(data_part_string)))
		{
			return -EFAULT;
		}
	}
	else if(op==2)
	{
		if(copy_to_user(&(((MSYS_STRING_INFO __user *)arg)->str[0]), (void*)mstar_property_path, sizeof(mstar_property_path)))
		{
			return -EFAULT;
		}
	}
	else if(op==3)
	{
		if(copy_from_user((void*)mstar_property_path,  &(((MSYS_STRING_INFO __user *)arg)->str[0]), sizeof(mstar_property_path)))
		{
			return -EFAULT;
		}
		printk("set mstar_property_path=%s\n", mstar_property_path);
	}
	else
	{
		MSYS_ERROR("[%s] unsupport op=%d!!\n", __FUNCTION__, op);
		return -EINVAL;
	}

    return 0;
}
#endif

int msys_read_uuid(unsigned long long* udid)
{

#ifdef CONFIG_ARCH_INFINITY2
    *udid = (u64)INREG16(BASE_REG_PMTOP_PA + REG_ID_00) ;
#else
    CLRREG16(BASE_REG_EFUSE_PA + REG_ID_03, BIT8);  //reg_sel_read_256[8]=0 to read a/b/c/d
    *udid = (u64)INREG16(BASE_REG_EFUSE_PA + REG_ID_16) |
            ((u64)(INREG16(BASE_REG_EFUSE_PA + REG_ID_17)) << 16) |
            ((u64)INREG16(BASE_REG_EFUSE_PA + REG_ID_18) << 32);
#endif
    return 0;
}
EXPORT_SYMBOL(msys_read_uuid);

CHIP_VERSION msys_get_chipVersion(void)
{
    CHIP_VERSION eRet = U01;
    eRet = (INREG16(BASE_REG_PMTOP_PA + REG_ID_01)>>8);
    return eRet;
}
EXPORT_SYMBOL(msys_get_chipVersion);

#ifdef CONFIG_IOCTL_IPCM_USELESS
int msys_get_chipVersion_verchk(unsigned long arg)
{
    MSYS_CHIPVER_INFO chipVer_info;
    if(copy_from_user((void*)&chipVer_info,  (void __user *)arg, sizeof(chipVer_info)))
    {
        return -EFAULT;
    }

    if ( CHK_VERCHK_HEADER(&(chipVer_info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(chipVer_info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                chipVer_info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(chipVer_info.VerChk_Size), sizeof(MSYS_CHIPVER_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_CHIPVER_INFO), chipVer_info.VerChk_Size);

                return -EINVAL;
            }
            else
            {
                chipVer_info.chipVersion = msys_get_chipVersion();
                if(copy_to_user((void __user *)arg, (void*)&chipVer_info, sizeof(chipVer_info)))
                {
                    return -EFAULT;
                }
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }

    return 0;
}
#endif
#ifdef CONFIG_IOCTL_MSYS_GET_UDID
int msys_get_udid_verchk(unsigned long arg)
{
    MSYS_UDID_INFO udid_info;
	if(copy_from_user((void*)&udid_info,  (void __user *)arg, sizeof(udid_info)))
	{
		return -EFAULT;
	}

    if ( CHK_VERCHK_HEADER(&(udid_info.VerChk_Version)) )
    {
        if( CHK_VERCHK_VERSION_LESS(&(udid_info.VerChk_Version), IOCTL_MSYS_VERSION) )
        {
            VERCHK_ERR("\n\33[1;31m[%s] verchk version (%04x) < ioctl verision (%04x) !!!\33[0m\n", __FUNCTION__,
                udid_info.VerChk_Version & VERCHK_VERSION_MASK, IOCTL_MSYS_VERSION);
            return -EINVAL;
        }
        else
        {
            if( CHK_VERCHK_SIZE(&(udid_info.VerChk_Size), sizeof(MSYS_UDID_INFO)) == 0 )
            {
                VERCHK_ERR("\n\33[1;31m[%s] struct size(%04x) != verchk size(%04x) !!!\33[0m\n", __FUNCTION__,
                    sizeof(MSYS_UDID_INFO), (udid_info.VerChk_Size));

                return -EINVAL;
            }
        }
    }
    else
    {
        VERCHK_ERR("\n\33[1;31m[%s] No verchk header !!!\33[0m\n", __FUNCTION__);
        return -EFAULT;
    }

	msys_read_uuid(&(udid_info.udid));

	if(copy_to_user((void __user *)arg, (void*)&udid_info, sizeof(udid_info)))
	{
		return -EFAULT;
	}

    return 0;
}
#endif
#ifdef CONFIG_IOCTL_IPCM_USELESS
static int msys_check_freq_cfg(unsigned long arg)
{
    MSYS_FREQGEN_INFO freq_info;

    if(copy_from_user((void*)&freq_info, (void __user *)arg, sizeof(MSYS_FREQGEN_INFO)))
    {
        return -EFAULT;
    }

    return msys_request_freq(&freq_info);
}
#endif
static long msys_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int err= 0;

    // wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
    if (_IOC_TYPE(cmd) != MSYS_IOCTL_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > IOCTL_SYS_MAXNR) return -ENOTTY;

    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if (err)
    {
        return -EFAULT;
    }


    switch(cmd)
    {
#ifdef CONFIG_IOCTL_MSYS_DMEM
        case IOCTL_MSYS_REQUEST_DMEM:
        {
            if((err=msys_request_dmem_verchk(arg)))
                MSYS_ERROR("IOCTL_MSYS_REQUEST_DMEM error!\n");
        }
        break;

        case IOCTL_MSYS_RELEASE_DMEM:
        {
            if((err=msys_release_dmem_verchk(arg)))
                MSYS_ERROR("IOCTL_MSYS_RELEASE_DMEM error!\n");
        }
        break;
        case IOCTL_MSYS_FIX_DMEM:
        {
            if((err=msys_fix_dmem_verchk(arg)))
                MSYS_ERROR("IOCTL_MSYS_FIX_DMEM error!\n");
        }
        break;

        case IOCTL_MSYS_UNFIX_DMEM:
        {
            if((err=msys_unfix_dmem_verchk(arg)))
                MSYS_ERROR("IOCTL_MSYS_UNFIX_DMEM error!\n");
        }
        break;

        case IOCTL_MSYS_FIND_DMEM_BY_NAME:
        {
              if((err=msys_find_dmem_by_name_verchk(arg)))
                  MSYS_ERROR("IOCTL_MSYS_FIND_DMEM_BY_NAME error!\n");
            msys_find_dmem_by_name_verchk(arg);
            err=0;
        }
        break;
#endif

#ifdef CONFIG_IOCTL_MSYS_ADDR_TRANS
        case IOCTL_MSYS_PHYS_TO_MIU:
        {
            if((err=msys_addr_translation_verchk(arg, 0)))
                MSYS_ERROR("IOCTL_MSYS_PHYS_TO_MIU error!\n");
        }
        break;

        case IOCTL_MSYS_MIU_TO_PHYS:
        {
            if((err=msys_addr_translation_verchk(arg, 1)))
                MSYS_ERROR("IOCTL_MSYS_MIU_TO_PHYS error!\n");
        }
        break;
#endif
        case IOCTL_MSYS_GET_RIU_MAP:
        {
            if((err=msys_get_riu_map_verchk(arg)))
                MSYS_ERROR("IOCTL_MSYS_GET_RIU_MAP error!\n");
        }
        break;
#ifdef CONFIG_IOCTL_MSYS_MIU_PROTECT
        case IOCTL_MSYS_MIU_PROTECT:
        {
            if((err=msys_miu_protect_verchk(arg)))
                MSYS_ERROR("IOCTL_MSYS_MIU_PROTECT error!\n");
        }
        break;
#endif
#ifdef CONFIG_IOCTL_MSYS_USER_TO_PHYSICAL
        case IOCTL_MSYS_USER_TO_PHYSICAL:
        {
            unsigned long addr,paddr;


            if(copy_from_user((void*)&addr,  (void __user *)arg, sizeof(addr)))
            {
                return -EFAULT;
            }

            if((err=msys_user_to_physical(addr,&paddr)))
                MSYS_ERROR("IOCTL_MSYS_GET_USER_PAGE error!\n");

            if(copy_to_user((void __user *)arg, (void*)&paddr, sizeof(paddr)))
            {
                return -EFAULT;
            }
        }
        break;
#endif
#ifdef CONFIG_MS_SYSTEM_PART_STRING
        case IOCTL_MSYS_GET_SYSP_STRING:
        {
            if((err=msys_string_verchk(arg, 0)))
                MSYS_ERROR("IOCTL_MSYS_GET_SYSP_STRING error!\n");
        }
        break;

        case IOCTL_MSYS_GET_DATAP_STRING:
        {
            if((err=msys_string_verchk(arg, 1)))
                MSYS_ERROR("IOCTL_MSYS_GET_DATAP_STRING error!\n");
        }
        break;

        case IOCTL_MSYS_GET_PROPERTY_PATH:
        {
            if((err=msys_string_verchk(arg, 2)))
                MSYS_ERROR("IOCTL_MSYS_GET_PROPERTY_PATH error!\n");
        }
        break;

        case IOCTL_MSYS_SET_PROPERTY_PATH:
        {
            if((err=msys_string_verchk(arg, 3)))
                MSYS_ERROR("IOCTL_MSYS_SET_PROPERTY_PATH error!\n");
        }
        break;
#endif

#ifdef CONFIG_MS_US_TICK_API
        case IOCTL_MSYS_GET_US_TICKS:
        {

            u64 us_ticks=Chip_Get_US_Ticks();
            if(copy_to_user((void __user *)arg, (void*)&us_ticks, sizeof(us_ticks)))
            {
                return -EFAULT;
            }
            return -EPERM;
        }
        break;
#endif
#ifdef CONFIG_IOCTL_MSYS_GET_UDID
        case IOCTL_MSYS_GET_UDID:
        {
            if((err=msys_get_udid_verchk(arg)))
                MSYS_ERROR("IOCTL_MSYS_GET_UDID error!\n");
        }
        break;
#endif
#ifdef CONFIG_MS_PIU_TICK_API
        case IOCTL_MSYS_PRINT_PIU_TIMER_TICKS:
        {
            int id=arg;
            printk(KERN_WARNING"PIU_T:%X#%d#\n",get_PIU_tick_count(),id);
        }
        break;
#endif
#ifdef CONFIG_MSYS_BENCH_MEMORY_FUNC
        case IOCTL_MSYS_BENCH_MEMORY:
        {
            int test_mem_size_in_MB=arg;
            extern void msys_bench_memory(unsigned int uMemSize);
            msys_bench_memory((unsigned int)test_mem_size_in_MB);
        }
        break;
#endif

#ifdef CONFIG_MSYS_REQUEST_PROC
        case IOCTL_MSYS_REQUEST_PROC_DEVICE:
        {
            MSYS_PROC_DEVICE proc_dev;
            if(copy_from_user((void*)&proc_dev, (void __user *)arg, sizeof(MSYS_PROC_DEVICE)))
                BUG();

            if((err = msys_request_proc_dev(&proc_dev)) == -EEXIST) {
                MSYS_PRINT("skip since device %s exist\n" , proc_attr->name);
            } else if(err != 0) {
                MSYS_ERROR("msys_request_proc_dev failed!!\n" );
                break;
            }

            if(copy_to_user((void*)arg, (void*)&proc_dev, sizeof(MSYS_PROC_DEVICE)))
                BUG();
        }
        break;

        case IOCTL_MSYS_RELEASE_PROC_DEVICE:
        {
            MSYS_PROC_DEVICE proc_dev;
            if(copy_from_user((void*)&proc_dev, (void __user *)arg, sizeof(MSYS_PROC_DEVICE)))
                BUG();

            if((err = msys_release_proc_dev(&proc_dev)) != 0) {
                MSYS_ERROR("msys_release_proc_dev failed!!\n" );
                break;
            }

            if(copy_to_user((void*)arg, (void*)&proc_dev, sizeof(MSYS_PROC_DEVICE)))
                BUG();
        }
        break;

        case IOCTL_MSYS_REQUEST_PROC_ATTRIBUTE:
        {
            MSYS_PROC_ATTRIBUTE proc_attr;

            if(copy_from_user((void*)&proc_attr, (void __user *)arg, sizeof(MSYS_PROC_ATTRIBUTE)))
                BUG();
            if((err = msys_request_proc_attr(&proc_attr)) == -EEXIST) {
                MSYS_PRINT("skip since attribute %s exist\n" , proc_attr.name);
            } else if(err != 0) {
                MSYS_ERROR("msys_request_proc_attribute failed!!\n" );
                break;
            }

            if(copy_to_user((void*)arg, (void*)&proc_attr, sizeof(MSYS_PROC_ATTRIBUTE)))
                BUG();
        }
        break;

        case IOCTL_MSYS_RELEASE_PROC_ATTRIBUTE:
        {
            MSYS_PROC_ATTRIBUTE proc_attr;

            if(copy_from_user((void*)&proc_attr, (void __user *)arg, sizeof(MSYS_PROC_ATTRIBUTE)))
                BUG();

            if((err = msys_release_proc_attr(&proc_attr)) != 0) {
                MSYS_ERROR("msys_release_proc_attr failed!!\n" );
                break;
            }

            if(copy_to_user((void*)arg, (void*)&proc_attr, sizeof(MSYS_PROC_ATTRIBUTE)))
                BUG();
        }
        break;
#endif
#ifdef CONFIG_IOCTL_FLUSH_CACHE
        case IOCTL_MSYS_FLUSH_CACHE:
        {
            if((err = msys_flush_cache(arg)))
                MSYS_ERROR("IOCTL_MSYS_FLUSH_CACHE error!\n");
        }
        break;

		case IOCTL_MSYS_FLUSH_MEMORY:
        {
			 __cpuc_flush_kern_all();//L1
			 Chip_Flush_Memory();//L3
        }
#endif
#ifdef CONFIG_IOCTL_IPCM_USELESS
        case IOCTL_MSYS_RESET_TO_UBOOT:
        {
            do
            {
                SETREG16(REG_ADDR_STATUS, FORCE_UBOOT_BIT);
            } while(!(INREG16(REG_ADDR_STATUS) & FORCE_UBOOT_BIT));
            OUTREG16(BASE_REG_PMSLEEP_PA + REG_ID_2E, 0x79);
        }
        break;

        case IOCTL_MSYS_REQUEST_FREQUENCY:
        {
            if((err = msys_check_freq_cfg(arg)))
                MSYS_ERROR("IOCTL_MSYS_REQUEST_FREQUENCY error!\n");
        }
        break;

        case IOCTL_MSYS_GET_CHIPVERSION:
        {
            if((err = msys_get_chipVersion_verchk(arg)))
                MSYS_ERROR("IOCTL_MSYS_GET_CHIPVERSION error!\n");
        }
        break;
#endif
#ifdef CONFIG_MS_CPU_FREQ
        case IOCTL_MSYS_READ_PM_TSENSOR:
        {
            int temp = ms_get_temp();
            if(copy_to_user( (void __user *)arg, &temp, sizeof(temp) ))
                return -EFAULT;
        }
#endif
        break;

        default:
            MSYS_ERROR("Unknown IOCTL Command 0x%08X\n", cmd);
            return -ENOTTY;
    }


    return err;
}

#ifdef CONFIG_MS_SYSTEM_PART_STRING
static int __init setup_system_part_string(char *arg)
{
    memcpy(system_part_string,(arg+1),strlen(arg)<sizeof(system_part_string)?strlen(arg):(sizeof(system_part_string)-1));
    MSYS_WARN("sysp: %s\n",system_part_string);
    return 0;
}
static int __init setup_data_part_string(char *arg)
{
    memcpy(data_part_string,(arg+1),strlen(arg)<sizeof(data_part_string)?strlen(arg):(sizeof(data_part_string)-1));
    MSYS_WARN("data: %s\n",data_part_string);

    return 0;
}

__setup("sysp",setup_system_part_string);
__setup("datap",setup_data_part_string);
#endif

#ifdef CONFIG_MS_US_TICK_API
static ssize_t us_ticks_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%llu\n",(u64)Chip_Get_US_Ticks());

    return (str - buf);
}
DEVICE_ATTR(us_ticks, 0444, us_ticks_show, NULL);
#endif

static ssize_t alloc_dmem(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        int error = 0;
        char name[15];
        unsigned int size;
        const char *str = buf;
        MSYS_DMEM_INFO mem_info;
        memset(mem_info.name,0,16);
        mem_info.phys=0;
        /* parsing input data */
        sscanf(buf, "%s %d", name, &size);

        while (*str && !isspace(*str)) str++;

        len = str - buf;
        if (!len) return -EINVAL;

        len=(len<15)?len:15;
        memcpy(mem_info.name,buf,len);

        if(0!=size)
        {
            mem_info.length=size;
            error = msys_request_dmem(&mem_info);
        }
        else
        {
            //MSYS_ERROR("Error size is NULL\n");
            return -EINVAL;
        }
        // for MIU BIST only
        if (strcmp(mem_info.name, "MIU_TEST") == 0)
        {
            CLRREG16(BASE_REG_MIU_PA + REG_ID_69, 0xFF);
            OUTREG16(BASE_REG_MIU_PA + REG_ID_71, (mem_info.phys - 0x20000000) >> 13); // test base address[15:0]
            CLRREG16(BASE_REG_MIU_PA + REG_ID_6F, BIT2|BIT3);   // test base address[17:16]
            OUTREG16(BASE_REG_MIU_PA + REG_ID_72, (mem_info.length >> 4) & 0xFFFF); // test length[15:0]
            OUTREG16(BASE_REG_MIU_PA + REG_ID_73, (mem_info.length >> 20) & 0x0FFF); // test length[27:16]
        }
        return n;
    }
    return n;
}
DEVICE_ATTR(dmem_alloc, 0200, NULL, alloc_dmem);

static ssize_t dmem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    struct list_head *ptr;
    struct DMEM_INFO_LIST *entry;
    int i=0;
    unsigned int total=0;

    list_for_each(ptr, &kept_mem_head)
    {
        entry = list_entry(ptr, struct DMEM_INFO_LIST, list);
        str += scnprintf(str, end - str, "%04d : 0x%08X@%08X [%s]\n",i,(unsigned int)entry->dmem_info.length,(unsigned int)entry->dmem_info.phys,entry->dmem_info.name);

        total+=(unsigned int)entry->dmem_info.length;
        i++;
    }

    str += scnprintf(str, end - str, "\nTOTAL: 0x%08X\n\n",total);

    return (str - buf);
}
DEVICE_ATTR(dmem, 0444, dmem_show, NULL);

static ssize_t release_dmem_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        int error = 0;
        const char *str = buf;
        MSYS_DMEM_INFO mem_info;
        memset(mem_info.name,0,16);
        mem_info.phys=0;

        while (*str && !isspace(*str)) str++;

        len = str - buf;
        if (!len) return -EINVAL;

        len=(len<15)?len:15;
        memcpy(mem_info.name,buf,len);

        error = msys_release_dmem(&mem_info);
        return n;
    }

    return 0;
}
DEVICE_ATTR(release_dmem, 0200, NULL, release_dmem_store);

#ifdef CONFIG_MSYS_DMEM_SYSFS_ALL
static ssize_t dmem_realloc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", dmem_realloc_enabled);
    return (str - buf);
}

static ssize_t dmem_realloc_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        const char *str = buf;

        while (*str && !isspace(*str)) str++;
        len = str - buf;

        if(1==len)
        {
            if('0'==buf[0])
            {
                dmem_realloc_enabled=0;
                MSYS_PRINT("dmem realloc disabled\n");
            }
            else if('1'==buf[0])
            {
                dmem_realloc_enabled=1;
                MSYS_PRINT("dmem realloc enabled\n");
            }
            return n;
        }
        return -EINVAL;
    }
    return -EINVAL;
}
DEVICE_ATTR(dmem_realloc, 0644, dmem_realloc_show, dmem_realloc_store);

static ssize_t unfix_dmem_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        int error = 0;
        const char *str = buf;
        char nbuf[16]={0};

        while (*str && !isspace(*str)) str++;

        len = str - buf;
        if (!len) return -EINVAL;

        len=(len<15)?len:15;
        memcpy(nbuf,buf,len);

        error = msys_unfix_dmem(nbuf);
        return error ? error : n;
    }

    return 0;
}
DEVICE_ATTR(unfix_dmem, 0200, NULL, unfix_dmem_store);

static ssize_t fixed_dmem_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        int error = 0;
        const char *str = buf;
        char nbuf[16]={0};

        while (*str && !isspace(*str)) str++;

        len = str - buf;
        if (!len) return -EINVAL;

        len=(len<15)?len:15;
        memcpy(nbuf,buf,len);

        if(1==len){
            if('0'==nbuf[0]){
                fixed_dmem_enabled=0;
                MSYS_ERROR("fix_dmem disabled\n" ) ;
            }else if('1'==nbuf[0]){
                fixed_dmem_enabled=1;
                MSYS_ERROR("fix_dmem enabled\n" ) ;
            }
        }

        error = msys_fix_dmem((char *)nbuf);
        return error ? error : n;
    }

    return 0;
}

static ssize_t fixed_dmem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    struct list_head *ptr;
    struct DMEM_INFO_LIST *entry;
    int i=0;

    list_for_each(ptr, &fixed_mem_head)
    {
        entry = list_entry(ptr, struct DMEM_INFO_LIST, list);
        str += scnprintf(str, end - str, "%04d: %s\n",i,entry->dmem_info.name);
        i++;
    }
    if (str > buf)  str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

DEVICE_ATTR(fixed_dmem, 0644, fixed_dmem_show, fixed_dmem_store);

static ssize_t dmem_retry_count_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        const char *str = buf;
        while (*str && !isspace(*str)) str++;
        len = str - buf;
        if(len)
        {
            dmem_retry_count = simple_strtoul(buf, NULL, 10);
            //MSYS_ERROR("dmem_retry_count=%d\n", dmem_retry_count);
            return n;
            /*
            if('0'==buf[0])
            {
                cma_monitor_enabled=0;
                return n;
            }
            else if('1'==buf[0])
            {
                cma_monitor_enabled=1;
                return n;
            }
            else
            {
                return -EINVAL;
            }*/
        }
        return -EINVAL;
    }
    return -EINVAL;
}

static ssize_t dmem_retry_count_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", dmem_retry_count);
    return (str - buf);
}
DEVICE_ATTR(dmem_retry_count, 0644, dmem_retry_count_show, dmem_retry_count_store);
#endif //END of CONFIG_MSYS_DMEM_SYSFS_ALL

#ifdef CONFIG_MS_CPU_FREQ
static ssize_t TEMP_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "Temperature %d\n", ms_get_temp());

    return (str - buf);
}

DEVICE_ATTR(TEMP_R, 0444, TEMP_show, NULL);
#endif

static ssize_t ms_dump_chip_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "Chip_Version: %d\n", msys_get_chipVersion());

    return (str - buf);
}

DEVICE_ATTR(CHIP_VERSION, 0444, ms_dump_chip_version, NULL);


#ifdef CONFIG_SS_PROFILING_TIME
extern void recode_timestamp(int mark, const char* name);
extern void recode_show(void);
static ssize_t profiling_booting_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        const char *str = buf;
        int mark=0;

        while (*str && !isspace(*str)) str++;
        len = str - buf;

        if(len)
        {
            mark = simple_strtoul(buf, NULL, 10);
            recode_timestamp(mark, "timestamp");
            return n;

        }
        return -EINVAL;
    }
    return -EINVAL;
}

static ssize_t profiling_booting_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    recode_show();
    return 0;
}
DEVICE_ATTR(booting_time, 0644, profiling_booting_show, profiling_booting_store);
#endif


#ifdef CONFIG_MSYS_REQUEST_PROC
struct list_head proc_info_head;
static struct mutex proc_info_mutex;
static struct proc_dir_entry* proc_class=NULL;
static struct proc_dir_entry* proc_zen_kernel=NULL;

struct proc_dir_entry* msys_get_proc_class(void)
{
	return proc_class;
}

struct proc_dir_entry* msys_get_proc_zen_kernel(void)
{
	return proc_zen_kernel;
}
EXPORT_SYMBOL(msys_get_proc_class);

static int msys_seq_show(struct seq_file*m, void *p)
{
  struct inode *inode = (struct inode *)m->private;

  PROC_INFO_LIST *proc_list = proc_get_parent_data(inode);
  MSYS_PROC_ATTRIBUTE* proc_attr = PDE_DATA(inode);

  switch(proc_attr->type)
  {
    case MSYS_PROC_ATTR_CHAR:
      seq_printf(m, "%c\n", *(unsigned int *)(proc_list->proc_addr +  proc_attr->offset));
      break;
    case MSYS_PROC_ATTR_UINT:
      seq_printf(m, "%u\n", *(unsigned int *)(proc_list->proc_addr +  proc_attr->offset));
      break;
    case MSYS_PROC_ATTR_INT:
      seq_printf(m, "%d\n", *(int *)(proc_list->proc_addr +  proc_attr->offset));
      break;
    case MSYS_PROC_ATTR_XINT:
      seq_printf(m, "0x%x\n", *(unsigned int *)(proc_list->proc_addr +  proc_attr->offset));
      break;
    case MSYS_PROC_ATTR_ULONG:
      seq_printf(m, "%lu\n", *(unsigned long *)(proc_list->proc_addr +  proc_attr->offset));
      break;
    case MSYS_PROC_ATTR_LONG:
      seq_printf(m, "%ld\n", *(long *)(proc_list->proc_addr +  proc_attr->offset));
      break;
    case MSYS_PROC_ATTR_XLONG:
      seq_printf(m, "0x%lx\n", *(unsigned long *)(proc_list->proc_addr +  proc_attr->offset));
      break;
    case MSYS_PROC_ATTR_ULLONG:
      seq_printf(m, "%llu\n", *(unsigned long long *)(proc_list->proc_addr +  proc_attr->offset));
      break;
    case MSYS_PROC_ATTR_LLONG:
      seq_printf(m, "%lld\n", *(long long *)(proc_list->proc_addr +  proc_attr->offset));
      break;
    case MSYS_PROC_ATTR_XLLONG:
      seq_printf(m, "0x%llx\n", *(unsigned long long*)(proc_list->proc_addr +  proc_attr->offset));
      break;
    case MSYS_PROC_ATTR_STRING:
      seq_printf(m, "%s\n", (unsigned char *)(proc_list->proc_addr +  proc_attr->offset));
      break;
    default:
      break;
  }

  return 0;
}

static int msys_proc_open(struct inode *inode, struct file *file)
{
     return single_open(file, msys_seq_show, inode);
}

static int msys_proc_mmap(struct file *file, struct vm_area_struct *vma)
{
  int ret = 0;
  struct page *page = NULL;

  struct inode *inode = (struct inode *)(((struct seq_file *)file->private_data)->private);

  PROC_INFO_LIST *proc_list = proc_get_parent_data(inode);

	size_t size = vma->vm_end - vma->vm_start;

  if (size > proc_list->proc_dev.size)
  {
      MSYS_ERROR("msys_proc_mmap - invalid size = %d\n", size);
      return -EINVAL;
  }

  page = virt_to_page((unsigned long)proc_list->proc_addr + (vma->vm_pgoff << PAGE_SHIFT));
  ret = remap_pfn_range(vma, vma->vm_start, page_to_pfn(page), size, vma->vm_page_prot);
  if (ret)
  {
    MSYS_ERROR("msys_proc_mmap - remap_pfn_range failed.\n");
    return ret;
  }
  //vma->vm_start = (unsigned long)info_addr;
	//vma->vm_end = vma->vm_start + PAGE_ALIGN(MAX_LEN);
	//vma->vm_flags |=  VM_SHARED | VM_WRITE | VM_READ;

  //vma->vm_ops = &rpr_vm_ops;

  //if (remap_page_range(start, page, PAGE_SIZE, PAGE_SHARED))
  //                      return -EAGAIN;
	return 0 ;
}


static const struct file_operations msys_proc_fops = {
 .owner = THIS_MODULE,
 .open  = msys_proc_open,
 .read  = seq_read,
 .llseek = seq_lseek,
 .release = single_release,
};

static const struct file_operations msys_proc_mmap_fops = {
 .owner = THIS_MODULE,
 .open = msys_proc_open,
 .mmap = msys_proc_mmap,
 .release = single_release,
};

static PROC_INFO_LIST *msys_get_proc_info(MSYS_PROC_DEVICE* proc_dev)
{
    struct list_head *tmp_proc_entry = NULL;
    PROC_INFO_LIST *tmp_proc_info = NULL;

    list_for_each(tmp_proc_entry, &proc_info_head) {
        tmp_proc_info = list_entry(tmp_proc_entry, PROC_INFO_LIST, list);
        if (tmp_proc_info->proc_dev.parent == proc_dev->parent
            && strcmp(tmp_proc_info->proc_dev.name, proc_dev->name) == 0) {
            //MSYS_ERROR("%s: Find %s handle = %p\n", __func__, proc_dev->name, tmp_proc_info);
            return tmp_proc_info;
        }
    }
    return NULL;
}

static PROC_INFO_LIST *msys_get_child_proc_info(PROC_INFO_LIST *parent_proc_info)
{
    struct list_head *tmp_proc_entry = NULL;
    PROC_INFO_LIST *tmp_proc_info = NULL;

    list_for_each(tmp_proc_entry, &proc_info_head) {
        tmp_proc_info = list_entry(tmp_proc_entry, PROC_INFO_LIST, list);
        if (tmp_proc_info->proc_dev.parent == parent_proc_info) {
            //MSYS_ERROR("%s; Find %s has child %s = %p\n", __func__, parent_proc_info->proc_dev.name, tmp_proc_info->proc_dev.name, tmp_proc_info);
            return tmp_proc_info;
        }
    }
    return NULL;
}


static int msys_request_proc_attr(MSYS_PROC_ATTRIBUTE* proc_attr)
{
    int err = 0;
    struct proc_dir_entry* tmp_proc_entry;
    PROC_INFO_LIST *parent_proc_info;
    MSYS_PROC_ATTRIBUTE *new_proc_attr;

    mutex_lock(&proc_info_mutex);
    if(/*proc_attr->name != NULL &&*/ proc_attr->name[0] != 0) {
        new_proc_attr = (MSYS_PROC_ATTRIBUTE *)kmalloc(sizeof(MSYS_PROC_ATTRIBUTE), GFP_KERNEL);
        if (!new_proc_attr) {
            MSYS_ERROR("kmalloc MSYS_PROC_ATTRIBUTE failed!!\n" );
            BUG();
        }
        *new_proc_attr = *proc_attr; //It will be freed when release device/attributes.

        parent_proc_info = new_proc_attr->handle;
        tmp_proc_entry = proc_create_data(new_proc_attr->name, 0, parent_proc_info->proc_entry, &msys_proc_fops, new_proc_attr);
        if (!tmp_proc_entry) {
            //MSYS_ERROR("Skip since attribute %s exists\n", proc_attr->name);
            err = -EEXIST;
            kfree(new_proc_attr);
        } else {
            //MSYS_ERROR("Set attribute %s handle = %p\n", proc_attr->name, proc_attr->handle);
        }
    }
    mutex_unlock(&proc_info_mutex);
    return err;
}

static int msys_release_proc_attr(MSYS_PROC_ATTRIBUTE* proc_attr)
{
    return 0;
}

static int msys_request_proc_dev(MSYS_PROC_DEVICE* proc_dev)
{
    int err = 0;
    PROC_INFO_LIST *new_proc_info = NULL;

    mutex_lock(&proc_info_mutex);

    if(/*proc_dev->name != NULL && */proc_dev->name[0] != 0) {
        if((proc_dev->handle = msys_get_proc_info(proc_dev)) != NULL) {
            //MSYS_ERROR("Device proc_info %s exist, return original handle = %p\n" , proc_dev->name, proc_dev->handle);
            err = -EEXIST;
            goto GG;
        }

        new_proc_info = (PROC_INFO_LIST *)kmalloc(sizeof(PROC_INFO_LIST), GFP_KERNEL);
        if (!new_proc_info) {
            MSYS_ERROR("kmalloc PROC_INFO_LIST failed!!\n" );
            err = -ENOMEM;
            goto GG;
        }

        new_proc_info->proc_entry = proc_mkdir_data(proc_dev->name, 0,
            (proc_dev->parent)?((PROC_INFO_LIST *)proc_dev->parent)->proc_entry:proc_class, new_proc_info);
        if (!new_proc_info->proc_entry) {
            MSYS_ERROR("Skip since device proc_entry %s exists\n", proc_dev->name);
            err = -EEXIST;
            kfree(new_proc_info);
            goto GG;
        }

        if (proc_dev->parent && proc_dev->size == 0) { //subdevice case
            new_proc_info->proc_addr = ((PROC_INFO_LIST *)proc_dev->parent)->proc_addr;
        }
        else { //device case
            if (proc_dev->size & ~PAGE_MASK) {
                proc_dev->size &= PAGE_MASK;
                proc_dev->size += PAGE_SIZE;
                //MSYS_ERROR("Size not align with %ld, resize to %ld\n", PAGE_SIZE, proc_dev->size);
            }
            if(proc_dev->size > KMALLOC_MAX_SIZE)
            {
                MSYS_ERROR("allocate %lu kernel memory for proc data error\n", proc_dev->size);
                err = -ENOMEM;
                kfree(new_proc_info);
                goto GG;
            }
            new_proc_info->proc_addr = kmalloc(proc_dev->size, GFP_KERNEL);
            if(!new_proc_info->proc_addr) {
                MSYS_ERROR("allocate %lu kernel memory for proc data error\n", proc_dev->size);
                err = -ENOMEM;
                kfree(new_proc_info);
                goto GG;
            }
            proc_create(".mmap", 0, new_proc_info->proc_entry, &msys_proc_mmap_fops); //It will be freed when relealse device.
        }

        proc_dev->handle = new_proc_info;
        new_proc_info->proc_dev = *proc_dev;
        list_add(&new_proc_info->list, &proc_info_head);
        //MSYS_ERROR("Set device %s handle = %p\n", new_proc_info->proc_dev.name, new_proc_info->proc_dev.handle);
    }
GG:
    mutex_unlock(&proc_info_mutex);
    return err;
}

static int msys_release_proc_dev(MSYS_PROC_DEVICE* proc_dev)
{
    int err = 0;
    PROC_INFO_LIST *tmp_proc_info = NULL;
    PROC_INFO_LIST *target_proc_info = NULL;
    PROC_INFO_LIST *parent_proc_info = NULL;
    PROC_INFO_LIST *child_proc_info = NULL;

    target_proc_info = msys_get_proc_info(proc_dev);
    mutex_lock(&proc_info_mutex);
    if(target_proc_info == NULL) {
        MSYS_ERROR("%s: Cannot find handle of %s\n", __func__, proc_dev->name);
        err = -ENODEV;
    } else {
        //Remove proc_entry
        proc_remove(target_proc_info->proc_entry);
        tmp_proc_info = target_proc_info;
        //Find all proc_info's child from proc_info_list and remove proc_info from bottom which doesn't have child.
        do {
            child_proc_info = msys_get_child_proc_info(tmp_proc_info);
            if(child_proc_info == NULL) {
                parent_proc_info = tmp_proc_info->proc_dev.parent;
                //MSYS_ERROR("%s: Free %s handle = %p\n", __func__, tmp_proc_info->proc_dev.name, tmp_proc_info->proc_dev.handle);
                __list_del_entry(&tmp_proc_info->list);
                kfree(tmp_proc_info);
                if(tmp_proc_info != target_proc_info) {
                    tmp_proc_info = parent_proc_info;
                } else {
                    break;
                }
            }else
                tmp_proc_info = child_proc_info;
        } while(1);
    }
    mutex_unlock(&proc_info_mutex);
    return err;
}
#endif
static struct class *msys_sysfs_class = NULL;

struct class *msys_get_sysfs_class(void)
{
  if (!msys_sysfs_class)
  {
      msys_sysfs_class = class_create(THIS_MODULE, "mstar");
      if (!msys_sysfs_class)
        MSYS_ERROR("cannot get class for sysfs\n");
  }
  return msys_sysfs_class;
}
EXPORT_SYMBOL(msys_get_sysfs_class);
#if defined(CONFIG_ARCH_INFINITY2)
extern int  msys_dma_copy(MSYS_DMA_COPY *cfg);
extern int msys_dma_fill(MSYS_DMA_FILL *pstDmaCfg);
extern int msys_dma_blit(MSYS_DMA_BLIT *pstMdmaCfg);

#if defined(CONFIG_MS_MOVE_DMA)
static ssize_t movedma(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
	if(NULL!=buf)
	{
            unsigned long long phyaddr_src; // MIU address of source
            unsigned long long phyaddr_dst; // MIU address of destination
            unsigned int lineofst_src;      // line-offset of source, set 0 to disable line offset
            unsigned int lineofst_dst;      // line-offset of destination, set 0 to disable line offset
            unsigned int width_src;         // width of source, set 0 to disable line offset
            unsigned int width_dst;         // width of destination, set 0 to disable line offset
            unsigned int length;            // total size (bytes)
            MSYS_DMA_BLIT dmaBlit_info;
            /* parsing input data */
            sscanf(buf, "%llu %llu %d %d %d %d %d", &phyaddr_src, &phyaddr_dst, &lineofst_src, &lineofst_dst, &width_src, &width_dst, &length);

            dmaBlit_info.phyaddr_src=phyaddr_src;
            dmaBlit_info.phyaddr_dst=phyaddr_dst;
            dmaBlit_info.lineofst_src=lineofst_src;
            dmaBlit_info.lineofst_dst=lineofst_dst;
            dmaBlit_info.width_src=width_src;
            dmaBlit_info.width_dst=width_dst;
            dmaBlit_info.length=length;
            msys_dma_blit(&dmaBlit_info);
            printk("msys_dma_blit end \n");
	}
	return n;
}
DEVICE_ATTR(movedma, 0200, NULL, movedma);
#endif

static ssize_t bytedmacp(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
	if(NULL!=buf)
	{
            unsigned long long phyaddr; // MIU address of source
            unsigned long long desaddr; // MIU address of source
//            unsigned int pattern;         // width of destination, set 0 to disable line offset
            unsigned int length;            // total size (bytes)
            MSYS_DMA_COPY dmaCopy_info;
            /* parsing input data */
            sscanf(buf, "%llu %d %llu", &phyaddr, &length, &desaddr);
            printk("phyaddr=%llu length=%x desaddr=%llu\n", phyaddr, length, desaddr);
            dmaCopy_info.phyaddr_src=phyaddr;
            dmaCopy_info.length=length;
            dmaCopy_info.phyaddr_dst=desaddr;
            msys_dma_copy(&dmaCopy_info);
            printk("msys_dma_copy end \n");
	}
	return n;
}
DEVICE_ATTR(bytedmacp, 0200, NULL, bytedmacp);

static ssize_t bytedma(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
	if(NULL!=buf)
	{
            unsigned long long phyaddr; // MIU address of source
            unsigned int pattern;         // width of destination, set 0 to disable line offset
            unsigned int length;            // total size (bytes)
            MSYS_DMA_FILL dmaFill_info;
            /* parsing input data */
            sscanf(buf, "%llu %d %d", &phyaddr, &length, &pattern);
            printk("phyaddr=%llu length=%x pattern=%x\n", phyaddr, length, pattern);
            dmaFill_info.phyaddr=phyaddr;
            dmaFill_info.length=length;
            dmaFill_info.pattern=pattern;
            msys_dma_fill(&dmaFill_info);
            printk("msys_dma_fill end \n");
	}
	return n;
}
DEVICE_ATTR(bytedma, 0200, NULL, bytedma);

#endif
static int __init msys_init(void)
{
    int ret;

    //ret = misc_register(&sys_dev);
    ret = register_chrdev(MAJOR_SYS_NUM, "msys", &msys_fops);
    if (ret != 0) {
        MSYS_ERROR("cannot register msys  (err=%d)\n", ret);
    }

    sys_dev.this_device = device_create(msys_get_sysfs_class(), NULL,
	    MKDEV(MAJOR_SYS_NUM, MINOR_SYS_NUM), NULL, "msys");

    sys_dev.this_device->dma_mask=&sys_dma_mask;
    sys_dev.this_device->coherent_dma_mask=sys_dma_mask;

    mutex_init(&dmem_mutex);
    INIT_LIST_HEAD(&kept_mem_head);
    INIT_LIST_HEAD(&fixed_mem_head);

    device_create_file(sys_dev.this_device, &dev_attr_dmem);
    device_create_file(sys_dev.this_device, &dev_attr_dmem_alloc);
    device_create_file(sys_dev.this_device, &dev_attr_release_dmem);
#ifdef CONFIG_MSYS_DMEM_SYSFS_ALL
    device_create_file(sys_dev.this_device, &dev_attr_fixed_dmem);
    device_create_file(sys_dev.this_device, &dev_attr_unfix_dmem);
    device_create_file(sys_dev.this_device, &dev_attr_dmem_retry_count);
    device_create_file(sys_dev.this_device, &dev_attr_dmem_realloc);
#endif

#ifdef CONFIG_MS_PIU_TICK_API
    device_create_file(sys_dev.this_device, &dev_attr_PIU_T);
#endif

#ifdef CONFIG_MS_CPU_FREQ
    device_create_file(sys_dev.this_device, &dev_attr_TEMP_R);
#endif

    device_create_file(sys_dev.this_device, &dev_attr_CHIP_VERSION);

#ifdef CONFIG_MS_US_TICK_API
    device_create_file(sys_dev.this_device, &dev_attr_us_ticks);
#endif

#ifdef CONFIG_SS_PROFILING_TIME
    device_create_file(sys_dev.this_device, &dev_attr_booting_time);
#endif
#if defined(CONFIG_ARCH_INFINITY2)
#if defined(CONFIG_MS_MOVE_DMA)
    device_create_file(sys_dev.this_device, &dev_attr_movedma);
#endif
    device_create_file(sys_dev.this_device, &dev_attr_bytedma);
    device_create_file(sys_dev.this_device, &dev_attr_bytedmacp);
#endif
#if defined(CONFIG_PROC_FS) && defined(CONFIG_MSYS_REQUEST_PROC)
    mutex_init(&proc_info_mutex);
    INIT_LIST_HEAD(&proc_info_head);
    proc_class=proc_mkdir("mstar",NULL);
    proc_zen_kernel=proc_mkdir("kernel",proc_class);
#endif

    return 0;
}

#ifdef CONFIG_MSYS_KFILE_API
//!!!! msys_kfile_* API has not been tested as they are not used. 2016/07/18
struct file* msys_kfile_open(const char* path, int flags, int rights)
{
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if(IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}
EXPORT_SYMBOL(msys_kfile_open);

void msys_kfile_close(struct file* fp)
{
    if(fp)
    {
        filp_close(fp,NULL);
    }
}
EXPORT_SYMBOL(msys_kfile_close);

int msys_kfile_write(struct file* fp, unsigned long long offset, unsigned char* data, unsigned int size)
{
    mm_segment_t oldfs;
    int ret=-EINVAL;

    if(fp)
    {
        oldfs = get_fs();
        set_fs(get_ds());
        ret = vfs_write(fp, data, size, &offset);
        set_fs(oldfs);
    }
    return ret;
}
EXPORT_SYMBOL(msys_kfile_write);

int msys_kfile_read(struct file* fp, unsigned long long offset, unsigned char* data, unsigned int size)
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(fp, data, size, &offset);

    set_fs(oldfs);
    return ret;
}
EXPORT_SYMBOL(msys_kfile_read);
#endif

#if 0
int ssys_get_HZ(void)
{
    return HZ;
}
EXPORT_SYMBOL(ssys_get_HZ);
#endif

subsys_initcall(msys_init);


MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SYSTEM driver");
MODULE_LICENSE("SSTAR");
