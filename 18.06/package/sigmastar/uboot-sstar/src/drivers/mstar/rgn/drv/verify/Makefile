#
# Makefile for MStar LDC kernel mode unit test.

obj-m       := rgn_kernel_ut.o
rgn_kernel_ut-objs := rgn_ut.o rgn_file_access.o

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

KERNEL = ../../../../../kernel/linux-4.9
CC  = $(CROSS_COMPILE)gcc
APP = rgn_kernel_ut

# general options
EXTRA_CFLAGS += -I$(PWD)/../pub
EXTRA_CFLAGS += -I$(PWD)/../../../gop/drv/pub
EXTRA_CFLAGS += -I$(PWD)/../../../cover/drv/pub
EXTRA_CFLAGS += -I$(PWD)/$(KERNEL)/drivers/sstar/include
EXTRA_CFLAGS += -I$(PWD)/$(KERNEL)/drivers/sstar/camdriver/common

all:
	make modules -C $(KERNEL)  M=`pwd`

clean:
	make modules clean -C $(KERNEL)  M=`pwd`
	rm -rf $(APP)
