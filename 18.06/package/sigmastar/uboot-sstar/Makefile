#
# Copyright (C) 2008-2012 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=uboot-sstar
PKG_VERSION=1.1
PKG_RELEASE:=1-20200901

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(BUILD_VARIANT)

include $(INCLUDE_DIR)/package.mk

UBOOT_MAKE_OPTS:= \
    LZMA=$(STAGING_DIR_HOST)/bin/lzma \
    ARCH=mips \
    CPU=mips \
    CROSS_COMPILE=$(TARGET_CROSS)

WT_BUILD_ARGS:=-DWT_UBOOT -DOPENWRT_UBI

ifdef BUILD_VARIANT
UBOOT_CONFIG:=$(BUILD_VARIANT)
UBOOT_IMAGE:=$(if $(IMAGE),$(IMAGE),wt-$(BOARD)$(if $(SUBTARGET),-$(SUBTARGET))-$(BUILD_VARIANT)-uboot.bin)
WT_BUILD_ARGS+=-D$(BUILD_VARIANT)
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)
endef

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR) $(UBOOT_CONFIG)_defconfig CROSS_COMPILE=$(TARGET_CROSS)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) WT_CFLAGS="$(WT_BUILD_ARGS)"
endef

define Package/uboot-sstar/common
  define Package/uboot-sstar-$(1)
    SECTION:=SigmStar
    CATEGORY:=SigmStar
    SUBMENU:=boot
    TITLE:=U-boot for $(1)
    DEPENDS:=@TARGET_sstar
    URL:=http://www.denx.de/wiki/U-Boot
    VARIANT:=$(1)
  endef
endef

define Package/uboot-nand-sstar/common
  define Package/uboot-nand-sstar-$(1)
    SECTION:=SigmStar
    CATEGORY:=SigmStar
    SUBMENU:=boot
    TITLE:=U-boot for $(1) NAND
    DEPENDS:=@TARGET_sstar
    URL:=http://www.denx.de/wiki/U-Boot
    VARIANT:=$(1)
  endef
endef

define Package/uboot-sstar
  $(call Package/uboot-sstar/common,$(1),$(2))

  define Package/uboot-sstar-$(1)/install
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/u-boot.xz.img.bin $(BIN_DIR)/$(UBOOT_IMAGE)
  endef

  $$(eval $$(call BuildPackage,uboot-sstar-$(1)))
endef

define Package/uboot-nand-sstar
  $(call Package/uboot-nand-sstar/common,$(1),$(2))

  define Package/uboot-nand-sstar-$(1)/install
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/u-boot_spinand.xz.img.bin $(BIN_DIR)/$(UBOOT_IMAGE)
  endef

  $$(eval $$(call BuildPackage,uboot-nand-sstar-$(1)))
endef

$(eval $(call Package/uboot-nand-sstar,WT2011))
$(eval $(call Package/uboot-nand-sstar,WT2022))
