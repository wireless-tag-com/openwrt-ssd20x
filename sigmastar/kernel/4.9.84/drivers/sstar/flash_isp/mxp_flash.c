/*
* mxp_flash.c- Sigmastar
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
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
#include <linux/slab.h>
#endif
#include <linux/types.h>
#include "part_mxp.h"
#include "MsTypes.h"


#ifdef CONFIG_MS_FLASH_ISP
static mxp_manager* nor_flash_mxp=NULL;

//#define NOR_FLASH_MXP_TABLE_BASE    0x0020000
#define NOR_FLASH_MXP_TABLE_SIZE    0x1000

extern MS_BOOL MDrv_SERFLASH_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer);

static int nor_flash_mxp_read_bytes(int offset,void* buf,int len)
{

    if (!MDrv_SERFLASH_Read(offset,len,buf))
    {
        printk("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
        return -1;
    }

    return 0;
}


static int nor_flash_mxp_write_bytes(int offset,void* buf,int len)
{
#if 0
    int ret;
    ret = spi_flash_erase(norflash, NOR_FLASH_MXP_TABLE_BASE, NOR_FLASH_MXP_TABLE_SIZE);
    if (ret) {
        printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
        return -1;
    }

    ret = spi_flash_write(norflash, NOR_FLASH_MXP_TABLE_BASE+offset, len, buf);
    if (ret) {
        printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
        return -1;
    }
#endif
    return 0;
}

static int nor_flash_mxp_update_byte(int offset, char byte)
{
    int ret=-1;
#if 0
    char b;
    if(spi_flash_read(norflash, (NOR_FLASH_MXP_TABLE_BASE+offset), (size_t)1, &b))
    {
        printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
        goto DONE;

    }

    printf("[nor_flash_mxp_update_byte] 0x%08X: 0x%02X -> 0x%02X\n",(NOR_FLASH_MXP_TABLE_BASE+offset),b,byte);
    if( (((char)(byte^b)) & (char)(~b)) >0 )
    {
        printf("  using general page flash write\n");
        return nor_flash_mxp_write_bytes(offset,&byte,1);
    }

    printf("  using single byte flash write\n");
    if(spi_flash_write(norflash, NOR_FLASH_MXP_TABLE_BASE+offset, 1, &byte)) {
            printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
            goto DONE;
    }

    ret=0;


DONE:
#endif

    return ret;

}

int mxp_init_nor_flash(void)
{
    int ret=-1;
    //printk(KERN_WARNING"[mxp_init_nor_flash]\n");
    if(nor_flash_mxp==NULL)
    {
        nor_flash_mxp=kmalloc(sizeof(mxp_manager),GFP_KERNEL);
        if(nor_flash_mxp==NULL)
        {
            printk(KERN_ERR "ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
            goto DONE;
        }

        //printk(KERN_WARNING"nor_flash_mxp allocated success!!\n");

    }
    else
    {
        return 1;
    }

//    if(norflash==NULL)
//    {
//      norflash = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);
//      if (!norflash) {
//            printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
//            goto DONE;
//      }
//    }

    nor_flash_mxp->read_table_bytes=nor_flash_mxp_read_bytes;
    nor_flash_mxp->write_table_bytes=nor_flash_mxp_write_bytes;
    nor_flash_mxp->update_table_byte=nor_flash_mxp_update_byte;

    if(0!=mxp_init(NOR_FLASH_MXP_TABLE_SIZE,nor_flash_mxp))
    {
        printk(KERN_ERR "ERROR!! [%s] can't find mxp table\n",__FUNCTION__);
        goto DONE;
    }

    ret=0;
DONE:
    return ret;

}
#endif


