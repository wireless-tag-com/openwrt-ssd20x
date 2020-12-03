#!/bin/bash

while getopts "a:c:j:" opt; do
  case $opt in
    a)
		alkaid_dir=$OPTARG
		;;
    c)
		chip=$OPTARG
		;;
    j)
		jobs=$OPTARG
		;;
	\?)
	  echo "Invalid option: -$OPTARG" >&2
	  ;;
  esac
done
if [ "${jobs}" = "" ]
then
	echo
	echo "Set default 2 jobs for make"
	jobs=2
fi

kernel_dir=$PWD
uImage_dir=${kernel_dir}/arch/arm/boot
LD_ADDR=0x20008000
MKIMAGE_BIN=scripts/mkimage
MZ_BIN=scripts/mz
TOOL_DIR=scripts/dtc

function replace_dtb_to_kernel()
{
    echo CROSS_COMPILE=$1
    CC=$1
    echo dts-name=$2
    dtb_name=$2
    # build dtb
    ${CC}gcc -E -nostdinc -I${uImage_dir}/dts -I${uImage_dir}/dts/include -undef -D__DTS__ -x assembler-with-cpp -o ${uImage_dir}/dts/${dtb_name}.dtb.tmp ${uImage_dir}/dts/${dtb_name}.dts
    ${TOOL_DIR}/dtc -O dtb -o ${uImage_dir}/dts/${dtb_name}.dtb -b 0 ${uImage_dir}/dts/${dtb_name}.dtb.tmp
    python scripts/ms_builtin_dtb_update.py ${uImage_dir}/Image ${uImage_dir}/dts/${dtb_name}.dtb
    IMGNAME=$(strings -a -T binary ${uImage_dir}/Image | grep 'MVX' | grep 'LX'  | sed 's/\\*MVX/MVX/g' | cut -c 1-32)
    # uImage
    ${MKIMAGE_BIN} -A arm -O linux -T kernel -C none -a ${LD_ADDR} -e ${LD_ADDR} -n ${IMGNAME} -d ${uImage_dir}/Image ${uImage_dir}/uImage
    # xz Image
    xz -z -k -f ${uImage_dir}/Image
    ${MKIMAGE_BIN} -A arm -O linux -C lzma -a ${LD_ADDR} -e ${LD_ADDR} -n ${IMGNAME} -d ${uImage_dir}/Image.xz ${uImage_dir}/uImage.xz
    # mz Image
    ${MZ_BIN} c ${uImage_dir}/Image ${uImage_dir}/Image.mz
    ${MKIMAGE_BIN} -A arm -O linux -C mz -a ${LD_ADDR} -e ${LD_ADDR} -n ${IMGNAME} -d ${uImage_dir}/Image.mz ${uImage_dir}/uImage.mz
}

#   .d8888b. 88888888888     d8888 8888888b. 88888888888      8888888 888888888
#  d88P  Y88b    888        d88888 888   Y88b    888            888   888
#  Y88b.         888       d88P888 888    888    888            888   888
#   "Y888b.      888      d88P 888 888   d88P    888            888   8888888b.
#      "Y88b.    888     d88P  888 8888888P"     888            888        "Y88b
#        "888    888    d88P   888 888 T88b      888            888          888
#  Y88b  d88P    888   d8888888888 888  T88b     888            888   Y88b  d88P
#   "Y8888P"     888  d88P     888 888   T88b    888          8888888  "Y8888P"
if [ "${chip}" = "i5" ]
then
	########################## gclibc 4.8.3 ###############################
	TOOLCHAIN_PATH="/tools/toolchain/arm-linux-gnueabihf-4.8.3-201404/bin:$PATH"
	declare -x PATH=${TOOLCHAIN_PATH}:$PATH
	declare -x ARCH="arm"
	declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
	whereis ${CROSS_COMPILE}gcc

	RELEASE_PATH="${alkaid_dir}/project/kbuild/4.9.84/"

	echo "infinity5_ssc007a_s01a_spinand_defconfig"
	make infinity5_ssc007a_s01a_spinand_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007A -p ipc -f spinand -o glibc
	cd ${kernel_dir}

	echo "infinity5_ssc007a_s01a_defconfig"
	make infinity5_ssc007a_s01a_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007A -p ipc -f nor -o glibc
	cd ${kernel_dir}

	echo "infinity5_ssc007b_s01b_spinand_defconfig"
	make infinity5_ssc007b_s01b_spinand_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007B -p ipc -f spinand -o glibc
	cd ${kernel_dir}

	echo "infinity5_ssc007b_s01b_defconfig"
	make infinity5_ssc007b_s01b_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007B -p ipc -f nor -o glibc
	cd ${kernel_dir}

	echo "infinity5_ssc007a_s01a_spinand_uvc_defconfig"
	make infinity5_ssc007a_s01a_spinand_uvc_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007A -p usb_cam -f spinand -o glibc
	cd ${kernel_dir}

	echo "infinity5_ssc007a_s01a_uvc_defconfig"
	make infinity5_ssc007a_s01a_uvc_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007A -p usb_cam -f nor -o glibc
	cd ${kernel_dir}

	echo "infinity5_ssc007b_s01b_spinand_uvc_defconfig"
	make infinity5_ssc007b_s01b_spinand_uvc_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007B -p usb_cam -f spinand -o glibc
	cd ${kernel_dir}

	echo "infinity5_ssc007b_s01b_uvc_defconfig"
	make infinity5_ssc007b_s01b_uvc_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007B -p usb_cam -f nor -o glibc
	cd ${kernel_dir}

	echo "infinity5_ssc007a_s01a_coprocessor_defconfig"
	make infinity5_ssc007a_s01a_coprocessor_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007A -p cop -f nor -o glibc
	cd ${kernel_dir}

	echo "infinity5_ssc007a_s01a_spinand_coprocessor_defconfig"
	make infinity5_ssc007a_s01a_spinand_coprocessor_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007A -p cop -f spinand -o glibc
	cd ${kernel_dir}

	########################## uclibc ###############################
	TOOLCHAIN_PATH="/tools/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4/bin"
	declare -x PATH=${TOOLCHAIN_PATH}:$PATH
	declare -x ARCH="arm"
	declare -x CROSS_COMPILE="arm-buildroot-linux-uclibcgnueabihf-"
	whereis ${CROSS_COMPILE}gcc

	echo "infinity5_ssc007b_s01b_defconfig"
	make infinity5_ssc007b_s01b_defconfig; make clean; make -j ${jobs}
	cd ${RELEASE_PATH}
	./release.sh -k ${kernel_dir} -b 007B -p ipc -f nor -o uclibc
	cd ${kernel_dir}
