#!/usr/bin/python

import re, fnmatch, os, sys, mmap, struct

if __name__ == '__main__':

    name=sys.argv[2]
    if sys.argv[3].upper().startswith( '0X' ):
        value=long(sys.argv[3],16)
    else:
        value=long(sys.argv[3])

    fmap=mmap.mmap(os.open(sys.argv[1],os.O_RDWR),0)

    offset=fmap.find(name)
#    print ('%s:%d\n' % (name,offset))
    if offset < 0:
        print ('error finding ms_bin_option:%s in %s\n' % (name,sys.argv[1]))
    else:
        fmap.seek(offset + 8, os.SEEK_SET)
    fmap.write(struct.pack('<I', value))

    fmap.close()
