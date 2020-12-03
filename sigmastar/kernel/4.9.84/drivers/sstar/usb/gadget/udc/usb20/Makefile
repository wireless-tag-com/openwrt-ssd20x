#
# Makefile for SStar USB device drivers.
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

EXTRA_CFLAGS	+= -I$(srctree)/drivers/sstar/usb/gadget/udc/usb20/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS	+= -I$(srctree)/drivers/sstar/usb/gadget/udc/usb20/include
EXTRA_CFLAGS    += -I$(srctree)/drivers/usb/gadget/udc

obj-$(CONFIG_USB_GADGET_SSTAR_DEVICE) = udc-msb250x.o
udc-msb250x-y :=  src/msb250x_gadget.o src/msb250x_dma.o src/msb250x_ep.o src/msb250x_udc.o
