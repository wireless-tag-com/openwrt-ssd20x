CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/flash_isp
EXTRA_CFLAGS      +=  -Idrivers/sstar/flash_isp/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/flash_isp/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/bdma/$(CONFIG_SSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
obj-$(CONFIG_MS_FLASH_ISP)  += drvSERFLASH.o mtd_serflash.o drvDeviceInfo.o
obj-$(CONFIG_MS_FLASH_ISP)  += $(CONFIG_SSTAR_CHIP_NAME)/halSERFLASH.o
obj-$(CONFIG_MS_FLASH_ISP_MXP_PARTS) += part_mxp.o mxp_flash.o



