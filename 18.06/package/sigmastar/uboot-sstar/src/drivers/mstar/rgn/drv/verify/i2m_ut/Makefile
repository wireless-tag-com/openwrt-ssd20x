#
# Makefile for kernel test
#
PWD         ?= $(shell pwd)
DIR_PATH    := $(PWD)/../../../
KVERSION    := $(shell uname -r)
#KERNEL_DIR   = $(PWD)/../../../../project/kbuild/3.18.30/
#KERNEL_DIR = $(PWD)/../../../../../../kernel/linux-4.9

MODULE_NAME  = MhalGop

SRC_PATH := $(DIR_PATH)

CHIP_DIR=infinity2m

KERNEL_DIR = $(PWD)/../../../../../../../../../kernel/linux-4.9

INC_PATH := -I$(DIR_PATH)
INC_PATH += -I$(DIR_PATH)/../include
INC_PATH += -I$(DIR_PATH)/../common
INC_PATH += -I$(KERNEL_DIR)/drivers/sstar/include
INC_PATH += -I$(KERNEL_DIR)/drivers/sstar/include/$(CHIP_DIR)


INC_PATH += -I$(DIR_PATH)//drv/inc
INC_PATH += -I$(DIR_PATH)//drv/inc/linux
INC_PATH += -I$(DIR_PATH)//drv/pub
INC_PATH += -I$(DIR_PATH)//hal/$(CHIP_DIR)/inc
INC_PATH += -I$(DIR_PATH)//hal/$(CHIP_DIR)
INC_PATH += -I$(DIR_PATH)//drv/verify/i2m_ut


EXTRA_CFLAGS += -DRGNOS_TYPE_LINUX_KERNEL -DGOP_OS_TYPE_LINUX_KERNEL_TEST
EXTRA_CFLAGS += $(INC_PATH)

obj-m       := $(MODULE_NAME).o

$(MODULE_NAME)-objs += .//drv_gop_module.o
$(MODULE_NAME)-objs += .//gop_sysfs.o
$(MODULE_NAME)-objs += .//gop_ut.o
$(MODULE_NAME)-objs += .//../../../hal/$(CHIP_DIR)/src/hal_gop.o
$(MODULE_NAME)-objs += .//../../../hal/$(CHIP_DIR)/src/hal_cover.o
$(MODULE_NAME)-objs += .//../../../hal/$(CHIP_DIR)/src/hal_osd.o
$(MODULE_NAME)-objs += .//../../../hal/$(CHIP_DIR)/src/hal_rgn_common.o
$(MODULE_NAME)-objs += .//../../../drv/src/drv_gop.o
$(MODULE_NAME)-objs += .//../../../drv/src/drv_cover.o
$(MODULE_NAME)-objs += .//../../../drv/src/drv_osd.o
$(MODULE_NAME)-objs += .//../../../drv/src/drv_rgn_common.o
$(MODULE_NAME)-objs += .//../../../drv/src/mhal_rgn.o
$(MODULE_NAME)-objs += .//../../../drv/src/linux/drv_rgn_os.o
$(MODULE_NAME)-objs += .//../../../drv/src/linux/rgn_proc.o

all:
	@echo EXTRA_CFLAGS=$(EXTRA_CFLAGS)
	@echo KERNEL_DIR = $(KERNEL_DIR)
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
