#include "qstr-internal.h"

#include <stdint.h>

#include <stdlib.h>
#include <string.h>

// any managed string will have a len len defined, have at least 1
static const unsigned char encoded_value_bytes[4] = {1, 2, 3, sizeof(size_t)};

static size_t next_pow2(size_t n) {
	n--;
	n |= n >>  1;
	n |= n >>  2;
	n |= n >>  4;
	n |= n >>  8;
	n |= n >>  16;
	n++;
	return n;
}

static size_t decode_value(unsigned char bits, unsigned char *data) {
	size_t result = 0;

	// 4 and 8 are the possible sizes of sizeof(size_t)
	switch(encoded_value_bytes[bits & 0x03]) {
#if 0
	case 8:
		result |= (((size_t)data[7]) << 56);
		result |= (((size_t)data[6]) << 48);
		result |= (((size_t)data[5]) << 40);
		result |= (((size_t)data[4]) << 32);
#endif
	case 4:
		result |= (((size_t)data[3]) << 24);
	case 3:
		result |= (((size_t)data[2]) << 16);
	case 2:
		result |= (((size_t)data[1]) <<  8);
	case 1:
		result |= (((size_t)data[0]) <<  0);
		return result;
	default:
		return 0;
	}
}

static void encode_value(size_t value, unsigned char *bitsptr, unsigned char *data) {
	unsigned char bits = (value >= (1 << 24)) ? 3 :
		                 (value >= (1 << 16)) ? 2 :
		                 (value >= (1 <<  8)) ? 1 : 0;

	if(bitsptr) {
		*bitsptr = bits;
	}

	if(!data) {
		return;
	}

	switch(encoded_value_bytes[bits]) {
#if 0
	case 8:
		data[7] = (value >> 57) & 0xff;
		data[6] = (value >> 48) & 0xff;
		data[5] = (value >> 40) & 0xff;
		data[4] = (value >> 32) & 0xff;
#endif
	case 4:
		data[3] = (value >> 24) & 0xff;
	case 3:
		data[2] = (value >> 16) & 0xff;
	case 2:
		data[1] = (value >>  8) & 0xff;
	case 1:
		data[0] = (value >>  0) & 0xff;
	default:
		break;
	}
}

struct qstr {
	unsigned char data[1];
	/* mm00 rrll
	 * mm        - string is managed, always 10xx xxxx, takes these two bits because its an invaild utf8 string
	 *   e       - allocation error
	 *    s      - appending mode, allocated length is next power of two
	 *      rr   - reference bytes
	 *        ll - length bytes
	 */
};

#define QSTR_BITS_MANAGED (1 << 7)
#define QSTR_BITS_MANAGED_MASK (3 << 6)
#define QSTR_BITS_APPEND (1 << 4)
#define QSTR_BITS_REF_MASK (3 << 2)
#define QSTR_BITS_LEN_MASK (3 << 0)

#define QSTR_MANAGED(qstr) (((qstr)->data[0] & QSTR_BITS_MANAGED_MASK) == QSTR_BITS_MANAGED)
#define QSTR_APPEND_MODE(qstr) (((qstr)->data[0] & QSTR_BITS_APPEND) == QSTR_BITS_APPEND)
#define QSTR_REF_LEN(qstr) encoded_value_bytes[((qstr)->data[0]>>2) & 0x03]
#define QSTR_REFS(qstr) (decode_value(((qstr)->data[0] >> 2) & 0x03, (qstr)->data + 1))
#define QSTR_LEN_LEN(qstr) encoded_value_bytes[(qstr)->data[0] & 0x03]
#define QSTR_LEN(qstr) (decode_value((qstr)->data[0] & 0x03, (qstr)->data + 1 + QSTR_REF_LEN(qstr)))
#define QSTR_DATA(qstr) ((char *)(qstr->data + 1 + QSTR_REF_LEN(qstr) + QSTR_LEN_LEN(qstr)))
#define QSTR_SIZE(qstr) (QSTR_APPEND_MODE(qstr) ? QSTR_LEN(qstr) + 1 : next_pow2(QSTR_LEN(qstr) + 1))

static struct qstr *new_qstr_empty(int append, size_t refs, size_t size) {
	struct qstr *result;
	unsigned char len_bits;
	unsigned char ref_bits;
	size_t msize;
	
	if(size == 0) {
		return QSTR("");
	}

	msize = append ? next_pow2(size + 1) : size + 1;
	
	encode_value(msize, &len_bits, NULL);
	encode_value(refs, &ref_bits, NULL);

	result = (struct qstr *)malloc(1 + encoded_value_bytes[ref_bits] + encoded_value_bytes[len_bits] + msize);
	memset(result, 0, 1 + encoded_value_bytes[ref_bits] + encoded_value_bytes[len_bits] + msize);
	encode_value(refs, NULL, result->data + 1);
	encode_value(size, NULL, result->data + 1 + encoded_value_bytes[ref_bits]);
	result->data[0] = QSTR_BITS_MANAGED | len_bits | (ref_bits << 2);

	if(append) {
		result->data[0] |= QSTR_BITS_APPEND;
	}

	return result;
}

static struct qstr *new_qstr(int append, size_t refs, size_t size, const char *data) {
	struct qstr *result;
	
	if(size == 0) {
		return QSTR("");
	}

	result = new_qstr_empty(append, refs, size);

	memcpy(QSTR_DATA(result), data, size);

	return result;
}

static struct qstr *qstr_add_undef(struct qstr *qstr, size_t size) {
	size_t len = qstr_len(qstr);
	struct qstr *result;

