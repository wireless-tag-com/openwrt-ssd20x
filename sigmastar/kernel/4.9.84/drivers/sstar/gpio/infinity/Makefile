#
# Makefile for MStar GPIO HAL drivers.
#


ifeq ($(MAKE_TYPE), MODULE_STANDALONE)
include $(TOP_DIR)/modules.mk
endif


CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

# general options
# EXTRA_CFLAGS      +=  -Idrivers/sstar/common
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/gpio
EXTRA_CFLAGS      +=  -Idrivers/sstar/gpio/$(CONFIG_SSTAR_CHIP_NAME)

# files
#obj-$(CONFIG_MSTAR_GPIO)             += mhal_gpio.o
