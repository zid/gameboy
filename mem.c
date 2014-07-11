#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "rom.h"
#include "lcd.h"
#include "mbc.h"
#include "interrupt.h"
#include "timer.h"

#include <stdio.h> /* For serial logging */

static unsigned char *mem;
static int DMA_pending = 0;

void mem_bank_switch(unsigned int n)
{
	unsigned char *b = rom_getbytes();

	memcpy(&mem[0x4000], &b[n * 0x4000], 0x4000);
}

/* LCD's access to VRAM */
unsigned char mem_get_raw(unsigned short p)
{
	return mem[p];
}

unsigned char mem_get_byte(unsigned short i)
{
	unsigned long elapsed;

	if(DMA_pending && i < 0xFF80)
	{
		elapsed = cpu_get_cycles() - DMA_pending;
		if(elapsed >= 160)
			DMA_pending = 0;
		else
			return mem[0xFE00+elapsed];
	}

	switch(i)
	{
		case 0xFF00:	/* Joypad */
			return 0xF;
		break;
		case 0xFF04:
			return timer_get_div();
		break;
		case 0xFF05:
			return timer_get_counter();
		break;
		case 0xFF06:
			return timer_get_modulo();
		break;
		case 0xFF07:
			return timer_get_tac();
		break;
		case 0xFF0F:
			return interrupt_get_IF();
		break;
		case 0xFF44:
			return lcd_get_line();
		break;
		case 0xFF4D:	/* GBC speed switch */
			return 0xFF;
		break;
		case 0xFFFF:
			return interrupt_get_mask();
		break;
	}
	return mem[i];
}

unsigned short mem_get_word(unsigned short i)
{
	unsigned long elapsed;
	if(DMA_pending && i < 0xFF80)
	{
		elapsed = cpu_get_cycles() - DMA_pending;
		if(elapsed >= 160)
			DMA_pending = 0;
		else
			return mem[0xFE00+elapsed];
	}
	return mem[i] | (mem[i+1]<<8);
}

void mem_write_byte(unsigned short d, unsigned char i)
{
	unsigned int filtered = 0;

	switch(rom_get_mapper())
	{
		case MBC1:
			filtered = MBC1_write_byte(d, i);
		break;
	}

	switch(d)
	{
		case 0xFF01: /* Link port data */
			fprintf(stderr, "%c", i);
		break;
		case 0xFF04:
			timer_set_div(i);
		break;
		case 0xFF05:
			timer_set_counter(i);
		break;
		case 0xFF06:
			timer_set_modulo(i);
		break;
		case 0xFF07:
			timer_set_tac(i);
		break;
		case 0xFF0F:
			interrupt_set_IF(i);
			return;
		break;
		case 0xFF40:
			lcd_write_control(i);
		break;
		case 0xFF46: /* OAM DMA */
			/* Copy bytes from i*0x100 to OAM */
			memcpy(&mem[0xFE00], &mem[i*0x100], 0xA0);
			DMA_pending = cpu_get_cycles();
		break;
		case 0xFFFF:
			interrupt_set_mask(i);
			return;
		break;
	}

	if(!filtered)
		mem[d] = i;
}

void mem_write_word(unsigned short d, unsigned short i)
{
	mem[d] = i&0xFF;
	mem[d+1] = i>>8;
}

void mem_init(void)
{
	unsigned char *bytes = rom_getbytes();

	mem = calloc(1, 0x10000);

	memcpy(&mem[0x0000], &bytes[0x0000], 0x4000);
	memcpy(&mem[0x4000], &bytes[0x4000], 0x4000);

	mem[0xFF10] = 0x80;
	mem[0xFF11] = 0xBF;
	mem[0xFF12] = 0xF3;
	mem[0xFF14] = 0xBF;
	mem[0xFF16] = 0x3F;
	mem[0xFF19] = 0xBF;
	mem[0xFF1A] = 0x7F;
	mem[0xFF1B] = 0xFF;
	mem[0xFF1C] = 0x9F;
	mem[0xFF1E] = 0xBF;
	mem[0xFF20] = 0xFF;
	mem[0xFF23] = 0xBF;
	mem[0xFF24] = 0x77;
	mem[0xFF25] = 0xF3;
	mem[0xFF26] = 0xF1;
	mem[0xFF40] = 0x91;
	mem[0xFF47] = 0xFC;
	mem[0xFF48] = 0xFF;
	mem[0xFF49] = 0xFF;
}
