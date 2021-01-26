CONFIG_SYS_CONFIG_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))
#obj-y += $(CONFIG_SYS_CONFIG_NAME)/drvSPINAND_uboot.o
obj-y += spi_nand.o
spi_nand-objs := hal/common/drvSPINAND_common_hal.o \
                 hal/$(CONFIG_SYS_CONFIG_NAME)/drvSPINAND_hal.o \
                 drvSPINAND.o \
                 drvSPINAND_api.o \
                 drvSPINAND_dev.o \
                 spinand.o


ccflags-y += -I$(srctree)/$(src)/inc/common
ccflags-y += -I$(srctree)/$(src)/inc/config
ccflags-y += -I$(srctree)/$(src)/inc
ccflags-y += -I$(srctree)/$(src)/inc/config/$(CONFIG_SYS_CONFIG_NAME)
