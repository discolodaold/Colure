#include "qstr-internal.h"

#include <stdint.h>

struct format_info {
	char type;
	int precision;
	int width;
	unsigned int flag;
	char pad;
	unsigned int length;
};

enum {
	FLAG_ALT         =  1, // '#'
	FLAG_SPACE       =  2, // ' '
	FLAG_LEFT        =  4, // '-'
	FLAG_SHOWSIGN    =  8, // '+'
	FLAG_ZERO        = 16, // '0'
	FLAG_PARSE       = 32
};

enum {
	LENGTH_CHAR,      // hh
	LENGTH_SHORT,     // h
	LENGTH_NONE,
	LENGTH_LONG,      // l
	LENGTH_LONGLONG,  // ll
	LENGTH_LONGDOUBLE // L
};

struct format_output {
	char *buffer;
	size_t length;
	size_t offset;
};

static void fo_emit(struct format_output *fo, char c) {
	if(++fo->offset < fo->length) {
		*fo->buffer++ = c;
	}
}

static void fo_emit_many(struct format_output *fo, char c, size_t times) {
	while(times--) {
		fo_emit(fo, c);
	}
}

#define CHAR_BIT 8
static void fo_emit_integer(struct format_output *fo, uintmax_t num, unsigned int base, char *chars) {
	char buffer[sizeof(uintmax_t) * CHAR_BIT];
	char *bptr = buffer;

	do {
		*bptr++ = chars[num % base];
	} while(num /= base);

	while(bptr != buffer) {
		fo_emit(fo, *--bptr);
	}
}

static int parse_num10(const char **format) {
	int x = 0;
	while((**format >= '0') && (**format <= '9'))
		x = x * 10 + (*((*format)++) - '0');
	return x;
}

#define FMT_START \
	{ \
		unsigned int pass; \
		struct format_output data_fo; \
		struct format_output begin_fo = *fo; \
		if(fi->precision < 0) { \
			fi->precision = 0; \
		} \
		for(pass = (fi->precision || fi->width) ? 0 : 1; pass < 2; ++pass) {

#define FMT_END \
			if(pass == 1) { \
				break; \
			} \
			fi->precision -= (fo->offset - data_fo.offset); \
			if(fi->precision < 0) { \
				fi->precision = 0; \
			} \
			fi->width -= (fo->offset - begin_fo.offset) + fi->precision; \
			if(fi->width < 0) { \
				fi->width = 0; \
			} \
			*fo = begin_fo; \
			if(fi->flag & FLAG_ZERO) { \
				fo_emit_many(fo, '0', fi->width); \
			} else if(!(fi->flag & FLAG_LEFT)) { \
				fo_emit_many(fo, ' ', fi->width); \
			} \
		} \
		if(fi->flag & FLAG_LEFT) { \
			fo_emit_many(fo, ' ', fi->width); \
		} \
	}

struct number {
	union {
		unsigned long long int longlong;
		unsigned long int word;
	} v;
	int is_negative;
};

static void get_signed(struct number *number, struct format_info *fi, va_list *ap_ptr) {
	if(fi->length == LENGTH_LONGLONG) {
		long long int signed_number = va_arg((*ap_ptr), long long int);
		number->is_negative = signed_number < 0;
		number->v.longlong = number->is_negative ? (- signed_number) : signed_number;
	} else {
		long int signed_number;
		if(fi->length == LENGTH_LONG) {
			signed_number = va_arg((*ap_ptr), long int);
		} else {
			signed_number = va_arg((*ap_ptr), int);
		}
		number->is_negative = signed_number < 0;
		number->v.word = number->is_negative ? (- signed_number) : signed_number;
	}
}

static void get_unsigned(struct number *number, struct format_info *fi, va_list *ap_ptr) {
	number->is_negative = 0;
	if(fi->length == LENGTH_LONGLONG) {
		number->v.longlong = va_arg((*ap_ptr), unsigned long long int);
	} else {
		if(fi->length == LENGTH_LONG) {
			number->v.word = va_arg((*ap_ptr), long int);
		} else {
			number->v.word = va_arg((*ap_ptr), int);
		}
	}
}

