rm -rf output
mkdir output
$CROSS_COMPILE\gcc riu.c -o output/riu_r
$CROSS_COMPILE\strip --strip-unneeded output/riu_r

cd output
ln -s riu_r riu_w
ln -s riu_r riux32_w
ln -s riu_r riux32_r
