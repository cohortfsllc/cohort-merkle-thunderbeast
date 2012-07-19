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
	}

	class hash_tree {
		public:
			const uint8_t k; // number of children per node

			hash_tree(uint8_t k) : k(k) {}

			// return the minimum depth required to hold the given number of leaves
			uint64_t depth(uint64_t leaves) const
			{
				return 1 + (uint64_t)(std::ceil(
							std::log((double)leaves) / std::log((double)k)));
			}

			// size: return the number of nodes in a tree of the given depth.
			// each level multiplies the node count by node_width and adds 1.
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
				// the root node has the highest index of its subtree
				return size(depth) - 1;
			}
	};

}

#endif // COHORT_HASH_TREE_H
