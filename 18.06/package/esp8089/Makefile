#
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=esp8089
PKG_RELEASE:=1.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SECTION:=WT
  CATEGORY:=WT
  TITLE:=WT esp8089 modules
  SUBMENU:=Kernel Modules
  DEPENDS:=+kmod-mmc +kmod-mmc-sstar
  FILES:=$(PKG_BUILD_DIR)/esp8089.ko
  #AUTOLOAD:=$(call AutoLoad,40,esp8089)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		modules
endef

define KernelPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/etc/uci-defaults $(1)/etc/init.d $(1)/lib/firmware
	#$(CP) ./files/*.ko $(1)/lib/modules/$(LINUX_VERSION)/
	$(INSTALL_DIR) $(1)/lib/wifi $(1)/usr/sbin
	$(INSTALL_DATA) ./files/lib/wifi/esp.sh $(1)/lib/wifi
	$(INSTALL_DIR) $(1)/etc/hotplug.d/ieee80211 $(1)/etc/hotplug.d/net
	$(INSTALL_DATA) ./files/esp.hotplug $(1)/etc/hotplug.d/ieee80211/12-esp-detect
	$(INSTALL_DATA) ./files/esp-net.hotplug $(1)/etc/hotplug.d/net/12-esp
	$(INSTALL_BIN) ./files/esp-wifi $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/esp8089-load $(1)/usr/sbin/
	#$(CP) $(PKG_BUILD_DIR)/firmware/*.bin $(1)/lib/firmware/
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
