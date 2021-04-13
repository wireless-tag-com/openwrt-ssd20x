#!/bin/sh

rm -rf logo

./dispcfggen -c -o logo -p 0x3300000 -s 0x300000 -d CC021I40R
./logogen -a -i ./image/wt480X480.jpg -o logo
./logogen -a -i ./image/upgrade.jpg -o logo
