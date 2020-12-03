# openwrt-sstar
wireless-tag 支持sigmstar SSD201/SSD202

# 安装依赖
ubuntu 16.04.7 64位系统

````sh
sudo apt-get install subversion build-essential libncurses5-dev zlib1g-dev gawk git ccache \
		gettext libssl-dev xsltproc libxml-parser-perl \
		gengetopt default-jre-headless ocaml-nox sharutils texinfo
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install zlib1g:i386 libstdc++6:i386 libc6:i386 libc6-dev-i386
````

# 下载代码
1. 下载主工程代码
```
git clone https://github.com/wireless-tag-com/openwrt-ssd20x.git
```

# 安装toolchian

1. 下载toolchain
链接：https://pan.baidu.com/s/1SUk1a-drbWo1tkHQzCgchg 
提取码：1o3d 

2.  解压缩toolchain

```
sudo tar wt-gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf.tag.gz -xvf -C /opt/
```

3. 设置环境变量，修改 ~/.profile文件, 将下面这行添加到文件末尾
```
PATH="/opt/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin:$PATH"
```

手动生效环境变量
```
source ~/.profile
```

测试交叉工具链
```
arm-linux-gnueabihf-gcc --version
```


# 编译

1. 生成机型配置文件

```
cd 18.06
./scripts/feeds update -a
./scripts/feeds install -a -f
make WT2022_wt
```

| 机型名 | 说明            |
| ------ | --------------- |
| WT2022 | SSD202+SPI NAND|
| WT2011 | SSD201+SPI NAND|

2. 编译

```
make V=s -j4
```

3. 编译产物
    位于bin/target/sstar/ssd20x/WT2022

| 文件名                   | 说明                 |
| ------------------------ | -------------------- |
| WT2022-sysupgrade.bin    | 升级文件             |
| WT2022-uImage.xz         | 内核文件             |
| WT2022-root-ubi.img      | 根文件系统(SPI NAND) |


# 升级
系统中通过串口或者telnet进入系统后台，执行如下命令：

```
cd /tmp
tftp -g 192.168.1.88 -r WT2022-sysupgrade.bin
sysupgrade WT2022-sysupgrade.bin
```

升级完成之后，系统将自动重启

uboot下通过串口和网口进行升级（上电阶段按下enter按键），执行如下命令：

设置环境变量，启动网络
```
setenv serverip 192.168.1.88
setenv ipaddr 192.168.1.11
saveenv
reset
```

## SPI NAND
升级kernel

```
tftp 0x21000000 WT2022-uImage.xz
nand erase.part KERNEL
nand write.e 0x21000000 KERNEL ${filesize}
nand erase.part RECOVERY
nand write.e 0x21000000 RECOVERY ${filesize}
```

升级rootfs

```
tftp 0x21000000 WT2022-root-ubi.img
nand erase.part UBI
nand write.e 0x21000000 UBI ${filesize}
```
