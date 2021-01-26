CONFIG_SYS_CONFIG_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))

ccflags-y += -I$(srctree)/$(src)/$(CONFIG_SYS_CONFIG_NAME)
ccflags-y += -I$(srctree)/$(src)

obj-y += $(CONFIG_SYS_CONFIG_NAME)/mhal_gpio.o
ifneq ("$(wildcard $(srctree)/$(src)/$(CONFIG_SYS_CONFIG_NAME)/mhal_pinmux.c)","")
obj-y += $(CONFIG_SYS_CONFIG_NAME)/mhal_pinmux.o
endif
obj-y += drvGPIO.o
obj-y += mdrv_sw_iic.o

