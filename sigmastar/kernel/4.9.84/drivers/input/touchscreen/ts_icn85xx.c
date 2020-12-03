/*
* drivers/input/touchscreen/ak37d_icn85xx.c
*
* Copyright (c) 2019 Zhipeng Zhang <zhangzhipeng@anyka.com>
* Copyright (c) 2019 Anyka Technology (Guangzhou) Co., Ltd.

* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*/

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include <linux/irq.h>
#include <linux/interrupt.h>

#include <asm/uaccess.h>

#include <linux/hrtimer.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>

#include <linux/kthread.h>

#include "ts_icn85xx.h"


#define DRIVER_VERSION "1.0.0"

static struct file  *fp;
static int g_status = R_OK;
static char fw_mode = 0;
static int fw_size = 0;
static unsigned char *fw_buf;
static char boot_mode = ICN85XX_WITHOUT_FLASH_87;

unsigned short icnt87_sram_crc = 0;
unsigned short icnt87_sram_length = 0;

void setbootmode(char bmode)
{
    boot_mode= bmode;
}
unsigned short const Crc16Table[]= {
    0x0000,0x8005,0x800F,0x000A,0x801B,0x001E,0x0014,0x8011,0x8033,0x0036,0x003C,0x8039,0x0028,0x802D,0x8027,0x0022,
    0x8063,0x0066,0x006C,0x8069,0x0078,0x807D,0x8077,0x0072,0x0050,0x8055,0x805F,0x005A,0x804B,0x004E,0x0044,0x8041,
    0x80C3,0x00C6,0x00CC,0x80C9,0x00D8,0x80DD,0x80D7,0x00D2,0x00F0,0x80F5,0x80FF,0x00FA,0x80EB,0x00EE,0x00E4,0x80E1,
    0x00A0,0x80A5,0x80AF,0x00AA,0x80BB,0x00BE,0x00B4,0x80B1,0x8093,0x0096,0x009C,0x8099,0x0088,0x808D,0x8087,0x0082,
    0x8183,0x0186,0x018C,0x8189,0x0198,0x819D,0x8197,0x0192,0x01B0,0x81B5,0x81BF,0x01BA,0x81AB,0x01AE,0x01A4,0x81A1,
    0x01E0,0x81E5,0x81EF,0x01EA,0x81FB,0x01FE,0x01F4,0x81F1,0x81D3,0x01D6,0x01DC,0x81D9,0x01C8,0x81CD,0x81C7,0x01C2,
    0x0140,0x8145,0x814F,0x014A,0x815B,0x015E,0x0154,0x8151,0x8173,0x0176,0x017C,0x8179,0x0168,0x816D,0x8167,0x0162,
    0x8123,0x0126,0x012C,0x8129,0x0138,0x813D,0x8137,0x0132,0x0110,0x8115,0x811F,0x011A,0x810B,0x010E,0x0104,0x8101,
    0x8303,0x0306,0x030C,0x8309,0x0318,0x831D,0x8317,0x0312,0x0330,0x8335,0x833F,0x033A,0x832B,0x032E,0x0324,0x8321,
    0x0360,0x8365,0x836F,0x036A,0x837B,0x037E,0x0374,0x8371,0x8353,0x0356,0x035C,0x8359,0x0348,0x834D,0x8347,0x0342,
    0x03C0,0x83C5,0x83CF,0x03CA,0x83DB,0x03DE,0x03D4,0x83D1,0x83F3,0x03F6,0x03FC,0x83F9,0x03E8,0x83ED,0x83E7,0x03E2,
    0x83A3,0x03A6,0x03AC,0x83A9,0x03B8,0x83BD,0x83B7,0x03B2,0x0390,0x8395,0x839F,0x039A,0x838B,0x038E,0x0384,0x8381,
    0x0280,0x8285,0x828F,0x028A,0x829B,0x029E,0x0294,0x8291,0x82B3,0x02B6,0x02BC,0x82B9,0x02A8,0x82AD,0x82A7,0x02A2,
    0x82E3,0x02E6,0x02EC,0x82E9,0x02F8,0x82FD,0x82F7,0x02F2,0x02D0,0x82D5,0x82DF,0x02DA,0x82CB,0x02CE,0x02C4,0x82C1,
    0x8243,0x0246,0x024C,0x8249,0x0258,0x825D,0x8257,0x0252,0x0270,0x8275,0x827F,0x027A,0x826B,0x026E,0x0264,0x8261,
    0x0220,0x8225,0x822F,0x022A,0x823B,0x023E,0x0234,0x8231,0x8213,0x0216,0x021C,0x8219,0x0208,0x820D,0x8207,0x0202,
};
void icn85xx_rawdatadump(short *mem, int size, char br)
{
    int i;
    for(i=0;i<size; i++)
    {
        if((i!=0)&&(i%br == 0))
            printk("\n");
        printk(" %5d", mem[i]);
    }
    printk("\n");
}

void icn85xx_memdump(char *mem, int size)
{
    int i;
    for(i=0;i<size; i++)
    {
        if(i%16 == 0)
            printk("\n");
        printk(" 0x%2x", mem[i]);
    }
    printk("\n");
}

int  icn85xx_checksum(int sum, char *buf, unsigned int size)
{
    int i;
    for(i=0; i<size; i++)
    {
        sum = sum + buf[i];
    }
    return sum;
}


int icn85xx_update_status(int status)
{
    g_status = status;
    return 0;
}

int icn85xx_get_status(void)
{
    return  g_status;
}

void icn85xx_set_fw(int size, unsigned char *buf)
{
    fw_size = size;
    fw_buf = buf;

}

int  icn85xx_open_fw( char *fw)
{
    int file_size;
    mm_segment_t fs;
    struct inode *inode = NULL;
    if(strcmp(fw, "icn85xx_firmware") == 0)
    {
        fw_mode = 1;  //use inner array
        return fw_size;
    }
    else
    {
        fw_mode = 0; //use file in file system
    }

    fp = filp_open(fw, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        pr_err("read fw file error\n");
        return -1;
    }
    else
        pr_info("open fw file ok\n");

    //inode = fp->f_dentry->d_inode;
    inode = file_inode(fp);
    file_size = inode->i_size;

    fs = get_fs();
    set_fs(KERNEL_DS);

    return  file_size;

}


int  icn85xx_read_fw(int offset, int length, char *buf)
{
    loff_t  pos = offset;
    if(fw_mode == 1)
    {
        memcpy(buf, fw_buf+offset, length);
    }
    else
    {
        vfs_read(fp, buf, length, &pos);
    }

    return 0;
}

int  icn85xx_close_fw(void)
{
    if(fw_mode == 0)
    {
        filp_close(fp, NULL);
    }

    return 0;
}

int icn85xx_readVersion(void)
{
    int err = 0;
    char tmp[2];
    short CurVersion;
    err = icn85xx_i2c_rxdata(0x000c, tmp, 2);
    if (err < 0) {
        pr_err("%s failed: %d\n", __func__, err);
        return 0;
    }
    CurVersion = (tmp[0]<<8) | tmp[1];
    return CurVersion;
}

int icn85xx_goto_progmode(void)
{
    int ret = -1;
    int retry = 3;
    unsigned char ucTemp;

    while(retry > 0)
    {
        icn85xx_set_prog_addr();
        ucTemp = 0x5a;
        ret = icn85xx_prog_i2c_txdata(0xcc3355, &ucTemp,1);
        if( ret < 0)
        {
            retry -- ;
            mdelay(2);
        }
        else
            break;

    }
    pr_info("icn85xx_goto_progmode over, ret: %d\n", ret);
    if(retry == 0)
        return -1;

    return 0;
}

int icn85xx_check_progmod(void)
{
    int ret;
    unsigned char ucTemp = 0x0;
    ret = icn85xx_prog_i2c_rxdata(0x040002, &ucTemp, 1);

    if(ret < 0)
    {
        pr_err("icn85xx_check_progmod error, ret: %d\n", ret);
        return ret;
    }
    if(ucTemp == 0x85)
        return 0;
    else
        return -1;

}

unsigned char FlashState(unsigned char State_Index)
{
    unsigned char ucTemp[4] = {0,0,0,0};

    ucTemp[2]=0x08;
    ucTemp[1]=0x10;
    ucTemp[0]=0x00;
    icn85xx_prog_i2c_txdata(0x4062c,ucTemp,3);

    if(State_Index==0)
    {
        ucTemp[0]=0x05;
    }
    else if(State_Index==1)
    {
        ucTemp[0]=0x35;
    }
    icn85xx_prog_i2c_txdata(0x40630,ucTemp,1);

    ucTemp[1]=0x00;
    ucTemp[0]=0x01;
    icn85xx_prog_i2c_txdata(0x40640,ucTemp,2);

    ucTemp[0]=1;
    icn85xx_prog_i2c_txdata(0x40644,ucTemp,1);
    while(ucTemp[0])
    {
        icn85xx_prog_i2c_rxdata(0x40644,ucTemp,1);
    }

    icn85xx_prog_i2c_rxdata(0x40648,ucTemp,1);
    return (unsigned char)(ucTemp[0]);
}

int  icn85xx_read_flashid(void)
{
    unsigned char ucTemp[4] = {0,0,0,0};
    int flashid=0;

    ucTemp[2]=0x08;
    ucTemp[1]=0x10;
    ucTemp[0]=0x00;
    icn85xx_prog_i2c_txdata(0x4062c,ucTemp,3);

    ucTemp[0]=0x9f;

    icn85xx_prog_i2c_txdata(0x40630,ucTemp,1);

    ucTemp[1] = 0x00;
    ucTemp[0] = 0x03;

    icn85xx_prog_i2c_txdata(0x40640,ucTemp,2);

    ucTemp[0]=1;
    icn85xx_prog_i2c_txdata(0x40644,ucTemp,1);
    while(ucTemp[0])
    {
        icn85xx_prog_i2c_rxdata(0x40644,ucTemp,1);
    }

    icn85xx_prog_i2c_rxdata(0x40648,(char *)&flashid,4);
    flashid=flashid&0x00ffffff;

    pr_info("flashid: 0x%x\n", flashid);
    return flashid;
}


int icn87xx_boot_sram(void)
{
    int ret = -1;
    unsigned char ucTemp = 0x03;
    ret = icn87xx_prog_i2c_txdata(0xf400,&ucTemp,1);
    return ret;
}
int icn87xx_boot_flash(void)
{
    int ret = -1;
    unsigned char ucTemp = 0x7f;
    ret = icn87xx_prog_i2c_txdata(0xf008,&ucTemp,1);//chip rest
    return ret;
}
int icn87xx_read_flashstate(void)
{
    int ret = -1;
    unsigned char ucTemp[2] = {0 , 0};
    ucTemp[0] = 1;
    while(ucTemp[0])
    {
        ret = icn87xx_prog_i2c_rxdata(SF_BUSY_87,&ucTemp[0],1);
        if(ret < 0)
        {
            return ret;
        }
    }
    ucTemp[0] = FLASH_CMD_READ_STATUS;
    ret = icn87xx_prog_i2c_txdata(CMD_SEL_87, &ucTemp[0],1);
    if(ret < 0)
    {
        return ret;
    }

    ucTemp[1] = (unsigned char)(SRAM_EXCHANGE_ADDR>>8);
    ucTemp[0] = (unsigned char)(SRAM_EXCHANGE_ADDR);
    ret = icn87xx_prog_i2c_txdata(SRAM_ADDR_87, &ucTemp[0],2);
    if(ret < 0)
    {
        return ret;
    }
    ucTemp[0] = 1;
    ret = icn87xx_prog_i2c_txdata(START_DEXC_87, &ucTemp[0],1);
    if(ret < 0)
    {
        return ret;
    }

    while(ucTemp[0])
    {
        ret = icn87xx_prog_i2c_rxdata(SF_BUSY_87,&ucTemp[0],1);
        if(ret < 0)
        {
            return ret;
        }
    }
    ret = icn87xx_prog_i2c_rxdata(SRAM_EXCHANGE_ADDR,&ucTemp[0],1);
    if(ret < 0)
    {
        return ret;
    }
    return ucTemp[0];
}

int  icn87xx_read_flashid(void)
{
   int ret = -1;
   int flash_id;
   unsigned char ucTemp[4];

    ucTemp[0] = FLASH_CMD_READ_IDENTIFICATION;
    ret = icn87xx_prog_i2c_txdata(CMD_SEL_87, ucTemp,1);
    if(ret < 0)
    {
        return ret;
    }
    ucTemp[1] = (unsigned char)(SRAM_EXCHANGE_ADDR1 >> 8);
    ucTemp[0] = (unsigned char)SRAM_EXCHANGE_ADDR1;
    ret = icn87xx_prog_i2c_txdata(SRAM_ADDR_87, ucTemp,2);
    if(ret < 0)
    {
        return ret;
    }
    ucTemp[0] = 1;
    ret = icn87xx_prog_i2c_txdata(START_DEXC_87, ucTemp,1);
    if(ret < 0)
    {
        return ret;
    }

    while(ucTemp[0])
    {
        ret = icn87xx_prog_i2c_rxdata(SF_BUSY_87,&ucTemp[0],1);
        if(ret < 0)
        {
            return ret;
        }
    }
    ret = icn87xx_prog_i2c_rxdata(SRAM_EXCHANGE_ADDR1, ucTemp,4);
    if(ret < 0)
    {
        return ret;
    }
    flash_id = (((ucTemp[0])<<16) + ((ucTemp[1])<<8) + ucTemp[2]);

    return flash_id;
}

int icn87xx_calculate_crc(unsigned short len)
{
    unsigned char ucTemp[4];
    int ret = -1;
    int crc = 0;

    //2.1 set address
    ucTemp[0] = 0x00;
    ucTemp[1] = 0x00;
    ret = icn87xx_prog_i2c_txdata(SRAM_ADDR_87, ucTemp,2);
    if(ret < 0)
    {
        return ret;
    }

    ucTemp[0] = len & 0xff;
    ucTemp[1] = (len >> 8 )& 0xff;
    ret = icn87xx_prog_i2c_txdata(DATA_LENGTH_87, ucTemp,2);
    if(ret < 0)
    {
        return ret;
    }

    ucTemp[0] = 0x01;
    ret = icn87xx_prog_i2c_txdata(SW_CRC_START_87, ucTemp,1);
    if(ret < 0)
    {
        return ret;
    }

    //2.4 poll status
    while(ucTemp[0])
    {
        ret = icn87xx_prog_i2c_rxdata(SF_BUSY_87, ucTemp,1);
        if(ret < 0)
        {
            return ret;
        }

    }
    ret = icn87xx_prog_i2c_rxdata(CRC_RESULT_87, ucTemp,2);
    if(ret < 0)
    {
        return ret;
    }
    crc = (((ucTemp[1])<<8) + (ucTemp[0]))&0x0000ffff;

    return crc;
}
unsigned short icn87xx_fw_Crc(unsigned short crc,unsigned char *buf ,unsigned short size)
{
    unsigned short u16CrcValue = crc;
    unsigned short u16Length = size;
    unsigned char  u8CheckData = 0;
    while(u16Length)
    {
        u8CheckData = *buf++;
        u16CrcValue = (u16CrcValue << 8)^Crc16Table[(u16CrcValue >> 8)^((u8CheckData)&0xff)];
        u16Length--;
    }
    return u16CrcValue;
}


