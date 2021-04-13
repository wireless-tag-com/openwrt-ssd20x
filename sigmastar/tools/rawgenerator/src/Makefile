all: dispcfggen logogen

CC=gcc
CFLAGS += -m32 -Wall -I./pnl

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

dispcfggen: ss_raw_header.o disp_data_main.o
	$(CC) $(CFLAGS) -o $@ $^

logogen: ss_raw_header.o logo_data_main.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *.o dispcfggen logogen
