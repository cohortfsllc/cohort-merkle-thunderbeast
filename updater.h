// vim: ts=2 sw=2 smarttab

#ifndef COHORT_UPDATER_H
#define COHORT_UPDATER_H

#include "hashtree.h"
#include <stack>
#include <algorithm>


namespace cohort {

	template <unsigned NodeWidth, unsigned HashSize=20>
		class updater {
			private:
				typedef hash_tree<NodeWidth, HashSize> tree;
				typedef blockset::const_iterator iter;
				typedef blockset::const_reverse_iterator rev;

				struct update
				{
					iter dstart, dend; // bounds of dirty block set
					uint64_t bstart, bend; // bounds of all blocks covered by this node's subtree
					uint64_t node;
					uint64_t depth;
					uint64_t progress; // number of children processed
					uint64_t parent;
					unsigned position; // child's position under parent node
				};
				typedef std::stack<struct update> update_stack;

			public:
				static unsigned update(const blockset &blocks, uint64_t maxblocks)
				{
					uint64_t leaves = maxblocks / NodeWidth +
						(maxblocks % NodeWidth ? 1 : 0);
					uint64_t depth = tree::depth(leaves);
					uint64_t root = tree::root(depth);
					unsigned count = 0;

					// initialize the stack with the root node
					update_stack stack;
					stack.push({ blocks.begin(), blocks.end(),
							0, leaves - 1, root, depth, 0, root + 1, 0 });
					size_t highest_stack = stack.size();

					while (!stack.empty())
					{
						struct update &node = stack.top();

						// base case
						if (node.depth == 1)
						{
							if (node.bend - node.bstart != NodeWidth)
								std::cerr << std::string(2*node.depth, ' ')
									<< "error: leaf node " << node.node
									<< " with " << node.bend - node.bstart << " blocks!" << std::endl;

							for (iter dirty = node.dstart; dirty != node.dend; ++dirty)
							{
								uint64_t position = *dirty - node.bstart;
								std::cout << std::string(2*node.depth, ' ')
								 	<< "block " << *dirty
									<< " hash written to node " << node.node << "." << position
									<< " at offset " << tree::hash_offset(node.node, position) << std::endl;
								count++;
							}

							stack.pop();
						}
						else if (node.progress == NodeWidth)
						{
							std::cout << std::string(2*node.depth, ' ') << "node " << node.node
								<< " hash written to node " << node.node << "." << node.position
								<< " at offset " << tree::hash_offset(node.node, node.position) << std::endl;
							count++;

							stack.pop();
						}
						else
						{
							struct update child;
							child.depth = node.depth - 1;
							child.parent = node.node;
							child.progress = 0;

							// distance between child nodes
							const uint64_t dnodes = tree::size(child.depth);
							// total number of blocks under each child
							const uint64_t dblocks = powi(NodeWidth, child.depth);

							for (; node.progress < NodeWidth; node.progress++)
							{
								child.position = node.progress;
								child.node = child.parent - 1 - dnodes *
									(NodeWidth - node.progress - 1);

								child.bstart = node.bstart + node.progress * dblocks;
								child.bend = child.bstart + dblocks;

								child.dstart = std::find_if(node.dstart, node.dend,
										std::bind2nd(std::greater_equal<uint64_t>(), child.bstart));
								child.dend = std::find_if(rev(node.dend), rev(node.dstart),
										std::bind2nd(std::less<uint64_t>(), child.bend)).base();

								if (child.dstart != child.dend)
								{
									std::cout << std::string(2*node.depth, ' ')
										<< "(node " << child.parent
										<< " pushing child " << child.node << ")" << std::endl;
									stack.push(child);
									highest_stack = std::max(highest_stack, stack.size());
									node.progress++;
									break;
								}
							}
						}
					}
					std::cout << "highest stack: " << highest_stack << std::endl;
					return count;
				}
		};

}

#endif // COHORT_UPDATER_H
