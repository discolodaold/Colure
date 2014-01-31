#include "game.h"

#include "engine.h"
#include "map.h"
#include "proc.h"
#include "qstr.h"
#include "sound.h"

#include <math.h>
#include <sdl/SDL_ttf.h>

static int creep_count;
static int gold_count;

PT_THREAD(creeper(proc_t *p)) {
	static ent_t *creep;
	static float dx, dy, mag, imag, ox, oy;

	creep = (ent_t *)p->data;

	PT_BEGIN(&(p->pt));
	while(1) {
		if(creep->x > MAP_W - AO_SIZE*2) {
			creep_count--;
			sound_play(SFXR_LASER);
			break;
		}

		// perform some strong arming to help prevent creeps to get stuck
		if(creep->x < AO_SIZE) {
			creep->x = AO_SIZE + 1;
		}
		if(creep->y < AO_SIZE) {
			creep->y = AO_SIZE + 1;
		}
		if(creep->y >= MAP_H - AO_SIZE) {
			creep->y = MAP_H - AO_SIZE - 1;
		}

		draw_surface(creep->surface, creep->x - creep->surface->w/2, creep->y - creep->surface->h/2);

		ox = creep->x;
		oy = creep->y;

		dx = map_get(creep->x + 1, creep->y) - map_get(creep->x - 1, creep->y);
		dy = map_get(creep->x, creep->y + 1) - map_get(creep->x, creep->y - 1);
		mag = sqrt(dx*dx + dy*dy);
		if(mag != 0.0f) {
			imag = 1.0f / mag;
			dx *= imag;
			dy *= imag;
			creep->x += dx;
			creep->y += dy;
		}

		map_set(ox, oy, 0.0);
		PT_YIELD(&p->pt);
	}
	PT_END(&(p->pt));
}

static TTF_Font *VeraMono;
static SDL_Surface *dot;

void draw_text(int x, int y, TTF_Font *font, int r, int g, int b, const char *format, ...) {
	SDL_Color fg = {r, g, b, 255}, bg = {0, 0, 0, 255};
	SDL_Surface *surface;
	qstr *q;
	va_list va;
	va_start(va, format);
	q = qstr_from_formatv(format, va);
	va_end(va);
	surface = TTF_RenderText(font, qstr_cstr(q), fg, bg);
	qstr_free(q);
	draw_surface(surface, x, y);
	SDL_FreeSurface(surface);
}

PT_THREAD(creep_spawner(proc_t *p)) {
	static Uint32 next;
	static ent_t *creep;
	PT_BEGIN(&(p->pt));

	dot = load_image("./data/dodads/dot.bmp");

	while(1) {
		if(SDL_GetTicks() >= next) {
			next = SDL_GetTicks() + 1000;
			creep = (ent_t *)malloc(sizeof(*creep));
			creep->x = 32;
			creep->y = 16 + (rand() % (MAP_H - 16));
			creep->surface = dot;
			proc_new(NULL, creeper, 1, creep);
			creep_count++;
		}
		draw_text(0, MAP_H + 32, VeraMono, 255, 255, 255, "creeps: %i", creep_count);
		PT_YIELD(&p->pt);
	}
	PT_END(&(p->pt));
}

PT_THREAD(game_rules(proc_t *p)) {
	PT_BEGIN(&(p->pt));

	TTF_Init();
	VeraMono = TTF_OpenFont("./data/fonts/VeraMono.ttf", 16);

	map_start();
	proc_new(NULL, creep_spawner, 0, NULL);

	while(1) {

		PT_YIELD(&p->pt);
	}
	PT_END(&(p->pt));
}

void game_start(void) {
	proc_new(NULL, game_rules, 0, NULL);
}
