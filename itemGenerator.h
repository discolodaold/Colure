#ifndef QUEST_ITEMGENERATOR
#define QUEST_ITEMGENERATOR

#include "item.h"

int generate_item(const char *name, struct item **item);
void generateN_items(const char *name, size_t n, size_t *items, struct item ***item);

#endif
