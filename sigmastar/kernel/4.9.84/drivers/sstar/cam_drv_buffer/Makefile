#
# Makefile for MStar cam_drv_buffer driver.
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
# EXTRA_CFLAGS      +=  -Idrivers/sstar/common
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/cam_drv_buffer
EXTRA_CFLAGS      +=  -Idrivers/sstar/cam_drv_buffer/pub


# specific options

# files
obj-y += src/cam_drv_buffer.o src/cam_drv_export.o

# export header files
EXPORT_H_FILES    += cam_drv_buffer.h
