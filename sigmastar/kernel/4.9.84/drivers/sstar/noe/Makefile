#
# Makefile for MStar device drivers.
#

# noe
obj-$(CONFIG_NOE) += drv/eth/

ifeq ($(CONFIG_NOE_NAT_HW),y)
obj-$(CONFIG_NOE_NAT_HW)  += drv/nat/hook/
obj-m         += drv/nat/hw_nat/
endif