int icn87xx_erase_flash(U8 u8EraseMode, U32 u32FlashAddr)
{
    U8 ucTemp[4];
    int ret = -1;
    if(FLASH_EARSE_4K == u8EraseMode)
    {
        ucTemp[0] = FLASH_CMD_ERASE_SECTOR;
    }
    else if(FLASH_EARSE_32K == u8EraseMode)
    {
        ucTemp[0] = FLASH_CMD_ERASE_BLOCK;
    }
    ret = icn87xx_prog_i2c_txdata(CMD_SEL_87, ucTemp,1);
    if(ret < 0)
    {
        return ret;
    }

    ucTemp[2] =(U8)(u32FlashAddr >> 16);
    ucTemp[1] =(U8)(u32FlashAddr >> 8);
    ucTemp[0] =(U8)(u32FlashAddr );
    ret = icn87xx_prog_i2c_txdata(FLASH_ADDR_87, ucTemp,3);
    if(ret < 0)
    {
        return ret;
    }
    ucTemp[0] = 1;
    ret = icn87xx_prog_i2c_txdata(START_DEXC_87, ucTemp,1);
    if(ret < 0)
    {
        return ret;
    }

    while(ucTemp[0])
    {
        ret = icn87xx_read_flashstate();
        if(ret < 0)
        {
            return ret;
        }
        ucTemp[0] = (unsigned char)(ret&0x01);
    }

    return 1;

}
int icn87xx_erase_chip(void)
{
    int ret = -1;
    ret = icn87xx_erase_flash(FLASH_EARSE_4K, 0xe000);
    if(ret < 0)
    {
        return ret;
    }
    ret = icn87xx_erase_flash(FLASH_EARSE_32K, 0);
    if(ret < 0)
    {
        return ret;
    }
    ret = icn87xx_erase_flash(FLASH_EARSE_4K, 0x8000);
    if(ret < 0)
    {
        return ret;
    }
    ret = icn87xx_erase_flash(FLASH_EARSE_4K, 0x9000);
    if(ret < 0)
    {
        return ret;
    }
    ret = icn87xx_erase_flash(FLASH_EARSE_4K, 0xa000);
    if(ret < 0)
    {
        return ret;
    }
    ret = icn87xx_erase_flash(FLASH_EARSE_4K, 0xb000);
    if(ret < 0)
    {
        return ret;
    }
    ret = icn87xx_erase_flash(FLASH_EARSE_4K, 0xc000);
    if(ret < 0)
    {
        return ret;
    }
    ret = icn87xx_erase_flash(FLASH_EARSE_4K, 0xd000);
    if(ret < 0)
    {
        return ret;
    }
    ret = icn87xx_erase_flash(FLASH_EARSE_32K, 0x10000);
    if(ret < 0)
    {
        return ret;
    }

    return ret;
}
int icn87xx_flash_write( U32 u32FlashAddr, U32 u32SramAddr, U32 u32Length)
{
    U8 ucTemp[3] = {0, 0, 0};
    int ret = -1;
    U32 u32FlashTempAddr = u32FlashAddr;
    U32 u32SramTempAddr = u32SramAddr;
    U16 u16NotAlignLength;

    U16 i = 0;
    for(i = 0; i < u32Length;)                         //should not i++
    {
        ucTemp[0] = 1;                                   //confirm the flash whether in busy state
        while(ucTemp[0])
        {
            ret = icn87xx_read_flashstate();
            if(ret < 0)
            {
                return ret;
            }
            ucTemp[0] = (unsigned char)(ret&0x01);

        }
        ucTemp[2] = (U8)(u32FlashTempAddr >> 16);
        ucTemp[1] = (U8)(u32FlashTempAddr >> 8);
        ucTemp[0] = (U8)(u32FlashTempAddr );
        ret = icn87xx_prog_i2c_txdata(FLASH_ADDR_87, ucTemp,3);
        if(ret < 0)
        {
            return ret;
        }

        ucTemp[2] = (U8)(u32SramTempAddr >> 16);
        ucTemp[1] = (U8)(u32SramTempAddr >> 8);
        ucTemp[0] = (U8)(u32SramTempAddr );
        ret = icn87xx_prog_i2c_txdata(SRAM_ADDR_87, ucTemp,3);
        if(ret < 0)
        {
            return ret;
        }

        if(u32FlashTempAddr % 0x100)  // not aglin
        {
            u16NotAlignLength = 0x100 - (u32FlashTempAddr % 0x100);
            if(u32Length <= u16NotAlignLength)
            {
                ucTemp[1] = (U8)(u32Length >> 8);
                ucTemp[0] = (U8)(u32Length );
                ret = icn87xx_prog_i2c_txdata(DATA_LENGTH_87, ucTemp,2);
                if(ret < 0)
                {
                    return ret;
                }

            }
            else
            {
                ucTemp[1] = (U8)(u16NotAlignLength >> 8);
                ucTemp[0] = (U8)(u16NotAlignLength );
                ret = icn87xx_prog_i2c_txdata(DATA_LENGTH_87, ucTemp,2);
                if(ret < 0)
                {
                    return ret;
                }
            }
            u32FlashTempAddr += u16NotAlignLength;                       //change the flash and sram address
            u32SramTempAddr += u16NotAlignLength;
            i += u16NotAlignLength;
        }
        else
        {
            if(i+256<=u32Length)
            {
                ucTemp[1] = (U8)(0x01);
                ucTemp[0] = (U8)(0x00 );
                ret = icn87xx_prog_i2c_txdata(DATA_LENGTH_87, ucTemp,2);
                if(ret < 0)
                {
                    return ret;
                }
            }
            else
            {
                ucTemp[1] = (U8)((u32Length-i) >> 8);
                ucTemp[0] = (U8)(u32Length-i );
                ret = icn87xx_prog_i2c_txdata(DATA_LENGTH_87, ucTemp,2);
                if(ret < 0)
                {
                    return ret;
                }
            }
            u32FlashTempAddr += 256;                       //change the flash and sram address
            u32SramTempAddr += 256;
            i += 256;
        }
        ucTemp[0] = FLASH_CMD_PAGE_PROGRAM;
        ret = icn87xx_prog_i2c_txdata(CMD_SEL_87, ucTemp,1);
        if(ret < 0)
        {
            return ret;
        }
        ucTemp[0] = 1;
        ret = icn87xx_prog_i2c_txdata(START_DEXC_87, ucTemp,1);
        if(ret < 0)
        {
            return ret;
        }

    }

    ucTemp[0] = 1;
    while(ucTemp[0])                                 //confirm whether the last pageprogram whether complete
    {
        ret = icn87xx_read_flashstate();
        if(ret < 0)
        {
            return ret;
        }
        ucTemp[0] = (unsigned char)(ret&0x01);

    }
    return ret;

}
int icn87xx_flash_read(U32 u32SramAddr, U32 u32FlashAddr, U16 u16Length)
{
    U8 ucTemp[4] = {0, 0, 0, 0};
    int ret = -1;
    ucTemp[0] = FLASH_CMD_FAST_READ;
    ret = icn87xx_prog_i2c_txdata(CMD_SEL_87, ucTemp,1);
    if(ret < 0)
    {
        return ret;
    }

    ucTemp[3] = (U8)(u32FlashAddr >> 24);
    ucTemp[2] = (U8)(u32FlashAddr >> 16);
    ucTemp[1] = (U8)(u32FlashAddr >> 8);
    ucTemp[0] = (U8)(u32FlashAddr);
    ret = icn87xx_prog_i2c_txdata(FLASH_ADDR_87, ucTemp,3);
    if(ret < 0)
    {
        return ret;
    }

    ucTemp[3] = (U8)(u32SramAddr >> 24);
    ucTemp[2] = (U8)(u32SramAddr >> 16);
    ucTemp[1] = (U8)(u32SramAddr >> 8);
    ucTemp[0] = (U8)(u32SramAddr);
    ret = icn87xx_prog_i2c_txdata(SRAM_ADDR_87, ucTemp,3);
    if(ret < 0)
    {
        return ret;
    }

    ucTemp[1] = (U8)(u16Length >> 8);
    ucTemp[0] = (U8)(u16Length);
    ret = icn87xx_prog_i2c_txdata(DATA_LENGTH_87, ucTemp,2);
    if(ret < 0)
    {
        return ret;
    }

    ucTemp[0] = 1;
    ret = icn87xx_prog_i2c_txdata(START_DEXC_87, ucTemp,1);
    if(ret < 0)
    {
        return ret;
    }

    while(ucTemp[0])
    {
       ret = icn87xx_read_flashstate();
        if(ret < 0)
        {
            return ret;
        }
        ucTemp[0] = (unsigned char)(ret&0x01);

    }
    return ret;
}

int icn87xx_write_pr_info(U32 u32FlashAddr, U32 u32SramAddr, U8 *buf, U16 u16Length)
{
    int ret = -1;
    ret = icn87xx_prog_i2c_txdata(u32SramAddr, buf, u16Length);
    if(ret < 0)
    {
        return ret;
    }
    ret = icn87xx_flash_write(u32FlashAddr, u32SramAddr, u16Length);
    return ret;
}

void FlashWriteEnable(void)
{
    unsigned char ucTemp[4] = {0,0,0,0};

    ucTemp[2]=0x00;
    ucTemp[1]=0x10;
    ucTemp[0]=0x00;
    icn85xx_prog_i2c_txdata(0x4062c,ucTemp,3);

    ucTemp[0]=0x06;
    icn85xx_prog_i2c_txdata(0x40630,ucTemp,1);

    ucTemp[0]=0x00;
    ucTemp[1]=0x00;
    icn85xx_prog_i2c_txdata(0x40640,ucTemp,2);

    ucTemp[0]=1;
    icn85xx_prog_i2c_txdata(0x40644,ucTemp,1);
    while(ucTemp[0])
    {
        icn85xx_prog_i2c_rxdata(0x40644,ucTemp,1);
    }

    ucTemp[0]=FlashState(0);
    while( (ucTemp[0]&0x02)!=0x02)
    {
        ucTemp[0]=FlashState(0);
    }
}

#ifndef  QUAD_OUTPUT_ENABLE
void ClearFlashState(void)
{
    unsigned char ucTemp[4] = {0,0,0,0};
    icn85xx_prog_i2c_rxdata(0x40603,ucTemp,1);
    ucTemp[0]=(ucTemp[0]|0x20);
    icn85xx_prog_i2c_txdata(0x40603, ucTemp, 1 );

    FlashWriteEnable();

    ucTemp[2]=0x00;
    ucTemp[1]=0x10;
    ucTemp[0]=0x10;
    icn85xx_prog_i2c_txdata(0x4062c,ucTemp,3);

    ucTemp[0]=0x01;
    icn85xx_prog_i2c_txdata(0x40630,ucTemp,1);

    ucTemp[0]=0x00;
    ucTemp[1]=0x00;
    icn85xx_prog_i2c_txdata(0x40640,ucTemp,2);

    ucTemp[0]=0x00;
    icn85xx_prog_i2c_txdata(0x40638,ucTemp,1);

    ucTemp[0]=1;
    icn85xx_prog_i2c_txdata(0x40644,ucTemp,1);
    while(ucTemp[0])
    {
        icn85xx_prog_i2c_rxdata(0x40644,ucTemp,1);
    }
    while(FlashState(0)&0x01);

}
#else
void ClearFlashState(void)
{
}
#endif


void EarseFlash(unsigned char erase_index,ulong flash_addr)
{
    unsigned char ucTemp[4] = {0,0,0,0};
    FlashWriteEnable();
    if(erase_index==0)            //erase the chip
    {
        ucTemp[0]=0xc7;
        icn85xx_prog_i2c_txdata(0x40630, ucTemp, 1 );
        ucTemp[2]=0x00;
        ucTemp[1]=0x10;
        ucTemp[0]=0x00;
        icn85xx_prog_i2c_txdata(0x4062c, ucTemp, 3 );
    }
    else if(erase_index==1)       //erase 32k space of the flash
    {
        ucTemp[0]=0x52;
        icn85xx_prog_i2c_txdata(0x40630, ucTemp, 1);
        ucTemp[2]=0x00;
        ucTemp[1]=0x13;
        ucTemp[0]=0x00;
        icn85xx_prog_i2c_txdata(0x4062c, ucTemp, 3);
    }
    else if(erase_index==2)     //erase 64k space of the flash
    {
        ucTemp[0]=0xd8;
        icn85xx_prog_i2c_txdata(0x40630, ucTemp,1);
        ucTemp[2]=0x00;
        ucTemp[1]=0x13;
        ucTemp[0]=0x00;
        icn85xx_prog_i2c_txdata(0x4062c, ucTemp, 3);
    }
    else if(erase_index==3)
    {
        ucTemp[0]=0x20;
        icn85xx_prog_i2c_txdata(0x40630, ucTemp, 1);
        ucTemp[2]=0x00;
        ucTemp[1]=0x13;
        ucTemp[0]=0x00;
        icn85xx_prog_i2c_txdata(0x4062c, ucTemp, 3);
    }
    ucTemp[2]=(unsigned char)(flash_addr>>16);
    ucTemp[1]=(unsigned char)(flash_addr>>8);
    ucTemp[0]=(unsigned char)(flash_addr);
    icn85xx_prog_i2c_txdata(0x40634, ucTemp, 3);

    ucTemp[1]=0x00;
    ucTemp[0]=0x00;
    icn85xx_prog_i2c_txdata(0x40640, ucTemp, 2 );

    ucTemp[0]=1;
    icn85xx_prog_i2c_txdata(0x40644, ucTemp, 1);
    while(ucTemp[0])
    {
        icn85xx_prog_i2c_rxdata(0x40644,ucTemp,1);
    }

}

int  icn85xx_erase_flash(void)
{
    int i;
    ClearFlashState();
    while(FlashState(0)&0x01);
    FlashWriteEnable();
    EarseFlash(1,0);
    while((FlashState(0)&0x01));

    for(i=0; i<7; i++)
    {
        FlashWriteEnable();
        EarseFlash(3,0x8000+i*0x1000);
        while((FlashState(0)&0x01));
    }
    for(i=0; i<4; i++)
    {
        FlashWriteEnable();
        EarseFlash(3,0x10000+i*0x1000);
        while((FlashState(0)&0x01));
    }
    return 0;
}

int  icn85xx_prog_buffer(unsigned int flash_addr,unsigned int sram_addr,unsigned int copy_length,unsigned char program_type)
{
    unsigned char ucTemp[4] = {0,0,0,0};
    unsigned char prog_state=0;

    unsigned int i=0;
    unsigned char program_commond=0;
    if(program_type == 0)
    {
        program_commond = 0x02;
    }
    else if(program_type == 1)
    {
        program_commond = 0xf2;
    }
    else
    {
        program_commond = 0x02;
    }


    for(i=0; i<copy_length; )
    {
        prog_state=(FlashState(0)&0x01);
        while(prog_state)
        {
            prog_state=(FlashState(0)&0x01);
        }
        FlashWriteEnable();

        ucTemp[2]=0;
        ucTemp[1]=0x13;
        ucTemp[0]=0;
        icn85xx_prog_i2c_txdata(0x4062c, ucTemp, 3);

        ucTemp[2]=(unsigned char)(flash_addr>>16);
        ucTemp[1]=(unsigned char)(flash_addr>>8);
        ucTemp[0]=(unsigned char)(flash_addr);
        icn85xx_prog_i2c_txdata(0x40634, ucTemp, 3);

        ucTemp[2]=(unsigned char)(sram_addr>>16);
        ucTemp[1]=(unsigned char)(sram_addr>>8);
        ucTemp[0]=(unsigned char)(sram_addr);
        icn85xx_prog_i2c_txdata(0x4063c, ucTemp, 3);

        if(i+256<=copy_length)
        {
            ucTemp[1]=0x01;
            ucTemp[0]=0x00;
        }
        else
        {
            ucTemp[1]=(unsigned char)((copy_length-i)>>8);
            ucTemp[0]=(unsigned char)(copy_length-i);
        }
        icn85xx_prog_i2c_txdata(0x40640, ucTemp,2);

        ucTemp[0]=program_commond;
        icn85xx_prog_i2c_txdata(0x40630, ucTemp,1);

        ucTemp[0]=0x01;
        icn85xx_prog_i2c_txdata(0x40644, ucTemp,1);

        flash_addr+=256;
        sram_addr+=256;
        i+=256;
        while(ucTemp[0])
        {
            icn85xx_prog_i2c_rxdata(0x40644,ucTemp,1);
        }

    }

    prog_state=(FlashState(0)&0x01);
    while(prog_state)
    {
        prog_state=(FlashState(0)&0x01);
    }
    return 0;
}


int  icn85xx_prog_data(unsigned int flash_addr, unsigned int data)
{
    unsigned char ucTemp[4] = {0,0,0,0};

    ucTemp[3]=(unsigned char)(data>>24);
    ucTemp[2]=(unsigned char)(data>>16);
    ucTemp[1]=(unsigned char)(data>>8);
    ucTemp[0]=(unsigned char)(data);

    icn85xx_prog_i2c_txdata(0x2a000, ucTemp,4);
    icn85xx_prog_buffer(flash_addr , 0x2a000, 0x04,  0);
    return 0;
}

void  icn85xx_read_flash(unsigned int sram_address,unsigned int flash_address,unsigned long copy_length,unsigned char i2c_wire_num)
{
    unsigned char ucTemp[4] = {0,0,0,0};

    if(i2c_wire_num==1)
    {
        ucTemp[2]=0x18;
        ucTemp[1]=0x13;
        ucTemp[0]=0x00;
    }
    else if(i2c_wire_num==2)
    {
        ucTemp[2]=0x1a;
        ucTemp[1]=0x13;
        ucTemp[0]=0x01;
    }
    else if(i2c_wire_num==4)
    {
        ucTemp[2]=0x19;
        ucTemp[1]=0x13;
        ucTemp[0]=0x01;
    }
    else
    {
        ucTemp[2]=0x18;
        ucTemp[1]=0x13;
        ucTemp[0]=0x01;
    }
    icn85xx_prog_i2c_txdata(0x4062c, ucTemp,3);

    if(i2c_wire_num==1)
    {
        ucTemp[0]=0x03;
    }
    else if(i2c_wire_num==2)
    {
        ucTemp[0]=0x3b;
    }
    else if(i2c_wire_num==4)
    {
        ucTemp[0]=0x6b;
    }
    else
    {
        ucTemp[0]=0x0b;
    }
    icn85xx_prog_i2c_txdata(0x40630, ucTemp,1);

    ucTemp[2]=(unsigned char)(flash_address>>16);
    ucTemp[1]=(unsigned char)(flash_address>>8);
    ucTemp[0]=(unsigned char)(flash_address);
    icn85xx_prog_i2c_txdata(0x40634, ucTemp,3);

    ucTemp[2]=(unsigned char)(sram_address>>16);
    ucTemp[1]=(unsigned char)(sram_address>>8);
    ucTemp[0]=(unsigned char)(sram_address);
    icn85xx_prog_i2c_txdata(0x4063c, ucTemp,3);

    ucTemp[1]=(unsigned char)(copy_length>>8);
    ucTemp[0]=(unsigned char)(copy_length);
    icn85xx_prog_i2c_txdata(0x40640, ucTemp,2);

    ucTemp[0]=0x01;

    icn85xx_prog_i2c_txdata(0x40644, ucTemp,1);
    while(ucTemp[0])
    {
        icn85xx_prog_i2c_rxdata(0x40644,ucTemp,1);
    }

}

