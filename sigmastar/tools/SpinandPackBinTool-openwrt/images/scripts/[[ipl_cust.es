tftp 0x21000000 ipl_cust_s.bin
nand erase.part IPL_CUST0
nand write.e 0x21000000 IPL_CUST0 ${filesize}
nand erase.part IPL_CUST1
nand write.e 0x21000000 IPL_CUST1 ${filesize}
% <- this is end of file symbol
