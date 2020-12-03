#
# Makefile for MStar EMAC device drivers.
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/emac
EXTRA_CFLAGS      +=  -Idrivers/sstar/emac/hal/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iinclude/linux
EXTRA_CFLAGS      +=  -Idrivers/sstar/bdma/

EXTRA_CFLAGS      +=  -DKERNEL_PHY=1

# specific options
# EXTRA_CFLAGS      +=  -DRED_LION
# files
obj-$(CONFIG_MS_EMAC)             := kdrv_emac.o
kdrv_emac-y    := mdrv_emac.o
kdrv_emac-y    += hal/$(CONFIG_SSTAR_CHIP_NAME)/mhal_emac.o