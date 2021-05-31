esp8089
======

ESP8089 Linux driver

v1.9 imported from the Rockchip Linux kernel github repo

Modified to build as a standalone module for SDIO devices.

Building:

 make $CROSS\_COMPILE\_FLAGS -C $KERNEL\_SOURCE M=$PWD
 # $CROSS\_COMPILE\_FLAGS means parameters for kernel cross-compiling (e.g. ARCH=xxx CROSS\_COMPILE=yyy-zzz-), $KERNEL\_SOURCE means the directory for kernel building (usually it's also the kernel source tree position).

Using:

Must load mac80211.ko first if not baked in.

 sudo modprobe esp8089.ko

If you get a wlan interface, but scanning shows no networks try using:

 sudo modprobe esp8089.ko config=crystal_26M_en=1

or:

 sudo modprobe esp8089.ko config=crystal_26M_en=2

To load the module.
