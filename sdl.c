#include <SDL2/SDL.h>
#include <stdio.h>

static unsigned int frames;

static SDL_Window   *window;
static SDL_Surface  *surface;

static int button_start, button_select, button_a, button_b, button_down, button_up, button_left, button_right;

const unsigned char *keystates;

void sdl_init(void)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow("Fer is an ejit",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640, 480,
		SDL_WINDOW_INPUT_FOCUS
	);

	surface = SDL_GetWindowSurface(window);
}

int sdl_update(void)
{
	SDL_Event e;

	keystates = SDL_GetKeyboardState(NULL);

	while(SDL_PollEvent(&e))
	{
		if(e.type == SDL_QUIT)
			return 1;
	}

	if(keystates[SDL_SCANCODE_A])
		button_a = 1;
	else
		button_a = 0;

	if(keystates[SDL_SCANCODE_S])
		button_b = 1;
	else
		button_b = 0;

	if(keystates[SDL_SCANCODE_D])
		button_select = 1;
	else
		button_select = 0;

	if(keystates[SDL_SCANCODE_F])
		button_start = 1;
	else
		button_start = 0;

	if(keystates[SDL_SCANCODE_LEFT])
		button_left = 1;
	else
		button_left = 0;

	if(keystates[SDL_SCANCODE_RIGHT])
		button_right = 1;
	else
		button_right = 0;

	if(keystates[SDL_SCANCODE_UP])
		button_up = 1;
	else
		button_up = 0;

	if(keystates[SDL_SCANCODE_DOWN])
		button_down = 1;
	else
		button_down = 0;

	if(keystates[SDL_SCANCODE_F1])
		cpu_print_debug();

	if(keystates[SDL_SCANCODE_ESCAPE])
		return 1;

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
