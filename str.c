#include "str.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

size_t string_length(const char *s) {
	size_t length = 0;
	while(s[length++]) ;
	return length;
}

char *string_format(const char *s, ...) {
	va_list v1;
	int numBytes;
	char *result;

	va_start(v1, s);
#ifdef _MSC_VER
	numBytes = _vscprintf(s, v1) + 1;
#else
	numBytes = vsnprintf(NULL, 0, s, v1) + 1;
#endif
	va_end(v1);

	result = (char *)malloc(sizeof(char) * numBytes);
	if(result == NULL) {
		return NULL;
	}

	va_start(v1, s);
#ifdef _MSC_VER
	vsprintf_s(result, sizeof(char) * numBytes, s, v1);
#else
	vsprintf(result, s, v1);
#endif
	va_end(v1);

	return result;
}

char *string_dup(const char *s) {
	int size = sizeof(char) * string_length(s) + 1;
	char *result = (char *)malloc(size);
	memcpy(result, s, size);
	return result;
}

int string_compare(const char *a, const char *b) {
	int result;

	while((result = *(unsigned char *)a - *(unsigned char *)b) == 0 && *b) {
		++a;
		++b;
	}

	return result < 0 ? -1 :
		   result > 0 ?  1 : 0;
}

