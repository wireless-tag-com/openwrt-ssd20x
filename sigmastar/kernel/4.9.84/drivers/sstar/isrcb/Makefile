#
# Makefile for interrupt router driver
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/cam_os_wrapper/pub
EXTRA_CFLAGS      +=  -Idrivers/sstar/isrcb/drv/pub

# files
obj-$(CONFIG_SS_ISP_ISRCB) := isrcb.o
isrcb-y := drv/src/common/drv_isrcb.o\
           drv/src/linux/drv_isrcb_module.o