fi

#   .d8888b. 88888888888     d8888 8888888b. 88888888888      8888888 .d8888b.
#  d88P  Y88b    888        d88888 888   Y88b    888            888  d88P  Y88b
#  Y88b.         888       d88P888 888    888    888            888  888
#   "Y888b.      888      d88P 888 888   d88P    888            888  888d888b.
#      "Y88b.    888     d88P  888 8888888P"     888            888  888P "Y88b
#        "888    888    d88P   888 888 T88b      888            888  888    888
#  Y88b  d88P    888   d8888888888 888  T88b     888            888  Y88b  d88P
#   "Y8888P"     888  d88P     888 888   T88b    888          8888888 "Y8888P"
if [ "${chip}" = "i6" ]
then
########################## gclibc 8.2.1 with glibc 2.28 ##########################
    TOOLCHAIN_PATH="/tools/toolchain/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin:$PATH"
    declare -x PATH=${TOOLCHAIN_PATH}:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
    whereis ${CROSS_COMPILE}gcc
    RELEASE_PATH="${alkaid_dir}/project/kbuild/4.9.84/"

    echo infinity6_defconfig
    make infinity6_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009A -p ipc -f nor -c i6 -l glibc -v 8.2.1
    cd ${kernel_dir}

    echo infinity6_uvc_defconfig
    make infinity6_uvc_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009A -p usbcam -f nor -c i6 -l glibc -v 8.2.1
    cd ${kernel_dir}

    echo infinity6_uvc_fastboot_defconfig
    make infinity6_uvc_fastboot_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009A -p usbcam-fastboot -f nor -c i6 -l glibc -v 8.2.1
    cd ${kernel_dir}

    echo infinity6_ssc009b_s01a_defconfig
    make infinity6_ssc009b_s01a_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009B -p ipc -f nor -c i6 -l glibc -v 8.2.1
    cd ${kernel_dir}

    echo infinity6_spinand_defconfig
    make infinity6_spinand_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009A -p ipc -f spinand -c i6 -l glibc -v 8.2.1
    cd ${kernel_dir}

    echo infinity6_spinand_uvc_defconfig
    make infinity6_spinand_uvc_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009A -p usbcam -f spinand -c i6 -l glibc -v 8.2.1
    cd ${kernel_dir}

    echo infinity6_ssc009b_s01a_spinand_defconfig
    make infinity6_ssc009b_s01a_spinand_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009B -p ipc -f spinand -c i6 -l glibc -v 8.2.1
    cd ${kernel_dir}

########################## uclibc GCC4.9.4 with uclibc 1.0.31 ##########################
	TOOLCHAIN_PATH="/tools/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4/bin:$PATH"
	declare -x PATH=${TOOLCHAIN_PATH}:$PATH
	declare -x ARCH="arm"
	declare -x CROSS_COMPILE="arm-buildroot-linux-uclibcgnueabihf-"
	whereis ${CROSS_COMPILE}gcc

    echo infinity6_defconfig
    make infinity6_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009A -p ipc -f nor -c i6 -l uclibc -v 4.9.4
    cd ${kernel_dir}

    echo infinity6_uvc_defconfig
    make infinity6_uvc_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009A -p usbcam -f nor -c i6 -l uclibc -v 4.9.4
    cd ${kernel_dir}

    echo infinity6_uvc_fastboot_defconfig
    make infinity6_uvc_fastboot_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009A -p usbcam-fastboot -f nor -c i6 -l uclibc -v 4.9.4
    cd ${kernel_dir}

    echo infinity6_ssc009b_s01a_defconfig
    make infinity6_ssc009b_s01a_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009B -p ipc -f nor -c i6 -l uclibc -v 4.9.4
    cd ${kernel_dir}

    # SPI_NOR cardv
    echo infinity6_cardv_defconfig
    make infinity6_cardv_defconfig;make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 009A -p cardv -f nor -c i6 -l uclibc -v 4.9.4
    cd ${kernel_dir}
fi

