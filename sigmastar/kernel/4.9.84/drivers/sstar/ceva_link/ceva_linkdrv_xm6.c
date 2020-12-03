/*
* ceva_linkdrv_xm6.c- Sigmastar
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
 * ceva_linkdrv_xm6.c
 *
 * Created on: Sep 12, 2013
 * Author: Ido Reis <ido.reis@tandemg.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/dma-mapping.h>
#include <asm/page.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/io.h>
#include <asm/cacheflush.h>

#include "ceva_linkdrv-generic.h"
#include "protected_mem_db.h"
#include "ceva_linkdrv.h"
#include "ceva_linkdrv_xm6.h"
#include "drv_debug.h"
#include "ceva_linkdrv_proc.h"

#include "boot_loader.h"
#include "ms_platform.h"
#include "mstar_chip.h"
//#include "ms_version.h"


#define XM6_DRV_DEVICE_COUNT    (1) // How many device will be installed
#define XM6_DRV_NAME            "mstar_dsp"
#define XM6_DRV_MINOR           (0)
#define XM6_DRV_CLASS_NAME      "mstar_dsp_class"

#define HPRINTF_MEM_SIZE        (16*4096)
// #define HPRINTF_DUMP_AFTER_RELEASE

#define MCCI_LINK_EVENT_FROM_HOST   0
#define MCCI_LINK_EVENT_FROM_DSP    1
#define MCCI_PRINT_FROM_DSP         2
#define MCCI_IRQ_UNEXPECT           0xFFFFFFF9 // only bit 1 & 2 will be trigger

MODULE_AUTHOR("Ido Reis <ido.reis@tandemg.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.1");
MODULE_DESCRIPTION("mapped pci character device driver");

//#define FUNC_SHOULD_REMOVE

static int xm6_drv_open(struct inode *inode, struct file *filp);
static int xm6_drv_release(struct inode *inode, struct file *filp);
static ssize_t xm6_drv_read(struct file *filp, char __user *buf, size_t count, loff_t *pos);
static ssize_t xm6_drv_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos);
static long xm6_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static void xm6_drv_dma_vma_open(struct vm_area_struct *vma);
static void xm6_drv_dma_vma_close(struct vm_area_struct *vma);
static int xm6_drv_mmap_dma(struct xm6_dev_data *dev_data, struct vm_area_struct *vma);
static int xm6_drv_mmap(struct file *filp, struct vm_area_struct *vma);
static void xm6_drv_cleanup_module(struct platform_device *plat_dev, struct xm6_dev_data *dev_data);
static int xm6_drv_setup_cdev(struct platform_device *plat_dev, struct xm6_dev_data *dev_data);
static int ceva_linkdrv_init_module(struct platform_device *plat_dev, struct xm6_dev_data *dev_data);
static void xm6_drv_dump_dsp_log(struct xm6_dev_data *dev_data);
//static u8 xm6_drv_get_revision(struct platform_device *plat_dev);
//static u8 xm6_drv_get_irq(struct platform_device *plat_dev);
//static struct platform_device* find_upstream_dev(struct platform_device *plat_dev);
//static void retrain_gen2(struct platform_device *plat_dev, struct xm6_dev_data *dev_data);
//static int probe_enable_pci_dev(struct platform_device *plat_dev);
//static void remove_disable_pci_dev(struct platform_device *plat_dev);
static int probe_get_hw_prop(struct platform_device* plat_dev, struct xm6_dev_data *dev_data);
static int ceva_link_generate_irq(struct xm6_dev_data *dev_data, ceva_event_t e);
//static int scan_bars(struct platform_device *plat_dev, struct xm6_dev_data *dev_data);
//static int map_bars(struct platform_device *plat_dev, struct xm6_dev_data *dev_data);
//static void free_bars(struct platform_device *plat_dev, struct xm6_dev_data *dev_data);
//static int probe_map_bars(struct platform_device *plat_dev, struct xm6_dev_data *dev_data);
//static void probe_map_dma(struct xm6_dev_data *dev_data);
static irqreturn_t ceva_linkdrv_interrupt(int irq, void *dev_id);
static int probe_init_irq(struct platform_device *plat_dev, struct xm6_dev_data *dev_data);
static void remove_deinit_irq(struct platform_device *plat_dev, struct xm6_dev_data *dev_data);
static int xm6_drv_probe(struct platform_device *plat_dev);
static s32 xm6_drv_remove(struct platform_device *plat_dev);
int __init xm6_drv_init_module(void);
void __exit xm6_drv_exit_module(void);
static void Init_XM6FW_VerInfo(void);
static void dump_XM6VerInfo(struct Version_Info *pVerInfo);

static ceva_hal_handle *pGceva_hal;

static unsigned int SendDSPCount=0;
static unsigned int GotDSPCount=0;

/*!
 * ops for dma mapped memory
 */
static struct vm_operations_struct xm6_drv_dma_vma_ops = {
        .open = xm6_drv_dma_vma_open,
        .close = xm6_drv_dma_vma_close
};

/*!
 * fops for this driver's device
 */
static struct file_operations xm6_drv_fops = {
        .owner = THIS_MODULE,
        .read = xm6_drv_read,
        .write = xm6_drv_write,
        .open = xm6_drv_open,
        .release = xm6_drv_release,
        .unlocked_ioctl = xm6_drv_ioctl,
        .llseek = no_llseek, /* This driver doesn't support llseek */
        .mmap = xm6_drv_mmap,
};

/*!
 * xm6 driver module fops
 */
static const struct of_device_id xm6_drv_match[] = {
    {
        .compatible = "sstar,infinity2-xm6",
        /*.data = NULL,*/
    },
    {},
};

static struct platform_driver xm6_drv_driver = {
    .probe      = xm6_drv_probe,
    .remove     = xm6_drv_remove,
    // .suspend    = xm6_drv_suspend,
    // .resume     = xm6_drv_resume,

    .driver = {
        .of_match_table = of_match_ptr(xm6_drv_match),
        .name   = "sstar_xm6",
        .owner  = THIS_MODULE,
    }
};

/* Use a struct to gather all global variable when registering a char device*/
static struct
{
    int major;              // cdev major number
    int minor_star;         // begining of cdev minor number
    int reg_count;          // registered count
    struct class *class;    // class pointer
} xm6_drv_data = {0, 0, 0, NULL};

static struct Version_Info *pVerInfo;
static struct Version_Info VerInfo;

static void dump_XM6VerInfo(struct Version_Info *pVerInfo)
{
  XM6_MSG(XM6_MSG_DBG, "Version_Info CEVA_VInfo Main.Second.Sub : %d.%d.%d \n", pVerInfo->CEVA_VInfo.Main,pVerInfo->CEVA_VInfo.Second,pVerInfo->CEVA_VInfo.Sub);
  XM6_MSG(XM6_MSG_DBG, "Version_Info FormalReleaseFlg: %d \n", pVerInfo->FormalReleaseFlg);
  XM6_MSG(XM6_MSG_DBG, "Version_Info GitBranch: %s \n", pVerInfo->GitBranch);
  XM6_MSG(XM6_MSG_DBG, "Version_Info GitVersion: %s \n", pVerInfo->GitVersion);
  XM6_MSG(XM6_MSG_DBG, "Version_Info BuildCodeData: %s \n", pVerInfo->BuildCodeData);
  return;
}

static void Init_XM6FW_VerInfo(void)
{
  pVerInfo=&VerInfo;

  memset(pVerInfo,0,sizeof(struct Version_Info));

  pVerInfo->CEVA_VInfo.Main=CEVA_SRC_VERSION;
  pVerInfo->CEVA_VInfo.Second=CEVA_LOCAL_LIB_VERSION;
  pVerInfo->CEVA_VInfo.Sub=CEVA_SUBVERSION;

  pVerInfo->FormalReleaseFlg=SERVER_BUILD;
  memcpy(pVerInfo->GitBranch,CEVA_XM6_GIT_BRANCH,strlen(CEVA_XM6_GIT_BRANCH));
  memcpy(pVerInfo->GitVersion,CEVA_XM6_GIT_VERSION,strlen(CEVA_XM6_GIT_VERSION));
  memcpy(pVerInfo->BuildCodeData,CEVA_XM6_BUILD_DATE,strlen(CEVA_XM6_BUILD_DATE));

  dump_XM6VerInfo(pVerInfo);
  return;
}

