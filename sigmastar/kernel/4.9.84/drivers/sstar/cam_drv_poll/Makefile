#
# Makefile for MStar cam_drv_poll driver.
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
# EXTRA_CFLAGS      +=  -Idrivers/sstar/common
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/cam_drv_poll/pub


# specific options

# files
obj-y += src/cam_drv_poll.o

# export header files
EXPORT_H_FILES    += cam_drv_poll.h