#   .d8888b. 88888888888     d8888 8888888b. 88888888888      d8b  .d8888b.              888                   888
#  d88P  Y88b    888        d88888 888   Y88b    888          Y8P d88P  Y88b             888                   888
#  Y88b.         888       d88P888 888    888    888              888                    888                   888
#   "Y888b.      888      d88P 888 888   d88P    888          888 888d888b.          .d88888 888  888  8888b.  888  .d88b.  .d8888b
#      "Y88b.    888     d88P  888 8888888P"     888          888 888P "Y88b        d88" 888 888  888     "88b 888 d88""88b 88K
#        "888    888    d88P   888 888 T88b      888          888 888    888        888  888 888  888 .d888888 888 888  888 "Y8888b.
#  Y88b  d88P    888   d8888888888 888  T88b     888          888 Y88b  d88P        Y88b 888 Y88b 888 888  888 888 Y88..88P      X88
#   "Y8888P"     888  d88P     888 888   T88b    888          888  "Y8888P" 88888888 "Y88888  "Y88888 "Y888888 888  "Y88P"   88888P'
if [ "${chip}" = "i6_dualos" ]
then

    # infinity6_ssc009a_s01a_lh_defconfig --> DualOS QFN88 for spinand
    # infinity6_ssc009a_s01a_lh_defconfig --> DualOS QFN88 for nor
    # infinity6_ssc009b_s01a_lh_defconfig --> DualOS QFN128 for spinand

    ########################## gclibc 8.2.1 ###############################
    TOOLCHAIN_PATH="/tools/toolchain/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin/"
    declare -x PATH=${TOOLCHAIN_PATH}:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
    env

    PRODUCT_TYPE="ipc-rtos"
    CHIP_TYPE="i6"
    CLIB="glibc"
    GCC_VERSION="8.2.1"
    API_VERSION="4.9.84"
    RELEASE_PATH="${alkaid_dir}/project/kbuild/${API_VERSION}/"

    # SPI_NAND Ramdisk 009A
    BOARD_TYPE="009A"
    FLASH_TYPE="spinand"
    KERNEL_DECONFIG="infinity6_ssc009a_s01a_lh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # SPI_NOR Ramdisk 009A
    BOARD_TYPE="009A"
    FLASH_TYPE="nor"
    KERNEL_DECONFIG="infinity6_ssc009a_s01a_lh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # SPI_NAND Ramdisk 009B
    BOARD_TYPE="009B"
    FLASH_TYPE="spinand"
    KERNEL_DECONFIG="infinity6_ssc009b_s01a_lh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

fi

#   .d8888b. 88888888888     d8888 8888888b. 88888888888      d8b  .d8888b.
#  d88P  Y88b    888        d88888 888   Y88b    888          Y8P d88P  Y88b
#  Y88b.         888       d88P888 888    888    888                     888
#   "Y888b.      888      d88P 888 888   d88P    888          888      .d88P 88888b.d88b.
#      "Y88b.    888     d88P  888 8888888P"     888          888  .od888P"  888 "888 "88b
#        "888    888    d88P   888 888 T88b      888          888 d88P"      888  888  888
#  Y88b  d88P    888   d8888888888 888  T88b     888          888 888"       888  888  888
#   "Y8888P"     888  d88P     888 888   T88b    888          888 888888888  888  888  888

if [ "${chip}" = "i2m" ]
then
    declare -x PATH="/tools/toolchain/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin/:$PATH"
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
    echo CROSS_COMPILE=$CROSS_COMPILE
    whereis ${CROSS_COMPILE}gcc
    GCC_VERSION=$(${CROSS_COMPILE}gcc --version | head -n 1 | sed -e 's/.*\([0-9]\.[0-9]\.[0-9]\).*/\1/')
    echo GCC_VERSION=${GCC_VERSION}

    # P2 SPI_NAND
    echo "infinity2m_spinand_ssc011a_s01a_minigui_defconfig"
    make infinity2m_spinand_ssc011a_s01a_minigui_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 011A -p nvr -f spinand -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # P2 SPI_NAND fastboot
    echo "infinity2m_spinand_ssc011a_s01a_minigui_fastboot_defconfig"
    make infinity2m_spinand_ssc011a_s01a_minigui_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 011A-fastboot -p nvr -f spinand -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # P2 SPI_NOR
    echo "infinity2m_ssc011a_s01a_minigui_defconfig"
    make infinity2m_ssc011a_s01a_minigui_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 011A -p nvr -f nor -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # P2 SPI_NOR fastboot
    echo "infinity2m_ssc011a_s01a_fastboot_defconfig"
    make infinity2m_ssc011a_s01a_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 011A-fastboot -p nvr -f nor -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    ##################### uclibc relese ###########################
    declare -x PATH="/tools/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4/bin:$PATH"
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-buildroot-linux-uclibcgnueabihf-"
    echo CROSS_COMPILE=$CROSS_COMPILE
    whereis ${CROSS_COMPILE}gcc
    GCC_VERSION=$(${CROSS_COMPILE}gcc --version | head -n 1 | sed -e 's/.*\([0-9]\.[0-9]\.[0-9]\).*/\1/')
    echo GCC_VERSION=${GCC_VERSION}

    # P2 SPI_NAND
    echo "infinity2m_spinand_ssc011a_s01a_minigui_defconfig"
    make infinity2m_spinand_ssc011a_s01a_minigui_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 011A -p nvr -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # P2 SPI_NAND fastboot
    echo "infinity2m_spinand_ssc011a_s01a_minigui_fastboot_defconfig"
    make infinity2m_spinand_ssc011a_s01a_minigui_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 011A-fastboot -p nvr -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # P2 SPI_NOR
    echo "infinity2m_ssc011a_s01a_minigui_defconfig"
    make infinity2m_ssc011a_s01a_minigui_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 011A -p nvr -f nor -c ${chip} -l uclibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # P2 SPI_NOR fastboot
    echo "infinity2m_ssc011a_s01a_fastboot_defconfig"
    make infinity2m_ssc011a_s01a_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 011A-fastboot -p nvr -f nor -c ${chip} -l uclibc -v ${GCC_VERSION}
    cd ${kernel_dir}

fi


#  d8b  .d8888b.                            888                   888
#  Y8P d88P  Y88b                           888                   888
#             888                           888                   888
#  888      .d88P 88888b.d88b.          .d88888 888  888  8888b.  888  .d88b.  .d8888b          8888b.  88888b.d88b.  88888b.
#  888  .od888P"  888 "888 "88b        d88" 888 888  888     "88b 888 d88""88b 88K                 "88b 888 "888 "88b 888 "88b
#  888 d88P"      888  888  888 888888 888  888 888  888 .d888888 888 888  888 "Y8888b. 888888 .d888888 888  888  888 888  888
#  888 888"       888  888  888        Y88b 888 Y88b 888 888  888 888 Y88..88P      X88        888  888 888  888  888 888 d88P
#  888 888888888  888  888  888         "Y88888  "Y88888 "Y888888 888  "Y88P"   88888P'        "Y888888 888  888  888 88888P"
#                                                                                                                     888
#                                                                                                                     888
#                                                                                                                     888