static void print_signed(struct format_output *fo, struct format_info *fi, struct number *number, int base, char *chars) {
	FMT_START
		if(number->is_negative) {
			fo_emit(fo, '-');
		} else if(fi->flag & FLAG_SHOWSIGN) {
			fo_emit(fo, '+');
		} else if(fi->flag & FLAG_SPACE) {
			fo_emit(fo, ' ');
		}

		fo_emit_many(fo, '0', pass * fi->precision);

		data_fo = *fo;

		if(fi->length == LENGTH_LONGLONG) {
			fo_emit_integer(fo, number->v.longlong, base, chars);
		} else {
			fo_emit_integer(fo, number->v.word, base, chars);
		}
	FMT_END
}

static void print_unsigned(struct format_output *fo, struct format_info *fi, struct number *number, int base, char *chars, char *alt) {
	char *tmp;
	FMT_START
		if(fi->flag & FLAG_ALT) {
			tmp = alt;
			while(*tmp) {
				fo_emit(fo, *tmp++);
			}
		}

		fo_emit_many(fo, '0', pass * fi->precision);

		data_fo = *fo;

		if(fi->length == LENGTH_LONGLONG) {
			fo_emit_integer(fo, number->v.longlong, base, chars);
		} else {
			fo_emit_integer(fo, number->v.word, base, chars);
		}
	FMT_END
}

/* Print an int as a signed decimal number. '%d' and '%i' are synonymous for
 * output, but are different when used with scanf() for input. */
static void convert_d(struct format_output *fo, struct format_info *fi, va_list *ap_ptr) {
	struct number number;
	get_signed(&number, fi, ap_ptr);
	print_signed(fo, fi, &number, 10, "0123456789");
}

/* Print decimal unsigned int. */
void convert_u(struct format_output *fo, struct format_info *fi, va_list *ap_ptr) {
	struct number number;
	get_unsigned(&number, fi, ap_ptr);
	print_unsigned(fo, fi, &number, 10, "0123456789", "");
}

/* Print a double in normal (fixed-point) notation. 'f' and 'F' only differs in
 * how the strings for an infinite number or NaN are printed ('inf', 'infinity'
 * and 'nan' for 'f', 'INF', 'INFINITY' and 'NAN' for 'F'). */
/* Print a double value in standard form ([-]d.ddd e[+/-]ddd). An E conversion
 * uses the letter E (rather than e) to introduce the exponent. The exponent
 * always contains at least two digits; if the value is zero, the exponent is
 * 00. */
/* Print a double in either normal or exponential notation, whichever is more
 * appropriate for its magnitude. 'g' uses lower-case letters, 'G' uses
 * upper-case letters. This type differs slightly from fixed-point notation in
 * that insignificant zeroes to the right of the decimal point are not
 * included. Also, the decimal point is not included on whole numbers. */

// horribly wrong implementation
static const long double pow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
void convert_feg(struct format_output *fo, struct format_info *fi, va_list *ap_ptr) {
	long double value, diff = 0.0, tmp;
	int neg, whole;
	char *alpha = fi->type == 'F' || fi->type == 'G' || fi->type == 'E' ? "NANINFE" : "naninfe";
	const double thres_max = (double)(0x7FFFFFFF);
    uint32_t frac;

	if(fi->length == LENGTH_LONGDOUBLE) {
		value = va_arg((*ap_ptr), long double);
	} else {
		value = (long double)va_arg((*ap_ptr), double);
	}

    if(fi->precision < 0) {
        fi->precision = 0;
    } else if(fi->precision > 9) {
		fi->precision = 9;
    }

    if(value < 0) {
        neg = 1;
        value = -value;
    }
	
    whole = (int)value;
    tmp = (value - whole) * pow10[fi->precision];
    frac = (uint32_t)(tmp);
    diff = tmp - frac;

    if (diff > 0.5) {
        ++frac;
        if (frac >= pow10[fi->precision]) {
            frac = 0;
            ++whole;
        }
    } else if(diff == 0.5 && ((frac == 0) || (frac & 1))) {
        ++frac;
    }

	if(fi->precision == 0) {
		diff = value - whole;
		if (diff > 0.5) {
			++whole;
		} else if (diff == 0.5 && (whole & 1)) {
			++whole;
		}
	}

	FMT_START
		if(!(value == value)) {
			fo_emit(fo, alpha[0]);
			fo_emit(fo, alpha[1]);
			fo_emit(fo, alpha[2]);
			continue;
		}

		if(neg) {
			fo_emit(fo, '-');
		}

		data_fo = *fo;
		
		fo_emit_integer(fo, whole, 10, "0123456789");
		fo_emit(fo, '.');
		fo_emit_integer(fo, frac, 10, "0123456789");
	FMT_END
}

