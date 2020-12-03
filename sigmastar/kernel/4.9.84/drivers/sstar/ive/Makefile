#
# Makefile for MStar Infinity3 IVE device drivers.

IVE_CHIP_DIR  := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/ive
EXTRA_CFLAGS      +=  -Idrivers/sstar/include

ccflags-$(CONFIG_MS_IVE)		+= -DMSOS_TYPE_LINUX_KERNEL

ifeq ($(CONFIG_SSTAR_CHIP_NAME), "infinity2")
    EXTRA_CFLAGS    += -DCONFIG_CHIP_INFINITY2
endif

# files
obj-$(CONFIG_MS_IVE) += mstar_ive.o
mstar_ive-y := mdrv_ive.o   \
               hal_ive.o    \
               hal_clk.o    \
               drv_ive.o    \
			   hal_ive_simulate.o
