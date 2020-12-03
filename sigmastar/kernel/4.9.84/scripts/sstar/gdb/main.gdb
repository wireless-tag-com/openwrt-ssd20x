# connect to remote JLinkGDBServer
target  remote gdbserver:2331
# load symobl
file vmlinux
#symbol-file
#add-symbol-file vmlinux 0x23f26000
#add-symbol-file vmlinux 0x23008000
