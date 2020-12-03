#
# Makefile for MStar mtlb device drivers.
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
ifdef CONFIG_MSTAR_PROJECT_NAME
    CONFIG_MSTAR_PROJECT_NAME := $(subst ",,$(CONFIG_MSTAR_PROJECT_NAME))
endif

# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/miu
EXTRA_CFLAGS      +=  -Iinclude/linux
# specific options
EXTRA_CFLAGS      +=


# files
obj-$(CONFIG_MSTAR_MIU)             := mdrv-miu.o

mdrv-miu-objs    := $(CONFIG_SSTAR_CHIP_NAME)/mdrv_miu.o
mdrv-miu-objs    += $(CONFIG_SSTAR_CHIP_NAME)/mhal_miu.o

