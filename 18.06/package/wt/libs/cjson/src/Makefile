
TARGET = libcJSON.so

all: $(TARGET)

%.lo: %.c
	$(CC) -fpic $(CFLAGS) -c -o $@ $^

libcJSON.so: cJSON.lo
	$(CC) $(LDFLAGS) -shared -o $@ $^ -lm

.PHONY: clean

clean:
	rm -rf *.o *.lo $(TARGET)

