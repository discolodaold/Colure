#include "map.h"

#include "engine.h"
#include "proc.h"
#include "game.h"

#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <time.h>

static char tile[TILE_W * TILE_H];
#define TILE(X, Y) tile[((Y)/TILE_SIZE) * TILE_W + ((X)/TILE_SIZE)]

static float ao[AO_W * AO_H];
#define AO(X, Y) ao[((Y)/AO_SIZE) * AO_W + ((X)/AO_SIZE)]

float map_getRaw(int x, int y) {
	if(x < 0) x = 0;
	if(x >= MAP_W) x = MAP_W-1;
	if(y < 0) y = 0;
	if(y >= MAP_H) y = MAP_H-1;
	return AO(x, y);
}

static float zero = 0.0f;

PT_THREAD(calculate_ao(proc_t *p)) {
	size_t x, y;
	static float v, w;
	PT_BEGIN(&(p->pt));
	while(1) {
		for(x = 0; x < MAP_W; x += AO_SIZE) {
			for(y = 0; y < MAP_H; y += AO_SIZE) {
				if(TILE(x, y) != ' ') {
					continue;
				}
				v = 0.0f;
				w = 0.0f;
				if(x > 0) { v += AO(x-AO_SIZE, y); w += 1.0f; }
				if(y > 0) { v += AO(x, y-AO_SIZE); w += 1.0f; }
				if(x < MAP_W-AO_SIZE) { v += AO(x+AO_SIZE, y); w += 1.0f; }
				if(y < MAP_H-AO_SIZE) { v += AO(x, y+AO_SIZE); w += 1.0f; }
				v /= w;
				if(v < -100.0f) v = -100.0f;
				if(v >  100.0f) v =  100.0f;
				AO(x, y) = v;
			}
		}
		PT_YIELD(&p->pt);
	}
	PT_END(&(p->pt));
}

static SDL_Surface *dirt;
static SDL_Surface *debris;
static SDL_Surface *building;
static SDL_Surface *bridge;


PT_THREAD(render(proc_t *p)) {
	static size_t x, y;
	PT_BEGIN(&(p->pt));

	dirt = load_image("./data/dodads/dirt.bmp");
	debris = load_image("./data/dodads/debris.bmp");
	building = load_image("./data/dodads/building.bmp");
	bridge = load_image("./data/dodads/bridge.bmp");

	while(1) {
		for(y = 0; y < MAP_H; y += AO_SIZE) {
			map_set(MAP_W - AO_SIZE, y,  100.0);
		}
		for(x = 0; x < MAP_W; x += TILE_SIZE) {
			for(y = 0; y < MAP_H; y += TILE_SIZE) {
				draw_surface(dirt, x, y);
				switch(TILE(x, y)) {
				case '.':
					draw_surface(debris, x, y);
					break;
				case 'H':
					draw_surface(bridge, x, y);
					break;
				case 'B':
					draw_surface(building, x, y);
					break;
				default:
					break;
				}
			}
		}

		PT_YIELD(&p->pt);
	}
	PT_END(&(p->pt));
}


void map_start(void) {
	size_t i, x, y;
	memset(tile, ' ', sizeof(tile));
	memset(ao, 0, sizeof(ao));

	srand ( time(NULL) );

	for(i = 0; i < TILE_W; ++i) {
		x = rand() % MAP_W;
		y = rand() % MAP_H;
		if(TILE(x, y) == ' ') {
			TILE(x, y) = '.';
			//++i;
		}
	}
	proc_new(NULL, calculate_ao, 0, NULL);
	proc_new(NULL, render, 0, NULL);

}

int map_blocked(int x, int y) {
	if(x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
		return 1;
	}
	return TILE(x, y) != ' ';
}

float map_get(int x, int y) {
	if(!map_blocked(x, y)) {
		// perform bilinear filtering
		float u = ((float)(x - AO_SIZE/2) / (float)AO_SIZE);
		float v = ((float)(y - AO_SIZE/2) / (float)AO_SIZE);
		int ui = (int)floor(u);
		int vi = (int)floor(v);
		float u_ratio = u - ui;
		float v_ratio = v - vi;
		float u_opposite = 1 - u_ratio;
		float v_opposite = 1 - v_ratio;
		return (map_getRaw((ui)*AO_SIZE, (vi)*AO_SIZE)   * u_opposite + map_getRaw((ui+1)*AO_SIZE, (vi)*AO_SIZE)   * u_ratio) * v_opposite +
			   (map_getRaw((ui)*AO_SIZE, (vi+1)*AO_SIZE) * u_opposite + map_getRaw((ui+1)*AO_SIZE, (vi+1)*AO_SIZE) * u_ratio) * v_ratio;
	}
	return 0.0f;
}

void map_setRaw(int x, int y, float v, float a) {
	if(x < 0) return;
	if(x >= MAP_W) return;
	if(y < 0) return;
	if(y >= MAP_H) return;
	AO(x, y) = (AO(x, y) * a) + (v * (1.0 - a));
}

void map_set(int x, int y, float value) {
	if(!map_blocked(x, y)) {
		// perform bilinear filtering
		float u = ((float)(x - AO_SIZE/2) / (float)AO_SIZE);
		float v = ((float)(y - AO_SIZE/2) / (float)AO_SIZE);
		int ui = (int)floor(u);
		int vi = (int)floor(v);
		float u_ratio = u - ui;
		float v_ratio = v - vi;
		float u_opposite = 1.0f - u_ratio;
		float v_opposite = 1.0f - v_ratio;
		map_setRaw((ui)*AO_SIZE, (vi)*AO_SIZE, value, u_opposite * v_opposite);
		map_setRaw((ui+1)*AO_SIZE, (vi)*AO_SIZE, value, u_ratio * v_opposite);
		map_setRaw((ui)*AO_SIZE, (vi+1)*AO_SIZE, value, u_opposite * v_ratio);
		map_setRaw((ui+1)*AO_SIZE, (vi+1)*AO_SIZE, value, u_ratio * v_ratio);
	}
}