short  icn85xx_read_fw_Ver(char *fw)
{
    short FWversion;
    char tmp[2];
    int file_size;
    file_size = icn85xx_open_fw(fw);
    if(file_size < 0)
    {
        return -1;
    }
    icn85xx_read_fw(0x100, 2, &tmp[0]);

    icn85xx_close_fw();
    FWversion = (tmp[1]<<8)|tmp[0];

    return FWversion;


}

int icn87xx_read_fw_info(char *fw, unsigned char *buffer, unsigned short u16Addr, unsigned char u8Length)
{
    int file_size;
    file_size = icn85xx_open_fw(fw);
    if(file_size < 0)
    {
        return -1;
    }
    icn85xx_read_fw(u16Addr, u8Length, buffer);

    icn85xx_close_fw();
    return 1;

}

#define ENABLE_BYTE_CHECK    0

int  icn85xx_fw_download(unsigned int offset, unsigned char * buffer, unsigned int size)
{
int i;
#ifdef ENABLE_BYTE_CHECK
    char testb[B_SIZE];
#endif

    icn85xx_prog_i2c_txdata(offset,buffer,size);
#ifdef ENABLE_BYTE_CHECK
    icn85xx_prog_i2c_rxdata(offset,testb,size);
    for(i = 0; i < size; i++)
    {
        if(buffer[i] != testb[i])
        {
            pr_err("buffer[%d]:%x  testb[%d]:%x\n",i,buffer[i],i,testb[i]);
            return -1;
        }
    }
#endif
    return 0;
}

int  icn87xx_fw_download(unsigned int offset, unsigned char * buffer, unsigned int size)
{
int i;
#ifdef ENABLE_BYTE_CHECK
    unsigned char testb[B_SIZE];
#endif

    icn87xx_prog_i2c_txdata(offset,buffer,size);
#ifdef ENABLE_BYTE_CHECK
    icn87xx_prog_i2c_rxdata(offset,testb,size);
    for(i = 0; i < size; i++)
    {
    //pr_err("buffer[%d]:%x  testb[%d]:%x\n",i,buffer[i],i,testb[i]);
        if(buffer[i] != testb[i])
        {
            pr_err("buffer[%d]:%x  testb[%d]:%x\n",i,buffer[i],i,testb[i]);
            return -1;
        }
    }
#endif
    return 0;
}

int  icn85xx_bootfrom_flash(int ictype)
{
    int ret = -1;
    unsigned char ucTemp;

    if(ictype == ICN85XX_WITH_FLASH_85)
    {
        ucTemp=0x7f;
        ret = icn85xx_prog_i2c_txdata(0x40004, &ucTemp, 1 );        //ICN85XX chip reset
        if (ret < 0) {
            pr_err("1 %s failed: %d\n", __func__, ret);
            return ret;
        }
    }
    else if(ictype == ICN85XX_WITH_FLASH_86)
    {
        ucTemp=0x7f;
        ret = icn85xx_prog_i2c_txdata(0x4046c, &ucTemp, 1 );        //ICN85EX chip reset
        if (ret < 0) {
            pr_err("2 %s failed: %d\n", __func__, ret);
            return ret;
        }
    }
    return ret;
}

int  icn85xx_bootfrom_sram(void)
{
    int ret = -1;
    unsigned char ucTemp = 0x03;
    unsigned long addr = 0x40400;
    pr_info("icn85xx_bootfrom_sram\n");
    ret = icn85xx_prog_i2c_txdata(addr, &ucTemp, 1 );           //change bootmode from sram
    return ret;
}


