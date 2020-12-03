/*
* drv_mdio_sw.c- Sigmastar
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
#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/completion.h>
#include<linux/param.h>
#include<linux/gpio.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/slab.h>
#include<asm/uaccess.h>
#include<linux/delay.h>
#include<linux/miscdevice.h>
#include"mdrv_types.h"
#include"gpio.h"

/* bb:bit-bang, Use software to control serial communication at general-purpose I/O pins*/
//#define MDC              PAD_SNR0_GPIO0  /* MDC  GPIO PAD */
//#define MDIO             PAD_SNR0_GPIO1  /* MDIO GPIO PAD */
#define MDIO_DELAY       250
#define MDIO_READ_DELAY  350

/*  Or MII_ADDR_C45 into regnum for read/write on mii_bus to enable the 21 bit
 *   IEEE 802.3ae clause 45 addressing mode used by 10GIGE phy chips.
 *   */
#define MII_ADDR_C45    (1<<30)

#define MDIO_C45        (1<<15)
#define MDIO_C45_ADDR   (MDIO_C45 | 0)
#define MDIO_C45_READ   (MDIO_C45 | 3)
#define MDIO_C45_WRITE  (MDIO_C45 | 1)

#define MDIO_READ        2
#define MDIO_WRITE       1

//#define MDIO_SETUP_TIME  10
//#define MDIO_HOLD_TIME   10

//#define READ_REG       0x37
//#define WRITE_REG      0x38

extern void MDrv_GPIO_Pad_Oen(U16 u16IndexGPIO);
extern void MDrv_GPIO_Pad_Odn(U16 u16IndexGPIO);
extern void MDrv_GPIO_Pull_High(U16 u16IndexGPIO);
extern void MDrv_GPIO_Pull_Low(U16 u16IndexGPIO);
extern U8   MDrv_GPIO_Pad_Read(U16 u16IndexGPIO);

#define MDIO_C45_TEST 0



typedef struct gpio_ctrl_blk{
    int pin;
    int value;
}gpio_cblk_t;

typedef struct phy_reg_blk{
    unsigned int phy_address;
    unsigned int reg_address;
    unsigned int reg_value;
}phy_reg_cblk_t;

gpio_cblk_t gpio_mdc_dev;
gpio_cblk_t gpio_mdio_dev;

#define MDIO_DEV_ID 't'
#define READ_REG             _IOWR (MDIO_DEV_ID,0x37,phy_reg_cblk_t)
#define WRITE_REG            _IOWR (MDIO_DEV_ID,0x38,phy_reg_cblk_t)

static void MDC_OUT(void);
static void MDIO_OUT(void);
static void MDIO_IN(void);
static void MDC_H(void);
static void MDC_L(void);
static int  GET_MDIO(void);
static void SET_MDIO(int val);


/* Set MDC as an output pin, set before the MDC output clock */
static void MDC_OUT(void)
{
    //gpio_cblk_t gpio_dev;
    //gpio_dev.pin = MDC;
    MDrv_GPIO_Pad_Oen(gpio_mdc_dev.pin);
}

/* Set MDIO's gpio pin as an output pin */
static void MDIO_OUT(void)
{
    //gpio_cblk_t gpio_dev;
    //gpio_dev.pin = MDIO;
    MDrv_GPIO_Pad_Oen(gpio_mdio_dev.pin);
}

/* Set the gpio pin of MDIO as an input pin */
static void MDIO_IN(void)
{
    //gpio_cblk_t gpio_dev;
    //gpio_dev.pin = MDIO;
    MDrv_GPIO_Pad_Odn(gpio_mdio_dev.pin);
}

/* MDC output high level, called after MDC is set to output */
static void MDC_H(void)
{
    //gpio_cblk_t gpio_dev;

    //gpio_dev.pin = MDC;
    //gpio_dev.value = 1;
    MDrv_GPIO_Pull_High(gpio_mdc_dev.pin);
}

/* MDC output low level, called after MDC is set to output */
static void MDC_L(void)
{
    //gpio_cblk_t gpio_dev;

    //gpio_dev.pin = MDC;
    //gpio_dev.value = 0;
    MDrv_GPIO_Pull_Low(gpio_mdc_dev.pin);
}

