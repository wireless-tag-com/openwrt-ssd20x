
CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)

obj-y += ms_chip.o atags_to_fdt.o
obj-$(CONFIG_TI_CMEM_STUB) += cmemk_stub.o
obj-y += $(CONFIG_SSTAR_CHIP_NAME)/
obj-$(CONFIG_SS_PROFILING_TIME) += prof.o
