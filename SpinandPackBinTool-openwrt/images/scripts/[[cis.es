# <- this is for comment / total file size must be less than 4KB
tftp 0x21000000 boot/SPINANDINFO.sni
tftp 0x21800000 boot/PARTINFO.pni
writecis 0x21000000 0x21800000 10 0 0 5
% <- this is end of file symbol
