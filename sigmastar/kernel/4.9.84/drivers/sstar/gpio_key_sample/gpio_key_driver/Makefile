#
# Makefile for the Linux network IOT device drivers.
#

KERNEL_DIR ?=../kernel
PWD := $(shell pwd)
MODULE_NAME = gpio_key_driver

#ifdef CONFIG_MSTAR_CHIP_NAME
#EXTRA_CFLAGS      +=  -I$(KERNEL_DIR)/drivers/mstar/include/$(CONFIG_MSTAR_CHIP_NAME)
#endif

#ifdef CONFIG_SSTAR_CHIP_NAME
#EXTRA_CFLAGS      +=  -I$(KERNEL_DIR)/drivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
#endif



obj-m += $(MODULE_NAME).o
obj-$(MODULE_NAME) += gpio_key_driver.c

modules:
	make -C $(KERNEL_DIR) M=$(PWD) modules
	
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

.PHONY: modules 