/* Print an unsigned int as a hexadecimal number. 'x' uses lower-case letters
 * and 'X' uses upper-case. */
void convert_x(struct format_output *fo, struct format_info *fi, va_list *ap_ptr) {
	struct number number;
	get_unsigned(&number, fi, ap_ptr);
	print_unsigned(fo, fi, &number, 16, fi->type == 'x' ? "0123456789abcdef" : "0123456789ABCDEF", fi->type == 'x' ? "0x" : "0X");
}

/* Print an unsigned int in octal. */
void convert_o(struct format_output *fo, struct format_info *fi, va_list *ap_ptr) {
	struct number number;
	get_unsigned(&number, fi, ap_ptr);
	print_unsigned(fo, fi, &number, 8, "01234567", "0");
}

/* Print a character string. */
void convert_s(struct format_output *fo, struct format_info *fi, va_list *ap_ptr) {
	const char *arg = va_arg((*ap_ptr), char *);
	const char *tmp = arg;
	unsigned int len = 0;
      
	if (!arg) arg = "";
	else while (*tmp++) ++len;

	if (fi->precision > 0) {  
		if (len > (unsigned int)fi->precision) len = fi->precision;
		if (fi->width > fi->precision) fi->width = fi->precision;
	}

	fi->width -= len;
	if(fi->width < 0) {
		fi->width = 0;
	}
      
	if(!(fi->flag & FLAG_LEFT)) {
		fo_emit_many(fo, ' ', fi->width);
	}
      
	while(len--) {
		fo_emit(fo, *arg++);
	}

	if(fi->flag & FLAG_LEFT) {
		fo_emit_many(fo, ' ', fi->width);
	}
}

/* Print a char (character). */
void convert_c(struct format_output *fo, struct format_info *fi, va_list *ap_ptr) {
	fo_emit(fo, va_arg((*ap_ptr), int));
}

/* Print a void * (pointer to void) in an implementation-defined format. */
void convert_p(struct format_output *fo, struct format_info *fi, va_list *ap_ptr) {
	struct number number;
	if(sizeof(unsigned long long int) == sizeof(uintptr_t)) {
		number.v.longlong = (unsigned long long int)(va_arg((*ap_ptr), void *));
		fi->length = LENGTH_LONGLONG;
	} else {
		number.v.word = (unsigned long int)(va_arg((*ap_ptr), void *));
		fi->length = LENGTH_NONE;
	}
	fi->precision = sizeof(void *) * 2;
	print_unsigned(fo, fi, &number, 16, "0123456789ABCDEF", "0X");
}

/* Print nothing, but write number of characters successfully written so
 * far into an integer pointer parameter. */
