CONFIG_SYS_CONFIG_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))

ccflags-y += -I$(srctree)/$(src)/$(CONFIG_SYS_CONFIG_NAME)
ccflags-y += -I$(srctree)/$(src)

obj-y += $(CONFIG_SYS_CONFIG_NAME)/halSERFLASH.o
obj-y += drvSERFLASH.o
obj-y += mtd_isp.o
obj-y += drvDeviceInfo.o
