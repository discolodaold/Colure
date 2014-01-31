#include "item.h"

#include <stdlib.h>
#include <string.h>

static LIST_HEAD(ib_cache);

DBSELECT(ib_get, "SELECT id, name, displayName, type, armor, damageMin, damageMax, range, value, strength, dexterity, magic, fire, water, wind, earth FROM Item WHERE id = ?", "i") {
	struct item_base *item = (struct item_base *)context;
	item->id             = DBINT( 0);
	item->name           = DBSTRING( 1);
	item->displayName    = DBSTRING( 2);
	item->type           = DBINT( 3);
	item->armor          = DBINT( 4);
	item->damageMin      = DBINT( 5);
	item->damageMax      = DBINT( 6);
	item->range          = DBINT( 7);
	item->attr.value     = DBINT( 8);
	item->attr.strength  = DBINT( 9);
	item->attr.dexterity = DBINT(10);
	item->attr.magic     = DBINT(11);
	item->attr.fire      = DBINT(12);
	item->attr.water     = DBINT(13);
	item->attr.wind      = DBINT(14);
	item->attr.earth     = DBINT(15);
	return 0;
}

struct item_base *get_item_base(int id) {
	struct item_base *ib;
	list_for_each_entry(ib, struct item_base, &ib_cache, cache) {
		if(ib->id == id) {
			return ib;
		}
	}
	ib = (struct item_base *)malloc(sizeof(*ib));
	ib_get(ib, id);
	list_add(&ib->cache, &ib_cache);
	return ib;
}

DBSELECT(ib_randomId, "SELECT id FROM Item WHERE type = ? ORDER BY RANDOM() LIMIT 1", "i") {
	*(int*)context = DBINT(0);
	return 0;
}

int randomId_item_base(int rank) {
	int id;
	ib_randomId(&id, rank);
	return id;
}

struct item *new_item(int unit, int rank) {
	struct item *result;
	result = (struct item *)malloc(sizeof(*result));
	result->base = get_item_base(unit);
	item_enchant(result, rank);
	return result;
}

void item_enchant(struct item *item, int rank) {
	char *s1 = NULL, *s2 = NULL;
	int e;
	struct attr *attr;

	while(rank > 0) {
		item->enchantment = (struct itemEnchantment **)realloc(item->enchantment, sizeof(*item->enchantment) * (item->enchantments + 1));
		item->enchantment[item->enchantments++] = new_itemEnchantment(rank);
		rank -= item->enchantment[item->enchantments-1]->base->rank;
	}

	qsort(item->enchantment, item->enchantments, sizeof(*item->enchantment), comparator_itemEnchantment);
	
	memset(&item->passive, 0, sizeof(item->passive));
	memset(&item->direct, 0, sizeof(item->direct));

	for(e = 0; e < item->enchantments; e++) {
		if(e == 0) {
			s1 = string_format(item->enchantment[e]->base->format, item->base->displayName);
		}
		if(e == 1) {
			s2 = string_format(item->enchantment[e]->base->format, s1);
		}
		attr = item->enchantment[e]->base->type == 0 ? &item->passive : &item->direct;
		attr->value     += item->enchantment[e]->attr.value;
		attr->strength  += item->enchantment[e]->attr.strength;
		attr->dexterity += item->enchantment[e]->attr.dexterity;
		attr->magic     += item->enchantment[e]->attr.magic;
		attr->fire      += item->enchantment[e]->attr.fire;
		attr->water     += item->enchantment[e]->attr.water;
		attr->wind      += item->enchantment[e]->attr.wind;
		attr->earth     += item->enchantment[e]->attr.earth;
	}
}
