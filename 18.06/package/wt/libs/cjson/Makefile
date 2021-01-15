include $(TOPDIR)/rules.mk

PKG_NAME:=libcjson
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=WT
  CATEGORY:=WT
  SUBMENU:=libs
  TITLE:=WT cjson lib
endef

define Package/$(PKG_NAME)/description
 WT cjson lib.
endef

TARGET_CFLAGS += -D_GNU_SOURCE

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/cJSON.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libcJSON.so $(1)/usr/lib/libcJSON.so
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libcJSON.so $(1)/usr/lib/libcJSON.so
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
