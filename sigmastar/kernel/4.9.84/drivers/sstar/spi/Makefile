#
# Makefile for kernel SPI drivers.
#
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

ccflags-$(CONFIG_SPI_DEBUG) := -DDEBUG

# SPI master controller drivers (bus)
obj-$(CONFIG_MS_SPI_INFINITY) += $(CONFIG_SSTAR_CHIP_NAME)/
