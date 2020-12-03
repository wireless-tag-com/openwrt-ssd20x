#!/bin/sh

KERNEL_ROOT_DIR=../../..
KERNEL_CONFIG_DIR=../../../arch/arm/configs
KERNEL_DTS_DIR=../../../arch/arm/boot/dts

for i in $@;do
    echo ">>>>>> Handle $i:"
    find $KERNEL_ROOT_DIR -type d -iname $i | xargs rm -rfv
    find $KERNEL_CONFIG_DIR -type f -iname "$i""_*defconfig" | xargs rm -rfv
    find $KERNEL_DTS_DIR -type f -regex ".*$i\(\.\|-\).*\(dts\|dtsi\|dtb\)" | xargs rm -rfv
    echo "<<<<<<"
    if [ -f "$i.blacklist" ]; then
        echo ">>>>>> Handle $i.blacklist:"
        while read -r line
        do
            echo "  remove $KERNEL_ROOT_DIR/$line"
            rm -rf "$KERNEL_ROOT_DIR/$line"
        done < $i.blacklist
        echo "<<<<<<"
    fi
done

# clear platform specify release blacklist file
find  -iname '*.blacklist' -exec rm {} \;


