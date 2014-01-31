#include "input.h"
#include "engine.h"
#include <sdl/SDL.h>

static int input_states[SDLK_LAST];

int input_take(SDLKey key) {
	if(input_states[key]) {
		input_states[key] = 0;
		return 1;
	}
	return 0;
}

PT_THREAD(input(proc_t *p)) {
	SDL_Event ev;
	PT_BEGIN(&(p->pt));
	while(1) {
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
			case SDL_KEYUP:
				input_states[ev.key.keysym.sym] = 0;
				break;
			case SDL_KEYDOWN:
				input_states[ev.key.keysym.sym] = 1;
				break;
			case SDL_QUIT:
				engine_quit();
				break;
			}
		}
		PT_YIELD(&(p->pt));
	}
	PT_END(&(p->pt));
}
