CFLAGS=-march=native -O2 -Wextra -Wall -Wno-switch -std=c99
LDFLAGS=-lSDL

all: clean gameboy

debug: CFLAGS += -g
debug: all

gameboy:
	$(CC) $(CFLAGS) *.c -o gameboy $(LDFLAGS)

clean:
	rm -f gameboy gameboy.exe
