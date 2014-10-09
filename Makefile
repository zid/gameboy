CFLAGS=-march=native -O2 -Wextra -Wall -pedantic
LDFLAGS=-lSDL

all: clean gameboy

debug: CFLAGS += -g
debug: all

gameboy:
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -o gameboy

clean:
	rm -f gameboy
