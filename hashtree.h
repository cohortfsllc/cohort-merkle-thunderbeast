// vim: ts=2 sw=2 smarttab

#ifndef COHORT_HASH_TREE_H
#define COHORT_HASH_TREE_H

#include <cmath>
#include <cstdint>
#include <iostream>
#include <set>
#include <algorithm>


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
				uint64_t tree_depth(uint64_t leaves) const
				{
					return 1 + static_cast<uint64_t>(std::ceil(
								logn<NodeWidth>(static_cast<double>(leaves))));
				}

				// return the root node index, given the total number of leaves
				uint64_t tree_root(uint64_t leaves) const
				{
					return tree_size<NodeWidth>()(leaves) - 1;
				}

				uint64_t hash_offset(uint64_t node, unsigned index)
				{
					return (node * NodeWidth + index) * HashSize;
				}

				unsigned update(uint64_t node, uint64_t depth,
						blockset::const_iterator dstart, blockset::const_iterator dend,
						uint64_t bstart, uint64_t bend)
				{
					unsigned count = 0;
					const std::string indent(2 * depth, ' ');

					// base case
					if (depth == 1)
					{
						if (bend - bstart != NodeWidth)
							std::cerr << indent << "error: leaf node " << node
								<< " with " << bend - bstart << " blocks!" << std::endl;

						for (blockset::const_iterator dirty = dstart; dirty != dend; ++dirty)
						{
							uint64_t position = *dirty - bstart;
							std::cout << indent << "block " << *dirty
								<< " hash written to node " << node << "." << position
								<< " at offset " << hash_offset(node, position) << std::endl;
							count++;
						}
						return count;
					}

					// distance between child nodes
					const uint64_t dnodes = tree_size<NodeWidth>()(depth - 1);
					uint64_t child = node - 1 - (NodeWidth - 1) * dnodes;
					// total number of blocks under each child
					const uint64_t dblocks = powi(NodeWidth, depth - 1);

					uint64_t children[NodeWidth] = { 0 };
					for (unsigned i = 0; i < NodeWidth; i++)
					{
						//auto lower = blocks.lower_bound(bstart);
						blockset::const_iterator lower = std::find_if(dstart, dend,
								std::bind2nd(std::greater_equal<uint64_t>(), bstart));
						//auto upper = blocks.upper_bound(bstart + dblocks - 1);
						typedef blockset::const_reverse_iterator rev;
						blockset::const_iterator upper = std::find_if(rev(dend), rev(dstart),
								std::bind2nd(std::less<uint64_t>(), bstart + dblocks)).base();
						if (lower != upper)
						{
							count += update(child, depth - 1,
									lower, upper, bstart, bstart + dblocks);

							children[i] = child + 1;
						}

						child += dnodes;
						bstart += dblocks;
					}

					for (unsigned i = 0; i < NodeWidth; i++)
					{
						if (children[i])
						{
							std::cout << indent << "node " << children[i] - 1
								<< " hash written to node " << node << "." << i
								<< " at offset " << hash_offset(node, i) << std::endl;
							count++;
						}
					}
					return count;
				}

				unsigned update(const blockset &blocks, uint64_t maxblocks)
				{
					uint64_t leaves = maxblocks / NodeWidth +
						(maxblocks % NodeWidth ? 1 : 0);
					uint64_t depth = tree_depth(leaves);

					return update(tree_root(depth), depth,
							blocks.begin(), blocks.end(), 0, leaves - 1);
				}
		};

}

#endif // COHORT_HASH_TREE_H
