// vim: ts=8 sw=2 smarttab
#ifndef COHORT_HASH_TREE_H
#define COHORT_HASH_TREE_H

#include <cmath>
#include <cstdint>


namespace cohort {

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
			/*uint64_t size = 0;
			while (depth--)
				size += powi(NodeWidth, depth);
			return size;*/
			uint64_t size = static_cast<uint64_t>(1ULL - powi(NodeWidth, depth));
			return size / (1ULL - NodeWidth);
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


	template <unsigned NodeWidth>
	class hash_tree {
	public:
		// return the index of the leaf node that corresponds to the given block
		// by counting the nodes in all subtrees to the left of this leaf.
		uint64_t find_leaf(uint64_t b) const
		{
			uint64_t node = 0;
			while (b) {
				// depth of the previous subtree
				uint64_t depth = 1 + static_cast<uint64_t>(logn<NodeWidth>(static_cast<double>(b)));
				// number of nodes in the previous subtree
				node += tree_size<NodeWidth>()(depth);
				// number of leaves in the previous subtree
				b -= powi(NodeWidth, depth - 1);
			}
			return node;
		}

	public:
	};

}

#endif // COHORT_HASH_TREE_H
