obj-$(CONFIG_MS_SDMMC)   += kdrv_sdmmc.o

EXTRA_CFLAGS	+= -Idrivers/sstar/include
EXTRA_CFLAGS	+= -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS	+= -Idrivers/sstar/sdmmc/inc
EXTRA_CFLAGS	+= -Idrivers/sstar/sdmmc/inc/$(CONFIG_SSTAR_CHIP_NAME)

kdrv_sdmmc-y =  src/hal_card_regs.o src/hal_card_timer.o
kdrv_sdmmc-y += src/hal_card_intr_v5.o
kdrv_sdmmc-y += src/hal_sdmmc_v5.o
kdrv_sdmmc-y += src/$(subst ",,$(CONFIG_SSTAR_CHIP_NAME))/hal_card_platform.o

kdrv_sdmmc-y += ms_sdmmc_lnx.o
