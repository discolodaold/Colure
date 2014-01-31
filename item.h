#ifndef QUEST_ITEM
#define QUEST_ITEM

#include "db.h"
#include "attr.h"
#include "itemEnchantment.h"

/**
 * Item data from the database.
 */
struct item_base {
	struct list_head   cache;
	int                id;
	char              *name;
	char              *displayName;
	int                type;
	int                armor;
	int                damageMin;
	int                damageMax;
	int                range;
	struct attr        attr;
};

/**
 * An instance of an item from the database, plus enchantments.
 */
struct item {
	struct item_base        *base;
	struct attr              passive;
	struct attr              direct;
	size_t                   enchantments;
	struct itemEnchantment **enchantment;
};

struct item *new_item(int unit, int rank);
void item_enchant(struct item *item, int rank);

#endif

