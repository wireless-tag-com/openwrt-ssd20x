#!/usr/bin/python

import re, fnmatch, os, sys, mmap, struct

if __name__ == '__main__':

    name=sys.argv[2]
#    if sys.argv[3].upper().startswith( '0X' ):
#        value=long(sys.argv[3],16)
#    else:
#        value=long(sys.argv[3])

    fmap=mmap.mmap(os.open(sys.argv[1],os.O_RDWR),0)

    offset=fmap.find(name)
#    print ('%s:%d\n' % (name,offset))
    if offset >= 0:
        fmap.seek(offset + 8, os.SEEK_SET)
        if len(sys.argv) > 3 and sys.argv[3].upper()==( '16' ):
            print ('0x%08X' % struct.unpack('<I', fmap.read(4)))
        else:
            print ('%d' % struct.unpack('<I', fmap.read(4)))

    fmap.close()