void convert_n(struct format_output *fo, struct format_info *fi, va_list *ap_ptr) {
	signed char *cp;
	short *sp;
	int *ip;
	long *lp;
	long long *llp;

	switch(fi->length) {
	case LENGTH_CHAR:
		cp = va_arg((*ap_ptr), signed char *);
		*cp = fo->offset;
		break;
	case LENGTH_SHORT:
		sp = va_arg((*ap_ptr), short *);
		*sp = fo->offset;
		break;
	case LENGTH_NONE:
		ip = va_arg((*ap_ptr), int *);
		*ip = fo->offset;
		break;
	case LENGTH_LONG:
		lp = va_arg((*ap_ptr), long *);
		*lp = fo->offset;
		break;
	case LENGTH_LONGLONG:
		llp = va_arg((*ap_ptr), long long *);
		*llp = fo->offset;
		break;
	}
}

/* Print a literal '%' character (this type doesn't accept any flags, width,
 * precision or length). */
void convert_ident(struct format_output *fo, struct format_info *fi, va_list *ap_ptr) {
	fo_emit(fo, '%');
}

struct {
	char type;
	void (*write)(struct format_output *, struct format_info *, va_list *);
} type_conversions[] = {
	{'d', convert_d},
	{'i', convert_d},
	{'u', convert_u},
	{'f', convert_feg},
	{'F', convert_feg},
	{'e', convert_feg},
	{'E', convert_feg},
	{'g', convert_feg},
	{'G', convert_feg},
	{'x', convert_x},
	{'X', convert_x},
	{'o', convert_o},
	{'s', convert_s},
	{'c', convert_c},
	{'p', convert_p},
	{'n', convert_n},
	{'%', convert_ident},
	{0, 0}
};

