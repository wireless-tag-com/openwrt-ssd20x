CONFIG_SYS_CONFIG_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))

ccflags-y += -I$(srctree)/$(src)/drv/inc
ccflags-y += -I$(srctree)/$(src)/drv/inc/uboot
ccflags-y += -I$(srctree)/$(src)/drv/pub

ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/inc
ccflags-y += -I$(srctree)/$(src)/../common


obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_rgn_common.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_cover.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_gop.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_osd.o
obj-y +=./drv/src/uboot/drv_rgn_os.o
obj-y +=./drv/src/drv_cover.o
obj-y +=./drv/src/drv_gop.o
obj-y +=./drv/src/drv_osd.o
obj-y +=./drv/src/drv_rgn_common.o
obj-y +=./drv/src/mhal_rgn.o