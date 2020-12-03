CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

#ifneq ($(wildcard drivers/sstar/usb/$(CONFIG_SSTAR_CHIP_NAME)/),)
obj-$(CONFIG_MS_USB_OLD_PLATFORM) += $(CONFIG_SSTAR_CHIP_NAME)/
#endif

obj-y += $(CONFIG_SSTAR_CHIP_NAME)/