/* Get MDIO data and get only one bit */
static int GET_MDIO(void)
{
    //gpio_cblk_t gpio_dev;

    //gpio_dev.pin = MDIO;
    gpio_mdio_dev.value = MDrv_GPIO_Pad_Read(gpio_mdio_dev.pin);

    return gpio_mdio_dev.value;
}

/* Set MDIO data, one bit*/
static void SET_MDIO(int val)
{
    //gpio_cblk_t gpio_dev;

    //gpio_dev.pin = MDIO;
    //gpio_dev.value = val;
    if(val == 0)
        MDrv_GPIO_Pull_Low(gpio_mdio_dev.pin);
    else
        MDrv_GPIO_Pull_High(gpio_mdio_dev.pin);
}


/* MDIO sends a bit of data, MDIO must have been configured as output */
static void mdio_bb_send_bit(int val)
{
    MDC_OUT();
    SET_MDIO(val);
    ndelay(MDIO_DELAY);
    MDC_L();
    ndelay(MDIO_DELAY);
    MDC_H();
}

/*  MDIO gets a bit of data, MDIO must have been configured as input. */
static int mdio_bb_get_bit(void)
{
    int value;

    MDC_OUT();
    ndelay(MDIO_DELAY);
    MDC_L();
    ndelay(MDIO_READ_DELAY);
//    ndelay(MDIO_DELAY);
    MDC_H();

    value = GET_MDIO();

    return value;
}

 /*
  *  MDIO sends a data and MDIO must be configured as an output mode.
  *  */
static void mdio_bb_send_num(unsigned int value ,int bits)
{
    int i;
    MDIO_OUT();

    for(i = bits - 1; i >= 0; i--)
        mdio_bb_send_bit((value >> i) & 1);
}

 /*
  *  MDIO gets one data, MDIO must be configured as input mode.
  *  */
static int mdio_bb_get_num(int bits)
{
    int i;
    int ret = 0;
    for(i = bits - 1; i >= 0; i--)
    {
        ret <<= 1;
        ret |= mdio_bb_get_bit();
    }

    return ret;
}



/*  Utility to send the preamble, address, and
*   register (common to read and write).
*/
static void mdio_bb_cmd(int op,int phy,int reg)
{
    int i = 0 ;
    MDIO_OUT();  //Set the MDIO pin as an output pin

    /*Send 32bit 1, this frame prefix field is not required, MDIO operation of some physical layer chips does not have this domain*/
    for(i = 0; i < 32; i++)
        mdio_bb_send_bit(1);


    /* Send start bit (01), and read opcode (10), write opcode (01)
     * Clause 45 operation, the starting bit is (00), (11) is read, (10) is written
    */
#if MDIO_C45_TEST
    mdio_bb_send_bit(0);
    if(op & MDIO_C45)
        mdio_bb_send_bit(0);
    else
        mdio_bb_send_bit(1);
#else
    mdio_bb_send_bit(0);
    mdio_bb_send_bit(1);

#endif

    mdio_bb_send_bit((op >> 1) & 1);
    mdio_bb_send_bit((op >> 0) & 1);

    mdio_bb_send_num(phy,5);
    mdio_bb_send_num(reg,5);

}

#if MDIO_C45_TEST
static int mdio_bb_cmd_addr(int phy,int addr)
{
    unsigned int dev_addr = (addr >> 16) & 0x1F;
    unsigned int reg = addr & 0xFFFF;

    mdio_bb_cmd(MDIO_C45_ADDR,phy,dev_addr);

    /*  send the turnaround (10) */
    mdio_bb_send_bit(1);
    mdio_bb_send_bit(0);

    mdio_bb_send_num(reg,16);

    MDIO_IN();
    mdio_bb_get_bit();

    return dev_addr;
}
#endif

void mdio_set_turnaround(void)
{
    int i = 0;

    MDIO_IN();
    MDC_OUT();
    for(i=0;i<2;i++)
    {
        ndelay(MDIO_DELAY);
        MDC_L();
        ndelay(MDIO_DELAY);
        MDC_H();
    }
}

