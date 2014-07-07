#include <SDL/SDL.h>

static SDL_Surface *screen;


void sdl_init(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
}

int sdl_update(void)
{
	SDL_Event e;

	SDL_PollEvent(&e);

	if(e.type == SDL_QUIT)
		return 1;

	return 0;
}

void sdl_frame(void)
{
	SDL_Flip(screen);
}

void sdl_quit()
{
	SDL_Quit();
}