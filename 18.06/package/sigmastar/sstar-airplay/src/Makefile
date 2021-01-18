C_FLAGS	   = $(CFLAGS) -std=c11 -I./internal/include

TARGET_NAME  = SsCloudPlay

C_SRCS    =  $(wildcard *.c */*.c)
C_SRCS    +=  $(wildcard ./internal/src/*.c */*.c)
C_OBJS = $(patsubst %.c, %.c.o, $(C_SRCS))

LIB_NAME += -lavformat -lavcodec -lavutil -lswscale -lswresample -lpthread -lm -lmi_vdec -lmi_sys -lmi_gfx -lmi_divp -lmi_disp -lmi_ao -lmi_panel -ldl -lmi_common

.PHONY: all prepare clean

all: $(TARGET_NAME)

clean:
	@rm -f $(C_OBJS)
	@rm -Rf $(TARGET_NAME)

$(TARGET_NAME): $(C_OBJS) $(C_SRCS)
	@echo "generate $@"
	@$(CC) -o $@ $(C_OBJS) $(LDFLAGS) $(LIB_NAME)

%.c.o : %.c
	@echo "compile $@"
	@$(CC) $(C_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@
