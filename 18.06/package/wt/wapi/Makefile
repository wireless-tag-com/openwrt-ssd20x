
include $(TOPDIR)/rules.mk

PKG_NAME:=wapi
PKG_VERSION:=1.0
PKG_RELEASE:=20210129

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=WT
  CATEGORY:=WT
  TITLE:=WT lua api scripts
  PKGARCH:=all
  SUBMENU:=api
  DEPENDS:=+lua +libubus-lua +libuci-lua +rpcd +lua-cjson +luci-lib-nixio +luci-lib-ip +luasec +luasocket +luafilesystem
endef

define Package/$(PKG_NAME)/description
WT lua api scripts.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/usr/lib/lua/wt
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) ./files/wt/* $(1)/usr/lib/lua/wt/
	$(INSTALL_BIN) ./files/wapi $(1)/usr/sbin/wapi
	$(INSTALL_DATA) ./files/wapi.config $(1)/etc/config/wapi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
