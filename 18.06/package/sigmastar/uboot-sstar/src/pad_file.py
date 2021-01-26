import os
import getopt
import sys

def GetFileLength(f):
    if type(f) == file:
        return os.fstat(f.fileno())[6]
    if type(f) == str:
        return os.stat(f)[6]
    

def main():

    inputFile = ''
    outputFile = ''
    padSize=1024*128
    try:
        opts, args = getopt.getopt(sys.argv[1:],"i:o:s:")
        if len(opts) <3:
            print 'Usage: paf_file -i <input file> -o <output file> -s <size>' 
            sys.exit(2)   
    except getopt.GetoptError:
        print 'Usage: paf_file -i <input file> -o <output file> -s <size>'
        sys.exit(2)  
    
    for opt, arg in opts:
        if opt == '-i':
            inputFile=arg
        elif opt == '-o':
            outputFile=arg
        elif opt == '-s':
            padSize=int(arg)

    inf = open(inputFile,'rb')        
    outf = open(outputFile,'wb')

    infSize=GetFileLength(inf)
    
    if infSize<=padSize :
        data=inf.read() #read_all
        outf.write(data)
        while(infSize < padSize):
            outf.write('\0')
            infSize+=1
  
    else:
        print "pad size is smaller than input file!!!"

    inf.close()
    outf.close()

if __name__ == "__main__":
    main() 