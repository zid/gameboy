#include <SDL/SDL.h>

static SDL_Surface *screen;

void sdl_init(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
}

int sdl_update(void)
{
	SDL_Event e;

	while(SDL_PollEvent(&e))
	{
		if(e.type == SDL_QUIT)
			return 1;
	}
	return 0;
}

unsigned int *sdl_get_framebuffer(void)
{
	return screen->pixels;
}

void sdl_frame(void)
{
	SDL_Flip(screen);
}

void sdl_quit()
{
	SDL_Quit();
}