C_FLAGS	   = $(CFLAGS) -std=c11

TARGET_NAME  = SsMp3Player

C_SRCS    =  $(wildcard *.c */*.c)
C_OBJS = $(patsubst %.c, %.c.o, $(C_SRCS))

LIB_NAME += -lpthread -lm -lmi_sys -lmi_ao -ldl -lmi_common -lmad


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
