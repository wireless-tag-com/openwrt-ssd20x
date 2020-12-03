CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/crypto/infinity
EXTRA_CFLAGS      +=  -Idrivers/sstar/bdma/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/movedma/

obj-$(CONFIG_MS_MSYS) = ms_msys.o
obj-$(CONFIG_MS_MSYS) += ms_msys_dma_wrapper.o
obj-$(CONFIG_MSYS_PERF_TEST) += ms_msys_perf_test.o
obj-$(CONFIG_MS_MSYS) += $(CONFIG_SSTAR_CHIP_NAME)/platform_msys.o
obj-$(CONFIG_MS_MSYS_LOG) += ms_msys_log.o
obj-$(CONFIG_MSYS_BENCH_MEMORY_FUNC) += ms_msys_memory_bench.o
obj-$(CONFIG_MSYS_MIU_PROTECT) += ms_msys_miu_protect.o
obj-$(CONFIG_MP_IRQ_TRACE) += ms_msys_irq_stat.o


ifneq ($(CONFIG_THUMB2_KERNEL),y)
	obj-$(CONFIG_MS_MSYS) += memcpy.o
endif
