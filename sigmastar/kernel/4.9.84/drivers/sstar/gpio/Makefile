#
# Makefile for MStar GPIO device drivers.
#
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/gpio
EXTRA_CFLAGS      +=  -Idrivers/sstar/gpio/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-$(CONFIG_MS_GPIO) += mdrv_gpio.o mdrv_gpio_io.o
obj-$(CONFIG_MS_GPIO) += $(CONFIG_SSTAR_CHIP_NAME)/
obj-$(CONFIG_MS_SW_I2C) += ms_gpioi2c.o mdrv_sw_iic.o
