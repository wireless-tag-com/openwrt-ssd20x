PLATFORM_NAME := "\#" #This is a multi platform component
CHIP_ID := "\#"

LOCAL_MODULE_NAME=cam_drv_poll_test
LOCAL_MODULE_TYPE=BIN
#LINUX_KERNEL_PATH ?= ../../../../kernel/API
LINUX_KERNEL_PATH = /home/david.tsai/I2/linux-3.18/
LIB_PATH = /home/david.tsai/I3/

KL_API_INCLUDE ?= $(LINUX_KERNEL_PATH)/drivers/sstar/include

LOCAL_CINCLUDES  += -I$(KL_API_INCLUDE)
LOCAL_CINCLUDES  += -I../../pub -I$(LINUX_KERNEL_PATH)/drivers/sstar/cam_drv_poll/pub
LOCAL_CINCLUDES  += -I$(LIB_PATH)/cam_os_wrapper/pub -I$(LIB_PATH)/cam_dev_wrapper/pub
LOCAL_LDFLAGS   += -L$(LIB_PATH)/cam_os_wrapper/.build/lib -L$(LIB_PATH)/cam_dev_wrapper/.build/lib
LOCAL_LIBS      += -lcam_os_wrapper -lcam_dev_wrapper


#LOCAL_CSRCS=$(wildcard ./*.c)
LOCAL_CSRCS=cam_drv_poll_test.c
BRANCH_ID := $(shell git rev-parse --abbrev-ref HEAD 2> /dev/null | sed -e 's/\//_/g')

#include ../../../rules.mk
CROSS_COMPILE ?=arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
STRIP ?= $(CROSS_COMPILE)strip

CFLAGS 	  = -Wall -Werror -O2 -fPIC -DPIC -g -ffunction-sections -funwind-tables -fstack-protector
CFLAGS 	 += -DCAM_OS_LINUX_USER

OMX_TOP_DIR=../../../
LOCAL_BUILD_DIR ?= .build
LOCAL_COBJS = $(patsubst %.c, %.c.o,  $(LOCAL_CSRCS))
LIBS 	  = -ldl -lpthread -lrt -lm
LDFLAGS   = -O2 -Bdirect -Wl,--hash-style=gnu

$(LOCAL_MODULE_NAME): $(LOCAL_COBJS)
	@mkdir -p $(LOCAL_BUILD_DIR)/bin
	@echo "  LD      $(LOCAL_BUILD_DIR)/bin/$@"
	@$(CC) $(LDFLAGS) $(LOCAL_LDFLAGS) -o $(LOCAL_BUILD_DIR)/bin/$@ $(LOCAL_COBJS) $(LIBS) $(LOCAL_LIBS)
	@echo

clean:
	@echo "  CLEAN $(shell pwd)"
	@rm -Rf $(COBJS)
	@rm -Rf $(LOCAL_COBJS)
	@rm -Rf $(LOCAL_BUILD_DIR)

%.c.o : %.c
	@mkdir -p $(dir $@)
	@echo "  CC      $<"
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(LOCAL_CINCLUDES)  -c $< -o $@
