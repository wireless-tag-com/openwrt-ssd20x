#
# Makefile for MStar padmux device drivers.
#
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/padmux
EXTRA_CFLAGS      +=  -Idrivers/sstar/padmux/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-$(CONFIG_MS_GPIO) += mdrv_padmux.o
# obj-$(CONFIG_MS_GPIO) += $(CONFIG_SSTAR_CHIP_NAME)/mhal_gpio.o
# obj-$(CONFIG_MS_GPIO) += $(CONFIG_SSTAR_CHIP_NAME)/mhal_pinmux.o
# obj-$(CONFIG_MS_SW_I2C) += ms_gpioi2c.o mdrv_sw_iic.o
