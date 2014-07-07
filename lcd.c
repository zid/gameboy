#include "lcd.h"
#include "cpu.h"
#include "interrupt.h"

static int lcd_line;

static int lcd_enabled;
static int window_tilemap_select;
static int tiledata_select;
static int window_enabled;
static int tilemap_select;
static int bg_tilemap_select;
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
	bg_tilemap_select     = !!(c & 0x08);
	tilemap_select        = !!(c & 0x10);
	window_enabled        = !!(c & 0x20);
	window_tilemap_select = !!(c & 0x40);
	lcd_enabled           = !!(c & 0x80);
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
		interrupt(INTR_VBLANK);
	}
	if(lcd_line == 0 && prev_line > 0)
	{
		sdl_frame();
	}

	return 1;
}
