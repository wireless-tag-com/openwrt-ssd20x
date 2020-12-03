#
# Makefile for SStar NOE device drivers.
#


CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/noe/drv/eth
EXTRA_CFLAGS      +=  -Idrivers/sstar/noe/hal/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iinclude/linux


EXTRA_CFLAGS      += -DMDRV_NOE_DBG_SKB_DUMP

obj-$(CONFIG_NOE) += mdrv-noe.o
mdrv-noe-objs :=  mdrv_noe_mac.o mdrv_noe_phy.o mdrv_noe_log.o mdrv_noe_proc.o mdrv_noe.o mdrv_noe_pdma.o mdrv_noe_utils.o


ifeq ($(CONFIG_NOE_QDMA),y)
mdrv-noe-objs += mdrv_noe_qdma.o
endif

ifeq ($(CONFIG_NOE_ETHTOOL),y)
mdrv-noe-objs += mdrv_noe_ethtool.o
endif

mdrv-noe-objs += mdrv_noe_lro.o
mdrv-noe-objs += mdrv_noe_lro_proc.o



ifeq ($(CONFIG_NOE_NAT_HW),y)
EXTRA_CFLAGS      +=  -Idrivers/sstar/noe/drv/nat/hw_nat
endif

# specific options
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

ccflags-y += -Werror

mdrv-noe-objs    += ../../hal/$(CONFIG_SSTAR_CHIP_NAME)/mhal_noe.o

