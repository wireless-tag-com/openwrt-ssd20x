# <- this is for comment / total file size must be less than 4KB
#mtdparts del CIS
#setenv mtdparts $(mtdparts),0x20000(KEY_CUST),0x60000(LOGO),0x500000(KERNEL),0x500000(RECOVERY),-(UBI)
saveenv
% <- this is end of file symbol
