#ifndef COLURE_INPUT
#define COLURE_INPUT

#include "proc.h"

#include <sdl/SDL.h>

int input_take(SDLKey key);
PT_THREAD(input(proc_t *p));

#endif
