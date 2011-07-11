#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "config.h"
#include "marfbed.h"

SDL_Surface * s    = NULL;

#define FONT_NAME "/usr/share/fonts/truetype/freefont/FreeSans.ttf"

void init_gfx(marfbed_t * b)
{
	/* graphics */
	SDL_Init(SDL_INIT_VIDEO);

	/* fonts */
	TTF_Init();
	b->font = TTF_OpenFont(FONT_NAME, 12);
	if (!b->font)
	{
		printf("ERROR: couldn't open %s: %s\n",
			FONT_NAME,
			TTF_GetError()
			);
		exit(1);
	}

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

	b->show_ring_1  = 1;
	b->show_ring_2  = 1;
	b->show_ring_3  = 1;
	b->show_lines   = 1;
	b->show_numbers = 0;
	b->show_info    = 0;

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

void Draw_Circle_Segments(
		SDL_Surface * s,
		uint16_t x0,
		uint16_t y0,
		uint16_t r,
		uint32_t color,
		uint8_t  segments
		)
{
	int f = 1 - r;
	int ddF_x = 1;
	int ddF_y = -2 * r;
	int x = 0;
	int y = r;

	if ((segments & 0x81) == 0x81)
		setPixel(s, x0, y0 - r, color); /* top */
	if ((segments & 0x18) == 0x18)
		setPixel(s, x0, y0 + r, color); /* bot */
	if ((segments & 0x06) == 0x06)
		setPixel(s, x0 + r, y0, color); /* right */
	if ((segments & 0x60) == 0x60)
		setPixel(s, x0 - r, y0, color); /* left */

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
		if (segments & 0x01)
			setPixel(s, x0 + x, y0 - y, color);
		if (segments & 0x02)
			setPixel(s, x0 + y, y0 - x, color);
		if (segments & 0x04)
			setPixel(s, x0 + y, y0 + x, color);
		if (segments & 0x08)
			setPixel(s, x0 + x, y0 + y, color);
		if (segments & 0x10)
			setPixel(s, x0 - x, y0 + y, color);
		if (segments & 0x20)
			setPixel(s, x0 - y, y0 + x, color);
		if (segments & 0x40)
			setPixel(s, x0 - y, y0 - x, color);
		if (segments & 0x80)
			setPixel(s, x0 - x, y0 - y, color);
	}
}

