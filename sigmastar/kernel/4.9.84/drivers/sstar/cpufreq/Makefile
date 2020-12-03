EXTRA_CFLAGS += -Idrivers/sstar/include
EXTRA_CFLAGS += -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
 
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
obj-y += $(CONFIG_SSTAR_CHIP_NAME)/cpufreq.o