void Dump_ARMDSP_HndShakeInfo(void)
{
	unsigned int GPOut=0;
	
	XM6_MSG(XM6_MSG_ERR, "%s  \n",__FUNCTION__);
	XM6_MSG(XM6_MSG_ERR, "ARM Side View: SendDSPCount:0x%x GotDSPCount:0x%x \n",SendDSPCount,GotDSPCount);
	GPOut=dsp_ceva_hal_read_gpout(pGceva_hal);
	XM6_MSG(XM6_MSG_ERR, "DSP Side View: DSPStatus:0x%x DSPProcessCount:0x%x \n",((GPOut>>24)&0xff),(GPOut&0xffffff));
	return;
}


/*!
 * open handler
 *
 * called when user opens the device.
 * each open call increment the ref_count variable.
 * when opened for the first time (e.g ref_count = 1) it initializes the
 * ceva_linkdrv logic module.
 * for each call, it also executes the ceva_linkdrv_open_cb to notify the
 * logical module of the user operation.
 * @param [in] inode
 * @param [in] filp
 * @return zero for success, nonzero for failure
 * @see ceva_linkdrv_init(), ceva_linkdrv_open_cb()
 */
static int xm6_drv_open(struct inode *inode, struct file *filp) {
    int ret = 0;
#if 0
    unsigned int mj = imajor(inode);
    unsigned int mn = iminor(inode);
#endif
    struct xm6_dev_data *dev_data = NULL;

#if 0
    if (mj != xm6_drv_major || mn < 0 ||
            mn >= CEVA_PCIDEV_NUMBER_OF_DEVICES) {
        XM6_MSG(XM6_MSG_DBG, "No device found with minor=%d and major=%d",
                mj, mn);
        ret = -ENODEV; /* No such device */
        goto exit;
    }
#endif

    /* store a pointer to struct cfake_dev here for other methods */
    dev_data = container_of(inode->i_cdev, struct xm6_dev_data, cdev);
    filp->private_data = dev_data;

    XM6_MSG(XM6_MSG_DBG, "open filp %p, device %p\n", filp, dev_data);

    if (inode->i_cdev != &dev_data->cdev) {
        XM6_MSG(XM6_MSG_WRN, "open: internal error\n");
        ret = -ENODEV; /* No such device */
        goto exit;
    }
    /* This method can't fail, so no need to check for return value */
    nonseekable_open(inode, filp);

    if (mutex_lock_interruptible(&dev_data->dev_mutex)) {
        return -EINTR;
    }

    if (dev_data->count == 0) {
        ret = ceva_linkdrv_init(&dev_data->linkdrv, dev_data);
    }

    if (!ret) {
        ret = ceva_linkdrv_open_cb(&dev_data->linkdrv);
        if (!ret) {
            dev_data->count++;
        } else {
            if (dev_data->count == 0) {
                ceva_linkdrv_deinit(&dev_data->linkdrv);
            }
        }
    }

    mutex_unlock(&dev_data->dev_mutex);

exit:
    return ret;
}

/*!
 * close handler
 *
 * called when user closes the device.
 * each open call decrement the ref_count variable.
 * for each call, it executes the ceva_linkdrv_release_cb to notify the
 * logical module of the user operation.
 * when closed for the last time (e.g ref_count = 0) it deinitializes the
 * ceva_linkdrv logic module.
 * @param [in] inode
 * @param [in] filp
 * @return zero for success, nonzero for failure
 * @see ceva_linkdrv_deinit(), ceva_linkdrv_release_cb()
 */
static int xm6_drv_release(struct inode *inode, struct file *filp) {
    int ret;
    struct xm6_dev_data *dev_data = (struct xm6_dev_data *) filp->private_data;

    XM6_MSG(XM6_MSG_DBG, "release filp %p, device %p\n", filp, dev_data);

    xm6_drv_dump_dsp_log(dev_data);

    if (mutex_lock_interruptible(&dev_data->dev_mutex)) {
        return -EINTR;
    }
    ret = ceva_linkdrv_release_cb(&dev_data->linkdrv);
    if (dev_data->count)
        dev_data->count--;
    if (!dev_data->count) {
        ceva_linkdrv_deinit(&dev_data->linkdrv);
    }
    mutex_unlock(&dev_data->dev_mutex);

    return ret;
}

/*!
 * read handler
 *
 * reads data from bar0 of the pci device, from a relative offset.
 * as the user interface does not include an offset argument, the user
 * has to write the offset value into the buf. the driver first read from
 * buf the offset value and user actual buffer address, and then reads
 * the data into the forwarded user supplied address.
 * @param [in] filp device handler
 * @param [in/out] buf output buffer to read data to
 * @param [in] count number of bytes to read
 * @param [in] pos offset (not used)
 * @return number of bytes read from bar0, negative values for errors
 * @see struct RWBuffer, xm6_drv_write()
 */
static ssize_t xm6_drv_read(struct file *filp, char __user *buf, size_t count, loff_t *pos) {
    struct xm6_dev_data *dev_data = (struct xm6_dev_data *) filp->private_data;
    struct RWBuffer rwbuf;
    ssize_t ret = 0;

    XM6_MSG(XM6_MSG_DBG, "read filp %p, device %p\n", filp, dev_data);

    if (!count) {
        return 0;
    }

    if (!access_ok(VERIFY_WRITE, buf, count)) {
        XM6_MSG(XM6_MSG_ERR, "access error\n");
        return -EFAULT;
    }

    if (mutex_lock_interruptible(&dev_data->dev_mutex)) {
        XM6_MSG(XM6_MSG_ERR, "unable to lock mutex\n");
        return -EINTR;
    }

    // read offset from buf
    if (copy_from_user(&rwbuf, buf, sizeof(rwbuf)) != 0) {
        XM6_MSG(XM6_MSG_ERR, "could not read from user buffer\n");
        ret = -EFAULT;
        goto out;
    }

#if 0 // ORIGINAL_PCI_BAR
    // read content of bar 0 to buf
    if (copy_to_user(rwbuf.buf,
            ((char *)dev_data->bar[0]) + rwbuf.offset, count) != 0) {
        XM6_MSG(XM6_MSG_ERR, "%s -> copy to user failed", __FUNCTION__);
        ret = -EFAULT;
        goto out;
    }
#else
    XM6_MSG(XM6_MSG_ERR, "copy_to_user bar[0], offset %lu, size %d\n", rwbuf.offset, count);
#endif
    *pos += ret;
    ret = count;
out:
    mutex_unlock(&dev_data->dev_mutex);
    return ret;
}

/*!
 * write handler
 *
 * writes data to bar0 of the pci device, to a relative offset.
 * as the user interface does not include an offset argument, the user
 * has to write the offset value into the buf. the driver first read from
 * buf the offset value and user actual buffer address, and then writes
 * the data from the user buffer to bar0 base address plus the offset.
 * @param [in] filp device handler
 * @param [in] buf output buffer to read data to
 * @param [in] count number of bytes to read
 * @param [in] pos offset (not used)
 * @return number of bytes read from bar0, negative values for errors
 * @see struct RWBuffer, xm6_drv_read()
 */
static ssize_t xm6_drv_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos) {
    struct xm6_dev_data *dev_data = (struct xm6_dev_data *) filp->private_data;
    struct RWBuffer rwbuf;
    ssize_t ret = 0;

    XM6_MSG(XM6_MSG_DBG, "write filp %p, device %p\n", filp, dev_data);

    if (!count) {
        return 0;
    }

    if (!access_ok(VERIFY_READ, buf, count)) {
        XM6_MSG(XM6_MSG_ERR, "access error\n");
        return -EFAULT;
    }

    if (mutex_lock_interruptible(&dev_data->dev_mutex)) {
        XM6_MSG(XM6_MSG_ERR, "unable to lock mutex\n");
        return -EINTR;
    }

    // read offset from buf
    if (copy_from_user(&rwbuf, buf, sizeof(rwbuf)) != 0) {
        XM6_MSG(XM6_MSG_ERR, "could not read from user buffer\n");
        ret = -EFAULT;
        goto out;
    }

#if 0 // ORIGINAL_PCI_BAR
    // write content of buf to bar 0
    if (copy_from_user(((char *)dev_data->bar[0]) + rwbuf.offset,
            rwbuf.buf, count) != 0) {
        XM6_MSG(XM6_MSG_ERR, "%s -> copy from user failed", __FUNCTION__);
        ret = -EFAULT;
        goto out;
    }
#else
    XM6_MSG(XM6_MSG_ERR, "copy_from_user bar[0], offset %lu, size %d\n", rwbuf.offset, count);
#endif

    *pos += count;
    ret = count;

out:
    mutex_unlock(&dev_data->dev_mutex);
    return ret;
}

