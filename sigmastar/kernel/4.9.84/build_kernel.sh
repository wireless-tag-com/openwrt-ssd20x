#!/bin/bash

while getopts "f:q:" opt; do
	case $opt in
		f)
			flashtype=$OPTARG
			;;
		q)
			fastboot=$OPTARG
			;;
		\?)
			echo "Invalid option: - $OPTARG"
			;;
	esac
done

declare -x ARCH="arm"
declare -x CROSS_COMPILE="arm-linux-gnueabihf-"

if [ "${flashtype}" = "nor" ]; then
	if [ "${fastboot}" = "fastboot" ];then
		echo "make nor fastboot kernel defconfig"
		make infinity2m_ssc011a_s01a_fastboot_defconfig
	else
		echo "make nor normal kernel defconfig"
		make infinity2m_ssc011a_s01a_display_for_mipi_defconfig
	fi
else
	if [ "${fastboot}" = "fastboot" ];then
		echo "make nand fastboot kernel defconfig"
		make infinity2m_spinand_ssc011a_s01a_minigui_fastboot_defconfig
	else
		echo "make  nand normal kernel defconfig"
		make infinity2m_spinand_ssc011a_s01a_minigui_defconfig
	fi
fi

make clean;make -j8

echo "build kernel done"
