#ifndef COLURE_MAP
#define COLURE_MAP

#define MAP_W (512)
#define MAP_H (256)

#define TILE_SIZE (32)
#define TILE_W (MAP_W / TILE_SIZE)
#define TILE_H (MAP_H / TILE_SIZE)

#define AO_SIZE (8)
#define AO_W (MAP_W / AO_SIZE)
#define AO_H (MAP_H / AO_SIZE)

void map_start(void);
int map_blocked(int x, int y);
float map_get(int x, int y);
void map_set(int x, int y, float v);

#endif