static phys_addr_t xm6_drv_miu2phys(u64 miu_addr)
{
    if(miu_addr >= (u64)ARM_MIU2_BASE_ADDR)
    {
        return (phys_addr_t)(miu_addr + ARM_MIU2_BUS_BASE - ARM_MIU2_BASE_ADDR);
    }
    else if(miu_addr >= (u64)ARM_MIU1_BASE_ADDR)
    {
        return  (phys_addr_t)(miu_addr + ARM_MIU1_BUS_BASE - ARM_MIU1_BASE_ADDR);
    }
    else if(miu_addr >= (u64)ARM_MIU0_BASE_ADDR)
    {
        return (phys_addr_t)(miu_addr + ARM_MIU0_BUS_BASE - ARM_MIU0_BASE_ADDR);
    }

    return (phys_addr_t)-1;
}

static u64 xm6_drv_phys2miu(phys_addr_t phys_addr)
{
    if(phys_addr >= (phys_addr_t)ARM_MIU2_BUS_BASE)
    {
        return (u64)(phys_addr - ARM_MIU2_BUS_BASE+ARM_MIU2_BASE_ADDR);
    }
    else if(phys_addr >= (phys_addr_t)ARM_MIU1_BUS_BASE)
    {
        return  (u64)(phys_addr - ARM_MIU1_BUS_BASE+ARM_MIU1_BASE_ADDR);
    }
    else if(phys_addr >= (phys_addr_t)ARM_MIU0_BUS_BASE)
    {
        return (u64)(phys_addr - ARM_MIU0_BUS_BASE+ARM_MIU0_BASE_ADDR);
    }

    return (u64)-1ULL;
}

static void* xm6_drv_ioremap(phys_addr_t phys, int size)
{
    unsigned long aligned_size, start, end;
    struct vm_struct *area;
    void *addr;
    int err;

    aligned_size = PAGE_ALIGN(size);

    XM6_MSG(XM6_MSG_DBG, "phys: 0x%x, size: %d, aligned_size: %ld (%ld)\n", phys, size, aligned_size, PAGE_SIZE);

    area = get_vm_area(aligned_size, /*VM_USERMAP*//*VM_MAP*/VM_IOREMAP);
    if(!area)
    {
        XM6_MSG(XM6_MSG_ERR, "get_vm_area_caller failed\n");
        return NULL;
    }

    area->phys_addr = phys; // ?
    addr = area->addr;

    start = (unsigned long)addr;
    end = start + aligned_size;

    XM6_MSG(XM6_MSG_DBG, "area->phys_addr: 0x%lx, area->addr: %p, start: 0x%lx, end: 0x%lx\n", (unsigned long)area->phys_addr, area->addr, start, end);

    err = ioremap_page_range(start, end, area->phys_addr, pgprot_writecombine(PAGE_KERNEL)/*PAGE_KERNEL*/);
    if(err)
    {
        vunmap(addr);
        XM6_MSG(XM6_MSG_ERR, "ioremap_page_range failed\n");
        return NULL;
    }

    return addr;
}

static void xm6_drv_iounmap(void *virt)
{
    vunmap(virt);
}

static int xm6_drv_assign_dsp_buffer(dev_dsp_buf *dev_buf, struct boot_buffer_t *boot_buf, int need_virt)
{
    memset(dev_buf, 0, sizeof(dev_dsp_buf));

    if (boot_buf->size == 0 )
        return -ENOMEM;

    // From physical
    if (boot_buf->phys != 0) {
        dev_buf->is_from_miu = 0;
        dev_buf->size = boot_buf->size;
        dev_buf->phys = boot_buf->phys;

        // When user assign the physical address, it means the memory is from Linux managed heap.
        // So we can get the virtual address via phys_to_virt() directly and ioremap() does not work in this case.
        // Please refer https://stackoverflow.com/questions/43127794/why-shouldnt-i-use-ioremap-on-system-memory-for-armv6
        if (need_virt)
            dev_buf->virt = phys_to_virt(dev_buf->phys);

    // From MIU
    } else if (boot_buf->miu != 0) {
        dev_buf->is_from_miu = 1;
        dev_buf->size = boot_buf->size;
        dev_buf->phys = xm6_drv_miu2phys(boot_buf->miu);

        // When user assign the miu buffer, it means memory is out of Linux management.
        // So we get the virtual address from ioremap(), and phys_to_virt() does not work in this case.
        if (need_virt)
            dev_buf->virt = xm6_drv_ioremap(dev_buf->phys, dev_buf->size);

    } else {
        return -ENOMEM;
    }

    dev_buf->size = boot_buf->size;

    return 0;
}

static int xm6_drv_init_dsp_buffer(struct xm6_dev_data *dev_data, struct boot_config_t *boot_config)
{
    // share memory size check
    if (boot_config->share_mem.size <= HPRINTF_MEM_SIZE)
    {
        XM6_MSG(XM6_MSG_ERR, "share memory is too small.\n");
        return -ENOMEM;
    }

    // boot_image
    if (xm6_drv_assign_dsp_buffer(&dev_data->boot_image, &boot_config->boot_image, 0) != 0) {
        XM6_MSG(XM6_MSG_ERR, "boot image is not assigned.\n");
        return -ENOMEM;
    }

    // ext_heap
    if (xm6_drv_assign_dsp_buffer(&dev_data->ext_heap, &boot_config->ext_heap, 0) != 0) {
        XM6_MSG(XM6_MSG_ERR, "external heap is not assigned.\n");
        return -ENOMEM;
    }

    // work_buffer, it could be NULL
    xm6_drv_assign_dsp_buffer(&dev_data->work_buffer, &boot_config->work_buffer, 0);

    // share_mem
    if (xm6_drv_assign_dsp_buffer(&dev_data->share_mem, &boot_config->share_mem, 1) != 0) {
        XM6_MSG(XM6_MSG_ERR, "share memory is not assigned.\n");
        return -ENOMEM;
    }

    // Check virtual address of share_mem
    if (dev_data->share_mem.virt == NULL) {
        XM6_MSG(XM6_MSG_ERR, "can't get virtual address for share memory.\n");
        return -EINVAL;
    }

    XM6_MSG(XM6_MSG_DBG, "share memory: miu: 0x%llx, phys: 0x%x, virt: %p\n", boot_config->share_mem.miu, (int)dev_data->share_mem.phys, dev_data->share_mem.virt);

    // Reset share_mem
    memset(dev_data->share_mem.virt, 0, dev_data->share_mem.size);

    // The sahre memory is seperated into 2 part, share (DMA) memory and hprintf buffer, both are shared between DSP and host.

    // Assign hprintf buffer, we put hprintf buffer first because this is also used for boot copier and it must page-alignment
    dev_data->hprintf_buf.virt = dev_data->share_mem.virt;
    dev_data->hprintf_buf.phys = dev_data->share_mem.phys;
    dev_data->hprintf_buf.size = HPRINTF_MEM_SIZE;
    dev_data->hprintf_buf.is_from_miu = dev_data->share_mem.is_from_miu;

    // Assign DMA buffer
    dev_data->dma_buf.size = dev_data->share_mem.size - HPRINTF_MEM_SIZE;
    dev_data->dma_buf.priv_data = dev_data;
    dev_data->dma_buf.dma_addr = (dma_addr_t)dev_data->share_mem.phys + HPRINTF_MEM_SIZE;
    dev_data->dma_buf.cpu_addr = dev_data->share_mem.virt + HPRINTF_MEM_SIZE;

    return 0;
}

