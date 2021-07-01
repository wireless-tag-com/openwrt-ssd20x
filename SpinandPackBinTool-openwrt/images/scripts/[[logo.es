# <- this is for comment / total file size must be less than 4KB
tftp 0x21000000 logo
nand erase.part LOGO
nand write.e 0x21000000 LOGO ${filesize}
% <- this is end of file symbol
