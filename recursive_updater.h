// vim: ts=2 sw=2 smarttab

#ifndef COHORT_RECURSIVE_UPDATER_H
#define COHORT_RECURSIVE_UPDATER_H

#include "hashtree.h"
#include <algorithm>


namespace cohort {

	template <unsigned NodeWidth, unsigned HashSize=20>
		class recursive_updater {
			private:
				typedef hash_tree<NodeWidth, HashSize> tree;
				typedef blockset::const_iterator iter;
				typedef blockset::const_reverse_iterator rev;

				static unsigned update_node(uint64_t node, uint64_t depth,
						iter dstart, iter dend, uint64_t bstart, uint64_t bend)
				{
					unsigned count = 0;
					const std::string indent(2 * depth, ' ');

					// base case
					if (depth == 1)
					{
						if (bend - bstart != NodeWidth)
							std::cerr << indent << "error: leaf node " << node
								<< " with " << bend - bstart << " blocks!" << std::endl;

						for (iter dirty = dstart; dirty != dend; ++dirty)
						{
							uint64_t position = *dirty - bstart;
							std::cout << indent << "block " << *dirty
								<< " hash written to node " << node << "." << position
								<< " at offset " << tree::hash_offset(node, position) << std::endl;
							count++;
						}
						return count;
					}

					// distance between child nodes
					const uint64_t dnodes = tree::size(depth - 1);
					uint64_t child = node - 1 - (NodeWidth - 1) * dnodes;
					// total number of blocks under each child
					const uint64_t dblocks = powi(NodeWidth, depth - 1);

					uint64_t children[NodeWidth] = { 0 };
					for (unsigned i = 0; i < NodeWidth; i++)
					{
						//iter lower = blocks.lower_bound(bstart);
						iter lower = std::find_if(dstart, dend,
								std::bind2nd(std::greater_equal<uint64_t>(), bstart));
						//iter upper = blocks.upper_bound(bstart + dblocks - 1);
						iter upper = std::find_if(rev(dend), rev(dstart),
								std::bind2nd(std::less<uint64_t>(), bstart + dblocks)).base();
						if (lower != upper)
						{
							count += update_node(child, depth - 1,
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
								<< " at offset " << tree::hash_offset(node, i) << std::endl;
							count++;
						}
					}
					return count;
				}

			public:
				static unsigned update(const blockset &blocks, uint64_t maxblocks)
				{
					uint64_t leaves = maxblocks / NodeWidth +
						(maxblocks % NodeWidth ? 1 : 0);
					uint64_t depth = tree::depth(leaves);

					return update_node(tree::root(depth), depth,
							blocks.begin(), blocks.end(), 0, leaves - 1);
				}
		};

}

#endif // COHORT_RECURSIVE_UPDATER_H
