#include "itemGenerator.h"

#include "db.h"

struct itemGenerator_rule {
	int         type;
	int         unit;
	const char *generator;
	int         enchantments;
};

DBSELECT(rules_getRandom, "SELECT type, unit, generator, enchantments FROM ItemGenRule WHERE name = ?" RANDWEIGHT, "s") {
	struct itemGenerator_rule *rule = (struct itemGenerator_rule *)context;
	rule->type         = DBINT(0);
	rule->unit         = DBINT(1);
	rule->generator    = DBSTRING(2);
	rule->enchantments = DBINT(3);
	return 0;
}

int generate_item(const char *name, struct item **item) {
	struct itemGenerator_rule rule;
	if(rules_getRandom(&rule, name) == 0) {
		return 0;
	}
	if(rule.type >= 0) {
		rule.unit = 0;
	}
	if(rule.unit >= 0) {
		*item = new_item(rule.unit, rule.enchantments);
		return 1;
	}
	return generate_item(rule.generator, item);
}

void generateN_items(const char *name, size_t n, size_t *items, struct item ***item) {
	struct item *nitem;
	while(n--) {
		if(generate_item(name, &nitem)) {
			*item = (struct item **)realloc(*item, sizeof(**item) * (*items + 1));
			(*item)[(*items)++] = nitem;
		}
	}
}

