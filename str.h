#ifndef QUEST_STR
#define QUEST_STR

#include <stdlib.h>

size_t string_length(const char *s);
char *string_format(const char *s, ...);
char *string_dup(const char *s);
int string_compare(const char *a, const char *b);

#endif
