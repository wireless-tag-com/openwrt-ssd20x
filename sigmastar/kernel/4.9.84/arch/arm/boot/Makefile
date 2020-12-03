#
# arch/arm/boot/Makefile
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies.
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1995-2002 Russell King
#

OBJCOPYFLAGS	:=-O binary -R .comment -S

ifneq ($(CONFIG_ARM_PATCH_PHYS_VIRT),)
LD_ADDR = 0x20008000
else
LD_ADDR = $(shell /bin/bash -c 'printf "0x%08x" $$[$(CONFIG_PHYS_OFFSET) + 0x8000]')
endif

MKIMAGE_BIN = scripts/mkimage
MZ_BIN = scripts/mz
KERNEL_RELEASE_FILE = include/config/kernel.release
ROOTFS = $(obj)/ramdisk_linaro4.8_minit.img.xz

_BUILTIN_DTB_NAME=$(strip $(shell echo ${CONFIG_SS_DTB_NAME}))
ifneq ($(_BUILTIN_DTB_NAME),)
ifneq ($(wildcard arch/arm/boot/dts/$(_BUILTIN_DTB_NAME).dts),)
SS_DTB_NAME=arch/arm/boot/dts/$(_BUILTIN_DTB_NAME).dtb
endif
endif

CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

ifeq ($(CONFIG_FPGA),y)
MS_EXTRA_DTBS=arch/arm/boot/dts/$(CHIP_NAME)-fpga.dtb
else
MS_EXTRA_DTBS=
endif

ifneq ($(MACHINE),)
include $(MACHINE)/Makefile.boot
endif

# Note: the following conditions must always be true:
#   ZRELADDR == virt_to_phys(PAGE_OFFSET + TEXT_OFFSET)
#   PARAMS_PHYS must be within 4MB of ZRELADDR
#   INITRD_PHYS must be in RAM
ZRELADDR    := $(zreladdr-y)
PARAMS_PHYS := $(params_phys-y)
INITRD_PHYS := $(initrd_phys-y)

export ZRELADDR INITRD_PHYS PARAMS_PHYS

targets := Image zImage xipImage bootpImage uImage

ifeq ($(CONFIG_XIP_KERNEL),y)

$(obj)/xipImage: vmlinux FORCE
	$(call if_changed,objcopy)
	@$(kecho) '  Physical Address of xipImage: $(CONFIG_XIP_PHYS_ADDR)'

$(obj)/Image $(obj)/zImage: FORCE
	@echo 'Kernel configured for XIP (CONFIG_XIP_KERNEL=y)'
	@echo 'Only the xipImage target is available in this case'
	@false

else

$(obj)/xipImage: FORCE
	@echo 'Kernel not configured for XIP (CONFIG_XIP_KERNEL!=y)'
	@false

$(obj)/Image: vmlinux FORCE
	$(call if_changed,objcopy)

$(obj)/compressed/vmlinux: $(obj)/Image FORCE  $(SS_DTB_NAME) $(MS_EXTRA_DTBS)
	$(Q)$(MAKE) $(build)=$(obj)/compressed $@

$(obj)/zImage:	$(obj)/compressed/vmlinux FORCE
	$(call if_changed,objcopy)
ifneq ($(CONFIG_ARCH_SSTAR),)
ifeq ($(CONFIG_SS_BUILTIN_DTB), y)
		#update builtin DTB
		@test "${SS_DTB_NAME}" = "" || \
		if [ -e ${SS_DTB_NAME} ]; then \
			echo "  IMAGE   $(obj)/Image"; \
			echo "  BNDTB ${SS_DTB_NAME}"; \
			python scripts/ms_builtin_dtb_update.py $(obj)/Image ${SS_DTB_NAME}; \
			echo; \
		fi;
		#update Image-fpga DTB
		@if [ -e "arch/arm/boot/dts/$(CHIP_NAME)-fpga.dtb" ]; then \
			echo "  IMAGE   $(obj)/Image-fpga"; \
			cp -f $(obj)/Image $(obj)/Image-fpga; \
			echo "  BNDTB   $(CHIP_NAME)-fpga.dtb"; \
			python scripts/ms_builtin_dtb_update.py $(obj)/Image-fpga "arch/arm/boot/dts/$(CHIP_NAME)-fpga.dtb"; \
			echo; \
		fi;
