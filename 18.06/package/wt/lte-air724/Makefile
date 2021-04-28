#
# Copyright (C)
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lte-air724
PKG_VERSION:=V1.0

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=LTE
  CATEGORY:=LTE
  TITLE:=LTE AIR724
  DEPENDS:=+comgt
endef

define Package/$(PKG_NAME)/description
Provide supporting for LTE AIR724
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults $(1)/etc/init.d
	$(INSTALL_BIN) ./files/lte-air724.default $(1)/etc/uci-defaults/90-lte-air724
	$(INSTALL_BIN) ./files/lte-air724.init $(1)/etc/init.d/lte
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
