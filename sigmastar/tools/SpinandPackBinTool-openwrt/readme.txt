
一、基本文件主要有下面三个

1.可执行程序 SstarMakeBin

2.初始化文件PNAND.INI(pnand), SPINAND.INI(spinand),看生成的是pnand的image还是spinand的image

3.要生成bin文件的scripts文件

执行生成bin的命令为

./SstarMakeBin -n  PNAND.INI       //生成pnand的image
./SstarMakeBin -n  SPINAND.INI     //生成spinand的image

二、XXX.INI文件说明

[path]                               //设置image和script和生成bin输出的路径
root_directory=./
image_directory=pnand/               //设置image的位置，记得别漏掉\号
script_file=auto_update_pnand.txt    //第一个执行的scripts文件
outpath=./

[nand]                               //设置nand的类型，和id
nandtype=PNAND                       //如果是pnand，则为PNAND,如果是spinand，则为SPINAND，大小写都可以
#nandtype=SPINAND
nandid=0x98-0xF1-0x80-0x15-0xF2      //要生成的image跑的板子上的flash的id号
//nandid=0xC2-0x12

[env]                               //设置环境变量的类型和变量分区的名字，以便生成bin文件时能够保存环境变量
env_type=NANDRAW                    //目前都是直接存在nandflash中，所以都设置为NANDRAW 
env_part=ENV                        //环境变量分区的名字，要与mtdparts中的环境变量分区名字一致
#env_volume=MPOOL
#env_offset=0

[cis]                               //设置nni文件和pni文件，主要是为了tool跑起来时load对应文件初始化flash变量的参数
ecctype=10                          //使用默认值就好
nni=NANDINFO.nni                    //pnand使用.nni文件
#nni=SPINANDINFO.sni                //spinand使用.sni文件 
pni=PARTINFO.pni                    //分区文件
#ppm=PAIRPAGEMAP_v2.ppm             //暂时没有用到

[fcie]                              //此处主要用于设置pnand计算ecc的类型，spinand用不到，目前基本都用fcie5
type=fcie5

[boot]
#boottype=BFN
#miu=bfn_miu.bin

[rawdata]
#type=nand

三、auto_update.txt说明

最主要的是第一行，创建分区，分区跟mtdparts的值一样
dynpart nand0:0x60000@0x140000(IPL0),0x60000(IPL1),0x60000(IPL_CUST0),0x60000(IPL_CUST1),0xC0000(UBOOT0),0xC0000(UBOOT1),0x60000(ENV),0x560000(KERNEL),0x560000(RECOVERY),-(UBI)

指向升级要加载的全部脚步的文件
estar images/mscript/nand.es

执行命令后生成的bin文件为nand.bin,分区文件为snchk.def，这两个文件在用烧录器烧录的时候会用到
这里需要主要一点，生成的pnand bin文件是包括oob区的，spinand的bin文件是不包括oob区的，spinand的oob区是spinand自己生成的，所有烧录的时候记得根据flahs类型，打开或是关闭oob选项

目前文件的加载只支持tftp指令，所以记得修改一下脚本

四、文件执行(加载)顺序

SstarMakeBin->SPINAND.INI->auto_update_spinand.txt->spinand.es->scripts folder file
spinand_example_img只是放在里面的一个例子image，是客户需要生成的完整bin所需要的image文件，客户实际生成时可以用自己的image文件替换掉

五、出错时的检查

1. 如果是执行生成bin时有报错，先检查一下是否所执行的命令不支持，比如sd的升级脚本需要改为tftp的脚本形式
2. 是否执行了一些save命令，重新保存了分区信息，导致分区信息与在auto_update_spinand.txt中指定的分区不一致
3. 如果是烧录到板子上跑不起来，首先根据分区信息，检查生成的bin文件对应偏移地址处的文件信息是否正确，比如IPL的分区偏移为0x140000，则检查生成的bin文件地址0x140000位置处的文件内容是否为IPL文件的内容
4. 检查烧录器烧录时oob区是否disable
5. 用flash tool工具分别dump出板子上flash开始处、IPL分区偏移处、IPL_CUST分区偏移处、UBOOT分区偏移处、kernel分区偏移处的部分文件对比是否与烧录的bin文件一致