#ifdef HPRINTF_DUMP_AFTER_RELEASE
static char* xm6_drv_dump_and_get_next_dsp_log(char *log)
{
    int i;

    for (i = 0; log[i] != '\0' && log[i] != '\n'; i++)
    {
        if (log[i] == '\r')
            log[i] = ' ';
    }

    if (log[i] == '\0')
    {
        return NULL;
    }

    log[i] = '\0';
    printk("%p: %s\n", log, log);
    log[i] = '\n';

    return log + i + 1;
}
#endif // HPRINTF_DUMP_AFTER_RELEASE

static void xm6_drv_dump_dsp_log(struct xm6_dev_data *dev_data)
{
#ifdef HPRINTF_DUMP_AFTER_RELEASE
    int i;
    char *log = dev_data->hprintf_buf.virt;

    if (log == NULL)
        return;

    printk("==== Dump DSP last log (from %p) ====\n", log);

    // Find the first half log
    for (i = 0; log[i] != '\0'; i++){}
    log += i + 1;

    printk("fist half: %p, %d\n", log, i);

    // Dump the first half log
    while (log != NULL)
    {
        log = xm6_drv_dump_and_get_next_dsp_log(log);
    }

    printk("second half: %p\n", dev_data->hprintf_buf.virt);

    // Dump the second half log
    log = dev_data->hprintf_buf.virt;
    while (log != NULL)
    {
        log = xm6_drv_dump_and_get_next_dsp_log(log);
    }

    printk("==== Dump DSP last log ====\n");
#endif // HPRINTF_DUMP_AFTER_RELEASE
}

static void xm6_drv_deinit_dsp_buffer(struct xm6_dev_data *dev_data)
{
    xm6_drv_dump_dsp_log(dev_data);

    if (dev_data->share_mem.is_from_miu) {
        xm6_drv_iounmap(dev_data->share_mem.virt);
    }

    memset(&dev_data->boot_image, 0, sizeof(dev_dsp_buf));
    memset(&dev_data->ext_heap, 0, sizeof(dev_dsp_buf));
    memset(&dev_data->share_mem, 0, sizeof(dev_dsp_buf));
    memset(&dev_data->hprintf_buf, 0, sizeof(dev_dsp_buf));
    memset(&dev_data->work_buffer, 0, sizeof(dev_dsp_buf));

    memset(&dev_data->dma_buf, 0, sizeof(xm6_dev_dma_buf));
}

static int xm6_drv_dsp_boot_up(struct xm6_dev_data *dev_data, struct boot_config_t *boot_config)
{
    int ret;
    dsp_mem_info_t dsp_mem_info;
    dev_dsp_buf copier_buf;
    u8 *image_virt=NULL;

    if (dev_data->is_dsp_boot_up)
    {
        XM6_MSG(XM6_MSG_ERR, "DSP is boot up already\n");
        return -EISCONN;
    }

    // backup data
    memcpy(&dev_data->boot_config, boot_config, sizeof(dev_data->boot_config));

    // init all dsp buffers
    ret = xm6_drv_init_dsp_buffer(dev_data, boot_config);
    if (unlikely(ret != 0)) {
        XM6_MSG(XM6_MSG_ERR, "probe_init_dsp_buffer() failed\n");
        return -EISCONN;
    }
    image_virt=dev_data->boot_image.virt;

    //if (dev_data->boot_config.log_enable != 0)
    if(0)
    {
        ret = host_log_init();
        if (unlikely(ret != 0)) {
            XM6_MSG(XM6_MSG_ERR, "host_log_init() failed\n");
            return -ENOMEM;
        }

        ret = dsp_log_init(dev_data->boot_config.dsp_log_addr,
                           dev_data->boot_config.dsp_log_size);
        if (unlikely(ret != 0)) {
            XM6_MSG(XM6_MSG_ERR, "dsp_log_init() failed\n");
            return -ENOMEM;
        }
    }
    else
    {
        dev_data->boot_config.dsp_log_addr = INVALID_DSP_LOG_ADDR;
    }

    memset(&dsp_mem_info, 0, sizeof(dsp_mem_info));
    dsp_mem_info.boot_image.addr  = dev_data->boot_image.phys;
    dsp_mem_info.boot_image.size  = dev_data->boot_image.size;
    dsp_mem_info.share_mem.addr   = dev_data->dma_buf.dma_addr;
    dsp_mem_info.share_mem.size   = dev_data->dma_buf.size;
    dsp_mem_info.ext_heap.addr    = dev_data->ext_heap.phys;
    dsp_mem_info.ext_heap.size    = dev_data->ext_heap.size;
    dsp_mem_info.hprintf_buf.addr  = dev_data->hprintf_buf.phys;
    dsp_mem_info.hprintf_buf.size  = dev_data->hprintf_buf.size;
    dsp_mem_info.dsp_log_buf.addr = dev_data->boot_config.dsp_log_addr;
    dsp_mem_info.dsp_log_buf.size = dev_data->boot_config.dsp_log_size;
    dsp_mem_info.work_buffer.addr = dev_data->work_buffer.phys;
    dsp_mem_info.work_buffer.size = dev_data->work_buffer.size;
    dsp_mem_info.custom_info = boot_config->custom_info;
    
    XM6_MSG(XM6_MSG_WRN, "custom_info = %d\n", boot_config->custom_info);

    // create copier from ext_heap, we use copier only in boot stage
    copier_buf.virt = dev_data->hprintf_buf.virt;
    copier_buf.phys = dev_data->hprintf_buf.phys;
    copier_buf.size = dev_data->hprintf_buf.size;
    copier_buf.is_from_miu = dev_data->hprintf_buf.is_from_miu;

#ifdef _FAST_DMA_BOOT_
    ret = dsp_dma_image_ext_transfer(dev_data->boot_config,image_virt);
    if (unlikely(ret != 0)) {
        XM6_MSG(XM6_MSG_ERR, "dsp_dma_image_ext_transfer() failed\n");
        return ret;
    }
#endif


    // boot up
    ret = dsp_boot_up(&dev_data->ceva_hal, &copier_buf, dev_data->dma_buf.cpu_addr, &dsp_mem_info);

    if (ret == 0)
    {
        dev_data->is_dsp_boot_up = 1;
    }
    else
    {
        XM6_MSG(XM6_MSG_ERR, "can't boot DSP, err: %d\n", ret);
    }

    return ret;
}

static int xm6_drv_dsp_shut_down(struct xm6_dev_data *dev_data)
{
    if (!dev_data->is_dsp_boot_up)
    {
        XM6_MSG(XM6_MSG_ERR, "DSP does not boot up yet\n");
        return -ENOTCONN;
    }

    XM6_MSG(XM6_MSG_DBG, "shut down DSP\n");

    dsp_shut_down(&dev_data->ceva_hal);

    xm6_drv_deinit_dsp_buffer(dev_data);

    dev_data->is_dsp_boot_up = 0;

    return 0;
}

