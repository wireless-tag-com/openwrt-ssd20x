/*
* mxp_flash.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: alterman.lin <alterman.lin@sigmastar.com.tw>
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

#include "common.h"
#include "part_mxp.h"
#include "malloc.h"
#include <spi_flash.h>
#include <spi.h>

#ifdef CONFIG_MS_ISP_FLASH
static mxp_manager* nor_flash_mxp=NULL;

#define NOR_FLASH_MXP_TABLE_SIZE	0x1000

static struct spi_flash *norflash=NULL;

static int nor_flash_mxp_read_bytes(int offset,void* buf,int len)
{
    int ret;
	ret = spi_flash_read(norflash, offset, len, buf);
	if (ret) {
		printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
		return -1;
	}

	return 0;
}


static int nor_flash_mxp_write_bytes(int offset,void* buf,int len)
{
    int ret;
	ret = spi_flash_erase(norflash, offset, NOR_FLASH_MXP_TABLE_SIZE);
	if (ret) {
		printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
		return -1;
	}

	ret = spi_flash_write(norflash, offset, len, buf);
	if (ret) {
		printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
		return -1;
	}

	return 0;
}

static int nor_flash_mxp_update_byte(int offset, char byte)
{
    char b;
    int ret=-1;
    if(spi_flash_read(norflash, offset, (size_t)1, &b))
    {
    	printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
    	goto DONE;

    }

    printf("[nor_flash_mxp_update_byte] 0x%08X: 0x%02X -> 0x%02X\n",offset,b,byte);
    if( (((char)(byte^b)) & (char)(~b)) >0 )
    {
    	printf("  using general page flash write\n");
    	return nor_flash_mxp_write_bytes(offset,&byte,1);
    }

    printf("  using single byte flash write\n");
    if(spi_flash_write(norflash, offset, 1, &byte)) {
			printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
			goto DONE;
	}
    ret=0;


DONE:
	return ret;

}

static int nor_flash_mxp_get_storage_info(mxp_storage_info* info)
{

	info->type=MXP_STORAGE_TYPE_FLASH;
	info->size=norflash->size;
	return 0;
}

int mxp_init_nor_flash(void)
{
	//printf("[mxp_init_nor_flash]\n");
	int ret=-1;
	if(nor_flash_mxp==NULL)
	{
		nor_flash_mxp=malloc(sizeof(mxp_manager));
		if(nor_flash_mxp==NULL)
		{
			printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
			goto DONE;
		}

		printf("nor_flash_mxp allocated success!!\n");

	}
	else
	{
		return 0;
	}

    if(norflash==NULL)
    {
	    norflash = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);
    	if (!norflash) {
            printf("ERROR!! [%s]: %d\n",__FUNCTION__,__LINE__);
            goto DONE;
	    }
    }

	nor_flash_mxp->read_table_bytes=nor_flash_mxp_read_bytes;
	nor_flash_mxp->write_table_bytes=nor_flash_mxp_write_bytes;
	nor_flash_mxp->update_table_byte=nor_flash_mxp_update_byte;
	nor_flash_mxp->get_storage_info=nor_flash_mxp_get_storage_info;

	if(0!=mxp_init(NOR_FLASH_MXP_TABLE_SIZE,nor_flash_mxp))
	{
		printf("ERROR!! [%s] can't find mxp table\n", __FUNCTION__);
		goto DONE;
	}

	ret=0;
DONE:
	return ret;

}
#endif


