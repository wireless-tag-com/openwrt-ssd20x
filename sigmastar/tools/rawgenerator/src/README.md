# 制作LOGO
## 编译工具

```
sudo apt-get install libc6-dev-i386
make
```

dispcfggen和logogen为最终的制作LOGO的工具

## 生成LOGO

SSD201

```
dispcfggen -c -o logo -p 0x7c00000 -s 0x300000 -d FRD720X720BK
logogen -a -i ./image/wt-logo.jpg -o logo
logogen -a -i ./image/upgrade.jpg -o logo
```

SSD202

```
dispcfggen -c -o logo -p 0x3300000 -s 0x300000 -d FRD720X720BK
logogen -a -i ./image/wt-logo.jpg -o logo
logogen -a -i ./image/upgrade.jpg -o logo
```


wt-logo.jpg为LOGO图片，必须为jpg格式，注意分辨率一定和屏幕对应。
FRD720X720BK为屏幕的名称，最终对应相关屏幕的初始化头文件。

logo为生成的文件

## 替换LOGO
telnet进入系统，执行以下命令进行替换

```
cd /tmp
tftp -g 192.168.1.106 -r logo
mtd write logo LOGO
```

重启系统就可以看到已经替换的LOGO了