static void dump_dbg_message(unsigned int cmd)
{
  if(IOC_CEVADRV_GET_PID_PROCID != cmd)
  {
    XM6_MSG(XM6_MSG_ERR, "IOCTL Error cmd:0x%x \n",cmd);

    XM6_MSG(XM6_MSG_ERR, "	IOC_CEVADRV_PRINT_DMA:		0x%x \n",IOC_CEVADRV_PRINT_DMA);
    XM6_MSG(XM6_MSG_ERR, "	IOC_CEVADRV_BYPASS_REQ:		0x%x \n",IOC_CEVADRV_BYPASS_REQ);
    XM6_MSG(XM6_MSG_ERR, "	IOC_CEVADRV_READ_DEBUG:		0x%x \n",IOC_CEVADRV_READ_DEBUG);
    XM6_MSG(XM6_MSG_ERR, "	IOC_CEVADRV_GENERATE_INT:	0x%x \n",IOC_CEVADRV_GENERATE_INT);
    XM6_MSG(XM6_MSG_ERR, "	IOC_CEVADRV_GET_PID_PROCID:0x%x \n",IOC_CEVADRV_GET_PID_PROCID);
    XM6_MSG(XM6_MSG_ERR, "	IOC_CEVADRV_BOOT_UP:				0x%x \n",IOC_CEVADRV_BOOT_UP);
    XM6_MSG(XM6_MSG_ERR, "	IOC_CEVADRV_SHUT_DOWN:			0x%x \n",IOC_CEVADRV_SHUT_DOWN);
    XM6_MSG(XM6_MSG_ERR, "	IOC_CEVADRV_BOOT_CHECK:			0x%x \n",IOC_CEVADRV_BOOT_CHECK);
    XM6_MSG(XM6_MSG_ERR, "	IOC_CEVADRV_MIU_2_PHYS:			0x%x \n",IOC_CEVADRV_MIU_2_PHYS);
    XM6_MSG(XM6_MSG_ERR, "	IOC_CEVADRV_PHYS_2_MIU:			0x%x \n",IOC_CEVADRV_PHYS_2_MIU);
  }

  return;
}
/*!
 * ioctl handler
 *
 * special commands executer.
 * in case the command is not supported by this module, it forwards it
 * to the ceva_linkdrv module to handle.
 * @param [in] filp device handler
 * @param [in] cmd command to execute
 * @param [in] arg additional argument (differ for each command)
 * @return zero for success, nonzero for failures
 * @see IOC_CEVADRV_GENERATE_INT, IOC_CEVADRV_PRINT_DMA, ceva_linkdrv_ioctl()
 */
static long xm6_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    int ret = 0;
    struct xm6_dev_data *dev_data = (struct xm6_dev_data *) filp->private_data;
    struct ceva_linkdrv* linkdrv = &dev_data->linkdrv;

    XM6_MSG(XM6_MSG_DBG, "ioctl filp %p, device %p, cmd 0x%x\n", filp, dev_data, cmd);

    /* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
    if (_IOC_TYPE(cmd) != IOC_CEVADRV_MAGIC || _IOC_NR(cmd) > IOC_CEVADRV_MAXNR) {
        return -ENOTTY;
    }

    switch (cmd) {
    case IOC_CEVADRV_GENERATE_INT:
        /* use arg as raw data for interrupt */
        XM6_MSG(XM6_MSG_DBG, "generating interrupt %lx\n", arg);
        ret = ceva_link_generate_irq(dev_data, arg);
        if (!ret) {
            DEBUG_INFO_INC(linkdrv, generated_interrupts);
        } else {
            DEBUG_INFO_INC(linkdrv, generate_interrupt_failures);
        }
        break;

    case IOC_CEVADRV_PRINT_DMA: {
#ifdef ENABLE_DEBUG
            xm6_dev_dma_buf *dma_buf = &dev_data->dma_buf;
            int off = (int)arg;
            int* addr = (int*)arg;
            int* p = dma_buf->cpu_addr;

            XM6_MSG(XM6_MSG_DBG, "IOC_CEVADRV_PRINT_DMA :: dma address @ 0x%p -> 0x%x\n", addr, p[off]);
#endif // ENABLE_DEBUG
        }
        break;

    case IOC_CEVADRV_BOOT_UP: {
            struct boot_config_t boot_config;

            // copy and reassign boot info from user argument
            ret = copy_from_user(&boot_config, (void*)arg, sizeof(boot_config));

            if (ret != 0) {
                XM6_MSG(XM6_MSG_ERR, "can't copy data from user space (%d copied %d)\n", sizeof(boot_config), ret);
                return -ENOMEM;
            }

            return xm6_drv_dsp_boot_up(dev_data, &boot_config);
        }

    case IOC_CEVADRV_SHUT_DOWN: {
            ret = copy_to_user((void*)arg, &dev_data->boot_config, sizeof(struct boot_config_t));
            if (ret != 0) {
                return ret;
            }

            ret = xm6_drv_dsp_shut_down(dev_data);
            if (ret != 0) {
                return ret;
            }

            return 0;
        }

    case IOC_CEVADRV_BOOT_CHECK: {
            ret = copy_to_user((void*)arg, &dev_data->is_dsp_boot_up, sizeof(dev_data->is_dsp_boot_up));
            if (ret != 0) {
                return ret;
            }

            return 0;
        }

    case IOC_CEVADRV_MIU_2_PHYS: {
            u64 miu, phys;
            ret = copy_from_user(&miu, (void*)arg, sizeof(miu));
            if (ret != 0) {
                XM6_MSG(XM6_MSG_ERR, "can't copy data from user space (%d copied %d)\n", sizeof(miu), ret);
                return -ENOMEM;
            }

            phys = xm6_drv_miu2phys(miu);

            XM6_MSG(XM6_MSG_DBG, "miu: 0x%llx ==> phys: 0x%llx\n", miu, phys);

            ret = copy_to_user((void*)arg, &phys, sizeof(phys));
            if (ret != 0) {
                XM6_MSG(XM6_MSG_ERR, "can't copy data to user space (%d copied %d)\n", sizeof(phys), ret);
                return -ENOMEM;
            }

            return 0;
        }

    case IOC_CEVADRV_PHYS_2_MIU: {
            u64 miu, phys;
            ret = copy_from_user(&phys, (void*)arg, sizeof(phys));
            if (ret != 0) {
                XM6_MSG(XM6_MSG_ERR, "can't copy data from user space (%d copied %d)\n", sizeof(miu), ret);
                return -ENOMEM;
            }

            miu = xm6_drv_phys2miu(phys);

            XM6_MSG(XM6_MSG_DBG, "phys: 0x%llx ==> miu: 0x%llx\n", phys, miu);

            ret = copy_to_user((void*)arg, &miu, sizeof(miu));
            if (ret != 0) {
                XM6_MSG(XM6_MSG_ERR, "can't copy data to user space (%d copied %d)\n", sizeof(miu), ret);
                return -ENOMEM;
            }

            return 0;
        }
     
     case IOC_CEVADRV_VERSION_CHECK: {
            XM6_MSG(XM6_MSG_INIT, "IOC_CEVADRV_VERSION_CHECK \n");
            dump_XM6VerInfo(pVerInfo);
            ret = copy_to_user((void*)arg, pVerInfo, sizeof(struct Version_Info));
            if (ret != 0) {
                XM6_MSG(XM6_MSG_ERR, "can't copy data to user space (%d copied %d)\n", sizeof(struct Version_Info), ret);
                return -ENOMEM;
            }
            return 0;
        }

    default:
    	  dump_dbg_message(cmd);
        ret = ceva_linkdrv_ioctl(linkdrv, cmd, arg);
    }

    return ret;
}

static void xm6_drv_dma_vma_open(struct vm_area_struct *vma) {
    xm6_dev_dma_buf* dma_buf;
    struct xm6_dev_data *dev_data;

    XM6_MSG(XM6_MSG_DBG, "xm6_drv_dma_vma_open: virt 0x%lx, phys 0x%lx\n", vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);

    dma_buf = vma->vm_private_data;
    dev_data = dma_buf->priv_data;
}

static void xm6_drv_dma_vma_close(struct vm_area_struct *vma) {
    xm6_dev_dma_buf* dma_buf;
    struct xm6_dev_data *dev_data;

    dma_buf = vma->vm_private_data;
    dev_data = dma_buf->priv_data;
}

/*!
 * mmap handler for dma memory
 *
 * this function remap the internal dma memory that was mapped during
 * probe function to userland.
 * @param [in] dev_data device driver object
 * @param [in] vma vma object
 * @return zero for success, nonzero for failures
 * @see xm6_drv_probe()
 */
