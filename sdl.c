#include <SDL/SDL.h>
#include <sys/time.h>
static SDL_Surface *screen;
static unsigned int frames;
static struct timeval tv1, tv2;



static int button_start, button_select, button_a, button_b, button_down, button_up, button_left, button_right;

void sdl_init(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("Fer is an ejit", NULL);
}

int sdl_update(void)
{
	SDL_Event e;

	while(SDL_PollEvent(&e))
	{
		if(e.type == SDL_QUIT)
			return 1;

		if(e.type == SDL_KEYDOWN)
		{
			switch(e.key.keysym.sym)
			{
				case SDLK_a:
					button_a = 1;
				break;
				case SDLK_s:
					button_b = 1;
				break;
				case SDLK_d:
					button_select = 1;
				break;
				case SDLK_f:
					button_start = 1;
				break;
				case SDLK_LEFT:
					button_left = 1;
				break;
				case SDLK_RIGHT:
					button_right = 1;
				break;
				case SDLK_DOWN:
					button_down = 1;
				break;
				case SDLK_UP:
					button_up = 1;
				break;
				case SDLK_ESCAPE:
					return 1;
			}
		}

		if(e.type == SDL_KEYUP)
		{
			switch(e.key.keysym.sym)
			{
				case SDLK_a:
					button_a = 0;
				break;
				case SDLK_s:
					button_b = 0;
				break;
				case SDLK_d:
					button_select = 0;
				break;
				case SDLK_f:
					button_start = 0;
				break;
				case SDLK_LEFT:
					button_left = 0;
				break;
				case SDLK_RIGHT:
					button_right = 0;
				break;
				case SDLK_DOWN:
					button_down = 0;
				break;
				case SDLK_UP:
					button_up = 0;
				break;
			}
		}

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
	return screen->pixels;
}

void sdl_frame(void)
{
	if(frames == 0)
		gettimeofday(&tv1, NULL);
	
	frames++;
	if(frames % 1000 == 0)
	{
		gettimeofday(&tv2, NULL);
		printf("Frames %d, seconds: %d, fps: %d\n", frames, tv2.tv_sec - tv1.tv_sec, frames/(tv2.tv_sec - tv1.tv_sec));
	}
	SDL_Flip(screen);
}

void sdl_quit()
{
	SDL_Quit();
}