	if(QSTR_MANAGED(qstr) && QSTR_APPEND_MODE(qstr) && (len + size < QSTR_SIZE(qstr)) && QSTR_REFS(qstr) == 1) {
		encode_value(len + size, NULL, qstr->data + 1 + QSTR_REF_LEN(qstr));
		return qstr;
	}

	// we may be appending more, get into the appending mood
	result = new_qstr_empty(1, 1, len + size);
	memcpy(QSTR_DATA(result), qstr_cstr(qstr), len);
	qstr_free(qstr);
	return result;
}


static void qstr_ref_dec(struct qstr *qstr) {
	size_t refs = decode_value((qstr->data[0] >> 2) & 0x03, qstr->data + 1);
	if(refs == 1) {
		free(qstr);
		return;
	}
	encode_value(refs - 1, NULL, qstr->data + 1);
}

static struct qstr *qstr_ref_inc(struct qstr *qstr) {
	size_t refs = decode_value((qstr->data[0] >> 2) & 0x03, qstr->data + 1);
	if(refs + 1 > ((size_t)1 << QSTR_REF_LEN(qstr)*8)) {
		return new_qstr(QSTR_APPEND_MODE(qstr), refs + 1, qstr_len(qstr), qstr_cstr(qstr));
	}
	encode_value(refs + 1, NULL, qstr->data + 1);
	return qstr;
}

size_t qstr_len(struct qstr *qstr) {
	if(QSTR_MANAGED(qstr)) {
		return QSTR_LEN(qstr);
	} else {
		// x86 only

		register size_t len = 0;
		const char *str = (const char *)qstr;
		register const uint32_t *c;

		// align pointer to 32 bits
		while((((intptr_t)str) & (sizeof(unsigned int)-1)) != 0) {
			if(*str++ == 0)
				return len;
			++len;
		}

		// scan for a null byte
		c = (const uint32_t *)str;
		while(1) {
			register uint32_t x = *c;
			if((x & 0x000000FFu) == 0) return len;
			if((x & 0x0000FF00u) == 0) return len + 1;
			if((x & 0x00FF0000u) == 0) return len + 2;
			if((x & 0xFF000000u) == 0) return len + 3;
			++c;
			len += sizeof(uint32_t);
		}
	}
}

const char *qstr_cstr(struct qstr *qstr) {
	return QSTR_MANAGED(qstr) ? QSTR_DATA(qstr) : (const char *)qstr;
}

struct qstr *qstr_dup(struct qstr *qstr) {
	if(QSTR_MANAGED(qstr)) {
		return qstr_ref_inc(qstr);
	} else {
		return new_qstr(0, 1, qstr_len(qstr), qstr_cstr(qstr));
	}
}

void qstr_free(struct qstr *qstr) {
	if(QSTR_MANAGED(qstr)) {
		qstr_ref_dec(qstr);
	}
}


struct qstr *qstr_substring(struct qstr *qstr, size_t start, size_t length) {
	size_t len = qstr_len(qstr);
	if(start >= len) {
		return QSTR("");
	}
	return new_qstr(0, 1, start + length < len ? length : len - start, qstr_cstr(qstr) + start);
}

int qstr_cmp(struct qstr *q1, struct qstr *q2) {
	const char *s1 = qstr_cstr(q1);
	const char *s2 = qstr_cstr(q2);
	while(*s1 == *s2++) {
		if(*s1++ == 0) {
			return 0;
		}
	}
	return *(unsigned char *)s1 - *(unsigned char *)--s2;
}

// from
struct qstr *qstr_from_buf(const char *data, size_t size) {
	return new_qstr(0, 1, size, data);
}

struct qstr *qstr_from_formatv(const char *format, va_list ap) {
	struct qstr *result;
	size_t length = 0;
	va_list start_ap = ap;
	length = qstr_formatv_impl(NULL, 0, format, ap);
	if(length == 0) {
		return QSTR("");
	}
	result = new_qstr_empty(0, 1, length);
	qstr_formatv_impl(QSTR_DATA(result), length + 1, format, start_ap);
	return result;
}

struct qstr *qstr_from_format(const char *format, ...) {
	struct qstr *result;
	va_list ap;
	va_start(ap, format);
	result = qstr_from_formatv(format, ap);
	va_end(ap);
	return result;
}

// add
struct qstr *qstr_add_buf(struct qstr *qstr, const char *data, size_t size) {
	size_t len = qstr_len(qstr);
	qstr = qstr_add_undef(qstr, size);
	memcpy(QSTR_DATA(qstr) + len, data, size);
	return qstr;
}

struct qstr *qstr_add_qstr(struct qstr *qstr, struct qstr *str) {
	return qstr_add_buf(qstr, qstr_cstr(str), qstr_len(str));
}

struct qstr *qstr_add_str(struct qstr *qstr, const char *str) {
	return qstr_add_buf(qstr, str, qstr_len(QSTR(str)));
}

struct qstr *qstr_add_formatv(struct qstr *qstr, const char *format, va_list ap) {
	size_t len = qstr_len(qstr);
	size_t size = qstr_formatv_impl(NULL, 0, format, ap);
	va_list start_ap = ap;
	qstr = qstr_add_undef(qstr, size);
	qstr_formatv_impl(QSTR_DATA(qstr) + len, size, format, start_ap);
	return qstr;
}

struct qstr *qstr_add_format(struct qstr *qstr, const char *format, ...) {
	struct qstr *result;
	va_list ap;
	va_start(ap, format);
	result = qstr_add_formatv(qstr, format, ap);
	va_end(ap);
	return result;
}
