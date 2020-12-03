CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

obj-$(CONFIG_MS_CRYPTO) += mdrv_crypto.o

EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/crypto
EXTRA_CFLAGS      +=  -Idrivers/sstar/crypto/hal/$(CONFIG_SSTAR_CHIP_NAME)

mdrv_crypto-objs := mdrv_aes.o \
               mdrv_sha.o \
               mdrv_rsa.o \
               mdrv_cipher.o \
               halAESDMA.o

obj-$(CONFIG_CRYPTODEV) += cryptodev/