static int xm6_drv_mmap_dma(struct xm6_dev_data *dev_data, struct vm_area_struct *vma) {
    xm6_dev_dma_buf *dma_buf = &dev_data->dma_buf;
    unsigned long pfn, size;

#ifdef ENABLE_DEBUG
    static int count = 0;
#endif // ENABLE_DEBUG

        XM6_MSG(XM6_MSG_DBG, "setting vma\n");

        vma->vm_ops = &xm6_drv_dma_vma_ops;
        vma->vm_private_data = dma_buf;
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

        xm6_drv_dma_vma_open(vma);

#ifdef ENABLE_DEBUG
        XM6_MSG(XM6_MSG_DBG, "enter #%d\n", ++count);
#endif

        pfn = vmalloc_to_pfn(dma_buf->cpu_addr);
        size = vma->vm_end - vma->vm_start;

        XM6_MSG(XM6_MSG_DBG, "remap started, vm_start =0x%lx, cpu_addr = 0x%p, dma_addr = 0x%lx, pfn = 0x%lx, size = %lu\n",
                vma->vm_start, dma_buf->cpu_addr, (unsigned long)dma_buf->dma_addr, pfn, size);

        if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
            XM6_MSG(XM6_MSG_ERR, "remap failed\n");
            return -EAGAIN;
        }

        XM6_MSG(XM6_MSG_DBG, "remap succeeded\n");
        return 0;
}

/*!
 * mmap handler
 *
 * wrapper function for mapping dma to user.
 * @param [in] filp device handler
 * @param [in] vma vma object
 * @return zero for success, nonzero for failures
 * @note the wrapper was implemented to allow mapping of pci memory in
 * future versions (if needed)
 * @see xm6_drv_mmap_dma()
 */
static int xm6_drv_mmap(struct file *filp, struct vm_area_struct *vma) {
    struct xm6_dev_data *dev_data = (struct xm6_dev_data *) filp->private_data;

    XM6_MSG(XM6_MSG_DBG, "mmap filp %p, device %p\n", filp, dev_data);
    return xm6_drv_mmap_dma(dev_data, vma);
}

/*!
 * cleanup logical part of this module
 *
 * this function deletes any non-pci related driver's objects (the chardev
 * properties).
 * @param [in] dev_data device handler
 */
static void xm6_drv_cleanup_module(struct platform_device *plat_dev, struct xm6_dev_data *dev_data) {
    device_destroy(xm6_drv_data.class, dev_data->cdev.dev);
    cdev_del(&dev_data->cdev);

    mutex_destroy(&dev_data->dev_mutex);
}

/*!
 * buildup the char device of this module
 *
 * this function handles the creation and setup of the char device
 * @param [in] dev_data device handler
 * @return negative error code is returned on failure
 * @see ceva_linkdrv_init_module()
 */
static int xm6_drv_setup_cdev(struct platform_device *plat_dev, struct xm6_dev_data *dev_data) {
    int ret;
    struct device *device = NULL;
    int dev_count, dev_no;

    dev_count = xm6_drv_data.minor_star + xm6_drv_data.reg_count;
    dev_no = MKDEV(xm6_drv_data.major, dev_count);

    cdev_init(&dev_data->cdev, &xm6_drv_fops);
    dev_data->cdev.owner = THIS_MODULE;
    ret = cdev_add(&dev_data->cdev, dev_no, 1);
    /* Fail gracefully if need be */
    if (ret < 0) {
        XM6_MSG(XM6_MSG_ERR, "setup_cdev: Error %d adding device\n", dev_no);
        goto exit;
    }

    device = device_create(
            xm6_drv_data.class,                 /* class */
            NULL,                               /* no parent device */
            dev_no,
            dev_data,
            CEVA_PCIDEV_DEVICE_NAME "%d",
            dev_count);

    if (IS_ERR(device)) {
        ret = PTR_ERR(device);
        XM6_MSG(XM6_MSG_ERR, "setup_cdev: Error %d while trying to create %s%d\n",
                ret,
                CEVA_PCIDEV_DEVICE_NAME,
                dev_count);
        cdev_del(&dev_data->cdev);
    }

    dev_data->count = 0;

    // Increase registered count
    xm6_drv_data.reg_count++;

    dev_set_drvdata(&plat_dev->dev, dev_data);

exit:
    return ret;

}

/*!
 * initialize logical part of this module
 *
 * this function allocate and setup all non-pci related driver's objects
 * (the chardev properties).
 * @param [in] dev_data device handler
 * @return zero for success, nonzero for failures
 * @see xm6_drv_setup_cdev()
 */
static int ceva_linkdrv_init_module(struct platform_device *plat_dev, struct xm6_dev_data *dev_data) {
    int ret;

    mutex_init(&dev_data->dev_mutex);

    ret = xm6_drv_setup_cdev(plat_dev, dev_data);
    if (ret < 0) {
        goto fail;
    }

    return 0;

fail:
    xm6_drv_cleanup_module(plat_dev, dev_data);
    return ret;
}

/*!
 * get PCIe hardware properties (irq, revision)
 * @param [in] plat_dev PCIe device handler
 * @param [in] dev_data device handler
 * @return zero for success, nonzero for failures
 * @see xm6_drv_probe()
 */
static inline int probe_get_hw_prop(struct platform_device* plat_dev, struct xm6_dev_data *dev_data) {
    struct resource *res = NULL;

    dev_data->hw_irq_id = irq_of_parse_and_map(plat_dev->dev.of_node, 0);
    if (dev_data->hw_irq_id == 0) {
        XM6_MSG(XM6_MSG_ERR, "Can't get IRQ ID\n");
        return -ENODEV;
    }

    res = platform_get_resource(plat_dev, IORESOURCE_MEM, 0);
    if (res == NULL) {
        XM6_MSG(XM6_MSG_ERR, "Can't get IO addr 0\n");
        return -ENODEV;
    }
    dev_data->hw_addr_xm6 = res->start;

    res = platform_get_resource(plat_dev, IORESOURCE_MEM, 1);
    if (res == NULL) {
        XM6_MSG(XM6_MSG_ERR, "Can't get IO addr 0\n");
        return -ENODEV;
    }
    dev_data->hw_addr_sys = res->start;

    res = platform_get_resource(plat_dev, IORESOURCE_MEM, 2);
    if (res == NULL) {
        XM6_MSG(XM6_MSG_ERR, "Can't get IO addr 1\n");
        return -ENODEV;
    }
    dev_data->hw_addr_axi2miu0 = res->start;

    res = platform_get_resource(plat_dev, IORESOURCE_MEM, 3);
    if (res == NULL) {
        XM6_MSG(XM6_MSG_ERR, "Can't get IO addr 2\n");
        return -ENODEV;
    }
    dev_data->hw_addr_axi2miu1 = res->start;

    res = platform_get_resource(plat_dev, IORESOURCE_MEM, 4);
    if (res == NULL) {
        XM6_MSG(XM6_MSG_ERR, "Can't get IO addr 3\n");
        return -ENODEV;
    }
    dev_data->hw_addr_axi2miu2 = res->start;

    res = platform_get_resource(plat_dev, IORESOURCE_MEM, 5);
    if (res == NULL) {
        XM6_MSG(XM6_MSG_ERR, "Can't get IO addr 4\n");
        return -ENODEV;
    }
    dev_data->hw_addr_axi2miu3 = res->start;


    res = platform_get_resource(plat_dev, IORESOURCE_MEM, 6);
    if (res == NULL) {
        XM6_MSG(XM6_MSG_ERR, "Can't get CPM addr\n");
        return -ENODEV;
    }
    dev_data->hw_addr_cpm = res->start;
    
    #if 1
    res = platform_get_resource(plat_dev, IORESOURCE_MEM, 7);
    if (res == NULL) {
        XM6_MSG(XM6_MSG_ERR, "Can't get hw_addr_cevatopctl addr\n");
        return -ENODEV;
    }
    dev_data->hw_addr_cevatopctl = res->start;
    
    res = platform_get_resource(plat_dev, IORESOURCE_MEM, 8);
    if (res == NULL) {
        XM6_MSG(XM6_MSG_ERR, "Can't get hw_addr_cevapllpower addr\n");
        return -ENODEV;
    }
    dev_data->hw_addr_cevapllpower = res->start;
    #endif 

    XM6_MSG(XM6_MSG_DBG, "irq id: %d, hw addr: %x, %x %x, %x, %x, %x, %x ,%x \n",
                            dev_data->hw_irq_id, dev_data->hw_addr_sys,
                            dev_data->hw_addr_axi2miu0, dev_data->hw_addr_axi2miu1,
                            dev_data->hw_addr_axi2miu2, dev_data->hw_addr_axi2miu3,
                            dev_data->hw_addr_cpm,dev_data->hw_addr_cevatopctl,dev_data->hw_addr_cevapllpower);
    return 0;
}

