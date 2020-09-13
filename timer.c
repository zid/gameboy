#include "timer.h"
#include "interrupt.h"
#include "cpu.h"

static unsigned int prev_time;
static unsigned int ticks;

static unsigned char tac;
static unsigned int started;
static unsigned int speed;
static unsigned int counter;
static unsigned int modulo;

void timer_set_div(unsigned char v)
{
	(void) v;

	ticks = 0;
}

unsigned char timer_get_div(void)
{
	return ticks>>8;
}

void timer_set_counter(unsigned char v)
{
	counter = v;
}

unsigned char timer_get_counter(void)
{
	return counter;
}

void timer_set_modulo(unsigned char v)
{
	modulo = v;
}

unsigned char timer_get_modulo(void)
{
	return modulo;
}

void timer_set_tac(unsigned char v)
{
	int speeds[] = {1024, 16, 64, 256};

	tac = v;
	started = v&4;
	speed = speeds[v&3];
}

unsigned char timer_get_tac(void)
{
	return tac;
}

static void timer_tick(int delta)
{
	/* cpu.c uses 1MHz ticks not 4MHz, convert */
	ticks += delta * 4;

	if(!started)
		return;

	if((ticks % speed) == 0)
	{
		counter++;
	}

	if(counter == 0x100)
	{
		interrupt(INTR_TIMER);
		counter = modulo;
	}
}

void timer_cycle(void)
{
	/* The amount of ticks since we last ran */
	unsigned int delta = cpu_get_cycles() - prev_time;
	prev_time = cpu_get_cycles();

	timer_tick(delta);
}
