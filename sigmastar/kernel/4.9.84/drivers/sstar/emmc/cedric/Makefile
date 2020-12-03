MS_ARCH_NAME=cedric
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
#
# Makefile for MStar eMMC host drivers.
#

# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/emmc/$(MS_ARCH_NAME)/eMMCDrv/inc/api/
EXTRA_CFLAGS      +=  -Idrivers/sstar/emmc/$(MS_ARCH_NAME)/eMMCDrv/inc/common/
EXTRA_CFLAGS      +=  -Idrivers/sstar/emmc/$(MS_ARCH_NAME)/eMMCDrv/inc/config/
EXTRA_CFLAGS += -Werror

# specific options
#EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX


obj-$(CONFIG_MS_EMMC_CEDRIC)                += kdrv_emmc.o

kdrv_emmc-y := 	mstar_mci.o \
                    eMMCDrv/src/api/eMMC_prg.o \
                    eMMCDrv/src/common/eMMC_hal.o \
                    eMMCDrv/src/common/eMMC_utl.o \
                    eMMCDrv/src/config/eMMC_platform.o \
