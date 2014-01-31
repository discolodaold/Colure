#include "shared.h"

struct entity head;

struct entity* entity_create(PT_THREAD((*fun)(struct entity* ent))) {
	struct entity* result = NULL;
	if(list_empty(&(head.free))) {
		result = malloc(sizeof(struct entity));
		list_add(&(head.active), &(result->active));
	} else {
		result = list_entry(&(head.free.next.free), struct entity, free);
		list_move(&(result->free), &(head.active));
	}
	
	result->fun = fun;
	result->fun(result); // use this to 'initialize' the new entity
}

void entity_delete(struct entity* ent) {
	list_move(&(ent->active), &(head.free));
}

static void entity_atexit(void) {
	struct entity *pos, *n;
	list_for_each_entry_safe(pos, n, &(head.active), active) {
		if(pos != &head)
			free(pos);
	}
	list_for_each_entry_safe(pos, n, &(head.free), free) {
		if(pos != &head)
			free(pos);
	}
}

void entity_init(void) {
	INIT_LIST_HEAD(&(head.list));
	atexit(entity_atexit);
}

