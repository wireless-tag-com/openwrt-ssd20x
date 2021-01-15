
TARGET = sstar-init

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

sstar-init: main.o
	$(CC) -o $@ $^ $(LDFLAGS) -lm -lmi_disp -lmi_panel -lmi_gfx -lmi_sys -lmi_common -ldl  -lpthread

.PHONY: clean

clean:
	rm -rf *.o $(TARGET)