if [ "${chip}" = "i2m-dualos-amp" ]
then
    declare -x PATH="/tools/toolchain/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin/:$PATH"
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
    echo CROSS_COMPILE=$CROSS_COMPILE
    whereis ${CROSS_COMPILE}gcc
    GCC_VERSION=$(${CROSS_COMPILE}gcc --version | head -n 1 | sed -e 's/.*\([0-9]\.[0-9]\.[0-9]\).*/\1/')
    echo GCC_VERSION=${GCC_VERSION}

    # NVR SPI_NAND
    echo "infinity2m_spinand_ssc010a_s01a_swtoe_defconfig"
    make infinity2m_spinand_ssc010a_s01a_swtoe_defconfig
    #echo "infinity2m_spinand_ssc010a_s01a_amp_defconfig"
    #make infinity2m_spinand_ssc010a_s01a_amp_defconfig

    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 010A -p nvr-rtos -f spinand -c i2m -l glibc -v ${GCC_VERSION} -i 4.9.84
    cd ${kernel_dir}
fi




#   .d8888b. 88888888888     d8888 8888888b. 88888888888      d8b  .d8888b.
#  d88P  Y88b    888        d88888 888   Y88b    888          Y8P d88P  Y88b
#  Y88b.         888       d88P888 888    888    888              888
#   "Y888b.      888      d88P 888 888   d88P    888          888 888d888b.   .d88b.
#      "Y88b.    888     d88P  888 8888888P"     888          888 888P "Y88b d8P  Y8b
#        "888    888    d88P   888 888 T88b      888          888 888    888 88888888
#  Y88b  d88P    888   d8888888888 888  T88b     888          888 Y88b  d88P Y8b.
#   "Y8888P"     888  d88P     888 888   T88b    888          888  "Y8888P"   "Y8888
#

