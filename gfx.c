
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "config.h"
#include "marfbed.h"

SDL_Event e;
SDL_Surface * s    = NULL;

void init_gfx(marfbed_t * b)
{
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
	s = SDL_SetVideoMode(
			GFX_X,
			GFX_Y,
			24,
			SDL_HWSURFACE | SDL_HWACCEL | SDL_ASYNCBLIT
			);
	atexit(SDL_Quit);
	if (!s)
	{
		printf("ERROR: couldn't SDL_SetVideoMode(): %s\n", SDL_GetError());
		exit(1);
	}
	SDL_WM_SetCaption("·-= marf testbed =-·", "marfbed");
	//	SDL_BlitSurface(p, 0, s, 0);
	SDL_Flip(s);
}

void setPixel(	/* FIXME: this must be simpler */
		SDL_Surface * s,
		uint16_t x,
		uint16_t y,
		uint32_t color
	     )
{
	SDL_Rect pixel;
	pixel.w = 1;
	pixel.h = 1;
	pixel.x = x;
	pixel.y = y;
	SDL_FillRect(s, &pixel, color);
}

void Draw_Circle(
		SDL_Surface * s,
		uint16_t x0,
		uint16_t y0,
		uint16_t r,
		uint32_t color
		)
{
	int f = 1 - r;
	int ddF_x = 1;
	int ddF_y = -2 * r;
	int x = 0;
	int y = r;

	setPixel(s, x0, y0 + r, color);
	setPixel(s, x0, y0 - r, color);
	setPixel(s, x0 + r, y0, color);
	setPixel(s, x0 - r, y0, color);

	while(x < y)
	{
		// ddF_x == 2 * x + 1;
		// ddF_y == -2 * y;
		// f == x*x + y*y - radius*radius + 2*x - y + 1;
		if(f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		setPixel(s, x0 + x, y0 + y, color);
		setPixel(s, x0 - x, y0 + y, color);
		setPixel(s, x0 + x, y0 - y, color);
		setPixel(s, x0 - x, y0 - y, color);
		setPixel(s, x0 + y, y0 + x, color);
		setPixel(s, x0 - y, y0 + x, color);
		setPixel(s, x0 + y, y0 - x, color);
		setPixel(s, x0 - y, y0 - x, color);
	}
}


void clearScreen(SDL_Surface * s)
{
	SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = GFX_X;
	r.h = GFX_Y;
	SDL_FillRect(s, &r, 0);
}


void mainloop_gfx(marfbed_t * b)
{
	SDL_PollEvent( &e );
	if (
			( (e.type == SDL_KEYDOWN) &&
			  ( (e.key.keysym.sym == SDLK_ESCAPE) ||
			    (e.key.keysym.sym == SDLK_q)
			  )
			) ||
			(e.type == SDL_QUIT)
	   )
		exit(0);

	clearScreen(s);

	int i;
	for(i=0; i<MARF_MAX; i++)
	{
		Draw_Circle(s,
				b->marf[i].x * GFX_X / SPACE_X,
				b->marf[i].y * GFX_Y / SPACE_Y,
				MARF_R_RADIO_JAM,
				COLOR_RADIO_JAM
			   );
	}
	for(i=0; i<MARF_MAX; i++)
	{
		Draw_Circle(s,
				b->marf[i].x * GFX_X / SPACE_X,
				b->marf[i].y * GFX_Y / SPACE_Y,
				MARF_R_RADIO,
				COLOR_RADIO
			   );
	}
	for(i=0; i<MARF_MAX; i++)
	{
		Draw_Circle(s,
				b->marf[i].x * GFX_X / SPACE_X,
				b->marf[i].y * GFX_Y / SPACE_Y,
				GFX_MARF_X,
				b->marf[i].color
			   );
	}


	SDL_Flip(s);
}
