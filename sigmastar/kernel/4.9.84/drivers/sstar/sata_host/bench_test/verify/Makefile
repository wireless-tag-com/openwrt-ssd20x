export CROSS_COMPILE=arm-linux-gnueabihf-
CC  = $(CROSS_COMPILE)gcc

INCLUDES  := -I../include/

TARGET := sata_test

all:
	$(CC) -o $(TARGET) $(TARGET).c $(INCLUDES)

clean:
	rm -rf $(TARGET)
