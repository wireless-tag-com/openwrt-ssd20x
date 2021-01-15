include $(TOPDIR)/rules.mk

PKG_NAME:=liblvgl
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=WT
  CATEGORY:=WT
  SUBMENU:=libs
  DEPENDS:=+libopenssl +libubox +libubus +libblobmsg-json +libcjson +libwtinfo
  TITLE:=WT liblvgl lib
endef

define Package/$(PKG_NAME)/description
 WT lvgl lib.
endef

TARGET_CFLAGS += -D_GNU_SOURCE

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) ./files/liblvgl.so $(1)/usr/lib/liblvgl.so
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) ./files/liblvgl.so $(1)/usr/lib/liblvgl.so
endef

$(eval $(call BuildPackage,$(PKG_NAME)))