CHIP_NAME := $(subst ",,$(CONFIG_SYS_CONFIG_NAME))


ccflags-y += -I$(srctree)/$(src)/$(CHIP_NAME)/inc

ccflags-y += -DDJPD_OS_TYPE_UBOOT

obj-y += ./$(CHIP_NAME)/src/jdapimin.o
obj-y += ./$(CHIP_NAME)/src/jdapistd.o
obj-y += ./$(CHIP_NAME)/src/jcomapi.o
obj-y += ./$(CHIP_NAME)/src/jdtrans.o
obj-y += ./$(CHIP_NAME)/src/jdmaster.o
obj-y += ./$(CHIP_NAME)/src/jdinput.o
obj-y += ./$(CHIP_NAME)/src/jdmainct.o
obj-y += ./$(CHIP_NAME)/src/jdcoefct.o
obj-y += ./$(CHIP_NAME)/src/jdpostct.o
obj-y += ./$(CHIP_NAME)/src/jdmarker.o
obj-y += ./$(CHIP_NAME)/src/jdhuff.o
obj-y += ./$(CHIP_NAME)/src/jdarith.o
obj-y += ./$(CHIP_NAME)/src/jddctmgr.o
obj-y += ./$(CHIP_NAME)/src/jidctint.o
obj-y += ./$(CHIP_NAME)/src/jidctfst.o
obj-y += ./$(CHIP_NAME)/src/jidctflt.o
obj-y += ./$(CHIP_NAME)/src/jdsample.o
obj-y += ./$(CHIP_NAME)/src/jdcolor.o
obj-y += ./$(CHIP_NAME)/src/jdmerge.o
obj-y += ./$(CHIP_NAME)/src/jquant1.o
obj-y += ./$(CHIP_NAME)/src/jquant2.o
obj-y += ./$(CHIP_NAME)/src/jdatasrc.o
obj-y += ./$(CHIP_NAME)/src/jaricom.o
obj-y += ./$(CHIP_NAME)/src/jerror.o
obj-y += ./$(CHIP_NAME)/src/jmemmgr.o
obj-y += ./$(CHIP_NAME)/src/jutils.o
obj-y += ./$(CHIP_NAME)/src/jmemmgr.o
obj-y += ./$(CHIP_NAME)/src/jmemnobs.o
