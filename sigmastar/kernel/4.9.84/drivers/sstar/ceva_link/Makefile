#
# Makefile for MStar Infinity3 IVE device drivers.

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
#EXTRA_CFLAGS      +=  -Idrivers/mstar/ceva_linkdrv
#EXTRA_CFLAGS      +=  -Idrivers/mstar/include
#EXTRA_CFLAGS      +=  -Idrivers/mstar/gpio     
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/bdma/$(CONFIG_SSTAR_CHIP_NAME)/ 


#ccflags-$(CONFIG_SSTAR_CEVAXM6)		+= -DMSOS_TYPE_LINUX_KERNEL

# files
#obj-$(CONFIG_SSTAR_CEVAXM6) += ceva_link.o
#ceva_link-y += ceva_link.o

#a
#ceva_link-y := ceva_linkdrv_xm6.o   \
#               ceva_linkdrv.o    \
#               protected_mem_db.o \
#               hal_ceva.o \
#               hal_timer.o \
#               hal_cpm.o \
#               boot_loader.o \
#               ceva_linkdrv_proc.o \
#               file_access.o

obj-$(CONFIG_SSTAR_CEVAXM6) += ceva_linkdrv_xm6.o  \
        ceva_linkdrv.o    \
        protected_mem_db.o \
        hal_ceva.o \
        hal_timer.o \
        hal_cpm.o \
        boot_loader.o \
        ceva_linkdrv_proc.o \
        file_access.o