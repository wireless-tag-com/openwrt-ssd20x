#
# Makefile for SStar cam_os_wrapper driver.
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/cam_fs_wrapper
EXTRA_CFLAGS      +=  -Idrivers/sstar/cam_fs_wrapper/pub


# specific options

# files
obj-y += src/cam_fs_wrapper.o src/cam_fs_export.o

# export header files
EXPORT_H_FILES    += cam_fs_wrapper.h
