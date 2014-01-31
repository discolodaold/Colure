#ifndef COLURE_PROC
#define COLURE_PROC

#include "list.h"
#include "pt.h"
#include "pt-sem.h"

typedef struct proc_s proc_t;

typedef void (*proc_eventHandler_f)(proc_t *, void *);
typedef struct proc_eventHandler_s proc_eventHandler_t;
struct proc_eventHandler_s {
	struct list_head     list;
	const char          *key;
	proc_eventHandler_f  handler;
};

struct proc_s {
	struct list_head list;

	struct pt pt;
	PT_THREAD((*fun)(proc_t *p));

	int free_data;
	void* data;

	struct list_head handlers;
};

void proc_remove(proc_t *p);
void proc_morph(proc_t *p, PT_THREAD((*fun)(proc_t* p)), int free_data, void* data);
void proc_init(void);
void proc_cleanup(void);
void proc_new(proc_t **p, PT_THREAD((*fun)(proc_t *p)), int free_data, void *data);
void proc_data(proc_t *p, int free_data, void *data);
void proc_addHandler(proc_t *p, const char *key, proc_eventHandler_f handler);
void proc_send(proc_t *p, const char *key, void *data);
void proc_loop(void);

#endif
