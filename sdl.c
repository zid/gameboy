#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

static unsigned int frames;

static SDL_Window   *window;
static SDL_Surface  *surface;

static int button_start, button_select;
static int button_a, button_b;
static int button_down, button_up, button_left, button_right;
static int button_debug, button_quit;

struct keymap
{
	SDL_Scancode code;
	int *key;
	void (*f)(void);
	int prev;
};

static void debug()
{
	cpu_print_debug();
}

static struct keymap keys[] =
{
	{SDL_SCANCODE_A,     &button_a,      NULL, 0},
	{SDL_SCANCODE_S,     &button_b,      NULL, 0},
	{SDL_SCANCODE_D,     &button_select, NULL, 0},
	{SDL_SCANCODE_F,     &button_start,  NULL, 0},
	{SDL_SCANCODE_LEFT,  &button_left,   NULL, 0},
	{SDL_SCANCODE_RIGHT, &button_right,  NULL, 0},
	{SDL_SCANCODE_UP,    &button_up,     NULL, 0},
	{SDL_SCANCODE_DOWN,  &button_down,   NULL, 0},
	{SDL_SCANCODE_ESCAPE,   &button_quit,   NULL, 0},
	{SDL_SCANCODE_F1,    &button_debug, debug, 0}
};


int sdl_init(void)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow(
		"Fer is an ejit",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640, 576,
		SDL_WINDOW_INPUT_FOCUS
	);

	surface = SDL_GetWindowSurface(window);
	
	return 0;
}

int sdl_update(void)
{
	SDL_Event e;
	const unsigned char *keystates;
	size_t i;

	keystates = SDL_GetKeyboardState(NULL);

	while(SDL_PollEvent(&e))
	{
		if(e.type == SDL_QUIT)
			return 1;
	}

	for(i = 0; i < sizeof (keys) / sizeof (struct keymap); i++)
	{
		if(!keystates[keys[i].code])
		{
			if(keys[i].key)
				*(keys[i].key) = 0;
			continue;
		}

		if(keys[i].f && keys[i].prev == 0)
		{
			*(keys[i].key) = 1;
			keys[i].f();
		}

		keys[i].prev = *(keys[i].key);
		*(keys[i].key) = keystates[keys[i].code];
	}

	if(button_quit)
	{
		printf("frames: %d\n", frames);
		return 1;
	}

	return 0;
}

unsigned int sdl_get_buttons(void)
{
	return (button_start*8) | (button_select*4) | (button_b*2) | button_a;
}

unsigned int sdl_get_directions(void)
{
	return (button_down*8) | (button_up*4) | (button_left*2) | button_right;
}

unsigned int *sdl_get_framebuffer(void)
{
	return surface->pixels;
}

void sdl_frame(void)
{
	frames++;

	SDL_UpdateWindowSurface(window);
}

void sdl_quit()
{
	SDL_Quit();
}
