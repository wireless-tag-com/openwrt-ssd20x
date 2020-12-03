#!/bin/bash
dir=arch/arm/configs
if [ "${1}" == "" ]; then echo -e "Usage ${0##*/} [PREFIX] \nPREFIX: 3 5 6 2m"; fi
pushd $dir > /dev/null
echo -e "to $dir/\n"
ls -1 infinity${1}*_defconfig
popd > /dev/null
