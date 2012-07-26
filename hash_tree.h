// vim: ts=2 sw=2 smarttab

#ifndef COHORT_HASH_TREE_H
#define COHORT_HASH_TREE_H

#include <cmath>
#include <cstdint>


namespace cohort {

	namespace math {
		// avoid floating-point casts involved with pow()
		static inline uint64_t powi(uint64_t base, uint64_t exponent)
		{
			// https://en.wikipedia.org/wiki/Exponentiation_by_squaring
			uint64_t result = 1;
			while (exponent)
			{
				if (exponent & 1)
					result *= base;

				exponent >>= 1;
				base *= base;
			}
			return result;
		}

		// integer logarithm for base=2^n
		static inline uint8_t log2n(uint64_t value, uint8_t n)
		{
			uint8_t result = 0;
			while (value >>= n)
				result++;
			return result;
		}

		// simulate ceil() by adding 1 when there's any remainder
		static inline uint8_t log2n_ceil(uint64_t value, uint8_t n)
		{
			uint8_t base = log2n(value, n);
			if (value != (1ULL << (base * n)))
				base++;
			return base;
		}
	}

	class hash_tree {
		public:
			const uint8_t k; // number of children per node

			hash_tree(uint8_t k) : k(k) {}

			// return the minimum depth required to hold the given number of leaves
			uint8_t depth(uint64_t leaves) const
			{
				// depth = 1 + ceil( logk(leaves) )
				switch (k) {
					// optimization for powers of 2
					case 2:   return 1 + math::log2n_ceil(leaves, 1);
					case 4:   return 1 + math::log2n_ceil(leaves, 2);
					case 8:   return 1 + math::log2n_ceil(leaves, 3);
					case 16:  return 1 + math::log2n_ceil(leaves, 4);
					case 32:  return 1 + math::log2n_ceil(leaves, 5);
					case 64:  return 1 + math::log2n_ceil(leaves, 6);
					case 128: return 1 + math::log2n_ceil(leaves, 7);
										// otherwise use std::log()
										// XXX: precision problems with larger values
					default:
										return 1 + (uint8_t)(std::ceil(
													std::log10((double)leaves) /
													std::log10((double)k)));
				}
			}

			// size: return the number of nodes in a tree of the given depth.
			// each level multiplies the node count by k and adds 1.
			// this can be simplified into the sum, over i=0 to depth-1, of k^i
			uint64_t size(uint64_t depth) const
			{
				// the summation can be further reduced to (1 - k^depth) / (1 - k),
				// but overflows on a greater range of values for 'depth'
				//return (math::powi(k, depth) - 1ULL) / (k - 1ULL);

				uint64_t size = 0;
				while (depth--)
					size += math::powi(k, depth);
				return size;
			}

			// return the root node index, given the depth of the tree
			uint64_t root(uint64_t depth) const
			{
				return depth <= 1 ? 0 : size(depth - 1);
			}

			// return the number of leaves, given the depth of the tree
			uint64_t leaves(uint64_t depth) const
			{
				return math::powi(k, depth);
			}

			// return the nth child node index of the given parent
			uint64_t child_index(uint64_t parent, uint8_t n,
					uint64_t root, uint64_t prev_root) const
			{
				if (parent == root)
				{
					// the left child of a root node is also a root node
					if (n == 0) return prev_root;
					// the second child comes right after the parent
					if (n == 1) return parent + 1;
					// subsequent children come at intervals of root
					return parent * n + 1;
				}
				// the left child comes right after the parent
				if (n == 0) return parent + 1;
				// subsequent children come at intervals of root
				return parent + 1 + n * root;
			}
	};

}

#endif // COHORT_HASH_TREE_H