/*!
 * generates an interrupt over the device
 *
 * this function checks for device's availability to receive new interrupts
 * and then writes the new interrupt data to the predefined address in bar0
 * @param dev_data device handler
 * @param e interrupt id to generate
 * @return zero for success, -EBUSY if device's fifo is full
 * @see xm6_drv_ioctl(), IOC_CEVADRV_GENERATE_INT
 */
static int ceva_link_generate_irq(struct xm6_dev_data *dev_data, ceva_event_t e) {
    u32 state;

    XM6_MSG(XM6_MSG_DBG, "send IRQ MCCI_LINK_EVENT_FROM_HOST to DSP\n");
    HOST_LOG("MCCI_LINK_EVENT_FROM_HOST\n");

    // disable MCCI IRQ temporarily
    dsp_ceva_hal_disable_irq(&dev_data->ceva_hal, CEVA_HAL_IRQ_TARGET_ARM, CEVA_HAL_IRQ_MCCI_RD);
    HOST_LOG("disable MCCI RD intr\n");

    // write MCCI_0 to pass data
    cmp_hal_mcci_write(&dev_data->cpm_hal, MCCI_LINK_EVENT_FROM_HOST, e);
    HOST_LOG("write MIIC 0 data=0x%x\n", e);

    // wait DSP response via MCCI_0
    state = dsp_ceva_hal_check_mcci_irq(&dev_data->ceva_hal, MCCI_LINK_EVENT_FROM_HOST);
    while (state == 0) {
        // delay ?
        // ndelay(10);

        dsp_ceva_hal_read_dummy_data(&dev_data->ceva_hal, 1);

        state = dsp_ceva_hal_check_mcci_irq(&dev_data->ceva_hal, MCCI_LINK_EVENT_FROM_HOST);
    }
    HOST_LOG("read MCCI IRQ state 0x%x\n", state);

    // clear MCCI_0 status to avoid a interrupt
    dsp_ceva_hal_clear_mcci_irq(&dev_data->ceva_hal, 0);

    XM6_MSG(XM6_MSG_DBG, "clear MCCI read IRQ, 0x%X\n", state);
    HOST_LOG("clear MCCI read IRQ 0x%x\n", state);

    // enable MCCI interrupt
    dsp_ceva_hal_enable_irq(&dev_data->ceva_hal, CEVA_HAL_IRQ_TARGET_ARM, CEVA_HAL_IRQ_MCCI_RD);
    HOST_LOG("enable MCCI RD intr\n");
    
    SendDSPCount++;

    return 0;
}

/*!
 * interrupt routine
 *
 * this function handles the device's interrupts.
 * it first reads from the device irq status register in order to tell
 * which interrupts where sent to the device (there up to 16), and then
 * reads the interrupts data, and forward it to the logical module to
 * handle.
 * @param [in] irq irq id
 * @param [in] dev_id device handler
 * @return IRQ_HANDLED for success, IRQ_NONE for error
 * @see ceva_linkdrv_broadcast_events()
 */
static irqreturn_t ceva_linkdrv_interrupt(int irq, void *dev_id) {
    struct xm6_dev_data *dev_data = (struct xm6_dev_data *) dev_id;
    u32 dsp_data;
    irqreturn_t ret = IRQ_NONE;
    CEVA_HAL_IRQ id;

    // printk("ISR\n");
    HOST_LOG("enter intr\n");

    id = dsp_ceva_hal_get_irq_status(&dev_data->ceva_hal);
    HOST_LOG("IRQ status = 0x%x\n", id);
    id &= dsp_ceva_hal_get_irq_mask(&dev_data->ceva_hal, CEVA_HAL_IRQ_TARGET_ARM);
    if ((id & CEVA_HAL_IRQ_MCCI_RD) == 0)
    {
        HOST_LOG("Not MCCI RD intr\n");
        return ret;
    }

    // MCCI_1 is for link
    if (dsp_ceva_hal_check_mcci_irq(&dev_data->ceva_hal, MCCI_LINK_EVENT_FROM_DSP))
    {
        HOST_LOG("MCCI_LINK_EVENT_FROM_DSP\n");

        // read out
        dsp_data = dsp_ceva_hal_read_dummy_data(&dev_data->ceva_hal, 0);

        XM6_MSG(XM6_MSG_DBG, "receive data 0x%08x from DSP\n", dsp_data);
        HOST_LOG("recv 0x%x from DSP\n", dsp_data);

        // clear interrupt
        dsp_ceva_hal_clear_mcci_irq(&dev_data->ceva_hal, MCCI_LINK_EVENT_FROM_DSP);

        // change dummy register to notify DSP that data is read
        dsp_ceva_hal_write_dummy_data(&dev_data->ceva_hal, 0, dsp_data-1);
        HOST_LOG("write dummy reg 0x%x\n", dsp_data - 1);

        // send to linkdrv layer
        ceva_linkdrv_broadcast_events(&dev_data->linkdrv, &dsp_data, 1);
        
        GotDSPCount++;

        ret = IRQ_HANDLED;
    }

    // MCCI_2 is for printf
    else if(dsp_ceva_hal_check_mcci_irq(&dev_data->ceva_hal, MCCI_PRINT_FROM_DSP))
    {
        HOST_LOG("MCCI_PRINT_FROM_DSP\n");

        // read out
        dsp_data = dsp_ceva_hal_read_dummy_data(&dev_data->ceva_hal, 0);

        XM6_MSG(XM6_MSG_DBG, "receive data 0x%08x from DSP\n", dsp_data);
        HOST_LOG("recv data 0x%08x from DSP\n", dsp_data);

        //
        printk(dev_data->hprintf_buf.virt);

        // clear interrupt
        dsp_ceva_hal_clear_mcci_irq(&dev_data->ceva_hal, MCCI_PRINT_FROM_DSP);

        // change dummy register to notify DSP that data is read
        dsp_ceva_hal_write_dummy_data(&dev_data->ceva_hal, 0, dsp_data-1);
        HOST_LOG("write dummy reg 0x%08x\n", dsp_data - 1);

        ret = IRQ_HANDLED;
    }

    // JBox may trigger other mcci pins
    else
    {
        dsp_data = dsp_ceva_hal_get_mcci_irq(&dev_data->ceva_hal);
        dsp_data &= MCCI_IRQ_UNEXPECT;
        if(dsp_data != 0)
        {
            XM6_MSG(XM6_MSG_DBG, "unexpect MCCI IRQ 0x%x (0x%X)\n", dsp_data, dsp_ceva_hal_read_dummy_data(&dev_data->ceva_hal, 1));
            HOST_LOG("unexpect MCCI IRQ 0x%x (0x%X)\n", dsp_data, dsp_ceva_hal_read_dummy_data(&dev_data->ceva_hal, 1));

            // clear interrupt
            dsp_ceva_hal_clear_mcci_irq_ex(&dev_data->ceva_hal, dsp_data);

            ret = IRQ_HANDLED;
        }
    }

    HOST_LOG("exit intr\n");
    return ret;
}

/*!
 * enable PCIe irq
 * @param [in] plat_dev PCIe device handler
 * @param [in] dev_data device handler
 * @return zero for success, nonzero for failures
 * @see xm6_drv_probe()
 */
static int probe_init_irq(struct platform_device *plat_dev, struct xm6_dev_data *dev_data) {
    int ret = request_irq(dev_data->hw_irq_id,
                          ceva_linkdrv_interrupt,
                          0,
                          DRIVER_NAME, dev_data);
    if (ret != 0) {
        XM6_MSG(XM6_MSG_ERR, "Could not request IRQ #%d, error %d\n", dev_data->hw_irq_id, ret);
        return -EIO;
    }

    // Enable MCCI interrupt (read/write) is used as a trigger signal between ARM and DSP
    dsp_ceva_hal_enable_irq(&dev_data->ceva_hal, CEVA_HAL_IRQ_TARGET_ARM, CEVA_HAL_IRQ_MCCI_RD);
    dsp_ceva_hal_enable_irq(&dev_data->ceva_hal, CEVA_HAL_IRQ_TARGET_XM6_INT0, CEVA_HAL_IRQ_MCCI_MES);

    XM6_MSG(XM6_MSG_DBG, "Successfully requested IRQ #%d\n", dev_data->hw_irq_id);
    return 0;
}

