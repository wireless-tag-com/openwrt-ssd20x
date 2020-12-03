#!/usr/bin/python

import re, fnmatch, os, sys, mmap, struct

if __name__ == '__main__':
    
    name='#MS_DTB#'
 
    dtb_file=open(sys.argv[2])
    dtb_file.seek(0,os.SEEK_END)
    size=dtb_file.tell()
    dtb_file.seek(0,os.SEEK_SET)
    dtb=dtb_file.read()
    dtb_file.close()
    
    if sys.getsizeof(dtb) > (64*1024):
        print ('DTB size 0x%08X too big to fit in 64K limit!!' % dtb.size())
        sys.exit() 
        
    fmap=mmap.mmap(os.open(sys.argv[1],os.O_RDWR),0)

    offset=fmap.find(name)    
    if offset >=0:
        print ('offset:0x%08X' % offset)
        print ('  size:0x%08X' % size )
        fmap.seek(offset + 8, os.SEEK_SET)
        fmap.write(struct.pack('<I', size))
        fmap.seek(offset + 16, os.SEEK_SET)
        fmap.write(dtb)

    fmap.close()
    
    