if [ "${chip}" = "i6e" ]
then
    ########################## gclibc 8.2.1 ###############################
    TOOLCHAIN_PATH="/tools/toolchain/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin/"
    declare -x PATH=${TOOLCHAIN_PATH}:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
    whereis ${CROSS_COMPILE}gcc

    GCC_VERSION=8.2.1
    API_VERSION="4.9.84"
    RELEASE_PATH="${alkaid_dir}/project/kbuild/${API_VERSION}/"

    # cardv DEMO SPI_NOR
    echo make infinity6e_cardv_defconfig [${GCC_VERSION}]
    make infinity6e_cardv_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b DEMO -p cardv -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    ########################## gclibc 9.1.0 ###############################
    TOOLCHAIN_PATH="/tools/toolchain/gcc-sigmastar-9.1.0-2019.11-x86_64_arm-linux-gnueabihf/bin/"
    declare -x PATH=${TOOLCHAIN_PATH}:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-9.1.0-"
    whereis ${CROSS_COMPILE}gcc

    GCC_VERSION=9.1.0

    # cardv DEMO SPI_NAND
    echo make infinity6e_cardv_spinand_defconfig [${GCC_VERSION}]
    make infinity6e_cardv_spinand_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b DEMO -p cardv -f nand -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # cardv DEMO SPI_NOR
    echo make infinity6e_cardv_defconfig [${GCC_VERSION}]
    make infinity6e_cardv_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b DEMO -p cardv -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 012B SPI_NOR
    echo make infinity6e_ssc012b_s01a_defconfig
    make infinity6e_ssc012b_s01a_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p ipc -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NOR
    echo make infinity6e_ssc013a_s01a_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p ipc -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 015A SPI_NOR
    echo make infinity6e_ssc015a_s01a_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc015a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 015A -p ipc -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 012B SPI_NAND
    echo make infinity6e_ssc012b_s01a_spinand_defconfig
    make infinity6e_ssc012b_s01a_spinand_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p ipc -f spinand -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NAND
    echo make infinity6e_ssc013a_s01a_spinand_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p ipc -f spinand -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 015A SPI_NAND
    echo make infinity6e_ssc015a_s01a_spinand_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc015a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 015A -p ipc -f spinand -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    ## ipcam fastboot
    # 012B SPI_NOR
    echo make infinity6e_ssc012b_s01a_fastboot_defconfig
    make infinity6e_ssc012b_s01a_fastboot_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p ipc-fastboot -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NOR
    echo make infinity6e_ssc013a_s01a_fastboot_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p ipc-fastboot -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 012B SPI_NAND
    echo make infinity6e_ssc012b_s01a_spinand_fastboot_defconfig
    make infinity6e_ssc012b_s01a_spinand_fastboot_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p ipc-fastboot -f spinand -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NAND
    echo make infinity6e_ssc013a_s01a_spinand_fastboot_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p ipc-fastboot -f spinand -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    ##### for usbcam
    # 012B SPI_NOR
    echo make infinity6e_ssc012b_s01a_usbcam_defconfig
    make infinity6e_ssc012b_s01a_usbcam_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p usbcam -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NOR
    echo make infinity6e_ssc013a_s01a_usbcam_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p usbcam -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 012B SPI_NAND
    echo make infinity6e_ssc012b_s01a_spinand_usbcam_defconfig
    make infinity6e_ssc012b_s01a_spinand_usbcam_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p usbcam -f spinand -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NAND
    echo make infinity6e_ssc013a_s01a_spinand_usbcam_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p usbcam -f spinand -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    ## usbcam fastboot
    echo make infinity6e_ssc012b_s01a_usbcam_fastboot_defconfig
    make infinity6e_ssc012b_s01a_usbcam_fastboot_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p usbcam-fastboot -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NOR
    echo make infinity6e_ssc013a_s01a_usbcam_fastboot_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p usbcam-fastboot -f nor -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 012B SPI_NAND
    echo make infinity6e_ssc012b_s01a_spinand_usbcam_fastboot_defconfig
    make infinity6e_ssc012b_s01a_spinand_usbcam_fastboot_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p usbcam-fastboot -f spinand -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NAND
    echo make infinity6e_ssc013a_s01a_spinand_usbcam_fastboot_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p usbcam-fastboot -f spinand -c ${chip} -l glibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}
    ########################## uclibc ###############################
    TOOLCHAIN_PATH="/tools/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4/bin"
    declare -x PATH=${TOOLCHAIN_PATH}:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-buildroot-linux-uclibcgnueabihf-"
    whereis ${CROSS_COMPILE}gcc
    GCC_VERSION=$(${CROSS_COMPILE}gcc --version | head -n 1 | sed -e 's/.*\([0-9]\.[0-9]\.[0-9]\).*/\1/')

    # 012B SPI_NOR
    echo make infinity6e_ssc012b_s01a_defconfig
    make infinity6e_ssc012b_s01a_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p ipc -f nor -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NOR
    echo make infinity6e_ssc013a_s01a_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p ipc -f nor -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 015A SPI_NOR
    echo make infinity6e_ssc015a_s01a_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc015a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 015A -p ipc -f nor -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}


    # 012B SPI_NAND
    echo make infinity6e_ssc012b_s01a_spinand_defconfig
    make infinity6e_ssc012b_s01a_spinand_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p ipc -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NAND
    echo make infinity6e_ssc013a_s01a_spinand_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p ipc -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 015A SPI_NAND
    echo make infinity6e_ssc015a_s01a_spinand_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc015a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 015A -p ipc -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    ## ipcam fastboot
    # 012B SPI_NOR
    echo make infinity6e_ssc012b_s01a_fastboot_defconfig
    make infinity6e_ssc012b_s01a_fastboot_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p ipc-fastboot -f nor -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NOR
    echo make infinity6e_ssc013a_s01a_fastboot_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p ipc-fastboot -f nor -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 012B SPI_NAND
    echo make infinity6e_ssc012b_s01a_spinand_fastboot_defconfig
    make infinity6e_ssc012b_s01a_spinand_fastboot_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B-fastboot -p ipc -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NAND
    echo make infinity6e_ssc013a_s01a_spinand_fastboot_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p ipc-fastboot -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    ## usbcam
    # 012B SPI_NOR
    echo make infinity6e_ssc012b_s01a_usbcam_defconfig
    make infinity6e_ssc012b_s01a_usbcam_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p usbcam -f nor -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NOR
    echo make infinity6e_ssc013a_s01a_usbcam_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p usbcam -f nor -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 012B SPI_NAND
    echo make infinity6e_ssc012b_s01a_spinand_usbcam_defconfig
    make infinity6e_ssc012b_s01a_spinand_usbcam_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p usbcam -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NAND
    echo make infinity6e_ssc013a_s01a_spinand_usbcam_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p usbcam -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    ## usbcam fastboot
    # 012B SPI_NOR
    echo make infinity6e_ssc012b_s01a_usbcam_fastboot_defconfig
    make infinity6e_ssc012b_s01a_usbcam_fastboot_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p usbcam-fastboot -f nor -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NOR
    echo make infinity6e_ssc013a_s01a_usbcam_fastboot_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p usbcam-fastboot -f nor -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 012B SPI_NAND
    echo make infinity6e_ssc012b_s01a_spinand_usbcam_fastboot_defconfig
    make infinity6e_ssc012b_s01a_spinand_usbcam_fastboot_defconfig; make clean; make -j ${jobs}
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 012B -p usbcam-fastboot -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}

    # 013A SPI_NAND
    echo make infinity6e_ssc013a_s01a_spinand_usbcam_fastboot_defconfig
    replace_dtb_to_kernel ${CROSS_COMPILE} infinity6e-ssc013a-s01a
    cd ${RELEASE_PATH}
    ./release.sh -k ${kernel_dir} -b 013A -p usbcam-fastboot -f spinand -c ${chip} -l uclibc -v ${GCC_VERSION} -i ${API_VERSION}
    cd ${kernel_dir}
fi

