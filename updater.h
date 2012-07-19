// vim: ts=2 sw=2 smarttab

#ifndef COHORT_UPDATER_H
#define COHORT_UPDATER_H

#include "hashtree.h"
#include "hash_file.h"
#include "hasher.h"

#include <iostream>
#include <set>
#include <stack>
#include <algorithm>


namespace cohort {

	// updater input: a set of dirty blocks
	typedef std::set<uint64_t> blockset;

	// updater: perform a depth-first traversal of the tree, ignoring
	//  nodes that don't contain any dirty blocks.
	class updater {
		private:
			const hash_tree &tree;
			hash_file &file;
			hasher &hash;

			typedef blockset::const_iterator iter;
			typedef blockset::const_reverse_iterator rev;

			struct update
			{
				iter dstart, dend; // bounds of dirty block set
				uint64_t bstart, bend; // bounds of all blocks covered by this node's subtree
				uint64_t node, parent;
				uint64_t dirty; // bitmask of children traversed. XXX: only supports k <= 64
				uint8_t progress; // number of children processed
				uint8_t position; // child's position under parent node [0, k-1]
			};
			typedef std::stack<struct update> update_stack;

			void hash_node(const struct update &node, uint64_t depth)
			{
				uint64_t read_offset = hasher::DIGEST_SIZE * node.node * tree.k;
				uint64_t write_offset = hasher::DIGEST_SIZE *
					(node.parent * tree.k + node.position);

				std::cout << std::string(2*depth, ' ')
					<< "node " << node.node << " at " << read_offset
					<< " hash written to node " << node.parent << "." << (uint32_t)node.position
					<< " at offset " << write_offset << std::endl;

				hash.init();
				hash.process(file.read(read_offset), tree.k * hasher::DIGEST_SIZE);
				hash.finish(file.write(write_offset));
			}

			void hash_block(uint64_t block, unsigned position, const struct update &node)
			{
				uint64_t write_offset = hasher::DIGEST_SIZE *
					(node.node * tree.k + position);

				std::cout << "block " << block
					<< " hash written to node " << node.node << "." << position
					<< " at offset " << write_offset << std::endl;

				unsigned char *buffer = file.write(write_offset);
				std::fill(buffer, buffer + hasher::DIGEST_SIZE, block & 0xFF);
			}

		public:
			updater(const hash_tree &tree, hash_file &file, hasher &hash)
				: tree(tree), file(file), hash(hash)
			{
			}

			unsigned update(const blockset &blocks, uint64_t maxblocks)
			{
				const uint64_t leaves = maxblocks / tree.k +
					(maxblocks % tree.k ? 1 : 0);
				uint64_t depth = tree.depth(leaves);
				unsigned count = 0;

				// initialize the stack and push the root node
				update_stack stack;

				struct update root;
				root.dstart = blocks.begin();
				root.dend = blocks.end();
				root.bstart = 0;
				root.bend = leaves - 1;
				root.node = tree.root(depth);
				root.parent = root.node + 1;
				root.progress = 0;
				root.dirty = 0;
				root.position = 0;
				stack.push(root);

				// make sure the file can hold the entire hash tree
				file.resize(hasher::DIGEST_SIZE * (root.parent * tree.k + 1));

				while (!stack.empty())
				{
					struct update &node = stack.top();

					// base case
					if (depth == 1)
					{
						if (node.bend - node.bstart != tree.k)
							std::cerr << std::string(2*depth, ' ')
								<< "error: leaf node " << node.node
								<< " with " << node.bend - node.bstart << " blocks!" << std::endl;

						for (iter dirty = node.dstart; dirty != node.dend; ++dirty)
						{
							hash_block(*dirty, *dirty - node.bstart, node);
							count++;
						}

						depth++;
						stack.pop();
					}
					else
					{
						struct update child;
						child.parent = node.node;
						child.progress = 0;
						child.dirty = 0;

						// distance between child nodes
						const uint64_t dnodes = tree.size(depth - 1);

						if (node.progress == tree.k)
						{
							// all child nodes have been processed
							for (child.position = 0; child.position < tree.k; child.position++)
							{
								// update hash for any dirty children
								if (node.dirty & (1ULL << child.position))
								{
									child.node = child.parent - 1 - dnodes *
										(tree.k - child.position - 1);
									hash_node(child, depth - 1);
									count++;
								}
							}

							depth++;
							stack.pop();
							continue;
						}

						// total number of blocks under each child
						const uint64_t dblocks = math::powi(tree.k, depth - 1);

						for (; node.progress < tree.k; node.progress++)
						{
							child.position = node.progress;
							child.node = child.parent - 1 - dnodes *
								(tree.k - node.progress - 1);

							child.bstart = node.bstart + node.progress * dblocks;
							child.bend = child.bstart + dblocks;

							child.dstart = std::find_if(node.dstart, node.dend,
									std::bind2nd(std::greater_equal<uint64_t>(), child.bstart));
							child.dend = std::find_if(rev(node.dend), rev(node.dstart),
									std::bind2nd(std::less<uint64_t>(), child.bend)).base();

							if (child.dstart != child.dend)
							{
								depth--;
								stack.push(child);
								node.dirty |= (1ULL << node.progress);
								node.progress++;
								break;
							}
						}
					}
				}

				// write final hash of root node
				hash_node(root, depth);
				return count;
			}
	};

}

#endif // COHORT_UPDATER_H
