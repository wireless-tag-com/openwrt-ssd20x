# <- this is for comment / total file size must be less than 4KB
tftp 0x21000000 root.ubi
nand erase.part ubi
nand write.e 0x21000000 ubi ${filesize}
% <- this is end of file symbol