#  d8b  .d8888b.                       888                   888
#  Y8P d88P  Y88b                      888                   888
#      888                             888                   888
#  888 888d888b.   .d88b.          .d88888 888  888  8888b.  888  .d88b.  .d8888b          8888b.  88888b.d88b.  88888b.
#  888 888P "Y88b d8P  Y8b        d88" 888 888  888     "88b 888 d88""88b 88K                 "88b 888 "888 "88b 888 "88b
#  888 888    888 88888888 888888 888  888 888  888 .d888888 888 888  888 "Y8888b. 888888 .d888888 888  888  888 888  888
#  888 Y88b  d88P Y8b.            Y88b 888 Y88b 888 888  888 888 Y88..88P      X88        888  888 888  888  888 888 d88P
#  888  "Y8888P"   "Y8888          "Y88888  "Y88888 "Y888888 888  "Y88P"   88888P'        "Y888888 888  888  888 88888P"
#                                                                                                                888
#                                                                                                                888
#                                                                                                                888
if [ "${chip}" = "i6e-dualos-amp" ]
then
    # infinity6e_ssc012b_s01a_spinand_amp_defconfig -> dualOS QFN for spinand
    # infinity6e_ssc013a_s01a_spinand_amp_defconfig -> dualOS BGA for spinand

    ########################## gclibc 8.2.1 ###############################
    TOOLCHAIN_PATH="/tools/toolchain/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin/"
    declare -x PATH=${TOOLCHAIN_PATH}:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
    env

    FLASH_TYPE="spinand"
    CHIP_TYPE="i6e"
    CLIB="glibc"
    GCC_VERSION="8.2.1"
    API_VERSION="4.9.84"
    RELEASE_PATH="${alkaid_dir}/project/kbuild/${API_VERSION}/"

    # 012B SPI_NAND
    BOARD_TYPE="012B"
    PRODUCT_TYPE="ipc-rtos"
    KERNEL_DECONFIG="infinity6e_ssc012b_s01a_spinand_amp_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # 013A SPI_NAND
    BOARD_TYPE="013A"
    PRODUCT_TYPE="ipc-rtos"
    KERNEL_DECONFIG="infinity6e_ssc013a_s01a_spinand_amp_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    ##### for usbcam
    # 012B SPI_NAND (for usbcam)
    BOARD_TYPE="012B"
    PRODUCT_TYPE="usbcam-rtos"
    KERNEL_DECONFIG="infinity6e_ssc012b_s01a_spinand_amp_usbcam_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # 013A SPI_NAND (for usbcam)
    BOARD_TYPE="013A"
    PRODUCT_TYPE="usbcam-rtos"
    KERNEL_DECONFIG="infinity6e_ssc013a_s01a_spinand_amp_usbcam_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    ########################## gclibc 9.1.0 ###############################
    TOOLCHAIN_PATH="/tools/toolchain/gcc-sigmastar-9.1.0-2019.11-x86_64_arm-linux-gnueabihf/bin/"
    declare -x PATH=${TOOLCHAIN_PATH}:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-9.1.0-"
    env

    FLASH_TYPE="spinand"
    CHIP_TYPE="i6e"
    CLIB="glibc"
    GCC_VERSION="9.1.0"
    API_VERSION="4.9.84"
    RELEASE_PATH="${alkaid_dir}/project/kbuild/${API_VERSION}/"

    # 012B SPI_NAND
    BOARD_TYPE="012B"
    PRODUCT_TYPE="ipc-rtos"
    KERNEL_DECONFIG="infinity6e_ssc012b_s01a_spinand_amp_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # 013A SPI_NAND
    BOARD_TYPE="013A"
    PRODUCT_TYPE="ipc-rtos"
    KERNEL_DECONFIG="infinity6e_ssc013a_s01a_spinand_amp_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    ##### for usbcam
    # 012B SPI_NAND (for usbcam)
    BOARD_TYPE="012B"
    PRODUCT_TYPE="usbcam-rtos"
    KERNEL_DECONFIG="infinity6e_ssc012b_s01a_spinand_amp_usbcam_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # 013A SPI_NAND (for usbcam)
    BOARD_TYPE="013A"
    PRODUCT_TYPE="usbcam-rtos"
    KERNEL_DECONFIG="infinity6e_ssc013a_s01a_spinand_amp_usbcam_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

fi

#  d8b  .d8888b.                       888                   888                                                          888 888
#  Y8P d88P  Y88b                      888                   888                                                          888 888
#      888                             888                   888                                                          888 888
#  888 888d888b.   .d88b.          .d88888 888  888  8888b.  888  .d88b.  .d8888b         .d8888b  88888b.d88b.  88888b.  888 88888b.
#  888 888P "Y88b d8P  Y8b        d88" 888 888  888     "88b 888 d88""88b 88K             88K      888 "888 "88b 888 "88b 888 888 "88b
#  888 888    888 88888888 888888 888  888 888  888 .d888888 888 888  888 "Y8888b. 888888 "Y8888b. 888  888  888 888  888 888 888  888
#  888 Y88b  d88P Y8b.            Y88b 888 Y88b 888 888  888 888 Y88..88P      X88             X88 888  888  888 888 d88P 888 888  888
#  888  "Y8888P"   "Y8888          "Y88888  "Y88888 "Y888888 888  "Y88P"   88888P'         88888P' 888  888  888 88888P"  888 888  888
#                                                                                                                888
#                                                                                                                888
#                                                                                                                888

if [ "${chip}" = "i6e-dualos-smplh" ]
then
    # infinity6e_ssc012b_s01a_spinand_smplh_defconfig -> dualOS QFN for spinand
    # infinity6e_ssc013a_s01a_spinand_smplh_defconfig -> dualOS BGA for spinand

    ########################## gclibc 8.2.1 ###############################
    TOOLCHAIN_PATH="/tools/toolchain/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin/"
    declare -x PATH=${TOOLCHAIN_PATH}:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
    env

    PRODUCT_TYPE="ipc-rtos-smplh"
    FLASH_TYPE="spinand"
    CHIP_TYPE="i6e"
    CLIB="glibc"
    GCC_VERSION="8.2.1"
    API_VERSION="4.9.84"
    RELEASE_PATH="${alkaid_dir}/project/kbuild/${API_VERSION}/"

    # 012B SPI_NAND
    BOARD_TYPE="012B"
    KERNEL_DECONFIG="infinity6e_ssc012b_s01a_spinand_smplh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # 013A SPI_NAND
    BOARD_TYPE="013A"
    KERNEL_DECONFIG="infinity6e_ssc013a_s01a_spinand_smplh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    ########################## gclibc 9.1.0 ###############################
    TOOLCHAIN_PATH="/tools/toolchain/gcc-sigmastar-9.1.0-2019.11-x86_64_arm-linux-gnueabihf/bin/"
    declare -x PATH=${TOOLCHAIN_PATH}:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-9.1.0-"
    env

    PRODUCT_TYPE="ipc-rtos-smplh"
    FLASH_TYPE="spinand"
    CHIP_TYPE="i6e"
    CLIB="glibc"
    GCC_VERSION="9.1.0"
    API_VERSION="4.9.84"
    RELEASE_PATH="${alkaid_dir}/project/kbuild/${API_VERSION}/"

    # 012B SPI_NAND
    BOARD_TYPE="012B"
    KERNEL_DECONFIG="infinity6e_ssc012b_s01a_spinand_smplh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # 013A SPI_NAND
    BOARD_TYPE="013A"
    KERNEL_DECONFIG="infinity6e_ssc013a_s01a_spinand_smplh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

