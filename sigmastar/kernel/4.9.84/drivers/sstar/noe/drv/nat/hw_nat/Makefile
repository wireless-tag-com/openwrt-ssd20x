CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
ifdef CONFIG_SSTAR_PROJECT_NAME
    CONFIG_SSTAR_PROJECT_NAME := $(subst ",,$(CONFIG_SSTAR_PROJECT_NAME))
endif


# general options

EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/noe/drv/eth
EXTRA_CFLAGS      +=  -Idrivers/sstar/noe/hal/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Iinclude/linux
EXTRA_CFLAGS      +=  -Idrivers/sstar/noe/drv/nat/hw_nat


ifeq ($(CONFIG_NOE_QDMA),y)
EXTRA_CFLAGS      +=  -DCONFIG_RAETH_QDMA
endif

ifeq ($(CONFIG_NOE_GMAC2),y)
EXTRA_CFLAGS      +=  -DCONFIG_RAETH_GMAC2
EXTRA_CFLAGS      +=  -DCONFIG_PSEUDO_SUPPORT
endif
EXTRA_CFLAGS      +=  -DCONFIG_RA_NAT_HW


obj-$(CONFIG_NOE_HW_NAT) += hw_nat.o
hw_nat-objs := mdrv_hwnat.o
hw_nat-objs += mdrv_hwnat_foe.o
hw_nat-objs += mdrv_hwnat_util.o
hw_nat-objs += mdrv_hwnat_ioctl.o
hw_nat-objs += mdrv_hwnat_log.o
hw_nat-objs += mdrv_hwnat_api.o


hw_nat-objs += mdrv_hwnat_mcast.o
hw_nat-objs += ../../../hal/$(CONFIG_SSTAR_CHIP_NAME)/mhal_hwnat.o

