# <- this is for comment / total file size must be less than 4KB
#tftp 0x21000000 kernel
#nand erase.part KERNEL
#nand write.e 0x21000000 KERNEL ${filesize}
#nand erase.part RECOVERY
#nand write.e 0x21000000 RECOVERY ${filesize}
% <- this is end of file symbol
