#
# Makefile for MStar dip device drivers.

# general options
EXTRA_CFLAGS                +=  -Idrivers/mstar/include
EXTRA_CFLAGS                +=  -Idrivers/mstar/sata_host/bench_test/include
EXTRA_CFLAGS                +=  -Idrivers/mstar/sata_host/bench_test/drv

# specific options
EXTRA_CFLAGS                +=

#ccflags-y		+= -DMSOS_TYPE_LINUX_KERNEL
#ccflags-y		+= -DDIP_REG_DUMP
#ccflags-y		+= -DDIP_UT_MFDEC_SIM_MODE
#ccflags-y		+= -DDIP_UT_ALLOC_MEM_FOR_3DDI
#ccflags-y		+= -DDIP_UT_ENABLE_RIU
#ccflags-y		+= -DDIP_UT_WO_IRQ

#--------------------- sources ---------------------
obj-$(CONFIG_MS_SATA_BENCH_TEST) += sata_bench_test.o
sata_bench_test-y := mdrv_sata_io.o
sata_bench_test-y += drv/mhal_sata_bench_test.o