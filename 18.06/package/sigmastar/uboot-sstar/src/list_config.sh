#!/bin/bash
dir=configs
if [ "${1}" == "" ]; then echo -e "Usage ${0##*/} [SURFFIX] \nSURFFIX: 3 5 6 2m"; fi
pushd $dir > /dev/null
echo -e "to $dir/\n"
ls -1 infinity${1}*_defconfig
popd > /dev/null
