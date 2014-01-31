#ifndef QUEST_ITEMENCHANTMENT
#define QUEST_ITEMENCHANTMENT

#include "db.h"
#include "attr.h"

#include "list.h"

/**
 * Item enchantment data from the database.
 */
struct itemEnchantment_base {
	struct list_head cache;
	int              id;
	const char      *format;
	int              type;
	int              usage;
	int              rank;
	int              min;
	int              max;
	struct attr      attr;
};

/**
 * An instance of a enchantment
 */
struct itemEnchantment {
	const struct itemEnchantment_base *base;
	struct attr                        attr;
};

struct itemEnchantment_base *random_itemEnchantment_base(int rank);
int comparator_itemEnchantment(const void *a, const void *b);
struct itemEnchantment *new_itemEnchantment(int rank);

#endif