/*
    This polynomial (0x04c11db7) is used at: AUTODIN II, Ethernet, & FDDI
*/
static unsigned int crc32table[256] = {
 0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
 0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
 0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
 0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
 0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
 0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
 0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
 0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
 0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
 0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
 0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
 0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
 0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
 0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
 0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
 0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
 0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
 0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
 0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
 0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
 0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
 0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

unsigned int icn85xx_crc_calc(unsigned crc_in, char *buf, int len)
{
    int i;
    unsigned int crc = crc_in;
    for(i = 0; i < len; i++)
        crc = (crc << 8) ^ crc32table[((crc >> 24) ^ *buf++) & 0xFF];
    return crc;
}


int  icn85xx_crc_enable(unsigned char enable)
{
    unsigned char ucTemp;
    int ret = 0;
    if(enable==1)
    {
        ucTemp = 1;
        ret = icn85xx_prog_i2c_txdata(0x40028, &ucTemp, 1 );
    }
    else if(enable==0)
    {
        ucTemp = 0;
        ret = icn85xx_prog_i2c_txdata(0x40028, &ucTemp, 1 );
    }
    return ret;
}

int  icn85xx_crc_check(unsigned int crc, unsigned int len)
{
    int ret;
    unsigned int crc_len;
    unsigned int crc_result;
    unsigned char ucTemp[4] = {0,0,0,0};

    ret= icn85xx_prog_i2c_rxdata(0x4002c, ucTemp, 4 );
    crc_result = ucTemp[3]<<24 | ucTemp[2]<<16 | ucTemp[1] << 8 | ucTemp[0];

    ret = icn85xx_prog_i2c_rxdata(0x40034, ucTemp, 2);
    crc_len = ucTemp[1] << 8 | ucTemp[0];

    if((crc_result == crc) && (crc_len == len))
        return 0;
    else
    {
        pr_info("crc_fw: 0x%x\n", crc);
        pr_info("crc_result: 0x%x\n", crc_result);
        pr_info("crc_len: %d\n", crc_len);
        return -1;
    }

}


int  icn85xx_fw_update(void *fw)
{
    int file_size, last_length;
    int j, num;
    char temp_buf[B_SIZE];
    unsigned int crc_fw;

    file_size = icn85xx_open_fw(fw);
    if(file_size < 0)
    {
        icn85xx_update_status(R_FILE_ERR);
        return R_FILE_ERR;
    }
    if(icn85xx_goto_progmode() != 0)
    {
        icn85xx_update_status(R_PROGRAM_ERR);
        pr_err("icn85xx_goto_progmode() != 0 error\n");
        return R_STATE_ERR;
    }
    msleep(1);
    icn85xx_crc_enable(1);

    num = file_size/B_SIZE;
    crc_fw = 0;
    for(j=0; j < num; j++)
    {
        icn85xx_read_fw(j*B_SIZE, B_SIZE, temp_buf);
        crc_fw = icn85xx_crc_calc(crc_fw, temp_buf, B_SIZE);
        if(icn85xx_fw_download(j*B_SIZE, temp_buf, B_SIZE) != 0)
        {
            pr_err("error j:%d\n",j);
            icn85xx_update_status(R_PROGRAM_ERR);
            icn85xx_close_fw();
            return R_PROGRAM_ERR;
        }
        icn85xx_update_status(5+(int)(60*j/num));
    }
    last_length = file_size - B_SIZE*j;
    if(last_length > 0)
    {
        icn85xx_read_fw(j*B_SIZE, last_length, temp_buf);
        crc_fw = icn85xx_crc_calc(crc_fw, temp_buf, last_length);
        if(icn85xx_fw_download(j*B_SIZE, temp_buf, last_length) != 0)
        {
            pr_err("error last length\n");
            icn85xx_update_status(R_PROGRAM_ERR);
            icn85xx_close_fw();
            return R_PROGRAM_ERR;
        }
    }
    icn85xx_close_fw();
    icn85xx_update_status(65);
    icn85xx_crc_enable(0);
    if(icn85xx_crc_check(crc_fw, file_size) != 0)
    {
        icn85xx_update_status(R_VERIFY_ERR);
        pr_info("down fw error, crc error\n");
        return R_PROGRAM_ERR;
    }
    else
    {
        //pr_info("downoad fw ok, crc ok\n");
    }
    icn85xx_update_status(70);

    if((ICN85XX_WITH_FLASH_85 == boot_mode) || (ICN85XX_WITH_FLASH_86 == boot_mode))
    {
        icn85xx_erase_flash();

        icn85xx_update_status(75);

        FlashWriteEnable();

        icn85xx_prog_buffer( 0, 0, file_size,0);

        icn85xx_update_status(85);

        while((FlashState(0)&0x01));
        FlashWriteEnable();

        icn85xx_prog_data(FLASH_CRC_ADDR, crc_fw);
        icn85xx_prog_data(FLASH_CRC_ADDR+4, file_size);

        icn85xx_update_status(90);


        icn85xx_crc_enable(1);
        icn85xx_read_flash( 0,  0, file_size,  2);
        icn85xx_crc_enable(0);
        if(icn85xx_crc_check(crc_fw, file_size) != 0)
        {
            pr_info("read flash data error, crc error\n");
            return R_PROGRAM_ERR;
        }
        else
        {
            pr_info("read flash data ok, crc ok\n");
        }
        while((FlashState(0)&0x01));
        icn85xx_update_status(95);

        if(icn85xx_bootfrom_sram() == 0)	//code already in ram
        {
            pr_err("icn85xx_bootfrom_flash error\n");
            icn85xx_update_status(R_STATE_ERR);
            return R_STATE_ERR;
        }
    }
    else if((ICN85XX_WITHOUT_FLASH_85 == boot_mode) || (ICN85XX_WITHOUT_FLASH_86 == boot_mode))
    {
        if(icn85xx_bootfrom_sram() == 0)
        {
            pr_err("icn85xx_bootfrom_sram error\n");
            icn85xx_update_status(R_STATE_ERR);
            return R_STATE_ERR;
        }
    }
    msleep(50);
    icn85xx_update_status(R_OK);
    pr_info("icn85xx upgrade ok\n");
    return R_OK;
}

int  icn87xx_fw_update(void *fw)
{
    int file_size, last_length;

    int ret = -1;
    int j, num;
    unsigned char temp_buf[B_SIZE];
    unsigned char buf[20];
    unsigned char version = 0;
    unsigned short crc_fw = 0;
    unsigned short sram_len = 0;
    unsigned short sram_crc = 0;
    unsigned short temp_crc = 0;
    unsigned short fw_version = 0;

    ret = icn87xx_read_fw_info(fw, buf, FIRMWARA_INFO_AT_BIN_ADDR,16); //contain sram lenth ,fwversion
    sram_len = (((buf[9])<<8) + buf[8]) + (((buf[6])<<16) + ((buf[5])<<8) + buf[4]) - (((buf[2])<<16) + ((buf[1])<<8) + buf[0]);
    fw_version = (((buf[13])<<8)+ buf[12]);


    file_size = icn85xx_open_fw(fw);

    pr_info("file_size:%d  \n",file_size);
    pr_info("boot mode:0x%x\n",boot_mode);
    if(file_size <= 0)
    {
        icn85xx_update_status(R_FILE_ERR);
        return R_FILE_ERR;
    }
    if(icn85xx_goto_progmode() < 0)
    {
        icn85xx_update_status(R_PROGRAM_ERR);
        pr_err("icn85xx_goto_progmode()  error\n");
        return R_STATE_ERR;
    }


    pr_info("sram_len:%d  \n",sram_len);
    msleep(1);

    if(ICN85XX_WITH_FLASH_87 == boot_mode)
    {
         ret = icn87xx_erase_chip();
         if(ret < 0)
         {
              pr_err("earse error\n");
              return ret;
         }
    }

    if(sram_len == file_size)
    {
        num = file_size/B_SIZE;
        crc_fw = 0;
        for(j=0; j < num; j++)
        {
            icn85xx_read_fw(j*B_SIZE, B_SIZE, temp_buf);
            crc_fw = icn87xx_fw_Crc(crc_fw, temp_buf, B_SIZE);
            if(icn87xx_fw_download(j*B_SIZE, temp_buf, B_SIZE) != 0)
            {
                pr_err("error j:%d\n",j);
                icn85xx_update_status(R_PROGRAM_ERR);
                icn85xx_close_fw();
                return R_PROGRAM_ERR;
            }
            icn85xx_update_status(5+(int)(60*j/num));
        }
        last_length = file_size - B_SIZE*num;
        if(last_length > 0)
        {
            icn85xx_read_fw(j*B_SIZE, last_length, temp_buf);
            crc_fw = icn87xx_fw_Crc(crc_fw, temp_buf, last_length);
            if(icn87xx_fw_download(j*B_SIZE, temp_buf, last_length) != 0)
            {
                pr_err("error last length\n");
                icn85xx_update_status(R_PROGRAM_ERR);
                icn85xx_close_fw();
                return R_PROGRAM_ERR;
            }
        }

        if( icn87xx_calculate_crc(sram_len) != crc_fw)
        {
            pr_err("crc error\n");
            return -1;
        }
        else
        {
            sram_crc =  crc_fw;
        }
        if(ICN85XX_WITH_FLASH_87 == boot_mode)
        {
            ret = icn87xx_flash_write(0, 0, file_size);
            if(ret < 0)
            {
                return ret;
            }
        }


    }
    else if(file_size > sram_len)
    {
        if(ICN85XX_WITHOUT_FLASH_87 == boot_mode)
        {
            pr_err("error!! file_size > sram_len\n");
            icn85xx_update_status(R_PROGRAM_ERR);
            icn85xx_close_fw();
            return R_PROGRAM_ERR;
        }
        num = sram_len/B_SIZE;
        crc_fw = 0;
        for(j = 0; j < num; j++)
        {
            icn85xx_read_fw(j*B_SIZE, B_SIZE, temp_buf);
            crc_fw = icn87xx_fw_Crc(crc_fw, temp_buf, B_SIZE);
            if(icn87xx_fw_download(j*B_SIZE, temp_buf, B_SIZE) != 0)
            {
                pr_err("error j:%d\n",j);
                icn85xx_update_status(R_PROGRAM_ERR);
                icn85xx_close_fw();
                return R_PROGRAM_ERR;
            }
            icn85xx_update_status(5+(int)(60*j/num));
        }

        last_length = sram_len - B_SIZE*num;

        if(last_length > 0)
        {
            icn85xx_read_fw(j*B_SIZE, last_length, temp_buf);
            crc_fw = icn87xx_fw_Crc(crc_fw, temp_buf, last_length);
            if(icn87xx_fw_download(j*B_SIZE, temp_buf, last_length) != 0)
            {
                pr_err("error last length\n");
                icn85xx_update_status(R_PROGRAM_ERR);
                icn85xx_close_fw();
                return R_PROGRAM_ERR;
            }
        }


        if(icn87xx_calculate_crc(sram_len) != crc_fw)
        {
            pr_err("crc error0\n");
            return -1;
        }
        else
        {
            sram_crc =  crc_fw;
        }

        ret = icn87xx_flash_write(0, 0, sram_len);
        if(ret < 0)
        {
            return ret;
        }

        num = (file_size - sram_len)/B_SIZE;

        temp_crc = 0;

        for(j=0; j < num; j++)
        {
            icn85xx_read_fw(sram_len + j*B_SIZE, B_SIZE, temp_buf);
            crc_fw = icn87xx_fw_Crc(crc_fw, temp_buf, B_SIZE);
            temp_crc = icn87xx_fw_Crc(temp_crc, temp_buf, B_SIZE);
            if(icn87xx_fw_download(j*B_SIZE, temp_buf, B_SIZE) != 0)
            {
                pr_err("error j:%d\n",j);
                icn85xx_update_status(R_PROGRAM_ERR);
                icn85xx_close_fw();
                return R_PROGRAM_ERR;
            }
            icn85xx_update_status(5+(int)(60*j/num));
        }
        last_length = (file_size - sram_len) - B_SIZE*num;
        if(last_length > 0)
        {

            icn85xx_read_fw( sram_len + j*B_SIZE, last_length, temp_buf);
            crc_fw = icn87xx_fw_Crc(crc_fw, temp_buf, last_length);
            temp_crc = icn87xx_fw_Crc(temp_crc, temp_buf, last_length);
            if(icn87xx_fw_download(j*B_SIZE, temp_buf, last_length) != 0)
            {
                pr_err("error last length\n");
                icn85xx_update_status(R_PROGRAM_ERR);
                icn85xx_close_fw();
                return R_PROGRAM_ERR;
            }

        }
        if(icn87xx_calculate_crc(file_size - sram_len) != temp_crc)
        {
            pr_err("crc error1\n");
            return -1;
        }
        ret = icn87xx_flash_write(sram_len, 0, file_size - sram_len);
        if(ret < 0)
        {
            return ret;
        }
    }


    if(ICN85XX_WITH_FLASH_87 == boot_mode)
    {
        pr_info("write flashinfo \n");
        icn85xx_update_status(85);

        buf[0] = (unsigned char)(crc_fw);
        buf[1] = (unsigned char)(crc_fw>>8);
        buf[2] = 0;
        buf[3] = 0;
        buf[4] = (unsigned char)(file_size);
        buf[5] = (unsigned char)(file_size>>8);
        buf[6] = 0;
        buf[7] = 0;
        buf[8] = 0x5a;
        buf[9] = 0xc4;
        buf[10] = 0;
        buf[11] = 0;
        buf[12] = (unsigned char)(sram_crc);
        buf[13] = (unsigned char)(sram_crc>>8);
        buf[14] = 0;
        buf[15] = 0;
        buf[16] = (unsigned char)(sram_len);
        buf[17] = (unsigned char)(sram_len>>8);
        buf[18] = 0;
        buf[19] = 0;

        for(j=0;j<5;j++){
            ret = icn87xx_write_pr_info(FLASH_STOR_INFO_ADDR+j*4,SRAM_EXCHANGE_ADDR1+j*4,buf+j*4,4);
        }
        icn85xx_update_status(95);

        msleep(5);

        while(version != 0x81)
        {
            if(icn87xx_boot_flash() < 0)    //code already in ram
            {
                pr_err("icn85xx_bootfrom_flash error\n");
                icn85xx_update_status(R_STATE_ERR);
                return R_STATE_ERR;
            }
            msleep(10);
            ret = icn85xx_read_reg(0xa, &version);

            printk("after upgrdate Value: %x\n",version);
            msleep(5);
        }
    }
    else if(ICN85XX_WITHOUT_FLASH_87 == boot_mode)
    {
        if(icn87xx_boot_sram() < 0)
        {
            pr_err("icn85xx_bootfrom_sram error\n");
            icn85xx_update_status(R_STATE_ERR);
            return R_STATE_ERR;
        }
    }
    icn85xx_update_status(R_OK);
    pr_info("icn87xx upgrade ok\n");
	//check crc in sram before boot
	icnt87_sram_crc = sram_crc;
	icnt87_sram_length = sram_len;

    return R_OK;
}


#if COMPILE_FW_WITH_DRIVER
       static char firmware[128] = "icn85xx_firmware";
#else
    #if SUPPORT_SENSOR_ID
        static char firmware[128] = {0};
    #else
       static char firmware[128] = {"/misc/modules/ICN8505.BIN"};
    #endif
#endif

#if SUPPORT_SENSOR_ID
   char cursensor_id,tarsensor_id,id_match;
   char invalid_id = 0;

   struct sensor_id {
                char value;
                const char bin_name[128];
                unsigned char *fw_name;
                int size;
        };

static struct sensor_id sensor_id_table[] = {
                   { 0x22, "/misc/modules/ICN8505_22_name9.BIN",fw_22_name9,sizeof(fw_22_name9)},//default bin or fw
                   { 0x00, "/misc/modules/ICN8505_00_name1.BIN",fw_00_name1,sizeof(fw_00_name1)},
                   { 0x20, "/misc/modules/ICN8505_20_name7.BIN",fw_20_name7,sizeof(fw_20_name7)},
                   { 0x10, "/misc/modules/ICN8505_10_name4.BIN",fw_10_name4,sizeof(fw_10_name4)},
                   { 0x11, "/misc/modules/ICN8505_11_name5.BIN",fw_11_name5,sizeof(fw_11_name5)},
                   { 0x12, "/misc/modules/ICN8505_12_name6.BIN",fw_12_name6,sizeof(fw_12_name6)},
                   { 0x01, "/misc/modules/ICN8505_01_name2.BIN",fw_01_name2,sizeof(fw_01_name2)},
                   { 0x21, "/misc/modules/ICN8505_21_name8.BIN",fw_21_name8,sizeof(fw_21_name8)},
                   { 0x02, "/misc/modules/ICN8505_02_name3.BIN",fw_02_name3,sizeof(fw_02_name3)},
                 // if you want support other sensor id value ,please add here
                 };
#endif


static struct i2c_client *this_client;
short log_basedata[COL_NUM][ROW_NUM] = {{0,0}};
short log_rawdata[COL_NUM][ROW_NUM] = {{0,0}};
short log_diffdata[COL_NUM][ROW_NUM] = {{0,0}};
unsigned int log_on_off = 0;
static void icn85xx_log(char diff);

static enum hrtimer_restart chipone_timer_func(struct hrtimer *timer);

#if SUPPORT_PROC_FS
pack_head cmd_head;
static struct proc_dir_entry *icn85xx_proc_entry;
int  DATA_LENGTH = 0;
GESTURE_DATA structGestureData;
STRUCT_PANEL_PARA_H g_structPanelPara;
#endif


#if SUPPORT_SYSFS


static ssize_t icn85xx_store_update(struct device_driver *drv,const char *buf,size_t count);
static ssize_t icn85xx_show_update(struct device_driver *drv,char* buf);

static ssize_t icn85xx_show_process(struct device_driver *drv, char* buf);
static ssize_t icn85xx_store_process(struct device_driver *drv,const char *buf, size_t len);


static DRIVER_ATTR(icn_update, 0644, icn85xx_show_update, icn85xx_store_update);
static DRIVER_ATTR(icn_process, 0644, icn85xx_show_process, icn85xx_store_process);

static ssize_t icn85xx_show_process(struct device_driver *drv, char* buf)
{
    ssize_t ret = 0;
    sprintf(buf, "icn85xx process\n");
    ret = strlen(buf) + 1;
    return ret;
}

static ssize_t icn85xx_store_process(struct device_driver *drv,const char *buf, size_t len)
{
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);
    unsigned long on_off = simple_strtoul(buf, NULL, 10);

    log_on_off = on_off;
    memset(&log_basedata[0][0], 0, COL_NUM*ROW_NUM*2);
    if(on_off == 0)
    {
        icn85xx_ts->work_mode = 0;
    }
    else if((on_off == 1) || (on_off == 2) || (on_off == 3))
    {
        if((icn85xx_ts->work_mode == 0) && (icn85xx_ts->use_irq == 1))
        {
            hrtimer_init(&icn85xx_ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
            icn85xx_ts->timer.function = chipone_timer_func;
            hrtimer_start(&icn85xx_ts->timer, ktime_set(CTP_START_TIMER/1000, (CTP_START_TIMER%1000)*1000000), HRTIMER_MODE_REL);
        }
        icn85xx_ts->work_mode = on_off;
    }
    else if(on_off == 10)
    {
        icn85xx_ts->work_mode = 4;
        mdelay(10);
        pr_info("update baseline\n");
        icn85xx_write_reg(4, 0x30);
        icn85xx_ts->work_mode = 0;
    }
    else
    {
        icn85xx_ts->work_mode = 0;
    }


    return len;
}


static ssize_t icn85xx_show_update(struct device_driver *drv, char* buf)
{
    ssize_t ret = 0;
    sprintf(buf, firmware);
    ret = strlen(buf) + 1;
    //pr_info("firmware: %s, ret: %d\n", firmware, ret);

    return ret;
}

static ssize_t icn85xx_store_update(struct device_driver *drv,const char *buf, size_t len)
{
    //pr_info("count: %d, update: %s\n", count, buf);
    char val;
	sscanf(buf,"%s",&val);
	pr_info("val: %c\n", val);
    switch(val){
	case 'u':
	case 'U':
		    pr_info("firmware: %s\n", firmware);
            pr_info("fwVersion : 0x%x\n", icn85xx_read_fw_Ver("/mnt/sdcard/ICN8505.bin"));
            pr_info("current version: 0x%x\n", icn85xx_readVersion());
		    if(R_OK == icn85xx_fw_update("/mnt/sdcard/ICN8505.bin"))
		    {
		        pr_info("update ok\n");
		    }
		    else
		    {
		        pr_info("update error\n");
		    }
			break;
	case 't':
	case 'T':
             icn85xx_ts_reset();
						 //DEBUG = 1;
	                       break;
	case 'r':
	case 'R':
		     icn85xx_log(0);

                             break;
	case 'd':
	case 'D':
		     icn85xx_log(1);
			 msleep(100);
		     break;
	default:
		pr_info("this conmand is unknow!!\n");
	break;
	}

	return len;
}


static struct attribute *icn_drv_attr[] = {
	&driver_attr_icn_update.attr,
	&driver_attr_icn_process.attr,
	NULL
};
static struct attribute_group icn_drv_attr_grp = {
	.attrs =icn_drv_attr,
};
static const struct attribute_group *icn_drv_grp[] = {
	&icn_drv_attr_grp,
	NULL
};
#endif

#if SUPPORT_PROC_FS
static struct task_struct *resume_download_task = NULL;

static ssize_t icn85xx_tool_write(struct file *file, const char __user * buffer, size_t count, loff_t * ppos)
{
     int ret = 0;
    int i;
    unsigned short addr;
    unsigned int prog_addr;
    char retvalue;
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);
    pr_info("%s \n",__func__);
    if(down_interruptible(&icn85xx_ts->sem))
    {
        return -1;
    }
    ret = copy_from_user(&cmd_head, buffer, CMD_HEAD_LENGTH);
    if(ret)
    {
        pr_info("copy_from_user failed.\n");
        goto write_out;
    }
    else
    {
        ret = CMD_HEAD_LENGTH;
    }

    pr_info("wr  :0x%02x.\n", cmd_head.wr);
    pr_info("flag:0x%02x.\n", cmd_head.flag);
    pr_info("circle  :%d.\n", (int)cmd_head.circle);
    pr_info("times   :%d.\n", (int)cmd_head.times);
    pr_info("retry   :%d.\n", (int)cmd_head.retry);
    pr_info("data len:%d.\n", (int)cmd_head.data_len);
    pr_info("addr len:%d.\n", (int)cmd_head.addr_len);
    pr_info("addr:0x%02x%02x.\n", cmd_head.addr[0], cmd_head.addr[1]);
    pr_info("len:%d.\n", (int)count);
    pr_info("data:0x%02x%02x.\n", buffer[CMD_HEAD_LENGTH], buffer[CMD_HEAD_LENGTH+1]);
    if (1 == cmd_head.wr)  // write para
    {
        pr_info("cmd_head_.wr == 1  \n");
        ret = copy_from_user(&cmd_head.data[0], &buffer[CMD_HEAD_LENGTH], cmd_head.data_len);
        if(ret)
        {
            pr_info("copy_from_user failed.\n");
            goto write_out;
        }

        memcpy(&g_structPanelPara, &cmd_head.data[0], cmd_head.data_len);
        //write para to tp
        for(i=0; i<cmd_head.data_len; )
        {
            int size = ((i+64) > cmd_head.data_len)?(cmd_head.data_len-i):64;
            ret = icn85xx_i2c_txdata(0x8000+i, &cmd_head.data[i], size);
            if (ret < 0) {
                pr_info("write para failed!\n");
                goto write_out;
            }
            i = i + 64;
        }
        ret = cmd_head.data_len + CMD_HEAD_LENGTH;
        icn85xx_ts->work_mode = 4; //reinit
        pr_info("reinit tp\n");
        icn85xx_write_reg(0, 1);
        mdelay(100);
        icn85xx_write_reg(0, 0);
        mdelay(100);
        icn85xx_ts->work_mode = 0;
        goto write_out;

    }
    else if(3 == cmd_head.wr)   //set update file
    {
        pr_info("cmd_head_.wr == 3  \n");
        ret = copy_from_user(&cmd_head.data[0], &buffer[CMD_HEAD_LENGTH], cmd_head.data_len);
        if(ret)
        {
            pr_info("copy_from_user failed.\n");
            goto write_out;
        }
        ret = cmd_head.data_len + CMD_HEAD_LENGTH;
        memset(firmware, 0, 128);
        memcpy(firmware, &cmd_head.data[0], cmd_head.data_len);
        pr_info("firmware : %s\n", firmware);
    }
    else if(5 == cmd_head.wr)  //start update
    {
        pr_info("cmd_head_.wr == 5 \n");
        icn85xx_update_status(1);

        if((icn85xx_ts->ictype == ICN85XX_WITH_FLASH_87) || (icn85xx_ts->ictype == ICN85XX_WITHOUT_FLASH_87))
        {
            //ret = kernel_thread(icn87xx_fw_update,firmware,CLONE_KERNEL);
            kthread_run(icn87xx_fw_update, firmware, "icn_update");
       }
        else
        {
            //ret = kernel_thread(icn85xx_fw_update,firmware,CLONE_KERNEL);
            kthread_run(icn85xx_fw_update, firmware, "icn_update");
        }

    }
    else if(11 == cmd_head.wr) //write hostcomm
    {
        icn85xx_ts->work_mode = cmd_head.flag; //for gesture test,you should set flag=6
        structGestureData.u8Status = 0;

        ret = copy_from_user(&cmd_head.data[0], &buffer[CMD_HEAD_LENGTH], cmd_head.data_len);
        if(ret)
        {
            pr_info("copy_from_user failed.\n");
            goto write_out;
        }
        addr = (cmd_head.addr[1]<<8) | cmd_head.addr[0];
        icn85xx_write_reg(addr, cmd_head.data[0]);

    }
    else if(13 == cmd_head.wr) //adc enable
    {
        pr_info("cmd_head_.wr == 13  \n");
        icn85xx_ts->work_mode = 4;
        mdelay(10);
        //set col
        icn85xx_write_reg(0x8000+STRUCT_OFFSET(STRUCT_PANEL_PARA_H, u8ColNum), 1);
        //u8RXOrder[0] = u8RXOrder[cmd_head.addr[0]];
        icn85xx_write_reg(0x8000+STRUCT_OFFSET(STRUCT_PANEL_PARA_H, u8RXOrder[0]), g_structPanelPara.u8RXOrder[cmd_head.addr[0]]);
        //set row
        icn85xx_write_reg(0x8000+STRUCT_OFFSET(STRUCT_PANEL_PARA_H, u8RowNum), 1);
        //u8TXOrder[0] = u8TXOrder[cmd_head.addr[1]];
        icn85xx_write_reg(0x8000+STRUCT_OFFSET(STRUCT_PANEL_PARA_H, u8TXOrder[0]), g_structPanelPara.u8TXOrder[cmd_head.addr[1]]);
        //scan mode
        icn85xx_write_reg(0x8000+STRUCT_OFFSET(STRUCT_PANEL_PARA_H, u8ScanMode), 0);
        //bit
        icn85xx_write_reg(0x8000+STRUCT_OFFSET(STRUCT_PANEL_PARA_H, u16BitFreq), 0xD0);
        icn85xx_write_reg(0x8000+STRUCT_OFFSET(STRUCT_PANEL_PARA_H, u16BitFreq)+1, 0x07);
        //freq
        icn85xx_write_reg(0x8000+STRUCT_OFFSET(STRUCT_PANEL_PARA_H, u16FreqCycleNum[0]), 0x64);
        icn85xx_write_reg(0x8000+STRUCT_OFFSET(STRUCT_PANEL_PARA_H, u16FreqCycleNum[0])+1, 0x00);
        //pga
        icn85xx_write_reg(0x8000+STRUCT_OFFSET(STRUCT_PANEL_PARA_H, u8PgaGain), 0x0);

        //config mode
        icn85xx_write_reg(0, 0x2);

        mdelay(1);
        icn85xx_read_reg(2, &retvalue);
        pr_info("retvalue0: %d\n", retvalue);
        while(retvalue != 1)
        {
            pr_info("retvalue: %d\n", retvalue);
            mdelay(1);
            icn85xx_read_reg(2, &retvalue);
        }

        if(icn85xx_goto_progmode() != 0)
        {
            pr_info("icn85xx_goto_progmode() != 0 error\n");
            goto write_out;
        }

        icn85xx_prog_write_reg(0x040870, 1);

    }
    else if(15 == cmd_head.wr) // write hostcomm multibyte
    {
        pr_info("cmd_head_.wr == 15  \n");
        ret = copy_from_user(&cmd_head.data[0], &buffer[CMD_HEAD_LENGTH], cmd_head.data_len);
        if(ret)
        {
            pr_info("copy_from_user failed.\n");
            goto write_out;
        }
        addr = (cmd_head.addr[1]<<8) | cmd_head.addr[0];
        pr_info("wr, addr: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", addr, cmd_head.data[0], cmd_head.data[1], cmd_head.data[2], cmd_head.data[3]);
        ret = icn85xx_i2c_txdata(addr, &cmd_head.data[0], cmd_head.data_len);
        if (ret < 0) {
            pr_info("write hostcomm multibyte failed!\n");
            goto write_out;
        }
    }
    else if(17 == cmd_head.wr)// write iic porgmode multibyte
    {
        pr_info("cmd_head_.wr == 17  \n");
        ret = copy_from_user(&cmd_head.data[0], &buffer[CMD_HEAD_LENGTH], cmd_head.data_len);
        if(ret)
        {
            pr_info("copy_from_user failed.\n");
            goto write_out;
        }
        prog_addr = (cmd_head.flag<<16) | (cmd_head.addr[1]<<8) | cmd_head.addr[0];
        icn85xx_goto_progmode();
        ret = icn85xx_prog_i2c_txdata(prog_addr, &cmd_head.data[0], cmd_head.data_len);
        if (ret < 0) {
            pr_info("write hostcomm multibyte failed!\n");
            goto write_out;
        }

    }

write_out:
    up(&icn85xx_ts->sem);
    pr_info("icn85xx_tool_write write_out  \n");
    return count;

}

