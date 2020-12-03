CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

obj-$(CONFIG_MS_SPINAND) += kdrv_spinand.o

# general options

#EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/spinand/hal/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/spinand/drv/inc
EXTRA_CFLAGS      +=  -Idrivers/mtd

# specific options
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

ifeq ($(CONFIG_SSTAR_CHIP_NAME), infinity2)
	MHAL_SOURCE = hal/$(CONFIG_SSTAR_CHIP_NAME)/mhal_spinand.o
else
	MHAL_SOURCE = hal/mhal_spinand.o
endif

kdrv_spinand-objs := drv/mdrv_spinand.o \
                     drv/mdrv_spinand_hal.o \
                     $(MHAL_SOURCE) \
                     hal/$(CONFIG_SSTAR_CHIP_NAME)/mhal_spinand_chip_config.o
