#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tinyalsa
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=SigmStar
  CATEGORY:=SigmStar
  SUBMENU:=utils
  DEPENDS:=+libpthread
  PKGARCH:=all
  TITLE:=SSD201/SD202 tinyalsa
endef

define Package/$(PKG_NAME)/description
 SSD20X tinyalsa
endef

MAKE_FLAGS += \
        CFLAGS="$(TARGET_CFLAGS) -I ../include -L ../src"

define Package/$(PKG_NAME)/install

endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/tinycap $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/tinymix $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/tinypcminfo $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/tinyplay $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
