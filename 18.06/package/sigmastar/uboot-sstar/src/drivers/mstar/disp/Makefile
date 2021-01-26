CONFIG_SYS_CONFIG_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))

ifeq ($(CONFIG_SSTAR_DISP_HDMITX_VGA),y)
EXTRA_CFLAGS += -DHDMITX_VGA_SUPPORTED
endif

ccflags-y += -I$(srctree)/$(src)/drv/disp/inc
ccflags-y += -I$(srctree)/$(src)/drv/disp/inc/uboot
ccflags-y += -I$(srctree)/$(src)/drv/disp/pub
ccflags-y += -I$(srctree)/$(src)/drv/midisp/pub
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/inc
ccflags-y += -I$(srctree)/$(src)/hal/$(CONFIG_SYS_CONFIG_NAME)/pub
ccflags-y += -I$(srctree)/$(src)/../common
ccflags-y += -I$(srctree)/$(src)/hal/pub

obj-y +=./drv/disp/src/uboot/drv_disp_os.o
obj-y +=./drv/disp/src/drv_disp_ctx.o
obj-y +=./drv/disp/src/drv_disp_irq.o
obj-y +=./drv/disp/src/drv_disp_if.o
obj-y +=./drv/midisp/src/mhal_disp.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_disp_if.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_disp_irq.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_disp_scaler_e.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_disp_pq.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_disp_color.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_disp_mop.o
obj-y +=./hal/$(CONFIG_SYS_CONFIG_NAME)/src/hal_disp_vga_timing_tbl.o
