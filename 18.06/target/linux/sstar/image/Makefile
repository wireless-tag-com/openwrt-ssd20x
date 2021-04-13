#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

UBIFS_OPTS := -m 2048 -e 124KiB -c 4096 -U -F
UBI_OPTS := -m 2048 -p 128KiB

define Image/Prepare
	cp $(LINUX_DIR)/./arch/arm/boot/uImage.xz $(KDIR)/uImage.xz
endef

define Image/BuildKernel
	cp $(KDIR)/uImage.xz $(BIN_DIR)/wt-$(BOARD)-uImage.xz
ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
	cp $(KDIR)/uImage.xz $(BIN_DIR)/wt-$(BOARD)-uImage-initramfs.bin
endif
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

define Image/Build/squashfs
	cp $(KDIR)/root.squashfs $(KDIR)/root.squashfs-raw
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/wt-$(BOARD)-$(1).img bs=64k conv=sync
endef

# $(1): ubinize ini file
# $(2): working directory
# $(3): output file
# $(4): physical erase block size
# $(5): minimum I/O unit size
# $(6): custom options
define ubinize
	$(CP) $(1) $(2)
	( cd $(2); $(STAGING_DIR_HOST)/bin/ubinize -o $(3) -p $(4) -m $(5) $(6) $(1))
endef

# $(1): board name
# $(2): root image
# $(3): kernel image
# $(4): compat string
define Image/Build/WTSysupgradeNAND
	mkdir -p "$(KDIR_TMP)/sysupgrade-$(if $(4),$(4),$(1))/"
	echo "BOARD=$(if $(4),$(4),$(1))" > "$(KDIR_TMP)/sysupgrade-$(if $(4),$(4),$(1))/CONTROL"
	$(CP) "$(2)" "$(KDIR_TMP)/sysupgrade-$(if $(4),$(4),$(1))/root"
	(cd "$(KDIR_TMP)"; $(TAR) czvf \
		"$(BIN_DIR)/$(1)/$(1)-sysupgrade.bin" sysupgrade-$(if $(4),$(4),$(1)) \
	)
endef

define Image/Build/WTNAND
	mkdir -p $(KDIR_TMP)
	mkdir -p $(BIN_DIR)/$(1)
	$(CP) $(KDIR)/uImage.xz $(KDIR_TMP)/uImage.xz
	$(CP) $(KDIR)/uImage.xz $(KDIR_TMP)/$(1)-uImage.xz
	$(CP) $(KDIR)/uImage.xz $(BIN_DIR)/$(1)/$(1)-uImage.xz
	$(CP) $(KDIR)/root.squashfs-raw $(KDIR_TMP)/root.squashfs
	$(call ubinize,ubinize-sstar.cfg,$(KDIR_TMP),$(KDIR_TMP)/$(1)-root.ubi,128KiB,2048,-E 5)
	$(CP) $(KDIR_TMP)/$(1)-root.ubi $(BIN_DIR)/$(1)/$(1)-root-ubi.img
	$(call Image/Build/WTSysupgradeNAND,$(1),$(KDIR_TMP)/$(1)-root.ubi)
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

define Image/Build/Profile/WT2011
	$(call Image/Build/WTNAND,WT2011)
endef

define Image/Build/Profile/WT2022
	$(call Image/Build/WTNAND,WT2022)
endef

define Image/Build/Profile/WT2020
	$(call Image/Build/WTNAND,WT2020)
endef

define Image/Build/Profile
	$(call Image/Build/Profile/$(1))
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
	$(call Image/Build/Profile,$(PROFILE))
endef

$(eval $(call BuildImage))
 