void Draw_Circle(
		SDL_Surface * s,
		uint16_t x0,
		uint16_t y0,
		uint16_t r,
		uint32_t color
		)
{
	Draw_Circle_Segments(s, x0, y0, r, color, 0xff);
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


void Draw_Line(SDL_Surface * s,
		uint16_t x0, uint16_t y0,
		uint16_t x1, uint16_t y1,
		uint32_t color)
{
	uint16_t x;
	uint16_t y;
	double m = ((double)y1 - (double)y0) / ((double)x1 - (double)x0);
	double delta = y0;
	if (m<1 && m>-1)
	{
		if (x1 < x0)
		{ /* swap */
			x  = x0;
			x0 = x1;
			x1 = x;
			x  = y0;
			y0 = y1;
			y1 = x;
		}
		delta = y0;
		for (x=x0; x <= x1; x++)
		{
			y = delta;
			setPixel(s, x, y, color);
			delta += m;
		}
	}else{
		if (y1 < y0)
		{ /* swap */
			x  = x0;
			x0 = x1;
			x1 = x;
			x  = y0;
			y0 = y1;
			y1 = x;
		}
		delta = x0;
		for (y=y0; y <= y1; y++)
		{
			x = delta;
			setPixel(s, x, y, color);
			delta += 1/m;
		}
	}
}


void mainloop_gfx(marfbed_t * b)
{
	SDL_Event e;
	int pause = 0;

	do
	{
		if (pause)
			usleep(100000);
		e.type = SDL_NOEVENT;
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
		/* key presses */
		if ( e.type == SDL_KEYDOWN )
		{
			/* p or space = pause */
			if ( e.key.keysym.sym == SDLK_SPACE ||
			     e.key.keysym.sym == SDLK_p )
				pause = pause ? 0 : 1;

			/* n = numbers */
			if ( e.key.keysym.sym == SDLK_n )
				b->show_numbers = b->show_numbers ? 0 : 1;

			/* i = info */
			if ( e.key.keysym.sym == SDLK_i )
				b->show_info = b->show_info ? 0 : 1;

			/* 1, 2, 3 = show ring 1, 2, 3 */
			if ( e.key.keysym.sym == SDLK_1 )
				b->show_ring_1 = b->show_ring_1 ? 0 : 1;
			if ( e.key.keysym.sym == SDLK_2 )
				b->show_ring_2 = b->show_ring_2 ? 0 : 1;
			if ( e.key.keysym.sym == SDLK_3 )
				b->show_ring_3 = b->show_ring_3 ? 0 : 1;

			/* l = lines */
			if ( e.key.keysym.sym == SDLK_l )
				b->show_lines = b->show_lines ? 0 : 1;
		}
	} while (pause);

	clearScreen(s);

	int i;
	if (b->show_lines)
	{
		for(i=0; i<MARF_MAX; i++)
		{
			if (!b->marf[i].enabled) continue;
			Draw_Line(s,
					b->marf[i].x,
					b->marf[i].y,
					b->marf[i].dest_x,
					b->marf[i].dest_y,
					0x00222222
				 );
		}
	}
	if (b->show_ring_3)
	{
		for(i=0; i<MARF_MAX; i++)
		{
			if (!b->marf[i].enabled) continue;

			Draw_Circle(s,
					b->marf[i].x * GFX_X / SPACE_X,
					b->marf[i].y * GFX_Y / SPACE_Y,
					RADIO_R_JAM,
					COLOR_RADIO_JAM
				   );
		}
	}
	if (b->show_ring_2)
	{
		for(i=0; i<MARF_MAX; i++)
		{
			if (!b->marf[i].enabled) continue;

			uint8_t seg = 0xff >> ((b->marf[i].proto.hello_count * 9) / b->marf[i].proto.hello_count_last_reload);
			Draw_Circle_Segments(s,
					b->marf[i].x * GFX_X / SPACE_X,
					b->marf[i].y * GFX_Y / SPACE_Y,
					RADIO_R,
					COLOR_RADIO,
					seg
				   );
		}
	}
	if (b->show_ring_1)
	{
		for(i=0; i<MARF_MAX; i++)
		{
			uint32_t col;
			if (!b->marf[i].enabled) continue;

			if (b->marf[i].moving)
				col = COLOR_MOVING;
			else
				col = b->marf[i].color;

			Draw_Circle(s,
					b->marf[i].x * GFX_X / SPACE_X,
					b->marf[i].y * GFX_Y / SPACE_Y,
					GFX_MARF_X,
					col
				   );
		}
	}


	if (b->show_numbers)
	{
		SDL_Color fg_color = {0xee, 0xee, 0xee, 0x00};
		SDL_Color bg_color = {0x33, 0x33, 0x33, 0x00};

		for(i=0; i<MARF_MAX; i++)
		{
			char buf[16];
			if (!b->marf[i].enabled) continue;

			SDL_Surface * surf_text;
			snprintf(buf, 16, "n%d", i);
#if 0
			surf_text = TTF_RenderText_Solid(
					b->font,
					buf,
					fg_color );
#else
			surf_text = TTF_RenderText_Shaded(
					b->font,
					buf,
					fg_color, bg_color );
#endif
			if (!surf_text)
			{
				printf("ERROR: font rendering: %s\n", TTF_GetError());
				exit(1);
			}
			SDL_Rect rect_welcome = { b->marf[i].x + 1, b->marf[i].y - 16, 40, 40 };
			if (SDL_BlitSurface(surf_text, NULL, s, &rect_welcome))
			{
				printf("ERROR: couldn't bit surface: %s\n", SDL_GetError());
				exit(1);
			}
			SDL_FreeSurface(surf_text);
		}
	}

	SDL_Flip(s);
}
