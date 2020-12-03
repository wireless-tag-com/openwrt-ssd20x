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
  FILES:= \
	$(LINUX_DIR)/drivers/usb/host/ehci-hcd.ko
  AUTOLOAD:=$(call AutoLoad,35,ehci-hcd)
  $(call AddDepends/usb)
endef
$(eval $(call KernelPackage,usb-sstar))


define KernelPackage/mmc-sstar
  SUBMENU:=Other modules
  TITLE:=sstar MMC
  DEPENDS:=@TARGET_sstar
  FILES:= \
	$(LINUX_DIR)/drivers/mmc/core/mmc_core.ko \
	$(LINUX_DIR)/drivers/mmc/core/pwrseq_simple.ko \
	$(LINUX_DIR)/drivers/mmc/core/pwrseq_emmc.ko \
	$(LINUX_DIR)/drivers/mmc/card/mmc_block.ko \
	$(LINUX_DIR)/drivers/sstar/sdmmc/kdrv_sdmmc.ko
  AUTOLOAD:=$(call AutoLoad,40,mmc_core pwrseq_simple pwrseq_emmc mmc_block kdrv_sdmmc)
endef

$(eval $(call KernelPackage,mmc-sstar))

define KernelPackage/emac-sstar
  SUBMENU:=Other modules
  TITLE:=sstar netphy
  DEPENDS:=@TARGET_sstar +kmod-of-mdio +kmod-libphy
  FILES:= \
	$(LINUX_DIR)/drivers/sstar/netphy/sstar_100_phy.ko \
	$(LINUX_DIR)/drivers/sstar/emac/kdrv_emac.ko
  AUTOLOAD:=$(call AutoProbe,sstar_100_phy kdrv_emac)
endef

$(eval $(call KernelPackage,emac-sstar))