static ssize_t icn85xx_tool_read(struct file *file, char __user * buffer, size_t count, loff_t * ppos)
{
    int i, j;
    int ret = 0;
    char row, column, retvalue, max_column;
    unsigned short addr;
    unsigned int prog_addr;
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);
    if(down_interruptible(&icn85xx_ts->sem))
    {
        return -1;
    }
    pr_info("%s: count:%d, off:%d, cmd_head.data_len: %d\n",__func__, count,(int)(* ppos),(int)cmd_head.data_len);
    if (cmd_head.wr % 2)
    {
        ret = 0;
        pr_info("cmd_head_.wr == 1111111  \n");
        goto read_out;
    }
    else if (0 == cmd_head.wr)   //read para
    {
        //read para
        pr_info("cmd_head_.wr == 0  \n");
        ret = icn85xx_i2c_rxdata(0x8000, &g_structPanelPara, cmd_head.data_len);
        if (ret < 0) {
            pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
        }
        ret = copy_to_user(buffer, (void*)(&g_structPanelPara), cmd_head.data_len);
        if(ret)
        {
            pr_info("copy_to_user failed.\n");
            goto read_out;
        }

        goto read_out;

    }
    else if(2 == cmd_head.wr)  //get update status
    {
        pr_info("cmd_head_.wr == 2  \n");
        retvalue = icn85xx_get_status();
        pr_info("status: %d\n", retvalue);
        ret =copy_to_user(buffer, (void*)(&retvalue), 1);
        if(ret)
        {
            pr_info("copy_to_user failed.\n");
            goto read_out;
        }
    }
    else if(4 == cmd_head.wr)  //read rawdata
    {
        pr_info("cmd_head_.wr == 4  \n");
        row = cmd_head.addr[1];
        column = cmd_head.addr[0];
        max_column = (cmd_head.flag==0)?(24):(cmd_head.flag);
        //scan tp rawdata
        icn85xx_write_reg(4, 0x20);
        mdelay(1);
        for(i=0; i<1000; i++)
        {
            mdelay(1);
            icn85xx_read_reg(2, &retvalue);
            if(retvalue == 1)
                break;
        }

        for(i=0; i<row; i++)
        {
            ret = icn85xx_i2c_rxdata(0x2000 + i*(max_column)*2,(char *) &log_rawdata[i][0], column*2);
            if (ret < 0) {
                pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
            }
            ret = copy_to_user(&buffer[column*2*i], (void*)(&log_rawdata[i][0]), column*2);
            if(ret)
            {
                pr_info("copy_to_user failed.\n");
                goto read_out;
            }
        }

        //finish scan tp rawdata
        icn85xx_write_reg(2, 0x0);
        icn85xx_write_reg(4, 0x21);
        goto read_out;
    }
    else if(6 == cmd_head.wr)  //read diffdata
    {
        pr_info("cmd_head_.wr == 6   \n");
        row = cmd_head.addr[1];
        column = cmd_head.addr[0];
        max_column = (cmd_head.flag==0)?(24):(cmd_head.flag);
        //scan tp rawdata
        icn85xx_write_reg(4, 0x20);
        mdelay(1);

        for(i=0; i<1000; i++)
        {
            mdelay(1);
            icn85xx_read_reg(2, &retvalue);
            if(retvalue == 1)
                break;
        }

        for(i=0; i<row; i++)
        {
            ret = icn85xx_i2c_rxdata(0x3000 + (i+1)*(max_column+2)*2 + 2,(char *) &log_diffdata[i][0], column*2);
            if (ret < 0) {
                pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
            }
            ret = copy_to_user(&buffer[column*2*i], (void*)(&log_diffdata[i][0]), column*2);
            if(ret)
            {
                pr_info("copy_to_user failed.\n");
                goto read_out;
            }

        }
        //finish scan tp rawdata
        icn85xx_write_reg(2, 0x0);
        icn85xx_write_reg(4, 0x21);

        goto read_out;
    }
    else if(8 == cmd_head.wr)  //change TxVol, read diff
    {
        pr_info("cmd_head_.wr == 8  \n");
        row = cmd_head.addr[1];
        column = cmd_head.addr[0];
        max_column = (cmd_head.flag==0)?(24):(cmd_head.flag);
        //scan tp rawdata
        icn85xx_write_reg(4, 0x20);
        mdelay(1);
        for(i=0; i<1000; i++)
        {
            mdelay(1);
            icn85xx_read_reg(2, &retvalue);
            if(retvalue == 1)
                break;
        }

        for(i=0; i<row; i++)
        {
            ret = icn85xx_i2c_rxdata(0x2000 + i*(max_column)*2,(char *) &log_rawdata[i][0], column*2);
            if (ret < 0) {
                pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
            }

        }

        icn85xx_write_reg(2, 0x0);
        icn85xx_write_reg(4, 0x21);

        icn85xx_write_reg(4, 0x12);

        //scan tp rawdata
        icn85xx_write_reg(4, 0x20);
        mdelay(1);
        for(i=0; i<1000; i++)
        {
            mdelay(1);
            icn85xx_read_reg(2, &retvalue);
            if(retvalue == 1)
                break;
        }

        for(i=0; i<row; i++)
        {
            ret = icn85xx_i2c_rxdata(0x2000 + i*(max_column)*2,(char *) &log_diffdata[i][0], column*2);
            if (ret < 0) {
                pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
            }

            for(j=0; j<column; j++)
            {
                log_basedata[i][j] = log_rawdata[i][j] - log_diffdata[i][j];
            }
            ret = copy_to_user(&buffer[column*2*i], (void*)(&log_basedata[i][0]), column*2);
            if(ret)
            {
                pr_info("copy_to_user failed.\n");
                goto read_out;
            }

        }

        //finish scan tp rawdata
        icn85xx_write_reg(2, 0x0);
        icn85xx_write_reg(4, 0x21);

        icn85xx_write_reg(4, 0x10);

        goto read_out;
    }
    else if(10 == cmd_head.wr)  //read adc data
    {
        pr_info("cmd_head_.wr == 10  \n");
        if(cmd_head.flag == 0)
        {
            icn85xx_prog_write_reg(0x040874, 0);
        }
        icn85xx_prog_read_page(2500*cmd_head.flag,(char *) &log_diffdata[0][0],cmd_head.data_len);
        ret = copy_to_user(buffer, (void*)(&log_diffdata[0][0]), cmd_head.data_len);
        if(ret)
        {
            pr_info("copy_to_user failed.\n");
            goto read_out;
        }

        if(cmd_head.flag == 9)
        {
            //reload code
            if((icn85xx_ts->ictype == ICN85XX_WITHOUT_FLASH_85) || (icn85xx_ts->ictype == ICN85XX_WITHOUT_FLASH_86))
            {
                if(R_OK == icn85xx_fw_update(firmware))
                {
                    icn85xx_ts->code_loaded_flag = 1;
                    pr_info("ICN85XX_WITHOUT_FLASH, reload code ok\n");
                }
                else
                {
                    icn85xx_ts->code_loaded_flag = 0;
                    pr_info("ICN85XX_WITHOUT_FLASH, reload code error\n");
                }
            }
            else
            {
                icn85xx_bootfrom_flash(icn85xx_ts->ictype);
                msleep(150);

            }
            icn85xx_ts->work_mode = 0;
        }
    }
    else if(12 == cmd_head.wr) //read hostcomm
    {
        pr_info("cmd_head_.wr == 12  \n");
        addr = (cmd_head.addr[1]<<8) | cmd_head.addr[0];
        icn85xx_read_reg(addr, &retvalue);
        ret = copy_to_user(buffer, (void*)(&retvalue), 1);
        if(ret)
        {
            pr_info("copy_to_user failed.\n");
            goto read_out;
        }
    }
    else if(14 == cmd_head.wr) //read adc status
    {
        pr_info("cmd_head_.wr == 14  \n");
        icn85xx_prog_read_reg(0x4085E, &retvalue);
        ret = copy_to_user(buffer, (void*)(&retvalue), 1);
        if(ret)
        {
            pr_info("copy_to_user failed.\n");
            goto read_out;
        }
        pr_info("0x4085E: 0x%x\n", retvalue);
    }
    else if(16 == cmd_head.wr)  //read gesture data
    {
        pr_info("cmd_head_.wr == 16  \n");
        ret = copy_to_user(buffer, (void*)(&structGestureData), sizeof(structGestureData));
        if(ret)
        {
            pr_info("copy_to_user failed.\n");
            goto read_out;
        }

        if(structGestureData.u8Status == 1)
            structGestureData.u8Status = 0;
    }
    else if(18 == cmd_head.wr) // read hostcomm multibyte
    {
        pr_info("cmd_head_.wr == 18  \n");
        addr = (cmd_head.addr[1]<<8) | cmd_head.addr[0];
        ret = icn85xx_i2c_rxdata(addr, &cmd_head.data[0], cmd_head.data_len);
        if(ret < 0)
        {
            pr_info("copy_to_user failed.\n");
            goto read_out;
        }
        ret = copy_to_user(buffer, &cmd_head.data[0], cmd_head.data_len);
        if (ret) {
            pr_err("read hostcomm multibyte failed: %d\n", ret);
        }
        pr_info("rd, addr: 0x%x, data_len: %d, data: 0x%x\n", addr, cmd_head.data_len, cmd_head.data[0]);
        goto read_out;

    }
    else if(20 == cmd_head.wr)// read iic porgmode multibyte
    {
        pr_info("cmd_head_.wr == 20  \n");
        prog_addr = (cmd_head.flag<<16) | (cmd_head.addr[1]<<8) | cmd_head.addr[0];
        icn85xx_goto_progmode();

        ret = icn85xx_prog_i2c_rxdata(prog_addr, &cmd_head.data[0], cmd_head.data_len);
        if (ret < 0) {
            pr_err("read iic porgmode multibyte failed: %d\n", ret);
        }
        ret = copy_to_user(buffer, &cmd_head.data[0], cmd_head.data_len);
        if(ret)
        {
            pr_info("copy_to_user failed.\n");
            goto read_out;
        }


        icn85xx_bootfrom_sram();
        goto read_out;

    }
		else if(22 == cmd_head.wr) //read ictype
		{
			pr_info("cmd_head_.wr == 22  \n");
			ret = copy_to_user(buffer, (void*)(&icn85xx_ts->ictype), 1);
			if(ret)
			{
				pr_info("copy_to_user failed.\n");
				goto read_out;
			}
		}
read_out:
    up(&icn85xx_ts->sem);
    pr_info("%s out: %d, cmd_head.data_len: %d\n\n",__func__, count, cmd_head.data_len);
    return cmd_head.data_len;
}

static const struct file_operations icn85xx_proc_fops = {
    .owner      = THIS_MODULE,
    .read       = icn85xx_tool_read,
    .write      = icn85xx_tool_write,
};

void init_proc_node(void)
{
    int i;
    memset(&cmd_head, 0, sizeof(cmd_head));
    cmd_head.data = NULL;

    i = 5;
    while ((!cmd_head.data) && i)
    {
        cmd_head.data = kzalloc(i * DATA_LENGTH_UINT, GFP_KERNEL);
        if (NULL != cmd_head.data)
        {
            break;
        }
        i--;
    }
    if (i)
    {
        DATA_LENGTH = i * DATA_LENGTH_UINT;
        pr_info("alloc memory size:%d.\n", DATA_LENGTH);
    }
    else
    {
        pr_info("alloc for memory failed.\n");
        return ;
    }

    icn85xx_proc_entry = proc_create(ICN85XX_ENTRY_NAME, 0666, NULL, &icn85xx_proc_fops);
    if (icn85xx_proc_entry == NULL)
    {
        pr_info("Couldn't create proc entry!\n");
        return ;
    }
    else
    {
        pr_info("Create proc entry success!\n");
    }

    return ;
}

void uninit_proc_node(void)
{
    kfree(cmd_head.data);
    cmd_head.data = NULL;
    remove_proc_entry(ICN85XX_ENTRY_NAME, NULL);
}

#endif


#if TOUCH_VIRTUAL_KEYS
static ssize_t virtual_keys_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf,
     __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":100:1030:50:60"
     ":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":280:1030:50:60"
     ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":470:1030:50:60"
     ":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":900:1030:50:60"
     "\n");
}

static struct kobj_attribute virtual_keys_attr = {
    .attr = {
        .name = "virtualkeys.chipone-ts",
        .mode = S_IRUGO,
    },
    .show = &virtual_keys_show,
};

static struct attribute *properties_attrs[] = {
    &virtual_keys_attr.attr,
    NULL
};

static struct attribute_group properties_attr_group = {
    .attrs = properties_attrs,
};

static void icn85xx_ts_virtual_keys_init(void)
{
    int ret = 0;

    struct kobject *properties_kobj;
    properties_kobj = kobject_create_and_add("board_properties", NULL);
    if (properties_kobj)
        ret = sysfs_create_group(properties_kobj,
                     &properties_attr_group);
    if (!properties_kobj || ret)
        pr_err("failed to create board_properties\n");
}
#endif

int icn85xx_ts_reset(void)
{
	struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);
    int err = 0;

	err = gpio_request(icn85xx_ts->board_info->irq_pin, "tp irq_pin");
	if (err) {
	   // printk("######%s----%d\n",__func__,__LINE__);
	    pr_err("icn87xx irq_pin gpio request failed.\n");
		return -1;
	}
	gpio_set_value(icn85xx_ts->board_info->irq_pin, 0);
       mdelay(10);

	gpio_set_value(icn85xx_ts->board_info->reset_pin, 0);
	mdelay(30);
	gpio_set_value(icn85xx_ts->board_info->reset_pin, 1);
	mdelay(50);

    gpio_set_value(icn85xx_ts->board_info->irq_pin, 1);

    gpio_free(icn85xx_ts->board_info->irq_pin);

   return 1;
}


void icn85xx_set_prog_addr(void)
{
    icn85xx_ts_reset();

}

/***********************************************************************************************
Name    :   icn85xx_irq_disable
Input   :   void
Output  :   ret
function    : this function is used to disable irq
***********************************************************************************************/
void icn85xx_irq_disable(void)
{
    unsigned long irqflags;
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);

    spin_lock_irqsave(&icn85xx_ts->irq_lock, irqflags);
    if (!icn85xx_ts->irq_is_disable)
    {
        icn85xx_ts->irq_is_disable = 1;
        disable_irq_nosync(icn85xx_ts->irq);

    }

    spin_unlock_irqrestore(&icn85xx_ts->irq_lock, irqflags);
}

/***********************************************************************************************
Name    :   icn85xx_irq_enable
Input   :   void
Output  :   ret
function    : this function is used to enable irq
***********************************************************************************************/
void icn85xx_irq_enable(void)
{
    unsigned long irqflags = 0;
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);

    spin_lock_irqsave(&icn85xx_ts->irq_lock, irqflags);
    if (icn85xx_ts->irq_is_disable)
    {
        enable_irq(icn85xx_ts->irq);
        icn85xx_ts->irq_is_disable = 0;
    }

    spin_unlock_irqrestore(&icn85xx_ts->irq_lock, irqflags);

}

int icn85xx_prog_i2c_rxdata(unsigned int addr, char *rxdata, int length)
{
    int ret = -1;
    int retries = 0;

    unsigned char tmp_buf[3];
    struct i2c_msg msgs[] = {
        {
            .addr   = ICN85XX_PROG_IIC_ADDR,//this_client->addr,
            .flags  = 0,
            .len    = 3,
            .buf    = tmp_buf,
        },
        {
            .addr   = ICN85XX_PROG_IIC_ADDR,//this_client->addr,
            .flags  = I2C_M_RD,
            .len    = length,
            .buf    = rxdata,
        },
    };

    tmp_buf[0] = (unsigned char)(addr>>16);
    tmp_buf[1] = (unsigned char)(addr>>8);
    tmp_buf[2] = (unsigned char)(addr);

    while(retries < IIC_RETRY_NUM)
    {
	ret =	i2c_transfer(this_client->adapter, msgs, 2);
   	if(ret == 2) break;
        retries++;
    }

    if (retries >= IIC_RETRY_NUM)
    {
        pr_err("%s i2c read error: %d\n", __func__, ret);
    }

    return ret;
}

int icn85xx_prog_i2c_txdata(unsigned int addr, char *txdata, int length)
{
    int ret = -1;
    char tmp_buf[128];
    int retries = 0;
    struct i2c_msg msg[] = {
        {
            .addr   = ICN85XX_PROG_IIC_ADDR,//this_client->addr,
            .flags  = 0,
            .len    = length + 3,
            .buf    = tmp_buf,
        },
    };

    if (length > 125)
    {
        pr_err("%s too big datalen = %d!\n", __func__, length);
        return -1;
    }

    tmp_buf[0] = (unsigned char)(addr>>16);
    tmp_buf[1] = (unsigned char)(addr>>8);
    tmp_buf[2] = (unsigned char)(addr);


    if (length != 0 && txdata != NULL)
    {
        memcpy(&tmp_buf[3], txdata, length);
    }

    while(retries < IIC_RETRY_NUM)
    {
	ret =	i2c_transfer(this_client->adapter, msg, 1);
        if(ret == 1) break;
        retries++;
    }

    if (retries >= IIC_RETRY_NUM)
    {
        pr_err("%s i2c write error: %d\n", __func__, ret);
//        icn85xx_ts_reset();
    }
    return ret;
}

