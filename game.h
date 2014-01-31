#ifndef COLURE_GAME
#define COLURE_GAME

#include <sdl/SDL.h>

typedef struct {
	float x;
	float y;
	SDL_Surface *surface;
} ent_t;

void game_start(void);

#endif