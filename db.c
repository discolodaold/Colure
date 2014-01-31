#include "db.h"

static sqlite3 *db = NULL;

sqlite3 *db_instance(void) {
	if(db == NULL) {
		sqlite3_open_v2("quest.db", &db, SQLITE_READONLY, NULL);
	}
	return db;
}

int dbRunStatement(sqlite3_stmt *stmt, char *params, va_list a, void *context, int (*cb)(void *, int, char **, char **)) {
	int c, columns, rows = 0, column;
	char **text, **name;
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	for(c = 0; params[c]; c++) {
		switch(params[c]) {
		case 'i':
			sqlite3_bind_int(stmt, c, va_arg(a, int));
			break;
		case 'd':
			sqlite3_bind_double(stmt, c, va_arg(a, double));
			break;
		case 't':
			{
				char *text = va_arg(a, char*);
				sqlite3_bind_text(stmt, c, text, qstr_len(QSTR(text)), SQLITE_TRANSIENT);
			}
			break;
		}
	}
	columns = sqlite3_column_count(stmt);
	text = (char **)malloc(sizeof(char*) * columns);
	name = (char **)malloc(sizeof(char*) * columns);
	while(1) {
		switch(sqlite3_step(stmt)) {
		case SQLITE_ROW:
			for(column = 0; column < columns; column++) {
				text[column] = (char *)sqlite3_column_text(stmt, column);
				name[column] = (char *)sqlite3_column_name(stmt, column);
			}
			cb(context, columns, text, name);
			rows++;
		default:
			goto cleanup;
		}
	}
cleanup:
	free(text);
	free(name);
	return rows;
}
