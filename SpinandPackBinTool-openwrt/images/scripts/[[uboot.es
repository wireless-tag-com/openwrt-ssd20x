# <- this is for comment / total file size must be less than 4KB
tftp 0x21000000 uboot_s.bin
nand erase.part UBOOT0
nand write.e 0x21000000 UBOOT0 ${filesize}
nand erase.part UBOOT1
nand write.e 0x21000000 UBOOT1 ${filesize}
% <- this is end of file symbol
