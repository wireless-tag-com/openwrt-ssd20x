#!/bin/bash

while getopts "i:a:x" opt; do
  case $opt in
	a)
    	arch=$OPTARG
		;;
	i)
    	api_version=$OPTARG
		;;
	x)
    	XIP=1
		;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done

#kl_dir=$1
#api_version=$2

if [ "${arch}" = "" ] 
then
	echo
#	echo "  ARCH is not specified via -a parameter, set to arm !!"
	echo
	arch=arm
fi


		 	
kl_dir=$(pwd)
XIP_PREFIX=

IMAGE=Image

if [ "${XIP}" = "1" ]; then
    IMAGE=xipImage
    XIP_PREFIX='xip'
fi


TMP=.pack_tmp
rm -Rf ${TMP}
#rm -Rf ${dst_dir}
mkdir -p ${TMP}



kernel_id=$(strings ${kl_dir}/arch/${arch}/boot/${IMAGE} | grep MVX.*KL_LX | cut -b 17-28 | sed -n 's/KL_\([^#]*\)#*/\1/p')
platform_id=$(strings ${kl_dir}/arch/${arch}/boot/${IMAGE} | grep MVX.*KL_LX | cut -b 7-8)
commit=$(strings ${kl_dir}/arch/${arch}/boot/${IMAGE} | grep MVX.*KL_LX | cut -b 10-16)

MODULE_DIR=${XIP_PREFIX}modules

echo "  Packing modules to '${kl_dir}/${MODULE_DIR}'"
echo
	
	rm -Rf ${TMP}/${platform_id}/${kernel_id}/${MODULE_DIR}
	mkdir -p ${TMP}/${platform_id}/${kernel_id}/${MODULE_DIR}
	rm -Rf ${kl_dir}/${MODULE_DIR}
	exec < ${kl_dir}/modules.order
	while read line
	do
	    module_file=$(echo ${line} | sed -e 's/^kernel\///g')
	    #echo ${module_file}
	    file_name=$(echo ${module_file} | sed 's/.*\///g')
	    #echo f=${file_name}
	    if [ -e "${TMP}/${platform_id}/${kernel_id}/${MODULE_DIR}/${file_name}" ]
	    then
		      echo "  ERROR!! Duplicated module file: ${module_file} "
		      echo "  Please check module list to avoid this problem. "
	          echo " Aborted"
		      exit -1 
	    fi
#	    echo "    cp -f ${kl_dir}/${module_file} ${TMP}/${platform_id}/${kernel_id}/${MODULE_DIR}/${file_name}"
	    cp -f ${kl_dir}/${module_file} ${TMP}/${platform_id}/${kernel_id}/${MODULE_DIR}/${file_name}
	    echo ${file_name} >> ${TMP}/${platform_id}/${kernel_id}/${MODULE_DIR}/modules.list
	done
        cp -Rf ${TMP}/${platform_id}/${kernel_id}/${MODULE_DIR} ${kl_dir}/ 
        chmod -Rf 777 ${kl_dir}/${MODULE_DIR}
        rm -Rf ${TMP}
echo

