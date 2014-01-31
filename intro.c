#include "intro.h"
#include "animation.h"
#include "engine.h"
#include "game.h"

#include <stdio.h>
#include <sdl/SDL.h>

static int count = 0;

static int done = 0;
static void _animationDone(proc_t *p, void *unused) {
	done = 1;
}

SDL_Surface *logo;
SDL_Rect logoRect;

PT_THREAD(intro(proc_t *p)) {
	static int i;
	PT_BEGIN(&(p->pt));

	logo = SDL_LoadBMP("./data/intro/sdllogo.bmp");
	logoRect.x = screen->w/2 - logo->w/2;

	proc_addHandler(p, ANIMATION_DONE, _animationDone);
	new_shortAnimation(p, &logoRect.y, 0, screen->h/2 - logo->h/2, 1000.0, easeOutBounce);
	while(done == 0) {
		SDL_BlitSurface(logo, NULL, screen, &logoRect);
		PT_YIELD(&p->pt);
	}

	game_start();

	PT_END(&(p->pt));
}
