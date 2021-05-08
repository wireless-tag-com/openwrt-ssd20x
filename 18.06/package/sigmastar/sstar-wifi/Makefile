#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=sstar-wifi
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=SigmStar
  CATEGORY:=SigmStar
  SUBMENU:=modules
  DEPENDS:=@TARGET_sstar +kmod-cfg80211-linux
  TITLE:=SSD20X wifi
endef

define Package/$(PKG_NAME)/description
 SSD201 wifi ko
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/etc/uci-defaults
	$(CP) ./files/*.ko $(1)/lib/modules/$(LINUX_VERSION)/
	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_DATA) ./files/lib/wifi/sstar.sh $(1)/lib/wifi
	$(INSTALL_DIR) $(1)/etc/hotplug.d/ieee80211 $(1)/etc/hotplug.d/net
	$(INSTALL_DATA) ./files/sstar.hotplug $(1)/etc/hotplug.d/ieee80211/12-sstar-detect
	$(INSTALL_BIN) ./files/sstar-wifi $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/sstar-wisp.default $(1)/etc/uci-defaults/91-sstar-wifi
endef

$(eval $(call BuildPackage,sstar-wifi))
