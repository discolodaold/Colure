#ifndef COLURE_DB
#define COLURE_DB

#include "sqlite3.h"
#include "qstr.h"

#include <stdlib.h>

static int dbGetColumnIndex(int columns, char **names, char *name) {
	int c;
	for(c = 0; c < columns; c++) {
		if(qstr_cmp(QSTR(names[c]), QSTR(name)) == 0) {
			return c;
		}
	}
	return 0;
}

int dbRunStatement(sqlite3_stmt *stmt, char *params, va_list a, void *context, int (*cb)(void *, int, char **, char **));

#define RANDWEIGHT " ORDER BY abs(RANDOM())/weight LIMIT 1"

#define DBSELECT(NAME, SQL, PARAMS) \
static sqlite3_stmt *NAME ## _stmt = NULL; \
static char *NAME ## _params = PARAMS; \
static int NAME ## _cb(void *context, int column, char **text, char **names); \
int NAME(void *context, ...) { \
	va_list a; \
	if(NAME ## _stmt == NULL) { \
		sqlite3_prepare_v2(db_instance(), SQL, -1, &NAME ## _stmt, NULL); \
	} \
	va_start(a, context); \
	dbRunStatement(NAME ## _stmt, NAME ## _params, a, context, NAME ## _cb); \
	va_end(a); \
} \
static int NAME ## _cb(void *context, int columns, char **text, char **names)

#define DBCALLBACK(name) int name (void *p, int columns, char **text, char **names)
#define DBCOLUMNINDEX(name) dbGetColumnIndex(columns, names, name)
#define DBSTRING(INDEX) string_dup(text[INDEX] ? text[INDEX] : "")
#define DBINT(INDEX) atoi(text[INDEX] ? text[INDEX] : "-1")
#define DBDOUBLE(INDEX) atof(text[INDEX] ? text[INDEX] : "-1")

sqlite3 *db_instance(void);

#endif

