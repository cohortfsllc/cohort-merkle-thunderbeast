#ifndef COHORT_MERKLE_TREE_H
#define COHORT_MERKLE_TREE_H

#include <math.h>
#include <stdint.h>


/* merkle tree calculations */

/* integer logarithm for base=2^n */
static inline uint8_t log2n(uint64_t value, uint8_t n)
{
	uint8_t result = 0;
	while (value >>= n)
		result++;
	return result;
}

/* simulate ceil() by adding 1 when there's any remainder */
static inline uint8_t log2n_ceil(uint64_t value, uint8_t n)
{
	uint8_t base = log2n(value, n);
	if (value != (1ULL << (base * n)))
		base++;
	return base;
}


/* return the minimum depth required to hold the given number of leaves */
static inline uint8_t merkle_depth(uint8_t k, uint64_t leaves)
{
	/* depth = 1 + ceil( logk(leaves) ) */
	switch (k) {
		/* optimization for powers of 2 */
		case 2:   return 1 + log2n_ceil(leaves, 1);
		case 4:   return 1 + log2n_ceil(leaves, 2);
		case 8:   return 1 + log2n_ceil(leaves, 3);
		case 16:  return 1 + log2n_ceil(leaves, 4);
		case 32:  return 1 + log2n_ceil(leaves, 5);
		case 64:  return 1 + log2n_ceil(leaves, 6);
		case 128: return 1 + log2n_ceil(leaves, 7);
	}
	/* otherwise use log10() and convert the base
	 * XXX: precision problems with larger values */
	return 1 + (uint8_t)(ceil(
				log10((double)leaves) /
				log10((double)k)));
}

/* return the nth child node index of the given parent */
static inline uint64_t merkle_child(uint64_t parent, uint8_t n,
		uint64_t root, uint64_t prev_root)
{
	if (parent == root)
	{
		/* the left child of a root node is also a root node */
		if (n == 0) return prev_root;
		/* the second child comes right after the parent */
		if (n == 1) return parent + 1;
		/* subsequent children come at intervals of root */
		return parent * n + 1;
	}
	/* the left child comes right after the parent */
	if (n == 0) return parent + 1;
	/* subsequent children come at intervals of root */
	return parent + 1 + n * root;
}

#endif /* COHORT_MERKLE_TREE_H */
