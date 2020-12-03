#!/usr/bin/python

import re, fnmatch, os, sys, subprocess, argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate MVXXVM .h file for P4 system')

    parser.add_argument("file", help="Output File")
    parser.add_argument("--changelist", help="Change List String", default="#0000000")
    parser.add_argument("--lib_type", help="Library Type",default="#")
    parser.add_argument("--chip_id", help="Chip ID",default="##")
    parser.add_argument("--comp_id", help="COMP_ID",default="############")
#    parser.add_argument("--p4_change", help="Write the $Change for P4",action="store_true")

    parser.add_argument("--branch", help="branch")
    parser.add_argument("--gitver", help="gitver")
    parser.add_argument("--builddate", help="builddate")
    parser.add_argument("--buildusr", help="buildusr")
    args=parser.parse_args()

    #print args
    
    if args.file is None :
        print 'ERROR: must specify output header file!!'
        sys.exit(-1)

    version_file=open(args.file,'a+')
    
#    if args.p4_change is not None:
#        version_file.write("// $Change: %s $\n" % args.changelist[1:])

    while len(args.comp_id) < 10 :
        args.comp_id += '#'
        
    while len(args.chip_id) < 4 :
        args.chip_id += '#'
        
    while len(args.changelist) < 8 :
        args.changelist += '#'

    while len(args.lib_type) < 2 :
        args.lib_type += '#'

    ext_val=''
    ext_str=''
    if args.branch is not None :
        ext_val += '[BR:'
        ext_val += args.branch
        ext_val += ']'

    if len(ext_val) > 0 :
            ext_str += ext_val

    version_file.write('/*\n')
    version_file.write(' * Modified Information for CEVA XM6 Version Control\n')
    version_file.write(' */\n')
    version_file.write('\n')

    version_file.write('#define CEVA_SRC_VERSION 1 \n')
    version_file.write('#define CEVA_LOCAL_LIB_VERSION 2 \n')
    version_file.write('#define CEVA_SUBVERSION 1 \n')

    version_file.write("\n")
    version_file.write('#define CEVA_XM6_GIT_BRANCH "' + args.branch + '"\n')
    version_file.write('#define CEVA_XM6_GIT_VERSION "' + args.gitver + '"\n')
    version_file.write('#define CEVA_XM6_BUILD_DATE "' + args.builddate + '"\n')
    version_file.write('#define CEVA_XM6_BUILD_USR "' + args.buildusr + '"\n\n')


    version_file.write('typedef struct \n')
    version_file.write('{ \n')
    version_file.write('    unsigned int Main;\n')
    version_file.write('    unsigned int Second;\n')
    version_file.write('    unsigned int Sub;\n')
    version_file.write('}CEVA_VerCTL_Info;\n')

    if(args.buildusr == 'causer'):
      version_file.write('#define SERVER_BUILD 1 \n')
    else:
      version_file.write('#define SERVER_BUILD 0 \n')
  
    version_file.write("\n")
    version_file.write('#define VERSION_INFO_LENGTH 128 \n')
    version_file.write('struct Version_Info{\n')
    version_file.write('   CEVA_VerCTL_Info CEVA_VInfo;\n')
    version_file.write('   char GitBranch[VERSION_INFO_LENGTH];\n')
    version_file.write('   char GitVersion[VERSION_INFO_LENGTH];\n')
    version_file.write('   char BuildCodeData[VERSION_INFO_LENGTH];\n')
    version_file.write('   unsigned int FormalReleaseFlg;\n')
    version_file.write('};\n')
    version_file.close()
