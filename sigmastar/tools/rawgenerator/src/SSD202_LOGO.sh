#!/bin/sh

rm -rf logo

./dispcfggen -c -o logo -p 0x7c00000 -s 0x300000 -d FRD720X720BK
./logogen -a -i ./image/wt720X720.jpg -o logo
./logogen -a -i ./image/upgrade.jpg -o logo
