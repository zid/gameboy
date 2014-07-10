#include "lcd.h"
#include "cpu.h"
#include "interrupt.h"
#include "sdl.h"
#include "mem.h"

#include "windows.h"

static int lcd_line;

static int lcd_enabled;
static int window_tilemap_select;
static int tiledata_select;
static int window_enabled;
static int tilemap_select;
static int bg_tiledata_select;
static int sprite_size;
static int sprites_enabled;
static int bg_enabled;

int lcd_get_line(void)
{
	return lcd_line;
}

void lcd_write_control(unsigned char c)
{
	bg_enabled            = !!(c & 0x01);
	sprites_enabled       = !!(c & 0x02);
	sprite_size           = !!(c & 0x04);
	tilemap_select        = !!(c & 0x08);
	bg_tiledata_select    = !!(c & 0x10);
	window_enabled        = !!(c & 0x20);
	window_tilemap_select = !!(c & 0x40);
	lcd_enabled           = !!(c & 0x80);
}

static void draw_stuff(void)
{
	unsigned int *b = sdl_get_framebuffer();
	int x, y, tx, ty;

//	printf("Frame: Tiles: %x, MAP: %x, LCD: %s\n", 0x8000 + (bg_tiledata_select * 0x800), 0x9800 + tilemap_select * 0x400, lcd_enabled ? "on" : "off");

	for(ty = 0; ty < 20; ty++)
	{
	for(tx = 0; tx < 18; tx++)
	{
	for(y = 0; y<8; y++)
	{
		unsigned int colours[4] = {0xFFFFFF, 0xC0C0C0, 0x808080, 0x0};
		unsigned char b1, b2;
//		int tileaddr = 0x8000 +  ty*0x100 + tx*16 + y*2;
		int tileaddr = 0x8000 + (bg_tiledata_select * 0x800) + mem_get_raw(0x9800 + tilemap_select * 0x400 + ty*32+tx)*16 + y*2;

		b1 = mem_get_raw(tileaddr);
		b2 = mem_get_raw(tileaddr+1);
		b[(ty*640*8)+(tx*8) + (y*640) + 0] = colours[(!!(b1&0x80))<<1 | !!(b2&0x80)];
		b[(ty*640*8)+(tx*8) + (y*640) + 1] = colours[(!!(b1&0x40))<<1 | !!(b2&0x40)];
		b[(ty*640*8)+(tx*8) + (y*640) + 2] = colours[(!!(b1&0x20))<<1 | !!(b2&0x20)];
		b[(ty*640*8)+(tx*8) + (y*640) + 3] = colours[(!!(b1&0x10))<<1 | !!(b2&0x10)];
		b[(ty*640*8)+(tx*8) + (y*640) + 4] = colours[(!!(b1&0x8))<<1 | !!(b2&0x8)];
		b[(ty*640*8)+(tx*8) + (y*640) + 5] = colours[(!!(b1&0x4))<<1 | !!(b2&0x4)];
		b[(ty*640*8)+(tx*8) + (y*640) + 6] = colours[(!!(b1&0x2))<<1 | !!(b2&0x2)];
		b[(ty*640*8)+(tx*8) + (y*640) + 7] = colours[(!!(b1&0x1))<<1 | !!(b2&0x1)];
	}
	}
	}


	for(ty = 0; ty < 20; ty++)
	{
	for(tx = 0; tx < 18; tx++)
	{
	for(y = 0; y<8; y++)
	{
		unsigned int colours[4] = {0xFFFFFF, 0xC0C0C0, 0x808080, 0x0};
		unsigned char b1, b2;
		int tileaddr = 0x8000 +  ty*0x100 + tx*16 + y*2;

		b1 = mem_get_raw(tileaddr);
		b2 = mem_get_raw(tileaddr+1);
		b[(ty*640*8)+(tx*8) + (y*640) + 0 + 0x1F400] = colours[(!!(b1&0x80))<<1 | !!(b2&0x80)];
		b[(ty*640*8)+(tx*8) + (y*640) + 1 + 0x1F400] = colours[(!!(b1&0x40))<<1 | !!(b2&0x40)];
		b[(ty*640*8)+(tx*8) + (y*640) + 2 + 0x1F400] = colours[(!!(b1&0x20))<<1 | !!(b2&0x20)];
		b[(ty*640*8)+(tx*8) + (y*640) + 3 + 0x1F400] = colours[(!!(b1&0x10))<<1 | !!(b2&0x10)];
		b[(ty*640*8)+(tx*8) + (y*640) + 4 + 0x1F400] = colours[(!!(b1&0x8))<<1 | !!(b2&0x8)];
		b[(ty*640*8)+(tx*8) + (y*640) + 5 + 0x1F400] = colours[(!!(b1&0x4))<<1 | !!(b2&0x4)];
		b[(ty*640*8)+(tx*8) + (y*640) + 6 + 0x1F400] = colours[(!!(b1&0x2))<<1 | !!(b2&0x2)];
		b[(ty*640*8)+(tx*8) + (y*640) + 7 + 0x1F400] = colours[(!!(b1&0x1))<<1 | !!(b2&0x1)];
	}
	}
	}
	sdl_frame();

	if(lcd_enabled)
		Sleep(10);
}

int lcd_cycle(void)
{
	int cycles = cpu_get_cycles();
	int this_frame, prev_line;

	if(sdl_update())
		return 0;

	this_frame = cycles % (70224/4);

	prev_line = lcd_line;

	lcd_line = this_frame / (456/4);

	if(prev_line == 143 && lcd_line == 144)
	{
		draw_stuff();
		interrupt(INTR_VBLANK);
	}
/*	if(lcd_line == 0 && prev_line > 0)
	{
		sdl_frame();
	}
*/
	return 1;
}