int icn85xx_prog_write_reg(unsigned int addr, char para)
{
    char buf[3];
    int ret = -1;

    buf[0] = para;
    ret = icn85xx_prog_i2c_txdata(addr, buf, 1);
    if (ret < 0) {
        pr_err("%s write reg failed! %#x ret: %d\n", __func__, buf[0], ret);
        return -1;
    }
    return ret;
}


int icn85xx_prog_read_reg(unsigned int addr, char *pdata)
{
    int ret = -1;
    ret = icn85xx_prog_i2c_rxdata(addr, pdata, 1);
    return ret;
}

int icn85xx_prog_read_page(unsigned int Addr,unsigned char *Buffer, unsigned int Length)
{
    int ret =0;
    unsigned int StartAddr = Addr;
    while(Length){
        if(Length > MAX_LENGTH_PER_TRANSFER){
            ret = icn85xx_prog_i2c_rxdata(StartAddr, Buffer, MAX_LENGTH_PER_TRANSFER);
            Length -= MAX_LENGTH_PER_TRANSFER;
            Buffer += MAX_LENGTH_PER_TRANSFER;
            StartAddr += MAX_LENGTH_PER_TRANSFER;
        }
        else{
            ret = icn85xx_prog_i2c_rxdata(StartAddr, Buffer, Length);
            Length = 0;
            Buffer += Length;
            StartAddr += Length;
            break;
        }
        pr_err("\n icn85xx_prog_read_page StartAddr:0x%x, length: %d\n",StartAddr,Length);
    }
    if (ret < 0) {
        pr_err("\n icn85xx_prog_read_page failed! StartAddr:  0x%x, ret: %d\n", StartAddr, ret);
        return ret;
    }
    else{
          pr_info("\n icn85xx_prog_read_page, StartAddr 0x%x, Length: %d\n", StartAddr, Length);
          return ret;
      }
}


int icn87xx_prog_read_page(unsigned int Addr,unsigned char *Buffer, unsigned int Length)
{
    int ret =0;
    unsigned int StartAddr = Addr;
    while(Length){
        if(Length > MAX_LENGTH_PER_TRANSFER){
            ret = icn87xx_prog_i2c_rxdata(StartAddr, Buffer, MAX_LENGTH_PER_TRANSFER);
            Length -= MAX_LENGTH_PER_TRANSFER;
            Buffer += MAX_LENGTH_PER_TRANSFER;
            StartAddr += MAX_LENGTH_PER_TRANSFER;
        }
        else{
            ret = icn87xx_prog_i2c_rxdata(StartAddr, Buffer, Length);
            Length = 0;
            Buffer += Length;
            StartAddr += Length;
            break;
        }
    }
    if (ret < 0) {
        pr_err("\n icn87xx_prog_read_page failed! StartAddr:  0x%x, ret: %d\n", StartAddr, ret);
        return ret;
    }
    else{
          pr_info("\n icn87xx_prog_read_page, StartAddr 0x%x, Length: %d\n", StartAddr, Length);
          return ret;
      }
}

 int icn85xx_i2c_rxdata(unsigned short addr, char *rxdata, int length)
{
    int ret = -1;
    int retries = 0;
    unsigned char tmp_buf[2];
    struct i2c_msg msgs[] = {
        {
            .addr   = 0x48,
            .flags  = 0,
            .len    = 2,
            .buf    = tmp_buf,
        },
        {
            .addr   = 0x48,
            .flags  = I2C_M_RD,
            .len    = length,
            .buf    = rxdata,
        },
    };

    tmp_buf[0] = (unsigned char)(addr>>8);
    tmp_buf[1] = (unsigned char)(addr);


    while(retries < IIC_RETRY_NUM)
    {
	ret = i2c_transfer(this_client->adapter, msgs, 2);
      	if(ret == 2) break;
        retries++;
    }

    if (retries >= IIC_RETRY_NUM)
    {
        pr_err("%s i2c read error: %d\n", __func__, ret);
    }
     return ret;
}


int icn85xx_i2c_txdata(unsigned short addr, char *txdata, int length)
{
    int ret = -1;
    unsigned char tmp_buf[128];
    int retries = 0;

    struct i2c_msg msg[] = {
        {
            .addr   = this_client->addr,
            .flags  = 0,
            .len    = length + 2,
            .buf    = tmp_buf,
        },
    };

    if (length > 125)
    {
        pr_err("%s too big datalen = %d!\n", __func__, length);
        return -1;
    }

    tmp_buf[0] = (unsigned char)(addr>>8);
    tmp_buf[1] = (unsigned char)(addr);

    if (length != 0 && txdata != NULL)
    {
        memcpy(&tmp_buf[2], txdata, length);
    }

    while(retries < IIC_RETRY_NUM)
    {
	 ret =	i2c_transfer(this_client->adapter, msg, 1);
        if(ret == 1) break;
        retries++;
    }

    if (retries >= IIC_RETRY_NUM)
    {
        pr_err("%s i2c write error: %d\n", __func__, ret);
     }

    return ret;
}



int icn87xx_prog_i2c_rxdata(unsigned int addr, char *rxdata, int length)
{
    int ret = -1;
    int retries = 0;

    unsigned char tmp_buf[3];
    struct i2c_msg msgs[] = {
        {
            .addr   = ICN87XX_PROG_IIC_ADDR,//this_client->addr,
            .flags  = 0,
            .len    = 2,
            .buf    = tmp_buf,
        },
        {
            .addr   = ICN87XX_PROG_IIC_ADDR,//this_client->addr,
            .flags  = I2C_M_RD,
            .len    = length,
            .buf    = rxdata,
        },
    };

    tmp_buf[0] = (unsigned char)(addr>>8);
    tmp_buf[1] = (unsigned char)(addr);


    while(retries < IIC_RETRY_NUM)
    {


	ret =	i2c_transfer(this_client->adapter, msgs, 2);
   	if(ret == 2) break;
        retries++;

    }

    if (retries >= IIC_RETRY_NUM)
    {
        pr_err("%s i2c read error: %d\n", __func__, ret);
     }

    return ret;
}

int icn87xx_prog_i2c_txdata(unsigned int addr, char *txdata, int length)
{
    int ret = -1;
    char tmp_buf[128];
    int retries = 0;
    struct i2c_msg msg[] = {
        {
            .addr   = ICN87XX_PROG_IIC_ADDR,//this_client->addr,
            .flags  = 0,
            .len    = length + 2,
            .buf    = tmp_buf,
        },
    };

    if (length > 125)
    {
        pr_err("%s too big datalen = %d!\n", __func__, length);
        return -1;
    }

    tmp_buf[0] = (unsigned char)(addr>>8);
    tmp_buf[1] = (unsigned char)(addr);


    if (length != 0 && txdata != NULL)
    {
        memcpy(&tmp_buf[2], txdata, length);
    }

    while(retries < IIC_RETRY_NUM)
    {
	ret =	i2c_transfer(this_client->adapter, msg, 1);
        if(ret == 1) break;
        retries++;
    }

    if (retries >= IIC_RETRY_NUM)
    {
        pr_err("%s i2c write error: %d\n", __func__, ret);
    }
    return ret;
}


int icn85xx_write_reg(unsigned short addr, char para)
{
    char buf[3];
    int ret = -1;

    buf[0] = para;
    ret = icn85xx_i2c_txdata(addr, buf, 1);
    if (ret < 0) {
        pr_err("write reg failed! %#x ret: %d\n", buf[0], ret);
        return -1;
    }

    return ret;
}


int icn85xx_read_reg(unsigned short addr, char *pdata)
{
    int ret = -1;

    ret = icn85xx_i2c_rxdata(addr, pdata, 1);
	pr_info("addr: 0x%x: 0x%x\n", addr, *pdata);

    return ret;
}


static void icn85xx_log(char diff)
{
    char row = 0;
    char column = 0;
    int i, j, ret;
    char retvalue = 0;

    icn85xx_read_reg(0x8004, &row);
    icn85xx_read_reg(0x8005, &column);

    //scan tp rawdata
    icn85xx_write_reg(4, 0x20);
    mdelay(1);
    for(i=0; i<1000; i++)
    {
        mdelay(1);
        icn85xx_read_reg(2, &retvalue);
        if(retvalue == 1)
            break;
    }
    if(diff == 0)
    {
        for(i=0; i<row; i++)
        {
            ret = icn85xx_i2c_rxdata(0x2000 + i*COL_NUM*2, (char *)&log_rawdata[i][0], column*2);
            if (ret < 0) {
                pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
            }
            icn85xx_rawdatadump(&log_rawdata[i][0], column, COL_NUM);

        }
    }
    if(diff == 1)
    {
        for(i=0; i<row; i++)
        {
            ret = icn85xx_i2c_rxdata(0x3000 + (i+1)*(COL_NUM+2)*2 + 2, (char *)&log_diffdata[i][0], column*2);
            if (ret < 0) {
                pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
            }
            icn85xx_rawdatadump(&log_diffdata[i][0], column, COL_NUM);

        }
    }
    else if(diff == 2)
    {
        for(i=0; i<row; i++)
        {
            ret = icn85xx_i2c_rxdata(0x2000 + i*COL_NUM*2, (char *)&log_rawdata[i][0], column*2);
            if (ret < 0) {
                pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
            }
            if((log_basedata[0][0] != 0) || (log_basedata[0][1] != 0))
            {
                for(j=0; j<column; j++)
                {
                    log_rawdata[i][j] = log_basedata[i][j] - log_rawdata[i][j];
                }
            }
            icn85xx_rawdatadump(&log_rawdata[i][0], column, COL_NUM);

        }
        if((log_basedata[0][0] == 0) && (log_basedata[0][1] == 0))
        {
            memcpy(&log_basedata[0][0], &log_rawdata[0][0], COL_NUM*ROW_NUM*2);
        }


    }

    //finish scan tp rawdata
    icn85xx_write_reg(2, 0x0);
    icn85xx_write_reg(4, 0x21);
}


static int icn85xx_iic_test(void)
{
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);
    int  ret = -1;
    unsigned char value = 0;
    unsigned char buf[3];
    int  retry = 0;
    int  flashid;
    icn85xx_ts->ictype = 0;

    icn85xx_ts->ictype = ICTYPE_UNKNOWN;

    while(retry++ < 3)
    {
        ret = icn85xx_read_reg(0xa, &value);

        if(ret > 0)
        {
            if(value == 0x85)
            {
                icn85xx_ts->ictype = ICN85XX_WITH_FLASH_85;
                setbootmode(ICN85XX_WITH_FLASH_85);
                return ret;
            }
            else if((value == 0x86)||(value == 0x88))
            {
                icn85xx_ts->ictype = ICN85XX_WITH_FLASH_86;
                setbootmode(ICN85XX_WITH_FLASH_86);
                return ret;
            }
            else if(value == 0x87)
            {
                icn85xx_ts->ictype = ICN85XX_WITH_FLASH_87;
                setbootmode(ICN85XX_WITH_FLASH_87);
                return ret;
            }
			else if(value == 0x81)
            {
                icn85xx_ts->ictype = ICN85XX_WITH_FLASH_87;
                setbootmode(ICN85XX_WITH_FLASH_87);
            return ret;
            }
        }
        pr_err("iic test error! retry = %d\n", retry);
        msleep(3);
    }

    // force ic enter progmode
    icn85xx_goto_progmode();
    msleep(10);

    retry = 0;
    while(retry++ < 3)
    {
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;

	ret = icn85xx_prog_i2c_txdata(0x040000, buf, 3);
	if (ret < 0)
	{
		pr_info("%s, %d\n", __func__, __LINE__);
		return ret;
	}

        ret = icn85xx_prog_i2c_rxdata(0x040000, buf, 3);
        if(ret > 0)
        {
            //if(value == 0x85)
            if((buf[2] == 0x85) && (buf[1] == 0x05))
            {
                flashid = icn85xx_read_flashid();
                if((MD25D40_ID1 == flashid) || (MD25D40_ID2 == flashid)
                    ||(MD25D20_ID1 == flashid) || (MD25D20_ID2 == flashid)
                    ||(GD25Q10_ID == flashid) || (MX25L512E_ID == flashid)
                    || (MD25D05_ID == flashid)|| (MD25D10_ID == flashid))
                {
                    icn85xx_ts->ictype = ICN85XX_WITH_FLASH_85;
                    setbootmode(ICN85XX_WITH_FLASH_85);
                }
                else
                {
                    icn85xx_ts->ictype = ICN85XX_WITHOUT_FLASH_85;
                    setbootmode(ICN85XX_WITHOUT_FLASH_85);
                }
                return ret;
            }
            else if((buf[2] == 0x85) && (buf[1] == 0x0e))
            {
                flashid = icn85xx_read_flashid();
                if((MD25D40_ID1 == flashid) || (MD25D40_ID2 == flashid)
                    ||(MD25D20_ID1 == flashid) || (MD25D20_ID2 == flashid)
                    ||(GD25Q10_ID == flashid) || (MX25L512E_ID == flashid)
                    || (MD25D05_ID == flashid)|| (MD25D10_ID == flashid))
                {
                    icn85xx_ts->ictype = ICN85XX_WITH_FLASH_86;
                    setbootmode(ICN85XX_WITH_FLASH_86);
                }
                else
                {
                    icn85xx_ts->ictype = ICN85XX_WITHOUT_FLASH_86;
                    setbootmode(ICN85XX_WITHOUT_FLASH_86);
                }
                return ret;
            }
            else  //for ICNT87
            {
                ret = icn87xx_prog_i2c_rxdata(0xf001, buf, 2);
                if(ret > 0)
                {
                    if(buf[1] == 0x87)
                    {
                        flashid = icn87xx_read_flashid();
                        pr_info("icnt87 flashid: 0x%x\n",flashid);
                        if((MD25D40_ID1 == flashid) || (MD25D40_ID2 == flashid)
                         ||(MD25D20_ID1 == flashid) || (MD25D20_ID2 == flashid)
                         ||(GD25Q10_ID == flashid) || (MX25L512E_ID == flashid)
                         || (MD25D05_ID == flashid)|| (MD25D10_ID == flashid))
                        {
                            icn85xx_ts->ictype = ICN85XX_WITH_FLASH_87;
                            setbootmode(ICN85XX_WITH_FLASH_87);
                        }
                        else
                        {
                            icn85xx_ts->ictype = ICN85XX_WITHOUT_FLASH_87;
                            setbootmode(ICN85XX_WITHOUT_FLASH_87);
                        }
                        return ret;
                    }
                    else if(buf[1] == 0x8b)
                    {
                        flashid = icn87xx_read_flashid();
                        pr_info("icnt87 flashid: 0x%x\n",flashid);
                        if((MD25D40_ID1 == flashid) || (MD25D40_ID2 == flashid)
                         ||(MD25D20_ID1 == flashid) || (MD25D20_ID2 == flashid)
                         ||(GD25Q10_ID == flashid) || (MX25L512E_ID == flashid)
                         || (MD25D05_ID == flashid)|| (MD25D10_ID == flashid))
                        {
                            icn85xx_ts->ictype = ICN85XX_WITH_FLASH_87;
                            setbootmode(ICN85XX_WITH_FLASH_87);
                        }
                        else
                        {
                            icn85xx_ts->ictype = ICN85XX_WITHOUT_FLASH_87;
                            setbootmode(ICN85XX_WITHOUT_FLASH_87);
                        }
                        return ret;
                    }
                }
            }
        }
        pr_err("iic2 test error! %d\n", retry);
        msleep(3);
    }

    return ret;
}

#if !CTP_REPORT_PROTOCOL

static void icn85xx_ts_release(void)
{
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);
    //pr_info("==icn85xx_ts_release ==\n");
    input_report_abs(icn85xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
    input_sync(icn85xx_ts->input_dev);
}

static void icn85xx_report_value_A(void)
{   //printk("######%s-----%d\n",__func__,__LINE__);
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);
    unsigned char buf[POINT_NUM*POINT_SIZE+3]={0};
    int ret = -1;
    int i=0, j=0, nBytes=0;
#if TOUCH_VIRTUAL_KEYS
    unsigned char button;
    static unsigned char button_last;
#endif

	nBytes = POINT_NUM*POINT_SIZE+2;
	while (nBytes) {
		i = nBytes > 4 ? 4 : nBytes;
		ret = icn85xx_i2c_rxdata(0x1000+j, buf+j, i);
		if (ret < 0) {
			pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
			return ;
		}
		j = j + i;
		nBytes = nBytes - i;
	}

#if TOUCH_VIRTUAL_KEYS
    button = buf[0];

    if((button_last != 0) && (button == 0))
    {
        icn85xx_ts_release();
        button_last = button;
        return ;
    }
    if(button != 0)
    {
        switch(button)
        {
            case ICN_VIRTUAL_BUTTON_HOME:
                pr_info("ICN_VIRTUAL_BUTTON_HOME down\n");
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 200);
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_POSITION_X, 280);
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_POSITION_Y, 1030);
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_WIDTH_MAJOR, 1);
                input_mt_sync(icn85xx_ts->input_dev);
                input_sync(icn85xx_ts->input_dev);
                break;
            case ICN_VIRTUAL_BUTTON_BACK:
                pr_info("ICN_VIRTUAL_BUTTON_BACK down\n");
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 200);
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_POSITION_X, 470);
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_POSITION_Y, 1030);
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_WIDTH_MAJOR, 1);
                input_mt_sync(icn85xx_ts->input_dev);
                input_sync(icn85xx_ts->input_dev);
                break;
            case ICN_VIRTUAL_BUTTON_MENU:
                pr_info("ICN_VIRTUAL_BUTTON_MENU down\n");
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 200);
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_POSITION_X, 100);
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_POSITION_Y, 1030);
                input_report_abs(icn85xx_ts->input_dev, ABS_MT_WIDTH_MAJOR, 1);
                input_mt_sync(icn85xx_ts->input_dev);
                input_sync(icn85xx_ts->input_dev);
                break;
            default:
                pr_info("other gesture\n");
                break;
        }
        button_last = button;
        return ;
    }