size_t qstr_formatv_impl(char *buffer, size_t buffer_length, const char *format, va_list ap) {
	struct format_output fo;
	struct format_info fi;
	int i;

	fo.buffer = buffer;
	fo.length = buffer_length;
	fo.offset = 0;

	while(*format) {
		if(*format != '%') {
			fo_emit(&fo, *format++);
			continue;
		}

		++format;

		fi.width = 0;
		fi.precision = -1;
		fi.pad = ' ';
		fi.flag = FLAG_PARSE;
		fi.length = LENGTH_NONE;

		// syntax is %[parameter][flags][width][.precision][length]type

		// parameter (unused)
		/* n$
		 *  n is the number of the parameter to display using this format
		 *  specifier, allowing the parameters provided to be output multiple
		 *  times, using varying format specifiers or in different orders. This
		 *  is a POSIX extension and not in C99. Example:
		 *  printf("%2$d %1$#x %1$d",16,17) produces "17 0x10 16"
		 */

		// flags
		/* a number (unused)
		 *  Causes printf to left-pad the output with spaces until the required
		 *  length of output is attained. If combined with '0' (see below), it will
		 *  cause the sign to become a space when positive, but the remaining
		 *  characters will be zero-padded
		 * +
		 *  Causes printf to always denote the sign '+' or '-' of a number (the
		 *  default is to omit the sign for positive numbers). Only applicable
		 *  to numeric types.
		 * -
		 *  Causes printf to left-align the output of this placeholder (the
		 *  default is to right-align the output).
		 * #
		 *  Alternate form. For 'g' and 'G', trailing zeros are not removed.
		 *  For 'f', 'F', 'e', 'E', 'g', 'G', the output always contains a
		 *  decimal point. For 'o', 'x', and 'X', a 0, 0x, and 0X,
		 *  respectively, is prepended to non-zero numbers.
		 * 0
		 *  Causes printf to use 0 instead of spaces to left-fill a
		 *  fixed-length field. For example, printf("%2d", 3) results in " 3",
		 *  while printf("%02d", 3) results in "03".
		 */
		while(*format && fi.flag & FLAG_PARSE) {
			switch(*format) {
			case '0': fi.flag |= FLAG_ZERO;     ++format; break;
			case '#': fi.flag |= FLAG_ALT;      ++format; break;
			case '-': fi.flag |= FLAG_LEFT;     ++format; break;
			case '+': fi.flag |= FLAG_SHOWSIGN; ++format; break;
			case ' ': fi.flag |= FLAG_SPACE;    ++format; break;
			default:  fi.flag &= ~FLAG_PARSE;             break;
			}
		}

		// width
		/* a number
		 *  Causes printf to pad the output of this placeholder with spaces
		 *  until it is at least number characters wide. As mentioned above, if
		 *  number has a leading '0', that is interpreted as a flag, and the
		 *  padding is done with '0' characters instead of spaces.
		 * *
		 *  Causes printf to pad the output until it is n characters wide,
		 *  where n is an integer value stored in the a function argument just
		 *  preceding that represented by the modified type. For example
		 *  printf("%*d", 5, 10) will result in "10" being printed with a width
		 *  of 5.
		 */
		if(*format == '*') {
			++format;
			fi.width = va_arg(ap, int);
		} else {
			fi.width = parse_num10(&format);
		}

		// precision
		/* a number
		 *  For non-integral numeric types, causes the decimal portion of the
		 *  output to be expressed in at least number digits. For the string
		 *  type, causes the output to be truncated at number characters. If
		 *  the precision is zero, nothing is printed for the corresponding
		 *  argument.
		 * *
		 *  Same as the above, but uses an integer value in the intaken argument to
		 *  determine the number of decimal places or maximum string length. For
		 *  example, printf("%.*s", 3, "abcdef") will result in "abc" being printed.
		 */
		if(*format == '.') {
			++format;
			if(*format == '*') {
				++format;
				fi.precision = va_arg(ap, int);
			} else {
				fi.precision = parse_num10(&format);
			}
		}

		// length
		/* hh
		 *  For integer types, causes printf to expect an int sized integer argument
		 *  which was promoted from a char.
		 * h
		 *  For integer types, causes printf to expect a int sized integer argument
		 *  which was promoted from a short.
		 * l
		 *  For integer types, causes printf to expect a long sized integer argument.
		 * ll
		 *  For integer types, causes printf to expect a long long sized integer
		 *  argument.
		 * L
		 *  For floating point types, causes printf to expect a long double argument.
		 * z
		 *  For integer types, causes printf to expect a size_t sized integer
		 *  argument.
		 * j
		 *  For integer types, causes printf to expect a intmax_t sized integer
		 *  argument.
		 * t
		 *  For integer types, causes printf to expect a ptrdiff_t sized
		 *  integer argument.
		 */
		if(*format == 'h') {
			++format;
			if(*format == 'h') {
				++format;
				fi.length = LENGTH_CHAR;
			} else {
				fi.length = LENGTH_SHORT;
			}
		} else if(*format == 'l') {
			++format;
			if(*format == 'l') {
				++format;
				fi.length = LENGTH_LONGLONG;
			} else {
				fi.length = LENGTH_LONG;
			}
		} else if(*format == 'L') {
			++format;
			fi.length = LENGTH_LONGDOUBLE;
		} else if(*format == 'z') {
			++format;
			fi.length = sizeof(size_t) > sizeof(unsigned long int) ? LENGTH_LONGLONG :
			            sizeof(size_t) > sizeof(unsigned int)      ? LENGTH_LONG     :
			                                                         LENGTH_NONE     ;
		} else if(*format == 'j') {
			++format;
			fi.length = sizeof(uintmax_t) > sizeof(unsigned long int) ? LENGTH_LONGLONG :
			            sizeof(uintmax_t) > sizeof(unsigned int)      ? LENGTH_LONG     :
			                                                            LENGTH_NONE     ;
		} else if(*format == 't') {
			++format;
			fi.length = sizeof(ptrdiff_t) > sizeof(unsigned long int) ? LENGTH_LONGLONG :
			            sizeof(ptrdiff_t) > sizeof(unsigned int)      ? LENGTH_LONG     :
			                                                            LENGTH_NONE     ;
		}

		// type
		fi.type = *format++;
		for(i = 0; type_conversions[i].type; i++) {
			if(type_conversions[i].type == fi.type) {
				type_conversions[i].write(&fo, &fi, &ap);
				i = 0;
				break;
			}
		}
		if(i) {
			return -1;
		}
	}

	fo_emit(&fo, '\0');

	return fo.offset - 1;
}
