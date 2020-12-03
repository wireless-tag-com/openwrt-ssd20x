#
# Makefile for kernel IR drivers.
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

EXTRA_CFLAGS += -Idrivers/sstar/include
EXTRA_CFLAGS += -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS += -Idrivers/sstar/ir/include

obj-$(CONFIG_MS_IR)  += ms_ir.o