endif

    #update the image size into Image	
	@python scripts/ms_bin_option_update_int.py  $(obj)/Image '#IMG_SZ#' $$(stat -c %s $(obj)/Image)
	#build Image
	@IMGNAME=$(shell strings -a -T binary $(obj)/Image | grep 'MVX' | grep 'LX'  | sed 's/\\*MVX/MVX/g' | cut -c 1-32 ); \
	if [ -e ${MKIMAGE_BIN} ]; then \
	    ${MKIMAGE_BIN} -A arm -O linux -T kernel -C none -a ${LD_ADDR} -e ${LD_ADDR} -n $${IMGNAME} -d $(obj)/Image $(obj)/uImage; \
	    echo; \
	    echo 'Compress Kernel Image'; \
	    xz -z -k -f $(obj)/Image; \
	    ${MKIMAGE_BIN} -A arm -O linux -C lzma -a ${LD_ADDR} -e ${LD_ADDR} -n $${IMGNAME} -d $(obj)/Image.xz $(obj)/uImage.xz; \
        ${MZ_BIN} c  $(obj)/Image  $(obj)/Image.mz ; \
        echo; \
        ${MKIMAGE_BIN} -A arm -O linux -C mz -a ${LD_ADDR} -e ${LD_ADDR} -n $${IMGNAME} -d $(obj)/Image.mz $(obj)/uImage.mz; \
        echo; \
	else \
	    echo ">> Can't find $${MKIMAGE}. Please check the u-boot path or build u-boot <<"; \
	fi; \
    if [ -e $(ROOTFS) ]; then \
        ${MKIMAGE_BIN} -A arm -O linux -T multi -C none -a ${LD_ADDR} -e ${LD_ADDR} -n $${IMGNAME} -d  $(obj)/Image:$(ROOTFS) $(obj)/kernel.img; \
        echo; \
        if [ -e ${MZ_BIN} ]; then \
	        ${MZ_BIN} c  $(obj)/Image  $(obj)/Image.mz ; \
	        echo; \
	        ${MKIMAGE_BIN} -A arm -O linux -T multi -C mz -a ${LD_ADDR} -e ${LD_ADDR} -n $${IMGNAME} -d $(obj)/Image.mz:$(ROOTFS) $(obj)/kernel.mz.img; \
	        echo; \
        fi; \
	fi;
	@echo
endif
endif

ifneq ($(LOADADDR),)
  UIMAGE_LOADADDR=$(LOADADDR)
else
  ifeq ($(CONFIG_ZBOOT_ROM),y)
    UIMAGE_LOADADDR=$(CONFIG_ZBOOT_ROM_TEXT)
  else
    UIMAGE_LOADADDR=$(ZRELADDR)
  endif
endif

check_for_multiple_loadaddr = \
if [ $(words $(UIMAGE_LOADADDR)) -ne 1 ]; then \
	echo 'multiple (or no) load addresses: $(UIMAGE_LOADADDR)'; \
	echo 'This is incompatible with uImages'; \
	echo 'Specify LOADADDR on the commandline to build an uImage'; \
	false; \
fi

$(obj)/uImage:	$(obj)/zImage FORCE
	@$(check_for_multiple_loadaddr)
	$(call if_changed,uimage)

$(obj)/bootp/bootp: $(obj)/zImage initrd FORCE
	$(Q)$(MAKE) $(build)=$(obj)/bootp $@

$(obj)/bootpImage: $(obj)/bootp/bootp FORCE
	$(call if_changed,objcopy)

PHONY += initrd install zinstall uinstall
initrd:
	@test "$(INITRD_PHYS)" != "" || \
	(echo This machine does not support INITRD; exit -1)
	@test "$(INITRD)" != "" || \
	(echo You must specify INITRD; exit -1)

install:
	$(CONFIG_SHELL) $(srctree)/$(src)/install.sh "$(KERNELRELEASE)" \
	$(obj)/Image System.map "$(INSTALL_PATH)"

zinstall:
	$(CONFIG_SHELL) $(srctree)/$(src)/install.sh "$(KERNELRELEASE)" \
	$(obj)/zImage System.map "$(INSTALL_PATH)"

uinstall:
	$(CONFIG_SHELL) $(srctree)/$(src)/install.sh "$(KERNELRELEASE)" \
	$(obj)/uImage System.map "$(INSTALL_PATH)"

subdir-	    := bootp compressed dts
