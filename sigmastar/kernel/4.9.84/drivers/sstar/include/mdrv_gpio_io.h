/*
* mdrv_gpio_io.h- Sigmastar
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
#include <asm/types.h>
#include "mdrv_types.h"
#include <linux/gpio.h>
//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
struct GPIO_Reg
{
    U32 u32Reg;
    U8 u8Enable;
    U8 u8BitMsk;
} __attribute__ ((packed));

typedef struct GPIO_Reg GPIO_Reg_t;

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define IOCTL_GPIO_INIT_NR                              (0)
#define IOCTL_GPIO_SET_NR                               (1)
#define IOCTL_GPIO_OEN_NR                               (2)
#define IOCTL_GPIO_ODN_NR                               (3)
#define IOCTL_GPIO_READ_NR                              (4)
#define IOCTL_GPIO_PULL_HIGH_NR                        (5)
#define IOCTL_GPIO_PULL_LOW_NR                         (6)
#define IOCTL_GPIO_INOUT_NR                            (7)
#define IOCTL_GPIO_WREGB_NR                            (8)

#define GPIO_IOC_MAGIC               'g'
#define IOCTL_GPIO_INIT               _IO(GPIO_IOC_MAGIC, IOCTL_GPIO_INIT_NR)
#define IOCTL_GPIO_SET                _IO(GPIO_IOC_MAGIC, IOCTL_GPIO_SET_NR)
#define IOCTL_GPIO_OEN                _IO(GPIO_IOC_MAGIC, IOCTL_GPIO_OEN_NR)
#define IOCTL_GPIO_ODN                _IO(GPIO_IOC_MAGIC, IOCTL_GPIO_ODN_NR)
#define IOCTL_GPIO_READ               _IO(GPIO_IOC_MAGIC, IOCTL_GPIO_READ_NR)
#define IOCTL_GPIO_PULL_HIGH          _IO(GPIO_IOC_MAGIC, IOCTL_GPIO_PULL_HIGH_NR)
#define IOCTL_GPIO_PULL_LOW           _IO(GPIO_IOC_MAGIC, IOCTL_GPIO_PULL_LOW_NR)
#define IOCTL_GPIO_INOUT              _IO(GPIO_IOC_MAGIC, IOCTL_GPIO_INOUT_NR)
#define IOCTL_GPIO_WREGB              _IO(GPIO_IOC_MAGIC, IOCTL_GPIO_WREGB_NR)

#define GPIO_IOC_MAXNR               9

//for rename use
//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void camdriver_gpio_set(struct gpio_chip *chip, unsigned offset, int value);
int camdriver_gpio_get(struct gpio_chip *chip, unsigned offset);
int camdriver_gpio_direction_input(struct gpio_chip *chip, unsigned offset);
int camdriver_gpio_request(struct gpio_chip *chip, unsigned offset);
int camdriver_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
                    int value);
int camdriver_gpio_to_irq(struct gpio_chip *chip, unsigned offset);

void __mod_gpio_init(void);
