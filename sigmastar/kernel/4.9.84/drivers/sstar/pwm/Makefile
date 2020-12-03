CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

# specific options
#EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/pwm/$(CONFIG_SSTAR_CHIP_NAME)

# files
obj-$(CONFIG_MS_PWM) += ms_pwm.o
ms_pwm-y             += mdrv_pwm.o $(CONFIG_SSTAR_CHIP_NAME)/mhal_pwm.o

