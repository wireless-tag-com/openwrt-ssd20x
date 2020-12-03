#
# Makefile for MStar GMAC device drivers.
#

CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/gmac
EXTRA_CFLAGS      +=  -Idrivers/sstar/gmac/hal/$(CONFIG_MSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iinclude/linux

# specific options
# EXTRA_CFLAGS      +=  -DRED_LION
# files
obj-$(CONFIG_MS_GMAC)             := mdrv-gmac.o


mdrv-gmac-objs    := mdrv_gmac_v3.o drv_mdio_sw.o

mdrv-gmac-objs    += hal/$(CONFIG_MSTAR_CHIP_NAME)/mhal_gmac_v3.o


ccflags-y += -Werror