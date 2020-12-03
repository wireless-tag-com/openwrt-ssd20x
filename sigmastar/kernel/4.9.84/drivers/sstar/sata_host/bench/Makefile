
KDIR=${PWD}/../../../../

EXTRA_CFLAGS      +=  -I$(KDIR)/include/linux
EXTRA_CFLAGS      +=  -I$(KDIR)/drivers/ata
EXTRA_CFLAGS      +=  -I$(KDIR)/drivers/sstar/include
EXTRA_CFLAGS      +=   -Wno-pointer-to-int-cast -Wno-unused-variable -Wno-unused-function -Wno-return-type

obj-m +=  sata_bench.o 

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KDIR) SUBDIRS=$(shell pwd) modules

clean:
	rm -rf *.ko module* Module*
