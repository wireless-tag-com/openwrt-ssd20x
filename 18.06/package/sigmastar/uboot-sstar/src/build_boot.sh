#!/bin/bash

while getopts "f:" opt;do
	case $opt in
		f)
			flashtype=$OPTARG
			;;
		\?)
			echo "Invalid option: -$OPTARG"
			;;
	esac
done


declare -x ARCH="arm"
declare -x CROSS_COMPILE="arm-linux-gnueabihf-"

if [ "${flashtype}" = "nor" ];then	
	echo "make nor defconfig"
	make infinity2m_usb_defconfig
else
	echo "make nand defconfig"
	make infinity2m_spinand_usb_defconfig
fi

make clean;make

if [ "${flashtype}" = "nor" ];then
	if [ -d ../project/board/i2m/boot/nor/uboot ];then
		cp u-boot.xz.img.bin ../project/board/i2m/boot/nor/uboot
	else
		echo "../project/board/i2m/nor/uboot does not exist"
	fi
else
	if [ -d ../project/board/i2m/boot/spinand/uboot ];then
		cp u-boot_spinand.xz.img.bin ../project/board/i2m/boot/spinand/uboot
	else
		echo "../project/board/i2m/boot/spinand/uboot does not exist"
	fi
fi

echo "build boot done"
