# <- this is for comment / total file size must be less than 4KB
tftp 0x21000000 ipl_s.bin
nand erase.part IPL0
nand write.e 0x21000000 IPL0 ${filesize}
nand erase.part IPL1
nand write.e 0x21000000 IPL1 ${filesize}
% <- this is end of file symbol
