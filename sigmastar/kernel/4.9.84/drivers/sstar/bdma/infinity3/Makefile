#
# Makefile for the kernel BDMA drivers.
#
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

EXTRA_CFLAGS += -Idrivers/sstar/include
EXTRA_CFLAGS += -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS += -Iinclude/linux

obj-y = hal_bdma.o
