SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

CFLAGS=-march=native -O2 -W -Wextra -Wall -Wno-switch -std=c99 -g
LDFLAGS=-lSDL2

all: clean gameboy

debug: CFLAGS += -g
debug: all

gameboy: $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o gameboy $(LDFLAGS) -fwhole-program

%.o : %.c
	$(CC) $(CFLAGS) -flto $^ -c 

clean:
	rm -f gameboy gameboy.exe *.o
