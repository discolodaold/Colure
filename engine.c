#include "proc.h"
#include "input.h"
#include "intro.h"
#include "animation.h"
#include "sound.h"
#include <sdl/SDL.h>
#include <sdl/SDL_image.h>

SDL_Surface *screen;

void engine_quit(void) {
	proc_cleanup();
	exit(0);
}

SDL_Surface *load_image(const char *filename) {
    SDL_Surface* loadedImage = NULL;
    SDL_Surface* optimizedImage = NULL;
    
    loadedImage = IMG_Load(filename);
    
    if(loadedImage != NULL) {
        optimizedImage = SDL_DisplayFormat(loadedImage);
        SDL_FreeSurface(loadedImage);
    }
    
	SDL_SetColorKey(optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB(optimizedImage->format, 0, 0, 0));

    return optimizedImage;
}

void draw_surface(SDL_Surface* source, int x, int y) {
    SDL_Rect offset;
    
    offset.x = x;
    offset.y = y;
    
    SDL_BlitSurface(source, NULL, screen, &offset);
}

static PT_THREAD(swap(proc_t *p)) {
	PT_BEGIN(&p->pt);
	while(1) {
		SDL_Flip(screen);
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
		//SDL_GL_SwapBuffers();
		PT_YIELD(&p->pt);
	}
	PT_END(&p->pt);
}

static PT_THREAD(die(proc_t *p)) {
	PT_BEGIN(&(p->pt));
	PT_WAIT_UNTIL(&(p->pt), input_take(SDLK_ESCAPE));
	engine_quit();
	PT_END(&(p->pt));
}

#define FPS 60

static PT_THREAD(fps(proc_t *p)) {
	static Uint32 framecount = 0;
	static double frametime = (1000.0 / (double)FPS);
	static Uint32 last_ticks, current_ticks, target_ticks;
	PT_BEGIN(&p->pt);
	last_ticks = SDL_GetTicks();
	while(1) {
		framecount++;
		current_ticks = SDL_GetTicks();
		target_ticks = last_ticks + (Uint32)((double)framecount * frametime);
		if(current_ticks < target_ticks) {
			SDL_Delay(target_ticks - current_ticks);
		} else {
			framecount = 0;
			last_ticks = current_ticks;
		}
		PT_YIELD(&p->pt);
	}
	PT_END(&p->pt);
}

int main(int argc, char *argv[]) {
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		printf("unable to initialize video: %s", SDL_GetError());
		return 1;
	}

	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(600, 400, 0, SDL_ANYFORMAT | SDL_DOUBLEBUF);
	if(screen == NULL) {
		printf("unable to set video mode: %s", SDL_GetError());
		return 1;
	}

	proc_init();
	sound_init();

	proc_new(NULL, swap,  0, NULL);
	proc_new(NULL, die,   0, NULL);
	proc_new(NULL, input, 0, NULL);
	proc_new(NULL, fps,   0, NULL);
	proc_new(NULL, intro, 0, NULL);

	proc_loop(); // never returns

	return 0;
}
