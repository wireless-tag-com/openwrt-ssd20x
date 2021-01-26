#
# Makefile for kernel test
#
PWD         ?= $(shell pwd)
DIR_PATH    := $(PWD)/../../../
KVERSION    := $(shell uname -r)
#KERNEL_DIR   = $(PWD)/../../../../project/kbuild/3.18.30/
#KERNEL_DIR = $(PWD)/../../../../../../kernel/linux-4.9

MODULE_NAME  = rgn_ut

SRC_PATH := $(DIR_PATH)

CHIP_DIR=infinity6e

KERNEL_DIR ?= $(PWD)/../../../../../../../../../kernel/linux-4.9

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
INC_PATH += -I$(DIR_PATH)//drv/verify/i6e_ut


EXTRA_CFLAGS += $(INC_PATH)

obj-m       := $(MODULE_NAME).o

$(MODULE_NAME)-objs += .//rgn_ut_module.o
$(MODULE_NAME)-objs += .//rgn_ut_I6e.o
$(MODULE_NAME)-objs += .//rgn_file_access.o
$(MODULE_NAME)-objs += .//rgn_ut_script.o

all:
	@echo EXTRA_CFLAGS=$(EXTRA_CFLAGS)
	@echo KERNEL_DIR = $(KERNEL_DIR)
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
