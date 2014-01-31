#include "proc.h"

#include <stdlib.h>
#include <string.h>

static struct list_head head_active;
static struct list_head head_free;
static struct list_head handler_free;

void proc_remove(proc_t *p) {
	proc_eventHandler_t *e, *n;

	if(p->free_data && p->data) {
		free(p->data);
		p->data = NULL;
		p->free_data = 0;
	}

	list_for_each_entry_safe(e, n, proc_eventHandler_t, &(p->handlers), list) {
		list_move(&e->list, &handler_free);
	}

	list_move(&p->list, &head_free);
}

void proc_morph(proc_t *p, PT_THREAD((*fun)(proc_t* p)), int free_data, void* data) {
	p->fun = fun;
	p->free_data = free_data;
	p->data = data;
	p->fun(p);
}

void proc_data(proc_t *p, int free_data, void* data) {
	p->free_data = free_data;
	p->data = data;
}

void proc_cleanup(void) {
	proc_t *p, *pn;
	proc_eventHandler_t *e, *en;

	list_for_each_entry_safe(p, pn, proc_t, &head_active, list) {
		proc_remove(p);
	}
	INIT_LIST_HEAD(&head_active);

	list_for_each_entry_safe(p, pn, proc_t, &head_free, list) {
		free(p);
	}
	INIT_LIST_HEAD(&head_free);

	list_for_each_entry_safe(e, en, proc_eventHandler_t, &handler_free, list) {
		free(e);
	}
	INIT_LIST_HEAD(&handler_free);
}

void proc_init(void) {
	INIT_LIST_HEAD(&head_active);
	INIT_LIST_HEAD(&head_free);
	INIT_LIST_HEAD(&handler_free);
	atexit(proc_cleanup);
}

void proc_new(proc_t **p, PT_THREAD((*fun)(proc_t* p)), int free_data, void* data) {
	proc_t* result;
	if(list_empty(&head_free)) {
		result = (proc_t *)malloc(sizeof(proc_t));
		list_add_tail(&(result->list), &head_active);
	} else {
		result = list_entry(head_free.next, proc_t, list);
		list_move_tail(&(result->list), &head_active);
	}
	
	PT_INIT(&(result->pt));
	result->fun = fun;
	result->free_data = free_data;
	result->data = data;
	INIT_LIST_HEAD(&result->handlers);
	if(NULL != p)
		*p = result;
}

void proc_addHandler(proc_t *p, const char *key, proc_eventHandler_f handler) {
	proc_eventHandler_t *result;
	if(list_empty(&handler_free)) {
		result = (proc_eventHandler_t *)malloc(sizeof(*result));
	} else {
		result = list_entry(handler_free.next, proc_eventHandler_t, list);
		list_del(&result->list);
	}
	result->key = key;
	result->handler = handler;
	list_add(&result->list, &p->handlers);
}

void proc_send(proc_t *p, const char *key, void *data) {
	proc_eventHandler_t *e, *n;
	list_for_each_entry_safe(e, n, proc_eventHandler_t, &p->handlers, list) {
		if(key == e->key) {
			e->handler(p, data);
		}
	}
}

void proc_loop(void) {
	proc_t* p;
	proc_t* n = list_entry(head_active.next, proc_t, list);

	while(0 == list_empty(&head_active)) {
		p = n;
		n = p->list.next == &head_active               ?
			list_entry(head_active.next, proc_t, list) :
			list_entry(p->list.next, proc_t, list)     ;
		if(0 == PT_SCHEDULE(p->fun(p))) {
			if(1 == list_empty(&head_active)) {
				return;
			}
			proc_remove(p);
		}
	}
}
