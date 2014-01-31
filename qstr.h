#ifndef COLURE_QSTR
#define COLURE_QSTR

#include <stdarg.h>
#include <stdlib.h>

struct qstr;

typedef struct qstr qstr;

#define QSTR(s) ((struct qstr *)s)

size_t qstr_len(struct qstr *);
const char *qstr_cstr(struct qstr *);
struct qstr *qstr_dup(struct qstr *);
void qstr_free(struct qstr *);

struct qstr *qstr_substring(struct qstr *, size_t, size_t);
int qstr_cmp(struct qstr *, struct qstr *);

struct qstr *qstr_from_buf(const char *, size_t);
struct qstr *qstr_from_formatv(const char *, va_list);
struct qstr *qstr_from_format(const char *, ...);

struct qstr *qstr_add_buf(struct qstr *, const char *, size_t);
struct qstr *qstr_add_qstr(struct qstr *, struct qstr *);
struct qstr *qstr_add_str(struct qstr *, const char *);
struct qstr *qstr_add_formatv(struct qstr *, const char *, va_list);
struct qstr *qstr_add_format(struct qstr *, const char *, ...);

#endif
