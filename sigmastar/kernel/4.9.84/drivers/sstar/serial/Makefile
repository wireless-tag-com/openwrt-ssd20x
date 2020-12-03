CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/serial/$(CONFIG_SSTAR_CHIP_NAME)

obj-$(CONFIG_MS_SERIAL) += ms_uart.o
obj-$(CONFIG_MS_SERIAL) += $(CONFIG_SSTAR_CHIP_NAME)/uart_pads.o
