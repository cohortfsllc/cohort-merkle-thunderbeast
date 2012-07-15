// vim: ts=8 sw=2 smarttab
#ifndef COHORT_HASH_TREE_H
#define COHORT_HASH_TREE_H

#include <cmath>
#include <cstdint>


namespace cohort {

	template <unsigned int NodeWidth>
	class hash_tree {
	public:
		// number of child hashes stored in each node
		static const unsigned int node_width = NodeWidth;

		typedef std::uint64_t block_t;
		typedef std::uint64_t node_t;

	private:
		// return the log with base 'node_width'
		double logn(double value) const
		{
			// calculate the log divisor once and cache the value
			static const double base_divisor = std::log(static_cast<double>(node_width));
			return std::log(value) / base_divisor;
		}

		// avoid floating point casts involved in calling pow()
		node_t powi(node_t base, node_t exp) const
		{
			node_t value = 1;
			while (exp--)
				value *= base;
			return value;
		}

	public:
		// return the number of nodes in a tree of the given depth.
		// each successive level multiplies the node count by node_width and adds 1.
		// this can be simplified into the sum, over i=0 to depth-1, of node_width^i
		node_t tree_size(node_t depth) const
		{
			// TODO: specialize for node_width=2: return powi(node_width, depth) - 1;
			node_t size = 0;
			while (depth--)
				size += powi(node_width, depth);
			return size;
		}

		// return the index of the leaf node that corresponds to the given block
		// by counting the nodes in all subtrees to the left of this leaf.
		node_t find_leaf(block_t b) const
		{
			node_t node = 0;
			while (b) {
				// depth of the previous subtree
				node_t depth = 1 + static_cast<node_t>(logn(static_cast<double>(b)));
				// number of nodes in the previous subtree
				node += tree_size(depth);
				// number of leaves in the previous subtree
				b -= powi(node_width, depth - 1);
			}
			return node;
		}

	public:
	};

}

#endif // COHORT_HASH_TREE_H
