CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
EXTRA_CFLAGS += -Idrivers/sstar/include
EXTRA_CFLAGS      += -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)

obj-$(CONFIG_SS_VOLTAGE_CTRL) += voltage_ctrl.o
obj-$(CONFIG_SS_VOLTAGE_CTRL) += $(CONFIG_SSTAR_CHIP_NAME)/