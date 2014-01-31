#ifndef QUEST_TILEMAP
#define QUEST_TILEMAP

#include "tileBlock.h"

struct tileMap {
	struct tileBlock *blocks[32][32];
};

void tileMap_generate(struct tileMap *tileMap, const char *tileset);

#endif
