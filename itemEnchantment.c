#include "itemEnchantment.h"

#include "db.h"
#include "attr.h"

static LIST_HEAD(ie_cache);

DBSELECT(ie_get, "SELECT id, format, type, usage, rank, min, max, value, strength, dexterity, magic, fire, water, wind, earth FROM ItemEnchantment WHERE id = ?", "i") {
	struct itemEnchantment_base *ie = (struct itemEnchantment_base *)context;
	ie->id             = DBINT( 0);
	ie->format         = DBSTRING( 1);
	ie->type           = DBINT( 2);
	ie->usage          = DBINT( 3);
	ie->rank           = DBINT( 4);
	ie->min            = DBINT( 5);
	ie->max            = DBINT( 6);
	ie->attr.value     = DBINT( 7);
	ie->attr.strength  = DBINT( 8);
	ie->attr.dexterity = DBINT( 9);
	ie->attr.magic     = DBINT(10);
	ie->attr.fire      = DBINT(11);
	ie->attr.water     = DBINT(12);
	ie->attr.wind      = DBINT(13);
	ie->attr.earth     = DBINT(14);
	return 0;
}

DBSELECT(ie_randomId, "SELECT id FROM ItemEnchantment WHERE rank <= ? ORDER BY RANDOM() LIMIT 1", "i") {
	*(int*)context = DBINT(0);
	return 0;
}

struct itemEnchantment_base *random_itemEnchantment_base(int rank) {
	int id;
	struct itemEnchantment_base *ie;
	ie_randomId(&id, rank);
	list_for_each_entry(ie, struct itemEnchantment_base, &ie_cache, cache) {
		if(ie->id == id) {
			return ie;
		}
	}
	ie = (struct itemEnchantment_base *)malloc(sizeof(*ie));
	ie_get(ie, id);
	list_add(&ie->cache, &ie_cache);
	return ie;
}

int comparator_itemEnchantment(const void *a, const void *b) {
	return ((struct itemEnchantment *)b)->base->rank - ((struct itemEnchantment *)a)->base->rank;
}

struct itemEnchantment *new_itemEnchantment(int rank) {
	struct itemEnchantment *ie;
	double scale;
	ie = (struct itemEnchantment *)malloc(sizeof(*ie));
	ie->base = random_itemEnchantment_base(rank);
	scale = (double)(ie->base->min + (rand() % (ie->base->max - ie->base->min))) / 100.0;
	ie->attr.value     = ie->base->attr.value     * scale;
	ie->attr.strength  = ie->base->attr.strength  * scale;
	ie->attr.dexterity = ie->base->attr.dexterity * scale;
	ie->attr.magic     = ie->base->attr.magic     * scale;
	ie->attr.fire      = ie->base->attr.fire      * scale;
	ie->attr.water     = ie->base->attr.water     * scale;
	ie->attr.wind      = ie->base->attr.wind      * scale;
	ie->attr.earth     = ie->base->attr.earth     * scale;
	return ie;
}
