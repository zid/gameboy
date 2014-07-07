#include <stdio.h>
#include "rom.h"
#include "mem.h"
#include "cpu.h"
#include "lcd.h"
#include "sdl.h"

int main(int argc, char *argv[])
{
	int r;

	r = rom_load("zelda.gb");
	if(!r)
		return 0;

	sdl_init();

	printf("ROM OK!\n");

	mem_init();
	printf("Mem OK!\n");

	cpu_init();
	printf("CPU OK!\n");

	while(1)
	{
		if(!cpu_cycle())
			break;

		if(!lcd_cycle())
			break;
	}

	sdl_quit();

	return 0;
}
