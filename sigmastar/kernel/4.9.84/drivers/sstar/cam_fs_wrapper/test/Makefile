obj-m       := cam_fs_test.o
cam_fs_test-objs := ../src/cam_fs_wrapper.o cam_fs_linux_kernel_test.o

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

KERNEL = $(PWD)/../../../../
CC  = $(CROSS_COMPILE)gcc
APP = cam_fs_linux_user_test
EXTRA_CFLAGS := -I$(KERNEL)/drivers/sstar/include/ -I$(KERNEL)/include/
USER_CFLAGS := -Wall -Werror -g -O2 -ffunction-sections -funwind-tables -fstack-protector -I../pub/ -I$(KERNEL)/drivers/sstar/include/
USER_LDFLAGS := -L$(PWD)/../.build/lib -lcam_fs_wrapper

all:
	make modules -C $(KERNEL)  M=`pwd`
	$(CC) $(USER_CFLAGS) $(USER_LDFLAGS) $(APP).c -o $(APP)

clean:
	make modules clean -C $(KERNEL)  M=`pwd`
	rm -rf $(APP)
