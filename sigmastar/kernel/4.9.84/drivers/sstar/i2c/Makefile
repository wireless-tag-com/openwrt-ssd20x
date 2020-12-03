#
# Makefile for MStar IIC device drivers.
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
# EXTRA_CFLAGS      +=  -Idrivers/sstar/common
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/i2c
EXTRA_CFLAGS      +=  -Idrivers/sstar/i2c/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)


# specific options
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-$(CONFIG_MS_I2C) += ms_iic.o $(CONFIG_SSTAR_CHIP_NAME)/mhal_iic.o

# export header files
EXPORT_H_FILES    += mdrv_iic_io.h
