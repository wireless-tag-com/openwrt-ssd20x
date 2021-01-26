CONFIG_SYS_CONFIG_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))

ccflags-y += -I$(srctree)/$(src)/$(CONFIG_SYS_CONFIG_NAME)

obj-y += $(CONFIG_SYS_CONFIG_NAME)/mhal_emac.o
obj-y += mdrv_emac.o 
obj-y += cmd_emac.o 
ifeq ($(CONFIG_ETHERNET_ICPLUS), y)
obj-y += icplus.o 
endif