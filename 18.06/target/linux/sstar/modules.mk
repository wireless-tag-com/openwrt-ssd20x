#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

OTHER_MENU:=Other modules

define KernelPackage/usb-sstar
  TITLE:=sstar EHCI controller support
  DEPENDS:=@TARGET_sstar
  KCONFIG:= CONFIG_USB_EHCI_HCD
  FILES:= \
	$(LINUX_DIR)/drivers/usb/host/ehci-hcd.ko
  AUTOLOAD:=$(call AutoLoad,35,ehci-hcd)
  $(call AddDepends/usb)
endef
$(eval $(call KernelPackage,usb-sstar))


define KernelPackage/mmc-sstar
  SUBMENU:=Other modules
  TITLE:=sstar MMC
  DEPENDS:=@TARGET_sstar +kmod-mmc
  KCONFIG:=
  	CONFIG_SS_FAST_MMC=y \
  	CONFIG_PWRSEQ_EMMC=n \
	CONFIG_PWRSEQ_SIMPLE=n \
  	CONFIG_MS_SDMMC
  FILES:= \
	$(LINUX_DIR)/drivers/sstar/sdmmc/kdrv_sdmmc.ko
  AUTOLOAD:=$(call AutoProbe,kdrv_sdmmc)
endef

$(eval $(call KernelPackage,mmc-sstar))

define KernelPackage/emac-sstar
  SUBMENU:=Other modules
  TITLE:=sstar netphy
  DEPENDS:=@TARGET_sstar +kmod-of-mdio +kmod-libphy
  KCONFIG:= CONFIG_MS_EMAC \
  	CONFIG_SSTAR_NETPHY 
  FILES:= \
	$(LINUX_DIR)/drivers/sstar/netphy/sstar_100_phy.ko \
	$(LINUX_DIR)/drivers/sstar/emac/kdrv_emac.ko
  AUTOLOAD:=$(call AutoProbe,sstar_100_phy kdrv_emac)
endef

$(eval $(call KernelPackage,emac-sstar))