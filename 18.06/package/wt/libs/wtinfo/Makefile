include $(TOPDIR)/rules.mk

PKG_NAME:=libwtinfo
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=WT
  CATEGORY:=WT
  SUBMENU:=libs
  DEPENDS:=+libopenssl +libcjson
  TITLE:=WT board info lib
endef

define Package/libwtinfo-lua
  SECTION:=WT
  CATEGORY:=WT
  SUBMENU:=libs
  DEPENDS:=+libwtinfo +liblua
  TITLE:=WT wtinfo lua lib
endef

define Package/wtinfo
  SECTION:=WT
  CATEGORY:=WT
  TITLE:=WT board info client
  DEPENDS:=+libwtinfo +libopenssl
  SUBMENU:=utils
endef

define Package/wtinfo/description
Command line frontend for the wtinfo information library.
endef

define Package/libwtinfo-lua/description
 WT info lua lib.
endef

define Package/$(PKG_NAME)/description
 WT wtinfo lib.
endef

TARGET_CFLAGS += -D_GNU_SOURCE

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) ./files/wtinfo.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) ./files/libwtinfo.so $(1)/usr/lib/libwtinfo.so
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) ./files/libwtinfo.so $(1)/usr/lib/libwtinfo.so
endef

define Package/libwtinfo-lua/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) ./files/wtinfo.so $(1)/usr/lib/lua/
endef

define Package/wtinfo/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/bin $(1)/lib/functions
	$(INSTALL_BIN) ./files/wtinfo $(1)/usr/bin/wtinfo
	$(INSTALL_BIN) ./files/wtinfo.sh $(1)/lib/functions/wtinfo.sh
	$(INSTALL_BIN) ./files/wtinfo.init $(1)/etc/init.d/wtinfo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,libwtinfo-lua))
$(eval $(call BuildPackage,wtinfo))
