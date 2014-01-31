#ifndef COLURE_MARKOV
#define COLURE_MARKOV

#include <stdlib.h>

/**
 * A generic markov chain implementation using N order indexes with next node
 * implemented using something akin to markov(N) == [bytes<N> :: prefix -> [(byte* :: next, int :: weight)]]
 * a hashmap of N bytes to a list of next M bytes and weights. Naturally a hash is made from N bytes and used as an index to the slot
 * a memcmp is used to test the data
 *
 * M must be smaller than N and be able to divide equally, N / M is the order of the makrov model
 */

typedef unsigned char byte;

struct markov {
	unsigned char      N;
	unsigned char      M;
	byte              *start;
	byte              *end;
	size_t             slots;
	struct markovSlot *slot;
};

struct markovSlot {
	size_t             hash;
	byte              *index; // N in length
	unsigned int       total_weight;
	size_t             pairs;
	struct markovPair *pair;
};

struct markovPair {
	size_t        hash;
	byte         *value; // M in length
	unsigned int  weight;
};

struct markov *new_markov(unsigned char N, unsigned char M, byte *start, byte *end);
void markov_build(struct markov *markov, size_t bytes, byte *value);
size_t markov_generate(struct markov *markov, size_t bytes, byte *value);
size_t markov_memory(struct markov *markov);
void markov_free(struct markov *markov);

#endif
