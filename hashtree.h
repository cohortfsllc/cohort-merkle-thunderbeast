// vim: ts=2 sw=2 smarttab

#ifndef COHORT_HASH_TREE_H
#define COHORT_HASH_TREE_H

#include <cmath>
#include <cstdint>
#include <iostream>
#include <set>


namespace cohort {

	typedef std::set<uint64_t> blockset;

	// avoid floating point casts involved in calling pow()
	static inline uint64_t powi(uint64_t base, uint64_t exp)
	{
		uint64_t value = 1;
		while (exp--)
			value *= base;
		return value;
	}

	// return the log with base 'NodeWidth'
	template <unsigned NodeWidth>
		static inline double logn(double value)
		{
			// calculate the log divisor once and cache the value
			static const double base_divisor = std::log(static_cast<double>(NodeWidth));
			return std::log(value) / base_divisor;
		}

	// tree_size: return the number of nodes in a tree of the given depth.
	// each successive level multiplies the node count by node_width and adds 1.
	// this can be simplified into the sum, over i=0 to depth-1, of node_width^i
	// the summation can be reduced to (1 - node_width^depth) / (1 - node_width)
	template <unsigned NodeWidth>
		class tree_size {
			public:
				uint64_t operator()(uint64_t depth) const
				{
					uint64_t size = 0;
					while (depth--)
						size += powi(NodeWidth, depth);
					return size;
					//uint64_t size = static_cast<uint64_t>(powi(NodeWidth, depth) - 1ULL);
					//return size / (NodeWidth - 1ULL);
				}
		};

	// for node_width=2, this is further simplified to (2^depth) - 1;
	template <> class tree_size<2> {
		public:
			uint64_t operator()(uint64_t depth) const
			{
				return (1ULL << depth) - 1ULL;
			}
	};


	template <unsigned NodeWidth, unsigned HashSize=20>
		class hash_tree {
			public:
				// return the minimum depth required to hold the given number of leaves
				static uint64_t depth(uint64_t leaves)
				{
					return 1 + static_cast<uint64_t>(std::ceil(
								logn<NodeWidth>(static_cast<double>(leaves))));
				}

				static uint64_t size(uint64_t depth)
				{
					return tree_size<NodeWidth>()(depth);
				}

				// return the root node index, given the total number of leaves
				static uint64_t root(uint64_t leaves)
				{
					return tree_size<NodeWidth>()(leaves) - 1;
				}

				static uint64_t hash_offset(uint64_t node, unsigned index)
				{
					return (node * NodeWidth + index) * HashSize;
				}
		};

}

#endif // COHORT_HASH_TREE_H