unsigned int mdio_bb_read(int phy,int reg)
{
    unsigned int ret,i;

#if MDIO_C45_TEST
    /* Whether the register is satisfied with the C45 flag */
    if(reg & MII_ADDR_C45)
    {
        reg = mdio_bb_cmd_addr(phy,reg);
        mdio_bb_cmd(MDIO_C45_READ,phy,reg);
    }
    else
        mdio_bb_cmd(MDIO_READ,phy,reg);
#else
        mdio_bb_cmd(MDIO_READ,phy,reg);
#endif
    MDIO_IN();
    //mdio_set_turnaround();
    /*  check the turnaround bit: the PHY should be driving it to zero */
    if(mdio_bb_get_bit() != 0)
    {
        /* PHY didn't driver TA low -- flush any bits it may be trying to send*/
        for(i = 0; i < 32; i++)
            mdio_bb_get_bit();
        return 0xFFFF;
    }

    ret = mdio_bb_get_num(16);
    mdio_bb_get_bit();

    return ret;
}

unsigned int mdio_bb_write(unsigned int phy,unsigned int reg,unsigned int val)
{
#if MDIO_C45_TEST
    if(reg & MII_ADDR_C45)
    {
        reg = mdio_bb_cmd_addr(phy,reg);
        mdio_bb_cmd(MDIO_C45_WRITE,phy,reg);
    }
    else
        mdio_bb_cmd(MDIO_WRITE,phy,reg);
#else
        mdio_bb_cmd(MDIO_WRITE,phy,reg);
#endif

#if 1
    /*  send the turnaround (10) */
    mdio_bb_send_bit(1);
    mdio_bb_send_bit(0);
#else
    mdio_set_turnaround();
#endif
    mdio_bb_send_num(val,16);

    MDIO_IN();
    //mdio_bb_get_bit();

    return 0;
}

void mdio_bb_init(int pin_mdio, int pin_mdc)
{
    gpio_mdio_dev.pin = pin_mdio;
    gpio_mdc_dev.pin = pin_mdc;
}
/*
static int mdio_drv_open(struct inode *inode, struct file *file )
{
    return 0;
}

static int mdio_drv_release(struct inode *inode, struct file *file )
{
    return 0;
}

static long mdio_drv_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    phy_reg_cblk_t phy_reg;
    int ret = 0;

     void __user *argp = (void __user *)arg;
     if( argp==NULL )
     {
         return -EFAULT;
     }

    if (copy_from_user(&phy_reg, argp, sizeof(phy_reg_cblk_t))) {
                return -EFAULT;
            }

    switch (cmd) {
    case READ_REG:
            phy_reg.reg_value = mdio_bb_read(phy_reg.phy_address,phy_reg.reg_address);
            if(copy_to_user(argp,&phy_reg,sizeof(phy_reg_cblk_t)))
            {
                return -EFAULT;
            }
            break;
    case WRITE_REG:
            ret = mdio_bb_write(phy_reg.phy_address,phy_reg.reg_address,phy_reg.reg_value);
    default:
        return -EINVAL;

    }

    return 0;
}

static struct file_operations mdio_drv_fileops = {
    .owner = THIS_MODULE,
    .open = mdio_drv_open,
    .unlocked_ioctl = mdio_drv_unlocked_ioctl,
    .release = mdio_drv_release
};

static struct miscdevice mdio_dev = {
    MISC_DYNAMIC_MINOR,
    "mdio_dev",
    &mdio_drv_fileops,
};

int mdio_drv_init(void)
{
    int ret = 0;

    ret = misc_register(&mdio_dev);
    if(ret != 0)
    {
        ret = -EFAULT;
        return ret;
    }
    printk("mdio_drv_init ok\n");
    return 0;
}

void mdio_drv_exit(void)
{
    misc_deregister(&mdio_dev);
    printk("mdio_drv_exit ok\n");
}

module_init(mdio_drv_init);
module_exit(mdio_drv_exit);
MODULE_LICENSE("GPL");*/