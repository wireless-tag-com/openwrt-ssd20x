#
# Makefile for SStar cam_os_wrapper driver.
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/cam_os_wrapper
EXTRA_CFLAGS      +=  -Idrivers/sstar/cam_os_wrapper/pub
EXTRA_CFLAGS      +=  -Wno-sync-nand

# specific options

# files
obj-y += src/cam_os_wrapper.o src/cam_os_export.o src/cam_os_informal_idr.o

# export header files
EXPORT_H_FILES    += cam_os_wrapper.h
