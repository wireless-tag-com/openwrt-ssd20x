/*
* mtd_isp.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include <common.h>
#include <flash.h>
#include <malloc.h>

#include <asm/errno.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/concat.h>
//#include <mtd/cfi_flash.h>

#include "MsTypes.h"
#include "drvDeviceInfo.h"
#include "drvSERFLASH.h"
extern MS_BOOL bDetect;
extern hal_SERFLASH_t _hal_SERFLASH;


#include "asm/arch/mach/platform.h"

#ifdef CONFIG_MS_MTD_ISP_FLASH
//#define CONFIG_MTD_PARTITIONS


/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_COUNT	100000
#define	CMD_SIZE		4



#ifdef CONFIG_MTD_PARTITIONS
#define	mtd_has_partitions()	(1)
#else
#define	mtd_has_partitions()	(0)
#endif

#define mutex_init(...)
#define mutex_lock(...)
#define mutex_unlock(...)
//struct mutex
//{
//};

//#define DEBUG(x...) printk(x...)

struct serflash
{
    struct mutex        lock;
    struct mtd_info     mtd;
    unsigned            partitioned:1;
    u8                  erase_opcode;
};


static inline struct serflash *mtd_to_serflash(struct mtd_info *mtd)
{
    return container_of(mtd, struct serflash, mtd);
}

/* Erase flash fully or part of it */
static int serflash_erase(struct mtd_info *mtd, struct erase_info *instr)
{

    struct serflash *flash = mtd_to_serflash(mtd);
    flash=flash;
    //ulong addr,len,start_sec,end_sec;
    ulong addr,len;
//    uint64_t addr_bak;

    printk(KERN_WARNING"%s: addr 0x%08x, len %ld\n",
           __func__, (u32)instr->addr, (long int)instr->len);

    /* range and alignment check */
    if (instr->addr + instr->len > mtd->size)
        return -EINVAL;

//    addr_bak = instr->addr;
//    if ((do_div(instr->addr , mtd->erasesize)) != 0 ||(do_div(instr->len, mtd->erasesize) != 0))
//    {
//        return -EINVAL;
//    }
//    instr->addr = addr_bak;

    addr = instr->addr;
    len = instr->len;

    mutex_lock(&flash->lock);


    /*write protect false before erase*/
    if (!MDrv_SERFLASH_WriteProtect(0))
    {
        mutex_unlock(&flash->lock);
        return -EIO;
    }
    /* erase the whole chip */
    if (len == mtd->size && !MDrv_SERFLASH_EraseChip())
    {
        instr->state = MTD_ERASE_FAILED;

        MDrv_SERFLASH_WriteProtect(1);
        mutex_unlock(&flash->lock);
        return -EIO;
    }
    else
    {
        if (len)
        {
            if (!MDrv_SERFLASH_AddressErase(addr, len, 1))
            {
                instr->state = MTD_ERASE_FAILED;

                MDrv_SERFLASH_WriteProtect(1);
                mutex_unlock(&flash->lock);
                return -EIO;
            }
        }
    }

    MDrv_SERFLASH_WriteProtect(1);
    mutex_unlock(&flash->lock);

    instr->state = MTD_ERASE_DONE;

    mtd_erase_callback(instr);

    return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int serflash_read(struct mtd_info *mtd, loff_t from, size_t len,
                         size_t *retlen, u_char *buf)
{
    struct serflash *flash = mtd_to_serflash(mtd);

    printk(KERN_WARNING "%s %s 0x%08x, len %zd\n",
           __func__, "from", (u32)from, len);

    /* sanity checks */
    if (!len)
        return 0;

    if (from + len > flash->mtd.size)
        return -EINVAL;

    mutex_lock(&flash->lock);

#if 0
    /* Wait till previous write/erase is done. */
    if (wait_till_ready(flash))
    {
        /* REVISIT status return?? */
        mutex_unlock(&flash->lock);
        return 1;
    }
#endif

    if (MDrv_SERFLASH_Read(from, len, (unsigned char *)buf))
    {
        *retlen = len;
    }
    else
    {
        *retlen = 0;
        mutex_unlock(&flash->lock);
        return -EIO;
    }

    mutex_unlock(&flash->lock);

    return 0;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int serflash_write(struct mtd_info *mtd, loff_t to, size_t len,
                          size_t *retlen, const u_char *buf)
{
    struct serflash *flash = mtd_to_serflash(mtd);
//    u32 erase_start, erase_end;

    printk(KERN_WARNING "%s %s 0x%08x, len %zd\n",
           __func__, "to", (u32)to, len);

    if (retlen)
        *retlen = 0;

    /* sanity checks */
    if (!len)
        return(0);

    if (to + len > flash->mtd.size)
        return -EINVAL;

    // calculate erase sectors
//    erase_start = (u32)to / mtd->erasesize;
//    erase_end = (u32)(to+len) / mtd->erasesize - 1;

    mutex_lock(&flash->lock);

#if 0
    /* Wait until finished previous write command. */
    if (wait_till_ready(flash))
    {
        mutex_unlock(&flash->lock);
        return 1;
    }
#endif

    if (!MDrv_SERFLASH_WriteProtect(0))
    {
        mutex_unlock(&flash->lock);
        return -EIO;
    }
//modified by daniel.lee 2010/0514
    /*
           if (!MDrv_SERFLASH_BlockErase(erase_start, erase_end, TRUE))
           {
               mutex_unlock(&flash->lock);
    	    return -EIO;
           }
    */
    if (MDrv_SERFLASH_Write(to, len, (unsigned char *)buf))
    {
        if (retlen)
            *retlen = len;
    }
    else
    {
        if (retlen)
            *retlen = 0;

        MDrv_SERFLASH_WriteProtect(1);
        mutex_unlock(&flash->lock);
        return -EIO;
    }

    MDrv_SERFLASH_WriteProtect(1);
    mutex_unlock(&flash->lock);

    return 0;
}

#endif //CONFIG_MS_MTD_ISP_FLASH


/*

#define MSTAR_SERFLASH_SIZE					(8 * 1024 * 1024)



#define SERFLASH_PART_PARTITION_0_OFFSET    0
#define SERFLASH_PART_PARTITION_0_SIZE		(32+512+32) * 1024

#define SERFLASH_PART_PARTITION_1_OFFSET	(SERFLASH_PART_PARTITION_0_OFFSET + SERFLASH_PART_PARTITION_0_SIZE)
#define SERFLASH_PART_PARTITION_1_SIZE		512 * 1024

#define SERFLASH_PART_PARTITION_2_OFFSET    (SERFLASH_PART_PARTITION_1_OFFSET + SERFLASH_PART_PARTITION_1_SIZE)
#define SERFLASH_PART_PARTITION_2_SIZE      512 * 1024

#define SERFLASH_PART_PARTITION_3_OFFSET    (SERFLASH_PART_PARTITION_2_OFFSET + SERFLASH_PART_PARTITION_2_SIZE)
#define SERFLASH_PART_PARTITION_3_SIZE      MSTAR_SERFLASH_SIZE - SERFLASH_PART_PARTITION_3_OFFSET

//
//#if ( (NAND_PART_APPLICATION_OFFSET) >= MSTAR_SERFLASH_SIZE)
//    #error "Error: NAND partition is not correct!!!"
//#endif


static const struct mtd_partition serflash_partition_info[] =
{
    {
        .name   = "boot",
        .offset = SERFLASH_PART_PARTITION_0_OFFSET,
        .size   = SERFLASH_PART_PARTITION_0_SIZE
    },
    {
        .name   = "kernel",
        .offset = SERFLASH_PART_PARTITION_1_OFFSET,
        .size   = SERFLASH_PART_PARTITION_1_SIZE
    },
    {
        .name   = "rootfs",
        .offset = SERFLASH_PART_PARTITION_2_OFFSET,
        .size   = SERFLASH_PART_PARTITION_2_SIZE
    },
    {
        .name   = "user",
        .offset = SERFLASH_PART_PARTITION_3_OFFSET,
        .size   = SERFLASH_PART_PARTITION_3_SIZE
    }

};

#define SERFLASH_NUM_PARTITIONS ARRAY_SIZE(serflash_partition_info)
*/


#ifdef CONFIG_MS_MTD_ISP_FLASH
unsigned int g_size=0;
/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */

int  serflash_probe(void)
{
    struct serflash			*flash;
    struct flash_info		*info;
    unsigned			i;

    // jedec_probe() will read id, so initialize hardware first
    MDrv_SERFLASH_Init();

  
    if (bDetect  == FALSE)
    {
        printk(KERN_WARNING"no flash_info!!");
        return -ENODEV;
    }

    flash = kzalloc(sizeof *flash, GFP_KERNEL);
    if (!flash)
        return -ENOMEM;

    mutex_init(&flash->lock);

    flash->mtd.priv = flash;

#if 0
    /*
     * Atmel serial flash tend to power up
     * with the software protection bits set
     */

    if (info->jedec_id >> 16 == 0x1f)
    {
        write_enable(flash);
        write_sr(flash, 0);
    }
#endif

    flash->mtd.name ="nor0";
    flash->mtd.type = MTD_NORFLASH;
    flash->mtd.writesize = 1;
    flash->mtd.flags = MTD_CAP_NORFLASH;
    MDrv_SERFLASH_DetectSize((MS_U32*)&flash->mtd.size);
    flash->mtd._erase = serflash_erase;
    flash->mtd._read = serflash_read;
    flash->mtd._write = serflash_write;

    g_size = flash->mtd.size;
    flash->mtd.erasesize = (_hal_SERFLASH.u32SecSize);

    //dev_info(&spi->dev, "%s (%d Kbytes)\n", info->name,
    //flash->mtd.size / 1024);

    printk(
           "mtd .name = %s, .size = 0x%.8x (%uMiB) "
           ".erasesize = 0x%.8x .numeraseregions = %d\n",
           flash->mtd.name,
           (unsigned int)flash->mtd.size, (unsigned int)(flash->mtd.size >>20),
           (unsigned int)flash->mtd.erasesize,
           flash->mtd.numeraseregions);

    if (flash->mtd.numeraseregions)
        for (i = 0; i < flash->mtd.numeraseregions; i++)
            printk(
                   "mtd.eraseregions[%d] = { .offset = 0x%.8x, "
                   ".erasesize = 0x%.8x (%uKiB), "
                   ".numblocks = %d }\n",
                   i, (u32)flash->mtd.eraseregions[i].offset,
                   (unsigned int)flash->mtd.eraseregions[i].erasesize,
                   (flash->mtd.eraseregions[i].erasesize >>10 ),
                   (unsigned int)flash->mtd.eraseregions[i].numblocks);


    /* partitions should match sector boundaries; and it may be good to
     * use readonly partitions for writeprotected sectors (BP2..BP0).
     */

/*
    if (mtd_has_partitions())
    {
        struct mtd_partition    *parts = NULL;
        int    nr_parts = 0;


        if (nr_parts > 0)
        {
            for (i = 0; i < nr_parts; i++)
            {
                printk(KERN_WARNING "partitions[%d] = "
                       "{.name = %s, .offset = 0x%.8x, "
                       ".size = 0x%.8x (%uKiB) }\n",
                       i, parts[i].name,
                       (unsigned int)(parts[i].offset),
                       (unsigned int)(parts[i].size),
                       (unsigned int)(parts[i].size / 1024));
            }
            flash->partitioned = 1;

            return add_mtd_partitions(&flash->mtd, parts, nr_parts);

        }
        else
            return add_mtd_partitions(&flash->mtd, serflash_partition_info, SERFLASH_NUM_PARTITIONS);

    }
    */

    return add_mtd_device(&flash->mtd) == 1 ? -ENODEV : 0;

}



unsigned long flash_init (void)
{
    serflash_probe();

    if (g_size>0)
        return (g_size);

    return 0;
}

#endif // CONFIG_MS_MTD_ISP_FLASH

#ifdef CONFIG_CMD_SF
#include <spi_flash.h>
int _spi_flash_read(struct spi_flash *flash, u32 offset,
		size_t len, void *buf)
{
    //printk(KERN_WARNING "%s from 0x%x, len 0x%x, to 0x%x ", __func__, (u32)offset, len, (unsigned int)buf);
    u32 ret = -EIO;
    u32 readsize = 0;
    //u32 starttime = get_timer(0);

    /* sanity checks */
    if (!len)
        return -EINVAL;

    if (offset + len > flash->size)
        return -EINVAL;
#if 0
    if (MDrv_SERFLASH_Read(offset, len, (unsigned char *)buf))
    {
        ret = 0;
    }
#else
    if (offset < 0x1000000)
    {
        MDrv_SERFLASH_WriteExtAddrRegister(0);
        readsize = len > (0x1000000 - offset) ? (0x1000000 - offset) : len;
        memcpy(buf, (const void *)(MS_SPI_ADDR|offset), readsize);
        len = len - readsize;
    }
    if (len)
    {
        u32 regEAR = 0;
        offset += readsize;
        regEAR = offset >> 24;
        if (regEAR)
            MDrv_SERFLASH_WriteExtAddrRegister(regEAR);
        memcpy(buf+readsize, (const void *)(MS_SPI_ADDR|(offset&0xFFFFFF)), len);
        if (regEAR)
            MDrv_SERFLASH_WriteExtAddrRegister(0);
    }
    ret = 0;
#endif

    //printk(KERN_WARNING "(cost %ld ms)\n", get_timer(0) - starttime);
    return ret;
}

int _spi_flash_write(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
    printk(KERN_WARNING "%s to 0x%x, len 0x%x from 0x%x ", __func__,(u32)offset, len, (unsigned int)buf);
    u32 ret = -EIO;
    u32 starttime = get_timer(0);

    /* sanity checks */
    if (!len)
        return (-EINVAL);
    if (offset + len > flash->size)
        return (-EINVAL);

    if (!MDrv_SERFLASH_WriteProtect(0))
    {
        return (-EIO);
    }

    if (MDrv_SERFLASH_Write(offset, len, (unsigned char *)buf))
    {
        ret = 0;
    }

    MDrv_SERFLASH_WriteProtect(1);

    printk(KERN_WARNING "(cost %ld ms)\n", get_timer(0) - starttime);
    return ret;
}
#define FLASH_BLOCK_SIZE 65536

int _spi_flash_erase(struct spi_flash *flash, u32 offset,
		size_t len)
{

    printk(KERN_WARNING"%s: addr 0x%x, len 0x%x ", __func__, (u32)offset, len);
    u32 ret = -EIO;
    u32 u32ErasedSize;
    u32 starttime = get_timer(0);

    /* range and alignment check */
    if (!len)
        return (-EINVAL);
    if (offset + len > flash->size)
        return (-EINVAL);

    /*write protect false before erase*/
    if (!MDrv_SERFLASH_WriteProtect(0))
    {
        return -EIO;
    }

    /* erase the whole chip */
    //if (len == flash->size)
    //{
    //    ret = MDrv_SERFLASH_EraseChip();
    //    goto Done;
    //}

    if(offset % FLASH_BLOCK_SIZE)
    {
        u32ErasedSize = FLASH_BLOCK_SIZE - offset%FLASH_BLOCK_SIZE;
        if (len < u32ErasedSize)
        {
            u32ErasedSize = len;
        }
        if(MDrv_SERFLASH_SectorErase(offset, offset+u32ErasedSize-1))
        {
            offset += u32ErasedSize;
            len -= u32ErasedSize;
            ret = 0;
        }
        else
        {
            ret =  (-EINVAL);
            goto Done;
        }
    }
    if(len >= FLASH_BLOCK_SIZE)
    {
        u32ErasedSize = len - len%FLASH_BLOCK_SIZE;

        if( MDrv_SERFLASH_AddressErase(offset, u32ErasedSize, 1))
        {
            offset += u32ErasedSize;
            len -= u32ErasedSize;
            ret = 0;
        }
        else
        {
             ret =  (-EINVAL);
            goto Done;
        }
    }

    if(len)
    {
        u32ErasedSize = len;
        if(MDrv_SERFLASH_SectorErase(offset, offset+u32ErasedSize-1))
        {
            offset += u32ErasedSize;
            len -= u32ErasedSize;
            ret = 0;
        }
        else
        {
             ret =  (-EINVAL);
            goto Done;
        }
    }


Done:
    MDrv_SERFLASH_WriteExtAddrRegister(0);
    MDrv_SERFLASH_WriteProtect(1);
    printk(KERN_WARNING "(cost %ld ms)\n", get_timer(0) - starttime);

	return ret;
}


struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode)
{
//	struct spi_slave *spi;
	struct spi_flash *flash = NULL;
    //const SERFLASH_Info *Info = NULL;

/*
	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
	if (!spi) {
		printf("SF: Failed to set up slave\n");
		return err_manufacturer_probe;
	}
*/

    // jedec_probe() will read id, so initialize hardware first
    MDrv_SERFLASH_Init();


    //bDetect is the global variable from halserflash.c
    if (bDetect  == FALSE)
    {
        printk(KERN_WARNING"no flash_info!!");
        goto err_manufacturer_probe;
    }

	flash = malloc(sizeof(struct spi_flash));
	if (!flash) {
		debug("SF: Failed to allocate memory\n");
		goto err_mem;
	}

	flash->spi = NULL;
	flash->name = "nor0";

	flash->write = _spi_flash_write;
	flash->erase = _spi_flash_erase;
	flash->read = _spi_flash_read;
    //Info = MDrv_SERFLASH_GetInfo();
	flash->page_size = (_hal_SERFLASH.u16PageSize);
	flash->sector_size = (_hal_SERFLASH.u32SecSize);
    MDrv_SERFLASH_DetectSize((MS_U32*)&flash->size);


	if (!flash) {
		printf("SF: Unsupported manufacturer\n");
		goto err_manufacturer_probe;
	}

	printf("SF: Detected %s with total size ", flash->name);
	print_size(flash->size, "\n");


	return flash;

err_manufacturer_probe:
//err_read_id:
//err_claim_bus:
err_mem:

	return NULL;
}

void spi_flash_free(struct spi_flash *flash)
{
	free(flash);
}


#endif


