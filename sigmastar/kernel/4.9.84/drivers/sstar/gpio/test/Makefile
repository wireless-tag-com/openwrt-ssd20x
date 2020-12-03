obj-m       := gpio_irq.o
gpio_irq-objs := gpio_irq_test.o

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

KERNEL = $(PWD)/../../../../
CC  = $(CROSS_COMPILE)gcc
EXTRA_CFLAGS := -I$(KERNEL)/drivers/sstar/include/ -I$(KERNEL)/include/

all:
	make modules -C $(KERNEL)  M=`pwd`

clean:
	make modules clean -C $(KERNEL)  M=`pwd`
