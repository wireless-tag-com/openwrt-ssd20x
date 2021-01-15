#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=sstar-init
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=SigmStar
  CATEGORY:=SigmStar
  SUBMENU:=utils
  DEPENDS:=@TARGET_sstar + sstar-mpp-lib
  TITLE:=SSD201/SD202 mpp init
endef

define Package/$(PKG_NAME)/description
 SSD201 MPP init
endef

define Package/$(PKG_NAME)/config
  source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += $(if $(CONFIG_SSTAR_LCD_CC0702I50R),-DLCD_CC0702I50R)
TARGET_CFLAGS += $(if $(CONFIG_SSTAR_LCD_SAT070CP50),-DLCD_SAT070CP50)
TARGET_CFLAGS += $(if $(CONFIG_SSTAR_LCD_FRD480X480),-DLCD_FRD480X480)
TARGET_CFLAGS += $(if $(CONFIG_SSTAR_LCD_FRD430H40045),-DLCD_FRD430H40045)
TARGET_CFLAGS += $(if $(CONFIG_SSTAR_LCD_FRD720X720),-DLCD_FRD720X720)
TARGET_CFLAGS += $(if $(CONFIG_SSTAR_LCD_FRD720X720BK),-DLCD_FRD720X720BK)

MAKE_FLAGS += \
        CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include/ssd201mpp"

ifdef CONFIG_SSTAR_LCD_CC0702I50R
SSTAR_LCD_DIR=CC0702I50R
endif

ifdef CONFIG_SSTAR_LCD_SAT070CP50
SSTAR_LCD_DIR=SAT070CP50
endif

ifdef CONFIG_SSTAR_LCD_FRD480X480
SSTAR_LCD_DIR=FRD480X480
endif

ifdef CONFIG_SSTAR_LCD_FRD430H40045
SSTAR_LCD_DIR=FRD430H40045
endif

ifdef CONFIG_SSTAR_LCD_FRD720X720
SSTAR_LCD_DIR=FRD720X720
endif

ifdef CONFIG_SSTAR_LCD_FRD720X720BK
SSTAR_LCD_DIR=FRD720X720BK
endif

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sstar-init $(1)/usr/sbin/
	$(CP) ./files/$(SSTAR_LCD_DIR)/* $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
