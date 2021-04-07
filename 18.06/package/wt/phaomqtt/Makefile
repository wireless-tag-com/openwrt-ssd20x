#
# Copyright (C) 2012-2021 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libphaomqtt
PKG_VERSION:=1.3.8
PKG_RELEASE:=1
PKG_SOURCE_PROTO:=git

PKG_SOURCE_URL:=https://github.com/eclipse/paho.mqtt.c.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=317fb008e1541838d1c29076d2bc5c3e4b6c4f53
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/${PKG_NAME}
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=phao.mqtt.c lib
  DEPENDS:=+libopenssl
endef

define Package/${PKG_NAME}/description
  phao.mqtt.c
endef

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/MQTTAsync.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src//MQTTClient.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src//MQTTClientPersistence.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src//MQTTProperties.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/MQTTReasonCodes.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/MQTTSubscribeOpts.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/MQTTExportDeclarations.h $(1)/usr/include
endef

define Package/${PKG_NAME}/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/build/output/*.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,${PKG_NAME}))
