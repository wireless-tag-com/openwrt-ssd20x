obj-m       := pollsample.o
pollsample-objs := src/drv/linux/drv_pollsample_module.o \
                   src/drv/linux/drv_pollsample_export.o \
                   src/drv/common/drv_pollsample_dev.o

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

KERNEL_PATH = /home/david.tsai/I2/linux-3.18
CC  = $(CROSS_COMPILE)gcc
EXTRA_CFLAGS += -I$(KERNEL_PATH)/drivers/sstar/cam_drv_poll/sample_driver/pub/ \
                -I$(KERNEL_PATH)/drivers/sstar/cam_drv_poll/sample_driver/inc \
                -I$(KERNEL_PATH)/drivers/sstar/include/

all:
	make modules -C $(KERNEL_PATH)  M=`pwd`
clean:
	make modules clean -C $(KERNEL_PATH)  M=`pwd`