#endif
 // pr_err("=====%s-----%d\n",__func__,__LINE__);
    icn85xx_ts->point_num = buf[1];
    if (icn85xx_ts->point_num == 0) {
        icn85xx_ts_release();
        return ;
    }

    if (icn85xx_ts->point_num > POINT_NUM) {
        icn85xx_ts->point_num = 1;
    }

    for(i=0;i<icn85xx_ts->point_num;i++){
        if(buf[8 + POINT_SIZE*i]  != 4)
        {
          break ;
        }
        else
        {

        }
    }

    if(i == icn85xx_ts->point_num) {
        icn85xx_ts_release();
        return ;
    }

    for(i=0; i<icn85xx_ts->point_num; i++)
    {    // pr_err("=====%s-----%d\n",__func__,__LINE__);
        icn85xx_ts->point_info[i].u8ID = buf[2 + POINT_SIZE*i];
        icn85xx_ts->point_info[i].u16PosX = (buf[4 + POINT_SIZE*i]<<8) + buf[3 + POINT_SIZE*i];
        icn85xx_ts->point_info[i].u16PosY = (buf[6 + POINT_SIZE*i]<<8) + buf[5 + POINT_SIZE*i];
        icn85xx_ts->point_info[i].u8Pressure = buf[7 + POINT_SIZE*i];
        icn85xx_ts->point_info[i].u8EventId = buf[8 + POINT_SIZE*i];
        #if SWAP_X_Y
        swap_ab(icn85xx_ts->point_info[i].u16PosX,icn85xx_ts->point_info[i].u16PosY);
        #endif
        if(1 == icn85xx_ts->board_info->revert_x_flag)
        {
            icn85xx_ts->point_info[i].u16PosX = icn85xx_ts->screen_max_x-1- icn85xx_ts->point_info[i].u16PosX;
        }
        if(1 == icn85xx_ts->board_info->revert_y_flag)
        {
            icn85xx_ts->point_info[i].u16PosY = icn85xx_ts->screen_max_y-1- icn85xx_ts->point_info[i].u16PosY;
        }
        if(icn85xx_ts->point_info[i].u8ID < POINT_NUM &&\
           icn85xx_ts->point_info[i].u16PosX < SCREEN_MAX_X &&\
           icn85xx_ts->point_info[i].u16PosY < SCREEN_MAX_Y &&\
           icn85xx_ts->point_info[i].u8Pressure < SCREEN_PRESSURE&&\
           icn85xx_ts->point_info[i].u8EventId < EVENT_ID){

        //printk("\n before=====%s-----%d u16PosX=%d u16PosY=%d \n",__func__,__LINE__,icn85xx_ts->point_info[i].u16PosX,icn85xx_ts->point_info[i].u16PosY);

        if(icn85xx_ts->point_info[i].u16PosX != 0)
        {
            icn85xx_ts->point_info[i].u16PosX = ((icn85xx_ts->point_info[i].u16PosX*800)/SCREEN_MAX_X);
        }
        if(icn85xx_ts->point_info[i].u16PosY != 0)
        {
            icn85xx_ts->point_info[i].u16PosY = ((icn85xx_ts->point_info[i].u16PosY*480)/SCREEN_MAX_Y);
        }

        //printk("\n after=====%s-----%d u16PosX=%d u16PosY=%d \n",__func__,__LINE__,icn85xx_ts->point_info[i].u16PosX,icn85xx_ts->point_info[i].u16PosY);

        /*point_id default is 0,but in order to keep up with gsl3692, we take it as 1. */
       //  pr_err("=====%s-----%d\n",__func__,__LINE__);
        ///input_report_abs(icn85xx_ts->input_dev, ABS_MT_TRACKING_ID, icn85xx_ts->point_info[i].u8ID + 1);
        ///input_report_abs(icn85xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, icn85xx_ts->point_info[i].u8Pressure);
        input_report_abs(icn85xx_ts->input_dev, ABS_MT_POSITION_X, icn85xx_ts->point_info[i].u16PosX);
        input_report_abs(icn85xx_ts->input_dev, ABS_MT_POSITION_Y, icn85xx_ts->point_info[i].u16PosY);
        ///input_report_abs(icn85xx_ts->input_dev, ABS_MT_WIDTH_MAJOR, 1);
        input_mt_sync(icn85xx_ts->input_dev);
        input_sync(icn85xx_ts->input_dev);
              }
    }


}
#else

static void icn85xx_report_value_B(void)
{
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);
    unsigned char buf[POINT_NUM*POINT_SIZE+4]={0};
    static unsigned char finger_last[POINT_NUM + 1]={0};
    unsigned char  finger_current[POINT_NUM + 1] = {0};
    unsigned int position = 0;
    int temp = 0;
    int ret = -1;
	int i = 0, j = 0, nBytes = 0;

   nBytes = POINT_NUM*POINT_SIZE+2;
   while (nBytes) {
	   i = nBytes > 32 ? 32 : nBytes;
	   ret = icn85xx_i2c_rxdata(0x1000+j, buf+j, i);
	   if (ret < 0) {
		   pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
		   return ;
	   }
	   j = j + i;
	   nBytes = nBytes - i;
   }

    icn85xx_ts->point_num = buf[1];
    if (icn85xx_ts->point_num > POINT_NUM)
     {
       return ;
     }

    if(icn85xx_ts->point_num > 0)
    {
        for(position = 0; position<icn85xx_ts->point_num; position++)
        {
            temp = buf[2 + POINT_SIZE*position] + 1;
			if (temp > POINT_NUM) {
				pr_info("ERROR: %s ,%d\n", __func__, __LINE__);
				return;
			}
            finger_current[temp] = 1;
            icn85xx_ts->point_info[temp].u8ID = buf[2 + POINT_SIZE*position];
            icn85xx_ts->point_info[temp].u16PosX = (buf[4 + POINT_SIZE*position]<<8) + buf[3 + POINT_SIZE*position];
            icn85xx_ts->point_info[temp].u16PosY = (buf[6 + POINT_SIZE*position]<<8) + buf[5 + POINT_SIZE*position];
            icn85xx_ts->point_info[temp].u8Pressure = buf[7 + POINT_SIZE*position];
            icn85xx_ts->point_info[temp].u8EventId = buf[8 + POINT_SIZE*position];

            if(icn85xx_ts->point_info[temp].u8EventId == 4)
                finger_current[temp] = 0;

            if(1 == icn85xx_ts->board_info->revert_x_flag)
            {
                icn85xx_ts->point_info[temp].u16PosX = icn85xx_ts->screen_max_x -1- icn85xx_ts->point_info[temp].u16PosX;
            }
            if(1 == icn85xx_ts->board_info->revert_y_flag)
            {
                icn85xx_ts->point_info[temp].u16PosY = icn85xx_ts->screen_max_y-1- icn85xx_ts->point_info[temp].u16PosY;
            }
		pr_info("temp %d\n", temp);
		pr_info("u8ID %d\n", icn85xx_ts->point_info[temp].u8ID);
		pr_info("u16PosX %d\n", icn85xx_ts->point_info[temp].u16PosX);
		pr_info("u16PosY %d\n", icn85xx_ts->point_info[temp].u16PosY);
		pr_info("u8Pressure %d\n", icn85xx_ts->point_info[temp].u8Pressure);
		pr_info("u8EventId %d\n", icn85xx_ts->point_info[temp].u8EventId);
		pr_info("u8Pressure %d\n", icn85xx_ts->point_info[temp].u8Pressure*16);
        }
    }
    else
    {
        for(position = 1; position < POINT_NUM+1; position++)
        {
            finger_current[position] = 0;
        }
        pr_info("no touch\n");
    }

    for(position = 1; position < POINT_NUM + 1; position++)
    {
        if((finger_current[position] == 0) && (finger_last[position] != 0))
        {
            pr_info("finger_current[position] == 0\n");
            icn85xx_point_info("one touch up: %d\n", position);
        }
        else if(finger_current[position])
        {
            pr_info("u16PosX:%d u16PosY:%d\n", icn85xx_ts->point_info[position].u16PosX, icn85xx_ts->point_info[position].u16PosY);
            /*input_mt_slot(icn85xx_ts->input_dev, position-1);
            input_mt_report_slot_state(icn85xx_ts->input_dev, MT_TOOL_FINGER, true);
            input_report_abs(icn85xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 1);*/
            //input_report_abs(icn85xx_ts->input_dev, ABS_MT_PRESSURE, icn85xx_ts->point_info[position].u8Pressure);
            /*input_report_abs(icn85xx_ts->input_dev, ABS_MT_PRESSURE, 200);
            input_report_abs(icn85xx_ts->input_dev, ABS_MT_POSITION_X, icn85xx_ts->point_info[position].u16PosX);
            input_report_abs(icn85xx_ts->input_dev, ABS_MT_POSITION_Y, icn85xx_ts->point_info[position].u16PosY);*/
           //pr_info(" ===position: %d, x = %d,y = %d, press = %d ====\n", position, icn85xx_ts->point_info[position].u16PosX,icn85xx_ts->point_info[position].u16PosY, icn85xx_ts->point_info[position].u8Pressure);
        }

    }
    input_sync(icn85xx_ts->input_dev);

    for(position = 1; position < POINT_NUM + 1; position++)
    {
        finger_last[position] = finger_current[position];
    }

}
#endif

/***********************************************************************************************
Name    :   icn85xx_ts_pen_irq_work
Input   :   void
Output  :
function    : work_struct
***********************************************************************************************/
static void icn85xx_ts_pen_irq_work(struct work_struct *work)
{
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(this_client);
#if SUPPORT_PROC_FS
    if(down_interruptible(&icn85xx_ts->sem))
    {
        return ;
    }
#endif

    if(icn85xx_ts->work_mode == 0)
    {
#if CTP_REPORT_PROTOCOL
        icn85xx_report_value_B();
#else
        icn85xx_report_value_A();
#endif
     //  pr_err("=====%s-----%d\n",__func__,__LINE__);
        if(icn85xx_ts->use_irq)
        {
            icn85xx_irq_enable();
        }
        if(log_on_off == 4)
        {
            pr_info("normal raw data\n");
            icn85xx_log(0);   //raw data
        }
        else if(log_on_off == 5)
        {
            pr_info("normal diff data\n");
            icn85xx_log(1);   //diff data
        }
        else if(log_on_off == 6)
        {
            pr_info("normal raw2diff\n");
            icn85xx_log(2);   //diff data
        }
    }
    else if(icn85xx_ts->work_mode == 1)
    {
        pr_info("raw data\n");
        icn85xx_log(0);   //raw data
    }
    else if(icn85xx_ts->work_mode == 2)
    {
        pr_info("diff data\n");
        icn85xx_log(1);   //diff data
    }
    else if(icn85xx_ts->work_mode == 3)
    {
        pr_info("raw2diff data\n");
        icn85xx_log(2);   //diff data
    }
    else if(icn85xx_ts->work_mode == 4)  //idle
    {
        ;
    }
    else if(icn85xx_ts->work_mode == 5)//write para, reinit
    {
        pr_info("reinit tp\n");
        icn85xx_write_reg(0, 1);
        mdelay(100);
        icn85xx_write_reg(0, 0);
        icn85xx_ts->work_mode = 0;
    }
    else if((icn85xx_ts->work_mode == 6) ||(icn85xx_ts->work_mode == 7))  //gesture test mode
    {
        #if SUPPORT_PROC_FS
        char buf[sizeof(structGestureData)]={0};
        int ret = -1;
        int i;
        ret = icn85xx_i2c_rxdata(0x7000, buf, 2);
        if (ret < 0) {
            pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
            return ;
        }
        structGestureData.u8Status = 1;
        structGestureData.u8Gesture = buf[0];
        structGestureData.u8GestureNum = buf[1];
        pr_debug("structGestureData.u8Gesture: 0x%x\n", structGestureData.u8Gesture);
        pr_debug("structGestureData.u8GestureNum: %d\n", structGestureData.u8GestureNum);

        ret = icn85xx_i2c_rxdata(0x7002, buf, structGestureData.u8GestureNum*6);
        if (ret < 0) {
            pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
            return ;
        }

        for(i=0; i<structGestureData.u8GestureNum; i++)
        {
            structGestureData.point_info[i].u16PosX = (buf[1 + 6*i]<<8) + buf[0+ 6*i];
            structGestureData.point_info[i].u16PosY = (buf[3 + 6*i]<<8) + buf[2 + 6*i];
            structGestureData.point_info[i].u8EventId = buf[5 + 6*i];
            pr_info("(%d, %d, %d)", structGestureData.point_info[i].u16PosX, structGestureData.point_info[i].u16PosY, structGestureData.point_info[i].u8EventId);
        }

        if(((icn85xx_ts->work_mode == 7) && (structGestureData.u8Gesture == 0xFB))
            || (icn85xx_ts->work_mode == 6))
        {
            pr_info("return normal mode\n");
            icn85xx_ts->work_mode = 0;  //return normal mode
        }

        #endif
    }

#if SUPPORT_PROC_FS
    up(&icn85xx_ts->sem);
#endif


}

static enum hrtimer_restart chipone_timer_func(struct hrtimer *timer)
{
    struct icn85xx_ts_data *icn85xx_ts = container_of(timer, struct icn85xx_ts_data, timer);
    queue_work(icn85xx_ts->ts_workqueue, &icn85xx_ts->pen_event_work);
    //pr_info("chipone_timer_func\n");
    if(icn85xx_ts->use_irq == 1)
    {
        if((icn85xx_ts->work_mode == 1) || (icn85xx_ts->work_mode == 2) || (icn85xx_ts->work_mode == 3))
        {
            hrtimer_start(&icn85xx_ts->timer, ktime_set(CTP_POLL_TIMER/1000, (CTP_POLL_TIMER%1000)*1000000), HRTIMER_MODE_REL);
        }
    }
    else
    {
        hrtimer_start(&icn85xx_ts->timer, ktime_set(CTP_POLL_TIMER/1000, (CTP_POLL_TIMER%1000)*1000000), HRTIMER_MODE_REL);
    }
    return HRTIMER_NORESTART;
}

/*  interrupt service routine
** when there is data available(a press) or in other case,
** interrupt pin gets active to signal this to the CPU.
*/
static irqreturn_t icn85xx_ts_interrupt(int irq, void *dev_id)
{
    struct icn85xx_ts_data *icn85xx_ts = dev_id;

    //pr_info("==========------icn85xx_ts TS Interrupt-----============\n");

    if(icn85xx_ts->use_irq)
        icn85xx_irq_disable();
    if (!work_pending(&icn85xx_ts->pen_event_work))
    {
        queue_work(icn85xx_ts->ts_workqueue, &icn85xx_ts->pen_event_work);

    }

    return IRQ_HANDLED;
}


static void icn85xx_request_io_port(struct icn85xx_ts_data *icn85xx_ts)
{
	icn85xx_ts->screen_max_x = icn85xx_ts->board_info->screen_max_x;
	icn85xx_ts->screen_max_y = icn85xx_ts->board_info->screen_max_y;
   return ;

}

static void icn85xx_free_io_port(struct icn85xx_ts_data *icn85xx_ts)
{
  return ;
}

static int icn85xx_request_irq(struct icn85xx_ts_data *icn85xx_ts)
{
    int err = -1;


	int irq = gpio_to_irq(icn85xx_ts->board_info->irq_pin);
	icn85xx_ts->irq = irq;

    gpio_direction_input(icn85xx_ts->board_info->irq_pin);
	err = request_irq(irq, icn85xx_ts_interrupt, IRQF_TRIGGER_FALLING|IRQF_ONESHOT, "icn85xx_ts", icn85xx_ts);
    if (err < 0)
    {
        icn85xx_ts->use_irq = 0;
        pr_err("icn85xx_ts_probe: request irq failed\n");
        return err;
    }
    icn85xx_irq_disable();
    icn85xx_ts->use_irq = 1;


    return 0;
}

