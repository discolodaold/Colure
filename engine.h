#ifndef COLURE_ENGINE
#define COLURE_ENGINE

#include <sdl/SDL.h>

extern SDL_Surface *screen;

void engine_quit(void);
SDL_Surface *load_image(const char *filename);
void draw_surface(SDL_Surface* source, int x, int y);

#endif
