# Copyright (c) 2013 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=sstar
BOARDNAME:=Sigmastar SSD2XX
SUBTAGETS:=ssd20x
FEATURES:=ubifs squashfs gpio fpu nand
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon
CFLAGS:=-Os -pipe -Wno-implicit-fallthrough -mfpu=neon -Wno-format-overflow

LINUX_VERSION:=4.9.84

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
