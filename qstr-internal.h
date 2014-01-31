#ifndef Q_QSTR_INTERNAL_H
#define Q_QSTR_INTERNAL_H

#include "qstr.h"

size_t qstr_formatv_impl(char *buffer, size_t buffer_length, const char *format, va_list ap);

#endif
