SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

CFLAGS=-march=native -O3 -Wextra -Wall -Wno-switch -std=c99
LDFLAGS=-lSDL2

all: clean gameboy

debug: CFLAGS += -g -DDEBUG=1
debug: clean gameboy

gameboy: $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o gameboy $(LDFLAGS) -fwhole-program

%.o : %.c
	$(CC) $(CFLAGS) -flto $^ -c

clean:
	rm -f gameboy gameboy.exe *.o