fi

#   .d8888b. 88888888888     d8888 8888888b. 88888888888      8888888 .d8888b.  888888b.    .d8888b.
#  d88P  Y88b    888        d88888 888   Y88b    888            888  d88P  Y88b 888  "88b  d88P  Y88b
#  Y88b.         888       d88P888 888    888    888            888  888        888  .88P  888    888
#   "Y888b.      888      d88P 888 888   d88P    888            888  888d888b.  8888888K.  888    888
#      "Y88b.    888     d88P  888 8888888P"     888            888  888P "Y88b 888  "Y88b 888    888
#        "888    888    d88P   888 888 T88b      888            888  888    888 888    888 888    888
#  Y88b  d88P    888   d8888888888 888  T88b     888            888  Y88b  d88P 888   d88P Y88b  d88P
#   "Y8888P"     888  d88P     888 888   T88b    888          8888888 "Y8888P"  8888888P"   "Y8888P"
#

if [ "${chip}" = "i6b0" ]
then
    declare -x PATH=/tools/toolchain/gcc-sigmastar-9.1.0-2019.11-x86_64_arm-linux-gnueabihf/bin/:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-9.1.0-"
    echo CROSS_COMPILE=$CROSS_COMPILE
    whereis ${CROSS_COMPILE}gcc
    GCC_VERSION=$(${CROSS_COMPILE}gcc --version | head -n 1 | sed -e 's/.*\([0-9]\.[0-9]\.[0-9]\).*/\1/')
    echo GCC_VERSION=${GCC_VERSION}

    ## For ipcam
    # qfn88  009A SPI_NOR
    echo "infinity6b0_ssc009a_s01a_defconfig"
    make infinity6b0_ssc009a_s01a_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p ipc -f nor -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn88  009A SPI_NAND
    echo "infinity6b0_ssc009a_s01a_spinand_defconfig"
    make infinity6b0_ssc009a_s01a_spinand_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p ipc -f spinand -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn88  009D SPI_NOR
    echo "infinity6b0_ssc009d_s01a_defconfig"
    make infinity6b0_ssc009d_s01a_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009D -p ipc -f nor -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn88  009D SPI_NAND
    echo "infinity6b0_ssc009d_s01a_spinand_defconfig"
    make infinity6b0_ssc009d_s01a_spinand_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009D -p ipc -f spinand -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn128  009B SPI_NOR
    echo "infinity6b0_ssc009b_s01a_defconfig"
    make infinity6b0_ssc009b_s01a_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p ipc -f nor -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn128  009B SPI_NAND
    echo "infinity6b0_ssc009b_s01a_spinand_defconfig"
    make infinity6b0_ssc009b_s01a_spinand_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p ipc -f spinand -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    ## For ipcam fastboot
    # qfn88  009A SPI_NOR
    echo "infinity6b0_ssc009a_s01a_fastboot_defconfig"
    make infinity6b0_ssc009a_s01a_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p ipc-fastboot -f nor -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn88  009A SPI_NAND
    echo "infinity6b0_ssc009a_s01a_spinand_fastboot_defconfig"
    make infinity6b0_ssc009a_s01a_spinand_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p ipc-fastboot -f spinand -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn128  009B SPI_NOR
    echo "infinity6b0_ssc009b_s01a_fastboot_defconfig"
    make infinity6b0_ssc009b_s01a_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p ipc-fastboot -f nor -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn128  009B SPI_NAND
    echo "infinity6b0_ssc009b_s01a_spinand_fastboot_defconfig"
    make infinity6b0_ssc009b_s01a_spinand_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p ipc-fastboot -f spinand -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    ## For usbcam
    # qfn88  009A SPI_NOR
    echo "infinity6b0_ssc009a_s01a_usbcam_defconfig"
    make infinity6b0_ssc009a_s01a_usbcam_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p usbcam -f nor -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn88  009A SPI_NAND
    echo "infinity6b0_ssc009a_s01a_spinand_usbcam_defconfig"
    make infinity6b0_ssc009a_s01a_spinand_usbcam_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p usbcam -f spinand -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn128  009B SPI_NOR
    echo "infinity6b0_ssc009b_s01a_usbcam_defconfig"
    make infinity6b0_ssc009b_s01a_usbcam_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p usbcam -f nor -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    # qfn128  009B SPI_NAND
    echo "infinity6b0_ssc009b_s01a_spinand_usbcam_defconfig"
    make infinity6b0_ssc009b_s01a_spinand_usbcam_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p usbcam -f spinand -c ${chip} -l glibc -v ${GCC_VERSION}
    cd ${kernel_dir}

    declare -x PATH="/tools/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4/bin/:$PATH"
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-buildroot-linux-uclibcgnueabihf-"

    ## For ipcam
    # qfn88  009A SPI_NOR
    echo "infinity6b0_ssc009a_s01a_defconfig"
    make infinity6b0_ssc009a_s01a_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p ipc -f nor -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    echo "infinity6b0_ssc009a_s01a_spinand_defconfig"
    make infinity6b0_ssc009a_s01a_spinand_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p ipc -f spinand -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    # qfn128  009B SPI_NOR
    echo "infinity6b0_ssc009b_s01a_defconfig"
    make infinity6b0_ssc009b_s01a_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p ipc -f nor -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    # qfn128  009B SPI_NAND
    echo "infinity6b0_ssc009b_s01a_spinand_defconfig"
    make infinity6b0_ssc009b_s01a_spinand_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p ipc -f spinand -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    ## for ipcam fastboot
    # qfn88  009A SPI_NOR
    echo "infinity6b0_ssc009a_s01a_fastboot_defconfig"
    make infinity6b0_ssc009a_s01a_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p ipc-fastboot -f nor -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    echo "infinity6b0_ssc009a_s01a_spinand_fastboot_defconfig"
    make infinity6b0_ssc009a_s01a_spinand_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p ipc-fastboot -f spinand -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    # qfn128  009B SPI_NOR
    echo "infinity6b0_ssc009b_s01a_fastboot_defconfig"
    make infinity6b0_ssc009b_s01a_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p ipc-fastboot -f nor -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    # qfn128  009B SPI_NAND
    echo "infinity6b0_ssc009b_s01a_spinand_fastboot_defconfig"
    make infinity6b0_ssc009b_s01a_spinand_fastboot_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p ipc-fastboot -f spinand -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    ## For usbcam
    # qfn88  009A SPI_NOR
    echo "infinity6b0_ssc009a_s01a_usbcam_defconfig"
    make infinity6b0_ssc009a_s01a_usbcam_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p usbcam -f nor -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    echo "infinity6b0_ssc009a_s01a_spinand_usbcam_defconfig"
    make infinity6b0_ssc009a_s01a_spinand_usbcam_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009A -p usbcam -f spinand -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    # qfn128  009B SPI_NOR
    echo "infinity6b0_ssc009b_s01a_usbcam_defconfig"
    make infinity6b0_ssc009b_s01a_usbcam_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p usbcam -f nor -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}

    # qfn128  009B SPI_NAND
    echo "infinity6b0_ssc009b_s01a_spinand_usbcam_defconfig"
    make infinity6b0_ssc009b_s01a_spinand_usbcam_defconfig
    make clean;make -j ${jobs}
    cd ${alkaid_dir}/project/kbuild/4.9.84/
    ./release.sh -k ${kernel_dir} -b 009B -p usbcam -f spinand -c ${chip} -l uclibc -v 4.9.4
    cd ${kernel_dir}
