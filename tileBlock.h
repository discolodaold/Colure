#ifndef QUEST_TILEBLOCK
#define QUEST_TILEBLOCK

#include "list.h"

struct tileBlock {
	struct list_head  cache;
	int               id;
	char             *tileset;
	char             *top;
	char             *left;
	char             *bottom;
	char             *right;
};

struct tileBlock *random_tileBlock(const char *tileset, const char *top, const char *right, const char *bottom, const char *left);

#endif
