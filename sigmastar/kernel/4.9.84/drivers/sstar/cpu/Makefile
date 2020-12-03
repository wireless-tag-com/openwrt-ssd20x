#
# Makefile for CPU LX size   drivers.
#
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/cpu
EXTRA_CFLAGS += -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
# files
obj-y     += mem-ory.o
mem-ory-objs                       += memory.o
