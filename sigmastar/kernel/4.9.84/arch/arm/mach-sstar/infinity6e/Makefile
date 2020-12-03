CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)

obj-y += soc.o
obj-$(CONFIG_PM_SLEEP) += pm.o
obj-y += sram.o
obj-y += miu_bw.o
obj-$(CONFIG_SMP) += smp_platform.o smp_head.o
obj-$(CONFIG_SS_MIU_ARBITRATION) += miu_arb.o