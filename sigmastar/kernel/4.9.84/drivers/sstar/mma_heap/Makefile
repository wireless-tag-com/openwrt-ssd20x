#
# Makefile for MStar MMA HEAP  drivers.
#

# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/sys/common
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/mma_heap
EXTRA_CFLAGS      +=  -Idrivers/sstar/miu
EXTRA_CFLAGS      +=  -Idrivers/sstar/cpu/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
# specific options
EXTRA_CFLAGS      +=  -Idrivers/staging/android/ion

# files
obj-$(CONFIG_MSTAR_MMAHEAP)     += mdrv-mma_heap.o
mdrv-mma_heap-objs                       += mdrv_mma_heap.o region_substract.o
