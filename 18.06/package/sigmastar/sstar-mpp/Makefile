#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=sstar-mpp
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=SigmStar
  CATEGORY:=SigmStar
  SUBMENU:=modules
  DEPENDS:=@TARGET_sstar
  TITLE:=SSD201/SD202 mpp
endef

define Package/$(PKG_NAME)/description
 SSD201 MPP ko
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

SSTAR_MPP_DIR=M128

ifdef CONFIG_SSTAR_MEM_64MB
SSTAR_MPP_DIR=M64
endif

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)
	$(INSTALL_DIR) $(1)/config $(1)/appconfigs $(1)/usr/sbin $(1)/etc/uci-defaults
	$(CP) ./files/$(LIBC)/modules/4.9.84/*.ko $(1)/lib/modules/$(LINUX_VERSION)/
	$(CP) ./files/config/* $(1)/config/
	$(CP) ./files/$(SSTAR_MPP_DIR)/* $(1)/config/
	$(INSTALL_BIN) ./files/sstar-mpp $(1)//usr/sbin/sstar-mpp
	$(INSTALL_BIN) ./files/sstar.default $(1)/etc/uci-defaults/99-sstar
endef

define Package/$(PKG_NAME)-lib
  SECTION:=SigmStar
  CATEGORY:=SigmStar
  SUBMENU:=libs
  DEPENDS:=@TARGET_sstar
  TITLE:=ssd201/202 mpp libs
endef

define Package/$(PKG_NAME)-lib/description
 SSD201 MPP lib
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/ssd201mpp $(1)/usr/lib
	$(CP) ./files/$(LIBC)/include/* $(1)/usr/include/ssd201mpp/
	$(CP) ./files/$(LIBC)/mi_libs/dynamic/*.so $(1)/usr/lib/
	$(CP) ./files/$(LIBC)/ex_libs/dynamic/*.so $(1)/usr/lib/
endef

define Package/$(PKG_NAME)-lib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) ./files/$(LIBC)/mi_libs/dynamic/*.so $(1)/usr/lib/
	$(CP) ./files/$(LIBC)/ex_libs/dynamic/*.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,sstar-mpp-lib))
$(eval $(call BuildPackage,sstar-mpp))
