#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sstar-mp3player
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=SigmStar
  CATEGORY:=SigmStar
  SUBMENU:=utils
  DEPENDS:=@TARGET_sstar +sstar-mpp-lib +libmad +libpthread
  PKGARCH:=all
  TITLE:=SSD201/SD202 mp3 player
endef

define Package/$(PKG_NAME)/description
 SSD201 mp3 player
endef

MAKE_FLAGS += \
        CFLAGS="$(TARGET_CFLAGS) -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include/ssd201mpp" 

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/SsMp3Player $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