/*!
 * disable PCIe irq
 * @param [in] plat_dev PCIe device handler
 * @param [in] dev_data device handler
 * @see xm6_drv_remove()
 */
static void remove_deinit_irq(struct platform_device *plat_dev, struct xm6_dev_data *dev_data) {
    free_irq(dev_data->hw_irq_id, dev_data);
    XM6_MSG(XM6_MSG_DBG, "irq freed");
}

/*!
 * XM6 probe function
 *
 * this function enables and starts CEVA PCIe device
 * @param [in/out] plat_dev PCIe device handler
 * @param [in] id not used
 * @return zero for success, nonzero for failures
 * @see find_upstream_dev(), retrain_gen2(), probe_get_hw_prop(),
 * probe_map_bars(), probe_map_dma(), probe_enable_msi(),probe_init_irq(),
 * ceva_linkdrv_init_module(), xm6_drv_remove()
 */
static int xm6_drv_probe(struct platform_device *plat_dev) {
    struct xm6_dev_data *dev_data = NULL;
    int ret = 0;

    XM6_MSG(XM6_MSG_DBG, "device probed (dev_data = 0x%p)\n", plat_dev);

    dev_data = devm_kcalloc(&plat_dev->dev, 1, sizeof(struct xm6_dev_data), GFP_KERNEL);
    if (unlikely(!dev_data)) {
        ret = -ENOMEM;
        XM6_MSG(XM6_MSG_ERR, "probe: allocation error\n");
        goto alloc_fail;
    }

    dev_data->plat_dev = plat_dev;

    /*
     * get hardware properties
     */
    ret = probe_get_hw_prop(plat_dev, dev_data);
    if (unlikely(ret != 0)) {
        goto enable_fail;
    }

    /* Init ceva wrapper hal */
    dsp_ceva_hal_init(&dev_data->ceva_hal, dev_data->hw_addr_sys,  dev_data->hw_addr_axi2miu0,  dev_data->hw_addr_axi2miu1,  dev_data->hw_addr_axi2miu2, dev_data->hw_addr_axi2miu3);
    dsp_ceva_hal_init_cevatop(&dev_data->ceva_hal, dev_data->hw_addr_cevatopctl, dev_data->hw_addr_cevapllpower);

    /* Init ceva cpm hal */
    cmp_hal_init(&dev_data->cpm_hal, dev_data->hw_addr_cpm);
    cmp_hal_init(&dev_data->cevatopctl_hal, dev_data->hw_addr_cevatopctl);
    cmp_hal_init(&dev_data->cevapllpower_hal, dev_data->hw_addr_cevapllpower);
    
    /* Get Init VCore */
    dev_data->InitVCore=dsp_ceva_hal_read_current_vcore();
    dsp_ceva_hal_init_VCore(&dev_data->ceva_hal, dev_data->InitVCore);
    XM6_MSG(XM6_MSG_DBG, "probe: InitVCore: %d \n",dev_data->InitVCore);

    pGceva_hal=&dev_data->ceva_hal;

    /*
     * request irq
     */
    ret = probe_init_irq(plat_dev, dev_data);
    if (unlikely(ret != 0)) {
        goto enable_fail;
    }

    /*
     * logical module init
     */
    ret = ceva_linkdrv_init_module(plat_dev, dev_data);
    if (unlikely(ret != 0)) {
        XM6_MSG(XM6_MSG_ERR, "probe: failed init logic module\n");
        goto logic_fail;
    }

    // Increase registered count
    xm6_drv_data.reg_count++;

    ret = ceva_linkdrv_proc_init(dev_data);
    if (ret != 0) {
        goto proc_fail;
    }

    /*
     * probed successfully
     */

    XM6_MSG(XM6_MSG_DBG, "CEVA device probed\n");

    return 0;

proc_fail:
logic_fail:
    remove_deinit_irq(plat_dev, dev_data);

enable_fail:
    devm_kfree(&dev_data->plat_dev->dev, dev_data);

alloc_fail:
    XM6_MSG(XM6_MSG_ERR, "probe: failed (ret=%d)\n", ret);

    return ret;
}

/*!
 * XM6 remove function
 *
 * this function stops and disables the CEVA PCIe device
 * @param [in/out] plat_dev PCIe device handler
 * @see xm6_drv_cleanup_module(), remove_deinit_irq(), pci_disable_msi(),
 * free_bars(), remove_disable_pci_dev(), xm6_drv_probe()
 */
static s32 xm6_drv_remove(struct platform_device *plat_dev) {
    struct xm6_dev_data *dev_data;

    dev_data = (struct xm6_dev_data *)dev_get_drvdata(&plat_dev->dev);
    if (unlikely(!dev_data)) {
        XM6_MSG(XM6_MSG_DBG, "device data is NULL.\n");
        return -ENODEV;
    }

    xm6_drv_cleanup_module(plat_dev, dev_data);
    dev_set_drvdata(&plat_dev->dev, NULL);

    /*
     * disable interrupt
     */
    remove_deinit_irq(plat_dev, dev_data);

    devm_kfree(&plat_dev->dev, dev_data);

    XM6_MSG(XM6_MSG_DBG, "Removed device\n");

    return 0;
}

/*!
 * driver't init function
 *
 * this function register the CEVA's supported PCI ids with
 * this driver.
 * it also inits the protected shared memory module.
 * @see xm6_drv_exit_module
 */
int __init xm6_drv_init_module(void) {
    int err;
    dev_t dev;

    XM6_MSG(XM6_MSG_DBG, "Moudle Init\n");

    if (protected_mem_db_init() != 0) {
        XM6_MSG(XM6_MSG_ERR, "failed to init protected memory db\n");
        return -ENODEV;
    }

    // Allocate cdev id
    err = alloc_chrdev_region(&dev, XM6_DRV_MINOR, XM6_DRV_DEVICE_COUNT, XM6_DRV_NAME);
    if (err) {
        XM6_MSG(XM6_MSG_ERR, "Unable allocate cdev id\n");
        return err;
    }

    xm6_drv_data.major = MAJOR(dev);
    xm6_drv_data.minor_star = MINOR(dev);
    xm6_drv_data.reg_count = 0;

    XM6_MSG(XM6_MSG_DBG, "allocate char dev: %d, %d\n", xm6_drv_data.major, xm6_drv_data.minor_star);

    // Register device class
    xm6_drv_data.class = class_create(THIS_MODULE, XM6_DRV_CLASS_NAME);
    if (IS_ERR(xm6_drv_data.class)) {
        XM6_MSG(XM6_MSG_ERR, "Failed at class_create().Please exec [mknod] before operate the device\n");
        err = PTR_ERR(xm6_drv_data.class);
        goto ERR_RETURN_1;
    }

    // Register platform driver
    err = platform_driver_register(&xm6_drv_driver);
    if (err != 0) {
        goto ERR_RETURN_2;
    }
    Init_XM6FW_VerInfo();

    return 0;

ERR_RETURN_2:
    class_destroy(xm6_drv_data.class);

ERR_RETURN_1:
    unregister_chrdev_region(MKDEV(xm6_drv_data.major, xm6_drv_data.minor_star), XM6_DRV_DEVICE_COUNT);

    return err;
}

/*!
 * driver's deinit function
 *
 * this function unregister the CEVA's supported PCI ids with
 * this driver.
 * it also de-inits the protected shared memory module.
 * @see xm6_drv_init_module
 */
void __exit xm6_drv_exit_module(void) {
    XM6_MSG(XM6_MSG_DBG, "Modules Exit\n");

    ceva_linkdrv_proc_exit();

    protected_mem_db_deinit();

    platform_driver_unregister(&xm6_drv_driver);
    class_destroy(xm6_drv_data.class);
    unregister_chrdev_region(MKDEV(xm6_drv_data.major, xm6_drv_data.minor_star), XM6_DRV_DEVICE_COUNT);
}

module_init(xm6_drv_init_module);
module_exit(xm6_drv_exit_module);
