#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=sstar-rawgenerator
PKG_VERSION:=1.0

include $(INCLUDE_DIR)/host-build.mk

define Host/Prepare
	mkdir -p $(HOST_BUILD_DIR)
	$(CP) ./src/* $(HOST_BUILD_DIR)/
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/mxpgenerator $(STAGING_DIR_HOST)/bin/sstar-mxpgenerator
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/pnigenerator $(STAGING_DIR_HOST)/bin/sstar-pnigenerator
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/sstar-mxpgenerator $(STAGING_DIR_HOST)/bin/sstar-pnigenerator
endef

$(eval $(call HostBuild))