fi

#   .d8888b. 88888888888     d8888 8888888b. 88888888888      8888888 .d8888b.  888888b.    .d8888b.             888                   888
#  d88P  Y88b    888        d88888 888   Y88b    888            888  d88P  Y88b 888  "88b  d88P  Y88b             888                   888
#  Y88b.         888       d88P888 888    888    888            888  888        888  .88P  888    888             888                   888
#   "Y888b.      888      d88P 888 888   d88P    888            888  888d888b.  8888888K.  888    888         .d88888 888  888  8888b.  888  .d88b.  .d8888b
#      "Y88b.    888     d88P  888 8888888P"     888            888  888P "Y88b 888  "Y88b 888    888        d88" 888 888  888     "88b 888 d88""88b 88K
#        "888    888    d88P   888 888 T88b      888            888  888    888 888    888 888    888        888  888 888  888 .d888888 888 888  888 "Y8888b.
#  Y88b  d88P    888   d8888888888 888  T88b     888            888  Y88b  d88P 888   d88P Y88b  d88P        Y88b 888 Y88b 888 888  888 888 Y88..88P      X88
#   "Y8888P"     888  d88P     888 888   T88b    888          8888888 "Y8888P"  8888888P"   "Y8888P"88888888 "Y88888  "Y88888 "Y888888 888  "Y88P"   88888P'
if [ "${chip}" = "i6b0_dualos" ]
then

    # infinity6b0_ssc009a_s01a_lh_defconfig --> DualOS QFN88 for spinand
    # infinity6b0_ssc009a_s01a_lh_defconfig --> DualOS QFN88 for nor
    # infinity6b0_ssc009b_s01a_lh_defconfig --> DualOS QFN128 for spinand

    ########################## gclibc 9.1.0 ###############################
    TOOLCHAIN_PATH="/tools/toolchain/gcc-sigmastar-9.1.0-2019.11-x86_64_arm-linux-gnueabihf/bin/"
    declare -x PATH=${TOOLCHAIN_PATH}:$PATH
    declare -x ARCH="arm"
    declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
    env

    PRODUCT_TYPE="ipc-rtos"
    CHIP_TYPE="i6b0"
    CLIB="glibc"
    GCC_VERSION="9.1.0"
    API_VERSION="4.9.84"
    RELEASE_PATH="${alkaid_dir}/project/kbuild/${API_VERSION}/"

    # SPI_NAND Ramdisk 009A
    BOARD_TYPE="009A"
    FLASH_TYPE="spinand"
    KERNEL_DECONFIG="infinity6b0_ssc009a_s01a_lh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # SPI_NOR Ramdisk 009A
    BOARD_TYPE="009A"
    FLASH_TYPE="nor"
    KERNEL_DECONFIG="infinity6b0_ssc009a_s01a_lh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # SPI_NAND Ramdisk 009B
    BOARD_TYPE="009B"
    FLASH_TYPE="spinand"
    KERNEL_DECONFIG="infinity6b0_ssc009b_s01a_lh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

    # SPI_NOR Ramdisk 009B
    BOARD_TYPE="009B"
    FLASH_TYPE="nor"
    KERNEL_DECONFIG="infinity6b0_ssc009b_s01a_lh_defconfig"
    RELEASE_SCRIPT="./release.sh -k ${kernel_dir} -b ${BOARD_TYPE} -p ${PRODUCT_TYPE} -f ${FLASH_TYPE} -c ${CHIP_TYPE} -l ${CLIB} -v ${GCC_VERSION} -i ${API_VERSION}"
    echo ${CHIP_TYPE}/${BOARD_TYPE}/${PRODUCT_TYPE}/${FLASH_TYPE}/${CLIB}/GCC_VERSION=${GCC_VERSION}/API_VERSION=${API_VERSION}
    echo make ${KERNEL_DECONFIG}
    make ${KERNEL_DECONFIG}
    make clean;make -j ${jobs}
    cd ${RELEASE_PATH}
    echo ${RELEASE_SCRIPT}
    ${RELEASE_SCRIPT}
    cd ${kernel_dir}

fi