/*
static void icn85xx_free_irq(struct icn85xx_ts_data *icn85xx_ts)
{
    if (icn85xx_ts)
    {
        if (icn85xx_ts->use_irq)
        {
            free_irq(icn85xx_ts->irq, icn85xx_ts);
        }
        else
        {
            hrtimer_cancel(&icn85xx_ts->timer);
        }
    }

}
*/
static int icn85xx_request_input_dev(struct icn85xx_ts_data *icn85xx_ts)
{
    int ret = -1;
    struct input_dev *input_dev;

    input_dev = input_allocate_device();
    if (!input_dev) {
        pr_err("failed to allocate input device\n");
        return -ENOMEM;
    }
    icn85xx_ts->input_dev = input_dev;

    icn85xx_ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
#if CTP_REPORT_PROTOCOL
//     __set_bit(INPUT_PROP_DIRECT, icn85xx_ts->input_dev->propbit);
//     input_mt_init_slots(icn85xx_ts->input_dev, POINT_NUM*2, 0);
#else
    set_bit(ABS_MT_TOUCH_MAJOR, icn85xx_ts->input_dev->absbit);
    set_bit(ABS_MT_POSITION_X, icn85xx_ts->input_dev->absbit);
    set_bit(ABS_MT_POSITION_Y, icn85xx_ts->input_dev->absbit);
    set_bit(ABS_MT_WIDTH_MAJOR, icn85xx_ts->input_dev->absbit);
#endif
    input_set_abs_params(icn85xx_ts->input_dev, ABS_MT_POSITION_X, 0, icn85xx_ts->board_info->screen_max_x, 0, 0);
    input_set_abs_params(icn85xx_ts->input_dev, ABS_MT_POSITION_Y, 0, icn85xx_ts->board_info->screen_max_y, 0, 0);
    input_set_abs_params(icn85xx_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(icn85xx_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(icn85xx_ts->input_dev, ABS_MT_TRACKING_ID, 0, POINT_NUM*2, 0, 0);

    __set_bit(KEY_MENU,  input_dev->keybit);
    __set_bit(KEY_BACK,  input_dev->keybit);
    __set_bit(KEY_HOME,  input_dev->keybit);
    __set_bit(KEY_SEARCH,  input_dev->keybit);

    input_dev->name = CTP_NAME;
    ret = input_register_device(input_dev);
    if (ret) {
        pr_err("Register %s input device failed\n", input_dev->name);
        input_free_device(input_dev);
        return -ENODEV;
    }

#ifdef CONFIG_HAS_EARLYSUSPEND
    pr_info("==register_early_suspend =\n");
    icn85xx_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
    icn85xx_ts->early_suspend.suspend = icn85xx_ts_suspend;
    icn85xx_ts->early_suspend.resume  = icn85xx_ts_resume;
    register_early_suspend(&icn85xx_ts->early_suspend);
#endif

    return 0;
}
#if SUPPORT_SENSOR_ID
static void read_sensor_id(void)
{
        int i,ret;
        ret = icn85xx_read_reg(0x10, &cursensor_id);
        if(ret > 0)
            {
              pr_info("cursensor_id= 0x%x\n", cursensor_id);
            }
        else
            {
              pr_err("icn85xx read cursensor_id failed.\n");
              cursensor_id = -1;
            }

         ret = icn85xx_read_reg(0x1e, &tarsensor_id);
        if(ret > 0)
            {
              pr_info("tarsensor_id= 0x%x\n", tarsensor_id);
              tarsensor_id = -1;
            }
        else
            {
              pr_err("icn85xx read tarsensor_id failed.\n");
            }
         ret = icn85xx_read_reg(0x1f, &id_match);
        if(ret > 0)
            {
              pr_info("match_flag= 0x%x\n", id_match); // 1: match; 0:not match
            }
        else
            {
              pr_err("icn85xx read id_match failed.\n");
              id_match = -1;
            }
   for(i = 0;i < (sizeof(sensor_id_table)/sizeof(sensor_id_table[0])); i++)  // not change tp
    {
        if (cursensor_id == sensor_id_table[i].value)
            {
                #if COMPILE_FW_WITH_DRIVER
                    icn85xx_set_fw(sensor_id_table[i].size, sensor_id_table[i].fw_name);
                #else
                    strcpy(firmware,sensor_id_table[i].bin_name);
                    pr_info("icn85xx matched firmware = %s\n", firmware);
                #endif
                    pr_info("icn85xx matched id = 0x%x\n", sensor_id_table[i].value);
                    invalid_id = 1;
                    break;
            }
        else
            {
              invalid_id = 0;
              pr_info("icn85xx not matched id%d= 0x%x\n", i,sensor_id_table[i].value);
              //pr_info("not match sensor_id_table[%d].value= 0x%x,bin_name = %s\n",i,sensor_id_table[i].value,sensor_id_table[i].bin_name);
            }
     }

}
static void compare_sensor_id(void)
{
   int retry = 5;

   read_sensor_id(); // select sensor id

   if(0 == invalid_id)   //not compare sensor id,update default fw or bin
    {
      pr_info("not compare sensor id table,update default: invalid_id= %d, cursensor_id= %d\n", invalid_id,cursensor_id);
      #if COMPILE_FW_WITH_DRIVER
               icn85xx_set_fw(sensor_id_table[0].size, sensor_id_table[0].fw_name);
      #else
                strcpy(firmware,sensor_id_table[0].bin_name);
                pr_info("match default firmware = %s\n", firmware);
      #endif

      while(retry > 0)
            {
                if(R_OK == icn85xx_fw_update(firmware))
                {
                    pr_info("icn85xx upgrade default firmware ok\n");
                    break;
                }
                retry--;
                pr_err("icn85xx_fw_update default firmware failed.\n");
            }
    }

   if ((1 == invalid_id)&&(0 == id_match))  // tp is changed,update current fw or bin
    {
        pr_info("icn85xx detect tp is changed!!! invalid_id= %d,id_match= %d,\n", invalid_id,id_match);
         while(retry > 0)
            {
                if(R_OK == icn85xx_fw_update(firmware))
                {
                    pr_info("icn85xx upgrade cursensor id firmware ok\n");
                    break;
                }
                retry--;
                pr_err("icn85xx_fw_update current id firmware failed.\n");
            }
    }
}
#endif

static void icn85xx_update(struct icn85xx_ts_data *icn85xx_ts)
{
    short fwVersion = 0;
    short curVersion = 0;
    int retry = 0;

    if((icn85xx_ts->ictype == ICN85XX_WITHOUT_FLASH_85) || (icn85xx_ts->ictype == ICN85XX_WITHOUT_FLASH_86))
    {
        #if (COMPILE_FW_WITH_DRIVER && !SUPPORT_SENSOR_ID)
            icn85xx_set_fw(sizeof(icn85xx_fw), &icn85xx_fw[0]);
        #endif

        #if SUPPORT_SENSOR_ID
        while(0 == invalid_id ) //reselect sensor id
        {
          compare_sensor_id();   // select sensor id
          pr_info("invalid_id= %d\n", invalid_id);
        }
        #else
        if(R_OK == icn85xx_fw_update(firmware))
        {
            icn85xx_ts->code_loaded_flag = 1;
            pr_info("ICN85XX_WITHOUT_FLASH_85, update default fw ok\n");
        }
        else
        {
            icn85xx_ts->code_loaded_flag = 0;
            pr_info("ICN85XX_WITHOUT_FLASH_85, update error\n");
        }
        #endif

    }
    else if((icn85xx_ts->ictype == ICN85XX_WITH_FLASH_85) || (icn85xx_ts->ictype == ICN85XX_WITH_FLASH_86))
    {
        #if (COMPILE_FW_WITH_DRIVER && !SUPPORT_SENSOR_ID)
            icn85xx_set_fw(sizeof(icn85xx_fw), &icn85xx_fw[0]);
        #endif

        #if SUPPORT_SENSOR_ID
        while(0 == invalid_id ) //reselect sensor id
        {
          compare_sensor_id();   // select sensor id
          if( 1 == invalid_id)
            {
              pr_info("select sensor id ok. begin compare fwVersion with curversion\n");
            }
        }
        #endif

        fwVersion = icn85xx_read_fw_Ver(firmware);
        curVersion = icn85xx_readVersion();
        pr_info("fwVersion : 0x%x\n", fwVersion);
        pr_info("current version: 0x%x\n", curVersion);

        #if FORCE_UPDATA_FW
            retry = 5;
            while(retry > 0)
            {
                if(icn85xx_goto_progmode() != 0)
                {
                    pr_info("icn85xx_goto_progmode() != 0 error\n");
                    return ;
                }
                icn85xx_read_flashid();
                if(R_OK == icn85xx_fw_update(firmware))
                {
                    break;
                }
                retry--;
                pr_err("icn85xx_fw_update failed.\n");
            }

        #else
           if(fwVersion > curVersion)
           {
                retry = 5;
                while(retry > 0)
                    {
                        if(R_OK == icn85xx_fw_update(firmware))
                        {
                            break;
                        }
                        retry--;
                        pr_err("icn85xx_fw_update failed.\n");
                    }
           }
        #endif
    }
    else if(icn85xx_ts->ictype == ICN85XX_WITHOUT_FLASH_87)
    {
        pr_info("icn85xx_update 87 without flash\n");
        #if (COMPILE_FW_WITH_DRIVER && !SUPPORT_SENSOR_ID)
            icn85xx_set_fw(sizeof(icn85xx_fw), &icn85xx_fw[0]);
        #endif

        if(R_OK == icn87xx_fw_update(firmware))
        {
            icn85xx_ts->code_loaded_flag = 1;
            pr_info("ICN85XX_WITHOUT_FLASH_87, update default fw ok\n");
        }
        else
        {
            icn85xx_ts->code_loaded_flag = 0;
            pr_info("ICN85XX_WITHOUT_FLASH_87, update error\n");
        }

    }
    else if(icn85xx_ts->ictype == ICN85XX_WITH_FLASH_87)
    {
        pr_info("icn85xx_update 87 with flash\n");

        #if (COMPILE_FW_WITH_DRIVER && !SUPPORT_SENSOR_ID)
            icn85xx_set_fw(sizeof(icn85xx_fw), &icn85xx_fw[0]);
        #endif
        fwVersion = icn85xx_read_fw_Ver(firmware);
        curVersion = icn85xx_readVersion();
        pr_info("fwVersion : 0x%x\n", fwVersion);
        pr_info("current version: 0x%x\n", curVersion);


        #if FORCE_UPDATA_FW
            if(R_OK == icn87xx_fw_update(firmware))
            {
                icn85xx_ts->code_loaded_flag = 1;
                pr_info("ICN87XX_WITH_FLASH, update default fw ok\n");
            }
            else
            {
                icn85xx_ts->code_loaded_flag = 0;
                pr_info("ICN87XX_WITH_FLASH, update error\n");
            }

        #else
            if(fwVersion > curVersion)
            {
                retry = 5;
                while(retry > 0)
                {
                    if(R_OK == icn87xx_fw_update(firmware))
                    {
                        break;
                    }
                    retry--;
                    pr_err("icn87xx_fw_update failed.\n");
                }
            }
        #endif
    }
    else
    {
       //for next ic type
    }
}



static struct tp_board_info tbi = {
	.screen_max_x = 1280,//1024,
	.screen_max_y = 800,//600,
	.revert_x_flag = 0,
	.revert_y_flag = 1,
};

//#define IRQ_PORT			36
//#define REST_PORT			38

static int icn85xx_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{   //printk("######%s----%d\n",__func__,__LINE__);
	struct device_node* np = client->dev.of_node;
    struct icn85xx_ts_data *icn85xx_ts;
    int err = 0;
    pr_info("-----------------icnt ====%s begin=====.\n", __func__);

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        pr_err("I2C check functionality failed.\n");
        return -ENODEV;
    }

    icn85xx_ts = kzalloc(sizeof(*icn85xx_ts), GFP_KERNEL);
    if (!icn85xx_ts)
    {
        pr_err("Alloc icn85xx_ts memory failed.\n");
        return -ENOMEM;
    }
    memset(icn85xx_ts, 0, sizeof(*icn85xx_ts));
	icn85xx_ts->board_info = &tbi;

    this_client = client;
    this_client->addr = client->addr;
    i2c_set_clientdata(client, icn85xx_ts);

	icn85xx_ts->client = client;
	icn85xx_ts->work_mode = 0;
	spin_lock_init(&icn85xx_ts->irq_lock);

	icn85xx_ts->board_info->reset_pin = of_get_named_gpio(np, "chipone_rst", 0);
	icn85xx_ts->board_info->irq_pin = of_get_named_gpio(np, "chipone_int", 0);

	pr_err("+++++++reset_pin=%d, irq_pin=%d\n", icn85xx_ts->board_info->reset_pin, icn85xx_ts->board_info->irq_pin);

	err = gpio_request(icn85xx_ts->board_info->reset_pin, "tp reset");
	if (err) {
        pr_err("icn83xx reset gpio request failed.\n");
		return -1;
	}

    icn85xx_ts_reset();

    err = icn85xx_iic_test();
    if (err <= 0)
    {
        pr_err("icn85xx_iic_test  failed.\n");
       goto err_free_mem;

    }
    else
    {
        pr_info("iic communication ok: 0x%x\n", icn85xx_ts->ictype);
    }

    /* download the firmware  */
    icn85xx_update(icn85xx_ts);

    /* download the firmware  */
    err= icn85xx_request_input_dev(icn85xx_ts);
    if (err < 0)
    {
        pr_err("request input dev failed\n");
        kfree(icn85xx_ts);
        return err;
    }


    icn85xx_request_io_port(icn85xx_ts);


    icn85xx_ts->irq = client->irq;

#if TOUCH_VIRTUAL_KEYS
    icn85xx_ts_virtual_keys_init();
#endif

#if SUPPORT_PROC_FS
	   sema_init(&icn85xx_ts->sem, 1);
	   init_proc_node();
#endif

    INIT_WORK(&icn85xx_ts->pen_event_work, icn85xx_ts_pen_irq_work);
    icn85xx_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
    if (!icn85xx_ts->ts_workqueue) {
        pr_err("create_singlethread_workqueue failed.\n");
        kfree(icn85xx_ts);
        return -ESRCH;
    }


    /*
    ** here we can use interrupts or hrtimer mode.
    ** but we consider interrupts mode first.
    */
    err = icn85xx_request_irq(icn85xx_ts);
    if (err != 0)
    {
        pr_err("request irq error, use timer\n");
        icn85xx_ts->use_irq = 0;
        hrtimer_init(&icn85xx_ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        icn85xx_ts->timer.function = chipone_timer_func;
        hrtimer_start(&icn85xx_ts->timer, ktime_set(CTP_START_TIMER/1000, (CTP_START_TIMER%1000)*1000000), HRTIMER_MODE_REL);
    }

    /* support sysfs  */
#if SUPPORT_SYSFS
   // icn85xx_create_sysfs(client);
#endif
    icn85xx_ts->irq_is_disable = 1;
    if(icn85xx_ts->use_irq)
    {
        icn85xx_irq_enable();
    }

	icn85xx_ts->is_suspend = 0;

    return 0;

err_free_mem:
	//input_free_device(input_dev);
	gpio_free(icn85xx_ts->board_info->reset_pin);
	kfree(icn85xx_ts);
	return err;

}

static int icn85xx_ts_remove(struct i2c_client *client)
{
    struct icn85xx_ts_data *icn85xx_ts = i2c_get_clientdata(client);
    pr_info("==icn85xx_ts_remove=\n");
    if(icn85xx_ts->use_irq)
        icn85xx_irq_disable();


#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&icn85xx_ts->early_suspend);
#endif

#if SUPPORT_PROC_FS
    uninit_proc_node();
#endif

#if SUPPORT_SYSFS
    //icn85xx_remove_sysfs(client);
#endif

    cancel_work_sync(&icn85xx_ts->pen_event_work);
    destroy_workqueue(icn85xx_ts->ts_workqueue);

     input_unregister_device(icn85xx_ts->input_dev);
    input_free_device(icn85xx_ts->input_dev);
    icn85xx_free_io_port(icn85xx_ts);
    kfree(icn85xx_ts);
    i2c_set_clientdata(client, NULL);
	gpio_free(icn85xx_ts->board_info->reset_pin);
    return 0;

}


static const struct of_device_id ak37d_touchscreen_of_ids[] = {
    { .compatible = "anyka,ak3790d-i2c-touchscreen-icn85xx" },
	{},
};
MODULE_DEVICE_TABLE(of, ak37d_touchscreen_of_ids);


static const struct i2c_device_id icn85xx_ts_id[] = {
    { "chipone-icn85xx", 0 },
    {}
};
MODULE_DEVICE_TABLE(i2c, icn85xx_ts_id);

static struct i2c_driver icn85xx_ts_driver = {
    .class      = I2C_CLASS_HWMON,
    .probe      = icn85xx_ts_probe,
    .remove     = icn85xx_ts_remove,
    .id_table   = icn85xx_ts_id,
    .driver = {
        .name   = "icn85xx",
        .of_match_table = of_match_ptr(ak37d_touchscreen_of_ids),
        .owner  = THIS_MODULE,
        #if SUPPORT_SYSFS
          .groups   = icn_drv_grp,
        #endif
    },
};

static int  icn85xx_ts_init(void)
{
    int ret = -1;

    ret = i2c_add_driver(&icn85xx_ts_driver);
    return ret;
}

static void  icn85xx_ts_exit(void)
{
    i2c_del_driver(&icn85xx_ts_driver);
}

module_init(icn85xx_ts_init);
module_exit(icn85xx_ts_exit);

MODULE_AUTHOR("<zhangzhipeng@anyka.com>");
MODULE_DESCRIPTION("Anyka icn85xx TouchScreen controller driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL v2");
