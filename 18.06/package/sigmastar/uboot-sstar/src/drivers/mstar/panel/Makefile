CONFIG_SYS_CONFIG_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))

ccflags-y += -I$(srctree)/$(src)/drv/pnl/inc
ccflags-y += -I$(srctree)/$(src)/drv/pnl/inc/uboot
ccflags-y += -I$(srctree)/$(src)/drv/pnl/pub
ccflags-y += -I$(srctree)/$(src)/drv/mipnl/pub
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/inc
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/pub
ccflags-y += -I$(srctree)/$(src)/hal/pub
ccflags-y += -I$(srctree)/$(src)/../common

obj-y +=./drv/pnl/src/uboot/drv_pnl_os.o
obj-y +=./drv/pnl/src/drv_pnl_ctx.o
obj-y +=./drv/pnl/src/drv_pnl_if.o
obj-y +=./drv/mipnl/src/mhal_pnl.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_pnl_if.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_pnl.o
